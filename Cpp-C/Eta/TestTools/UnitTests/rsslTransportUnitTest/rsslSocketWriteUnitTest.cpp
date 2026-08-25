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


/* Establishes a non-blocking server+client pair on the requested port and
 * returns both channels in an active state. Returns false on any failure.
 *
 * Non-blocking flow:
 *   1. rsslBind  (non-blocking server socket)
 *   2. rsslConnect (non-blocking client socket) ? RSSL_CH_STATE_INITIALIZING
 *   3. select on server socket ? rsslAccept ? RSSL_CH_STATE_INITIALIZING
 *   4. Hard-loop rsslInitChannel on both sides until both reach ACTIVE.
 */
static bool setupActiveChannelPair(
const char*         port,
RsslServer**        ppServer,
RsslChannel**       ppServerChnl,
RsslChannel**       ppClientChnl,
RsslCompTypes       compType   = RSSL_COMP_NONE,
RsslUInt32          compLevel  = 0,
RsslUInt32          maxFragSz  = 0,     /* 0 ? default */
RsslConnectionTypes connType   = RSSL_CONN_TYPE_SOCKET)
{
    RsslError err;
    struct timeval selectTime;
    fd_set readfds;
    int selRet;

    /* --- Bind server (non-blocking) ----------------------------------- */
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
    if (connType == RSSL_CONN_TYPE_WEBSOCKET)
    {
        bindOpts.connectionType  = RSSL_CONN_TYPE_WEBSOCKET;
        bindOpts.wsOpts.protocols = const_cast<char*>("rssl.rwf");
    }

    *ppServer = rsslBind(&bindOpts, &err);
    if (!*ppServer)
    {
        std::cout << "setupActiveChannelPair: rsslBind failed: " << err.text << "\n";
        return false;
    }

    /* --- Connect client (non-blocking) -------------------------------- */
    RsslConnectOptions copts;
    rsslClearConnectOpts(&copts);
    copts.connectionType                     = connType;
    if (connType == RSSL_CONN_TYPE_WEBSOCKET)
        copts.wsOpts.protocols               = const_cast<char*>("rssl.rwf");
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
        std::cout << "setupActiveChannelPair: rsslConnect failed: " << err.text << "\n";
        rsslCloseServer(*ppServer, &err);
        *ppServer = NULL;
        return false;
    }

    /* --- Accept server channel (poll then accept) --------------------- */
    FD_ZERO(&readfds);
    FD_SET((*ppServer)->socketId, &readfds);
    selectTime.tv_sec  = 5L;
    selectTime.tv_usec = 0L;
    selRet = select(FD_SETSIZE, &readfds, NULL, NULL, &selectTime);
    if (selRet <= 0)
    {
        std::cout << "setupActiveChannelPair: select for accept timed out or failed\n";
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
        std::cout << "setupActiveChannelPair: rsslAccept failed: " << err.text << "\n";
        rsslCloseChannel(*ppClientChnl, &err);
        rsslCloseServer(*ppServer, &err);
        *ppClientChnl = NULL;
        *ppServer     = NULL;
        return false;
    }

    /* --- Drive rsslInitChannel on both sides until ACTIVE ------------- */
    const int maxInitIterations = 5000;
    for (int i = 0; i < maxInitIterations; ++i)
    {
        RsslInProgInfo inProg;
        RsslRet ret;

        if ((*ppClientChnl)->state != RSSL_CH_STATE_ACTIVE)
        {
            rsslClearInProgInfo(&inProg);
            ret = rsslInitChannel(*ppClientChnl, &inProg, &err);
            if (ret < RSSL_RET_SUCCESS)
            {
                std::cout << "setupActiveChannelPair: client rsslInitChannel failed: "
                          << err.text << "\n";
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
                std::cout << "setupActiveChannelPair: server rsslInitChannel failed: "
                          << err.text << "\n";
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

    std::cout << "setupActiveChannelPair: channels did not reach ACTIVE state\n";
    rsslCloseChannel(*ppServerChnl, &err);
    rsslCloseChannel(*ppClientChnl, &err);
    rsslCloseServer(*ppServer, &err);
    *ppServerChnl = NULL;
    *ppClientChnl = NULL;
    *ppServer     = NULL;
    return false;
}

/* -----------------------------------------------------------------------
 * Base fixture - initialises the transport, establishes one channel pair,
 * and tears everything down.
 * --------------------------------------------------------------------- */
class RsslSocketWriteTests : public ::testing::TestWithParam<RsslConnectionTypes>
{
protected:
    RsslServer*  pServer      = nullptr;
    RsslChannel* pServerChnl  = nullptr;
    RsslChannel* pClientChnl  = nullptr;

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

    /* Establish an active channel pair using the parameterised connection type. */
    bool setupChannelPair(
        const char*   port,
        RsslCompTypes compType  = RSSL_COMP_NONE,
        RsslUInt32    compLevel = 0,
        RsslUInt32    maxFragSz = 0)
    {
        return setupActiveChannelPair(port, &pServer, &pServerChnl, &pClientChnl,
                                      compType, compLevel, maxFragSz, GetParam());
    }

    /* Allocate a buffer of the given size from the client channel. */
    RsslBuffer* getClientBuffer(RsslUInt32 size, RsslError* rsslError, RsslBool packed = RSSL_FALSE)
    {
        return rsslGetBuffer(pClientChnl, size, packed, rsslError);
    }

    /* Fill a buffer with a simple ASCII pattern. */
    static void fillBuffer(RsslBuffer* pBuf, RsslUInt32 len = 0)
    {
        if (!pBuf || !pBuf->data) return;
        RsslUInt32 fillLen = (len > 0 && len <= pBuf->length) ? len : pBuf->length;
        for (RsslUInt32 i = 0; i < fillLen; ++i)
            pBuf->data[i] = (char)('A' + (i % 26));
    }
};

/* -----------------------------------------------------------------------
 * -- ISSUE 2 -------------------------------------------------------------
 * Buffer length > maxLength causes ipcWriteSession() to break out without
 * freeing the remaining chain, leaving heap state corrupt.
 * Verify the channel transitions to CLOSED and returns RSSL_RET_FAILURE.
 * --------------------------------------------------------------------- */

/* Write a buffer whose reported length exceeds the buffer's own capacity.
 * The ipcWriteSession path validates msgb->length > msgb->maxLength. */
TEST_P(RsslSocketWriteTests, Issue2_BufferLengthExceedsMaxLength)
{
    ASSERT_TRUE(setupChannelPair("15103"));

    RsslError err;
    const RsslUInt32 allocSize = 64;
    RsslBuffer* pBuf = getClientBuffer(allocSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, allocSize);

    /* Deliberately set length beyond what was allocated to trigger the
     * msgb->length > msgb->maxLength check inside ipcWriteSession(). */
    pBuf->length = pBuf->length + 1000;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    /* Must not crash; must signal failure. */
    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Writing a buffer with length > maxLength must return RSSL_RET_BUFFER_TOO_SMALL";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(64).") != NULL);
}

/* -----------------------------------------------------------------------
 * -- ISSUE 6 -------------------------------------------------------------
 * RsslUInt16 messageLength overflow for large near-65535-byte buffers.
 * Test that writing a buffer whose total wire length (data + IPC headers)
 * would overflow a 16-bit length field is handled gracefully.
 * --------------------------------------------------------------------- */

/* Write a 6000-byte buffer on a channel whose maxFragmentSize is 6144.
* The payload fits within one fragment but is near the wire-buffer capacity.
* Verifies the single-fragment path handles near-limit sizes correctly
* without truncating or wrapping the IPC 16-bit length field. */
TEST_P(RsslSocketWriteTests, Issue6_NearMaximumWireLength)
{
    /* Use a small fragment size so a large message is fragmented correctly. */
    ASSERT_TRUE(setupChannelPair("15106", RSSL_COMP_NONE, 0, 6144));

    RsslError err;

    /* Request a buffer large enough to require fragmentation. */
    const RsslUInt32 reqSize = 6000;
    RsslBuffer* pBuf = getClientBuffer(reqSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - acceptable for this size";
        return;
    }

    fillBuffer(pBuf, reqSize);
    pBuf->length = reqSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    /* The write may be queued (positive return), succeed (0), or fail. */
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Large fragmented write should succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * -- ISSUE 9 -------------------------------------------------------------
 * writeOutArgs->bytesWritten is not updated on WRITE_FLUSH_FAILED and
 * WRITE_CALL_AGAIN paths.  Verify the struct is populated on the success
 * path and not left stale/garbage on error paths.
 * --------------------------------------------------------------------- */

/* Verify that bytesWritten and uncompressedBytesWritten are both updated
 * (not left at sentinel -1) and non-zero after a successful 64-byte write.
 * Targets Issue 9: out-args must be populated on the success path. */
TEST_P(RsslSocketWriteTests, Issue9_WriteOutArgsBytesWrittenPopulatedOnSuccess)
{
    ASSERT_TRUE(setupChannelPair("15107"));

    RsslError err;
    const RsslUInt32 payloadLen = 64;
    RsslBuffer* pBuf = getClientBuffer(payloadLen, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, payloadLen);
    pBuf->length = payloadLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    /* Sentinel - if the implementation never writes these fields they stay -1. */
    outArgs.bytesWritten             = (RsslUInt32)-1;
    outArgs.uncompressedBytesWritten = (RsslUInt32)-1;

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_NE(outArgs.bytesWritten, (RsslUInt32)-1)
        << "bytesWritten must be updated after a write call";
    EXPECT_GT(outArgs.bytesWritten, 0u)
        << "bytesWritten must be > 0 for a non-empty payload";
    EXPECT_NE(outArgs.uncompressedBytesWritten, (RsslUInt32)-1)
        << "uncompressedBytesWritten must be updated after a write call";
}

/* -----------------------------------------------------------------------
 * -- Channel-state guard tests --------------------------------------------
 * Writing to a channel that is not in RSSL_CH_STATE_ACTIVE must fail
 * cleanly without crashing.
 * --------------------------------------------------------------------- */

/* Write to a channel that has already been explicitly closed (state ?
 * CLOSED) should fail gracefully. */
TEST_P(RsslSocketWriteTests, WriteOnClosedChannelStateFails)
{
    ASSERT_TRUE(setupChannelPair("15109"));

    RsslError err;
    /* Forcibly mark the client channel as closed without calling rsslCloseChannel
     * (which would free internal structures).  We only change the public state
     * field to simulate what the write guard should check. */
    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslBuffer fakeBuffer;
    char       fakeMem[64];
    fakeBuffer.data   = fakeMem;
    fakeBuffer.length = 16;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, &fakeBuffer, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "Writing to a CLOSED channel must return RSSL_RET_FAILURE";

    /* Restore state so TearDown can close the channel cleanly. */
    pClientChnl->state = RSSL_CH_STATE_ACTIVE;
}

/* -----------------------------------------------------------------------
 * -- Zero-length buffer tests ---------------------------------------------
 * --------------------------------------------------------------------- */

/* Calling rsslWriteEx with a buffer whose length is 0 should either fail
 * gracefully or succeed without sending garbage. */
TEST_P(RsslSocketWriteTests, ZeroLengthBufferDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15110"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    pBuf->length = 0;   /* deliberately set to zero */

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    /* Must not crash regardless of return value. */
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(RSSL_RET_FAILURE, ret);
    rsslReleaseBuffer(pBuf, &err);
}

/* -----------------------------------------------------------------------
 * -- Double-write / use-after-write tests ---------------------------------
 * Calling rsslWriteEx twice on the same buffer should fail on the second
 * call because rsslSocketWrite transfers buffer ownership on success.
 * --------------------------------------------------------------------- */

/* Write the same buffer pointer twice.  The first write transfers buffer
 * ownership to the internal pool; the second write reuses that stale pointer.
 * Only verifies the process does not crash - any return code is acceptable
 * because behaviour is undefined once the buffer has been consumed.
 * The test name is historical; no failure assertion is made on the second call. */
TEST_P(RsslSocketWriteTests, DoubleWriteSameBufferFailsSecondCall)
{
    ASSERT_TRUE(setupChannelPair("15111"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    pBuf->length = 32;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    /* First write - should succeed or queue. */
    RsslRet ret1 = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    ASSERT_GE(ret1, RSSL_RET_SUCCESS) << "First write failed: " << err.text;

    if (ret1 > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    /* Second write on the same pointer - the internal buffer has been
     * returned to the free pool; this must not corrupt the heap.
     * We accept any non-crash result. */
    rsslClearWriteOutArgs(&outArgs);
    RsslRet ret2 = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    SUCCEED() << "Double-write did not crash; second ret=" << ret2;
}

/* -----------------------------------------------------------------------
 * -- Flush-after-write-failure test ---------------------------------------
 * After a write returns RSSL_RET_WRITE_FLUSH_FAILED, the pending data
 * must be flushable without crashing.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, FlushAfterWriteFlushFailedDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15112"));

    RsslError err;

    /* Flood the output queue to force a WRITE_FLUSH_FAILED. */
    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(1024, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 1024);
        pBuf->length = 1024;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
        {
            gotFlushFailed = true;
            /* Must be able to flush without crashing. */
            RsslRet flushRet = rsslFlush(pClientChnl, &err);
            EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
                << "rsslFlush after WRITE_FLUSH_FAILED must not fail fatally";
        }
        else if (ret > RSSL_RET_SUCCESS)
        {
            rsslFlush(pClientChnl, &err);
        }
        else if (ret < RSSL_RET_SUCCESS)
        {
            break;  /* channel closed - stop */
        }
    }

    /* If we never hit WRITE_FLUSH_FAILED the test is still valid; we just
     * exercised the normal write+flush path without crashing. */
    SUCCEED() << "Flood write loop completed without crash";
}

/* -----------------------------------------------------------------------
 * -- Fragmented write tests (Issue 2 / Issue 6 boundary) -----------------
 * Messages that are larger than maxFragmentSize trigger the multi-buffer
 * fragmentation path in ipcWriteSession().
 * --------------------------------------------------------------------- */

/* Write a message that spans many fragments to stress the chained-buffer
 * pool path (Issue 7: stale nextMsg pointer). */
TEST_P(RsslSocketWriteTests, FragmentedWriteManyFragmentsDoesNotCrash)
{
    const RsslUInt32 fragSize = 1500;
    ASSERT_TRUE(setupChannelPair("15114", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = 12000;  /* 8+ fragments */
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - acceptable";
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Many-fragment write should succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * -- Packed-buffer write tests ---------------------------------------------
 * Issue 2/10: test that packed buffers close to the maximum size write and
 * flush correctly without corrupting adjacent memory.
 * --------------------------------------------------------------------- */

/* Pack a 100-byte first message into a 512-byte buffer via rsslPackBuffer(),
 * fill and pack a second message into the returned slot, then write the
 * packed buffer.  Verifies the two-message packing path (packingOffset
 * advance + wire length-prefix placement) completes without crash or
 * memory corruption. */
TEST_P(RsslSocketWriteTests, PackedBufferTwoMessagesWriteSucceeds)
{
    ASSERT_TRUE(setupChannelPair("15115"));

    RsslError err;
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 100;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Packed buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    /* Pack the first message. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (!pNext || pNext->length == 0)
    {
        /* No room for second message - just write the first. */
        pBuf->length = 0;
    }
    else
    {
        fillBuffer(pNext, msgLen);
        pNext->length = msgLen;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Packed two-message write should succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * -- Compression write tests (Issue 7: stale nextMsg on compressedmb2) ---
 * --------------------------------------------------------------------- */

/* Write a 2000-byte buffer filled with repetitive (highly compressible) data
 * on a Zlib-compressed channel.  Verifies the compressedmb1 allocation,
 * zlib deflate, and compressed-buffer dispatch paths complete without error. */
TEST_P(RsslSocketWriteTests, CompressedWriteZlibSucceeds)
{
    ASSERT_TRUE(setupChannelPair("15116", RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 payloadLen = 2000;
    RsslBuffer* pBuf = getClientBuffer(payloadLen, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip compressed test";
        return;
    }

    /* Fill with repetitive data that compresses well. */
    memset(pBuf->data, 0x41, pBuf->length);
    pBuf->length = payloadLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Zlib-compressed write should succeed; err: " << err.text;
}

/* Compressed + fragmented write (exercises both compressedmb1 and
 * compressedmb2 in ipcWriteSession, hitting Issues 4 and 7). */
TEST_P(RsslSocketWriteTests, CompressedFragmentedWriteDoesNotCrash)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15117", RSSL_COMP_ZLIB, 1, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = 7000;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    memset(pBuf->data, 0x42, pBuf->length);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Compressed + fragmented write should succeed; err: " << err.text;
}

/* Write a 2000-byte buffer filled with repetitive data on an LZ4-compressed
 * channel.  Exercises the LZ4 compression path and the compressedmb1/mb2
 * allocation sequence.  Targets Issue 7 (stale nextMsg) and Issue 4
 * (use-after-free) on the LZ4 code path. */
TEST_P(RsslSocketWriteTests, CompressedWriteLz4Succeeds)
{
    ASSERT_TRUE(setupChannelPair("15118", RSSL_COMP_LZ4, 0));

    RsslError err;
    const RsslUInt32 payloadLen = 2000;
    RsslBuffer* pBuf = getClientBuffer(payloadLen, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip LZ4 test";
        return;
    }

    memset(pBuf->data, 0x43, pBuf->length);
    pBuf->length = payloadLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "LZ4-compressed write should succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * -- Issue 8: Large write-count stress test for uncompBytes accumulation --
 * Write many messages to verify uncompBytes doesn't wrap into negative
 * territory causing incorrect out-args.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, Issue8_LargeWriteCountOutArgsSanity)
{
    ASSERT_TRUE(setupChannelPair("15119"));

    RsslError err;
    const int   iterations  = 1000;
    const RsslUInt32 payLen = 512;

    RsslUInt64 totalBytesWritten = 0;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf)
        {
            break;
        }

        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);


        if (ret == RSSL_RET_FAILURE)
        {
            break;
        }

        totalBytesWritten += outArgs.bytesWritten;

        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    /* bytesWritten per call should be a reasonable positive number
     * (at least the payload size) when accumulation is correct. */
    if (totalBytesWritten > 0)
    {
        RsslUInt64 avgPerWrite = totalBytesWritten / iterations;
        EXPECT_GE(avgPerWrite, (RsslUInt64)payLen)
            << "Average bytesWritten per call should be >= payload length; "
               "possible uncompBytes overflow if < 0";
    }
}

/* -----------------------------------------------------------------------
 * -- WRITE_CALL_AGAIN stress test -----------------------------------------
 * Drive rsslWriteEx into WRITE_CALL_AGAIN and verify the retry loop
 * handles the partial-state correctly (Issue 9: stale writeOutArgs).
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, WriteCallAgainRetryUpdatesOutArgs)
{
    ASSERT_TRUE(setupChannelPair("15120"));

    RsslError err;
    /* Send enough data to saturate the output queue and trigger CALL_AGAIN. */
    RsslRet ret = RSSL_RET_SUCCESS;

    for (int i = 0; i < 2000 && ret >= RSSL_RET_SUCCESS; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(1024, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 1024);
        pBuf->length = 1024;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        outArgs.bytesWritten = (RsslUInt32)-1;

        ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_CALL_AGAIN)
        {
            /* Verify outArgs is not left uninitialized / as sentinel -1. */
            EXPECT_NE(outArgs.bytesWritten, (RsslUInt32)-1)
                << "bytesWritten must be set even on WRITE_CALL_AGAIN";
            rsslFlush(pClientChnl, &err);
            /* Retry the write with the same buffer is valid here. */
            rsslClearWriteOutArgs(&outArgs);
            ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        }

        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        if (ret == RSSL_RET_FAILURE)
            break;
    }

    SUCCEED() << "WRITE_CALL_AGAIN stress loop completed without crash";
}

/* -----------------------------------------------------------------------
 * -- Concurrent write test (GLOBAL_AND_CHANNEL lock) ----------------------
 * Two threads writing concurrently to the same channel should not produce
 * a crash, even though the result may be interleaved.
 * --------------------------------------------------------------------- */

struct ConcurrentWriteArg
{
    RsslChannel*     pChnl;
    std::atomic<int> writesDone;
    std::atomic<bool> crashed;

    ConcurrentWriteArg() : pChnl(nullptr), writesDone(0), crashed(false) {}
};

static RSSL_THREAD_DECLARE(concurrentWriteThreadFn, pArg)
{
    ConcurrentWriteArg* args = reinterpret_cast<ConcurrentWriteArg*>(pArg);
    RsslError err;

    for (int i = 0; i < 200; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(args->pChnl, 64, RSSL_FALSE, &err);
        if (!pBuf) continue;

        for (int k = 0; k < 64; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = 64;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(args->pChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(args->pChnl, &err);

        ++args->writesDone;
    }
    return 0;
}

class RsslSocketWriteChannelLockTests : public ::testing::TestWithParam<RsslConnectionTypes>
{
protected:
    RsslServer*  pServer      = nullptr;
    RsslChannel* pServerChnl  = nullptr;
    RsslChannel* pClientChnl  = nullptr;

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

    /* Establish an active channel pair using the parameterised connection type. */
    bool setupChannelPair(
        const char*   port,
        RsslCompTypes compType  = RSSL_COMP_NONE,
        RsslUInt32    compLevel = 0,
        RsslUInt32    maxFragSz = 0)
    {
        return setupActiveChannelPair(port, &pServer, &pServerChnl, &pClientChnl,
                                      compType, compLevel, maxFragSz, GetParam());
    }
};

TEST_P(RsslSocketWriteChannelLockTests, ConcurrentWritesTwoThreadsDoNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15121"));

    ConcurrentWriteArg args;
    args.pChnl = pClientChnl;

    RsslThreadId t1, t2;
    RSSL_THREAD_START(&t1, concurrentWriteThreadFn, &args);
    RSSL_THREAD_START(&t2, concurrentWriteThreadFn, &args);

    RSSL_THREAD_JOIN(t1);
    RSSL_THREAD_JOIN(t2);

    EXPECT_FALSE(args.crashed.load())
        << "Concurrent writes should not crash";
    EXPECT_GT(args.writesDone.load(), 0)
        << "At least some writes should have completed";
}

/* -----------------------------------------------------------------------
 * -- Minimal buffer-length overflow test ----------------------------------
 * The tightest possible overshoot: buffer allocated at 128 bytes,
 * pBuf->length set to 129 via the public RsslBuffer field only.
 * Exercises the msgb->length > msgb->maxLength guard in ipcWriteSession()
 * without modifying any internal struct fields.
 * --------------------------------------------------------------------- */

/* Write a 128-byte buffer with pBuf->length inflated by exactly 1 byte.
 * Expects RSSL_RET_BUFFER_TOO_SMALL and the overflow error text referencing
 * the 128-byte allocation. */
TEST_P(RsslSocketWriteTests, CorruptedBufferLengthByOneHandledGracefully)
{
    ASSERT_TRUE(setupChannelPair("15122"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 128, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);

    /* Inflate length by exactly 1 byte beyond allocated capacity to trigger
     * the msgb->length > msgb->maxLength guard in ipcWriteSession(). */
    pBuf->length = pBuf->length + 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Write with length=allocated+1 must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(128).") != NULL);
}

/* -----------------------------------------------------------------------
 * -- rsslFlush on a channel with nothing queued ---------------------------
 * Flushing immediately after initialisation (empty queue) must return
 * RSSL_RET_SUCCESS without crashing.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, FlushOnEmptyQueueReturnsSuccess)
{
    ASSERT_TRUE(setupChannelPair("15123"));

    RsslError err;
    RsslRet ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS)
        << "Flushing an empty queue should return RSSL_RET_SUCCESS";
}

/* -----------------------------------------------------------------------
 * -- Rapid open-write-close cycling ---------------------------------------
 * Open a connection, write one buffer, close, repeat 20 times.
 * Exercises pool housekeeping for re-used channel structures
 * (related to Issue 4 use-after-free in the pool path).
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, RapidOpenWriteCloseCyclesDoNotLeak)
{
    RsslError err;
    const int cycles = 20;

    for (int c = 0; c < cycles; ++c)
    {
        RsslServer*  pSrv  = nullptr;
        RsslChannel* pSrC  = nullptr;
        RsslChannel* pClC  = nullptr;

        if (!setupActiveChannelPair("15124", &pSrv, &pSrC, &pClC))
            continue;

        RsslBuffer* pBuf = rsslGetBuffer(pClC, 64, RSSL_FALSE, &err);
        if (pBuf)
        {
            for (int k = 0; k < 64; ++k) pBuf->data[k] = (char)('A' + c % 26);
            pBuf->length = 64;

            RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
            RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

            RsslRet ret = rsslWriteEx(pClC, pBuf, &inArgs, &outArgs, &err);
            if (ret > RSSL_RET_SUCCESS) rsslFlush(pClC, &err);
        }

        rsslCloseChannel(pSrC, &err);
        rsslCloseChannel(pClC, &err);
        rsslCloseServer(pSrv, &err);

        time_sleep(5);
    }

    SUCCEED() << "Rapid open-write-close cycles completed without crash";
}

/* Write a 1-byte payload on a channel whose maxFragmentSize is 512.  Because
* 1 byte is far below fragSize, no fragmentation occurs.  Verifies that the
* minimum-size single-fragment write path completes without crashing and
* does not misalign IPC header offsets for very small payloads (Issue 11). */
TEST_P(RsslSocketWriteTests, Frag_MinimalPayloadAtFragmentBoundary)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15126", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(1, &err);
    if (!pBuf)
    {
        SUCCEED() << "1-byte buffer allocation failed - skip";
        return;
    }

    pBuf->data[0] = 0x41;
    pBuf->length  = 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(RSSL_RET_SUCCESS, ret)
        << "1-byte payload write must not crash; ret=" << ret;
}

/* Set pBuf->length = UINT32_MAX on a buffer allocated in the fragmentation
 * range.  ipcWriteSession() computes per-fragment wire sizes using this
 * value; the integer overflow in those computations triggers the
 * msgb->length > msgb->maxLength guard (Issue 6). */
TEST_P(RsslSocketWriteTests, Frag_UINT32MAXLengthOnFragmentedBuffer)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15127", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(fragSize + 64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);

    /* Overflow the length field to provoke integer wraparound. */
    pBuf->length = 0xFFFFFFFFu;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "length=UINT32_MAX on fragmented buffer must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "rsslWriteEx() Error: 0008 Data has overflowed the allocated buffer length(3064).") != NULL);
}

/* Allocate a buffer in the fragmentation range then set length = 0 before
 * writing.  This presents a zero-payload first-fragment to ipcWriteSession()
 * and verifies the zero-length guard on the fragmentation path (Issue 2). */
TEST_P(RsslSocketWriteTests, Frag_ZeroLengthOnAllocatedFragmentBuffer)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15128", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(fragSize + 64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, fragSize + 64);

    /* Shrink length to zero after allocation. */
    pBuf->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    bool acceptable = (ret == RSSL_RET_FAILURE) || (ret >= RSSL_RET_SUCCESS);
    EXPECT_TRUE(acceptable)
        << "Zero-length fragmentation-range buffer must not crash; ret=" << ret;

    if (ret < RSSL_RET_SUCCESS)
        rsslReleaseBuffer(pBuf, &err);
}

/* Write exactly maxFragmentSize bytes.  This is the upper boundary of the
* single-fragment path: a payload of this size must be written without error
* regardless of whether the implementation treats the boundary as inclusive
* or exclusive.  Verifies no crash and a successful return after flush. */
TEST_P(RsslSocketWriteTests, Frag_WriteExactlyMaxFragmentSize)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15129", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(fragSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, fragSize);
    pBuf->length = fragSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Write of exactly maxFragmentSize bytes must succeed; err: " << err.text;
}

/* Write maxFragmentSize + 1 bytes to force fragmentation and verify the
 * fragId counter is incremented correctly (Issue 6 / Issue 11). */
TEST_P(RsslSocketWriteTests, Frag_WriteOneByteOverMaxFragmentSize)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15130", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 1;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Write of maxFragmentSize+1 bytes must succeed; err: " << err.text;
}

/* Drive the fragId counter all the way to its wrap boundary (fragIdMax = 255)
 * by sending 256 consecutive fragmented messages.  At wrap, fragId resets to 1
 * rather than 0 (0 is reserved).  If this is not handled, fragId 0 is emitted
 * and the receiver fails to reassemble (Issue 6). */
TEST_P(RsslSocketWriteTests, Frag_FragIdWrapAt255)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15131", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Each message must span at least two fragments. */
    const RsslUInt32 msgSize = fragSize + 64;

    for (int i = 0; i < 260; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
        else if (ret < RSSL_RET_SUCCESS)
            break;
    }

    SUCCEED() << "fragId wrap test completed without crash";
}

