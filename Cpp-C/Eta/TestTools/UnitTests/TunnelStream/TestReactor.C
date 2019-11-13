/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestReactor.h"
#include "TestUtil.h"
#include "Consumer.h"
#include "Provider.h"
#include "TunnelStreamGetTime.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace std;

TestReactor::TestReactor()
{
	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	RsslCreateReactorOptions reactorOptions;
		
	rsslClearCreateReactorOptions(&reactorOptions);
	_pReactor = rsslCreateReactor(&reactorOptions, &_errorInfo);
	FD_SET(_pReactor->eventFd, &readFds);
}

RsslReactor* TestReactor::reactor()
{
	return _pReactor;
}

void TestReactor::enableReactorXmlTracing()
{
	_enableReactorXmlTracing = true;
}

void TestReactor::dispatch(RsslInt expectedEventCount)
{
	RsslUInt timeoutMsec;
		
	if (expectedEventCount > 0)
		timeoutMsec = 5000;
	else
		timeoutMsec = 1000;
	dispatch(expectedEventCount, timeoutMsec);
}
	
	
void TestReactor::dispatch(RsslInt expectedEventCount, RsslUInt timeoutMsec)
{
	RsslInt selectRet = 0;
	RsslUInt currentTimeUsec, stopTimeUsec;
	RsslInt lastDispatchRet = 0;
	bool newStopTimeSet = false;

	/* Ensure no events were missed from previous calls to dispatch. */
	ASSERT_EQ(0, _eventQueue.size());

	currentTimeUsec = getTimeNano()/1000;
		
	stopTimeUsec =  (RsslUInt)(timeoutMsec);
	stopTimeUsec *= 1000;
	stopTimeUsec += currentTimeUsec;

	do
	{
		if (lastDispatchRet == 0)
		{
			struct timeval selectTime;
			fd_set useReadFds;
			time_t timeoutUsec;

			FD_ZERO(&useReadFds);

			for (list<TestReactorComponent *>::iterator it=_componentList.begin(); it != _componentList.end(); ++it)
			{
				TestReactorComponent* pComponent = *it;
				FD_SET(pComponent->testReactor()->reactor()->eventFd, &useReadFds);
				if (pComponent->reactorChannel() != NULL && pComponent->reactorChannelIsUp())
				{
					FD_SET(pComponent->reactorChannel()->socketId, &useReadFds);
				}
			}
	
			timeoutUsec = (currentTimeUsec < stopTimeUsec) ? 
				(stopTimeUsec - currentTimeUsec) : (time_t)0;

			selectTime.tv_sec = (long)timeoutUsec/1000000;
			selectTime.tv_usec = (long)(timeoutUsec - selectTime.tv_sec * 1000000);
			selectRet = select(FD_SETSIZE, &useReadFds, NULL, NULL, &selectTime);
		}
		else
			selectRet = 1;

		if (selectRet > 0)
		{
			RsslErrorInfo rsslErrorInfo;
			RsslReactorDispatchOptions dispatchOpts;

			do
			{
				rsslClearReactorDispatchOptions(&dispatchOpts);
				lastDispatchRet = rsslReactorDispatch(_pReactor, &dispatchOpts, &rsslErrorInfo);
				ASSERT_TRUE(lastDispatchRet >= 0) << "Dispatch failed: " << lastDispatchRet << "(" << _errorInfo.errorLocation << " -- " << _errorInfo.rsslError.text << ")";
			} while (lastDispatchRet > 0);
		}

		currentTimeUsec = getTimeNano()/1000;
			
		/* If we've hit our expected number of events, drop the stopping time to at most 100ms from now.  
			* Keep dispatching a short time to ensure no unexpected events are received, and that any internal flush events are processed. */
		if (expectedEventCount > 0 && _eventQueue.size() == expectedEventCount && newStopTimeSet == false)
		{
			RsslUInt stopTimeUsecNew = currentTimeUsec + 100000;
			if ((stopTimeUsec - stopTimeUsecNew) > 0)
			{
				stopTimeUsec = stopTimeUsecNew;
				newStopTimeSet = true;
			}
		}

	} while(currentTimeUsec < stopTimeUsec);
		
	ASSERT_EQ(expectedEventCount, _eventQueue.size());
}
	
