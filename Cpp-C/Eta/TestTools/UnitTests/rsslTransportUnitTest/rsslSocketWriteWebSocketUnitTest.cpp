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
#include <iostream>
#include <string>
#include <vector>
#include <atomic>

#include "gtest/gtest.h"

#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslChanManagement.h"
#include "TransportUnitTest.h"
#include "rtr/ripc_int.h"

#if defined(_WIN32)
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#endif

/* Portable memmem: find needle in haystack. */
static const void* wsMemmem(const void* haystack, size_t haystackLen,
                             const void* needle,   size_t needleLen)
{
    if (needleLen == 0) return haystack;
    if (haystackLen < needleLen) return nullptr;
    const char* h = static_cast<const char*>(haystack);
    const char* n = static_cast<const char*>(needle);
    for (size_t i = 0; i <= haystackLen - needleLen; ++i)
        if (memcmp(h + i, n, needleLen) == 0) return h + i;
    return nullptr;
}

/* =========================================================================
* WebSocket fake-server thread state
 * ========================================================================= */
static const int kRxBufSize = 327680;

struct WsFakeServerArg
{
    unsigned short    port;
    std::atomic<bool> ready;
    std::atomic<bool> done;

    /* Bytes received after the WS+RIPC handshake phase */
    char              rxData[kRxBufSize];
    int               rxLen;

    /* Number of RSSL messages the server should wait for before closing */
    int               expectedMsgCount;

    /* Store the last error */
    char              errText[256];

    /* Compression settings negotiated during the RIPC handshake.
     * Leave as RSSL_COMP_NONE (0) for no compression.             */
    RsslCompTypes      compressionType;
    int                compressionLevel;

    /* When true the server calls rsslAccept with nakMount = RSSL_TRUE
     * to explicitly reject the incoming connection.                  */
    bool               nakMount;

    WsFakeServerArg()
        : port(0), ready(false), done(false),
          rxLen(0), expectedMsgCount(1),
          compressionType(RSSL_COMP_NONE), compressionLevel(0),
          nakMount(false)
    {
        memset(rxData, 0, sizeof(rxData));
        memset(errText, 0, sizeof(errText));
    }
};

/* =========================================================================
 * Fake WebSocket+RIPC server thread – uses the real RSSL server API so
 * that the full WebSocket + RIPC handshake is handled by the library.
 * After the channel reaches ACTIVE, it reads up to expectedMsgCount
 * messages and stores the payload bytes in rxData.
 * ========================================================================= */
static RSSL_THREAD_DECLARE(wsFakeServerThread, pArg)
{
    WsFakeServerArg* arg = reinterpret_cast<WsFakeServerArg*>(pArg);

    RsslError err;
    RsslBindOptions bindOpts;
    rsslClearBindOpts(&bindOpts);

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%u",
             static_cast<unsigned>(arg->port));
    bindOpts.serviceName             = portStr;
    bindOpts.connectionType          = RSSL_CONN_TYPE_WEBSOCKET;
    bindOpts.majorVersion            = RSSL_RWF_MAJOR_VERSION;
    bindOpts.minorVersion            = RSSL_RWF_MINOR_VERSION;
    bindOpts.wsOpts.protocols        = const_cast<char*>("rssl.json.v2");
    bindOpts.guaranteedOutputBuffers = 50;
    bindOpts.maxOutputBuffers        = 50;
    bindOpts.maxFragmentSize         = 6144;
    bindOpts.compressionType         = arg->compressionType;
    bindOpts.compressionLevel        = arg->compressionLevel;

    RsslServer* pSrv = rsslBind(&bindOpts, &err);
    if (!pSrv) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslBind failed: %s", err.text);
        arg->done = true; arg->ready = true;
        return 0;
    }
    arg->ready = true;

    /* Accept one connection */
    RsslAcceptOptions accOpts;
    rsslClearAcceptOpts(&accOpts);
    if (arg->nakMount)
        accOpts.nakMount = RSSL_TRUE;
    RsslChannel* pSrvChnl = nullptr;
    for (int i = 0; i < 500 && !pSrvChnl; ++i) {
        pSrvChnl = rsslAccept(pSrv, &accOpts, &err);
        if (!pSrvChnl) time_sleep(10);
    }
    if (!pSrvChnl) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslAccept failed: %s", err.text);
        rsslCloseServer(pSrv, &err);
        arg->done = true; return 0;
    }

    /* When nakMount is set the channel is expected to close quickly
     * after the rejection is sent to the client.                    */
    if (arg->nakMount) {
        for (int i = 0; i < 100; ++i) {
            if (pSrvChnl->state == RSSL_CH_STATE_CLOSED) break;
            RsslInProgInfo inProg;
            rsslClearInProgInfo(&inProg);
            rsslInitChannel(pSrvChnl, &inProg, &err);
            time_sleep(10);
        }
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true;
        return 0;
    }

    /* Drive server channel to ACTIVE */
    for (int i = 0; i < 400; ++i) {
        if (pSrvChnl->state == RSSL_CH_STATE_ACTIVE) break;
        if (pSrvChnl->state == RSSL_CH_STATE_CLOSED)  break;
        RsslInProgInfo inProg;
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pSrvChnl, &inProg, &err);
        time_sleep(5);
    }

    if (pSrvChnl->state != RSSL_CH_STATE_ACTIVE) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "Server channel not ACTIVE");
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true; return 0;
    }

    /* Read messages until expectedMsgCount or timeout */
    int msgCount  = 0;
    int waitedMs  = 0;
    const int maxWaitMs = 3000;
    while (msgCount < arg->expectedMsgCount && waitedMs < maxWaitMs) {
        RsslRet rRet;
        RsslBuffer* pInBuf = rsslRead(pSrvChnl, &rRet, &err);
        if (pInBuf && pInBuf->data && pInBuf->length > 0) {
            int copyLen = (int)pInBuf->length;
            if (arg->rxLen + copyLen > kRxBufSize - 1)
                copyLen = kRxBufSize - 1 - arg->rxLen;
            if (copyLen > 0) {
                memcpy(arg->rxData + arg->rxLen, pInBuf->data, copyLen);
                arg->rxLen += copyLen;
            }
            msgCount++;
        } else if (rRet == RSSL_RET_FAILURE) {
            break;
        } else {
            time_sleep(10);
            waitedMs += 10;
        }
    }

    rsslCloseChannel(pSrvChnl, &err);
    rsslCloseServer(pSrv, &err);
    arg->done = true;
    return 0;
}

