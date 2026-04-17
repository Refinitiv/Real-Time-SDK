/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */
/************************************************************************
 *  rsslWaitAck Unit Tests
 *
 *  Unit testing for the ipcWaitAck() method in rsslSocketTransportImpl.c.
 *  ipcWaitAck runs on the *client* side: it reads the server's CONNACK
 *  (or CONNNAK) response and drives the client channel to ACTIVE or ERROR.
 *
 *  Test strategy
 *  -------------
 *  Each test spins up a "fake server" thread that:
 *    1. Binds a plain TCP listen socket.
 *    2. Signals "ready" via an atomic flag.
 *    3. Accepts the first connection from the RSSL client.
 *    4. Drains the RSSL CONNECT request (so the client is not stalled).
 *    5. Sends a crafted CONNACK/CONNNAK/garbage payload.
 *    6. Optionally closes the connection.
 *
 *  The main thread then calls rsslConnect() (non-blocking) to reach the
 *  fake server and polls rsslInitChannel() until the client channel either
 *  reaches RSSL_CH_STATE_ACTIVE or returns RSSL_RET_FAILURE /
 *  RSSL_RET_CHAN_INIT_REFUSED.
 *
 *  CONNACK wire layout (server → client, minimal 10-byte frame):
 *    [0..1]  total length  (u16 big-endian) = IPC_100_CONN_ACK (10)
 *    [2]     standard flags byte            = IPC_EXTENDED_FLAGS | IPC_DATA (0x03)
 *    [3]     extended opCode byte           = IPC_CONNACK (0x01) or
 *                                             IPC_CONNNAK (0x02)
 *    [4..5]  header length field            = IPC_100_CONN_ACK (10)
 *    [6..9]  RIPC version (u32 big-endian)  = RIPC_VERSION_14 (0x17)
 *    [10..11] maxMsgSize (u16 big-endian)
 *    [12]    rsslFlags
 *    [13]    pingTimeout
 *    [14]    majorVersion
 *    [15]    minorVersion
 *    [16..17] compression type (u16 big-endian)
 *    [18]    zlibCompLevel (versions > 10 only)
 *
 *  ipcWaitAck validates:
 *    - cc >= IPC_100_CONN_ACK (10) after reading
 *    - IPC_EXTENDED_FLAGS set in flags byte [2]
 *    - opCode is IPC_CONNACK (0x01) or IPC_CONNNAK (0x02)
 *    - versionNumber is one of the known RIPC_VERSION_xx values
 *    - comp (compression type) <= RSSL_COMP_MAX_TYPE
 *
 *  Tests cover all rejection paths and the successful CONNACK path.
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>
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
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#endif

/* -------------------------------------------------------------------------
 * Platform socket aliases
 * ------------------------------------------------------------------------- */
#if defined(_WIN32)
typedef SOCKET WaitAckRawSocket;
static const WaitAckRawSocket kWaitAckInvalidSocket = INVALID_SOCKET;
static void waitAckCloseSocket(WaitAckRawSocket s) { closesocket(s); }
static int  waitAckSend(WaitAckRawSocket s, const char* buf, int len)
{
    return send(s, buf, len, 0);
}
static int waitAckRecv(WaitAckRawSocket s, char* buf, int len)
{
    return recv(s, buf, len, 0);
}
#else
typedef int WaitAckRawSocket;
static const WaitAckRawSocket kWaitAckInvalidSocket = -1;
static void waitAckCloseSocket(WaitAckRawSocket s) { close(s); }
static int  waitAckSend(WaitAckRawSocket s, const char* buf, int len)
{
    return static_cast<int>(send(s, buf, len, 0));
}
static int waitAckRecv(WaitAckRawSocket s, char* buf, int len)
{
    return static_cast<int>(recv(s, buf, len, 0));
}
#endif

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

static void waitAckSleepMs(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    if (ms > 0)
    {
        struct timespec ts;
        ts.tv_sec  = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }
#endif
}

/* Send all bytes in buf to s; returns false on error. */
static bool waitAckSendAll(WaitAckRawSocket s, const unsigned char* buf, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int n = waitAckSend(s, reinterpret_cast<const char*>(buf + sent), len - sent);
        if (n <= 0)
            return false;
        sent += n;
    }
    return true;
}

/* Drain up to maxBytes from s (discards data). */
static void waitAckDrain(WaitAckRawSocket s, int maxBytes = 512)
{
    char tmp[256];
    int  total = 0;
    while (total < maxBytes)
    {
        int n = waitAckRecv(s, tmp, sizeof(tmp));
        if (n <= 0)
            break;
        total += n;
        if (n < static_cast<int>(sizeof(tmp)))
            break;
    }
}

/* Bind a plain TCP listen socket on port. Returns kWaitAckInvalidSocket on
 * failure. */
