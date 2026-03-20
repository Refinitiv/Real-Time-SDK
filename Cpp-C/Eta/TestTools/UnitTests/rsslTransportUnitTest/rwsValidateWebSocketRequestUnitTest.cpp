/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *  Unit Tests for rwsValidateWebSocketRequest
 *
 *  Tests negative and edge cases for the WebSocket handshake validation
 *  function rwsValidateWebSocketRequest in rwsutils.c
 *
 /**********************************************************************/

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
}

/* Mock function for writeTransport - simulates a successful write */
static int mockWriteTransport(void* transportInfo, char* buf, int outLen, ripcRWFlags flags, RsslError* error); // forward declaration

/* Mock transport functions structure with the mock writeTransport */
static ripcTransportFuncs mockTransportFuncs = {
    NULL,                   /* bindSrvr */
    NULL,                   /* newSrvrConnection */
    NULL,                   /* connectSocket */
    NULL,                   /* newClientConnection */
    NULL,                   /* initializeTransport */
    NULL,                   /* shutdownTransport */
    NULL,                   /* readTransport */
    mockWriteTransport,     /* writeTransport */
    NULL,                   /* writeVTransport */
    NULL,                   /* reconnectClient */
    NULL,                   /* acceptSocket */
    NULL,                   /* shutdownSrvrError */
    NULL,                   /* sessIoctl */
    NULL,                   /* getSockName */
    NULL,                   /* setSockOpts */
    NULL,                   /* getSockOpts */
    NULL,                   /* connected */
    NULL,                   /* shutdownServer */
    NULL                    /* uninitialize */
};

/*
 * Test fixture for rwsValidateWebSocketRequest unit tests
 */
class RwsValidateWebSocketRequestTests : public ::testing::Test {

public:
    char writeTransportBuffer[RWS_MAX_HTTP_HEADER_SIZE];
    int writeTransportBufferLength;

protected:
    RsslError error;
    rtr_bufferpool_t* bufferPool;
    rtr_msgb_t* inputBuffer;
	RsslSocketChannel* pRsslSocketChannel; // Cleaning up the socket channel after each test to avoid memory leaks
    
    virtual void SetUp() override
    {
        memset(&error, 0, sizeof(RsslError));
        bufferPool = NULL;
        inputBuffer = NULL;
        rsslInitialize(RSSL_LOCK_NONE, &error);
        memset(&writeTransportBuffer, 0, RWS_MAX_HTTP_HEADER_SIZE);
		pRsslSocketChannel = NULL;
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
        
        /* Allocate the buffer pool */
        bufferPool = rtr_smplcAllocatePool(NULL);
        if (bufferPool == NULL)
            return NULL;
        
        /* Allocate the message buffer from the pool */
        inputBuffer = rtr_smplcAllocMsg(bufferPool, size);
        
        return inputBuffer;
    }

    /* Helper function to initialize RsslSocketChannel with mock transport functions and input buffer */
    RsslSocketChannel* initSocketChannelWithMock(RsslServerSocketChannel* serverSocketChannel, size_t inputBufferSize)
    {
        pRsslSocketChannel = (RsslSocketChannel*)malloc(sizeof(RsslSocketChannel));

        if(pRsslSocketChannel == NULL)
			return NULL;

		ripcClearRsslSocketChannel(pRsslSocketChannel);
        pRsslSocketChannel->server = serverSocketChannel;
        pRsslSocketChannel->transportFuncs = &mockTransportFuncs;
        pRsslSocketChannel->inputBuffer = createInputBuffer(inputBufferSize);
        pRsslSocketChannel->transportInfo = this;
        return pRsslSocketChannel;
    }

    /* Helper function to create a modifiable buffer from a handshake string */
    static char* createModifiableHandshake(const char* handshake, RsslInt32* outLen)
    {
        size_t len = strlen(handshake);
        char* buffer = (char*)malloc(len + 1);
        if (buffer)
        {
            memcpy(buffer, handshake, len + 1);
            *outLen = (RsslInt32)len;
        }
        return buffer;
    }

