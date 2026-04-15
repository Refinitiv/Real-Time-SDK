/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *  Unit Tests for rwsWaitResponseHandshake
 *
 *  Tests negative and edge cases for the WebSocket client-side handshake
 *  response validation function rwsWaitResponseHandshake in rwsutils.c.
 *  This function is called by a WebSocket client to read and validate
 *  the server's HTTP 101 Switching Protocols response.
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

/* Include internal headers for testing internal functions */
extern "C" {
#include "rtr/ripc_int.h"
#include "rtr/rwsutils.h"
#include "rtr/cutilsmplcbuffer.h"
#include "rtr/cutildfltcbuffer.h"
}

/*
 * Test fixture for rwsWaitResponseHandshake unit tests
 */
class RwsWaitResponseHandshakeTests : public ::testing::Test {

public:
    /* Buffer that the mock readTransport will return to the caller */
    char readTransportBuffer[RWS_MAX_HTTP_HEADER_SIZE];
    int readTransportBufferLength;

protected:
    RsslError error;
    rtr_bufferpool_t* bufferPool;
    rtr_msgb_t* inputBuffer;
    RsslSocketChannel* pRsslSocketChannel;

    /* Mock function for readTransport - returns pre-loaded response data */
    static int mockReadTransport(void* transportInfo, char* buf, int outLen, ripcRWFlags flags, RsslError* error)
    {
        RwsWaitResponseHandshakeTests* pTests = (RwsWaitResponseHandshakeTests*)transportInfo;

        if (pTests->readTransportBufferLength <= 0)
            return 0;

        int bytesToCopy = pTests->readTransportBufferLength;
        if (bytesToCopy > outLen)
            bytesToCopy = outLen;

        memcpy(buf, pTests->readTransportBuffer, bytesToCopy);
        return bytesToCopy;
    }

    /* Mock function for readTransport - simulates read error */
    static int mockReadTransportError(void* transportInfo, char* buf, int outLen, ripcRWFlags flags, RsslError* error)
    {
        return -1;
    }

    /* Mock function for readTransport - simulates no data available (WOULD_BLOCK) */
    static int mockReadTransportNoData(void* transportInfo, char* buf, int outLen, ripcRWFlags flags, RsslError* error)
    {
        return 0;
    }

    static ripcTransportFuncs mockTransportFuncs;
    static ripcTransportFuncs mockTransportFuncsError;
    static ripcTransportFuncs mockTransportFuncsNoData;

    virtual void SetUp() override
    {
        memset(&error, 0, sizeof(RsslError));
        bufferPool = NULL;
        inputBuffer = NULL;
        pRsslSocketChannel = NULL;
        memset(readTransportBuffer, 0, RWS_MAX_HTTP_HEADER_SIZE);
        readTransportBufferLength = 0;
        rsslInitialize(RSSL_LOCK_NONE, &error);
    }

    virtual void TearDown() override
    {
        if (inputBuffer != NULL && bufferPool != NULL)
        {
            rtr_smplcFreeMsg(inputBuffer);
            inputBuffer = NULL;
        }
        if (bufferPool != NULL)
        {
            rtr_smplcDropRef(bufferPool);
            bufferPool = NULL;
        }
        if (pRsslSocketChannel != NULL)
        {
            if (pRsslSocketChannel->rwsSession != NULL)
            {
                rwsReleaseSession((rwsSession_t*)pRsslSocketChannel->rwsSession);
                free(pRsslSocketChannel->rwsSession);
                pRsslSocketChannel->rwsSession = NULL;
            }

            if (pRsslSocketChannel->guarBufPool != NULL)
            {
                rtr_dfltcDropRef(&pRsslSocketChannel->guarBufPool->bufpool);
                pRsslSocketChannel->guarBufPool = NULL;
            }

            free(pRsslSocketChannel);
            pRsslSocketChannel = NULL;
        }
        rsslUninitialize();
    }

