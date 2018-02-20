/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtil.h"
#include "Consumer.h"
#include "CoreComponent.h"
#include "ReadEvent.h"
#include "gtest/gtest.h"

using namespace testing;

// space RDM messages
RsslBuffer _loginMsgBuffer;
char _loginMsgBufferMemory[1024];
RsslBuffer _directoryMsgBuffer;
char _directoryMsgBufferMemory[1024];
RsslBuffer _dictionaryMsgBuffer;
char _dictionaryMsgBufferMemory[1024];
RsslBuffer _tsMsgBuffer;
char _tsMsgBufferMemory[1024];
RsslBuffer _msgBuffer;
char _msgBufferMemory[1024];
RsslState _state;
RsslTunnelStreamAuthInfo _authInfo;
RsslRDMLoginMsg _loginMsg;
char serviceNameBufferData[] = "DEFAULT_SERVICE";

/* A default service that can be used when setting up a connection. */
RsslRDMService _defaultService;
RsslBuffer serviceNameBuffer;
RsslUInt serviceCapabilities[5];
RsslQos qosList[2];
bool serviceCreated;

RsslRDMService* defaultService()
{
	if (serviceCreated != true)
	{
		serviceCreated = true;
		rsslClearRDMService(&_defaultService);
		_defaultService.flags |= RDM_SVCF_HAS_INFO;
		_defaultService.flags |= RDM_SVCF_HAS_STATE;
		_defaultService.serviceId = 1;

		serviceNameBuffer.data = serviceNameBufferData;
		serviceNameBuffer.length = (RsslUInt32)strlen(serviceNameBufferData);
		_defaultService.info.serviceName = serviceNameBuffer;

		serviceCapabilities[0] = RSSL_DMT_DICTIONARY;
		serviceCapabilities[1] = RSSL_DMT_MARKET_PRICE;
		serviceCapabilities[2] = RSSL_DMT_MARKET_BY_ORDER;
		serviceCapabilities[3] = RSSL_DMT_SYMBOL_LIST;
		serviceCapabilities[4] = RSSL_DMT_SYSTEM;
		_defaultService.info.capabilitiesList = serviceCapabilities;
		_defaultService.info.capabilitiesCount = 5;

		_defaultService.info.flags = RDM_SVC_IFF_HAS_QOS;
		rsslClearQos(&qosList[0]);
		qosList[0].timeliness = RSSL_QOS_TIME_DELAYED;
		qosList[0].rate = RSSL_QOS_RATE_JIT_CONFLATED;
		qosList[0].dynamic = false;
		qosList[0].timeInfo = 0;
		qosList[0].rateInfo = 0;
		rsslClearQos(&qosList[1]);
		qosList[1].timeliness = RSSL_QOS_TIME_REALTIME;
		qosList[1].rate = RSSL_QOS_RATE_TICK_BY_TICK;
		qosList[1].dynamic = false;
		qosList[1].timeInfo = 0;
		qosList[1].rateInfo = 0;
		_defaultService.info.qosList = qosList;
		_defaultService.info.qosCount = 2;
		
		_defaultService.state.flags = RDM_SVC_STF_HAS_ACCEPTING_REQS;
		_defaultService.state.acceptingRequests = 1;
		_defaultService.state.serviceState = 1;
	}

	return &_defaultService;
}

void copyMsgEvent(RsslMsgEvent* srcEvent, RsslMsgEvent* destEvent)
{
	if (srcEvent->pRsslMsg != NULL)
	{
		_msgBuffer.data =  _msgBufferMemory;
		_msgBuffer.length = sizeof(_msgBufferMemory);
		destEvent->pRsslMsg = rsslCopyMsg(srcEvent->pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, &_msgBuffer);
	}
	else
	{
		destEvent->pRsslMsg = NULL;
	}

	if (srcEvent->pStreamInfo != NULL)
	{
		destEvent->pStreamInfo->pUserSpec = srcEvent->pStreamInfo->pUserSpec;

		if (srcEvent->pStreamInfo->pServiceName != NULL)
			destEvent->pStreamInfo->pServiceName = srcEvent->pStreamInfo->pServiceName;
	}
	else
	{
		destEvent->pStreamInfo = NULL;
	}
}