    /* Helper function to create a minimal valid WebSocket handshake request */
    static const char* getValidWebSocketHandshake()
    {
        return "GET /WebSocket HTTP/1.1\r\n"
               "Host: localhost:15000\r\n"
               "Upgrade: websocket\r\n"
               "Connection: Upgrade\r\n"
               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
               "Sec-WebSocket-Version: 13\r\n"
               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
               "\r\n";
    }

    /* Create a handshake with a missing required header field */
    static std::string createHandshakeWithMissingField(const char* fieldToOmit)
    {
        std::string handshake = "GET /WebSocket HTTP/1.1\r\n";
        
        if (strcmp(fieldToOmit, "Host") != 0)
            handshake += "Host: localhost:15000\r\n";
        if (strcmp(fieldToOmit, "Upgrade") != 0)
            handshake += "Upgrade: websocket\r\n";
        if (strcmp(fieldToOmit, "Connection") != 0)
            handshake += "Connection: Upgrade\r\n";
        if (strcmp(fieldToOmit, "Sec-WebSocket-Key") != 0)
            handshake += "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n";
        if (strcmp(fieldToOmit, "Sec-WebSocket-Version") != 0)
            handshake += "Sec-WebSocket-Version: 13\r\n";
        if (strcmp(fieldToOmit, "Sec-WebSocket-Protocol") != 0)
            handshake += "Sec-WebSocket-Protocol: rssl.json.v2\r\n";
        
        handshake += "\r\n";
        return handshake;
    }

    /* Create a handshake with an invalid value for a header field */
    static std::string createHandshakeWithInvalidValue(const char* fieldToInvalidate, const char* invalidValue)
    {
        std::string handshake = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n";
        
        if (strcmp(fieldToInvalidate, "Sec-WebSocket-Version") == 0)
            handshake += std::string("Sec-WebSocket-Version: ") + invalidValue + "\r\n";
        else
            handshake += "Sec-WebSocket-Version: 13\r\n";
        
        if (strcmp(fieldToInvalidate, "Sec-WebSocket-Protocol") == 0)
            handshake += std::string("Sec-WebSocket-Protocol: ") + invalidValue + "\r\n";
        else
            handshake += "Sec-WebSocket-Protocol: rssl.json.v2\r\n";
        
        handshake += "\r\n";
        return handshake;
    }
};

/* Mock function for writeTransport - simulates a successful write */
static int mockWriteTransport(void* transportInfo, char* buf, int outLen, ripcRWFlags flags, RsslError* error)
{
    /* Simply copy write data and return the length to indicate successful write */
    RwsValidateWebSocketRequestTests* pValidateWebSocketRequestTests = (RwsValidateWebSocketRequestTests*)transportInfo;

    memcpy(&pValidateWebSocketRequestTests->writeTransportBuffer, buf, outLen);
    pValidateWebSocketRequestTests->writeTransportBufferLength = outLen;
    return outLen;
}


/* Default input buffer size for tests */
static const size_t DEFAULT_INPUT_BUFFER_SIZE = 4096;

/*
 * Test Case: Null server rwsServer pointer
 * Expected: Returns RIPC_CONN_ERROR with RSSL_WS_REJECT_CONN_ERROR
 */
TEST_F(RwsValidateWebSocketRequestTests, NullServerRwsServer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    serverSocketChannel.rwsServer = NULL; // Null rwsServer

	RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when rwsServer is NULL";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request format (not a GET request)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpMethod)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // POST instead of GET - use modifiable buffer
    const char* handshakeStr = "POST /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for invalid HTTP method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL);
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Empty request buffer
 * Expected: Returns RIPC_CONN_ERROR (incomplete header)
 */
TEST_F(RwsValidateWebSocketRequestTests, EmptyRequestBuffer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    char handshake[4] = "GET";
    RsslInt32 len = 3;
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete header ";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
}

/*
 * Test Case: Incomplete HTTP header (no terminating CRLF CRLF)
 * Expected: Returns RIPC_CONN_IN_PROGRESS (waiting for more data)
 */
TEST_F(RwsValidateWebSocketRequestTests, IncompleteHttpHeader)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Missing final CRLF - use modifiable buffer
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
        "Host: localhost:15000\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Protocol: rssl.json.v2\r\n";


    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete header ";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Missing Host header field
 * Expected: Returns RIPC_CONN_ERROR with rejection
 */
TEST_F(RwsValidateWebSocketRequestTests, MissingHostField)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    std::string handshakeStr = createHandshakeWithMissingField("Host");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Host field is missing";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Host field.")) << "Error message should mention missing Host field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n", 
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Missing Upgrade header field
 * Expected: Returns RIPC_CONN_ERROR with rejection
 */