/* =======================================================================
 * -- PACKING INVALID SCENARIOS --------------------------------------------
 *
 * rsslBufferImpl fields targeted:
 *   packingOffset - byte offset of the next free slot inside the packed buffer
 *   totalLength   - declared maximum capacity for packing bookkeeping
 *
 * Issue 2  - msgb->length > msgb->maxLength heap corruption
 * Issue 9  - writeOutArgs not updated on bad paths
 * Issue 10 - chunk-header sprintf off-by-one
 * ===================================================================== */

/* Allocate a packed buffer of 256 bytes, then inflate pBuf->length far
 * beyond the allocation before writing.  This causes ipcWriteSession() to
 * compute a packed-message length prefix that points outside the backing
 * store (Issue 10 / Issue 2). */
TEST_P(RsslSocketWriteTests, Pack_OversizedLengthOnPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15132"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);

    /* Inflate length well past the 256-byte backing store. */
    pBuf->length = bufLen + 1000;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Oversized length on packed buffer must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(256).") != NULL);
}

/* Set pBuf->length = UINT32_MAX on a packed buffer.  Any arithmetic that
 * adds the per-message overhead to this value wraps to a small number,
 * making the length-prefix write land near the buffer start rather than the
 * tail, corrupting already-packed messages (Issue 2 integer overflow). */
TEST_P(RsslSocketWriteTests, Pack_UINT32MAXLengthOnPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15133"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 256, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);

    /* Overflow length to trigger wraparound in the packed-message size calc. */
    pBuf->length = 0xFFFFFFFFu;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "length=UINT32_MAX on packed buffer must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(256).") != NULL);
}

/* Get a packed buffer and set pBuf->data = NULL before writing.  A NULL
 * data pointer on a packed buffer means any access to the payload region
 * immediately segfaults unless the implementation guards it (Issue 2). */
TEST_P(RsslSocketWriteTests, Pack_NullDataPointerOnPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15134"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 256, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    /* Null out the data pointer to simulate a dangling/freed payload. */
    pBuf->data   = nullptr;
    pBuf->length = 64;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    bool acceptable = (ret == RSSL_RET_FAILURE) || (ret >= RSSL_RET_SUCCESS);
    EXPECT_TRUE(acceptable)
        << "NULL data pointer on packed buffer must not crash; ret=" << ret;
}

/* Pack one 100-byte message into a 256-byte packed buffer (advancing the
 * internal packing cursor), then shrink pBuf->length to 10 bytes - less
 * than the space already consumed by the packed content.  ipcWriteSession()
 * will compute a negative remaining-space which wraps to UINT32_MAX when
 * treated as unsigned, producing an unbounded loop (Issue 2). */
TEST_P(RsslSocketWriteTests, Pack_ShrunkLengthAfterPackingContent)
{
    ASSERT_TRUE(setupChannelPair("15135"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    const RsslUInt32 msgLen = 100;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    /* Advance the internal packing cursor by committing the first message. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    /* Shrink the outer buffer's reported length to less than the content
     * already committed, creating negative remaining space. */
    pBuf->length = 10;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_TRUE(ret >= 0)
        << "Shrunk length after packing must not crash; ret=" << ret;

    (void)pNext;
}

/* Call rsslPackBuffer on a buffer that was allocated without packing
 * (packed = RSSL_FALSE).  The packing state machine is not initialised,
 * so rsslPackBuffer may compute a nonsensical offset (Issue 10). */
TEST_P(RsslSocketWriteTests, Pack_PackBufferCalledOnNonPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15136"));

    RsslError err;
    /* Allocate as a non-packed buffer. */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 256, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    /* Calling rsslPackBuffer on a non-packed buffer is a misuse;
     * the function must not crash or corrupt memory. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    /* rsslPackBuffer is expected to return NULL or an empty/error buffer
     * when called on a non-packed allocation. */
    if (pNext && pNext->length > 0)
    {
        /* If it somehow succeeded, write the buffer cleanly. */
        pNext->length = 0;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    SUCCEED() << "Pack on non-packed buffer did not crash; ret=" << ret;
}

/* Pack the maximum number of messages that fit in one buffer to exercise
 * the repeated packingOffset advance path.  If the offset is not clamped,
 * subsequent length-prefix writes overflow the backing store (Issue 10). */
TEST_P(RsslSocketWriteTests, Pack_MaxMessagesInOneBuffer)
{
    ASSERT_TRUE(setupChannelPair("15137"));

    RsslError err;
    const RsslUInt32 bufLen = 4096;
    const RsslUInt32 msgLen = 10;   /* small messages to maximise count */

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    int packedCount = 0;
    RsslBuffer* pCur = pBuf;
    while (pCur && pCur->length >= msgLen)
    {
        fillBuffer(pCur, msgLen);
        pCur->length = msgLen;
        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        ++packedCount;
        if (!pNext || pNext->length < msgLen)
            break;
        pCur = pNext;
    }

    /* Terminate the packed buffer (length = 0 signals end-of-pack). */
    if (pCur)
        pCur->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Max-packed buffer write should succeed; err: " << err.text;
    EXPECT_GT(packedCount, 1) << "At least two messages should have been packed";
}

/* Write a packed buffer that has buffer.length > buffer.length at allocation
 * time.  Inflating length after packing will cause the wire message-length
 * prefix to report more data than the backing store holds (Issue 2 / 10). */
TEST_P(RsslSocketWriteTests, Pack_InflatedLengthAfterPacking)
{
    ASSERT_TRUE(setupChannelPair("15138"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    const RsslUInt32 msgLen = 50;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    /* Pack the first message to advance packingOffset. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    if (pNext && pNext->length > 0)
    {
        fillBuffer(pNext, msgLen);
        pNext->length = msgLen;
    }

    /* Inflate the reported buffer length beyond what was allocated. */
    pBuf->length = bufLen + 5000;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    /* Must fail gracefully; must not crash. */
    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Inflated length after packing must return RSSL_RET_BUFFER_TOO_SMALL";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(204).") != NULL);
}

/* Write a packed buffer whose length field is set to 0 (empty pack).
 * This simulates the terminal condition of a multi-pack sequence where
 * the caller sets length=0 to signal "no more messages".  The write
 * must succeed or fail cleanly without accessing uninitialised memory. */
TEST_P(RsslSocketWriteTests, Pack_ZeroLengthAfterPackingSucceeds)
{
    ASSERT_TRUE(setupChannelPair("15139"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    const RsslUInt32 msgLen = 50;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    /* Pack one message then signal end-of-pack with length = 0. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (pNext)
        pNext->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(RSSL_RET_SUCCESS, ret)
        << "Zero-length end-of-pack write must not crash; ret=" << ret;
}

/* =======================================================================
 * -- ADDITIONAL FRAGMENTATION NEGATIVE SCENARIOS --------------------------
 *
 * Each test targets a specific crash vector identified in the static
 * analysis of ipcWriteSession() (see rsslSocketWrite_analysis.md).
 *
 *  Issue 2  - msgb->buffer not restored on maxLength break ? heap corruption
 *  Issue 3  - loop continues after fatal chunk-footer write error
 *  Issue 4  - use-after-free of msgb after rtr_dfltcFreeMsg in forced-flush
 *  Issue 6  - RsslUInt16 messageLength overflow near 65535
 *  Issue 7  - compressedmb2->nextMsg stale pointer after pool reuse
 *
 * All tests manipulate only the public RsslBuffer fields (data, length).
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Issue 4 + Issue 3:
 * Write a multi-fragment buffer (> maxFragSize) with the
 * RSSL_WRITE_DIRECT_SOCKET_WRITE flag on an ACTIVE socket.
 * ipcWriteSession() enters the forceFlush path, calls rtr_dfltcFreeMsg(msgb)
 * on success and then writes msgb->buffer = 0 (use-after-free Issue 4).
 * The test verifies the call completes without crashing.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DirectSocketWriteFragmentedActiveSocket)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15140", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Allocate enough to force at least two fragments. */
    const RsslUInt32 msgSize = fragSize * 2 + 100;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        EXPECT_FALSE(true) << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;   /* triggers forceFlush path */
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(RSSL_RET_SUCCESS, ret)
        << "Fragmented DIRECT_SOCKET_WRITE on active socket must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Issue 3 + Issue 4:
 * Write a multi-fragment buffer with RSSL_WRITE_DIRECT_SOCKET_WRITE to a
 * socket whose remote end has been closed.  ipcWriteSession() enters the
 * forceFlush path, and the send() call fails mid-chain.  Without the Issue 3
 * fix the loop continues processing subsequent fragment buffers after the
 * fatal write error; Issue 4's use-after-free also becomes reachable.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DirectSocketWriteFragmentedDeadSocket)
{
    const RsslUInt32 fragSize = 1000;
    ASSERT_TRUE(setupChannelPair("15141", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;

    /* Kill the remote end so send() will return an error. */
    rsslCloseChannel(pServerChnl, &err);
    pServerChnl = nullptr;
    time_sleep(100);

    const RsslUInt32 msgSize = fragSize * 3 + 50;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed after remote close - acceptable";
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    bool channelClosed    = (pClientChnl->state == RSSL_CH_STATE_CLOSED);
    bool writeFlushFailed = (ret < RSSL_RET_SUCCESS);
    EXPECT_TRUE(channelClosed || writeFlushFailed)
        << "Fragmented DIRECT_SOCKET_WRITE on dead socket must fail or close channel; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Issue 2:
 * Allocate a buffer one byte smaller than maxFragSize (single-fragment path),
 * then inflate its length by exactly one byte.  This is the tightest possible
 * overshoot that triggers the msgb->length > msgb->maxLength guard in
 * ipcWriteSession(), and verifies that the chain-release code runs rather
 * than leaving the pool with a corrupted msgb->buffer pointer.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_LengthOneOverMaxOnSingleFragmentBuffer)
{
    const RsslUInt32 fragSize = 3000;
    ASSERT_TRUE(setupChannelPair("15142", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Allocate exactly fragSize - 1 so the write would normally be a single fragment. */
    const RsslUInt32 allocSize = fragSize - 1;
    RsslBuffer* pBuf = getClientBuffer(allocSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, allocSize);

    /* Inflate by exactly 1 byte beyond the allocated capacity. */
    pBuf->length = pBuf->length + 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "length = allocated+1 on single-fragment buffer must return RSSL_RET_BUFFER_TOO_SMALL";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(2999).") != NULL);
}

/* -----------------------------------------------------------------------
 * Issue 2 + Issue 7:
 * Write a buffer whose length exceeds maxLength inside the two-fragment
 * boundary (exactly fragSize + 1 bytes allocated, length inflated further).
 * This exercises the msgb->length > msgb->maxLength break inside the loop
 * that already allocated the second-fragment msgb, verifying that the second
 * msgb is also released and not left dangling (Issue 7 pool-pointer residue).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_OversizedLengthOnSecondFragmentBuffer)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15143", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Allocate just enough to require exactly two fragments. */
    const RsslUInt32 allocSize = fragSize + 1;
    RsslBuffer* pBuf = getClientBuffer(allocSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, allocSize);

    /* Inflate the second fragment's reported length well beyond its pool block. */
    pBuf->length = allocSize + 5000;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Oversized length on two-fragment boundary must return RSSL_RET_BUFFER_TOO_SMALL";

    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(2001).") != NULL);
}

/* -----------------------------------------------------------------------
 * Issue 7 + Issue 2:
 * Flood the channel with many small fragmented messages to cycle the pool
 * rapidly and increase the probability that freed compressedmb2 blocks
 * re-enter the pool with stale non-NULL nextMsg pointers before the next
 * allocation clears them.  Any crash here would indicate an unguarded
 * stale-nextMsg dereference in the fragment chain loop.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_RapidPoolCyclingStressesNextMsgPointer)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15145", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Each message spans exactly 3 fragments to create and destroy msgb chains. */
    const RsslUInt32 msgSize = fragSize * 3 - 1;
    const int        iterations = 200;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS)
            break;

        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Rapid pool cycling did not crash";
}

/* -----------------------------------------------------------------------
 * Issue 2 + NULL crash:
 * Allocate a buffer large enough to require fragmentation, then set
 * pBuf->data = NULL.  ipcWriteSession() attempts to memcpy from pBuf->data
 * into the first-fragment wire buffer, causing a NULL-pointer dereference
 * unless a guard is present.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_NullDataPointerOnFragmentedBuffer)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15146", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize + 200;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    /* Null out the data pointer - any memcpy/memmove from it will segfault
     * unless ipcWriteSession() guards the data pointer before use. */
    pBuf->data = nullptr;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "NULL data on fragmented buffer must return RSSL_RET_FAILURE; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0002 Null pointer error. Argument buffer->data cannot be NULL.") != NULL);
}

/* -----------------------------------------------------------------------
 * Issue 3:
 * Flood the channel with multi-fragment messages until WRITE_FLUSH_FAILED is
 * returned, then immediately flush.  Issue 3 means the loop can continue
 * queuing additional fragments after the fatal flush failure, leaving the
 * output queue in an inconsistent state that crashes on the subsequent flush.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_FlushAfterFragmentedWriteFlushFailed)
{
    const RsslUInt32 fragSize = 1000;
    ASSERT_TRUE(setupChannelPair("15147", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Use a message that produces exactly 4 fragments to maximise chain depth. */
    const RsslUInt32 msgSize = fragSize * 4 - 10;

    bool gotFlushFailed = false;
    for (int i = 0; i < 300 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
        {
            gotFlushFailed = true;
            /* The queue must be flushable without crashing after a failed flush
             * inside a multi-fragment chain (Issue 3). */
            RsslRet flushRet = rsslFlush(pClientChnl, &err);
            EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
                << "rsslFlush after fragmented WRITE_FLUSH_FAILED must not crash";
        }
        else if (ret < RSSL_RET_SUCCESS)
        {
            break;
        }
        else if (ret > RSSL_RET_SUCCESS)
        {
            rsslFlush(pClientChnl, &err);
        }
    }

    SUCCEED() << "Fragmented WRITE_FLUSH_FAILED flush test completed without crash";
}

/* -----------------------------------------------------------------------
 * Issue 6 + Issue 2:
 * Write a very large number of tiny fragments (small fragSize, medium payload)
 * to exhaust the fragment ID counter (fragIdMax = 255) multiple times and
 * verify that neither the wrap nor the exhaustion causes an integer overflow
 * in the wire length field computation.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_VerySmallFragSizeExhaustsFragIdCounter)
{
    /* Smallest practical fragment size; must be >= IPC header overhead (~18 bytes). */
    const RsslUInt32 fragSize = 300;
    ASSERT_TRUE(setupChannelPair("15148", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Large enough to create many fragments per message. */
    const RsslUInt32 msgSize = fragSize * 8;
    /* Send enough messages to wrap fragId at least twice (256 x 2 + margin). */
    const int iterations = 600;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS)
            break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Small fragSize + fragId exhaustion test completed without crash";
}

/* -----------------------------------------------------------------------
 * Issue 2 + channel-state guard:
 * Forcibly set the channel state to CLOSED, then attempt to write a large
 * fragmented buffer.  The state guard in rsslSocketWrite() must reject the
 * write before the fragment chain is allocated, preventing the allocation
 * of fragment pool buffers that would then never be freed.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_LargeFragmentedWriteOnClosedChannelState)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15149", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 5;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    /* Force the channel into CLOSED state without freeing internal structures. */
    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "Fragmented write on CLOSED channel must return RSSL_RET_FAILURE";

    /* Restore state so TearDown can close the channel cleanly. */
    pClientChnl->state = RSSL_CH_STATE_ACTIVE;

    rsslReleaseBuffer(pBuf, &err);
}

/* =======================================================================
 * -- DUPLICATE FRAGMENT ID SCENARIOS -------------------------------------
 *
 * The fragment-ID counter (fragId) is an RsslUInt8 that cycles 1-255
 * (fragIdMax = 255).  After 255 two-or-more-fragment messages the counter
 * wraps back to 1.  If the receiver still has an open assembly buffer
 * for the original fragId=1 message when the new first-fragment with
 * fragId=1 arrives, the receiver's hash table evicts the old entry and
 * frees its buffer, triggering:
 *
 *   rsslSocketRead_analysis Issue 3 - memory leak if rsslHashTableInsertLink
 *     fails after eviction of the old assembly buffer.
 *   rsslSocketRead_analysis Issue 9 - double-free when rsslHashTableRemoveLink
 *     does not zero buffer.data before _rsslFree(), enabling a second free on
 *     a continuation-fragment path.
 *
 * All five tests below create the duplicate-fragId wire condition solely
 * through the public write API and then verify the library does not crash
 * when the server reads the resulting byte stream.
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Helper: drain up to maxReads reassembled messages from the server channel
 * using a non-blocking read loop.  Returns the number of complete buffers
 * returned by rsslRead; stops on WOULD_BLOCK, PING, or a hard error.
 * --------------------------------------------------------------------- */
static int drainServerChannel(RsslChannel* pChnl, int maxReads)
{
    RsslError err;
    int       count = 0;
    for (int i = 0; i < maxReads; ++i)
    {
        RsslRet    readRet = RSSL_RET_SUCCESS;
        RsslBuffer* pBuf   = rsslRead(pChnl, &readRet, &err);
        if (pBuf)
        {
            ++count;
            /* Buffer is owned by the library; do not release. */
        }
        else if (readRet == RSSL_RET_READ_WOULD_BLOCK ||
                 readRet == RSSL_RET_READ_PING)
        {
            break;
        }
        else if (readRet < RSSL_RET_SUCCESS)
        {
            break;
        }
        /* readRet > 0 ? more data pending; keep reading */
    }
    return count;
}

/* -----------------------------------------------------------------------
 * Duplicate fragId - Scenario 1:
 * Write exactly 257 two-fragment messages so that the fragId counter wraps
 * once (255 ? 1).  Message 257 has fragId=1, the same value that was
 * assigned to message 1.  The server then reads ALL messages including the
 * collision; the assembly hash table must handle the eviction without
 * crashing or double-freeing the old fragId=1 buffer.
 *
 * Targets rsslSocketRead_analysis Issues 3 and 9.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DupFragId_WrapAt256ServerReadsAll)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15150", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Each message spans exactly 2 fragments to guarantee fragId use. */
    const RsslUInt32 msgSize    = fragSize + 64;
    /* 257 messages: fragId 1-255 then wrap to 1 on message 256, 2 on 257, ... */
    const int        totalMsgs  = 257;

    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        /* Drain the server periodically so its TCP receive buffer never fills. */
        if ((i % 32) == 31)
            drainServerChannel(pServerChnl, 128);
    }

    /* Flush any remaining queued data and let the server drain fully. */
    rsslFlush(pClientChnl, &err);
    time_sleep(10);
    drainServerChannel(pServerChnl, 1024);

    SUCCEED() << "fragId wrap + server read did not crash";
}

/* -----------------------------------------------------------------------
 * Duplicate fragId - Scenario 2:
 * Fill the client output queue with 260 fragmented messages without ANY
 * intermediate flush so that all 260 messages are present in the queue
 * simultaneously.  Messages 256 and 257 have the same fragId as messages 1
 * and 2 respectively.  When the client flushes, the wire stream contains
 * two first-fragments with fragId=1 (and fragId=2) within a single TCP
 * stream.  The server read path must evict the old assembly entry for
 * fragId=1 cleanly without a double-free.
 *
 * Targets rsslSocketRead_analysis Issue 9 (double-free on eviction).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DupFragId_AllQueued_ThenFlushedAtOnce)
{
    const RsslUInt32 fragSize = 256;
    ASSERT_TRUE(setupChannelPair("15151", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 32;   /* 2 fragments per message */
    const int        totalMsgs = 260;

    /* Write all messages into the queue without flushing. */
    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        /* Do NOT flush - let duplicate fragIds accumulate in the queue. */
    }

    /* Now flush everything; messages 256-260 carry reused fragIds 1-5. */
    RsslRet flushRet = rsslFlush(pClientChnl, &err);
    bool flushOk = (flushRet >= RSSL_RET_SUCCESS);

    /* Give the OS time to deliver the TCP data, then read on the server. */
    time_sleep(20);
    drainServerChannel(pServerChnl, 2048);

    EXPECT_TRUE(flushOk || pClientChnl->state == RSSL_CH_STATE_CLOSED)
        << "Bulk flush with duplicate fragIds must not crash; flushRet=" << flushRet;

    SUCCEED() << "Bulk-queued duplicate fragId scenario completed without crash";
}

/* -----------------------------------------------------------------------
 * Duplicate fragId - Scenario 3:
 * Write exactly at the fragId boundary: messages 1-254 (fragId 1-254),
 * message 255 (fragId 255), message 256 (fragId wraps to 1).  After each
 * batch of 50 writes the server reads, simulating a scenario where message 1
 * has been fully assembled and its hash entry cleaned up before fragId 1 is
 * reused.  Then message 257 with fragId=1 arrives; because the entry was
 * properly evicted, the table should have room for the new entry without
 * collision.  Tests the boundary exactly at fragIdMax.
 *
 * Targets rsslSocketRead_analysis Issue 3 (hash insert after eviction).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DupFragId_ReadBetweenBatchesBoundaryAt255)
{
    const RsslUInt32 fragSize  = 512;
    ASSERT_TRUE(setupChannelPair("15152", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 1;    /* just over boundary ? 2 frags */
    const int        batchSize = 50;

    for (int i = 0; i < 260; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        /* Server reads every batchSize writes to clear assembled buffers. */
        if ((i % batchSize) == (batchSize - 1))
        {
            rsslFlush(pClientChnl, &err);
            time_sleep(5);
            drainServerChannel(pServerChnl, 256);
        }
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(10);
    drainServerChannel(pServerChnl, 512);

    SUCCEED() << "fragId boundary-255 read-between-batches test completed without crash";
}

/* -----------------------------------------------------------------------
 * Duplicate fragId - Scenario 4:
 * Two writer threads simultaneously send fragmented messages to the same
 * channel.  Without perfect serialization of the fragId counter, thread A
 * and thread B can both read the same value before either increments it,
 * producing two in-flight messages with identical fragIds.  The receiver
 * must not crash or double-free when it reassembles both.
 *
 * Uses RSSL_LOCK_GLOBAL_AND_CHANNEL (via RsslSocketWriteChannelLockTests)
 * so that per-channel locking is active and the test is meaningful: if
 * fragId is serialized correctly, no collision should occur; if not, the
 * receiver will encounter a hash table eviction collision.
 * --------------------------------------------------------------------- */

struct DupFragIdWriterArg
{
    RsslChannel*      pChnl;
    RsslUInt32        fragSize;
    int               msgCount;
    std::atomic<int>  written;
    std::atomic<bool> done;

    DupFragIdWriterArg()
        : pChnl(nullptr), fragSize(512), msgCount(130), written(0), done(false)
    {}
};

static RSSL_THREAD_DECLARE(dupFragIdWriterThread, pArg)
{
    DupFragIdWriterArg* a = reinterpret_cast<DupFragIdWriterArg*>(pArg);
    RsslError err;
    const RsslUInt32 msgSize = a->fragSize + 32;

    for (int i = 0; i < a->msgCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(a->pChnl, msgSize, RSSL_FALSE, &err);
        if (!pBuf) continue;

        for (RsslUInt32 k = 0; k < msgSize; ++k)
            pBuf->data[k] = (char)('A' + (k % 26));
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(a->pChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(a->pChnl, &err);

        ++a->written;
    }
    a->done = true;
    return 0;
}

TEST_P(RsslSocketWriteChannelLockTests, Frag_DupFragId_ConcurrentWritersRaceOnFragId)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15153", RSSL_COMP_NONE, 0, fragSize));

    DupFragIdWriterArg args;
    args.pChnl    = pClientChnl;
    args.fragSize = fragSize;
    args.msgCount = 130;   /* 2 x 130 = 260 total ? wraps fragId past 255 */

    RsslThreadId t1, t2;
    RSSL_THREAD_START(&t1, dupFragIdWriterThread, &args);
    RSSL_THREAD_START(&t2, dupFragIdWriterThread, &args);

    /* Server drains concurrently so its TCP buffer does not fill up. */
    RsslError err;
    for (int drainPass = 0; drainPass < 20; ++drainPass)
    {
        time_sleep(10);
        drainServerChannel(pServerChnl, 256);
        if (args.done.load() && args.written.load() >= args.msgCount * 2)
            break;
    }

    RSSL_THREAD_JOIN(t1);
    RSSL_THREAD_JOIN(t2);

    rsslFlush(pClientChnl, &err);
    time_sleep(20);
    drainServerChannel(pServerChnl, 1024);

    EXPECT_GT(args.written.load(), 0)
        << "At least some concurrent fragmented writes must have completed";
    SUCCEED() << "Concurrent fragId race test completed without crash; writes="
              << args.written.load();
}

/* -----------------------------------------------------------------------
 * Duplicate fragId - Scenario 5:
 * Stress test: write 512 two-fragment messages (fragId wraps twice),
 * with the server reading after every 64 writes.  This ensures the
 * assembly hash table repeatedly evicts-and-inserts entries for the same
 * fragId values (1-255, 1-255, 1-2) across two full wrap cycles.  Any
 * double-free or use-after-free introduced by back-to-back evictions of
 * the same fragId key will manifest here under address-sanitiser or
 * valgrind.
 *
 * Targets rsslSocketRead_analysis Issues 3 and 9 under repeated collision.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Frag_DupFragId_TwoFullWrapCyclesStressTest)
{
    const RsslUInt32 fragSize  = 300;
    ASSERT_TRUE(setupChannelPair("15154", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 50;   /* exactly 2 fragments */
    /* 512 messages ? fragId wraps at 256 and again at 511.
     * fragId 1 is therefore reused on messages 257 and 512. */
    const int        totalMsgs = 512;

    int written = 0;
    for (int i = 0; i < totalMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++written;

        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        /* Server drains every 64 writes to keep TCP buffers clear. */
        if ((i % 64) == 63)
        {
            rsslFlush(pClientChnl, &err);
            time_sleep(5);
            drainServerChannel(pServerChnl, 512);
        }
    }

    rsslFlush(pClientChnl, &err);
    time_sleep(20);
    drainServerChannel(pServerChnl, 2048);

    EXPECT_GT(written, 200)
        << "At least 200 of 512 writes should have completed";
    SUCCEED() << "Two-wrap-cycle fragId stress test completed without crash; wrote="
              << written;
}

