/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gtest/gtest.h"

#include "TestUtil.h"
#include "TestReactorEvent.h"
#include "TestReactor.h"
#include "Consumer.h"
#include "Provider.h"
#include "TunnelStreamProvider.h"

using namespace testing;

void tunnelStreamMsgExchangeTest(bool authenticate, bool enableWatchlist);
void tunnelStreamOpenWhileDisconnectedTest(bool enableWatchlist);
void tunnelStreamLongNameTest(bool enableWatchlist);
void tunnelStreamMaxMsgSizeTest(bool enableWatchlist);
void tunnelStreamBufferUsedTest(bool enableWatchlist);
void tunnelStreamClientClosedTest();

int main(int argc, char *argv[])
{
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TunnelStream, tunnelStreamMsgExchangeTest_NoAuthenticateNoWatchlist)
{
	tunnelStreamMsgExchangeTest(false, false);
}

TEST(TunnelStream, tunnelStreamMsgExchangeTest_AuthenticateNoWatchlist)
{
	tunnelStreamMsgExchangeTest(true, false);
}

TEST(TunnelStream, tunnelStreamMsgExchangeTest_NoAuthenticateWatchlist)
{
	tunnelStreamMsgExchangeTest(false, true);
}

TEST(TunnelStream, tunnelStreamMsgExchangeTest_AuthenticateWatchlist)
{
	tunnelStreamMsgExchangeTest(true, true);
}

TEST(TunnelStream, tunnelStreamOpenWhileDisconnectedTest_NoWatchlist)
{
	tunnelStreamOpenWhileDisconnectedTest(false);
}

TEST(TunnelStream, tunnelStreamOpenWhileDisconnectedTest_Watchlist)
{
	tunnelStreamOpenWhileDisconnectedTest(true);
}

TEST(TunnelStream, tunnelStreamLongNameTest_NoWatchlist)
{
	tunnelStreamLongNameTest(false);
}

TEST(TunnelStream, tunnelStreamLongNameTest_Watchlist)
{
	tunnelStreamLongNameTest(true);
}

TEST(TunnelStream, tunnelStreamMaxMsgSizeTest_NoWatchlist)
{
	tunnelStreamMaxMsgSizeTest(false);
}

TEST(TunnelStream, tunnelStreamMaxMsgSizeTest_Watchlist)
{
	tunnelStreamMaxMsgSizeTest(true);
}

TEST(TunnelStream, tunnelStreamBufferUsedTest_NoWatchlist)
{
	tunnelStreamBufferUsedTest(false);
}

TEST(TunnelStream, tunnelStreamBufferUsedTest_Watchlist)
{
	tunnelStreamBufferUsedTest(true);
}

TEST(TunnelStream, tunnelStreamClientClosedTest_NoWatchlist)
{
	tunnelStreamClientClosedTest();
}

TEST(TunnelStream, tunnelStreamGetInfo_ErrorInfoArgTest)
{
	/* the structures will be ignored, used as real pointers */
	RsslTunnelStream tunnelStream = { 0 };
	RsslTunnelStreamInfo streamInfo = { 0 };

	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, rsslTunnelStreamGetInfo(&tunnelStream, &streamInfo, NULL));
}

TEST(TunnelStream, tunnelStreamGetInfo_TunnelStreamArgTest)
{
	/* the structure will be ignored, it is used as a pointer */
	RsslTunnelStreamInfo streamInfo = { 0 };

	RsslErrorInfo errorInfo;

	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, rsslTunnelStreamGetInfo(NULL, &streamInfo, &errorInfo));
	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, errorInfo.rsslError.rsslErrorId);
	ASSERT_TRUE(strncmp("RsslTunnelStream not provided.", errorInfo.rsslError.text, strlen("RsslTunnelStream not provided.")) == 0);
	ASSERT_EQ(RSSL_EIC_FAILURE, errorInfo.rsslErrorInfoCode);
}

TEST(TunnelStream, tunnelStreamGetInfo_StreamInfoArgTest)
{
	/* the structure will be ignored, it is used as a pointer */
	RsslTunnelStream tunnelStream = { 0 };

	RsslErrorInfo errorInfo;

	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, rsslTunnelStreamGetInfo(&tunnelStream, NULL, &errorInfo));
	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, errorInfo.rsslError.rsslErrorId);
	ASSERT_TRUE(strncmp("ValuePtr not provided.", errorInfo.rsslError.text, strlen("ValuePtr not provided.")) == 0);
	ASSERT_EQ(RSSL_EIC_FAILURE, errorInfo.rsslErrorInfoCode);
}