    /* Helper function to create an input buffer using rtr_smplcAllocMsg */
    rtr_msgb_t* createInputBuffer(size_t size)
    {
        if (inputBuffer != NULL && bufferPool != NULL)
        {
            rtr_smplcFreeMsg(inputBuffer);
            inputBuffer = NULL;
        }
        if (bufferPool != NULL)
        {
            rtr_smplcDropRef(bufferPool);
            bufferPool = NULL;
        }

        bufferPool = rtr_smplcAllocatePool(NULL);
        if (bufferPool == NULL)
            return NULL;

        inputBuffer = rtr_smplcAllocMsg(bufferPool, size);

        return inputBuffer;
    }

    /* Helper function to initialize RsslSocketChannel with mock transport and a pre-created rwsSession */
    RsslSocketChannel* initSocketChannelForClient(ripcTransportFuncs* transportFuncs)
    {
        pRsslSocketChannel = (RsslSocketChannel*)malloc(sizeof(RsslSocketChannel));
        if (pRsslSocketChannel == NULL)
            return NULL;

        ripcClearRsslSocketChannel(pRsslSocketChannel);
        pRsslSocketChannel->transportFuncs = transportFuncs;
        pRsslSocketChannel->transportInfo = this;

        /* Allocate guarBufPool same as ipcCreatePool in rsslSocketConnect:
         * rtr_dfltcAllocPool(1, max_bufs, 10, 0, 0, mutex) */
        RsslInt32 numGuarOutputBufs = 10;
        rtr_dfltcbufferpool_t* cpool = rtr_dfltcAllocPool(numGuarOutputBufs, numGuarOutputBufs, 10, 0, 0, NULL);
        if (cpool == NULL)
        {
            free(pRsslSocketChannel);
            pRsslSocketChannel = NULL;
            return NULL;
        }
        pRsslSocketChannel->guarBufPool = cpool;

        /* Allocate and initialize rwsSession for client side */
        rwsSession_t* wsSess = (rwsSession_t*)malloc(sizeof(rwsSession_t));
        if (wsSess == NULL)
        {
            rtr_dfltcDropRef(&pRsslSocketChannel->guarBufPool->bufpool);
            free(pRsslSocketChannel);
            pRsslSocketChannel = NULL;
            return NULL;
        }
        rwsClearSession(wsSess);
        wsSess->isClient = RSSL_TRUE;

		/* Set a known key for testing Sec-WebSocket-Accept validation */
        size_t len = strlen("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
		wsSess->keyAccept.length = (RsslUInt32)len;
		wsSess->keyAccept.data = (char*)malloc(len + 1);
        if (wsSess->keyAccept.data == 0)
            return NULL;

        memset(wsSess->keyAccept.data, 0, len + 1);
		memcpy(wsSess->keyAccept.data, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", len);
        pRsslSocketChannel->rwsSession = wsSess;

        return pRsslSocketChannel;
    }

    /* Load a server response into the mock read buffer */
    void loadReadBuffer(const char* response)
    {
        size_t len = strlen(response);
        memcpy(readTransportBuffer, response, len);
        readTransportBufferLength = (int)len;
    }

    /* Helper: valid 101 Switching Protocols response with Sec-WebSocket-Accept for key "dGhlIHNhbXBsZSBub25jZQ==" */
    static const char* getValidSwitchingProtocolsResponse()
    {
        return "HTTP/1.1 101 Switching Protocols\r\n"
               "Upgrade: websocket\r\n"
               "Connection: Upgrade\r\n"
               "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
               "Sec-Websocket-Protocol: rssl.json.v2\r\n"
               "\r\n";
    }

    /* Helper: create a response with a specific field missing */
    static std::string createResponseWithMissingField(const char* fieldToOmit)
    {
        std::string response = "HTTP/1.1 101 Switching Protocols\r\n";

        if (strcmp(fieldToOmit, "Upgrade") != 0)
            response += "Upgrade: websocket\r\n";
        if (strcmp(fieldToOmit, "Connection") != 0)
            response += "Connection: Upgrade\r\n";
        if (strcmp(fieldToOmit, "Sec-Websocket-Accept") != 0)
            response += "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n";
        if (strcmp(fieldToOmit, "Sec-Websocket-Protocol") != 0)
            response += "Sec-Websocket-Protocol: rssl.json.v2\r\n";

        response += "\r\n";
        return response;
    }

    /* Helper: create a response with a specific field replaced */
    static std::string createResponseWithReplacedField(const char* fieldToReplace, const char* newValue)
    {
        std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n";

        if (strcmp(fieldToReplace, "Sec-Websocket-Accept") == 0)
            response += std::string("Sec-Websocket-Accept: ") + newValue + "\r\n";
        else
            response += "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n";

        if (strcmp(fieldToReplace, "Sec-Websocket-Protocol") == 0)
            response += std::string("Sec-Websocket-Protocol: ") + newValue + "\r\n";
        else
            response += "Sec-Websocket-Protocol: rssl.json.v2\r\n";

        response += "\r\n";
        return response;
    }
};

/* Static member initialization for mock transport functions */
ripcTransportFuncs RwsWaitResponseHandshakeTests::mockTransportFuncs = {
    NULL,                                                      /* bindSrvr */
    NULL,                                                      /* newSrvrConnection */
    NULL,                                                      /* connectSocket */
    NULL,                                                      /* newClientConnection */
    NULL,                                                      /* initializeTransport */
    NULL,                                                      /* shutdownTransport */
    RwsWaitResponseHandshakeTests::mockReadTransport,          /* readTransport */
    NULL,                                                      /* writeTransport */
    NULL,                                                      /* writeVTransport */
    NULL,                                                      /* reconnectClient */
    NULL,                                                      /* acceptSocket */
    NULL,                                                      /* shutdownSrvrError */
    NULL,                                                      /* sessIoctl */
    NULL,                                                      /* getSockName */
    NULL,                                                      /* setSockOpts */
    NULL,                                                      /* getSockOpts */
    NULL,                                                      /* connected */
    NULL,                                                      /* shutdownServer */
    NULL                                                       /* uninitialize */
};

ripcTransportFuncs RwsWaitResponseHandshakeTests::mockTransportFuncsError = {
    NULL,                                                      /* bindSrvr */
    NULL,                                                      /* newSrvrConnection */
    NULL,                                                      /* connectSocket */
    NULL,                                                      /* newClientConnection */
    NULL,                                                      /* initializeTransport */
    NULL,                                                      /* shutdownTransport */
    RwsWaitResponseHandshakeTests::mockReadTransportError,     /* readTransport */
    NULL,                                                      /* writeTransport */
    NULL,                                                      /* writeVTransport */
    NULL,                                                      /* reconnectClient */
    NULL,                                                      /* acceptSocket */
    NULL,                                                      /* shutdownSrvrError */
    NULL,                                                      /* sessIoctl */
    NULL,                                                      /* getSockName */
    NULL,                                                      /* setSockOpts */
    NULL,                                                      /* getSockOpts */
    NULL,                                                      /* connected */
    NULL,                                                      /* shutdownServer */
    NULL                                                       /* uninitialize */
};

ripcTransportFuncs RwsWaitResponseHandshakeTests::mockTransportFuncsNoData = {
    NULL,                                                      /* bindSrvr */
    NULL,                                                      /* newSrvrConnection */
    NULL,                                                      /* connectSocket */
    NULL,                                                      /* newClientConnection */
    NULL,                                                      /* initializeTransport */
    NULL,                                                      /* shutdownTransport */
    RwsWaitResponseHandshakeTests::mockReadTransportNoData,    /* readTransport */
    NULL,                                                      /* writeTransport */
    NULL,                                                      /* writeVTransport */
    NULL,                                                      /* reconnectClient */
    NULL,                                                      /* acceptSocket */
    NULL,                                                      /* shutdownSrvrError */
    NULL,                                                      /* sessIoctl */
    NULL,                                                      /* getSockName */
    NULL,                                                      /* setSockOpts */
    NULL,                                                      /* getSockOpts */
    NULL,                                                      /* connected */
    NULL,                                                      /* shutdownServer */
    NULL                                                       /* uninitialize */
};

/*
 * Test Case: Null rsslSocketChannel pointer
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, NullChannel)
{
    RsslSocketChannel* pChannel = nullptr;

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when rsslSocketChannel is NULL";
    EXPECT_TRUE(strstr(error.text, "Error: 1001 rsslSocketChannel is NULL") != NULL) << "Error message should mention NULL";
}

/*
 * Test Case: Null rwsSession pointer
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, NullRwsSession)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    /* Free the rwsSession and set to NULL to test null check */
    rwsReleaseSession((rwsSession_t*)pChannel->rwsSession);
    free(pChannel->rwsSession);
    pChannel->rwsSession = NULL;

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when rwsSession is NULL";
    EXPECT_TRUE(strstr(error.text, "Error: 1001 wsSess is NULL") != NULL) << "Error message should mention NULL";
}

/*
 * Test Case: Read transport returns error (negative return)
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, ReadTransportError)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncsError);
    ASSERT_NE(pChannel, nullptr);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when readTransport returns error";
    EXPECT_TRUE(strstr(error.text, "Error: 1002 Could not read IPC Mount Ack.  Connection attempt has failed.") != NULL) << "Error message should mention read failure";
}

/*
 * Test Case: Read transport returns zero bytes (no data, WOULD_BLOCK)
 * Expected: Returns RIPC_CONN_IN_PROGRESS
 */
TEST_F(RwsWaitResponseHandshakeTests, ReadTransportNoData)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncsNoData);
    ASSERT_NE(pChannel, nullptr);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_IN_PROGRESS) << "Expected RIPC_CONN_IN_PROGRESS when no data available";
}