static WaitAckRawSocket waitAckBindListen(unsigned short port)
{
    WaitAckRawSocket ls = static_cast<WaitAckRawSocket>(
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (ls == kWaitAckInvalidSocket)
        return kWaitAckInvalidSocket;

    int reuse = 1;
#if defined(_WIN32)
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
#else
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(ls, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0 ||
        listen(ls, 5) != 0)
    {
        waitAckCloseSocket(ls);
        return kWaitAckInvalidSocket;
    }
    return ls;
}

/* Accept one connection on ls (blocking). */
static WaitAckRawSocket waitAckAcceptOne(WaitAckRawSocket ls)
{
    return static_cast<WaitAckRawSocket>(
        accept(ls, nullptr, nullptr));
}

/* -------------------------------------------------------------------------
 * Build a minimal valid RIPC CONNACK frame.
 *
 * Layout (IPC_100_CONN_ACK = 10 bytes):
 *   [0..1]  length = 10 (big-endian u16)
 *   [2]     flags  = IPC_EXTENDED_FLAGS | IPC_DATA = 0x03
 *   [3]     opCode = IPC_CONNACK = 0x01
 *   [4]     headerLength = IPC_100_CONN_ACK (10)
 *   [5]     pad = 0
 *   [6..9]  RIPC version (RIPC_VERSION_14 = 0x00000017, big-endian u32)
 *   [10..11] maxMsgSize (u16 big-endian) = 0x1700
 *   [12]    rsslFlags = 0
 *   [13]    pingTimeout = 60
 *   [14]    majorVersion = 14
 *   [15]    minorVersion = 1
 *   [16..17] compression = RSSL_COMP_NONE = 0
 *   [18]    zlibCompLevel = 0
 *
 * Returns the number of bytes written (IPC_100_CONN_ACK + 9 = 19 bytes,
 * matching ipcWaitAck's parse path for version 14).
 * ------------------------------------------------------------------------- */
static int buildValidConnAck(unsigned char* buf, int bufLen,
                             RsslUInt32 ripcVersion = RIPC_VERSION_14,
                             RsslUInt16 compType    = RSSL_COMP_NONE)
{
    /* ipcWaitAck reads up to buf[18] for version > 10, so we need 23 bytes */
    const int TOTAL = 23;
    if (bufLen < TOTAL)
        return 0;

    memset(buf, 0, TOTAL);

    /* [0..1] length = 23 declared (msgLen parsed by ipcWaitAck) */
    buf[0] = 0x00;
    buf[1] = static_cast<unsigned char>(TOTAL);

    /* [2] standard flags = IPC_EXTENDED_FLAGS (0x01) | IPC_DATA (0x02) */
    buf[2] = 0x03;

    /* [3] extended opCode = IPC_CONNACK */
    buf[3] = static_cast<unsigned char>(IPC_CONNACK);

    /* [4] headerLength = IPC_100_CONN_ACK (10) */
    buf[4] = static_cast<unsigned char>(IPC_100_CONN_ACK);

    /* [5] pad */
    buf[5] = 0x00;

    /* [6..9] RIPC version (big-endian u32) */
    buf[6] = static_cast<unsigned char>((ripcVersion >> 24) & 0xFF);
    buf[7] = static_cast<unsigned char>((ripcVersion >> 16) & 0xFF);
    buf[8] = static_cast<unsigned char>((ripcVersion >>  8) & 0xFF);
    buf[9] = static_cast<unsigned char>( ripcVersion        & 0xFF);

    /* [10..11] maxMsgSize = 0x1700 (big-endian u16) */
    buf[10] = 0x17;
    buf[11] = 0x00;

    /* [12] rsslFlags */
    buf[12] = 0x00;

    /* [13] pingTimeout = 60 */
    buf[13] = 60;

    /* [14] majorVersion */
    buf[14] = 14;

    /* [15] minorVersion */
    buf[15] = 1;

    /* [16..17] compression type (big-endian u16) */
    buf[16] = static_cast<unsigned char>((compType >> 8) & 0xFF);
    buf[17] = static_cast<unsigned char>( compType       & 0xFF);

    /* [18] zlibCompLevel (only read for versions > RIPC_VERSION_10) */
    buf[18] = 0x00;

    /* Key exchange */
    buf[19] = 0x8;

    /* Enc type */
    buf[20] = 0;

    /* Enc length */
    buf[21] = 0;

    /* Component version length */
    buf[22] = 0;

    return TOTAL;
}

/* -------------------------------------------------------------------------
 * Fake-server thread infrastructure
 * ------------------------------------------------------------------------- */

struct FakeServerArg
{
    unsigned short      port;
    unsigned char       response[128];
    int                 responseLen;
    bool                closeAfterSend;   /* close conn after sending       */
    bool                keepOpen;         /* keep listen socket open        */
    std::atomic<bool>   ready;            /* set once listener is bound     */
    std::atomic<bool>   done;             /* set when thread exits          */
    WaitAckRawSocket    listenSock;       /* valid after ready == true      */

    FakeServerArg()
        : port(0), responseLen(0),
          closeAfterSend(true), keepOpen(false),
          ready(false), done(false),
          listenSock(kWaitAckInvalidSocket)
    {
        memset(response, 0, sizeof(response));
    }
};

static RSSL_THREAD_DECLARE(fakeConnAckServerThread, pArg)
{
    FakeServerArg* arg = reinterpret_cast<FakeServerArg*>(pArg);

    arg->listenSock = waitAckBindListen(arg->port);
    if (arg->listenSock == kWaitAckInvalidSocket)
    {
        arg->done  = true;
        arg->ready = true;  /* unblock waiter even on failure               */
        return 0;
    }

    arg->ready = true;

    WaitAckRawSocket conn = waitAckAcceptOne(arg->listenSock);
    if (conn != kWaitAckInvalidSocket)
    {
        /* Drain the RSSL CONNECT request so the client is not blocked      */
        waitAckDrain(conn, 512);

        /* Send the crafted response                                         */
        if (arg->responseLen > 0)
            waitAckSendAll(conn, arg->response, arg->responseLen);

        if (arg->closeAfterSend)
        {
            waitAckCloseSocket(conn);
        }
        else
        {
            /* Hold the connection open briefly so the client can parse it  */
            waitAckSleepMs(300);
            waitAckCloseSocket(conn);
        }
    }

    if (!arg->keepOpen)
    {
        waitAckCloseSocket(arg->listenSock);
        arg->listenSock = kWaitAckInvalidSocket;
    }

    arg->done = true;
    return 0;
}

/* -------------------------------------------------------------------------
 * Drive rsslInitChannel on pClientChnl until the channel reaches a terminal
 * state (ACTIVE, FAILURE, or CHAN_INIT_REFUSED).
 *
 * Returns true  when the channel reached ACTIVE.
 * Returns false when it was rejected (FAILURE / REFUSED / CLOSED).
 * ------------------------------------------------------------------------- */
static bool driveClientToTerminal(RsslChannel* pClientChnl, RsslError *pError,
                                  int maxTries = 400,
                                  int sleepMs  = 5)
{
    RsslInProgInfo inProg;

    for (int i = 0; i < maxTries; ++i)
    {
        if (pClientChnl->state == RSSL_CH_STATE_ACTIVE)
            return true;

        if (pClientChnl->state == RSSL_CH_STATE_CLOSED ||
            pClientChnl->state == RSSL_CH_STATE_INACTIVE)
            return false;

        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pClientChnl, &inProg, pError);
        if (ret == RSSL_RET_FAILURE || ret == RSSL_RET_CHAN_INIT_REFUSED)
            return false;

        waitAckSleepMs(sleepMs);
    }

    return (pClientChnl->state == RSSL_CH_STATE_ACTIVE);
}