RsslInt TestReactor::handleChannelEvent(RsslReactorChannelEvent* pEvent)
{
	TestReactorComponent* pComponent = (TestReactorComponent *)pEvent->pReactorChannel->userSpecPtr;
	bool isUp = false;
				
	switch(pEvent->channelEventType)
	{
	case RSSL_RC_CET_CHANNEL_OPENED:
		isUp = pComponent->reactorChannelIsUp();
		EXPECT_EQ(isUp, false);
		break;
	case RSSL_RC_CET_CHANNEL_UP:
		pComponent->reactorChannelIsUp(true);
		break;
	case RSSL_RC_CET_CHANNEL_DOWN:
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		if (pComponent->reactorChannelIsUp())
		{
			/* If channel was up, the socketId should be present. */
			EXPECT_TRUE(pComponent->reactorChannel()->socketId) << "ReactorChannel up with no socketId";

			/* Remove socketId from fdset. */
			FD_CLR(pComponent->reactorChannel()->socketId, &readFds);
		}

		pComponent->reactorChannelIsUp(RSSL_FALSE);
		break;
	case RSSL_RC_CET_CHANNEL_READY:
	case RSSL_RC_CET_FD_CHANGE:
	case RSSL_RC_CET_WARNING:
		EXPECT_TRUE(pComponent->reactorChannel() != NULL);
		break;
	default:
		ADD_FAILURE() << "Unhandled ReactorChannelEventType: " << pEvent->channelEventType;
	}
		
	if (pComponent->reactorChannel() != NULL)
	{
		EXPECT_TRUE(pComponent->reactorChannel() == pEvent->pReactorChannel);
	}
	else
	{
		pComponent->reactorChannel(pEvent->pReactorChannel);
	}

	_eventQueue.push_back(new TestReactorEvent(CHANNEL_EVENT, pEvent));
	return RSSL_RET_SUCCESS;
}
	
RsslInt TestReactor::handleLoginMsgEvent(RsslRDMLoginMsgEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(LOGIN_MSG, pEvent));
	return RSSL_RET_SUCCESS;
}
	
RsslInt TestReactor::handleDirectoryMsgEvent(RsslRDMDirectoryMsgEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(DIRECTORY_MSG, pEvent));
	return RSSL_RET_SUCCESS;
}

RsslInt TestReactor::handleDictionaryMsgEvent(RsslRDMDictionaryMsgEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(DICTIONARY_MSG, pEvent));	
	return RSSL_RET_SUCCESS;
}
	
RsslInt TestReactor::handleDefaultMsgEvent(RsslMsgEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(DEFAULT_MSG, pEvent));
	return RSSL_RET_SUCCESS;
}
	
RsslInt TestReactor::handleTunnelStreamMsgEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(TUNNEL_STREAM_MSG, pTunnelStream, pEvent));
	return RSSL_RET_SUCCESS;
}
	
RsslInt TestReactor::handleTunnelStreamStatusEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(TUNNEL_STREAM_STATUS, pTunnelStream, pEvent));
	return RSSL_RET_SUCCESS;
}
 
RsslInt TestReactor::handleTunnelStreamRequestEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamRequestEvent* pEvent)
{
	_eventQueue.push_back(new TestReactorEvent(pTunnelStream, pEvent));
	return RSSL_RET_SUCCESS;
}
	
TestReactorEvent* TestReactor::pollEvent()
{
	TestReactorEvent* pEvent = NULL;
	
	if (!_eventQueue.empty())
	{
		pEvent = _eventQueue.front();
		_eventQueue.pop_front();
	}

	return pEvent;
}
	
void TestReactor::addComponent(TestReactorComponent* pComponent)
{
	bool found = (find(_componentList.begin(), _componentList.end(), pComponent) != _componentList.end());
	ASSERT_EQ(found, false);
	_componentList.push_back(pComponent);
}
	
void TestReactor::registerComponentServer(TestReactorComponent* pComponent)
{
	ASSERT_TRUE(pComponent->server() != NULL);
	FD_SET(pComponent->server()->socketId, &readFds);
}

void TestReactor::removeComponent(TestReactorComponent* pComponent)
{
	bool found = (find(_componentList.begin(), _componentList.end(), pComponent) != _componentList.end());
	ASSERT_EQ(found, true);
	_componentList.remove(pComponent);
}
	