/*
 * Test Case: Valid HTTP 101 Switching Protocols response
 * Expected: Returns RIPC_CONN_ACTIVE
 */
TEST_F(RwsWaitResponseHandshakeTests, CompleteHttpResponse)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

	std::string response = getValidSwitchingProtocolsResponse();
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ACTIVE) << "Expected RIPC_CONN_ACTIVE for complete response";
}


/*
 * Test Case: Incomplete HTTP response (no terminating CRLF CRLF)
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, IncompleteHttpResponse)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* incompleteHttpResponse = "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
        "Sec-Websocket-Protocol: rssl.json.v2\r\n";

    loadReadBuffer(incompleteHttpResponse);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete HTTP response message";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP header, the end of HTTP header is not found") != NULL) << "Error message should mention end of HTTP header is not found";
}

/*
 * Test Case: HTTP response with non-101 status code (e.g., 400 Bad Request)
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, Non101StatusCode)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 400 Bad Request\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: close\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for non-101 status code";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 400 ") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: HTTP 403 Forbidden response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, Http403Forbidden)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 403 Forbidden\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: close\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for HTTP 403 response";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 403 ") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: HTTP 500 Internal Server Error response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, Http500InternalServerError)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 500 Internal Server Error\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: close\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for HTTP 500 response";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 500 ") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: Missing Upgrade header in 101 response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, MissingUpgradeField)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithMissingField("Upgrade");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Upgrade field is missing";
    EXPECT_TRUE(strstr(error.text, "No Upgrade: key received. ") != NULL) << "Error message should mention missing Upgrade";
}

/*
 * Test Case: Missing Connection header in 101 response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, MissingConnectionField)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithMissingField("Connection");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Connection field is missing";
    EXPECT_TRUE(strstr(error.text, "No Connection: key received. ") != NULL) << "Error message should mention missing Connection";
}

/*
 * Test Case: Missing Sec-Websocket-Accept header in 101 response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, MissingSecWebSocketAcceptField)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithMissingField("Sec-Websocket-Accept");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Sec-Websocket-Accept is missing";
    EXPECT_TRUE(strstr(error.text, "No Sec-Websocket-Accept: key received, key expected 's3pPLMBiTxaQ9kYGzzhZRbK+xOo='. ") != NULL || strstr(error.text, "key") != NULL) << "Error message should mention missing Sec-Websocket-Accept";
}

/*
 * Test Case: Missing Sec-Websocket-Protocol header in 101 response
 * Expected: Returns RIPC_CONN_ERROR (protocol is required for RSSL)
 */