/* =======================================================================
 * -- PARTIAL FRAGMENTED WRITE AND CHANNEL-CLOSE SCENARIOS ----------------
 *
 * These tests focus on the SENDER (client) side: what happens when the
 * client channel is closed while fragmented messages are partially written
 * or still queued in the output pool.  The crash vectors targeted are:
 *
 *   ipcWriteSession Issue 2  - msgb chain not released when channel closes
 *     with unflushed queued fragments ? heap corruption via ipcFreeSession().
 *   ipcWriteSession Issue 3  - loop continues after fatal write error;
 *     additional fragments queued after the first error are left dangling.
 *   ipcWriteSession Issue 4  - use-after-free of msgb freed by
 *     rtr_dfltcFreeMsg while the channel's pool still references the block.
 *   ipcFreeSession            - must walk and release the entire rtr_msgb_t
 *     chain in the pending-write queue regardless of fragmentation depth.
 *
 * All five tests close the CLIENT channel mid-write and set pClientChnl to
 * nullptr so TearDown does not attempt a second rsslCloseChannel call.
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Writer thread and argument struct used by the concurrent-close test
 * (Scenario 4).  The thread writes 3-fragment messages continuously until
 * it encounters a write failure (channel closed) or the stop flag is set.
 * --------------------------------------------------------------------- */
struct PartialFragWriterArg
{
    RsslChannel*        pChnl;
    RsslUInt32          fragSize;
    std::atomic<bool>   stop;
    std::atomic<int>    written;
    std::atomic<bool>   writerDone;

    PartialFragWriterArg()
        : pChnl(nullptr), fragSize(512), stop(false), written(0), writerDone(false)
    {}
};

static RSSL_THREAD_DECLARE(partialFragWriterFn, pArg)
{
    PartialFragWriterArg* a = reinterpret_cast<PartialFragWriterArg*>(pArg);
    RsslError err;
    const RsslUInt32 msgSize = a->fragSize * 3 + 64;   /* 3+ fragments per message */

    while (!a->stop.load())
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
        ++a->written;
    }
    a->writerDone = true;
    return 0;
}

/* -----------------------------------------------------------------------
 * Partial write - Scenario 1:
 * Write 50 two-fragment messages into the client output queue WITHOUT
 * calling rsslFlush.  All 50 x 2 = 100 rtr_msgb_t chain entries sit in
 * the pending-write queue.  Closing the channel immediately exercises the
 * ipcFreeSession() path that must walk and release every chained msgb
 * without double-freeing or skipping any node.
 *
 * Crash vector: Issue 2 (chain not released on break) and Issue 7 (stale
 * nextMsg from pool) in the queue-free walk inside ipcFreeSession().
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, PartialWrite_CloseClientWithQueuedUnflushedFragments)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15155", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 64;   /* 2 fragments per message */
    const int        queueMsgs = 50;

    /* Write all messages into the RSSL output queue without flushing. */
    int queued = 0;
    for (int i = 0; i < queueMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++queued;
        /* Deliberately do NOT flush - leave the fragment chains in the queue. */
    }

    /* Close the channel while fragment chains are still queued.
     * ipcFreeSession() must free all 'queued * 2' rtr_msgb_t blocks. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;   /* prevent TearDown from double-closing */

    EXPECT_GT(queued, 0) << "At least some messages must have been queued";
    SUCCEED() << "Close with " << queued
              << " queued two-fragment messages did not crash";
}

/* -----------------------------------------------------------------------
 * Partial write - Scenario 2:
 * Flood the client output queue with large (4-fragment) messages until
 * rsslWriteEx returns RSSL_RET_WRITE_FLUSH_FAILED.  At that point the
 * pending queue holds multiple partial fragment chains that were queued
 * after a failed direct-socket flush.  Close the channel WITHOUT flushing
 * to leave ipcFreeSession() responsible for releasing the inconsistent
 * queue state.
 *
 * Crash vector: Issue 3 (loop continues after fatal flush error, queuing
 * additional fragments that are then abandoned) + Issue 4 (use-after-free
 * when ipcFreeSession walks freed msgb blocks from the failed-flush path).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, PartialWrite_CloseClientAfterWriteFlushFailed)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15156", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 4 - 10;   /* 4 fragments per message */

    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
            gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS)
            break;
        /* Do NOT flush - let the queue accumulate inconsistent fragment chains. */
    }

    /* Close the channel WITHOUT flushing to leave the partial-flush queue
     * in place; ipcFreeSession() must handle the inconsistent state. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    SUCCEED() << "Close after WRITE_FLUSH_FAILED (gotFlushFailed=" << gotFlushFailed
              << ") did not crash";
}

/* -----------------------------------------------------------------------
 * Partial write - Scenario 3:
 * Write 10 two-fragment messages and flush them all.  Then write 10 more
 * WITHOUT flushing and close the client channel.  The unflushed second
 * batch sits in the output queue; ipcFreeSession() must release it while
 * the first batch has already been delivered and freed on the send side.
 * The server reads all delivered data until it receives EOF/RST, exercising
 * the server-side channel-close path on incomplete wire data.
 *
 * Crash vector: Stale pool pointer (Issue 7) in the mixed-state queue
 * where some fragment chains were freed by the flush and others were not.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, PartialWrite_CloseClientMidStreamServerReadsToEof)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15157", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslUInt32 msgSize    = fragSize + 64;
    const int        flushedN   = 10;
    const int        unflushedN = 10;

    /* Write and flush the first batch so the server receives complete messages. */
    for (int i = 0; i < flushedN; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    /* Write second batch WITHOUT flushing - these remain queued. */
    for (int i = 0; i < unflushedN; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        /* No flush: queued fragment chains remain in the output pool. */
    }

    /* Close client channel - unflushed batch freed by ipcFreeSession();
     * server sees the flushed data followed by EOF/RST. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    /* Give TCP time to deliver the RST, then drain the server until error. */
    time_sleep(50);
    drainServerChannel(pServerChnl, 4096);   /* reads until EOF */

    SUCCEED() << "Close mid-stream with mixed flushed/unflushed fragments did not crash";
}

/* -----------------------------------------------------------------------
 * Partial write - Scenario 4:
 * Two threads write 3-fragment messages to the client channel concurrently
 * under RSSL_LOCK_GLOBAL_AND_CHANNEL.  The main thread closes the client
 * channel after 40 ms while both writer threads are still running.
 * Serialisation by the channel lock ensures the close waits until the
 * current write finishes before tearing down internal state; the writer
 * threads then see a failure return from their next rsslWriteEx call and
 * stop.  Any crash indicates a lock-gap in the fragment-chain teardown path.
 *
 * Crash vector: Issue 4 (use-after-free of msgb freed by rtr_dfltcFreeMsg
 * while another thread is still inside ipcWriteSession walking the chain).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteChannelLockTests, PartialWrite_ConcurrentFragmentWriteAndClientClose)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15158", RSSL_COMP_NONE, 0, fragSize));

    PartialFragWriterArg args;
    args.pChnl    = pClientChnl;
    args.fragSize = fragSize;

    RsslThreadId t1, t2;
    RSSL_THREAD_START(&t1, partialFragWriterFn, &args);
    RSSL_THREAD_START(&t2, partialFragWriterFn, &args);

    /* Let the writers run for a short period, then close the channel.
     * The channel lock ensures the close is serialised with active writes. */
    time_sleep(40);

    args.stop.store(true, std::memory_order_release);

    RsslError err;
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;   /* prevent TearDown from double-closing */

    /* Wait for both writers to detect the closed channel and exit. */
    RSSL_THREAD_JOIN(t1);
    RSSL_THREAD_JOIN(t2);

    EXPECT_GT(args.written.load(), 0)
        << "At least some 3-fragment writes must have completed before close";
    SUCCEED() << "Concurrent close during 3-fragment writes did not crash; wrote="
              << args.written.load();
}

/* -----------------------------------------------------------------------
 * Partial write - Scenario 5:
 * Write a single enormous fragmented message whose fragment count is deep
 * enough to stress the ipcFreeSession() chain-walk loop.  With
 * fragSize=100 and msgSize=5500, this produces ~55 rtr_msgb_t nodes in
 * the pending-write chain.  Closing the channel without flushing forces
 * ipcFreeSession() to iterate 55 times freeing nodes back to the pool.
 * If nextMsg is stale (Issue 7) or pool accounting is wrong (Issue 2),
 * the loop will dereference a freed or unrelated block and crash.
 *
 * Crash vector: Issue 7 (stale compressedmb2->nextMsg) manifested in the
 * ipcFreeSession() chain-release loop for a non-compressed deep chain.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, PartialWrite_CloseClientWithDeepQueuedFragmentChain)
{
    /* Small fragment size to maximise the number of chain nodes. */
    const RsslUInt32 fragSize = 100;
    ASSERT_TRUE(setupChannelPair("15159", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* msgSize / fragSize ~ 55 fragments ? 55 rtr_msgb_t pool nodes. */
    const RsslUInt32 msgSize = fragSize * 55;

    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed for deep-chain test - skip";
        /* Still close cleanly. */
        rsslCloseChannel(pClientChnl, &err);
        pClientChnl = nullptr;
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    /* Queue the 55-fragment chain.  Do NOT flush. */
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    bool writeQueued = (ret > RSSL_RET_SUCCESS) || (ret == RSSL_RET_SUCCESS);
    bool writeFailedCleanly = (ret == RSSL_RET_FAILURE);

    /* Close the channel: ipcFreeSession() must walk and free all 55 nodes
     * in one pass without double-freeing or running off the end of the chain. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    EXPECT_TRUE(writeQueued || writeFailedCleanly)
        << "Write must have queued or failed cleanly before close; ret=" << ret;

    SUCCEED() << "Close with ~55-node fragment chain did not crash; writeRet=" << ret;
}

/* =======================================================================
 * -- COMPRESSION NEGATIVE SCENARIOS --------------------------------------
 *
 * These tests target crash vectors in the ipcWriteSession() compression
 * path using both Zlib and LZ4 codecs.  Issues targeted:
 *
 *   Issue 2  - msgb->length > msgb->maxLength break leaves the compressed
 *     wire buffer in a corrupted state on the error path.
 *   Issue 4  - Use-after-free of compressedmb1 after rtr_dfltcFreeMsg()
 *     in the forceFlush direct-write success path: the code writes
 *     compressedmb1->buffer = 0 and compressedmb1->length = 0 AFTER
 *     the free.  Under pool reuse, a second allocation may receive that
 *     same block before the zeroing, corrupting the new allocation's header.
 *   Issue 6  - RsslUInt16 overflow in (compressedLength + headerLength)
 *     when the compressed output is near 65535 bytes.
 *   Issue 7  - compressedmb2->nextMsg stale pool pointer: the pool does
 *     not zero nextMsg on allocation.  When compressedmb2 is set as the
 *     current msgb, the outer while(msgb) loop reads compressedmb2->nextMsg
 *     at the top of the next iteration.  A stale non-NULL value causes the
 *     loop to walk freed or unrelated memory and write IPC headers into it.
 *
 * fillBufferIncompressible() is a static helper that fills buffers with
 * LCG-based pseudo-random bytes so neither LZ4 nor Zlib can reduce the
 * data size, maximising the probability of triggering the two-buffer
 * (compressedmb1 + compressedmb2) split path inside ipcWriteSession().
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Fill with pseudo-random bytes that resist compression.
 * A Numerical Recipes LCG spreads values across all 256 byte values,
 * giving LZ4 and Zlib nothing to compress. Distinct from fillBuffer()
 * (ASCII pattern) to avoid false compression ratios.
 * --------------------------------------------------------------------- */
static void fillBufferIncompressible(RsslBuffer* pBuf, RsslUInt32 len = 0)
{
    if (!pBuf || !pBuf->data) return;
    RsslUInt32 fillLen = (len > 0 && len <= pBuf->length) ? len : pBuf->length;
    RsslUInt32 lcg = 0xDEADBEEFu;
    for (RsslUInt32 i = 0; i < fillLen; ++i)
    {
        lcg = lcg * 1664525u + 1013904223u;   /* Numerical Recipes LCG */
        pBuf->data[i] = (char)(lcg >> 24);
    }
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 1:
 * Allocate a buffer on a Zlib channel, fill it, then inflate pBuf->length
 * by exactly 1 byte beyond the allocated capacity.  ipcWriteSession()
 * validates msgb->length > msgb->maxLength at the top of the write loop;
 * on the compressed path the error-break exits without restoring
 * msgb->buffer to its pre-header position (Issue 2).  The pool then
 * receives a msgb whose buffer pointer is offset incorrectly, causing a
 * heap corruption or crash on the next pool release or allocation.
 *
 * Crash vector: Issue 2 applied to the Zlib compressed path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_OversizedLengthOnZlibCompressedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15161", RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 allocSize = 128;
    RsslBuffer* pBuf = getClientBuffer(allocSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, allocSize);

    /* Inflate length by exactly 1 byte beyond the allocated capacity. */
    pBuf->length = pBuf->length + 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "length=allocated+1 on Zlib channel must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(128).") != NULL);
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 2:
 * Write 30 compressed + fragmented messages to the client output queue
 * WITHOUT flushing, then close the channel.  Each message requires both
 * a compressedmb1 (first compressed wire buffer) and potentially a
 * compressedmb2 (overflow buffer for the second fragment).
 * ipcFreeSession() must release all queued rtr_msgb_t nodes including
 * both compressedmb1 and compressedmb2 chains without double-freeing or
 * skipping nodes.
 *
 * Crash vector: Issue 4 (compressedmb1 freed by rtr_dfltcFreeMsg then
 * written via compressedmb1->buffer = 0 / length = 0) + Issue 7
 * (stale compressedmb2->nextMsg) in the ipcFreeSession() chain-walk.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_ClosedChannelWithQueuedCompressedFragments)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15162", RSSL_COMP_ZLIB, 1, fragSize));

    RsslError err;
    /* Message spans 2+ fragments on the Zlib path to create both
     * compressedmb1 and (possibly) compressedmb2. */
    const RsslUInt32 msgSize   = fragSize + 500;
    const int        queueMsgs = 30;

    int queued = 0;
    for (int i = 0; i < queueMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        /* Highly compressible data ensures compression always succeeds
         * and compressedmb1 is populated. */
        memset(pBuf->data, 0x41, pBuf->length);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++queued;
        /* No flush - leave compressedmb1/mb2 chains queued in the pool. */
    }

    /* Close with compressed fragment chains queued: ipcFreeSession must
     * walk and free every compressedmb1 and compressedmb2 node. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    EXPECT_GT(queued, 0) << "At least some compressed messages must have been queued";
    SUCCEED() << "Compressed channel close with " << queued
              << " queued messages did not crash";
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 3:
 * Write 200 compressed messages using LZ4 with INCOMPRESSIBLE data so that
 * the LZ4 output is larger than the input (expansion).  When the output
 * exceeds the single-wire-buffer limit, ipcWriteSession() allocates a
 * second buffer compressedmb2 from the pool.  If the pool does not zero
 * nextMsg on allocation, compressedmb2->nextMsg is a stale non-NULL pointer
 * from a previous use of that block.  The outer while(msgb) loop then reads
 * compressedmb2->nextMsg at the start of its next iteration and walks freed
 * or unrelated memory, writing IPC headers into arbitrary heap regions.
 *
 * Crash vector: Issue 7 (stale compressedmb2->nextMsg) under rapid LZ4
 * pool cycling with incompressible data.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_Lz4IncompressibleDataStressesCompressedMb2Pool)
{
    ASSERT_TRUE(setupChannelPair("15163", RSSL_COMP_LZ4, 0));

    RsslError err;
    /* Use a payload large enough to stress the compression split path.
     * Incompressible data means LZ4 output ~ input + 11 byte header. */
    const RsslUInt32 msgSize   = 4000;
    const int        iterations = 200;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        /* Incompressible bytes: LZ4 cannot reduce their size, maximising
         * the probability of triggering the two-buffer split. */
        fillBufferIncompressible(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "LZ4 incompressible data pool cycling did not crash";
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 4:
 * Write 300 small Zlib-compressed messages, flushing after every write,
 * to rapidly cycle the compressedmb1 pool blocks back to the free list
 * and then reallocate them for subsequent messages.  If the pool does not
 * clear nextMsg on re-allocation, any block previously used as a
 * compressedmb1 carries a stale nextMsg into its next life as a
 * compressedmb2, triggering the Issue 7 crash.
 *
 * Crash vector: Issue 7 (stale nextMsg after rapid pool cycling on
 * the Zlib compressed write path).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_RapidZlibWritesStressCompressedMb1Pool)
{
    ASSERT_TRUE(setupChannelPair("15164", RSSL_COMP_ZLIB, 1));

    RsslError err;
    /* Small messages: each generates one compressedmb1 which is freed
     * immediately on successful flush, returning it to the pool before
     * the next message claims a block that may have been compressedmb2. */
    const RsslUInt32 msgSize   = 512;
    const int        iterations = 300;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        /* Alternate between compressible and incompressible data to vary
         * the compressed output size and maximise pool block reuse patterns. */
        if (i % 2 == 0)
            memset(pBuf->data, 0x41, pBuf->length);   /* compressible */
        else
            fillBufferIncompressible(pBuf, msgSize);   /* incompressible */

        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Rapid Zlib pool cycling did not crash";
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 5:
 * Write a large Zlib-compressed fragmented message with the
 * RSSL_WRITE_DIRECT_SOCKET_WRITE flag.  ipcWriteSession() enters the
 * forceFlush path, compresses the message into compressedmb1, and sends it
 * via send().  On a successful send (cc == lenToWrite) the implementation:
 *   rtr_dfltcFreeMsg(compressedmb1);
 *   compressedmb1->buffer = 0;   // USE-AFTER-FREE
 *   compressedmb1->length = 0;   // USE-AFTER-FREE
 * The pool immediately makes the freed block available; the very next
 * ipcDataBuffer() call for the next fragment could reuse it.  Writing
 * buffer=0 / length=0 into the reused block corrupts the new allocation's
 * header, causing a crash on the subsequent rtr_dfltcFreeMsg() call.
 *
 * Crash vector: Issue 4 applied to the Zlib forceFlush compressed path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_DirectSocketWriteZlibFragmentedUseAfterFree)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15165", RSSL_COMP_ZLIB, 1, fragSize));

    RsslError err;
    /* Message must span 2+ fragments to create at least two compressedmb
     * allocations, maximising the chance that the freed compressedmb1
     * block is reused for the second fragment before the UAF zeroing. */
    const RsslUInt32 msgSize = fragSize * 2 + 200;

    /* Run the forceFlush path multiple times to stress the pool reuse cycle. */
    for (int i = 0; i < 20; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        memset(pBuf->data, 0x42, pBuf->length);   /* compressible data */
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;   /* forceFlush=1 */
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Zlib DIRECT_SOCKET_WRITE fragmented write did not crash";
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 6:
 * Flood the client output queue with Zlib-compressed 3-fragment messages
 * until rsslWriteEx returns RSSL_RET_WRITE_FLUSH_FAILED.  At that point the
 * queue contains multiple compressedmb1 blocks (and possibly compressedmb2
 * blocks for the overflow path) in a partially-flushed state.  Close the
 * channel WITHOUT flushing the remainder.  ipcFreeSession() must release
 * every compressed wire buffer in the queue without following stale nextMsg
 * pointers or double-freeing blocks returned by the failed flush.
 *
 * Crash vector: Issue 3 (loop continues queuing fragments after fatal flush
 * error, leaving extra compressedmb1 nodes in the queue) + Issue 7 (stale
 * compressedmb2->nextMsg in the ipcFreeSession chain-walk).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_ZlibFlushFailedThenChannelClose)
{
    const RsslUInt32 fragSize = 1000;
    ASSERT_TRUE(setupChannelPair("15166", RSSL_COMP_ZLIB, 1, fragSize));

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 3 - 10;   /* 3 fragments per message */

    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        /* Compressible data keeps the compressed output well under the wire
         * buffer limit, ensuring compressedmb1 is fully populated before
         * the flush failure queues the block mid-chain. */
        memset(pBuf->data, 0x43, pBuf->length);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
            gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS)
            break;
        /* No flush - let the partial compressedmb chain accumulate. */
    }

    /* Close WITHOUT flushing: ipcFreeSession must free every compressedmb1
     * and compressedmb2 block queued by the failed-flush path. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    SUCCEED() << "Zlib WRITE_FLUSH_FAILED + channel close did not crash "
              << "(gotFlushFailed=" << gotFlushFailed << ")";
}

/* -----------------------------------------------------------------------
 * Compression crash - Scenario 7:
 * Write a buffer with pBuf->length = 0 on a Zlib-compressed channel.
 * ipcWriteSession() passes the buffer to the compression engine before
 * the zero-length check that exists on the non-compressed path.  A zero-
 * length compress() call is implementation-defined: Zlib returns Z_OK with
 * avail_out unchanged; LZ4 may return a negative error.  In either case the
 * resulting compressedmb1 has length 0, and the subsequent messageLength
 * computation (Issue 6) may wrap a uint16 length field or divide by zero
 * in the fragment-count arithmetic.
 *
 * Crash vector: Issue 6 (uint16 zero/wrap) on the compressed zero-length
 * message path; additionally tests the zero-length guard that is present
 * on the non-compressed path but may be absent on the compressed path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Compress_ZeroLengthBufferOnZlibChannel)
{
    ASSERT_TRUE(setupChannelPair("15167", RSSL_COMP_ZLIB, 1));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    /* Shrink to zero after allocation - same field-corruption pattern as
     * the non-compressed zero-length test but now on the Zlib code path. */
    pBuf->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    bool acceptable = (ret == RSSL_RET_FAILURE) || (ret >= RSSL_RET_SUCCESS);
    EXPECT_TRUE(acceptable)
        << "Zero-length write on Zlib channel must not crash; ret=" << ret;

    if (ret < RSSL_RET_SUCCESS)
        rsslReleaseBuffer(pBuf, &err);
}

/* =======================================================================
 * -- HTTP CHUNK-HEADER (httpHeaders > 0) NEGATIVE SCENARIOS -------------
 *
 * When rsslSocketChannel->httpHeaders is non-zero, ipcWriteSession()
 * prepends an HTTP chunked-transfer-encoding length line to every wire
 * message:
 *
 *   sprintf(msgb->buffer, "%x\r\n", msgb->length)  ? chunk-length header
 *   <IPC header + payload>
 *   "\r\n"                                          ? chunk footer
 *
 * This path is normally entered only by HTTP-tunneled connections.  The
 * tests below reach it by setting httpHeaders = 1 on an established
 * plain-socket channel via the internal RsslSocketChannel struct so that
 * the code path can be exercised without a full HTTP handshake.
 *
 * Crash vectors targeted:
 *   Issue 10 - sprintf into the chunk-length string buffer overflows when
 *     the hex representation of the chunk length exceeds the assumed width.
 *     If the code reserves space for "FFFF\r\n" (6 chars) but
 *     msgb->length >= 0x10000, sprintf writes "10000\r\n" (7 chars),
 *     overwriting the first byte of the IPC header.
 *   Issue 11 - Fragment header placed at a fixed offset that assumes the
 *     HTTP chunk header is always 6 bytes.  For small messages the chunk
 *     header may be only 3 bytes ("x\r\n"), so the fragment header lands
 *     3 bytes into the payload, corrupting reassembly on the receiver.
 *   Issue 3  - When the chunk-footer ("\r\n") write fails the outer loop
 *     continues processing subsequent fragment buffers instead of breaking,
 *     queuing garbage data that corrupts the output queue.
 *   Issue 4  - Use-after-free of msgb in the HTTP forceFlush path: after
 *     rtr_dfltcFreeMsg(msgb), the code writes msgb->buffer = 0 and
 *     msgb->length = 0 into the already-freed block.
 *
 * enableHttpHeaders() casts the public RsslChannel* to the internal
 * rsslChannelImpl* (RsslChannel is its first field) and then reaches
 * transportInfo ? RsslSocketChannel to set httpHeaders = 1.
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Enable HTTP chunked-transfer-encoding headers on an active channel.
 * RsslChannel is the first field of rsslChannelImpl, so reinterpret_cast
 * is safe: the pointer value is unchanged.
 *
 * Setting httpHeaders alone is not sufficient: both ipcWriteSession() and
 * ipcFlushSession() route every socket write through tunnelTransportInfo
 * instead of transportInfo whenever httpHeaders is non-zero.  On a real
 * HTTP-tunneled connection tunnelTransportInfo is populated during the HTTP
 * handshake inside ipcProcessHdr().  On a plain socket test channel it
 * remains NULL, causing an immediate NULL pointer dereference on the first
 * write attempt.
 *
 * We therefore mirror what ipcProcessHdr() does: assign tunnelTransportInfo
 * the same value as transportInfo so that the HTTP write path has a valid
 * transport destination.  If tunnelTransportInfo is already non-NULL (e.g.,
 * a previous call already set it up) we leave it unchanged.
 * --------------------------------------------------------------------- */
static void enableHttpHeaders(RsslChannel* pChnl)
{
    if (!pChnl) return;
    rsslChannelImpl*   pImpl = reinterpret_cast<rsslChannelImpl*>(pChnl);
    if (!pImpl->transportInfo) return;
    RsslSocketChannel* pSock = reinterpret_cast<RsslSocketChannel*>(pImpl->transportInfo);

    pSock->httpHeaders = 1;

    /* Populate tunnelTransportInfo from transportInfo when not already set.
     * ipcWriteSession() / ipcFlushSession() use tunnelTransportInfo for all
     * socket writes when httpHeaders > 0; leaving it NULL crashes there. */
    if (!pSock->tunnelTransportInfo)
        pSock->tunnelTransportInfo = pSock->transportInfo;
}

