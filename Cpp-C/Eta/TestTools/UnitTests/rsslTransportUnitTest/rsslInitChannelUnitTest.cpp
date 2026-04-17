/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */
/************************************************************************
 *	rsslInitChannel Unit Tests
 *
 *  Unit testing for the rsslInitChannel() / rsslSocketInitChannel() method
 *  in rsslSocketTransportImpl.c.
 *
 *  Tests cover:
 *    - Successful channel initialization (blocking and non-blocking)
 *    - TCP Socket, WebSocket (RWF and JSON), and Encrypted connections
 *    - Channel state transitions (INITIALIZING -> ACTIVE)
 *    - RSSL_IP_FD_CHANGE detection and new/old socket ID population
 *    - Error cases: NULL arguments, closed channel, uninitialized transport
 *    - Multiple simultaneous initializations (many-channels pattern)
 *    - Channel info validation after successful initialization
 *    - Compression type negotiation during init
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
#include <pthread.h>
#include <signal.h>
#endif

/* -------------------------------------------------------------------------
 * Helpers shared across test cases
 * ------------------------------------------------------------------------- */

static void time_sleep_ms(int ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    if (ms > 0)
    {
        struct timespec ts;
        ts.tv_sec  = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
#endif
}

/* -------------------------------------------------------------------------
 * Thread arguments and thread functions for blocking connect / accept
 * ------------------------------------------------------------------------- */

struct BlockingConnectArg
{
    RsslConnectOptions  connectOpts;        /* filled before thread start   */
    RsslChannel*        pChannel;           /* result – set by thread       */
    RsslError           err;               /* error – set by thread        */

    BlockingConnectArg() : pChannel(nullptr)
    {
        rsslClearConnectOpts(&connectOpts);
        memset(&err, 0, sizeof(err));
    }
};

struct BlockingAcceptArg
{
    RsslServer*         pServer;            /* input  – set before thread start */
    RsslChannel*        pChannel;           /* result – set by thread           */
    RsslError           err;               /* error  – set by thread           */

    BlockingAcceptArg() : pServer(nullptr), pChannel(nullptr)
    {
        memset(&err, 0, sizeof(err));
    }
};

static RSSL_THREAD_DECLARE(blockingConnectThread, pArg)
{
    BlockingConnectArg* arg = reinterpret_cast<BlockingConnectArg*>(pArg);
    arg->pChannel = rsslConnect(&arg->connectOpts, &arg->err);
    return 0;
}

static RSSL_THREAD_DECLARE(blockingAcceptThread, pArg)
{
    BlockingAcceptArg* arg = reinterpret_cast<BlockingAcceptArg*>(pArg);
    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);

    /* Poll until a connection arrives (blocking accept may return
     * EWOULDBLOCK briefly before a client connects).                        */
    const int MAX_ACCEPT_TRIES = 300;
    for (int i = 0; i < MAX_ACCEPT_TRIES && !arg->pChannel; ++i)
    {
        arg->pChannel = rsslAccept(arg->pServer, &acceptOpts, &arg->err);
        if (!arg->pChannel)
            time_sleep_ms(10);
    }
    return 0;
}

/* Establish a connection pair (server + client) that are both
 * in RSSL_CH_STATE_ACTIVE state.  Returns false if setup failed.
 *
 * Blocking mode: rsslConnect() and rsslAccept() are called concurrently on
 * separate threads so that neither side deadlocks waiting for the other.
 * Non-blocking mode: the caller's thread drives both sides.                */
static bool setupActiveChannelPair(
    RsslServer** ppServer,
    RsslChannel** ppServerChnl,
    RsslChannel** ppClientChnl,
    const char* port,
    RsslConnectionTypes connType,
    RsslConnectionTypes encryptedProtocol,
    RsslUInt8 wsProtocolType,
    RsslCompTypes compressType,
    RsslUInt32 compressLevel,
    bool blocking)
{
    RsslError err;
    RsslBool rsslBlocking = (blocking ? RSSL_TRUE : RSSL_FALSE);

    /* --- Bind server --- */
    TUServerConfig serverConfig;
    clearTUServerConfig(&serverConfig);
    serverConfig.blocking         = rsslBlocking;
    serverConfig.connType         = connType;
    serverConfig.compressionType  = compressType;
    serverConfig.compressionLevel = compressLevel;
    strncpy(serverConfig.portNo, port, sizeof(serverConfig.portNo));
    snprintf(serverConfig.wsProtocolList, sizeof(serverConfig.wsProtocolList),
             "rssl.json.v2, rssl.rwf, tr_json2");

    if (connType == RSSL_CONN_TYPE_ENCRYPTED)
    {
        snprintf(serverConfig.serverKey,  sizeof(serverConfig.serverKey),  "%s", getPathServerKey());
        snprintf(serverConfig.serverCert, sizeof(serverConfig.serverCert), "%s", getPathServerCert());
    }

    *ppServer = bindRsslServer(&serverConfig);
    if (!*ppServer)
        return false;

    /* --- Build client config / connect options --- */
    TUClientConfig clientConfig;
    clearTUClientConfig(&clientConfig);
    clientConfig.blocking        = rsslBlocking;
    clientConfig.connType        = connType;
    clientConfig.compressionType = compressType;
    strncpy(clientConfig.portNo, port, sizeof(clientConfig.portNo));

    if (connType == RSSL_CONN_TYPE_ENCRYPTED)
    {
        clientConfig.encryptedProtocol = encryptedProtocol;
        snprintf(clientConfig.openSSLCAStore, sizeof(clientConfig.openSSLCAStore),
                 "%s", getOpenSSLCAStore());
    }
    if (connType == RSSL_CONN_TYPE_WEBSOCKET ||
        (connType == RSSL_CONN_TYPE_ENCRYPTED && encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET))
    {
        if (wsProtocolType == RSSL_JSON_PROTOCOL_TYPE)
            snprintf(clientConfig.wsProtocolList, sizeof(clientConfig.wsProtocolList), "rssl.json.v2");
        else
            snprintf(clientConfig.wsProtocolList, sizeof(clientConfig.wsProtocolList), "rssl.rwf");
    }

    /* Populate connect options – wsProtocolList and openSSLCAStore need to
     * stay alive for the duration of rsslConnect(), so we use the
     * clientConfig buffers directly.                                        */
    RsslConnectOptions connectOpts;
    rsslClearConnectOpts(&connectOpts);
    connectOpts.connectionType                     = connType;
    connectOpts.connectionInfo.unified.address     = (char*)"localhost";
    connectOpts.connectionInfo.unified.serviceName = clientConfig.portNo;
    connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    connectOpts.tcp_nodelay                        = RSSL_TRUE;
    connectOpts.blocking                           = rsslBlocking;
    connectOpts.compressionType                    = compressType;

    if (connType == RSSL_CONN_TYPE_ENCRYPTED)
    {
        connectOpts.encryptionOpts.encryptionProtocolFlags = clientConfig.encryptionProtocolFlags;
        connectOpts.encryptionOpts.encryptedProtocol       = encryptedProtocol;
        connectOpts.encryptionOpts.openSSLCAStore           = clientConfig.openSSLCAStore;
    }
    if (connType == RSSL_CONN_TYPE_WEBSOCKET ||
        (connType == RSSL_CONN_TYPE_ENCRYPTED && encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET))
    {
        connectOpts.wsOpts.protocols = clientConfig.wsProtocolList;
    }

    if (blocking)
    {
        /* Blocking mode: rsslConnect() and rsslAccept() both block until the
         * handshake completes, so they must run on separate threads.        */
        BlockingConnectArg connectArg;
        connectArg.connectOpts = connectOpts;

        BlockingAcceptArg acceptArg;
        acceptArg.pServer = *ppServer;

        RsslThreadId connectTid, acceptTid;
        RSSL_THREAD_START(&acceptTid,  blockingAcceptThread,  &acceptArg);
        RSSL_THREAD_START(&connectTid, blockingConnectThread, &connectArg);

        RSSL_THREAD_JOIN(connectTid);
        RSSL_THREAD_JOIN(acceptTid);

        RSSL_THREAD_DETACH(&connectTid);
        RSSL_THREAD_DETACH(&acceptTid);

        *ppClientChnl = connectArg.pChannel;
        *ppServerChnl = acceptArg.pChannel;

        if (!*ppClientChnl || !*ppServerChnl)
            return false;
    }
    else
    {
        /* Non-blocking mode: rsslConnect() returns immediately; poll for
         * an incoming connection with rsslAccept().                         */
        *ppClientChnl = rsslConnect(&connectOpts, &err);
        if (!*ppClientChnl)
            return false;

        RsslAcceptOptions acceptOpts;
        rsslClearAcceptOpts(&acceptOpts);
        const int MAX_ACCEPT_TRIES = 200;
        for (int i = 0; i < MAX_ACCEPT_TRIES && !*ppServerChnl; ++i)
        {
            *ppServerChnl = rsslAccept(*ppServer, &acceptOpts, &err);
            if (!*ppServerChnl)
                time_sleep_ms(10);
        }
        if (!*ppServerChnl)
            return false;

        /* Drive non-blocking init handshake for both sides */
        const int MAX_INIT_TRIES = 500;
        RsslInProgInfo inProg;

        for (int i = 0; i < MAX_INIT_TRIES; ++i)
        {
            bool serverDone = ((*ppServerChnl)->state == RSSL_CH_STATE_ACTIVE);
            bool clientDone = ((*ppClientChnl)->state == RSSL_CH_STATE_ACTIVE);
            if (serverDone && clientDone)
                break;

            if (!serverDone)
            {
                rsslClearInProgInfo(&inProg);
                rsslInitChannel(*ppServerChnl, &inProg, &err);
            }
            if (!clientDone)
            {
                rsslClearInProgInfo(&inProg);
                rsslInitChannel(*ppClientChnl, &inProg, &err);
            }
            time_sleep_ms(5);
        }
    }

    return ((*ppServerChnl)->state == RSSL_CH_STATE_ACTIVE &&
            (*ppClientChnl)->state == RSSL_CH_STATE_ACTIVE);
}