/* =========================================================================
 * Test Fixture: RsslWaitAckInvalidResponseTests
 *
 * Pattern for each test:
 *   1. Configure FakeServerArg with the crafted response bytes.
 *   2. Call startFakeServer() to start the listener thread.
 *   3. Call connectClient() to open the non-blocking RSSL client channel.
 *   4. Call driveClientToTerminal() and assert on the outcome.
 * ========================================================================= */

class RsslWaitAckInvalidResponseTests : public ::testing::Test
{
protected:
    RsslChannel*    pClientChnl = nullptr;
    RsslThreadId    fakeSrvTid;
    FakeServerArg   fakeArg;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
        memset(&fakeSrvTid, 0, sizeof(fakeSrvTid));
    }

    void TearDown() override
    {
        /* Close the listen socket if it is still open (forces thread exit) */
        if (fakeArg.listenSock != kWaitAckInvalidSocket)
        {
            waitAckCloseSocket(fakeArg.listenSock);
            fakeArg.listenSock = kWaitAckInvalidSocket;
        }

        /* Wait for the fake-server thread to finish */
        for (int i = 0; i < 200 && !fakeArg.done.load(); ++i)
            waitAckSleepMs(10);

        RsslError err;
        if (pClientChnl)
        {
            rsslCloseChannel(pClientChnl, &err);
            pClientChnl = nullptr;
        }
        rsslUninitialize();
    }

    /* Start the fake-server thread and wait until the listener is ready.   */
    bool startFakeServer()
    {
        RSSL_THREAD_START(&fakeSrvTid, fakeConnAckServerThread, &fakeArg);
        for (int i = 0; i < 500 && !fakeArg.ready.load(); ++i)
            waitAckSleepMs(2);
        return fakeArg.ready.load() &&
               fakeArg.listenSock != kWaitAckInvalidSocket;
    }

    /* Connect the RSSL client (non-blocking) to the fake server.           */
    bool connectClient()
    {
        char portStr[16];
        snprintf(portStr, sizeof(portStr), "%u",
                 static_cast<unsigned>(fakeArg.port));

        RsslError err;
        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = portStr;
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
        opts.blocking                           = RSSL_FALSE;

        pClientChnl = rsslConnect(&opts, &err);
        return pClientChnl != nullptr;
    }
};

/* =========================================================================
 * Test 1 – Immediate EOF (server sends nothing and closes)
 *
 * ipcWaitAck reads 0 bytes (EOF), cc < IPC_100_CONN_ACK, returns
 * RIPC_CONN_ERROR. The client channel must not reach ACTIVE.               */