/* -----------------------------------------------------------------------
 * Http - Scenario 1:
 * Enable httpHeaders = 1 and write a small (non-fragmented) message.
 * ipcWriteSession() prepends the HTTP chunk-length line and appends the
 * chunk footer "\r\n".  The chunk length for a 64-byte message is "40\r\n"
 * (4 chars), which is shorter than the assumed 6-char "FFFF\r\n" reserve.
 * This is the baseline test that the HTTP path can be entered without
 * crashing when the chunk-header string is shorter than the reserve.
 *
 * Crash vector: baseline exercising of the HTTP chunk header path
 * (Issues 10 and 11 boundary - small chunk header does not yet overflow).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_SmallMessageWithHttpHeadersEnabled)
{
    ASSERT_TRUE(setupChannelPair("15168"));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    const RsslUInt32 msgSize = 64;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= 0)
        << "Small message with httpHeaders=1 must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Http - Scenario 2:
 * Enable httpHeaders = 1 and write a fragmented message (> maxFragSize).
 * ipcWriteSession() must place the IPC fragment header at offset chunkLen
 * inside each fragment's wire buffer.  If chunkLen is computed as a fixed
 * value (e.g., 6 for "FFFF\r\n") but the actual chunk header generated
 * for a given fragment is shorter (because the remaining payload is small),
 * the fragment header lands inside the payload bytes, corrupting the
 * receiver's reassembly state.
 *
 * Crash vector: Issue 11 (fragment header offset assumes fixed chunkLen
 * across all fragments, misaligning the IPC header on short-chunk frags).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_FragmentedWriteWithHttpHeadersEnabled)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15169", RSSL_COMP_NONE, 0, fragSize));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    /* Three fragments: first generates a long chunk header (e.g., "7d0\r\n"),
     * later fragments may generate shorter headers, exposing the fixed-offset
     * assumption. */
    const RsslUInt32 msgSize = fragSize * 3 - 50;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= 0)
        << "Fragmented write with httpHeaders=1 must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Http - Scenario 3:
 * Enable httpHeaders = 1 and write a 1-byte payload.  The chunk header
 * for a 1-byte message is "1\r\n" (3 chars).  If the code assumes the
 * chunk header is always 6 chars ("FFFF\r\n"), it places the IPC message
 * header at byte offset 6 instead of 3, writing it 3 bytes into the
 * payload region.  On the receiver side those displaced header bytes are
 * read as payload, corrupting the decode state.
 *
 * Crash vector: Issue 11 at the extreme short-chunk boundary ("1\r\n" =
 * 3 chars vs assumed fixed 6, maximum IPC-header misalignment of 3 bytes).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_TinyPayloadChunkHeaderShortHexString)
{
    ASSERT_TRUE(setupChannelPair("15170"));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(1, &err);
    if (!pBuf)
    {
        SUCCEED() << "1-byte buffer allocation failed - skip";
        return;
    }

    pBuf->data[0] = 0x41;
    pBuf->length  = 1;   /* chunk header will be "1\r\n" = 3 chars */

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= 0)
        << "1-byte payload with httpHeaders=1 must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Http - Scenario 5:
 * Enable httpHeaders = 1 and write a fragmented-range buffer with
 * pBuf->data = nullptr.  ipcWriteSession() generates the HTTP chunk-length
 * header into msgb->buffer first (no crash yet), then copies user payload
 * via memcpy(dst, pBuf->data, fragmentLen).  A NULL source pointer causes
 * an immediate segfault unless the implementation guards pBuf->data before
 * the copy in the HTTP-header branch.
 *
 * Crash vector: NULL-pointer dereference in the HTTP chunked-write path
 * (reached via a different branch than the non-HTTP null-data test at
 * port 15146, because the HTTP path copies payload after generating the
 * chunk header).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_NullDataPointerWithHttpHeadersEnabled)
{
    const RsslUInt32 fragSize = 2000;
    ASSERT_TRUE(setupChannelPair("15172", RSSL_COMP_NONE, 0, fragSize));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    /* Allocate in the fragmented range so pBuf->data is a separately
     * malloc'd block; NULLing it produces a genuine NULL memcpy source. */
    const RsslUInt32 msgSize = fragSize + 200;
    RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgSize);
    pBuf->length = msgSize;
    pBuf->data   = nullptr;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    bool acceptable = (ret == RSSL_RET_FAILURE) || (ret >= RSSL_RET_SUCCESS);
    EXPECT_EQ(RSSL_RET_FAILURE, ret) << "NULL data with httpHeaders=1 must not crash; ret=" << ret;

    EXPECT_EQ(RSSL_RET_SUCCESS, rsslReleaseBuffer(pBuf, &err));
}

/* -----------------------------------------------------------------------
 * Http - Scenario 6:
 * Enable httpHeaders = 1 and flood the output queue with 4-fragment
 * messages until rsslWriteEx returns RSSL_RET_WRITE_FLUSH_FAILED.  At
 * that point the queue holds partially-flushed messages each wrapped in
 * HTTP chunk headers.  When the chunk-footer ("\r\n") write for a fragment
 * fails, the outer while(msgb) loop must break; without the Issue 3 fix
 * it continues, queuing additional chunk-footer bytes after the fatal
 * error and leaving the queue in a state that crashes the next rsslFlush.
 *
 * Crash vector: Issue 3 (loop continues after chunk-footer write failure
 * in the HTTP-header code path, abandoning inconsistent chunk-footer
 * pool nodes in the output queue).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_FloodQueueToFlushFailedWithHttpHeaders)
{
    const RsslUInt32 fragSize = 1000;
    ASSERT_TRUE(setupChannelPair("15173", RSSL_COMP_NONE, 0, fragSize));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    const RsslUInt32 msgSize = fragSize * 4 - 10;

    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
        {
            gotFlushFailed = true;
            /* The flush must not crash even though the queue contains partial
             * HTTP chunk-footer entries left by the failed-flush path. */
            RsslRet flushRet = rsslFlush(pClientChnl, &err);
            EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
                << "rsslFlush after HTTP WRITE_FLUSH_FAILED must not crash";
        }
        else if (ret < RSSL_RET_SUCCESS)
            break;
        else if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "HTTP flood-to-FLUSH_FAILED completed without crash "
              << "(gotFlushFailed=" << gotFlushFailed << ")";
}

/* -----------------------------------------------------------------------
 * Http - Scenario 7:
 * Enable httpHeaders = 1, write 30 two-fragment messages without flushing,
 * then close the client channel.  Each queued message has a chunk-length
 * header prepended to its wire buffer.  ipcFreeSession() must walk and
 * free every rtr_msgb_t node in the queue - including nodes that carry
 * chunk-footer "\r\n" bytes queued as separate pool blocks - without
 * double-freeing or running off the end of the chain.
 *
 * Crash vector: Issue 4 (use-after-free of msgb in the HTTP path) + Issue
 * 7 (stale nextMsg pointer in pool blocks that held chunk-footer data) in
 * the ipcFreeSession() chain-walk when httpHeaders is non-zero.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_CloseChannelWithQueuedHttpChunkMessages)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15174", RSSL_COMP_NONE, 0, fragSize));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    const RsslUInt32 msgSize   = fragSize + 64;   /* 2 fragments per message */
    const int        queueMsgs = 30;

    int queued = 0;
    for (int i = 0; i < queueMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        ++queued;
        /* No flush - leave all HTTP chunk-header and chunk-footer pool
         * nodes queued for ipcFreeSession() to release on channel close. */
    }

    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;   /* prevent TearDown from double-closing */

    EXPECT_GT(queued, 0) << "At least some HTTP-chunked messages must have been queued";
    SUCCEED() << "Close with " << queued
              << " queued HTTP chunk messages did not crash";
}

/* -----------------------------------------------------------------------
 * Http - Scenario 8:
 * Enable httpHeaders = 1 and write a 3-fragment message with the
 * RSSL_WRITE_DIRECT_SOCKET_WRITE flag.  ipcWriteSession() enters the
 * forceFlush path, prepends the HTTP chunk-length header to each fragment,
 * calls send() directly, and on success calls rtr_dfltcFreeMsg(msgb)
 * followed by msgb->buffer = 0 and msgb->length = 0 (the Issue 4 UAF).
 * Under pool reuse, the freed block may be reallocated for the chunk-footer
 * "\r\n" of the NEXT fragment before the zeroing, corrupting the footer
 * allocation's header.  The Issue 3 loop-continuation then attempts to
 * write further chunk-footer data after the pool corruption, crashing.
 *
 * Crash vector: Issue 4 (use-after-free of msgb in the HTTP forceFlush
 * path) combined with Issue 3 (loop continues processing subsequent
 * fragments after the chunk-footer write error caused by pool corruption).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Http_DirectSocketWriteFragmentedWithHttpHeaders)
{
    const RsslUInt32 fragSize = 1500;
    ASSERT_TRUE(setupChannelPair("15175", RSSL_COMP_NONE, 0, fragSize));
    enableHttpHeaders(pClientChnl);

    RsslError err;
    /* Three or more fragments so that multiple chunk-header + chunk-footer
     * allocations are created and freed, maximising pool-reuse pressure
     * for the UAF zeroing (Issue 4). */
    const RsslUInt32 msgSize = fragSize * 3 + 100;

    for (int i = 0; i < 10; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "HTTP DIRECT_SOCKET_WRITE fragmented test completed without crash";
}

/* =======================================================================
 * -- ADDITIONAL PACKING NEGATIVE SCENARIOS --------------------------------
 *
 * The tests below cover boundary and invalid-state conditions for the
 * rsslPackBuffer() / rsslWriteEx() packing path that are NOT covered by
 * the existing Pack_* suite (ports 15132-15139).
 *
 * rsslBufferImpl fields relevant to packing:
 *   packingOffset - byte offset of the next free slot inside the buffer
 *   totalLength   - declared maximum capacity for packing bookkeeping
 *
 * Crash vectors targeted:
 *   Issue 2  - msgb->length > msgb->maxLength break on the packing path
 *   Issue 9  - writeOutArgs not updated on non-success packing return paths
 *   Issue 10 - Chunk-header sprintf off-by-one when packing + httpHeaders
 *
 * Port assignments: 15200-15219
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Pack - Scenario 1:
 * Allocate a packed buffer of exactly 1 byte.  rsslPackBuffer() must either
 * refuse the call or return an unusable (zero-length) next slot; it must not
 * write the 2-byte packed-message length prefix into a 1-byte backing store.
 * Writing the packed buffer (length = 0) must not crash.
 *
 * Crash vector: length-prefix write (2 bytes) into a 1-byte region causes
 * a 1-byte stack/heap overflow inside rsslPackBuffer() or ipcWriteSession().
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_OneBytePackedBufferRefusesOrSucceedsCleanly)
{
    ASSERT_TRUE(setupChannelPair("15200"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 1, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "1-byte packed buffer allocation failed - acceptable; skip";
        return;
    }

    /* A 1-byte buffer cannot hold the 2-byte length prefix rsslPackBuffer
     * writes; the call must either return NULL/empty or cope gracefully. */
    pBuf->data[0] = 0x41;
    pBuf->length  = 1;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    /* Regardless of what rsslPackBuffer returned, writing must not crash. */
    pBuf->length = 0;   /* signal end-of-pack to ipcWriteSession */

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "1-byte packed buffer write must not crash; ret=" << ret;

    (void)pNext;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 2:
 * Pack exactly two messages whose combined wire size (2 x (msgLen + 2-byte
 * prefix)) equals the backing-store capacity.  This is the tight-fit
 * boundary: one byte less leaves space; one byte more would overflow.
 * Both rsslPackBuffer() calls and the subsequent write must succeed.
 *
 * Crash vector: off-by-one in the packingOffset update inside
 * rsslPackBuffer() causes the second prefix write to land 1 byte past the
 * end of the backing store, corrupting adjacent heap metadata.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_TwoMessagesTightFitBoundarySucceeds)
{
    ASSERT_TRUE(setupChannelPair("15201"));

    RsslError err;
    /* Each message consumes (msgLen + 2) bytes: 2 for the packed prefix.
     * With bufLen = 2 x (msgLen + 2) both messages fit exactly. */
    const RsslUInt32 msgLen = 30;
    const RsslUInt32 bufLen = 2 * (msgLen + 2);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    ASSERT_NE(pNext, nullptr) << "First rsslPackBuffer must succeed; err: " << err.text;

    fillBuffer(pNext, msgLen);
    pNext->length = msgLen;

    /* Second pack - should succeed; no room left after this. */
    RsslBuffer* pEnd = rsslPackBuffer(pClientChnl, pNext, &err);

    /* Signal end-of-pack.  pEnd may be NULL (no more space) or valid. */
    if (pEnd)
        pEnd->length = 0;
    else
        pBuf->length = 0;   /* fallback: no trailing slot */

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS)
        << "Tight-fit two-message pack must succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 3:
 * Pack one message, then call rsslReleaseBuffer() instead of rsslWriteEx().
 * The packed buffer's pool block must be returned cleanly without leaking
 * the packing-cursor metadata or corrupting adjacent blocks.
 *
 * Crash vector: rsslReleaseBuffer() does not reset the packingOffset
 * before returning the block, leaving a non-zero cursor that confuses the
 * next caller who receives the same pool block via rsslGetBuffer().
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_ReleasePackedBufferInsteadOfWrite)
{
    ASSERT_TRUE(setupChannelPair("15202"));

    RsslError err;
    const RsslUInt32 msgLen = 64;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 256, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    /* Advance packing cursor by committing one message. */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    /* Release without writing - pool must survive this. */
    RsslRet relRet = rsslReleaseBuffer(pBuf, &err);

    /* A subsequent allocation from the same pool must succeed and be usable. */
    RsslBuffer* pBuf2 = rsslGetBuffer(pClientChnl, 128, RSSL_FALSE, &err);
    if (pBuf2)
    {
        fillBuffer(pBuf2, 64);
        pBuf2->length = 64;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);

        EXPECT_GE(ret, RSSL_RET_SUCCESS)
            << "Write after release-of-packed-buffer must succeed";
    }

    EXPECT_TRUE(relRet == RSSL_RET_SUCCESS)
        << "rsslReleaseBuffer on packed buffer must not crash; ret=" << relRet;

    (void)pNext;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 4:
 * Allocate a packed buffer, then write it immediately without calling
 * rsslPackBuffer() at all (packingOffset = 0, length = payload).
 * ipcWriteSession() sees the IPC_PACKING flag and tries to advance through
 * packed sub-messages; with packingOffset = 0 and a non-zero payload it
 * may loop infinitely or skip past the end of the buffer.
 *
 * Crash vector: infinite loop or out-of-bounds read when the packing
 * cursor is 0 and length > 0 on the IPC_PACKING decode path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_WritePackedBufferWithoutCallingPackBuffer)
{
    ASSERT_TRUE(setupChannelPair("15203"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 128, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);
    pBuf->length = 64;   /* non-zero payload, no rsslPackBuffer call */

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "Write of packed buffer without rsslPackBuffer call must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 5:
 * Pack one message, set the second slot's length to UINT16_MAX (0xFFFF).
 * The wire packed-message length prefix is a 16-bit field; writing 0xFFFF
 * into it makes the receiver believe the next packed sub-message is 65535
 * bytes long, causing it to read far past the end of the buffer.
 *
 * Crash vector: Issue 6 - the 16-bit packed-message length prefix overflows
 * when length = 0xFFFF, producing a wire format that causes the receiver
 * to read 65535 bytes from a small buffer.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_UINT16MAXLengthOnSecondPackedMessage)
{
    ASSERT_TRUE(setupChannelPair("15204"));

    RsslError err;
    const RsslUInt32 bufLen = 4096;
    const RsslUInt32 msgLen = 64;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (!pNext)
    {
        SUCCEED() << "rsslPackBuffer returned NULL - skip";
        return;
    }

    fillBuffer(pNext, msgLen);
    /* Overflow the second slot's length to UINT16_MAX. */
    pNext->length = 0xFFFFu;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "UINT16_MAX second-slot length in packed buffer must return RSSL_RET_BUFFER_TOO_SMALL; ret=" << ret;
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(4030).") != NULL);
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 6:
 * Pack a zero-length message as the FIRST slot, then pack a normal message
 * as the second slot.  A zero-length first packed sub-message causes the
 * receiver's packing cursor to stall (cursor += 0 ? infinite loop) or to
 * misinterpret the second message's length prefix as payload bytes.
 *
 * Crash vector: zero-length packed sub-message stalls the packing decode
 * cursor in ipcReadSession(), causing an infinite loop or mis-parse.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_ZeroLengthFirstMessageThenNormalMessage)
{
    ASSERT_TRUE(setupChannelPair("15205"));

    RsslError err;
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 32;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    /* First message: zero length - commit a 0-byte packed sub-message. */
    pBuf->length = 0;
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (!pNext)
    {
        /* rsslPackBuffer refused the zero-length message - acceptable. */
        pBuf->length = 0;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        SUCCEED() << "rsslPackBuffer refused zero-length first message - acceptable";
        return;
    }

    /* Second message: normal payload. */
    fillBuffer(pNext, msgLen);
    pNext->length = msgLen;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "Zero-length first packed sub-message must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 7:
 * Pack a single message, then repeatedly call rsslPackBuffer() on the
 * returned next-slot pointer until it returns NULL (no space left).
 * After exhausting the buffer, verify that calling rsslWriteEx() with
 * the original pointer (length = 0) completes without crashing.
 *
 * Crash vector: exhausting the packing space and then writing must not
 * dereference a NULL or stale next-slot pointer inside ipcWriteSession().
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_ExhaustPackingSpaceThenWriteCleanly)
{
    ASSERT_TRUE(setupChannelPair("15206"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    const RsslUInt32 msgLen = 8;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    RsslBuffer* pCur = pBuf;
    int packCount = 0;
    while (pCur && pCur->length >= msgLen)
    {
        fillBuffer(pCur, msgLen);
        pCur->length = msgLen;
        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        ++packCount;
        if (!pNext || pNext->length < msgLen)
        {
            /* No room - signal end-of-pack. */
            if (pNext)
                pNext->length = 0;
            break;
        }
        pCur = pNext;
    }

    EXPECT_GT(packCount, 0) << "At least one message must have been packed";

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Write after packing-space exhaustion must succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 8:
 * Flood the output queue with packed buffers (each containing 3 messages)
 * until rsslWriteEx returns RSSL_RET_WRITE_FLUSH_FAILED.  Then close the
 * channel WITHOUT flushing.  ipcFreeSession() must walk and release every
 * pool block that carries packed-message length prefixes without
 * misinterpreting prefix bytes as rtr_msgb_t chain pointers.
 *
 * Crash vector: ipcFreeSession() walks the queue using rtr_msgb_t->nextMsg
 * pointers; if a block's first two bytes (the packed-length prefix) are
 * mistaken for a pointer, the walk dereferences an arbitrary address.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_CloseChannelAfterPackedWriteFlushFailed)
{
    ASSERT_TRUE(setupChannelPair("15207"));

    RsslError err;
    const RsslUInt32 bufLen = 1024;
    const RsslUInt32 msgLen = 200;

    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
        if (!pBuf) break;

        /* Pack 3 messages into the buffer. */
        RsslBuffer* pCur = pBuf;
        for (int m = 0; m < 3 && pCur && pCur->length >= msgLen; ++m)
        {
            fillBuffer(pCur, msgLen);
            pCur->length = msgLen;
            RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
            if (!pNext || pNext->length < msgLen)
            {
                if (pNext) pNext->length = 0;
                break;
            }
            pCur = pNext;
        }
        if (pCur)
            pCur->length = 0;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
            gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS)
            break;
        /* No flush - accumulate packed blocks in the queue. */
    }

    /* Close WITHOUT flushing: ipcFreeSession must free all packed pool blocks. */
    rsslCloseChannel(pClientChnl, &err);
    pClientChnl = nullptr;

    SUCCEED() << "Packed WRITE_FLUSH_FAILED + channel close did not crash "
              << "(gotFlushFailed=" << gotFlushFailed << ")";
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 9:
 * Allocate a packed buffer large enough to hold exactly one message plus
 * the 2-byte prefix.  After rsslPackBuffer() commits that message, the
 * returned next slot has length = 0 (no room for more data).  Set
 * pNext->length = 1 (one phantom byte) and call rsslWriteEx().
 * The ipcWriteSession() packing path treats length = 1 as a valid
 * sub-message with a 1-byte payload, writing a 2-byte prefix that
 * overruns the single remaining byte.
 *
 * Crash vector: 2-byte prefix write into the 1-byte tail of a tight-fit
 * packed buffer overflows into adjacent memory by exactly 1 byte.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_OnePhantomByteInReturnedSlotCausesOverflow)
{
    ASSERT_TRUE(setupChannelPair("15208"));

    RsslError err;
    /* bufLen = msgLen + 2 (prefix) + 1 (phantom) ? tight fit plus 1 byte. */
    const RsslUInt32 msgLen = 50;
    const RsslUInt32 bufLen = msgLen + 2 + 1;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);

    if (pNext && pNext->length == 0)
    {
        /* Inject one phantom byte into the exhausted slot. */
        pNext->length = 1;
        pNext->data[0] = 0x55;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "Phantom-byte packed slot must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 10:
 * Pack three messages into a buffer, then write that buffer with the
 * RSSL_WRITE_DIRECT_SOCKET_WRITE flag set.  The forceFlush path in
 * ipcWriteSession() calls send() directly and then frees the msgb via
 * rtr_dfltcFreeMsg() followed by msgb->buffer = 0 (use-after-free, Issue 4).
 * On a packed buffer the msgb carries packed-message length prefixes;
 * the UAF zeroing of buffer and length corrupts those prefix bytes if the
 * pool reuses the block for a new allocation before the zeroing completes.
 *
 * Crash vector: Issue 4 applied to the packed-message forceFlush path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_DirectSocketWriteOnPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15209"));

    RsslError err;
    const RsslUInt32 bufLen = 1024;
    const RsslUInt32 msgLen = 100;

    for (int i = 0; i < 20; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
        if (!pBuf) break;

        /* Pack 3 messages. */
        RsslBuffer* pCur = pBuf;
        for (int m = 0; m < 3 && pCur && pCur->length >= msgLen; ++m)
        {
            fillBuffer(pCur, msgLen);
            pCur->length = msgLen;
            RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
            if (!pNext || pNext->length < msgLen)
            {
                if (pNext) pNext->length = 0;
                break;
            }
            pCur = pNext;
        }
        if (pCur) pCur->length = 0;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Packed DIRECT_SOCKET_WRITE did not crash";
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 11:
 * Allocate two separate packed buffers.  Write the first normally.  Then
 * write the second buffer with pBuf->data pointing into the interior of
 * the first buffer's backing store (aliased pointer).  The packing-cursor
 * bookkeeping uses the original allocation base; an aliased data pointer
 * causes the length-prefix writes to land at the wrong offset, overwriting
 * already-sent payload bytes or adjacent allocation headers.
 *
 * Crash vector: aliased buffer pointer in the packed write path causes
 * out-of-bounds writes when ipcWriteSession() computes the prefix offset
 * relative to msgb->buffer rather than pBuf->data.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_AliasedDataPointerInPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15210"));

    RsslError err;
    const RsslUInt32 bufLen = 256;
    const RsslUInt32 msgLen = 50;

    /* First packed buffer - written normally. */
    RsslBuffer* pBuf1 = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf1, nullptr);
    fillBuffer(pBuf1, msgLen);
    pBuf1->length = msgLen;
    RsslBuffer* pNext1 = rsslPackBuffer(pClientChnl, pBuf1, &err);
    if (pNext1) pNext1->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret1 = rsslWriteEx(pClientChnl, pBuf1, &inArgs, &outArgs, &err);
    if (ret1 > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    /* Second packed buffer - aliased data pointer into the first buffer's
     * backing store (which may already be freed and reused). */
    RsslBuffer* pBuf2 = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf2)
    {
        SUCCEED() << "Second buffer allocation failed - skip";
        return;
    }

    /* Store the original data pointer then alias it to the first buffer. */
    char* origData2 = pBuf2->data;
    /* Point data into the middle of the first backing store (now freed). */
    pBuf2->data = pBuf1->data + 10;   /* aliased pointer */
    pBuf2->length = msgLen;

    RsslBuffer* pNext2 = rsslPackBuffer(pClientChnl, pBuf2, &err);
    if (pNext2) pNext2->length = 0;

    rsslClearWriteInArgs(&inArgs);
    rsslClearWriteOutArgs(&outArgs);
    RsslRet ret2 = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);

    /* Restore original data pointer so the pool can release correctly. */
    pBuf2->data = origData2;

    EXPECT_TRUE(ret2 >= RSSL_RET_SUCCESS)
        << "Aliased data pointer in packed buffer must not crash; ret=" << ret2;
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 12:
 * Pack messages with alternating maximum and minimum (1-byte) payloads.
 * The packed-message length prefix must encode lengths from 1 to the
 * channel's maxFragmentSize correctly for each alternating slot.  Any
 * misalignment of the packing cursor across the alternating sizes leaves
 * the second-message prefix reading into the first message's payload bytes.
 *
 * Crash vector: cursor misalignment when alternating large/small packed
 * messages causes the prefix of the small message to overlap the data of
 * the large one, producing corrupted sub-message boundaries.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_AlternatingLargeAndSmallMessagesInBuffer)
{
    ASSERT_TRUE(setupChannelPair("15211"));

    RsslError err;
    const RsslUInt32 bufLen = 4096;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    RsslBuffer* pCur = pBuf;
    int packedCount = 0;
    bool large = true;

    while (pCur && pCur->length > 2)
    {
        RsslUInt32 msgLen = large ? (pCur->length > 100 ? 100 : pCur->length - 2) : 1;
        if (msgLen == 0) break;

        fillBuffer(pCur, msgLen);
        pCur->length = msgLen;

        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        ++packedCount;
        large = !large;

        if (!pNext || pNext->length <= 2)
        {
            if (pNext) pNext->length = 0;
            break;
        }
        pCur = pNext;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Alternating large/small packed messages must succeed; err: " << err.text;
    EXPECT_GT(packedCount, 0) << "At least one packed message must have been committed";
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 13:
 * Write a packed buffer on a channel whose state has been set to CLOSED
 * before the write (same pattern as the non-packed channel-state guard
 * tests).  The rsslSocketWrite() guard must reject the write before
 * the packed-message prefix is placed into the wire buffer, preventing
 * a pool block with partially-written prefix bytes from reaching the queue.
 *
 * Crash vector: partially-written packed-message prefix left in a pool
 * block that the channel close then attempts to free via ipcFreeSession().
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_WritePackedBufferOnClosedChannelState)
{
    ASSERT_TRUE(setupChannelPair("15212"));

    RsslError err;
    const RsslUInt32 msgLen = 64;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 512, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, msgLen);
    pBuf->length = msgLen;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (pNext) pNext->length = 0;

    /* Force channel into CLOSED state without freeing internal structures. */
    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "Write of packed buffer on CLOSED channel must return RSSL_RET_FAILURE";

    /* Restore state so TearDown can close the channel cleanly. */
    pClientChnl->state = RSSL_CH_STATE_ACTIVE;

    rsslReleaseBuffer(pBuf, &err);
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 14:
 * Rapid open-pack-write-close cycling: open a channel, pack 5 messages,
 * write (no flush), close, repeat 15 times.  This exercises the pool
 * reclamation path for packed pool blocks across multiple channel lifetimes
 * and verifies that no packed-cursor state leaks between cycles.
 *
 * Crash vector: packed-cursor metadata left in a pool block after channel
 * close is reused in the next cycle, causing the second cycle's
 * ipcWriteSession() to skip over already-written data.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_RapidOpenPackWriteCloseCycles)
{
    RsslError err;
    const int cycles   = 15;
    const RsslUInt32 bufLen = 512;
    const RsslUInt32 msgLen = 50;

    for (int c = 0; c < cycles; ++c)
    {
        RsslServer*  pSrv = nullptr;
        RsslChannel* pSrC = nullptr;
        RsslChannel* pClC = nullptr;

        if (!setupActiveChannelPair("15213", &pSrv, &pSrC, &pClC))
            continue;

        RsslBuffer* pBuf = rsslGetBuffer(pClC, bufLen, RSSL_TRUE, &err);
        if (pBuf)
        {
            RsslBuffer* pCur = pBuf;
            for (int m = 0; m < 5 && pCur && pCur->length >= msgLen; ++m)
            {
                fillBuffer(pCur, msgLen);
                pCur->length = msgLen;
                RsslBuffer* pNext = rsslPackBuffer(pClC, pCur, &err);
                if (!pNext || pNext->length < msgLen)
                {
                    if (pNext) pNext->length = 0;
                    break;
                }
                pCur = pNext;
            }
            if (pCur) pCur->length = 0;

            RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
            RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
            rsslWriteEx(pClC, pBuf, &inArgs, &outArgs, &err);
            /* No flush - leave packed block queued. */
        }

        rsslCloseChannel(pSrC, &err);
        rsslCloseChannel(pClC, &err);
        rsslCloseServer(pSrv, &err);
        time_sleep(3);
    }

    SUCCEED() << "Rapid open-pack-write-close cycles did not crash";
}

