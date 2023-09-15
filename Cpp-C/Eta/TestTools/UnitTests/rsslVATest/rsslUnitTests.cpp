/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "rsslTestFramework.h"
#include "rtr/rsslGetTime.h"
#include "gtest/gtest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#define strtok_r strtok_s
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

static void rsslUnitTests_DirectoryPacking();
static void rsslUnitTests_LoginPacking();
static void rsslUnitTests_WriteFailure(int fragMsg);
static void rsslUnitTests_ProvideHttpHdr();

static const RsslInt32 
			LOGIN_STREAM_ID = 1,
			DIRECTORY_STREAM_ID = 2,
			FIELD_DICTIONARY_STREAM_ID = 3,
			ENUM_TYPE_DICTIONARY_STREAM_ID = 4,
			ITEM_STREAM_ID_START = 5;

TEST(RsslUnitTests, LoginPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_LoginPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests_WebSocket_rssl_rwf, LoginPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_LoginPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests, DirectoryPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_DirectoryPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests_WebSocket_rssl_rwf, DirectoryPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_DirectoryPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests, WriteFailureTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	pOpts.serverMaxOutputBuffers = 20;
	pOpts.serverGuaranteedOutputBuffers = 5;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(0);
	rsslTestUninitialize();
}

TEST(RsslUnitTests_WebSocket_rssl_rwf, WriteFailureTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	pOpts.serverMaxOutputBuffers = 20;
	pOpts.serverGuaranteedOutputBuffers = 5;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(0);
	rsslTestUninitialize();
}

TEST(RsslUnitTests, WriteFailureCompressionTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	pOpts.serverMaxOutputBuffers = 30;
	pOpts.serverGuaranteedOutputBuffers = 5;
	pOpts.compressionType = RSSL_COMP_ZLIB;
	pOpts.compressionLevel = 0;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(0);
	rsslTestUninitialize();
}

TEST(RsslUnitTests_WebSocket_rssl_rwf, WriteFailureCompressionTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	pOpts.serverMaxOutputBuffers = 30;
	pOpts.serverGuaranteedOutputBuffers = 5;
	pOpts.compressionType = RSSL_COMP_ZLIB;
	pOpts.compressionLevel = 0;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(0);
	rsslTestUninitialize();
}

TEST(RsslUnitTests, WriteFailureFragmentedTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	pOpts.serverMaxOutputBuffers = 30;
	pOpts.serverGuaranteedOutputBuffers = 3;
	pOpts.compressionType = RSSL_COMP_ZLIB;
	pOpts.compressionLevel = 0;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(1);
	rsslTestUninitialize();
}

TEST(RsslUnitTests_WebSocket_rssl_rwf, WriteFailureFragmentedTest)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	pOpts.serverMaxOutputBuffers = 30;
	pOpts.serverGuaranteedOutputBuffers = 3;
	pOpts.compressionType = RSSL_COMP_ZLIB;
	pOpts.compressionLevel = 0;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_WriteFailure(1);
	rsslTestUninitialize();
}

#ifdef COMPILE_64BITS
TEST(RsslUnitTests_Multicast, LoginPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_LoginPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests_Multicast, DirectoryPacking)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	pOpts.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
	rsslTestInitialize(&pOpts);

	rsslUnitTests_DirectoryPacking();
	rsslTestUninitialize();
}

TEST(RsslUnitTests, ProvideHttpHdr)
{
	rsslTestInitOpts pOpts;

	rsslTestClearInitOpts(&pOpts);
	setCallbackToHttpHdr(true);

	pOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;

	rsslTestInitialize(&pOpts);

	rsslUnitTests_ProvideHttpHdr();
	setCallbackToHttpHdr(false);
	rsslTestUninitialize();
}

#endif