void tunnelStreamMsgExchangeTest(bool authenticate, bool enableWatchlist)
{
	/* Test opening a TunnelStream and exchanging a couple messages (consumer to prov, then prov to consumer). */
	   
	TestReactorEvent* pEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslTunnelStreamMsgEvent* pTsMsgEvent;
	RsslTunnelStream* pConsTunnelStream;
	RsslTunnelStream* pProvTunnelStream;
	RsslBuffer* pBuffer;
	RsslTunnelStreamSubmitOptions tsSubmitOpts;
	RsslTunnelStreamGetBufferOptions tsGetBufferOptions;
	RsslErrorInfo errorInfo;
	RsslTunnelStreamCloseOptions tsCloseOptions;
		
	/* Setup some sample data. */
	char sampleString[] = "PETER CAPALDI"; 
			   
	/* Create reactors. */
	TestReactor consumerReactor = TestReactor();
	TestReactor providerReactor = TestReactor();
				
	/* Create consumer. */
	Consumer consumer = Consumer(&consumerReactor);
	RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
	RsslRDMLoginRequest loginRequest;
	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	RsslRDMDirectoryRequest directoryRequest;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	pConsumerRole->pLoginRequest = &loginRequest;
	pConsumerRole->pDirectoryRequest = &directoryRequest;
	pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
	pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
	pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
	pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
	pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
	pConsumerRole->watchlistOptions.enableWatchlist = enableWatchlist;
		
	/* Create provider. */
	TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor);
	TunnelStreamProvider::maxMsgSize(DEFAULT_MAX_MSG_SIZE);
	TunnelStreamProvider::maxFragmentSize(DEFAULT_MAX_FRAG_SIZE);
	RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
	pProviderRole->base.channelEventCallback = provider.channelEventCallback;
	pProviderRole->loginMsgCallback = provider.loginMsgCallback;
	pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
	pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
	pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
	pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;

	/* Connect the consumer and provider. Setup login & directory streams automatically. */
	ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
	opts.setupDefaultLoginStream(true);
	opts.setupDefaultDirectoryStream(true);
	provider.bind(&opts);
	TestReactor::openSession(&consumer, &provider, &opts);
		
	/* Open a TunnelStream. */
	RsslTunnelStreamOpenOptions tsOpenOpts;
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	char name[] = "Tunnel1";
	tsOpenOpts.name = name;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
	tsOpenOpts.streamId = 5;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;
	if (authenticate)
		tsOpenOpts.classOfService.authentication.type = RDM_COS_AU_OMM_LOGIN;

	OpenedTunnelStreamInfo* pOpenedTsInfo = consumer.openTunnelStream(&provider, &tsOpenOpts);
	ASSERT_TRUE(pOpenedTsInfo != NULL);
	pConsTunnelStream = pOpenedTsInfo->consumerTunnelStream();
	pProvTunnelStream = pOpenedTsInfo->providerTunnelStream();
	delete pOpenedTsInfo;
		
	/* Test tunnel stream accessors */
	ASSERT_EQ(5, pConsTunnelStream->streamId);
	if (!enableWatchlist) /* Watchlist will likely use different stream ID. */
		ASSERT_EQ(5, pProvTunnelStream->streamId);
	ASSERT_EQ(RSSL_DMT_SYSTEM, pConsTunnelStream->domainType);
	ASSERT_EQ(RSSL_DMT_SYSTEM, pProvTunnelStream->domainType);
	ASSERT_EQ(defaultService()->serviceId, pConsTunnelStream->serviceId);
	ASSERT_EQ(defaultService()->serviceId, pProvTunnelStream->serviceId);
	ASSERT_EQ(consumer.reactorChannel(), pConsTunnelStream->pReactorChannel);
	ASSERT_EQ(provider.reactorChannel(), pProvTunnelStream->pReactorChannel);
	ASSERT_TRUE(strncmp("Tunnel1", pConsTunnelStream->name, strlen("Tunnel1")) == 0);
	ASSERT_TRUE(strncmp("Tunnel1", pProvTunnelStream->name, strlen("Tunnel1")) == 0);
	ASSERT_EQ(&consumer, pConsTunnelStream->userSpecPtr);
	ASSERT_EQ(NULL, pProvTunnelStream->userSpecPtr);
	ASSERT_EQ(RSSL_STREAM_OPEN, pConsTunnelStream->state.streamState);
	ASSERT_EQ(RSSL_DATA_OK, pConsTunnelStream->state.dataState);
	ASSERT_EQ(RSSL_SC_NONE, pConsTunnelStream->state.code);
	ASSERT_EQ(RSSL_STREAM_OPEN, pProvTunnelStream->state.streamState);
	ASSERT_EQ(RSSL_DATA_OK, pProvTunnelStream->state.dataState);
	ASSERT_EQ(RSSL_SC_NONE, pProvTunnelStream->state.code);

	/* Test consumer tunnel stream class of service values */
	ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pConsTunnelStream->classOfService.common.protocolType);
	ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pConsTunnelStream->classOfService.common.protocolMajorVersion);
	ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pConsTunnelStream->classOfService.common.protocolMinorVersion);
	ASSERT_EQ(DEFAULT_MAX_MSG_SIZE, pConsTunnelStream->classOfService.common.maxMsgSize);
	ASSERT_EQ(DEFAULT_MAX_FRAG_SIZE, pConsTunnelStream->classOfService.common.maxFragmentSize);
	if (authenticate)
		ASSERT_EQ(RDM_COS_AU_OMM_LOGIN, pConsTunnelStream->classOfService.authentication.type);
	else
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pConsTunnelStream->classOfService.authentication.type);
	ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pConsTunnelStream->classOfService.flowControl.type);
	ASSERT_EQ(RDM_COS_DI_RELIABLE, pConsTunnelStream->classOfService.dataIntegrity.type);
	ASSERT_EQ(RDM_COS_GU_NONE, pConsTunnelStream->classOfService.guarantee.type);

	/* Test provider tunnel stream class of service values */
	ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pProvTunnelStream->classOfService.common.protocolType);
	ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pProvTunnelStream->classOfService.common.protocolMajorVersion);
	ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pProvTunnelStream->classOfService.common.protocolMinorVersion);
	ASSERT_EQ(DEFAULT_MAX_MSG_SIZE, pProvTunnelStream->classOfService.common.maxMsgSize);
	ASSERT_EQ(DEFAULT_MAX_FRAG_SIZE, pProvTunnelStream->classOfService.common.maxFragmentSize);
	if (authenticate)
		ASSERT_EQ(RDM_COS_AU_OMM_LOGIN, pProvTunnelStream->classOfService.authentication.type);
	else
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pProvTunnelStream->classOfService.authentication.type);
	ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pProvTunnelStream->classOfService.flowControl.type);
	ASSERT_EQ(RDM_COS_DI_RELIABLE, pProvTunnelStream->classOfService.dataIntegrity.type);
	ASSERT_EQ(RDM_COS_GU_NONE, pProvTunnelStream->classOfService.guarantee.type);
	  
	/* Consumer sends an opaque buffer to the provider. */
	rsslClearTunnelStreamSubmitOptions(&tsSubmitOpts);
	tsSubmitOpts.containerType = RSSL_DT_OPAQUE;
	rsslClearTunnelStreamGetBufferOptions(&tsGetBufferOptions);
	tsGetBufferOptions.size = (RsslUInt32)strlen(sampleString);
	ASSERT_TRUE((pBuffer = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);
	pBuffer->length = (RsslUInt32)strlen(sampleString);
	strcpy(pBuffer->data, sampleString);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamSubmit(pConsTunnelStream, pBuffer, &tsSubmitOpts, &errorInfo));
	consumerReactor.dispatch(0);
		
	/* Provider receives the buffer. */
	providerReactor.dispatch(1);
	pEvent = providerReactor.pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_MSG, pEvent->type());
	pTsMsgEvent = (RsslTunnelStreamMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_DT_OPAQUE, pTsMsgEvent->containerType);
	ASSERT_TRUE((pBuffer = pTsMsgEvent->pRsslBuffer) != NULL);
	ASSERT_EQ(strlen(sampleString), pBuffer->length);
	ASSERT_TRUE(strncmp(pBuffer->data, sampleString, strlen(sampleString)) == 0);
	delete pEvent;
		
	/* Provider sends an opaque buffer to the consumer. */
	rsslClearTunnelStreamSubmitOptions(&tsSubmitOpts);
	tsSubmitOpts.containerType = RSSL_DT_OPAQUE;
	rsslClearTunnelStreamGetBufferOptions(&tsGetBufferOptions);
	tsGetBufferOptions.size = (RsslUInt32)strlen(sampleString);
	ASSERT_TRUE((pBuffer = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);
	pBuffer->length = (RsslUInt32)strlen(sampleString);
	strcpy(pBuffer->data, sampleString);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamSubmit(pProvTunnelStream, pBuffer, &tsSubmitOpts, &errorInfo));
	providerReactor.dispatch(0);
		
	/* Consumer receives the buffer. */
	consumerReactor.dispatch(1);
	pEvent = consumerReactor.pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_MSG, pEvent->type());
	pTsMsgEvent = (RsslTunnelStreamMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_DT_OPAQUE, pTsMsgEvent->containerType);
	ASSERT_TRUE((pBuffer = pTsMsgEvent->pRsslBuffer) != NULL);
	ASSERT_EQ(strlen(sampleString), pBuffer->length);
	ASSERT_TRUE(strncmp(pBuffer->data, sampleString, strlen(sampleString)) == 0);
	delete pEvent;

	/* Provider closes the tunnel stream, starting FIN/ACK teardown */
	rsslClearTunnelStreamCloseOptions(&tsCloseOptions);
	rsslReactorCloseTunnelStream(pProvTunnelStream, &tsCloseOptions, &errorInfo);
	providerReactor.dispatch(0);
		
	/* Consumer receives the open/suspect event */
	consumerReactor.dispatch(1);
	pEvent = consumerReactor.pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	ASSERT_EQ(NULL, pTsStatusEvent->pAuthInfo);
	ASSERT_EQ(consumer.reactorChannel(), pTsStatusEvent->pReactorChannel);
	ASSERT_EQ(RSSL_STREAM_OPEN, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_EQ(RSSL_SC_NONE, pTsStatusEvent->pState->code);
	delete pEvent;
	consumerReactor.dispatch(0);
		
	/* Provider Reactor internally responds to FIN (no message). */
	providerReactor.dispatch(0);
		
	/* Consumer receives the close event */
	consumerReactor.dispatch(1);
	pEvent = consumerReactor.pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	ASSERT_EQ(NULL, pTsStatusEvent->pAuthInfo);
	ASSERT_EQ(consumer.reactorChannel(), pTsStatusEvent->pReactorChannel);
	ASSERT_EQ(RSSL_STREAM_CLOSED_RECOVER, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_OK, pTsStatusEvent->pState->dataState);
	ASSERT_EQ(RSSL_SC_NONE, pTsStatusEvent->pState->code);
	delete pEvent;
	consumerReactor.dispatch(0);
		
	TestReactorComponent::closeSession(&consumer, &provider);
	consumerReactor.close();
	providerReactor.close();
}

