/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *  WebSocket rssl.json.v2 Protocol Unit Tests
 *
 *  Unit tests that establish a WebSocket connection between a client and
 *  a server using the rssl.json.v2 sub-protocol, then exercise the
 *  rsslWrite / rsslWriteEx / rsslRead / rsslFlush APIs over that channel.
 *
 *  All connection-setup and write/read patterns follow the style of
 *  rsslSocketWriteUnitTest.cpp; only the WebSocket sub-protocol string
 *  ("rssl.json.v2") and the transport protocolType differ.
 *
 ************************************************************************/

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
#include "rtr/rsslAlloc.h"

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
#include <arpa/inet.h>
#endif

/* WebSocket sub-protocol string for rssl.json.v2 */
static const char* const WS_JSON_V2_PROTOCOL = "rssl.json.v2";

static const char* const HTTP_PROTOCOL_VERSION = "HTTP/1.1";

/* RSSL_JSON_PROTOCOL_TYPE = 2  (binary encoded as an integer for clarity) */
static const RsslUInt8 JSON_PROTOCOL_TYPE = (RsslUInt8)2;

/* -----------------------------------------------------------------------
 * setupJsonV2ChannelPair
 *
 * Establishes a non-blocking server + client channel pair over WebSocket
 * with the rssl.json.v2 sub-protocol.  Returns true when both channels
 * reach RSSL_CH_STATE_ACTIVE; false on any failure.
 *
 * Parameters mirror setupActiveChannelPair in rsslSocketWriteUnitTest.cpp.
 * --------------------------------------------------------------------- */
