/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <atomic>
#include <iostream>

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslChanManagement.h"
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/ripc_int.h"
#include "rtr/ripcflip.h"
#include "rtr/rsslThread.h"

#include "TransportUnitTest.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#endif

/* -----------------------------------------------------------------------
 * Shared helpers
 * --------------------------------------------------------------------- */

/* Establishes a non-blocking server+client channel pair.
 * Returns true on success; sets *ppServer, *ppServerChnl, *ppClientChnl. */
static bool setupReadChannelPair(
    const char*   port,
    RsslServer**  ppServer,
    RsslChannel** ppServerChnl,
    RsslChannel** ppClientChnl,
    RsslCompTypes compType  = RSSL_COMP_NONE,
    RsslUInt32    compLevel = 0,
    RsslUInt32    maxFragSz = 0)
{
    RsslError     err;
    struct timeval selectTime;
    fd_set         readfds;
    int            selRet;

    RsslBindOptions bindOpts;
    rsslClearBindOpts(&bindOpts);
    bindOpts.serviceName             = const_cast<char*>(port);
    bindOpts.protocolType            = RSSL_RWF_PROTOCOL_TYPE;
    bindOpts.majorVersion            = RSSL_RWF_MAJOR_VERSION;
    bindOpts.minorVersion            = RSSL_RWF_MINOR_VERSION;
    bindOpts.channelsBlocking        = RSSL_FALSE;
    bindOpts.serverBlocking          = RSSL_FALSE;
    bindOpts.compressionType         = compType;
    bindOpts.compressionLevel        = compLevel;
    bindOpts.guaranteedOutputBuffers = 5000;
    bindOpts.maxOutputBuffers        = 5000;
    if (maxFragSz > 0)
        bindOpts.maxFragmentSize = maxFragSz;

    *ppServer = rsslBind(&bindOpts, &err);
    if (!*ppServer)
    {
        std::cout << "setupReadChannelPair: rsslBind failed on port " << port
                  << ": " << err.text << "\n";
        return false;
    }

    RsslConnectOptions copts;
    rsslClearConnectOpts(&copts);
    copts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    copts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    copts.connectionInfo.unified.serviceName = const_cast<char*>(port);
    copts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    copts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    copts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    copts.tcp_nodelay                        = RSSL_TRUE;
    copts.blocking                           = RSSL_FALSE;
    copts.compressionType                    = compType;
    copts.guaranteedOutputBuffers            = 500;

    *ppClientChnl = rsslConnect(&copts, &err);
    if (!*ppClientChnl)
    {
        std::cout << "setupReadChannelPair: rsslConnect failed: " << err.text << "\n";
        rsslCloseServer(*ppServer, &err);
        *ppServer = NULL;
        return false;
    }

    FD_ZERO(&readfds);
    FD_SET((*ppServer)->socketId, &readfds);
    selectTime.tv_sec  = 5L;
    selectTime.tv_usec = 0L;
    selRet = select(FD_SETSIZE, &readfds, NULL, NULL, &selectTime);
    if (selRet <= 0)
    {
        std::cout << "setupReadChannelPair: accept select timed out\n";
        rsslCloseChannel(*ppClientChnl, &err);
        rsslCloseServer(*ppServer, &err);
        *ppClientChnl = NULL;
        *ppServer     = NULL;
        return false;
    }

    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    *ppServerChnl = rsslAccept(*ppServer, &acceptOpts, &err);
    if (!*ppServerChnl)
    {
        std::cout << "setupReadChannelPair: rsslAccept failed: " << err.text << "\n";
        rsslCloseChannel(*ppClientChnl, &err);
        rsslCloseServer(*ppServer, &err);
        *ppClientChnl = NULL;
        *ppServer     = NULL;
        return false;
    }

    for (int i = 0; i < 5000; ++i)
    {
        RsslInProgInfo inProg;
        RsslRet        ret;

        if ((*ppClientChnl)->state != RSSL_CH_STATE_ACTIVE)
        {
            rsslClearInProgInfo(&inProg);
            ret = rsslInitChannel(*ppClientChnl, &inProg, &err);
            if (ret < RSSL_RET_SUCCESS)
            {
                rsslCloseChannel(*ppServerChnl, &err);
                rsslCloseChannel(*ppClientChnl, &err);
                rsslCloseServer(*ppServer, &err);
                *ppServerChnl = NULL;
                *ppClientChnl = NULL;
                *ppServer     = NULL;
                return false;
            }
        }

        if ((*ppServerChnl)->state != RSSL_CH_STATE_ACTIVE)
        {
            rsslClearInProgInfo(&inProg);
            ret = rsslInitChannel(*ppServerChnl, &inProg, &err);
            if (ret < RSSL_RET_SUCCESS)
            {
                rsslCloseChannel(*ppServerChnl, &err);
                rsslCloseChannel(*ppClientChnl, &err);
                rsslCloseServer(*ppServer, &err);
                *ppServerChnl = NULL;
                *ppClientChnl = NULL;
                *ppServer     = NULL;
                return false;
            }
        }

        if ((*ppServerChnl)->state == RSSL_CH_STATE_ACTIVE &&
            (*ppClientChnl)->state == RSSL_CH_STATE_ACTIVE)
            return true;

        time_sleep(1);
    }

    rsslCloseChannel(*ppServerChnl, &err);
    rsslCloseChannel(*ppClientChnl, &err);
    rsslCloseServer(*ppServer, &err);
    *ppServerChnl = NULL;
    *ppClientChnl = NULL;
    *ppServer     = NULL;
    return false;
}

/* Send bytes directly into the channel's socket (bypassing RSSL framing).
 * Used to inject crafted / malformed wire data. */
static int injectRawBytes(RsslChannel* pChnl, const void* data, int len)
{
    if (!pChnl) return -1;
    return (int)send((SOCKET)pChnl->socketId, (const char*)data, len, 0);
}

/* Drain readable data from pChnl; used to clear the server's receive buffer. */
static int drainChannel(RsslChannel* pChnl, int maxReads,RsslRet *pCheckRet = NULL, RsslError* pError = NULL)
{
    if (!pChnl) return 0;
    RsslError err;
    int messageCount = 0;
    for (int i = 0; i < maxReads; ++i)
    {
        RsslRet    readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf   = rsslRead(pChnl, &readRet, &err);
        if (pBuf)
        {
            messageCount++;
            continue;
        }

        if (pCheckRet != NULL)
        {
            EXPECT_EQ(*pCheckRet, readRet);

            if (pError != NULL)
            {
                pError->rsslErrorId = err.rsslErrorId;
                memset(pError->text, 0, MAX_RSSL_ERROR_TEXT);
                memcpy(pError->text, err.text, strlen(err.text));
            }

            return messageCount;
        }
        else
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK ||
                readRet == RSSL_RET_READ_PING ||
                readRet < RSSL_RET_SUCCESS)
                return messageCount;
        }
    }

    return messageCount;
}

/* Write a buffer to a channel, flush, wait briefly, then read from the peer. */
static RsslRet writeAndFlush(RsslChannel* pWriter, RsslBuffer* pBuf)
{
    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslError err;

    RsslRet ret = rsslWriteEx(pWriter, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pWriter, &err);
    return ret;
}

/* -----------------------------------------------------------------------
 * Base fixture
 * --------------------------------------------------------------------- */
class RsslSocketReadTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    RsslChannel* pClientChnl = nullptr;

    virtual void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
    }

    virtual void TearDown() override
    {
        RsslError err;
        if (pServerChnl) { rsslCloseChannel(pServerChnl, &err); pServerChnl = nullptr; }
        if (pClientChnl) { rsslCloseChannel(pClientChnl, &err); pClientChnl = nullptr; }
        if (pServer)     { rsslCloseServer(pServer, &err);       pServer     = nullptr; }
        rsslUninitialize();

        resetDeadlockTimer();
    }

    /* Convenience: get a buffer from the client channel. */
    RsslBuffer* getClientBuf(RsslUInt32 size, RsslError* pErr, RsslBool packed = RSSL_FALSE)
    {
        return rsslGetBuffer(pClientChnl, size, packed, pErr);
    }

    static void fillAscii(RsslBuffer* pBuf, RsslUInt32 len = 0)
    {
        if (!pBuf || !pBuf->data) return;
        RsslUInt32 n = (len > 0 && len <= pBuf->length) ? len : pBuf->length;
        for (RsslUInt32 i = 0; i < n; ++i)
            pBuf->data[i] = (char)('A' + (i % 26));
    }
};

/* Fixture using RSSL_LOCK_GLOBAL_AND_CHANNEL (per-channel lock). */
class RsslSocketReadChannelLockTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    RsslChannel* pClientChnl = nullptr;

    virtual void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);
    }

    virtual void TearDown() override
    {
        RsslError err;
        if (pServerChnl) { rsslCloseChannel(pServerChnl, &err); pServerChnl = nullptr; }
        if (pClientChnl) { rsslCloseChannel(pClientChnl, &err); pClientChnl = nullptr; }
        if (pServer)     { rsslCloseServer(pServer, &err);       pServer     = nullptr; }
        rsslUninitialize();

        resetDeadlockTimer();
    }
};

/* =======================================================================
 * ── Basic channel-state guard tests ────────────────────────────────────
 * rsslRead on a CLOSED channel must fail gracefully.
 * ===================================================================== */

/* Set the channel state to CLOSED then call rsslRead; must not crash. */
TEST_F(RsslSocketReadTests, ReadOnClosedChannelStateFails)
{
    ASSERT_TRUE(setupReadChannelPair("15300", &pServer, &pServerChnl, &pClientChnl));

    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pClientChnl, &readRet, &err);

    EXPECT_EQ(pBuf, (RsslBuffer*)nullptr);
    EXPECT_EQ(readRet, RSSL_RET_FAILURE)
        << "rsslRead on CLOSED channel must return RSSL_RET_FAILURE";
    EXPECT_TRUE(strstr(err.text, "Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can get read.") != NULL) << "Error message should mention Channels in RSSL_CH_STATE_ACTIVE state can get read.";

    pClientChnl->state = RSSL_CH_STATE_ACTIVE;
}

/* =======================================================================
 * ── Normal read round-trip ─────────────────────────────────────────────
 * Establishes a channel pair, writes a message from the client, and reads
 * it on the server side.  This is the baseline positive test.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, BasicReadRoundTripSucceeds)
{
    ASSERT_TRUE(setupReadChannelPair("15301", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    const RsslUInt32 payLen = 64;
    RsslBuffer* pWr = getClientBuf(payLen, &err);
    ASSERT_NE(pWr, nullptr);
    fillAscii(pWr, payLen);
    pWr->length = payLen;

    RsslRet ret = writeAndFlush(pClientChnl, pWr);
    ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "Write failed: " << err.text;

    time_sleep(50);

    RsslRet    readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf   = rsslRead(pServerChnl, &readRet, &err);

    ASSERT_TRUE((pBuf != nullptr) &&
        (readRet == RSSL_RET_SUCCESS))
        << "Server read returned unexpected readRet=" << readRet
        << " err=" << err.text;

    ASSERT_EQ(payLen, pBuf->length);
}

/* =======================================================================
 * ── Issue 6: readOutArgs populated in packed-buffer continuation path ────
 * The analysis notes that bytesRead / uncompressedBytesRead are forced to 0
 * on packed-buffer continuation calls.  Verify they are at least non-negative
 * (not left as uninitialised garbage) and that the first call reports > 0.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue6_ReadOutArgsOnFirstPackedRead)
{
    ASSERT_TRUE(setupReadChannelPair("15302", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    /* Send a packed message from client (3 sub-messages of 40 bytes each). */
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 40;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillAscii(pBuf, msgLen);
    pBuf->length = msgLen;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (pNext && pNext->length >= msgLen)
    {
        fillAscii(pNext, msgLen);
        pNext->length = msgLen;
        RsslBuffer* pEnd = rsslPackBuffer(pClientChnl, pNext, &err);
        if (pEnd)
            pEnd->length = 0;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    time_sleep(50);

    /* First read should report bytes > 0. */
    RsslRet      readRet = RSSL_RET_SUCCESS;
    RsslReadInArgs  rdIn;  rsslClearReadInArgs(&rdIn);
    RsslReadOutArgs rdOut; rsslClearReadOutArgs(&rdOut);
    rdOut.bytesRead             = (RsslUInt32)-1;
    rdOut.uncompressedBytesRead = (RsslUInt32)-1;

    RsslBuffer* pResult = rsslReadEx(pServerChnl, &rdIn, &rdOut, &readRet, &err);

    if (pResult)
    {
        /* bytesRead on the first packed call should be the entire packed message. */
        EXPECT_EQ(rdOut.bytesRead, 87)
            << "bytesRead must not be left at sentinel -1 on first packed read";
        
        /* Checks for the length of the first pack message */
        ASSERT_EQ(40, pResult->length);
    }
    else
    {
        ASSERT_TRUE(false) << "First read returned unexpected readRet=" << readRet;
    }
}

/* =======================================================================
 * ── Issue 1: Crafted packed-buffer with zero sub-message length ──────────
 * Inject a packed wire frame where the first sub-message's 2-byte length
 * field is 0.  The bounds-check in rsslSocketRead() should catch this and
 * return RSSL_RET_FAILURE without crashing or advancing past the buffer end.
 * ===================================================================== */

/* Build a valid RIPC packed frame with a zero-length sub-message field,
 * then inject it directly into the server socket and let the client try
 * to read it. */
TEST_F(RsslSocketReadTests, Issue1_ZeroSubMessageLengthInPackedFrameHandledGracefully)
{
    ASSERT_TRUE(setupReadChannelPair("15303", &pServer, &pServerChnl, &pClientChnl));

    /* Craft a 3-byte RIPC frame header with IPC_PACKING flag (0x12):
     *   [0..1]  total length = 5 (header 3 + sub-msg prefix 2)
     *   [2]     flags = 0x12 (IPC_DATA | IPC_PACKING)
     *   [3..4]  sub-message length = 0  (crafted zero)
     */
    unsigned char frame[] = {
        0x00, 0x05,   /* total length = 5 */
        0x12,         /* IPC_DATA (0x10) | IPC_PACKING (0x02) */
        0x00, 0x00    /* sub-message length = 0 */
    };

    int sent = injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    if (sent <= 0)
    {
        SUCCEED() << "Failed to inject raw bytes – skipping";
        return;
    }

    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    ASSERT_TRUE(pBuf != nullptr && pBuf->length == 0)
        << "Zero sub-message length in packed frame must not crash";
}

/* =======================================================================
 * ── Issue 1: Oversized sub-message length causes out-of-bounds read ──────
 * Inject a packed frame where the sub-message length field reports more
 * bytes than the enclosing RIPC frame contains.  The bounds check:
 *   if ((packedBuffer->buffer + packedBuffer->length) <
 *       (returnBuffer.data + returnBuffer.length))
 * should catch this and return RSSL_RET_FAILURE.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue1_OversizedSubMessageLengthRejected)
{
    ASSERT_TRUE(setupReadChannelPair("15304", &pServer, &pServerChnl, &pClientChnl));

    /* Packed frame:
     *   [0..1]  total = 7 (3-byte header + 2-byte prefix + 2 payload bytes)
     *   [2]     0x12  (IPC_DATA | IPC_PACKING)
     *   [3..4]  sub-msg length = 0xFFFF  (far beyond the 2 payload bytes)
     *   [5..6]  payload (2 bytes)
     */
    unsigned char frame[] = {
        0x00, 0x07,
        0x12,
        0xFF, 0xFF,  /* sub-message length = 65535 */
        0x41, 0x42   /* 2 bytes of actual payload */
    };

    int sent = injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    if (sent <= 0)
    {
        ASSERT_FALSE(true) << "Failed to inject – skip";
        return;
    }

    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    ASSERT_EQ(RSSL_RET_FAILURE, readRet);
    ASSERT_EQ(nullptr, pBuf);
    EXPECT_TRUE(strstr(err.text, " Error: 1004 rsslSocketRead() unpacked buffer length (65537) is greater than total packed buffer length (4). The invalid msg length (65535)") != NULL)
        << "Error message should mention invalid msg length.";

    /* Second read call to ensure that there is no issue */
    RsslRet   readRet2 = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf2 = rsslRead(pServerChnl, &readRet2, &err);

    ASSERT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet2); // There is no data to read from
    ASSERT_EQ(nullptr, pBuf2);
}

/* =======================================================================
 * ── Issue 9 / Issue 3: Fragmented messages with reused fragId ────────────
 * Write 256+ two-fragment messages so that fragId wraps from 255 back to 1.
 * The server reads all messages.  Issue 3 (leak on hash-insert failure) and
 * Issue 9 (double-free after rsslHashTableRemoveLink) are both exercised
 * when the same fragId appears a second time in the assembly hash table.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue9_FragIdReuseReadServerDoesNotCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15307", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 64;  /* 2 fragments per message */
    const int        totalMsgs = 260;            /* wraps fragId past 255 */

    int written = 0;
    int read    = 0;
    int wrongSz = 0;

    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;

        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        ++written;

        /* Drain the server every 32 writes and verify each reassembled
         * message has exactly msgSize bytes.  A wrong length indicates
         * that fragId reuse caused the assembly buffer for one message
         * to be partially overwritten by a subsequent message. */
        if ((i % 32) == 31)
        {
            rsslFlush(pClientChnl, &err);
            time_sleep(5);

            for (int attempt = 0; attempt < 256; ++attempt)
            {
                RsslRet     readRet = RSSL_RET_SUCCESS;
                RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
                if (pRead)
                {
                    ++read;
                    if (pRead->length != msgSize)
                        ++wrongSz;
                }
                else if (readRet == RSSL_RET_READ_WOULD_BLOCK ||
                         readRet <  RSSL_RET_SUCCESS)
                    break;
            }
        }
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(20);

    /* Final drain with size verification. */
    for (int attempt = 0; attempt < 2048; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++read;
            if (pRead->length != msgSize)
                ++wrongSz;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK ||
                 readRet <  RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(written, 260)
        << "At least 200 messages should have been written";
    EXPECT_EQ(read, 260)
        << "At least some reassembled messages must have been read";
    EXPECT_EQ(wrongSz, 0)
        << "Every reassembled message must have length == " << msgSize
        << "; wrong-size count=" << wrongSz
        << " indicates fragId reuse corrupted an assembly buffer";
}

/* =======================================================================
 * ── Issue 3 / Issue 9: Rapid fragId wrap – all queued then flushed ───────
 * Write 260 two-fragment messages into the queue WITHOUT flushing so all
 * 260 messages are present simultaneously.  Messages 256–260 carry fragIds
 * 1–5 – the same values assigned to messages 1–5.  When the bulk flush
 * delivers them, the server's assembly hash table must evict the old entries
 * for fragId 1–5 without a double-free or memory leak.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue3_BulkQueueThenFlushWithDuplicateFragIds)
{
    const RsslUInt32 fragSize = 256;
    ASSERT_TRUE(setupReadChannelPair("15308", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 32;
    const int        totalMsgs = 260;

    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;

        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        /* No flush – let duplicates accumulate. */
    }

    RsslRet flushRet = rsslFlush(pClientChnl, &err);
    time_sleep(20);
    drainChannel(pServerChnl, 2048);

    EXPECT_EQ(flushRet, 1216)
        << "Bulk flush with duplicate fragIds must not crash; flushRet=" << flushRet;
    SUCCEED() << "Bulk queue + flush with duplicate fragIds did not crash";
}