TEST_F(RsslWaitAckInvalidResponseTests, ImmediateEof_ClientRejectsConnection)
{
    fakeArg.port          = 15800;
    fakeArg.responseLen   = 0;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15800";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE on immediate EOF";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 ipcConnecting() client connect() failed.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 2 – Truncated CONNACK (only 4 bytes, below IPC_100_CONN_ACK = 10)
 *
 * ipcWaitAck checks (cc < IPC_100_CONN_ACK) and returns RIPC_CONN_ERROR.  */
TEST_F(RsslWaitAckInvalidResponseTests, TruncatedConnAck_ClientRejectsConnection)
{
    fakeArg.port = 15801;
    /* 4-byte payload: length field (0x00 0x04), flags (0x03), opCode (0x01) */
    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = 0x04;
    fakeArg.response[2] = 0x03;  /* IPC_EXTENDED_FLAGS | IPC_DATA         */
    fakeArg.response[3] = 0x01;  /* IPC_CONNACK                           */
    fakeArg.responseLen  = 4;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15801";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with truncated CONNACK";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Ack.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 3 – CONNNAK response (IPC_CONNNAK = 0x02)
 *
 * ipcWaitAck detects IPC_CONNNAK and returns RIPC_CONN_REFUSED, which maps
 * to RSSL_RET_CHAN_INIT_REFUSED on the RSSL channel.                       */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNak_ClientRefused)
{
    fakeArg.port = 15802;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0) << "buildValidConnAck failed";

    /* Replace IPC_CONNACK (0x01) with IPC_CONNNAK (0x02)                   */
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);
    fakeArg.responseLen  = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15802";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE on IPC_CONNNAK";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 4 – CONNNAK with error text appended
 *
 * ipcWaitAck reads the NAK text when msgLen > IPC_100_OTHER_HEADER_SIZE.
 * The channel must still be rejected cleanly.                              */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNakWithErrorText_ClientRefused)
{
    fakeArg.port = 15803;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);

    /* Append a 20-byte NAK text after the base CONNACK frame:
     * ipcWaitAck reads nakTextLen from buf[6..7] (u16 big-endian).
     * Set nakTextLen = 20 at offsets [6..7].                               */
    fakeArg.response[6] = 0x00;
    fakeArg.response[7] = 20;

    /* Fill the error text bytes */
    const char* errMsg = "Connection rejected!!";  /* exactly 20 chars     */
    memcpy(fakeArg.response + 8, errMsg, 20);

    /* Adjust declared length to cover base + nakTextLen bytes             */
    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = static_cast<unsigned char>(8 + 20);  /* 28 bytes */
    fakeArg.responseLen  = 28;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15803";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE on CONNNAK + error text";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server with text: Connection rejected!") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 5 – No IPC_EXTENDED_FLAGS set (flags byte = IPC_DATA = 0x02 only)
 *
 * Without IPC_EXTENDED_FLAGS the opCode byte is not read, so opCode stays
 * 0 (neither IPC_CONNACK nor IPC_CONNNAK).  ipcWaitAck falls through to
 * the "!(opCode & IPC_CONNACK)" check and returns RIPC_CONN_ERROR.         */