TEST_F(RwsWaitResponseHandshakeTests, MissingSecWebSocketProtocolField)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithMissingField("Sec-Websocket-Protocol");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Sec-Websocket-Protocol is missing";
    EXPECT_TRUE(strstr(error.text, "Unsupported Websocket protocol type received(-1)") != NULL) << "Error message should mention unsupported protocol";
}

/*
 * Test Case: Unsupported sub-protocol in server response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, UnsupportedSubProtocol)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithReplacedField("Sec-Websocket-Protocol", "unknown.protocol");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for unsupported sub-protocol";
    EXPECT_TRUE(strstr(error.text, "Error with protocol response ") != NULL) << "Error message should mention unsupported protocol";
}

/*
 * Test Case: Malformed HTTP response - garbage data
 * Expected: Returns RIPC_CONN_ERROR or handles gracefully
 */
TEST_F(RwsWaitResponseHandshakeTests, MalformedGarbageResponse)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    /* Construct garbage data terminated with CRLFCRLF */
    char garbage[68];
    for (int i = 0; i < 64; i++)
        garbage[i] = (char)(i + 128);
    garbage[64] = '\r';
    garbage[65] = '\n';
    garbage[66] = '\r';
    garbage[67] = '\n';

    memcpy(readTransportBuffer, garbage, 68);
    readTransportBufferLength = 68;

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for garbage response data";
    EXPECT_TRUE(strstr(error.text, "Bad HTTP status/request line") != NULL) << "Error message should mention bad HTTP status";
}