void copyLoginMsgEvent(RsslRDMLoginMsgEvent* srcEvent, RsslRDMLoginMsgEvent* destEvent)
{
	copyMsgEvent(&srcEvent->baseMsgEvent, &destEvent->baseMsgEvent);

	if (srcEvent->pRDMLoginMsg != NULL)
	{
		_loginMsgBuffer.data = _loginMsgBufferMemory;
		_loginMsgBuffer.length = sizeof(_loginMsgBufferMemory);
		rsslCopyRDMLoginMsg(destEvent->pRDMLoginMsg, srcEvent->pRDMLoginMsg, &_loginMsgBuffer);
	}
	else
	{
		destEvent->pRDMLoginMsg = NULL;
	}
}

void copyDirectoryMsgEvent(RsslRDMDirectoryMsgEvent* srcEvent, RsslRDMDirectoryMsgEvent* destEvent)
{
	copyMsgEvent(&srcEvent->baseMsgEvent, &destEvent->baseMsgEvent);

	if (srcEvent->pRDMDirectoryMsg != NULL)
	{
		_directoryMsgBuffer.data = _directoryMsgBufferMemory;
		_directoryMsgBuffer.length = sizeof(_directoryMsgBufferMemory);
		rsslCopyRDMDirectoryMsg(destEvent->pRDMDirectoryMsg, srcEvent->pRDMDirectoryMsg, &_directoryMsgBuffer);
	}
	else
	{
		destEvent->pRDMDirectoryMsg = NULL;
	}
}

void copyDictionaryMsgEvent(RsslRDMDictionaryMsgEvent* srcEvent, RsslRDMDictionaryMsgEvent* destEvent)
{
	copyMsgEvent(&srcEvent->baseMsgEvent, &destEvent->baseMsgEvent);

	if (srcEvent->pRDMDictionaryMsg != NULL)
	{
		_dictionaryMsgBuffer.data = _dictionaryMsgBufferMemory;
		_dictionaryMsgBuffer.length = sizeof(_dictionaryMsgBufferMemory);
		rsslCopyRDMDictionaryMsg(destEvent->pRDMDictionaryMsg, srcEvent->pRDMDictionaryMsg, &_directoryMsgBuffer);
	}
	else
	{
		destEvent->pRDMDictionaryMsg = NULL;
	}
}

void copyTunnelStreamStatusEvent(RsslTunnelStreamStatusEvent* srcEvent, RsslTunnelStreamStatusEvent* destEvent)
{
	if (srcEvent->pRsslMsg != NULL)
	{
		_msgBuffer.data =  _msgBufferMemory;
		_msgBuffer.length = sizeof(_msgBufferMemory);
		destEvent->pRsslMsg = rsslCopyMsg(srcEvent->pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, &_msgBuffer);
	}
	else
	{
		destEvent->pRsslMsg =  NULL;
	}

	if (srcEvent->pAuthInfo != NULL)
	{
		destEvent->pAuthInfo = &_authInfo;
		if (srcEvent->pAuthInfo->pLoginMsg != NULL)
		{
			destEvent->pAuthInfo->pLoginMsg = &_loginMsg;
			_loginMsgBuffer.data = _loginMsgBufferMemory;
			_loginMsgBuffer.length = sizeof(_loginMsgBufferMemory);
			rsslCopyRDMLoginMsg(destEvent->pAuthInfo->pLoginMsg, srcEvent->pAuthInfo->pLoginMsg, &_loginMsgBuffer);
		}
		else
		{
			destEvent->pAuthInfo->pLoginMsg = NULL;
		}
	}
	else
	{
		destEvent->pAuthInfo = NULL;
	}

	if (srcEvent->pState != NULL)
	{
		destEvent->pState = &_state;
		destEvent->pState->code = srcEvent->pState->code;
		destEvent->pState->dataState = srcEvent->pState->dataState;
		destEvent->pState->streamState = srcEvent->pState->streamState;
		destEvent->pState->text.data = srcEvent->pState->text.data;
		destEvent->pState->text.length = srcEvent->pState->text.length;
	}
	else
	{
		destEvent->pState = NULL;
	}
}

