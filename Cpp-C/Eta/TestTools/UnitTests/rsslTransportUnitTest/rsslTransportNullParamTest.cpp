/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *	Transport NULL Parameter Unit Tests
 *
 *  Unit tests to verify that all RSSL Transport API functions properly
 *  handle NULL pointer parameters without crashing.
 *
 ************************************************************************/

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslIterators.h"

#include <cstring>

/*
 * Test fixture for NULL parameter tests.
 * These tests verify that the transport API functions properly handle
 * NULL pointer arguments without crashing.
 */
class RsslTransportNullParamTests : public ::testing::Test {
protected:
    RsslError error;
    RsslChannel channel;
    RsslServer server;
    RsslBuffer buffer;
    RsslChannelInfo channelInfo;
    RsslChannelStats channelStats;
    RsslServerInfo serverInfo;
    RsslInProgInfo inProgInfo;
    RsslConnectOptions connectOpts;
    RsslBindOptions bindOpts;
    RsslAcceptOptions acceptOpts;
    RsslReadInArgs readInArgs;
    RsslReadOutArgs readOutArgs;
    RsslWriteInArgs writeInArgs;
    RsslWriteOutArgs writeOutArgs;
    RsslTraceOptions traceOptions;
    RsslInitializeExOpts initExOpts;
    RsslLibraryVersionInfo versionInfo;
    RsslDebugFunctionsExOpts debugFunctionsExOpts;

    virtual void SetUp() override
    {
        clearError();
        memset(&channel, 0, sizeof(channel));
        memset(&server, 0, sizeof(server));
        memset(&buffer, 0, sizeof(buffer));
        memset(&channelInfo, 0, sizeof(channelInfo));
        memset(&channelStats, 0, sizeof(channelStats));
        memset(&serverInfo, 0, sizeof(serverInfo));
        memset(&inProgInfo, 0, sizeof(inProgInfo));
        rsslClearConnectOpts(&connectOpts);
        rsslClearBindOpts(&bindOpts);
        rsslClearAcceptOpts(&acceptOpts);
        rsslClearReadInArgs(&readInArgs);
        rsslClearReadOutArgs(&readOutArgs);
        rsslClearWriteInArgs(&writeInArgs);
        rsslClearWriteOutArgs(&writeOutArgs);
        rsslClearTraceOptions(&traceOptions);
        rsslClearDebugFunctionsExOpts(&debugFunctionsExOpts);
        
        initExOpts.rsslLocking = RSSL_LOCK_NONE;
        initExOpts.jitOpts.libsslName = NULL;
        initExOpts.jitOpts.libcryptoName = NULL;
        initExOpts.jitOpts.libcurlName = NULL;
        initExOpts.initConfig = NULL;
        initExOpts.initConfigSize = 0;
        initExOpts.initCurlDebug = RSSL_FALSE;
        initExOpts.shouldInitializeCPUIDlib = RSSL_TRUE;

        RsslRet ret = rsslInitialize(RSSL_LOCK_NONE, &error);
        EXPECT_EQ(ret, RSSL_RET_SUCCESS) << "rsslInitialize should succeed";
    }

    virtual void TearDown() override
    {
        RsslRet ret = rsslUninitialize();
        EXPECT_EQ(ret, RSSL_RET_SUCCESS) << "rsslUninitialize should succeed";
    }

    // Helper function to clear error structure before each test
    void clearError()
    {
        memset(&error, 0, sizeof(error));
    }

    // Helper function to verify error was populated with specific error code
    void verifyErrorPopulatedWithCode(const char* expectedErrorText, RsslRet expectedCode)
    {
        EXPECT_EQ(error.rsslErrorId, expectedCode) 
            << " rsslErrorId should be set to: " << expectedCode;
        EXPECT_TRUE(strstr(error.text, expectedErrorText) != NULL)
            << " Error text should should be set to: " << expectedErrorText;
    }
};

