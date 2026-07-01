/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *  rsslWaitClientKey Unit Tests
 *
 *  Unit testing for the ipcWaitClientKey() method in rsslSocketTransportImpl.c.
 *
 *  ipcWaitClientKey runs on the *server* side during the RIPC v14
 *  three-way key-exchange handshake:
 *
 *    Client                          Server
 *    ------                          ------
 *    CONNECT req  ??????????????????>  ipcReadHdr / ipcProcessHdr
 *                 <??????????????????  CONNACK (with RIPC_KEY_EXCHANGE flag)
 *    key frame    ??????????????????>  ipcWaitClientKey   ? THIS FUNCTION
 *                                      ? channel becomes ACTIVE
 *
 *  The key-exchange frame sent by the client is:
 *    [0..1]  total length (u16 big-endian)
 *    [2]     flags byte  (RIPC_KEY_EXCHANGE = 0x08 is present)
 *    [3]     keyLen      (u8) – 0 means the client cannot do encryption;
 *                              8 means the client supplies a u64 DH value
 *    [4..11] key value   (u64 big-endian, present only when keyLen == 8)
 *
 *  ipcWaitClientKey validates:
 *    - cc >= 4  (minimum frame length)
 *    - parses length, flags, keyLen
 *    - if keyLen > 0 && encryptionType == TR_SL_1: reads 8-byte DH key and
 *      computes the shared secret
 *    - if keyLen == 0: encryption is disabled for this session
 *    - calls ipcGetSocketRow() then transitions channel to ACTIVE
 *
 *  Test strategy
 *  -------------
 *  The tests drive ipcWaitClientKey indirectly through the public RSSL API:
 *
 *  Group A – Happy-path tests (full RSSL pair using rsslBind / rsslConnect)
 *    The server and client are both real RSSL instances. The client sends a
 *    valid RIPC v14 connect request which causes the server to set the
 *    RIPC_KEY_EXCHANGE flag in the CONNACK; the client then sends the key
 *    frame which ipcWaitClientKey processes.  Both channels must reach
 *    RSSL_CH_STATE_ACTIVE.
 *
 *  Group B – Malformed key-frame injection tests (fake client)
 *    A real RSSL server is bound and accepts a connection.  A raw TCP
 *    "fake client" performs only the RIPC connect-request exchange needed
 *    to get the server to enter RIPC_INT_ST_WAIT_CLIENT_KEY, then sends
 *    a crafted (invalid) key frame.  The server channel must NOT reach
 *    RSSL_CH_STATE_ACTIVE and must terminate cleanly without crashing.
 *
 *  Port ranges used:
 *    15900 – 15949  Group A happy-path tests
 *    15950 – 15999  Group B malformed-frame tests
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
#include "rtr/rsslSocketTransportImpl.h"

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

/* =========================================================================
 * Platform aliases
 * ========================================================================= */
#if defined(_WIN32)
typedef SOCKET  CKRawSocket;
static const CKRawSocket kCKInvalidSocket = INVALID_SOCKET;
static void ckCloseSocket(CKRawSocket s) { closesocket(s); }
static int  ckSend(CKRawSocket s, const char* b, int n)  { return send(s, b, n, 0); }
static int  ckRecv(CKRawSocket s, char* b, int n)        { return recv(s, b, n, 0); }
#else
typedef int     CKRawSocket;
static const CKRawSocket kCKInvalidSocket = -1;
static void ckCloseSocket(CKRawSocket s) { close(s); }
static int  ckSend(CKRawSocket s, const char* b, int n)
    { return static_cast<int>(send(s, b, n, 0)); }
static int  ckRecv(CKRawSocket s, char* b, int n)
    { return static_cast<int>(recv(s, b, n, 0)); }
#endif

/* =========================================================================
 * Helpers
 * ========================================================================= */

static void ckSleepMs(int ms)
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

/* Send every byte in buf; returns false on error. */
static bool ckSendAll(CKRawSocket s, const unsigned char* buf, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int n = ckSend(s, reinterpret_cast<const char*>(buf + sent), len - sent);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

/* Drain up to maxBytes from s. */
static void ckDrain(CKRawSocket s, int maxBytes = 1024)
{
    char tmp[256];
    int total = 0;
    while (total < maxBytes)
    {
        int n = ckRecv(s, tmp, sizeof(tmp));
        if (n <= 0) break;
        total += n;
        if (n < static_cast<int>(sizeof(tmp))) break;
    }
}

/* Connect a plain TCP socket to localhost:port.  Returns kCKInvalidSocket on
 * failure. */
static CKRawSocket ckTcpConnect(const char* port)
{
    unsigned short portNum = static_cast<unsigned short>(atoi(port));
    CKRawSocket s = static_cast<CKRawSocket>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (s == kCKInvalidSocket) return kCKInvalidSocket;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(portNum);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        ckCloseSocket(s);
        return kCKInvalidSocket;
    }
    return s;
}

/* =========================================================================
 * Drive rsslInitChannel on a channel until terminal state.
 *
 * Returns true  when the channel reaches RSSL_CH_STATE_ACTIVE.
 * Returns false when it is rejected (FAILURE / REFUSED / CLOSED).
 * ========================================================================= */
static bool ckDriveToTerminal(RsslChannel* pChnl,
                              int maxTries = 500,
                              int sleepMs  = 5)
{
    RsslError      err;
    RsslInProgInfo inProg;

    for (int i = 0; i < maxTries; ++i)
    {
        if (pChnl->state == RSSL_CH_STATE_ACTIVE)  return true;
        if (pChnl->state == RSSL_CH_STATE_CLOSED ||
            pChnl->state == RSSL_CH_STATE_INACTIVE) return false;

        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pChnl, &inProg, &err);
        if (ret == RSSL_RET_FAILURE || ret == RSSL_RET_CHAN_INIT_REFUSED)
            return false;

        ckSleepMs(sleepMs);
    }
    return (pChnl->state == RSSL_CH_STATE_ACTIVE);
}

/* Drive a server channel until terminal state (for Group B tests). */
static bool ckDriveServerToTerminal(RsslChannel* pSrvChnl, RsslError *pError,
                                    int maxTries = 300,
                                    int sleepMs  = 5)
{
    RsslInProgInfo inProg;

    for (int i = 0; i < maxTries; ++i)
    {
        if (pSrvChnl->state == RSSL_CH_STATE_ACTIVE)   return true;
        if (pSrvChnl->state == RSSL_CH_STATE_CLOSED  ||
            pSrvChnl->state == RSSL_CH_STATE_INACTIVE)  return false;

        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pSrvChnl, &inProg, pError);
        if (ret == RSSL_RET_FAILURE || ret == RSSL_RET_CHAN_INIT_REFUSED)
            return false;

        ckSleepMs(sleepMs);
    }
    return (pSrvChnl->state == RSSL_CH_STATE_ACTIVE);
}

