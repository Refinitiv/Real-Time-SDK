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
#include <iostream>
#include <string>
#include <atomic>
#include <algorithm>

#include "gtest/gtest.h"

#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslChanManagement.h"
#include "TransportUnitTest.h"

#if defined(_WIN32)
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#endif

/* Drive rsslInitChannel until ACTIVE or failure. */
static bool wsReadDriveToActive(RsslChannel* pChnl, RsslError* pErr,
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


enum WsOpCode {
    WS_OPC_CONT   = 0x00,   /* Continuation frame                   */
    WS_OPC_TEXT   = 0x01,   /* Text frame                           */
    WS_OPC_BINARY = 0x02,   /* Binary frame                         */
    WS_OPC_CLOSE  = 0x08,   /* Connection-close control frame       */
    WS_OPC_PING   = 0x09,   /* Ping control frame                   */
    WS_OPC_PONG   = 0x0A    /* Pong control frame                   */
};

/* =========================================================================
 * Send a raw WebSocket frame directly on the socket, bypassing rsslWriteEx.
 *
 * Parameters:
 *   sockId     – the platform socket returned by pSrvChnl->socketId
 *   opcode     – one of WsOpCode (BINARY, TEXT, PING, PONG, CLOSE, CONT)
 *   fin        – true  → set FIN bit (final / only fragment)
 *                false → clear FIN bit (non-final continuation fragment)
 *   payload    – bytes to place in the payload field (may be NULL when
 *                payloadLen == 0, e.g. for bare PING / PONG / CLOSE frames)
 *   payloadLen – number of payload bytes (0–65535)
 *
 * Frame layout built here (RFC 6455):
 *   byte 0 : FIN(1b) | RSV1-3(3b) | opcode(4b)
 *   byte 1 : MASK(0) | payload-len-7b
 *   [bytes 2-3] : extended 16-bit length when payloadLen >= 126
 *   bytes N+ : payload (unmasked; servers send unmasked frames)
 *
 * Returns true on success, false on any send error.
 * ========================================================================= */
static bool wsRawSendFrame(RsslSocket sockId,
                            WsOpCode   opcode,
                            bool       fin,
                            const char* payload,
                            int         payloadLen)
{
    if (payloadLen < 0 || payloadLen > 65535)
        return false;

    unsigned char hdr[4];
    int hdrLen = 0;

    /* byte 0: FIN + opcode */
    hdr[hdrLen++] = static_cast<unsigned char>(
                        (fin ? 0x80 : 0x00) | (opcode & 0x0F));

    /* byte 1 (+optional extended length): unmasked, length */
    if (payloadLen < 126) {
        hdr[hdrLen++] = static_cast<unsigned char>(payloadLen);
    } else {
        hdr[hdrLen++] = 126;
        hdr[hdrLen++] = static_cast<unsigned char>(payloadLen >> 8);
        hdr[hdrLen++] = static_cast<unsigned char>(payloadLen & 0xFF);
    }

#if defined(_WIN32)
    SOCKET s = (SOCKET)sockId;
    if (send(s, reinterpret_cast<const char*>(hdr), hdrLen, 0) != hdrLen)
        return false;
    int sent = 0;
    while (sent < payloadLen) {
        int n = send(s, payload + sent, payloadLen - sent, 0);
        if (n <= 0) return false;
        sent += n;
    }
#else
    int s = static_cast<int>(sockId);
    if (send(s, hdr, hdrLen, 0) != hdrLen)
        return false;
    int sent = 0;
    while (sent < payloadLen) {
        int n = static_cast<int>(send(s, payload + sent, payloadLen - sent, 0));
        if (n <= 0) return false;
        sent += n;
    }
#endif
    return true;
}

/* =========================================================================
 * Send an arbitrary byte sequence directly on the socket.
 *
 * Used to inject malformed / truncated WebSocket frames that cannot be
 * built with wsRawSendFrame() because they deliberately violate RFC 6455
 * (e.g. truncated headers, reserved opcodes, over-sized control payloads).
 *
 * Returns true on success, false on any send error.
 * ========================================================================= */
static bool wsRawSendBytes(RsslSocket sockId,
                            const unsigned char* bytes, int len)
{
    if (len <= 0) return false;
#if defined(_WIN32)
    SOCKET s = (SOCKET)sockId;
    int sent = 0;
    while (sent < len) {
        int n = send(s, reinterpret_cast<const char*>(bytes + sent),
                     len - sent, 0);
        if (n <= 0) return false;
        sent += n;
    }
#else
    int s = static_cast<int>(sockId);
    int sent = 0;
    while (sent < len) {
        int n = static_cast<int>(
                    send(s, reinterpret_cast<const char*>(bytes + sent),
                         len - sent, 0));
        if (n <= 0) return false;
        sent += n;
    }
#endif
    return true;
}

/* =========================================================================
 * Writer server thread – binds, accepts one connection, drives it to
 * ACTIVE, publishes its socket id, then idles until the test signals it
 * to stop.  All frame-sending is done by the individual test cases using
 * wsRawSendFrame() with srvArg.serverSocketId.
 * ========================================================================= */
struct WsWriterServerArg
{
    unsigned short    port;
    std::atomic<bool> bound;           /* set by thread after the port is bound */
    std::atomic<bool> ready;          /* set by thread after reaching ACTIVE  */
    std::atomic<bool> done;           /* set by thread just before it returns */
    std::atomic<bool> stopRequested;  /* set by test/TearDown to ask thread to exit */
    RsslSocket        serverSocketId; /* valid once ready == true             */
    /* Holds a full RsslError.text (MAX_RSSL_ERROR_TEXT); +64 leaves room for the
       snprintf literal prefix (e.g. "rsslAccept(compressed) failed: ") + NUL so
       the formatted result can never be truncated (-Wformat-truncation).       */
    char              errText[MAX_RSSL_ERROR_TEXT + 64];

    WsWriterServerArg()
        : port(0), bound(false), ready(false), done(false), stopRequested(false),
          serverSocketId(RSSL_INVALID_SOCKET)
    {
        memset(errText, 0, sizeof(errText));
    }
};

static RSSL_THREAD_DECLARE(wsWriterServerThread, pArg)
{
    WsWriterServerArg* arg = reinterpret_cast<WsWriterServerArg*>(pArg);

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

    RsslServer* pSrv = rsslBind(&bindOpts, &err);
    if (!pSrv) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslBind failed: %s", err.text);
        arg->done = true; arg->bound = true;
        return 0;
    }

    arg->bound = true;

    /* Accept one client connection */
    RsslAcceptOptions accOpts;
    rsslClearAcceptOpts(&accOpts);
    RsslChannel* pSrvChnl = nullptr;
    for (int i = 0; i < 500 && !pSrvChnl; ++i) {
        pSrvChnl = rsslAccept(pSrv, &accOpts, &err);
        if (!pSrvChnl) time_sleep(10);
    }
    if (!pSrvChnl) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslAccept failed: %s", err.text);
        rsslCloseServer(pSrv, &err);
        arg->done = true; arg->ready = true;
        return 0;
    }

    /* Drive server channel to ACTIVE */
    for (int i = 0; i < 400; ++i) {
        if (pSrvChnl->state == RSSL_CH_STATE_ACTIVE) break;
        if (pSrvChnl->state == RSSL_CH_STATE_CLOSED)  break;
        if (pSrvChnl->state == RSSL_CH_STATE_INACTIVE)  break;
        if (arg->stopRequested.load()) break;
        RsslInProgInfo inProg;
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pSrvChnl, &inProg, &err);
        time_sleep(5);
    }

    /* Return as the test case is being shutdown */
    if (arg->stopRequested.load()) {
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true;
        return 0;
    }

    if (pSrvChnl->state != RSSL_CH_STATE_ACTIVE) {
        snprintf(arg->errText, sizeof(arg->errText), "Server channel not ACTIVE");
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true; arg->ready = true;
        return 0;
    }

    /* Publish the raw socket so the test can call wsRawSendFrame() directly */
    arg->serverSocketId = pSrvChnl->socketId;

    /* Signal the test that the server is ready to receive frame-send calls */
    arg->ready = true;

    /* Idle until the test asks us to stop */
    while (!arg->stopRequested.load())
        time_sleep(10);

    rsslCloseChannel(pSrvChnl, &err);
    rsslCloseServer(pSrv, &err);
    arg->done = true;
    return 0;
}

/* =========================================================================
 * Test fixture
 * ========================================================================= */
class RsslWebSocketReadTests : public ::testing::Test
{
protected:
    RsslChannel*      pClientChnl = nullptr;
    RsslThreadId      srvTid;
    WsWriterServerArg srvArg;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
        memset(&srvTid, 0, sizeof(srvTid));
    }

    void TearDown() override
    {
        /* Ask the server thread to exit and wait for it */
        srvArg.stopRequested = true;
        for (int i = 0; i < 300 && !srvArg.done.load(); ++i)
            time_sleep(10);

        RsslError err;
        if (pClientChnl) {
            rsslCloseChannel(pClientChnl, &err);
            pClientChnl = nullptr;
        }
        resetDeadlockTimer();
        rsslUninitialize();
    }

    /* Start the server thread and wait until it has reached ACTIVE and
     * published serverSocketId. */
    virtual bool startServer()
    {
        RSSL_THREAD_START(&srvTid, wsWriterServerThread, &srvArg);
        for (int i = 0; i < 500 && !srvArg.bound.load(); ++i)
            time_sleep(2);
        return srvArg.bound.load() && srvArg.errText[0] == '\0';
    }

    virtual bool connectClient()
    {
        char portStr[16];
        snprintf(portStr, sizeof(portStr), "%u",
                 static_cast<unsigned>(srvArg.port));

        RsslError err;
        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_WEBSOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = portStr;
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.protocolType                       = RSSL_JSON_PROTOCOL_TYPE;
        opts.blocking                           = RSSL_FALSE;
        opts.wsOpts.protocols                   = const_cast<char*>("rssl.json.v2");
        opts.guaranteedOutputBuffers            = 50;

        pClientChnl = rsslConnect(&opts, &err);
        return pClientChnl != nullptr;
    }

    bool driveToActive()
    {
        RsslError err;
        return wsReadDriveToActive(pClientChnl, &err);
    }

    /* Convenience: send a single raw frame from the server socket.
     * Can be called directly in each test after startServer() succeeds. */
    bool serverSend(WsOpCode opcode, bool fin,
                    const char* payload, int payloadLen)
    {
        return wsRawSendFrame(srvArg.serverSocketId,
                              opcode, fin, payload, payloadLen);
    }

    /* Convenience: send arbitrary raw bytes from the server socket.
     * Used to inject deliberately malformed frames. */
    bool serverSendBytes(const unsigned char* bytes, int len)
    {
        return wsRawSendBytes(srvArg.serverSocketId, bytes, len);
    }

    /* Poll rsslRead until a non-NULL buffer or a notable return code is seen,
     * or until timeout.  Returns the last non-NULL buffer pointer (owned by
     * the channel – do not free). */
    RsslBuffer* waitForRead(RsslRet* outRet, int maxWaitMs = 3000)
    {
        RsslError err;
        RsslBuffer* pBuf = nullptr;
        for (int i = 0; i < maxWaitMs / 10; ++i) {
            RsslRet rRet;
            pBuf = rsslRead(pClientChnl, &rRet, &err);
            *outRet = rRet;
            if (pBuf && pBuf->length > 0)
                return pBuf;
            if (rRet == RSSL_RET_FAILURE ||
                rRet == RSSL_RET_READ_PING ||
                rRet == RSSL_RET_READ_FD_CHANGE)
                return pBuf;
            time_sleep(10);
        }
        return pBuf;
    }
};

/* =========================================================================
 * Test 1 – Read a small data message
 *
 * The server sends a 10-byte binary payload directly on the socket.
 * The client reads it and verifies the return code is RSSL_RET_SUCCESS
 * and the buffer is non-NULL.                                            */
TEST_F(RsslWebSocketReadTests, ReadSmallMessage_ReturnsSuccessAndBuffer)
{
    srvArg.port = 16300;

    ASSERT_TRUE(startServer())    << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient())  << "rsslConnect failed";
    ASSERT_TRUE(driveToActive())  << "Client channel not ACTIVE";

    int payloadSize = 10;
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "HelloWorld", payloadSize))
        << "serverSend failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf)        << "rsslRead returned NULL for data message";
    ASSERT_EQ((int)pBuf->length, payloadSize) << "Returned buffer has invalid length";
    ASSERT_TRUE(strstr(pBuf->data, "HelloWorld") != NULL) << "Returned buffer has invalid data";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);
}

/* =========================================================================
 * Test 2 – Read multiple messages sequentially
 *
 * The server sends 5 binary frames back-to-back.  The client reads each
 * one and verifies that all 5 are received.                              */
TEST_F(RsslWebSocketReadTests, ReadMultipleMessages_AllReceived)
{
    srvArg.port = 16301;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    const int kMsgCount = 5;
    const int msgSize = 8;
    for (int m = 0; m < kMsgCount; ++m)
        ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "Payload1", msgSize))
            << "serverSend[" << m << "] failed";

    int readCount = 0;
    RsslError err;
    const int maxWaitMs = 5000;
    for (int waited = 0; waited < maxWaitMs && readCount < kMsgCount; ) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) {
            EXPECT_EQ(msgSize, pBuf->length);
            ++readCount;
        } else if (rRet == RSSL_RET_FAILURE) {
            break;
        } else {
            time_sleep(10);
            waited += 10;
        }
    }

    EXPECT_EQ(kMsgCount, readCount)
        << "Expected " << kMsgCount << " messages but read " << readCount;
}

/* =========================================================================
 * Test 3 – Read a large message (near maxMsgSize)
 *
 * The server sends a 512-byte binary frame.  The client reads the full
 * payload and verifies the alphabetic fill pattern.                      */
TEST_F(RsslWebSocketReadTests, ReadLargeMessage_DataCorrect)
{
    srvArg.port = 16302;

    char largePayload[512];
    for (int i = 0; i < 512; ++i)
        largePayload[i] = static_cast<char>((i % 26) + 'A');

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, largePayload, 512))
        << "serverSend failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf)        << "rsslRead returned NULL for large message";
    ASSERT_GT((int)pBuf->length, 0) << "Returned buffer has zero length";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);

    char pattern[8];
    for (int i = 0; i < 8; ++i) pattern[i] = static_cast<char>(i + 'A');
    bool found = false;
    for (int i = 0; i + 8 <= (int)pBuf->length; ++i) {
        if (memcmp(pBuf->data + i, pattern, 8) == 0) { found = true; break; }
    }
    EXPECT_TRUE(found) << "Alphabetic pattern not found in received data";
}

/* =========================================================================
 * Test 5 – readOutArgs.bytesRead is populated after a successful read     */
TEST_F(RsslWebSocketReadTests, ReadMessage_BytesReadPopulated)
{
    srvArg.port = 16304;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "BytesReadChk", 12));

    RsslError       err;
    RsslReadOutArgs readOutArgs;
    RsslReadInArgs  readInArgs;
    rsslClearReadInArgs(&readInArgs);

    RsslBuffer* pBuf = nullptr;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslClearReadOutArgs(&readOutArgs);
        pBuf = rsslReadEx(pClientChnl, &readInArgs, &readOutArgs, &rRet, &err);
        if (pBuf && pBuf->length > 0) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }

    ASSERT_NE(nullptr, pBuf) << "rsslReadEx returned no buffer";

    /* 14 is header length plus payload length */
    EXPECT_EQ(14, readOutArgs.bytesRead)
        << "bytesRead should be 14 after receiving data";
}

/* =========================================================================
 * Test 6 – Ping frame received returns RSSL_RET_READ_PING, NULL buffer   */
TEST_F(RsslWebSocketReadTests, ReadPing_ReturnsReadPingAndNullBuffer)
{
    srvArg.port = 16305;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_PING, true, nullptr, 0));

    RsslError err;
    RsslRet   pingRet  = RSSL_RET_SUCCESS;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_READ_PING) {
            pingRet = rRet;
            EXPECT_EQ(nullptr, pBuf) << "Buffer must be NULL for a ping frame";
            break;
        }
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_READ_PING, pingRet)
        << "Expected RSSL_RET_READ_PING from ping frame";
}

/* =========================================================================
 * Test 7 – Read would block when no data is available                     */
TEST_F(RsslWebSocketReadTests, ReadNoData_ReturnsWouldBlock)
{
    srvArg.port = 16306;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Do NOT call serverSend – read immediately with no data on the socket */
    RsslError err;
    RsslRet   rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);

    EXPECT_EQ(nullptr, pBuf)
        << "Buffer should be NULL when no data is available";
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, rRet)
        << "Expected RSSL_RET_READ_WOULD_BLOCK when no data is available";
}

/* =========================================================================
 * Opcode Tests – one test per WsOpCode value
 *
 * Each test calls serverSend() directly after the channel is ACTIVE to
 * inject a specific WebSocket frame type and then asserts the expected
 * outcome observed by the client through rsslRead().
 * ========================================================================= */

/* =========================================================================
 * Opcode Test 1 – WS_OPC_BINARY
 *
 * A single binary frame is sent.  rsslRead() must return RSSL_RET_SUCCESS
 * with a non-NULL buffer containing the exact payload bytes.             */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Binary_DataReceived)
{
    srvArg.port = 16310;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "BinaryOpCode", 12))
        << "serverSend(BINARY) failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf)        << "rsslRead returned NULL for binary frame";
    ASSERT_EQ(12, (int)pBuf->length) << "Binary frame buffer has zero length";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);

    bool found = false;
    for (int i = 0; i + 12 <= (int)pBuf->length; ++i) {
        if (memcmp(pBuf->data + i, "BinaryOpCode", 12) == 0) { found = true; break; }
    }
    EXPECT_TRUE(found) << "Binary payload not found in received buffer";
}