/* =========================================================================
 * Drive rsslInitChannel on pChnl until ACTIVE or failure.
 * ========================================================================= */
static bool wsDriverToActive(RsslChannel* pChnl, RsslError* pErr,
                              int maxTries = 400, int sleepMs = 5)
{
    RsslInProgInfo inProg;
    for (int i = 0; i < maxTries; ++i) {
        if (pChnl->state == RSSL_CH_STATE_ACTIVE)  return true;
        if (pChnl->state == RSSL_CH_STATE_CLOSED ||
            pChnl->state == RSSL_CH_STATE_INACTIVE) return false;
        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pChnl, &inProg, pErr);
        if (ret == RSSL_RET_FAILURE || ret == RSSL_RET_CHAN_INIT_REFUSED)
            return false;
        time_sleep(sleepMs);
    }
    return (pChnl->state == RSSL_CH_STATE_ACTIVE);
}

/* =========================================================================
 * Flush the channel until the write queue is empty or an error occurs.
 * ========================================================================= */
static RsslRet wsFlushAll(RsslChannel* pChnl, RsslError* pErr,
                           int maxTries = 200, int sleepMs = 5)
{
    RsslRet ret = RSSL_RET_SUCCESS;
    for (int i = 0; i < maxTries; ++i) {
        ret = rsslFlush(pChnl, pErr);
        if (ret <= 0) break;
        time_sleep(sleepMs);
    }
    return ret;
}

/* =========================================================================
 * Test fixture
 * ========================================================================= */
class RsslSocketWriteWebSocketTests : public ::testing::Test
{
protected:
    RsslChannel*    pClientChnl = nullptr;
    RsslThreadId    fakeSrvTid;
    WsFakeServerArg fakeArg;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
        memset(&fakeSrvTid, 0, sizeof(fakeSrvTid));
    }

    void TearDown() override
    {
        for (int i = 0; i < 300 && !fakeArg.done.load(); ++i)
            time_sleep(10);

        RsslError err;
        if (pClientChnl) {
            rsslCloseChannel(pClientChnl, &err);
            pClientChnl = nullptr;
        }
        rsslUninitialize();

        resetDeadlockTimer();
    }

    bool startFakeServer()
    {
        RSSL_THREAD_START(&fakeSrvTid, wsFakeServerThread, &fakeArg);
        for (int i = 0; i < 500 && !fakeArg.ready.load(); ++i)
            time_sleep(2);
        return fakeArg.ready.load() && fakeArg.errText[0] == '\0';
    }

    bool connectClient(RsslCompTypes compressionType = RSSL_COMP_NONE)
    {
        char portStr[16];
        snprintf(portStr, sizeof(portStr), "%u",
                 static_cast<unsigned>(fakeArg.port));

        RsslError err;
        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_WEBSOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = portStr;
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.blocking                           = RSSL_FALSE;
        
        if (compressionType != RSSL_COMP_NONE)
        {
            opts.compressionType = compressionType;
        }
        
        /* Request rssl.json.v2 sub-protocol so the write goes through
         * rsslSocketWrite (the function under test). */
        opts.wsOpts.protocols                   = const_cast<char*>("rssl.json.v2");

        pClientChnl = rsslConnect(&opts, &err);
        return pClientChnl != nullptr;
    }

    /* Wait until the fake server has received at least minBytes in rxData. */
    bool waitForRxData(int minBytes, int maxWaitMs = 2000)
    {
        for (int i = 0; i < maxWaitMs / 10; ++i) {
            if (fakeArg.rxLen >= minBytes) return true;
            time_sleep(10);
        }
        return fakeArg.rxLen >= minBytes;
    }
};

/* =========================================================================
 * Test – Write a small message and verify receipt
 *
 * The client writes a 10-byte payload.  After flushing, the server should
 * receive at least the payload bytes (wrapped in a RIPC + WS frame).     */
TEST_F(RsslSocketWriteWebSocketTests, WriteSmallMessage_ServerReceivesData)
{
    fakeArg.port = 16100;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16100";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    bool active = wsDriverToActive(pClientChnl, &err);
    ASSERT_TRUE(active) << "Channel not ACTIVE: " << err.text;

    /* Get a buffer and write a small payload */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 10, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    const char* payload = "HelloWorld";
    memcpy(pBuf->data, payload, 10);
    pBuf->length = 10;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    /* Flush pending data */
    wsFlushAll(pClientChnl, &err);

    /* Wait for the server to receive at least 1 byte */
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    /* The server's rxData contains the raw WS frame bytes.  The RIPC header
     * is 3 bytes (length u16 + opcode u8) followed by the user payload.
     * Verify that the payload appears somewhere in the received bytes. */
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, 10) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in received WS frame bytes";
}

/* =========================================================================
 * Test – Write multiple small messages sequentially
 *
 * Write 5 messages of 8 bytes each.  Verify all payloads appear in the
 * server receive buffer.                                                  */
TEST_F(RsslSocketWriteWebSocketTests, WriteMultipleSmallMessages_AllReceived)
{
    fakeArg.port = 16101;
    fakeArg.expectedMsgCount = 5;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16101";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const int kMsgCount    = 5;
    const int kPayloadSize = 8;
    const char* payloads[kMsgCount] = {
        "Payload1", "Payload2", "Payload3", "Payload4", "Payload5"
    };

    for (int i = 0; i < kMsgCount; ++i) {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, kPayloadSize,
                                          RSSL_FALSE, &err);
        ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer[" << i << "] failed";
        memcpy(pBuf->data, payloads[i], kPayloadSize);
        pBuf->length = kPayloadSize;

        RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
        RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
        EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx[" << i << "] failed";
    }

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(kMsgCount * (kPayloadSize + 2)))
        << "Server did not receive all messages; rxLen=" << fakeArg.rxLen;

    for (int i = 0; i < kMsgCount; ++i) {
        bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen,
                             payloads[i], kPayloadSize) != nullptr);
        EXPECT_TRUE(found) << "Payload[" << i << "] not found in received data";
    }
}

/* =========================================================================
 * Test – Write with RSSL_WRITE_DIRECT_SOCKET_WRITE flag
 *
 * The forceFlush = 1 path in ipcWriteSession attempts to write the buffer
 * directly to the socket without queuing.  Verify the server still
 * receives the data.                                                      */