/* =========================================================================
 * Test Fixture: RsslInitChannelTests
 * ========================================================================= */

class RsslInitChannelTests : public ::testing::Test
{
protected:
    RsslServer*  pServer      = nullptr;
    RsslChannel* pServerChnl  = nullptr;
    RsslChannel* pClientChnl  = nullptr;

    virtual void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
        pServer     = nullptr;
        pServerChnl = nullptr;
        pClientChnl = nullptr;
    }

    virtual void TearDown() override
    {
        RsslError err;
        if (pClientChnl)  { rsslCloseChannel(pClientChnl,  &err); pClientChnl  = nullptr; }
        if (pServerChnl)  { rsslCloseChannel(pServerChnl,  &err); pServerChnl  = nullptr; }
        if (pServer)      { rsslCloseServer(pServer,        &err); pServer      = nullptr; }
        rsslUninitialize();
    }

    /* Helper: initiate channels and loop rsslInitChannel until both sides are
     * active (non-blocking mode).  Returns true on success.                  */
    bool driveInitToActive(RsslChannel* cSrv, RsslChannel* cCli,
                           int maxTries = 500, int sleepMs = 5)
    {
        RsslError err;
        RsslInProgInfo inProg;
        for (int i = 0; i < maxTries; ++i)
        {
            bool srvActive = (cSrv->state == RSSL_CH_STATE_ACTIVE);
            bool cliActive = (cCli->state == RSSL_CH_STATE_ACTIVE);
            if (srvActive && cliActive)
                return true;

            if (!srvActive)
            {
                rsslClearInProgInfo(&inProg);
                int ret = rsslInitChannel(cSrv, &inProg, &err);
                if (ret == RSSL_RET_FAILURE)
                {
                    std::cout << err.text << std::endl;
                }
            }
            if (!cliActive)
            {
                rsslClearInProgInfo(&inProg);
                int ret = rsslInitChannel(cCli, &inProg, &err);
                if (ret == RSSL_RET_FAILURE)
                {
                    std::cout << err.text << std::endl;
                }
            }
            time_sleep_ms(sleepMs);
        }
        return (cSrv->state == RSSL_CH_STATE_ACTIVE &&
                cCli->state == RSSL_CH_STATE_ACTIVE);
    }
};

/* Test: rsslInitChannel on a CLOSED channel must return RSSL_RET_FAILURE */
TEST_F(RsslInitChannelTests, ClosedChannelReturnsFailure)
{
    RsslError err;
    RsslInProgInfo inProg;
    rsslClearInProgInfo(&inProg);

    RsslChannel dummyChnl;
    memset(&dummyChnl, 0, sizeof(dummyChnl));
    dummyChnl.state = RSSL_CH_STATE_CLOSED;

    RsslRet ret = rsslInitChannel(&dummyChnl, &inProg, &err);

    EXPECT_EQ(RSSL_RET_FAILURE, ret);
    EXPECT_NE(nullptr, strstr(err.text, "rsslInitChannel() Error: 0007 Channel has been closed due to prior rejection or failure, cannot continue to initialize connection."));
}

/* Test: rsslInitChannel before rsslInitialize must return
 * RSSL_RET_INIT_NOT_INITIALIZED                                             */
TEST_F(RsslInitChannelTests, NotInitializedReturnsError)
{
    /* Tear down initialisation done in SetUp() */
    rsslUninitialize();

    RsslError err;
    RsslInProgInfo inProg;
    rsslClearInProgInfo(&inProg);
    RsslChannel dummyChnl;
    memset(&dummyChnl, 0, sizeof(dummyChnl));

    RsslRet ret = rsslInitChannel(&dummyChnl, &inProg, &err);
    EXPECT_NE(nullptr, strstr(err.text, "rsslInitChannel() Error: 0001 RSSL not initialized."));

    EXPECT_EQ(RSSL_RET_INIT_NOT_INITIALIZED, ret);

    /* Re-initialize so TearDown() succeeds */
    rsslInitialize(RSSL_LOCK_GLOBAL, &err);
}

/* =========================================================================
 * Successful initialization – non-blocking TCP socket
 * ========================================================================= */

/* Test: Non-blocking TCP socket channel handshake driven by repeated calls
 * to rsslInitChannel transitions both ends to RSSL_CH_STATE_ACTIVE.         */
TEST_F(RsslInitChannelTests, NonBlockingTCPChannelReachesActiveState)
{
    RsslError err;

    /* --- Bind server (non-blocking) --- */
    TUServerConfig serverConfig;
    clearTUServerConfig(&serverConfig);
    serverConfig.blocking = RSSL_FALSE;
    serverConfig.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(serverConfig.portNo, "15601", sizeof(serverConfig.portNo));
    pServer = bindRsslServer(&serverConfig);
    ASSERT_NE(nullptr, pServer) << "Server creation failed";

    /* --- rsslConnect (non-blocking) --- */
    RsslConnectOptions connectOpts;
    rsslClearConnectOpts(&connectOpts);
    connectOpts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    connectOpts.connectionInfo.unified.address     = (char*)"localhost";
    connectOpts.connectionInfo.unified.serviceName = (char*)"15601";
    connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    connectOpts.tcp_nodelay                        = RSSL_TRUE;
    connectOpts.blocking                           = RSSL_FALSE;

    pClientChnl = rsslConnect(&connectOpts, &err);
    ASSERT_NE(nullptr, pClientChnl) << "rsslConnect failed: " << err.text;

    /* The channel is at least in INITIALIZING after rsslConnect */
    EXPECT_NE(RSSL_CH_STATE_CLOSED, pClientChnl->state);

    /* Accept */
    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    for (int i = 0; i < 200 && !pServerChnl; ++i)
    {
        pServerChnl = rsslAccept(pServer, &acceptOpts, &err);
        if (!pServerChnl)
            time_sleep_ms(10);
    }
    ASSERT_NE(nullptr, pServerChnl) << "rsslAccept failed";

    /* Drive initialization */
    bool reachedActive = driveInitToActive(pServerChnl, pClientChnl);
    EXPECT_TRUE(reachedActive) << "Channels did not reach ACTIVE state";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* Test: During non-blocking initialization the inProg.flags may be set to
 * RSSL_IP_FD_CHANGE, inProg.oldSocket and inProg.newSocket are populated.
 * We verify at minimum that when the channel ends up active the final
 * call returns RSSL_RET_SUCCESS and the channel socketId is valid.          */
TEST_F(RsslInitChannelTests, NonBlockingInitProgressFlagsObserved)
{
    RsslError err;

    TUServerConfig serverConfig;
    clearTUServerConfig(&serverConfig);
    serverConfig.blocking = RSSL_FALSE;
    serverConfig.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(serverConfig.portNo, "15603", sizeof(serverConfig.portNo));
    pServer = bindRsslServer(&serverConfig);
    ASSERT_NE(nullptr, pServer);

    RsslConnectOptions connectOpts;
    rsslClearConnectOpts(&connectOpts);
    connectOpts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    connectOpts.connectionInfo.unified.address     = (char*)"localhost";
    connectOpts.connectionInfo.unified.serviceName = (char*)"15603";
    connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    connectOpts.blocking                           = RSSL_FALSE;

    pClientChnl = rsslConnect(&connectOpts, &err);
    ASSERT_NE(nullptr, pClientChnl);

    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    for (int i = 0; i < 200 && !pServerChnl; ++i)
    {
        pServerChnl = rsslAccept(pServer, &acceptOpts, &err);
        if (!pServerChnl) time_sleep_ms(10);
    }
    ASSERT_NE(nullptr, pServerChnl);

    /* Drive handshake, recording any FD-change events */
    bool fdChangeObservedSrv = false;
    bool fdChangeObservedCli = false;
    RsslInProgInfo inProg;

    for (int i = 0; i < 500; ++i)
    {
        bool srvDone = (pServerChnl->state == RSSL_CH_STATE_ACTIVE);
        bool cliDone = (pClientChnl->state == RSSL_CH_STATE_ACTIVE);
        if (srvDone && cliDone) break;

        if (!srvDone)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pServerChnl, &inProg, &err);
            if (ret >= RSSL_RET_SUCCESS && (inProg.flags & RSSL_IP_FD_CHANGE))
            {
                fdChangeObservedSrv = true;
                EXPECT_NE(RSSL_INVALID_SOCKET, inProg.newSocket);
                EXPECT_NE(RSSL_INVALID_SOCKET, inProg.oldSocket);
            }
        }
        if (!cliDone)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pClientChnl, &inProg, &err);
            if (ret >= RSSL_RET_SUCCESS && (inProg.flags & RSSL_IP_FD_CHANGE))
            {
                fdChangeObservedCli = true;
                EXPECT_NE(RSSL_INVALID_SOCKET, inProg.newSocket);
                EXPECT_NE(RSSL_INVALID_SOCKET, inProg.oldSocket);
            }
        }
        time_sleep_ms(5);
    }

    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
    /* socketId must be valid once active */
    EXPECT_NE(RSSL_INVALID_SOCKET, pServerChnl->socketId);
    EXPECT_NE(RSSL_INVALID_SOCKET, pClientChnl->socketId);
    /* Suppress unused-variable warnings if FD change did not occur */
    (void)fdChangeObservedSrv;
    (void)fdChangeObservedCli;
}

