/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"
#include "ReadEvent.h"
#include "CoreComponent.h"
#include "TunnelStreamCoreProvider.h"
#include "TunnelStreamProvider.h"
#include "TestUtil.h"
#include "gtest/gtest.h"

using namespace testing;

Consumer::Consumer(TestReactor* pTestReactor) : TestReactorComponent(pTestReactor)
{
	_ackRangeList = { 0 };
	_nakRangeList = { 0 };
	rsslClearOMMConsumerRole(&_consumerRole);
	_pReactorRole = (RsslReactorChannelRole *)&_consumerRole;
}

RsslReactorCallbackRet Consumer::channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleChannelEvent(pEvent);
}

RsslReactorCallbackRet Consumer::channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleChannelEvent(pEvent);
}

RsslReactorCallbackRet Consumer::defaultMsgCallback(RsslReactor*pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleDefaultMsgEvent(pEvent);
}

RsslReactorCallbackRet Consumer::loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleLoginMsgEvent(pEvent);
}

RsslReactorCallbackRet Consumer::directoryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleDirectoryMsgEvent(pEvent);
}

RsslReactorCallbackRet Consumer::dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleDictionaryMsgEvent(pEvent);
}

RsslReactorCallbackRet Consumer::tunnelStreamDefaultMsgCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pTunnelStream->pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleTunnelStreamMsgEvent(pTunnelStream, pEvent);
}

RsslReactorCallbackRet Consumer::tunnelStreamStatusEventCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent)
{
	TestReactor* pTestReactor = ((Consumer *)pTunnelStream->pReactorChannel->userSpecPtr)->testReactor();
	EXPECT_TRUE(pTestReactor != NULL);
	return (RsslReactorCallbackRet)pTestReactor->handleTunnelStreamStatusEvent(pTunnelStream, pEvent);
}
	