TEST_F(RsslWaitAckInvalidResponseTests, NoExtendedFlags_ClientRejectsConnection)
{
    fakeArg.port = 15804;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Clear IPC_EXTENDED_FLAGS – leave only IPC_DATA (0x02)               */
    fakeArg.response[2] = 0x02;
    fakeArg.responseLen  = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15804";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE without IPC_EXTENDED_FLAGS";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Opcode: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 6 – Extended opCode = 0x00 (neither CONNACK nor CONNNAK)
 *
 * IPC_EXTENDED_FLAGS is set but the opCode byte [3] is 0x00.
 * !(opCode & IPC_CONNACK) fires → RIPC_CONN_ERROR.                        */
TEST_F(RsslWaitAckInvalidResponseTests, ZeroOpCode_ClientRejectsConnection)
{
    fakeArg.port = 15805;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    fakeArg.response[3] = 0x00;  /* neither CONNACK nor CONNNAK             */
    fakeArg.responseLen  = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15805";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with zero extended opCode";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Opcode: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 7 – Unknown RIPC version number (0xDEADBEEF)
 *
 * ipcWaitAck reads versionNumber from buf[6..9] and switches on it.  An
 * unrecognised value hits the default case → RIPC_CONN_ERROR.             */
TEST_F(RsslWaitAckInvalidResponseTests, UnknownRipcVersion_ClientRejectsConnection)
{
    fakeArg.port = 15806;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Overwrite the 4-byte version field with an unknown value            */
    fakeArg.response[6] = 0xDE;
    fakeArg.response[7] = 0xAD;
    fakeArg.response[8] = 0xBE;
    fakeArg.response[9] = 0xEF;
    fakeArg.responseLen  = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15806";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with unknown RIPC version";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid IPC Version: (-559038737)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 9 – All-0xFF garbage (32 bytes)
 *
 * Length field [0..1] = 0xFFFF.  The cc < IPC_100_CONN_ACK check passes
 * (cc = 32 >= 10), but flags = 0xFF sets IPC_EXTENDED_FLAGS, opCode = 0xFF
 * sets both IPC_CONNNAK (0x02) and IPC_CONNACK (0x01) bits simultaneously.
 * The IPC_CONNNAK branch is evaluated first → RIPC_CONN_REFUSED.          */
TEST_F(RsslWaitAckInvalidResponseTests, AllFFGarbage_ClientRejectsConnection)
{
    fakeArg.port = 15808;
    memset(fakeArg.response, 0xFF, 32);
    fakeArg.responseLen   = 32;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15808";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with all-0xFF garbage";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server with text: ") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 10 – Compression type exceeds RSSL_COMP_MAX_TYPE
 *
 * ipcWaitAck reads comp from buf[16..17] and checks
 * (comp > RSSL_COMP_MAX_TYPE) → RIPC_CONN_ERROR.
 * Setting comp = 0xFF (255) exceeds the maximum.                          */
TEST_F(RsslWaitAckInvalidResponseTests, InvalidCompressionType_ClientRejectsConnection)
{
    fakeArg.port = 15809;
    /* Use RSSL_COMP_MAX_TYPE + 1 as the compression type value            */
    const RsslUInt16 badComp = static_cast<RsslUInt16>(RSSL_COMP_MAX_TYPE + 1);
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, badComp);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15809";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with invalid compression type";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (3)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 11 – Version field = RIPC_VERSION_10 with unknown compression type
 *
 * RIPC_VERSION_10 uses a different compression branch in ipcWaitAck
 * (inDecompress path only, not bidirectional). Setting comp > MAX_TYPE
 * exercises the version-10 compression-function null-check path.          */
TEST_F(RsslWaitAckInvalidResponseTests, V10InvalidCompressionType_ClientRejectsConnection)
{
    fakeArg.port = 15810;
    const RsslUInt16 badComp = static_cast<RsslUInt16>(RSSL_COMP_MAX_TYPE + 1);
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_10, badComp);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15810";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with v10 + invalid compression type";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (3)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 13 – Maximum wire length field (0xFFFF) with only 19 bytes, then EOF
 *
 * ipcWaitAck reads cc = 19 bytes.  cc >= IPC_100_CONN_ACK passes.
 * But then parsing proceeds with a valid opCode / version, and the channel
 * might try to allocate buffers with maxMsgSize derived from the message.
 * We also set an unknown version to force a definitive rejection.          */
TEST_F(RsslWaitAckInvalidResponseTests, MaxLengthFieldEarlyClose_ClientRejectsConnection)
{
    fakeArg.port = 15812;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Declare 65535 bytes but send only 19, then close                    */
    fakeArg.response[0] = 0xFF;
    fakeArg.response[1] = 0xFF;

    /* Force an unknown version to ensure rejection                        */
    fakeArg.response[6] = 0xDE;
    fakeArg.response[7] = 0xAD;
    fakeArg.response[8] = 0xBE;
    fakeArg.response[9] = 0xEF;

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15812";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with max-length + unknown version";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid IPC Version: (-559038737)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 14 – Alternating 0x00/0xFF pattern (32 bytes)
 *
 * Byte pattern: 0x00, 0xFF, 0x00, 0xFF, ...
 * Length = 0x00FF (255, > IPC_100_CONN_ACK), flags = 0xFF (extended set),
 * opCode = 0x00 (neither CONNACK nor CONNNAK) → RIPC_CONN_ERROR.          */
TEST_F(RsslWaitAckInvalidResponseTests, AlternatingPattern_ClientRejectsConnection)
{
    fakeArg.port = 15813;
    for (int i = 0; i < 32; ++i)
        fakeArg.response[i] = static_cast<unsigned char>((i % 2 == 0) ? 0x00u : 0xFFu);
    fakeArg.responseLen   = 32;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15813";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with alternating pattern";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Opcode: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 16 – CONNNAK with zero-length error text (nakTextLen = 0)
 *
 * ipcWaitAck checks msgLen > IPC_100_OTHER_HEADER_SIZE before reading NAK
 * text.  When msgLen = IPC_100_CONN_ACK (10, below the threshold) the NAK
 * text is not read.  The channel must still be refused.                   */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNakZeroNakText_ClientRefused)
{
    fakeArg.port = 15815;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Set IPC_CONNNAK and keep msgLen = IPC_100_CONN_ACK (< threshold)   */
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);
    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = static_cast<unsigned char>(IPC_100_CONN_ACK);
    fakeArg.responseLen  = IPC_100_CONN_ACK;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15815";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE on CONNNAK with zero-length NAK text";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 17 – RIPC_VERSION_11 with invalid compression type
 *
 * RIPC_VERSION_11 uses the bidirectional compression branch in ipcWaitAck
 * (same branch as versions 12–14). An invalid comp type → RIPC_CONN_ERROR. */
TEST_F(RsslWaitAckInvalidResponseTests, V11InvalidCompression_ClientRejectsConnection)
{
    fakeArg.port = 15816;
    const RsslUInt16 badComp = static_cast<RsslUInt16>(RSSL_COMP_MAX_TYPE + 1);
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_11, badComp);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15816";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with v11 + invalid compression";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (3)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 18 – RIPC_VERSION_12 with invalid compression type
 * ========================================================================= */
TEST_F(RsslWaitAckInvalidResponseTests, V12InvalidCompression_ClientRejectsConnection)
{
    fakeArg.port = 15817;
    const RsslUInt16 badComp = static_cast<RsslUInt16>(RSSL_COMP_MAX_TYPE + 1);
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_12, badComp);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15817";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with v12 + invalid compression";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (3)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 19 – RIPC_VERSION_13 with invalid compression type
 * ========================================================================= */
TEST_F(RsslWaitAckInvalidResponseTests, V13InvalidCompression_ClientRejectsConnection)
{
    fakeArg.port = 15818;
    const RsslUInt16 badComp = static_cast<RsslUInt16>(RSSL_COMP_MAX_TYPE + 1);
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_13, badComp);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15818";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with v13 + invalid compression";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (3)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 20 – Version number = 0x00000000 (not a valid RIPC version)
 *
 * The default case in the version switch → RIPC_CONN_ERROR.               */
TEST_F(RsslWaitAckInvalidResponseTests, ZeroVersion_ClientRejectsConnection)
{
    fakeArg.port = 15819;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                0x00000000, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15819";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with version = 0x00000000";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid IPC Version: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 21 – Extended opCode has both CONNACK and CONNNAK bits set
 *
 * opCode = IPC_CONNACK | IPC_CONNNAK = 0x03.  ipcWaitAck evaluates
 * (opCode & IPC_CONNNAK) first → the NAK branch fires → RIPC_CONN_REFUSED. */
TEST_F(RsslWaitAckInvalidResponseTests, BothConnAckAndConnNakBits_ClientRefused)
{
    fakeArg.port = 15820;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Set both IPC_CONNACK (0x01) and IPC_CONNNAK (0x02) */
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNACK | IPC_CONNNAK);
    fakeArg.responseLen  = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15820";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with both CONNACK+CONNNAK bits set";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 22 – Exactly IPC_100_CONN_ACK - 1 bytes (9 bytes), then EOF
 *
 * cc = 9 < IPC_100_CONN_ACK (10) → "Invalid IPC Mount Ack" error path.   */
TEST_F(RsslWaitAckInvalidResponseTests, NineBytes_OneShortOfMinimum_ClientRejectsConnection)
{
    fakeArg.port = 15821;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response));
    ASSERT_GT(len, 0);

    /* Send only 9 bytes – one less than IPC_100_CONN_ACK                  */
    fakeArg.responseLen   = IPC_100_CONN_ACK - 1;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15821";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached) << "Client should NOT reach ACTIVE with only 9 bytes (IPC_100_CONN_ACK - 1)";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Ack.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 23 – Valid CONNACK for RIPC_VERSION_14, no compression (happy path)
 *
 * This test verifies the positive path: a well-formed CONNACK causes
 * the client channel to reach RSSL_CH_STATE_ACTIVE.                        */
TEST_F(RsslWaitAckInvalidResponseTests, ValidConnAck_V14_ClientReachesActive)
{
    fakeArg.port = 15822;
    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);
    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15822";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_TRUE(reached) << "Client SHOULD reach ACTIVE with a valid v14 CONNACK";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Crash-focused negative tests
 *
 * These tests target the field-parsing paths inside ipcWaitAck that read
 * variable-length or user-controlled numeric values and use them directly
 * in memcpy / malloc calls without upper-bound validation.  Each test
 * sends a syntactically plausible CONNACK (or CONNNAK) with one field set
 * to an extreme value so that the library must handle it without crashing,
 * corrupting the heap, or writing past the end of a fixed-size buffer.
 *
 * The library is expected to detect the invalid condition and reject the
 * channel (RSSL_RET_FAILURE or RSSL_RET_CHAN_INIT_REFUSED).  A test passes
 * if the process survives and the channel is not ACTIVE.
 * ========================================================================= */

/* =========================================================================
 * Test 24 – CONNNAK with nakTextLen = 0xFFFF (claims 65535 error-text bytes)
 *
 * ipcWaitAck reads nakTextLen = buf[6..7] (u16 big-endian) when opCode =
 * IPC_CONNNAK.  It then copies nakTextLen bytes into the fixed
 * MAX_RSSL_ERROR_TEXT buffer (~4096 bytes).  A value of 0xFFFF (65535)
 * must not be honoured: the library must clamp or reject before memcpy.  */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNak_OversizedNakTextLen_NoHeapOverrun)
{
    fakeArg.port = 15830;

    memset(fakeArg.response, 0, sizeof(fakeArg.response));

    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = static_cast<unsigned char>(sizeof(fakeArg.response));
    fakeArg.response[2] = 0x03;  /* IPC_EXTENDED_FLAGS | IPC_DATA          */
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);
    fakeArg.response[4] = static_cast<unsigned char>(IPC_100_CONN_ACK);
    fakeArg.response[5] = 0x00;

    /* nakTextLen = 0xFFFF – claims 65535 bytes of error text              */
    fakeArg.response[6] = 0xFF;
    fakeArg.response[7] = 0xFF;

    memset(fakeArg.response + 8, 'A', sizeof(fakeArg.response) - 8);

    fakeArg.responseLen   = static_cast<int>(sizeof(fakeArg.response));
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15830";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with CONNNAK nakTextLen = 0xFFFF";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server with text: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 26 – CONNACK with maxMsgSize = 0 (zero-size buffer allocation)
 *
 * After a successful CONNACK parse, downstream buffer-pool setup uses
 * maxMsgSize to size allocations.  A value of 0 must not result in a
 * null-pointer dereference or infinite loop.                              */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_ZeroMaxMsgSize_NoNullDeref)
{
    fakeArg.port = 15832;

    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);

    fakeArg.response[10] = 0x00;
    fakeArg.response[11] = 0x00;  /* maxMsgSize = 0                        */

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15832";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached);
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid maximum user message size: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 27 – CONNACK with maxMsgSize = 0xFFFF (maximum u16 value)
 *
 * Checks that (maxMsgSize + overhead) arithmetic does not overflow to a
 * near-zero allocation size, causing a subsequent heap overwrite.         */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_MaxMsgSizeMax_NoIntegerOverflow)
{
    fakeArg.port = 15833;

    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);

    fakeArg.response[10] = 0xFF;
    fakeArg.response[11] = 0xFF;  /* maxMsgSize = 65535                    */

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15833";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_TRUE(reached);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Test 28 – CONNACK with maxMsgSize = 1 (below any usable RIPC minimum)
 *
 * Buffer arithmetic like (maxMsgSize - dataHeaderLen) may underflow to a
 * large positive value when maxMsgSize < dataHeaderLen.                   */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_MaxMsgSizeOne_NoCrash)
{
    fakeArg.port = 15834;

    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);

    fakeArg.response[10] = 0x00;
    fakeArg.response[11] = 0x01;  /* maxMsgSize = 1                        */

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15834";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_TRUE(reached);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Test 29 – CONNACK with compVerLen = 0xFF at buf[22]
 *
 * For versions >= RIPC_VERSION_13, ipcWaitAck reads compVerLen from
 * buf[23] and calls _rsslMalloc(compVerLen) + memcpy(compVerLen bytes
 * from buf+23).  With only the 23-byte frame received, buf+23 is already
 * one byte past the end of the TCP buffer – a 255-byte memcpy from there
 * reads into uninitialised heap.                                          */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_OversizedCompVerLen_NoBufferOverread)
{
    fakeArg.port = 15835;

    static const int FRAME = 23;
    unsigned char buf[FRAME];
    int baselen = buildValidConnAck(buf, sizeof(buf), RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(baselen, 0);

    buf[22] = 0xFF;   /* compVerLen = 255 at the byte right after the base frame */

    buf[0] = 0x00;
    buf[1] = FRAME;   /* declare the frame as 23 bytes                     */

    memcpy(fakeArg.response, buf, FRAME);
    fakeArg.responseLen   = FRAME;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15835";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached);
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid component version length: (255)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 30 – CONNACK with pingTimeout = 0xFF (maximum u8 value)
 *
 * Valid range is IPC_MINIMUM_PINGTIMEOUT (1) to IPC_MAXIMUM_PINGTIMEOUT
 * (0xFF = 255).  0xFF is the largest allowed value and must be stored
 * without triggering any guard or crash.                                  */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_PingTimeoutMax_AcceptedOrRejectedCleanly)
{
    fakeArg.port = 15836;

    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);

    fakeArg.response[13] = 0xFF;  /* pingTimeout = 255 at [13]             */

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15836";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_EQ(0xFF, pClientChnl->pingTimeout);
    EXPECT_TRUE(reached);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Test 31 – CONNACK with compression = 0xFFFF (u16 max)
 *
 * Uses the absolute maximum u16 value to ensure no integer wrap occurs
 * before the (comp > RSSL_COMP_MAX_TYPE) bounds check.                   */
TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_CompressionU16Max_NoIntegerWrap)
{
    fakeArg.port = 15837;

    int len = buildValidConnAck(fakeArg.response, sizeof(fakeArg.response),
                                RIPC_VERSION_14,
                                static_cast<RsslUInt16>(0xFFFF));
    ASSERT_GT(len, 0);

    fakeArg.responseLen   = len;
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15837";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)<< "Client should NOT reach ACTIVE with compression = 0xFFFF";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Server has specified an unknown compression type (65535)") != NULL);
    EXPECT_NE(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Test 32 – CONNACK delivered byte-by-byte (1 byte per 2 ms)
 *
 * Exercises the RIPC_RW_WAITALL retry loop inside ipcWaitAck for the case
 * where the TCP stack delivers one byte at a time.  Tests that no
 * out-of-bounds access occurs when cc < expected on each partial read.    */

struct ByteDripServerArg
{
    unsigned short    port;
    unsigned char     frame[32];
    int               frameLen;
    int               bytePauseMs;
    std::atomic<bool> ready;
    std::atomic<bool> done;
    WaitAckRawSocket  listenSock;

    ByteDripServerArg()
        : port(0), frameLen(0), bytePauseMs(2),
          ready(false), done(false),
          listenSock(kWaitAckInvalidSocket)
    {
        memset(frame, 0, sizeof(frame));
    }
};

static RSSL_THREAD_DECLARE(byteDripServerThread, pArg)
{
    ByteDripServerArg* arg = reinterpret_cast<ByteDripServerArg*>(pArg);

    arg->listenSock = waitAckBindListen(arg->port);
    if (arg->listenSock == kWaitAckInvalidSocket)
    {
        arg->done  = true;
        arg->ready = true;
        return 0;
    }
    arg->ready = true;

    WaitAckRawSocket conn = waitAckAcceptOne(arg->listenSock);
    if (conn != kWaitAckInvalidSocket)
    {
        waitAckDrain(conn, 512);
        for (int i = 0; i < arg->frameLen; ++i)
        {
            waitAckSendAll(conn, arg->frame + i, 1);
            waitAckSleepMs(arg->bytePauseMs);
        }
        waitAckSleepMs(300);
        waitAckCloseSocket(conn);
    }

    waitAckCloseSocket(arg->listenSock);
    arg->listenSock = kWaitAckInvalidSocket;
    arg->done = true;
    return 0;
}

TEST_F(RsslWaitAckInvalidResponseTests, ConnAck_ByteByByteDrip_V14_NoCrash)
{
    ByteDripServerArg dripArg;
    dripArg.port        = 15838;
    dripArg.bytePauseMs = 2;

    int len = buildValidConnAck(dripArg.frame, sizeof(dripArg.frame),
                                RIPC_VERSION_14, RSSL_COMP_NONE);
    ASSERT_GT(len, 0);
    dripArg.frameLen = len;

    RsslThreadId dripTid;
    RSSL_THREAD_START(&dripTid, byteDripServerThread, &dripArg);

    for (int i = 0; i < 500 && !dripArg.ready.load(); ++i)
        waitAckSleepMs(2);
    ASSERT_TRUE(dripArg.ready.load()) << "Drip server not ready";

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%u",
             static_cast<unsigned>(dripArg.port));

    RsslError err;
    RsslConnectOptions opts;
    rsslClearConnectOpts(&opts);
    opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    opts.connectionInfo.unified.serviceName = portStr;
    opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    opts.blocking                           = RSSL_FALSE;

    pClientChnl = rsslConnect(&opts, &err);
    ASSERT_NE(nullptr, pClientChnl) << "rsslConnect failed for drip test";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError, 600, 5);
    EXPECT_FALSE(reached);
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Ack.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);

    for (int i = 0; i < 200 && !dripArg.done.load(); ++i)
        waitAckSleepMs(10);
}