TEST_F(RwsValidateWebSocketRequestTests, MissingUpgradeField)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    std::string handshakeStr = createHandshakeWithMissingField("Upgrade");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Upgrade field is missing";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Upgrade field.") != NULL) << "Error message should mention missing Upgrade field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Missing Connection header field  
 * Expected: Returns RIPC_CONN_ERROR with rejection
 */
TEST_F(RwsValidateWebSocketRequestTests, MissingConnectionField)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    std::string handshakeStr = createHandshakeWithMissingField("Connection");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Connection field is missing";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Connection field.") != NULL) << "Error message should mention missing Connection field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Missing Sec-WebSocket-Key header field
 * Expected: Returns RIPC_CONN_ERROR with rejection
 */
TEST_F(RwsValidateWebSocketRequestTests, MissingSecWebSocketKeyField)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    std::string handshakeStr = createHandshakeWithMissingField("Sec-WebSocket-Key");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR when Sec-WebSocket-Key is missing";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Sec-WebSocket-Key field.") != NULL) << "Error message should mention missing Sec-WebSocket-Key field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Unsupported WebSocket version
 * Expected: Returns RIPC_CONN_ERROR with RSSL_WS_REJECT_UNSUPPORTED_VERSION
 */
TEST_F(RwsValidateWebSocketRequestTests, UnsupportedWebSocketVersion)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Use version 12 instead of 13
    std::string handshakeStr = createHandshakeWithInvalidValue("Sec-WebSocket-Version", "12");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for unsupported WebSocket version";
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version (12), version supported (13)") != NULL) << "Error message should mention version mismatch";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nSec-WebSocket-Version: 13\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Unsupported sub-protocol
 * Expected: Returns RIPC_CONN_ERROR with RSSL_WS_REJECT_UNSUPPORTED_SUB_PROTOCOL
 */
TEST_F(RwsValidateWebSocketRequestTests, UnsupportedSubProtocol)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Use unknown protocol
    std::string handshakeStr = createHandshakeWithInvalidValue("Sec-WebSocket-Protocol", "unknown.protocol");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for unsupported sub-protocol";
    EXPECT_TRUE(strstr(error.text, "Unsupported Websocket protocol type(-1)")) << "Error message should mention unsupported protocol";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Empty sub-protocol
 * Expected: Returns RIPC_CONN_ERROR with RSSL_WS_REJECT_UNSUPPORTED_SUB_PROTOCOL
 */
TEST_F(RwsValidateWebSocketRequestTests, EmptySubProtocol)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));

    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    // Use unknown protocol
    std::string handshakeStr = createHandshakeWithInvalidValue("Sec-WebSocket-Protocol", "");
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);

    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);

    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for unsupported sub-protocol";
    EXPECT_TRUE(strstr(error.text, "Unsupported Websocket protocol type(-1)")) << "Error message should mention unsupported protocol";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Test Case: Missing required headers (Host, Upgrade, Connection, Sec-WebSocket-Key, Sec-WebSocket-Version, Sec-WebSocket-Protocol)
 * Expected: Returns RIPC_CONN_ERROR with detailed rejection reason
 */