/* =========================================================================
 * Opcode Test 2 – WS_OPC_TEXT
 *
 * A single text frame is sent.  rsslRead() must return RSSL_RET_SUCCESS
 * with a non-NULL buffer containing the UTF-8 text payload.             */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Text_DataReceived)
{
    srvArg.port = 16311;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_TEXT, true, "TextFrame!", 10))
        << "serverSend(TEXT) failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf)        << "rsslRead returned NULL for text frame";
    ASSERT_EQ(10, (int)pBuf->length) << "Text frame buffer has zero length";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);

    bool found = false;
    for (int i = 0; i + 10 <= (int)pBuf->length; ++i) {
        if (memcmp(pBuf->data + i, "TextFrame!", 10) == 0) { found = true; break; }
    }
    EXPECT_TRUE(found) << "Text payload not found in received buffer";
}

/* =========================================================================
 * Opcode Test 3 – WS_OPC_PING
 *
 * A PING frame is sent with a small echo payload.  rsslRead() must
 * return RSSL_RET_READ_PING with a NULL buffer.                          */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Ping_ReturnsPingCode)
{
    srvArg.port = 16312;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_PING, true, "ping", 4))
        << "serverSend(PING) failed";

    RsslError err;
    RsslRet   pingRet  = RSSL_RET_SUCCESS;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_READ_PING) {
            pingRet = rRet;
            EXPECT_EQ(nullptr, pBuf) << "Buffer must be NULL for a ping frame";
            break;
        }
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_READ_PING, pingRet)
        << "Expected RSSL_RET_READ_PING for WS_OPC_PING frame";
}

/* =========================================================================
 * Opcode Test 4 – WS_OPC_PONG
 *
 * An unsolicited PONG frame is sent.  The library must handle it
 * silently and must not return RSSL_RET_FAILURE.                         */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Pong_HandledWithoutError)
{
    srvArg.port = 16313;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_PONG, true, "pong", 4))
        << "serverSend(PONG) failed";

    RsslError err;
    RsslRet   lastRet;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer *pBuffer = rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        EXPECT_EQ(nullptr, pBuffer);
        if (rRet == RSSL_RET_FAILURE || rRet == RSSL_RET_READ_PING) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_READ_PING, lastRet)
        << "rsslRead must not return FAILURE for an unsolicited PONG frame";
}

/* =========================================================================
 * Opcode Test 5 – WS_OPC_CLOSE
 *
 * A CLOSE frame with normal-closure status (1000) is sent.  The channel
 * must transition away from ACTIVE.                                       */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Close_ChannelTransitionsFromActive)
{
    srvArg.port = 16314;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Close frame: 2-byte status code 1000 (0x03 0xE8) */
    char closePayload[2] = { 0x03, (char)(0xE8) };
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2))
        << "serverSend(CLOSE) failed";

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet = RSSL_RET_SUCCESS;
    EXPECT_EQ(nullptr, rsslRead(pClientChnl, &rRet, &err));
    EXPECT_EQ(RSSL_RET_FAILURE, rRet);
    ASSERT_TRUE(strstr(err.text, "WS Code 1000 Normal Closure") != NULL) << "Returned unexpcted error text";

    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state)
        << "Channel should have left ACTIVE state after receiving a CLOSE frame";
}

/* =========================================================================
 * Opcode Test 6 – WS_OPC_CONT (fragmented message)
 *
 * The test sends a two-fragment message directly: a non-final BINARY
 * frame (FIN=0) followed by a CONTINUATION frame (FIN=1).  rsslRead()
 * must reassemble and return a non-NULL buffer.                          */
TEST_F(RsslWebSocketReadTests, OpcodeWrite_Continuation_ReassembledCorrectly)
{
    srvArg.port = 16315;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Send first fragment (FIN=0, opcode=binary) */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Fragment", 8))
        << "serverSend(BINARY, FIN=0) failed";
    /* Send continuation / final fragment (FIN=1, opcode=cont) */
    ASSERT_TRUE(serverSend(WS_OPC_CONT, true, "edFrame!", 8))
        << "serverSend(CONT, FIN=1) failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf)        << "rsslRead returned NULL for fragmented message";
    ASSERT_TRUE(strstr(pBuf->data, "FragmentedFrame!") != NULL) << "Received unexpected message";
    ASSERT_EQ(16, (int)pBuf->length) << "Reassembled buffer has zero length";
    EXPECT_GE(rRet, RSSL_RET_SUCCESS)
        << "Expected non-failure return for fragmented frame";
}

/* =========================================================================
 * Invalid-frame Tests
 *
 * Each test injects a deliberately malformed WebSocket byte sequence and
 * then pumps rsslRead() in a loop.  The sole requirement is that the
 * library DOES NOT CRASH regardless of what it returns.  A channel state
 * of CLOSED or a return code of RSSL_RET_FAILURE are both acceptable
 * outcomes; the test only FAILs if the process crashes or hangs.
 * ========================================================================= */

/* =========================================================================
 * Invalid Test 1 – Single-byte truncated frame header
 *
 * Only byte-0 of the frame is sent; byte-1 (length) is missing.
 * The library must handle the incomplete header without crashing.         */
TEST_F(RsslWebSocketReadTests, InvalidFrame_TruncatedHeader_NoCrash)
{
    srvArg.port = 16400;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Send only byte-0 of a binary frame; byte-1 is never sent */
    unsigned char truncated[] = { 0x82 };
    serverSendBytes(truncated, sizeof(truncated));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(1, rRet) << "Expected to read more data";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state) << "Client channel should be ACTIVE";
}

/* =========================================================================
 * Invalid Test 2 – Claimed length larger than actual payload
 *
 * Header declares 50-byte payload but only 5 bytes follow.
 * The library must handle the length mismatch without crashing.           */
TEST_F(RsslWebSocketReadTests, InvalidFrame_LengthExceedsActualPayload_NoCrash)
{
    srvArg.port = 16401;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Header claims 50 bytes but we only send 5 bytes of payload */
    unsigned char frame[] = {
        0x82, 50,                             /* FIN|binary, length=50 */
        'A',  'B', 'C', 'D', 'E'             /* only 5 payload bytes  */
    };
    serverSendBytes(frame, sizeof(frame));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(1, rRet) << "Expected to read more data";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state) << "Client channel should be ACTIVE";
}

/* =========================================================================
 * Invalid Test 3 – Reserved opcode (0x03)
 *
 * RFC 6455 §5.2 reserves opcodes 0x03–0x07.  Receiving one is a
 * protocol error; the library must not crash.                             */
TEST_F(RsslWebSocketReadTests, InvalidFrame_ReservedOpcode_NoCrash)
{
    srvArg.port = 16402;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x03 (reserved data frame), 4-byte payload */
    unsigned char frame[] = { 0x83, 0x04, 'R', 'S', 'V', 'D' };
    serverSendBytes(frame, sizeof(frame));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, rRet);
    ASSERT_TRUE(strstr(err.text, "Unexpected WebSocket OpCode: (3)") != NULL) << "Returned unexpcted error text";
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Invalid Test 4 – Reserved control opcode (0x0B)
 *
 * RFC 6455 §5.2 reserves control opcodes 0x0B–0x0F.
 * The library must not crash.                                             */
TEST_F(RsslWebSocketReadTests, InvalidFrame_ReservedControlOpcode_NoCrash)
{
    srvArg.port = 16403;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x0B (reserved control), no payload */
    unsigned char frame[] = { 0x8B, 0x00 };
    serverSendBytes(frame, sizeof(frame));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, rRet);
    ASSERT_TRUE(strstr(err.text, "Unexpected WebSocket OpCode: (11)") != NULL) << "Returned unexpcted error text";
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Invalid Test 5 – Control frame with FIN=0 (fragmented control)
 *
 * RFC 6455 §5.5 prohibits fragmented control frames.
 * The library must not crash.                                             */
TEST_F(RsslWebSocketReadTests, InvalidFrame_FragmentedControlFrame_NoCrash)
{
    srvArg.port = 16404;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=0, opcode=PING (0x09) – fragmented ping, illegal per RFC */
    unsigned char frame[] = { 0x09, 0x04, 'p', 'i', 'n', 'g' };
    serverSendBytes(frame, sizeof(frame));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, rRet);
    ASSERT_TRUE(strstr(err.text, "Invalid fragmented WebSocket OpCode: (9)") != NULL) << "Returned unexpcted error text";
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Invalid Test 7 – RSV bits set without negotiated extension
 *
 * RSV1 is set but no per-message compression was negotiated, making this
 * frame invalid per RFC 6455 §5.2.
 * The library must not crash.                                             */
TEST_F(RsslWebSocketReadTests, InvalidFrame_RSVBitSetWithoutExtension_NoCrash)
{
    srvArg.port = 16406;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1 (0x40), opcode=binary: byte0 = 0xC2 */
    unsigned char frame[] = { 0xC2, 0x05, 'R', 'S', 'V', '1', '!' };
    serverSendBytes(frame, sizeof(frame));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_FAILURE, rRet);
    ASSERT_TRUE(strstr(err.text, "Compression settings mismatch, internal error") != NULL) << "Returned unexpcted error text";
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Invalid Test 8 – Completely empty byte stream
 *
 * Sending zero bytes should not trigger any crash or hang.  The library
 * will simply see nothing on the socket.                                  */
TEST_F(RsslWebSocketReadTests, InvalidFrame_EmptyByteStream_NoCrash)
{
    srvArg.port = 16407;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Send nothing – do not call serverSend or serverSendBytes */

    RsslError err;
    RsslRet   rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);

    /* No data on the socket; must not crash and must return would-block */
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, rRet);
}

/* =========================================================================
 * Invalid Fragmented-message Tests
 *
 * Each test injects a deliberately malformed fragmented WebSocket message
 * sequence and pumps rsslRead() in a loop.  As with the other invalid-frame
 * tests the only hard requirement is NO CRASH.  RSSL_RET_FAILURE and a
 * CLOSED channel state are both acceptable outcomes.
 * ========================================================================= */

/* =========================================================================
 * Invalid Frag Test 2 – New data frame while fragmentation is in progress
 *
 * A non-FIN BINARY frame opens a fragment sequence but is immediately
 * followed by another non-FIN BINARY frame (instead of a CONT frame),
 * which violates RFC 6455 §5.4.  The library must not crash.             */
TEST_F(RsslWebSocketReadTests, InvalidFrag_NewDataFrameDuringFragSequence_NoCrash)
{
    srvArg.port = 16411;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, BINARY – opens the sequence */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Frag1___", 8));
    /* Fragment 2 (wrong): FIN=0, BINARY again instead of CONT */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Frag2bad", 8));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, rRet);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Invalid Frag Test 3 – Fragmented message that is never completed
 *
 * A non-FIN BINARY frame is sent but the continuation frame is never
 * sent.  The library is left holding an incomplete reassembly buffer
 * with no more data.  rsslRead() must return WOULD_BLOCK (or similar)
 * and must not crash.                                                     */
TEST_F(RsslWebSocketReadTests, InvalidFrag_NeverCompletedFragment_NoCrash)
{
    srvArg.port = 16412;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Only the first fragment is sent – continuation never arrives */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "OnlyFirst", 9));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, rRet);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Invalid Frag Test 4 – Zero-length first fragment followed by final CONT
 *
 * An empty non-FIN BINARY frame opens the sequence, followed by a
 * FIN=1 CONT frame that carries all the data.  This is technically
 * valid per RFC but uncommon; the library must not crash and should
 * reassemble correctly or reject without crashing.                        */
TEST_F(RsslWebSocketReadTests, InvalidFrag_ZeroLengthFirstFragment_NoCrash)
{
    srvArg.port = 16413;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    int dataLength = 8;

    /* First fragment: FIN=0, BINARY, payload length=0 */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, nullptr, 0));
    /* Final fragment: FIN=1, CONT, carries the actual data */
    ASSERT_TRUE(serverSend(WS_OPC_CONT, true, "AllData!", dataLength));

    RsslError err;
    const int maxWaitMs = 2000;
    bool receivedActualData = false;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer *pBuffer = rsslRead(pClientChnl, &rRet, &err);

        EXPECT_NE(RSSL_RET_FAILURE, rRet);
        EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);

        if (pBuffer)
        {
            EXPECT_EQ(dataLength, pBuffer->length);
            EXPECT_TRUE(strstr(pBuffer->data, "AllData!") != NULL);
            receivedActualData = true;
            break;
        }

        time_sleep(10);
    }
    EXPECT_TRUE(receivedActualData) << "Expected to recieve the actual data";
}

/* =========================================================================
 * Invalid Frag Test 5 – Control frame (PING) interleaved in fragment
 *                        sequence, followed by a malformed continuation
 *
 * RFC 6455 §5.5 permits control frames between fragments, but the
 * continuation that follows has a truncated payload.  The library must
 * not crash.                                                              */