/* =========================================================================
 * Channel info validation after successful initialization
 * ========================================================================= */

/* Test: After a successful non-blocking TCP channel initialization,
 * rsslGetChannelInfo succeeds and reports sensible maxFragmentSize /
 * maxOutputBuffers values.                                                   */
TEST_F(RsslInitChannelTests, ChannelInfoValidAfterInit)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15604",
        RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_SOCKET, 0,
        RSSL_COMP_NONE, 0, false);
    ASSERT_TRUE(ok) << "Channel pair setup failed";

    RsslError err;
    RsslChannelInfo info;
    memset(&info, 0, sizeof(info));

    RsslRet ret = rsslGetChannelInfo(pClientChnl, &info, &err);
    ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "rsslGetChannelInfo failed: " << err.text;

    EXPECT_GT(info.maxFragmentSize, 0U);
    EXPECT_GT(info.maxOutputBuffers, 0U);
    EXPECT_EQ(pClientChnl->state, RSSL_CH_STATE_ACTIVE);
}

/* =========================================================================
 * WebSocket + RWF protocol initialization
 * ========================================================================= */

/* Test: Non-blocking WebSocket (RWF protocol) channel initialization
 * drives both sides to RSSL_CH_STATE_ACTIVE.                                */
TEST_F(RsslInitChannelTests, NonBlockingWebSocketRWFReachesActiveState)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15605",
        RSSL_CONN_TYPE_WEBSOCKET, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE,
        RSSL_COMP_NONE, 0, false);

    ASSERT_TRUE(ok) << "WebSocket/RWF channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
    EXPECT_EQ(RSSL_CONN_TYPE_WEBSOCKET, pClientChnl->connectionType);
}

/* =========================================================================
 * WebSocket + JSON protocol initialization
 * ========================================================================= */

/* Test: Non-blocking WebSocket (JSON protocol) channel initialization
 * drives both sides to RSSL_CH_STATE_ACTIVE.                                */
TEST_F(RsslInitChannelTests, NonBlockingWebSocketJSONReachesActiveState)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15606",
        RSSL_CONN_TYPE_WEBSOCKET, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE,
        RSSL_COMP_NONE, 0, false);

    ASSERT_TRUE(ok) << "WebSocket/JSON channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
    EXPECT_EQ(RSSL_CONN_TYPE_WEBSOCKET, pClientChnl->connectionType);
}

/* =========================================================================
 * Blocking WebSocket + RWF protocol initialization
 * ========================================================================= */

/* Test: Blocking WebSocket (RWF) channel initialization completes with both
 * channels in RSSL_CH_STATE_ACTIVE.                                         */
TEST_F(RsslInitChannelTests, BlockingWebSocketRWFReachesActiveState)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15607",
        RSSL_CONN_TYPE_WEBSOCKET, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE,
        RSSL_COMP_NONE, 0, true);

    ASSERT_TRUE(ok) << "Blocking WebSocket/RWF channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Encrypted (SSL) TCP channel initialization
 * ========================================================================= */

/* Test: Non-blocking encrypted TCP channel initialization reaches
 * RSSL_CH_STATE_ACTIVE when certificate files are available.               */
TEST_F(RsslInitChannelTests, NonBlockingEncryptedTCPReachesActiveState)
{
    if (!checkCertificateFiles() || !checkClientCertificateFiles())
    {
        GTEST_SKIP() << "Certificate files not found – skipping encrypted test";
    }

    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15608",
        RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_SOCKET, 0,
        RSSL_COMP_NONE, 0, false);

    ASSERT_TRUE(ok) << "Encrypted TCP channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
    EXPECT_EQ(RSSL_CONN_TYPE_ENCRYPTED, pClientChnl->connectionType);
}

/* =========================================================================
 * Encrypted WebSocket + RWF channel initialization
 * ========================================================================= */

/* Test: Non-blocking encrypted WebSocket (RWF) channel initialization
 * reaches RSSL_CH_STATE_ACTIVE when certificate files are available.       */
TEST_F(RsslInitChannelTests, NonBlockingEncryptedWebSocketRWFReachesActiveState)
{
    if (!checkCertificateFiles() || !checkClientCertificateFiles())
    {
        GTEST_SKIP() << "Certificate files not found – skipping encrypted WebSocket test";
    }

    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15609",
        RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE,
        RSSL_COMP_NONE, 0, false);

    ASSERT_TRUE(ok) << "Encrypted WebSocket/RWF channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Encrypted WebSocket + JSON channel initialization
 * ========================================================================= */

/* Test: Non-blocking encrypted WebSocket (JSON) channel initialization
 * reaches RSSL_CH_STATE_ACTIVE when certificate files are available.       */
TEST_F(RsslInitChannelTests, NonBlockingEncryptedWebSocketJSONReachesActiveState)
{
    if (!checkCertificateFiles() || !checkClientCertificateFiles())
    {
        GTEST_SKIP() << "Certificate files not found – skipping encrypted WebSocket JSON test";
    }

    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15610",
        RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE,
        RSSL_COMP_NONE, 0, false);

    ASSERT_TRUE(ok) << "Encrypted WebSocket/JSON channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Compression negotiation during initialization
 * ========================================================================= */

/* Test: Non-blocking TCP channel with ZLIB compression negotiated during
 * the rsslInitChannel handshake results in an ACTIVE channel.              */
TEST_F(RsslInitChannelTests, NonBlockingTCPWithZlibCompressionReachesActiveState)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15611",
        RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_SOCKET, 0,
        RSSL_COMP_ZLIB, 0, false);

    ASSERT_TRUE(ok) << "TCP + ZLIB channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* Test: Non-blocking TCP channel with LZ4 compression results in an ACTIVE
 * channel after the rsslInitChannel handshake.                              */