TEST_F(RwsValidateWebSocketRequestTests, MissingRequiredHeaders)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "\r\n"; // Missing Connection, Sec-WebSocket-Key, Sec-WebSocket-Version, Sec-WebSocket-Protocol
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR with reason for missing headers";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Connection field.")) << "Error message should mention missing Connection field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (non-existent HTTP version)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineHttpVersion)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Invalid HTTP version
    const char* handshakeStr = "GET /WebSocket HTTP/2.0\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for invalid HTTP version";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (missing method)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineMissingMethod)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Missing HTTP method
    const char* handshakeStr = "/WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for missing HTTP method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received")) << "Error message should mention missing HTTP method";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (too many spaces)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineTooManySpaces)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Too many spaces in request line
    const char* handshakeStr = "GET      /WebSocket      HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for invalid request line format";
    EXPECT_TRUE(strstr(error.text, "Bad HTTP status/request line") != NULL) << "Error message should mention invalid request line format";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (non-absolute URI)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineNonAbsoluteUri)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Non-absolute URI (missing scheme)
    const char* handshakeStr = "GET WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for non-absolute URI";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL) << "Error message should mention invalid URI";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (unsupported HTTP method)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineUnsupportedMethod)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // PATCH method instead of GET
    const char* handshakeStr = "PATCH /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for disallowed PATCH method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL) << "Error message should mention invalid URI";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Invalid HTTP request line (LF instead of CRLF)
 * Expected: Returns RIPC_CONN_ERROR due to parsing failure
 */
TEST_F(RwsValidateWebSocketRequestTests, InvalidHttpRequestLineLf)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // LF instead of CRLF
    const char* handshakeStr = "GET /WebSocket\n"
                               "Host: localhost:15000\n"
                               "\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for LF line endings";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid line endings";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Malformed GET request - binary garbage data
 * Expected: Returns RIPC_CONN_ERROR or handles gracefully
 */
TEST_F(RwsValidateWebSocketRequestTests, MalformedBinaryGarbage)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Binary garbage data
    char handshake[64];
    for (int i = 0; i < 60; i++)
    {
        handshake[i] = (char)(i + 128); // Non-ASCII bytes
    }
    handshake[60] = '\r';
    handshake[61] = '\n';
    handshake[62] = '\r';
    handshake[63] = '\n';
    RsslInt32 len = 64;
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    // Should handle binary garbage gracefully
    EXPECT_TRUE(result == RIPC_CONN_ERROR || result == RIPC_CONN_IN_PROGRESS) << "Should handle binary garbage data gracefully";
    EXPECT_TRUE(strstr(error.text, "Bad HTTP status/request line") != NULL) << "Error message should mention invalid request line format";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
}

/*
 * Test Case: Malformed GET request - URI with special characters
 * Expected: Should handle or reject gracefully
 */
TEST_F(RwsValidateWebSocketRequestTests, MalformedGetUriWithSpecialChars)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // URI with special characters
    const char* handshakeStr = "GET /WebSocket?param=value&foo=bar#fragment HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    // Should handle URI with query string and fragment
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle URI with special characters gracefully";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid line endings";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Test Case: Malformed GET request - request line only without headers
 * Expected: Returns RIPC_CONN_IN_PROGRESS or RIPC_CONN_ERROR
 */
TEST_F(RwsValidateWebSocketRequestTests, MalformedGetRequestLineOnly)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Only request line, no headers, with terminating CRLF CRLF
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    // Should fail due to missing required headers
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR with reason for missing headers";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Host field.") != NULL) << "Error message should mention missing Host field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Disallowed HTTP method (TRACE)
 * Expected: Returns RIPC_CONN_ERROR due to unsupported method
 */
TEST_F(RwsValidateWebSocketRequestTests, DisallowedHttpMethodTrace)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // TRACE method is not allowed
    const char* handshakeStr = "TRACE /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for disallowed TRACE method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL) << "Error message should mention invalid URI";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Disallowed HTTP method (PUT)
 * Expected: Returns RIPC_CONN_ERROR due to unsupported method
 */
TEST_F(RwsValidateWebSocketRequestTests, DisallowedHttpMethodPut)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // PUT method is not allowed
    const char* handshakeStr = "PUT /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for disallowed PUT method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL) << "Error message should mention invalid URI";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Disallowed HTTP method (DELETE)
 * Expected: Returns RIPC_CONN_ERROR due to unsupported method
 */