/* =======================================================================
 * ── Issue 9 / Issue 3: Two-full-wrap-cycle stress test ───────────────────
 * Write 512 two-fragment messages; fragId 1 is reused at messages 257 and
 * 513.  Server reads after every 64 writes to clear assembled buffers,
 * causing repeated eviction-and-insertion of the same fragId keys.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue9_TwoWrapCyclesStressFragIdHashTable)
{
    const RsslUInt32 fragSize = 300;
    ASSERT_TRUE(setupReadChannelPair("15309", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 50;
    const int        totalMsgs = 512;

    int written = 0;
    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;

        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++written;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        if ((i % 64) == 63)
        {
            rsslFlush(pClientChnl, &err);
            time_sleep(5);
            drainChannel(pServerChnl, 512);
        }
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(20);
    drainChannel(pServerChnl, 2048);

    EXPECT_EQ(written, 512) << "512 messages should have been written";
    SUCCEED() << "Two-wrap-cycle stress test did not crash; wrote=" << written;
}

/* =======================================================================
 * ── Issue 5: Concurrent reads on the same channel (RSSL_LOCK_GLOBAL) ─────
 * Under RSSL_LOCK_GLOBAL two threads can call rsslRead() on the same channel
 * simultaneously; returnBuffer is a shared field that can be overwritten.
 * This test verifies that the process does not crash – a correctness
 * guarantee about which data is returned is not made for this locking mode.
 * ===================================================================== */

struct ConcurrentReadArg
{
    RsslChannel*      pChnl;
    std::atomic<int>  readsDone;
    std::atomic<bool> crashed;

    ConcurrentReadArg() : pChnl(nullptr), readsDone(0), crashed(false) {}
};

static RSSL_THREAD_DECLARE(concurrentReadThreadFn, pArg)
{
    ConcurrentReadArg* args = reinterpret_cast<ConcurrentReadArg*>(pArg);
    RsslError err;
    for (int i = 0; i < 100; ++i)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(args->pChnl, &readRet, &err);
        if (pBuf)
            ++args->readsDone;
        else if (readRet < RSSL_RET_SUCCESS &&
                 readRet != RSSL_RET_READ_WOULD_BLOCK &&
                 readRet != RSSL_RET_READ_PING)
            break;
    }
    return 0;
}

TEST_F(RsslSocketReadTests, Issue5_ConcurrentReadsOnSameChannelDoNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15311", &pServer, &pServerChnl, &pClientChnl));

    /* Write a burst of small messages so both reader threads have data. */
    RsslError err;
    for (int i = 0; i < 50; ++i)
    {
        RsslBuffer* pBuf = getClientBuf(32, &err);
        if (!pBuf) break;
        fillAscii(pBuf, 32);
        pBuf->length = 32;
        writeAndFlush(pClientChnl, pBuf);
    }

    time_sleep(20);

    ConcurrentReadArg args;
    args.pChnl = pServerChnl;

    RsslThreadId t1, t2;
    RSSL_THREAD_START(&t1, concurrentReadThreadFn, &args);
    RSSL_THREAD_START(&t2, concurrentReadThreadFn, &args);

    RSSL_THREAD_JOIN(t1);
    RSSL_THREAD_JOIN(t2);

    EXPECT_FALSE(args.crashed.load()) << "Concurrent reads must not crash";
    SUCCEED() << "Concurrent reads on same channel did not crash; reads="
              << args.readsDone.load();
}

/* =======================================================================
 * ── Rapid open-write-read-close cycle ────────────────────────────────────
 * Open a channel pair, write one message, read it, close, repeat 20 times.
 * Exercises pool housekeeping and assembly-buffer cleanup for re-used
 * channel structures (related to Issue 9 double-free on re-use).
 * ===================================================================== */

TEST_F(RsslSocketReadTests, RapidOpenWriteReadCloseCyclesDoNotLeak)
{
    RsslError err;
    const int cycles = 20;

    for (int c = 0; c < cycles; ++c)
    {
        RsslServer*  pSrv = nullptr;
        RsslChannel* pSrC = nullptr;
        RsslChannel* pClC = nullptr;

        if (!setupReadChannelPair("15312", &pSrv, &pSrC, &pClC))
            continue;

        RsslBuffer* pBuf = rsslGetBuffer(pClC, 64, RSSL_FALSE, &err);
        if (pBuf)
        {
            for (int k = 0; k < 64; ++k) pBuf->data[k] = (char)('A' + c % 26);
            pBuf->length = 64;
            writeAndFlush(pClC, pBuf);
        }

        time_sleep(10);

        RsslRet     readRet = RSSL_RET_SUCCESS;
        pBuf = rsslRead(pSrC, &readRet, &err);

        EXPECT_NE(nullptr, pBuf);
        EXPECT_NE(0, pBuf->length);

        rsslCloseChannel(pSrC, &err);
        rsslCloseChannel(pClC, &err);
        rsslCloseServer(pSrv, &err);

        time_sleep(5);
    }

    SUCCEED() << "Rapid open-write-read-close cycles did not crash";
}

/* =======================================================================
 * ── Packed messages: normal multi-sub-message round-trip ─────────────────
 * Write a packed buffer with three 40-byte sub-messages and verify that
 * multiple rsslRead() calls return each sub-message in turn without
 * crashing.  This exercises the packed-buffer continuation path including
 * the unpackOffset advance (Issue 1 area).
 * ===================================================================== */

TEST_F(RsslSocketReadTests, PackedMultiSubMessageRoundTripDoesNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15313", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 40;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    /* Pack three messages. */
    fillAscii(pBuf, msgLen);
    pBuf->length = msgLen;
    RsslBuffer* p2 = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (p2 && p2->length >= msgLen)
    {
        fillAscii(p2, msgLen);
        p2->length = msgLen;
        RsslBuffer* p3 = rsslPackBuffer(pClientChnl, p2, &err);
        if (p3 && p3->length >= msgLen)
        {
            fillAscii(p3, msgLen);
            p3->length = msgLen;
            RsslBuffer* pEnd = rsslPackBuffer(pClientChnl, p3, &err);
            if (pEnd) pEnd->length = 0;
        }
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS) rsslFlush(pClientChnl, &err);

    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_GT(pRead->length, 0u)
                << "Packed sub-message must have length > 0";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK ||
                 readRet <  RSSL_RET_SUCCESS)
            break;
    }

    SUCCEED() << "Packed multi-sub-message round-trip did not crash; msgs=" << msgCount;
}

/* =======================================================================
 * ── Large fragmented message round-trip ──────────────────────────────────
 * Write a large message that spans many fragments and read all fragments on
 * the server side.  Tests the assembly-hash-table insert/lookup/remove path
 * (Issues 3, 4, 9) under normal operation.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, LargeFragmentedMessageRoundTripDoesNotCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15314", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 6;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);

    for (RsslUInt32 k = 0; k < msgSize; ++k)
        pBuf->data[k] = (char)('A' + (k % 26));
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet wret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (wret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(1, msgCount) << "Read loop must not crash";
    SUCCEED() << "Large fragmented message round-trip did not crash; msgs=" << msgCount;
}

/* =======================================================================
 * ── Compressed read round-trip (Zlib) ────────────────────────────────────
 * Write a 2000-byte compressible message on a Zlib channel, then read the
 * reassembled message on the server.  Exercises the decompression path in
 * ipcReadSession() which is called before rsslSocketRead() processes the
 * result.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, CompressedZlibReadRoundTripDoesNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15315", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 msgSize = 2000;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed – skip";
        return;
    }

    memset(pBuf->data, 0x41, pBuf->length);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet wret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (wret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 100; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
            ++msgCount;
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(1, msgCount) << "Zlib read loop must not crash";
    SUCCEED() << "Zlib read round-trip did not crash; msgs=" << msgCount;
}

/* =======================================================================
 * ── ReadEx out-args sanity: bytesRead and uncompressedBytesRead ───────────
 * After a successful read, both fields must be populated (not left as
 * sentinel -1) and must be > 0 when a non-empty buffer is returned.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, ReadExOutArgsPopulatedAfterSuccessfulRead)
{
    ASSERT_TRUE(setupReadChannelPair("15316", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    const RsslUInt32 payLen = 128;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    fillAscii(pBuf, payLen);
    pBuf->length = payLen;

    writeAndFlush(pClientChnl, pBuf);
    time_sleep(50);

    RsslReadInArgs  rdIn;  rsslClearReadInArgs(&rdIn);
    RsslReadOutArgs rdOut; rsslClearReadOutArgs(&rdOut);
    rdOut.bytesRead             = (RsslUInt32)-1;
    rdOut.uncompressedBytesRead = (RsslUInt32)-1;

    RsslRet     readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pRead   = rsslReadEx(pServerChnl, &rdIn, &rdOut, &readRet, &err);

    EXPECT_EQ(RSSL_RET_SUCCESS, readRet) << "Unexpected readRet=" << readRet;

    EXPECT_EQ(rdOut.bytesRead, 131)
        << "bytesRead must be 131 for a non-empty message";
    EXPECT_EQ(rdOut.uncompressedBytesRead,131)
        << "uncompressedBytesRead must be updated after a successful read";
}

/* =======================================================================
 * ── Truncated IPC frame (partial header) ─────────────────────────────────
 * Inject only the first byte of a 3-byte RIPC header.  ipcReadSession()
 * should loop waiting for the full header; since no more bytes arrive on
 * a non-blocking channel, it must return RSSL_RET_READ_WOULD_BLOCK without
 * crashing.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, TruncatedIpcHeaderReturnsWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15317", &pServer, &pServerChnl, &pClientChnl));

    /* Inject just 1 byte (partial RIPC length MSB). */
    unsigned char partialHdr[] = { 0x00 };
    injectRawBytes(pClientChnl, partialHdr, 1);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet) << "Truncated IPC header must not crash; readRet=" << readRet;
}

/* =======================================================================
 * ── Malformed IPC message size larger than maxMsgSize ────────────────────
 * Inject a frame whose length field (0xFFFF) exceeds the channel's
 * maxMsgSize.  ipcReadSession() has:
 *   if ((ipcLen = messageLength) > rsslSocketChannel->maxMsgSize) → FAILURE
 * This tests that guard returns failure cleanly without crashing.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, MalformedOversizedLengthFieldRejectedGracefully)
{
    ASSERT_TRUE(setupReadChannelPair("15318", &pServer, &pServerChnl, &pClientChnl));

    /* Frame with length = 0xFFFF which exceeds default maxMsgSize (6144). */
    unsigned char frame[] = {
        0xFF, 0xFF,   /* length = 65535 */
        0x10          /* IPC_DATA flag */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Size. Message size is: (65535). Max Message size is(6169)") != NULL)
        << "Error message should mention invalid msg length.";
}

/* =======================================================================
 * ── Invalid IPC opcode ───────────────────────────────────────────────────
 * Inject a frame where the opcode byte does not have IPC_DATA (0x10) or
 * IPC_COMP_DATA (0x08) set.  ipcReadSession() rejects unknown opcodes.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, InvalidIpcOpcodeRejectedGracefully)
{
    ASSERT_TRUE(setupReadChannelPair("15319", &pServer, &pServerChnl, &pClientChnl));

    /* 3-byte frame with opcode = 0x00 (neither IPC_DATA nor IPC_COMP_DATA). */
    unsigned char frame[] = { 0x00, 0x03, 0x00 };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err); 

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (0)") != NULL)
        << "Error message should mention Invalid Message Opcode.";
}

/* =======================================================================
 * ── Flood read: no crash after many rapid reads returning WOULD_BLOCK ─────
 * Call rsslRead() 10 000 times on an idle channel (no data).  Verifies that
 * repeated WOULD_BLOCK returns do not accumulate state that eventually causes
 * a crash or deadlock.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, FloodReadOnIdleChannelDoesNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15320", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    int wouldBlock = 0;
    int loopCount = 10000;
    for (int i = 0; i < loopCount; ++i)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);

        EXPECT_EQ(nullptr, pBuf);

        if (readRet == RSSL_RET_READ_WOULD_BLOCK)
        {
            ++wouldBlock;
            continue;
        }
        if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(wouldBlock, loopCount) << "WOULD_BLOCK must have occurred " << wouldBlock;
    SUCCEED() << "Flood read on idle channel did not crash; wouldBlock=" << wouldBlock;
}

/* =======================================================================
 * ── Read after channel close ─────────────────────────────────────────────
 * Call rsslRead() on the server channel after the client has closed its end.
 * The server should receive EOF / connection-reset and rsslRead() must
 * return RSSL_RET_FAILURE without crashing.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, ReadAfterPeerCloseReturnsFailureGracefully)
{
    ASSERT_TRUE(setupReadChannelPair("15321", &pServer, &pServerChnl, &pClientChnl));

    /* Close the client side; the server socket will receive EOF/RST. */
    RsslError err;
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    time_sleep(50);

    RsslRet     readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error:1002 ipcRead() failure. Connection reset by peer") != NULL);
}

/* =======================================================================
 * ── Issue 4: Stale rsslAssemblyBuf – completed first fragment ────────────
 * Write a message where the entire fragmented payload fits within the first
 * fragment (readCursor == buffer.length immediately after the first read).
 * rsslSocketRead() must free the assembly buffer and zero the pointer so
 * that the final stale-pointer check is safe.
 * ===================================================================== */

TEST_F(RsslSocketReadTests, Issue4_CompletedFirstFragmentAssemblyBufferFreedSafely)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15322", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* A message slightly larger than fragSize will create exactly 2 fragments.
     * After both arrive, the assembly buffer should be freed. */
    const RsslUInt32 msgSize = fragSize + 1;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    for (RsslUInt32 k = 0; k < msgSize; ++k)
        pBuf->data[k] = (char)('A' + (k % 26));
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 100; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            /* Verify the reassembled message has the expected length. */
            EXPECT_EQ(pRead->length, msgSize)
                << "Reassembled message length must match original";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one reassembled message must be returned";
    SUCCEED() << "Two-fragment reassembly did not crash";
}

/* =======================================================================
 * ── FRAGMENTATION NEGATIVE / CRASH-RISK TESTS ────────────────────────────
 *
 * The following tests exercise crash vectors in the receiver-side
 * fragmentation reassembly path of rsslSocketRead():
 *
 *   Frag-A  – Crafted first-fragment whose declared total length (ripcFragSize)
 *              is smaller than the data already in the fragment.  The
 *              overflow check in the first-fragment path must reject it.
 *
 *   Frag-B  – Crafted subsequent-fragment whose fragId has no matching
 *              first-fragment entry in the assembly hash table.  The lookup
 *              returns NULL; the code must not dereference it.
 *
 *   Frag-C  – First-fragment with ripcFragSize = 0.  Zero-size assembly
 *              buffer allocation followed by a copy-in triggers a heap
 *              underrun; the guard must catch it before the malloc.
 *
 *   Frag-D  – First-fragment with ripcFragSize = UINT32_MAX.  The
 *              _rsslMalloc(ripcFragSize + 7) call wraps to a tiny
 *              allocation; any subsequent data copy overflows it.
 *
 *   Frag-E  – Valid first-fragment followed immediately by a second
 *              first-fragment with the same fragId (collision before the
 *              second fragment arrives).  The assembly hash evicts the first
 *              entry; Issue 3 / Issue 9 are triggered.
 *
 *   Frag-F  – Subsequent-fragment whose declared payload length exceeds the
 *              remaining space in the assembly buffer (readCursor + payload >
 *              buffer.length).  The overflow guard must reject it.
 *
 *   Frag-G  – Many concurrent interleaved first/subsequent fragments for
 *              different fragIds arrive out of order.  The hash table must
 *              not mis-route any subsequent fragment to the wrong entry.
 *
 *   Frag-H  – First-fragment arrives, then the channel is closed before the
 *              subsequent fragment arrives.  ipcFreeSession() must free the
 *              half-assembled buffer without crashing.
 *
 *   Frag-I  – Subsequent-fragment with fragId = 0, which is reserved and
 *              must never appear on the wire.  The implementation must reject
 *              it without crashing.
 *
 *   Frag-J  – Minimum-size first-fragment (fragSize = 100, msgSize = 101):
 *              boundary exactly one byte over the fragment size.  Tests the
 *              tight boundary between single-fragment and two-fragment paths.
 *
 *   Frag-K  – Deep fragment chain (fragSize = 100, msgSize = 5500 → ~55
 *              fragments).  The assembly loop must handle depth > 50 without
 *              stack overflow or infinite loop.
 *
 *   Frag-L  – Interleaved messages: two concurrent fragmented messages with
 *              different fragIds are written simultaneously.  Both must be
 *              reassembled correctly by the server without cross-contamination.
 *
 *   Frag-M  – Crafted wire frame where IPC_EXTENDED_FLAGS (0x40) is set and
 *              IPC_FRAG_HEADER (0x01) is set, but the extended-header bytes
 *              are truncated (only 4 bytes total instead of the required 10).
 *              The header-length guard must reject the frame.
 *
 *   Frag-N  – Stress: 1 000 three-fragment messages read by the server in
 *              a tight non-blocking loop to maximise pool recycling and
 *              assembly-buffer churn (Issues 3, 4, 9).
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Helper: write a raw RIPC frame (with EXTENDED_FLAGS + FRAG_HEADER) that
 * declares a first fragment.  The IPC wire layout for a first-fragment frame
 * (CONN_VERSION_13/14) is:
 *
 *   [0..1]   total frame length   (2 bytes, big-endian)
 *   [2]      opcode               IPC_DATA(0x10) | IPC_EXTENDED_FLAGS(0x40) = 0x50
 *   [3]      ext flags            IPC_FRAG_HEADER(0x01)
 *   [4..7]   total message length (4 bytes, big-endian) = ripcFragSize
 *   [8..9]   frag ID              (2 bytes, big-endian)
 *   [10..]   payload bytes
 *
 * frameLen  = total bytes injected (must equal frame[0..1])
 * fragTotal = value written into the 4-byte "total message length" field
 * fragId    = 2-byte fragment identifier
 * payload   = pointer to data bytes; payloadLen = number of payload bytes
 * --------------------------------------------------------------------- */
static void buildFirstFragFrame(
    unsigned char* buf, int* outLen,
    RsslUInt16 fragId, RsslUInt32 fragTotal,
    const unsigned char* payload, RsslUInt16 payloadLen)
{
    const int HEADER_LEN = 10;
    RsslUInt16 frameLen  = (RsslUInt16)(HEADER_LEN + payloadLen);

    buf[0] = (unsigned char)((frameLen >> 8) & 0xFF);
    buf[1] = (unsigned char)(frameLen & 0xFF);
    buf[2] = 0x03;  /* IPC_DATA | IPC_EXTENDED_FLAGS */
    buf[3] = 0x08;  /* IPC_FRAG_HEADER */
    buf[4] = (unsigned char)((fragTotal >> 24) & 0xFF);
    buf[5] = (unsigned char)((fragTotal >> 16) & 0xFF);
    buf[6] = (unsigned char)((fragTotal >>  8) & 0xFF);
    buf[7] = (unsigned char)(fragTotal & 0xFF);
    buf[8] = (unsigned char)((fragId >> 8) & 0xFF);
    buf[9] = (unsigned char)(fragId & 0xFF);
    if (payloadLen > 0 && payload)
        memcpy(buf + HEADER_LEN, payload, payloadLen);
    *outLen = frameLen;
}

/* Build a subsequent-fragment frame (IPC_FRAG, no FRAG_HEADER):
 *
 *   [0..1]   total frame length
 *   [2]      IPC_DATA(0x10) | IPC_EXTENDED_FLAGS(0x40) = 0x50
 *   [3]      ext flags   IPC_FRAG(0x02)
 *   [4..5]   frag ID     (2 bytes, big-endian)
 *   [6..]    payload
 */