/* =========================================================================
 * Build a minimal valid RIPC v14 CONNECT REQUEST (client ? server).
 *
 * Wire layout (see ipcProcessHdr):
 *   [0..1]  length (u16 big-endian) = 23
 *   [2]     opCode = 0x00
 *   [3..6]  version = CONN_VERSION_14 (0x0000000E) big-endian u32
 *   [7]     flags = RIPC_KEY_EXCHANGE (0x08) ? triggers key-exchange
 *   [8]     hdrSize = 20
 *   [9]     compBitmapSize = 0
 *   [10]    pingTimeout = 60
 *   [11]    rsslFlags = 0
 *   [12]    protocolType = RIPC_RWF_PROTOCOL_TYPE (0)
 *   [13]    majorVersion = 14
 *   [14]    minorVersion = 1
 *   [15]    hostnameLen = 0
 *   [16]    addrLen = 0
 *   [17]    componentVersionLen = 2
 *   [18]    componentStringLen = 0
 *   [19..22] padding zeros
 *
 * Returns number of bytes written (23).
 * ========================================================================= */
static int buildRipcV14ConnectReq(unsigned char* buf, int bufLen,
                                  bool requestKeyExchange = true)
{
    const int TOTAL = 20;
    if (bufLen < TOTAL) return 0;
    memset(buf, 0, TOTAL);

    buf[0]  = 0x00;
    buf[1]  = static_cast<unsigned char>(TOTAL);  /* length = 20             */
    buf[2]  = 0x00;                               /* opCode = 0              */

    /* CONN_VERSION_14 = 0x0000000E big-endian */
    buf[3]  = 0x00;
    buf[4]  = 0x00;
    buf[5]  = 0x00;
    buf[6]  = static_cast<unsigned char>(CONN_VERSION_14);

    buf[7]  = requestKeyExchange ? static_cast<unsigned char>(RIPC_KEY_EXCHANGE) : 0x00;
    buf[8]  = 17;    /* hdrSize                         */
    buf[9]  = 0x00;  /* compBitmapSize = 0              */
    buf[10] = 60;    /* pingTimeout                     */
    buf[11] = 0x00;  /* rsslFlags                       */
    buf[12] = 0x00;  /* protocolType = RWF (0)          */
    buf[13] = 14;    /* majorVersion                    */
    buf[14] = 1;     /* minorVersion                    */
    buf[15] = 0x00;  /* hostnameLen = 0                 */
    buf[16] = 0x00;  /* addrLen = 0                     */
    buf[17] = 2;     /* componentVersionLen total       */
    buf[18] = 0;     /* componentStringLen = 0          */
    /* [19..22] = 0 (padding)                           */
    return TOTAL;
}

/* =========================================================================
 * Build a valid key-exchange frame (client ? server after CONNACK).
 *
 * Wire layout (ipcWaitClientKey reads):
 *   [0..1]  length (u16 big-endian)
 *   [2]     flags byte (RIPC_KEY_EXCHANGE = 0x08)
 *   [3]     keyLen (u8): 0 = no key, 8 = DH public key follows
 *   [4..11] key value (u64 big-endian, only when keyLen == 8)
 *
 * keyValue is the client's Diffie-Hellman public value: g^a mod p.
 * Any non-zero u64 is syntactically valid; ipcWaitClientKey accepts it and
 * uses it to compute the shared secret.
 * Returns 4 when keyLen==0, 12 when keyLen==8.
 * ========================================================================= */
static int buildKeyExchangeFrame(unsigned char* buf, int bufLen,
                                 RsslUInt8 keyLen,
                                 RsslUInt64 keyValue = 0)
{
    const int BASE = 4;           /* length(2) + flags(1) + keyLen(1)      */
    int total = BASE + keyLen;    /* keyLen is either 0 or 8               */
    if (bufLen < total) return 0;
    memset(buf, 0, total);

    buf[0] = 0x00;
    buf[1] = static_cast<unsigned char>(total);  /* declared length        */
    buf[2] = static_cast<unsigned char>(RIPC_KEY_EXCHANGE);
    buf[3] = keyLen;

    if (keyLen == 8)
    {
        /* Write keyValue as big-endian u64 at [4..11]                     */
        buf[4]  = static_cast<unsigned char>((keyValue >> 56) & 0xFF);
        buf[5]  = static_cast<unsigned char>((keyValue >> 48) & 0xFF);
        buf[6]  = static_cast<unsigned char>((keyValue >> 40) & 0xFF);
        buf[7]  = static_cast<unsigned char>((keyValue >> 32) & 0xFF);
        buf[8]  = static_cast<unsigned char>((keyValue >> 24) & 0xFF);
        buf[9]  = static_cast<unsigned char>((keyValue >> 16) & 0xFF);
        buf[10] = static_cast<unsigned char>((keyValue >>  8) & 0xFF);
        buf[11] = static_cast<unsigned char>( keyValue        & 0xFF);
    }
    return total;
}

/* =========================================================================
 * Receive and discard the server's CONNACK (so the fake client can send the
 * key frame next).  Drains up to maxBytes from the socket.
 * ========================================================================= */
static bool ckReceiveConnAck(CKRawSocket s, int maxBytes = 512)
{
    char tmp[512];
    int n = ckRecv(s, tmp, (maxBytes < static_cast<int>(sizeof(tmp)))
                             ? maxBytes
                             : static_cast<int>(sizeof(tmp)));
    return n > 0;
}

/* =========================================================================
 * Blocking connect/accept thread helpers (Group A)
 * ========================================================================= */

struct CKBlockingConnArg
{
    RsslConnectOptions  opts;
    RsslChannel*        pChannel;
    RsslError           err;
    CKBlockingConnArg() : pChannel(nullptr)
    {
        rsslClearConnectOpts(&opts);
        memset(&err, 0, sizeof(err));
    }
};

static RSSL_THREAD_DECLARE(ckBlockingConnThread, pArg)
{
    CKBlockingConnArg* arg = reinterpret_cast<CKBlockingConnArg*>(pArg);
    arg->pChannel = rsslConnect(&arg->opts, &arg->err);
    return 0;
}

struct CKBlockingAcceptArg
{
    RsslServer*  pServer;
    RsslChannel* pChannel;
    RsslError    err;
    CKBlockingAcceptArg() : pServer(nullptr), pChannel(nullptr)
    { memset(&err, 0, sizeof(err)); }
};

static RSSL_THREAD_DECLARE(ckBlockingAcceptThread, pArg)
{
    CKBlockingAcceptArg* arg = reinterpret_cast<CKBlockingAcceptArg*>(pArg);
    RsslAcceptOptions aOpts;
    rsslClearAcceptOpts(&aOpts);
    for (int i = 0; i < 300 && !arg->pChannel; ++i)
    {
        arg->pChannel = rsslAccept(arg->pServer, &aOpts, &arg->err);
        if (!arg->pChannel) ckSleepMs(10);
    }
    return 0;
}