TEST_F(RsslSocketWriteWebSocketTests, WriteDirectSocketWrite_ServerReceivesData)
{
    fakeArg.port = 16102;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16102";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "DirectWrite!";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen,
                         payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in received WS frame bytes";
}

/* =========================================================================
 * Test – Write a message with maximum priority
 *
 * RSSL allows specifying priority (HIGH / MEDIUM / LOW).  Verify a write
 * with explicitly set high priority delivers the message.                */
TEST_F(RsslSocketWriteWebSocketTests, WriteHighPriority_ServerReceivesData)
{
    fakeArg.port = 16103;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16103";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "HighPriMsg";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    wIn.rsslPriority      = RSSL_HIGH_PRIORITY;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen,
                         payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in received WS frame bytes";
}

/* =========================================================================
 * Test – Write a message with medium priority
 *
 * Verify a write with RSSL_MEDIUM_PRIORITY delivers the payload.         */
TEST_F(RsslSocketWriteWebSocketTests, WriteMediumPriority_ServerReceivesData)
{
    fakeArg.port = 16105;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16105";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "MedPriMsg";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    wIn.rsslPriority      = RSSL_MEDIUM_PRIORITY;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in received WS frame bytes";
}

/* =========================================================================
 * Test – Write a message with low priority
 *
 * Verify a write with RSSL_LOW_PRIORITY delivers the payload.            */
TEST_F(RsslSocketWriteWebSocketTests, WriteLowPriority_ServerReceivesData)
{
    fakeArg.port = 16106;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16106";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "LowPriMsg";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    wIn.rsslPriority      = RSSL_LOW_PRIORITY;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in received WS frame bytes";
}

/* =========================================================================
 * Test – Write messages at all three priority levels sequentially
 *
 * One message is queued at each of HIGH / MEDIUM / LOW priority without
 * the DIRECT_SOCKET_WRITE flag so they all enter the output queue before
 * rsslFlush is called.  After flushing, all three payloads must appear in
 * the server receive buffer regardless of the order they were drained.   */
TEST_F(RsslSocketWriteWebSocketTests, WriteAllPriorities_AllMessagesReceived)
{
    fakeArg.port             = 16108;
    fakeArg.expectedMsgCount = 3;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16108";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    struct {
        const char*         payload;
        int                 payLen;
        RsslWritePriorities priority;
    } msgs[3] = {
        { "HighPri!", 8, RSSL_HIGH_PRIORITY   },
        { "MedPri!!", 8, RSSL_MEDIUM_PRIORITY },
        { "LowPri!!", 8, RSSL_LOW_PRIORITY    }
    };

    for (int i = 0; i < 3; ++i) {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, msgs[i].payLen, RSSL_FALSE, &err);
        ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer[" << i << "] failed";
        memcpy(pBuf->data, msgs[i].payload, msgs[i].payLen);
        pBuf->length = msgs[i].payLen;

        RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
        wIn.rsslPriority      = msgs[i].priority;
        RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
        EXPECT_GE(wRet, RSSL_RET_SUCCESS)
            << "rsslWriteEx[" << i << "] priority=" << msgs[i].priority
            << " failed: " << err.text;
    }

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(3 * (msgs[0].payLen + 2), 3000))
        << "Server did not receive all priority messages; rxLen=" << fakeArg.rxLen;

    for (int i = 0; i < 3; ++i) {
        bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen,
                               msgs[i].payload, msgs[i].payLen) != nullptr);
        EXPECT_TRUE(found)
            << "Priority payload[" << i << "] (\"" << msgs[i].payload
            << "\") not found in received data";
    }
}

/* =========================================================================
 * Invalid Flag Test IF1 – Unknown single flag bit
 *
 * writeInFlags = 0x08 is not a defined RsslWriteFlags value.  The library
 * must not crash; delivery is acceptable either way.                      */
TEST_F(RsslSocketWriteWebSocketTests, WriteInvalidFlags_UnknownBit_NoCrash)
{
    fakeArg.port             = 16250;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16250";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "UnknownFlag";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = static_cast<RsslWriteFlags>(0x08); /* undefined bit */
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    /* Must not crash; The server should receive the data */
    EXPECT_TRUE(wRet >= RSSL_RET_SUCCESS);
    wsFlushAll(pClientChnl, &err);
    waitForRxData(payLen);
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not received after write with unknown flag bit";
}

/* =========================================================================
 * Invalid Flag Test IF4 – DIRECT_SOCKET_WRITE combined with unknown bit
 *
 * Combines a valid flag (RSSL_WRITE_DIRECT_SOCKET_WRITE) with an
 * undefined bit to ensure the library still handles the known flag path
 * without crashing.                                                        */
TEST_F(RsslSocketWriteWebSocketTests, WriteInvalidFlags_DirectWritePlusUnknownBit_NoCrash)
{
    fakeArg.port             = 16253;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16253";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "DirectPlusUnknown";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    /* RSSL_WRITE_DIRECT_SOCKET_WRITE | an arbitrary unknown bit */
    wIn.writeInFlags = static_cast<RsslWriteFlags>(RSSL_WRITE_DIRECT_SOCKET_WRITE | 0x40);
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    EXPECT_EQ(RSSL_RET_SUCCESS, wRet) << "Expect to write data to the network without queing the message";
    wsFlushAll(pClientChnl, &err);
    waitForRxData(payLen);
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found)
        << "Payload not found after DIRECT_SOCKET_WRITE + unknown flag bit";
}

/* =========================================================================
 * Invalid Priority Test IP1 – Out-of-range priority value (99)
 *
 * rsslWebSocketWrite always overrides the caller's priority with
 * RSSL_HIGH_PRIORITY internally, so the message should still be
 * delivered.  Must not crash.                                              */
TEST_F(RsslSocketWriteWebSocketTests, WriteInvalidPriority_OutOfRange_NoCrash)
{
    fakeArg.port             = 16254;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16254";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "BadPriority99";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.rsslPriority      = static_cast<RsslWritePriorities>(99);
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    /* rsslWriteEx forces RSSL_MEDIUM_PRIORITY so delivery is expected */
    EXPECT_TRUE(wRet >= RSSL_RET_SUCCESS);
    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1, 3000))
        << "Server received no data for out-of-range priority write";
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found)
        << "Payload not found after write with priority=99";
}