/*These tests checks that packed buffers containing a source directory request 
and other messages are received properly. Test 1: packs 2 MP refresh messages
into the same buffer, then adds a third directory refresh message to the buffer, and sends the buffer.
Test 2: packs 2 MP refresh messages into the same buffer, then adds a third directory refresh message to 
the buffer and packs it. This only differs from the first test by calling packbuffer() on
the buffer after adding the directory refresh. All tests check if the messages are sent and 
received correctly*/
static void rsslUnitTests_DirectoryPacking()
{

	RsslChannel* pConsumerChannel;
    RsslChannel* pProviderChannel;
	RsslBuffer *pSendBuffer = NULL;
	RsslBuffer *pRecvBuffer = NULL;
    RsslError error;
	RsslErrorInfo errorInfo;
	RsslRet ret;
	RsslDecodeIterator dIter;
	RsslEncodeIterator eIter;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslMsg msg2 = RSSL_INIT_MSG;
	RsslBuffer *msgBuf;
	RsslRDMLoginMsg loginMsg;
	char loginMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, loginMsgChar };
	RsslRDMDirectoryRequest directoryRequest;
	RsslRequestMsg requestMsg;
	char msgKeyData[3] = {'T', 'R' , 'I'};
	RsslBuffer msgKeyBuffer = {3, msgKeyData};

	rsslTestStart();
    pConsumerChannel = rsslTestCreateConsumerChannel();
    pProviderChannel = rsslTestCreateProviderChannel();
	rsslTestInitChannels(pConsumerChannel, pProviderChannel);

	/*Cons sends login request*/
	sendLoginRequest(pConsumerChannel, LOGIN_STREAM_ID);

	/*Prov reads in buffer and processes login request*/
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	/*decode buffer*/
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pProviderChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);
	EXPECT_FALSE(ret != RSSL_RET_SUCCESS) << printf("\nrsslDecodeMsg() Error %d on SessionData fd=%llu Size %d \n", ret, (RsslUInt64)pProviderChannel->socketId, pRecvBuffer->length);
	
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_LOGIN) << printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	EXPECT_FALSE((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
		<< printf("rsslDecodeRDMLoginMsg() failed! Ret: %i text: %s\n", ret, errorInfo.rsslError.text);

	ret = processLoginRequest(pProviderChannel, &msg, &dIter);

	/*Cons reads in buffer and processes login refresh*/
	memset(loginMsgChar,0,sizeof(loginMsgChar));
	pRecvBuffer = NULL;

	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pConsumerChannel, &ret, &error);

	/*decode buffer*/
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_LOGIN) << printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	EXPECT_FALSE((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
		<< printf("rsslDecodeRDMLoginMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);

	EXPECT_FALSE(loginMsg.rdmMsgBase.rdmMsgType != RDM_LG_MT_REFRESH) << printf("Unexpected RDM message type %d\n", loginMsg.rdmMsgBase.rdmMsgType);
	
	/*Test 1:
	Encode and pack two MP request into the same buffer. Then encode 
	a source directory message into the same buffer WITHOUT packing and send it
	*/
	if (!(msgBuf = rsslGetBuffer(pConsumerChannel, 2000, RSSL_TRUE, &error)))
	{
		rsslSetErrorInfoLocation(&errorInfo, __FILE__, __LINE__);
		EXPECT_TRUE(msgBuf);
	}

	rsslClearEncodeIterator(&eIter);

	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);

	rsslClearRequestMsg(&requestMsg);

	requestMsg.msgBase.streamId = 5;
	requestMsg.msgBase.domainType = 6;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	requestMsg.msgBase.msgKey.flags = 3;
	requestMsg.msgBase.msgKey.serviceId = 1;
	requestMsg.msgBase.msgKey.name.length = msgKeyBuffer.length;
	requestMsg.msgBase.msgKey.name.data = msgKeyBuffer.data;

	EXPECT_FALSE((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	/*Second message (this message is identical to the first)*/
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	/*Reencode the message and pack it in the buffer a second time*/
	EXPECT_FALSE((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");
	
	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");
		
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, DIRECTORY_STREAM_ID);

	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

	EXPECT_FALSE((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS);

	EXPECT_FALSE(sendMessage(pConsumerChannel, msgBuf) != RSSL_RET_SUCCESS);
	
	rsslReleaseBuffer(msgBuf, &error);

	/*Prov reads in buffer and processes directory request and item request*/
	rsslClearMsg(&msg);

	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the first message should be the request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the second message should be another request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));
	
	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the third message should be the directory request*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_SOURCE) << printf("Unexpected domain type %s\n",
		rsslDomainTypeToOmmString(msg.msgBase.domainType));
	
	/*Test 2:
	Encode and pack two MP request into the same buffer. Then encode and PACK
	a source directory message into the same buffer, and send it.
	*/
	
	/*Send two request messages packed with a directory request*/
	if (!(msgBuf = rsslGetBuffer(pConsumerChannel, 2000, RSSL_TRUE, &error)))
	{
		rsslSetErrorInfoLocation(&errorInfo, __FILE__, __LINE__);
		EXPECT_TRUE(msgBuf);
	}

	rsslClearEncodeIterator(&eIter);

	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);

	rsslClearRequestMsg(&requestMsg);

	requestMsg.msgBase.streamId = 5;
	requestMsg.msgBase.domainType = 6;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	requestMsg.msgBase.msgKey.flags = 3;
	requestMsg.msgBase.msgKey.serviceId = 1;
	requestMsg.msgBase.msgKey.name.length = msgKeyBuffer.length;
	requestMsg.msgBase.msgKey.name.data = msgKeyBuffer.data;

	EXPECT_FALSE ((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	/*Second message (this message is identical to the first)*/
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	/*Reencode the message and pack it in the buffer a second time*/
	EXPECT_FALSE((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, DIRECTORY_STREAM_ID);

	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

	if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		return;

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	msgBuf->length = 0;

	if ( sendMessage(pConsumerChannel, msgBuf) != RSSL_RET_SUCCESS)
			return;

	/*Prov reads in buffer and processes directory request and item request*/
	rsslClearMsg(&msg);

	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the first message should be the request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the second message should be another request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the third message should be the directory request*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_SOURCE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	rsslCloseChannel(pConsumerChannel, &error);
	rsslCloseChannel(pProviderChannel, &error);
	rsslTestFinish();
}

/*This test packs 2 MP refresh messages into the same buffer, then adds a login
refresh message to the buffer before sending it. It then checks to see if the
messages are received correctly.*/
static void rsslUnitTests_LoginPacking()
{
	RsslChannel* pConsumerChannel;
    RsslChannel* pProviderChannel;
	RsslBuffer *pSendBuffer = NULL;
	RsslBuffer *pRecvBuffer = NULL;
    RsslError error;
	RsslErrorInfo errorInfo;
	RsslRet ret;
	RsslDecodeIterator dIter;
	RsslEncodeIterator eIter;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslMsg msg2 = RSSL_INIT_MSG;
	RsslBuffer *msgBuf;
	RsslRDMLoginMsg loginMsg;
	char loginMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, loginMsgChar };
	RsslRequestMsg requestMsg;
	char msgKeyData[3] = {'T', 'R' , 'I'};
	RsslBuffer applicationName = { 16, (char*)"rsslVATest" } ;
	RsslRDMLoginRequest loginRequest;
	RsslBuffer msgKeyBuffer = {3, msgKeyData};

	rsslTestStart();
    pConsumerChannel = rsslTestCreateConsumerChannel();
    pProviderChannel = rsslTestCreateProviderChannel();

	ASSERT_NE(pConsumerChannel, (RsslChannel*)NULL) << "Consumer channel not initialized.  If this is a multicast test, please make sure the reliable MCast shared library is present";
	ASSERT_NE(pProviderChannel, (RsslChannel*)NULL) << "Provider channel not initialized.  If this is a multicast test, please make sure the reliable MCast shared library is present";

	rsslTestInitChannels(pConsumerChannel, pProviderChannel);

	/*Cons sends login request*/

	if (!(msgBuf = rsslGetBuffer(pConsumerChannel, 2000, RSSL_TRUE, &error)))
	{
		rsslSetErrorInfoLocation(&errorInfo, __FILE__, __LINE__);
		return;
	}

	rsslClearEncodeIterator(&eIter);

	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);

	rsslClearRequestMsg(&requestMsg);

	requestMsg.msgBase.streamId = 5;
	requestMsg.msgBase.domainType = 6;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	requestMsg.msgBase.msgKey.flags = 3;
	requestMsg.msgBase.msgKey.serviceId = 1;
	requestMsg.msgBase.msgKey.name.length = msgKeyBuffer.length;
	requestMsg.msgBase.msgKey.name.data = msgKeyBuffer.data;

	EXPECT_FALSE((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	/*Second message (this message is identical to the first)*/
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	/*Reencode the message and pack it in the buffer a second time*/
	EXPECT_FALSE((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		<< printf("rsslEncodeMsg failed\n");

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	EXPECT_FALSE((msgBuf = rsslPackBuffer(pConsumerChannel, msgBuf, &error)) == 0)
		<< printf("Error occured while attempting to pack buffer\n");

	/*Third message is login request*/
	rsslInitDefaultRDMLoginRequest(&loginRequest, LOGIN_STREAM_ID);
	
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, msgBuf);
	rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRequest, &msgBuf->length, &errorInfo);


	loginRequest.flags |= RDM_LG_RQF_HAS_ROLE | RDM_LG_RQF_HAS_APPLICATION_NAME;
	loginRequest.applicationName = applicationName;
	EXPECT_TRUE(sendMessage(pConsumerChannel, msgBuf) == RSSL_RET_SUCCESS);
	
	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the first message should be the request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pConsumerChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);

	/*the first message should be the request from the market price domain*/
	EXPECT_FALSE(msg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		<< printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));

	/*the third message should be a login request*/
	rsslClearMsg(&msg);
	pRecvBuffer = NULL;
	while(pRecvBuffer == NULL)
		pRecvBuffer = rsslRead(pProviderChannel, &ret, &error);
	
	/*decode buffer*/
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pProviderChannel->majorVersion, pProviderChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pRecvBuffer);
	ret = rsslDecodeMsg(&dIter, &msg);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%llu  Size %d \n", ret, (RsslUInt64)pProviderChannel->socketId, pRecvBuffer->length);
		return;
	}

	if (msg.msgBase.domainType != RSSL_DMT_LOGIN)
	{
		printf("Unexpected domain type %s\n", rsslDomainTypeToOmmString(msg.msgBase.domainType));
		return;
	}

	if ((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeRDMLoginMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		return;
	}


	EXPECT_TRUE(msg.msgBase.domainType == RSSL_DMT_LOGIN);
	
	rsslCloseChannel(pConsumerChannel, &error);
	rsslCloseChannel(pProviderChannel, &error);
	rsslTestFinish();
}

/*This test does the following:
1. Opens up a consumer and server provider channel.  Server has been configured in calling function.
2. Kills the consumer channel
3. Attempts to ping.  This should succeed, as the SYN-ACK has not been sent yet by the server.
4. Gets 10 buffers from the prov channel.  This should get into the shared buffer pool
5. Attempts to write all buffers, until rsslWrite attempts to flush. This should fail, and the channel should be in the close state.
6. Closes the provider channel.  This should succeed.

The high watermark is set to be approximately 80% of the buffer pull size, to ensure that this test is getting shared pool buffers.
*/
static void rsslUnitTests_WriteFailure(int fragMsg)
{

	RsslChannel* pConsumerChannel;
	RsslChannel* pProviderChannel;
	RsslBuffer *pSendBuffer[10];
	RsslError error;
	RsslRet ret;
	RsslWriteOutArgs writeOutArgs = RSSL_INIT_WRITE_OUT_ARGS;
	RsslWriteInArgs writeInArgs = RSSL_INIT_WRITE_IN_ARGS;
	int watermark;
	RsslChannelInfo chnlInfo;
	int i;
	RsslUInt32 msgSize;

	rsslTestStart();
	pConsumerChannel = rsslTestCreateConsumerChannel();
	pProviderChannel = rsslTestCreateProviderChannel();
	rsslTestInitChannels(pConsumerChannel, pProviderChannel);

	rsslGetChannelInfo(pProviderChannel, &chnlInfo, &error);
	
	rsslCloseChannel(pConsumerChannel, &error);
	
	if (fragMsg == 0)
	{
		msgSize = chnlInfo.maxFragmentSize;
		watermark = chnlInfo.maxFragmentSize * 8;
	}
	else
	{
		msgSize = 2 * chnlInfo.maxFragmentSize;
		watermark = chnlInfo.maxFragmentSize * 16;
	}

	rsslIoctl(pProviderChannel, RSSL_HIGH_WATER_MARK, &watermark, &error);

	/* Ping should succeed, but this should prime the next flush to fail */
	ret = rsslPing(pProviderChannel, &error);
	EXPECT_EQ(ret, RSSL_RET_SUCCESS);

	for (i = 0; i < 10; ++i)
	{
		pSendBuffer[i] = rsslGetBuffer(pProviderChannel, msgSize, RSSL_FALSE, &error);
		EXPECT_TRUE(pSendBuffer[i] != NULL);
	}
	i = 0;
	do
	{
		ret = rsslWriteEx(pProviderChannel, pSendBuffer[i], &writeInArgs, &writeOutArgs, &error);
		i++;
	} while (ret >= RSSL_RET_SUCCESS);

	EXPECT_EQ(ret, RSSL_RET_WRITE_FLUSH_FAILED);
	EXPECT_EQ(pProviderChannel->state, RSSL_CH_STATE_CLOSED);

	ret = rsslCloseChannel(pProviderChannel, &error);
	EXPECT_EQ(ret, RSSL_RET_SUCCESS);
	rsslTestFinish();
}

/*This test does the following:
1. Opens up a consumer and server provider channel.  Server has been configured in calling function.
2. Http header callback HttpCallbackFunction() would be called
3. Closes the provider channel.  This should succeed.
*/

static void rsslUnitTests_ProvideHttpHdr() 
{
	RsslChannel* pConsumerChannel;
	RsslChannel* pProviderChannel;
	RsslError error;
	RsslRet ret;

	rsslTestStart();
	pConsumerChannel = rsslTestCreateConsumerChannel();
	pProviderChannel = rsslTestCreateProviderChannel();

	rsslTestInitChannels(pConsumerChannel, pProviderChannel);

	ret = rsslCloseChannel(pConsumerChannel, &error);
	EXPECT_EQ(ret, RSSL_RET_SUCCESS);

	ret = rsslCloseChannel(pProviderChannel, &error);
	EXPECT_EQ(ret, RSSL_RET_SUCCESS);
	rsslTestFinish();
}