/*
 * Test Case: Non-HTTP response (plain text)
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, NonHttpResponse)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "This is not an HTTP response\r\n\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for non-HTTP response";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: HTTP response with wrong HTTP version
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, WrongHttpVersion)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/2.0 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for wrong HTTP version in response";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: Channel shutdown pending during read
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, ChannelShutdownPending)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncsNoData);
    ASSERT_NE(pChannel, nullptr);

    /* Set the shutdown pending flag */
    pChannel->workState |= RIPC_INT_SHTDOWN_PEND;

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when channel shutdown is pending";
    EXPECT_TRUE(strstr(error.text, "Error: 1003 failed due to channel shutting down.") != NULL) << "Error message should mention channel shutting down";
}

/*
 * Test Case: Empty response body with valid status line only
 * Expected: Returns RIPC_CONN_ERROR (missing required headers)
 */
TEST_F(RwsWaitResponseHandshakeTests, StatusLineOnlyNoHeaders)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\r\n\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when required headers are missing in 101 response";
    EXPECT_TRUE(strstr(error.text, "No Upgrade: key received.") != NULL) << "Error message should mention no upgrade key received";
}

/*
 * Test Case: Response with invalid Upgrade value (not "websocket")
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, InvalidUpgradeValue)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: http2\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for invalid Upgrade value";
    EXPECT_TRUE(strstr(error.text, "No Upgrade: key received.") != NULL) << "Error message should mention no upgrade key received";
}

/*
 * Test Case: Response with invalid Connection value (not "Upgrade")
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, InvalidConnectionValue)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: keep-alive\r\n"
                           "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for invalid Connection value";
    EXPECT_TRUE(strstr(error.text, "No Connection: key received.") != NULL) << "Error message should mention no connection key received";
}

/*
 * Test Case: HTTP 301 redirect response
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, HttpRedirectResponse)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 301 Moved Permanently\r\n"
                           "Location: wss://newhost:15000/WebSocket\r\n"
                           "Connection: close\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for HTTP redirect response";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 301") != NULL) << "Error message should mention invalid HTTP response";
}

/*
 * Test Case: Very long response header value
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsWaitResponseHandshakeTests, VeryLongHeaderValue)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string longValue(8000, 'X');
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\r\n"
                           "X-Custom-Header: " + longValue + "\r\n"
                           "\r\n";
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    /* Should not crash regardless of result */
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle very long header values gracefully without crash";
}

/*
 * Test Case: Response with LF line endings instead of CRLF
 * Expected: Returns RIPC_CONN_ACTIVE
 */
TEST_F(RwsWaitResponseHandshakeTests, LfLineEndingsInsteadOfCrlf)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\n"
                           "Upgrade: websocket\n"
                           "Connection: Upgrade\n"
                           "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\n"
                           "\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ACTIVE) << "Expected RIPC_CONN_ACTIVE for LF line endings";
}