/* =========================================================================
 * Test – Write a medium-sized message (512 bytes)
 *
 * Tests that messages larger than a typical MTU but smaller than the max
 * message size are delivered correctly.                                   */
TEST_F(RsslSocketWriteWebSocketTests, WriteMediumMessage_ServerReceivesData)
{
    fakeArg.port = 16104;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16104";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const int payLen = 512;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    /* Fill with a recognisable pattern */
    for (int i = 0; i < payLen; ++i)
        pBuf->data[i] = (char)((i % 26) + 'A');
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(payLen)) << "Server did not receive full message";

    /* Verify the first 26 bytes of the pattern appear in received data */
    char pattern[26];
    for (int i = 0; i < 26; ++i) pattern[i] = (char)(i + 'A');
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, pattern, 26) != nullptr);
    EXPECT_TRUE(found) << "Pattern not found in received WS frame bytes";
}

/* =========================================================================
 * Test – rsslWrite with zero-length user payload
 *
 * Writing a zero-length buffer must not crash and must return a
 * non-failure code (or an appropriate error).  Because the payload is
 * empty the RSSL library carries no user bytes inside the RIPC frame;
 * rsslRead on the server therefore returns no buffer with length > 0.
 * The test verifies:
 *   a) rsslWriteEx does not return RSSL_RET_FAILURE;
 *   b) the channel remains ACTIVE after the write;
 *   c) the server receives no user-payload bytes (rxLen == 0).           */
TEST_F(RsslSocketWriteWebSocketTests, WriteZeroLengthPayload_NoCrash)
{
    fakeArg.port             = 16107;
    /* A zero-length write carries no RSSL payload, so rsslRead on the
     * server never fires with length > 0.  Set expectedMsgCount to 0
     * so the server thread exits after its timeout rather than waiting
     * for a message that will never arrive with user data.              */
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16107";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    /* Allocate a 1-byte buffer but set its length to 0 before writing */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 1, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    pBuf->length = 0;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    /* Zero-length write must not crash; any non-FAILURE return is valid */
    EXPECT_GT(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx with zero-length payload returned more than zero" << err.text;

    wsFlushAll(pClientChnl, &err);

    /* (a) Channel must still be usable after the zero-length write */
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel state changed after zero-length write";

    /* Give the server a short window to process any stray bytes */
    time_sleep(200);

    /* (b) The server must not have received any user-payload bytes.
     * A zero-length RSSL message carries no data past the framing
     * headers, so rxLen should remain 0.                              */
    EXPECT_EQ(0, fakeArg.rxLen)
        << "Server unexpectedly received " << fakeArg.rxLen
        << " byte(s) for a zero-length write";
}

/* =========================================================================
 * Negative Write Test NW1 – Buffer length set to a value exceeding the
 *                            allocated buffer size
 *
 * rsslGetBuffer allocates a buffer of a known size.  Setting pBuf->length
 * to a value larger than the allocation before writing is an API misuse.
 * The library must detect this and return RSSL_RET_FAILURE.              */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_LengthExceedsAllocatedSize_ReturnsFailure)
{
    fakeArg.port = 16120;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16120";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const int allocSize = 8;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, allocSize, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    memcpy(pBuf->data, "SmallBuf", allocSize);
    /* Set length far beyond what was allocated */
    pBuf->length = static_cast<RsslUInt32>(allocSize) * 1000;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    EXPECT_EQ(RSSL_RET_BUFFER_TOO_SMALL, ret)
        << "Expected FAILURE when buffer length exceeds allocated size";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(8).") != NULL);
    ret = rsslReleaseBuffer(pBuf, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, ret);
}

/* =========================================================================
 * Negative Write Test NW2 – Buffer length set to a negative (wrap-around)
 *                            value
 *
 * RsslUInt32 length is unsigned so a "-1" assignment wraps to 0xFFFFFFFF.
 * The library must detect the absurd length and return RSSL_RET_FAILURE.  */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_NegativeLengthValue_ReturnsFailure)
{
    fakeArg.port = 16121;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16121";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 16, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    memcpy(pBuf->data, "NegLenTest!", 11);
    /* Assign wrap-around "negative" length */
    pBuf->length = static_cast<RsslUInt32>(-1);

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    EXPECT_EQ(RSSL_RET_BUFFER_TOO_SMALL, ret)
        << "Expected FAILURE when buffer length exceeds allocated size";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(16).") != NULL);
    ret = rsslReleaseBuffer(pBuf, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, ret);
}

/* =========================================================================
 * Negative Write Test NW3 – Buffer length set to get buffer size plus two bytes
 *
 * RsslUInt32 length is payload size plus two bytes
 * The library must detect the invalid length and return RSSL_RET_FAILURE.  */
TEST_F(RsslSocketWriteWebSocketTests, Write_LengthExceedsAllocatedSize_bytwo_ReturnsFailure)
{
    fakeArg.port = 16121;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16121";
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 16, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    memcpy(pBuf->data, "WriteMoreThanAllo", 17);
    pBuf->length = 17;

    RsslWriteInArgs  wIn = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    EXPECT_EQ(RSSL_RET_BUFFER_TOO_SMALL, ret)
        << "Expected FAILURE when buffer length exceeds allocated size";
    EXPECT_TRUE(strstr(err.text, "Error: 0008 Data has overflowed the allocated buffer length(16).") != NULL);
    ret = rsslReleaseBuffer(pBuf, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, ret);
}


/* =========================================================================
 * Negative Write Test NW4 – Write a buffer that has already been released
 *
 * After rsslReleaseBuffer() the buffer is returned to the pool.  Passing
 * the stale pointer to rsslWriteEx is undefined API usage.  The library
 * must not crash (the exact return code is implementation-defined).       */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_ReleasedBuffer_NoCrash)
{
    fakeArg.port = 16122;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16122";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 8, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    memcpy(pBuf->data, "Released", 8);
    pBuf->length = 8;

    /* Release the buffer back to the pool first */
    RsslRet relRet = rsslReleaseBuffer(pBuf, &err);
    ASSERT_EQ(RSSL_RET_SUCCESS, relRet) << "rsslReleaseBuffer failed";

    /* Now attempt to write the already-released buffer – must not crash */
    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);

    EXPECT_EQ(RSSL_RET_FAILURE, wRet)
        << "Expected FAILURE when the buffer is released.";
    EXPECT_TRUE(strstr(err.text, "Error: 0009 Buffer of length zero cannot be written") != NULL);
}