TEST_F(RwsValidateWebSocketRequestTests, DisallowedHttpMethodDelete)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // DELETE method is not allowed
    const char* handshakeStr = "DELETE /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for disallowed DELETE method";
    EXPECT_TRUE(strstr(error.text, "Invalid GET request received") != NULL) << "Error message should mention invalid URI";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Malformed GET request - query string and fragment
 * Expected: Returns RIPC_CONN_ERROR due to bad request
 */
TEST_F(RwsValidateWebSocketRequestTests, MalformedGetRequestWithQueryAndFragment)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Query string and fragment in GET request
    const char* handshakeStr = "GET /WebSocket?param=value#fragment HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for query string and fragment in GET request";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid line endings";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Test Case: Valid WebSocket handshake
 * Expected: Returns RIPC_CONN_ACTIVE
 */
TEST_F(RwsValidateWebSocketRequestTests, ValidWebSocketHandshake)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ACTIVE) << "Expected RIPC_CONN_ACTIVE for valid handshake";
    
    free(handshake);
}

/*
 * Security Test Case: Null pointer for handshake buffer
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNullHandshakeBuffer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Null handshake buffer
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, NULL, 100, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle null handshake buffer gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Error: 0002 Null pointer error. Argument hdrStart cannot be NULL.") != NULL) << "Error message should mention NULL argument";
}

/*
 * Security Test Case: Null RsslSocketChannel pointer
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNullSocketChannel)
{
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    // Null socket channel - this may crash, but we test to ensure it doesn't
    ripcSessInit result = rwsValidateWebSocketRequest(NULL, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle null socket channel gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Error: 0002 Null pointer error. Argument rsslSocketChannel cannot be NULL.") != NULL) << "Error message should mention NULL argument";
    
    free(handshake);
}

/*
 * Security Test Case: Null error pointer
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNullErrorPointer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    // Null error pointer
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, NULL);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle null error pointer gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Negative length value
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNegativeLength)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    // Negative length
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, -1, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle negative length gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP request size: (-1).") != NULL) << "Error message should mention invalid size";

    free(handshake);
}

/*
 * Security Test Case: Very large length value (integer overflow attempt)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityVeryLargeLength)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    // Very large length value (potential integer overflow)
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, 0x7FFFFFFF, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle very large length gracefully without crash";

    free(handshake);
}

/*
 * Security Test Case: Format string attack in header value
 * Expected: Should not interpret format specifiers
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityFormatStringAttackInHeaderValue)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Format string attack in Host header
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: %s%s%s%s%s%s%s%s%s%s%n%n%n%n\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle format string attack gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Format string attack in URI
 * Expected: Should not interpret format specifiers
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityFormatStringAttackInUri)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Format string attack in URI
    const char* handshakeStr = "GET /WebSocket%s%s%s%n%n%n HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle format string attack in URI gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";

    free(handshake);
}

/*
 * Security Test Case: Buffer with embedded null bytes throughout
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityEmbeddedNullBytes)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Create buffer with multiple embedded null bytes
    char handshake[256];
    for (int i = 0; i < 256; i++)
    {
        handshake[i] = (i % 2 == 0) ? 0x00 : (char)0xFF;
    }
    
    RsslInt32 len = 256;
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_EQ(result, RIPC_CONN_ERROR) << "Expected RIPC_CONN_ERROR for incomplete header ";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
}

/*
 * Security Test Case: Stack buffer overflow attempt with very long request line
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityStackOverflowLongRequestLine)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Create a very long request line (potential stack overflow)
    std::string longUri(32000, 'A');
    std::string handshakeStr = "GET /WebSocket" + longUri + " HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle very long request line gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    
    free(handshake);
}

/*
 * Security Test Case: Heap buffer overflow attempt with many headers
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityHeapOverflowManyHeaders)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Create request with thousands of headers
    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n";
    for (int i = 0; i < 10000; i++)
    {
        handshakeStr += "X-Header-" + std::to_string(i) + ": value" + std::to_string(i) + "\r\n";
    }
    handshakeStr += "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                    "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle many headers gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Off-by-one buffer read
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityOffByOneBufferRead)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Create a buffer where len is exactly at boundary
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\nHost: localhost:15000\r\n\r\n";
    size_t actualLen = strlen(handshakeStr);
    char* handshake = (char*)malloc(actualLen); // No null terminator space
    ASSERT_NE(handshake, nullptr);
    memcpy(handshake, handshakeStr, actualLen);
    
    // Pass exact length (off-by-one scenario)
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, (RsslInt32)actualLen, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle off-by-one buffer read gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Upgrade field.") != NULL) << "Error message should mention missing Upgrade field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Control characters in header name
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityControlCharsInHeaderName)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Control characters in header name
    char handshake[512];
    snprintf(handshake, sizeof(handshake),
             "GET /WebSocket HTTP/1.1\r\n"
             "Host: localhost:15000\r\n"
             "X-Blank\x01\x02\x03Header: value\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
             "Sec-WebSocket-Version: 13\r\n"
             "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
             "\r\n");
    RsslInt32 len = (RsslInt32)strlen(handshake);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle control characters in header name gracefully without crash";
}

/*
 * Security Test Case: Control characters in header value
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityControlCharsInHeaderValue)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Control characters in header value
    char handshake[512];
    snprintf(handshake, sizeof(handshake),
             "GET /WebSocket HTTP/1.1\r\n"
             "Host: localhost:15000\r\n"
             "X-Test: \x80\x81\x82\xFF\xFE\xFD\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
             "Sec-WebSocket-Version: 13\r\n"
             "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
             "\r\n");
    RsslInt32 len = (RsslInt32)strlen(handshake);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle control characters in header value gracefully without crash";
}

/*
 * Security Test Case: High-bit ASCII characters (potential encoding issues)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityHighBitAsciiChars)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // High-bit ASCII characters
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "X-Test: \xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle high-bit ASCII characters gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: UTF-8 multi-byte sequences
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityUtf8MultiByteSequences)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // UTF-8 multi-byte sequences (including invalid ones)
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "X-Test: \xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle UTF-8 multi-byte sequences gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Excessively long header name
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityExcessivelyLongHeaderName)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Excessively long header name
    std::string longHeaderName(16000, 'X');
    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               + longHeaderName + ": value\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle excessively long header name gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Header with commas in key
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityHeaderWithCommasInKey)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Header with commas in key
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "Sec-WebSocket-Extensions: permessage-deflate; ; ; ;;;; blank=value; ; ;\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle header with commas in key gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid Sec-WebSocket-Extensions parameter list format") != NULL) << "Error message should mention invalid Sec-WebSocket-Extensions parameter";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Security Test Case: Zero length handshake buffer
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityZeroLengthBuffer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    char handshake[16] = "GET /WebSocket";
    RsslInt32 len = 0; // Zero length
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_IN_PROGRESS) << "Should handle zero length buffer gracefully without crash";
}

/*
 * Security Test Case: Double free protection - call with same buffer twice
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityDoubleCallSameBuffer)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    RsslInt32 len;
    char* handshake = createModifiableHandshake(getValidWebSocketHandshake(), &len);
    ASSERT_NE(handshake, nullptr);
    
    // First call
    ripcSessInit result1 = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    // Second call with same buffer - tests state handling
    ripcSessInit result2 = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result1 == RIPC_CONN_ACTIVE) << "First call should complete without crash";

    EXPECT_TRUE(result2 == RIPC_CONN_ERROR) << "Second call should complete without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP header, duplicate fields received, Sec-Websocket-Key") != NULL) << "Error message should mention duplicate Sec-Websocket-Key field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Security Test Case: Extremely long header value (potential heap overflow)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityExtremelyLongHeaderValue)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, 131072);
    
    // Very long header value
    std::string longValue(64000, 'X');
    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: " + longValue + "\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle very long header value gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Invalid Base64 in WebSocket key
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityInvalidBase64WebSocketKey)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    // Invalid Base64 characters in key
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: !@#$%^&*()_+{}|:<>?\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);

    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);

    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle invalid Base64 in WebSocket key gracefully without crash";

    free(handshake);
}

/*
 * Security Test Case: Empty WebSocket key
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityEmptyWebSocketKey)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Empty WebSocket key
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key:\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle empty WebSocket key gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Sec-WebSocket-Key field.") != NULL) << "Error message should mention missing Sec-WebSocket-Key field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Security Test Case: Very long WebSocket key (buffer overflow attempt)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityVeryLongWebSocketKey)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Very long WebSocket key (should be 16 bytes base64 encoded = 24 chars)
    std::string longKey(1000, 'A');
    longKey += "==";
    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: " + longKey + "\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle very long WebSocket key gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "The value of the Sec-Websocket-Key field is too long: (1002)") != NULL) << "Error message should mention missing Sec-WebSocket-Key field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Very long WebSocket version (buffer overflow attempt)
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityVeryLongWebSocketVersion)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));

    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    // Very long WebSocket version
    std::string longVersion(55, '5');

    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
        "Host: localhost:15000\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: " + longVersion + "\r\n"
        "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
        "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);

    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);

    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle very long WebSocket key gracefully without crash";

    /* This is platform specific output due to the behavior of the atoi() function. */