static bool setupJsonV2ChannelPair(
    const char*       port,
    RsslServer**      ppServer,
    RsslChannel**     ppServerChnl,
    RsslChannel**     ppClientChnl,
    RsslUserCookies*  serverCookies = NULL,
    RsslUserCookies*  clientCookies = NULL,
    RsslHttpCallback* serverHttpCallback = NULL,
    RsslHttpCallback* clientHttpCallback = NULL,
    void*             pServerUserSpec = NULL,
    void*             pClientUserSpec = NULL)
{
    RsslError err;
    struct timeval selectTime;
    fd_set readfds;
    int selRet;

    /* --- Bind server (non-blocking, rssl.json.v2 over WebSocket) ------- */
    RsslBindOptions bindOpts;
    rsslClearBindOpts(&bindOpts);
    bindOpts.serviceName              = const_cast<char*>(port);
    bindOpts.connectionType           = RSSL_CONN_TYPE_WEBSOCKET;
    bindOpts.wsOpts.protocols         = const_cast<char*>(WS_JSON_V2_PROTOCOL);
    bindOpts.wsOpts.httpCallback      = serverHttpCallback;
    bindOpts.majorVersion             = RSSL_RWF_MAJOR_VERSION;
    bindOpts.minorVersion             = RSSL_RWF_MINOR_VERSION;
    bindOpts.channelsBlocking         = RSSL_FALSE;
    bindOpts.serverBlocking           = RSSL_FALSE;
    bindOpts.guaranteedOutputBuffers  = 5000;
    bindOpts.maxOutputBuffers         = 5000;

    if(serverCookies)
        bindOpts.wsOpts.cookies = *serverCookies;

    *ppServer = rsslBind(&bindOpts, &err);
    if (!*ppServer)
    {
        std::cout << "setupJsonV2ChannelPair: rsslBind failed on port " << port
                  << ": " << err.text << "\n";
        return false;
    }

    /* --- Connect client (non-blocking, rssl.json.v2 over WebSocket) ---- */
    RsslConnectOptions copts;
    rsslClearConnectOpts(&copts);
    copts.connectionType                     = RSSL_CONN_TYPE_WEBSOCKET;
    copts.wsOpts.protocols                   = const_cast<char*>(WS_JSON_V2_PROTOCOL);
    copts.wsOpts.httpCallback                = clientHttpCallback;
    copts.connectionInfo.unified.address     = const_cast<char*>("localhost");
    copts.connectionInfo.unified.serviceName = const_cast<char*>(port);
    copts.protocolType                       = JSON_PROTOCOL_TYPE;
    copts.majorVersion                       = RSSL_RWF_MAJOR_VERSION;
    copts.minorVersion                       = RSSL_RWF_MINOR_VERSION;
    copts.tcp_nodelay                        = RSSL_TRUE;
    copts.blocking                           = RSSL_FALSE;
    copts.guaranteedOutputBuffers            = 500;
    copts.userSpecPtr                        = pClientUserSpec;

    if(clientCookies)
        copts.wsOpts.cookies = *clientCookies;

    *ppClientChnl = rsslConnect(&copts, &err);
    if (!*ppClientChnl)
    {
        std::cout << "setupJsonV2ChannelPair: rsslConnect failed: " << err.text << "\n";
        rsslCloseServer(*ppServer, &err);
        *ppServer = NULL;
        return false;
    }

    /* --- Accept server channel ----------------------------------------- */
    FD_ZERO(&readfds);
    FD_SET((*ppServer)->socketId, &readfds);
    selectTime.tv_sec  = 5L;
    selectTime.tv_usec = 0L;
    selRet = select(FD_SETSIZE, &readfds, NULL, NULL, &selectTime);
    if (selRet <= 0)
    {
        std::cout << "setupJsonV2ChannelPair: select for accept timed out or failed\n";
        rsslCloseChannel(*ppClientChnl, &err);
        rsslCloseServer(*ppServer, &err);
        *ppClientChnl = NULL;
        *ppServer     = NULL;
        return false;
    }

    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);
    acceptOpts.userSpecPtr = pServerUserSpec;
    *ppServerChnl = rsslAccept(*ppServer, &acceptOpts, &err);
    if (!*ppServerChnl)
    {
        std::cout << "setupJsonV2ChannelPair: rsslAccept failed: " << err.text << "\n";
        rsslCloseChannel(*ppClientChnl, &err);
        rsslCloseServer(*ppServer, &err);
        *ppClientChnl = NULL;
        *ppServer     = NULL;
        return false;
    }

    /* --- Drive rsslInitChannel on both sides until ACTIVE -------------- */
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
                std::cout << "setupJsonV2ChannelPair: client rsslInitChannel failed: "
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
                std::cout << "setupJsonV2ChannelPair: server rsslInitChannel failed: "
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

    std::cout << "setupJsonV2ChannelPair: channels did not reach ACTIVE state\n";
    rsslCloseChannel(*ppServerChnl, &err);
    rsslCloseChannel(*ppClientChnl, &err);
    rsslCloseServer(*ppServer, &err);
    *ppServerChnl = NULL;
    *ppClientChnl = NULL;
    *ppServer     = NULL;
    return false;
}

/* -----------------------------------------------------------------------
 * Base test fixture – RSSL_LOCK_GLOBAL, rssl.json.v2 WebSocket channel.
 * --------------------------------------------------------------------- */
class RsslWebSocketHttpCallbackTests : public ::testing::Test
{
protected:
    RsslServer*  pServer     = nullptr;
    RsslChannel* pServerChnl = nullptr;
    RsslChannel* pClientChnl = nullptr;
    static RsslUserCookies* pClientUserCookies;
    static RsslUserCookies* pServerUserCookies;
    static void* pClientUserSpec;
    static void* pServerUserSpec;
    static RsslError* pHttpCallbackError;