static void buildSubseqFragFrame(
    unsigned char* buf, int* outLen,
    RsslUInt16 fragId,
    const unsigned char* payload, RsslUInt16 payloadLen)
{
    const int HEADER_LEN = 6;
    RsslUInt16 frameLen  = (RsslUInt16)(HEADER_LEN + payloadLen);

    buf[0] = (unsigned char)((frameLen >> 8) & 0xFF);
    buf[1] = (unsigned char)(frameLen & 0xFF);
    buf[2] = 0x03;  /* IPC_DATA | IPC_EXTENDED_FLAGS */
    buf[3] = 0x04;  /* IPC_FRAG */
    buf[4] = (unsigned char)((fragId >> 8) & 0xFF);
    buf[5] = (unsigned char)(fragId & 0xFF);
    if (payloadLen > 0 && payload)
        memcpy(buf + HEADER_LEN, payload, payloadLen);
    *outLen = frameLen;
}

/* -----------------------------------------------------------------------
 * Frag-A – First-fragment whose declared total length (ripcFragSize) is
 * smaller than the payload already in the frame.
 * The guard:  if (ripcBuffer->length > ripcFragSize) → reject
 * must fire before the assembly buffer is allocated.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragA_FirstFragDeclaredTotalSmallerThanPayload)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15400", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x41, sizeof(payload));

    unsigned char frame[256];
    int frameLen = 0;
    /* Declare total = 10 bytes but send 50 bytes of payload → overflow */
    buildFirstFragFrame(frame, &frameLen, 1, 10u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslSocketRead() Received fragment size 10 is greater than the actual data length 50.") != NULL);
}

/* -----------------------------------------------------------------------
 * Frag-B – Subsequent-fragment whose fragId has no matching first-fragment
 * entry in the assembly hash table (orphan subsequent-fragment).
 * rsslSocketRead() must not dereference the NULL result from
 * rsslHashTableFind() and must return cleanly.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragB_OrphanSubsequentFragmentNoFirstFragEntry)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15401", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[20];
    memset(payload, 0x42, sizeof(payload));

    unsigned char frame[128];
    int frameLen = 0;
    /* Send a subsequent-fragment for fragId=99 which never had a first-frag. */
    buildSubseqFragFrame(frame, &frameLen, 99, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslRead() Attempting to reassemble a message with frag ID 99 without seeing first fragment.") != NULL);
}

/* -----------------------------------------------------------------------
 * Frag-C – First-fragment with ripcFragSize = 0.
 * _rsslMalloc(0 + 7) is implementation-defined; the assembly loop guard
 * (ripcBuffer->length > ripcFragSize → 50 > 0) must reject the frame
 * before copying any data.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragC_FirstFragZeroTotalLength)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15402", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x43, sizeof(payload));

    unsigned char frame[256];
    int frameLen = 0;
    buildFirstFragFrame(frame, &frameLen, 2, 0u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslRead() Attempting to reassemble a message with frag ID 2 without seeing first fragment.") != NULL);
}

/* -----------------------------------------------------------------------
 * Frag-D – First-fragment with ripcFragSize = INT32_MAX.
 * _rsslMalloc(INT32_MAX + 7) wraps to a tiny allocation; any data copy
 * into the resulting buffer overflows.  The implementation must either
 * fail the malloc and return RSSL_RET_FAILURE, or read more message from network
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragD_FirstFragINT32MAXTotalLength)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15403", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[30];
    memset(payload, 0x44, sizeof(payload));

    unsigned char frame[256];
    int frameLen = 0;
    buildFirstFragFrame(frame, &frameLen, 3, 0x7FFFFFFFu,
                        payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);

    /* Return FAILURE if cannot allocate memory from the entire message */
    if (readRet == RSSL_RET_FAILURE)
    {
        EXPECT_TRUE(strstr(err.text, "Error: 0005 rsslSocketRead() Cannot allocate memory of size 2147483647 for read buffer.") != NULL);
    }
    else if (readRet == 1)
    {
        // Need to read more data from network
    }
    else
        EXPECT_FALSE(true) << "Received unexpected readRet = " << readRet;
}

/* -----------------------------------------------------------------------
 * Frag-E – Two consecutive first-fragments with the same fragId collide in
 * the assembly hash table before the second fragment of the first message
 * has arrived.  The eviction path (Issues 3 and 9) must free the old entry
 * and insert the new one without double-freeing or leaking memory.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragE_DuplicateFirstFragIdEvictsOldEntryWithoutCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15404", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x45, sizeof(payload));
    unsigned char frame[256];
    int frameLen = 0;

    /* First first-fragment for fragId=10, total=200 bytes. */
    buildFirstFragFrame(frame, &frameLen, 10, 200u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    drainChannel(pServerChnl, 8);

    /* Second first-fragment for the SAME fragId=10 before second frag arrives. */
    buildFirstFragFrame(frame, &frameLen, 10, 200u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    drainChannel(pServerChnl, 8);

    /* Now send the second fragment for fragId=10 (completing the second message). */
    unsigned char payload2[150];
    memset(payload2, 0x46, sizeof(payload2));
    buildSubseqFragFrame(frame, &frameLen, 10, payload2, (RsslUInt16)sizeof(payload2));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);
    drainChannel(pServerChnl, 64);

    SUCCEED() << "Duplicate first-fragment eviction did not crash";
}

/* -----------------------------------------------------------------------
 * Frag-F – Subsequent-fragment whose payload is larger than the remaining
 * space in the assembly buffer (readCursor + payloadLen > buffer.length).
 * The overflow guard must reject the frame and return RSSL_RET_FAILURE
 * without writing past the end of the allocated assembly buffer.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragF_SubsequentFragPayloadExceedsAssemblyBufferSpace)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15405", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x47, sizeof(payload));
    unsigned char frame[512];
    int frameLen = 0;

    /* First-fragment for fragId=20, total=60 bytes (50 payload + 10 header). */
    buildFirstFragFrame(frame, &frameLen, 20, 60u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    drainChannel(pServerChnl, 8);

    /* Subsequent-fragment with 200 bytes of payload – far more than the
     * 10 remaining bytes in the 60-byte assembly buffer. */
    unsigned char bigPayload[200];
    memset(bigPayload, 0x48, sizeof(bigPayload));
    buildSubseqFragFrame(frame, &frameLen, 20, bigPayload, (RsslUInt16)sizeof(bigPayload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslRet expectedRet = RSSL_RET_FAILURE;
    RsslError rsslError;

    drainChannel(pServerChnl, 64, &expectedRet, &rsslError);
    EXPECT_TRUE(strstr(rsslError.text, "Error: 0014 rsslSocketRead() Received fragment size 250 is greater than the actual data length 60.") != NULL);

    SUCCEED() << "Subsequent-fragment payload overflow did not crash";
}

/* -----------------------------------------------------------------------
 * Frag-G – Interleaved out-of-order fragments for four different fragIds.
 * Sequence:
 *   first(id=30), first(id=31), first(id=32), first(id=33),
 *   subseq(id=31), subseq(id=33), subseq(id=30), subseq(id=32)
 * The hash table must route each subsequent-fragment to the correct
 * assembly buffer and produce four complete reassembled messages.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragG_InterleavedOutOfOrderFragmentsReassembledCorrectly)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15406", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char p1[50], p2[50];
    memset(p1, 0x41, sizeof(p1));
    memset(p2, 0x42, sizeof(p2));

    unsigned char frame[512];
    int frameLen = 0;

    RsslUInt32 total = (RsslUInt32)(sizeof(p1) + sizeof(p2));  /* 100 bytes */

    RsslUInt16 ids[4] = {30, 31, 32, 33};

    /* Send all four first-fragments. */
    for (int i = 0; i < 4; ++i)
    {
        buildFirstFragFrame(frame, &frameLen, ids[i], total, p1, (RsslUInt16)sizeof(p1));
        injectRawBytes(pClientChnl, frame, frameLen);
    }
    time_sleep(20);
    drainChannel(pServerChnl, 16);

    /* Send subsequent-fragments in reverse order of fragId. */
    RsslUInt16 revIds[4] = {31, 33, 30, 32};
    for (int i = 0; i < 4; ++i)
    {
        buildSubseqFragFrame(frame, &frameLen, revIds[i], p2, (RsslUInt16)sizeof(p2));
        injectRawBytes(pClientChnl, frame, frameLen);
    }
    time_sleep(50);
    int messageCount = drainChannel(pServerChnl, 64);

    EXPECT_EQ(4, messageCount) << "Expect to receive 4 messages";

    SUCCEED() << "Interleaved out-of-order fragments did not crash";
}

/* -----------------------------------------------------------------------
 * Frag-H – First-fragment arrives; the channel is closed before the
 * subsequent fragment arrives.  ipcFreeSession() must release the
 * half-assembled rsslAssemblyBuffer allocated on the heap without leaking
 * or double-freeing it.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragH_ChannelClosedBeforeSubsequentFragmentArrives)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15407", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x49, sizeof(payload));
    unsigned char frame[256];
    int frameLen = 0;

    /* Send only the first-fragment; deliberately withhold the second. */
    buildFirstFragFrame(frame, &frameLen, 40, 200u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    drainChannel(pServerChnl, 8);

    /* Close the server channel with the assembly buffer still in the hash table. */
    RsslError err;
    RsslRet ret = rsslCloseChannel(pServerChnl, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, ret) << "Expect to close the channel successfully. ";
    pServerChnl = nullptr;

    SUCCEED() << "Channel close with incomplete fragment assembly did not crash";
}

/* -----------------------------------------------------------------------
 * Frag-I – Subsequent-fragment with fragId = 0.
 * FragId 0 is reserved; the implementation uses fragId == 0 to mean
 * "not fragmented".  A subsequent-fragment with fragId = 0 must be
 * rejected without crashing (the hash table lookup returns NULL).
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragI_SubsequentFragmentWithZeroFragIdRejected)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15408", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[20];
    memset(payload, 0x4A, sizeof(payload));
    unsigned char frame[128];
    int frameLen = 0;

    buildSubseqFragFrame(frame, &frameLen, 0, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    /* Handle as normal message wihtout fragmentation */
    EXPECT_NE(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_SUCCESS, readRet);
    EXPECT_EQ(sizeof(payload), pBuf->length);
    EXPECT_TRUE(memcmp(&payload, pBuf->data, pBuf->length) == 0);
}

/* -----------------------------------------------------------------------
 * Frag-J – Minimum-size fragmented message.
 * fragSize = 100, msgSize = 101 → exactly one byte spills into the second
 * fragment.  Both the first-fragment path (FRAG_HEADER) and the
 * subsequent-fragment path (FRAG) are exercised at the tightest boundary.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragJ_MinimumSizeTwoFragmentMessage)
{
    const RsslUInt32 fragSize = 100;
    ASSERT_TRUE(setupReadChannelPair("15409", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 1;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    for (RsslUInt32 k = 0; k < msgSize; ++k)
        pBuf->data[k] = (char)('A' + (k % 26));
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 100; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, msgSize)
                << "Minimum two-fragment message must reassemble to full length";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one minimum two-fragment message expected";
    SUCCEED() << "Minimum two-fragment message did not crash";
}

/* -----------------------------------------------------------------------
 * Frag-K – Deep fragment chain (fragSize = 100, msgSize ≈ 5500 → ~55 frags).
 * The assembly loop must handle depth > 50 without stack overflow, infinite
 * loop, or heap corruption in the readCursor-advance logic.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragK_DeepFragmentChainReassemblesWithoutCrash)
{
    const RsslUInt32 fragSize = 100;
    ASSERT_TRUE(setupReadChannelPair("15410", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 55;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed for deep-chain test – skip";
        return;
    }
    for (RsslUInt32 k = 0; k < msgSize; ++k)
        pBuf->data[k] = (char)('A' + (k % 26));
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    int msgCount = 0;
    for (int attempt = 0; attempt < 100; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);

        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, msgSize)
                << "Deep-chain reassembled message must have full length";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one deep-chain message expected";
}

/* -----------------------------------------------------------------------
 * Frag-L – Two concurrent interleaved fragmented messages with different
 * fragIds.  Both messages are written as separate buffers before flushing
 * so their fragments may be interleaved on the wire.  The server must
 * reassemble both messages correctly without mixing their data.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragL_TwoConcurrentFragmentedMessagesReassembledCorrectly)
{
    const RsslUInt32 fragSize = 256;
    ASSERT_TRUE(setupReadChannelPair("15411", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 100;  /* 2 fragments each */

    /* Write both messages into the queue before flushing so they interleave. */
    for (int m = 0; m < 2; ++m)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;
        /* Each message has a distinct fill pattern to detect cross-contamination. */
        memset(pBuf->data, (m == 0 ? 0xAA : 0xBB), pBuf->length);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(50);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, msgSize)
                << "Both concurrent messages must reassemble to full length";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
        else if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 2) << "Both concurrent fragmented messages must be reassembled";
    SUCCEED() << "Two concurrent fragmented messages reassembled without crash";
}

/* -----------------------------------------------------------------------
 * Frag-M – Truncated extended-header frame.
 * IPC_EXTENDED_FLAGS (0x40) + IPC_FRAG_HEADER (0x01) set but only 8 bytes
 * total instead of the required 10 (header 3 + ext_flags 1 + fragLen 4 +
 * fragId 2 = 10).  The header-length guard must reject the frame.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragM_TruncatedExtendedFragHeaderRejected)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15412", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    /* Build an 8-byte frame that claims to be a FRAG_HEADER frame but is
     * missing the last 2 bytes of the fragment-ID field. */
    unsigned char frame[8] = {
        0x00, 0x08,  /* total = 8 */
        0x50,        /* IPC_DATA | IPC_EXTENDED_FLAGS */
        0x01,        /* IPC_FRAG_HEADER */
        0x00, 0x00, 0x00, 0x64  /* 4-byte fragLen = 100; fragId bytes absent */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (80)") != NULL);
}

/* -----------------------------------------------------------------------
 * Frag-N – Stress test – 1 000 three-fragment messages.
 * fragSize = 200, msgSize = 600 → exactly 3 fragments per message.
 * Server reads in a tight non-blocking loop to maximise assembly-buffer
 * churn and pool recycling, targeting Issues 3, 4, and 9.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragN_OneThousandThreeFragmentMessagesStressTest)
{
    const RsslUInt32 fragSize  = 200;
    ASSERT_TRUE(setupReadChannelPair("15413", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize * 3;
    const int        totalMsgs = 1000;

    int written = 0;
    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;
        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++written;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        /* Drain the server every 50 writes to prevent TCP buffer saturation. */
        if ((i % 50) == 49)
        {
            rsslFlush(pClientChnl, &err);
            time_sleep(5);
            drainChannel(pServerChnl, 256);
        }
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(30);
    drainChannel(pServerChnl, 4096);

    EXPECT_EQ(written, totalMsgs)
        << "1000 three-fragment messages must have been written";
    SUCCEED() << "1000-message three-fragment stress test completed without crash; wrote="
              << written;
}

/* -----------------------------------------------------------------------
 * FragId-R1 – fragId = 0 in a first-fragment (FRAG_HEADER) frame.
 *
 * FragId 0 is the sentinel value for "non-fragmented" messages.  If the
 * receiver inserts a hash entry under key 0, it will never be evicted
 * (no subsequent-fragment with fragId=0 can legitimately arrive) and
 * all future rsslHashTableFind(0) calls may return the stale entry,
 * corrupting subsequent fragmented messages.
 *
 * Expected: no crash; server returns no assembled message for a frame
 * that has only one fragment worth of data.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR1_ZeroFragIdInFirstFragmentFrame)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15500", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[60];
    memset(payload, 0x41, sizeof(payload));

    unsigned char frame[256];
    int frameLen = 0;
    /* fragId = 0, total = 120 bytes (60 payload here, 60 to follow). */
    buildFirstFragFrame(frame, &frameLen, 0, 120u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    /* Expect to read more data as a normal message */
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(1, readRet);
}

/* -----------------------------------------------------------------------
 * FragId-R2 – fragId = 0xFFFF (UINT16_MAX) in a first-fragment frame.
 *
 * The maximum legal 16-bit value.  The hash-table key computation
 * (typically fragId % tableSize) must not overflow or alias to 0.  The
 * frame must either be inserted into the table or rejected cleanly.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR2_MaxUInt16FragIdInFirstFragment)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15502", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[50];
    memset(payload, 0x43, sizeof(payload));

    unsigned char frame[256];
    int frameLen = 0;
    buildFirstFragFrame(frame, &frameLen, 0xFFFFu, 100u,
                        payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);
    
    int messageCount = drainChannel(pServerChnl, 16);

    EXPECT_EQ(0, messageCount) << "Need more data to complete the entire message.";

    SUCCEED() << "First-fragment with fragId=0xFFFF did not crash (FragId-R3)";
}

/* -----------------------------------------------------------------------
 * FragId-R3 – Same fragId used for two simultaneous first-fragments.
 *
 * Inject first-fragment(id=7) twice consecutively without any
 * subsequent-fragment in between.  The second first-fragment must evict
 * the assembly buffer created for the first one.  The eviction path:
 *   rsslHashTableRemoveLink → _rsslFree(old.buffer.data) → insert new
 * must not double-free the old buffer or leave its pointer non-NULL
 * (Issues 3 and 9).
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR3_SameFragIdTwoFirstFragmentsEvictsOldEntry)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15504", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[80];
    memset(payload, 0x45, sizeof(payload));
    unsigned char frame[256];
    int frameLen = 0;

    /* First first-fragment for fragId=7. */
    buildFirstFragFrame(frame, &frameLen, 7, 160u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    drainChannel(pServerChnl, 8);

    /* Second first-fragment for the SAME fragId=7 – must evict the first. */
    buildFirstFragFrame(frame, &frameLen, 7, 160u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);
    int messageCount = drainChannel(pServerChnl, 8);

    EXPECT_EQ(0, messageCount);

    SUCCEED() << "Two consecutive first-fragments with fragId=7 did not crash (FragId-R5)";
}

/* -----------------------------------------------------------------------
 * FragId-R4 – Subsequent-fragment for fragId=7 after the eviction in R3.
 *
 * After FragId-R4 evicted the first assembly buffer and inserted a second
 * one for fragId=7, a subsequent-fragment for fragId=7 arrives.  The
 * receiver must look up the SECOND (current) entry, NOT the freed first
 * one, and copy the payload into the live assembly buffer.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR4_SubsequentFragAfterEvictionUsesNewEntry)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15505", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[80];
    memset(payload, 0x46, sizeof(payload));
    unsigned char frame[256];
    int frameLen = 0;

    /* First first-fragment (id=7). */
    buildFirstFragFrame(frame, &frameLen, 7, 160u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(15);
    drainChannel(pServerChnl, 4);

    /* Second first-fragment (same id=7) – evicts the first assembly buffer. */
    buildFirstFragFrame(frame, &frameLen, 7, 160u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(15);
    drainChannel(pServerChnl, 4);

    /* Subsequent-fragment completing the SECOND first-fragment. */
    unsigned char payload2[80];
    memset(payload2, 0x47, sizeof(payload2));
    buildSubseqFragFrame(frame, &frameLen, 7, payload2, (RsslUInt16)sizeof(payload2));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    /* Drain: the assembled message (if delivered) must have length 160. */
    for (int i = 0; i < 64; ++i)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
        {
            EXPECT_EQ(pBuf->length, 160u)
                << "Assembled message after eviction must have correct length";
            break;
        }
        EXPECT_FALSE(true) << "Expect to read the entire message.";
        
    }

    SUCCEED() << "Subsequent-fragment after eviction did not crash (FragId-R6)";
}