void tunnelStreamOpenWhileDisconnectedTest(bool enableWatchlist)
{
	/* Make sure a TunnelStream cannot be opened while the channel is down. */
		
	TestReactorEvent* pEvent;
	RsslReactorChannelEvent* pChannelEvent;
	RsslRDMLoginMsgEvent* pLoginMsgEvent;
	RsslRDMDirectoryMsgEvent* pDirectoryMsgEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslErrorInfo errorInfo;

	/* Create reactors. */
	TestReactor consumerReactor = TestReactor();
	TestReactor providerReactor = TestReactor();
				
	/* Create consumer. */
	Consumer consumer = Consumer(&consumerReactor);
	RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
	RsslRDMLoginRequest loginRequest;
	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	RsslRDMDirectoryRequest directoryRequest;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	pConsumerRole->pLoginRequest = &loginRequest;
	pConsumerRole->pDirectoryRequest = &directoryRequest;
	pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
	pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
	pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
	pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
	pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
	pConsumerRole->watchlistOptions.enableWatchlist = enableWatchlist;

	/* Create provider. */
	TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor);
	RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
	pProviderRole->base.channelEventCallback = provider.channelEventCallback;
	pProviderRole->loginMsgCallback = provider.loginMsgCallback;
	pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
	pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
	pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
	pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;
		
	/* Connect the consumer and provider. Setup login & directory streams automatically. */
	ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
	opts.setupDefaultLoginStream(true);
	opts.setupDefaultDirectoryStream(true);
	opts.reconnectAttemptLimit(-1);
	provider.bind(&opts);
	TestReactor::openSession(&consumer, &provider, &opts);
		
	/* Disconnect channel. */
	provider.close();
			   
	/* Consumer receives disconnection event */
	consumerReactor.dispatch(1 + (enableWatchlist ? 2 : 0));



	if (enableWatchlist)
	{
		pEvent = consumer.testReactor()->pollEvent();
		ASSERT_EQ(LOGIN_MSG, pEvent->type());
		pLoginMsgEvent = (RsslRDMLoginMsgEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RDM_LG_MT_STATUS, pLoginMsgEvent->pRDMLoginMsg->rdmMsgBase.rdmMsgType);
		delete pEvent;

		pEvent = consumer.testReactor()->pollEvent();
		ASSERT_EQ(DIRECTORY_MSG, pEvent->type());
		pDirectoryMsgEvent = (RsslRDMDirectoryMsgEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RDM_DR_MT_UPDATE, pDirectoryMsgEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType);
		delete pEvent;

		pEvent = consumer.testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent*)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING, pChannelEvent->channelEventType);
		delete pEvent;
	}
	else
	{
		pEvent = consumer.testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent*)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING, pChannelEvent->channelEventType);
		delete pEvent;
	}

	/* Open a TunnelStream. */
	RsslTunnelStreamOpenOptions tsOpenOpts;
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	char name[] = "Tunnel1";
	tsOpenOpts.name = name;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.streamId = 5;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorOpenTunnelStream(consumer.reactorChannel(), &tsOpenOpts, &errorInfo));
	consumer.testReactor()->dispatch(1);

	/* Consumer should receive tunnel stream status event that open failed. */
	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	ASSERT_EQ(NULL, pTsStatusEvent->pAuthInfo);
	ASSERT_EQ(consumer.reactorChannel(), pTsStatusEvent->pReactorChannel);
	ASSERT_EQ(RSSL_STREAM_CLOSED_RECOVER, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_EQ(RSSL_SC_NONE, pTsStatusEvent->pState->code);
	delete pEvent;

	consumer.close();
	providerReactor.close();
	consumerReactor.close();
}