/* =========================================================================
 * Test 33 – CONNNAK nakTextLen = MAX_RSSL_ERROR_TEXT - 1 (off-by-one edge)
 *
 * ipcWaitAck copies min(nakTextLen, MAX_RSSL_ERROR_TEXT - 1) characters
 * into the error text buffer.  Setting the declared length exactly at the
 * boundary exposes any off-by-one in the clamping guard.                 */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNak_NakTextLenAtErrorTextEdge_NoOffByOne)
{
    fakeArg.port = 15839;

    memset(fakeArg.response, 0, sizeof(fakeArg.response));

    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = static_cast<unsigned char>(sizeof(fakeArg.response));
    fakeArg.response[2] = 0x03;
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);
    fakeArg.response[4] = static_cast<unsigned char>(IPC_100_CONN_ACK);
    fakeArg.response[5] = 0x00;

    fakeArg.response[6] = 0x0F;
    fakeArg.response[7] = 0xFF;

    memset(fakeArg.response + 8, 'X', sizeof(fakeArg.response) - 8);

    fakeArg.responseLen   = static_cast<int>(sizeof(fakeArg.response));
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15839";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with CONNNAK nakTextLen=MAX_RSSL_ERROR_TEXT-1";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server with text: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 34 – All-0x80 pattern (high-bit set in every byte)
 *
 * 0x80 = 128.  For the flags byte this means IPC_EXTENDED_FLAGS (bit 0)
 * is clear, so opCode stays 0 and !(opCode & IPC_CONNACK) fires.  Tests
 * that no sign-extension bug in u8 → signed integer casts causes a crash
 * when every byte has the high bit set.                                   */