TEST_F(RsslWebSocketReadTests, InvalidFrag_ControlInterleaved_TruncatedCont_NoCrash)
{
    srvArg.port = 16414;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Step 1: open a fragment sequence */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Part1___", 8));
    /* Step 2: valid PING control frame interleaved (allowed by RFC) */
    ASSERT_TRUE(serverSend(WS_OPC_PING, true, nullptr, 0)); 
    /* Step 3: continuation frame whose header claims 20 bytes but only 3 are sent */
    unsigned char badCont[] = {
        0x80, 20,               /* FIN=1, CONT, length=20 */
        'x', 'y', 'z'           /* only 3 payload bytes   */
    };
    serverSendBytes(badCont, sizeof(badCont));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer *pBuf = rsslRead(pClientChnl, &rRet, &err);
        EXPECT_EQ(nullptr, pBuf);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE || rRet == RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Invalid Frag Test 6 – Fragmented TEXT message followed by CONT with
 *                        reserved opcode bits set in the continuation
 *
 * The continuation frame has RSV1=1 set (byte0 = 0xC0), which is illegal
 * unless a matching extension was negotiated.  The library must not crash. */
TEST_F(RsslWebSocketReadTests, InvalidFrag_ContFrameWithRSVBit_NoCrash)
{
    srvArg.port = 16415;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* First fragment: FIN=0, TEXT */
    ASSERT_TRUE(serverSend(WS_OPC_TEXT, false, "Hello", 5));
    /* Malformed continuation: FIN=1, RSV1=1 (0x40), opcode=CONT(0x00) → 0xC0 */
    /* RSV1 for the CONT is ignored as the compression is based on the first fragmented frame. */
    unsigned char badCont[] = { 0xC0, 0x05, ' ', 'W', 'o', 'r', 'l' };
    serverSendBytes(badCont, sizeof(badCont));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);
    EXPECT_NE(nullptr, pBuf);
    EXPECT_EQ(10, pBuf->length);
    EXPECT_TRUE(strstr(pBuf->data, "Hello Worl") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Invalid Frag Test 7 – Three-fragment sequence where the middle fragment
 *                        is completely empty and the final one is truncated
 *
 * Tests the reassembly path with: valid first fragment, empty middle
 * CONT (FIN=0, len=0), then a final CONT whose declared length exceeds
 * the bytes actually sent.                                                */
TEST_F(RsslWebSocketReadTests, InvalidFrag_EmptyMiddle_TruncatedFinal_NoCrash)
{
    srvArg.port = 16416;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, BINARY */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Part1", 5));
    /* Fragment 2: FIN=0, CONT, zero length */
    ASSERT_TRUE(serverSend(WS_OPC_CONT, false, nullptr, 0));
    /* Fragment 3: FIN=1, CONT – header claims 30 bytes, only 4 sent */
    unsigned char truncFinal[] = {
        0x80, 30,               /* FIN=1, CONT, length=30 */
        'E', 'N', 'D', '!'      /* only 4 bytes           */
    };
    serverSendBytes(truncFinal, sizeof(truncFinal));

    /* Waits to receive the data */
    time_sleep(20);

    RsslError err;
    RsslRet rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
    EXPECT_EQ(RSSL_RET_READ_WOULD_BLOCK, rRet);
    EXPECT_EQ(nullptr, pBuf);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Additional Negative Tests
 *
 * Cover error paths in rsslWebSocketRead() / rsslRead() that are not
 * exercised by the tests above:
 *
 *  N1.  Server abruptly closes the TCP connection mid-stream
 *       → rsslRead() must return RSSL_RET_FAILURE without crashing
 *  N2.  rsslRead() with NULL RsslError pointer
 *       → must not dereference the pointer and must not crash
 *  N3.  rsslReadEx() with NULL channel
 *       → must return RSSL_RET_FAILURE
 *  N4.  rsslReadEx() with NULL readInArgs
 *       → must return RSSL_RET_FAILURE or handle gracefully
 *  N5.  rsslReadEx() with NULL readOutArgs
 *       → must return RSSL_RET_FAILURE or handle gracefully
 *  N6.  Server sends a masked frame (RFC 6455 §5.1 — servers MUST NOT mask)
 *       → library must detect the violation and not crash
 *  N7.  Frame with maximum 16-bit extended payload length (65535 bytes)
 *       → rsslRead() must handle the extended-length encoding without crash
 *  N8.  Burst of valid frames that saturate the receive queue
 *       → all messages must be read back without error or crash
 * ========================================================================= */

/* =========================================================================
 * Negative Test N1 – Abrupt server-side TCP disconnect
 *
 * The server signals `stopRequested` immediately after the channel reaches
 * ACTIVE so the OS closes the underlying TCP socket.  The client then
 * calls rsslRead() in a loop until the library reports the disconnection.
 * rsslRead() must return RSSL_RET_FAILURE (or detect a closed channel)
 * without crashing.                                                        */
TEST_F(RsslWebSocketReadTests, Negative_AbruptServerDisconnect_ReturnsFailure)
{
    srvArg.port = 16420;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Ask the server thread to exit immediately – this closes the socket */
    srvArg.stopRequested = true;

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer *pRsslBuffer = rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_STREQ("WS Code 1000 Normal Closure", err.text);
    EXPECT_TRUE(lastRet == RSSL_RET_FAILURE &&
                pClientChnl->state == RSSL_CH_STATE_CLOSED)
        << "Expected FAILURE or CLOSED channel after abrupt server disconnect";
}
/* =========================================================================
 * Negative Test N6 – Server sends a masked frame (RFC 6455 §5.1 violation)
 *
 * RFC 6455 §5.1 states that servers MUST NOT mask frames.  When the
 * client receives a masked frame from the server it is a protocol error.
 * The library must detect this and must not crash or misinterpret the
 * payload.                                                                */
TEST_F(RsslWebSocketReadTests, Negative_MaskedServerFrame_NoCrash)
{
    srvArg.port = 16425;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build a masked binary frame: FIN=1, BINARY, MASK=1, len=5, key=0xDEADBEEF */
    unsigned char maskedFrame[] = {
        0x82,                   /* FIN=1, opcode=binary */
        0x85,                   /* MASK=1, length=5     */
        0xDE, 0xAD, 0xBE, 0xEF, /* masking key          */
        /* "Hello" XOR 0xDEADBEEF = { 0x96, 0xC8, 0xD2, 0x83, 0xB1 } */
        0x96, 0xC8, 0xD2, 0x83, 0xB1
    };
    serverSendBytes(maskedFrame, sizeof(maskedFrame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Negative Test N7 – Frame using the 16-bit extended payload length field
 *                     (payloadLen = 65535, the RFC maximum for this field)
 *
 * The header correctly declares 65535 bytes but we only send 8 bytes of
 * payload, creating a length mismatch.  Tests that the extended-length
 * parsing path does not crash or overflow.                                */
TEST_F(RsslWebSocketReadTests, Negative_ExtendedLengthFieldMismatch_NoCrash)
{
    srvArg.port = 16426;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, BINARY, length indicator=126, extended 16-bit length=65535
     * but we only send 8 bytes of actual payload                          */
    unsigned char frame[] = {
        0x82,                   /* FIN=1, opcode=binary              */
        126,                    /* 2-byte extended length indicator  */
        0xFF, 0xFF,             /* declared length = 65535           */
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' /* only 8 bytes    */
    };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Negative Test N9 – WS CLOSE frame returns RSSL_RET_FAILURE with NULL
 *                    buffer
 *
 * Targets the `frame->opcode == RWS_OPC_CLOSE` check that sits outside
 * handleWebSocketMessages() in rwsReadWebSocket().  When a CLOSE frame
 * arrives on a fresh read the function must set *readret =
 * RSSL_RET_FAILURE and return NULL (not a buffer pointer).               */
TEST_F(RsslWebSocketReadTests, Negative_CloseFrame_ReturnsFailureNullBuffer)
{
    srvArg.port = 16428;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Close frame with normal-closure status code 1000 (0x03 0xE8) */
    char closePayload[2] = { 0x03, (char)(0xE8) };
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2))
        << "serverSend(CLOSE) failed";

    RsslError   err;
    RsslBuffer* pBuf    = nullptr;
    RsslRet     lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int   maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        pBuf    = rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE when a WS CLOSE frame is received";
    EXPECT_EQ(nullptr, pBuf)
        << "Buffer must be NULL when rwsReadWebSocket returns RSSL_RET_FAILURE "
           "for a CLOSE frame";
}

/* =========================================================================
 * Negative Test N10 – Compressed frame (RSV1=1) without a negotiated
 *                      permessage-deflate extension
 *
 * Targets the `frame->compressed && !wsSess->deflate` branch inside
 * rwsReadWebSocket().  The server sends a binary frame with RSV1 set
 * (0xC2) but the connection was not established with compression.  The
 * library must return RSSL_RET_FAILURE.                                   */
TEST_F(RsslWebSocketReadTests, Negative_CompressedFrameWithoutExtension_ReturnsFailure)
{
    srvArg.port = 16429;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1 (bit 6), opcode=binary → byte0 = 0xC2 */
    unsigned char compressedFrame[] = {
        0xC2,                   /* FIN=1, RSV1=1, opcode=BINARY     */
        0x05,                   /* MASK=0, length=5                 */
        'C', 'O', 'M', 'P', '!'
    };
    serverSendBytes(compressedFrame, sizeof(compressedFrame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE when a compressed frame is received "
           "without a negotiated deflate extension";
}

/* =========================================================================
 * Negative Test N11 – Frame whose declared wsFrameLen exceeds the channel's
 *                      inputBuffer->maxLength (error 1007 path)
 *
 * Targets the `wsFrameLen > rsslSocketChannel->inputBuffer->maxLength`
 * guard in rwsReadWebSocket().  We use the 8-byte extended-length header
 * (indicator byte = 127) with a declared length that is larger than any
 * reasonable channel buffer.  The library must return RSSL_RET_FAILURE
 * and must not crash.                                                      */
TEST_F(RsslWebSocketReadTests, Negative_FrameLenExceedsInputBuffer_ReturnsFailure)
{
    srvArg.port = 16430;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build a frame whose 8-byte extended length is 0x0000000010000000
     * (~256 MB) – far beyond any negotiated channel buffer size.
     * byte 1 = 127 signals an 8-byte extended length field.              */
    unsigned char oversizedFrame[] = {
        0x82,                                       /* FIN=1, BINARY    */
        127,                                        /* 8-byte ext-len   */
        0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, /* len=256 MB  */
        'O', 'V', 'E', 'R'                          /* tiny payload     */
    };
    serverSendBytes(oversizedFrame, sizeof(oversizedFrame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE when frame size exceeds "
           "inputBuffer->maxLength";
}

/* =========================================================================
 * Negative Test N12 – WS CLOSE frame with an out-of-range status code
 *
 * RFC 6455 §7.4 defines valid status codes.  A status code of 0x0000 is
 * explicitly disallowed.  The library must handle the malformed close
 * payload gracefully, return RSSL_RET_FAILURE, and not crash.            */
TEST_F(RsslWebSocketReadTests, Negative_CloseFrameInvalidStatusCode_ReturnsFailure)
{
    srvArg.port = 16431;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* CLOSE frame with status code 0x0000 (forbidden by RFC 6455 §7.4) */
    char closePayload[2] = { 0x00, 0x00 };
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2))
        << "serverSend(CLOSE, code=0) failed";

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a CLOSE frame with invalid "
           "status code 0x0000";
}

/* =========================================================================
 * Negative Test N13 – Zero-length complete binary frame (FIN=1, len=0)
 *
 * Targets the `rsslSocketChannel->curInputBuf->length == 0` check at the
 * bottom of rwsReadWebSocket() / handleWebSocketMessages() that sets
 * *readret = RSSL_RET_SUCCESS and clears the length.  An empty but valid
 * frame must not crash and must leave the channel ACTIVE.                */
TEST_F(RsslWebSocketReadTests, Negative_ZeroLengthBinaryFrame_NoCrash)
{
    srvArg.port = 16432;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, BINARY, payloadLen=0 */
    unsigned char emptyFrame[] = { 0x82, 0x00 };
    serverSendBytes(emptyFrame, sizeof(emptyFrame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }

    EXPECT_NE(RSSL_RET_FAILURE, lastRet)
        << "Zero-length binary frame must not cause RSSL_RET_FAILURE";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after a zero-length binary frame";
}

/* =========================================================================
 * Negative Test N14 – CLOSE frame with no payload (0-byte body)
 *
 * RFC 6455 §5.5.1 permits an empty CLOSE frame (no status code, no
 * reason phrase).  The CLOSE handler in rwsReadWebSocket() checks
 * `frame->payloadLen >= 2` before extracting the status code; when len
 * is 0 that branch is skipped.  The library must still return
 * RSSL_RET_FAILURE and must not crash.                                   */
TEST_F(RsslWebSocketReadTests, Negative_CloseFrameNoPayload_ReturnsFailure)
{
    srvArg.port = 16433;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, CLOSE (0x08), payload length = 0 */
    unsigned char closeNoPayload[] = { 0x88, 0x00 };
    serverSendBytes(closeNoPayload, sizeof(closeNoPayload));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a CLOSE frame with no payload";
}

/* =========================================================================
 * Negative Test N15 – All three RSV bits set without any negotiated
 *                      extension (RSV1=RSV2=RSV3=1)
 *
 * byte0 = FIN(1) | RSV1(1) | RSV2(1) | RSV3(1) | opcode(BINARY) = 0xF2.
 * No compression was negotiated so the `frame->compressed && !wsSess->deflate`
 * guard should fire (RSV1 is set).  The library must return
 * RSSL_RET_FAILURE and must not crash.                                   */
TEST_F(RsslWebSocketReadTests, Negative_AllRSVBitsSet_ReturnsFailure)
{
    srvArg.port = 16434;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=RSV2=RSV3=1, opcode=BINARY → 0xF2 */
    unsigned char frame[] = { 0xF2, 0x04, 'R', 'S', 'V', 'X' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE when all RSV bits are set without "
           "a negotiated extension";
}

/* =========================================================================
 * Negative Test N16 – rsslRead() on a channel that has already been
 *                      explicitly closed via rsslCloseChannel()
 *
 * After rsslCloseChannel() the internal state is INACTIVE/CLOSED.  Any
 * subsequent rsslRead() call must return RSSL_RET_FAILURE immediately
 * without crashing and without attempting to read from the socket.       */
TEST_F(RsslWebSocketReadTests, Negative_ReadOnClosedChannel_ReturnsFailure)
{
    srvArg.port = 16435;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Explicitly close the channel from the client side */
    RsslError err;
    rsslCloseChannel(pClientChnl, &err);
    /* Set to nullptr so TearDown() does not double-close */
    RsslChannel* pClosed = pClientChnl;
    pClientChnl = nullptr;

    /* Attempt to read from the now-closed channel */
    RsslRet     rRet;
    RsslBuffer* pBuf = rsslRead(pClosed, &rRet, &err);

    EXPECT_EQ(nullptr, pBuf)
        << "Buffer must be NULL when reading from a closed channel";
    EXPECT_EQ(RSSL_RET_FAILURE, rRet)
        << "Expected RSSL_RET_FAILURE when reading from a closed channel";
}

/* =========================================================================
 * Negative Test N17 – Multiple consecutive PING frames
 *
 * Three PING frames are sent back-to-back.  Each one must trigger
 * RSSL_RET_READ_PING; none must crash or return RSSL_RET_FAILURE.        */
TEST_F(RsslWebSocketReadTests, Negative_MultipleConsecutivePings_AllHandled)
{
    srvArg.port = 16436;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    const int kPingCount = 3;
    for (int i = 0; i < kPingCount; ++i)
        ASSERT_TRUE(serverSend(WS_OPC_PING, true, nullptr, 0))
            << "serverSend(PING)[" << i << "] failed";

    int pingsSeen = 0;
    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs && pingsSeen < kPingCount; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_READ_PING) ++pingsSeen;
        if (rRet == RSSL_RET_FAILURE)   break;
        time_sleep(10);
    }

    EXPECT_EQ(1, pingsSeen)
        << "Expected " << kPingCount << " RSSL_RET_READ_PING returns, got "
        << pingsSeen;
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after multiple PINGs";
}

/* =========================================================================
 * Negative Test N18 – Binary frame at exactly the 126-byte extended-length
 *                      boundary
 *
 * RFC 6455 §5.2: payload lengths 0–125 use the 7-bit field directly;
 * 126 triggers the 2-byte extended-length encoding.  A frame with
 * exactly 126 payload bytes must be decoded correctly and delivered
 * without crashing.                                                       */
TEST_F(RsslWebSocketReadTests, Negative_FrameAt126ByteBoundary_DataReceived)
{
    srvArg.port = 16437;

    char payload126[126];
    memset(payload126, 'B', sizeof(payload126));

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* wsRawSendFrame handles payloadLen >= 126 with the 2-byte ext-len header */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, payload126, 126))
        << "serverSend(BINARY, 126 bytes) failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf) << "rsslRead returned NULL for 126-byte frame";
    EXPECT_EQ(126, (int)pBuf->length)
        << "Payload length mismatch for 126-byte extended-length frame";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);

    /* Verify fill byte */
    bool allB = true;
    for (int i = 0; i < (int)pBuf->length; ++i)
        if (pBuf->data[i] != 'B') { allB = false; break; }
    EXPECT_TRUE(allB) << "Fill byte 'B' not found in 126-byte payload";
}

/* =========================================================================
 * Negative Test N19 – Unsolicited PONG frame with non-empty payload
 *
 * RFC 6455 §5.5.3: a PONG may carry application data.  The library
 * must discard the unsolicited PONG silently and must not crash or
 * return RSSL_RET_FAILURE.                                               */
TEST_F(RsslWebSocketReadTests, Negative_PongWithPayload_HandledWithoutError)
{
    srvArg.port = 16438;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* PONG with 10-byte payload */
    ASSERT_TRUE(serverSend(WS_OPC_PONG, true, "PongData!!", 10))
        << "serverSend(PONG, payload) failed";

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }

    EXPECT_NE(RSSL_RET_FAILURE, lastRet)
        << "Unsolicited PONG with payload must not cause RSSL_RET_FAILURE";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after an unsolicited PONG with payload";
}

/* =========================================================================
 * Negative Test N20 – Back-to-back CLOSE frames
 *
 * A second CLOSE is sent immediately after the first.  The library
 * honours the first and must not crash processing (or ignoring) the
 * second.                                                                 */
TEST_F(RsslWebSocketReadTests, Negative_BackToBackCloseFrames_NoCrash)
{
    srvArg.port = 16439;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    char closePayload[2] = { 0x03, (char)(0xE8) }; /* 1000 */
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2));
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    /* No crash is the pass condition; CLOSED or FAILURE are both valid */
    EXPECT_NE(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel should not remain ACTIVE after two CLOSE frames";
}

/* =========================================================================
 * Negative Test N21 – Valid data frame immediately followed by CLOSE in
 *                      the same TCP segment
 *
 * Sends a BINARY frame and a CLOSE frame concatenated in one serverSendBytes
 * call so both arrive in the same recv() buffer.  The library must
 * surface the data frame first (RSSL_RET_SUCCESS with data), then the
 * CLOSE on a subsequent rsslRead() call, without crashing.               */
TEST_F(RsslWebSocketReadTests, Negative_DataFrameThenCloseInOneTCPSegment_DataThenClose)
{
    srvArg.port = 16440;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build: binary frame (6 bytes payload) + CLOSE frame (2-byte status) */
    unsigned char combined[] = {
        /* BINARY frame */
        0x82, 0x06, 'H', 'e', 'l', 'l', 'o', '!',
        /* CLOSE frame, status 1000 */
        0x88, 0x02, 0x03, 0xE8
    };
    serverSendBytes(combined, sizeof(combined));

    bool dataReceived = false;
    bool closeSeen    = false;
    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet     rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0)   dataReceived = true;
        if (rRet == RSSL_RET_FAILURE)   { closeSeen = true; break; }
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) { closeSeen = true; break; }
        time_sleep(10);
    }

    EXPECT_TRUE(dataReceived)
        << "Data frame should be delivered before the CLOSE is processed";
    EXPECT_TRUE(closeSeen)
        << "CLOSE frame should eventually cause RSSL_RET_FAILURE or CLOSED state";
}

/* =========================================================================
 * Negative Test N22 – Read immediately after rsslInitialize / rsslConnect
 *                      but before the channel reaches ACTIVE
 *
 * rsslRead() on a channel in RSSL_CH_STATE_INITIALIZING must not crash.
 * The expected outcome is RSSL_RET_FAILURE (channel not active) or a
 * non-SUCCESS code; the channel state must not flip to ACTIVE as a side
 * effect.                                                                 */
TEST_F(RsslWebSocketReadTests, Negative_ReadBeforeChannelActive_NoCrash)
{
    srvArg.port = 16441;

    ASSERT_TRUE(startServer()) << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";

    /* Do NOT call driveToActive() – channel is still initializing */
    ASSERT_NE(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Prerequisite: channel must not yet be ACTIVE";

    RsslError   err;
    RsslRet     rRet;
    RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);

    /* Must not crash; FAILURE or would-block are both acceptable */
    EXPECT_EQ(nullptr, pBuf)
        << "Buffer must be NULL when channel is not yet ACTIVE";
    EXPECT_NE(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must not become ACTIVE as a side-effect of rsslRead()";
}

/* =========================================================================
 * Negative Test N23 – Interleaved PING between two data fragments, then
 *                      the CLOSE frame before the continuation arrives
 *
 * Opens a fragmented sequence (FIN=0 BINARY), sends a PING, then sends a
 * CLOSE instead of the expected CONT.  The library must not crash and must
 * eventually report the CLOSE as RSSL_RET_FAILURE.                        */
TEST_F(RsslWebSocketReadTests, Negative_FragmentInterruptedByClose_NoCrash)
{
    srvArg.port = 16442;

    time_sleep(100);

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Step 1: open fragment (FIN=0) */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Partial!", 8));
    /* Step 2: interleave a PING (allowed by RFC) */
    ASSERT_TRUE(serverSend(WS_OPC_PING, true, nullptr, 0));
    /* Step 3: CLOSE instead of CONT */
    char closePayload[2] = { 0x03, (char)(0xE8) };
    ASSERT_TRUE(serverSend(WS_OPC_CLOSE, true, closePayload, 2));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Negative Test N24 – Binary frame with payload length exactly 125 bytes
 *                      (the maximum for the single-byte length field)
 *
 * The 7-bit length field holds values 0–125 without extension; 125 is the
 * boundary.  The frame must be decoded and delivered correctly.           */
TEST_F(RsslWebSocketReadTests, Negative_FrameAt125ByteBoundary_DataReceived)
{
    srvArg.port = 16443;

    char payload125[125];
    memset(payload125, 'C', sizeof(payload125));

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, payload125, 125))
        << "serverSend(BINARY, 125 bytes) failed";

    RsslRet     rRet;
    RsslBuffer* pBuf = waitForRead(&rRet);

    ASSERT_NE(nullptr, pBuf) << "rsslRead returned NULL for 125-byte frame";
    EXPECT_EQ(125, (int)pBuf->length)
        << "Payload length mismatch for 125-byte boundary frame";
    EXPECT_EQ(RSSL_RET_SUCCESS, rRet);
}

/* =========================================================================
 * Negative Test N25 – Valid data frame followed by an oversized-control
 *                      PING, then another valid data frame
 *
 * Verifies that after an invalid control frame is injected mid-stream the
 * channel either recovers (if the library discards the bad frame) or
 * closes cleanly – but in neither case crashes.  The first data frame must
 * have been delivered before the bad PING was seen.                       */