void tunnelStreamClientClosedTest()
{
	TestReactorEvent* pEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslErrorInfo errorInfo;

	/* Create reactors. */
	TestReactor consumerReactor = TestReactor();
	TestReactor providerReactor = TestReactor();

	/* Create consumer. */
	Consumer consumer = Consumer(&consumerReactor);
	RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
	RsslRDMLoginRequest loginRequest;
	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	RsslRDMDirectoryRequest directoryRequest;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	pConsumerRole->pLoginRequest = &loginRequest;
	pConsumerRole->pDirectoryRequest = &directoryRequest;
	pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
	pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
	pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
	pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
	pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;

	/* Create provider. */
	TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor, RSSL_TRUE);
	RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
	pProviderRole->base.channelEventCallback = provider.channelEventCallback;
	pProviderRole->loginMsgCallback = provider.loginMsgCallback;
	pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
	pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
	pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
	pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;

	/* Connect the consumer and provider. Setup login & directory streams automatically. */
	ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
	opts.setupDefaultLoginStream(true);
	opts.setupDefaultDirectoryStream(true);
	opts.reconnectAttemptLimit(-1);
	provider.bind(&opts);
	TestReactor::openSession(&consumer, &provider, &opts);

	/* Open a TunnelStream. */
	RsslTunnelStreamOpenOptions tsOpenOpts;
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	char name[] = "Tunnel1";
	tsOpenOpts.name = name;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.streamId = 5;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;
	tsOpenOpts.responseTimeout = 1;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorOpenTunnelStream(consumer.reactorChannel(), &tsOpenOpts, &errorInfo));
	consumer.testReactor()->dispatch(2);

	/* Consumer should receive tunnel stream status event */
	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	delete pEvent;

	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	delete pEvent;

	provider.testReactor()->dispatch(2);

	pEvent = provider.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_REQUEST, pEvent->type());
	delete pEvent;

	pEvent = provider.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_STREAM_CLOSED, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_STREQ("Received a close message from the remote end.", pTsStatusEvent->pState->text.data);
	delete pEvent;


	/* Open another TunnelStream. */
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	tsOpenOpts.name = name;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.streamId = 6;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;
	tsOpenOpts.responseTimeout = 1;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorOpenTunnelStream(consumer.reactorChannel(), &tsOpenOpts, &errorInfo));

	consumer.testReactor()->dispatch(2);

	/* Consumer should receive tunnel stream status event that open failed. */
	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	delete pEvent;

	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	delete pEvent;

	provider.testReactor()->dispatch(2);

	pEvent = provider.testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_REQUEST, pEvent->type());
	delete pEvent;

	pEvent = provider.testReactor()->pollEvent();

	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent*)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_STREAM_CLOSED, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_STREQ("Received a close message from the remote end.", pTsStatusEvent->pState->text.data);
	delete pEvent;

	consumer.close();
	provider.close();
	providerReactor.close();
	consumerReactor.close();
}

void tunnelStreamLongNameTest(bool enableWatchlist)
{
	/* Test using a couple long TunnelStream names -- 255 is the maximum possible name. */

	RsslErrorInfo errorInfo;

	RsslTunnelStream* pConsTunnelStream;
	RsslTunnelStream* pProvTunnelStream;
		
	/* This name is 256 characters long. Opening a TunnelStream with this name should fail. */
	char tsName256[] = "5555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555";

	/* This name is 255 characters long. Opening a TunnelStream with this name should succeed. */
	char tsName255[] = "555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555";
	   
	/* Create reactors. */
	TestReactor consumerReactor = TestReactor();
	TestReactor providerReactor = TestReactor();
				
	/* Create consumer. */
	Consumer consumer = Consumer(&consumerReactor);
	RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
	RsslRDMLoginRequest loginRequest;
	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	RsslRDMDirectoryRequest directoryRequest;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	pConsumerRole->pLoginRequest = &loginRequest;
	pConsumerRole->pDirectoryRequest = &directoryRequest;
	pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
	pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
	pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
	pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
	pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
	pConsumerRole->watchlistOptions.enableWatchlist = enableWatchlist;
		
	/* Create provider. */
	TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor);
	RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
	pProviderRole->base.channelEventCallback = provider.channelEventCallback;
	pProviderRole->loginMsgCallback = provider.loginMsgCallback;
	pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
	pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
	pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
	pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;

	/* Connect the consumer and provider. Setup login & directory streams automatically. */
	ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
	opts.setupDefaultLoginStream(true);
	opts.setupDefaultDirectoryStream(true);
	provider.bind(&opts);
	TestReactor::openSession(&consumer, &provider, &opts);

	/* Negative test -- open a TunnelStream with a 256-character name. This should fail. */
	RsslTunnelStreamOpenOptions tsOpenOpts;
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	tsOpenOpts.name = tsName256;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.streamId = 5;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;
	ASSERT_EQ(RSSL_RET_INVALID_ARGUMENT, rsslReactorOpenTunnelStream(consumer.reactorChannel(), &tsOpenOpts, &errorInfo));

	/* Open a TunnelStream with a  255-character name. This should succeed. */
	rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
	tsOpenOpts.name = tsName255;
	tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
	tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
	tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tsOpenOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
	tsOpenOpts.streamId = 5;
	tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
	tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
	tsOpenOpts.userSpecPtr = &consumer;

	OpenedTunnelStreamInfo* pOpenedTsInfo = consumer.openTunnelStream(&provider, &tsOpenOpts);
	ASSERT_TRUE(pOpenedTsInfo != NULL);
	pConsTunnelStream = pOpenedTsInfo->consumerTunnelStream();
	pProvTunnelStream = pOpenedTsInfo->providerTunnelStream();
	delete pOpenedTsInfo;

	ASSERT_EQ(5, pConsTunnelStream->streamId);
	if (!enableWatchlist) /* Watchlist will likely use different stream ID. */
		ASSERT_EQ(5, pProvTunnelStream->streamId);
	ASSERT_TRUE(strncmp(tsName255, pConsTunnelStream->name, strlen(tsName255)) == 0);
	ASSERT_TRUE(strncmp(tsName255, pProvTunnelStream->name, strlen(tsName255)) == 0);

	TestReactorComponent::closeSession(&consumer, &provider);
	consumerReactor.close();
	providerReactor.close();
}