    virtual void SetUp() override
    {
        RsslError err;
        rsslInitialize(RSSL_LOCK_GLOBAL, &err);

        pClientUserCookies = NULL;
        pServerUserCookies = NULL;
        pClientUserSpec = NULL;
        pServerUserSpec = NULL;
        pHttpCallbackError = NULL;
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

    bool setupChannelPair(
        const char*       port,
        RsslUserCookies*  serverCookies = NULL,
        RsslUserCookies*  clientCookies = NULL,
        RsslHttpCallback* serverHttpCallback = NULL,
        RsslHttpCallback* clientHttpCallback = NULL,
        void*             pServerUserSpec = NULL,
        void*             pClientUserSpec = NULL)
    {
        return setupJsonV2ChannelPair(port, &pServer, &pServerChnl, &pClientChnl,
                                     serverCookies, clientCookies, serverHttpCallback, 
            clientHttpCallback, pServerUserSpec, pClientUserSpec);
    }

    RsslBuffer* getClientBuffer(RsslUInt32 size, RsslError* pErr, RsslBool packed = RSSL_FALSE)
    {
        return rsslGetBuffer(pClientChnl, size, packed, pErr);
    }

public:

    /* Compares the cookie content of pExpectedCookies (RsslUserCookies, as passed by the
     * test into RsslWSocketOpts.cookies) against the cookies queue populated on the
     * RsslHttpMessage delivered to an httpCallback.  Each cookie's value string must
     * match exactly, in order, and the counts must be equal. */
    static bool cookiesMatch(const RsslUserCookies* pExpectedCookies, const RsslQueue* pActualCookies)
    {
        if (pExpectedCookies == NULL || pActualCookies == NULL)
            return false;

        if (pExpectedCookies->numberOfCookies != pActualCookies->count)
            return false;

        RsslQueueLink* pLink = NULL;
        RsslInt32 n = 0;

        for (pLink = rsslQueuePeekFront(const_cast<RsslQueue*>(pActualCookies));
             pLink != NULL;
             pLink = rsslQueuePeekNext(const_cast<RsslQueue*>(pActualCookies), pLink), ++n)
        {
            if (n >= pExpectedCookies->numberOfCookies)
                return false;

            RsslHttpHdrData* pData = RSSL_QUEUE_LINK_TO_OBJECT(RsslHttpHdrData, link, pLink);
            const RsslBuffer& expected = pExpectedCookies->cookie[n];

            if (pData->value.data == NULL || expected.data == NULL)
                return false;

            if (strlen(pData->value.data) != expected.length)
                return false;

            if (strncmp(pData->value.data, expected.data, expected.length) != 0)
                return false;
        }

        return (n == pExpectedCookies->numberOfCookies);
    }

    /* Validates every HTTP header entry contained in the RsslHttpMessage.headers
     * queue delivered to an httpCallback during the WebSocket handshake.
     *
     * Verifies:
     *   - The headers queue is non-empty.
     *   - Every RsslHttpHdrData link has non-NULL/non-empty name and value buffers
     *     whose lengths match the actual string content (i.e. no corrupted data).
     *   - The mandatory WebSocket upgrade headers ("Upgrade: websocket" and a
     *     "Sec-WebSocket-Protocol" line naming the negotiated rssl.json.v2
     *     sub-protocol) are present somewhere in the list. */
    static bool checkAllHeaders(const RsslQueue* pHeaders, bool isServerCallback)
    {
        if (pHeaders == NULL || pHeaders->count == 0)
            return false;

        bool sawUpgradeHeader = false;
        bool sawProtocolHeader = false;
        bool sawWebSocketVersion = false; // server
        bool sawWebSocketKey = false; // server
        bool sawWebSocketAccept = false; // client
        bool sawUserAgent = false; // server
        bool sawConnectionUpgrade = false; 

        RsslQueueLink* pLink = NULL;
        for (pLink = rsslQueuePeekFront(const_cast<RsslQueue*>(pHeaders));
             pLink != NULL;
             pLink = rsslQueuePeekNext(const_cast<RsslQueue*>(pHeaders), pLink))
        {
            RsslHttpHdrData* pData = RSSL_QUEUE_LINK_TO_OBJECT(RsslHttpHdrData, link, pLink);

            /* Every header must have well-formed, non-empty name/value buffers. */
            if (pData->name.data == NULL || pData->name.length == 0)
                return false;
            if (pData->value.data == NULL || pData->value.length == 0)
                return false;
            if (strlen(pData->name.data) < pData->name.length)
                return false;
            if (strlen(pData->value.data) < pData->value.length)
                return false;

            if (strstr(pData->name.data, "Upgrade") != NULL &&
                strstr(pData->value.data, "websocket") != NULL)
            {
                sawUpgradeHeader = true;
            }
            else if (strstr(pData->name.data, "Sec-WebSocket-Protocol") != NULL &&
                strstr(pData->value.data, "tr_json2") != NULL) // Provider always response with tr_json2 for backward compatibility.
            {
                sawProtocolHeader = true;
            }
            else if (strstr(pData->name.data, "Sec-WebSocket-Version") != NULL &&
                strstr(pData->value.data, "13") != NULL)
            {
                sawWebSocketVersion = true;
            }
            else if (strstr(pData->name.data, "Sec-WebSocket-Key") != NULL)
            {
                sawWebSocketKey = true;
            }
            else if (strstr(pData->name.data, "Sec-WebSocket-Accept") != NULL)
            {
                sawWebSocketAccept = true;
            }
            else if (strstr(pData->name.data, "User-Agent") != NULL &&
                strstr(pData->value.data, "Mozilla/5.0") != NULL)
            {
                sawUserAgent = true;
            }
            else if (strstr(pData->name.data, "Connection") != NULL)
            {
                if (isServerCallback)
                {
                    if (strstr(pData->value.data, "keep-alive, Upgrade") != NULL)
                        sawConnectionUpgrade = true;
                }
                else
                {
                    if (strstr(pData->value.data, "Upgrade") != NULL)
                        sawConnectionUpgrade = true;
                }
            }
        }

        bool commonCheck = sawUpgradeHeader && sawProtocolHeader && sawConnectionUpgrade;

        return  isServerCallback ? commonCheck && sawWebSocketVersion && sawWebSocketKey && sawUserAgent :
            commonCheck && sawWebSocketAccept;
    }

    /* Compares the fields of an expected RsslError (as configured by the test via
     * pHttpCallbackError) against the RsslError actually delivered to an httpCallback.
     * Compares rsslErrorId, sysError, channel, and the text content (only when the
     * expected text is non-empty; an empty expected text is treated as "don't care"
     * so tests that use a placeholder empty string are not forced to match). */
    static bool compareRsslErrors(const RsslError* pExpected, const RsslError* pActual)
    {
        if (pExpected == NULL || pActual == NULL)
            return false;

	EXPECT_EQ(pExpected->rsslErrorId, pActual->rsslErrorId);

        if (pExpected->rsslErrorId != pActual->rsslErrorId)
            return false;

        /* Skip checking as it depends on the last errno
        EXPECT_EQ(pExpected->sysError, pActual->sysError);

        if (pExpected->sysError != pActual->sysError)
            return false; */

	EXPECT_EQ(pExpected->channel, pActual->channel);

        if (pExpected->channel != pActual->channel)
            return false;

        if (pExpected->text[0] != '\0')
        {
	    const char* result = strstr(pActual->text, pExpected->text);
	   
	    EXPECT_TRUE(result != NULL);

            if (result == NULL)
                return false;
        }

        return true;
    }

    static void serverHttpCallbackHandler(RsslHttpMessage* httpMessage, RsslError* pError)
    {
        if (pHttpCallbackError)
        {
            ASSERT_TRUE(pError != NULL);
            ASSERT_TRUE(compareRsslErrors(pHttpCallbackError, pError))
                << "RsslError delivered to server-side httpCallback must match the expected error";
        }
        else
        {
            ASSERT_TRUE(httpMessage != NULL);
            ASSERT_EQ(0, httpMessage->statusCode);
            ASSERT_EQ(RSSL_HTTP_GET, httpMessage->httpMethod);
            ASSERT_STREQ(HTTP_PROTOCOL_VERSION, httpMessage->protocolVersion);
            ASSERT_EQ(10, httpMessage->url.length);
            ASSERT_STREQ("/WebSocket HTTP/1.1", httpMessage->url.data);
            ASSERT_TRUE(checkAllHeaders(&httpMessage->headers, true))
                << "Server-side httpCallback must receive well-formed headers including WebSocket upgrade headers";
            if (pClientUserCookies)
            {
                ASSERT_TRUE(cookiesMatch(pClientUserCookies, &httpMessage->cookies))
                    << "Server-side httpCallback cookies must match the client's RsslWSocketOpts.cookies content";
            }
            ASSERT_EQ(pServerUserSpec, httpMessage->userSpecPtr);
        }
    }

    static void clientHttpCallbackHandler(RsslHttpMessage* httpMessage, RsslError* pError)
    {
        if (pHttpCallbackError)
        {
            ASSERT_TRUE(pError != NULL);
            ASSERT_TRUE(compareRsslErrors(pHttpCallbackError, pError))
                << "RsslError delivered to client-side httpCallback must match the expected error";
        }
        else
        {
            ASSERT_TRUE(httpMessage != NULL);
            ASSERT_EQ(101, httpMessage->statusCode);
            ASSERT_EQ(RSSL_HTTP_UNKNOWN, httpMessage->httpMethod);
            ASSERT_STREQ(HTTP_PROTOCOL_VERSION, httpMessage->protocolVersion);
            ASSERT_TRUE(checkAllHeaders(&httpMessage->headers, false))
                << "Client-side httpCallback must receive well-formed headers including WebSocket upgrade headers";
            if (pServerUserCookies)
            {
                ASSERT_TRUE(cookiesMatch(pServerUserCookies, &httpMessage->cookies))
                    << "Client-side httpCallback cookies must match the server's RsslWSocketOpts.cookies content";
            }
            ASSERT_EQ(pClientUserSpec, httpMessage->userSpecPtr);
        }
    }
};

RsslUserCookies* RsslWebSocketHttpCallbackTests::pClientUserCookies;
RsslUserCookies* RsslWebSocketHttpCallbackTests::pServerUserCookies;
void* RsslWebSocketHttpCallbackTests::pClientUserSpec;
void* RsslWebSocketHttpCallbackTests::pServerUserSpec;
RsslError* RsslWebSocketHttpCallbackTests::pHttpCallbackError;

static std::atomic<int> g_customMallocInvokeCount(0);
static std::atomic<int> g_customMallocAllocationFailedCount(0);

/* Custom malloc override - forwards to the default malloc() implementation
 * but counts invocations so the test can verify it was actually used. */
static void* customCountingMalloc(size_t size)
{
    ++g_customMallocInvokeCount;

#if defined(_WIN32)
    if (g_customMallocInvokeCount != g_customMallocAllocationFailedCount)
#else
    if (g_customMallocInvokeCount != (g_customMallocAllocationFailedCount + 3)) // Add addtional allocation for Linux platform
#endif
        return malloc(size);
    else
        return NULL;
}

/* =======================================================================
 * HTTP callback tests
 * ===================================================================== */

/* Setting RsslWSocketOpts.httpCallback on both the connect and bind options
 * must result in the callback being invoked during the WebSocket HTTP
 * handshake for the rssl.json.v2 sub-protocol. */
TEST_F(RsslWebSocketHttpCallbackTests, HttpCallback_with_client_cookies)
{
    RsslBuffer cookiesData[2] = { {100, (char*)"positionName=localhost; applicationName=rsslTransportTest; authTokenName=AuthToken; authToken=xxxxxx"},
                                           {69,  (char*)"session_id=xyz123abc; Secure; HttpOnly; SameSite=Strict; Max-Age=3600"} };

    RsslUserCookies userCookies = { const_cast<RsslBuffer*>(cookiesData), 2 };

    RsslWebSocketHttpCallbackTests::pClientUserCookies = &userCookies;

    RsslWebSocketHttpCallbackTests::pServerUserSpec = (void*)1;

    ASSERT_TRUE(setupChannelPair("16200", NULL, &userCookies,
        RsslWebSocketHttpCallbackTests::serverHttpCallbackHandler,
        NULL, (void*)1, NULL));

}

TEST_F(RsslWebSocketHttpCallbackTests, HttpCallback_with_server_cookies)
{
    RsslBuffer cookiesData[3] = { {69,  (char*)"session_id=xyz123abc; Secure; HttpOnly; SameSite=Strict; Max-Age=3600"},
                                           {25,  (char*)"id=a3fWa; Max-Age=2592000"},
                                           {30,  (char*)"id=fserfesrf1; Max-Age=2592000"} };

    RsslUserCookies userCookies = { const_cast<RsslBuffer*>(cookiesData), 3 };

    RsslWebSocketHttpCallbackTests::pServerUserCookies = &userCookies;

    RsslWebSocketHttpCallbackTests::pClientUserSpec = (void*)2;

    ASSERT_TRUE(setupChannelPair("16201", &userCookies, NULL, NULL,
        RsslWebSocketHttpCallbackTests::clientHttpCallbackHandler, NULL, (void*)2));
}

TEST_F(RsslWebSocketHttpCallbackTests, HttpCallback_with_client_request_memoryallocation_failed)
{
    RsslMallocFunc originalMallocFunc = rsslMallocFunc;
    g_customMallocInvokeCount = 0;

    /* Override the global allocator function pointer with our custom one. */
    rsslMallocFunc = customCountingMalloc;

    RsslError rsslError;

    rsslError.channel = NULL;
    rsslError.rsslErrorId = RSSL_RET_FAILURE;

    memset(rsslError.text, 0, MAX_RSSL_ERROR_TEXT);

    const char* text = "Failed to allocate memory for HTTP headers";
    strncpy(rsslError.text, text, strlen(text));

    RsslWebSocketHttpCallbackTests::pHttpCallbackError = &rsslError;

    g_customMallocAllocationFailedCount = 27;

    /* Expect the setupChannelPair() method to return FALSE from the memory allocation failure.*/
    ASSERT_FALSE(setupChannelPair("16202", NULL, NULL,
        RsslWebSocketHttpCallbackTests::serverHttpCallbackHandler,
        NULL, (void*)1, NULL));

    /* Restore the default allocator so subsequent tests are unaffected. */
    rsslMallocFunc = originalMallocFunc;
}

TEST_F(RsslWebSocketHttpCallbackTests, HttpCallback_with_server_response_memoryallocation_failed)
{
    RsslMallocFunc originalMallocFunc = rsslMallocFunc;
    g_customMallocInvokeCount = 0;

    /* Override the global allocator function pointer with our custom one. */
    rsslMallocFunc = customCountingMalloc;

    RsslError rsslError;

    rsslError.channel = NULL;
    rsslError.rsslErrorId = RSSL_RET_FAILURE;

    memset(rsslError.text, 0, MAX_RSSL_ERROR_TEXT);

    const char* text = "Failed to allocate memory for HTTP headers";
    strncpy(rsslError.text, text, strlen(text));

    RsslWebSocketHttpCallbackTests::pHttpCallbackError = &rsslError;

    g_customMallocAllocationFailedCount = 35;

    /* Expect the setupChannelPair() method to return FALSE from the memory allocation failure.*/
    ASSERT_FALSE(setupChannelPair("16203", NULL, NULL, NULL,
        RsslWebSocketHttpCallbackTests::clientHttpCallbackHandler, NULL, NULL));

    /* Restore the default allocator so subsequent tests are unaffected. */
    rsslMallocFunc = originalMallocFunc;
}

/* =======================================================================
 * WebSocket handshake with data body test
 * ===================================================================== */

/* Global(s) used only by the data-body handshake test below to capture
 * the dataBody delivered to the server-side httpCallback, since the
 * standard serverHttpCallbackHandler doesn't check for a data body. */
static const char* g_expectedHandshakeDataBody = NULL;
static bool        g_sawExpectedHandshakeDataBody = false;

/* httpCallback used solely to verify RsslHttpMessage::dataBody content when
 * a WebSocket GET handshake request is sent with a data body (e.g. a
 * Content-Length header and trailing payload). */
static void serverHttpCallbackWithDataBodyHandler(RsslHttpMessage* httpMessage, RsslError* pError)
{
    ASSERT_TRUE(httpMessage != NULL);
    ASSERT_EQ(0, httpMessage->statusCode);
    ASSERT_EQ(RSSL_HTTP_GET, httpMessage->httpMethod);

    g_sawExpectedHandshakeDataBody =
        (httpMessage->dataBody.data != NULL) &&
        (g_expectedHandshakeDataBody != NULL) &&
        (httpMessage->dataBody.length == strlen(g_expectedHandshakeDataBody)) &&
        (strncmp(httpMessage->dataBody.data, g_expectedHandshakeDataBody, httpMessage->dataBody.length) == 0);
}

/* Sends a raw WebSocket GET handshake request, including a Content-Length
 * header and a trailing data body, directly over a plain TCP socket
 * connected to the bound server.  This bypasses rsslConnect() (which does
 * not attach a body to the handshake) so that the server's httpCallback
 * can be verified to correctly parse and deliver a non-empty dataBody. */
TEST_F(RsslWebSocketHttpCallbackTests, HandshakeRequestWithDataBodyDeliveredToServer)
{
    const char* port = "16204";
    const char* body = "{\"test\":\"data\"}";

    g_expectedHandshakeDataBody = body;
    g_sawExpectedHandshakeDataBody = false;

    /* --- Bind server with the data-body-checking httpCallback ---------- */
    RsslError err;
    RsslBindOptions bindOpts;
    rsslClearBindOpts(&bindOpts);
    bindOpts.serviceName             = const_cast<char*>(port);
    bindOpts.connectionType          = RSSL_CONN_TYPE_WEBSOCKET;
    bindOpts.wsOpts.protocols        = const_cast<char*>(WS_JSON_V2_PROTOCOL);
    bindOpts.wsOpts.httpCallback     = serverHttpCallbackWithDataBodyHandler;
    bindOpts.majorVersion            = RSSL_RWF_MAJOR_VERSION;
    bindOpts.minorVersion            = RSSL_RWF_MINOR_VERSION;
    bindOpts.channelsBlocking        = RSSL_FALSE;
    bindOpts.serverBlocking          = RSSL_FALSE;
    bindOpts.guaranteedOutputBuffers = 5000;
    bindOpts.maxOutputBuffers        = 5000;

    pServer = rsslBind(&bindOpts, &err);
    ASSERT_NE(pServer, nullptr) << "rsslBind failed: " << err.text;

    /* --- Build and send a raw handshake request with a data body ------- */
    std::string requestBody(body);
    std::string handshake =
        "GET /WebSocket HTTP/1.1\r\n"
        "Host: localhost:" + std::string(port) + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Protocol: " + std::string(WS_JSON_V2_PROTOCOL) + "\r\n"
        "Content-Length: " + std::to_string(requestBody.length()) + "\r\n"
        "\r\n" + requestBody;

#if defined(_WIN32)
    SOCKET rawSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(rawSocket, INVALID_SOCKET) << "Failed to create raw socket";
#else
    int rawSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(rawSocket, 0) << "Failed to create raw socket";
#endif

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons((unsigned short)atoi(port));
#if defined(_WIN32)
    InetPtonA(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
#else
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
#endif

    int connectRet = connect(rawSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    ASSERT_EQ(connectRet, 0) << "Raw socket connect to server failed";

    int sendRet = send(rawSocket, handshake.c_str(), (int)handshake.length(), 0);
    ASSERT_GT(sendRet, 0) << "Raw socket send of handshake with data body failed";

    /* --- Drive rsslAccept + rsslInitChannel to process the handshake ---- */
    RsslAcceptOptions acceptOpts;
    rsslClearAcceptOpts(&acceptOpts);

    bool accepted = false;
    for (int i = 0; i < 500 && !accepted; ++i)
    {
        pServerChnl = rsslAccept(pServer, &acceptOpts, &err);
        if (pServerChnl)
        {
            accepted = true;
            break;
        }
        time_sleep(10);
    }
    ASSERT_TRUE(accepted) << "rsslAccept failed to accept the raw handshake connection: " << err.text;

    for (int i = 0; i < 500 && pServerChnl->state != RSSL_CH_STATE_ACTIVE; ++i)
    {
        RsslInProgInfo inProg;
        rsslClearInProgInfo(&inProg);
        rsslInitChannel(pServerChnl, &inProg, &err);
        time_sleep(10);
    }

    EXPECT_TRUE(g_sawExpectedHandshakeDataBody)
        << "Server-side httpCallback must receive the data body sent with the WebSocket handshake request";

#if defined(_WIN32)
    closesocket(rawSocket);
#else
    close(rawSocket);
#endif
}