TEST_F(RsslInitChannelTests, NonBlockingTCPWithLZ4CompressionReachesActiveState)
{
    bool ok = setupActiveChannelPair(
        &pServer, &pServerChnl, &pClientChnl,
        "15612",
        RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_SOCKET, 0,
        RSSL_COMP_LZ4, 0, false);

    ASSERT_TRUE(ok) << "TCP + LZ4 channel pair setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Multiple sequential initializations on the same port
 * ========================================================================= */

/* Test: Opening and closing N channel pairs on the same port in sequence
 * all succeed, verifying that rsslInitChannel works correctly for repeated
 * connection cycles.                                                         */
TEST_F(RsslInitChannelTests, SequentialTCPChannelInitCycles)
{
    const int CYCLES = 5;

    for (int cycle = 0; cycle < CYCLES; ++cycle)
    {
        RsslError err;

        TUServerConfig serverConfig;
        clearTUServerConfig(&serverConfig);
        serverConfig.blocking = RSSL_FALSE;
        serverConfig.connType = RSSL_CONN_TYPE_SOCKET;
        strncpy(serverConfig.portNo, "15613", sizeof(serverConfig.portNo));
        RsslServer* srv = bindRsslServer(&serverConfig);
        ASSERT_NE(nullptr, srv) << "Cycle " << cycle << ": bind failed";

        RsslConnectOptions connectOpts;
        rsslClearConnectOpts(&connectOpts);
        connectOpts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
        connectOpts.connectionInfo.unified.address     = (char*)"localhost";
        connectOpts.connectionInfo.unified.serviceName = (char*)"15613";
        connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
        connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
        connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
        connectOpts.blocking                           = RSSL_FALSE;

        RsslChannel* cli = rsslConnect(&connectOpts, &err);
        ASSERT_NE(nullptr, cli) << "Cycle " << cycle << ": connect failed";

        RsslChannel* srvChnl = nullptr;
        RsslAcceptOptions acceptOpts;
        rsslClearAcceptOpts(&acceptOpts);
        for (int i = 0; i < 200 && !srvChnl; ++i)
        {
            srvChnl = rsslAccept(srv, &acceptOpts, &err);
            if (!srvChnl) time_sleep_ms(10);
        }
        ASSERT_NE(nullptr, srvChnl) << "Cycle " << cycle << ": accept failed";

        /* Drive to active */
        bool reached = driveInitToActive(srvChnl, cli);
        EXPECT_TRUE(reached) << "Cycle " << cycle << ": did not reach ACTIVE";
        EXPECT_EQ(RSSL_CH_STATE_ACTIVE, srvChnl->state);
        EXPECT_EQ(RSSL_CH_STATE_ACTIVE, cli->state);

        rsslCloseChannel(cli,     &err);
        rsslCloseChannel(srvChnl, &err);
        rsslCloseServer(srv,      &err);

        time_sleep_ms(20);
    }
}

/* =========================================================================
 * Concurrent initialization – two independent channel pairs
 * ========================================================================= */

/* Thread argument for concurrent init test */
struct ConcurrentInitArg
{
    const char*       port;
    std::atomic<bool> success;
    std::atomic<bool> done;

    ConcurrentInitArg() : port(nullptr), success(false), done(false) {}
};

static RSSL_THREAD_DECLARE(concurrentInitThread, pArg)
{
    ConcurrentInitArg* arg = reinterpret_cast<ConcurrentInitArg*>(pArg);
    RsslError err;

    TUServerConfig serverConfig;
    clearTUServerConfig(&serverConfig);
    serverConfig.blocking = RSSL_FALSE;
    serverConfig.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(serverConfig.portNo, arg->port, sizeof(serverConfig.portNo));
    RsslServer* srv = bindRsslServer(&serverConfig);
    if (!srv) { arg->done = true; return 0; }

    RsslConnectOptions connectOpts;
    rsslClearConnectOpts(&connectOpts);
    connectOpts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    connectOpts.connectionInfo.unified.address     = (char*)"localhost";
    connectOpts.connectionInfo.unified.serviceName = (char*)arg->port;
    connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    connectOpts.blocking                           = RSSL_FALSE;

    RsslChannel* cli = rsslConnect(&connectOpts, &err);
    if (!cli) { rsslCloseServer(srv, &err); arg->done = true; return 0; }

    RsslChannel* srvChnl = nullptr;
    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    for (int i = 0; i < 200 && !srvChnl; ++i)
    {
        srvChnl = rsslAccept(srv, &acceptOpts, &err);
        if (!srvChnl) time_sleep_ms(10);
    }

    bool reached = false;
    if (srvChnl)
    {
        RsslInProgInfo inProg;
        for (int i = 0; i < 500; ++i)
        {
            bool sd = (srvChnl->state == RSSL_CH_STATE_ACTIVE);
            bool cd = (cli->state     == RSSL_CH_STATE_ACTIVE);
            if (sd && cd) { reached = true; break; }
            if (!sd) { rsslClearInProgInfo(&inProg); rsslInitChannel(srvChnl, &inProg, &err); }
            if (!cd) { rsslClearInProgInfo(&inProg); rsslInitChannel(cli,     &inProg, &err); }
            time_sleep_ms(5);
        }
    }

    arg->success = reached;

    rsslCloseChannel(cli,     &err);
    if (srvChnl) rsslCloseChannel(srvChnl, &err);
    rsslCloseServer(srv, &err);
    arg->done = true;
    return 0;
}

/* Test: Two independent channel pairs are initialized concurrently.  Both
 * must reach RSSL_CH_STATE_ACTIVE.                                          */
TEST_F(RsslInitChannelTests, ConcurrentChannelInitializationsSucceed)
{
    ConcurrentInitArg arg1, arg2;
    arg1.port = "15614";
    arg2.port = "15615";

    RsslThreadId tid1, tid2;
    RSSL_THREAD_START(&tid1, concurrentInitThread, &arg1);
    RSSL_THREAD_START(&tid2, concurrentInitThread, &arg2);

    RSSL_THREAD_JOIN(tid1);
    RSSL_THREAD_JOIN(tid2);

    EXPECT_TRUE(arg1.success.load()) << "Thread 1 channel pair did not reach ACTIVE";
    EXPECT_TRUE(arg2.success.load()) << "Thread 2 channel pair did not reach ACTIVE";
}

/* =========================================================================
 * Return-code contract for in-progress channel
 * ========================================================================= */

/* Test: While the non-blocking handshake is still in progress, rsslInitChannel
 * returns either RSSL_RET_CHAN_INIT_IN_PROGRESS (>0) or RSSL_RET_SUCCESS (0).
 * It must never return a negative value before the first error.             */
TEST_F(RsslInitChannelTests, NonBlockingInitReturnCodesAreNonNegative)
{
    RsslError err;

    TUServerConfig serverConfig;
    clearTUServerConfig(&serverConfig);
    serverConfig.blocking = RSSL_FALSE;
    serverConfig.connType = RSSL_CONN_TYPE_SOCKET;
    strncpy(serverConfig.portNo, "15616", sizeof(serverConfig.portNo));
    pServer = bindRsslServer(&serverConfig);
    ASSERT_NE(nullptr, pServer);

    RsslConnectOptions connectOpts;
    rsslClearConnectOpts(&connectOpts);
    connectOpts.connectionType                     = RSSL_CONN_TYPE_SOCKET;
    connectOpts.connectionInfo.unified.address     = (char*)"localhost";
    connectOpts.connectionInfo.unified.serviceName = (char*)"15616";
    connectOpts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    connectOpts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    connectOpts.protocolType                       = RSSL_RWF_PROTOCOL_TYPE;
    connectOpts.blocking                           = RSSL_FALSE;

    pClientChnl = rsslConnect(&connectOpts, &err);
    ASSERT_NE(nullptr, pClientChnl);

    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    for (int i = 0; i < 200 && !pServerChnl; ++i)
    {
        pServerChnl = rsslAccept(pServer, &acceptOpts, &err);
        if (!pServerChnl) time_sleep_ms(10);
    }
    ASSERT_NE(nullptr, pServerChnl);

    RsslInProgInfo inProg;
    for (int i = 0; i < 500; ++i)
    {
        bool srvDone = (pServerChnl->state == RSSL_CH_STATE_ACTIVE);
        bool cliDone = (pClientChnl->state == RSSL_CH_STATE_ACTIVE);
        if (srvDone && cliDone) break;

        if (!srvDone)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pServerChnl, &inProg, &err);
            EXPECT_GE(ret, RSSL_RET_SUCCESS)
                << "Server rsslInitChannel returned negative: " << ret
                << " err: " << err.text;
        }
        if (!cliDone)
        {
            rsslClearInProgInfo(&inProg);
            RsslRet ret = rsslInitChannel(pClientChnl, &inProg, &err);
            EXPECT_GE(ret, RSSL_RET_SUCCESS)
                << "Client rsslInitChannel returned negative: " << ret
                << " err: " << err.text;
        }
        time_sleep_ms(5);
    }

    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pClientChnl->state);
}

/* =========================================================================
 * Parametrised fixture: connection-type matrix
 * ========================================================================= */

struct InitChannelParams
{
    RsslConnectionTypes connType;
    RsslConnectionTypes encryptedProtocol;
    RsslUInt8           wsProtocol;
    const char*         port;
    const char*         description;
};

class RsslInitChannelParamTests
    : public ::testing::TestWithParam<InitChannelParams>
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pSrvChnl   = nullptr;
    RsslChannel* pCliChnl   = nullptr;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
    }

    void TearDown() override
    {
        RsslError err;
        if (pCliChnl) { rsslCloseChannel(pCliChnl, &err); pCliChnl = nullptr; }
        if (pSrvChnl) { rsslCloseChannel(pSrvChnl, &err); pSrvChnl = nullptr; }
        if (pServer)  { rsslCloseServer(pServer,   &err); pServer  = nullptr; }
        rsslUninitialize();
    }
};

TEST_P(RsslInitChannelParamTests, ChannelReachesActiveState)
{
    const InitChannelParams& p = GetParam();

    if (p.connType == RSSL_CONN_TYPE_ENCRYPTED)
    {
        if (!checkCertificateFiles() || !checkClientCertificateFiles())
            GTEST_SKIP() << "Skipping encrypted test: certificate files not found";
    }

    bool ok = setupActiveChannelPair(
        &pServer, &pSrvChnl, &pCliChnl,
        p.port,
        p.connType, p.encryptedProtocol, p.wsProtocol,
        RSSL_COMP_NONE, 0,
        false  /* non-blocking */);

    ASSERT_TRUE(ok)  << p.description << ": setup failed";
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pSrvChnl->state) << p.description;
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pCliChnl->state) << p.description;
}

INSTANTIATE_TEST_SUITE_P(
    ConnectionTypes,
    RsslInitChannelParamTests,
    ::testing::Values(
        InitChannelParams{ RSSL_CONN_TYPE_SOCKET,    RSSL_CONN_TYPE_SOCKET,    0,                        "15620", "TCP Socket" },
        InitChannelParams{ RSSL_CONN_TYPE_WEBSOCKET, RSSL_CONN_TYPE_SOCKET,    RSSL_RWF_PROTOCOL_TYPE,   "15621", "WebSocket RWF" },
        InitChannelParams{ RSSL_CONN_TYPE_WEBSOCKET, RSSL_CONN_TYPE_SOCKET,    RSSL_JSON_PROTOCOL_TYPE,  "15622", "WebSocket JSON" },
        InitChannelParams{ RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_SOCKET,    0,                        "15623", "Encrypted TCP" },
        InitChannelParams{ RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE,   "15624", "Encrypted WebSocket RWF" },
        InitChannelParams{ RSSL_CONN_TYPE_ENCRYPTED, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE,  "15625", "Encrypted WebSocket JSON" }
    )
);

/* =========================================================================
 * Invalid-message injection helpers
 *
 * These helpers open a plain TCP socket (bypassing rsslConnect) and write
 * crafted bytes directly onto the wire so the server's rsslInitChannel /
 * ipcReadHdr / ipcProcessHdr error-handling paths can be exercised.
 *
 * Platform note: the file already includes <winsock2.h> / <sys/socket.h>
 * via the conditional block at the top, so we just alias the type and the
 * close call.
 * ========================================================================= */

#if defined(_WIN32)
typedef SOCKET  RawSocket;
static const RawSocket kInvalidRawSocket = INVALID_SOCKET;
static void rawSocketClose(RawSocket s) { closesocket(s); }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int     RawSocket;
static const RawSocket kInvalidRawSocket = -1;
static void rawSocketClose(RawSocket s) { close(s); }
#endif

/* Open a plain (non-RSSL) TCP connection to localhost:port.
 * Returns kInvalidRawSocket on failure.                                     */
static RawSocket rawTcpConnect(const char* port)
{
    unsigned short portNum = static_cast<unsigned short>(atoi(port));

    RawSocket s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == kInvalidRawSocket)
        return kInvalidRawSocket;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(portNum);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        rawSocketClose(s);
        return kInvalidRawSocket;
    }
    return s;
}

/* Send all bytes in buf to the raw socket. Returns false on error.         */
static bool rawSendAll(RawSocket s, const unsigned char* buf, int len)
{
    int sent = 0;
    while (sent < len)
    {
#if defined(_WIN32)
        int n = send(s, reinterpret_cast<const char*>(buf + sent), len - sent, 0);
        if (n == SOCKET_ERROR)
            return false;
#else
        int n = static_cast<int>(send(s, buf + sent, len - sent, 0));
        if (n <= 0)
            return false;
#endif
        sent += n;
    }
    return true;
}