void tunnelStreamMaxMsgSizeTest(bool enableWatchlist)
{
	int maxMsgSizes[] = {1000, 6144, 12000};
	RsslErrorInfo errorInfo;
	RsslTunnelStreamCloseOptions tsCloseOptions;
	RsslTunnelStreamGetBufferOptions tsGetBufferOptions;
	RsslBuffer* pBuffer;

	/* Test getting buffers with different maxMsgSizes. */
	for (int i = 0; i < 3; i++)
	{
		RsslTunnelStream* pConsTunnelStream;
		RsslTunnelStream* pProvTunnelStream;

		/* Create reactors. */
		TestReactor consumerReactor = TestReactor();
		TestReactor providerReactor = TestReactor();

		/* Create consumer. */
		Consumer consumer = Consumer(&consumerReactor);
		RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
		RsslRDMLoginRequest loginRequest;
		rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
		RsslRDMDirectoryRequest directoryRequest;
		rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
		pConsumerRole->pLoginRequest = &loginRequest;
		pConsumerRole->pDirectoryRequest = &directoryRequest;
		pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
		pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
		pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
		pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
		pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
		pConsumerRole->watchlistOptions.enableWatchlist = enableWatchlist;
		
		/* Create provider. */
		TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor);
		TunnelStreamProvider::maxMsgSize(maxMsgSizes[i]);
		TunnelStreamProvider::maxFragmentSize(maxMsgSizes[i]/2);
		RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
		pProviderRole->base.channelEventCallback = provider.channelEventCallback;
		pProviderRole->loginMsgCallback = provider.loginMsgCallback;
		pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
		pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
		pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
		pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;

		/* Connect the consumer and provider. Setup login & directory streams automatically. */
		ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
		opts.setupDefaultLoginStream(true);
		opts.setupDefaultDirectoryStream(true);
		provider.bind(&opts);
		TestReactor::openSession(&consumer, &provider, &opts);

		/* Open a TunnelStream. */
		RsslTunnelStreamOpenOptions tsOpenOpts;
		rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
		char name[] = "Tunnel1";
		tsOpenOpts.name = name;
		tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
		tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
		tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
		tsOpenOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
		tsOpenOpts.streamId = 5;
		tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
		tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
		tsOpenOpts.userSpecPtr = &consumer;

		OpenedTunnelStreamInfo* pOpenedTsInfo = consumer.openTunnelStream(&provider, &tsOpenOpts);
		ASSERT_TRUE(pOpenedTsInfo != NULL);
		pConsTunnelStream = pOpenedTsInfo->consumerTunnelStream();
		pProvTunnelStream = pOpenedTsInfo->providerTunnelStream();
		delete pOpenedTsInfo;

		/* Test tunnel stream accessors */
		ASSERT_EQ(5, pConsTunnelStream->streamId);
		if (!enableWatchlist) /* Watchlist will likely use different stream ID. */
			ASSERT_EQ(5, pProvTunnelStream->streamId);
		ASSERT_EQ(RSSL_DMT_SYSTEM, pConsTunnelStream->domainType);
		ASSERT_EQ(RSSL_DMT_SYSTEM, pProvTunnelStream->domainType);
		ASSERT_EQ(defaultService()->serviceId, pConsTunnelStream->serviceId);
		ASSERT_EQ(defaultService()->serviceId, pProvTunnelStream->serviceId);
		ASSERT_EQ(consumer.reactorChannel(), pConsTunnelStream->pReactorChannel);
		ASSERT_EQ(provider.reactorChannel(), pProvTunnelStream->pReactorChannel);
		ASSERT_TRUE(strncmp("Tunnel1", pConsTunnelStream->name, strlen("Tunnel1")) == 0);
		ASSERT_TRUE(strncmp("Tunnel1", pProvTunnelStream->name, strlen("Tunnel1")) == 0);
		ASSERT_EQ(&consumer, pConsTunnelStream->userSpecPtr);
		ASSERT_EQ(NULL, pProvTunnelStream->userSpecPtr);
		ASSERT_EQ(RSSL_STREAM_OPEN, pConsTunnelStream->state.streamState);
		ASSERT_EQ(RSSL_DATA_OK, pConsTunnelStream->state.dataState);
		ASSERT_EQ(RSSL_SC_NONE, pConsTunnelStream->state.code);
		ASSERT_EQ(RSSL_STREAM_OPEN, pProvTunnelStream->state.streamState);
		ASSERT_EQ(RSSL_DATA_OK, pProvTunnelStream->state.dataState);
		ASSERT_EQ(RSSL_SC_NONE, pProvTunnelStream->state.code);

		/* Test consumer tunnel stream class of service values */
		ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pConsTunnelStream->classOfService.common.protocolType);
		ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pConsTunnelStream->classOfService.common.protocolMajorVersion);
		ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pConsTunnelStream->classOfService.common.protocolMinorVersion);
		ASSERT_EQ(maxMsgSizes[i], pConsTunnelStream->classOfService.common.maxMsgSize);
		ASSERT_EQ(maxMsgSizes[i]/2, pConsTunnelStream->classOfService.common.maxFragmentSize);
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pConsTunnelStream->classOfService.authentication.type);
		ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pConsTunnelStream->classOfService.flowControl.type);
		ASSERT_EQ(RDM_COS_DI_RELIABLE, pConsTunnelStream->classOfService.dataIntegrity.type);
		ASSERT_EQ(RDM_COS_GU_NONE, pConsTunnelStream->classOfService.guarantee.type);

		/* Test provider tunnel stream class of service values */
		ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pProvTunnelStream->classOfService.common.protocolType);
		ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pProvTunnelStream->classOfService.common.protocolMajorVersion);
		ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pProvTunnelStream->classOfService.common.protocolMinorVersion);
		ASSERT_EQ(maxMsgSizes[i], pProvTunnelStream->classOfService.common.maxMsgSize);
		ASSERT_EQ(maxMsgSizes[i]/2, pProvTunnelStream->classOfService.common.maxFragmentSize);
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pProvTunnelStream->classOfService.authentication.type);
		ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pProvTunnelStream->classOfService.flowControl.type);
		ASSERT_EQ(RDM_COS_DI_RELIABLE, pProvTunnelStream->classOfService.dataIntegrity.type);
		ASSERT_EQ(RDM_COS_GU_NONE, pProvTunnelStream->classOfService.guarantee.type);
			
		/* Consumer gets a tunnelstream buffer. This should fail. */
		rsslClearTunnelStreamGetBufferOptions(&tsGetBufferOptions);
		tsGetBufferOptions.size = maxMsgSizes[i] + 1;
		ASSERT_FALSE((pBuffer = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		/* Provider gets a tunnelstream buffer. This should fail. */
		ASSERT_FALSE((pBuffer = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		/* Consumer gets a tunnelstream buffer. This should succeed. */
		tsGetBufferOptions.size = maxMsgSizes[i];
		ASSERT_TRUE((pBuffer = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		RsslTunnelStreamInfo streamInfo;
		rsslClearTunnelStreamInfo(&streamInfo);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pConsTunnelStream, &streamInfo, &errorInfo));
		ASSERT_EQ(1, streamInfo.buffersUsed);

		/* Provider gets a tunnelstream buffer. This should succeed. */
		ASSERT_TRUE((pBuffer = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		rsslClearTunnelStreamInfo(&streamInfo);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
		ASSERT_EQ(1, streamInfo.buffersUsed);

		/* Close the tunnelstreams. */
		rsslClearTunnelStreamCloseOptions(&tsCloseOptions);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pProvTunnelStream, &tsCloseOptions, &errorInfo));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pConsTunnelStream, &tsCloseOptions, &errorInfo));

		TestReactorComponent::closeSession(&consumer, &provider);
		consumerReactor.close();
		providerReactor.close();
	}
}