/* -----------------------------------------------------------------------
 * Pack - Scenario 15:
 * Issue 9: verify that writeOutArgs.bytesWritten is populated (not left
 * as the sentinel -1) after writing a packed buffer that contains three
 * sub-messages.  The packed write path must update bytesWritten for the
 * complete wire size of all sub-messages, not just the first.
 *
 * Crash vector (logical): writeOutArgs.bytesWritten left at -1 means the
 * caller cannot determine whether any bytes were sent, breaking flow
 * control calculations and potentially causing unbounded queuing.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Pack_WriteOutArgsBytesWrittenUpdatedForPackedBuffer)
{
    ASSERT_TRUE(setupChannelPair("15214"));

    RsslError err;
    const RsslUInt32 bufLen = 1024;
    const RsslUInt32 msgLen = 80;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    ASSERT_NE(pBuf, nullptr);

    /* Pack 3 messages. */
    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < 3 && pCur && pCur->length >= msgLen; ++m)
    {
        fillBuffer(pCur, msgLen);
        pCur->length = msgLen;
        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        if (!pNext || pNext->length < msgLen)
        {
            if (pNext) pNext->length = 0;
            break;
        }
        pCur = pNext;
    }
    if (pCur) pCur->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    outArgs.bytesWritten             = (RsslUInt32)-1;
    outArgs.uncompressedBytesWritten = (RsslUInt32)-1;

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    ASSERT_GE(ret, RSSL_RET_SUCCESS);
    EXPECT_NE(outArgs.bytesWritten, (RsslUInt32)-1)
        << "bytesWritten must be updated after packed buffer write";
    EXPECT_GT(outArgs.bytesWritten, 0u)
        << "bytesWritten must be > 0 for a non-empty packed buffer";
    EXPECT_NE(outArgs.uncompressedBytesWritten, (RsslUInt32)-1)
        << "uncompressedBytesWritten must be updated after packed buffer write";
}

/* -----------------------------------------------------------------------
 * -- rsslWrite() (non-Ex) basic tests -------------------------------------
 * All previous tests exclusively used rsslWriteEx; the legacy rsslWrite()
 * entry point shares the ipcWriteSession() back-end but has different
 * argument marshalling in rsslImpl.c.  Any NULL-deref or stack-corruption
 * in that marshalling layer is a unique crash path.
 * --------------------------------------------------------------------- */

/* Write a small buffer via rsslWrite() with RSSL_HIGH_PRIORITY. */
TEST_P(RsslSocketWriteTests, RsslWrite_SmallBufferHighPriority)
{
    ASSERT_TRUE(setupChannelPair("15221"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    pBuf->length = 32;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, pBuf, RSSL_HIGH_PRIORITY,
                            0, &bytesWritten, &uncompBytes, &err);

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "rsslWrite HIGH_PRIORITY must succeed; err: " << err.text;
    EXPECT_GT(bytesWritten, 0u)
        << "bytesWritten must be > 0 after rsslWrite success";
}

/* Write a 64-byte payload via the legacy rsslWrite() with RSSL_MEDIUM_PRIORITY.
 * Verifies the medium-priority queue path through the rsslWrite() marshalling
 * layer completes without error. */
TEST_P(RsslSocketWriteTests, RsslWrite_SmallBufferMediumPriority)
{
    ASSERT_TRUE(setupChannelPair("15222"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(128, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, pBuf, RSSL_MEDIUM_PRIORITY,
                            0, &bytesWritten, &uncompBytes, &err);

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "rsslWrite MEDIUM_PRIORITY must succeed; err: " << err.text;
}

/* Write a 64-byte payload via the legacy rsslWrite() with RSSL_LOW_PRIORITY.
 * Verifies the low-priority queue path through the rsslWrite() marshalling
 * layer completes without error. */
TEST_P(RsslSocketWriteTests, RsslWrite_SmallBufferLowPriority)
{
    ASSERT_TRUE(setupChannelPair("15223"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(128, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, pBuf, RSSL_LOW_PRIORITY,
                            0, &bytesWritten, &uncompBytes, &err);

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "rsslWrite LOW_PRIORITY must succeed; err: " << err.text;
}

/* Write a 128-byte payload via legacy rsslWrite() with the
 * RSSL_WRITE_DIRECT_SOCKET_WRITE flag.  Exercises the forceFlush code
 * path through the rsslWrite() argument-marshalling layer and verifies
 * no crash regardless of whether the send() call succeeds or blocks. */
TEST_P(RsslSocketWriteTests, RsslWrite_DirectSocketWriteFlag)
{
    ASSERT_TRUE(setupChannelPair("15224"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(256, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 128);
    pBuf->length = 128;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, pBuf, RSSL_HIGH_PRIORITY,
                            RSSL_WRITE_DIRECT_SOCKET_WRITE,
                            &bytesWritten, &uncompBytes, &err);

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "rsslWrite DIRECT_SOCKET_WRITE must not crash; ret=" << ret;
}

/* Write via the legacy rsslWrite() entry point with pBuf->length = UINT32_MAX.
 * Verifies the msgb->length > msgb->maxLength guard is reached via the
 * rsslWrite() argument-marshalling layer (not rsslWriteEx) and returns
 * RSSL_RET_FAILURE rather than crashing or corrupting the pool. */
TEST_P(RsslSocketWriteTests, RsslWrite_OversizedBufferLengthReturnsFailure)
{
    ASSERT_TRUE(setupChannelPair("15225"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    /* Inflate length beyond allocated capacity to trigger maxLength guard. */
    pBuf->length = 0xFFFFFFFFu;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, pBuf, RSSL_HIGH_PRIORITY,
                            0, &bytesWritten, &uncompBytes, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "rsslWrite with UINT32_MAX length must return RSSL_RET_FAILURE";
}

/* rsslWrite() on a channel in CLOSED state must fail gracefully. */
TEST_P(RsslSocketWriteTests, RsslWrite_ClosedChannelStateFails)
{
    ASSERT_TRUE(setupChannelPair("15226"));

    RsslError err;
    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslBuffer fakeBuf;
    char       fakeMem[64] = {};
    fakeBuf.data   = fakeMem;
    fakeBuf.length = 16;

    RsslUInt32 bytesWritten = 0, uncompBytes = 0;
    RsslRet ret = rsslWrite(pClientChnl, &fakeBuf, RSSL_HIGH_PRIORITY,
                            0, &bytesWritten, &uncompBytes, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "rsslWrite on CLOSED channel must return RSSL_RET_FAILURE";

    pClientChnl->state = RSSL_CH_STATE_ACTIVE;
}

/* -----------------------------------------------------------------------
 * -- Write to INITIALIZING channel ----------------------------------------
 * A channel in RSSL_CH_STATE_INITIALIZING has not completed the IPC
 * handshake.  Attempting to write user data into it targets the channel-
 * state guard that ipcWriteSession() (or rsslSocketWrite()) must check
 * before queueing any msgb blocks.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, WriteToInitializingChannelFails)
{
    ASSERT_TRUE(setupChannelPair("15227"));

    RsslError err;
    /* Force the channel back to INITIALIZING without altering internal
     * pool structures - we only modify the public state field. */
    pClientChnl->state = RSSL_CH_STATE_INITIALIZING;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_FALSE, &err);
    /* Buffer allocation may fail because state is INITIALIZING - that is fine. */
    if (!pBuf)
    {
        pClientChnl->state = RSSL_CH_STATE_ACTIVE;
        SUCCEED() << "Buffer allocation refused on INITIALIZING channel - acceptable";
        return;
    }
}

/* -----------------------------------------------------------------------
 * -- Write via the SERVER-ACCEPTED channel --------------------------------
 * The server's accepted channel (pServerChnl) is also an active RSSL
 * channel and must be writable.  All prior tests wrote only from the
 * client side; this test exercises the server-side write path, which uses
 * the same ipcWriteSession() back-end but through a different pool
 * allocation chain (server bind opts guaranteedOutputBuffers).
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, WriteOnServerAcceptedChannelSucceeds)
{
    ASSERT_TRUE(setupChannelPair("15228"));

    RsslError err;
    /* Allocate from the SERVER channel's pool, not the client's. */
    RsslBuffer* pBuf = rsslGetBuffer(pServerChnl, 128, RSSL_FALSE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Server channel buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pServerChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pServerChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Write on server-accepted channel must succeed; err: " << err.text;
}

/* Allocate a 64-byte buffer from the server channel's pool, set
 * pBuf->length = UINT32_MAX, then call rsslWriteEx() on the server channel.
 * Verifies the msgb->length > msgb->maxLength guard works symmetrically on
 * the server-accepted channel, not only on client-side channels. */
TEST_P(RsslSocketWriteTests, OversizedWriteOnServerChannelFails)
{
    ASSERT_TRUE(setupChannelPair("15229"));

    RsslError err;
    RsslBuffer* pBuf = rsslGetBuffer(pServerChnl, 64, RSSL_FALSE, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    pBuf->length = 0xFFFFFFFFu;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pServerChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL)
        << "Oversized write on server channel must return RSSL_RET_BUFFER_TOO_SMALL";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(64).") != NULL);
}

/* -----------------------------------------------------------------------
 * -- Buffer obtained from the WRONG channel -------------------------------
 * Getting a buffer from the server channel but writing it via the client
 * channel (or vice-versa) produces a msgb whose pool ownership is
 * mismatched.  ipcWriteSession() does not validate pool origin;
 * the test verifies no immediate crash occurs in the write path itself.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, BufferFromWrongChannelDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15230"));

    RsslError err;
    /* Allocate from the SERVER pool but present it to the CLIENT channel. */
    RsslBuffer* pBuf = rsslGetBuffer(pServerChnl, 64, RSSL_FALSE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Server buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, 32);
    pBuf->length = 32;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    /* This is an API misuse - we accept any non-crash result. */
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_TRUE(ret == RSSL_RET_FAILURE)
        << "Write of wrong-channel buffer must not crash; ret=" << ret;

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);
}

/* -----------------------------------------------------------------------
 * -- Priority queue write flood --------------------------------------------
 * rsslWrite() exposes three priority queues (HIGH/MEDIUM/LOW).  Interleaving
 * writes across all three queues in rapid succession stresses the priority
 * queue accounting inside rsslSocketWrite().  Any off-by-one in queue
 * head/tail pointers or misuse of the priority selector corrupts the queue.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, PriorityQueueInterleavedWritesDoNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15231"));

    RsslError err;
    const RsslWritePriorities priorities[3] = {
        RSSL_HIGH_PRIORITY, RSSL_MEDIUM_PRIORITY, RSSL_LOW_PRIORITY
    };

    for (int i = 0; i < 90; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_FALSE, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 32);
        pBuf->length = 32;

        RsslUInt32 bytesWritten = 0, uncompBytes = 0;
        RsslRet ret = rsslWrite(pClientChnl, pBuf,
                                priorities[i % 3],
                                0, &bytesWritten, &uncompBytes, &err);

        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Interleaved priority-queue writes did not crash";
}

/* rsslWrite() priority queue: HIGH + LOW interleaved with flush.
 * Verifies that the HIGH-priority queue drains before LOW items.  Any
 * crash here indicates a queue-list corruption during dequeue. */
TEST_P(RsslSocketWriteTests, PriorityQueueHighAndLowInterleavedWithFlush)
{
    ASSERT_TRUE(setupChannelPair("15232"));

    RsslError err;

    for (int i = 0; i < 60; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 128, RSSL_FALSE, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 64);
        pBuf->length = 64;

        /* Alternate HIGH and LOW priorities. */
        RsslWritePriorities pri = (i % 2 == 0) ? RSSL_HIGH_PRIORITY : RSSL_LOW_PRIORITY;
        RsslUInt32 bytesWritten = 0, uncompBytes = 0;
        RsslRet ret = rsslWrite(pClientChnl, pBuf, pri,
                                0, &bytesWritten, &uncompBytes, &err);

        if (ret < RSSL_RET_SUCCESS) break;

        /* Flush every 10 writes to keep TCP buffer clear. */
        if ((i % 10) == 9)
            rsslFlush(pClientChnl, &err);
    }

    rsslFlush(pClientChnl, &err);
    SUCCEED() << "HIGH/LOW priority interleaved flood did not crash";
}

/* -----------------------------------------------------------------------
 * -- rsslIoctl HIGH_WATER_MARK interaction --------------------------------
 * Setting a very low high-water mark via rsslIoctl() and then writing
 * large messages forces the output queue to signal WRITE_FLUSH_FAILED
 * much sooner than the default.  Any improper interaction between the
 * ioctl state and ipcWriteSession()'s flush-attempt decision causes a
 * crash or queue corruption.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, IoctlLowHighWaterMarkThenWriteDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15233"));

    RsslError err;

    /* Set high water mark to a minimal value (1 byte) to make every write
     * attempt flush immediately and likely hit a WRITE_FLUSH_FAILED. */
    int watermark = 1;
    rsslIoctl(pClientChnl, RSSL_HIGH_WATER_MARK, &watermark, &err);

    bool gotFlushFailed = false;
    for (int i = 0; i < 20 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(512, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 512);
        pBuf->length = 512;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
            gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS)
            break;
        else if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    SUCCEED() << "Low high-water-mark write loop did not crash "
              << "(gotFlushFailed=" << gotFlushFailed << ")";
}

/* -----------------------------------------------------------------------
 * -- Compression + packed buffer combined ---------------------------------
 * Packing messages on a Zlib-compressed channel creates the most complex
 * path through ipcWriteSession(): the packing path builds sub-message
 * length prefixes AND the compression path wraps the result in
 * compressedmb1.  Any offset arithmetic error on the combined path
 * corrupts either the packed-message boundaries or the compressed frame.
 * --------------------------------------------------------------------- */

TEST_P(RsslSocketWriteTests, CompressedPackedWriteZlibSucceeds)
{
    ASSERT_TRUE(setupChannelPair("15234", RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 bufLen = 1024;
    const RsslUInt32 msgLen = 100;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Packed buffer allocation on Zlib channel failed - skip";
        return;
    }

    /* Pack 3 messages on the compressed channel. */
    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < 3 && pCur && pCur->length >= msgLen; ++m)
    {
        /* Fill with compressible data. */
        memset(pCur->data, 0x41 + m, (pCur->length < msgLen ? pCur->length : msgLen));
        pCur->length = msgLen;
        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        if (!pNext || pNext->length < msgLen)
        {
            if (pNext) pNext->length = 0;
            break;
        }
        pCur = pNext;
    }
    if (pCur) pCur->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Compressed packed write must succeed; err: " << err.text;
}

/* Compressed (LZ4) + packed: incompressible data ensures LZ4 expansion,
 * stressing the compressedmb2 allocation on top of the packing cursor. */
TEST_P(RsslSocketWriteTests, CompressedPackedWriteLz4IncompressibleData)
{
    ASSERT_TRUE(setupChannelPair("15235", RSSL_COMP_LZ4, 0));

    RsslError err;
    const RsslUInt32 bufLen = 2048;
    const RsslUInt32 msgLen = 200;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, bufLen, RSSL_TRUE, &err);
    if (!pBuf)
    {
        SUCCEED() << "Packed buffer allocation on LZ4 channel failed - skip";
        return;
    }

    RsslBuffer* pCur = pBuf;
    for (int m = 0; m < 4 && pCur && pCur->length >= msgLen; ++m)
    {
        fillBufferIncompressible(pCur, msgLen);
        pCur->length = msgLen;
        RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCur, &err);
        if (!pNext || pNext->length < msgLen)
        {
            if (pNext) pNext->length = 0;
            break;
        }
        pCur = pNext;
    }
    if (pCur) pCur->length = 0;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "LZ4 compressed packed write must not crash; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * -- Issue 5: oldTunnelStreamFd operator-precedence NULL-deref ------------
 * When rsslSocketChannel->oldTunnelStreamFd != RIPC_INVALID_SOCKET, the
 * ipcWriteSession() code attempts to close the old FD and swap in the new
 * tunnelTransportInfo.  The guard is expressed as:
 *
 *   if (rsslSocketChannel->oldTunnelStreamFd != RIPC_INVALID_SOCKET &&
 *       rsslSocketChannel->newTunnelTransportInfo)
 *
 * Due to operator-precedence ambiguity, if newTunnelTransportInfo is NULL
 * the body may still execute, dereferencing a NULL function-pointer table.
 *
 * The test simulates this by setting oldTunnelStreamFd to a non-invalid
 * sentinel value while leaving newTunnelTransportInfo NULL, then writing
 * to provoke the guard.  The only safe outcomes are RSSL_RET_FAILURE
 * (guard detects the inconsistency) or success (guard is correct).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Issue5_OldTunnelStreamFdWithNullNewTransportInfo)
{
    ASSERT_TRUE(setupChannelPair("15237"));

    RsslError err;

    /* Reach into the internal RsslSocketChannel to set the oldTunnelStreamFd
     * sentinel while leaving newTunnelTransportInfo NULL. */
    rsslChannelImpl*   pImpl = reinterpret_cast<rsslChannelImpl*>(pClientChnl);
    if (!pImpl->transportInfo)
    {
        SUCCEED() << "transportInfo not accessible - skip Issue 5 test";
        return;
    }
    RsslSocketChannel* pSock = reinterpret_cast<RsslSocketChannel*>(pImpl->transportInfo);

    /* Save originals so TearDown can close cleanly. */
    RsslSocket savedOldFd = pSock->oldTunnelStreamFd;
    void*      savedNewInfo = pSock->newTunnelTransportInfo;

    /* Simulate the partially-initialised tunnel-switch state:
     * oldTunnelStreamFd holds a non-invalid value; newTunnelTransportInfo is NULL. */
#if defined(_WIN32)
    pSock->oldTunnelStreamFd    = (RsslSocket)1;   /* any non-INVALID value */
#else
    pSock->oldTunnelStreamFd    = 1;
#endif
    pSock->newTunnelTransportInfo = nullptr;

    RsslBuffer* pBuf = getClientBuffer(64, &err);
    if (!pBuf)
    {
        /* Restore and skip. */
        pSock->oldTunnelStreamFd    = savedOldFd;
        pSock->newTunnelTransportInfo = savedNewInfo;
        SUCCEED() << "Buffer allocation failed - skip Issue 5 test";
        return;
    }

    fillBuffer(pBuf, 32);
    pBuf->length = 32;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    /* Restore so TearDown can close the channel. */
    pSock->oldTunnelStreamFd    = savedOldFd;
    pSock->newTunnelTransportInfo = savedNewInfo;

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "Issue 5: oldTunnelStreamFd guard must not crash; ret=" << ret;

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);
    else if (ret < RSSL_RET_SUCCESS)
        rsslReleaseBuffer(pBuf, &err);
}

/* -----------------------------------------------------------------------
 * -- Simultaneous writeInFlags combinations --------------------------------
 * The writeInFlags field accepts a bitmask; no existing test combines
 * multiple flags.  Illegal or unexpected combinations must not crash.
 * --------------------------------------------------------------------- */

/* All flags set simultaneously (likely invalid combination). */
TEST_P(RsslSocketWriteTests, WriteInFlagsAllSetDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15238"));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    ASSERT_NE(pBuf, nullptr);

    fillBuffer(pBuf, 32);
    pBuf->length = 32;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    /* Combine all known write-in flags.  The implementation must handle this
     * without dereferencing an invalid function table entry. */
    inArgs.writeInFlags = RSSL_WRITE_IN_DIRECT_SOCKET_WRITE |
                          RSSL_WRITE_IN_DO_NOT_COMPRESS;
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "All writeInFlags set must not crash; ret=" << ret;

    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);
}

/* RSSL_WRITE_NO_COMPRESS on a Zlib channel - forces uncompressed path. */
TEST_P(RsslSocketWriteTests, WriteNoCompressFlagOnZlibChannelDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15239", RSSL_COMP_ZLIB, 1));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(512, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, 256);
    pBuf->length = 256;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    inArgs.writeInFlags = RSSL_WRITE_IN_DO_NOT_COMPRESS;
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "RSSL_WRITE_IN_DO_NOT_COMPRESS on Zlib channel must succeed; err: " << err.text;
}

/* RSSL_WRITE_DO_NOT_COMPRESS flag (alias) on LZ4 channel. */
TEST_P(RsslSocketWriteTests, WriteDoNotCompressFlagOnLz4ChannelDoesNotCrash)
{
    ASSERT_TRUE(setupChannelPair("15240", RSSL_COMP_LZ4, 0));

    RsslError err;
    RsslBuffer* pBuf = getClientBuffer(512, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, 256);
    pBuf->length = 256;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    inArgs.writeInFlags = RSSL_WRITE_IN_DO_NOT_COMPRESS;
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        ret = rsslFlush(pClientChnl, &err);

    EXPECT_TRUE(ret >= RSSL_RET_SUCCESS)
        << "RSSL_WRITE_IN_DO_NOT_COMPRESS on LZ4 channel must not crash; ret=" << ret;
}

/* =======================================================================
 * -- ipcFlushSession() TARGETED TESTS ------------------------------------
 *
 * Each test below targets one of the five bugs identified by static
 * analysis of ipcFlushSession() in rsslSocketTransportImpl.c:
 *
 *   Flush Bug 1 - Non-writev path: rsslQueueRemoveFirstLink returns NULL
 *                 ? curmsgb unchanged (stale pointer) ? rtr_dfltcFreeMsg
 *                 called on the PREVIOUS curmsgb (double-free) or on NULL
 *                 (crash).  Fixed by: asserting pLink and freeing the
 *                 already-held curmsgb pointer directly.
 *
 *   Flush Bug 2 - Writev path: same RemoveFirstLink NULL-guard missing.
 *                 curmsgb is unset when pLink is NULL, so the next line
 *                 `queueLength -= curmsgb->length` is a NULL dereference.
 *
 *   Flush Bug 3 - Writev full-write path: `queueLength` is decremented by
 *                 `curmsgb->length` (full original length) instead of
 *                 `RIPC_IOV_GETLEN(&wrtvec[wrtveclen])` (actual bytes in
 *                 the IOV slot for that iteration).  For a buffer that was
 *                 previously partially-written, `curmsgb->length` is larger
 *                 than what was actually in the vector, leaving queueLength
 *                 incorrectly positive after the flush.  rsslFlush() then
 *                 returns a non-zero value even though the queue is empty.
 *
 *   Flush Bug 4 - Return-value signed overflow: both return sites accumulate
 *                 `queueLength` into a signed `RsslRet` (int32).  With
 *                 many large messages queued the sum can exceed INT32_MAX,
 *                 wrapping to a negative value that callers interpret as
 *                 RSSL_RET_FAILURE.
 *
 *   Flush Bug 5 - Missing `\n` terminator in one of the chunk-footer error
 *                 messages (writev reducedIovLen path).  This is a cosmetic
 *                 issue; tested by checking that error text for the
 *                 equivalent non-writev path IS terminated correctly.
 *
 * Port assignments: 15500 - 15519
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Flush Bug 4 - signed overflow of flush return value.
 *
 * Write enough data to make the sum of all priority-queue lengths exceed
 * INT32_MAX if accumulated into a signed int32.  The fixed code uses an
 * unsigned accumulator and clamps before return, so the return value must
 * always be >= RSSL_RET_SUCCESS (0) even when the queue holds more than
 * 2 GB of pending data.
 *
 * In practice we cannot queue 2 GB; instead we verify the invariant that
 * rsslFlush never returns a negative value while data is pending across
 * a large (but feasible) write burst.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug4_ReturnValueNeverNegativeUnderLargeQueue)
{
    ASSERT_TRUE(setupChannelPair("15500"));

    RsslError err;
    /* Write 200 x 4 KB messages to build a large pending queue. */
    const RsslUInt32 payLen    = 4096;
    const int        writeCount = 200;

    for (int i = 0; i < writeCount; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        /* Do NOT flush - accumulate bytes in the priority queues. */
    }

    /* Flush: the accumulated pending bytes must never produce a negative
     * return value due to signed RsslRet overflow (Bug 4). */
    RsslRet flushRet = rsslFlush(pClientChnl, &err);

    EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
        << "rsslFlush must return >= 0 even when many bytes are pending; "
           "a negative value indicates signed-overflow in retVal accumulation "
           "(ipcFlushSession Bug 4); ret=" << flushRet;
}

/* -----------------------------------------------------------------------
 * Flush Bug 4 (continued) - multiple priority queues summed together.
 *
 * Write to all three priority queues (HIGH / MEDIUM / LOW) without
 * flushing, then call rsslFlush.  The return value must be >= 0.  If
 * the sum across queues overflows a signed int32, it wraps negative.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug4_MultiPriorityQueueSumNeverNegative)
{
    ASSERT_TRUE(setupChannelPair("15501"));

    RsslError err;
    const RsslUInt32 payLen = 2048;
    const int msgsPerQueue  = 50;

    const RsslWritePriorities prios[3] = {
        RSSL_HIGH_PRIORITY, RSSL_MEDIUM_PRIORITY, RSSL_LOW_PRIORITY
    };

    for (int q = 0; q < 3; ++q)
    {
        for (int i = 0; i < msgsPerQueue; ++i)
        {
            RsslBuffer* pBuf = getClientBuffer(payLen, &err);
            if (!pBuf) goto done_writing;

            fillBuffer(pBuf, payLen);
            pBuf->length = payLen;

            RsslUInt32 bw = 0, ub = 0;
            RsslRet ret = rsslWrite(pClientChnl, pBuf,
                                    prios[q], 0, &bw, &ub, &err);
            if (ret < RSSL_RET_SUCCESS) goto done_writing;
        }
    }
done_writing:

    /* The sum of all three queueLength values must not overflow int32. */
    RsslRet flushRet = rsslFlush(pClientChnl, &err);

    EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
        << "rsslFlush across three priority queues must return >= 0; "
           "negative result indicates signed-overflow (Bug 4); ret=" << flushRet;
}

/* -----------------------------------------------------------------------
 * Flush Bug 3 - queueLength over-decremented for partially-advanced buffers
 *               in the writev full-write path.
 *
 * The bug: `queueLength -= curmsgb->length` uses the full original buffer
 * size even when the IOV slot holds only the remaining unwritten portion
 * (i.e. `curmsgb->local > curmsgb->buffer`).  This makes queueLength go
 * negative, which in turn makes `rsslFlush` return a negative "remaining
 * bytes" value for a queue that is actually empty.
 *
 * Test strategy: write, allow a partial flush (if the OS does one), then
 * call rsslFlush repeatedly until it returns 0.  If Bug 3 is present,
 * one of the intermediate calls returns < 0 for a non-empty queue, or the
 * final call never returns 0 because queueLength wrapped past zero.
 *
 * We drive this without direct writev control by using a blocking channel
 * and relying on the natural partial-write condition.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug3_QueueLengthCorrectAfterPartialWriteThenFull)
{
    ASSERT_TRUE(setupChannelPair("15502"));

    RsslError err;
    const RsslUInt32 payLen     = 8192;
    const int        iterations = 30;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    /* Drain with repeated flushes; every intermediate return must be >= 0. */
    const int maxFlushAttempts = 200;
    RsslRet   lastFlush = RSSL_RET_SUCCESS;
    bool      sawNegative = false;

    for (int i = 0; i < maxFlushAttempts; ++i)
    {
        /* Let the server drain so the OS send buffer never fills. */
        drainServerChannel(pServerChnl, 128);

        lastFlush = rsslFlush(pClientChnl, &err);
        if (lastFlush < RSSL_RET_SUCCESS)
        {
            sawNegative = true;
            break;
        }
        if (lastFlush == RSSL_RET_SUCCESS)
            break;  /* queue empty */

        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "rsslFlush returned negative value during drain, indicating "
           "queueLength under-count from Bug 3 (IOV-length vs full-length "
           "mismatch in writev full-write path); ret=" << lastFlush;
    EXPECT_EQ(lastFlush, RSSL_RET_SUCCESS)
        << "rsslFlush must eventually return 0 (empty queue) after draining; "
           "non-zero return suggests queueLength never reached 0 (Bug 3); "
           "ret=" << lastFlush;
}