/* Poll rsslInitChannel on pServerChnl until it returns RSSL_RET_FAILURE or
 * the channel state transitions to CLOSED/INACTIVE.
 * Returns true when the server channel definitively rejected the connection
 * (state != RSSL_CH_STATE_ACTIVE after maxTries attempts).                 */
static bool driveServerUntilFinished(RsslChannel* pServerChnl, RsslError *pError,
                                     int maxTries = 300,
                                     int sleepMs  = 5)
{
    RsslInProgInfo inProg;

    for (int i = 0; i < maxTries; ++i)
    {
        if (pServerChnl->state == RSSL_CH_STATE_CLOSED ||
            pServerChnl->state == RSSL_CH_STATE_INACTIVE)
            return true;

        if (pServerChnl->state == RSSL_CH_STATE_ACTIVE)
            return false;   /* unexpected success */

        rsslClearInProgInfo(&inProg);
        RsslRet ret = rsslInitChannel(pServerChnl, &inProg, pError);
        if (ret == RSSL_RET_FAILURE || ret == RSSL_RET_CHAN_INIT_REFUSED)
            return true;

        time_sleep_ms(sleepMs);
    }

    /* Final state check */
    return (pServerChnl->state != RSSL_CH_STATE_ACTIVE);
}

/* -------------------------------------------------------------------------
 * Test Fixture: RsslInitChannelInvalidMsgTests
 *
 * Each test:
 *  1. Binds a non-blocking RSSL server.
 *  2. Opens a raw TCP socket to the server (simulating the client).
 *  3. Writes invalid/malformed bytes so ipcProcessHdr rejects the message.
 *  4. Calls rsslAccept() to obtain the server-side RsslChannel.
 *  5. Drives rsslInitChannel() and verifies the server channel does NOT
 *     reach RSSL_CH_STATE_ACTIVE (it must reject with FAILURE).
 * ========================================================================= */

class RsslInitChannelInvalidMsgTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    RawSocket    rawClient   = kInvalidRawSocket;

    void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);
    }

    void TearDown() override
    {
        if (rawClient != kInvalidRawSocket)
        {
            rawSocketClose(rawClient);
            rawClient = kInvalidRawSocket;
        }
        RsslError err;
        if (pServerChnl) { rsslCloseChannel(pServerChnl, &err); pServerChnl = nullptr; }
        if (pServer)      { rsslCloseServer(pServer,      &err); pServer     = nullptr; }
        rsslUninitialize();
    }

    /* Bind a non-blocking TCP server on port and store it in pServer.      */
    bool bindServer(const char* port)
    {
        TUServerConfig cfg;
        clearTUServerConfig(&cfg);
        cfg.blocking  = RSSL_FALSE;
        cfg.connType  = RSSL_CONN_TYPE_SOCKET;
        strncpy(cfg.portNo, port, sizeof(cfg.portNo));
        pServer = bindRsslServer(&cfg);
        return pServer != nullptr;
    }

    /* Connect the raw client socket to the server and poll rsslAccept until
     * the server-side channel appears.  Returns true on success.           */
    bool connectRawAndAccept(const char* port)
    {
        rawClient = rawTcpConnect(port);
        if (rawClient == kInvalidRawSocket)
            return false;

        RsslError       err;
        RsslAcceptOptions acceptOpts;
        rsslClearAcceptOpts(&acceptOpts);
        for (int i = 0; i < 200 && !pServerChnl; ++i)
        {
            pServerChnl = rsslAccept(pServer, &acceptOpts, &err);
            if (!pServerChnl)
                time_sleep_ms(10);
        }
        return pServerChnl != nullptr;
    }
};

/* -------------------------------------------------------------------------
 * Helper: build a syntactically correct RIPC v14 connect-request header.
 * The server uses ipcProcessHdr to parse this so every field must be valid.
 * Returns the number of bytes written into buf (buf must be >= 64 bytes).
 * ------------------------------------------------------------------------- */
static int buildValidRipcHeader(unsigned char* buf, int bufLen, unsigned int ripcVersion = CONN_VERSION_14)
{
    /* Minimum v10 connection header: 17 bytes (V10_MIN_CONN_HDR).
     * We build a minimal version-13 frame with:
     *   [0..1]  total length (u16 big-endian)
     *   [2]     opCode = 0
     *   [3..6]  version = CONN_VERSION_14 (0x0D in the wire format 0x0017)
     *   [7]     flags = 0 (no key exchange)
     *   [8]     hdrSize (the fixed header portion length)
     *   [9]     compBitmapSize = 0
     *   [10]    pingTimeout = 60
     *   [11]    rsslFlags = 0
     *   [12]    protocolType = RIPC_RWF_PROTOCOL_TYPE (0)
     *   [13]    majorVersion = 14
     *   [14]    minorVersion = 1
     *   [15]    hostnameLen = 0
     *   [16]    addrLen = 0
     *   [17]    componentVersionLen (total) = 2
     *   [18]    componentStringLen = 0
     *
     * For v14, hdrSize = V10_MIN_CONN_HDR + hostnameLen + addrLen + 2
     *                  = 17 + 0 + 0 + 1 (protocolType byte) + 2
     * Total message length = hdrSize + componentVersionLen + 1
     *                      = 20 + 2 + 1 = wait — let's follow the exact
     * formula from ipcProcessHdr:
     *   hdrSize + compVerLen + 1 == totalMsgLength
     * We set hdrSize=20, compVerLen=2 so totalMsgLength=23 which is what
     * we put in bytes [0..1].                                              */

    if (bufLen < 24)
        return 0;

    memset(buf, 0, 24);

    /* Total length = 17 (big-endian u16) */
    buf[0] = 0x00;
    buf[1] = 23;

    /* opCode = 0 */
    buf[2] = 0x00;

    /* version (big-endian u32) */
    buf[3] = static_cast<unsigned char>((ripcVersion >> 24) & 0xFF);
    buf[4] = static_cast<unsigned char>((ripcVersion >> 16) & 0xFF);
    buf[5] = static_cast<unsigned char>((ripcVersion >>  8) & 0xFF);
    buf[6] = static_cast<unsigned char>(ripcVersion & 0xFF);

    buf[7]  = 0x00;  /* flags                            */
    buf[8]  = 20;    /* hdrSize                          */
    buf[9]  = 0x00;  /* compBitmapSize = 0               */
    buf[10] = 60;    /* pingTimeout                      */
    buf[11] = 0x00;  /* rsslFlags                        */
    buf[12] = 0x00;  /* protocolType = RWF (0)           */
    buf[13] = 14;    /* majorVersion                     */
    buf[14] = 1;     /* minorVersion                     */
    buf[15] = 0x00;  /* hostnameLen = 0                  */
    buf[16] = 0x00;  /* addrLen = 0                      */
    buf[17] = 2;     /* componentVersionLength (total)   */
    buf[18] = 0;     /* componentStringLen = 0           */

    /* Pad to totalMsgLength = 23 bytes (indices 0..22)                     */
    /* hdrSize=20 covers [0..19]; componentVersionLen=2 covers [17..18];
     * The formula is: buf[hdrSize - 1] is the last hdr byte (index 19)
     * and the next two bytes (indices 20,21) are the component version
     * block.  totalMsgLength = hdrSize + compVerLen + 1 ? 20+2+1 = 23.   */

    return 23;
}

/* =========================================================================
 * Test 1 – All-zeros garbage payload
 *
 * The client writes 32 zero bytes.  ipcProcessHdr sees version_number=0
 * which is not a known RIPC version, so it returns RIPC_CONN_ERROR causing
 * rsslInitChannel to return RSSL_RET_FAILURE on the server side.           */