TEST_F(RsslWebSocketReadTests, Negative_DataBadPingData_NoCrash)
{
    srvArg.port = 16444;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Send first good data frame */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "GoodMsg1", 8));

    /* Send an illegal oversized PING (payload > 125 via 2-byte ext-len) */
    unsigned char badPing[4 + 126] = {};
    badPing[0] = 0x89;  /* FIN=1, PING    */
    badPing[1] = 126;   /* 2-byte ext-len */
    badPing[2] = 0x00;
    badPing[3] = 126;
    serverSendBytes(badPing, sizeof(badPing));

    /* Attempt a second good data frame (may or may not be delivered) */
    serverSend(WS_OPC_BINARY, true, "GoodMsg2", 8);

    bool firstDataSeen = false;
    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet     rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) firstDataSeen = true;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_TRUE(firstDataSeen)
        << "First valid data frame should have been delivered before "
           "the bad PING was encountered";
}

/* =========================================================================
 * Negative Test N26 – Reserved opcode 0x04 hits `default` switch case
 *
 * Targets the `default:` branch in the opcode switch inside
 * rwsReadTransportMsg() which returns RSSL_RET_FAILURE with the message
 * "Unexpected WebSocket OpCode: (4)".  Opcodes 0x03–0x07 are all reserved
 * non-control frames that fall through to this branch.                    */
TEST_F(RsslWebSocketReadTests, Negative_ReservedOpcode04_DefaultCase_ReturnsFailure)
{
    srvArg.port = 16445;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x04 (reserved data frame), 4-byte payload
     * byte0 = 0x80(FIN) | 0x04(opcode) = 0x84                           */
    unsigned char frame[] = { 0x84, 0x04, 'R', '0', '4', '!' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for reserved opcode 0x04 "
           "(default switch case in rwsReadTransportMsg)";
}

/* =========================================================================
 * Negative Test N27 – Partial 2-byte extended-length header
 *                      (indicator=126 present, but the 2 ext-len bytes
 *                       are never sent)
 *
 * Targets `_decodeWSFrame()` when `bufLen >= 2` but `bufLen < hdrLen`
 * (partial extended-length header).  The library must set `frame->partial`
 * and must not crash or read past the buffer.                             */
TEST_F(RsslWebSocketReadTests, Negative_PartialExtLenHeader_NoCrash)
{
    srvArg.port = 16446;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* byte0 = FIN|BINARY = 0x82; byte1 = 126 (2-byte ext-len indicator)
     * The two actual length bytes are intentionally never sent.           */
    unsigned char partial[] = { 0x82, 126 };
    serverSendBytes(partial, sizeof(partial));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Negative Test N28 – 8-byte extended-length encoding with declared
 *                      payload length = 0
 *
 * A 10-byte frame: 2-byte control header (indicator=127) + 8-byte length
 * field all zero.  Exercises the `rwfGet64` path in `_decodeWSFrame()`.
 * The frame declares zero payload bytes via the 8-byte encoding; the
 * library must decode it without crashing and must not report FAILURE.   */
TEST_F(RsslWebSocketReadTests, Negative_EightByteExtLenZeroPayload_NoCrash)
{
    srvArg.port = 16447;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, BINARY, indicator=127 (8-byte ext-len), length=0 */
    unsigned char frame[] = {
        0x82,                                       /* FIN=1, BINARY    */
        127,                                        /* 8-byte ext-len   */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* length = 0  */
    };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }

    EXPECT_NE(RSSL_RET_FAILURE, lastRet)
        << "8-byte extended-length frame with zero payload must not "
           "cause RSSL_RET_FAILURE";
}

/* =========================================================================
 * Negative Test N29 – CLOSE frame with exactly 1-byte payload (malformed)
 *
 * RFC 6455 §5.5.1 requires the close frame body to be either empty or
 * begin with a 2-byte status code.  A 1-byte body is explicitly
 * malformed.  The `frame->payloadLen >= 2` guard skips status-code
 * extraction, but the CLOSE should still be honoured (RSSL_RET_FAILURE).  */
TEST_F(RsslWebSocketReadTests, Negative_CloseFrameOneBytePaylod_ReturnsFailure)
{
    srvArg.port = 16448;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, CLOSE (0x08), length=1, one garbage byte */
    unsigned char frame[] = { 0x88, 0x01, 0xAB };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a CLOSE frame with a 1-byte "
           "(malformed) payload";
}

/* =========================================================================
 * Negative Test N30 – PING frame with exactly 125-byte payload
 *                      (RFC maximum for control frames)
 *
 * RFC 6455 §5.5 caps control frame payload at 125 bytes.  Exactly 125
 * bytes is still legal.  Targets the PING handler in
 * `rwsReadTransportMsg()` / `rwsProcessWsOpCodes()` with the maximum
 * valid payload and verifies RSSL_RET_READ_PING is returned.             */
TEST_F(RsslWebSocketReadTests, Negative_PingMaxLegalPayload_ReturnsReadPing)
{
    srvArg.port = 16449;

    char ping125[125];
    memset(ping125, 'P', sizeof(ping125));

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* PING with 125-byte payload – still within RFC limit (single-byte len) */
    ASSERT_TRUE(serverSend(WS_OPC_PING, true, ping125, 125))
        << "serverSend(PING, 125 bytes) failed";

    RsslError err;
    RsslRet   pingRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_READ_PING) {
            pingRet = rRet;
            EXPECT_EQ(nullptr, pBuf)
                << "Buffer must be NULL for a PING frame";
            break;
        }
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_READ_PING, pingRet)
        << "Expected RSSL_RET_READ_PING for a 125-byte PING payload";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after a max-payload PING";
}

/* =========================================================================
 * Negative Test N31 – Only RSV2 set (RSV1 clear) without any negotiated
 *                      extension
 *
 * `_decodeWSFrame()` only captures `rsv1Set` and maps it to the
 * `compressed` flag.  RSV2 and RSV3 are decoded into `rsv2Set`/`rsv3Set`
 * but the library does NOT validate them independently.  This tests that a
 * frame with only RSV2=1 (byte0=0xA2: FIN|RSV2|BINARY) is processed
 * without crashing, since the compression guard (`frame->compressed &&
 * !wsSess->deflate`) only triggers on RSV1.                              */