/*
 * Test Case: Response header without colon separator
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, HeaderWithoutColonSeparator)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for header line without colon";
    EXPECT_TRUE(strstr(error.text, "Improper formatted Field. Missing ':'") != NULL) << "Error message should mention improper formatted field";
}

/*
 * Test Case: Response with empty Sec-Websocket-Accept value
 * Expected: Returns RIPC_CONN_ERROR
 */
TEST_F(RwsWaitResponseHandshakeTests, EmptySecWebSocketAcceptValue)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    std::string response = createResponseWithReplacedField("Sec-Websocket-Accept", "");
    loadReadBuffer(response.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for empty Sec-Websocket-Accept";
    EXPECT_TRUE(strstr(error.text, "Key received '' is not key expected 's3pPLMBiTxaQ9kYGzzhZRbK+xOo='") != NULL) << "Error message should mention not key expected";
}

/*
 * Test Case: Format string attack in response header value
 * Expected: Should not interpret format specifiers, no crash
 */
TEST_F(RwsWaitResponseHandshakeTests, FormatStringAttackInHeader)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    const char* response = "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-Websocket-Accept: %s%s%s%s%n%n%n\r\n"
                           "Sec-Websocket-Protocol: rssl.json.v2\r\n"
                           "\r\n";
    loadReadBuffer(response);

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    /* Should not crash - the accept key won't match, so expect error */
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle format string attack gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Key received 's' is not key expected 's3pPLMBiTxaQ9kYGzzhZRbK+xOo='") != NULL) << "Error message should mention not key expected";
}

/*
 * Test Case: Response with embedded null bytes
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsWaitResponseHandshakeTests, EmbeddedNullBytes)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    /* Build a response with null bytes interspersed */
    char response[256];
    memset(response, 0, sizeof(response));
    for (int i = 0; i < 252; i++)
        response[i] = (i % 2 == 0) ? 0x00 : (char)0x41;
    response[252] = '\r';
    response[253] = '\n';
    response[254] = '\r';
    response[255] = '\n';

    memcpy(readTransportBuffer, response, 256);
    readTransportBufferLength = 256;

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    /* Should handle gracefully - expected error since data is not valid HTTP */
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle embedded null bytes gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Bad HTTP status/request line") != NULL) << "Error message should mention bad HTTP status";
}

/*
 * Test Case: Very long HTTP status code (buffer overflow attempt)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsWaitResponseHandshakeTests, VeryLongHttpStatusCode)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    // Very long HTTP status code
    std::string longStatusCode(55, '1');
    std::string incompleteHttpResponse = "HTTP/1.1 " + longStatusCode + "Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-Websocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"
        "Sec-Websocket-Protocol: rssl.json.v2\r\n"
		"\r\n";

    loadReadBuffer(incompleteHttpResponse.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete HTTP response message";

    /* This is platform specific output due to the behavior of the atoi() function. */
#if defined(_WIN32)
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 2147483647 ") != NULL) << "Error message should mention invalid HTTP response";
#else
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code -1 ") != NULL) << "Error message should mention invalid HTTP response";
#endif
}

/*
 * Test Case: Very long websocket version (buffer overflow attempt)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsWaitResponseHandshakeTests, VeryLongWebSocketVersion)
{
    RsslSocketChannel* pChannel = initSocketChannelForClient(&mockTransportFuncs);
    ASSERT_NE(pChannel, nullptr);

    // Very long WebSocket version
    std::string longWebsocketVersion(55, '1');
    std::string incompleteHttpResponse = "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Cache-Control: no-cache, private, no-store\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Sec-WebSocket-Version: " + longWebsocketVersion + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    loadReadBuffer(incompleteHttpResponse.c_str());

    ripcSessInProg inProgress;
    memset(&inProgress, 0, sizeof(ripcSessInProg));

    ripcSessInit result = rwsWaitResponseHandshake(pChannel, &inProgress, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete HTTP response message";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP response, status code 400") != NULL) << "Error message should mention invalid HTTP response";
}