TEST_F(RsslInitChannelInvalidMsgTests, AllZerosBytesRejectedByServer)
{
    const char* port = "15700";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Send 32 zero bytes */
    unsigned char zeros[32];
    memset(zeros, 0, sizeof(zeros));
    ASSERT_TRUE(rawSendAll(rawClient, zeros, sizeof(zeros)))
        << "Failed to send zero bytes";

    /* Server must reject the channel */
    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected all-zeros message but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 2 – Truncated header (fewer than 7 bytes)
 *
 * ipcProcessHdr requires at least 7 bytes (cc >= 7 check). Sending only
 * 4 bytes causes RIPC_CONN_IN_PROGRESS on the first poll, but once the
 * raw socket is closed without completing the header the server must
 * eventually return RSSL_RET_FAILURE.                                       */
TEST_F(RsslInitChannelInvalidMsgTests, TruncatedHeaderRejectedByServer)
{
    const char* port = "15701";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Send only 4 bytes – not enough for a valid RIPC header */
    unsigned char shortMsg[4] = { 0x00, 0x04, 0x00, 0x00 };
    ASSERT_TRUE(rawSendAll(rawClient, shortMsg, sizeof(shortMsg)))
        << "Failed to send truncated header";

    /* Close the raw socket immediately – the server will see EOF */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    /* Server must reject the channel */
    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected truncated header but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 3 – Unknown RIPC version number
 *
 * A well-sized message (>= 7 bytes) but with an unknown version field
 * (0xDEADBEEF) triggers the default case in ipcProcessHdr returning
 * RIPC_CONN_ERROR.                                                          */
TEST_F(RsslInitChannelInvalidMsgTests, UnknownVersionRejectedByServer)
{
    const char* port = "15702";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Build a 17-byte message with an invalid version (0xDEADBEEF)        */
    unsigned char msg[17];
    memset(msg, 0, sizeof(msg));

    /* Total length = 17 (big-endian u16) */
    msg[0] = 0x00;
    msg[1] = 17;

    /* opCode = 0 */
    msg[2] = 0x00;

    /* Unknown version 0xDEADBEEF (big-endian u32) */
    msg[3] = 0xDE;
    msg[4] = 0xAD;
    msg[5] = 0xBE;
    msg[6] = 0xEF;

    /* Rest of header padding */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send unknown-version message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected unknown version but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 4 – Valid RIPC version but mismatched hdrSize field
 *
 * Start with a correctly-versioned v14 header, then corrupt the hdrSize
 * byte so that (hdrSize + compVerLen + 1) != totalMsgLength.  The server
 * sets rsslSocketChannel->mountNak which causes ipcFinishSess to call
 * ipcRejectSession, ultimately returning RIPC_CONN_ERROR / CONN_REFUSED.   */
TEST_F(RsslInitChannelInvalidMsgTests, MismatchedHeaderSizeRejectedByServer)
{
    const char* port = "15703";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    /* Corrupt hdrSize at byte [8]: set it to an impossible value (0xFF)
     * so hdrSize + compVerLen + 1 = 0xFF + 2 + 1 = 258 != totalMsgLength(23) */
    msg[8] = 0xFF;

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send mismatched-hdrSize message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected mismatched hdrSize but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 65538") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 5 – Valid RIPC version but mismatched protocolType
 *
 * Start with a valid v14 header, then set protocolType to 0xFF, which
 * differs from the server's RWF protocol type (0).  The server must set
 * rsslSocketChannel->mountNak which causes ipcFinishSess to call
 * ipcRejectSession, ultimately returning RIPC_CONN_ERROR / CONN_REFUSED.   */
TEST_F(RsslInitChannelInvalidMsgTests, MismatchedProtocolTypeRejectedByServer)
{
    const char* port = "15704";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    /* Corrupt protocolType at byte [12]: 0xFF is not a valid protocol type
     * and will never match the server's RWF protocol type (0).             */
    msg[12] = 0xFF;

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send mismatched-protocolType message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected mismatched protocol type but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1006 ipcRejectSession() Connection refused.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 6 – Immediate EOF (zero bytes sent, then socket closed)
 *
 * The client connects and immediately closes the TCP connection without
 * sending any data.  The server's ipcReadHdr() receives cc = 0 or a read
 * error, which ipcProcessHdr treats as RIPC_CONN_ERROR because cc < 7.    */
TEST_F(RsslInitChannelInvalidMsgTests, ImmediateEofRejectedByServer)
{
    const char* port = "15705";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Close without sending any data – the server sees EOF on first read */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    /* Server must reject the channel */
    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected zero-byte / EOF client but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 7 – All-0xFF garbage payload
 *
 * Sending 32 bytes of 0xFF.  The length field [0..1] = 0xFFFF makes the
 * server think the message is 65535 bytes long, far exceeding the input
 * buffer size, which triggers the "Invalid Message Size" guard in
 * ipcReadSession / ipcProcessHdr and returns RSSL_RET_FAILURE.            */
TEST_F(RsslInitChannelInvalidMsgTests, AllOxFFBytesRejectedByServer)
{
    const char* port = "15706";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[17];
    memset(msg, 0xFF, sizeof(msg));

    /* Declare a wildly oversized length (big-endian u16 = 0x7FFF) */
    msg[0] = 0x7F;
    msg[1] = 0xFF;
    msg[2] = 0x00;  /* opCode */

    /* CONN_VERSION_10 = 0x0000000A */
    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = 0x0A;

    msg[7]  = 0x00;  /* flags                            */
    msg[8]  = 20;    /* hdrSize                          */
    msg[9]  = 0x00;  /* compBitmapSize = 0               */
    msg[10] = 60;    /* pingTimeout                      */
    msg[11] = 0x00;  /* rsslFlags                        */
    msg[12] = 0x00;  /* protocolType = RWF (0)           */
    msg[13] = 14;    /* majorVersion                     */
    msg[14] = 1;     /* minorVersion                     */
    msg[15] = 0x00;  /* hostnameLen = 0                  */
    msg[16] = 0x00;  /* addrLen = 0                      */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send all-0xFF payload";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected all-0xFF message but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 8 – Oversized length field
 *
 * Build a message whose 2-byte length field claims 0x7FFF (32767) bytes
 * with a known RIPC version at [3..6], but only 17 actual bytes are
 * delivered.  The server reads the 17 bytes, parses the header, then when
 * it checks (hdrSize + compVerLen + 1) against the declared totalMsgLength
 * the mismatch causes RIPC_CONN_ERROR without any buffer over-read.
 *
 * However, the boundary case where the length field is exactly equal to
 * the buffer size minus the RIPC header size must be considered:
 * the parser must not overrun the end of the input buffer in that case.
 *     [RipcFrame.c line 800 ipcProcessHdr() at "unknown version"]       */
TEST_F(RsslInitChannelInvalidMsgTests, OversizedLengthFieldRejectedByServer)
{
    const char* port = "15707";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Craft a v12 message with compBitmapSize=0x20 (32) but only 23
     * bytes in the buffer – the parser will find header-size mismatch. */
    unsigned char msg[23];
    memset(msg, 0, sizeof(msg));

    /* Declare a wildly oversized length (big-endian u16 = 0x7FFF) */
    msg[0] = 0x7F;
    msg[1] = 0xFF;
    msg[2] = 0x00;  /* opCode */

    /* CONN_VERSION_10 = 0x0000000A */
    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = 0x0A;

    msg[7]  = 0x00;  /* flags                            */
    msg[8]  = 20;    /* hdrSize                          */
    msg[9]  = 0x20;   /* compBitmapSize = 32 – inconsistent with actual bytes */
    msg[10] = 60;    /* pingTimeout                      */
    msg[11] = 0x00;  /* rsslFlags                        */
    msg[12] = 0x00;  /* protocolType = RWF (0)           */
    msg[13] = 14;    /* majorVersion                     */
    msg[14] = 1;     /* minorVersion                     */
    msg[15] = 0x00;  /* hostnameLen = 0                  */
    msg[16] = 0x00;  /* addrLen = 0                      */

    /* Component versioning (total=2, stringLen=0) */
    msg[17] = 2;     /* componentVersionLength (total)   */
    msg[18] = 0;     /* componentStringLen = 0           */

    /* Pad to totalMsgLength = 23 bytes (indices 0..22)                     */
    /* hdrSize=20 covers [0..19]; componentVersionLen=2 covers [17..18];
     * The formula is: buf[hdrSize - 1] is the last hdr byte (index 19)
     * and the next two bytes (indices 20,21) are the component version
     * block.  totalMsgLength = hdrSize + compVerLen + 1 ? 20+2+1 = 23.   */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send oversized-length message";

    /* Close so the server is not waiting for 65535 bytes */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected oversized length field but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 9 – RIPC v10 header that is below V10_MIN_CONN_HDR (17 bytes)
 *
 * ipcProcessHdr checks (totalMsgLength < V10_MIN_CONN_HDR) for CONN_VERSION_10.
 * We send a message that claims version 10 but is only 10 bytes, triggering
 * the "Invalid Ripc connection request size" error path.                   */
TEST_F(RsslInitChannelInvalidMsgTests, V10TooShortHeaderRejectedByServer)
{
    const char* port = "15731";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Send only 4 bytes – not enough for a valid RIPC header */
    unsigned char shortMsg[4] = { 0x00, 0x04, 0x00, 0x00 };
    ASSERT_TRUE(rawSendAll(rawClient, shortMsg, sizeof(shortMsg)))
        << "Failed to send truncated header";

    /* Close the raw socket immediately – the server will see EOF */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    /* Server must reject the channel */
    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected truncated header but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 10 – RIPC v12 with oversized compBitmapSize
 *
 * In ipcProcessHdr, when the RIPC_KEY_EXCHANGE flag (0x01) is present in
 * the flags byte [7], the parser expects a DH key block immediately after
 * the fixed header.  We set the flag but send no extra bytes, so the server
 * reads past the end of the received data and must handle the short-read
 * gracefully without crashing.                                             */
TEST_F(RsslInitChannelInvalidMsgTests, V12OversizedCompBitmapRejectedByServer)
{
    const char* port = "15724";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg), CONN_VERSION_12);
    ASSERT_GT(len, 0);

    /* Corrupt protocolType at byte [12]: 0xFF is not a valid protocol type
     * and will never match the server's RWF protocol type (0).             */
    msg[9]  = 0x20;  /* compBitmapSize = 32                                   */
    msg[12] = 0xFF;  /* protocolType = unknown                               */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send oversized compBitmapSize message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected oversized compBitmapSize but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 12 header size 20") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 11 – v10 with all-ones length field (0xFFFF), socket closed early
 *
 * The length field claims 65535 bytes with CONN_VERSION_10 but only 17
 * bytes are delivered and the socket is then closed.  After the EOF the
 * server must error out without waiting indefinitely.                       */
TEST_F(RsslInitChannelInvalidMsgTests, V10AllOnesLengthFieldRejectedByServer)
{
    const char* port = "15721";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[17];
    memset(msg, 0, sizeof(msg));

    /* Claim 65535-byte message with CONN_VERSION_10                       */
    msg[0] = 0xFF;
    msg[1] = 0xFF;
    msg[2] = 0x00;  /* opCode */

    /* CONN_VERSION_10 = 0x0000000A */
    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = CONN_VERSION_10;

    msg[7]  = 0x00;  /* flags                            */
    msg[8]  = 17;    /* hdrSize                          */
    msg[9]  = 0x00;  /* compBitmapSize = 0               */
    msg[10] = 60;    /* pingTimeout                      */
    msg[11] = 0x00;  /* rsslFlags                        */
    msg[12] = 0x00;  /* protocolType = RWF (0)           */
    msg[13] = 14;    /* majorVersion                     */
    msg[14] = 1;     /* minorVersion                     */
    msg[15] = 0x00;  /* hostnameLen = 0                  */
    msg[16] = 0x00;  /* addrLen = 0                      */

    /* Close so the server is not left waiting for 65512 more bytes */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected v10 all-ones length but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1002 Could not read IPC Mount Request.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 12 – RIPC v12 with corrupted hdrSize (0xFF)
 *
 * CONN_VERSION_14 (0x0E) shares the v14 branch.  hdrSize=0xFF makes
 * (0xFF + compVerLen(2) + 1) = 276 != totalMsgLength(23).
 * The header-size check must catch this small value just as reliably.     */
TEST_F(RsslInitChannelInvalidMsgTests, V14CorruptedHdrSizeRejectedByServer)
{
    const char* port = "15714";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg), CONN_VERSION_12);
    ASSERT_GT(len, 0);

    msg[8] = 0xFF;  /* hdrSize = 255, impossible */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send v14 corrupted-hdrSize message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected v14 corrupted hdrSize but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 12 header size 65535") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 13 – Byte-by-byte slow drip with invalid hdrSize
 *
 * The client sends a 17-byte RIPC v14 header one byte at a time.  hdrSize
 * is set to 0, making (0 + compVerLen(2) + 1) = 1 != totalMsgLength(17).

 * The parser enters the "totalMsgLength < V10_MIN_CONN_HDR" branch and returns
 * RIPC_CONN_ERROR.  We deliver exactly 16 bytes to avoid partial-read
 * ambiguity.                                                                */
TEST_F(RsslInitChannelInvalidMsgTests, ByteByByteDripRejectedByServer)
{
    const char* port = "15712";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[17];
    memset(msg, 0, sizeof(msg));
    msg[0] = 0x00;
    msg[1] = 17;
    msg[2] = 0x00;
    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = 0x0016;  /* CONN_VERSION_13 */
    msg[7] = 0x00;
    msg[8] = 0x00;  /* hdrSize = 0, invalid */

    for (int i = 0; i < 17; ++i)
    {
        ASSERT_TRUE(rawSendAll(rawClient, msg + i, 1))
            << "Failed to send byte " << i;
        time_sleep_ms(2);
    }

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected byte-by-byte drip but channel state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 13 header") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* -------------------------------------------------------------------------
 * Test 14 – Zero pingTimeout in a well-formed v14 header
 *
 * pingTimeout is read at hdrStart[10] + compBitmapSize.  With both at zero
 * the read occurs at hdrStart[10], which is still within the fixed header.
 * The server accepts the request and uses its minimum ping timeout instead.  */
TEST_F(RsslInitChannelInvalidMsgTests, ZeroPingTimeoutAcceptedByServer)
{
    const char* port = "15714";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[10] = 0x00;  /* pingTimeout = 0 */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send zero-pingTimeout message";

    RsslBindOptions bindOptions = RSSL_INIT_BIND_OPTS;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_FALSE(rejected)
        << "Server should have accepted zero pingTimeout and state is "
        << pServerChnl->state;
    EXPECT_EQ(RSSL_CH_STATE_ACTIVE, pServerChnl->state);
    EXPECT_EQ(bindOptions.minPingTimeout, pServerChnl->pingTimeout);
}

/* =========================================================================
 * Test 15 – hostnameLen overflow past totalMsgLength
 *
 * hostnameLen=255 causes the computed hdrSize to far exceed totalMsgLength,
 * triggering the header-size validation error in ipcProcessHdr before any
 * out-of-bounds memcpy of the hostname string occurs.                      */
TEST_F(RsslInitChannelInvalidMsgTests, HostnameLenOverflowRejectedByServer)
{
    const char* port = "15715";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    /* hostnameLen = 255 causes addrLen and componentVersionLen reads to
     * overflow past the end of the 23-byte buffer, triggering the size check. */
    msg[15] = 0xFF;

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send hostnameLen overflow message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected hostnameLen overflow but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 16 – addrLen = 0xFF forces memcpy target past buffer end
 *
 * With hostnameLen=0 and compBitmapSize=0, addrLen lives at byte [16].
 * Setting addrLen=0xFF (255) would cause a 255-byte memcpy of the IP-
 * address string starting at hdrStart[17], running 239 bytes past the end
 * of the 23-byte message.  The hdrSize validation must fire first.         */
TEST_F(RsslInitChannelInvalidMsgTests, AddrLenOverflowRejectedByServer)
{
    const char* port = "15731";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[15] = 0x00;  /* hostnameLen = 0                                     */
    msg[16] = 0xFF;  /* addrLen = 255: hdrSize >> 23, fires before memcpy   */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send addrLen-overflow message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected addrLen overflow but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 17 – componentVersionLen exceeds remaining buffer bytes
 *
 * componentVersionLen is read at hdrStart[17 + compBitmapSize +
 * hostnameLen + addrLen].  With all three zero that is byte [17].
 * Setting componentVersionLen=0xFF makes compVerLen=255, so
 * hdrSize(20) + 255 + 1 = 276 != totalMsgLength(23).  The size check must
 * fire before any memcpy of the component version string.                  */
TEST_F(RsslInitChannelInvalidMsgTests, ComponentVersionLenOverflowRejectedByServer)
{
    const char* port = "15727";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[17] = 0xFF;  /* componentVersionLen = 255                            */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send compVerLen overflow message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected componentVersionLen overflow but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 276") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 18 – hdrSize underflow (value 1)
 *
 * hdrSize=1 makes (1 + compVerLen(2) + 1) = 4 != totalMsgLength(23).
 * This is the opposite extreme from Test 12's hdrSize=0xFF overflow.
 * The header-size check must catch this small value just as reliably.     */
TEST_F(RsslInitChannelInvalidMsgTests, HdrSizeUnderflowRejectedByServer)
{
    const char* port = "15729";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[8] = 0x01;  /* hdrSize = 1                                    */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send hdrSize-underflow message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected hdrSize underflow but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 4") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 19 – Maximum compBitmapSize (0xFF) pushes all variable-field reads
 *            far beyond the 23 received bytes
 *
 * ipcProcessHdr reads hostnameLen at hdrStart[15 + compBitmapSize].
 * With compBitmapSize=255 that is byte [270], well past the 23-byte buffer.
 * The hdrSize check must fire and reject the connection without crashing. */
TEST_F(RsslInitChannelInvalidMsgTests, MaxCompBitmapSizeOobRejectedByServer)
{
    const char* port = "15732";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Craft a v12 message with compBitmapSize=0x20 (32) but only 23
     * bytes in the buffer – the parser will find header-size mismatch. */
    unsigned char msg[23];
    memset(msg, 0, sizeof(msg));

    /* Declare a wildly oversized length (big-endian u16 = 0x7FFF) */
    msg[0] = 0x7F;
    msg[1] = 0xFF;
    msg[2] = 0x00;  /* opCode */

    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = CONN_VERSION_10;

    msg[7]  = 0x00;  /* flags                            */
    msg[8]  = 20;    /* hdrSize                          */
    msg[9]  = 0xFF;  /* compBitmapSize = 255 (impossible)  */
    msg[10] = 60;    /* pingTimeout                      */
    msg[11] = 0x00;  /* rsslFlags                        */
    msg[12] = 0x00;  /* protocolType = RWF (0)           */
    msg[13] = 14;    /* majorVersion                     */
    msg[14] = 1;     /* minorVersion                     */
    msg[15] = 0x00;  /* hostnameLen = 0                  */
    msg[16] = 0x00;  /* addrLen = 0                      */

    /* Component versioning (total=2, stringLen=0) */
    msg[17] = 2;     /* componentVersionLength (total)   */
    msg[18] = 0;     /* componentStringLen = 0           */

    /* Pad to totalMsgLength = 23 bytes (indices 0..22)                     */
    /* hdrSize=20 covers [0..19]; componentVersionLen=2 covers [17..18];
     * The formula is: buf[hdrSize - 1] is the last hdr byte (index 19)
     * and the next two bytes (indices 20,21) are the component version
     * block.  totalMsgLength = hdrSize + compVerLen + 1 ? 20+2+1 = 23.   */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send max-compBitmapSize message";

    /* Close so the server is not waiting for 65535 bytes */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected max compBitmapSize but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 10 header size 20") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 20 – hdrSize and componentVersionLen both at 0xFE (near-wrap)
 *
 * hdrSize=0xFE(254) + compVerLen=0xFE(254) + 1 = 509.  Against
 * totalMsgLength=23 this is an enormous mismatch.  The check must not
 * wrap via unsigned arithmetic and must definitively reject.               */
TEST_F(RsslInitChannelInvalidMsgTests, HdrSizeAndCompVerLenBothMaxRejectedByServer)
{
    const char* port = "15721";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[8]  = 0xFE;  /* hdrSize = 254                                       */
    msg[17] = 0xFE;  /* componentVersionLen = 254                            */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send both-max hdrSize+compVerLen message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected both-max hdrSize+compVerLen but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 65789") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 21 – compBitmapSize shifts componentVersionLen read past buffer end
 *
 * With compBitmapSize=4, hostnameLen is read at hdrStart[19], addrLen at
 * hdrStart[20], and componentVersionLen at hdrStart[21].  Setting
 * componentVersionLen=0xFF makes compVerLen=255, so
 * hdrSize(20) + 255 + 1 = 276 != totalMsgLength(23).  The size check must
 * fire before any memcpy of the component version string.                  */
TEST_F(RsslInitChannelInvalidMsgTests, CompBitmapShiftsCompVerLenOobRejectedByServer)
{
    const char* port = "15722";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Use a 32-byte buffer so bytes [19..21] are within the local array,
     * but totalMsgLength declared in the wire header stays 23.            */
    unsigned char msg[32];
    memset(msg, 0, sizeof(msg));

    int baseLen = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(baseLen, 0);

    /* compBitmapSize=4: hostnameLen@[19]=0, addrLen@[20]=0,
     * componentVersionLen@[21]=0xFF -> compVerLen=255.
     * hdrSize(20)+255+1=276 != 23 fires the size check first.             */
    msg[9]  = 0x04;  /* compBitmapSize = 4                                   */
    msg[19] = 0x00;  /* hostnameLen = 0 (shifted by compBitmapSize)          */
    msg[20] = 0x00;  /* addrLen = 0                                           */
    msg[21] = 0xFF;  /* componentVersionLen = 255, overflow if reached        */

    /* Send only the 23 bytes declared in msg[0..1]                         */
    ASSERT_TRUE(rawSendAll(rawClient, msg, 23))
        << "Failed to send compBitmapSize-shifted compVerLen OOB message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected compBitmapSize-shifted compVerLen OOB but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 276") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 22 – Non-zero opCode byte
 *
 * ipcProcessHdr checks that opCode (byte [2]) is 0x00 (IPC_CONNECT_REQ).
 * Setting it to 0x03 (IPC_DATA) triggers the "Invalid connection request"
 * error path and must cause rejection without any crash.                   */
TEST_F(RsslInitChannelInvalidMsgTests, NonZeroOpCodeRejectedByServer)
{
    const char* port = "15723";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int len = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_GT(len, 0);

    msg[2] = 0x03;  /* IPC_DATA instead of IPC_CONNECT_REQ (0x00) */

    ASSERT_TRUE(rawSendAll(rawClient, msg, len))
        << "Failed to send non-zero opCode message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected non-zero opCode but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Ripc opcode (3) for connection request") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 24 – totalMsgLength field = 0 (below the cc >= 7 minimum)
 *
 * A two-byte length field of 0x0000 means totalMsgLength = 0.  After the
 * server receives enough bytes for the outer length field it enters
 * ipcProcessHdr with cc < 7, triggering RIPC_CONN_IN_PROGRESS.  Closing
 * the socket then causes an EOF-induced RSSL_RET_FAILURE.                 */
TEST_F(RsslInitChannelInvalidMsgTests, ZeroTotalMsgLengthRejectedByServer)
{
    const char* port = "15725";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Send exactly 7 bytes with totalMsgLength = 0 (wire value) */
    unsigned char msg[7];
    memset(msg, 0, sizeof(msg));
    msg[0] = 0x00;
    msg[1] = 0x00;   /* totalMsgLength = 0 */
    msg[2] = 0x00;   /* opCode            */
    msg[3] = 0x00;
    msg[4] = 0x00;
    msg[5] = 0x00;
    msg[6] = 0x0D;   /* CONN_VERSION_13   */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send zero-totalMsgLength message";

    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected zero totalMsgLength but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 25 – Alternating 0x00 / 0xFF byte pattern (mixed garbage)
 *
 * A 32-byte payload whose bytes alternate 0x00 and 0xFF produces a
 * length field of 0x00FF (255) and a version field of 0xFF00FF00, neither
 * of which matches a known RIPC version.  The parser must reject cleanly
 * without any heap access past the input buffer.                           */
TEST_F(RsslInitChannelInvalidMsgTests, AlternatingPatternRejectedByServer)
{
    const char* port = "15726";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[32];
    for (int i = 0; i < 32; ++i)
        msg[i] = (i % 2 == 0) ? 0x00u : 0xFFu;

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send alternating-pattern message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected alternating-pattern message but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Unknown connection type.") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 30 – Valid v14 header followed by a large excess payload
 *
 * The first 23 bytes form a syntactically correct RIPC v14 connect-request
 * (as built by buildValidRipcv14Header).  We then append 4096 additional
 * bytes of 0xAB padding, making totalMsgLength (the actual byte count in
 * the server's input buffer) = 4119.
 *
 * Inside ipcProcessHdr the check:
 *   (hdrSize + compVerLen + 1) != totalMsgLength
 *   => (20 + 2 + 1) = 23 != 4119
 * fires before any field pointer dereference, causing RIPC_CONN_ERROR.
 * The server must reject cleanly without any heap overrun.                 */
TEST_F(RsslInitChannelInvalidMsgTests, ValidHdrWithLargeExcessPayloadRejectedByServer)
{
    const char* port = "15731";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* Build the 23-byte valid v14 header */
    static const int HDR_LEN     = 23;
    static const int EXCESS_LEN  = 4096;
    static const int TOTAL_LEN   = HDR_LEN + EXCESS_LEN;

    unsigned char msg[TOTAL_LEN];
    int hdrLen = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_EQ(hdrLen, HDR_LEN);

    /* Fill excess payload with a recognisable pattern */
    memset(msg + HDR_LEN, 0xAB, EXCESS_LEN);

    ASSERT_TRUE(rawSendAll(rawClient, msg, TOTAL_LEN))
        << "Failed to send valid-header + large-excess-payload message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected valid header + large excess payload but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 23") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 31 – Maximum wire length field (0xFFFF) with a valid v14 header body
 *
 * The two-byte wire length field at bytes [0..1] is set to 0xFFFF (65535),
 * claiming a 65535-byte message.  The remaining bytes form a valid v14
 * header structure, but the declared wire length far exceeds both the
 * actual bytes sent and the server's input-buffer capacity.
 *
 * The server's ipcReadHdr reads up to inputBuffer->maxLength bytes.  After
 * the read, totalMsgLength (actual bytes in buffer) is much less than
 * 65535.  Because the declared wire length (0xFFFF) does not match
 * (hdrSize + compVerLen + 1) = 23, and totalMsgLength does not equal
 * the declared wire length either, ipcProcessHdr returns RIPC_CONN_ERROR.
 *
 * The raw socket is closed immediately after sending to prevent the server
 * from blocking waiting for the remaining ~65512 bytes.                   */
TEST_F(RsslInitChannelInvalidMsgTests, MaxWireLengthFieldWithValidv14BodyRejectedByServer)
{
    const char* port = "15732";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    unsigned char msg[24];
    int hdrLen = buildValidRipcHeader(msg, sizeof(msg));
    ASSERT_EQ(hdrLen, 23);

    /* Override the two-byte wire length field to the maximum u16 value */
    msg[0] = 0xFF;
    msg[1] = 0xFF;   /* declared totalMsgLength = 65535 */

    ASSERT_TRUE(rawSendAll(rawClient, msg, sizeof(msg)))
        << "Failed to send max-wire-length message";

    /* Close immediately so the server is not left waiting for 65512 more bytes */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected max wire-length field but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 23") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}

/* =========================================================================
 * Test 32 – Wire length field matches exactly V10_MIN_CONN_HDR (17) but
 *            body contains an intentionally oversized excess to fill the
 *            server's input buffer close to its capacity
 *
 * V10_MIN_CONN_HDR = 17.  We craft a v14 header whose [0..1] length field
 * declares only 17 bytes, but we actually send 32 KB – 1 byte (32767 B),
 * which is the largest single TCP send that a u16 length field could ever
 * represent without wrapping.  The server reads all data into its input
 * buffer (bounded by maxLength), then computes:
 *   totalMsgLength = actual bytes in buffer >> 17
 * The version switch enters the v14 branch:
 *   totalMsgLength (32767) >= V10_MIN_CONN_HDR (17) ? enters inner block
 *   (hdrSize + compVerLen + 1) = 23 != 32767 ? RIPC_CONN_ERROR
 *
 * This exercises the large-buffer read path that precedes the size check,
 * probing for any buffer-length arithmetic overflow on the server side.   */
TEST_F(RsslInitChannelInvalidMsgTests, LargeExcessBeyondWireLengthFillsInputBufferRejectedByServer)
{
    const char* port = "15733";
    ASSERT_TRUE(bindServer(port)) << "Failed to bind server on " << port;
    ASSERT_TRUE(connectRawAndAccept(port)) << "Raw connect / accept failed";

    /* 32767 bytes: largest value representable as a positive signed i16,
     * still fits in u16 without sign confusion in the server's RTR_GET_16
     * macro used in ipcReadSession, and large enough to stress the
     * buffer-fill path in ipcReadHdr.                                     */
    static const int SEND_LEN = 32767;

    unsigned char* msg = new unsigned char[SEND_LEN];
    ASSERT_NE(msg, nullptr);

    /* Place a valid v14 header in the first 23 bytes */
    int hdrLen = buildValidRipcHeader(msg, SEND_LEN);
    ASSERT_EQ(hdrLen, 23);

    /* Declare only 17 bytes in the wire length field (= V10_MIN_CONN_HDR),
     * so the server's totalMsgLength reflects actual bytes read, not this
     * field: the mismatch fires regardless.                               */
    msg[0] = 0x00;
    msg[1] = 17;    /* wire-declared length = 17 */

    /* Pad the remaining bytes with 0xCD */
    memset(msg + 23, 0xCD, SEND_LEN - 23);

    bool sent = rawSendAll(rawClient, msg, SEND_LEN);
    delete[] msg;

    /* Close immediately to unblock the server if it is waiting for more  */
    rawSocketClose(rawClient);
    rawClient = kInvalidRawSocket;

    ASSERT_TRUE(sent) << "Failed to send large-excess message";

    RsslError rsslError;
    bool rejected = driveServerUntilFinished(pServerChnl, &rsslError);
    EXPECT_TRUE(rejected)
        << "Server should have rejected large excess beyond wire length but state is "
        << pServerChnl->state;
    EXPECT_TRUE(strstr(rsslError.text, "Error: 1007 Invalid Conn Ver 14 header size 23") != NULL);
    EXPECT_EQ(RSSL_CH_STATE_CLOSED, pServerChnl->state);
}