OpenedTunnelStreamInfo* Consumer::openTunnelStream(TunnelStreamProvider* pProvider, RsslTunnelStreamOpenOptions* pTsOpenOpts)
{
	RsslReactorChannel* pReactorChannel = reactorChannel(); 
	RsslTunnelStream* pConsTunnelStream;
	RsslTunnelStream* pProvTunnelStream;
	TestReactorEvent* pEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslTunnelStreamRequestEvent* pTsRequestEvent;

	/* Open a RsslTunnelStream. */
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslReactorOpenTunnelStream(pReactorChannel, pTsOpenOpts, &_errorInfo));
	testReactor()->dispatch(0);
		
	/* Provider receives tunnel stream request event. */
	pProvider->testReactor()->dispatch(2);
	pEvent = pProvider->testReactor()->pollEvent();
	EXPECT_EQ(TUNNEL_STREAM_REQUEST, pEvent->type());
	pTsRequestEvent = pEvent->tunnelStreamRequestEvent();
	EXPECT_TRUE(strncmp(pTsOpenOpts->name, pTsRequestEvent->name, strlen(pTsOpenOpts->name)) == 0);
	EXPECT_EQ(defaultService()->serviceId, pTsRequestEvent->serviceId);
	EXPECT_EQ(pProvider->reactorChannel(), pTsRequestEvent->pReactorChannel);
	delete pEvent;
 
	/* Provider already accepted in its callback, so it should get the status event as well. */
	pEvent = pProvider->testReactor()->pollEvent();
	EXPECT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	EXPECT_EQ(NULL, pTsStatusEvent->pAuthInfo);
	EXPECT_EQ(pProvider->reactorChannel(), pTsStatusEvent->pReactorChannel);
	EXPECT_EQ(RSSL_STREAM_OPEN, pTsStatusEvent->pState->streamState);
	EXPECT_EQ(RSSL_DATA_OK, pTsStatusEvent->pState->dataState);
	EXPECT_EQ(RSSL_SC_NONE, pTsStatusEvent->pState->code);
	pProvTunnelStream = pEvent->tunnelStream();
	EXPECT_TRUE(pProvTunnelStream != NULL);
	delete pEvent;

	if (pTsOpenOpts->classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN)
	{
		RsslMsg msg;
		RsslRDMLoginRequest loginRequest;
		RsslRDMLoginRefresh loginRefresh;
		RsslDecodeIterator dIter;
		RsslErrorInfo errorInfo;
		RsslTunnelStreamSubmitMsgOptions tsSubmitMsgOptions;

		/* Consumer receives nothing yet (sends authentication request). */
		testReactor()->dispatch(0);
		testReactor()->dispatch(0);

		/* Provider receives authentication login request. */
		pProvider->testReactor()->dispatch(1);
		pEvent = pProvider->testReactor()->pollEvent();
		EXPECT_EQ(TUNNEL_STREAM_MSG, pEvent->type());
		RsslTunnelStreamMsgEvent* pTsMsgEvent = (RsslTunnelStreamMsgEvent *)pEvent->reactorEvent();
		EXPECT_EQ(RSSL_DT_MSG, pTsMsgEvent->containerType);
		EXPECT_TRUE(pTsMsgEvent->pRsslMsg != NULL);
		EXPECT_TRUE(pTsMsgEvent->pRsslBuffer  != NULL);
		rsslClearDecodeIterator(&dIter);
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorRWFVersion(&dIter, pProvTunnelStream->classOfService.common.protocolMajorVersion, pProvTunnelStream->classOfService.common.protocolMinorVersion));
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorBuffer(&dIter, pTsMsgEvent->pRsslBuffer));
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&dIter, &msg));
		delete pEvent;

		EXPECT_EQ(RSSL_MC_REQUEST, msg.msgBase.msgClass);
		EXPECT_EQ(RSSL_DMT_LOGIN, msg.msgBase.domainType);
		loginRequest.rdmMsgBase.rdmMsgType = RDM_LG_MT_REQUEST;
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMLoginMsg(&dIter, &msg, (RsslRDMLoginMsg *)&loginRequest, NULL, &errorInfo));

		/* Provider sends login refresh. */
		rsslClearRDMLoginRefresh(&loginRefresh);
		loginRefresh.rdmMsgBase.rdmMsgType = RDM_LG_MT_REFRESH;
		loginRefresh.rdmMsgBase.streamId = loginRequest.rdmMsgBase.streamId;
		loginRefresh.state.streamState = RSSL_STREAM_OPEN;
		loginRefresh.state.dataState = RSSL_DATA_OK;
		loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;
		loginRefresh.userName = loginRequest.userName;
		loginRefresh.flags |= RDM_LG_RFF_SOLICITED;

		if (loginRequest.flags & RDM_LG_RFF_HAS_USERNAME_TYPE)
		{
			loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
			loginRefresh.userNameType = loginRequest.userNameType;
		}

		rsslClearTunnelStreamSubmitMsgOptions(&tsSubmitMsgOptions);
		tsSubmitMsgOptions.pRDMMsg = (RsslRDMMsg *)&loginRefresh;
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslTunnelStreamSubmitMsg(pProvTunnelStream, &tsSubmitMsgOptions, &errorInfo));
		pProvider->testReactor()->dispatch(0);
	}

	/* Consumer receives tunnel stream status event */
	testReactor()->dispatch(1);
	pEvent = testReactor()->pollEvent();
	EXPECT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	EXPECT_EQ(reactorChannel(), pTsStatusEvent->pReactorChannel);
	EXPECT_EQ(RSSL_STREAM_OPEN, pTsStatusEvent->pState->streamState);
	EXPECT_EQ(RSSL_DATA_OK, pTsStatusEvent->pState->dataState);
	EXPECT_EQ(RSSL_SC_NONE, pTsStatusEvent->pState->code);  
	pConsTunnelStream = pEvent->tunnelStream();
	EXPECT_TRUE(pConsTunnelStream != NULL);

	if (pTsOpenOpts->classOfService.authentication.type == RDM_COS_AU_OMM_LOGIN)
	{
		/* A LoginRefresh should be present. */
		EXPECT_TRUE(pTsStatusEvent->pAuthInfo != NULL);
		EXPECT_TRUE(pTsStatusEvent->pAuthInfo->pLoginMsg != NULL);
		EXPECT_EQ(RDM_LG_MT_REFRESH, pTsStatusEvent->pAuthInfo->pLoginMsg->rdmMsgBase.rdmMsgType);

		RsslRDMLoginRefresh* pLoginRefresh = (RsslRDMLoginRefresh *)pTsStatusEvent->pAuthInfo->pLoginMsg;
		EXPECT_EQ(RSSL_STREAM_OPEN, pLoginRefresh->state.streamState);
		EXPECT_EQ(RSSL_DATA_OK, pLoginRefresh->state.dataState);
		EXPECT_EQ(RSSL_SC_NONE, pLoginRefresh->state.code);
	}
	else
	{
		EXPECT_TRUE(pTsStatusEvent->pAuthInfo == NULL);
	}
	delete pEvent;

	return new OpenedTunnelStreamInfo(pConsTunnelStream, pProvTunnelStream, pProvTunnelStream->streamId);
}
	