void tunnelStreamBufferUsedTest(bool enableWatchlist)
{
	int maxMsgSizes[] = { 1000, 6144, 12000 };
	RsslErrorInfo errorInfo;
	RsslTunnelStreamCloseOptions tsCloseOptions;
	RsslTunnelStreamGetBufferOptions tsGetBufferOptions;
	const int BUFFERS_TO_TEST = 5;
	RsslBuffer* pConsBuffers[BUFFERS_TO_TEST]; /* few buffers will be got */
	RsslBuffer* pProvBuffers[BUFFERS_TO_TEST]; /* few buffers will be got */

	/* Test getting buffers with different maxMsgSizes. */
	for (int i = 0; i < 3; i++)
	{
		RsslTunnelStream* pConsTunnelStream;
		RsslTunnelStream* pProvTunnelStream;

		/* Create reactors. */
		TestReactor consumerReactor = TestReactor();
		TestReactor providerReactor = TestReactor();

		/* Create consumer. */
		Consumer consumer = Consumer(&consumerReactor);
		RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
		RsslRDMLoginRequest loginRequest;
		rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
		RsslRDMDirectoryRequest directoryRequest;
		rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
		pConsumerRole->pLoginRequest = &loginRequest;
		pConsumerRole->pDirectoryRequest = &directoryRequest;
		pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
		pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
		pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
		pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
		pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
		pConsumerRole->watchlistOptions.enableWatchlist = enableWatchlist;

		/* Create provider. */
		TunnelStreamProvider provider = TunnelStreamProvider(&providerReactor);
		TunnelStreamProvider::maxMsgSize(maxMsgSizes[i]);
		TunnelStreamProvider::maxFragmentSize(maxMsgSizes[i] / 2);
		RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
		pProviderRole->base.channelEventCallback = provider.channelEventCallback;
		pProviderRole->loginMsgCallback = provider.loginMsgCallback;
		pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
		pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
		pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;
		pProviderRole->tunnelStreamListenerCallback = provider.tunnelStreamListenerCallback;

		/* Connect the consumer and provider. Setup login & directory streams automatically. */
		ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
		opts.setupDefaultLoginStream(true);
		opts.setupDefaultDirectoryStream(true);
		provider.bind(&opts);
		TestReactor::openSession(&consumer, &provider, &opts);

		/* Open a TunnelStream. */
		RsslTunnelStreamOpenOptions tsOpenOpts;
		rsslClearTunnelStreamOpenOptions(&tsOpenOpts);
		char name[] = "Tunnel1";
		tsOpenOpts.name = name;
		tsOpenOpts.statusEventCallback = consumer.tunnelStreamStatusEventCallback;
		tsOpenOpts.defaultMsgCallback = consumer.tunnelStreamDefaultMsgCallback;
		tsOpenOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
		tsOpenOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
		tsOpenOpts.streamId = 5;
		tsOpenOpts.serviceId = (RsslUInt16)defaultService()->serviceId;
		tsOpenOpts.domainType = RSSL_DMT_SYSTEM;
		tsOpenOpts.userSpecPtr = &consumer;

		OpenedTunnelStreamInfo* pOpenedTsInfo = consumer.openTunnelStream(&provider, &tsOpenOpts);
		ASSERT_TRUE(pOpenedTsInfo != NULL);
		pConsTunnelStream = pOpenedTsInfo->consumerTunnelStream();
		pProvTunnelStream = pOpenedTsInfo->providerTunnelStream();
		delete pOpenedTsInfo;

		/* Test tunnel stream accessors */
		ASSERT_EQ(5, pConsTunnelStream->streamId);
		if (!enableWatchlist) /* Watchlist will likely use different stream ID. */
			ASSERT_EQ(5, pProvTunnelStream->streamId);
		ASSERT_EQ(RSSL_DMT_SYSTEM, pConsTunnelStream->domainType);
		ASSERT_EQ(RSSL_DMT_SYSTEM, pProvTunnelStream->domainType);
		ASSERT_EQ(defaultService()->serviceId, pConsTunnelStream->serviceId);
		ASSERT_EQ(defaultService()->serviceId, pProvTunnelStream->serviceId);
		ASSERT_EQ(consumer.reactorChannel(), pConsTunnelStream->pReactorChannel);
		ASSERT_EQ(provider.reactorChannel(), pProvTunnelStream->pReactorChannel);
		ASSERT_TRUE(strncmp("Tunnel1", pConsTunnelStream->name, strlen("Tunnel1")) == 0);
		ASSERT_TRUE(strncmp("Tunnel1", pProvTunnelStream->name, strlen("Tunnel1")) == 0);
		ASSERT_EQ(&consumer, pConsTunnelStream->userSpecPtr);
		ASSERT_EQ(NULL, pProvTunnelStream->userSpecPtr);
		ASSERT_EQ(RSSL_STREAM_OPEN, pConsTunnelStream->state.streamState);
		ASSERT_EQ(RSSL_DATA_OK, pConsTunnelStream->state.dataState);
		ASSERT_EQ(RSSL_SC_NONE, pConsTunnelStream->state.code);
		ASSERT_EQ(RSSL_STREAM_OPEN, pProvTunnelStream->state.streamState);
		ASSERT_EQ(RSSL_DATA_OK, pProvTunnelStream->state.dataState);
		ASSERT_EQ(RSSL_SC_NONE, pProvTunnelStream->state.code);

		/* Test consumer tunnel stream class of service values */
		ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pConsTunnelStream->classOfService.common.protocolType);
		ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pConsTunnelStream->classOfService.common.protocolMajorVersion);
		ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pConsTunnelStream->classOfService.common.protocolMinorVersion);
		ASSERT_EQ(maxMsgSizes[i], pConsTunnelStream->classOfService.common.maxMsgSize);
		ASSERT_EQ(maxMsgSizes[i] / 2, pConsTunnelStream->classOfService.common.maxFragmentSize);
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pConsTunnelStream->classOfService.authentication.type);
		ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pConsTunnelStream->classOfService.flowControl.type);
		ASSERT_EQ(RDM_COS_DI_RELIABLE, pConsTunnelStream->classOfService.dataIntegrity.type);
		ASSERT_EQ(RDM_COS_GU_NONE, pConsTunnelStream->classOfService.guarantee.type);

		/* Test provider tunnel stream class of service values */
		ASSERT_EQ(RSSL_RWF_PROTOCOL_TYPE, pProvTunnelStream->classOfService.common.protocolType);
		ASSERT_EQ(RSSL_RWF_MAJOR_VERSION, pProvTunnelStream->classOfService.common.protocolMajorVersion);
		ASSERT_EQ(RSSL_RWF_MINOR_VERSION, pProvTunnelStream->classOfService.common.protocolMinorVersion);
		ASSERT_EQ(maxMsgSizes[i], pProvTunnelStream->classOfService.common.maxMsgSize);
		ASSERT_EQ(maxMsgSizes[i] / 2, pProvTunnelStream->classOfService.common.maxFragmentSize);
		ASSERT_EQ(RDM_COS_AU_NOT_REQUIRED, pProvTunnelStream->classOfService.authentication.type);
		ASSERT_EQ(RDM_COS_FC_BIDIRECTIONAL, pProvTunnelStream->classOfService.flowControl.type);
		ASSERT_EQ(RDM_COS_DI_RELIABLE, pProvTunnelStream->classOfService.dataIntegrity.type);
		ASSERT_EQ(RDM_COS_GU_NONE, pProvTunnelStream->classOfService.guarantee.type);

		/* Consumer gets a tunnelstream buffer. This should fail. */
		rsslClearTunnelStreamGetBufferOptions(&tsGetBufferOptions);
		tsGetBufferOptions.size = maxMsgSizes[i] + 1;
		ASSERT_FALSE((pConsBuffers[0] = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		/* Provider gets a tunnelstream buffer. This should fail. */
		ASSERT_FALSE((pProvBuffers[0] = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		/* Consumer gets a tunnelstream buffer. This should succeed. */
		tsGetBufferOptions.size = maxMsgSizes[i];
		ASSERT_TRUE((pConsBuffers[0] = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		RsslTunnelStreamInfo streamInfo;
		rsslClearTunnelStreamInfo(&streamInfo);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pConsTunnelStream, &streamInfo, &errorInfo));
		ASSERT_EQ(1, streamInfo.buffersUsed);

		/* Provider gets a tunnelstream buffer. This should succeed. */
		ASSERT_TRUE((pProvBuffers[0] = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

		rsslClearTunnelStreamInfo(&streamInfo);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
		ASSERT_EQ(1, streamInfo.buffersUsed);

		/* repeate getting tunnelstream buffers to check GetInfo value consistency */
		for (int i = 1; i < BUFFERS_TO_TEST; ++i)
		{
			ASSERT_TRUE((pConsBuffers[i] = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pConsTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i + 1, streamInfo.buffersUsed);

			ASSERT_TRUE((pProvBuffers[i] = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i + 1, streamInfo.buffersUsed);

		}

		/* release the tunnelstream buffers and check the numbers */
		for (int i = BUFFERS_TO_TEST - 1; i >= 0; --i)
		{
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamReleaseBuffer(pConsBuffers[i], &errorInfo));

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pConsTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i, streamInfo.buffersUsed);

			/* check it twice : provider buffer is not released yet */
			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i + 1, streamInfo.buffersUsed);

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamReleaseBuffer(pProvBuffers[i], &errorInfo));

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i, streamInfo.buffersUsed);
		}

		/* do it again : getting tunnelstream buffers to check GetInfo value consistency */
		for (int i = 0; i < BUFFERS_TO_TEST; ++i)
		{
			ASSERT_TRUE((pConsBuffers[i] = rsslTunnelStreamGetBuffer(pConsTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pConsTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i + 1, streamInfo.buffersUsed);

			ASSERT_TRUE((pProvBuffers[i] = rsslTunnelStreamGetBuffer(pProvTunnelStream, &tsGetBufferOptions, &errorInfo)) != NULL);

			rsslClearTunnelStreamInfo(&streamInfo);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamGetInfo(pProvTunnelStream, &streamInfo, &errorInfo));
			ASSERT_EQ(i + 1, streamInfo.buffersUsed);

		}

		/* Close the tunnelstreams. */
		rsslClearTunnelStreamCloseOptions(&tsCloseOptions);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pProvTunnelStream, &tsCloseOptions, &errorInfo));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pConsTunnelStream, &tsCloseOptions, &errorInfo));

		TestReactorComponent::closeSession(&consumer, &provider);
		consumerReactor.close();
		providerReactor.close();
	}
}