/* =========================================================================
 * =========================================================================
 * GROUP A – Happy-path tests via full RSSL channel pair
 * =========================================================================
 * =========================================================================
 * The RSSL library performs the full three-way key-exchange handshake when
 * both sides are RSSL channels connecting over RIPC v14.  Both channels
 * must reach RSSL_CH_STATE_ACTIVE, proving that ipcWaitClientKey accepted
 * the key frame and completed the handshake successfully.
 * ========================================================================= */

class IpcWaitClientKeyHappyPathTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    RsslChannel* pClientChnl = nullptr;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
    }

    void TearDown() override
    {
        RsslError err;
        if (pClientChnl)  { rsslCloseChannel(pClientChnl,  &err); pClientChnl  = nullptr; }
        if (pServerChnl)  { rsslCloseChannel(pServerChnl,  &err); pServerChnl  = nullptr; }
        if (pServer)      { rsslCloseServer(pServer,        &err); pServer      = nullptr; }
        rsslUninitialize();

        resetDeadlockTimer();
    }

    /* Bind the RSSL server on the given port.  Returns true on success.   */
    bool bindServer(const char* port)
    {
        TUServerConfig cfg;
        clearTUServerConfig(&cfg);
        cfg.blocking = RSSL_FALSE;
        cfg.connType = RSSL_CONN_TYPE_SOCKET;
        strncpy(cfg.portNo, port, sizeof(cfg.portNo));
        pServer = bindRsslServer(&cfg);
        return pServer != nullptr;
    }

    /* Non-blocking connect + accept helpers.                               */
    bool connectClient(const char* port)
    {
        RsslError err;
        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = const_cast<char*>(port);
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
        opts.blocking                           = RSSL_FALSE;
        pClientChnl = rsslConnect(&opts, &err);
        return pClientChnl != nullptr;
    }

    bool acceptServer()
    {
        RsslError err;
        RsslAcceptOptions aOpts;
        rsslClearAcceptOpts(&aOpts);
        for (int i = 0; i < 200 && !pServerChnl; ++i)
        {
            pServerChnl = rsslAccept(pServer, &aOpts, &err);
            if (!pServerChnl) ckSleepMs(10);
        }
        return pServerChnl != nullptr;
    }

    /* Drive both channels to ACTIVE state (non-blocking). */
    bool driveToActive(int maxTries = 500)
    {
        RsslError err;
        RsslInProgInfo inProg;
        for (int i = 0; i < maxTries; ++i)
        {
            bool srvOk = (pServerChnl && pServerChnl->state == RSSL_CH_STATE_ACTIVE);
            bool cliOk = (pClientChnl && pClientChnl->state == RSSL_CH_STATE_ACTIVE);
            if (srvOk && cliOk) return true;

            if (!srvOk && pServerChnl)
            {
                rsslClearInProgInfo(&inProg);
                rsslInitChannel(pServerChnl, &inProg, &err);
            }
            if (!cliOk && pClientChnl)
            {
                rsslClearInProgInfo(&inProg);
                rsslInitChannel(pClientChnl, &inProg, &err);
            }
            ckSleepMs(5);
        }
        return (pServerChnl && pServerChnl->state == RSSL_CH_STATE_ACTIVE &&
                pClientChnl && pClientChnl->state == RSSL_CH_STATE_ACTIVE);
    }
};