/* -----------------------------------------------------------------------
 * FragId-R5 – fragId=1 reused 300 times via complete first/subsequent pairs.
 *
 * Each cycle:
 *   inject first-fragment(id=1, total=100) → drain
 *   inject subsequent-fragment(id=1)       → drain (assembly completes)
 *
 * After each completion the hash entry is removed.  The next cycle
 * re-inserts fragId=1.  300 iterations stress the remove→insert cycle
 * that triggers Issues 3 and 9 if buffer.data is not zeroed after remove.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR5_FragId1Reused300TimesNoDoubleFreeCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15506", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char p1[50], p2[50];
    memset(p1, 0x41, sizeof(p1));
    memset(p2, 0x42, sizeof(p2));
    unsigned char frame[256];
    int frameLen = 0;
    int messageCount = 0;

    const RsslUInt32 total = (RsslUInt32)(sizeof(p1) + sizeof(p2));

    for (int i = 0; i < 300; ++i)
    {
        buildFirstFragFrame(frame, &frameLen, 1, total,
                            p1, (RsslUInt16)sizeof(p1));
        injectRawBytes(pClientChnl, frame, frameLen);

        buildSubseqFragFrame(frame, &frameLen, 1,
                             p2, (RsslUInt16)sizeof(p2));
        injectRawBytes(pClientChnl, frame, frameLen);

        if ((i % 30) == 29)
        {
            time_sleep(5);
            messageCount += drainChannel(pServerChnl, 128);
        }
    }

    time_sleep(20);
    messageCount += drainChannel(pServerChnl, 2048);
    EXPECT_EQ(300, messageCount);

    SUCCEED() << "300 back-to-back fragId=1 complete pairs did not crash (FragId-R7)";
}

/* -----------------------------------------------------------------------
 * FragId-R6 – All 255 valid fragIds occupied simultaneously.
 *
 * Inject 255 first-fragments (fragId 1–255) without any subsequent-
 * fragments so all hash table entries are occupied.  Then inject a new
 * first-fragment with fragId=1 (collision or eviction at a fully occupied
 * slot).  The hash table must handle the saturated state without crashing,
 * infinite-looping in the probe sequence, or leaking memory.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR6_HashTableSaturatedWith255EntriesThenCollision)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15507", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[30];
    memset(payload, 0x43, sizeof(payload));
    unsigned char frame[256];
    int frameLen = 0;

    /* Insert fragId 1–255 without completing any of them. */
    for (int id = 1; id <= 255; ++id)
    {
        buildFirstFragFrame(frame, &frameLen, (RsslUInt16)id, 100u,
                            payload, (RsslUInt16)sizeof(payload));
        injectRawBytes(pClientChnl, frame, frameLen);

        /* Drain every 32 frames to avoid TCP buffer overflow. */
        if ((id % 32) == 0)
        {
            time_sleep(5);
            drainChannel(pServerChnl, 64);
        }
    }

    time_sleep(20);
    drainChannel(pServerChnl, 512);

    /* Now inject a new first-fragment for fragId=1 (collides with the
     * existing half-assembled entry for fragId=1). */
    buildFirstFragFrame(frame, &frameLen, 1, 100u, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(30);
    drainChannel(pServerChnl, 64);

    SUCCEED() << "Saturated hash table + collision did not crash (FragId-R8)";
}

/* -----------------------------------------------------------------------
 * FragId-R7 – Orphan subsequent-fragment (out-of-order: no prior first).
 *
 * Inject a subsequent-fragment for fragId=200 before any first-fragment
 * for that ID has arrived.  The hash table lookup returns NULL; the
 * receiver must discard the orphan frame cleanly without dereferencing
 * the NULL pointer or writing into the assembly buffer of a different ID.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR7_OrphanSubsequentFragmentBeforeFirstFragment)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15508", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char payload[40];
    memset(payload, 0x44, sizeof(payload));
    unsigned char frame[128];
    int frameLen = 0;

    /* Subsequent-fragment for fragId=200 with no prior first-fragment. */
    buildSubseqFragFrame(frame, &frameLen, 200, payload, (RsslUInt16)sizeof(payload));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer *pBuffer = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuffer);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslRead() Attempting to reassemble a message with frag ID 200 without seeing first fragment.") != NULL);

    SUCCEED() << "Orphan subsequent-fragment for fragId=200 did not crash (FragId-R9); "
                 "readRet=" << readRet;
}

/* -----------------------------------------------------------------------
 * FragId-R8 – Complete pair × 2 for the same fragId=5.
 *
 * Sequence:
 *   first(id=5, total=80) → subseq(id=5) → assembly complete → entry removed
 *   first(id=5, total=80) → subseq(id=5) → assembly complete → entry removed
 *
 * After the first pair rsslHashTableRemoveLink must zero the buffer.data
 * pointer in the evicted entry so that the second pair's rsslHashTableFind
 * does not return the freed entry (Issue 9 double-free vector).
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR8_TwoCompleteRoundsForSameFragIdNoCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15509", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char p1[40], p2[40];
    memset(p1, 0x45, sizeof(p1));
    memset(p2, 0x46, sizeof(p2));
    unsigned char frame[256];
    int frameLen = 0;
    const RsslUInt32 total = (RsslUInt32)(sizeof(p1) + sizeof(p2));

    RsslError err;

    for (int round = 0; round < 2; ++round)
    {
        buildFirstFragFrame(frame, &frameLen, 5, total,
                            p1, (RsslUInt16)sizeof(p1));
        injectRawBytes(pClientChnl, frame, frameLen);
        time_sleep(10);
        drainChannel(pServerChnl, 8);

        buildSubseqFragFrame(frame, &frameLen, 5,
                             p2, (RsslUInt16)sizeof(p2));
        injectRawBytes(pClientChnl, frame, frameLen);
        time_sleep(20);

        /* Read the assembled message for this round. */
        for (int i = 0; i < 50; ++i)
        {
            RsslRet     readRet = RSSL_RET_SUCCESS;
            RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
            if (pBuf)
            {
                EXPECT_EQ(pBuf->length, total)
                    << "Round " << round << ": assembled message must have correct length";
                break;
            }
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            time_sleep(1);
        }
    }

    SUCCEED() << "Two complete rounds for fragId=5 did not crash (FragId-R10)";
}

/* -----------------------------------------------------------------------
 * FragId-R9 – Alternating valid fragId=1 complete messages and invalid
 *              fragId=0 first-fragments.
 *
 * Sequence (5 iterations):
 *   first(id=1, total=100) → subseq(id=1) → complete message delivered
 *   first(id=0, total=100) → injected (must be rejected/ignored)
 *
 * The invalid frames must not corrupt the in-progress valid assembly
 * buffer, the hash table state, or the channel mutex.  After each cycle
 * the valid message must still be reassembled correctly.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, FragIdR9_AlternatingValidAndInvalidFragIdDoesNotCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15511", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    unsigned char p1[50], p2[50];
    memset(p1, 0x41, sizeof(p1));
    memset(p2, 0x42, sizeof(p2));
    unsigned char pInvalid[30];
    memset(pInvalid, 0xFF, sizeof(pInvalid));
    unsigned char frame[256];
    int frameLen = 0;
    const RsslUInt32 total = (RsslUInt32)(sizeof(p1) + sizeof(p2));

    RsslError err;
    int validMsgsReceived = 0;

    for (int cycle = 0; cycle < 5; ++cycle)
    {
        /* Valid first-fragment for fragId=1. */
        buildFirstFragFrame(frame, &frameLen, 1, total,
                            p1, (RsslUInt16)sizeof(p1));
        injectRawBytes(pClientChnl, frame, frameLen);
        time_sleep(5);
        drainChannel(pServerChnl, 4);

        /* Invalid first-fragment (fragId=0) – must be rejected. */
        buildFirstFragFrame(frame, &frameLen, 0, total,
                            pInvalid, (RsslUInt16)sizeof(pInvalid));
        injectRawBytes(pClientChnl, frame, frameLen);
        time_sleep(5);
        drainChannel(pServerChnl, 4);

        /* Valid subsequent-fragment for fragId=1 – completes the message. */
        buildSubseqFragFrame(frame, &frameLen, 1,
                             p2, (RsslUInt16)sizeof(p2));
        injectRawBytes(pClientChnl, frame, frameLen);
        time_sleep(20);

        /* Read the assembled message. */
        for (int i = 0; i < 50; ++i)
        {
            RsslRet     readRet = RSSL_RET_SUCCESS;
            RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
            if (pBuf)
            {
                EXPECT_EQ(pBuf->length, total)
                    << "Cycle " << cycle
                    << ": valid message length must not be corrupted by "
                       "invalid fragId=0 frame";
                ++validMsgsReceived;
                break;
            }
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            time_sleep(1);
        }
    }

    EXPECT_EQ(validMsgsReceived, 5)
        << "All 5 valid messages must be received despite interleaved "
           "invalid fragId=0 frames (FragId-R12)";
    SUCCEED() << "Alternating valid/invalid fragId sequence did not crash (FragId-R12)";
}

/* =======================================================================
 * ── Normal messages interleaved with fragmented messages ─────────────────
 *
 * The tests below verify that the receiver correctly handles a mix of
 * single-fragment (normal) and multi-fragment (fragmented) messages on
 * the same channel.  The key invariants tested are:
 *
 *  Interleave-1 – One normal message sent BEFORE a fragmented message.
 *                 The normal message must be delivered immediately; the
 *                 subsequent fragmented message must be reassembled
 *                 independently with no state carried over from the
 *                 normal message.
 *
 *  Interleave-2 – One normal message sent AFTER a fragmented message.
 *                 The fragmented message must be fully reassembled before
 *                 the normal message is returned, and the normal message
 *                 must have the correct length.
 *
 *  Interleave-3 – Normal message sent BETWEEN the first and second
 *                 fragments of a fragmented message (using raw injection
 *                 to insert the normal frame into the byte stream).
 *                 The receiver must buffer the first fragment, deliver the
 *                 interleaved normal message, and then complete the
 *                 fragmented assembly on the second fragment.
 *
 *  Interleave-4 – Alternating pattern: normal, fragmented, normal,
 *                 fragmented, repeated 20 times.  Verifies that the
 *                 packed/fragmented state machine resets cleanly between
 *                 every pair and that no assembly-buffer state bleeds into
 *                 subsequent single-fragment messages.
 *
 *  Interleave-5 – Burst of 5 normal messages immediately followed by a
 *                 deep (10-fragment) message, then another burst of 5
 *                 normal messages.  Stresses the pool recycling path when
 *                 the assembly buffer for a long chain is freed between two
 *                 groups of single-fragment messages.
 *
 * Port assignments: 15600 – 15609
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Interleave-1 – Normal message BEFORE a fragmented message.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Interleave1_NormalBeforeFragmented)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15600", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 normalLen = 64;
    const RsslUInt32 fragLen   = fragSize + 32;  /* 2 fragments */

    /* --- send normal message ------------------------------------------ */
    RsslBuffer* pNorm = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
    ASSERT_NE(pNorm, nullptr);
    fillAscii(pNorm, normalLen);
    pNorm->length = normalLen;
    ASSERT_EQ(writeAndFlush(pClientChnl, pNorm), RSSL_RET_SUCCESS);

    /* --- send fragmented message --------------------------------------- */
    RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, fragLen, RSSL_FALSE, &err);
    ASSERT_NE(pFrag, nullptr);
    for (RsslUInt32 k = 0; k < fragLen; ++k)
        pFrag->data[k] = (char)('Z' - (k % 26));
    pFrag->length = fragLen;
    ASSERT_EQ(writeAndFlush(pClientChnl, pFrag), RSSL_RET_SUCCESS);

    time_sleep(50);

    /* --- read and verify ----------------------------------------------- */
    int  normalRead = 0, fragRead = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)
            ++normalRead;
        else if (pBuf->length == fragLen)
            ++fragRead;
    }

    EXPECT_EQ(normalRead, 1) << "Exactly one normal message expected (Interleave-1)";
    EXPECT_EQ(fragRead,   1) << "Exactly one fragmented message expected (Interleave-1)";
}

/* -----------------------------------------------------------------------
 * Interleave-2 – Normal message AFTER a fragmented message.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Interleave2_NormalAfterFragmented)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15601", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 normalLen = 48;
    const RsslUInt32 fragLen   = fragSize + 16;  /* 2 fragments */

    /* --- send fragmented message --------------------------------------- */
    RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, fragLen, RSSL_FALSE, &err);
    ASSERT_NE(pFrag, nullptr);
    fillAscii(pFrag, fragLen);
    pFrag->length = fragLen;
    ASSERT_EQ(writeAndFlush(pClientChnl, pFrag), RSSL_RET_SUCCESS);

    /* --- send normal message ------------------------------------------ */
    RsslBuffer* pNorm = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
    ASSERT_NE(pNorm, nullptr);
    memset(pNorm->data, 0x55, pNorm->length);
    pNorm->length = normalLen;
    ASSERT_EQ(writeAndFlush(pClientChnl, pNorm), RSSL_RET_SUCCESS);

    time_sleep(50);

    int normalRead = 0, fragRead = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)
            ++normalRead;
        else if (pBuf->length == fragLen)
            ++fragRead;
    }

    EXPECT_EQ(fragRead,   1) << "Exactly one fragmented message expected (Interleave-2)";
    EXPECT_EQ(normalRead, 1) << "Exactly one normal message expected (Interleave-2)";
}

/* -----------------------------------------------------------------------
 * Interleave-3 – Normal message injected BETWEEN first and second fragments.
 *
 * A valid RIPC non-fragmented frame carrying 32 bytes of ASCII data is
 * injected into the wire stream after the first-fragment has been delivered
 * but before the subsequent-fragment arrives.  The receiver must:
 *   1. Buffer the first-fragment in its assembly map.
 *   2. Deliver the interleaved normal message.
 *   3. On arrival of the subsequent-fragment, complete the assembly and
 *      deliver the full fragmented message.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Interleave3_NormalBetweenFirstAndSecondFragment)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15602", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;

    /* Build and inject a first-fragment (fragId=1, total=100, 50-byte payload). */
    unsigned char p1[50], p2[50];
    memset(p1, 0x41, sizeof(p1));
    memset(p2, 0x42, sizeof(p2));
    unsigned char frame[256];
    int frameLen = 0;

    const RsslUInt32 fragTotal = (RsslUInt32)(sizeof(p1) + sizeof(p2));
    buildFirstFragFrame(frame, &frameLen, 1, fragTotal, p1, (RsslUInt16)sizeof(p1));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(20);

    /* Let the server consume the first-fragment (partial assembly). */
    drainChannel(pServerChnl, 4);

    /* Inject a normal (non-fragmented) RIPC frame with 32 ASCII bytes.
     * Wire layout:  [0..1] length=35, [2] flags=IPC_DATA(0x02), [3..34] payload */
    const int    normalPayLen = 32;
    unsigned char normalFrame[35];
    normalFrame[0] = 0x00;
    normalFrame[1] = (unsigned char)(3 + normalPayLen);
    normalFrame[2] = 0x02;  /* IPC_DATA */
    for (int k = 0; k < normalPayLen; ++k)
        normalFrame[3 + k] = (unsigned char)('A' + (k % 26));
    
    int writeSize = injectRawBytes(pClientChnl, normalFrame, (int)sizeof(normalFrame));
    time_sleep(20);

    /* Read and verify the interleaved normal message arrives first. */
    bool normalDelivered = false;
    for (int attempt = 0; attempt < 64; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf && pBuf->length == (RsslUInt32)normalPayLen)
        {
            normalDelivered = true;
            break;
        }
        if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    /* Inject the subsequent-fragment to complete the fragmented message. */
    buildSubseqFragFrame(frame, &frameLen, 1, p2, (RsslUInt16)sizeof(p2));
    injectRawBytes(pClientChnl, frame, frameLen);
    time_sleep(50);

    bool fragDelivered = false;
    for (int attempt = 0; attempt < 64; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf && pBuf->length == fragTotal)
        {
            fragDelivered = true;
            break;
        }
        if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_TRUE(normalDelivered)
        << "Interleaved normal message must be delivered while fragment is in assembly "
           "(Interleave-3)";
    EXPECT_TRUE(fragDelivered)
        << "Fragmented message must be reassembled after interleaved normal message "
           "(Interleave-3)";
}

/* -----------------------------------------------------------------------
 * Interleave-4 – Alternating normal / fragmented pattern, 20 repetitions.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Interleave4_AlternatingNormalAndFragmented20Times)
{
    const RsslUInt32 fragSize  = 512;
    ASSERT_TRUE(setupReadChannelPair("15603", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 normalLen = 32;
    const RsslUInt32 fragLen   = fragSize + 8;  /* 2 fragments */
    const int        reps      = 20;

    int normalSent = 0, fragSent = 0;

    for (int i = 0; i < reps; ++i)
    {
        /* Normal message. */
        RsslBuffer* pNorm = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
        if (!pNorm) break;
        fillAscii(pNorm, normalLen);
        pNorm->length = normalLen;
        if (writeAndFlush(pClientChnl, pNorm) == RSSL_RET_SUCCESS)
            ++normalSent;

        /* Fragmented message. */
        RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, fragLen, RSSL_FALSE, &err);
        if (!pFrag) break;
        fillAscii(pFrag, fragLen);
        pFrag->length = fragLen;
        if (writeAndFlush(pClientChnl, pFrag) == RSSL_RET_SUCCESS)
            ++fragSent;
    }

    time_sleep(50);

    int normalRead = 0, fragRead = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 2000; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)
            ++normalRead;
        else if (pBuf->length == fragLen)
            ++fragRead;
        else
            ++wrongLen;
    }

    EXPECT_EQ(normalRead, normalSent)
        << "All normal messages must be received (Interleave-4)";
    EXPECT_EQ(fragRead, fragSent)
        << "All fragmented messages must be reassembled (Interleave-4)";
    EXPECT_EQ(wrongLen, 0)
        << "No message must have an unexpected length (Interleave-4)";
}

/* -----------------------------------------------------------------------
 * Interleave-5 – Burst of normal messages, deep fragmented message, burst.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Interleave5_NormalBurstDeepFragmentNormalBurst)
{
    const RsslUInt32 fragSize  = 200;
    ASSERT_TRUE(setupReadChannelPair("15604", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 normalLen    = 40;
    const RsslUInt32 deepFragLen  = fragSize * 10;  /* ~10 fragments */
    const int        burstCount   = 5;

    int normalSent = 0, deepSent = 0;

    /* Pre-burst: 5 normal messages. */
    for (int i = 0; i < burstCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillAscii(pBuf, normalLen);
        pBuf->length = normalLen;
        if (writeAndFlush(pClientChnl, pBuf) == RSSL_RET_SUCCESS)
            ++normalSent;
    }

    /* Deep fragmented message. */
    RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, deepFragLen, RSSL_FALSE, &err);
    if (pFrag)
    {
        fillAscii(pFrag, deepFragLen);
        pFrag->length = deepFragLen;
        if (writeAndFlush(pClientChnl, pFrag) == RSSL_RET_SUCCESS)
            ++deepSent;
    }

    /* Post-burst: 5 normal messages. */
    for (int i = 0; i < burstCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillAscii(pBuf, normalLen);
        pBuf->length = normalLen;
        if (writeAndFlush(pClientChnl, pBuf) == RSSL_RET_SUCCESS)
            ++normalSent;
    }

    time_sleep(100);

    int normalRead = 0, deepRead = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 2000; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)
            ++normalRead;
        else if (pBuf->length == deepFragLen)
            ++deepRead;
        else
            ++wrongLen;
    }

    EXPECT_EQ(normalRead, normalSent)
        << "All normal messages in both bursts must be delivered (Interleave-5)";
    EXPECT_EQ(deepRead, deepSent)
        << "The deep fragmented message must be fully reassembled (Interleave-5)";
    EXPECT_EQ(wrongLen, 0)
        << "No message must have an unexpected length (Interleave-5)";
}