OpenedTunnelStreamInfo* Consumer::openTunnelStream(TunnelStreamCoreProvider* pProvider, RsslTunnelStreamOpenOptions* pTsOpenOpts, RsslClassOfService* pProvClassOfService)
{
	RsslReactorChannel* pReactorChannel = reactorChannel(); 
	TestReactorEvent* pEvent;
	ReadEvent* pReadEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslMsg* pMsg;
	RsslTunnelStream* pTunnelStream;
		
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslReactorOpenTunnelStream(pReactorChannel, pTsOpenOpts, &_errorInfo));
	testReactor()->dispatch(0);
		
	/* Provider receives tunnel stream request event. */
	pProvider->dispatch(1);
	pReadEvent = (ReadEvent *)pProvider->pollEvent();
	pMsg = pReadEvent->msg();
				
	EXPECT_EQ(RSSL_MC_REQUEST, pMsg->msgBase.msgClass);
	EXPECT_EQ(pTsOpenOpts->domainType, pMsg->msgBase.domainType);
	pProvider->acceptTunnelStreamRequest((RsslRequestMsg *)pMsg, pProvClassOfService);
	delete pReadEvent;
		
	/* Consumer receives tunnel stream status event. */
	testReactor()->dispatch(1);
	pEvent = testReactor()->pollEvent();
	EXPECT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	EXPECT_EQ(NULL, pTsStatusEvent->pAuthInfo);
	EXPECT_EQ(reactorChannel(), pTsStatusEvent->pReactorChannel);
	EXPECT_EQ(RSSL_STREAM_OPEN, pTsStatusEvent->pState->streamState);
	EXPECT_EQ(RSSL_DATA_OK, pTsStatusEvent->pState->dataState);
	EXPECT_EQ(RSSL_SC_NONE, (RsslStateCodes)pTsStatusEvent->pState->code);  
	pTunnelStream = pEvent->tunnelStream();
	EXPECT_TRUE(pTunnelStream != NULL);
	delete pEvent;

	testReactor()->dispatch(0);
		
	return new OpenedTunnelStreamInfo(pTunnelStream, NULL, pMsg->msgBase.streamId);
}

void Consumer::closeTunnelStream(TunnelStreamProvider* pProvider, RsslTunnelStream* pConsTunnelStream, RsslTunnelStream* pProvTunnelStream, bool finalStatusEvent)
{
	TestReactorEvent* pEvent;
	RsslTunnelStreamStatusEvent* pTsStatusEvent;
	RsslTunnelStreamCloseOptions tunnelStreamCloseOptions;
		
	/* Close tunnel stream from consumer. */
	tunnelStreamCloseOptions.finalStatusEvent = finalStatusEvent;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pConsTunnelStream, &tunnelStreamCloseOptions, &_errorInfo));
	testReactor()->dispatch(0);
		
	/* Provider receives & acks FIN, and gets open/suspect status. */
	pProvider->testReactor()->dispatch(1);
	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_STREAM_OPEN, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_EQ(pProvTunnelStream, pEvent->tunnelStream());
	pProvider->testReactor()->dispatch(0);
	delete pEvent;
		
	/* Consumer receives tunnel ack, and tunnel fin, sends close. */
	if (finalStatusEvent)
	{
		/* Consumer receives close status. */
		testReactor()->dispatch(1);
		pEvent = testReactor()->pollEvent();
		ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
		pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_STREAM_CLOSED_RECOVER, pTsStatusEvent->pState->streamState);
		ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
		ASSERT_EQ(pConsTunnelStream, pEvent->tunnelStream());
		delete pEvent;
	}
	else
		testReactor()->dispatch(0);
		
	/* Provider receives tunnel ack, tunnel fin, and close. */
	pProvider->testReactor()->dispatch(1);
	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
	pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_STREAM_CLOSED, pTsStatusEvent->pState->streamState);
	ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
	ASSERT_EQ(pProvTunnelStream, pEvent->tunnelStream());
	delete pEvent;
}
	