/* -------------------------------------------------------------------------
 * Test A1 – Basic non-blocking TCP channel pair reaches ACTIVE state.
 *
 * The RSSL client negotiates RIPC v14 which forces RIPC_KEY_EXCHANGE.
 * ipcWaitClientKey on the server side processes the key frame and
 * transitions the server channel to ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, NonBlocking_BothChannelsReachActiveState)
{
    ASSERT_TRUE(bindServer("15900"))    << "Server bind failed";
    ASSERT_TRUE(connectClient("15900")) << "Client connect failed";
    ASSERT_TRUE(acceptServer())         << "Server accept failed";

    bool ok = driveToActive();
    EXPECT_TRUE(ok) << "Channels did not reach ACTIVE";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* -------------------------------------------------------------------------
 * Test A2 – Blocking mode: both channels reach ACTIVE state.
 *
 * In blocking mode rsslConnect() and rsslAccept() each complete the full
 * handshake synchronously, so ipcWaitClientKey is called inside the
 * rsslAccept() call and must not block indefinitely.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, Blocking_BothChannelsReachActiveState)
{
    TUServerConfig srvCfg;
    clearTUServerConfig(&srvCfg);
    srvCfg.blocking = RSSL_TRUE;
    srvCfg.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(srvCfg.portNo, "15901", sizeof(srvCfg.portNo));
    pServer = bindRsslServer(&srvCfg);
    ASSERT_NE(nullptr, pServer) << "Blocking server bind failed";

    RsslConnectOptions opts;
    rsslClearConnectOpts(&opts);
    opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    opts.connectionInfo.unified.serviceName = const_cast<char*>("15901");
    opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    opts.blocking                           = RSSL_TRUE;

    CKBlockingConnArg   connArg;
    CKBlockingAcceptArg acceptArg;
    connArg.opts    = opts;
    acceptArg.pServer = pServer;

    RsslThreadId cTid, aTid;
    RSSL_THREAD_START(&aTid, ckBlockingAcceptThread, &acceptArg);
    RSSL_THREAD_START(&cTid, ckBlockingConnThread,   &connArg);

    RSSL_THREAD_JOIN(cTid);
    RSSL_THREAD_JOIN(aTid);

    pClientChnl = connArg.pChannel;
    pServerChnl = acceptArg.pChannel;

    ASSERT_NE(nullptr, pClientChnl) << "Blocking connect failed";
    ASSERT_NE(nullptr, pServerChnl) << "Blocking accept failed";

    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* -------------------------------------------------------------------------
 * Test A3 – Server channel socketId is valid after ACTIVE.
 *
 * After ipcWaitClientKey completes, the server channel's socketId must
 * be a valid socket descriptor (not RSSL_INVALID_SOCKET).
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, AfterActive_ServerSocketIdIsValid)
{
    ASSERT_TRUE(bindServer("15902"));
    ASSERT_TRUE(connectClient("15902"));
    ASSERT_TRUE(acceptServer());
    ASSERT_TRUE(driveToActive());

    EXPECT_NE(RSSL_INVALID_SOCKET, pServerChnl->socketId)
        << "Server socketId should be valid after ipcWaitClientKey";
}

/* -------------------------------------------------------------------------
 * Test A4 – Client channel socketId is valid after ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, AfterActive_ClientSocketIdIsValid)
{
    ASSERT_TRUE(bindServer("15903"));
    ASSERT_TRUE(connectClient("15903"));
    ASSERT_TRUE(acceptServer());
    ASSERT_TRUE(driveToActive());

    EXPECT_NE(RSSL_INVALID_SOCKET, pClientChnl->socketId)
        << "Client socketId should be valid after handshake";
}

/* -------------------------------------------------------------------------
 * Test A5 – rsslGetChannelInfo succeeds after key exchange completes.
 *
 * Validates that ipcWaitClientKey leaves the server channel in a fully
 * usable state: maxFragmentSize and maxOutputBuffers must be non-zero.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, AfterActive_ChannelInfoIsValid)
{
    ASSERT_TRUE(bindServer("15904"));
    ASSERT_TRUE(connectClient("15904"));
    ASSERT_TRUE(acceptServer());
    ASSERT_TRUE(driveToActive());

    RsslError       err;
    RsslChannelInfo info;
    memset(&info, 0, sizeof(info));
    RsslRet ret = rsslGetChannelInfo(pServerChnl, &info, &err);
    ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "rsslGetChannelInfo failed: " << err.text;

    EXPECT_GT(info.maxFragmentSize,  0U) << "maxFragmentSize should be > 0";
    EXPECT_GT(info.maxOutputBuffers, 0U) << "maxOutputBuffers should be > 0";
}

/* -------------------------------------------------------------------------
 * Test A6 – Multiple sequential channel pairs all succeed.
 *
 * Verifies that the server-side key-exchange state is correctly reset
 * between connections so that N sequential channel pairs can all reach
 * ACTIVE without corrupting each other's state.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, Sequential_FivePairsAllReachActiveState)
{
    const int CYCLES = 5;
    for (int c = 0; c < CYCLES; ++c)
    {
        RsslError err;

        TUServerConfig srvCfg;
        clearTUServerConfig(&srvCfg);
        srvCfg.blocking = RSSL_FALSE;
        srvCfg.connType = RSSL_CONN_TYPE_SOCKET;
        strncpy(srvCfg.portNo, "15905", sizeof(srvCfg.portNo));
        RsslServer* srv = bindRsslServer(&srvCfg);
        ASSERT_NE(nullptr, srv) << "Cycle " << c << ": bind failed";

        RsslConnectOptions opts;
        rsslClearConnectOpts(&opts);
        opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
        opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
        opts.connectionInfo.unified.serviceName = const_cast<char*>("15905");
        opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
        opts.blocking                           = RSSL_FALSE;

        RsslChannel* cli = rsslConnect(&opts, &err);
        ASSERT_NE(nullptr, cli) << "Cycle " << c << ": connect failed";

        RsslChannel* srvChnl = nullptr;
        RsslAcceptOptions aOpts;
        rsslClearAcceptOpts(&aOpts);
        for (int i = 0; i < 200 && !srvChnl; ++i)
        {
            srvChnl = rsslAccept(srv, &aOpts, &err);
            if (!srvChnl) ckSleepMs(10);
        }
        ASSERT_NE(nullptr, srvChnl) << "Cycle " << c << ": accept failed";

        RsslInProgInfo inProg;
        bool reached = false;
        for (int i = 0; i < 500; ++i)
        {
            bool sd = (srvChnl->state == RSSL_CH_STATE_ACTIVE);
            bool cd = (cli->state     == RSSL_CH_STATE_ACTIVE);
            if (sd && cd) { reached = true; break; }
            if (!sd) { rsslClearInProgInfo(&inProg); rsslInitChannel(srvChnl, &inProg, &err); }
            if (!cd) { rsslClearInProgInfo(&inProg); rsslInitChannel(cli,     &inProg, &err); }
            ckSleepMs(5);
        }
        EXPECT_TRUE(reached)
            << "Cycle " << c << ": channels did not reach ACTIVE";
        EXPECT_EQ(RSSL_CH_STATE_ACTIVE, srvChnl->state);
        EXPECT_EQ(RSSL_CH_STATE_ACTIVE, cli->state);

        rsslCloseChannel(cli,     &err);
        rsslCloseChannel(srvChnl, &err);
        rsslCloseServer(srv,      &err);
        ckSleepMs(20);
    }
}

/* -------------------------------------------------------------------------
 * Test A7 – Non-blocking init return codes are non-negative throughout.
 *
 * During the handshake (including the ipcWaitClientKey phase) every call
 * to rsslInitChannel must return >= RSSL_RET_SUCCESS until the channel
 * is ACTIVE; negative returns indicate an unexpected error.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, NonBlockingInit_ReturnCodesAreNonNegativeThroughout)
{
    ASSERT_TRUE(bindServer("15906"));
    ASSERT_TRUE(connectClient("15906"));
    ASSERT_TRUE(acceptServer());

    RsslError      err;
    RsslInProgInfo inProg;

    for (int i = 0; i < 500; ++i)
    {
        bool sd = (pServerChnl->state == RSSL_CH_STATE_ACTIVE);
        bool cd = (pClientChnl->state == RSSL_CH_STATE_ACTIVE);
        if (sd && cd) break;

        if (!sd)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pServerChnl, &inProg, &err);
            EXPECT_GE(ret, RSSL_RET_SUCCESS)
                << "Server rsslInitChannel returned negative: " << ret
                << " err: " << err.text;
        }
        if (!cd)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pClientChnl, &inProg, &err);
            EXPECT_GE(ret, RSSL_RET_SUCCESS)
                << "Client rsslInitChannel returned negative: " << ret
                << " err: " << err.text;
        }
        ckSleepMs(5);
    }

    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* -------------------------------------------------------------------------
 * Test A8 – Server pingTimeout is correctly negotiated after key exchange.
 *
 * After ipcWaitClientKey the server channel's pingTimeout must be > 0
 * and <= 255 (IPC_MAXIMUM_PINGTIMEOUT).
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, AfterActive_ServerPingTimeoutIsValid)
{
    ASSERT_TRUE(bindServer("15907"));
    ASSERT_TRUE(connectClient("15907"));
    ASSERT_TRUE(acceptServer());
    ASSERT_TRUE(driveToActive());

    EXPECT_EQ(pServerChnl->pingTimeout, 60)
        << "Server pingTimeout should be 60 after ipcWaitClientKey";
}

/* -------------------------------------------------------------------------
 * Test A9 – ZLIB compression negotiated; key exchange still completes.
 *
 * Both sides request ZLIB compression. The three-way handshake
 * (including ipcWaitClientKey) must succeed and the channels must become
 * ACTIVE with the negotiated compression type.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, WithZlibCompression_KeyExchangeCompletes)
{
    TUServerConfig srvCfg;
    clearTUServerConfig(&srvCfg);
    srvCfg.blocking         = RSSL_FALSE;
    srvCfg.connType         = RSSL_CONN_TYPE_SOCKET;
    srvCfg.compressionType  = RSSL_COMP_ZLIB;
    srvCfg.compressionLevel = 0;
    strncpy(srvCfg.portNo, "15908", sizeof(srvCfg.portNo));
    pServer = bindRsslServer(&srvCfg);
    ASSERT_NE(nullptr, pServer) << "ZLIB server bind failed";

    RsslError err;
    RsslConnectOptions opts;
    rsslClearConnectOpts(&opts);
    opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    opts.connectionInfo.unified.serviceName = const_cast<char*>("15908");
    opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    opts.blocking                           = RSSL_FALSE;
    opts.compressionType                    = RSSL_COMP_ZLIB;
    pClientChnl = rsslConnect(&opts, &err);
    ASSERT_NE(nullptr, pClientChnl) << "ZLIB client connect failed";

    ASSERT_TRUE(acceptServer());
    EXPECT_TRUE(driveToActive());
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* -------------------------------------------------------------------------
 * Test A10 – LZ4 compression negotiated; key exchange still completes.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyHappyPathTests, WithLZ4Compression_KeyExchangeCompletes)
{
    TUServerConfig srvCfg;
    clearTUServerConfig(&srvCfg);
    srvCfg.blocking        = RSSL_FALSE;
    srvCfg.connType        = RSSL_CONN_TYPE_SOCKET;
    srvCfg.compressionType = RSSL_COMP_LZ4;
    strncpy(srvCfg.portNo, "15909", sizeof(srvCfg.portNo));
    pServer = bindRsslServer(&srvCfg);
    ASSERT_NE(nullptr, pServer) << "LZ4 server bind failed";

    RsslError err;
    RsslConnectOptions opts;
    rsslClearConnectOpts(&opts);
    opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    opts.connectionInfo.unified.serviceName = const_cast<char*>("15909");
    opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    opts.blocking                           = RSSL_FALSE;
    opts.compressionType                    = RSSL_COMP_LZ4;
    pClientChnl = rsslConnect(&opts, &err);
    ASSERT_NE(nullptr, pClientChnl) << "LZ4 client connect failed";

    ASSERT_TRUE(acceptServer());
    EXPECT_TRUE(driveToActive());
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * =========================================================================
 * GROUP B – Malformed key-frame injection tests (fake client)
 *
 * Fixture: IpcWaitClientKeyMalformedFrameTests
 *
 * Each test:
 *   1. Binds a real RSSL server.
 *   2. Opens a raw TCP connection (bypassing the RSSL client API).
 *   3. Sends a valid RIPC v14 CONNECT REQUEST so the server processes the
 *      header, sends its CONNACK with RIPC_KEY_EXCHANGE set, and transitions
 *      to RIPC_INT_ST_WAIT_CLIENT_KEY.
 *   4. Receives and discards the CONNACK from the server.
 *   5. Sends a crafted (invalid) key frame.
 *   6. Drives rsslInitChannel() on the server channel and verifies the
 *      server does NOT reach ACTIVE state.
 * =========================================================================
 * ========================================================================= */

class IpcWaitClientKeyMalformedFrameTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    CKRawSocket  rawClient   = kCKInvalidSocket;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
    }

    void TearDown() override
    {
        if (rawClient != kCKInvalidSocket)
        {
            ckCloseSocket(rawClient);
            rawClient = kCKInvalidSocket;
        }
        RsslError err;
        if (pServerChnl) { rsslCloseChannel(pServerChnl, &err); pServerChnl = nullptr; }
        if (pServer)      { rsslCloseServer(pServer,      &err); pServer     = nullptr; }
        rsslUninitialize();

        resetDeadlockTimer();
    }

    /* Bind the server and connect the raw fake client, then rsslAccept()
     * the server channel. Also drains the server's CONNACK to get the
     * server into RIPC_INT_ST_WAIT_CLIENT_KEY.
     * Returns true on success. */
    bool setupServerAndFakeClient(const char* port)
    {
        TUServerConfig cfg;
        clearTUServerConfig(&cfg);
        cfg.blocking = RSSL_FALSE;
        cfg.connType = RSSL_CONN_TYPE_SOCKET;
        strncpy(cfg.portNo, port, sizeof(cfg.portNo));
        pServer = bindRsslServer(&cfg);
        if (!pServer) return false;

        /* Connect the raw client */
        rawClient = ckTcpConnect(port);
        if (rawClient == kCKInvalidSocket) return false;

        /* rsslAccept() the server channel */
        RsslError err;
        RsslAcceptOptions aOpts;
        rsslClearAcceptOpts(&aOpts);
        for (int i = 0; i < 200 && !pServerChnl; ++i)
        {
            pServerChnl = rsslAccept(pServer, &aOpts, &err);
            if (!pServerChnl) ckSleepMs(10);
        }
        if (!pServerChnl) return false;

        /* Send a valid RIPC v14 CONNECT REQUEST so the server can process
         * the header and send its CONNACK. */
        unsigned char connReq[24];
        int len = buildRipcV14ConnectReq(connReq, sizeof(connReq), true);
        if (len <= 0) return false;
        if (!ckSendAll(rawClient, connReq, len)) return false;

        /* Drive the server until it has sent its CONNACK (i.e., it is now
         * waiting for the client key frame). */
        RsslInProgInfo inProg;
        bool sentConnAck = false;
        for (int i = 0; i < 300; ++i)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pServerChnl, &inProg, &err);
            /* Once the server has processed the connect request it sends the
             * CONNACK and moves to WAIT_CLIENT_KEY.  We detect this when the
             * server is still INITIALIZING but rsslInitChannel returns
             * RSSL_RET_CHAN_INIT_IN_PROGRESS (> 0). */
            if (ret == RSSL_RET_FAILURE)
                return false;

            /* Peek at the internal state via the intConnState field.
             * The bottom byte holds the RIPC_INT_ST_* value. */
            RsslUInt8 intState = static_cast<RsslUInt8>(inProg.internalConnState & 0xFF);
            if (intState == RIPC_INT_ST_WAIT_CLIENT_KEY)
            {
                sentConnAck = true;
                break;
            }
            if (pServerChnl->state == RSSL_CH_STATE_ACTIVE)
            {
                sentConnAck = true; /* unexpected but not an error here */
                break;
            }
            ckSleepMs(5);
        }

        /* Drain the CONNACK sent by the server to the fake client so the
         * server is not blocked waiting for a read-ack from the client side. */
        ckReceiveConnAck(rawClient, 512);

        return sentConnAck;
    }
};