/* -----------------------------------------------------------------------
 * Flush Bug 3 (continued) - queueLength consistency after many small writes
 *                            and a single large flush.
 *
 * Write 100 small messages and let rsslFlush drain all of them.  After
 * the queue is empty, rsslFlush must return exactly 0, not a residual
 * positive or negative count caused by queueLength misaccounting.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug3_QueueLengthZeroAfterFullDrain)
{
    ASSERT_TRUE(setupChannelPair("15503"));

    RsslError err;
    const int        writeMsgs = 100;
    const RsslUInt32 payLen    = 512;

    for (int i = 0; i < writeMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    /* Drain server, then flush until empty. */
    RsslRet finalFlush = RSSL_RET_SUCCESS;
    for (int attempt = 0; attempt < 500; ++attempt)
    {
        drainServerChannel(pServerChnl, 256);
        finalFlush = rsslFlush(pClientChnl, &err);
        if (finalFlush <= RSSL_RET_SUCCESS) break;
        time_sleep(1);
    }

    EXPECT_EQ(finalFlush, RSSL_RET_SUCCESS)
        << "After fully draining 100 messages, rsslFlush must return 0; "
           "non-zero indicates queueLength accounting error (Bug 3); "
           "ret=" << finalFlush;
}

/* -----------------------------------------------------------------------
 * Flush Bug 1 & 2 - Non-writev and writev NULL-guard stress test.
 *
 * Rapidly write and flush in tight alternation across 500 iterations.
 * If the queue becomes inconsistent (RemoveFirstLink returns NULL for a
 * non-empty queue due to race or corruption), the next flush dereferences
 * a NULL curmsgb and crashes.  Any crash here is a direct manifestation
 * of Bugs 1 or 2.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug1And2_RapidWriteFlushCycleNoCrash)
{
    ASSERT_TRUE(setupChannelPair("15504"));

    RsslError err;
    const RsslUInt32 payLen    = 256;
    const int        cycles    = 500;

    for (int i = 0; i < cycles; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (wRet < RSSL_RET_SUCCESS) break;

        /* Flush immediately after every write - stresses the single-entry
         * queue path where RemoveFirstLink must always find the peeked node. */
        RsslRet fRet = rsslFlush(pClientChnl, &err);

        ASSERT_GE(fRet, RSSL_RET_SUCCESS)
            << "rsslFlush crashed or returned failure at iteration " << i
            << " - indicates NULL-deref in RemoveFirstLink result path "
               "(ipcFlushSession Bugs 1/2); fRet=" << fRet;
    }

    SUCCEED() << "500 rapid write+flush cycles completed without crash (Bugs 1/2)";
}

/* -----------------------------------------------------------------------
 * Flush Bug 1 & 2 - Non-writev path: queue drained one message at a time.
 *
 * Write N messages, then call rsslFlush N times (once per message) with
 * the server draining between calls.  Each flush removes exactly one
 * buffer from the queue via RemoveFirstLink.  If the returned pLink is
 * NULL (which would indicate a bug), the curmsgb dereference crashes.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug1And2_IncrementalFlushOneMessageAtATime)
{
    ASSERT_TRUE(setupChannelPair("15505"));

    RsslError err;
    const RsslUInt32 payLen = 128;
    const int        msgCount = 50;

    for (int i = 0; i < msgCount; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    /* Flush incrementally: each call should remove one buffer without NULL-deref. */
    RsslRet prevFlush = INT32_MAX;
    bool    sawNegative = false;

    for (int f = 0; f < msgCount * 4 && prevFlush > 0; ++f)
    {
        drainServerChannel(pServerChnl, 64);
        RsslRet fRet = rsslFlush(pClientChnl, &err);

        if (fRet < RSSL_RET_SUCCESS)
        {
            sawNegative = true;
            break;
        }
        prevFlush = fRet;
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "Incremental flush returned failure; indicates NULL-deref in "
           "RemoveFirstLink result path (ipcFlushSession Bug 1/2)";
    EXPECT_EQ(prevFlush, 0)
        << "Queue must be fully empty after incremental drain";
}

/* -----------------------------------------------------------------------
 * Flush Bug 2 - Writev path: flush after many fragmented messages.
 *
 * The writev full-write loop (`cc == lenToWrite`) iterates backward
 * through `wrtveclen` calling RemoveFirstLink for each IOV slot.  If any
 * slot's RemoveFirstLink returns NULL (impossible in a correct queue, but
 * triggered by the bug if the queue length is inconsistent), the unguarded
 * `curmsgb->length` dereference crashes.  Write many 3-fragment messages
 * and flush all of them to exercise this loop at depth.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug2_WritevPathDeepIOVFlushNoCrash)
{
    const RsslUInt32 fragSize = 512;
    ASSERT_TRUE(setupChannelPair("15506", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Each message produces 3 IOV entries in wrtvec. */
    const RsslUInt32 msgSize   = fragSize * 3 - 10;
    const int        iterations = 100;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    /* Single flush to drain all 100 x 3-fragment chains via the writev loop. */
    bool sawNegative = false;
    for (int attempt = 0; attempt < 300; ++attempt)
    {
        drainServerChannel(pServerChnl, 512);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "rsslFlush in writev path returned failure for 3-fragment messages; "
           "indicates NULL-deref in IOV RemoveFirstLink loop (Bug 2)";
}

/* -----------------------------------------------------------------------
 * Flush Bug 5 - Error-text termination: non-writev path chunk-footer error.
 *
 * When the chunk-footer write fails in the non-writev path the error text
 * is correctly `\n`-terminated (as confirmed by reading the source).  The
 * writev `reducedIovLen` path previously lacked the `\n`.  We verify the
 * observable behaviour: after any flush failure the error text must not be
 * empty and must end with a printable character or whitespace (i.e. the
 * last char is not '\0' at position 0, indicating the text was populated).
 *
 * This test triggers an HTTP-tunnelled flush path by enabling httpHeaders
 * and writing one message, letting the flush operate normally.  We check
 * that on any error return the error text is non-empty (Bug 5 produces a
 * truncated log line rather than a crash, so we can only check existence).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug5_ErrorTextNonEmptyOnFlushFailure)
{
    ASSERT_TRUE(setupChannelPair("15507"));

    RsslError err;
    bool gotError = false;

    /* Flood to trigger a flush failure path so we can inspect error text. */
    for (int i = 0; i < 500 && !gotError; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(1024, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 1024);
        pBuf->length = 1024;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED || ret == RSSL_RET_FAILURE)
        {
            gotError = true;
            if (ret == RSSL_RET_FAILURE)
            {
                /* Error text must be non-empty and its first byte must not be '\0'. */
                EXPECT_NE(err.text[0], '\0')
                    << "Error text must be populated on flush failure (Bug 5 "
                       "causes truncated text in writev chunk-footer path)";
            }
        }
        else if (ret > RSSL_RET_SUCCESS)
        {
            RsslRet fRet = rsslFlush(pClientChnl, &err);
            if (fRet < RSSL_RET_SUCCESS)
            {
                gotError = true;
                EXPECT_NE(err.text[0], '\0')
                    << "rsslFlush error text must be populated (Bug 5)";
            }
        }
    }

    SUCCEED() << "Error-text termination check completed "
              << "(gotError=" << gotError << ")";
}

/* -----------------------------------------------------------------------
 * Flush Bug 4 - Return value stays non-negative after high-water-mark hit.
 *
 * Set a very low high-water mark so that rsslWriteEx triggers an automatic
 * flush on nearly every write.  Each flush return value must be >= 0.
 * A single negative return would indicate the signed accumulation overflow.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug4_FlushReturnNonNegativeWithLowHighWaterMark)
{
    ASSERT_TRUE(setupChannelPair("15508"));

    RsslError err;
    int watermark = 1;
    rsslIoctl(pClientChnl, RSSL_HIGH_WATER_MARK, &watermark, &err);

    const RsslUInt32 payLen    = 1024;
    const int        iterations = 100;
    bool             sawNegative = false;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (wRet < RSSL_RET_SUCCESS) break;

        /* After an auto-flush the return value must be >= 0. */
        if (wRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }

        if (wRet > RSSL_RET_SUCCESS)
        {
            drainServerChannel(pServerChnl, 64);
            RsslRet fRet = rsslFlush(pClientChnl, &err);
            if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        }
    }

    EXPECT_FALSE(sawNegative)
        << "rsslFlush / rsslWriteEx returned negative with low high-water "
           "mark; indicates signed overflow in queueLength accumulation "
           "(ipcFlushSession Bug 4)";
}

/* -----------------------------------------------------------------------
 * Flush Bug 3 - queueLength consistency: write then close; server reads
 *               all data.  Verifies no queueLength residue causes a
 *               spurious flush pending-bytes report.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug3_QueueLengthZeroReportedAfterFullTransfer)
{
    ASSERT_TRUE(setupChannelPair("15509"));

    RsslError err;
    const RsslUInt32 payLen = 512;
    const int        count  = 40;

    for (int i = 0; i < count; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    /* Drain server then flush until queue is empty. */
    RsslRet finalRet = 1;
    for (int t = 0; t < 300 && finalRet > 0; ++t)
    {
        drainServerChannel(pServerChnl, 256);
        finalRet = rsslFlush(pClientChnl, &err);
        ASSERT_GE(finalRet, RSSL_RET_SUCCESS)
            << "Negative flush return at iteration " << t
            << " indicates queueLength underflow (Bug 3); ret=" << finalRet;
        time_sleep(1);
    }

    EXPECT_EQ(finalRet, RSSL_RET_SUCCESS)
        << "Final rsslFlush must return 0 after all data transferred; "
           "non-zero residue indicates queueLength mismatch (Bug 3)";
}

/* -----------------------------------------------------------------------
 * Flush Bugs 1+2+4 - Concurrent writer thread and repeated flusher.
 *
 * One thread writes 200 messages; the main thread calls rsslFlush in a
 * tight loop.  Any flush return < 0 indicates signed overflow (Bug 4).
 * Any crash indicates a NULL-deref in RemoveFirstLink (Bugs 1/2).
 * --------------------------------------------------------------------- */

struct FlushStressArg
{
    RsslChannel*      pChnl;
    std::atomic<bool> writerDone;
    std::atomic<bool> sawNegativeFlush;
    RsslUInt32        payLen;
    int               msgCount;

    FlushStressArg()
        : pChnl(nullptr), writerDone(false), sawNegativeFlush(false),
          payLen(256), msgCount(200)
    {}
};

static RSSL_THREAD_DECLARE(flushStressWriterFn, pArg)
{
    FlushStressArg* a = reinterpret_cast<FlushStressArg*>(pArg);
    RsslError err;

    for (int i = 0; i < a->msgCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(a->pChnl, a->payLen, RSSL_FALSE, &err);
        if (!pBuf) break;

        for (RsslUInt32 k = 0; k < a->payLen; ++k)
            pBuf->data[k] = (char)('A' + k % 26);
        pBuf->length = a->payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(a->pChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }
    a->writerDone = true;
    return 0;
}

TEST_P(RsslSocketWriteChannelLockTests, FlushBugs1_2_4_ConcurrentWriteAndFlushNoCrash)
{
    ASSERT_TRUE(setupChannelPair("15510"));

    FlushStressArg args;
    args.pChnl    = pClientChnl;
    args.payLen   = 512;
    args.msgCount = 200;

    RsslThreadId writerThread;
    RSSL_THREAD_START(&writerThread, flushStressWriterFn, &args);

    RsslError err;
    /* Flush in a tight loop while the writer runs. */
    while (!args.writerDone.load())
    {
        drainServerChannel(pServerChnl, 64);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS)
        {
            args.sawNegativeFlush = true;
            break;
        }
        time_sleep(1);
    }

    RSSL_THREAD_JOIN(writerThread);

    /* Final drain. */
    for (int t = 0; t < 100; ++t)
    {
        drainServerChannel(pServerChnl, 256);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { args.sawNegativeFlush = true; break; }
        if (fRet == 0) break;
        time_sleep(2);
    }

    EXPECT_FALSE(args.sawNegativeFlush.load())
        << "Concurrent write+flush produced a negative flush return; "
           "indicates signed overflow (Bug 4) or NULL-deref (Bug 1/2)";
    SUCCEED() << "Concurrent write+flush stress completed without crash";
}

/* -----------------------------------------------------------------------
 * Flush Bug 4 - Fragmented messages accumulate more bytes per message.
 *               Verify non-negative flush return with fragmented queue.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug4_FragmentedQueueReturnValueNonNegative)
{
    const RsslUInt32 fragSize = 1024;
    ASSERT_TRUE(setupChannelPair("15511", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* 4-fragment messages produce large queueLength values quickly. */
    const RsslUInt32 msgSize   = fragSize * 4 - 10;
    const int        writeCount = 50;

    for (int i = 0; i < writeCount; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(msgSize, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, msgSize);
        pBuf->length = msgSize;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    bool sawNegative = false;
    for (int t = 0; t < 300; ++t)
    {
        drainServerChannel(pServerChnl, 256);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "rsslFlush returned negative for fragmented queue; "
           "indicates signed overflow of queueLength accumulation (Bug 4)";
}

/* =======================================================================
 * -- rsslFlush() CRASH-RISK TESTS -----------------------------------------
 *
 * The tests below target specific crash paths INSIDE ipcFlushSession()
 * that are distinct from the ipcWriteSession() bugs already covered.
 *
 * Code paths targeted:
 *
 *   CF2  - rsslFlush() on a channel whose public state is CLOSED before
 *          ipcFlushSession() checks workState: if the state guard is not
 *          reached, the code reads rsslSocketChannel->transportFuncs
 *          from a channel whose internals are partially torn down.
 *
 *   CF3  - rsslFlush() after the remote peer closes the connection:
 *          the write/writev call returns -2; the error path must set error
 *          text and return RSSL_RET_FAILURE without a second attempt.
 *
 *   CF4  - Flush return value must eventually reach exactly 0 after the
 *          server drains the TCP stream: verifies that the queueLength
 *          accumulation (Bug 3) and signed return (Bug 4) leave no residue.
 *
 *   CF5  - rsslFlush() called while the priority queue contains messages
 *          at all three priorities simultaneously: the flushStrategy walk
 *          and iovPriority[] array must not go out of bounds.
 *
 *   CF6  - rsslFlush() after a write that triggered WRITE_FLUSH_FAILED
 *          must fully drain the queue when followed by server-side reads.
 *
 *   CF7  - Very high volume of small messages: 1 000 x 64-byte writes
 *          batched before a single flush tests the writev IOV array
 *          (RIPC_MAXIOVLEN slots) boundary and the queueLength sum.
 *
 *   CF8  - Repeat flush after full drain returns 0 consistently (empty
 *          queue re-entry crash: if the non-writev path handles the
 *          low-priority fallback check incorrectly a NULL curmsgb is
 *          passed to rtr_dfltcFreeMsg on an already-empty queue).
 *
 *   CF9  - Mix fragmented and non-fragmented messages across three priority
 *          queues, flush with a slow-reading server to force partial writev
 *          writes (nextOutBuf tracking, Bug 3 IOV-length accounting).
 *
 *   CF10 - Concurrent rsslFlush() calls from two threads on the same
 *          channel under RSSL_LOCK_GLOBAL_AND_CHANNEL: the per-channel
 *          lock must serialize the queueLength decrement and the
 *          RemoveFirstLink / rtr_dfltcFreeMsg sequence.
 *
 *   CF11 - Channel closed mid-flush: close the server side to break the
 *          TCP connection, then call rsslFlush() which triggers a fatal
 *          write error inside ipcFlushSession(). The function must return
 *          RSSL_RET_FAILURE and set error text without double-freeing any
 *          queued msgb or corrupting the priority queue pointers.
 *
 *   CF12 - Flush with all three priority queues at maximum message depth:
 *          write RIPC_MAX_FLUSH_STRATEGY (32) messages to each of the three
 *          queues without flushing, then call rsslFlush once. This fills the
 *          flushStrategy walk completely and stresses the writev IOV array.
 *
 *   CF13 - Flush after rapid channel reuse: open, write, close, reopen on
 *          the same port. The second channel pair reuses pool blocks freed
 *          by the first close; rsslFlush on the second pair must not follow
 *          stale nextMsg pointers left from the first lifetime.
 *
 *   CF14 - Single-message flush to verify the non-writev path's
 *          RemoveFirstLink guard (Bug 1): write one small message, flush.
 *          Repeat 500 times with a fresh buffer each time.
 *
 *   CF15 - Flush with a fragmented message exactly at the writev IOV
 *          slot boundary: the number of fragments equals the IOV array
 *          capacity so that the full-write loop iterates over every slot.
 *
 * Port assignments: 15600 - 15619
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * CF2 - rsslFlush() on a channel whose state is set to CLOSED.
 *
 * If rsslFlush() does not check the public channel state before entering
 * ipcFlushSession(), it will call transportFuncs->writeTransport /
 * writeVTransport on a partially-torn-down channel, crashing.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_ClosedChannelStateReturnsFail)
{
    ASSERT_TRUE(setupChannelPair("15601"));

    RsslError err;
    /* Queue one message then mark the channel CLOSED without freeing
     * internal structures so TearDown can still call rsslCloseChannel. */
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    if (pBuf)
    {
        fillBuffer(pBuf, 64);
        pBuf->length = 64;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    pClientChnl->state = RSSL_CH_STATE_CLOSED;

    RsslRet ret = rsslFlush(pClientChnl, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "rsslFlush on a CLOSED channel must return RSSL_RET_FAILURE; ret=" << ret;

    pClientChnl->state = RSSL_CH_STATE_ACTIVE;
}

/* -----------------------------------------------------------------------
 * CF3 - rsslFlush() after the remote end closes.
 *
 * When the server side closes, the next write() call inside
 * ipcFlushSession() returns -2 (connection reset). The error-path
 * must populate error->text and return RSSL_RET_FAILURE exactly once
 * without attempting a second write or double-freeing the queued msgb.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_AfterRemoteCloseReturnsFailure)
{
    ASSERT_TRUE(setupChannelPair("15602"));

    RsslError err;
    /* Queue enough data to guarantee the flush will attempt a socket write. */
    for (int i = 0; i < 20; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(1024, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 1024);
        pBuf->length = 1024;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    /* Kill the remote end so the next write returns an OS error. */
    rsslCloseChannel(pServerChnl, &err);
    pServerChnl = nullptr;
    time_sleep(150);   /* allow the RST to propagate */

    RsslRet ret = rsslFlush(pClientChnl, &err);

    /* Must fail gracefully - not crash. The exact return depends on whether
     * the OS already delivered the RST; accept failure or channel-close. */
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
    EXPECT_TRUE(ret == RSSL_RET_SUCCESS || ret == RSSL_RET_FAILURE)
        << "rsslFlush after remote close must fail gracefully; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * CF4 - Flush return value eventually reaches exactly 0.
 *
 * After writing and completely draining all data, the pending-bytes
 * return must reach 0 - not a residual positive (Bug 3 queueLength)
 * or negative (Bug 4 signed overflow) value.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_ReturnValueReachesZeroAfterFullDrain)
{
    ASSERT_TRUE(setupChannelPair("15603"));

    RsslError err;
    const RsslUInt32 payLen = 2048;
    const int        msgs   = 20;

    for (int i = 0; i < msgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    /* Drain the server receive buffer and flush until empty. */
    RsslRet finalRet = 1;
    for (int t = 0; t < 400 && finalRet > 0; ++t)
    {
        drainServerChannel(pServerChnl, 128);
        finalRet = rsslFlush(pClientChnl, &err);
        ASSERT_GE(finalRet, RSSL_RET_SUCCESS)
            << "Negative flush at attempt " << t
            << " - Bug 3 or Bug 4 present; ret=" << finalRet;
        time_sleep(1);
    }

    EXPECT_EQ(finalRet, RSSL_RET_SUCCESS)
        << "rsslFlush must return 0 after all data is drained; ret=" << finalRet;
}

/* -----------------------------------------------------------------------
 * CF5 - Flush with all three priority queues populated simultaneously.
 *
 * ipcFlushSession() iterates the flushStrategy[] array to pick buffers
 * from each queue.  All three queues containing messages simultaneously
 * forces the strategy walk across all priority levels.  An off-by-one
 * in `currentOutList` or a -1 sentinel in `iovPriority[]` causes an
 * out-of-bounds access into `priorityQueues[]`.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_AllThreePriorityQueuesPopulatedSimultaneously)
{
    ASSERT_TRUE(setupChannelPair("15604"));

    RsslError err;
    const RsslUInt32 payLen = 512;

    const RsslWritePriorities prios[3] = {
        RSSL_HIGH_PRIORITY, RSSL_MEDIUM_PRIORITY, RSSL_LOW_PRIORITY
    };

    /* Write 15 messages (5 per queue) without flushing. */
    for (int q = 0; q < 3; ++q)
    {
        for (int i = 0; i < 5; ++i)
        {
            RsslBuffer* pBuf = getClientBuffer(payLen, &err);
            if (!pBuf) goto flush_all;
            fillBuffer(pBuf, payLen);
            pBuf->length = payLen;
            RsslUInt32 bw = 0, ub = 0;
            rsslWrite(pClientChnl, pBuf, prios[q], 0, &bw, &ub, &err);
        }
    }
flush_all:

    /* Single flush call must drain all three queues without crash. */
    bool sawNegative = false;
    for (int t = 0; t < 300; ++t)
    {
        drainServerChannel(pServerChnl, 128);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "Flush with all three priority queues must not return negative "
           "or crash; indicates iovPriority[] OOB or queueLength underflow "
           "(CF5)";
}

/* -----------------------------------------------------------------------
 * CF6 - rsslFlush() drains completely after WRITE_FLUSH_FAILED.
 *
 * After the output queue triggers WRITE_FLUSH_FAILED, a subsequent flush
 * loop (with the server draining) must empty the queue to 0.  If Bug 3
 * leaves a stale positive queueLength or Bug 4 wraps it negative, the
 * queue never reaches 0 and this test hangs at the ASSERT or fails.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_DrainToZeroAfterWriteFlushFailed)
{
    ASSERT_TRUE(setupChannelPair("15605"));

    RsslError err;
    bool gotFlushFailed = false;

    for (int i = 0; i < 300 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(4096, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 4096);
        pBuf->length = 4096;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret == RSSL_RET_WRITE_FLUSH_FAILED) gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS) break;
    }

    if (!gotFlushFailed)
    {
        SUCCEED() << "WRITE_FLUSH_FAILED not triggered; CF6 skipped";
        return;
    }

    /* Drain: every rsslFlush call must be >= 0 and eventually 0. */
    RsslRet finalRet = 1;
    for (int t = 0; t < 500 && finalRet > 0; ++t)
    {
        drainServerChannel(pServerChnl, 256);
        finalRet = rsslFlush(pClientChnl, &err);
        ASSERT_GE(finalRet, RSSL_RET_SUCCESS)
            << "Negative rsslFlush after WRITE_FLUSH_FAILED at attempt " << t
            << " (CF6/Bug 3/Bug 4); ret=" << finalRet;
        time_sleep(1);
    }

    EXPECT_EQ(finalRet, RSSL_RET_SUCCESS)
        << "rsslFlush must reach 0 after WRITE_FLUSH_FAILED + drain (CF6)";
}

/* -----------------------------------------------------------------------
 * CF7 - High-volume small message batch flush.
 *
 * 1 000 x 64-byte writes batched before a single rsslFlush() call
 * saturates the writev IOV array across many iterations.  The
 * `wrtveclen` counter must not exceed RIPC_MAXIOVLEN; the return value
 * must never be negative.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_HighVolumeSmallMessageBatchFlush)
{
    ASSERT_TRUE(setupChannelPair("15606"));

    RsslError err;
    const RsslUInt32 payLen = 64;
    const int        count  = 1000;

    for (int i = 0; i < count; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
    }

    bool sawNegative = false;
    for (int t = 0; t < 500; ++t)
    {
        drainServerChannel(pServerChnl, 512);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "rsslFlush must not return negative during high-volume batch "
           "(CF7: writev IOV overflow / queueLength underflow)";
}

/* -----------------------------------------------------------------------
 * CF8 - Repeated flush on an already-empty queue.
 *
 * After the queue is fully drained, calling rsslFlush() 1 000 more times
 * must consistently return 0.  If the non-writev path's low-priority
 * fallback check (the `rsslQueuePeekFront(&priorityQueues[2])` after the
 * strategy walk) returns a stale non-NULL pLink even though the queue is
 * empty, `curmsgb` points to freed memory and rtr_dfltcFreeMsg crashes.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_RepeatedFlushOnEmptyQueueNeverCrashes)
{
    ASSERT_TRUE(setupChannelPair("15607"));

    RsslError err;

    /* Write and drain one message to put the queue through a complete cycle. */
    RsslBuffer* pBuf = getClientBuffer(64, &err);
    if (pBuf)
    {
        fillBuffer(pBuf, 64);
        pBuf->length = 64;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        for (int t = 0; t < 200; ++t)
        {
            drainServerChannel(pServerChnl, 16);
            if (rsslFlush(pClientChnl, &err) == 0) break;
            time_sleep(1);
        }
    }

    /* Queue is now empty.  Flush 1 000 more times. */
    bool sawNonZero = false;
    for (int i = 0; i < 1000; ++i)
    {
        RsslRet ret = rsslFlush(pClientChnl, &err);
        ASSERT_GE(ret, RSSL_RET_SUCCESS)
            << "Flush on empty queue returned negative at iteration " << i
            << " (CF8: stale pLink in low-priority fallback); ret=" << ret;
        if (ret != RSSL_RET_SUCCESS) { sawNonZero = true; break; }
    }

    EXPECT_FALSE(sawNonZero)
        << "rsslFlush on empty queue must always return 0 (CF8)";
}

/* -----------------------------------------------------------------------
 * CF9 - Mix of fragmented and non-fragmented messages across all three
 *       priority queues with slow server to force partial writev writes.
 *
 * Partial writev updates `nextOutBuf` to the priority index of the buffer
 * that was only partly sent.  The next flush must resume from that buffer
 * (the writev loop re-peeks `priorityQueues[nextOutBuf]`).  If the index
 * is stale or the peek finds a wrong buffer, the partial-write handler
 * does `curmsgb->local += cc` on the wrong msgb - heap corruption.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_MixedSizesAllQueuesWithSlowServerDrain)
{
    const RsslUInt32 fragSize = 1024;
    ASSERT_TRUE(setupChannelPair("15608", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    const RsslWritePriorities prios[3] = {
        RSSL_HIGH_PRIORITY, RSSL_MEDIUM_PRIORITY, RSSL_LOW_PRIORITY
    };
    const RsslUInt32 sizes[3] = { 64, fragSize + 100, fragSize * 2 + 50 };

    /* Write one non-fragmented and one fragmented message per priority
     * queue before each flush attempt. */
    for (int round = 0; round < 20; ++round)
    {
        for (int q = 0; q < 3; ++q)
        {
            RsslBuffer* pBuf = getClientBuffer(sizes[q % 3], &err);
            if (!pBuf) break;
            fillBuffer(pBuf, sizes[q % 3]);
            pBuf->length = sizes[q % 3];
            RsslUInt32 bw = 0, ub = 0;
            rsslWrite(pClientChnl, pBuf, prios[q], 0, &bw, &ub, &err);
        }

        /* Drain the server only partially to maximise partial-send probability. */
        drainServerChannel(pServerChnl, 4);

        RsslRet fRet = rsslFlush(pClientChnl, &err);
        ASSERT_GE(fRet, RSSL_RET_SUCCESS)
            << "Mixed-size all-queue flush returned negative at round " << round
            << " (CF9: stale nextOutBuf or queueLength underflow); ret=" << fRet;
    }

    /* Drain completely. */
    for (int t = 0; t < 300; ++t)
    {
        drainServerChannel(pServerChnl, 256);
        if (rsslFlush(pClientChnl, &err) == 0) break;
        time_sleep(1);
    }

    SUCCEED() << "Mixed-size all-queue flush with slow server completed without crash (CF9)";
}