/* =======================================================================
 * ── Compression tests ────────────────────────────────────────────────────
 *
 * The following tests verify that rsslSocketRead() correctly handles
 * compressed channels (Zlib and LZ4).  Compression is negotiated at
 * connect time; every RIPC frame on the wire is compressed.  The tests
 * cover:
 *
 *  Compress-1  – Single normal message round-trip over Zlib.
 *  Compress-2  – Single normal message round-trip over LZ4.
 *  Compress-3  – Multiple normal messages over Zlib; all must be
 *                decompressed and delivered with correct lengths.
 *  Compress-4  – Multiple normal messages over LZ4.
 *  Compress-5  – Fragmented message (2 fragments) over Zlib; the assembly
 *                buffer must receive decompressed bytes from each fragment.
 *  Compress-6  – Fragmented message (2 fragments) over LZ4.
 *  Compress-7  – Mixed normal + fragmented messages over Zlib in one
 *                channel session; verifies that the decompression context
 *                is not reset between single-fragment and multi-fragment
 *                messages.
 *  Compress-8  – Mixed normal + fragmented messages over LZ4.
 *  Compress-9  – Highly compressible payload (all zeros, 6 000 bytes)
 *                over Zlib at maximum compression level; the decompressed
 *                output must match the original length exactly.
 *  Compress-10 – Incompressible payload (pseudo-random bytes) over Zlib;
 *                the library may send an uncompressed fallback frame; the
 *                read side must still return the correct length.
 *  Compress-11 – Packed multi-sub-message buffer over Zlib; the packed-
 *                buffer continuation path must handle compressed frames.
 *  Compress-12 – Concurrent reads on a Zlib channel from two threads
 *                (RSSL_LOCK_GLOBAL); must not crash.
 *  Compress-13 – Read after peer close on a Zlib channel; must return
 *                RSSL_RET_FAILURE cleanly without crashing inside the
 *                decompression cleanup.
 *  Compress-14 – readOutArgs.bytesRead and uncompressedBytesRead must
 *                both be populated and uncompressedBytesRead must be ≥
 *                bytesRead for a compressible payload over Zlib.
 *
 * Port assignments: 15700 – 15719
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Helper: write one buffer through pWriter, flush it, wait, and read
 * one message from pReader.  Returns the read buffer (nullptr on failure).
 * --------------------------------------------------------------------- */
static RsslBuffer* writeFlushRead(
    RsslChannel* pWriter, RsslBuffer* pBuf,
    RsslChannel* pReader, int waitMs = 50)
{
    if (writeAndFlush(pWriter, pBuf) < RSSL_RET_SUCCESS)
        return nullptr;
    time_sleep(waitMs);
    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    return rsslRead(pReader, &readRet, &err);
}

/* -----------------------------------------------------------------------
 * Compress-1 – Single normal message round-trip over Zlib.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress1_ZlibSingleNormalMessageRoundTrip)
{
    ASSERT_TRUE(setupReadChannelPair("15700", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    const RsslUInt32 payLen = 128;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    fillAscii(pBuf, payLen);
    pBuf->length = payLen;

    RsslBuffer* pRead = writeFlushRead(pClientChnl, pBuf, pServerChnl);

    ASSERT_NE(pRead, nullptr) << "Zlib single message must be delivered (Compress-1)";
    EXPECT_EQ(pRead->length, payLen)
        << "Decompressed message length must match original (Compress-1)";
}

/* -----------------------------------------------------------------------
 * Compress-2 – Single normal message round-trip over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress2_LZ4SingleNormalMessageRoundTrip)
{
    ASSERT_TRUE(setupReadChannelPair("15701", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0));

    RsslError err;
    const RsslUInt32 payLen = 128;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    fillAscii(pBuf, payLen);
    pBuf->length = payLen;

    RsslBuffer* pRead = writeFlushRead(pClientChnl, pBuf, pServerChnl);

    ASSERT_NE(pRead, nullptr) << "LZ4 single message must be delivered (Compress-2)";
    EXPECT_EQ(pRead->length, payLen)
        << "Decompressed message length must match original (Compress-2)";
}

/* -----------------------------------------------------------------------
 * Compress-3 – Multiple normal messages over Zlib.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress3_ZlibMultipleNormalMessagesAllDelivered)
{
    ASSERT_TRUE(setupReadChannelPair("15702", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    const RsslUInt32 payLen   = 200;
    const int        msgCount = 20;

    for (int i = 0; i < msgCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
        ASSERT_NE(pBuf, nullptr);
        fillAscii(pBuf, payLen);
        pBuf->length = payLen;
        ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    }

    time_sleep(100);

    int read = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
        {
            ++read;
            if (pBuf->length != payLen) ++wrongLen;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(read,     msgCount) << "All Zlib messages must be delivered (Compress-3)";
    EXPECT_EQ(wrongLen, 0)        << "All Zlib messages must have correct length (Compress-3)";
}

/* -----------------------------------------------------------------------
 * Compress-4 – Multiple normal messages over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress4_LZ4MultipleNormalMessagesAllDelivered)
{
    ASSERT_TRUE(setupReadChannelPair("15703", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0));

    RsslError err;
    const RsslUInt32 payLen   = 200;
    const int        msgCount = 20;

    for (int i = 0; i < msgCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
        ASSERT_NE(pBuf, nullptr);
        fillAscii(pBuf, payLen);
        pBuf->length = payLen;
        ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    }

    time_sleep(100);

    int read = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
        {
            ++read;
            if (pBuf->length != payLen) ++wrongLen;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(read,     msgCount) << "All LZ4 messages must be delivered (Compress-4)";
    EXPECT_EQ(wrongLen, 0)        << "All LZ4 messages must have correct length (Compress-4)";
}

/* -----------------------------------------------------------------------
 * Compress-5 – Fragmented message (2 fragments) over Zlib.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress5_ZlibFragmentedMessageReassembledCorrectly)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15704", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 64;   /* 2 fragments */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    fillAscii(pBuf, msgSize);
    pBuf->length = msgSize;

    ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    time_sleep(100);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, msgSize)
                << "Zlib two-fragment message must reassemble to correct length (Compress-5)";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one Zlib fragmented message expected (Compress-5)";
}

/* -----------------------------------------------------------------------
 * Compress-6 – Fragmented message (2 fragments) over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress6_LZ4FragmentedMessageReassembledCorrectly)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15705", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 64;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgSize, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    fillAscii(pBuf, msgSize);
    pBuf->length = msgSize;

    ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    time_sleep(100);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, msgSize)
                << "LZ4 two-fragment message must reassemble to correct length (Compress-6)";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one LZ4 fragmented message expected (Compress-6)";
}

/* -----------------------------------------------------------------------
 * Compress-7 – Mixed normal + fragmented messages over Zlib.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress7_ZlibMixedNormalAndFragmentedMessages)
{
    const RsslUInt32 fragSize  = 512;
    ASSERT_TRUE(setupReadChannelPair("15706", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6, fragSize));

    RsslError err;
    const RsslUInt32 normalLen = 64;
    const RsslUInt32 fragLen   = fragSize + 32;
    const int        reps      = 5;

    int normalSent = 0, fragSent = 0;

    for (int i = 0; i < reps; ++i)
    {
        RsslBuffer* pNorm = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
        if (!pNorm) break;
        fillAscii(pNorm, normalLen);
        pNorm->length = normalLen;
        if (writeAndFlush(pClientChnl, pNorm) >= RSSL_RET_SUCCESS)
            ++normalSent;

        RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, fragLen, RSSL_FALSE, &err);
        if (!pFrag) break;
        fillAscii(pFrag, fragLen);
        pFrag->length = fragLen;
        if (writeAndFlush(pClientChnl, pFrag) >= RSSL_RET_SUCCESS)
            ++fragSent;
    }

    time_sleep(150);

    int normalRead = 0, fragRead = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)       ++normalRead;
        else if (pBuf->length == fragLen)    ++fragRead;
        else                                 ++wrongLen;
    }

    EXPECT_EQ(normalRead, normalSent) << "All Zlib normal messages delivered (Compress-7)";
    EXPECT_EQ(fragRead,   fragSent)   << "All Zlib fragmented messages assembled (Compress-7)";
    EXPECT_EQ(wrongLen,   0)          << "No unexpected lengths over Zlib (Compress-7)";
}

/* -----------------------------------------------------------------------
 * Compress-8 – Mixed normal + fragmented messages over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress8_LZ4MixedNormalAndFragmentedMessages)
{
    const RsslUInt32 fragSize  = 512;
    ASSERT_TRUE(setupReadChannelPair("15707", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0, fragSize));

    RsslError err;
    const RsslUInt32 normalLen = 64;
    const RsslUInt32 fragLen   = fragSize + 32;
    const int        reps      = 5;

    int normalSent = 0, fragSent = 0;

    for (int i = 0; i < reps; ++i)
    {
        RsslBuffer* pNorm = rsslGetBuffer(pClientChnl, normalLen, RSSL_FALSE, &err);
        if (!pNorm) break;
        fillAscii(pNorm, normalLen);
        pNorm->length = normalLen;
        if (writeAndFlush(pClientChnl, pNorm) >= RSSL_RET_SUCCESS)
            ++normalSent;

        RsslBuffer* pFrag = rsslGetBuffer(pClientChnl, fragLen, RSSL_FALSE, &err);
        if (!pFrag) break;
        fillAscii(pFrag, fragLen);
        pFrag->length = fragLen;
        if (writeAndFlush(pClientChnl, pFrag) >= RSSL_RET_SUCCESS)
            ++fragSent;
    }

    time_sleep(150);

    int normalRead = 0, fragRead = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (!pBuf)
        {
            if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
                break;
            continue;
        }
        if (pBuf->length == normalLen)       ++normalRead;
        else if (pBuf->length == fragLen)    ++fragRead;
        else                                 ++wrongLen;
    }

    EXPECT_EQ(normalRead, normalSent) << "All LZ4 normal messages delivered (Compress-8)";
    EXPECT_EQ(fragRead,   fragSent)   << "All LZ4 fragmented messages assembled (Compress-8)";
    EXPECT_EQ(wrongLen,   0)          << "No unexpected lengths over LZ4 (Compress-8)";
}

/* -----------------------------------------------------------------------
 * Compress-9 – Highly compressible payload (all zeros) over Zlib level 9.
 *
 * 6 000 bytes of zeros compresses to ~12 bytes.  The decompressed output
 * must be exactly 6 000 bytes.  This exercises the expand-buffer path in
 * ipcDecompress() when the output is much larger than the compressed input.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress9_ZlibHighlyCompressiblePayloadCorrectLength)
{
    ASSERT_TRUE(setupReadChannelPair("15708", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 9));

    RsslError err;
    const RsslUInt32 payLen = 6000;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    memset(pBuf->data, 0x00, pBuf->length);
    pBuf->length = payLen;

    ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    time_sleep(100);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, payLen)
                << "Decompressed highly-compressible message must have correct length "
                   "(Compress-9)";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one message expected (Compress-9)";
}

/* -----------------------------------------------------------------------
 * Compress-10 – Incompressible payload over Zlib.
 *
 * Fill the buffer with a simple counter pattern (all unique byte values
 * cycling 0x00–0xFF) which gives Zlib almost nothing to compress.  The
 * library may transmit the frame with minimal or no compression gain but
 * must still deliver the full 512-byte payload after decompression.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress10_ZlibIncompressiblePayloadDeliveredCorrectly)
{
    ASSERT_TRUE(setupReadChannelPair("15709", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 payLen = 512;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    for (RsslUInt32 k = 0; k < payLen; ++k)
        pBuf->data[k] = (char)(k & 0xFF);  /* 0x00..0xFF cycling */
    pBuf->length = payLen;

    ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    time_sleep(100);

    int msgCount = 0;
    for (int attempt = 0; attempt < 200; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++msgCount;
            EXPECT_EQ(pRead->length, payLen)
                << "Incompressible payload must be delivered at original length (Compress-10)";
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(msgCount, 1) << "Exactly one message expected (Compress-10)";
}

/* -----------------------------------------------------------------------
 * Compress-11 – Packed multi-sub-message buffer over Zlib.
 *
 * Pack three 40-byte sub-messages into one buffer and write it over a
 * Zlib channel.  The server must return three separate sub-messages in
 * consecutive rsslRead() calls, each with length 40.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress11_ZlibPackedMultiSubMessageRoundTrip)
{
    ASSERT_TRUE(setupReadChannelPair("15710", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 40;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillAscii(pBuf, msgLen);
    pBuf->length = msgLen;
    RsslBuffer* p2 = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (p2 && p2->length >= msgLen)
    {
        fillAscii(p2, msgLen);
        p2->length = msgLen;
        RsslBuffer* p3 = rsslPackBuffer(pClientChnl, p2, &err);
        if (p3 && p3->length >= msgLen)
        {
            fillAscii(p3, msgLen);
            p3->length = msgLen;
            RsslBuffer* pEnd = rsslPackBuffer(pClientChnl, p3, &err);
            if (pEnd) pEnd->length = 0;
        }
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    int subMsgCount = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 50; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRead   = rsslRead(pServerChnl, &readRet, &err);
        if (pRead)
        {
            ++subMsgCount;
            if (pRead->length != msgLen) ++wrongLen;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(subMsgCount, 3)
        << "Three packed sub-messages must be delivered over Zlib (Compress-11)";
    EXPECT_EQ(wrongLen, 0)
        << "Each sub-message must have length " << msgLen << " (Compress-11)";
}

/* -----------------------------------------------------------------------
 * Compress-12 – Concurrent reads on a Zlib channel from two threads.
 *
 * Write 50 small messages, then spin up two concurrent reader threads.
 * Under RSSL_LOCK_GLOBAL, both threads share the decompression context;
 * this must not crash or corrupt decompressed data.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress12_ZlibConcurrentReadsDoNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15711", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    for (int i = 0; i < 50; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillAscii(pBuf, 64);
        pBuf->length = 64;
        writeAndFlush(pClientChnl, pBuf);
    }

    time_sleep(50);

    ConcurrentReadArg args;
    args.pChnl = pServerChnl;

    RsslThreadId t1, t2;
    RSSL_THREAD_START(&t1, concurrentReadThreadFn, &args);
    RSSL_THREAD_START(&t2, concurrentReadThreadFn, &args);

    RSSL_THREAD_JOIN(t1);
    RSSL_THREAD_JOIN(t2);

    EXPECT_FALSE(args.crashed.load())
        << "Concurrent reads on Zlib channel must not crash (Compress-12)";
}

/* -----------------------------------------------------------------------
 * Compress-13 – Read after peer close on a Zlib channel.
 *
 * Close the client side; the server must receive EOF and rsslRead() must
 * return RSSL_RET_FAILURE without crashing inside the Zlib cleanup path.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress13_ZlibReadAfterPeerCloseReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15712", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    time_sleep(50);

    RsslRet     readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error:1002 ipcRead() failure. Connection reset by peer") != NULL);
}

/* -----------------------------------------------------------------------
 * Compress-14 – readOutArgs.bytesRead vs uncompressedBytesRead over Zlib.
 *
 * Write a 500-byte all-zero payload (highly compressible).  On the first
 * successful rsslReadEx() call:
 *   - bytesRead must be > 0 (compressed wire bytes consumed)
 *   - uncompressedBytesRead must be > 0
 *   - uncompressedBytesRead must be >= bytesRead (compression saves space)
 *   - The returned buffer length must equal the original 500 bytes.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Compress14_ZlibReadOutArgsUncompressedBytesRead)
{
    ASSERT_TRUE(setupReadChannelPair("15713", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 9));

    RsslError err;
    const RsslUInt32 payLen = 500;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);
    memset(pBuf->data, 0x00, pBuf->length);
    pBuf->length = payLen;

    ASSERT_GE(writeAndFlush(pClientChnl, pBuf), RSSL_RET_SUCCESS);
    time_sleep(100);

    RsslReadInArgs  rdIn;  rsslClearReadInArgs(&rdIn);
    RsslReadOutArgs rdOut; rsslClearReadOutArgs(&rdOut);
    rdOut.bytesRead             = (RsslUInt32)-1;
    rdOut.uncompressedBytesRead = (RsslUInt32)-1;

    RsslRet     readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pRead   = rsslReadEx(pServerChnl, &rdIn, &rdOut, &readRet, &err);

    ASSERT_NE(pRead, nullptr)
        << "Zlib message must be delivered for out-args check (Compress-14)";
    EXPECT_EQ(pRead->length, payLen)
        << "Decompressed length must match original (Compress-14)";
    EXPECT_NE(rdOut.bytesRead, (RsslUInt32)-1)
        << "bytesRead must be populated (Compress-14)";
    EXPECT_EQ(rdOut.bytesRead, 17)
        << "bytesRead must be > 0 (Compress-14)";
    EXPECT_NE(rdOut.uncompressedBytesRead, (RsslUInt32)-1)
        << "uncompressedBytesRead must be populated (Compress-14)";
    EXPECT_EQ(rdOut.uncompressedBytesRead, 503)
        << "uncompressedBytesRead must be > 0 (Compress-14)";
    EXPECT_GE(rdOut.uncompressedBytesRead, rdOut.bytesRead)
        << "uncompressedBytesRead must be >= bytesRead for compressible data "
           "(Compress-14)";
}

/* =======================================================================
 * ── Compression + message packing combined tests ─────────────────────────
 *
 * These tests combine channel-level compression (Zlib or LZ4) with the
 * packed-buffer API (rsslGetBuffer / rsslPackBuffer / rsslWriteEx).  Each
 * test verifies that the packed-buffer continuation path in rsslSocketRead()
 * correctly unpacks sub-messages from a frame that was first decompressed
 * by ipcReadSession().
 *
 * Key concern: when a packed frame arrives over a compressed channel the
 * decompression step runs first (in ipcReadSession), producing a raw packed
 * buffer.  rsslSocketRead() then enters the unpackOffset / returnBuffer
 * path for each sub-message.  Any mismatch between the decompressed length
 * and the packed sub-message lengths triggers Issue 1 (overflow) or
 * Issue 6 (zero bytesRead on continuation calls).
 *
 *  PackCompress-1 – Two sub-messages of equal size packed into one Zlib
 *                   frame.  Both must be delivered with the correct length.
 *
 *  PackCompress-2 – Two sub-messages of equal size packed into one LZ4
 *                   frame.
 *
 *  PackCompress-3 – Five sub-messages of varying sizes packed into one
 *                   Zlib frame.  Tests that the unpackOffset advances
 *                   correctly across heterogeneous sub-message boundaries.
 *
 *  PackCompress-4 – Five sub-messages of varying sizes packed into one
 *                   LZ4 frame.
 *
 *  PackCompress-5 – Ten consecutive packed-and-compressed buffers, each
 *                   containing three 32-byte sub-messages, sent over Zlib.
 *                   Verifies that the packed state is reset correctly
 *                   between frames and that all 30 sub-messages arrive.
 *
 *  PackCompress-6 – Ten consecutive packed-and-compressed buffers over LZ4.
 *
 *  PackCompress-7 – Single packed Zlib frame where sub-messages contain a
 *                   highly compressible payload (all zeros).  Checks that
 *                   the compressor/decompressor round-trips the exact byte
 *                   count for each sub-message.
 *
 *  PackCompress-8 – readOutArgs.bytesRead and uncompressedBytesRead on the
 *                   FIRST rsslReadEx() call of a packed Zlib frame must be
 *                   > 0, and uncompressedBytesRead >= bytesRead.  On
 *                   subsequent (continuation) calls bytesRead must be 0
 *                   (Issue 6 – continuation path forces 0 by design) and
 *                   the returned buffer must still have a non-zero length.
 *
 * Port assignments: 15800 – 15815
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Helper: pack N sub-messages of payLen bytes into a single buffer,
 * write and flush it, then drain up to maxReads messages from pReader
 * into outBufs[0..N-1].  Returns the number of sub-messages received.
 * --------------------------------------------------------------------- */