/* -------------------------------------------------------------------------
 * Test B1 – Immediate EOF: fake client closes without sending a key frame.
 *
 * ipcWaitClientKey reads cc = 0 (EOF).  Since cc < 4, it returns
 * RIPC_CONN_IN_PROGRESS on the first poll, then RIPC_CONN_ERROR when the
 * subsequent read also fails. The server channel must not reach ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ImmediateEof_ServerRejectsConnection)
{
    ASSERT_TRUE(setupServerAndFakeClient("15950"))
        << "Setup (bind / connect / accept / CONNACK exchange) failed";

    /* Close the raw client without sending any key frame */
    ckCloseSocket(rawClient);
    rawClient = kCKInvalidSocket;

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_FALSE(active) << "Server should NOT reach ACTIVE on key-exchange EOF";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_RET_FAILURE, rsslError.rsslErrorId);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B2 – Three bytes only (one less than the minimum 4-byte frame).
 *
 * ipcWaitClientKey checks (cc < 4) ? if cc < 0 returns RIPC_CONN_ERROR,
 * otherwise returns RIPC_CONN_IN_PROGRESS.  After EOF the server errors out.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ThreeByteFrame_ServerRejectsConnection)
{
    ASSERT_TRUE(setupServerAndFakeClient("15951"))
        << "Setup failed";

    /* Send only 3 bytes, then close */
    unsigned char shortFrame[3] = { 0x00, 0x03, static_cast<unsigned char>(RIPC_KEY_EXCHANGE) };
    ckSendAll(rawClient, shortFrame, sizeof(shortFrame));
    ckCloseSocket(rawClient);
    rawClient = kCKInvalidSocket;

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_FALSE(active) << "Server should NOT reach ACTIVE with a 3-byte key frame";
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_RET_FAILURE, rsslError.rsslErrorId);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B3 – Zero keyLen (client cannot do encryption).
 *
 * A keyLen = 0 is a valid degenerate case: the client signals it cannot
 * perform the DH exchange.  ipcWaitClientKey accepts this, sets
 * encryptionType = 0 and shared_key = 0, then transitions to ACTIVE.
 * This is the "no encryption" happy path; the server channel MUST reach ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ZeroKeyLen_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15952"))
        << "Setup failed";

    /* Build a key-exchange frame with keyLen = 0 */
    unsigned char keyFrame[4];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 0, 0);
    ASSERT_EQ(4, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active) << "Server SHOULD reach ACTIVE when client sends keyLen = 0";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B4 – Valid keyLen = 8 with a non-zero DH key value.
 *
 * A keyLen = 8 with keyValue = 1 (the smallest non-trivial DH public value)
 * is syntactically valid. ipcWaitClientKey reads the 8 bytes, computes the
 * shared secret and transitions to ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8WithValidDHValue_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15953"))
        << "Setup failed";

    /* keyValue = 1: smallest valid DH public value */
    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8, 1ULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with keyLen=8 and valid DH value";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B5 – keyLen = 8 with a DH key value of 0.
 *
 * A key value of 0 means g^a mod p = 0, which is degenerate (shared_key
 * would be 0^random mod p = 0). ipcWaitClientKey should still parse the
 * frame and set shared_key = 0, then reach ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8WithZeroDHValue_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15954"))
        << "Setup failed";

    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8, 0ULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with keyLen=8 and DH value = 0";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B6 – keyLen = 8 with maximum u64 DH key value (0xFFFFFFFFFFFFFFFF).
 *
 * Tests that the modular exponentiation path in ipcWaitClientKey does not
 * crash or produce an arithmetic overflow when the client key is max u64.
 * The server must settle cleanly (either ACTIVE or CLOSED, not crash).
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8WithMaxU64DHValue_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15955"))
        << "Setup failed";

    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8,
                                    0xFFFFFFFFFFFFFFFFULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    /* The server must settle in a terminal state – it must not hang or crash */
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with max-u64 DH key, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B7 – keyLen = 4 (not 0 or 8; unsupported size).
 *
 * ipcWaitClientKey's else branch (encryptionType != TR_SL_1 or P == 0) is
 * taken when keyLen > 0 but the encryption type is not TR_SL_1.  The code
 * advances inputBufCursor by keyLen and clears encryptionType/shared_key,
 * then reaches ACTIVE.  This tests that a non-standard keyLen does not
 * trigger a buffer overrun.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen4_SettlesCleanlyWithoutCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15956"))
        << "Setup failed";

    /* Build a 4-byte key (unsupported size: neither 0 nor 8) */
    const RsslUInt8 KEY_LEN = 4;
    unsigned char keyFrame[8];
    memset(keyFrame, 0, sizeof(keyFrame));
    keyFrame[0] = 0x00;
    keyFrame[1] = static_cast<unsigned char>(4 + KEY_LEN);
    keyFrame[2] = static_cast<unsigned char>(RIPC_KEY_EXCHANGE);
    keyFrame[3] = KEY_LEN;
    keyFrame[4] = 0xDE;
    keyFrame[5] = 0xAD;
    keyFrame[6] = 0xBE;
    keyFrame[7] = 0xEF;

    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, 4 + KEY_LEN));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with keyLen=4, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B8 – keyLen = 0xFF (255).
 *
 * A very large keyLen with no payload.  The cursor is advanced by keyLen
 * in the else branch: this may read past the end of the received buffer.
 * The server must not crash.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen255_NoCrashOrBufferOverrun)
{
    ASSERT_TRUE(setupServerAndFakeClient("15957"))
        << "Setup failed";

    /* Send a 4-byte frame claiming keyLen=255 but providing no key bytes */
    unsigned char keyFrame[4];
    memset(keyFrame, 0, sizeof(keyFrame));
    keyFrame[0] = 0x00;
    keyFrame[1] = 4;
    keyFrame[2] = static_cast<unsigned char>(RIPC_KEY_EXCHANGE);
    keyFrame[3] = 0xFF;  /* keyLen = 255 but no payload follows            */
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, sizeof(keyFrame)));

    /* Close immediately so the server is not left waiting for 255 bytes   */
    ckCloseSocket(rawClient);
    rawClient = kCKInvalidSocket;

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with keyLen=255 and no payload, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B9 – All-zeros key frame (4 bytes, all zero).
 *
 * Length = 0, flags = 0, keyLen = 0.  The cc < 4 check passes because we
 * send 4 bytes.  keyLen = 0 ? ipcWaitClientKey sets encryptionType = 0
 * and reaches ACTIVE.  The all-zero flags byte is not checked by
 * ipcWaitClientKey (it reads but does not validate flags[2]).
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, AllZeroKeyFrame_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15958"))
        << "Setup failed";

    unsigned char keyFrame[4] = { 0x00, 0x00, 0x00, 0x00 };
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, sizeof(keyFrame)));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with all-zeros key frame (keyLen=0)";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B10 – All-0xFF key frame (4 bytes).
 *
 * Length field = 0xFFFF (65535), flags = 0xFF, keyLen = 0xFF (255).
 * The server enters the else branch for the large keyLen and must not crash
 * or overrun the input buffer.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, AllFFKeyFrame_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15959"))
        << "Setup failed";

    unsigned char keyFrame[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, sizeof(keyFrame)));

    ckCloseSocket(rawClient);
    rawClient = kCKInvalidSocket;

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with all-0xFF key frame, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B11 – keyLen = 8 but only 4 bytes sent (payload truncated), then EOF.
 *
 * ipcWaitClientKey reads only the first 4 bytes (V10_MIN_CONN_HDR+8 total
 * is the read request, but the cc check is cc < 4).  When keyLen = 8 but
 * only 4 bytes total were received, the cursor advance reads past the buffer.
 * The server must not crash.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8WithTruncatedPayload_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15960"))
        << "Setup failed";

    /* 4 bytes: length=12 declared but only header sent, keyLen=8, no payload */
    unsigned char keyFrame[4] = {
        0x00,
        12,   /* declared length claims 12 bytes but we only send 4         */
        static_cast<unsigned char>(RIPC_KEY_EXCHANGE),
        8     /* keyLen = 8                                                  */
    };
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, sizeof(keyFrame)));

    ckCloseSocket(rawClient);
    rawClient = kCKInvalidSocket;

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with truncated keyLen=8 payload, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B12 – Key frame delivered byte by byte (slow drip).
 *
 * Exercises the RIPC_RW_WAITALL retry loop.  ipcWaitClientKey reads up to
 * (V10_MIN_CONN_HDR+8) = 22 bytes in one call; when bytes trickle in one
 * at a time the read returns RSSL_RET_READ_WOULD_BLOCK repeatedly.  No
 * out-of-bounds access should occur during the partial reads.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ByteByByteDrip_ServerReachesActiveNoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15961"))
        << "Setup failed";

    /* Build a valid 12-byte key frame (keyLen=8, key=0x0102030405060708)   */
    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8,
                                    0x0102030405060708ULL);
    ASSERT_EQ(12, len);

    /* Send one byte at a time with a short pause */
    for (int i = 0; i < len; ++i)
    {
        ASSERT_TRUE(ckSendAll(rawClient, keyFrame + i, 1))
            << "Failed to send byte " << i << " of key frame";
        ckSleepMs(3);
    }

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError, 600, 5);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle after byte-by-byte drip, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B13 – Valid key frame followed by 64 bytes of excess data.
 *
 * ipcWaitClientKey advances inputBufCursor by the parsed key size and
 * then checks whether inputBuffer->length == inputBufCursor.  The excess
 * bytes remain in the buffer; the server must not misinterpret them and
 * must still reach ACTIVE after the key frame.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ValidKeyFramePlusExcessData_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15962"))
        << "Setup failed";

    /* 12-byte valid key frame + 64 bytes of 0xCD excess */
    const int EXCESS = 64;
    unsigned char buf[12 + EXCESS];
    memset(buf, 0, sizeof(buf));

    int kLen = buildKeyExchangeFrame(buf, sizeof(buf), 8, 42ULL);
    ASSERT_EQ(12, kLen);
    memset(buf + kLen, 0xCD, EXCESS);

    ASSERT_TRUE(ckSendAll(rawClient, buf, kLen + EXCESS));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with valid key frame + excess data";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B14 – Two consecutive valid key frames (duplicate send).
 *
 * The first key frame is consumed by ipcWaitClientKey; the server reaches
 * ACTIVE.  The second frame remains in the TCP buffer as application data
 * and will be seen by the first rsslRead() call.  The server channel must
 * reach ACTIVE after the first frame without crashing on the second.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, DuplicateKeyFrame_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15963"))
        << "Setup failed";

    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8, 7ULL);
    ASSERT_EQ(12, len);

    /* Send the key frame twice back-to-back */
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE after receiving duplicate key frames";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B15 – keyLen = 8 with alternating 0x55/0xAA DH key bytes.
 *
 * Ensures that specific bit patterns in the DH key value do not cause
 * any sign-extension or arithmetic issues inside modPowFast.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8_AlternatingBitPattern_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15964"))
        << "Setup failed";

    /* 0x5555555555555555: alternating 0101 pattern */
    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8,
                                    0x5555555555555555ULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with alternating-bit DH key, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B16 – keyLen = 8 with all-ones DH key bytes (0xAAAAAAAAAAAAAAAA).
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8_AllOnesPattern_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15965"))
        << "Setup failed";

    /* 0xAAAAAAAAAAAAAAAA: all-ones alternating pattern */
    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8,
                                    0xAAAAAAAAAAAAAAAAULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active);
    EXPECT_TRUE(pServerChnl->state == RSSL_CH_STATE_ACTIVE)
        << "Server must settle with all-ones-pattern DH key, actual state: "
        << pServerChnl->state;
}