/*******************************************************************************
 * rsslInitialize / rsslInitializeEx / rsslUninitialize Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, InitializeWithNullError)
{
    // rsslInitialize with NULL error parameter
    // This should not crash even if error is NULL
    RsslRet ret = rsslInitialize(RSSL_LOCK_NONE, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitialize should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, InitializeExWithNullOpts)
{
    // rsslInitializeEx with NULL options parameter
    RsslRet ret = rsslInitializeEx(NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitializeEx should fail with NULL options";
    verifyErrorPopulatedWithCode("rsslInitializeEx() Error: 0002 Null pointer error. Argument rsslInitOpts cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, InitializeExWithNullError)
{
    // rsslInitializeEx with NULL error parameter
    RsslRet ret = rsslInitializeEx(&initExOpts, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitializeEx should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, InitializeExWithBothNull)
{
    // rsslInitializeEx with both parameters NULL
    RsslRet ret = rsslInitializeEx(NULL, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitializeEx should fail with both NULL";
}

/*******************************************************************************
 * rsslConnect Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ConnectWithNullOpts)
{
    // rsslConnect with NULL options parameter
    RsslChannel* pChannel = rsslConnect(NULL, &error);
    
    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslConnect should return NULL with NULL options";
    verifyErrorPopulatedWithCode("rsslConnect() Error: 0002 Null pointer error. Argument opts cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ConnectWithNullError)
{
    // rsslConnect with NULL error parameter
    RsslChannel* pChannel = rsslConnect(&connectOpts, NULL);
    
	EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslConnect should return NULL with NULL error";
}

TEST_F(RsslTransportNullParamTests, ConnectWithBothNull)
{
    // rsslConnect with both parameters NULL
    RsslChannel* pChannel = rsslConnect(NULL, NULL);
    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslConnect should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslReconnectClient Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ReconnectClientWithNullChannel)
{
    // rsslReconnectClient with NULL channel parameter
    RsslRet ret = rsslReconnectClient(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReconnectClient should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslReconnectClient() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReconnectClientWithNullError)
{
    // rsslReconnectClient with NULL error parameter
    RsslRet ret = rsslReconnectClient(&channel, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReconnectClient should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, ReconnectClientWithBothNull)
{
    // rsslReconnectClient with both parameters NULL
    RsslRet ret = rsslReconnectClient(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReconnectClient should fail with NULL parameters";
}

/*******************************************************************************
 * rsslBind Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, BindWithNullOpts)
{
    // rsslBind with NULL options parameter
    RsslServer* pServer = rsslBind(NULL, &error);
    
    EXPECT_EQ(pServer, (RsslServer*)NULL) << "rsslBind should return NULL with NULL options";
    verifyErrorPopulatedWithCode("rsslBind() Error: 0002 Null pointer error. Argument opts cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BindWithNullError)
{
    // rsslBind with NULL error parameter
    RsslServer* pServer = rsslBind(&bindOpts, NULL);

    EXPECT_EQ(pServer, (RsslServer*)NULL) << "rsslBind should return NULL with NULL error";
}

TEST_F(RsslTransportNullParamTests, BindWithBothNull)
{
    // rsslBind with both parameters NULL
    RsslServer* pServer = rsslBind(NULL, NULL);
    EXPECT_EQ(pServer, (RsslServer*)NULL) << "rsslBind should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslAccept Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, AcceptWithNullServer)
{
    // rsslAccept with NULL server parameter
    RsslChannel* pChannel = rsslAccept(NULL, &acceptOpts, &error);
    
    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslAccept should return NULL with NULL server";
    verifyErrorPopulatedWithCode("rsslAccept() Error: 0002 Null pointer error. Argument srvr cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, AcceptWithNullOpts)
{
    // rsslAccept with NULL options parameter
    RsslChannel* pChannel = rsslAccept(&server, NULL, &error);

    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslAccept should return NULL with NULL options";
    verifyErrorPopulatedWithCode("rsslAccept() Error: 0002 Null pointer error. Argument opts cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, AcceptWithNullError)
{
    // rsslAccept with NULL error parameter
    RsslChannel* pChannel = rsslAccept(&server, &acceptOpts, NULL);
    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslAccept should return NULL with NULL error";;
}

TEST_F(RsslTransportNullParamTests, AcceptWithAllNull)
{
    // rsslAccept with all parameters NULL
    RsslChannel* pChannel = rsslAccept(NULL, NULL, NULL);
    EXPECT_EQ(pChannel, (RsslChannel*)NULL) << "rsslAccept should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslCloseServer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, CloseServerWithNullServer)
{
    // rsslCloseServer with NULL server parameter
    RsslRet ret = rsslCloseServer(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseServer should fail with NULL server";
    verifyErrorPopulatedWithCode("rsslCloseServer() Error: 0002 Null pointer error. Argument srvr cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, CloseServerWithNullError)
{
    // rsslCloseServer with NULL error parameter
    RsslRet ret = rsslCloseServer(&server, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseServer should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, CloseServerWithBothNull)
{
    // rsslCloseServer with both parameters NULL
    RsslRet ret = rsslCloseServer(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseServer should fail with NULL parameters";
}

/*******************************************************************************
 * rsslInitChannel Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, InitChannelWithNullChannel)
{
    // rsslInitChannel with NULL channel parameter
    RsslRet ret = rsslInitChannel(NULL, &inProgInfo, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitChannel should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslInitChannel() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, InitChannelWithNullInProg)
{
    // rsslInitChannel with NULL inProg parameter
    RsslRet ret = rsslInitChannel(&channel, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitChannel should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslInitChannel() Error: 0002 Null pointer error. Argument inProg cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, InitChannelWithNullError)
{
    // rsslInitChannel with NULL error parameter
    RsslRet ret = rsslInitChannel(&channel, &inProgInfo, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitChannel should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, InitChannelWithAllNull)
{
    // rsslInitChannel with all parameters NULL
    RsslRet ret = rsslInitChannel(NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslInitChannel should fail with NULL parameters";
}

/*******************************************************************************
 * rsslCloseChannel Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, CloseChannelWithNullChannel)
{
    // rsslCloseChannel with NULL channel parameter
    RsslRet ret = rsslCloseChannel(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseChannel should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslCloseChannel() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, CloseChannelWithNullError)
{
    // rsslCloseChannel with NULL error parameter
    RsslRet ret = rsslCloseChannel(&channel, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseChannel should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, CloseChannelWithBothNull)
{
    // rsslCloseChannel with both parameters NULL
    RsslRet ret = rsslCloseChannel(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslCloseChannel should fail with NULL parameters";
}

/*******************************************************************************
 * rsslRead / rsslReadEx Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ReadWithNullChannel)
{
    // rsslRead with NULL channel parameter
    RsslRet readRet;
    RsslBuffer* pBuffer = rsslRead(NULL, &readRet, &error);
    
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslRead should return NULL with NULL channel";
    EXPECT_EQ(readRet, RSSL_RET_FAILURE) << "rsslRead should return failure code with NULL channel";
    verifyErrorPopulatedWithCode("rsslRead() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReadWithNullReadRet)
{
    // rsslRead with NULL readRet parameter
    RsslBuffer* pBuffer = rsslRead(&channel, NULL, &error);

    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslRead should return NULL with NULL readRet";
    verifyErrorPopulatedWithCode("rsslRead() Error: 0002 Null pointer error. Argument readRet cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReadWithNullError)
{
    // rsslRead with NULL error parameter
    RsslRet readRet;
    RsslBuffer* pBuffer = rsslRead(&channel, &readRet, NULL);
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslRead should return NULL with NULL error";
}

TEST_F(RsslTransportNullParamTests, ReadWithAllNull)
{
    // rsslRead with all parameters NULL
    RsslBuffer* pBuffer = rsslRead(NULL, NULL, NULL);
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslRead should return NULL with NULL parameters";
}

TEST_F(RsslTransportNullParamTests, ReadExWithNullChannel)
{
    // rsslReadEx with NULL channel parameter
    RsslRet readRet;
    RsslBuffer* pBuffer = rsslReadEx(NULL, &readInArgs, &readOutArgs, &readRet, &error);
    
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslReadEx should return NULL with NULL channel";
    verifyErrorPopulatedWithCode("rsslRead() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReadExWithNullInArgs)
{
    // rsslReadEx with NULL readInArgs parameter
    RsslRet readRet;
    RsslBuffer* pBuffer = rsslReadEx(&channel, NULL, &readOutArgs, &readRet, &error);

    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslReadEx should return NULL with NULL channel";
    verifyErrorPopulatedWithCode("rsslRead() Error: 0002 Null pointer error. Argument readInArgs cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReadExWithNullOutArgs)
{
    // rsslReadEx with NULL readOutArgs parameter
    RsslRet readRet;
    RsslBuffer* pBuffer = rsslReadEx(&channel, &readInArgs, NULL, &readRet, &error);

    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslReadEx should return NULL with NULL channel";
    verifyErrorPopulatedWithCode("rsslRead() Error: 0002 Null pointer error. Argument readOutArgs cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReadExWithAllNull)
{
    // rsslReadEx with all parameters NULL
    RsslBuffer* pBuffer = rsslReadEx(NULL, NULL, NULL, NULL, NULL);
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslReadEx should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslGetBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, GetBufferWithNullChannel)
{
    // rsslGetBuffer with NULL channel parameter
    RsslBuffer* pBuffer = rsslGetBuffer(NULL, 100, RSSL_FALSE, &error);
    
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslGetBuffer should return NULL with NULL channel";
    verifyErrorPopulatedWithCode("rsslGetBuffer() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetBufferWithNullError)
{
    // rsslGetBuffer with NULL error parameter
    RsslBuffer* pBuffer = rsslGetBuffer(&channel, 100, RSSL_FALSE, NULL);

    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslGetBuffer should return NULL with NULL error";
}

TEST_F(RsslTransportNullParamTests, GetBufferWithBothNull)
{
    // rsslGetBuffer with both channel and error NULL
    RsslBuffer* pBuffer = rsslGetBuffer(NULL, 100, RSSL_FALSE, NULL);
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslGetBuffer should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslReleaseBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ReleaseBufferWithNullBuffer)
{
    // rsslReleaseBuffer with NULL buffer parameter
    RsslRet ret = rsslReleaseBuffer(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReleaseBuffer should fail with NULL buffer";
    verifyErrorPopulatedWithCode("rsslReleaseBuffer", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ReleaseBufferWithNullError)
{
    // rsslReleaseBuffer with NULL error parameter
    RsslRet ret = rsslReleaseBuffer(&buffer, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReleaseBuffer should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, ReleaseBufferWithBothNull)
{
    // rsslReleaseBuffer with both parameters NULL
    RsslRet ret = rsslReleaseBuffer(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslReleaseBuffer should fail with NULL parameters";
}

/*******************************************************************************
 * rsslPackBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, PackBufferWithNullChannel)
{
    // rsslPackBuffer with NULL channel parameter
    RsslBuffer* pBuffer = rsslPackBuffer(NULL, &buffer, &error);
    
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslPackBuffer should return NULL with NULL channel";
    verifyErrorPopulatedWithCode("rsslPackBuffer() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, PackBufferWithNullBuffer)
{
    // rsslPackBuffer with NULL buffer parameter
    RsslBuffer* pBuffer = rsslPackBuffer(&channel, NULL, &error);
    
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslPackBuffer should return NULL with NULL buffer";
    verifyErrorPopulatedWithCode("rsslPackBuffer() Error: 0002 Null pointer error. Argument buffer cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, PackBufferWithNullError)
{
    // rsslPackBuffer with NULL error parameter
    RsslBuffer* pBuffer = rsslPackBuffer(&channel, &buffer, NULL);

    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslPackBuffer should return NULL with NULL error";
}

TEST_F(RsslTransportNullParamTests, PackBufferWithAllNull)
{
    // rsslPackBuffer with all parameters NULL
    RsslBuffer* pBuffer = rsslPackBuffer(NULL, NULL, NULL);
    EXPECT_EQ(pBuffer, (RsslBuffer*)NULL) << "rsslPackBuffer should return NULL with NULL parameters";
}

/*******************************************************************************
 * rsslWrite / rsslWriteEx Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, WriteWithNullChannel)
{
    // rsslWrite with NULL channel parameter
    RsslUInt32 bytesWritten, uncompressedBytesWritten;
    RsslRet ret = rsslWrite(NULL, &buffer, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompressedBytesWritten, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteWithNullBuffer)
{
    // rsslWrite with NULL buffer parameter
    RsslUInt32 bytesWritten, uncompressedBytesWritten;
    RsslRet ret = rsslWrite(&channel, NULL, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompressedBytesWritten, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL buffer";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument buffer cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteWithNullBytesWritten)
{
    // rsslWrite with NULL bytesWritten parameter
    RsslUInt32 uncompressedBytesWritten;
    RsslRet ret = rsslWrite(&channel, &buffer, RSSL_HIGH_PRIORITY, 0, NULL, &uncompressedBytesWritten, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL bytesWritten";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument bytesWritten cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteWithNullUncompressedBytesWritten)
{
    // rsslWrite with NULL uncompressedBytesWritten parameter
    RsslUInt32 bytesWritten;
    RsslRet ret = rsslWrite(&channel, &buffer, RSSL_HIGH_PRIORITY, 0, &bytesWritten, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL uncompressedBytesWritten";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument uncompressedBytesWritten cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteWithNullError)
{
    // rsslWrite with NULL error parameter
    RsslUInt32 bytesWritten, uncompressedBytesWritten;
    RsslRet ret = rsslWrite(&channel, &buffer, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompressedBytesWritten, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL error";;
}

TEST_F(RsslTransportNullParamTests, WriteWithAllNull)
{
    // rsslWrite with all parameters NULL
    RsslRet ret = rsslWrite(NULL, NULL, RSSL_HIGH_PRIORITY, 0, NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWrite should fail with NULL parameters";
}

TEST_F(RsslTransportNullParamTests, WriteExWithNullChannel)
{
    // rsslWriteEx with NULL channel parameter
    RsslRet ret = rsslWriteEx(NULL, &buffer, &writeInArgs, &writeOutArgs, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWriteEx should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteExWithNullBuffer)
{
    // rsslWriteEx with NULL buffer parameter
    RsslRet ret = rsslWriteEx(&channel, NULL, &writeInArgs, &writeOutArgs, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWriteEx should fail with NULL buffer";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument buffer cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteExWithNullInArgs)
{
    // rsslWriteEx with NULL writeInArgs parameter
    RsslRet ret = rsslWriteEx(&channel, &buffer, NULL, &writeOutArgs, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWriteEx should fail with NULL writeInArgs";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument writeInArgs cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteExWithNullOutArgs)
{
    // rsslWriteEx with NULL writeOutArgs parameter
    RsslRet ret = rsslWriteEx(&channel, &buffer, &writeInArgs, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWriteEx should fail with NULL writeOutArgs";
    verifyErrorPopulatedWithCode("rsslWrite() Error: 0002 Null pointer error. Argument writeOutArgs cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, WriteExWithAllNull)
{
    // rsslWriteEx with all parameters NULL
    RsslRet ret = rsslWriteEx(NULL, NULL, NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslWriteEx should fail with NULL parameters";
}

/*******************************************************************************
 * rsslFlush Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, FlushWithNullChannel)
{
    // rsslFlush with NULL channel parameter
    RsslRet ret = rsslFlush(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslFlush should fail with NULL channel";
    verifyErrorPopulatedWithCode(" rsslFlush() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, FlushWithNullError)
{
    // rsslFlush with NULL error parameter
    RsslRet ret = rsslFlush(&channel, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslFlush should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, FlushWithBothNull)
{
    // rsslFlush with both parameters NULL
    RsslRet ret = rsslFlush(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslFlush should fail with NULL parameters";
}

/*******************************************************************************
 * rsslPing Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, PingWithNullChannel)
{
    // rsslPing with NULL channel parameter
    RsslRet ret = rsslPing(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslPing should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslPing() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, PingWithNullError)
{
    // rsslPing with NULL error parameter
    RsslRet ret = rsslPing(&channel, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslPing should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, PingWithBothNull)
{
    // rsslPing with both parameters NULL
    RsslRet ret = rsslPing(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslPing should fail with NULL parameters";
}

/*******************************************************************************
 * rsslGetChannelInfo Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, GetChannelInfoWithNullChannel)
{
    // rsslGetChannelInfo with NULL channel parameter
    RsslRet ret = rsslGetChannelInfo(NULL, &channelInfo, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelInfo should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslGetChannelInfo() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetChannelInfoWithNullInfo)
{
    // rsslGetChannelInfo with NULL info parameter
    RsslRet ret = rsslGetChannelInfo(&channel, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelInfo should fail with NULL info";
    verifyErrorPopulatedWithCode("rsslGetChannelInfo() Error: 0002 Null pointer error. Argument info cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetChannelInfoWithNullError)
{
    // rsslGetChannelInfo with NULL error parameter
    RsslRet ret = rsslGetChannelInfo(&channel, &channelInfo, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelInfo should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, GetChannelInfoWithAllNull)
{
    // rsslGetChannelInfo with all parameters NULL
    RsslRet ret = rsslGetChannelInfo(NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelInfo should fail with NULL parameters";
}

/*******************************************************************************
 * rsslGetChannelStats Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, GetChannelStatsWithNullChannel)
{
    // rsslGetChannelStats with NULL channel parameter
    RsslRet ret = rsslGetChannelStats(NULL, &channelStats, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelStats should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslGetChannelStats() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetChannelStatsWithNullStats)
{
    // rsslGetChannelStats with NULL stats parameter
    RsslRet ret = rsslGetChannelStats(&channel, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelStats should fail with NULL stats";
    verifyErrorPopulatedWithCode("rsslGetChannelStats() Error: 0002 Null pointer error. Argument stats cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetChannelStatsWithNullError)
{
    // rsslGetChannelStats with NULL error parameter
    RsslRet ret = rsslGetChannelStats(&channel, &channelStats, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelStats should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, GetChannelStatsWithAllNull)
{
    // rsslGetChannelStats with all parameters NULL
    RsslRet ret = rsslGetChannelStats(NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetChannelStats should fail with NULL parameters";
}

/*******************************************************************************
 * rsslIoctl Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, IoctlWithNullChannel)
{
    // rsslIoctl with NULL channel parameter
    int value = 0;
    RsslRet ret = rsslIoctl(NULL, RSSL_MAX_NUM_BUFFERS, &value, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslIoctl should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslIoctl() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, IoctlWithNullValue)
{
    // rsslIoctl with NULL value parameter
    RsslRet ret = rsslIoctl(&channel, RSSL_MAX_NUM_BUFFERS, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslIoctl should fail with NULL value";
    verifyErrorPopulatedWithCode("rsslIoctl() Error: 0002 Null pointer error. Argument value cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, IoctlWithNullError)
{
    // rsslIoctl with NULL error parameter
    int value = 0;
    RsslRet ret = rsslIoctl(&channel, RSSL_MAX_NUM_BUFFERS, &value, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslIoctl should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, IoctlWithAllNull)
{
    // rsslIoctl with all parameters NULL (except code)
    RsslRet ret = rsslIoctl(NULL, RSSL_MAX_NUM_BUFFERS, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslIoctl should fail with NULL parameters";
}

/*******************************************************************************
 * rsslBufferUsage Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, BufferUsageWithNullChannel)
{
    // rsslBufferUsage with NULL channel parameter
    RsslInt32 usage = rsslBufferUsage(NULL, &error);
    
    EXPECT_LT(usage, 0) << "rsslBufferUsage should return failure with NULL channel";
    verifyErrorPopulatedWithCode("rsslBufferUsage() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferUsageWithNullError)
{
    // rsslBufferUsage with NULL error parameter
    RsslInt32 usage = rsslBufferUsage(&channel, NULL);
    EXPECT_LT(usage, 0) << "rsslBufferUsage should return failure with NULL error";
}

TEST_F(RsslTransportNullParamTests, BufferUsageWithBothNull)
{
    // rsslBufferUsage with both parameters NULL
    RsslInt32 usage = rsslBufferUsage(NULL, NULL);
    EXPECT_LT(usage, 0) << "rsslBufferUsage should return failure with NULL parameters";
}

/*******************************************************************************
 * rsslGetServerInfo Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, GetServerInfoWithNullServer)
{
    // rsslGetServerInfo with NULL srvr parameter
    RsslRet ret = rsslGetServerInfo(NULL, &serverInfo, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetServerInfo should fail with NULL srvr";
    verifyErrorPopulatedWithCode("rsslGetServerInfo() Error: 0002 Null pointer error. Argument srvr cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetServerInfoWithNullInfo)
{
    // rsslGetServerInfo with NULL info parameter
    RsslRet ret = rsslGetServerInfo(&server, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetServerInfo should fail with NULL info";
    verifyErrorPopulatedWithCode("rsslGetServerInfo() Error: 0002 Null pointer error. Argument info cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, GetServerInfoWithNullError)
{
    // rsslGetServerInfo with NULL error parameter
    RsslRet ret = rsslGetServerInfo(&server, &serverInfo, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetServerInfo should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, GetServerInfoWithAllNull)
{
    // rsslGetServerInfo with all parameters NULL
    RsslRet ret = rsslGetServerInfo(NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetServerInfo should fail with NULL parameters";
}

/*******************************************************************************
 * rsslServerIoctl Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ServerIoctlWithNullServer)
{
    // rsslServerIoctl with NULL srvr parameter
    int value = 0;
    RsslRet ret = rsslServerIoctl(NULL, RSSL_SERVER_NUM_POOL_BUFFERS, &value, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslServerIoctl should fail with NULL srvr";
    verifyErrorPopulatedWithCode("rsslServerIoctl() Error: 0002 Null pointer error. Argument srvr cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ServerIoctlWithNullValue)
{
    // rsslServerIoctl with NULL value parameter
    RsslRet ret = rsslServerIoctl(&server, RSSL_SERVER_NUM_POOL_BUFFERS, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslServerIoctl should fail with NULL value";
    verifyErrorPopulatedWithCode("rsslServerIoctl() Error: 0002 Null pointer error. Argument value cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ServerIoctlWithNullError)
{
    // rsslServerIoctl with NULL error parameter
    int value = 0;
    RsslRet ret = rsslServerIoctl(&server, RSSL_SERVER_NUM_POOL_BUFFERS, &value, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslServerIoctl should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, ServerIoctlWithAllNull)
{
    // rsslServerIoctl with all parameters NULL (except code)
    RsslRet ret = rsslServerIoctl(NULL, RSSL_SERVER_NUM_POOL_BUFFERS, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslServerIoctl should fail with NULL parameters";
}

/*******************************************************************************
 * rsslServerBufferUsage Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, ServerBufferUsageWithNullServer)
{
    // rsslServerBufferUsage with NULL server parameter
    RsslInt32 usage = rsslServerBufferUsage(NULL, &error);
    
    EXPECT_EQ(usage, -1) << "rsslServerBufferUsage should return failure with NULL srvr";
    verifyErrorPopulatedWithCode("rsslServerBufferUsage() Error: 0002 Null pointer error. Argument srvr cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, ServerBufferUsageWithNullError)
{
    // rsslServerBufferUsage with NULL error parameter
    RsslInt32 usage = rsslServerBufferUsage(&server, NULL);
    EXPECT_EQ(usage, -1) << "rsslServerBufferUsage should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, ServerBufferUsageWithBothNull)
{
    // rsslServerBufferUsage with both parameters NULL
    RsslInt32 usage = rsslServerBufferUsage(NULL, NULL);
    EXPECT_EQ(usage, -1) << "rsslServerBufferUsage should return failure with NULL parameters";
}

/*******************************************************************************
 * rsslHostByName Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, HostByNameWithNullHostName)
{
    // rsslHostByName with NULL hostName parameter
    RsslUInt32 ipAddr;
    RsslRet ret = rsslHostByName(NULL, &ipAddr);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslHostByName should fail with NULL hostName";
}

TEST_F(RsslTransportNullParamTests, HostByNameWithNullIpAddr)
{
    // rsslHostByName with NULL ipAddr parameter
    RsslBuffer hostName = { 9, (char*)"localhost" };
    RsslRet ret = rsslHostByName(&hostName, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslHostByName should fail with NULL ipAddr";
}

TEST_F(RsslTransportNullParamTests, HostByNameWithBothNull)
{
    // rsslHostByName with both parameters NULL
    RsslRet ret = rsslHostByName(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslHostByName should fail with NULL parameters";
}

/*******************************************************************************
 * rsslGetUserName Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, GetUserNameWithNullUserName)
{
    // rsslGetUserName with NULL userName parameter
    RsslRet ret = rsslGetUserName(NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslGetUserName should fail with NULL userName";
}

/*******************************************************************************
 * rsslQueryTransportLibraryVersion Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, QueryTransportLibraryVersionWithNullVerInfo)
{
    // rsslQueryTransportLibraryVersion with NULL pVerInfo parameter
    // This function should not crash even with NULL parameter
    rsslQueryTransportLibraryVersion(NULL);
    // No return value to check, just verify it doesn't crash
}

/*******************************************************************************
 * rsslCalculateEncryptedSize Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, CalculateEncryptedSizeWithNullBuffer)
{
    // rsslCalculateEncryptedSize with NULL buffer parameter
    RsslUInt32 size = rsslCalculateEncryptedSize(NULL);
    EXPECT_EQ(size, 0U) << "rsslCalculateEncryptedSize should return 0 with NULL buffer";
}

/*******************************************************************************
 * rsslEncryptBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, EncryptBufferWithNullChannel)
{
    // rsslEncryptBuffer with NULL channel parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslEncryptBuffer(NULL, &buffer, &output, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslEncryptBuffer should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslEncryptBuffer() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, EncryptBufferWithNullInput)
{
    // rsslEncryptBuffer with NULL unencryptedInput parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslEncryptBuffer(&channel, NULL, &output, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslEncryptBuffer should fail with NULL unencryptedInput";
    verifyErrorPopulatedWithCode("rsslEncryptBuffer() Error: 0002 Null pointer error. Argument unencryptedInput cannot be NULL.", RSSL_RET_FAILURE);
    free(output.data);
}

TEST_F(RsslTransportNullParamTests, EncryptBufferWithNullOutput)
{
    // rsslEncryptBuffer with NULL encryptedOutput parameter
    RsslRet ret = rsslEncryptBuffer(&channel, &buffer, NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslEncryptBuffer should fail with NULL encryptedOutput";
    verifyErrorPopulatedWithCode("rsslEncryptBuffer() Error: 0002 Null pointer error. Argument encryptedOutput cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, EncryptBufferWithNullError)
{
    // rsslEncryptBuffer with NULL error parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslEncryptBuffer(&channel, &buffer, &output, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslEncryptBuffer should fail with NULL output error";
}

TEST_F(RsslTransportNullParamTests, EncryptBufferWithAllNull)
{
    // rsslEncryptBuffer with all parameters NULL
    RsslRet ret = rsslEncryptBuffer(NULL, NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslEncryptBuffer should fail with NULL parameters";
}

/*******************************************************************************
 * rsslDecryptBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullChannel)
{
    // rsslDecryptBuffer with NULL chnl parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslDecryptBuffer(NULL, &buffer, &output, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL chnl";
    verifyErrorPopulatedWithCode("rsslDecryptBuffer() Error: 0002 Null pointer error. Argument chnl cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullInput)
{
    // rsslDecryptBuffer with NULL encryptedInput parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslDecryptBuffer(&channel, NULL, &output, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL encryptedInput";
    verifyErrorPopulatedWithCode("rsslDecryptBuffer() Error: 0002 Null pointer error. Argument encryptedInput cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullInputData)
{
    // rsslDecryptBuffer with NULL encryptedInput->data
    RsslBuffer encryptedInput = RSSL_INIT_BUFFER;
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslDecryptBuffer(&channel, &encryptedInput, &output, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL encryptedInput->data";
    verifyErrorPopulatedWithCode("rsslDecryptBuffer() Error: 0002 Null pointer error. Argument encryptedInput->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullOutput)
{
    // rsslDecryptBuffer with NULL decryptedOutput parameter
	buffer.data = (char*)"encrypted data";
    RsslRet ret = rsslDecryptBuffer(&channel, &buffer, NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL decryptedOutput";
    verifyErrorPopulatedWithCode("rsslDecryptBuffer() Error: 0002 Null pointer error. Argument decryptedOutput cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullOutputData)
{
    // rsslDecryptBuffer with NULL decryptedOutput->data
    buffer.data = (char*)"encrypted data";
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslDecryptBuffer(&channel, &buffer, &output, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL decryptedOutput->data";
    verifyErrorPopulatedWithCode("rsslDecryptBuffer() Error: 0002 Null pointer error. Argument decryptedOutput->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithNullError)
{
    // rsslDecryptBuffer with NULL error parameter
	RsslBuffer input = { 10, (char*)"input data" };
    RsslBuffer output = { 11, (char*)"output data" };
    RsslRet ret = rsslDecryptBuffer(&channel, &buffer, &output, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, DecryptBufferWithAllNull)
{
    // rsslDecryptBuffer with all parameters NULL
    RsslRet ret = rsslDecryptBuffer(NULL, NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDecryptBuffer should fail with NULL parameters";
}

/*******************************************************************************
 * rsslCalculateHexDumpOutputSize Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, CalculateHexDumpOutputSizeWithNullBuffer)
{
    // rsslCalculateHexDumpOutputSize with NULL bufferToHexDump parameter
    RsslUInt32 size = rsslCalculateHexDumpOutputSize(NULL, 16);
    EXPECT_EQ(size, 0) << "rsslCalculateHexDumpOutputSize should return 0 with NULL bufferToHexDump";
}

/*******************************************************************************
 * rsslBufferToHexDump Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithNullInput)
{
    // rsslBufferToHexDump with NULL bufferToHexDump parameter
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslBufferToHexDump(NULL, &output, 16, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL bufferToHexDump";
    verifyErrorPopulatedWithCode("rsslBufferToHexDump() Error: 0002 Null pointer error. Argument bufferToHexDump cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithNullInputData)
{
    // rsslBufferToHexDump with NULL bufferToHexDump->data
    RsslBuffer input = RSSL_INIT_BUFFER;
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslBufferToHexDump(&input, &output, 16, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL bufferToHexDump->data";
    verifyErrorPopulatedWithCode("rsslBufferToHexDump() Error: 0002 Null pointer error. Argument bufferToHexDump->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithNullOutput)
{
    // rsslBufferToHexDump with NULL hexDumpOutput parameter
    RsslBuffer input = { 10, (char*)"input data" };
    RsslRet ret = rsslBufferToHexDump(&input, NULL, 16, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL hexDumpOutput";
    verifyErrorPopulatedWithCode("rsslBufferToHexDump() Error: 0002 Null pointer error. Argument hexDumpOutput cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithNullOutputData)
{
    // rsslBufferToHexDump with NULL hexDumpOutput->data
    RsslBuffer input = { 10, (char*)"input data" };
    RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslBufferToHexDump(&input, &output, 16, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL hexDumpOutput->data";
    verifyErrorPopulatedWithCode("rsslBufferToHexDump() Error: 0002 Null pointer error. Argument hexDumpOutput->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithNullError)
{
    // rsslBufferToHexDump with NULL error parameter
    RsslBuffer input = { 10, (char*)"input data" };
    RsslBuffer output = { 11, (char*)"output data" };
    RsslRet ret = rsslBufferToHexDump(&input, &output, 16, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, BufferToHexDumpWithAllNull)
{
    // rsslBufferToHexDump with all parameters NULL
    RsslRet ret = rsslBufferToHexDump(NULL, NULL, 16, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL parameters";
}

/*******************************************************************************
 * rsslBufferToRawHexDump Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithNullInput)
{
    // rsslBufferToRawHexDump with NULL bufferToHexDumpr parameter
    RsslBuffer output = { 11, (char*)"output data" };
    RsslRet ret = rsslBufferToRawHexDump(NULL, &output, 16, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToRawHexDump should fail with NULL bufferToHexDump";
    verifyErrorPopulatedWithCode("rsslBufferToRawHexDump() Error: 0002 Null pointer error. Argument bufferToHexDump cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithNullInputData)
{
    // rsslBufferToRawHexDump with NULL bufferToHexDumpr->data
	RsslBuffer input = RSSL_INIT_BUFFER;
    RsslBuffer output = { 11, (char*)"output data" };
    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 16, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToRawHexDump should fail with NULL bufferToHexDumpr->data";
    verifyErrorPopulatedWithCode("rsslBufferToRawHexDump() Error: 0002 Null pointer error. Argument bufferToHexDump->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithNullOutput)
{
    // rsslBufferToRawHexDump with NULL hexDumpOutput parameter
    RsslBuffer input = {10, (char*)"test data"};
    RsslRet ret = rsslBufferToRawHexDump(&input, NULL, 16, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToRawHexDump should fail with NULL hexDumpOutput";
    verifyErrorPopulatedWithCode("rsslBufferToRawHexDump() Error: 0002 Null pointer error. Argument hexDumpOutput cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithNullOutputData)
{
    // rsslBufferToRawHexDump with NULL hexDumpOutput->data
    RsslBuffer input = { 10, (char*)"test data" };
	RsslBuffer output = RSSL_INIT_BUFFER;
    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 16, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToRawHexDump should fail with NULL hexDumpOutput->data";
    verifyErrorPopulatedWithCode("rsslBufferToRawHexDump() Error: 0002 Null pointer error. Argument hexDumpOutput->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithNullError)
{
    // rsslBufferToRawHexDump with NULL error parameter
    RsslBuffer input = {10, (char*)"test data"};
    RsslBuffer output = { 11, (char*)"output data" };
    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 16, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToHexDump should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, BufferToRawHexDumpWithAllNull)
{
    // rsslBufferToRawHexDump with all parameters NULL
    RsslRet ret = rsslBufferToRawHexDump(NULL, NULL, 16, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslBufferToRawHexDump should fail with NULL parameters";
}

/*******************************************************************************
 * rsslSetDebugFunctions Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, SetDebugFunctionsWithNullError)
{
    // rsslSetDebugFunctions with NULL error parameter
    RsslRet ret = rsslSetDebugFunctions(NULL, NULL, NULL, NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslSetDebugFunctions should fail with NULL parameters";
}

/*******************************************************************************
 * rsslSetDebugFunctionsEx Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, SetDebugFunctionsExWithNullOpts)
{
    // rsslSetDebugFunctionsEx with NULL pOpts parameter
    RsslRet ret = rsslSetDebugFunctionsEx(NULL, &error);
    
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslSetDebugFunctionsEx should fail with NULL pOpts";
    verifyErrorPopulatedWithCode("rsslSetDebugFunctionsEx() Error: 0002 Null pointer error. Argument pOpts cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, SetDebugFunctionsExWithNullError)
{
    // rsslSetDebugFunctionsEx with NULL error parameter
    RsslRet ret = rsslSetDebugFunctionsEx(&debugFunctionsExOpts, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslSetDebugFunctionsEx should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, SetDebugFunctionsExWithBothNull)
{
    // rsslSetDebugFunctionsEx with both parameters NULL
    RsslRet ret = rsslSetDebugFunctionsEx(NULL, NULL);
    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslSetDebugFunctionsEx should fail with NULL parameters";
}

/*******************************************************************************
 * rsslDumpBuffer Tests
 ******************************************************************************/