void TestReactor::connect(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent, RsslInt port)
{
	RsslReactorConnectOptions connectOpts;
	RsslReactorConnectInfo connectInfo;
	RsslRet ret;

	rsslClearReactorConnectOptions(&connectOpts);
	rsslClearReactorConnectInfo(&connectInfo);

	connectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	connectInfo.rsslConnectOptions.connectionType = (RsslConnectionTypes)pOpts->connectionType();
	connectInfo.rsslConnectOptions.userSpecPtr = pComponent;
	connectOpts.rsslConnectOptions.userSpecPtr = pComponent;
	connectOpts.reconnectAttemptLimit = pOpts->reconnectAttemptLimit();
	connectOpts.reconnectMinDelay = pOpts->reconnectMinDelay();
	connectOpts.reconnectMaxDelay = pOpts->reconnectMaxDelay();
	connectInfo.rsslConnectOptions.pingTimeout = pOpts->pingTimeout();
	connectOpts.initializationTimeout = pOpts->consumerChannelInitTimeout();

	if (pOpts->connectionType() != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		char address[] = "localhost";
		char portStr[32];
		snprintf(portStr, 32, "%d", (int)port);
		connectInfo.rsslConnectOptions.connectionInfo.unified.address = address;
		connectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = portStr;
	}

	connectOpts.connectionCount = 1;
	connectOpts.reactorConnectionList = &connectInfo;

	ret = rsslReactorConnect(_pReactor, &connectOpts, pComponent->reactorRole(), &_errorInfo);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS) << "Connect failed: " << ret << " (" << _errorInfo.errorLocation << " -- " << _errorInfo.rsslError.text << ")";
}
	
void TestReactor::accept(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent)
{
	accept(pOpts, pComponent, 5000);
}
	
void TestReactor::accept(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent, RsslUInt timeoutMsec)
{
	RsslReactorAcceptOptions acceptOpts;
	RsslInt selRet;
	struct timeval selectTime;
	RsslUInt currentTimeUsec, stopTimeUsec, timeToSelect;

	if (pOpts->connectionType() != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		ASSERT_TRUE(pComponent->server() != NULL);
		ASSERT_EQ(NULL, pComponent->reactorChannel());
			
		/* Wait for server channel to trigger. */
		currentTimeUsec = getTimeNano()/1000;
		
		stopTimeUsec =  (RsslUInt)(timeoutMsec);
		stopTimeUsec *= 1000;
		stopTimeUsec += currentTimeUsec;
					
		do
		{
			timeToSelect = stopTimeUsec - currentTimeUsec;
					
			selectTime.tv_sec = (long)timeToSelect/1000000;
			selectTime.tv_usec = (long)(timeToSelect - selectTime.tv_sec * 1000000);
			selRet = select(FD_SETSIZE, &readFds, NULL, &exceptFds, &selectTime);
			ASSERT_TRUE(selRet >= 0);

			if (FD_ISSET(_pReactor->eventFd, &readFds))
			{
				/* Reactor's channel triggered. Should get no events. */
				dispatch(0, 100);
			}
			if (FD_ISSET(pComponent->server()->socketId, &readFds))
			{
				rsslClearReactorAcceptOptions(&acceptOpts);
				acceptOpts.rsslAcceptOptions.userSpecPtr = pComponent;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorAccept(_pReactor, pComponent->server(), &acceptOpts, pComponent->reactorRole(), &_errorInfo));
				return;
			}

		   currentTimeUsec = getTimeNano()/1000;
		} while (currentTimeUsec < stopTimeUsec);
			
		FAIL() << "Server did not receive accept notification.";
	}
}
	