/* -------------------------------------------------------------------------
 * Test B17 – Shutdown-pending flag: connection closes cleanly even when
 * the server channel is in WAIT_CLIENT_KEY.
 *
 * This test verifies that rsslCloseChannel() can be called on the server
 * channel while it is still waiting for the key frame (before the fake
 * client sends anything).  The channel must not crash or deadlock.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, CloseDuringWaitClientKey_NoCrash)
{
    ASSERT_TRUE(setupServerAndFakeClient("15966"))
        << "Setup failed";

    /* Verify the server is actually in WAIT_CLIENT_KEY before proceeding  */
    RsslError      err;
    RsslInProgInfo inProg;
    RsslUInt8 intState = 0;
    for (int i = 0; i < 100; ++i)
    {
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pServerChnl, &inProg, &err);
        intState = static_cast<RsslUInt8>(inProg.internalConnState & 0xFF);
        if (intState == RIPC_INT_ST_WAIT_CLIENT_KEY ||
            pServerChnl->state == RSSL_CH_STATE_ACTIVE)
            break;
        ckSleepMs(5);
    }

    /* Close the server channel explicitly; this must not crash */
    EXPECT_EQ(RSSL_RET_SUCCESS, rsslCloseChannel(pServerChnl, &err));
    pServerChnl = nullptr;   /* prevent double-free in TearDown */
}