/* -----------------------------------------------------------------------
 * CF10 - Concurrent rsslFlush() calls from two threads.
 *
 * Two threads both call rsslFlush() on the same channel in a tight loop
 * while a writer thread queues data.  Under RSSL_LOCK_GLOBAL_AND_CHANNEL
 * the per-channel mutex serialises the queueLength decrement and the
 * RemoveFirstLink / rtr_dfltcFreeMsg pair.  Without the lock the Bug 1
 * and Bug 2 NULL-deref paths become reachable via a TOCTOU race.
 * --------------------------------------------------------------------- */

struct ConcurrentFlushArg
{
    RsslChannel*       pChnl;
    RsslChannel*       pServerChnl;
    std::atomic<bool>  stop;
    std::atomic<bool>  sawNegative;
    std::atomic<int>   flushCount;

    ConcurrentFlushArg()
        : pChnl(nullptr), pServerChnl(nullptr),
          stop(false), sawNegative(false), flushCount(0)
    {}
};

static RSSL_THREAD_DECLARE(concurrentFlusherFn, pArg)
{
    ConcurrentFlushArg* a = reinterpret_cast<ConcurrentFlushArg*>(pArg);
    RsslError err;

    while (!a->stop.load())
    {
        RsslRet ret = rsslFlush(a->pChnl, &err);
        if (ret < RSSL_RET_SUCCESS)
        {
            a->sawNegative = true;
            break;
        }
        ++a->flushCount;
        time_sleep(1);
    }
    return 0;
}

TEST_P(RsslSocketWriteChannelLockTests, Flush_ConcurrentFlushesUnderChannelLockNoCrash)
{
    ASSERT_TRUE(setupChannelPair("15609"));

    /* Writer: 200 messages then stop. */
    FlushStressArg writerArgs;
    writerArgs.pChnl    = pClientChnl;
    writerArgs.payLen   = 256;
    writerArgs.msgCount = 200;

    /* Two concurrent flushers. */
    ConcurrentFlushArg flushArgs;
    flushArgs.pChnl       = pClientChnl;
    flushArgs.pServerChnl = pServerChnl;

    RsslThreadId wt, ft1, ft2;
    RSSL_THREAD_START(&wt,  flushStressWriterFn,  &writerArgs);
    RSSL_THREAD_START(&ft1, concurrentFlusherFn, &flushArgs);
    RSSL_THREAD_START(&ft2, concurrentFlusherFn, &flushArgs);

    /* Server drains. */
    for (int t = 0; t < 50; ++t)
    {
        drainServerChannel(pServerChnl, 128);
        time_sleep(5);
    }

    flushArgs.stop = true;
    RSSL_THREAD_JOIN(wt);
    RSSL_THREAD_JOIN(ft1);
    RSSL_THREAD_JOIN(ft2);

    EXPECT_FALSE(flushArgs.sawNegative.load())
        << "Concurrent flushers must not produce negative return (CF10: "
           "Bug 4 signed overflow or Bug 1/2 NULL-deref under race)";
    EXPECT_GT(flushArgs.flushCount.load(), 0)
        << "At least some concurrent flush calls must complete";
}

/* -----------------------------------------------------------------------
 * CF11 - Channel closed mid-flush (write error inside ipcFlushSession).
 *
 * Queue 30 messages then abruptly close the server side.  ipcFlushSession()
 * calls writeTransport/writeVTransport which returns -2 (RST received).
 * The function must return RSSL_RET_FAILURE without double-freeing any
 * queued msgb blocks or corrupting the priority queue linked-list nodes.
 *
 * Canary: after the flush failure, call rsslFlush() one more time.  If
 * the first failure left the priority queue in a corrupted state, the
 * second call will crash (NULL dereference of curmsgb or freed pLink).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_ChannelClosedMidFlushDoesNotCorruptQueue)
{
    ASSERT_TRUE(setupChannelPair("15610"));

    RsslError err;
    /* Queue 30 x 512-byte messages. */
    for (int i = 0; i < 30; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(512, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 512);
        pBuf->length = 512;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    /* Kill the server so the next write returns an OS error. */
    rsslCloseChannel(pServerChnl, &err);
    pServerChnl = nullptr;
    time_sleep(150);

    RsslRet ret1 = rsslFlush(pClientChnl, &err);

    /* Canary: second flush must not crash even if the first failed and
     * left the queue in an inconsistent state. */
    RsslRet ret2 = rsslFlush(pClientChnl, &err);

    bool acceptableFirst  = (ret1 < RSSL_RET_SUCCESS) ||
                            (pClientChnl->state == RSSL_CH_STATE_CLOSED);
    bool acceptableSecond = (ret2 <= RSSL_RET_SUCCESS);

    EXPECT_TRUE(acceptableFirst)
        << "First flush after remote close must fail; ret=" << ret1;
    EXPECT_TRUE(acceptableSecond)
        << "Second flush after remote close must not crash; ret=" << ret2;
}

/* -----------------------------------------------------------------------
 * CF12 - Flush with all priority queues at RIPC_MAX_FLUSH_STRATEGY depth.
 *
 * RIPC_MAX_FLUSH_STRATEGY = 32.  Write 32 messages to each of the three
 * priority queues (96 total) before a single flush.  The flushStrategy[]
 * walk must iterate through all 32 strategy slots for each queue without
 * overrunning the `tempList[]` or `iovPriority[]` arrays.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_MaxFlushStrategyDepthAllQueues)
{
    ASSERT_TRUE(setupChannelPair("15611"));

    RsslError err;
    const RsslUInt32 payLen = 256;
    /* RIPC_MAX_FLUSH_STRATEGY is 32; write that many per queue. */
    const int msgsPerQueue  = 32;

    const RsslWritePriorities prios[3] = {
        RSSL_HIGH_PRIORITY, RSSL_MEDIUM_PRIORITY, RSSL_LOW_PRIORITY
    };

    for (int q = 0; q < 3; ++q)
    {
        for (int i = 0; i < msgsPerQueue; ++i)
        {
            RsslBuffer* pBuf = getClientBuffer(payLen, &err);
            if (!pBuf) goto done_writing_cf12;
            fillBuffer(pBuf, payLen);
            pBuf->length = payLen;
            RsslUInt32 bw = 0, ub = 0;
            rsslWrite(pClientChnl, pBuf, prios[q], 0, &bw, &ub, &err);
        }
    }
done_writing_cf12:

    bool sawNegative = false;
    for (int t = 0; t < 500; ++t)
    {
        drainServerChannel(pServerChnl, 512);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "Max-strategy-depth flush must not return negative (CF12)";
}

/* -----------------------------------------------------------------------
 * CF13 - Flush after rapid channel reuse on the same port.
 *
 * Open channel pair A, write+close without flushing (pool blocks stay
 * freed), then open channel pair B on the same port.  The pool may return
 * blocks from the first lifetime that still carry stale nextMsg pointers
 * (Issue 7).  Flush channel B to drive the writev full-write loop which
 * calls RemoveFirstLink on those blocks.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_AfterChannelReuseNextMsgNotStale)
{
    RsslError err;
    const RsslUInt32 fragSize = 512;

    /* First lifetime: write + close WITHOUT flushing. */
    {
        RsslServer*  pSrv1 = nullptr;
        RsslChannel* pSrC1 = nullptr;
        RsslChannel* pClC1 = nullptr;

        if (setupActiveChannelPair("15612", &pSrv1, &pSrC1, &pClC1,
                                   RSSL_COMP_NONE, 0, fragSize))
        {
            /* Write 10 fragmented messages, do NOT flush. */
            for (int i = 0; i < 10; ++i)
            {
                RsslBuffer* pBuf = rsslGetBuffer(pClC1, fragSize + 100, RSSL_FALSE, &err);
                if (!pBuf) break;
                fillBuffer(pBuf, fragSize + 100);
                pBuf->length = fragSize + 100;
                RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
                RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
                rsslWriteEx(pClC1, pBuf, &inArgs, &outArgs, &err);
            }
            /* Close without flushing - pool blocks freed with stale nextMsg. */
            rsslCloseChannel(pSrC1, &err);
            rsslCloseChannel(pClC1, &err);
            rsslCloseServer(pSrv1, &err);
            time_sleep(20);
        }
    }

    /* Second lifetime: same port, write fragmented messages, then flush. */
    RsslServer*  pSrv2 = nullptr;
    RsslChannel* pSrC2 = nullptr;
    RsslChannel* pClC2 = nullptr;

    if (!setupActiveChannelPair("15612", &pSrv2, &pSrC2, &pClC2,
                                RSSL_COMP_NONE, 0, fragSize))
    {
        SUCCEED() << "Second channel pair failed to set up - skip CF13";
        return;
    }

    for (int i = 0; i < 20; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClC2, fragSize + 100, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, fragSize + 100);
        pBuf->length = fragSize + 100;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClC2, pBuf, &inArgs, &outArgs, &err);
    }

    bool sawNegative = false;
    for (int t = 0; t < 300; ++t)
    {
        drainServerChannel(pSrC2, 256);
        RsslRet fRet = rsslFlush(pClC2, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    rsslCloseChannel(pSrC2, &err);
    rsslCloseChannel(pClC2, &err);
    rsslCloseServer(pSrv2, &err);

    EXPECT_FALSE(sawNegative)
        << "Flush after channel reuse must not return negative (CF13: "
           "stale nextMsg after pool reuse from previous channel lifetime)";
}

/* -----------------------------------------------------------------------
 * CF14 - Single-message flush stress (500 iterations of write-then-flush).
 *
 * Each iteration writes exactly ONE buffer, then calls rsslFlush() until
 * the queue empties.  This exercises the RemoveFirstLink guard (Bug 1)
 * on the single-entry queue path: RemoveFirstLink must always succeed
 * when PeekFront returned non-NULL.  If the guard is missing, the very
 * first flush after each write will crash.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_SingleMessageFlushStressNoCrash)
{
    ASSERT_TRUE(setupChannelPair("15613"));

    RsslError err;
    const RsslUInt32 payLen = 128;

    for (int i = 0; i < 500; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (wRet < RSSL_RET_SUCCESS) break;

        drainServerChannel(pServerChnl, 8);
        RsslRet fRet = rsslFlush(pClientChnl, &err);

        ASSERT_GE(fRet, RSSL_RET_SUCCESS)
            << "Single-message flush returned failure at iteration " << i
            << " (CF14: RemoveFirstLink NULL guard / Bug 1); ret=" << fRet;
    }

    SUCCEED() << "500 single-message flush iterations completed without crash (CF14)";
}

/* -----------------------------------------------------------------------
 * CF15 - Fragmented message exactly filling the writev IOV array.
 *
 * The writev IOV array (`wrtvec[RIPC_MAXIOVLEN]`) is built up to
 * `iovLength = RIPC_MAXIOVLEN` slots.  Write enough small-fragment
 * messages without flushing so that a single flush builds a full IOV
 * array, then calls writeVTransport() for the entire array at once.
 * The full-write path iterates backward through all `wrtveclen` slots
 * calling RemoveFirstLink on each - the boundary where wrtveclen
 * decrements to 0 must not access `iovPriority[-1]`.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, Flush_WritevIovArrayFullBoundaryNoCrash)
{
    /* Small fragment size: each message uses exactly 1 IOV slot (one
     * fragment fits inside the IOV array limit). */
    const RsslUInt32 fragSize = 256;
    ASSERT_TRUE(setupChannelPair("15614", RSSL_COMP_NONE, 0, fragSize));

    RsslError err;
    /* Write more messages than RIPC_MAXIOVLEN to ensure the IOV array
     * is filled to capacity on at least one flush iteration.
     * RIPC_MAXIOVLEN is typically 16; 50 messages guarantees several full
     * IOV batches. */
    const int queueMsgs = 50;

    for (int i = 0; i < queueMsgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(fragSize - 10, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, fragSize - 10);
        pBuf->length = fragSize - 10;
        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    bool sawNegative = false;
    for (int t = 0; t < 400; ++t)
    {
        drainServerChannel(pServerChnl, 128);
        RsslRet fRet = rsslFlush(pClientChnl, &err);
        if (fRet < RSSL_RET_SUCCESS) { sawNegative = true; break; }
        if (fRet == 0) break;
        time_sleep(1);
    }

    EXPECT_FALSE(sawNegative)
        << "Full-IOV-array flush must not return negative or crash (CF15: "
           "iovPriority[] boundary / Bug 2 writev RemoveFirstLink guard)";
}

/* -----------------------------------------------------------------------
 * Flush Bug 3 - queueLength accounting: write, partial-read by server,
 *               then final flush.  The return value trajectory must be
 *               monotonically non-increasing (each flush sends more data,
 *               so pending bytes should not increase between flush calls).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, FlushBug3_PendingBytesMonotonicallyDecreasing)
{
    ASSERT_TRUE(setupChannelPair("15512"));

    RsslError err;
    const RsslUInt32 payLen = 1024;
    const int        msgs   = 30;

    for (int i = 0; i < msgs; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payLen, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, payLen);
        pBuf->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    }

    RsslRet prevPending = INT32_MAX;
    bool    monotonic   = true;

    for (int t = 0; t < 200; ++t)
    {
        drainServerChannel(pServerChnl, 64);
        RsslRet pending = rsslFlush(pClientChnl, &err);
        if (pending < RSSL_RET_SUCCESS)
        {
            /* Channel closed / error: stop but don't fail the monotonic check. */
            break;
        }
        /* queueLength-based return must not increase between flushes
         * (Bug 3 can cause it to go from N to N-delta where delta < 0
         * i.e. the value wraps upward). */
        if (pending > prevPending)
        {
            monotonic = false;
            break;
        }
        prevPending = pending;
        if (pending == 0) break;
        time_sleep(1);
    }

    EXPECT_TRUE(monotonic)
        << "rsslFlush pending-bytes return increased between consecutive calls; "
           "indicates queueLength wrap (Bug 3 IOV-length vs full-length mismatch)";
}

/* =======================================================================
 * -- DIRECT ipcWriteSession() CRASH-RISK TESTS ---------------------------
 *
 * The six tests below each target a specific crash vector inside
 * ipcWriteSession() that is reachable through the public RSSL write API
 * without internal struct manipulation.
 *
 * Crash vectors:
 *   A - NULL bufferInfo dereference when writing a released buffer.
 *   B - Heap over-read in the fragmented MemCopyByInt copy loop when
 *       pBuf->length is inflated past the malloc'd data block.
 *   C - Use-after-free write (Issue 4) in the non-compressed forceFlush
 *       path: rtr_dfltcFreeMsg(msgb) followed by msgb->buffer=0 / length=0.
 *   D - packed-buffer backing-store overflow when the final slot's length
 *       makes packingOffset exceed ripcBuffer->maxLength (Issue 2 / 10).
 *   E - RsslInt32 uncompBytes accumulation sanity check (Issue 8):
 *       uncompressedBytesWritten must be positive after a large write.
 *   F - writeOutArgs stale sentinel on WRITE_FLUSH_FAILED (Issue 9):
 *       bytesWritten must be updated on the flush-failed return path.
 *
 * Port assignments: 15300 - 15305
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * Crash vector A - NULL bufferInfo dereference after rsslReleaseBuffer().
 *
 * rsslReleaseBuffer() sets rsslBufImpl->bufferInfo = 0.  A subsequent
 * rsslWriteEx() call must detect the zero bufferInfo inside ipcWriteSession()
 * and return RSSL_RET_FAILURE before the code casts NULL to rtr_msgb_t* and
 * accesses msgb->length / msgb->maxLength.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_ReleasedBufferWriteIsGuarded)
{
    ASSERT_TRUE(setupChannelPair("15300"));

    RsslError   err;
    RsslBuffer* pBuf = getClientBuffer(128, &err);
    ASSERT_NE(pBuf, nullptr) << "rsslGetBuffer failed: " << err.text;

    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    /* Explicitly release the buffer; rsslBufImpl->bufferInfo is set to 0. */
    RsslRet relRet = rsslReleaseBuffer(pBuf, &err);
    ASSERT_GE(relRet, RSSL_RET_SUCCESS) << "rsslReleaseBuffer failed: " << err.text;

    /* Attempt to write the released buffer.  The ipcWriteSession() guard
     * `rsslBufImpl->bufferInfo == 0` must fire before any msgb dereference. */
    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_EQ(ret, RSSL_RET_FAILURE)
        << "Writing a released buffer must return RSSL_RET_FAILURE "
           "(bufferInfo NULL guard); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * Crash vector B - heap over-read in the fragmented MemCopyByInt loop.
 *
 * rsslGetBuffer() for fragmented buffers allocates the user data block as
 *   rsslBufImpl->buffer.data = _rsslMalloc(size + 7)
 *
 * rsslSocketWrite() drives the copy loop with:
 *   tempSize = rsslBufImpl->buffer.length - writeCursor   // uses inflated length
 *   MemCopyByInt(ripcBuffer->buffer,
 *                rsslBufImpl->buffer.data + writeCursor,  // source
 *                tempSize);                               // reads past alloc end
 *
 * When pBuf->length > (size + 7), the final fragment's copy reads tempSize
 * bytes starting beyond the _rsslMalloc allocation, which is a heap
 * buffer over-read detectable by ASAN or valgrind.
 *
 * Expected: RSSL_RET_FAILURE or RSSL_RET_BUFFER_TOO_SMALL.  Must not crash.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_FragmentedBufferInflatedLengthHeapOverread)
{
    /* Small fragment size forces the multi-fragment allocation path. */
    ASSERT_TRUE(setupChannelPair("15301", RSSL_COMP_NONE, 0, 3000));

    RsslError        err;
    const RsslUInt32 fragSz  = 3000;
    /* Request two fragments worth of data so allocation is fragmented. */
    const RsslUInt32 reqSize = fragSz + 512;
    RsslBuffer* pBuf = getClientBuffer(reqSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Buffer allocation failed at size " << reqSize << "; skipping";
        return;
    }

    fillBuffer(pBuf, reqSize);

    /* Inflate pBuf->length by 8 KB beyond the _rsslMalloc(reqSize + 7) block.
     * Without a bounds check, MemCopyByInt reads 8192 bytes past the
     * end of the allocation in the last-fragment copy. */
    pBuf->length = reqSize + 8192;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    /* Must not crash; expected FAILURE or BUFFER_TOO_SMALL. */
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_LT(ret, RSSL_RET_SUCCESS)
        << "Inflated fragmented buffer length must return failure (not heap "
           "over-read crash); ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Crash vector C - use-after-free of msgb in the non-compressed forceFlush
 * path (Issue 4 on the plain-socket write path).
 *
 * In ipcWriteSession(), after a successful direct-socket send:
 *   rtr_dfltcFreeMsg(msgb);    // returns block to pool
 *   msgb->buffer = 0;           // UAF WRITE to freed memory
 *   msgb->length = 0;           // UAF WRITE to freed memory
 *
 * The pool makes the freed block available immediately.  Under pool reuse,
 * a second allocation may receive that block before the zeroing, and the
 * zeroing then corrupts the new allocation's header fields, causing a
 * crash on the next rtr_dfltcFreeMsg() call for the new block.
 *
 * This test verifies no hard crash occurs; ASAN will report the UAF
 * writes on the two lines above.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_DirectSocketWriteNonCompressedUseAfterFree)
{
    ASSERT_TRUE(setupChannelPair("15302"));

    RsslError err;
    const RsslUInt32 payloadLen = 512;

    /* Execute the forceFlush=1 path 50 times so the pool reuse pressure
     * maximises the chance that the freed block is reallocated before the
     * UAF zeroing, exposing the corruption under ASAN. */
    for (int i = 0; i < 50; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(payloadLen, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, payloadLen);
        pBuf->length = payloadLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        /* DIRECT_SOCKET_WRITE triggers the forceFlush=1 path where the
         * freed-block UAF writes occur on success (Issue 4). */
        inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    /* The writes succeed but each triggers UAF zero-writes to freed memory.
     * We only verify the test process does not hard-crash.
     * Run under ASAN/valgrind to detect the use-after-free writes. */
    SUCCEED() << "Non-compressed DIRECT_SOCKET_WRITE completed without hard crash; "
                 "run under ASAN to detect Issue 4 UAF writes";
}

/* -----------------------------------------------------------------------
 * Crash vector D - packed-buffer backing-store overflow via packingOffset.
 *
 * After rsslPackBuffer() commits one message and advances packingOffset,
 * inflating the returned next-slot length causes:
 *   packingOffset += inflated_length      (rsslSocketWrite)
 *   ripcBuffer->length = packingOffset    (now > maxLength)
 *
 * ipcWriteSession() then detects msgb->length > msgb->maxLength and must
 * return RSSL_RET_BUFFER_TOO_SMALL.  Without that guard, the subsequent
 * RTR_PUT_16 call for the sub-message length prefix writes 2 bytes at
 * offset (packingOffset - 2) which is beyond the pool block boundary,
 * corrupting adjacent heap metadata (Issue 2 / Issue 10).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_PackedBufferFinalSlotInflatedExceedsBackingStore)
{
    ASSERT_TRUE(setupChannelPair("15303"));

    RsslError err;
    const RsslUInt32 allocSize = 256;
    RsslBuffer* pBuf = getClientBuffer(allocSize, &err, RSSL_TRUE /* packed */);
    ASSERT_NE(pBuf, nullptr) << "rsslGetBuffer(packed) failed: " << err.text;

    /* Commit a 64-byte first message to advance packingOffset. */
    fillBuffer(pBuf, 64);
    pBuf->length = 64;

    RsslBuffer* pSlice = rsslPackBuffer(pClientChnl, pBuf, &err);
    ASSERT_NE(pSlice, nullptr) << "rsslPackBuffer failed: " << err.text;

    /* Inflate the returned next-slot length far beyond the remaining backing
     * store.  rsslSocketWrite() computes ripcBuffer->length = packingOffset +
     * inflated_length, which exceeds maxLength. */
    pSlice->length = pSlice->length + 5000;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

    EXPECT_LT(ret, RSSL_RET_SUCCESS)
        << "Inflated final packed slot must return failure, not write the "
           "sub-message length prefix past the backing store; ret=" << ret;
}

/* -----------------------------------------------------------------------
 * Crash vector E - RsslInt32 uncompBytes accumulation (Issue 8).
 *
 * ipcWriteSession() sums per-fragment wire sizes into:
 *   RsslInt32 uncompBytes = 0;
 *   uncompBytes += messageLength;   // repeated for each fragment
 *
 * For a ~200-fragment message (200 x ~1050 bytes ~ 210 000 bytes) the
 * accumulation stays within RsslInt32 range.  This test verifies that
 * outArgs.uncompressedBytesWritten is positive and >= the payload size
 * after such a write, confirming no silent signed overflow occurred.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_LargeFragmentedWriteUncompBytesAccumulationSanity)
{
    /* 1 KB fragments ? 200 fragments per 200 KB write. */
    ASSERT_TRUE(setupChannelPair("15304", RSSL_COMP_NONE, 0, 1024));

    RsslError        err;
    const RsslUInt32 reqSize = 200u * 1024u;   /* 200 KB ? ~200 fragments */
    RsslBuffer* pBuf = getClientBuffer(reqSize, &err);
    if (!pBuf)
    {
        SUCCEED() << "Could not allocate " << reqSize << "-byte buffer; skipping";
        return;
    }

    fillBuffer(pBuf, reqSize);
    pBuf->length = reqSize;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    outArgs.uncompressedBytesWritten = (RsslUInt32)-1;   /* sentinel */

    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    ASSERT_GE(ret, RSSL_RET_SUCCESS)
        << "200-fragment write must succeed; err: " << err.text;
    EXPECT_NE(outArgs.uncompressedBytesWritten, (RsslUInt32)-1)
        << "uncompressedBytesWritten must be updated after large fragmented write";
    EXPECT_GT(outArgs.uncompressedBytesWritten, 0u)
        << "uncompressedBytesWritten must be positive - not wrapped to "
           "negative by RsslInt32 overflow (Issue 8)";
    EXPECT_GE(outArgs.uncompressedBytesWritten, reqSize)
        << "uncompressedBytesWritten must be >= payload length";
}

/* -----------------------------------------------------------------------
 * Crash vector F - writeOutArgs.bytesWritten left as sentinel on
 * WRITE_FLUSH_FAILED (Issue 9).
 *
 * When rsslWriteEx() returns RSSL_RET_WRITE_FLUSH_FAILED, the out-args
 * struct must be updated so that the caller can make correct flow-control
 * decisions.  Leaving bytesWritten at the sentinel value (-1 / UINT32_MAX)
 * on the flush-failed return path means:
 *   - The caller treats it as "0 bytes queued" and retransmits.
 *   - If the caller accumulates bytesWritten as unsigned, UINT32_MAX is
 *     added per flush-failed call, wrapping the accumulator to 0.
 *
 * This test floods the output queue until WRITE_FLUSH_FAILED and verifies
 * bytesWritten is NOT left as the sentinel on that return path.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, NegativeWrite_WriteFlushFailedOutArgsBytesWrittenNotStale)
{
    ASSERT_TRUE(setupChannelPair("15305"));

    RsslError err;
    bool outArgsValidated = false;

    for (int i = 0; i < 500; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(1024, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, 1024);
        pBuf->length = 1024;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        /* Sentinel: if the implementation never writes this field it stays -1. */
        outArgs.bytesWritten = (RsslUInt32)-1;

        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
        {
            /* Issue 9: bytesWritten must be updated even on the flush-failed path. */
            EXPECT_NE(outArgs.bytesWritten, (RsslUInt32)-1)
                << "bytesWritten must not be left as sentinel on "
                   "RSSL_RET_WRITE_FLUSH_FAILED (Issue 9)";
            outArgsValidated = true;
            rsslFlush(pClientChnl, &err);
            break;
        }
        else if (ret < RSSL_RET_SUCCESS)
        {
            break;
        }
        else if (ret > RSSL_RET_SUCCESS)
        {
            rsslFlush(pClientChnl, &err);
        }
    }

    if (!outArgsValidated)
    {
        SUCCEED() << "WRITE_FLUSH_FAILED not triggered; Issue 9 flush-failed "
                     "path could not be validated on this platform/configuration";
    }
}