TEST_F(RsslTransportNullParamTests, DumpBufferWithNullChannel)
{
    // rsslDumpBuffer with NULL channel parameter
    RsslBuffer dumpBuffer = {10, (char*)"test data"};
    RsslRet ret = rsslDumpBuffer(NULL, RSSL_RWF_PROTOCOL_TYPE, &dumpBuffer, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDumpBuffer should fail with NULL channel";
    verifyErrorPopulatedWithCode("rsslDumpBuffer() Error: 0002 Null pointer error. Argument channel cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DumpBufferWithNullBuffer)
{
    // rsslDumpBuffer with NULL buffer parameter
    RsslRet ret = rsslDumpBuffer(&channel, RSSL_RWF_PROTOCOL_TYPE, NULL, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDumpBuffer should fail with NULL buffer";
    verifyErrorPopulatedWithCode("rsslDumpBuffer() Error: 0002 Null pointer error. Argument buffer cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DumpBufferWithNullBufferData)
{
    // rsslDumpBuffer with NULL buffer->data
	RsslBuffer dumpBuffer = RSSL_INIT_BUFFER;
    RsslRet ret = rsslDumpBuffer(&channel, RSSL_RWF_PROTOCOL_TYPE, &dumpBuffer, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDumpBuffer should fail with NULL buffer->data";
    verifyErrorPopulatedWithCode("rsslDumpBuffer() Error: 0002 Null pointer error. Argument buffer->data cannot be NULL.", RSSL_RET_FAILURE);
}

TEST_F(RsslTransportNullParamTests, DumpBufferWithNullError)
{
    // rsslDumpBuffer with NULL error parameter
    RsslBuffer dumpBuffer = {10, (char*)"test data"};
    RsslRet ret = rsslDumpBuffer(&channel, RSSL_RWF_PROTOCOL_TYPE, &dumpBuffer, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDumpBuffer should fail with NULL error";
}

TEST_F(RsslTransportNullParamTests, DumpBufferWithAllNull)
{
    // rsslDumpBuffer with all parameters NULL
    RsslRet ret = rsslDumpBuffer(NULL, RSSL_RWF_PROTOCOL_TYPE, NULL, NULL);

    EXPECT_EQ(ret, RSSL_RET_FAILURE) << "rsslDumpBuffer should fail with NULL parameters";
}