#if defined(_WIN32)
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version (2147483647), version supported (13)") != NULL) << "Error message should mention invalid WebSocket version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nSec-WebSocket-Version: 13\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
#else
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version") != NULL) << "Error message should mention invalid WebSocket version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
#endif

    free(handshake);
}

/*
 * Security Test Case: Tab characters in header values
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityTabCharactersInHeaders)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Tab characters in headers
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host:\t\t\tlocalhost:15000\r\n"
                               "Upgrade:\twebsocket\r\n"
                               "Connection:\t\tUpgrade\r\n"
                               "Sec-WebSocket-Key:\tdGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version:\t13\r\n"
                               "Sec-WebSocket-Protocol:\trssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle tab characters in headers gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Mixed case header names
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityMixedCaseHeaderNames)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Mixed case header names
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "hOsT: localhost:15000\r\n"
                               "uPgRaDe: websocket\r\n"
                               "cOnNeCtIoN: Upgrade\r\n"
                               "sEc-WeBsOcKeT-kEy: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "sEc-WeBsOcKeT-vErSiOn: 13\r\n"
                               "sEc-WeBsOcKeT-pRoToCoL: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle mixed case header names gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Non-numeric version number
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNonNumericVersionNumber)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Non-numeric version number
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: thirteen\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle non-numeric version number gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version") != NULL) << "Error message should mention missing Sec-WebSocket-Key field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Multiple CRLF sequences before headers end
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityMultipleCrlfSequences)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Multiple CRLF sequences
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "\r\n"
                               "\r\n"
                               "\r\n"
                               "Extra: data\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle multiple CRLF sequences gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Upgrade field.") != NULL) << "Error message should mention missing Upgrade field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Only CR without LF
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityOnlyCrWithoutLf)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Only CR without LF
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r"
                               "Host: localhost:15000\r"
                               "\r";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle only CR without LF gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid Websocket request. Missing Upgrade field.") != NULL) << "Error message should mention missing Upgrade field";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: URI path traversal attempt
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityUriPathTraversal)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // URI path traversal attempt
    const char* handshakeStr = "GET /WebSocket/../../../etc/passwd HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle URI path traversal gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Null byte in URI
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityNullByteInUri)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Build buffer with null byte in URI
    char handshake[512];
    int offset = 0;
    const char* part1 = "GET /WebSocket";
    memcpy(handshake + offset, part1, strlen(part1));
    offset += (int)strlen(part1);
    handshake[offset++] = '\0'; // Null byte
    const char* part2 = "blank HTTP/1.1\r\nHost: localhost:15000\r\n\r\n";
    memcpy(handshake + offset, part2, strlen(part2) + 1);
    RsslInt32 len = offset + (RsslInt32)strlen(part2);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle null byte in URI gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
}

/*
 * Security Test Case: Extremely deep header nesting simulation
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityDeepHeaderNesting)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Headers with deeply nested-looking structure
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "X-Nested: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle deep header nesting gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Special URL encoding sequences
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecuritySpecialUrlEncodingSequences)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Special URL encoding sequences
    const char* handshakeStr = "GET /WebSocket%00%0d%0a%20%2f%2e%2e HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle special URL encoding sequences gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Backslash in URI
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityBackslashInUri)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Backslash in URI
    const char* handshakeStr = "GET /WebSocket\\..\\..\\etc\\passwd HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle backslash in URI gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid HTTP version received") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n\r\n", writeTransportBuffer, writeTransportBufferLength) == 0);
    
    free(handshake);
}

/*
 * Security Test Case: Extremely long Origin header
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityExtremelyLongOriginHeader)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, 131072);
    
    // Extremely long Origin header
    std::string longOrigin(64000, 'x');
    std::string handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Origin: http://" + longOrigin + ".com\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr.c_str(), &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle extremely long Origin header gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Cookie header with special characters
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityCookieHeaderSpecialChars)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));
    
    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);
    
    // Cookie header with special characters
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
                               "Host: localhost:15000\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
                               "Cookie: session=<script>blank()</script>; path=/; domain=.blank.com\r\n"
                               "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);
    
    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);
    
    EXPECT_TRUE(result == RIPC_CONN_ACTIVE) << "Should handle cookie header special chars gracefully without crash";
    
    free(handshake);
}

/*
 * Security Test Case: Invalid websocket older version.
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityInvalidWebSocketOlderVersion)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));

    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    // Special URL encoding sequences
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
        "Host: localhost:15000\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 12\r\n"
        "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
        "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);

    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);

    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle special URL encoding sequences gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version (12), version supported (13)") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nSec-WebSocket-Version: 13\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}

/*
 * Security Test Case: Invalid websocket newer version.
 * Expected: Should handle gracefully without crash
 */