void TestReactor::openSession(Consumer* pConsumer, Provider* pProvider, ConsumerProviderSessionOptions* pOpts, bool recoveringChannel)
{
	TestReactorEvent* pEvent;
	RsslReactorChannelEvent* pChannelEvent;
	RsslRDMLoginMsgEvent* pLoginMsgEvent;
	RsslRDMDirectoryMsgEvent* pDirectoryMsgEvent;
	RsslReactorOMMConsumerRole* pConsumerRole = (RsslReactorOMMConsumerRole *)pConsumer->reactorRole();
	RsslReactorSubmitMsgOptions submitOptions;

	if (!recoveringChannel)
		pConsumer->testReactor()->connect(pOpts, pConsumer, pProvider->serverPort());

	/* Preset login message required if automatically setting up login stream. */
	ASSERT_TRUE(pOpts->setupDefaultLoginStream() == RSSL_FALSE || pConsumerRole->pLoginRequest != NULL);

	/* Preset directory message required, or watchlist must be enabled, if automatically setting up directory stream. */
	ASSERT_TRUE(pOpts->setupDefaultDirectoryStream() == RSSL_FALSE 
		|| pConsumerRole->watchlistOptions.enableWatchlist == RSSL_TRUE
			|| pConsumerRole->pDirectoryRequest != NULL);
		
	/* If watchlist enabled, should get ChannelOpenCallback */
	if (pConsumerRole->watchlistOptions.enableWatchlist && pConsumerRole->watchlistOptions.channelOpenCallback != NULL
			&& recoveringChannel == RSSL_FALSE)
	{
		pEvent = pConsumer->testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_OPENED, pChannelEvent->channelEventType);
		delete pEvent;
	}
	else
	{
		pConsumer->testReactor()->dispatch(0);
	}
		
	pProvider->testReactor()->accept(pOpts, pProvider);
		
	/* Provider receives channel-up/channel-ready */
	pProvider->testReactor()->dispatch(2);
		
	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
	pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_RC_CET_CHANNEL_UP, pChannelEvent->channelEventType);
	delete pEvent;

	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
	pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
	delete pEvent;
		
	/* Consumer receives channel-up and any status events due the watchlist items submitted in channel open callback. */
	if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
	{
		pConsumer->testReactor()->dispatch(1 + pOpts->numStatusEvents());
	}
	else
	{
		pConsumer->testReactor()->dispatch(2 + pOpts->numStatusEvents());
	}
	for (RsslInt i = 0; i < pOpts->numStatusEvents(); i++)
	{
		pEvent = pConsumer->testReactor()->pollEvent();
		ASSERT_EQ(DEFAULT_MSG, pEvent->type());
		RsslMsgEvent* pMsgEvent = (RsslMsgEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_MC_STATUS, pMsgEvent->pRsslMsg->msgBase.msgClass);
		delete pEvent;
	}
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
	pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RSSL_RC_CET_CHANNEL_UP, pChannelEvent->channelEventType);
	delete pEvent;
		
	if (pConsumerRole->pLoginRequest == NULL || pConsumerRole->watchlistOptions.enableWatchlist == RSSL_TRUE)
	{
		/* Consumer receives channel-ready, then we're done. */
		if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
		{
			pConsumer->testReactor()->dispatch(1);
		}
		pEvent = pConsumer->testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
		delete pEvent;
			
		if (pConsumerRole->pLoginRequest == NULL)
			return;
	}
		
	if (pOpts->setupDefaultLoginStream() == RSSL_FALSE)
		return;
		
	/* Provider receives login request. */
	pProvider->testReactor()->dispatch(1);
	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(LOGIN_MSG, pEvent->type());
	pLoginMsgEvent = (RsslRDMLoginMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RDM_LG_MT_REQUEST, pLoginMsgEvent->pRDMLoginMsg->rdmMsgBase.rdmMsgType);
		
	/* Provider sends a default login refresh. */
	RsslRDMLoginRequest* pLoginRequest = &pLoginMsgEvent ->pRDMLoginMsg->request;
	RsslRDMLoginRefresh loginRefresh;

	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.rdmMsgType = RDM_LG_MT_REFRESH;
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_USERNAME;
	loginRefresh.userName = pLoginRequest->userName;
	loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;
	loginRefresh.supportOMMPost = 1;
	loginRefresh.state.streamState = RSSL_STREAM_OPEN;
	loginRefresh.state.dataState = RSSL_DATA_OK;
	loginRefresh.state.code = RSSL_SC_NONE;
	RsslBuffer loginText;
	char loginTextData[] = "Login OK";
	loginText.data = loginTextData;
	loginText.length = (RsslUInt32)strlen(loginTextData);
	loginRefresh.state.text = loginText;
		
	rsslClearReactorSubmitMsgOptions(&submitOptions);
	submitOptions.pRDMMsg = (RsslRDMMsg *)&loginRefresh;
	ASSERT_TRUE(pProvider->submitAndDispatch(&submitOptions) >= RSSL_RET_SUCCESS);

	/* Consumer receives login refresh. */		
	if (pConsumerRole->pDirectoryRequest == NULL && pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
		pConsumer->testReactor()->dispatch(2);
	else
		pConsumer->testReactor()->dispatch(1);
		
	/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
	pConsumer->defaultSessionLoginStreamId(pConsumerRole->pLoginRequest->rdmMsgBase.streamId);
	pProvider->defaultSessionLoginStreamId(pLoginRequest->rdmMsgBase.streamId);
	delete pEvent;
		
	pEvent = pConsumer->testReactor()->pollEvent();
	ASSERT_EQ(LOGIN_MSG, pEvent->type());
	pLoginMsgEvent = (RsslRDMLoginMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RDM_LG_MT_REFRESH, pLoginMsgEvent->pRDMLoginMsg->rdmMsgBase.rdmMsgType);
	delete pEvent;
		
	if (pConsumerRole->pDirectoryRequest == NULL && pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
	{
		/* Consumer receives channel-ready. */
		pEvent = pConsumer->testReactor()->pollEvent();
		ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
		pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
		ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
		delete pEvent;
			
		pProvider->testReactor()->dispatch(0);

		/* If watchlist is not enabled, no directory exchange occurs. We're done. */
		if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
			return;
	}
		
	if (pOpts->setupDefaultDirectoryStream() == RSSL_FALSE && pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
		return;

	/* Provider receives directory request. */
	pProvider->testReactor()->dispatch(1);
	pEvent = pProvider->testReactor()->pollEvent();
	ASSERT_EQ(DIRECTORY_MSG, pEvent->type());
	pDirectoryMsgEvent = (RsslRDMDirectoryMsgEvent *)pEvent->reactorEvent();
	ASSERT_EQ(RDM_DR_MT_REQUEST, pDirectoryMsgEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType);

	/* Provider sends a default directory refresh. */
	RsslRDMDirectoryRequest* pDirectoryRequest = &pDirectoryMsgEvent->pRDMDirectoryMsg->request;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslInt32 requestStreamId = pDirectoryRequest->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = requestStreamId;
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
	rsslClearReactorSubmitMsgOptions(&submitOptions);
	submitOptions.pRDMMsg = (RsslRDMMsg *)&directoryRefresh;
	ASSERT_TRUE(pProvider->submitAndDispatch(&submitOptions) >= RSSL_RET_SUCCESS);
	delete pEvent;

	if (pOpts->setupDefaultDirectoryStream() == true)
	{
		if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
			pConsumer->testReactor()->dispatch(2);
		else
			pConsumer->testReactor()->dispatch(1);
	}
	else
		pConsumer->testReactor()->dispatch(1);
		
	/* Consumer receives directory refresh. */
	pEvent = pConsumer->testReactor()->pollEvent();
	if (pOpts->setupDefaultDirectoryStream() == true)
	{
		ASSERT_EQ(DIRECTORY_MSG, pEvent->type());
		pDirectoryMsgEvent = (RsslRDMDirectoryMsgEvent *)pEvent->reactorEvent();
		if (!recoveringChannel || pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
			ASSERT_EQ(RDM_DR_MT_REFRESH, pDirectoryMsgEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType);
		else
			ASSERT_EQ(RDM_DR_MT_UPDATE, pDirectoryMsgEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType);
			
		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
		pConsumer->defaultSessionDirectoryStreamId(pConsumerRole->pDirectoryRequest->rdmMsgBase.streamId);
		pProvider->defaultSessionDirectoryStreamId(requestStreamId);
		delete pEvent;

		if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
		{
			/* Consumer receives channel-ready. */
			pEvent = pConsumer->testReactor()->pollEvent();
			ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
			pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
			ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
			delete pEvent;
		}
	}
	else // only channel pEvent comes in this case
	{
		if (pConsumerRole->watchlistOptions.enableWatchlist == RSSL_FALSE)
		{
			/* Consumer receives channel-ready. */
			ASSERT_EQ(CHANNEL_EVENT, pEvent->type());
			pChannelEvent = (RsslReactorChannelEvent *)pEvent->reactorEvent();
			ASSERT_EQ(RSSL_RC_CET_CHANNEL_READY, pChannelEvent->channelEventType);
		}
		delete pEvent;
	}
}

void TestReactor::close()
{
	if (_pReactor != NULL)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDestroyReactor(_pReactor, &_errorInfo));
		_pReactor = NULL;
	}
}	