void copyTunnelStreamMsgEvent(RsslTunnelStreamMsgEvent* srcEvent, RsslTunnelStreamMsgEvent* destEvent)
{
	if (srcEvent->pRsslMsg != NULL)
	{
		_msgBuffer.data =  _msgBufferMemory;
		_msgBuffer.length = sizeof(_msgBufferMemory);
		destEvent->pRsslMsg = rsslCopyMsg(srcEvent->pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, &_msgBuffer);
	}
	else
	{
		destEvent->pRsslMsg =  NULL;
	}

	if (srcEvent->pRsslBuffer != NULL)
	{
		ASSERT_TRUE(srcEvent->pRsslBuffer->length <= sizeof(_tsMsgBufferMemory));
		memcpy(_tsMsgBufferMemory, srcEvent->pRsslBuffer->data, srcEvent->pRsslBuffer->length);
		_tsMsgBuffer.data = _tsMsgBufferMemory;
		_tsMsgBuffer.length = srcEvent->pRsslBuffer->length;
		destEvent->pRsslBuffer = &_tsMsgBuffer;
	}
	else
	{
		destEvent->pRsslBuffer =  NULL;
	}

	destEvent->containerType = srcEvent->containerType;
}

void openSession(Consumer* pConsumer, CoreComponent* pProvider, ConsumerProviderSessionOptions* pOpts, bool recoveringChannel)
{
	TestReactorEvent* pEvent;
	RsslReactorChannelEvent* pChannelEvent;
	RsslRDMLoginMsgEvent* pLoginMsgEvent;
	RsslRDMDirectoryMsgEvent* pDirectoryMsgEvent;
	ReadEvent* pReadEvent;
	RsslRDMLoginMsg* pLoginMsg;
	RsslRDMDirectoryMsg* pDirectoryMsg;
	RsslRDMLoginRequest loginRequest;
	RsslRDMDirectoryRequest dirRequest;

	/* Initialize the default login request(Use 1 as the Login Stream ID). */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslInitDefaultRDMLoginRequest(&loginRequest, 1));
	
	/* Initialize the default directory request(Use 2 as the Directory Stream Id) */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslInitDefaultRDMDirectoryRequest(&dirRequest, 2));

	/* Create Consumer. */
	RsslReactorOMMConsumerRole consumerRole;
	rsslClearOMMConsumerRole(&consumerRole);
	consumerRole.pLoginRequest = &loginRequest;
	consumerRole.pDirectoryRequest = &dirRequest;
	consumerRole.base.channelEventCallback = pConsumer->channelEventCallback;
	consumerRole.loginMsgCallback = pConsumer->loginMsgCallback;
	consumerRole.directoryMsgCallback = pConsumer->directoryMsgCallback;
	consumerRole.dictionaryMsgCallback = pConsumer->dictionaryMsgCallback;
	consumerRole.base.defaultMsgCallback = pConsumer->defaultMsgCallback;

	pOpts->setupDefaultLoginStream(true);
	pOpts->setupDefaultDirectoryStream(true);
	 
	if (!recoveringChannel)
	{
		/* Connect the Consumer and Provider-> */
		pProvider->bind(pOpts);
		pConsumer->testReactor()->connect(pOpts, pConsumer, pProvider->serverPort());
	}
	else
	{
		ASSERT_TRUE(pProvider->server() != NULL);
	}
		
	pProvider->acceptAndInitChannel(pOpts->connectionType());

	/* Consumer receives channel-up */
	pConsumer->testReactor()->dispatch(1);
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
	pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_RC_CET_CHANNEL_UP, pChannelEvent->channelEventType);
	delete pEvent;
		
	/* Provider receives login request. */
	pProvider->dispatch(1);
	pReadEvent = pProvider->pollEvent();
	pLoginMsg = pReadEvent->loginMsg();
	ASSERT_EQ(pLoginMsg->rdmMsgBase.rdmMsgType, RDM_LG_MT_REQUEST);
		
	/* Provider sends login refresh. */
	loginRequest = pLoginMsg->request;
	RsslRDMLoginRequest* pLoginRequest = &pLoginMsg->request;
	RsslRDMLoginRefresh loginRefresh;

	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.rdmMsgType = RDM_LG_MT_REFRESH;
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = pLoginRequest->userName;
	loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;
	loginRefresh.state.streamState = RSSL_STREAM_OPEN;
	loginRefresh.state.dataState = RSSL_DATA_OK;
	loginRefresh.state.code = RSSL_SC_NONE;
	RsslBuffer loginText;
	char loginTextData[] = "Login OK";
	loginText.data = loginTextData;
	loginText.length = (RsslUInt32)strlen(loginTextData);
	loginRefresh.state.text = loginText;

	pProvider->submit(&loginRefresh.rdmMsgBase);
	delete pReadEvent;
		
	/* Consumer receives loginRefresh. */
	pConsumer->testReactor()->dispatch(1);
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(LOGIN_MSG, pEvent->type());
	pLoginMsgEvent = (RsslRDMLoginMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RDM_LG_MT_REFRESH, pLoginMsgEvent->pRDMLoginMsg->rdmMsgBase.rdmMsgType);
	delete pEvent;
		
	if (consumerRole.pDirectoryRequest == NULL)
	{
		/* Consumer receives channel-ready. */
		pEvent = pConsumer->testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
		delete pEvent;
			
		pProvider->dispatch(0);

		/* If watchlist is not enabled, no directory exchange occurs. We're done. */
		if (consumerRole.watchlistOptions.enableWatchlist == false)
			return;
	}
		
	if (pOpts->setupDefaultDirectoryStream() == false)
		return;

	/* Provider receives directory request. */
	pProvider->dispatch(1);
	pReadEvent = pProvider->pollEvent();
	pDirectoryMsg = pReadEvent->directoryMsg();
	ASSERT_EQ(RDM_DR_MT_REQUEST, pDirectoryMsg->rdmMsgBase.rdmMsgType);
		
	/* Provider sends a default directory refresh. */
	RsslRDMDirectoryRequest* pDirectoryRequest = &pDirectoryMsg->request;
	RsslRDMDirectoryRefresh directoryRefresh;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;
	directoryRefresh.filter = pDirectoryRequest->filter;
	directoryRefresh.flags |= RDM_DR_RFF_SOLICITED;
	directoryRefresh.flags |= RDM_DR_RFF_CLEAR_CACHE;
	directoryRefresh.state.streamState = RSSL_STREAM_OPEN;
	directoryRefresh.state.dataState = RSSL_DATA_OK;
	directoryRefresh.state.code = RSSL_SC_NONE;
	RsslBuffer directoryText;
	char directoryTextData[] = "Source Directory Refresh Complete";
	directoryText.data = directoryTextData;
	directoryText.length = (RsslUInt32)strlen(directoryTextData);
	directoryRefresh.state.text = directoryText;

	directoryRefresh.serviceList = defaultService();
	directoryRefresh.serviceCount = 1;
	pProvider->submit(&directoryRefresh.rdmMsgBase);
	delete pReadEvent;

	pConsumer->testReactor()->dispatch(2);
		
	/* Consumer receives directory refresh. */
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(DIRECTORY_MSG, pEvent->type());
	pDirectoryMsgEvent = (RsslRDMDirectoryMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ((recoveringChannel && consumerRole.watchlistOptions.enableWatchlist ? RDM_DR_MT_UPDATE : RDM_DR_MT_REFRESH), pDirectoryMsgEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType);
	delete pEvent;
		
	/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
	pConsumer->defaultSessionDirectoryStreamId(consumerRole.pDirectoryRequest->rdmMsgBase.streamId);
	pProvider->defaultSessionDirectoryStreamId(pDirectoryRequest->rdmMsgBase.streamId);
		
	/* Consumer receives channel-ready. */
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
	pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
	delete pEvent;
}