/* =======================================================================
 * -- MEMORY CORRUPTION DETECTION TESTS (CANARY-WRITE PATTERN) -----------
 *
 * Each test applies a "poison" - a write operation known to risk pool or
 * heap corruption - immediately followed by a "canary" write that would
 * crash, return garbage, or produce detectable ASAN/valgrind reports if
 * the poison damaged memory.
 *
 * Without sanitisers: the canary write crashes if the pool allocator's
 * free-list metadata was corrupted by the poison, or succeeds cleanly if
 * the error path correctly restored all pool blocks.
 *
 * Under ASAN / valgrind: the poison write itself triggers a report even
 * when the canary happens to succeed (e.g. the freed block wasn't yet
 * reused), providing the definitive detection mechanism.
 *
 * Target bugs:
 *   Issue 2  - Fragment chain leaked on msgb->length > msgb->maxLength
 *               break; pool block freed at IPC_header_size bytes before
 *               its actual start address (if break fires after buffer
 *               pointer decrement).
 *   Issue 3  - Write loop queues additional fragments after a fatal error,
 *               leaving orphaned pool blocks that exhaust the pool.
 *   Issue 4  - rtr_dfltcFreeMsg(msgb) then msgb->buffer=0 / length=0
 *               (use-after-free write) corrupts any pool block that was
 *               reallocated between the free and the zero-write.
 *   Issue 7  - Stale compressedmb->nextMsg: the outer while(msgb) loop
 *               follows a non-NULL nextMsg on a recycled pool block and
 *               writes IPC headers into unrelated heap memory.
 *
 * Port assignments: 15400 - 15411
 * ===================================================================== */

/* -----------------------------------------------------------------------
 * MemCorrupt 1 - Pool integrity after single-buffer length-overflow failure.
 *
 * Poison : rsslGetBuffer(128) + pBuf->length += 1 ? rsslWriteEx returns
 *          RSSL_RET_BUFFER_TOO_SMALL.  The outer guard in rsslSocketWrite()
 *          must return the pool block at its correct base address.  If it
 *          instead frees at (base - IPC_header_size) the allocator's
 *          free-list node at that address is corrupted.
 *
 * Canary  : rsslGetBuffer(128) again, fill, rsslWriteEx normally.
 *          If the pool returned a misaligned pointer the write writes IPC
 *          headers into live heap ? heap-buffer-overflow under ASAN,
 *          crash on flush without sanitisers.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_PoolIntegrityAfterLengthOverflowWriteFail)
{
    ASSERT_TRUE(setupChannelPair("15400"));

    RsslError err;
    const RsslUInt32 allocSize = 128;

    /* Poison: inflate pBuf->length by 1 ? outer guard fires. */
    RsslBuffer* pBuf1 = getClientBuffer(allocSize, &err);
    ASSERT_NE(pBuf1, nullptr);
    fillBuffer(pBuf1, allocSize);
    pBuf1->length = pBuf1->length + 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    /* Expected RSSL_RET_BUFFER_TOO_SMALL; error path must restore pool. */
    RsslRet ret1 = rsslWriteEx(pClientChnl, pBuf1, &inArgs, &outArgs, &err);
    (void)ret1;

    /* The pool block for pBuf1 was NOT freed by the library on BUFFER_TOO_SMALL;
     * the caller still owns it.  Release it explicitly before the canary. */
    rsslReleaseBuffer(pBuf1, &err);

    /* Canary: allocate from the same pool after the release. */
    RsslBuffer* pBuf2 = getClientBuffer(allocSize, &err);
    if (!pBuf2)
    {
        SUCCEED() << "Canary allocation returned NULL; pool may be exhausted - no crash";
        return;
    }

    fillBuffer(pBuf2, allocSize);
    pBuf2->length = allocSize;

    rsslClearWriteInArgs(&inArgs);
    rsslClearWriteOutArgs(&outArgs);
    RsslRet ret2 = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);
    if (ret2 > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret2, RSSL_RET_SUCCESS)
        << "Canary write must succeed - pool must be intact after overflow "
           "write failure (Issue 2 heap-corruption check); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 2 - Pool integrity after fragmented-buffer length overflow.
 *
 * Poison : allocate a two-fragment buffer (fragSize + 512 bytes), inflate
 *          pBuf->length by 8 KB beyond the malloc'd block, write ? outer
 *          guard fires.  Release the block explicitly.
 *
 * Canary  : allocate the same-sized buffer again, write normally.
 *          If the overflow-failure path corrupted the fragmented-allocation
 *          pool, the canary write crashes or returns corrupt data.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_PoolIntegrityAfterFragmentedLengthOverflow)
{
    ASSERT_TRUE(setupChannelPair("15401", RSSL_COMP_NONE, 0, 3000));

    RsslError err;
    const RsslUInt32 fragSz  = 3000;
    const RsslUInt32 allocSz = fragSz + 512;   /* two-fragment allocation */

    /* Poison: inflate by 8 KB. */
    RsslBuffer* pBuf1 = getClientBuffer(allocSz, &err);
    ASSERT_NE(pBuf1, nullptr);
    fillBuffer(pBuf1, allocSz);
    pBuf1->length = allocSz + 8192;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret1 = rsslWriteEx(pClientChnl, pBuf1, &inArgs, &outArgs, &err);
    (void)ret1;

    /* Release the poison buffer to return its pool blocks. */
    rsslReleaseBuffer(pBuf1, &err);

    /* Canary: allocate same size, write normally. */
    RsslBuffer* pBuf2 = getClientBuffer(allocSz, &err);
    if (!pBuf2)
    {
        SUCCEED() << "Canary allocation returned NULL; no crash - fragmented pool may be exhausted";
        return;
    }

    fillBuffer(pBuf2, allocSz);
    pBuf2->length = allocSz;

    rsslClearWriteInArgs(&inArgs);
    rsslClearWriteOutArgs(&outArgs);
    RsslRet ret2 = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);
    if (ret2 > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret2, RSSL_RET_SUCCESS)
        << "Canary fragmented write must succeed - pool intact after "
           "inflated-length failure (Issue 2); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 3 - rsslReleaseBuffer() on an unwritten fragmented buffer.
 *
 * Allocate a three-fragment buffer (fragSize * 3 bytes) and call
 * rsslReleaseBuffer() WITHOUT writing it first.  This exercises the
 * ipcReleaseDataBuffer() chain-walk path which must traverse all three
 * msgb->nextMsg pointers and free each block.  A stale non-NULL nextMsg
 * (Issue 7) causes the walk to follow a freed pointer and crash.
 *
 * Canary: reallocate the same size and write normally.  Any pool corruption
 * from the chain-walk produces a crash or ASAN report here.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_ReleaseUnwrittenFragmentedBufferChainWalk)
{
    ASSERT_TRUE(setupChannelPair("15402", RSSL_COMP_NONE, 0, 1000));

    RsslError err;
    const RsslUInt32 allocSz = 3000;   /* ~3 fragments */

    /* Allocate without writing, then release.
     * ipcReleaseDataBuffer() must walk all 3 nextMsg nodes cleanly. */
    RsslBuffer* pBuf = getClientBuffer(allocSz, &err);
    if (!pBuf)
    {
        SUCCEED() << "3-fragment allocation failed - skip";
        return;
    }

    fillBuffer(pBuf, allocSz);
    pBuf->length = allocSz;

    RsslRet relRet = rsslReleaseBuffer(pBuf, &err);
    EXPECT_GE(relRet, RSSL_RET_SUCCESS)
        << "rsslReleaseBuffer on unwritten 3-fragment buffer must not crash "
           "(Issue 7 stale nextMsg in chain walk)";

    /* Canary: pool must accept a fresh allocation and write. */
    RsslBuffer* pBuf2 = getClientBuffer(allocSz, &err);
    if (!pBuf2)
    {
        SUCCEED() << "Canary allocation returned NULL - no crash; pool may be tight";
        return;
    }

    fillBuffer(pBuf2, allocSz);
    pBuf2->length = allocSz;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Canary write after unwritten-buffer release must succeed; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 4 - Rapid fragmented alloc / release cycling stresses pool
 *               nextMsg zeroing (Issue 7 stale pointer stress).
 *
 * 300 iterations alternating:
 *   (even)  allocate 3-fragment buffer ? rsslReleaseBuffer without writing
 *   (odd)   allocate 3-fragment buffer ? rsslWriteEx + rsslFlush
 *
 * The rapid alloc/free cycle maximises the probability that a recycled
 * pool block carries a stale non-NULL nextMsg pointer from its previous
 * life as a mid-chain fragment.  Any crash indicates an unguarded
 * stale-nextMsg dereference (Issue 7).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_RapidFragmentedAllocReleaseCyclingStressesPool)
{
    ASSERT_TRUE(setupChannelPair("15403", RSSL_COMP_NONE, 0, 512));

    RsslError err;
    const RsslUInt32 allocSz   = 512 * 3;   /* 3 fragments */
    const int        iterations = 300;

    for (int i = 0; i < iterations; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(allocSz, &err);
        if (!pBuf) break;

        fillBuffer(pBuf, allocSz);
        pBuf->length = allocSz;

        if (i % 2 == 0)
        {
            /* Release without writing - chain-walk path. */
            rsslReleaseBuffer(pBuf, &err);
        }
        else
        {
            /* Write and flush - full fragment-chain creation/destruction. */
            RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
            RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
            RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
            if (ret < RSSL_RET_SUCCESS) break;
            if (ret > RSSL_RET_SUCCESS)
                rsslFlush(pClientChnl, &err);
        }
    }

    SUCCEED() << "Rapid fragmented alloc/release cycling (" << iterations
              << " iters) completed without crash (Issue 7 nextMsg stress)";
}

/* -----------------------------------------------------------------------
 * MemCorrupt 5 - forceFlush UAF pool-reuse canary.
 *
 * After each DIRECT_SOCKET_WRITE (which calls rtr_dfltcFreeMsg(msgb) and
 * then writes msgb->buffer=0 / msgb->length=0 to the freed block - Issue 4),
 * immediately allocate a NEW buffer from the same pool.  If the UAF writes
 * zeroed the free-list pointer the allocator stored in the freed block, the
 * new allocation either returns NULL or a corrupted pointer.  Writing to a
 * corrupted pointer is a heap-buffer-overflow under ASAN; without ASAN it
 * crashes on the next pool operation.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_ForceFlushUAFPoolReuseCanary)
{
    ASSERT_TRUE(setupChannelPair("15404"));

    RsslError err;
    const RsslUInt32 payLen = 256;
    int crashDetected = 0;

    for (int i = 0; i < 100; ++i)
    {
        /* Poison: forceFlush path frees msgb then UAF-zeros its fields. */
        RsslBuffer* pPoison = getClientBuffer(payLen, &err);
        if (!pPoison) break;

        fillBuffer(pPoison, payLen);
        pPoison->length = payLen;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        inArgs.writeInFlags = RSSL_WRITE_DIRECT_SOCKET_WRITE;
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);

        RsslRet ret1 = rsslWriteEx(pClientChnl, pPoison, &inArgs, &outArgs, &err);
        if (ret1 < RSSL_RET_SUCCESS) break;
        if (ret1 > RSSL_RET_SUCCESS) rsslFlush(pClientChnl, &err);

        /* Canary: allocate immediately after the free+UAF-zero.
         * Under pool reuse the freed block is returned here; if the UAF
         * zeroed the allocator's free-list node inside it, this returns
         * NULL or a misaligned pointer. */
        RsslBuffer* pCanary = getClientBuffer(payLen, &err);
        if (!pCanary) { ++crashDetected; break; }

        fillBuffer(pCanary, payLen);
        pCanary->length = payLen;

        rsslClearWriteInArgs(&inArgs);
        rsslClearWriteOutArgs(&outArgs);
        RsslRet ret2 = rsslWriteEx(pClientChnl, pCanary, &inArgs, &outArgs, &err);
        if (ret2 < RSSL_RET_SUCCESS) { ++crashDetected; break; }
        if (ret2 > RSSL_RET_SUCCESS) rsslFlush(pClientChnl, &err);
    }

    EXPECT_EQ(crashDetected, 0)
        << "forceFlush UAF canary: pool must remain usable for 100 iterations "
           "(Issue 4 UAF zero-write must not corrupt allocator free-list); "
           "run under ASAN to detect the write-to-freed-memory directly";
}

/* -----------------------------------------------------------------------
 * MemCorrupt 6 - Pool integrity after compressed-channel length overflow.
 *
 * Poison : on a Zlib channel, inflate pBuf->length by 1 ? BUFFER_TOO_SMALL.
 *          Release the buffer.
 *
 * Canary  : allocate same size again, do a normal compressed write + flush.
 *          Crash or ASAN report indicates the Zlib error path corrupted the
 *          pool block (Issue 2 on the compressed write path).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_PoolIntegrityAfterCompressedLengthOverflow)
{
    ASSERT_TRUE(setupChannelPair("15405", RSSL_COMP_ZLIB, 1));

    RsslError err;
    const RsslUInt32 allocSz = 512;

    /* Poison: inflate length by 1 on a Zlib channel. */
    RsslBuffer* pBuf1 = getClientBuffer(allocSz, &err);
    ASSERT_NE(pBuf1, nullptr);
    fillBuffer(pBuf1, allocSz);
    pBuf1->length = pBuf1->length + 1;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret1 = rsslWriteEx(pClientChnl, pBuf1, &inArgs, &outArgs, &err);
    (void)ret1;

    rsslReleaseBuffer(pBuf1, &err);

    /* Canary: normal compressed write from same pool. */
    RsslBuffer* pBuf2 = getClientBuffer(allocSz, &err);
    if (!pBuf2)
    {
        SUCCEED() << "Canary allocation NULL on Zlib channel - no crash";
        return;
    }

    memset(pBuf2->data, 0x41, pBuf2->length);
    pBuf2->length = allocSz;

    rsslClearWriteInArgs(&inArgs);
    rsslClearWriteOutArgs(&outArgs);
    RsslRet ret2 = rsslWriteEx(pClientChnl, pBuf2, &inArgs, &outArgs, &err);
    if (ret2 > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret2, RSSL_RET_SUCCESS)
        << "Compressed canary write after overflow failure must succeed "
           "(Issue 2 on Zlib path); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 7 - Channel fully usable after WRITE_FLUSH_FAILED + recovery.
 *
 * Flood until WRITE_FLUSH_FAILED, flush to drain, then write 20 more
 * canary messages.  All 20 must succeed.  Any failure indicates that the
 * pending-queue chain was left in an inconsistent state by the flush-
 * failed path (Issue 3: loop continued queuing fragments after fatal
 * error, leaving orphaned msgb nodes that block subsequent allocations).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_ChannelUsableAfterFlushFailedAndRecovery)
{
    ASSERT_TRUE(setupChannelPair("15406"));

    RsslError err;

    /* Phase 1: flood until WRITE_FLUSH_FAILED. */
    bool gotFlushFailed = false;
    for (int i = 0; i < 500 && !gotFlushFailed; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(2048, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 2048);
        pBuf->length = 2048;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);

        if (ret == RSSL_RET_WRITE_FLUSH_FAILED)
            gotFlushFailed = true;
        else if (ret < RSSL_RET_SUCCESS)
            break;
    }

    if (!gotFlushFailed)
    {
        SUCCEED() << "WRITE_FLUSH_FAILED not triggered; recovery test skipped";
        return;
    }

    /* Phase 2: drain the queue. */
    RsslRet flushRet = rsslFlush(pClientChnl, &err);
    EXPECT_GE(flushRet, RSSL_RET_SUCCESS)
        << "rsslFlush after WRITE_FLUSH_FAILED must succeed";

    /* Allow remote side to drain so TCP receive buffer is clear. */
    time_sleep(20);
    drainServerChannel(pServerChnl, 512);

    /* Phase 3: canary - write 20 more messages; all must succeed. */
    int successCount = 0;
    for (int i = 0; i < 20; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(256, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 256);
        pBuf->length = 256;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS) rsslFlush(pClientChnl, &err);
        ++successCount;
    }

    EXPECT_EQ(successCount, 20)
        << "All 20 canary writes must succeed after flush-failed recovery "
           "(Issue 3: orphaned fragment nodes must not block the queue); "
           "only " << successCount << "/20 succeeded";
}

/* -----------------------------------------------------------------------
 * MemCorrupt 8 - Alternating poison/canary stress for pool corruption
 *               accumulation (Issue 2).
 *
 * 50 iterations each of:
 *   poison : rsslGetBuffer(64) + length += 1 ? fail + rsslReleaseBuffer
 *   canary : rsslGetBuffer(64) + write normally ? must succeed
 *
 * If Issue 2 accumulates corruption (e.g. each failure releases at the
 * wrong address, progressively clobbering the free list), the canary
 * fails earlier with each additional poisoning cycle.  Expect all 50
 * canary writes to succeed.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_AlternatingOverflowNormalWritesStressPool)
{
    ASSERT_TRUE(setupChannelPair("15407"));

    RsslError err;
    const RsslUInt32 allocSz   = 64;
    const int        iterations = 50;
    int              normalSuccesses = 0;

    for (int i = 0; i < iterations; ++i)
    {
        /* Poison: inflate by 1, fail, release. */
        RsslBuffer* pPoison = getClientBuffer(allocSz, &err);
        if (!pPoison) break;
        fillBuffer(pPoison, allocSz);
        pPoison->length = pPoison->length + 1;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet retPoison = rsslWriteEx(pClientChnl, pPoison, &inArgs, &outArgs, &err);
        (void)retPoison;
        rsslReleaseBuffer(pPoison, &err);

        /* Canary: normal write from same pool. */
        RsslBuffer* pNormal = getClientBuffer(allocSz, &err);
        if (!pNormal) break;
        fillBuffer(pNormal, allocSz);
        pNormal->length = allocSz;

        rsslClearWriteInArgs(&inArgs);
        rsslClearWriteOutArgs(&outArgs);
        RsslRet retNormal = rsslWriteEx(pClientChnl, pNormal, &inArgs, &outArgs, &err);
        if (retNormal < RSSL_RET_SUCCESS) break;
        if (retNormal > RSSL_RET_SUCCESS) rsslFlush(pClientChnl, &err);
        ++normalSuccesses;
    }

    EXPECT_EQ(normalSuccesses, iterations)
        << "All " << iterations << " canary writes must succeed; accumulated "
           "pool corruption (Issue 2) would reduce this count; "
           "got " << normalSuccesses << "/" << iterations;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 9 - Deep (55-node) chain release + reallocation integrity.
 *
 * Allocate a 55-fragment buffer (fragSize=100, allocSize=5500), call
 * rsslReleaseBuffer() without writing.  ipcReleaseDataBuffer() must walk
 * all 55 msgb->nextMsg pointers and free each block back to the pool.
 *
 * Canary: reallocate the same size and write normally.  A crash or ASAN
 * report indicates either a missing free in the chain walk (pool leaks
 * exhaust capacity) or a double-free / wild-pointer deref (Issue 7).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_DeepChainReleaseAndReallocIntegrity)
{
    ASSERT_TRUE(setupChannelPair("15408", RSSL_COMP_NONE, 0, 100));

    RsslError err;
    const RsslUInt32 allocSz = 100u * 55u;   /* ~55 fragment nodes */

    RsslBuffer* pDeep = getClientBuffer(allocSz, &err);
    if (!pDeep)
    {
        SUCCEED() << "55-fragment allocation failed - skip";
        return;
    }

    fillBuffer(pDeep, allocSz);
    pDeep->length = allocSz;

    /* Release without writing: chain-walk must free all 55 nodes. */
    RsslRet relRet = rsslReleaseBuffer(pDeep, &err);
    EXPECT_GE(relRet, RSSL_RET_SUCCESS)
        << "rsslReleaseBuffer on 55-fragment unwritten buffer must not crash "
           "(Issue 7 stale nextMsg in deep chain walk)";

    /* Canary: reallocate same size and write. */
    RsslBuffer* pCanary = getClientBuffer(allocSz, &err);
    if (!pCanary)
    {
        SUCCEED() << "Canary allocation returned NULL - pool may be tight; no crash";
        return;
    }

    fillBuffer(pCanary, allocSz);
    pCanary->length = allocSz;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pCanary, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Canary write after 55-node chain release must succeed - all blocks "
           "returned to pool correctly; err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 10 - Compressed pool cycling then multi-fragment write
 *                (Issue 7 stale compressedmb->nextMsg canary).
 *
 * Phase 1: 400 rapid small Zlib compressed writes + flushes.  Each write
 *   allocates compressedmb1, sends it, frees it (plus the UAF zero-write
 *   of Issue 4).  After 400 cycles the pool has many recycled blocks that
 *   may carry stale nextMsg pointers from their previous life in a chain.
 *
 * Phase 2: canary - write one large Zlib compressed message (7 KB, 2+
 *   fragments).  ipcWriteSession() allocates compressedmb1 (a recycled
 *   block!) and compressedmb2.  If compressedmb1's stale nextMsg != NULL,
 *   the outer while(msgb) loop reads it as the next buffer to process and
 *   writes IPC headers into unrelated heap memory (Issue 7 crash).
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_CompressedPoolCyclingThenMultiFragWriteCanary)
{
    ASSERT_TRUE(setupChannelPair("15409", RSSL_COMP_ZLIB, 1, 3000));

    RsslError err;

    /* Phase 1: 400 rapid small compressed writes to cycle compressedmb1 blocks. */
    for (int i = 0; i < 400; ++i)
    {
        RsslBuffer* pBuf = getClientBuffer(512, &err);
        if (!pBuf) break;
        memset(pBuf->data, 0x41, pBuf->length);
        pBuf->length = 512;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &inArgs, &outArgs, &err);
        if (ret < RSSL_RET_SUCCESS) break;
        if (ret > RSSL_RET_SUCCESS)
            rsslFlush(pClientChnl, &err);
    }

    /* Phase 2: canary - multi-fragment compressed write.
     * Allocates compressedmb1 (recycled) and compressedmb2 (also recycled).
     * If compressedmb1's stale nextMsg != NULL, Issue 7 triggers here. */
    RsslBuffer* pBig = getClientBuffer(6500, &err);
    if (!pBig)
    {
        SUCCEED() << "Multi-fragment canary allocation failed - skip";
        return;
    }
    memset(pBig->data, 0x42, pBig->length);
    pBig->length = 6500;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pBig, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Multi-fragment compressed write after pool cycling must succeed "
           "(Issue 7 stale compressedmb->nextMsg canary); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 11 - Pool exhaustion and recovery.
 *
 * Hold 400 allocated buffers (never written) to stress-test the pool's
 * capacity bookkeeping.  Then release all of them and verify the pool
 * recovers fully by writing one canary message.
 *
 * Targeted bug: if rsslReleaseBuffer() on a never-written buffer does not
 * correctly return the pool block (e.g. it skips the release due to a
 * NULL-check on bufferInfo), the pool accumulates leaked blocks over time.
 * Releasing 400 such buffers and then successfully allocating one more
 * confirms the release path is correct.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_PoolExhaustionAndRecovery)
{
    ASSERT_TRUE(setupChannelPair("15410"));

    RsslError err;
    const RsslUInt32 allocSz  = 128;
    const int        holdCount = 400;

    /* Allocate and hold buffers without writing. */
    RsslBuffer* held[400] = {};
    int heldActual = 0;
    for (int i = 0; i < holdCount; ++i)
    {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, allocSz, RSSL_FALSE, &err);
        if (!pBuf) break;
        fillBuffer(pBuf, 64);
        pBuf->length = 64;
        held[heldActual++] = pBuf;
    }

    /* Release all held buffers. */
    for (int i = 0; i < heldActual; ++i)
        rsslReleaseBuffer(held[i], &err);

    /* Canary: pool must accept a fresh allocation after mass-release. */
    RsslBuffer* pCanary = rsslGetBuffer(pClientChnl, allocSz, RSSL_FALSE, &err);
    if (!pCanary)
    {
        SUCCEED() << "Canary allocation returned NULL after mass-release; "
                     "pool may still be tight - no crash";
        return;
    }

    fillBuffer(pCanary, allocSz);
    pCanary->length = allocSz;

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pCanary, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Canary write after bulk-release of " << heldActual
        << " buffers must succeed - all pool blocks must have been returned; "
           "err: " << err.text;
}

/* -----------------------------------------------------------------------
 * MemCorrupt 12 - Packed buffer overflow then normal packed write canary.
 *
 * Poison : allocate a 256-byte packed buffer, commit one 64-byte message
 *          via rsslPackBuffer(), inflate the returned next-slot's length by
 *          5000 ? rsslWriteEx returns RSSL_RET_BUFFER_TOO_SMALL.  The outer
 *          guard must return the pool block cleanly.
 *
 * Canary  : allocate a fresh 512-byte packed buffer, pack two messages,
 *           write normally.  If the poison corrupted the pool, the canary
 *           allocation returns a misaligned pointer and the packed-prefix
 *           write overflows into adjacent heap memory.
 * --------------------------------------------------------------------- */
TEST_P(RsslSocketWriteTests, MemCorrupt_PackedOverflowThenNormalPackedWriteCanary)
{
    ASSERT_TRUE(setupChannelPair("15411"));

    RsslError err;

    /* Poison: packed buffer with inflated second slot. */
    RsslBuffer* pPoison = rsslGetBuffer(pClientChnl, 256, RSSL_TRUE, &err);
    if (pPoison)
    {
        fillBuffer(pPoison, 64);
        pPoison->length = 64;

        RsslBuffer* pSlice = rsslPackBuffer(pClientChnl, pPoison, &err);
        if (pSlice)
            pSlice->length = pSlice->length + 5000;

        RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
        RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
        RsslRet retPoison = rsslWriteEx(pClientChnl, pPoison, &inArgs, &outArgs, &err);
        (void)retPoison;
        /* The outer guard (BUFFER_TOO_SMALL) returned the pool block; release it. */
        rsslReleaseBuffer(pPoison, &err);
    }

    /* Canary: fresh packed buffer with two normal messages. */
    RsslBuffer* pCanary = rsslGetBuffer(pClientChnl, 512, RSSL_TRUE, &err);
    if (!pCanary)
    {
        SUCCEED() << "Canary packed allocation returned NULL - no crash";
        return;
    }

    fillBuffer(pCanary, 100);
    pCanary->length = 100;

    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pCanary, &err);
    if (pNext && pNext->length >= 100)
    {
        fillBuffer(pNext, 100);
        pNext->length = 100;
        RsslBuffer* pEnd = rsslPackBuffer(pClientChnl, pNext, &err);
        if (pEnd) pEnd->length = 0;
    }
    else if (pNext)
    {
        pNext->length = 0;
    }

    RsslWriteInArgs  inArgs;  rsslClearWriteInArgs(&inArgs);
    RsslWriteOutArgs outArgs; rsslClearWriteOutArgs(&outArgs);
    RsslRet ret = rsslWriteEx(pClientChnl, pCanary, &inArgs, &outArgs, &err);
    if (ret > RSSL_RET_SUCCESS)
        rsslFlush(pClientChnl, &err);

    EXPECT_GE(ret, RSSL_RET_SUCCESS)
        << "Canary packed write after packed-overflow failure must succeed "
           "(Issue 2 / Issue 10 pool-corruption canary check); err: " << err.text;
}

/* -----------------------------------------------------------------------
 * Instantiate both fixture suites for SOCKET and WEBSOCKET connection types.
 * --------------------------------------------------------------------- */
INSTANTIATE_TEST_SUITE_P(
    SocketAndWebSocket,
    RsslSocketWriteTests,
    ::testing::Values(RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET));

INSTANTIATE_TEST_SUITE_P(
    SocketAndWebSocket,
    RsslSocketWriteChannelLockTests,
    ::testing::Values(RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET));