/* =========================================================================
 * Negative Write Test NW5 – rsslGetBuffer with size 0
 *
 * Requesting a zero-byte buffer is an API misuse.  The library must
 * return NULL or RSSL_RET_FAILURE without crashing.                       */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_GetBufferSizeZero_NullOrFailure)
{
    fakeArg.port = 16123;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16123";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 0, RSSL_FALSE, &err);

    EXPECT_EQ(nullptr, pBuf) << "Expected null RsslBuffer.";
    EXPECT_TRUE(strstr(err.text, "Error: 0010 Invalid buffer size specified.") != NULL);
}

/* =========================================================================
 * Negative Write Test NW6 – rsslGetBuffer with the UINT32_MAX bytes.
 *
 * Requesting a buffer larger than the negotiated maxMsgSize must be
 * rejected.  The library should return NULL without crashing.             */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_GetBufferOversized_NullOrFailure)
{
    fakeArg.port = 16124;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16124";
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    /* Request UINT32_MAX bytes – far beyond any negotiated maxMsgSize */
    const int oversize = static_cast<int>(0xFFFFFFFFu);
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, oversize, RSSL_FALSE, &err);

    EXPECT_EQ(nullptr, pBuf) << "Expected null RsslBuffer.";
    EXPECT_TRUE(strstr(err.text, "Error: 0010 Invalid buffer size specified.") != NULL);
}

/* =========================================================================
 * Compressed Write Test CW1 – Small message with ZLIB compression (level 0)
 *
 * The server is bound with RSSL_COMP_ZLIB / level 0.  After the RIPC
 * handshake both sides agree to compress.  The client writes a 10-byte
 * payload; rsslRead on the server delivers the decompressed bytes.        */
TEST_F(RsslSocketWriteWebSocketTests, WriteComp_ZlibLevel0_SmallMessage_ServerReceivesData)
{
    fakeArg.port             = 16140;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16140";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB))   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "CompSmall!";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found) << "Payload not found in decompressed server data";
}

/* =========================================================================
 * Compressed Write Test CW2 – Highly-compressible 512-byte message
 *
 * The payload is 512 bytes of the repeating character 'X'.  With ZLIB this
 * compresses to a fraction of the original size.  The test verifies:
 *   a) the server receives the decompressed pattern;
 *   b) bytesWritten reported by rsslWriteEx is less than the payload size
 *      (confirming that compression actually reduced the on-wire byte count).
 * Note: bytesWritten may equal payLen if the RIPC framing overhead exceeds
 * the compression saving for tiny messages, so (b) is a soft check.      */
TEST_F(RsslSocketWriteWebSocketTests, WriteComp_Zlib_HighlyCompressible_BytesWrittenLEPayload)
{
    fakeArg.port             = 16141;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 6;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16141";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB))   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const int payLen = 512;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memset(pBuf->data, 'X', payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    /* Verify the repeating 'X' pattern is present in decompressed data */
    char pattern[8];
    memset(pattern, 'X', sizeof(pattern));
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, pattern, sizeof(pattern)) != nullptr);
    EXPECT_TRUE(found) << "Repeating-X pattern not found in decompressed server data";

    /* Soft check: bytesWritten should be less than the raw payload size for
     * a highly-compressible 512-byte buffer.                               */
    EXPECT_LT(wOut.bytesWritten, static_cast<RsslUInt32>(payLen))
        << "Expected compressed bytesWritten < " << payLen
        << "; got " << wOut.bytesWritten
        << " (compression may not have engaged for this message size)";
}

/* =========================================================================
 * Compressed Write Test CW3 – Multiple messages with ZLIB compression
 *
 * Five 16-byte messages are written sequentially on a ZLIB-compressed
 * channel.  All five payloads must appear in the server's receive buffer. */
TEST_F(RsslSocketWriteWebSocketTests, WriteComp_Zlib_MultipleMessages_AllReceived)
{
    fakeArg.port             = 16142;
    fakeArg.expectedMsgCount = 5;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16142";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB))   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const int   kMsgCount = 5;
    const int   kPayLen   = 21;
    const char* payloads[kMsgCount] = {
        "CompPayload001_______", "CompPayload002_______",
        "CompPayload003_______", "CompPayload004_______",
        "CompPayload005_______"
    };

    for (int i = 0; i < kMsgCount; ++i) {
        RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, kPayLen, RSSL_FALSE, &err);
        ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer[" << i << "] failed";
        memcpy(pBuf->data, payloads[i], kPayLen);
        pBuf->length = kPayLen;

        RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
        RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
        RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
        EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx[" << i << "] failed";
        EXPECT_GT(wOut.uncompressedBytesWritten, wOut.bytesWritten) << "Uncompressed bytes written must be more than byte written";
    }

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(kMsgCount * kPayLen))
        << "Server did not receive all compressed messages; rxLen=" << fakeArg.rxLen;

    for (int i = 0; i < kMsgCount; ++i) {
        std::string message;
        message.append("[").append(payloads[i]).append("]");
        bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen,
                message.c_str(), kPayLen) != nullptr);
        EXPECT_TRUE(found)
            << "Compressed payload[" << i << "] not found in server data";
    }
}

/* =========================================================================
 * Compressed Write Test CW5 – RSSL_WRITE_DIRECT_SOCKET_WRITE with ZLIB
 *
 * Combines the direct-flush flag with server-side ZLIB compression to
 * verify that force-flush and compression interact correctly.             */
TEST_F(RsslSocketWriteWebSocketTests, WriteComp_Zlib_DirectSocketWrite_ServerReceivesData)
{
    fakeArg.port             = 16144;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16144";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB))   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    const char payload[] = "DirectCompWrite!";
    const int  payLen    = (int)strlen(payload);

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;
    memcpy(pBuf->data, payload, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, payload, payLen) != nullptr);
    EXPECT_TRUE(found)
        << "Payload not found after direct-socket-write with ZLIB compression";
}

/* =========================================================================
 * Compressed Write Test CW6 – Server requests RSSL_COMP_LZ4 (negative)
 *
 * RSSL_COMP_LZ4 may not be supported by all builds.  The test binds the
 * server with RSSL_COMP_LZ4 and attempts a full connection.  Acceptable
 * outcomes is:
 *   a) The connection succeeds and data flows normally. The client side ignores the RSSL_COMP_LZ4 
 *      connection type as it supports only RSSL_COMP_ZLIB.
 * In all cases there must be no crash.                                    */