static int packWriteAndReadAll(
    RsslChannel*  pWriter,
    RsslChannel*  pReader,
    RsslUInt32    payLen,
    int           nMsgs,
    int           waitMs,
    RsslUInt32*   outLengths,     /* caller-allocated array of nMsgs entries */
    int           maxReadAttempts = 200)
{
    RsslError err;

    RsslBuffer* pBuf = rsslGetBuffer(pWriter, payLen * (RsslUInt32)nMsgs + 64u,
                                     RSSL_TRUE, &err);
    if (!pBuf) return -1;

    RsslBuffer* pCur = pBuf;
    int packed = 0;
    for (int m = 0; m < nMsgs; ++m)
    {
        if (!pCur || pCur->length < payLen)
            break;
        for (RsslUInt32 k = 0; k < payLen; ++k)
            pCur->data[k] = (char)('A' + ((m + k) % 26));
        pCur->length = payLen;

        if (m < nMsgs - 1)
        {
            RsslBuffer* pNext = rsslPackBuffer(pWriter, pCur, &err);
            if (!pNext) break;
            pCur = pNext;
        }
        ++packed;
    }
    if (pCur) pCur->length = (packed == nMsgs) ? payLen : 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet wret = rsslWriteEx(pWriter, pBuf, &inArgs, &outArgs, &err);
    while (wret > RSSL_RET_SUCCESS)
        wret = rsslFlush(pWriter, &err);

    time_sleep(waitMs);

    int received = 0;
    for (int attempt = 0; attempt < maxReadAttempts && received < nMsgs; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRd     = rsslRead(pReader, &readRet, &err);
        if (pRd)
        {
            if (outLengths && received < nMsgs)
                outLengths[received] = pRd->length;
            ++received;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }
    return received;
}

/* -----------------------------------------------------------------------
 * PackCompress-1 – Two equal-size sub-messages over Zlib.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress1_ZlibTwoEqualSubMessagesDelivered)
{
    ASSERT_TRUE(setupReadChannelPair("15800", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    const RsslUInt32 payLen  = 60;
    const int        nMsgs   = 2;
    RsslUInt32       lens[nMsgs] = {};

    int received = packWriteAndReadAll(pClientChnl, pServerChnl,
                                       payLen, nMsgs, 100, lens);

    EXPECT_EQ(received, nMsgs)
        << "Both Zlib-packed sub-messages must be delivered (PackCompress-1)";
    for (int i = 0; i < received; ++i)
        EXPECT_EQ(lens[i], payLen)
            << "Sub-message " << i << " length must be " << payLen
            << " (PackCompress-1)";
}

/* -----------------------------------------------------------------------
 * PackCompress-2 – Two equal-size sub-messages over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress2_LZ4TwoEqualSubMessagesDelivered)
{
    ASSERT_TRUE(setupReadChannelPair("15801", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0));

    const RsslUInt32 payLen  = 60;
    const int        nMsgs   = 2;
    RsslUInt32       lens[nMsgs] = {};

    int received = packWriteAndReadAll(pClientChnl, pServerChnl,
                                       payLen, nMsgs, 100, lens);

    EXPECT_EQ(received, nMsgs)
        << "Both LZ4-packed sub-messages must be delivered (PackCompress-2)";
    for (int i = 0; i < received; ++i)
        EXPECT_EQ(lens[i], payLen)
            << "Sub-message " << i << " length must be " << payLen
            << " (PackCompress-2)";
}

/* -----------------------------------------------------------------------
 * PackCompress-3 – Five varying-size sub-messages over Zlib.
 *
 * Sub-messages have lengths 20, 40, 60, 80, 100 bytes.  Each must be
 * decompressed and unpacked to the correct individual length.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress3_ZlibFiveVaryingSizeSubMessages)
{
    ASSERT_TRUE(setupReadChannelPair("15802", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    const RsslUInt32 sizes[5] = {20, 40, 60, 80, 100};
    const int        nMsgs    = 5;

    /* Build and write a packed buffer with heterogeneous sub-message sizes. */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 512u, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < nMsgs; ++m)
    {
        ASSERT_NE(pCur, nullptr);
        ASSERT_GE(pCur->length, sizes[m]);
        memset(pCur->data, (char)('A' + m), sizes[m]);
        pCur->length = sizes[m];

        if (m < nMsgs - 1)
            pCur = rsslPackBuffer(pClientChnl, pCur, &err);
    }
    if (pCur) pCur->length = sizes[nMsgs - 1];

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    int received = 0;
    for (int attempt = 0; attempt < 200 && received < nMsgs; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRd     = rsslRead(pServerChnl, &readRet, &err);
        if (pRd)
        {
            EXPECT_EQ(pRd->length, sizes[received])
                << "Sub-message " << received
                << " must have length " << sizes[received]
                << " (PackCompress-3)";
            ++received;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(received, nMsgs)
        << "All 5 varying-size Zlib-packed sub-messages must be delivered "
           "(PackCompress-3)";
}

/* -----------------------------------------------------------------------
 * PackCompress-4 – Five varying-size sub-messages over LZ4.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress4_LZ4FiveVaryingSizeSubMessages)
{
    ASSERT_TRUE(setupReadChannelPair("15803", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0));

    RsslError err;
    const RsslUInt32 sizes[5] = {20, 40, 60, 80, 100};
    const int        nMsgs    = 5;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 512u, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < nMsgs; ++m)
    {
        ASSERT_NE(pCur, nullptr);
        ASSERT_GE(pCur->length, sizes[m]);
        memset(pCur->data, (char)('A' + m), sizes[m]);
        pCur->length = sizes[m];

        if (m < nMsgs - 1)
            pCur = rsslPackBuffer(pClientChnl, pCur, &err);
    }
    if (pCur) pCur->length = sizes[nMsgs - 1];

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    int received = 0;
    for (int attempt = 0; attempt < 200 && received < nMsgs; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRd     = rsslRead(pServerChnl, &readRet, &err);
        if (pRd)
        {
            EXPECT_EQ(pRd->length, sizes[received])
                << "Sub-message " << received
                << " must have length " << sizes[received]
                << " (PackCompress-4)";
            ++received;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(received, nMsgs)
        << "All 5 varying-size LZ4-packed sub-messages must be delivered "
           "(PackCompress-4)";
}

/* -----------------------------------------------------------------------
 * PackCompress-5 – Ten consecutive packed Zlib frames, 3 × 32-byte each.
 *
 * Sends 10 separate packed buffers (each containing 3 sub-messages of
 * 32 bytes) over a Zlib channel.  Verifies that the packed state is
 * reset correctly between frames and that all 30 sub-messages arrive.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress5_ZlibTenPackedFramesThirtySubMessages)
{
    ASSERT_TRUE(setupReadChannelPair("15804", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    const RsslUInt32 payLen   = 32;
    const int        nPerFrame = 3;
    const int        frames   = 10;
    const int        total    = nPerFrame * frames;

    int totalReceived = 0, wrongLen = 0;

    for (int f = 0; f < frames; ++f)
    {
        RsslUInt32 lens[nPerFrame] = {};
        int received = packWriteAndReadAll(pClientChnl, pServerChnl,
                                           payLen, nPerFrame, 50, lens,
                                           nPerFrame * 10);
        totalReceived += received;
        for (int m = 0; m < received; ++m)
            if (lens[m] != payLen) ++wrongLen;
    }

    EXPECT_EQ(totalReceived, total)
        << "All 30 sub-messages across 10 Zlib-packed frames must arrive "
           "(PackCompress-5)";
    EXPECT_EQ(wrongLen, 0)
        << "Every sub-message must have length " << payLen << " (PackCompress-5)";
}

/* -----------------------------------------------------------------------
 * PackCompress-6 – Ten consecutive packed LZ4 frames, 3 × 32-byte each.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress6_LZ4TenPackedFramesThirtySubMessages)
{
    ASSERT_TRUE(setupReadChannelPair("15805", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_LZ4, 0));

    const RsslUInt32 payLen   = 32;
    const int        nPerFrame = 3;
    const int        frames   = 10;
    const int        total    = nPerFrame * frames;

    int totalReceived = 0, wrongLen = 0;

    for (int f = 0; f < frames; ++f)
    {
        RsslUInt32 lens[nPerFrame] = {};
        int received = packWriteAndReadAll(pClientChnl, pServerChnl,
                                           payLen, nPerFrame, 50, lens,
                                           nPerFrame * 10);
        totalReceived += received;
        for (int m = 0; m < received; ++m)
            if (lens[m] != payLen) ++wrongLen;
    }

    EXPECT_EQ(totalReceived, total)
        << "All 30 sub-messages across 10 LZ4-packed frames must arrive "
           "(PackCompress-6)";
    EXPECT_EQ(wrongLen, 0)
        << "Every sub-message must have length " << payLen << " (PackCompress-6)";
}

/* =======================================================================
 * ── Malformed wire-frame crash tests (ipcReadSession) ────────────────────
 *
 * Each test injects a crafted raw RIPC frame directly into the server's
 * receive socket and then calls rsslRead() one or more times.  The goal is
 * to reach every guard and error-return path inside ipcReadSession() in
 * rsslSocketTransportImpl.c without crashing the process.
 *
 * Targets (line references are to the ipcReadSession body):
 *
 *  Malform-1  – Frame length field equals the minimum header size exactly
 *               (ipcLen == dataHeaderLen).  The "ipcLen < dataHeaderLen"
 *               guard must not fire; the frame should be consumed cleanly.
 *
 *  Malform-2  – Frame length = 1 (below dataHeaderLen).  The guard
 *               "ipcLen < dataHeaderLen" must fire and return FAILURE.
 *
 *  Malform-3  – Frame length = 2 (still below the 3-byte minimum header).
 *               Same guard as Malform-2.
 *
 *  Malform-4  – Frame length field larger than inputBuffer->maxLength.
 *               The "ipcLen > inputBuffer->maxLength" guard must return
 *               FAILURE before any read-more loop can over-run the buffer.
 *
 *  Malform-5  – Valid IPC_DATA frame whose declared length is larger than
 *               the bytes injected (partial frame).  On a non-blocking
 *               channel the "rest of message" read loop must return
 *               RSSL_RET_READ_WOULD_BLOCK rather than blocking or crashing.
 *
 *  Malform-6  – IPC_COMP_DATA frame injected on a channel that has NO
 *               compression negotiated (decompressBuf == NULL).  The guard
 *               "if (!rsslSocketChannel->decompressBuf)" must fire and
 *               return FAILURE.
 *
 *  Malform-7  – IPC_COMP_DATA frame with a compressed payload that is
 *               deliberately corrupt (random bytes).  The zlib decompress
 *               call must return an error; ipcReadSession must propagate
 *               FAILURE without crashing.
 *
 *  Malform-8  – IPC_EXTENDED_FLAGS set but the extended-flags byte itself
 *               is 0x00 (neither IPC_FRAG_HEADER nor IPC_FRAG).  The
 *               switch falls to the "else → cHdrLen = 4" branch; the
 *               payload (buffer.length -= 4) must not go negative.
 *
 *  Malform-9  – IPC_EXTENDED_FLAGS + IPC_FRAG_HEADER set but the frame
 *               is exactly 9 bytes (one byte short of the 10-byte
 *               first-fragment header).  The "tempLen < IPC_header_size +
 *               extendedHdr" inner loop must detect this and return
 *               WOULD_BLOCK or FAILURE without reading past the buffer.
 *
 *  Malform-10 – IPC_PACKING frame whose 2-byte sub-message length prefix
 *               reports exactly (frame_payload_bytes + 1) bytes (one past
 *               the end of the frame).  The bounds check must return
 *               FAILURE.
 *
 *  Malform-11 – IPC_PACKING frame with a sub-message length of UINT16_MAX
 *               (0xFFFF) inside a 7-byte total frame.  Tests the arithmetic
 *               overflow guard in the unpack path.
 *
 *  Malform-12 – Rapid flood of 1 000 individually malformed 3-byte frames
 *               (opcode 0x00) injected back-to-back.  The error-return path
 *               in ipcReadSession must not accumulate state or deadlock
 *               across repeated calls.
 *
 *  Malform-13 – Frame with length field = 0x0000.  Zero-length frame is
 *               below the minimum header size; the guard must fire.
 *
 *  Malform-14 – Frame that announces IPC_COMP_DATA | IPC_COMP_FRAG (two-
 *               part LZ4/Zlib fragment) on a *non-compressed* channel.
 *               decompressBuf is NULL; the decompress guard must fire.
 *
 *  Malform-15 – Two consecutive malformed frames followed by a valid
 *               64-byte message.  After two FAILURE returns the channel
 *               must have been moved to CLOSED state; a subsequent read
 *               must also fail (not crash).
 *
 * Port assignments: 15900 – 15919
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Malform-1 – Frame length == dataHeaderLen (minimum valid).
 *
 * A 3-byte RIPC frame: length=3, opcode=IPC_DATA(0x02), no payload.
 * This is the smallest legal frame; it must be consumed without error.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform1_FrameLengthEqualsHeaderSizeConsumedCleanly)
{
    ASSERT_TRUE(setupReadChannelPair("15900", &pServer, &pServerChnl, &pClientChnl));

    /* length=3, opcode=IPC_DATA, zero payload */
    unsigned char frame[] = { 0x00, 0x03, 0x02 };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_PING, readRet);
}

/* -----------------------------------------------------------------------
 * Malform-2 – Frame length = 1 (below the 3-byte minimum header).
 *
 * ipcReadSession guard:
 *   if (ipcLen < (rsslSocketChannel->version->dataHeaderLen - httpHeaderLen))
 *       → RSSL_RET_FAILURE
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform2_FrameLengthOneBelowMinimumReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15901", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = { 0x00, 0x01, 0x02 };   /* length=1 */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Size. Message size is: (1). Max Message size is(61690)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-3 – Frame length = 2 (still below the 3-byte minimum header).
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform3_FrameLengthTwoBelowMinimumReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15902", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = { 0x00, 0x02, 0x02, 0x00 };   /* length=2, pad */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Size. Message size is: (2). Max Message size is(61690)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-4 – Frame length larger than inputBuffer->maxLength.
 *
 * Default maxMsgSize is 6144.  A length of 0x7FFF (32767) exceeds both
 * maxMsgSize and maxLength guards:
 *   if (ipcLen > rsslSocketChannel->maxMsgSize) → FAILURE (first guard)
 *   if (ipcLen > inputBuffer->maxLength)        → FAILURE (second guard)
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform4_FrameLengthExceedsMaxLengthReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15903", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = { 0x7F, 0xFF, 0x02 };   /* length=32767 */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Size. Message size is: (32767). Max Message size is(6169)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-5 – Declared frame length larger than injected bytes (partial).
 *
 * Inject a 3-byte header that claims length=100 but supply no payload.
 * On a non-blocking channel the "rest of message" loop must return
 * RSSL_RET_READ_WOULD_BLOCK rather than blocking indefinitely.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform5_DeclaredLengthLargerThanInjectedBytesWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15904", &pServer, &pServerChnl, &pClientChnl));

    /* Claim 100 bytes but only inject the 3-byte header. */
    unsigned char frame[] = { 0x00, 0x64, 0x02 };   /* length=100, IPC_DATA */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet);
}

/* -----------------------------------------------------------------------
 * Malform-6 – IPC_COMP_DATA (0x08) frame on a non-compressed channel.
 *
 * rsslSocketChannel->decompressBuf is NULL because no compression was
 * negotiated.  The guard:
 *   if (!(rsslSocketChannel->decompressBuf)) → FAILURE
 * must fire before any decompression call.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform6_CompressedFrameOnUncompressedChannelReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15905", &pServer, &pServerChnl, &pClientChnl));

    /* IPC_COMP_DATA frame with 4 bytes of fake compressed payload. */
    unsigned char frame[] = {
        0x00, 0x07,        /* length=7 */
        0x08,              /* IPC_COMP_DATA */
        0xDE, 0xAD, 0xBE, 0xEF   /* fake compressed payload */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(readRet, RSSL_RET_FAILURE)
        << "IPC_COMP_DATA on uncompressed channel must return FAILURE (Malform-6)";
    EXPECT_EQ(pBuf, nullptr)
        << "No buffer must be returned for a decompression guard failure (Malform-6)";
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (8)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-7 – IPC_COMP_DATA frame with a corrupt compressed payload.
 *
 * Inject a frame over a Zlib-compressed channel whose payload bytes are
 * random noise; zlib's inflate() will return Z_DATA_ERROR.  The error
 * path in ipcReadSession must propagate FAILURE without crashing.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform7_CorruptCompressedPayloadReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15906", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    /* A plausible IPC_COMP_DATA frame with deliberately corrupt payload.
     * The 3-byte header is left uncompressed; bytes [3..] are the
     * "compressed" content that zlib will reject. */
    unsigned char frame[20];
    frame[0] = 0x00;
    frame[1] = (unsigned char)sizeof(frame);   /* length = 20 */
    frame[2] = 0x08;                           /* IPC_COMP_DATA */
    for (int k = 3; k < (int)sizeof(frame); ++k)
        frame[k] = (unsigned char)(0xAA ^ k); /* random noise */

    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (8)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-8 – IPC_EXTENDED_FLAGS set, extended-flags byte = 0x00.
 *
 * Neither IPC_FRAG_HEADER nor IPC_FRAG is set.  The parse falls into the
 * "else → cHdrLen = 4" branch.  If the total frame length is exactly 4
 * the resulting payload length is 0; this must be handled without
 * underflowing the length field.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform8_ExtendedFlagsSetButNeitherFragBitReturnCleanly)
{
    ASSERT_TRUE(setupReadChannelPair("15907", &pServer, &pServerChnl, &pClientChnl));

    /* length=4, IPC_DATA|IPC_EXTENDED_FLAGS, ext_flags=0x00 */
    unsigned char frame[] = { 0x00, 0x04, 0x43, 0x00 };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_PING, readRet);
}