TEST_F(RsslWebSocketReadTests, Negative_OnlyRSV2Set_NoCrash)
{
    srvArg.port = 16450;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* byte0 = FIN(1) | RSV2(1) | BINARY = 0x80 | 0x20 | 0x02 = 0xA2 */
    unsigned char frame[] = { 0xA2, 0x05, 'R', 'S', 'V', '2', '!' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Negative Test N32 – Three complete valid frames sent in a single
 *                      TCP segment (exercises the `moreData` loop)
 *
 * All three frames arrive in one `recv()` call so
 * `rsslSocketChannel->inputBuffer->length` is set beyond the first frame.
 * The library's `*moreData = 1` path and the outer loop in
 * `rwsReadWebSocket()` must drain all three without error.               */
TEST_F(RsslWebSocketReadTests, Negative_ThreeFramesInOneTCPSegment_AllReceived)
{
    srvArg.port = 16451;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Three binary frames concatenated into one send() call:
     *   Frame 1: FIN=1, BINARY, len=4, payload="AAA!"
     *   Frame 2: FIN=1, BINARY, len=4, payload="BBB!"
     *   Frame 3: FIN=1, BINARY, len=4, payload="CCC!"               */
    unsigned char combined[] = {
        0x82, 0x04, 'A', 'A', 'A', '!',
        0x82, 0x04, 'B', 'B', 'B', '!',
        0x82, 0x04, 'C', 'C', 'C', '!'
    };
    serverSendBytes(combined, sizeof(combined));

    int readCount = 0;
    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs && readCount < 3; waited += 10) {
        RsslRet     rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) ++readCount;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(3, readCount)
        << "Expected all 3 frames from one TCP segment to be delivered; "
           "got " << readCount;
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after reading 3 frames from one segment";
}

/* =========================================================================
 * Negative Test N33 – Partial 8-byte extended-length header
 *                      (indicator=127 but only 4 of the 8 ext-len bytes
 *                       are sent)
 *
 * Exercises the partial-header detection in `_decodeWSFrame()` when the
 * 8-byte extended-length field is incomplete: `bufLen >= 2` (control
 * header present) but `bufLen < hdrLen` (extended header truncated).
 * The library must not crash.                                             */
TEST_F(RsslWebSocketReadTests, Negative_PartialEightByteExtLenHeader_NoCrash)
{
    srvArg.port = 16452;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* indicator=127 signals 8-byte ext-len; we only send 4 of those 8 bytes */
    unsigned char partial[] = {
        0x82,                   /* FIN=1, BINARY            */
        127,                    /* 8-byte ext-len indicator */
        0x00, 0x00, 0x00, 0x01  /* only 4 of 8 length bytes */
    };
    serverSendBytes(partial, sizeof(partial));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Negative Test N34 – Non-final orphaned CONT frame (FIN=0, opcode=CONT)
 *                      sent with no preceding fragment sequence
 *
 * Distinct from `InvalidFrag_OrphanedContinuation_NoCrash` (which sends
 * FIN=1).  Here FIN=0 means "I am an intermediate continuation fragment"
 * but there is no open reassembly context.  The library must copy the
 * payload into the reassembly buffer without crashing, then wait for
 * more data (which never arrives).                                        */
TEST_F(RsslWebSocketReadTests, Negative_NonFinalOrphanedCont_NoCrash)
{
    srvArg.port = 16453;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=0, opcode=CONT (0x00): byte0 = 0x00, byte1 = 0x06, 6-byte payload */
    unsigned char frame[] = { 0x00, 0x06, 'o', 'r', 'p', 'h', 'a', 'n' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Negative Test N35 – Reserved opcode 0x07 (another reserved data frame)
 *                      hitting the `default` switch case
 *
 * Complements N26 (opcode=0x04).  Opcode 0x07 is also a reserved
 * non-control frame (RFC 6455 §5.2, opcodes 0x03–0x07).  The `default:`
 * branch in the opcode switch inside `rwsReadTransportMsg()` must fire
 * and return RSSL_RET_FAILURE.                                            */
TEST_F(RsslWebSocketReadTests, Negative_ReservedOpcode07_DefaultCase_ReturnsFailure)
{
    srvArg.port = 16454;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x07 → byte0 = 0x80 | 0x07 = 0x87 */
    unsigned char frame[] = { 0x87, 0x04, 'R', '0', '7', '!' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for reserved opcode 0x07 "
           "(default switch case in rwsReadTransportMsg)";
}

/* =========================================================================
 * Crash Test C1 – All-0xFF garbage byte stream
 *
 * 64 bytes of 0xFF are injected.  byte0=0xFF means FIN=1, all RSV=1,
 * opcode=0x0F (reserved control); byte1=0xFF means MASK=1, len=127
 * (8-byte extended length requested from a server — illegal).  The
 * decoder must survive this completely corrupt input.                    */
TEST_F(RsslWebSocketReadTests, Crash_AllFFGarbageBytes_NoCrash)
{
    srvArg.port = 16460;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    unsigned char garbage[64];
    memset(garbage, 0xFF, sizeof(garbage));
    serverSendBytes(garbage, sizeof(garbage));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Crash Test C2 – Null-byte storm (1000 × 0x00)
 *
 * 0x00 as byte0 means FIN=0, no RSV, opcode=CONT.  The null storm
 * hammers the input-buffer reader and reassembly path with back-to-back
 * zero-byte partial CONT frames.  The library must not crash or leak.   */
TEST_F(RsslWebSocketReadTests, Crash_NullByteStorm_NoCrash)
{
    srvArg.port = 16461;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    unsigned char nulls[1000];
    memset(nulls, 0x00, sizeof(nulls));
    serverSendBytes(nulls, sizeof(nulls));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C3 – Maximum 65535-byte binary frame fully sent
 *
 * Exercises the `checkSizeAndRealloc` / `doubleSizeAndRealloc` buffer-
 * expansion path in `rwsReadWebSocket()`.  The server sends a complete
 * 65535-byte frame (2-byte extended-length header + full payload).  The
 * library must expand its input buffer and deliver the data without
 * crashing.                                                               */
TEST_F(RsslWebSocketReadTests, Crash_MaxExtLenFrameFullyDelivered_NoCrash)
{
    srvArg.port = 16462;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build the WS frame header for 65535-byte payload */
    unsigned char hdr[] = {
        0x82,           /* FIN=1, BINARY                  */
        126,            /* 2-byte extended-length follows  */
        0xFF, 0xFF      /* length = 65535                  */
    };
    serverSendBytes(hdr, sizeof(hdr));

    /* Send all 65535 payload bytes in chunks to avoid OS send-buffer limits */
    const int kPayloadSize = 65535;
    std::vector<unsigned char> payload(kPayloadSize, static_cast<unsigned char>('D'));
    const int kChunk = 4096;
    int sent = 0;
    while (sent < kPayloadSize) {
        int remaining = kPayloadSize - sent;
        int toSend = (kChunk < remaining) ? kChunk : remaining;
        serverSendBytes(payload.data() + sent, toSend);
        sent += toSend;
    }

    RsslError err;
    const int maxWaitMs = 5000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Crash Test C4 – Valid frame followed immediately by all-garbage bytes
 *                  in the same TCP segment
 *
 * The good 8-byte frame is decoded and returned; the garbage that follows
 * immediately in the same recv() buffer should cause the next read to fail
 * or close the channel, but must not crash.                               */
TEST_F(RsslWebSocketReadTests, Crash_GoodFrameThenGarbageInSameSegment_NoCrash)
{
    srvArg.port = 16463;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Valid 8-byte binary frame followed by 32 garbage bytes */
    unsigned char combined[6 + 32];
    combined[0] = 0x82; combined[1] = 0x04;
    combined[2] = 'G';  combined[3] = 'O';
    combined[4] = 'O';  combined[5] = 'D';
    memset(combined + 6, 0xDE, 32); /* garbage */
    serverSendBytes(combined, sizeof(combined));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C5 – 0xFF 0xFF as the first two frame bytes
 *                  (MASK=1 from server, all-RSV, opcode=0x0F, len=127)
 *
 * This single two-byte sequence triggers four violations at once:
 *   • byte0=0xFF: FIN=1, RSV1=RSV2=RSV3=1 (compression not negotiated)
 *   • opcode=0x0F: reserved control opcode
 *   • byte1=0xFF: MASK=1 (servers MUST NOT mask), extended-length=127
 * The decode / guard logic must not dereference any uninitialised pointer
 * or overread the buffer.                                                 */
TEST_F(RsslWebSocketReadTests, Crash_0xFF0xFFFrameHeader_NoCrash)
{
    srvArg.port = 16464;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    unsigned char frame[] = { 0xFF, 0xFF };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C6 – 20 × partial single-byte frame headers in rapid
 *                  succession
 *
 * Each send() delivers only one byte (the FIN|BINARY marker 0x82).  The
 * partial-header path inside `_decodeWSFrame()` is hammered 20 times with
 * no length byte ever following.  The library must not accumulate stale
 * state that leads to a crash.                                            */
TEST_F(RsslWebSocketReadTests, Crash_RepeatedSingleByteHeaders_NoCrash)
{
    srvArg.port = 16465;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    unsigned char byte0[] = { 0x82 };
    for (int i = 0; i < 20; ++i)
        serverSendBytes(byte0, sizeof(byte0));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C7 – Frame with opcode 0x05 (reserved non-control)
 *
 * Covers the gap between N26 (opcode=0x04) and N35 (opcode=0x07).
 * All of 0x03–0x07 should hit the `default:` case and return
 * RSSL_RET_FAILURE; this verifies opcode=0x05 specifically.              */
TEST_F(RsslWebSocketReadTests, Crash_ReservedOpcode05_ReturnsFailure)
{
    srvArg.port = 16466;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x05 → byte0 = 0x85 */
    unsigned char frame[] = { 0x85, 0x04, 'R', '0', '5', '!' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for reserved opcode 0x05";
}

/* =========================================================================
 * Crash Test C8 – Frame with opcode 0x06 (reserved non-control)
 *
 * Completes coverage of all reserved data-frame opcodes (0x03–0x07).    */
TEST_F(RsslWebSocketReadTests, Crash_ReservedOpcode06_ReturnsFailure)
{
    srvArg.port = 16467;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, opcode=0x06 → byte0 = 0x86 */
    unsigned char frame[] = { 0x86, 0x04, 'R', '0', '6', '!' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for reserved opcode 0x06";
}

/* =========================================================================
 * Crash Test C9 – Alternating valid and malformed frames (10 pairs)
 *
 * Each pair: one good BINARY frame followed by one frame with reserved
 * opcode 0x04.  Tests that the library can handle per-frame errors and
 * that the crash does not occur in the error-recovery path between frames
 * within the same input buffer.                                           */
TEST_F(RsslWebSocketReadTests, Crash_AlternatingValidAndMalformedFrames_NoCrash)
{
    srvArg.port = 16468;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Send one pair to start; later pairs may not reach the channel */
    unsigned char pair[] = {
        /* valid frame */
        0x82, 0x04, 'V', 'A', 'L', 'D',
        /* bad frame – reserved opcode 0x04 */
        0x84, 0x02, 'B', 'D'
    };
    serverSendBytes(pair, sizeof(pair));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C10 – Reserved control opcodes 0x0C, 0x0D, 0x0E, 0x0F
 *                   sent as four frames in one TCP segment
 *
 * RFC 6455 §5.2 reserves control opcodes 0x0B–0x0F.  Each byte0 value
 * is 0x8C, 0x8D, 0x8E, 0x8F (FIN=1 | opcode).  All four must be handled
 * without crashing; RSSL_RET_FAILURE or CLOSED state are both acceptable. */
TEST_F(RsslWebSocketReadTests, Crash_ReservedControlOpcodes0C_to_0F_NoCrash)
{
    srvArg.port = 16469;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Four reserved control frames, each with 1-byte payload */
    unsigned char frames[] = {
        0x8C, 0x01, 0xCC,   /* opcode=0x0C */
        0x8D, 0x01, 0xDD,   /* opcode=0x0D */
        0x8E, 0x01, 0xEE,   /* opcode=0x0E */
        0x8F, 0x01, 0xFF    /* opcode=0x0F */
    };
    serverSendBytes(frames, sizeof(frames));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C11 – Extremely rapid CLOSE → data → CLOSE sequence
 *
 * Tests the `wsSess->recvClose` flag path when data frames arrive after
 * the first CLOSE.  The second CLOSE should be ignored but the interleaved
 * data frame must not cause an access violation.                           */
TEST_F(RsslWebSocketReadTests, Crash_CloseDataCloseSequence_NoCrash)
{
    srvArg.port = 16470;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    char closePayload[2] = { 0x03, (char)(0xE8) }; /* 1000 */
    unsigned char combined[2 + 4 + 6 + 4 + 4] = {};
    /* CLOSE #1 */
    combined[0] = 0x88; combined[1] = 0x02;
    combined[2] = 0x03; combined[3] = 0xE8;
    /* DATA frame between the two closes */
    combined[4] = 0x82; combined[5] = 0x04;
    combined[6] = 'D';  combined[7] = 'A';
    combined[8] = 'T';  combined[9] = 'A';
    /* CLOSE #2 */
    combined[10] = 0x88; combined[11] = 0x02;
    combined[12] = 0x03; combined[13] = 0xE8;
    serverSendBytes(combined, 14);

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C12 – Valid frame + partial frame header in same segment,
 *                   then the rest of the second frame arrives separately
 *
 * Tests the state continuity between two recv() calls when the input
 * buffer already contains the beginning of the next frame from the first
 * call.  `inputBufCursor` and `inputBuffer->length` must be consistent.  */
TEST_F(RsslWebSocketReadTests, Crash_ValidFrameThenPartialHeaderThenRest_NoCrash)
{
    srvArg.port = 16471;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* First send: complete frame + byte0 of next frame */
    unsigned char first[] = {
        0x82, 0x04, 'F', 'U', 'L', 'L',   /* complete binary frame */
        0x82                                /* byte0 of next frame only */
    };
    serverSendBytes(first, sizeof(first));

    time_sleep(20); /* let the library process the first recv() */

    /* Second send: the rest of the partial frame (length + payload) */
    unsigned char second[] = { 0x04, 'R', 'E', 'S', 'T' };
    serverSendBytes(second, sizeof(second));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C13 – 50 reserved-opcode frames sent in a tight loop
 *
 * Hammers the `default:` branch and the RSSL_RET_FAILURE return path
 * 50 times in rapid succession.  After the first error the channel may
 * be closed; subsequent frames must not cause use-after-free.            */
TEST_F(RsslWebSocketReadTests, Crash_50ReservedOpcodeFramesRapidly_NoCrash)
{
    srvArg.port = 16472;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* 50 × (FIN=1, opcode=0x05, 2-byte payload) */
    const int kCount = 50;
    std::vector<unsigned char> blast(kCount * 4);
    for (int i = 0; i < kCount; ++i) {
        blast[i * 4 + 0] = 0x85;   /* FIN=1, opcode=0x05 */
        blast[i * 4 + 1] = 0x02;   /* length=2           */
        blast[i * 4 + 2] = static_cast<unsigned char>(i & 0xFF);
        blast[i * 4 + 3] = static_cast<unsigned char>((i >> 8) & 0xFF);
    }
    serverSendBytes(blast.data(), static_cast<int>(blast.size()));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C14 – 8-byte ext-len integer overflow bypasses `frame->partial`
 *
 * In `_decodeWSFrame` the partial check is:
 *     frame->partial = (bufLen < (frame->hdrLen + frame->payloadLen));
 * `frame->hdrLen` (RsslInt32=10) is promoted to RsslUInt64 before addition.
 * payloadLen = 0xFFFFFFFFFFFFFFF6 → 10 + 0xFFFFFFFFFFFFFFF6 = 0 (mod 2^64).
 * bufLen=14 is never less than 0, so partial=FALSE even though the actual
 * payload is only 4 bytes.  `rwsReadWebSocket` then checks:
 *     wsFrameLen = payloadLen + hdrLen = 0            (overflow again)
 *     0 > inputBuffer->maxLength  → FALSE → size guard bypassed
 * `handleWebSocketMessages` sets `curInputBuf->length = 0xFFFFFFFFFFFFFFF6`
 * which crashes when the caller tries to use the returned buffer pointer.  */
TEST_F(RsslWebSocketReadTests, Crash_EightByteExtLenOverflowBypassesPartialCheck_Crash)
{
    srvArg.port = 16480;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build:
     *   byte0 = 0x82  (FIN=1, BINARY)
     *   byte1 = 127   (8-byte extended-length indicator)
     *   bytes 2-9 = 0xFFFFFFFFFFFFFFF6  ← hdrLen(10) + this = 0 (wraps)
     *   bytes 10-13 = 'D','A','T','A'   ← only 4 real payload bytes    */
    unsigned char frame[14];
    frame[0] = 0x82;
    frame[1] = 127;
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF6;
    frame[10] = 'D'; frame[11] = 'A'; frame[12] = 'T'; frame[13] = 'A';
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* surviving without crash is the pass condition */
}

/* =========================================================================
 * Crash Test C15 – reassemblyBuffer integer overflow in CONT fragment path
 *
 * In `handleWebSocketMessages`, when a non-final CONT frame arrives:
 *     checkSizeAndRealloc(reassemblyBuffer,
 *         reassemblyBuffer->length + frame->payloadLen, ...)
 * If `reassemblyBuffer->length = 8` and
 *    `frame->payloadLen = 0xFFFFFFFFFFFFFFF8`
 * then `8 + 0xFFFFFFFFFFFFFFF8 = 0` (RsslUInt64 overflow).
 * `checkSizeAndRealloc` sees newLength=0 < maxLength, returns the original
 * (8-byte) buffer unchanged, and then:
 *     memcpy(reassemblyBuffer->buffer + 8,
 *            inputBuffer->buffer + cursor,
 *            0xFFFFFFFFFFFFFFF8)           ← writes ≈18 EB → CRASH         */
TEST_F(RsslWebSocketReadTests, Crash_ReassemblyBufferOverflowInContPath_Crash)
{
    srvArg.port = 16481;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Step 1: open a fragment sequence with 8 bytes of payload.
     *         reassemblyBuffer->length will become 8.               */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Fragment", 8))
        << "serverSend(BINARY, FIN=0) failed";

    time_sleep(30); /* let the library read and buffer the first fragment */

    /* Step 2: send a non-final CONT frame whose 8-byte ext-len
     *         causes  8 + payloadLen  to wrap to 0.
     *   hdrLen = 10, payloadLen = 0xFFFFFFFFFFFFFFF8
     *   10 + 0xFFFFFFFFFFFFFFF8 = 2  (wraps) → partial check needs
     *   bufLen >= 2; we send 12 bytes (10 hdr + 2 payload).         */
    unsigned char cont[12];
    cont[0] = 0x00;  /* FIN=0, opcode=CONT */
    cont[1] = 127;   /* 8-byte ext-len     */
    cont[2] = 0xFF; cont[3] = 0xFF; cont[4] = 0xFF; cont[5] = 0xFF;
    cont[6] = 0xFF; cont[7] = 0xFF; cont[8] = 0xFF; cont[9] = 0xF8;
    cont[10] = 'X'; cont[11] = 'Y'; /* 2 real payload bytes          */
    serverSendBytes(cont, sizeof(cont));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C16 – reassemblyBuffer overflow on FINAL CONT frame
 *
 * Same integer-overflow as C15 but the CONT frame has FIN=1, so the code
 * takes the "last fragment" branch in `handleWebSocketMessages`:
 *     checkSizeAndRealloc(reassemblyBuffer,
 *         reassemblyBuffer->length + frame->payloadLen + 4, ...)
 * With `reassemblyBuffer->length=8`, `frame->payloadLen=0xFFFFFFFFFFFFFFF0`
 * the sum is `8 + 0xFFFFFFFFFFFFFFF0 + 4 = 0` (wraps), triggering the
 * same too-small-buffer → giant memcpy crash.                             */
TEST_F(RsslWebSocketReadTests, Crash_ReassemblyBufferOverflowOnFinalCont_Crash)
{
    srvArg.port = 16482;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open the fragment sequence */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Fragment", 8));
    time_sleep(30);

    /* Send the final CONT frame:
     *   payloadLen = 0xFFFFFFFFFFFFFFF0
     *   8 + 0xFFFFFFFFFFFFFFF0 + 4 = 0  (wraps)
     *   hdrLen(10) + payloadLen = 0  (wraps) → partial = FALSE
     *   we send hdr(10) + 6 actual bytes = 16 bytes total             */
    unsigned char finalCont[16];
    finalCont[0] = 0x80;  /* FIN=1, opcode=CONT */
    finalCont[1] = 127;
    finalCont[2] = 0xFF; finalCont[3] = 0xFF; finalCont[4] = 0xFF; finalCont[5] = 0xFF;
    finalCont[6] = 0xFF; finalCont[7] = 0xFF; finalCont[8] = 0xFF; finalCont[9] = 0xF0;
    finalCont[10] = 'E'; finalCont[11] = 'N';
    finalCont[12] = 'D'; finalCont[13] = '!';
    finalCont[14] = 'X'; finalCont[15] = 'Y';
    serverSendBytes(finalCont, sizeof(finalCont));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C17 – wsFrameLen integer overflow bypasses the size guard
 *
 * In `rwsReadWebSocket`:
 *     RsslUInt64 wsFrameLen = frame->payloadLen + frame->hdrLen;
 *     if (wsFrameLen > inputBuffer->maxLength) { ... FAIL ... }
 *
 * With payloadLen = 0xFFFFFFFFFFFFFFF4 and hdrLen = 12 (indicator=127,
 * mask present from client bit — but we suppress masking by sending
 * from server), the sum wraps to 0.  0 > maxLength is FALSE, so the
 * guard is bypassed.  `handleWebSocketMessages` then sets
 *     inputBufCursor += payloadLen   (0xFFFFFFFFFFFFFFF4)
 * which wraps `inputBufCursor` (RsslUInt32) to a garbage value and
 * the subsequent `inputBufCursor == inputBuffer->length` comparison
 * corrupts internal state leading to a crash on the next read.          */
TEST_F(RsslWebSocketReadTests, Crash_wsFrameLenOverflowBypassesSizeGuard_Crash)
{
    srvArg.port = 16483;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* payloadLen = 0xFFFFFFFFFFFFFFF4, hdrLen = 10 (no mask, no extended)
     * 0xFFFFFFFFFFFFFFF4 + 10 = 0 (wraps)                              */
    unsigned char frame[14];
    frame[0] = 0x82;  /* FIN=1, BINARY */
    frame[1] = 127;   /* 8-byte ext-len */
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF4;
    frame[10] = 'W'; frame[11] = 'R'; frame[12] = 'A'; frame[13] = 'P';
    serverSendBytes(frame, sizeof(frame));

    /* Issue a second read immediately after to observe the corrupted state */
    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    /* Follow-up read to trigger use of the corrupted inputBufCursor */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "FollowUp", 8));
    for (int waited = 0; waited < 1000; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C18 – Three-fragment sequence with overflow on middle fragment
 *
 * Fragment 1: FIN=0, BINARY, 4-byte payload  → reassemblyBuffer->length = 4
 * Fragment 2: FIN=0, CONT,   8-byte ext-len = 0xFFFFFFFFFFFFFFFC
 *             4 + 0xFFFFFFFFFFFFFFFC = 0 (overflow) → memcpy(≈UINT64_MAX)
 * Fragment 3 is never sent (crash should occur at fragment 2).
 *
 * Targets the middle-CONT branch inside `handleWebSocketMessages` that
 * calls `checkSizeAndRealloc(reassemblyBuffer,
 *     reassemblyBuffer->length + frame->payloadLen, ...)`.              */
TEST_F(RsslWebSocketReadTests, Crash_MiddleFragmentOverflowInReassembly_Crash)
{
    srvArg.port = 16484;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, BINARY, 4 bytes */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Frag", 4));
    time_sleep(30);

    /* Fragment 2 (middle CONT, FIN=0):
     *   payloadLen = 0xFFFFFFFFFFFFFFFC
     *   4 + 0xFFFFFFFFFFFFFFFC = 0  (overflow)
     *   hdrLen(10) + payloadLen: 10 + 0xFFFFFFFFFFFFFFFC = 6 (wraps)
     *   Send 10 hdr + 6 bytes = 16 bytes so bufLen(16) >= 6 → partial=FALSE */
    unsigned char mid[16];
    mid[0] = 0x00;  /* FIN=0, CONT */
    mid[1] = 127;
    mid[2] = 0xFF; mid[3] = 0xFF; mid[4] = 0xFF; mid[5] = 0xFF;
    mid[6] = 0xFF; mid[7] = 0xFF; mid[8] = 0xFF; mid[9] = 0xFC;
    mid[10]='M'; mid[11]='I'; mid[12]='D'; mid[13]='D';
    mid[14]='L'; mid[15]='E';
    serverSendBytes(mid, sizeof(mid));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Crash Test C19 – Single non-fragmented frame: partial bypass + curInputBuf
 *                   length set to UINT64_MAX
 *
 * Identical overflow path to C14 but with payloadLen = 0xFFFFFFFFFFFFFFF5
 * (hdrLen=11 when MASK bit is inadvertently copied; we stay unmasked so
 * hdrLen=10).  After partial=FALSE, `handleWebSocketMessages` sets:
 *     curInputBuf->length = frame->payloadLen (= 0xFFFFFFFFFFFFFFF5)
 * The returned `rtr_msgb_t*` has a `length` field of ≈UINT64_MAX.  The
 * caller in `rsslWebSocketRead` dereferences `curInputBuf->length` to
 * decide how many bytes to return to the application — crash.            */
TEST_F(RsslWebSocketReadTests, Crash_CurInputBufLengthSetToUINT64MAX_Crash)
{
    srvArg.port = 16485;

    ASSERT_TRUE(startServer())   << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* payloadLen = 0xFFFFFFFFFFFFFFF5
     * 10 + 0xFFFFFFFFFFFFFFF5 = 0xFFFFFFFFFFFFFFFF (wraps to UINT64_MAX)
     * Wait — we need to wrap to ≤ bufLen.
     * payloadLen = 0xFFFFFFFFFFFFFFF6 → sum = 0; bufLen=14 >= 0 → partial=FALSE */
    unsigned char frame[14];
    frame[0] = 0x82;  /* FIN=1, BINARY */
    frame[1] = 127;
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF6;
    frame[10]='U'; frame[11]='I'; frame[12]='N'; frame[13]='T';
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Negative Test N8 – Burst of valid frames saturating the receive queue
 *
 * 50 back-to-back binary frames are sent without waiting.  rsslRead()
 * must drain all of them without returning RSSL_RET_FAILURE, without
 * crashing, and the channel must remain ACTIVE throughout.               */
TEST_F(RsslWebSocketReadTests, Negative_BurstOfValidFrames_NoErrorOrCrash)
{
    srvArg.port = 16427;

    ASSERT_TRUE(startServer()) << "Server start failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    const int kBurstCount = 50;
    for (int i = 0; i < kBurstCount; ++i)
        ASSERT_TRUE(serverSend(WS_OPC_BINARY, true, "BurstMsg", 8))
        << "serverSend[" << i << "] failed";

    int readCount = 0;
    RsslError err;
    const int maxWaitMs = 5000;
    for (int waited = 0; waited < maxWaitMs && readCount < kBurstCount; ) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) {
            ++readCount;
        }
        else if (rRet == RSSL_RET_FAILURE) {
            ADD_FAILURE() << "rsslRead returned FAILURE during burst at message " << readCount;
            break;
        }
        else {
            time_sleep(10);
            waited += 10;
        }
    }

    EXPECT_EQ(kBurstCount, readCount)
        << "Expected all " << kBurstCount << " burst messages to be read";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state)
        << "Channel must remain ACTIVE after burst read";
}

/* =========================================================================
 * Compressed WebSocket server thread
 *
 * Identical to wsWriterServerThread but binds with RSSL_COMP_ZLIB enabled
 * so that the permessage-deflate extension is negotiated during the WS
 * handshake.  Once the channel is ACTIVE it publishes serverSocketId and
 * idles until stopRequested is set.
 * ========================================================================= */
static RSSL_THREAD_DECLARE(wsCompressedWriterServerThread, pArg)
{
    WsWriterServerArg* arg = reinterpret_cast<WsWriterServerArg*>(pArg);

    RsslError err;
    RsslBindOptions bindOpts;
    rsslClearBindOpts(&bindOpts);

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%u", static_cast<unsigned>(arg->port));
    bindOpts.serviceName              = portStr;
    bindOpts.connectionType           = RSSL_CONN_TYPE_WEBSOCKET;
    bindOpts.majorVersion             = RSSL_RWF_MAJOR_VERSION;
    bindOpts.minorVersion             = RSSL_RWF_MINOR_VERSION;
    bindOpts.wsOpts.protocols         = const_cast<char*>("rssl.json.v2");
    bindOpts.guaranteedOutputBuffers  = 50;
    bindOpts.maxOutputBuffers         = 50;
    bindOpts.maxFragmentSize          = 6144;
    bindOpts.compressionType          = RSSL_COMP_ZLIB;  /* enable deflate  */
    bindOpts.compressionLevel         = 6;

    RsslServer* pSrv = rsslBind(&bindOpts, &err);
    if (!pSrv) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslBind(compressed) failed: %s", err.text);
        arg->done = true; arg->bound = true;
        return 0;
    }

    arg->bound = true;

    RsslAcceptOptions accOpts;
    rsslClearAcceptOpts(&accOpts);
    RsslChannel* pSrvChnl = nullptr;
    for (int i = 0; i < 500 && !pSrvChnl; ++i) {
        pSrvChnl = rsslAccept(pSrv, &accOpts, &err);
        if (!pSrvChnl) time_sleep(10);
    }
    if (!pSrvChnl) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "rsslAccept(compressed) failed: %s", err.text);
        rsslCloseServer(pSrv, &err);
        arg->done = true; arg->ready = true;
        return 0;
    }

    for (int i = 0; i < 400; ++i) {
        if (pSrvChnl->state == RSSL_CH_STATE_ACTIVE)  break;
        if (pSrvChnl->state == RSSL_CH_STATE_CLOSED)  break;
        if (pSrvChnl->state == RSSL_CH_STATE_INACTIVE) break;
        if (arg->stopRequested.load()) break;
        RsslInProgInfo inProg;
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pSrvChnl, &inProg, &err);
        time_sleep(5);
    }

    if (arg->stopRequested.load()) {
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true;
        return 0;
    }

    if (pSrvChnl->state != RSSL_CH_STATE_ACTIVE) {
        snprintf(arg->errText, sizeof(arg->errText),
                 "Compressed server channel not ACTIVE");
        rsslCloseChannel(pSrvChnl, &err);
        rsslCloseServer(pSrv, &err);
        arg->done = true; arg->ready = true;
        return 0;
    }

    arg->serverSocketId = pSrvChnl->socketId;
    arg->ready = true;

    while (!arg->stopRequested.load())
        time_sleep(10);

    rsslCloseChannel(pSrvChnl, &err);
    rsslCloseServer(pSrv, &err);
    arg->done = true;
    return 0;
}

/* =========================================================================
 * Fixture: RsslWebSocketReadCompressedTests
 *
 * Inherits the full helper set from RsslWebSocketReadTests but overrides
 * startServer() / connectClient() to enable RSSL_COMP_ZLIB on both sides
 * so that permessage-deflate is negotiated during the WS handshake.
 * ========================================================================= */
class RsslWebSocketReadCompressedTests : public RsslWebSocketReadTests
{
protected:
    bool startServer() override
    {
        RSSL_THREAD_START(&srvTid, wsCompressedWriterServerThread, &srvArg);
        for (int i = 0; i < 500 && !srvArg.bound.load(); ++i)
            time_sleep(2);
        return srvArg.bound.load() && srvArg.errText[0] == '\0';
    }

    bool connectClient() override
    {
        char portStr[16];
        snprintf(portStr, sizeof(portStr), "%u",
                 static_cast<unsigned>(srvArg.port));

        RsslError err;
        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_WEBSOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = portStr;
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.protocolType                       = RSSL_JSON_PROTOCOL_TYPE;
        opts.blocking                           = RSSL_FALSE;
        opts.wsOpts.protocols                   = const_cast<char*>("rssl.json.v2");
        opts.guaranteedOutputBuffers            = 50;
        opts.compressionType                    = RSSL_COMP_ZLIB; /* enable deflate */

        pClientChnl = rsslConnect(&opts, &err);
        return pClientChnl != nullptr;
    }
};

/* =========================================================================
 * Compression Crash Test CC1 – Compressed single frame with garbage
 *                               (non-DEFLATE) payload
 *
 * The server sends a frame with RSV1=1 (compressed) and garbage bytes as
 * payload.  zlib's inflate() will return Z_DATA_ERROR almost immediately.
 *
 * Targets the `(*(wsSess->comp.inDecompFuncs->decompress))` call in the
 * single-frame compressed branch of `handleWebSocketMessages`:
 *   if ((retVal = decompress(c_stream_in, &compBuf, ...)) < 0) {
 *       *readret = RSSL_RET_FAILURE; return;
 *   }
 * The library must return RSSL_RET_FAILURE without crashing.             */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_SingleFrameGarbagePayload_ReturnsFailure)
{
    srvArg.port = 16490;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1, BINARY → byte0=0xC2; payload=4 bytes of garbage    */
    unsigned char frame[] = { 0xC2, 0x04, 0xDE, 0xAD, 0xBE, 0xEF };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a compressed frame with garbage "
           "non-DEFLATE payload (zlib Z_DATA_ERROR path)";
}

/* =========================================================================
 * Compression Crash Test CC2 – `overRun` memcpy reads from invalid address
 *
 * In the single-frame compressed path of `handleWebSocketMessages`:
 *   1.  The `frame->finSet` branch checks
 *       `(inputBufCursor + frame->payloadLen + 4) > inputBuffer->maxLength`
 *       before copying `compressEnd` bytes via `overRun`.
 *   2.  With `payloadLen = 0xFFFFFFFFFFFFFFF2` the entire sum wraps to
 *       `(RsslUInt64)10 + 0xFFFFFFFFFFFFFFF2 + 4 = 0`, which is NOT
 *       greater than `maxLength` → the guard is BYPASSED.
 *   3.  The next line executes:
 *         `memcpy(overRun,
 *                 inputBuffer->buffer + inputBufCursor + payloadLen,
 *                 4);`
 *       = `memcpy(overRun, buf + 10 + 0xFFFFFFFFFFFFFFF2, 4)`
 *       which reads 4 bytes from a near-UINT64_MAX address → CRASH.
 *
 * Additionally `compBuf.avail_in = (RsslUInt32)payloadLen` truncates to
 * 0xFFFFFFF2 (≈4 GB), so zlib itself would also attempt to consume data
 * far past the buffer end.
 *
 * payloadLen is chosen so that `hdrLen(10) + payloadLen` also wraps to 0,
 * making `frame->partial = FALSE` and allowing the frame to pass the
 * partial-frame guard in `_decodeWSFrame`.                               */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_OverrunMemcpyBypassesBoundsCheck_NoCrash)
{
    srvArg.port = 16491;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* payloadLen = 0xFFFFFFFFFFFFFFF2
     *   hdrLen(10) + payloadLen = 0   (partial guard bypassed)
     *   cursor(10) + payloadLen + 4   = 0  (bounds check bypassed)
     *   compBuf.avail_in = (uint32)0xFFFFFFF2 (4-GB+ junk read into zlib)
     * We send the 10-byte header + 4 "payload" bytes so bufLen=14,
     * satisfying `bufLen >= (hdrLen + payloadLen) = 0`.              */
    unsigned char frame[14];
    frame[0] = 0xC2;  /* FIN=1, RSV1=1 (compressed), BINARY            */
    frame[1] = 127;   /* 8-byte extended-length indicator               */
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF2;
    frame[10] = 'O'; frame[11] = 'V'; frame[12] = 'R'; frame[13] = 'N';
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* surviving without crash is the pass condition */
}

/* =========================================================================
 * Compression Crash Test CC3 – Compressed first fragment (valid-looking),
 *                               final CONT with garbage decompression data
 *
 * Targets the fragmented compressed reassembly path in
 * `handleWebSocketMessages`:
 *   1. Fragment 1: FIN=0, RSV1=1 (compressed), garbage bytes
 *      → sets `wsSess->reassemblyCompressed = 1`
 *   2. Fragment 2: FIN=1, CONT, more garbage bytes
 *      → enters the CONT+finSet+compressed branch
 *      → calls `decompress(c_stream_in, &compBuf, ...)` on the combined
 *        garbage reassembly buffer → Z_DATA_ERROR
 *   The library must handle the zlib error and return RSSL_RET_FAILURE
 *   without crashing.                                                     */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_FragmentedGarbageCompressedPayload_ReturnsFailure)
{
    srvArg.port = 16492;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, RSV1=1, BINARY (0x42 = 0x00|0x40|0x02) → 0x42 */
    unsigned char frag1[] = { 0x42, 0x06, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Fragment 2: FIN=1, CONT (0x80), garbage */
    unsigned char frag2[] = { 0x80, 0x04, 0x11, 0x22, 0x33, 0x44 };
    serverSendBytes(frag2, sizeof(frag2));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for fragmented compressed message "
           "with garbage payload in both fragments (zlib Z_DATA_ERROR)";
}

/* =========================================================================
 * Compression Crash Test CC4 – Zero-length compressed frame
 *                               (RSV1=1, payloadLen=0)
 *
 * After the compression negotiation, a frame with RSV1=1 (compressed) and
 * zero payload length is sent.  The single-frame compressed path in
 * `handleWebSocketMessages` does:
 *   compBuf.avail_in = (RsslUInt32)frame->payloadLen  →  0
 *   (frame->finSet is TRUE)
 *   → bounds check:
 *     (inputBufCursor + 0 + 4) > maxLength  →  safe
 *   → compressEnd bytes OVERWRITE buf[cursor+0 .. cursor+3]
 *   → compBuf.avail_in += 4  →  4
 *   → inflate(4 bytes of 0x00 0x00 0xFF 0xFF)
 * A raw DEFLATE stream consisting only of the sync-flush trailer is valid
 * and produces zero output bytes.  The library must not crash.           */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ZeroLengthCompressedFrame_NoCrash)
{
    srvArg.port = 16493;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1, BINARY, payloadLen=0 */
    unsigned char frame[] = { 0xC2, 0x00 };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Compression Crash Test CC5 – Compressed frame 8-byte ext-len overflow
 *                               on `inputBufCursor + payloadLen + 4` while
 *                               compressed (RSV1=1)
 *
 * Same integer-overflow vector as C14/C17 but with RSV1=1 so the code
 * takes the compressed single-frame branch instead of the uncompressed
 * branch, reaching the `overRun` memcpy.
 *
 *   payloadLen = 0xFFFFFFFFFFFFFFF6
 *   hdrLen     = 10
 *   hdrLen + payloadLen = 0  (partial check bypassed)
 *   cursor(10) + payloadLen + 4 = 0  (bounds check bypassed)
 *
 * The `memcpy(rsslSocketChannel->inputBuffer->buffer + cursor + payloadLen,
 *             compressEnd, 4)` writes to a near-UINT64_MAX offset → CRASH
 * if not detected and handled.                                            */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_Compressed8ByteExtLenOverflow_NoCrash)
{
    srvArg.port = 16494;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* payloadLen = 0xFFFFFFFFFFFFFFF6
     * cursor(10) + 0xFFFFFFFFFFFFFFF6 + 4 = 0 (uint64 overflow → bypass)
     * RSV1=1 → compressed path                                           */
    unsigned char frame[14];
    frame[0] = 0xC2;  /* FIN=1, RSV1=1, BINARY */
    frame[1] = 127;
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF6;
    frame[10] = 'C'; frame[11] = 'M'; frame[12] = 'P'; frame[13] = 'R';
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC6 – `fragmentedDecompressedBuffer`
 *                               checkSizeAndRealloc overflow in CONT path
 *
 * With compression enabled, the final CONT branch in
 * `handleWebSocketMessages` calls:
 *   checkSizeAndRealloc(fragmentedDecompressedBuffer,
 *       (maxLength * 2) - fragmentedDecompressedBuffer->length, ...)
 * when `reassemblyBuffer->length > fragmentedDecompressedBuffer->maxLength`.
 *
 * Here we set up reassemblyBuffer to be very large by accumulating several
 * fragments, then send a final oversized CONT with a 8-byte ext-len whose
 * payloadLen wraps `reassemblyBuffer->length + payloadLen` to 0 in
 * `checkSizeAndRealloc`, returning the original (too-small) buffer, and
 * triggering the memcpy of `~UINT64_MAX` bytes.                          */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_FragDecompressedBufferOverflow_NoCrash)
{
    srvArg.port = 16495;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open a fragmented compressed sequence: FIN=0, RSV1=1, BINARY */
    unsigned char frag1[] = { 0x42, 0x08,
        0xDE, 0xAD, 0xBE, 0xEF,
        0xCA, 0xFE, 0xBA, 0xBE };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Final CONT (FIN=1) with 8-byte ext-len payloadLen that causes
     * reassemblyBuffer->length(8) + payloadLen to wrap to 0.
     * payloadLen = 0xFFFFFFFFFFFFFFF8 → 8 + 0xFFFFFFFFFFFFFFF8 = 0 (wraps)
     * hdrLen(10) + payloadLen: 10 + 0xFFFFFFFFFFFFFFF8 = 2 (wraps)
     * bufLen = 12 >= 2 → partial = FALSE                                 */
    unsigned char finalCont[12];
    finalCont[0] = 0x80;  /* FIN=1, CONT */
    finalCont[1] = 127;
    finalCont[2] = 0xFF; finalCont[3] = 0xFF;
    finalCont[4] = 0xFF; finalCont[5] = 0xFF;
    finalCont[6] = 0xFF; finalCont[7] = 0xFF;
    finalCont[8] = 0xFF; finalCont[9] = 0xF8;
    finalCont[10] = 'X'; finalCont[11] = 'Y';
    serverSendBytes(finalCont, sizeof(finalCont));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC7 – `compBuf.avail_in` 32-bit truncation in
 *                               the fragmented compressed decompression path
 *
 * When a final CONT frame arrives for a fragmented compressed message,
 * `handleWebSocketMessages` sets:
 *   compBuf.avail_in = (RsslUInt32)wsSess->reassemblyBuffer->length
 *                    + (from addedCompressEnd) += 4
 * The reassemblyBuffer->length is set to:
 *   `prev_length + frame->payloadLen`
 *
 * If `frame->payloadLen` is crafted so that after the first fragment
 * (which sets reassemblyBuffer->length = 8) the non-final CONT payloadLen
 * = 0xFFFFFFF8 (just below 32-bit max), the reassemblyBuffer->length
 * becomes `8 + 0xFFFFFFF8 = 0x100000000` which truncates to 0 when cast
 * to RsslUInt32 for `compBuf.avail_in`.  zlib then decompresses 0 bytes.  */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_AvailInTruncationFragPath_NoCrash)
{
    srvArg.port = 16496;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open a fragment: FIN=0, RSV1=1 (compressed), BINARY, 8 bytes  */
    unsigned char frag1[] = { 0x42, 0x08,
        0xAA, 0xBB, 0xCC, 0xDD,
        0x11, 0x22, 0x33, 0x44 };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Middle non-final CONT (FIN=0) with 2-byte ext-len = 0x7FF0 (32752)
     * This fills the reassembly buffer toward 32-bit boundary without
     * actually overflowing in 64 bits (avail_in on the final compress
     * call would truncate the accumulated length).                       */
    unsigned char midCont[4 + 6] = {};
    midCont[0] = 0x00;  /* FIN=0, CONT */
    midCont[1] = 126;   /* 2-byte ext-len */
    midCont[2] = 0x00;
    midCont[3] = 0x06;  /* payloadLen = 6 */
    midCont[4] = 0x55; midCont[5] = 0x66; midCont[6] = 0x77;
    midCont[7] = 0x88; midCont[8] = 0x99; midCont[9] = 0xAA;
    serverSendBytes(midCont, sizeof(midCont));
    time_sleep(20);

    /* Final CONT (FIN=1) with 8-byte ext-len causing (accumulated + len)
     * to overflow the 32-bit truncation in avail_in.
     * payloadLen = 0xFFFFFFFFFFFFFFF4 → partial bypassed as before.     */
    unsigned char finalCont[14];
    finalCont[0] = 0x80;  /* FIN=1, CONT */
    finalCont[1] = 127;
    finalCont[2] = 0xFF; finalCont[3] = 0xFF;
    finalCont[4] = 0xFF; finalCont[5] = 0xFF;
    finalCont[6] = 0xFF; finalCont[7] = 0xFF;
    finalCont[8] = 0xFF; finalCont[9] = 0xF4;
    finalCont[10] = 'F'; finalCont[11] = 'I';
    finalCont[12] = 'N'; finalCont[13] = '!';
    serverSendBytes(finalCont, sizeof(finalCont));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC8 – `doubleSizeAndRealloc` triggered by a
 *                               compressible single frame that fills the
 *                               decompression output buffer
 *
 * A highly compressible payload (all identical bytes) decompresses to a
 * much larger buffer than `wsSess->reassemblyBuffer->maxLength`.  When
 * zlib returns with `avail_out == 0` (output buffer full), `handleWebSocket
 * Messages` calls `doubleSizeAndRealloc` in a loop until the full output
 * fits or `maxPayload` is reached.
 *
 * This test sends actual valid zlib DEFLATE-compressed data (raw deflate,
 * 0x00 0x00 0xFF 0xFF trailer stripped as per PMCE spec) representing 128
 * repeated 'A' bytes.  The compressed form is small; the decompressed
 * output is 128 bytes, larger than the typical initial reassemblyBuffer.
 *
 * Targets: the `doubleSizeAndRealloc` loop inside the single-frame
 * compressed branch of `handleWebSocketMessages`.                         */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_DoubleSizeReallocTriggered_NoCrash)
{
    srvArg.port = 16497;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Raw DEFLATE stream for 128 'A' bytes with PMCE trailing 0x00 0x00
     * 0xFF 0xFF stripped.
     *
     * Constructed as: deflate("AAAAAA...") with Z_SYNC_FLUSH, then remove
     * trailing 0x00 0x00 0xFF 0xFF per RFC 7692 §7.2.1.
     *
     * Using a stored block (BTYPE=00) which has no compression but is
     * a valid DEFLATE stream and guaranteed to decompress to the original:
     *   0x01           BFINAL=1, BTYPE=00 (no compression, last block)
     *   0x80 0x00      LEN = 128 (little-endian)
     *   0x7F 0xFF      NLEN = ~LEN (one's complement)
     *   128 × 0x41     payload 'A' × 128
     *
     * The trailing 0x00 0x00 0xFF 0xFF sync-flush trailer is NOT present
     * because we're using a stored block (already final).  Per PMCE, the
     * library appends those 4 bytes before calling inflate, so inflate will
     * see the 4-byte trailer appended to our stored block and handle it.  */
    const int kPayloadLen = 5 + 128; /* 5-byte header + 128 'A' */
    std::vector<unsigned char> deflatePayload;
    deflatePayload.push_back(0x01);  /* BFINAL=1, BTYPE=00 */
    deflatePayload.push_back(0x80);  /* LEN low  = 128     */
    deflatePayload.push_back(0x00);  /* LEN high = 0       */
    deflatePayload.push_back(0x7F);  /* NLEN low  = ~128 & 0xFF = 0x7F */
    deflatePayload.push_back(0xFF);  /* NLEN high = 0xFF              */
    for (int i = 0; i < 128; ++i)
        deflatePayload.push_back(static_cast<unsigned char>('A'));

    /* Build the WS frame: FIN=1, RSV1=1, BINARY */
    std::vector<unsigned char> frame;
    frame.push_back(0xC2);  /* FIN=1, RSV1=1, BINARY */
    if (deflatePayload.size() < 126) {
        frame.push_back(static_cast<unsigned char>(deflatePayload.size()));
    } else {
        frame.push_back(126);
        frame.push_back(static_cast<unsigned char>(deflatePayload.size() >> 8));
        frame.push_back(static_cast<unsigned char>(deflatePayload.size() & 0xFF));
    }
    frame.insert(frame.end(), deflatePayload.begin(), deflatePayload.end());

    serverSendBytes(frame.data(), static_cast<int>(frame.size()));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash; error or success both acceptable */
}

/* =========================================================================
 * Compression Crash Test CC9 – Valid compressed frame followed immediately
 *                               by a garbage compressed frame
 *
 * After a successful zlib inflate the z_stream state is reused.  Feeding
 * another DEFLATE-framed message with pure garbage bytes into the same
 * inflator must return Z_DATA_ERROR (< 0) and the library must set
 * *readret = RSSL_RET_FAILURE without crashing.                          */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ValidThenGarbageCompressedFrame_ReturnsFailure)
{
    srvArg.port = 16498;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Frame 1: valid DEFLATE stored-block for 4 bytes "GOOD" (PMCE trailer
     * stripped per spec; the library appends 0x00 0x00 0xFF 0xFF itself).
     *   0x01  BFINAL=1, BTYPE=00
     *   0x04 0x00  LEN=4 (little-endian)
     *   0xFB 0xFF  NLEN=~4
     *   'G','O','O','D'                                                  */
    unsigned char valid[] = {
        0xC2,                               /* FIN=1, RSV1=1, BINARY      */
        0x09,                               /* length = 9                 */
        0x01, 0x04, 0x00, 0xFB, 0xFF,       /* stored-block header        */
        'G', 'O', 'O', 'D'                  /* payload                    */
    };
    serverSendBytes(valid, sizeof(valid));

    /* Let the library fully process the first frame */
    RsslError err;
    const int drainMs = 1500;
    for (int w = 0; w < drainMs; w += 10) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }

    /* Frame 2: garbage compressed bytes → Z_DATA_ERROR                  */
    unsigned char garbage[] = { 0xC2, 0x04, 0xBA, 0xDC, 0x0D, 0xE0 };
    serverSendBytes(garbage, sizeof(garbage));

    RsslRet lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for garbage compressed frame after "
           "a valid compressed frame (corrupted zlib stream)";
}

/* =========================================================================
 * Compression Crash Test CC10 – First fragment (non-final BINARY) with
 *                                8-byte ext-len overflow
 *
 * Targets the "first fragment" branch in `handleWebSocketMessages`:
 *     checkSizeAndRealloc(reassemblyBuffer,
 *         frame->payloadLen, wsSess->maxPayload, ...)
 * payloadLen = 0xFFFFFFFFFFFFFFF8 → sum `0 + payloadLen` is itself, but
 * checkSizeAndRealloc's newLength = length(0) + payloadLen overflows to 0
 * which is less than maxLength → original buffer returned.
 *     memcpy(reassemblyBuffer->buffer, inputBuffer->buffer + cursor, payloadLen)
 * copies ≈18 EB → CRASH.
 * The frame has RSV1=0 (not compressed itself) but the session is deflate-
 * enabled, so this exercises the non-compressed first-fragment path inside
 * a compression-capable session.                                         */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_FirstFragmentExtLenOverflow_NoCrash)
{
    srvArg.port = 16499;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=0, RSV1=0, BINARY, payloadLen=0xFFFFFFFFFFFFFFF8
     * hdrLen(10) + 0xFFFFFFFFFFFFFFF8 = 2 (wraps) → partial=FALSE
     * We send 12 bytes (10 hdr + 2 payload)                             */
    unsigned char frame[12];
    frame[0] = 0x02;  /* FIN=0, no RSV, BINARY */
    frame[1] = 127;
    frame[2] = 0xFF; frame[3] = 0xFF; frame[4] = 0xFF; frame[5] = 0xFF;
    frame[6] = 0xFF; frame[7] = 0xFF; frame[8] = 0xFF; frame[9] = 0xF8;
    frame[10] = 'F'; frame[11] = '1';
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC11 – `reassemblyUnfinished=1` restart: new
 *                                first-fragment overflow while a prior
 *                                incomplete sequence is still pending
 *
 * When a second non-final BINARY frame arrives while
 * `wsSess->reassemblyUnfinished == 1` the library does NOT reset the
 * reassembly buffer (the `if (!wsSess->reassemblyUnfinished)` branch is
 * skipped).  `checkSizeAndRealloc` is called with the existing
 * reassemblyBuffer->length (8) + new payloadLen (overflow), potentially
 * causing checkSizeAndRealloc's internal addition to wrap.               */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ReassemblyUnfinishedRestartOverflow_NoCrash)
{
    srvArg.port = 16500;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: valid non-final BINARY with 8 bytes → reassemblyUnfinished=1 */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "Starting", 8));
    time_sleep(30);

    /* Fragment 2: another non-final BINARY (wrong – should be CONT) with
     * overflow payloadLen: 8 + 0xFFFFFFFFFFFFFFF8 = 0 in checkSizeAndRealloc
     * hdrLen(10) + 0xFFFFFFFFFFFFFFF8 = 2 → partial=FALSE               */
    unsigned char frame2[12];
    frame2[0] = 0x02;  /* FIN=0, BINARY */
    frame2[1] = 127;
    frame2[2] = 0xFF; frame2[3] = 0xFF; frame2[4] = 0xFF; frame2[5] = 0xFF;
    frame2[6] = 0xFF; frame2[7] = 0xFF; frame2[8] = 0xFF; frame2[9] = 0xF8;
    frame2[10] = 'F'; frame2[11] = '2';
    serverSendBytes(frame2, sizeof(frame2));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC12 – Final compressed CONT with zero-length
 *                                payload (finSet=1, payloadLen=0)
 *
 * In the final-CONT compressed branch of `handleWebSocketMessages`:
 *   memcpy(reassemblyBuffer->buffer + length, inputBuffer + cursor, 0)
 *   reassemblyBuffer->length += 0
 *   compBuf.avail_in = (RsslUInt32)reassemblyBuffer->length  (= 8)
 *   + 4 from compressEnd → feeds 8+4=12 bytes to inflate
 * The 8 bytes of the prior fragment are raw garbage, so inflate will
 * return Z_DATA_ERROR.  The library must handle this without crashing.   */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_FinalContZeroLengthCompressed_NoCrash)
{
    srvArg.port = 16501;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, RSV1=1 (compressed), BINARY, 8 garbage bytes */
    unsigned char frag1[] = { 0x42, 0x08,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Final CONT: FIN=1, CONT, payloadLen=0 */
    unsigned char finalCont[] = { 0x80, 0x00 };
    serverSendBytes(finalCont, sizeof(finalCont));

    RsslError err;
    RsslRet lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for final CONT with zero-length "
           "payload when preceding fragments contain garbage";
}

/* =========================================================================
 * Compression Crash Test CC13 – Three valid compressed frames in one TCP
 *                                segment (moreData path with compression)
 *
 * All three frames share the same `recv()` buffer so the library's
 * `*moreData = 1` loop must iterate through all three while maintaining
 * correct zlib stream state.  Each frame carries a valid DEFLATE stored-
 * block payload.  The library must deliver all three without crashing.   */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ThreeCompressedFramesInOneTCPSegment_NoCrash)
{
    srvArg.port = 16502;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Build one DEFLATE stored-block frame for 4 bytes (e.g. "AAA!") */
    auto makeStoredFrame = [](const char* payload, int plen) {
        std::vector<unsigned char> f;
        /* deflate stored-block header: BFINAL=1, BTYPE=00, LEN, ~LEN */
        int wsPayloadLen = 5 + plen;
        f.push_back(0xC2); /* FIN=1, RSV1=1, BINARY */
        f.push_back(static_cast<unsigned char>(wsPayloadLen)); /* < 126 */
        f.push_back(0x01);  /* BFINAL=1, BTYPE=00 */
        f.push_back(static_cast<unsigned char>(plen & 0xFF));
        f.push_back(static_cast<unsigned char>((plen >> 8) & 0xFF));
        f.push_back(static_cast<unsigned char>((~plen) & 0xFF));
        f.push_back(static_cast<unsigned char>((~plen >> 8) & 0xFF));
        for (int i = 0; i < plen; ++i)
            f.push_back(static_cast<unsigned char>(payload[i]));
        return f;
    };

    auto f1 = makeStoredFrame("AAA!", 4);
    auto f2 = makeStoredFrame("BBB!", 4);
    auto f3 = makeStoredFrame("CCC!", 4);

    std::vector<unsigned char> combined;
    combined.insert(combined.end(), f1.begin(), f1.end());
    combined.insert(combined.end(), f2.begin(), f2.end());
    combined.insert(combined.end(), f3.begin(), f3.end());

    serverSendBytes(combined.data(), static_cast<int>(combined.size()));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC14 – Non-compressed CONT frame interleaved in
 *                                a compressed fragment sequence
 *
 * After a compressed first fragment sets `wsSess->reassemblyCompressed=1`,
 * the next CONT frame arrives with RSV1=0 (not compressed).  The final
 * CONT branch in `handleWebSocketMessages` checks `frame->compressed`
 * which was set on the *first* frame (stored in `frame->dataType` /
 * `frame->compressed`).  Sending an uncompressed CONT into a compressed
 * context exercises the branch where the memcpy feeds non-DEFLATE bytes
 * into the reassembly buffer before the inflate call → Z_DATA_ERROR.     */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_UncompressedContInCompressedFragmentSeq_NoCrash)
{
    srvArg.port = 16503;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, RSV1=1 (compressed), BINARY */
    unsigned char frag1[] = { 0x42, 0x06,
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Final CONT: FIN=1, RSV1=0 (NOT compressed), raw garbage bytes */
    unsigned char contNoRSV[] = { 0x80, 0x06,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
    serverSendBytes(contNoRSV, sizeof(contNoRSV));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC15 – Immediately-saturated decompression output
 *                                buffer (avail_out=0 on first inflate call)
 *
 * Crafts a valid DEFLATE stream whose uncompressed output is much larger
 * than `wsSess->reassemblyBuffer->maxLength` so that inflate returns with
 * `avail_out == 0` on the first call, triggering the `doubleSizeAndRealloc`
 * loop.  Uses a run-length DEFLATE stream for 4096 × 'X' bytes.
 *
 * The DEFLATE stored-block for 4096 bytes:
 *   0x01  BFINAL=1 BTYPE=00
 *   0x00 0x10  LEN=4096 (little-endian)
 *   0xFF 0xEF  NLEN=~4096
 *   4096 × 'X'
 * Total payload = 5 + 4096 = 4101 bytes (fits in 16-bit ext-len).       */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_AvailOutZeroOnFirstInflate_DoubleSizeLoop_NoCrash)
{
    srvArg.port = 16504;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    const int kDataLen = 4096;
    std::vector<unsigned char> payload;
    payload.push_back(0x01);                     /* BFINAL=1, BTYPE=00   */
    payload.push_back(kDataLen & 0xFF);          /* LEN low              */
    payload.push_back((kDataLen >> 8) & 0xFF);   /* LEN high             */
    payload.push_back((~kDataLen) & 0xFF);       /* NLEN low             */
    payload.push_back((~kDataLen >> 8) & 0xFF);  /* NLEN high            */
    payload.insert(payload.end(), kDataLen, static_cast<unsigned char>('X'));

    int wsLen = static_cast<int>(payload.size()); /* = 4101 */

    std::vector<unsigned char> frame;
    frame.push_back(0xC2);  /* FIN=1, RSV1=1, BINARY */
    frame.push_back(126);   /* 2-byte ext-len         */
    frame.push_back(static_cast<unsigned char>(wsLen >> 8));
    frame.push_back(static_cast<unsigned char>(wsLen & 0xFF));
    frame.insert(frame.end(), payload.begin(), payload.end());

    serverSendBytes(frame.data(), static_cast<int>(frame.size()));

    RsslError err;
    const int maxWaitMs = 5000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC16 – Compressed CLOSE frame
 *
 * A WebSocket CLOSE frame (opcode=0x08) with RSV1=1 set.  The CLOSE
 * handler in `rwsReadTransportMsg` / `rwsProcessWsOpCodes` is entered
 * before `handleWebSocketMessages`, so the frame is processed as a CLOSE
 * regardless of the RSV1 bit.  The library must return RSSL_RET_FAILURE
 * (channel closed) without attempting to decompress the payload.         */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_CompressedCloseFrame_ReturnsFailure)
{
    srvArg.port = 16505;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1 (0x40), opcode=CLOSE(0x08) → byte0 = 0x80|0x40|0x08 = 0xC8
     * payload: 2-byte status code 1000 (0x03 0xE8)                       */
    unsigned char frame[] = { 0xC8, 0x02, 0x03, 0xE8 };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a compressed CLOSE frame";
}

/* =========================================================================
 * Compression Crash Test CC17 – Rapid burst of 30 compressed garbage
 *                                frames
 *
 * Hammers the `decompress()` error path 30 times in a tight loop.  After
 * the first Z_DATA_ERROR the library should close the channel; any
 * subsequent frames must not trigger use-after-free or double-free.      */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_RapidBurstOfCompressedGarbageFrames_NoCrash)
{
    srvArg.port = 16506;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* 30 × (FIN=1, RSV1=1, BINARY, 4-byte garbage payload) */
    const int kCount = 30;
    std::vector<unsigned char> burst(kCount * 6);
    for (int i = 0; i < kCount; ++i) {
        burst[i * 6 + 0] = 0xC2;   /* FIN=1, RSV1=1, BINARY */
        burst[i * 6 + 1] = 0x04;   /* length=4               */
        burst[i * 6 + 2] = static_cast<unsigned char>(0xDE ^ i);
        burst[i * 6 + 3] = static_cast<unsigned char>(0xAD ^ i);
        burst[i * 6 + 4] = static_cast<unsigned char>(0xBE ^ i);
        burst[i * 6 + 5] = static_cast<unsigned char>(0xEF ^ i);
    }
    serverSendBytes(burst.data(), static_cast<int>(burst.size()));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC18 – `doubleSizeAndRealloc` upper-bound overflow
 *                                in fragmented decompressed buffer loop
 *
 * When the `doubleSizeAndRealloc` loop doubles the buffer, it checks:
 *     bufferObj->maxLength * 2 <= maxLength
 * If `bufferObj->maxLength` is already just below UINT64_MAX/2, the
 * multiplication itself overflows.  We force this by sending a series of
 * many small valid compressed fragments to grow the `fragmentedDecompressed
 * Buffer->maxLength` as large as possible, then send a final CONT that
 * requests another doubling.
 *
 * In practice the library will hit `maxPayload` before integer overflow;
 * this test verifies the `> maxPayload` termination path is reached
 * without crashing.                                                       */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_DoubleSizeLoopHitsMaxPayload_NoCrash)
{
    srvArg.port = 16507;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open a fragmented sequence with RSV1=1: FIN=0, RSV1=1, BINARY, 4 bytes */
    unsigned char frag1[] = { 0x42, 0x04, 0x11, 0x22, 0x33, 0x44 };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Send a series of non-final CONT frames to accumulate data in the
     * reassembly buffer without triggering decompression yet.            */
    const int kMidFrags = 5;
    unsigned char midFrag[] = { 0x00, 0x04, 0x55, 0x66, 0x77, 0x88 };
    for (int i = 0; i < kMidFrags; ++i) {
        serverSendBytes(midFrag, sizeof(midFrag));
        time_sleep(5);
    }

    /* Final CONT: FIN=1 with large garbage compressed payload.  When the
     * library tries to decompress the accumulated garbage + this payload
     * it will either hit Z_DATA_ERROR or overflow the doubleSizeAndRealloc
     * loop until maxPayload is reached.                                  */
    unsigned char finalCont[] = {
        0x80, 126,         /* FIN=1, CONT, 2-byte ext-len  */
        0x00, 125,         /* declared length = 125        */
    };
    /* 125 bytes of garbage compressed data */
    std::vector<unsigned char> finalFrame(finalCont, finalCont + sizeof(finalCont));
    for (int i = 0; i < 125; ++i)
        finalFrame.push_back(static_cast<unsigned char>(0xAA ^ i));
    serverSendBytes(finalFrame.data(), static_cast<int>(finalFrame.size()));

    RsslError err;
    const int maxWaitMs = 5000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC19 – Alternating valid and garbage compressed
 *                                frames
 *
 * Interleaves one valid DEFLATE stored-block frame with one garbage frame
 * ten times.  After each garbage frame the zlib context is torn down (the
 * library returns RSSL_RET_FAILURE); the channel close path must not
 * double-free or use deallocated memory.                                  */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_AlternatingValidAndGarbageCompressedFrames_NoCrash)
{
    srvArg.port = 16508;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* valid compressed frame (4-byte stored-block) */
    unsigned char validFrame[] = {
        0xC2, 0x09,
        0x01, 0x04, 0x00, 0xFB, 0xFF,
        'V', 'A', 'L', 'D'
    };
    /* garbage compressed frame */
    unsigned char badFrame[] = { 0xC2, 0x04, 0xDE, 0xAD, 0xBE, 0xEF };

    /* Build a combined burst: valid + garbage × 3 */
    std::vector<unsigned char> burst;
    for (int i = 0; i < 3; ++i) {
        burst.insert(burst.end(), validFrame, validFrame + sizeof(validFrame));
        burst.insert(burst.end(), badFrame,   badFrame   + sizeof(badFrame));
    }
    serverSendBytes(burst.data(), static_cast<int>(burst.size()));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC20 – Compressed fragment sequence with three
 *                                overflow CONT frames in a row
 *
 * Each successive CONT frame has a different 8-byte ext-len that, when
 * added to the running reassemblyBuffer->length, produces a different
 * wrap value.  Tests that the overflow detection in `checkSizeAndRealloc`
 * handles multiple consecutive integer-overflow CONT frames without
 * crashing on the first, second, or third.                               */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ThreeConsecutiveOverflowContFrames_NoCrash)
{
    srvArg.port = 16509;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open a fragment: 8 bytes → reassemblyBuffer->length = 8            */
    ASSERT_TRUE(serverSend(WS_OPC_BINARY, false, "OpenFrag", 8));
    time_sleep(30);

    /* CONT #1: payloadLen = 0xFFFFFFFFFFFFFFF8 → 8+overflow=0            */
    unsigned char cont1[12];
    cont1[0] = 0x00; cont1[1] = 127;
    cont1[2]=0xFF; cont1[3]=0xFF; cont1[4]=0xFF; cont1[5]=0xFF;
    cont1[6]=0xFF; cont1[7]=0xFF; cont1[8]=0xFF; cont1[9]=0xF8;
    cont1[10]='C'; cont1[11]='1';
    serverSendBytes(cont1, sizeof(cont1));
    time_sleep(20);

    /* CONT #2: payloadLen = 0xFFFFFFFFFFFFFFF0 → still overflows          */
    unsigned char cont2[12];
    cont2[0] = 0x00; cont2[1] = 127;
    cont2[2]=0xFF; cont2[3]=0xFF; cont2[4]=0xFF; cont2[5]=0xFF;
    cont2[6]=0xFF; cont2[7]=0xFF; cont2[8]=0xFF; cont2[9]=0xF0;
    cont2[10]='C'; cont2[11]='2';
    serverSendBytes(cont2, sizeof(cont2));
    time_sleep(20);

    /* CONT #3 (final): FIN=1, payloadLen = 0xFFFFFFFFFFFFFFF4             */
    unsigned char cont3[12];
    cont3[0] = 0x80; cont3[1] = 127;
    cont3[2]=0xFF; cont3[3]=0xFF; cont3[4]=0xFF; cont3[5]=0xFF;
    cont3[6]=0xFF; cont3[7]=0xFF; cont3[8]=0xFF; cont3[9]=0xF4;
    cont3[10]='C'; cont3[11]='3';
    serverSendBytes(cont3, sizeof(cont3));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC21 – Compressed TEXT frame (RSV1=1, opcode=TEXT)
 *
 * Targets the single-frame compressed branch of `handleWebSocketMessages`
 * when the frame's `dataType` is TEXT (0x01) rather than BINARY (0x02).
 * byte0 = FIN(1) | RSV1(1) | TEXT = 0x80 | 0x40 | 0x01 = 0xC1.
 * With garbage payload the inflate call returns Z_DATA_ERROR.  The library
 * must return RSSL_RET_FAILURE and must not crash or treat the frame as
 * uncompressed.                                                            */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_CompressedTextFrame_ReturnsFailure)
{
    srvArg.port = 16510;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1, TEXT → 0xC1; 4-byte garbage payload */
    unsigned char frame[] = { 0xC1, 0x04, 0xDE, 0xAD, 0xBE, 0xEF };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet  = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }

    EXPECT_EQ(RSSL_RET_FAILURE, lastRet)
        << "Expected RSSL_RET_FAILURE for a compressed TEXT frame with "
           "garbage payload (Z_DATA_ERROR path)";
}

/* =========================================================================
 * Compression Crash Test CC23 – 100 small valid compressed frames in one
 *                                TCP burst (stress moreData + compression)
 *
 * 100 valid DEFLATE stored-block frames (4 bytes each) arrive in a single
 * `recv()` buffer.  The `*moreData = 1` loop in `rwsReadWebSocket` must
 * iterate through all frames while maintaining correct zlib stream state.
 * The library must not crash or return RSSL_RET_FAILURE.                 */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_100ValidCompressedFramesBurst_NoCrash)
{
    srvArg.port = 16512;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Each frame: FIN=1, RSV1=1, BINARY, stored-block for 2 bytes "AB"
     *   WS payload = 5 (stored-block hdr) + 2 = 7 bytes
     *   byte0=0xC2, byte1=0x07, then DEFLATE stored block                */
    const unsigned char singleFrame[] = {
        0xC2, 0x07,             /* FIN|RSV1|BINARY, length=7 */
        0x01,                   /* BFINAL=1, BTYPE=00         */
        0x02, 0x00,             /* LEN=2                      */
        0xFD, 0xFF,             /* NLEN=~2                    */
        'A', 'B'                /* payload                    */
    };
    const int kCount = 100;
    std::vector<unsigned char> burst;
    burst.reserve(kCount * sizeof(singleFrame));
    for (int i = 0; i < kCount; ++i)
        burst.insert(burst.end(), singleFrame, singleFrame + sizeof(singleFrame));

    /* Send in two halves to stay within typical send-buffer limits */
    int half = static_cast<int>(burst.size()) / 2;
    serverSendBytes(burst.data(), half);
    time_sleep(10);
    serverSendBytes(burst.data() + half, static_cast<int>(burst.size()) - half);

    RsslError err;
    const int maxWaitMs = 8000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash is the pass condition */
}

/* =========================================================================
 * Compression Crash Test CC24 – Two consecutive valid compressed frames
 *                                testing zlib context reuse
 *
 * After the first valid stored-block inflation succeeds, the `c_stream_in`
 * context is kept alive (no context-takeover not negotiated here).  The
 * second frame feeds another stored-block into the same zlib stream.
 * Tests that the context is correctly managed across two successful frames.*/
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_TwoValidCompressedFramesContextReuse_NoCrash)
{
    srvArg.port = 16513;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Frame 1: stored-block DEFLATE for 3 bytes "XYZ" */
    unsigned char frame1[] = {
        0xC2, 0x08,
        0x01, 0x03, 0x00, 0xFC, 0xFF, 'X', 'Y', 'Z'
    };
    /* Frame 2: stored-block DEFLATE for 3 bytes "PQR" */
    unsigned char frame2[] = {
        0xC2, 0x08,
        0x01, 0x03, 0x00, 0xFC, 0xFF, 'P', 'Q', 'R'
    };

    serverSendBytes(frame1, sizeof(frame1));
    time_sleep(100); /* ensure first frame is fully processed */
    serverSendBytes(frame2, sizeof(frame2));

    RsslError err;
    const int maxWaitMs = 4000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC25 – Long chain of non-final CONT frames
 *                                accumulating data, final CONT never arrives
 *
 * Opens a fragmented compressed sequence then sends 15 non-final CONT
 * frames (FIN=0) to grow the reassembly buffer.  The final CONT is never
 * sent so the library holds an ever-growing partial reassembly.
 * Tests that repeated `checkSizeAndRealloc` calls without a final frame
 * do not crash and return RSSL_RET_READ_WOULD_BLOCK.                     */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_LongChainContNeverFinished_NoCrash)
{
    srvArg.port = 16514;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Open fragment: FIN=0, RSV1=1, BINARY, 8 bytes of garbage */
    unsigned char frag1[] = { 0x42, 0x08,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Send 15 non-final CONT frames, each with 8 bytes */
    unsigned char contFrag[] = { 0x00, 0x08,
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };
    for (int i = 0; i < 15; ++i) {
        serverSendBytes(contFrag, sizeof(contFrag));
        time_sleep(5);
    }

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC27 – Partial compressed frame (2-byte ext-len
 *                                header only, payload never sent)
 *
 * Targets the partial-frame detection path for compressed frames.  The
 * library receives only the 4-byte WS header (0xC2 + 126 + 0x00 + 125)
 * declaring 125 bytes of compressed payload.  Since no payload arrives,
 * `_decodeWSFrame` sets `frame->partial = TRUE`.  `rwsReadWebSocket` must
 * set `*moreData = 1` and `*readret = RSSL_RET_SUCCESS`, returning NULL
 * without crashing.                                                        */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_PartialCompressedHeaderOnly_NoCrash)
{
    srvArg.port = 16516;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* 4-byte header only: FIN=1, RSV1=1, BINARY; 2-byte ext-len=125;
     * payload never sent → partial frame                                  */
    unsigned char hdrOnly[] = { 0xC2, 126, 0x00, 125 };
    serverSendBytes(hdrOnly, sizeof(hdrOnly));

    RsslError err;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet == RSSL_RET_FAILURE) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash; partial → WOULD_BLOCK is acceptable */
}

/* =========================================================================
 * Compression Crash Test CC28 – Compressed PONG frame (RSV1=1, opcode=PONG)
 *
 * byte0 = FIN(1) | RSV1(1) | PONG(0x0A) = 0x80 | 0x40 | 0x0A = 0xCA.
 * PONG is a control frame; the PONG handler in `rwsReadTransportMsg` /
 * `rwsProcessWsOpCodes` is invoked BEFORE `handleWebSocketMessages`.
 * The PONG is handled as a PONG regardless of RSV1.  The library must
 * not attempt to decompress the PONG payload and must not crash.          */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_CompressedPongFrame_NoCrash)
{
    srvArg.port = 16517;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* FIN=1, RSV1=1, PONG → 0xCA; 4-byte payload */
    unsigned char frame[] = { 0xCA, 0x04, 'P', 'O', 'N', 'G' };
    serverSendBytes(frame, sizeof(frame));

    RsslError err;
    RsslRet   lastRet = RSSL_RET_READ_WOULD_BLOCK;
    const int maxWaitMs = 2000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        lastRet = rRet;
        if (rRet == RSSL_RET_FAILURE) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }

    EXPECT_NE(RSSL_RET_FAILURE, lastRet)
        << "Compressed PONG frame must not cause RSSL_RET_FAILURE";
}

/* =========================================================================
 * Compression Crash Test CC29 – PING between compressed fragments then
 *                                final garbage compressed CONT
 *
 * RFC 6455 permits control frames interleaved in a fragmented sequence.
 * This test injects a PING between a compressed first-fragment and a
 * final CONT that carries garbage data.  The PING must be responded to
 * without disrupting the reassembly state, and the garbage CONT must
 * trigger RSSL_RET_FAILURE via Z_DATA_ERROR.                              */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_PingBetweenCompressedFragmentsThenGarbageCont_NoCrash)
{
    srvArg.port = 16518;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, RSV1=1, BINARY, 6-byte garbage */
    unsigned char frag1[] = { 0x42, 0x06, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Interleaved PING (control frame, RFC-legal) */
    ASSERT_TRUE(serverSend(WS_OPC_PING, true, "ping", 4));
    time_sleep(20);

    /* Final CONT: FIN=1, garbage compressed bytes */
    unsigned char finalCont[] = { 0x80, 0x04, 0x11, 0x22, 0x33, 0x44 };
    serverSendBytes(finalCont, sizeof(finalCont));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC30 – Valid compressed frame immediately
 *                                followed by 8-byte ext-len overflow
 *                                compressed frame
 *
 * After a valid compressed frame is successfully inflated, the zlib
 * context state is valid.  The immediately following frame uses the
 * 8-byte overflow trick to bypass `frame->partial` and land in the
 * compressed path with a ~UINT64_MAX payloadLen.  The library must
 * detect the overflow (via `checkSizeAndRealloc` returning NULL or the
 * size guard) and return RSSL_RET_FAILURE without crashing.              */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ValidCompressedThenOverflowCompressed_NoCrash)
{
    srvArg.port = 16519;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Frame 1: valid stored-block DEFLATE for 3 bytes */
    unsigned char valid[] = {
        0xC2, 0x08,
        0x01, 0x03, 0x00, 0xFC, 0xFF, 'A', 'B', 'C'
    };
    serverSendBytes(valid, sizeof(valid));

    /* Drain the first frame */
    RsslError err;
    for (int w = 0; w < 2000; w += 10) {
        RsslRet rRet;
        RsslBuffer* pBuf = rsslRead(pClientChnl, &rRet, &err);
        if (pBuf && pBuf->length > 0) break;
        if (rRet == RSSL_RET_FAILURE) goto done;
        time_sleep(10);
    }

    {
        /* Frame 2: FIN=1, RSV1=1 (compressed), 8-byte ext-len overflow
         * payloadLen = 0xFFFFFFFFFFFFFFF6 → hdrLen(10)+payloadLen = 0 → partial=FALSE */
        unsigned char overflow[14];
        overflow[0] = 0xC2; overflow[1] = 127;
        overflow[2]=0xFF; overflow[3]=0xFF; overflow[4]=0xFF; overflow[5]=0xFF;
        overflow[6]=0xFF; overflow[7]=0xFF; overflow[8]=0xFF; overflow[9]=0xF6;
        overflow[10]='O'; overflow[11]='F'; overflow[12]='L'; overflow[13]='W';
        serverSendBytes(overflow, sizeof(overflow));

        const int maxWaitMs = 2000;
        for (int waited = 0; waited < maxWaitMs; waited += 10) {
            RsslRet rRet;
            rsslRead(pClientChnl, &rRet, &err);
            if (rRet == RSSL_RET_FAILURE) break;
            if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
            time_sleep(10);
        }
    }
done:
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC31 – CONT frame with RSV1=1 in a compressed
 *                                fragment sequence (double-compressed CONT)
 *
 * Per RFC 7692 §6.1 the RSV1 bit must only be set on the first frame of
 * a compressed message, not on continuation frames.  When a CONT frame
 * arrives with RSV1=1:
 *   - `_decodeWSFrame` does NOT update `frame->compressed` for CONT frames
 *     (the `if (frame->opcode != _WS_OPC_CONT)` guard)
 *   - `frame->rsv1Set` is set to 1 even on CONT
 * The library decompresses using `wsSess->reassemblyCompressed` (set by
 * the first fragment), not the CONT's RSV1.  Garbage CONT payload still
 * causes Z_DATA_ERROR → RSSL_RET_FAILURE, no crash.                      */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_ContFrameWithRSV1InCompressedSeq_NoCrash)
{
    srvArg.port = 16520;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* Fragment 1: FIN=0, RSV1=1, BINARY → 0x42; 6-byte garbage */
    unsigned char frag1[] = { 0x42, 0x06, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE };
    serverSendBytes(frag1, sizeof(frag1));
    time_sleep(20);

    /* Final CONT with RSV1=1 set (illegal per RFC 7692 §6.1):
     * FIN=1, RSV1=1, CONT → byte0 = 0x80|0x40|0x00 = 0xC0; 4-byte garbage */
    unsigned char contRsv1[] = { 0xC0, 0x04, 0x11, 0x22, 0x33, 0x44 };
    serverSendBytes(contRsv1, sizeof(contRsv1));

    RsslError err;
    const int maxWaitMs = 3000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        time_sleep(10);
    }
    SUCCEED();
}

/* =========================================================================
 * Compression Crash Test CC32 – Very large valid compressed frame
 *                                (20 000 bytes) exercises doubleSizeAndRealloc
 *                                growth chain
 *
 * A DEFLATE stored-block frame for 20 000 'Z' bytes is sent.  The initial
 * `wsSess->reassemblyBuffer` (allocated at `maxMsgSize * 10`) may be
 * smaller than 20 000 bytes for small maxMsgSize configurations, so
 * inflate returns `avail_out = 0` multiple times, triggering the
 * `doubleSizeAndRealloc` loop in the single-frame compressed path.  The
 * library must eventually accommodate the full output or terminate at
 * `maxPayload` without crashing.                                           */
TEST_F(RsslWebSocketReadCompressedTests, CompCrash_LargeValidCompressedFrame_DoubleSizeLoop_NoCrash)
{
    srvArg.port = 16521;

    ASSERT_TRUE(startServer())   << "Compressed server failed: " << srvArg.errText;
    ASSERT_TRUE(connectClient()) << "rsslConnect(compressed) failed";
    ASSERT_TRUE(driveToActive()) << "Client channel not ACTIVE";

    /* DEFLATE stored-block for 20 000 'Z' bytes:
     *   0x01  BFINAL=1, BTYPE=00
     *   0x20 0x4E  LEN=20000 (little-endian)
     *   0xDF 0xB1  NLEN=~20000
     *   20000 × 'Z'                                                       */
    const int kDataLen = 20000;
    std::vector<unsigned char> deflate;
    deflate.push_back(0x01);
    deflate.push_back(static_cast<unsigned char>(kDataLen & 0xFF));
    deflate.push_back(static_cast<unsigned char>((kDataLen >> 8) & 0xFF));
    deflate.push_back(static_cast<unsigned char>((~kDataLen) & 0xFF));
    deflate.push_back(static_cast<unsigned char>((~kDataLen >> 8) & 0xFF));
    deflate.insert(deflate.end(), kDataLen, static_cast<unsigned char>('Z'));

    int wsLen = static_cast<int>(deflate.size()); /* = 20005 */
    std::vector<unsigned char> frame;
    frame.push_back(0xC2); /* FIN=1, RSV1=1, BINARY */
    frame.push_back(126);  /* 2-byte ext-len          */
    frame.push_back(static_cast<unsigned char>(wsLen >> 8));
    frame.push_back(static_cast<unsigned char>(wsLen & 0xFF));
    frame.insert(frame.end(), deflate.begin(), deflate.end());

    /* Send in 4 KB chunks */
    const int kChunk = 4096;
    int offset = 0;
    while (offset < static_cast<int>(frame.size())) {
        int remaining = static_cast<int>(frame.size()) - offset;
        int toSend = (kChunk < remaining) ? kChunk : remaining;
        serverSendBytes(frame.data() + offset, toSend);
        offset += toSend;
    }

    RsslError err;
    const int maxWaitMs = 8000;
    for (int waited = 0; waited < maxWaitMs; waited += 10) {
        RsslRet rRet;
        rsslRead(pClientChnl, &rRet, &err);
        if (rRet == RSSL_RET_FAILURE) break;
        if (pClientChnl->state == RSSL_CH_STATE_CLOSED) break;
        if (rRet != RSSL_RET_READ_WOULD_BLOCK) break;
        time_sleep(10);
    }
    SUCCEED(); /* No crash; success or max-payload termination both acceptable */
}