TEST_F(RsslSocketWriteWebSocketTests, WriteComp_Neg_LZ4Compression_NoCrash)
{
    fakeArg.port             = 16145;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_LZ4;
    fakeArg.compressionLevel = 0;

    ASSERT_TRUE(startFakeServer());
    ASSERT_TRUE(connectClient(RSSL_COMP_LZ4));

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err, 100, 5)) << err.text;

    /* If we reached ACTIVE, attempt a normal write and verify no crash */
    const char payload[] = "LZ4TestMsg!";
    const int  payLen    = (int)strlen(payload);
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    if (pBuf) {
        memcpy(pBuf->data, payload, payLen);
        pBuf->length = payLen;
        RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
        RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
        RsslRet ret = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
        EXPECT_GT(ret, RSSL_RET_SUCCESS);

        /* There is no compression as RSSL_COMP_LZ4 is not supported for the websocket conneciton.*/
        EXPECT_EQ(wOut.uncompressedBytesWritten, wOut.bytesWritten);

        wsFlushAll(pClientChnl, &err);
    }
    SUCCEED();
}

/* =========================================================================
 * Negative Write Test NW8 – rsslReleaseBuffer called twice on the same
 *                            buffer (double-release)
 *
 * The first release returns the buffer to the pool.  The second call
 * passes a stale pointer; the library must not crash.                     */
TEST_F(RsslSocketWriteWebSocketTests, WriteNeg_DoubleReleaseBuffer_NoCrash)
{
    fakeArg.port = 16127;
    fakeArg.expectedMsgCount = 0;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16127";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 8, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed";
    memcpy(pBuf->data, "DblRel!!", 8);
    pBuf->length = 8;

    /* First release – expected to succeed */
    RsslRet firstRel = rsslReleaseBuffer(pBuf, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, firstRel) << "First rsslReleaseBuffer failed";

    /* Second release on the same stale pointer – must not crash */
    RsslRet secondRel = rsslReleaseBuffer(pBuf, &err);
    EXPECT_EQ(RSSL_RET_FAILURE, secondRel);
    EXPECT_TRUE(strstr(err.text, "Error: 0011 RSSL Buffer can not be released due to integrity issues.") != NULL);
}
 /*
 * Tests the boundary between non-fragmented and fragmented paths.  The
 * server only needs to receive at least 1 byte of payload to pass.       */
TEST_F(RsslSocketWriteWebSocketTests, WriteNearMaxSizeMessage_ServerReceivesData)
{
    fakeArg.port = 16110;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16110";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    /* Use a size just below the negotiated maxMsgSize (5888 - header ~ 5880) */
    const int payLen = 5000;
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer failed: " << err.text;

    memset(pBuf->data, 0xAB, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1)) << "Server received no data";

    /* Verify the fill byte 0xAB appears in received data */
    bool found = false;
    for (int i = 0; i < fakeArg.rxLen; ++i) {
        if ((unsigned char)fakeArg.rxData[i] == 0xAB) { found = true; break; }
    }
    EXPECT_TRUE(found) << "Fill byte 0xAB not found in received data";
}

/* =========================================================================
 * Large Message Test LM1 – 2 × maxFragmentSize payload
 *
 * rsslGetBuffer is called with 2 × maxFragmentSize.  RIPC splits the
 * message into at least two fragments.  rsslRead on the server
 * reassembles and delivers the complete 0xCD-filled payload.             */
TEST_F(RsslSocketWriteWebSocketTests, WriteLargeMessage_TwoFragments_ServerReceivesFullPayload)
{
    fakeArg.port             = 16160;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16160";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for 2×maxFragmentSize (" << payLen << "): "
        << err.text;

    memset(pBuf->data, 0x2A, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for 2×maxFragmentSize: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(payLen + 2, 4000)) << "Server received no data after flush";

    /* The server rxData holds the reassembled user payload.
     * Verify that the 0x2A fill byte is present. */
    EXPECT_EQ(payLen + 2, fakeArg.rxLen);
    EXPECT_EQ('[', fakeArg.rxData[0]);
    EXPECT_EQ(0x2A, fakeArg.rxData[1]) << "Fill byte 0x2A not found after 2-fragment reassembly; ";
    EXPECT_EQ(0x2A, fakeArg.rxData[fakeArg.rxLen-2]);
    EXPECT_EQ(']', fakeArg.rxData[fakeArg.rxLen - 1]);
}

/* =========================================================================
 * Large Message Test LM2 – 3 × maxFragmentSize payload
 *
 * Forces at least three RIPC fragments.  The server must reassemble all
 * three and return a single complete buffer via rsslRead.                 */
TEST_F(RsslSocketWriteWebSocketTests, WriteLargeMessage_ThreeFragments_ServerReceivesFullPayload)
{
    fakeArg.port             = 16161;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16161";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 3;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for 3×maxFragmentSize (" << payLen << "): "
        << err.text;

    /* Use an alternating pattern so we can detect byte-order issues */
    for (int i = 0; i < payLen; ++i)
        pBuf->data[i] = static_cast<char>((i & 1) ? 0x55 : 0xAA);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for 3×maxFragmentSize: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(payLen + 2, 4000)) << "Server received no data after flush";

    /* Verify at least one of the pattern bytes is present */
    EXPECT_EQ(payLen + 2, fakeArg.rxLen);
    EXPECT_EQ('[', fakeArg.rxData[0]);
    EXPECT_EQ((char)0xAA, fakeArg.rxData[1]) << "Fill byte 0xAA not found after 2-fragment reassembly; ";
    EXPECT_EQ((char)0x55, fakeArg.rxData[2]) << "Fill byte 0x55 not found after 2-fragment reassembly; ";
    EXPECT_EQ((char)0x55, fakeArg.rxData[fakeArg.rxLen - 2]);
    EXPECT_EQ(']', fakeArg.rxData[fakeArg.rxLen - 1]);
}

/* =========================================================================
 * Large Message Test LM3 – RSSL_WRITE_DIRECT_SOCKET_WRITE with fragmentation
 *
 * The DIRECT_SOCKET_WRITE flag requests an immediate socket send for
 * each fragment.  Combined with a 2×maxFragmentSize payload this exercises
 * the direct-flush + fragmentation code path together.                    */