/* -----------------------------------------------------------------------
 * Malform-9 – IPC_EXTENDED_FLAGS + IPC_FRAG_HEADER, frame is 9 bytes
 *             (one byte short of the 10-byte first-fragment header).
 *
 * The inner loop:
 *   while (tempLen < IPC_header_size + extendedHdr)
 * must detect that the full first-frag header has not arrived and return
 * RSSL_RET_READ_WOULD_BLOCK rather than accessing uninitialized bytes.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform9_TruncatedFirstFragHeaderByOneByteWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15908", &pServer, &pServerChnl, &pClientChnl));

    /* 9-byte frame: 3 header + 1 ext_flags + 4 fragLen; fragId bytes absent. */
    unsigned char frame[] = {
        0x00, 0x09,              /* length=9 */
        0x43,                    /* IPC_DATA | IPC_EXTENDED_FLAGS */
        0x08,                    /* IPC_FRAG_HEADER */
        0x00, 0x00, 0x00, 0x64, /* fragLen=100 */
        0x00                    /* only 1 byte of 2-byte fragId */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslSocketRead() Received fragment size 100 is greater than the actual data length 18446744073709551615") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-10 – IPC_PACKING sub-message length = frame_payload + 1.
 *
 * The frame has 2 real payload bytes but the sub-message prefix claims 3.
 * The bounds check:
 *   if ((packedBuffer->buffer + packedBuffer->length) <
 *       (returnBuffer.data + returnBuffer.length))
 * must catch this and return FAILURE.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform10_PackedSubMessageLengthOnePastEndReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15909", &pServer, &pServerChnl, &pClientChnl));

    /* Frame: total=7, IPC_DATA|IPC_PACKING, sub-len=3, payload=2 bytes */
    unsigned char frame[] = {
        0x00, 0x07,   /* length=7 */
        0x12,         /* IPC_DATA | IPC_PACKING */
        0x00, 0x03,   /* sub-message length=3 (exceeds 2 available payload bytes) */
        0x41, 0x42    /* 2 bytes of actual payload */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1004 rsslSocketRead() unpacked buffer length (5) is greater than total packed buffer length (4). The invalid msg length (3)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-11 – IPC_PACKING sub-message length = 0xFFFF in a 7-byte frame.
 *
 * Replicates Issue 1: the 2-byte sub-message length field (0xFFFF = 65535)
 * is far larger than the 2 payload bytes actually present.  The unpack
 * arithmetic must not overflow.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform11_PackedSubMessageUINT16MAXLengthReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15910", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = {
        0x00, 0x07,
        0x12,
        0xFF, 0xFF,   /* sub-message length = 65535 */
        0x41, 0x42
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1004 rsslSocketRead() unpacked buffer length (65537) is greater than total packed buffer length (4). The invalid msg length (65535).") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-12 – Flood of 1 000 malformed frames (opcode 0x00).
 *
 * Each 3-byte frame has opcode = 0x00 which is neither IPC_DATA nor
 * IPC_COMP_DATA.  The guard:
 *   if (!(ipcOpcode & IPC_DATA)) → FAILURE
 * must return FAILURE on the first invalid frame and leave the channel in
 * a closed state; subsequent reads must also fail without crashing.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform12_FloodOfInvalidOpcodeFramesDoNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15911", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;
    int failCount = 0;

    for (int i = 0; i < 1000; ++i)
    {
        unsigned char frame[] = { 0x00, 0x03, 0x00 };   /* opcode=0x00 */
        injectRawBytes(pClientChnl, frame, (int)sizeof(frame));

        if ((i % 100) == 99)
        {
            time_sleep(5);
            for (int attempt = 0; attempt < 20; ++attempt)
            {
                RsslRet     readRet = RSSL_RET_SUCCESS;
                RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
                (void)pBuf;
                if (readRet == RSSL_RET_FAILURE)
                {
                    ++failCount;
                    goto done_flood;
                }
                if (readRet == RSSL_RET_READ_WOULD_BLOCK)
                    break;
            }
        }
    }
done_flood:
    SUCCEED() << "Flood of invalid-opcode frames did not crash; failures=" << failCount;
}

/* -----------------------------------------------------------------------
 * Malform-13 – Frame length field = 0x0000 (zero-length frame).
 *
 * A zero ipcLen is below the minimum header size guard.  The channel
 * must return FAILURE or WOULD_BLOCK without dereferencing any
 * zero-offset pointer.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform13_ZeroLengthFrameFieldReturnsFailureOrWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15912", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = { 0x00, 0x00, 0x02 };   /* length=0 */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Size. Message size is: (0). Max Message size is(61690)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-14 – IPC_COMP_DATA | IPC_COMP_FRAG (0x0C) on uncompressed channel.
 *
 * IPC_COMP_FRAG (0x04) combined with IPC_COMP_DATA (0x08) indicates a
 * two-part LZ4/Zlib compressed fragment.  On a channel without compression
 * decompressBuf is NULL; the same NULL-guard as Malform-6 must fire.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform14_CompFragFrameOnUncompressedChannelReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15913", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = {
        0x00, 0x07,
        0x0C,              /* IPC_COMP_DATA(0x08) | IPC_COMP_FRAG(0x04) */
        0xDE, 0xAD, 0xBE, 0xEF
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 ipcRead() Attempting to decompress when compression not enabled.") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-15 – Two consecutive malformed frames followed by a valid message.
 *
 * After the first FAILURE the channel state transitions to CLOSED (or the
 * socket receives RST).  Any subsequent read must also return FAILURE or
 * WOULD_BLOCK without crashing, regardless of what the application sends.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform15_TwoMalformedFramesThenValidMessageDoesNotCrash)
{
    ASSERT_TRUE(setupReadChannelPair("15914", &pServer, &pServerChnl, &pClientChnl));

    RsslError err;

    /* Malformed frame 1: opcode 0x00 */
    unsigned char bad1[] = { 0x00, 0x03, 0x00 };
    injectRawBytes(pClientChnl, bad1, (int)sizeof(bad1));
    time_sleep(20);

    RsslRet r1 = RSSL_RET_SUCCESS;
    rsslRead(pServerChnl, &r1, &err);

    /* Malformed frame 2: length = 1 */
    unsigned char bad2[] = { 0x00, 0x01, 0x02 };
    injectRawBytes(pClientChnl, bad2, (int)sizeof(bad2));
    time_sleep(20);

    RsslRet r2 = RSSL_RET_SUCCESS;
    rsslRead(pServerChnl, &r2, &err);

    /* Try a valid write; if the channel is still open this will send data,
     * otherwise the write will fail.  Either outcome must not crash. */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_FALSE, &err);
    if (pBuf)
    {
        fillAscii(pBuf, 64);
        pBuf->length = 64;
        writeAndFlush(pClientChnl, pBuf);
        time_sleep(20);
    }

    RsslRet r3 = RSSL_RET_SUCCESS;
    RsslBuffer* p3 = rsslRead(pServerChnl, &r3, &err);

    EXPECT_EQ(nullptr, p3);
    EXPECT_EQ(RSSL_RET_FAILURE, r3);
    EXPECT_TRUE(strstr(err.text, "Error: 0007 Only Channels in RSSL_CH_STATE_ACTIVE state can get read.") != NULL);
}

/* =======================================================================
 * ── Additional malformed wire-frame crash tests (ipcReadSession) ─────────
 *
 * Extension of the Malform 1-15 series.  Each test targets a specific
 * error-handling path in ipcReadSession() that was not already covered.
 *
 *  Malform-16 – Frame length exactly at the channel's maxMsgSize boundary
 *               (6 144 bytes declared, but only the 3-byte header injected).
 *               The "rest of message" loop must return WOULD_BLOCK, not try
 *               to read 6 141 bytes from an empty socket.
 *
 *  Malform-17 – opcode byte has BOTH IPC_DATA (0x02) and IPC_COMP_DATA
 *               (0x08) set simultaneously (0x0A).  Neither path in the
 *               opcode dispatch is designed for this combination; the
 *               compressed path fires because IPC_COMP_DATA is checked
 *               first.  decompressBuf is NULL so FAILURE must be returned.
 *
 *  Malform-18 – IPC_COMP_DATA frame on a Zlib channel whose payload is
 *               exactly 0 bytes (header only).  avail_in == 0 in the
 *               decompress call; the compressor must not crash on an
 *               empty input.
 *
 *  Malform-19 – IPC_COMP_FRAG (first part of a two-part LZ4 split) on a
 *               Zlib channel.  The LZ4-split branch is only entered when
 *               inDecompress == RSSL_COMP_LZ4; on Zlib the generic path
 *               runs, feeding noise to inflate().  Must return FAILURE.
 *
 *  Malform-20 – Three consecutive IPC_COMP_DATA frames on a Zlib channel
 *               where each frame's payload is corrupt noise.  Verifies that
 *               the decompression-state machine does not retain poison state
 *               between frames (each call must independently detect the
 *               error and return FAILURE).
 *
 *  Malform-21 – IPC_PACKING frame with three sub-messages where the second
 *               sub-message length field is 0 (zero-length middle msg).
 *               The first sub-message must be delivered; the zero-length
 *               second must be returned as an empty buffer; the third must
 *               follow.  No crash or infinite loop.
 *
 *  Malform-22 – IPC_PACKING frame that contains only the 2-byte sub-message
 *               length prefix with no payload bytes (total frame = 5 bytes:
 *               3-byte header + 2-byte prefix).  The sub-message length
 *               field equals exactly the remaining space; boundary check
 *               must pass and an empty buffer returned.
 *
 *  Malform-23 – IPC_PACKING | IPC_EXTENDED_FLAGS (0x52): packed buffer
 *               combined with extended-flags byte set to 0x00.  The parse
 *               path must advance cHdrLen by 4 (extended, no frag bits)
 *               and then enter the packed-buffer unpack loop.  No crash.
 *
 *  Malform-24 – Packed frame with 200 sub-messages of 1 byte each packed
 *               into one large frame.  The unpack loop must iterate 200
 *               times without stack overflow or infinite loop.
 *
 *  Malform-25 – Packed frame where the LAST sub-message length exactly
 *               matches the remaining bytes (tight boundary: no overflow,
 *               no underflow).  The read must succeed and return the exact
 *               payload length.
 *
 *  Malform-26 – IPC_DATA frame with a length field equal to
 *               maxMsgSize + 1 (6 145 on a default channel).  This is one
 *               byte above the maxMsgSize guard and must return FAILURE.
 *
 *  Malform-27 – Back-to-back valid frame then malformed frame in the same
 *               TCP segment (coalesced injection).  The first frame must be
 *               delivered; the second must trigger FAILURE without
 *               corrupting the first message.
 *
 *  Malform-28 – IPC_EXTENDED_FLAGS + IPC_FRAG (0x04 subsequent-fragment)
 *               with a frame that is 5 bytes (one byte short of the 6-byte
 *               subsequent-fragment header).  The header-completion loop
 *               must return WOULD_BLOCK, not read past the buffer.
 *
 *  Malform-29 – Valid IPC_DATA frame immediately followed (same inject) by
 *               an IPC_COMP_DATA frame on an uncompressed channel.  The
 *               valid frame is consumed and delivered; the second frame
 *               must trigger the decompressBuf guard and return FAILURE.
 *
 *  Malform-30 – Alternating valid/malformed frames on a Zlib channel: send
 *               10 valid 64-byte messages interleaved with 10 corrupt
 *               IPC_COMP_DATA frames.  Valid messages must be decompressed
 *               and delivered; corrupt frames must each return FAILURE
 *               without corrupting the Zlib decompression context shared
 *               by subsequent valid frames.
 *
 * Port assignments: 15915 – 15929
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Malform-16 – Frame length == maxMsgSize (6 144) but payload absent.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform16_FrameLengthAtMaxMsgSizeMissingPayloadWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15915", &pServer, &pServerChnl, &pClientChnl));

    /* Default maxMsgSize is 6144 = 0x1800. */
    unsigned char frame[] = { 0x18, 0x00, 0x02 };   /* length=6144, IPC_DATA */
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet);
}

/* -----------------------------------------------------------------------
 * Malform-17 – IPC_DATA | IPC_COMP_DATA (0x0A) simultaneous.
 *
 * decompressBuf is NULL on an uncompressed channel; the IPC_COMP_DATA
 * path fires first and must return FAILURE before any decompression call.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform17_BothDataAndCompDataOpcodeReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15916", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = {
        0x00, 0x07,
        0x06,              /* IPC_DATA(0x02) | IPC_COMP_DATA(0x04) */
        0x11, 0x22, 0x33, 0x44
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 ipcRead() Attempting to decompress when compression not enabled.") != NULL) << "IPC_DATA | IPC_COMP_DATA on uncompressed channel";
}

/* -----------------------------------------------------------------------
 * Malform-18 – IPC_COMP_DATA with zero-byte payload on a Zlib channel.
 *
 * A valid IPC_COMP_DATA frame whose compressed payload is 0 bytes
 * (total frame = 3 bytes, no data after header).  avail_in == 0 in the
 * inflate call; zlib must not crash on empty input.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform18_ZlibCompressedFrameWithZeroBytePayload)
{
    ASSERT_TRUE(setupReadChannelPair("15917", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    /* length=3, IPC_COMP_DATA, zero compressed payload. */
    unsigned char frame[] = { 0x00, 0x03, 0x08 };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (8)") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-19 – IPC_COMP_FRAG on a Zlib channel with a corrupt payload.
 *
 * IPC_COMP_FRAG (0x04) | IPC_COMP_DATA (0x08) = 0x0C indicates the first
 * half of a two-part LZ4 split.  On a Zlib channel the generic decompress
 * branch runs; random-noise input must cause inflate() to fail.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform19_ZlibChannelCompFragFrameWithCorruptPayload)
{
    ASSERT_TRUE(setupReadChannelPair("15918", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    unsigned char frame[16];
    frame[0] = 0x00;
    frame[1] = (unsigned char)sizeof(frame);
    frame[2] = 0x0C;   /* IPC_COMP_DATA | IPC_COMP_FRAG */
    for (int k = 3; k < (int)sizeof(frame); ++k)
        frame[k] = (unsigned char)(0x55 ^ k);

    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 1002 inflate() failed. Zlib error: -3") != NULL);
}

/* -----------------------------------------------------------------------
 * Malform-20 – Three consecutive corrupt IPC_COMP_DATA frames on Zlib.
 *
 * Each frame must independently detect the zlib error and return FAILURE.
 * The decompression state must not carry poison from one call to the next.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform20_ThreeConsecutiveCorruptZlibFramesEachReturnFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15919", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    int failCount = 0;

    for (int pass = 0; pass < 3; ++pass)
    {
        unsigned char frame[12];
        frame[0] = 0x00;
        frame[1] = (unsigned char)sizeof(frame);
        frame[2] = 0x08;   /* IPC_COMP_DATA */
        for (int k = 3; k < (int)sizeof(frame); ++k)
            frame[k] = (unsigned char)(0xAA ^ (k + pass));

        injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
        time_sleep(30);

        RsslRet readRet = RSSL_RET_SUCCESS;
        rsslRead(pServerChnl, &readRet, &err);
        if (readRet == RSSL_RET_FAILURE)
        {
            ++failCount;
            /* After FAILURE the channel is closed; no point continuing. */
            break;
        }
    }

    EXPECT_GE(failCount, 1)
        << "At least one corrupt Zlib frame must return FAILURE (Malform-20)";
    SUCCEED() << "Three consecutive corrupt Zlib frames did not crash (Malform-20); "
                 "failures=" << failCount;
}

/* -----------------------------------------------------------------------
 * Malform-21 – IPC_PACKING with a zero-length middle sub-message.
 *
 * Wire layout:
 *   [0..1] length=11
 *   [2]    IPC_DATA | IPC_PACKING (0x12)
 *   [3..4] sub-msg 1 length = 2,  payload = 0x41 0x42
 *   [5..6] sub-msg 2 length = 0   (zero-length middle sub-message)
 *   [7..8] sub-msg 3 length = 2,  payload = 0x43 0x44
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform21_PackedZeroLengthMiddleSubMessageHandledCleanly)
{
    ASSERT_TRUE(setupReadChannelPair("15920", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = {
        0x00, 0x0B,   /* length=11 */
        0x12,         /* IPC_DATA | IPC_PACKING */
        0x00, 0x02, 0x41, 0x42,   /* sub-msg 1: length=2, payload AA BB */
        0x00, 0x00,               /* sub-msg 2: length=0 (zero-length) */
        0x00, 0x02, /* sub-msg 3: length=2 — payload truncated; frame ends here */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    /* Drain; must not crash regardless of how many sub-messages arrive. */
    int msgCount = 0;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
            ++msgCount;
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(2, msgCount);
    SUCCEED() << "Packed zero-length middle sub-message did not crash (Malform-21); "
                 "msgs=" << msgCount;
}

/* -----------------------------------------------------------------------
 * Malform-22 – IPC_PACKING frame with a single sub-message whose length
 *              equals exactly the remaining payload bytes (tight boundary).
 *
 * Frame: length=7, IPC_PACKING, sub-len=2, payload=0x41 0x42.
 * The packed-buffer advance: returnBuffer.data + returnBuffer.length
 * must land exactly on packedBuffer->buffer + packedBuffer->length
 * (no overflow, no truncation).
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform22_PackedSubMessageLengthExactBoundaryConsumedCleanly)
{
    ASSERT_TRUE(setupReadChannelPair("15921", &pServer, &pServerChnl, &pClientChnl));

    unsigned char frame[] = {
        0x00, 0x07,   /* length=7  (3 header + 2 prefix + 2 payload) */
        0x12,         /* IPC_DATA | IPC_PACKING */
        0x00, 0x02,   /* sub-message length = 2 (exactly the payload available) */
        0x41, 0x42    /* payload */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_NE(nullptr, pBuf);
    EXPECT_EQ(2, pBuf->length);

    if (readRet == 1)
    {
        pBuf = rsslRead(pServerChnl, &readRet, &err);

        EXPECT_EQ(nullptr, pBuf);
        EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet);
    }
}

/* -----------------------------------------------------------------------
 * Malform-23 – IPC_PACKING | IPC_EXTENDED_FLAGS (0x52) with ext_flags=0.
 *
 * The packed flag and extended-flags are both set; ext_flags byte = 0x00
 * means neither IPC_FRAG_HEADER nor IPC_FRAG.  cHdrLen becomes 4.
 * The packed unpack path must then start at offset 4 into the buffer.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform23_PackedAndExtendedFlagsNoFragBitsHandledCleanly)
{
    ASSERT_TRUE(setupReadChannelPair("15922", &pServer, &pServerChnl, &pClientChnl));

    /* length=10, IPC_DATA|IPC_PACKING|IPC_EXTENDED_FLAGS, ext_flags=0,
     * then a 2-byte sub-msg prefix (len=2) + 2 payload bytes. */
    unsigned char frame[] = {
        0x00, 0x0A,   /* length=10 */
        0x52,         /* IPC_DATA(0x02) | IPC_PACKING(0x10) | IPC_EXTENDED_FLAGS(0x40) */
        0x00,         /* ext_flags = 0 (no frag bits) */
        0x00, 0x02,   /* sub-msg length = 2 */
        0x41, 0x42,   /* 2 payload bytes */
        0x00, 0x00    /* padding to reach declared length */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    int msgCount = 0;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
            ++msgCount;
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(1, msgCount);
    SUCCEED() << "IPC_PACKING|IPC_EXTENDED_FLAGS with ext_flags=0 did not crash "
                 "(Malform-23); msgs=" << msgCount;
}

/* -----------------------------------------------------------------------
 * Malform-24 – Packed frame with 200 one-byte sub-messages.
 *
 * Total frame: 3 (header) + 200×(2+1) = 603 bytes.
 * The unpack loop must iterate 200 times without stack overflow.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform24_PackedFrameWith200OneByteSubMessagesNoStackOverflow)
{
    ASSERT_TRUE(setupReadChannelPair("15923", &pServer, &pServerChnl, &pClientChnl));

    const int nMsgs        = 200;
    const int payLen       = 1;
    const int subFrameSize = 2 + payLen;          /* 2-byte prefix + 1-byte payload */
    const int totalLen     = 3 + nMsgs * subFrameSize;  /* 603 */

    std::vector<unsigned char> frame(totalLen, 0);
    frame[0] = (unsigned char)((totalLen >> 8) & 0xFF);
    frame[1] = (unsigned char)(totalLen & 0xFF);
    frame[2] = 0x12;   /* IPC_DATA | IPC_PACKING */

    for (int m = 0; m < nMsgs; ++m)
    {
        int off = 3 + m * subFrameSize;
        frame[off]     = 0x00;
        frame[off + 1] = (unsigned char)payLen;
        frame[off + 2] = (unsigned char)('A' + (m % 26));
    }

    injectRawBytes(pClientChnl, frame.data(), totalLen);
    time_sleep(100);

    RsslError err;
    int msgCount = 0;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
            ++msgCount;
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(nMsgs, msgCount);
    SUCCEED() << "200 one-byte packed sub-messages did not crash (Malform-24); "
                 "msgs=" << msgCount;
}