void Consumer::closeTunnelStream(TunnelStreamCoreProvider* pProvider, RsslTunnelStream* pConsTunnelStream, RsslInt provTunnelStreamId, RsslInt provFinSeqNum, bool finalStatusEvent)
{
	TunnelStreamMsg* pTunnelMsg;
	TestReactorEvent* pEvent;
	ReadEvent* pReadEvent;
	RsslMsg* pMsg;
	TunnelStreamAck tunnelStreamAck;
	RsslTunnelStreamCloseOptions tunnelStreamCloseOptions;

	/* Close tunnel stream from consumer. */
	tunnelStreamCloseOptions.finalStatusEvent = finalStatusEvent;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseTunnelStream(pConsTunnelStream, &tunnelStreamCloseOptions, &_errorInfo));
	testReactor()->dispatch(0);
		
	/* Provider receives FIN. */
	pProvider->dispatch(1);
	pReadEvent = (ReadEvent *)pProvider->pollEvent();
	pMsg = pReadEvent->msg();
	pTunnelMsg = pReadEvent->tunnelMsg(pMsg, &_ackRangeList, &_nakRangeList);
	ASSERT_EQ(TS_MC_ACK, pTunnelMsg->base.opcode);
	ASSERT_EQ(provTunnelStreamId, pTunnelMsg->base.streamId);
	ASSERT_EQ(pConsTunnelStream->domainType, pTunnelMsg->base.domainType);
	ASSERT_EQ(pTunnelMsg->ackHeader.flags, 0x1);
		
	/* Provider acks FIN. */
	tunnelStreamAckClear(&tunnelStreamAck);
	tunnelStreamAck.recvWindow = 12288;
	tunnelStreamAck.seqNum = pTunnelMsg->ackHeader.seqNum;
	tunnelStreamAck.base.streamId = (RsslInt32)provTunnelStreamId;
	tunnelStreamAck.base.domainType = pConsTunnelStream->domainType;
	pProvider->submitTunnelStreamAck(&tunnelStreamAck);
		
	/* Provider sends FIN. */
	tunnelStreamAckClear(&tunnelStreamAck);
	tunnelStreamAck.recvWindow = 12288;
	tunnelStreamAck.base.streamId = (RsslInt32)provTunnelStreamId;
	tunnelStreamAck.base.domainType = pConsTunnelStream->domainType;
	tunnelStreamAck.seqNum = (RsslUInt32)provFinSeqNum;
	pProvider->submitTunnelStreamAck(&tunnelStreamAck, NULL, NULL, 0x1);
	delete pReadEvent;
		
	/* Consumer receives tunnel ack, and tunnel fin, sends close. */
	if (finalStatusEvent)
	{
		/* Consumer receives close status. */
		RsslTunnelStreamStatusEvent* pTsStatusEvent;
		testReactor()->dispatch(1);
		pEvent = testReactor()->pollEvent();
		ASSERT_EQ(TUNNEL_STREAM_STATUS, pEvent->type());
		pTsStatusEvent = (RsslTunnelStreamStatusEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_STREAM_CLOSED_RECOVER, pTsStatusEvent->pState->streamState);
		ASSERT_EQ(RSSL_DATA_SUSPECT, pTsStatusEvent->pState->dataState);
		ASSERT_EQ(pConsTunnelStream, pEvent->tunnelStream());
		delete pEvent;
	}
	else
		testReactor()->dispatch(0);
		
	/* Provider receives ack of FIN. */
	pProvider->dispatch(2);
	pReadEvent = (ReadEvent *)pProvider->pollEvent();
	pMsg = pReadEvent->msg();
	pTunnelMsg = pReadEvent->tunnelMsg(pMsg, &_ackRangeList, &_nakRangeList);
	ASSERT_EQ(TS_MC_ACK, pTunnelMsg->base.opcode);
	ASSERT_EQ(provTunnelStreamId, pTunnelMsg->base.streamId);
	ASSERT_EQ(pConsTunnelStream->domainType, pTunnelMsg->base.domainType);
	ASSERT_EQ(provFinSeqNum, pTunnelMsg->ackHeader.seqNum);
	delete pReadEvent;
		
	/* Provider receives close. */
	pReadEvent = (ReadEvent *)pProvider->pollEvent();
	pMsg = pReadEvent->msg();
	ASSERT_EQ(RSSL_MC_CLOSE, pMsg->msgBase.msgClass);
	delete pReadEvent;
}