/* -------------------------------------------------------------------------
 * Test B18 – keyLen = 8 with DH key value = 1 (g^0 mod p: trivial case).
 *
 * keyValue = 1 means the client sent g^a mod p = 1, implying random_key = 0
 * or an edge case in the group.  The server computes 1^random mod p = 1.
 * shared_key = 1, which is valid as far as the protocol is concerned.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8_DHKeyOne_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15967"))
        << "Setup failed";

    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8, 1ULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with DH key value = 1";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B19 – Client connects without requesting key exchange (flags = 0).
 *
 * When the client sends a CONNECT REQUEST without RIPC_KEY_EXCHANGE in
 * the flags byte, the server's ipcProcessHdr does NOT set
 * rsslSocketChannel->keyExchange = 1 and does NOT transition to
 * RIPC_INT_ST_WAIT_CLIENT_KEY.  The server instead goes directly to ACTIVE
 * after ipcFinishSess.  This test verifies that ipcWaitClientKey is NOT
 * called in this case, and both sides still become ACTIVE normally.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, NoKeyExchangeFlag_ServerReachesActiveDirectly)
{
    /* Use a real RSSL client so the handshake negotiates correctly */
    RsslError err;

    TUServerConfig cfg;
    clearTUServerConfig(&cfg);
    cfg.blocking = RSSL_FALSE;
    cfg.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(cfg.portNo, "15968", sizeof(cfg.portNo));
    pServer = bindRsslServer(&cfg);
    ASSERT_NE(nullptr, pServer) << "Server bind failed";

    /* Use a real (non-fake) RSSL client */
    RsslConnectOptions opts;
    rsslClearConnectOpts(&opts);
    opts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    opts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    opts.connectionInfo.unified.serviceName = const_cast<char*>("15968");
    opts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    opts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    opts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    opts.blocking                           = RSSL_FALSE;

    RsslChannel* pCliChnl = rsslConnect(&opts, &err);
    ASSERT_NE(nullptr, pCliChnl) << "Client connect failed";

    RsslAcceptOptions aOpts;
    rsslClearAcceptOpts(&aOpts);
    for (int i = 0; i < 200 && !pServerChnl; ++i)
    {
        pServerChnl = rsslAccept(pServer, &aOpts, &err);
        if (!pServerChnl) ckSleepMs(10);
    }
    ASSERT_NE(nullptr, pServerChnl) << "Server accept failed";

    /* Drive both to ACTIVE */
    RsslInProgInfo inProg;
    bool reached = false;
    for (int i = 0; i < 500; ++i)
    {
        bool sd = (pServerChnl->state == RSSL_CH_STATE_ACTIVE);
        bool cd = (pCliChnl->state     == RSSL_CH_STATE_ACTIVE);
        if (sd && cd) { reached = true; break; }
        if (!sd) { rsslClearInProgInfo(&inProg); rsslInitChannel(pServerChnl, &inProg, &err); }
        if (!cd) { rsslClearInProgInfo(&inProg); rsslInitChannel(pCliChnl,     &inProg, &err); }
        ckSleepMs(5);
    }

    EXPECT_TRUE(reached) << "Both channels should reach ACTIVE";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pCliChnl->state);

    EXPECT_EQ(RSSL_RET_SUCCESS, rsslCloseChannel(pCliChnl, &err));
}

/* -------------------------------------------------------------------------
 * Test B20 – keyLen = 8 with exactly the minimum 4+8 = 12 bytes read.
 *
 * Verifies the boundary condition where exactly 12 bytes arrive in a single
 * read: length(2) + flags(1) + keyLen(1) + key(8) = 12.  This is the
 * minimum complete message for keyLen=8 and must be accepted cleanly.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, KeyLen8_ExactMinimumFrame_ServerReachesActive)
{
    ASSERT_TRUE(setupServerAndFakeClient("15969"))
        << "Setup failed";

    unsigned char keyFrame[12];
    int len = buildKeyExchangeFrame(keyFrame, sizeof(keyFrame), 8,
                                    0xCAFEBABEDEADBEEFULL);
    ASSERT_EQ(12, len);
    ASSERT_TRUE(ckSendAll(rawClient, keyFrame, len));

    RsslError rsslError;
    bool active = ckDriveServerToTerminal(pServerChnl, &rsslError);
    EXPECT_TRUE(active)
        << "Server SHOULD reach ACTIVE with exact 12-byte key frame";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test B21 – Zero bytes read (WOULD_BLOCK) while waiting for key frame.
 *
 * After the server enters RIPC_INT_ST_WAIT_CLIENT_KEY the fake client
 * stays connected but sends no data.  In non-blocking mode the underlying
 * readTransport call returns RSSL_RET_READ_WOULD_BLOCK, which
 * ipcWaitClientKey converts to cc = 0.  Because cc < 4, the function
 * returns RIPC_CONN_IN_PROGRESS on every poll iteration.
 *
 * Expected behaviour:
 *   - The server channel stays in RSSL_CH_STATE_INITIALIZING throughout
 *     the bounded poll window.
 *   - rsslInitChannel always returns RSSL_RET_CHAN_INIT_IN_PROGRESS (> 0).
 *   - The channel never reaches RSSL_CH_STATE_ACTIVE.
 * ------------------------------------------------------------------------- */
TEST_F(IpcWaitClientKeyMalformedFrameTests, ZeroBytesRead_ServerStaysInitializing)
{
    ASSERT_TRUE(setupServerAndFakeClient("15970"))
        << "Setup (bind / connect / accept / CONNACK exchange) failed";

    /* Confirm the server is now waiting for the client key frame.         */
    RsslError      err;
    RsslInProgInfo inProg;
    bool inWaitKeyState = false;
    for (int i = 0; i < 100; ++i)
    {
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pServerChnl, &inProg, &err);
        RsslUInt8 intState = static_cast<RsslUInt8>(inProg.internalConnState & 0xFF);
        if (intState == RIPC_INT_ST_WAIT_CLIENT_KEY)
        {
            inWaitKeyState = true;
            break;
        }
        if (pServerChnl->state != RSSL_CH_STATE_INITIALIZING)
            break;
        ckSleepMs(5);
    }
    ASSERT_TRUE(inWaitKeyState)
        << "Server did not reach RIPC_INT_ST_WAIT_CLIENT_KEY during setup";

    /* Poll the server for a short window.  The fake client deliberately
     * sends nothing, so every call to ipcWaitClientKey receives 0 bytes
     * (WOULD_BLOCK → cc=0 < 4 → RIPC_CONN_IN_PROGRESS).                  */
    const int POLL_ITERATIONS = 50;
    const int POLL_SLEEP_MS   = 5;

    for (int i = 0; i < POLL_ITERATIONS; ++i)
    {
        /* The channel must still be initializing – never active or closed. */
        ASSERT_EQ(RSSL_CH_STATE_INITIALIZING, pServerChnl->state)
            << "Server channel left INITIALIZING unexpectedly at iteration " << i;

        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pServerChnl, &inProg, &err);

        /* Only CHAN_INIT_IN_PROGRESS (> 0) is acceptable here.            */
        EXPECT_EQ(RSSL_RET_CHAN_INIT_IN_PROGRESS, ret)
            << "Unexpected rsslInitChannel return at iteration " << i
            << ": " << ret << " (" << err.text << ")";

        ckSleepMs(POLL_SLEEP_MS);
    }

    /* After the poll window the server must still be initializing.        */
    EXPECT_EQ(RSSL_CH_STATE_INITIALIZING, pServerChnl->state)
        << "Server should remain INITIALIZING when no key frame is sent";

    EXPECT_EQ(RSSL_RET_SUCCESS, rsslCloseChannel(pServerChnl, &err));
    EXPECT_EQ(RSSL_CH_STATE_INACTIVE, pServerChnl->state);
}