TEST_F(RsslSocketWriteWebSocketTests, WriteLargeMessage_DirectFlush_ServerReceivesData)
{
    fakeArg.port             = 16162;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16162";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for direct-flush large message: " << err.text;

    memset(pBuf->data, 0xEF, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx (direct) failed for large message: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1, 4000)) << "Server received no data after direct flush";

    bool found = false;
    for (int i = 0; i < fakeArg.rxLen; ++i) {
        if ((unsigned char)fakeArg.rxData[i] == 0xEF) { found = true; break; }
    }
    EXPECT_TRUE(found) << "Fill byte 0xEF not found after direct-flush large message; "
                       << "rxLen=" << fakeArg.rxLen;
}

/* =========================================================================
 * Large Message Test LM4 – bytesWritten includes fragmentation overhead
 *
 * When RIPC fragments a message, each fragment carries its own RIPC and
 * WebSocket frame headers.  The total wire bytes reported in
 * wOut.bytesWritten must therefore be >= the user payload size.          */
TEST_F(RsslSocketWriteWebSocketTests, WriteLargeMessage_BytesWrittenGEPayloadSize)
{
    fakeArg.port             = 16163;
    fakeArg.expectedMsgCount = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16163";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for bytesWritten large message test: " << err.text;

    memset(pBuf->data, 'Z', payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for bytesWritten large message test: " << err.text;

    wsFlushAll(pClientChnl, &err);

    /* Wire bytes must be >= payload because framing headers are added */
    EXPECT_GE(wOut.bytesWritten, static_cast<RsslUInt32>(payLen))
        << "bytesWritten (" << wOut.bytesWritten
        << ") should be >= payload size (" << payLen << ")";

    /* uncompressedBytesWritten must be >= payload size as well */
    EXPECT_GE(wOut.uncompressedBytesWritten, static_cast<RsslUInt32>(payLen))
        << "uncompressedBytesWritten (" << wOut.uncompressedBytesWritten
        << ") should be >= payload size (" << payLen << ")";

    ASSERT_TRUE(waitForRxData(1, 4000)) << "Server received no data";
}

/* =========================================================================
 * Large Message + Compression Test LMC1 –
 * 2 × maxFragmentSize payload with ZLIB compression
 *
 * rsslGetBuffer is called with 2 × maxFragmentSize on a ZLIB-compressed
 * channel.  RIPC compresses each fragment before sending.  rsslRead on
 * the server decompresses and reassembles the complete payload.          */
TEST_F(RsslSocketWriteWebSocketTests,
       WriteLargeComp_Zlib_TwoFragments_ServerReceivesFullPayload)
{
    fakeArg.port             = 16170;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 6;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16170";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB)) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for 2×maxFragmentSize compressed: " << err.text;

    /* Fill with a recognisable repeating pattern */
    for (int i = 0; i < payLen; ++i)
        pBuf->data[i] = static_cast<char>((i % 26) + 'A');
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for compressed 2×maxFragmentSize: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(payLen + 2, 5000))
        << "Server received no data for compressed large message";

    /* Verify the alphabetic pattern appears in the decompressed payload */
    EXPECT_EQ(payLen + 2, fakeArg.rxLen);
    EXPECT_EQ('[', fakeArg.rxData[0]);
    EXPECT_EQ('A', fakeArg.rxData[1]) << "Fill byte A not found after 2-fragment reassembly; ";
    EXPECT_EQ('Z', fakeArg.rxData[fakeArg.rxLen - 2]);
    EXPECT_EQ(']', fakeArg.rxData[fakeArg.rxLen - 1]);

    /* uncompressedBytesWritten must be >= payload; bytesWritten should be
     * smaller due to compression (soft check – may not hold for all data). */
    EXPECT_GE(wOut.uncompressedBytesWritten, static_cast<RsslUInt32>(payLen))
        << "uncompressedBytesWritten should be >= payload size";
}

/* =========================================================================
 * Large Message + Compression Test LMC2 –
 * 3 × maxFragmentSize payload with ZLIB compression
 *
 * Forces at least three compressed RIPC fragments.  The server must
 * decompress and reassemble all three, returning a single buffer.        */
TEST_F(RsslSocketWriteWebSocketTests,
       WriteLargeComp_Zlib_ThreeFragments_ServerReceivesFullPayload)
{
    fakeArg.port             = 16171;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 1;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16171";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB)) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 3;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for 3×maxFragmentSize compressed: " << err.text;

    /* Alternating pattern to help detect reassembly byte-order issues */
    for (int i = 0; i < payLen; ++i)
        pBuf->data[i] = static_cast<char>((i & 1) ? 0x55 : 0xAA);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for compressed 3×maxFragmentSize: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(payLen + 2, 5000))
        << "Server received no data for 3-fragment compressed message";

    EXPECT_EQ(payLen + 2, fakeArg.rxLen);
    EXPECT_EQ('[', fakeArg.rxData[0]);
    EXPECT_EQ((char)0xAA, fakeArg.rxData[1]) << "Fill byte 0xAA not found after 2-fragment reassembly; ";
    EXPECT_EQ((char)0x55, fakeArg.rxData[fakeArg.rxLen - 2]);
    EXPECT_EQ(']', fakeArg.rxData[fakeArg.rxLen - 1]);

    EXPECT_GE(wOut.uncompressedBytesWritten, static_cast<RsslUInt32>(payLen))
        << "uncompressedBytesWritten should be >= payload size";
}

/* =========================================================================
 * Large Message + Compression Test LMC3 –
 * 2 × maxFragmentSize + ZLIB + RSSL_WRITE_DIRECT_SOCKET_WRITE
 *
 * Exercises the three-way combination of direct-flush, compression, and
 * fragmentation.  The server must receive and reassemble the payload.    */