TEST(ReactorInteraction, SimpleRequestTest_Watchlist)
{
	/* Test a simple request/refresh exchange with the watchlist enabled. */

	TestReactorEvent* pEvent;
	RsslMsgEvent* pMsgEvent;
	RsslRequestMsg requestMsg;
	RsslRequestMsg* pReceivedRequestMsg;
	RsslRefreshMsg refreshMsg;
	RsslRefreshMsg* pReceivedRefreshMsg;
	int providerStreamId;
	RsslReactorSubmitMsgOptions submitMsgOptions;
				
	/* Create reactors. */
	TestReactor consumerReactor = TestReactor();
	TestReactor providerReactor = TestReactor();
				
	/* Create consumer. */
	Consumer consumer = Consumer(&consumerReactor);
	RsslReactorOMMConsumerRole* pConsumerRole = &consumer.reactorRole()->ommConsumerRole;
	RsslRDMLoginRequest loginRequest;
	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	RsslRDMDirectoryRequest directoryRequest;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	pConsumerRole->pLoginRequest = &loginRequest;
	pConsumerRole->pDirectoryRequest = &directoryRequest;
	pConsumerRole->base.channelEventCallback = consumer.channelEventCallback;
	pConsumerRole->loginMsgCallback = consumer.loginMsgCallback;
	pConsumerRole->directoryMsgCallback = consumer.directoryMsgCallback;
	pConsumerRole->dictionaryMsgCallback = consumer.dictionaryMsgCallback;
	pConsumerRole->base.defaultMsgCallback = consumer.defaultMsgCallback;
	pConsumerRole->watchlistOptions.enableWatchlist = RSSL_TRUE;
	pConsumerRole->watchlistOptions.channelOpenCallback = consumer.channelOpenCallback;
		
	/* Create provider. */
	Provider provider = Provider(&providerReactor);
	RsslReactorOMMProviderRole* pProviderRole = &provider.reactorRole()->ommProviderRole;
	pProviderRole->base.channelEventCallback = provider.channelEventCallback;
	pProviderRole->loginMsgCallback = provider.loginMsgCallback;
	pProviderRole->directoryMsgCallback = provider.directoryMsgCallback;
	pProviderRole->dictionaryMsgCallback = provider.dictionaryMsgCallback;
	pProviderRole->base.defaultMsgCallback = provider.defaultMsgCallback;

	/* Connect the consumer and provider. Setup login & directory streams automatically. */
	ConsumerProviderSessionOptions opts = ConsumerProviderSessionOptions();
	opts.setupDefaultLoginStream(true);
	opts.setupDefaultDirectoryStream(true);
	provider.bind(&opts);
	TestReactor::openSession(&consumer, &provider, &opts);
		
	/* Consumer sends request. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	requestMsg.msgBase.streamId = 5;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	char name[] = "TRI.N";
	requestMsg.msgBase.msgKey.name.data = name;
	requestMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(name);
	rsslClearReactorSubmitMsgOptions(&submitMsgOptions);
	submitMsgOptions.pServiceName = &defaultService()->info.serviceName;
	submitMsgOptions.pRsslMsg = (RsslMsg *)&requestMsg;
	ASSERT_TRUE(consumer.submitAndDispatch(&submitMsgOptions) >= RSSL_RET_SUCCESS);

	/* Provider receives request. */
	provider.testReactor()->dispatch(1);
	pEvent = provider.testReactor()->pollEvent();
	ASSERT_EQ(DEFAULT_MSG, pEvent->type());
	pMsgEvent = (RsslMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_MC_REQUEST, pMsgEvent->pRsslMsg->msgBase.msgClass);
		
	pReceivedRequestMsg = (RsslRequestMsg *)pMsgEvent->pRsslMsg;
	ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pReceivedRequestMsg->msgBase.msgKey));
	ASSERT_TRUE(rsslRequestMsgCheckStreaming(pReceivedRequestMsg));
	ASSERT_FALSE(rsslRequestMsgCheckNoRefresh(pReceivedRequestMsg));
	ASSERT_EQ(defaultService()->serviceId, pReceivedRequestMsg->msgBase.msgKey.serviceId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&pReceivedRequestMsg->msgBase.msgKey));
	ASSERT_TRUE(strncmp(pReceivedRequestMsg->msgBase.msgKey.name.data, "TRI.N", pReceivedRequestMsg->msgBase.msgKey.name.length) == 0);
	ASSERT_EQ(RSSL_DMT_MARKET_PRICE, pReceivedRequestMsg->msgBase.domainType);
		
	providerStreamId = pReceivedRequestMsg->msgBase.streamId;
	delete pEvent;
		
	/* Provider sends refresh .*/
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME;
	refreshMsg.msgBase.msgKey.serviceId = (RsslUInt16)defaultService()->serviceId;
	refreshMsg.msgBase.msgKey.name.data = name;
	refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(name);
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	rsslClearReactorSubmitMsgOptions(&submitMsgOptions);
	submitMsgOptions.pRsslMsg = (RsslMsg *)&refreshMsg;
	ASSERT_TRUE(provider.submitAndDispatch(&submitMsgOptions) >= RSSL_RET_SUCCESS);
		
	/* Consumer receives refresh. */
	consumer.testReactor()->dispatch(1);
	pEvent = consumer.testReactor()->pollEvent();
	ASSERT_EQ(DEFAULT_MSG, pEvent->type());
	pMsgEvent = (RsslMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_MC_REFRESH, pMsgEvent->pRsslMsg->msgBase.msgClass);

	pReceivedRefreshMsg = (RsslRefreshMsg *)pMsgEvent->pRsslMsg;
	ASSERT_TRUE(rsslRefreshMsgCheckHasMsgKey(pReceivedRefreshMsg));
	ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pReceivedRefreshMsg->msgBase.msgKey));
	ASSERT_EQ(defaultService()->serviceId, pReceivedRefreshMsg->msgBase.msgKey.serviceId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&pReceivedRefreshMsg->msgBase.msgKey));
	ASSERT_TRUE(strncmp(pReceivedRefreshMsg->msgBase.msgKey.name.data, "TRI.N", pReceivedRefreshMsg->msgBase.msgKey.name.length) == 0);
	ASSERT_EQ(RSSL_DMT_MARKET_PRICE, pReceivedRefreshMsg->msgBase.domainType);
	ASSERT_EQ(RSSL_DT_NO_DATA, pReceivedRefreshMsg->msgBase.containerType);
	ASSERT_EQ(RSSL_STREAM_OPEN, pReceivedRefreshMsg->state.streamState);
	ASSERT_EQ(RSSL_DATA_OK, pReceivedRefreshMsg->state.dataState);
	ASSERT_TRUE(pMsgEvent->pStreamInfo != NULL);
	ASSERT_TRUE(pMsgEvent->pStreamInfo->pServiceName != NULL);
	ASSERT_TRUE(strncmp(pMsgEvent->pStreamInfo->pServiceName->data, defaultService()->info.serviceName.data, pMsgEvent->pStreamInfo->pServiceName->length) == 0);
	delete pEvent;
		
	TestReactorComponent::closeSession(&consumer, &provider);
}