TEST_F(RsslWaitAckInvalidResponseTests, AllHighBitPattern_NoSignExtensionCrash)
{
    fakeArg.port = 15840;
    memset(fakeArg.response, 0x80, 32);
    fakeArg.responseLen   = 32;
    fakeArg.closeAfterSend = true;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15840";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with all-0x80 pattern";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Invalid IPC Mount Opcode: (0)") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}

/* =========================================================================
 * Test 35 – CONNNAK with nakTextLen = 1 (minimum non-zero NAK text)
 *
 * Smallest value that exercises the NAK-text memcpy branch.  Verifies
 * the copy does not underrun or corrupt adjacent memory when the declared
 * length is exactly 1.                                                    */
TEST_F(RsslWaitAckInvalidResponseTests, ConnNak_NakTextLenOne_NoUnderrun)
{
    fakeArg.port = 15841;

    memset(fakeArg.response, 0, sizeof(fakeArg.response));
    fakeArg.response[0] = 0x00;
    fakeArg.response[1] = static_cast<unsigned char>(sizeof(fakeArg.response));
    fakeArg.response[2] = 0x03;
    fakeArg.response[3] = static_cast<unsigned char>(IPC_CONNNAK);
    fakeArg.response[4] = static_cast<unsigned char>(IPC_100_CONN_ACK);
    fakeArg.response[5] = 0x00;

    /* nakTextLen = 1 */
    fakeArg.response[6] = 0x00;
    fakeArg.response[7] = 0x01;

    /* 1-byte error text */
    fakeArg.response[8] = 'E';

    fakeArg.responseLen   = static_cast<int>(sizeof(fakeArg.response));
    fakeArg.closeAfterSend = false;

    ASSERT_TRUE(startFakeServer()) << "Fake server failed to start on port 15841";
    ASSERT_TRUE(connectClient())   << "rsslConnect failed";

    RsslError rsslError;
    bool reached = driveClientToTerminal(pClientChnl, &rsslError);
    EXPECT_FALSE(reached)
        << "Client should NOT reach ACTIVE with CONNNAK nakTextLen=1";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 This connection has received a negative acknowledgement response from the server with text: E") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pClientChnl->state);
}