TEST_F(RsslSocketWriteWebSocketTests,
       WriteLargeComp_Zlib_DirectFlush_ServerReceivesData)
{
    fakeArg.port             = 16172;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 3;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16172";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB)) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for direct+compressed large message: " << err.text;

    memset(pBuf->data, 0xBC, payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    wIn.writeInFlags      = RSSL_WRITE_DIRECT_SOCKET_WRITE;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx (direct+compressed) failed for large message: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1, 5000))
        << "Server received no data after direct+compressed flush";

    bool found = false;
    for (int i = 0; i < fakeArg.rxLen; ++i) {
        if ((unsigned char)fakeArg.rxData[i] == 0xBC) { found = true; break; }
    }
    EXPECT_TRUE(found)
        << "Fill byte 0xBC not found after direct+compressed large message; "
        << "rxLen=" << fakeArg.rxLen;

    EXPECT_GE(wOut.uncompressedBytesWritten, static_cast<RsslUInt32>(payLen))
        << "uncompressedBytesWritten should be >= payload size";
}

/* =========================================================================
 * Large Message + Compression Test LMC4 –
 * Highly-compressible 2 × maxFragmentSize payload (repeating byte)
 *
 * A payload of 2 × maxFragmentSize filled with a single repeating byte
 * compresses very efficiently with ZLIB.  The test verifies:
 *   a) the server receives the decompressed payload;
 *   b) wOut.bytesWritten < payLen (compression reduced the wire size).  */
TEST_F(RsslSocketWriteWebSocketTests,
       WriteLargeComp_Zlib_HighlyCompressible_LargeMessage_BytesWrittenLTPayload)
{
    fakeArg.port             = 16173;
    fakeArg.expectedMsgCount = 1;
    fakeArg.compressionType  = RSSL_COMP_ZLIB;
    fakeArg.compressionLevel = 6;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16173";
    ASSERT_TRUE(connectClient(RSSL_COMP_ZLIB)) << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    RsslChannelInfo chnlInfo;
    ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(pClientChnl, &chnlInfo, &err))
        << "rsslGetChannelInfo failed: " << err.text;

    const int payLen = static_cast<int>(chnlInfo.maxFragmentSize) * 2;
    ASSERT_LT(payLen, kRxBufSize) << "Payload too large for server rx buffer";

    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, payLen, RSSL_FALSE, &err);
    ASSERT_NE(nullptr, pBuf)
        << "rsslGetBuffer failed for highly-compressible large message: " << err.text;

    /* Repeating single byte – ZLIB achieves very high ratio on this data */
    memset(pBuf->data, 'Q', payLen);
    pBuf->length = payLen;

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS)
        << "rsslWriteEx failed for highly-compressible large message: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(1, 5000))
        << "Server received no data for highly-compressible large message";

    /* (a) Decompressed payload must contain the fill byte */
    bool found = false;
    for (int i = 0; i < fakeArg.rxLen; ++i) {
        if (fakeArg.rxData[i] == 'Q') { found = true; break; }
    }
    EXPECT_TRUE(found)
        << "Fill byte 'Q' not found in decompressed server data; "
        << "rxLen=" << fakeArg.rxLen;

    /* (b) Compressed wire size must be smaller than the raw payload */
    EXPECT_LT(wOut.bytesWritten, static_cast<RsslUInt32>(payLen))
        << "Expected bytesWritten (" << wOut.bytesWritten
        << ") < payload size (" << payLen
        << ") for a highly-compressible repeating-byte buffer";

    /* uncompressedBytesWritten must still account for the full payload */
    EXPECT_GE(wOut.uncompressedBytesWritten, static_cast<RsslUInt32>(payLen))
        << "uncompressedBytesWritten should be >= payload size";
}

/* =========================================================================
 * Test 13 – rsslWrite packed buffer (two messages packed together)
 *
 * RSSL supports packing multiple logical messages into a single transport
 * buffer.  Both sub-messages should be delivered to the server.           */
TEST_F(RsslSocketWriteWebSocketTests, WritePackedMessages_ServerReceivesBoth)
{
    fakeArg.port = 16112;
    fakeArg.expectedMsgCount = 1; /* one WS frame containing both messages */

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16112";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    ASSERT_TRUE(wsDriverToActive(pClientChnl, &err)) << err.text;

    /* Allocate a packed buffer large enough for two small messages */
    RsslBuffer* pBuf = rsslGetBuffer(pClientChnl, 64, RSSL_TRUE, &err);
    ASSERT_NE(nullptr, pBuf) << "rsslGetBuffer (packed) failed: " << err.text;

    /* First packed sub-message */
    memcpy(pBuf->data, "MsgA1234", 8);
    pBuf->length = 8;

    /* Pack and get the next sub-message slot */
    RsslBuffer* pNext = rsslPackBuffer(pClientChnl, pBuf, &err);
    if (pNext && pNext->data && pNext->length >= 8) {
        memcpy(pNext->data, "MsgB5678", 8);
        pNext->length = 8;
    }

    RsslWriteInArgs  wIn  = RSSL_INIT_WRITE_IN_ARGS;
    RsslWriteOutArgs wOut = RSSL_INIT_WRITE_OUT_ARGS;
    RsslRet wRet = rsslWriteEx(pClientChnl, pBuf, &wIn, &wOut, &err);
    EXPECT_GE(wRet, RSSL_RET_SUCCESS) << "rsslWriteEx (packed) failed: " << err.text;

    wsFlushAll(pClientChnl, &err);
    ASSERT_TRUE(waitForRxData(8)) << "Server received no data for packed messages";

    /* At least the first message content should appear */
    bool found = (wsMemmem(fakeArg.rxData, fakeArg.rxLen, "[MsgA1234,MsgB5678]", 19) != nullptr);
    EXPECT_TRUE(found) << "First packed sub-message not found in received data";
}

/* =========================================================================
 * Negative Connection Test NC1 – Server rejects the WebSocket connection
 *
 * The server is started with nakMount = RSSL_TRUE so that rsslAccept
 * explicitly refuses the incoming client connection.  The client must not
 * crash and its channel must NOT reach RSSL_CH_STATE_ACTIVE.             */
TEST_F(RsslSocketWriteWebSocketTests, ServerRejectsConnection_ClientNotActive)
{
    fakeArg.port             = 16300;
    fakeArg.expectedMsgCount = 0;
    fakeArg.nakMount         = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed on port 16300";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError err;
    bool active = wsDriverToActive(pClientChnl, &err, 200, 10);

    /* The server rejected the connection, so the client must not be ACTIVE */
    EXPECT_FALSE(active) << "Client channel should NOT reach ACTIVE when server rejects connection";
    EXPECT_TRUE(strstr(err.text, "Invalid HTTP response, status code 400") != NULL);

    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state)
        << "Channel state should be CLOSED or INACTIVE after rejection";
}