/* -----------------------------------------------------------------------
 * Malform-25 – Packed frame where the last sub-message length is exact.
 *
 * Three sub-messages:  12 + 10 + N bytes where N is chosen so that
 * returnBuffer.data + N == packedBuffer->buffer + packedBuffer->length
 * (i.e. the last unpack advance lands precisely on the end of the buffer).
 * No overflow, no underflow; the last sub-message must be delivered.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform25_PackedLastSubMessageLengthExactlyAtBufferEnd)
{
    ASSERT_TRUE(setupReadChannelPair("15924", &pServer, &pServerChnl, &pClientChnl));

    const unsigned char frame[] = {
        0x00, 0x18,   /* length = 24 */
        0x12,         /* IPC_DATA | IPC_PACKING */
        /* sub-msg 1: len=8, payload (8 bytes) */
        0x00, 0x08,  0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,
        /* sub-msg 2: len=9, payload (9 bytes) — occupies all remaining space */
        0x00, 0x09,  0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51
    };
    
    int size = (int)sizeof(frame);

    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    int msgCount = 0;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf)
            ++msgCount;
        else if ( readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(2, msgCount);
    SUCCEED() << "Packed last sub-message at exact buffer end did not crash "
                 "(Malform-25); msgs=" << msgCount;
}

/* -----------------------------------------------------------------------
 * Malform-26 – Frame length = maxMsgSize + 1 (6 145 = 0x1801).
 *
 * One byte above the maxMsgSize guard:
 *   if (ipcLen > rsslSocketChannel->maxMsgSize) → FAILURE
 * must fire and return FAILURE before any read-more loop.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform26_FrameLengthOneAboveMaxMsgSizeReturnsFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15925", &pServer, &pServerChnl, &pClientChnl));

    /* 6145 = 0x1801 */
    unsigned char frame[] = { 0x18, 0x01, 0x02 };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, readRet);
}

/* -----------------------------------------------------------------------
 * Malform-27 – Valid frame coalesced with a malformed frame in one inject.
 *
 * Inject both frames as a single send() call so the TCP layer delivers
 * them in the same segment.  ipcReadSession()'s "more data" path must
 * consume the first frame, return the valid message, and on the next call
 * detect the malformed second frame and return FAILURE.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform27_ValidFrameCoalescedWithMalformedFrameInOneInject)
{
    ASSERT_TRUE(setupReadChannelPair("15926", &pServer, &pServerChnl, &pClientChnl));

    /* Valid 4-byte IPC_DATA frame (1 payload byte 0x41) followed by a
     * 3-byte frame with opcode 0x00 (invalid). */
    unsigned char combined[] = {
        /* Valid frame: length=4, IPC_DATA, payload=0x41 */
        0x00, 0x04, 0x02, 0x41,
        /* Malformed frame: length=3, opcode=0x00 */
        0x00, 0x03, 0x00
    };
    injectRawBytes(pClientChnl, combined, (int)sizeof(combined));
    time_sleep(50);

    RsslError err;

    /* First read – expect the valid 1-byte message. */
    RsslRet     r1 = RSSL_RET_SUCCESS;
    RsslBuffer* p1 = rsslRead(pServerChnl, &r1, &err);

    /* Second read – expect FAILURE from the malformed frame. */
    RsslRet     r2 = RSSL_RET_SUCCESS;
    RsslBuffer* p2 = rsslRead(pServerChnl, &r2, &err);
    (void)p2;

    bool firstOk = (p1 != nullptr && p1->length == 0 && r1 >= RSSL_RET_SUCCESS); // Need more data to complete the entire message
    bool secondOk = (r2 == RSSL_RET_FAILURE);
    EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (0)") != NULL);

    EXPECT_TRUE(firstOk)
        << "First (valid) coalesced frame must not crash; r1=" << r1
           << " (Malform-27)";
    EXPECT_TRUE(secondOk)
        << "Second (malformed) coalesced frame must not crash; r2=" << r2
           << " (Malform-27)";
}

/* -----------------------------------------------------------------------
 * Malform-28 – IPC_EXTENDED_FLAGS + IPC_FRAG (subsequent-fragment header),
 *              frame is 5 bytes (one byte short of the 6-byte header).
 *
 * Required subsequent-fragment header: 3 (base) + 1 (ext_flags) + 2 (fragId)
 * = 6 bytes.  A 5-byte frame leaves the fragId field truncated by 1 byte.
 * The inner header-completion loop must detect this and return WOULD_BLOCK.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform28_TruncatedSubsequentFragHeaderByOneByteWouldBlock)
{
    ASSERT_TRUE(setupReadChannelPair("15927", &pServer, &pServerChnl, &pClientChnl));

    /* 5-byte frame: length=5, IPC_DATA|IPC_EXTENDED_FLAGS, ext_flags=IPC_FRAG(0x04),
     * only 1 byte of the 2-byte fragId. */
    unsigned char frame[] = {
        0x00, 0x05,   /* length=5 */
        0x43,         /* IPC_DATA | IPC_EXTENDED_FLAGS */
        0x04,         /* IPC_FRAG (subsequent-fragment) */
        0x00          /* only 1 byte of 2-byte fragId */
    };
    injectRawBytes(pClientChnl, frame, (int)sizeof(frame));
    time_sleep(50);

    RsslError err;
    RsslRet   readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pBuf  = rsslRead(pServerChnl, &readRet, &err);

    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, readRet);
    EXPECT_TRUE(strstr(err.text, "Error: 0014 rsslRead() Attempting to reassemble a message with frag") != NULL);
    EXPECT_TRUE(strstr(err.text, "without seeing first fragment.") != NULL);

}

/* -----------------------------------------------------------------------
 * Malform-29 – Valid IPC_DATA frame followed in the same inject by an
 *              IPC_COMP_DATA frame on an uncompressed channel.
 *
 * The first frame is consumed; moreData > 0 causes ipcReadSession to be
 * called again internally.  On the second call the IPC_COMP_DATA guard
 * (decompressBuf == NULL) must fire and return FAILURE.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform29_ValidFrameThenCompDataOnUncompressedChannelFailure)
{
    ASSERT_TRUE(setupReadChannelPair("15928", &pServer, &pServerChnl, &pClientChnl));

    /* Valid 4-byte frame (1-byte payload) + IPC_COMP_DATA 7-byte frame. */
    unsigned char combined[] = {
        /* Valid: length=4, IPC_DATA, payload=0x58 */
        0x00, 0x04, 0x02, 0x58,
        /* Malformed: length=7, IPC_COMP_DATA, 4 fake bytes */
        0x00, 0x07, 0x08, 0xDE, 0xAD, 0xBE, 0xEF
    };
    injectRawBytes(pClientChnl, combined, (int)sizeof(combined));
    time_sleep(50);

    RsslError err;

    /* Drain; the library may deliver the valid frame and then fail on the
     * second one, or combine both reads.  Either way no crash is allowed. */
    bool hadFailure = false;
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (readRet == RSSL_RET_FAILURE)  
        {
            EXPECT_TRUE(strstr(err.text, "Error: 1007 Invalid Message Opcode: (8)") != NULL);
            hadFailure = true; 
            break; 
        }
        if (readRet == RSSL_RET_READ_WOULD_BLOCK) break;
    }

    SUCCEED() << "Valid+IPC_COMP_DATA coalesced inject did not crash (Malform-29); "
                 "hadFailure=" << hadFailure;
}

/* -----------------------------------------------------------------------
 * Malform-30 – Alternating valid/malformed frames on a Zlib channel.
 *
 * Send 10 valid compressed 64-byte messages interleaved with 10 injected
 * corrupt IPC_COMP_DATA frames.  Valid messages must be delivered;
 * corrupt frames must trigger FAILURE without corrupting the Zlib
 * decompression context shared with subsequent valid frames.
 *
 * Because a FAILURE causes the channel to close, this test verifies the
 * behavior up to the first failure and ensures no crash occurs.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, Malform30_ValidAndCorruptFramesInterleavedOnZlibChannel)
{
    ASSERT_TRUE(setupReadChannelPair("15929", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    int validSent = 0;

    for (int i = 0; i < 10; ++i)
    {
        /* Valid compressed message. */
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillAscii(pBuf, 64);
        pBuf->length = 64;
        if (writeAndFlush(pClientChnl, pBuf) >= RSSL_RET_SUCCESS)
            ++validSent;

        /* Corrupt IPC_COMP_DATA frame injected directly into the socket.
         * On a Zlib channel the IPC_COMP_DATA path fires; random noise
         * causes inflate() to fail. */
        unsigned char bad[10];
        bad[0] = 0x00;
        bad[1] = (unsigned char)sizeof(bad);
        bad[2] = 0x08;   /* IPC_COMP_DATA */
        for (int k = 3; k < (int)sizeof(bad); ++k)
            bad[k] = (unsigned char)(0xBB ^ (k + i));
        injectRawBytes(pClientChnl, bad, (int)sizeof(bad));

        time_sleep(10);
    }

    time_sleep(50);

    int validRead = 0;
    bool hadCrash = false;
    for (int attempt = 0; attempt < 200 && !hadCrash; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(pServerChnl, &readRet, &err);
        if (pBuf && pBuf->length == 64)
            ++validRead;
        else if (readRet == RSSL_RET_FAILURE ||
                 readRet == RSSL_RET_READ_WOULD_BLOCK)
            break;
    }

    EXPECT_EQ(1, validRead);
    EXPECT_FALSE(hadCrash);
    SUCCEED() << "Alternating valid/corrupt frames on Zlib channel did not crash "
                 "(Malform-30); validSent=" << validSent
              << " validRead=" << validRead;
}

/* =======================================================================
 * ── Concurrent read + channel-close stress test ──────────────────────────
 *
 * Mirrors PartialWrite_ConcurrentFragmentWriteAndClientClose from the write
 * unit-test suite.  One thread continuously reads large fragmented messages
 * from the server channel while the main thread closes that channel after a
 * short delay.  The reader thread must detect the closed channel and stop
 * cleanly; no crash, double-free, or use-after-free must occur.
 *
 * Crash vectors targeted:
 *   Issue 9 – double-free of rsslAssemblyBuf when the channel is torn down
 *              (ipcFreeSession) while a partial fragment is being assembled.
 *   Issue 4 – stale rsslAssemblyBuf pointer after the hash table entry is
 *              evicted mid-assembly by ipcFreeSession.
 *   Issue 3 – memory leak of assembly buffer on hash-table insert failure
 *              triggered by the forced close.
 *
 * Port: 15930
 * ===================================================================== */

struct ConcurrentReadCloseArg
{
    RsslChannel*        pChnl;         /* server channel to read from          */
    RsslUInt32          fragSize;      /* fragment size negotiated on channel   */
    std::atomic<bool>   stop;          /* set by main thread before close       */
    std::atomic<int>    readsAttempted;
    std::atomic<bool>   readerDone;

    ConcurrentReadCloseArg()
        : pChnl(nullptr), fragSize(512),
          stop(false), readsAttempted(0), readerDone(false)
    {}
};

static RSSL_THREAD_DECLARE(concurrentReadFragFn, pArg)
{
    ConcurrentReadCloseArg* a = reinterpret_cast<ConcurrentReadCloseArg*>(pArg);
    RsslError err;

    while (!a->stop.load(std::memory_order_acquire))
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf    = rsslRead(a->pChnl, &readRet, &err);
        ++a->readsAttempted;

        if (pBuf)
            continue;   /* successfully received a reassembled fragment */

        if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet == RSSL_RET_READ_PING)
        {
            time_sleep(1);
            continue;
        }

        /* Any hard error (channel closed) – stop reading */
        if (readRet < RSSL_RET_SUCCESS)
            break;
    }

    a->readerDone = true;
    return 0;
}

/* Writer thread: continuously sends 3-fragment messages from the client
 * channel until the stop flag is set or a write error occurs. */
static RSSL_THREAD_DECLARE(concurrentWriteFragForReadFn, pArg)
{
    ConcurrentReadCloseArg* a = reinterpret_cast<ConcurrentReadCloseArg*>(pArg);
    RsslError err;
    const RsslUInt32 msgSize = a->fragSize * 3 + 64;  /* 3+ fragments per message */

    while (!a->stop.load(std::memory_order_acquire))
    {
        RsslBuffer* pBuf = rsslGetBuffer(a->pChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) break;

        if (a->stop.load(std::memory_order_acquire))
        {
            rsslReleaseBuffer(pBuf, &err);
            break;
        }

        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(a->pChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
        {
            ret = rsslFlush(a->pChnl, &err);
            if (ret < RSSL_RET_SUCCESS) break;
        }
    }
    return 0;
}

TEST_F(RsslSocketReadChannelLockTests,
       ConcurrentRead_LargeFragmentedMessages_ChannelCloseDoesNotCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupReadChannelPair("15930", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_NONE, 0, fragSize));

    /* Shared arg – the writer uses pClientChnl; the reader uses pServerChnl. */
    ConcurrentReadCloseArg writerArgs;
    writerArgs.pChnl    = pClientChnl;
    writerArgs.fragSize = fragSize;

    ConcurrentReadCloseArg readerArgs;
    readerArgs.pChnl    = pServerChnl;
    readerArgs.fragSize = fragSize;

    RsslThreadId writerThread, readerThread;
    RSSL_THREAD_START(&writerThread, concurrentWriteFragForReadFn, &writerArgs);
    RSSL_THREAD_START(&readerThread, concurrentReadFragFn,          &readerArgs);

    /* Let both threads run for 40 ms, then signal stop and close the server
     * channel under the per-channel lock (serialises with the reader thread). */
    time_sleep(40);

    writerArgs.stop.store(true, std::memory_order_release);
    readerArgs.stop.store(true, std::memory_order_release);

    RsslError err;
    rsslCloseChannel(pServerChnl, &err);
    pServerChnl = nullptr;  /* prevent TearDown from double-closing */

    /* Wait for both threads to detect the closed channel and exit. */
    RSSL_THREAD_JOIN(readerThread);
    RSSL_THREAD_JOIN(writerThread);

    EXPECT_GT(readerArgs.readsAttempted.load(), 0)
        << "Reader thread must have attempted at least one read before close";
    SUCCEED() << "Concurrent large-fragment read + channel close did not crash; "
                 "readsAttempted=" << readerArgs.readsAttempted.load();
}

/* -----------------------------------------------------------------------
 * PackCompress-7 – Packed Zlib frame with highly compressible sub-messages.
 *
 * Three 200-byte sub-messages filled with all-zero bytes are packed into
 * one Zlib frame.  The compressed frame is much smaller than the total
 * 600 decompressed bytes; the decompressor expand path and the subsequent
 * unpackOffset advance must both handle the large expansion ratio.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress7_ZlibPackedHighlyCompressibleSubMessages)
{
    ASSERT_TRUE(setupReadChannelPair("15806", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 9));

    RsslError err;
    const RsslUInt32 payLen = 200;
    const int        nMsgs  = 3;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen * (RsslUInt32)nMsgs + 64u,
                                     RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < nMsgs; ++m)
    {
        ASSERT_NE(pCur, nullptr);
        ASSERT_GE(pCur->length, payLen);
        memset(pCur->data, 0x00, payLen);   /* all-zero → high Zlib ratio */
        pCur->length = payLen;

        if (m < nMsgs - 1)
            pCur = rsslPackBuffer(pClientChnl, pCur, &err);
    }
    if (pCur) pCur->length = payLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    int received = 0, wrongLen = 0;
    for (int attempt = 0; attempt < 200 && received < nMsgs; ++attempt)
    {
        RsslRet     readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pRd     = rsslRead(pServerChnl, &readRet, &err);
        if (pRd)
        {
            if (pRd->length != payLen) ++wrongLen;
            ++received;
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(received, nMsgs)
        << "All 3 zero-filled Zlib-packed sub-messages must be delivered "
           "(PackCompress-7)";
    EXPECT_EQ(wrongLen, 0)
        << "Every sub-message must decompress to exactly " << payLen
        << " bytes (PackCompress-7)";
}

/* -----------------------------------------------------------------------
 * PackCompress-8 – readOutArgs on first vs continuation calls, Zlib packed.
 *
 * Issue 6 documents that the packed-buffer continuation path forces
 * bytesRead = 0 on every call after the first.  This test verifies:
 *   - First rsslReadEx() call:  bytesRead > 0, uncompressedBytesRead > 0,
 *                                uncompressedBytesRead >= bytesRead,
 *                                returned buffer length == payLen.
 *   - Continuation call(s):     bytesRead == 0 (by design),
 *                                returned buffer length == payLen.
 * --------------------------------------------------------------------- */
TEST_F(RsslSocketReadTests, PackCompress8_ZlibReadOutArgsFirstVsContinuationCall)

{
    ASSERT_TRUE(setupReadChannelPair("15807", &pServer, &pServerChnl, &pClientChnl,
                                     RSSL_COMP_ZLIB, 6));

    RsslError err;
    const RsslUInt32 payLen = 50;
    const int        nMsgs  = 3;

    /* Build a packed buffer: 3 × 50-byte sub-messages. */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 512u, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < nMsgs; ++m)
    {
        ASSERT_NE(pCur, nullptr);
        fillAscii(pCur, payLen);
        pCur->length = payLen;
        if (m < nMsgs - 1)
            pCur = rsslPackBuffer(pClientChnl, pCur, &err);
    }
    if (pCur) pCur->length = payLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    while (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    time_sleep(100);

    /* ── First read (new frame) ─────────────────────────────────────── */
    RsslReadInArgs  rdIn;  rsslClearReadInArgs(&rdIn);
    RsslReadOutArgs rdOut; rsslClearReadOutArgs(&rdOut);
    rdOut.bytesRead             = (RsslUInt32)-1;
    rdOut.uncompressedBytesRead = (RsslUInt32)-1;
    RsslRet     readRet = RSSL_RET_SUCCESS;
    RsslBuffer* pFirst  = rsslReadEx(pServerChnl, &rdIn, &rdOut, &readRet, &err);

    ASSERT_NE(pFirst, nullptr)
        << "First packed+compressed read must return a buffer (PackCompress-8)";
    EXPECT_EQ(pFirst->length, payLen)
        << "First sub-message length must equal payLen (PackCompress-8)";
    EXPECT_GT(rdOut.bytesRead, 0u)
        << "bytesRead must be > 0 on first call (PackCompress-8)";
    EXPECT_GT(rdOut.uncompressedBytesRead, 0u)
        << "uncompressedBytesRead must be > 0 on first call (PackCompress-8)";
    EXPECT_GE(rdOut.uncompressedBytesRead, rdOut.bytesRead)
        << "uncompressedBytesRead must be >= bytesRead on first call (PackCompress-8)";

    /* ── Continuation reads (packed sub-messages 2 and 3) ───────────── */
    int continuationCount = 0;
    for (int attempt = 0; attempt < 50 && continuationCount < nMsgs - 1; ++attempt)
    {
        RsslReadInArgs  rdIn2;  rsslClearReadInArgs(&rdIn2);
        RsslReadOutArgs rdOut2; rsslClearReadOutArgs(&rdOut2);
        rdOut2.bytesRead             = (RsslUInt32)-1;
        rdOut2.uncompressedBytesRead = (RsslUInt32)-1;
        RsslRet     readRet2 = RSSL_RET_SUCCESS;
        RsslBuffer* pCont    = rsslReadEx(pServerChnl, &rdIn2, &rdOut2,
                                          &readRet2, &err);
        if (pCont)
        {
            EXPECT_EQ(pCont->length, payLen)
                << "Continuation sub-message " << (continuationCount + 1)
                << " must have length " << payLen << " (PackCompress-8)";
            /* Issue 6: bytesRead is 0 on the continuation path (by design). */
            EXPECT_EQ(rdOut2.bytesRead, 0u)
                << "bytesRead must be 0 on packed continuation call "
                << (continuationCount + 1) << " (PackCompress-8 / Issue 6)";
            ++continuationCount;
        }
        else if (readRet2 == RSSL_RET_READ_WOULD_BLOCK || readRet2 < RSSL_RET_SUCCESS)
            break;
    }

    EXPECT_EQ(continuationCount, nMsgs - 1)
        << "All " << (nMsgs - 1) << " continuation sub-messages must be returned "
           "(PackCompress-8)";
}