TEST_F(RwsValidateWebSocketRequestTests, SecurityInvalidWebSocketNewerVersion)
{
    RsslServerSocketChannel serverSocketChannel;
    memset(&serverSocketChannel, 0, sizeof(RsslServerSocketChannel));

    rwsServer_t wsServer;
    memset(&wsServer, 0, sizeof(rwsServer_t));
    wsServer.protocolList = (char*)"rssl.json.v2, rssl.rwf";
    wsServer.version = 13;
    serverSocketChannel.rwsServer = &wsServer;

    RsslSocketChannel* pRsslSocketChannel = initSocketChannelWithMock(&serverSocketChannel, DEFAULT_INPUT_BUFFER_SIZE);

    // Special URL encoding sequences
    const char* handshakeStr = "GET /WebSocket HTTP/1.1\r\n"
        "Host: localhost:15000\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 14\r\n"
        "Sec-WebSocket-Protocol: rssl.json.v2\r\n"
        "\r\n";
    RsslInt32 len;
    char* handshake = createModifiableHandshake(handshakeStr, &len);
    ASSERT_NE(handshake, nullptr);

    ripcSessInit result = rwsValidateWebSocketRequest(pRsslSocketChannel, handshake, len, &error);

    EXPECT_TRUE(result == RIPC_CONN_ERROR) << "Should handle special URL encoding sequences gracefully without crash";
    EXPECT_TRUE(strstr(error.text, "Invalid WebSocket Version (14), version supported (13)") != NULL) << "Error message should mention invalid HTTP version";
    EXPECT_TRUE(strncmp("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache, private, no-store\r\nTransfer-Encoding: chunked\r\nSec-WebSocket-Version: 13\r\nConnection: close\r\n\r\n",
        writeTransportBuffer, writeTransportBufferLength) == 0);

    free(handshake);
}


