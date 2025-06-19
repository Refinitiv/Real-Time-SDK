/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "AppUtil.h"
#include "ProviderPerfClient.h"
#include "ProvItemInfo.h"


using namespace std;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

using namespace perftool::common;

ProviderPerfClient::ProviderPerfClient(ProviderThread* provThread, IProvPerfConfig& provConfig) :
	providerThread(provThread), provPerfConfig(provConfig), lastActiveClientHandle(0)
{
}

ProviderPerfClient::~ProviderPerfClient()
{
}

void ProviderPerfClient::onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent)
{
	//cout << endl << "event channel info (onReqMsg)" << endl << ommEvent.getChannelInformation();

	switch (reqMsg.getDomainType()) {
	case MMT_LOGIN:
		processLoginRequest(reqMsg, ommEvent);
		break;
	case MMT_MARKET_PRICE:
		processMarketPriceRequest(reqMsg, ommEvent);
		break;
	default:
		processInvalidItemRequest(reqMsg, ommEvent);
		break;
	}
}

void ProviderPerfClient::processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent)
{
	using namespace refinitiv::ema::domain::login;

	Login::LoginReq loginRequest(reqMsg);

	Login::LoginRefresh loginRefresh = Login::LoginRefresh();

	loginRefresh.applicationId("256");

	loginRefresh.applicationName("EmaCppIProvPerf");

	loginRefresh.position(provPerfConfig.loginPosition);

	loginRefresh.name(loginRequest.getName());

	loginRefresh.nameType(USER_NAME);

	loginRefresh.singleOpen(false);  /* this provider does not support SingleOpen behavior */

	loginRefresh.supportOMMPost(true);

	loginRefresh.solicited(true);

	loginRefresh.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted");

	ommEvent.getProvider().submit(loginRefresh.getMessage(), ommEvent.getHandle());

	UInt64 clientHandle = ommEvent.getClientHandle();

	clientHandlesMutex.lock();
	if (clientHandles.count(clientHandle) == 0)
		clientHandles.insert( std::pair<UInt64, bool>(clientHandle, true) );
	lastActiveClientHandle = clientHandle;
	clientHandlesMutex.unlock();

	cout << endl << "event channel info (processLoginRequest)" << endl;
	cout << "ClientHandle: " << clientHandle << endl;
	cout << ommEvent.getChannelInformation() << endl << endl;
}

void ProviderPerfClient::processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent)
{
	providerThread->getProviderStats().itemRequestCount.countStatIncr();

	UInt8 itemFlags = ITEM_IS_SOLICITED;
	/* get IsStreamingRequest */
	if (reqMsg.getInterestAfterRefresh())  // RSSL_RQMF_STREAMING
	{
		itemFlags |= ITEM_IS_STREAMING_REQ;
	}

	/* check if the request is for a private stream */
	if (reqMsg.getPrivateStream())
	{
		itemFlags |= ITEM_IS_PRIVATE;
	}

	ProvItemInfo* itemInfo = new ProvItemInfo(ommEvent.getHandle(), ommEvent.getClientHandle(), reqMsg.getDomainType(), itemFlags, reqMsg.getName());

	// Adds the requested item to the list for processing in the working thread
	providerThread->addRefreshItem(itemInfo);
}

void ProviderPerfClient::processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent)
{
	ommEvent.getProvider().submit(StatusMsg().name(reqMsg.getName()).
		serviceName(reqMsg.getServiceName()).
		domainType(reqMsg.getDomainType()).
		state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found"),
		ommEvent.getHandle());

	cout << endl << "event channel info (processInvalidItemRequest)" << endl << reqMsg << endl;
}

void ProviderPerfClient::onPostMsg(const PostMsg& postMsg, const OmmProviderEvent& event)
{
	// if the post is on the login stream, then it's an off-stream post, else it's an on-stream post
	//cout << "Received an " << (event.getHandle() == loginStreamHandle ? "off-stream" : "on-stream") << endl;
	//cout << "PostMsg: " << postMsg << endl;

	providerThread->getProviderStats().postMsgCount.countStatIncr();

	UpdateMsg updateMsg = postMsg.getPayload().getUpdateMsg();

	updateMsg.domainType(postMsg.getDomainType());

	updateMsg.publisherId(postMsg.getPublisherIdUserId(), postMsg.getPublisherIdUserAddress());

	if (postMsg.hasSeqNum())
	{
		updateMsg.seqNum(postMsg.getSeqNum());
	}

	event.getProvider().submit(updateMsg, event.getHandle());
}

// called when a client disconnects or when an item is unregistered.
void ProviderPerfClient::onClose(const ReqMsg&, const OmmProviderEvent& ommEvent)
{
	cout << endl << "event channel info (onClose)" << endl;
	cout << "ClientHandle: " << ommEvent.getClientHandle() << endl << endl;

	UInt64 clientHandle = ommEvent.getClientHandle();

	clientHandlesMutex.lock();
	std::map<UInt64, bool>::iterator it = clientHandles.find(clientHandle);
	if (it != clientHandles.end())
	{
		clientHandles.erase(it);
	}
	if (lastActiveClientHandle == clientHandle)
	{
		lastActiveClientHandle = 0U;
	}
	clientHandlesMutex.unlock();

	// notifies the working thread that the client disconnected
	providerThread->addClosedClientHandle(clientHandle);

	providerThread->getProviderStats().closeMsgCount.countStatIncr();

	return;
}

void ProviderPerfClient::onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	//cout << "GenericMsg: " << GenericMsg << endl;
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();
	TimeTrack timeTracker;
	EmaString errText;
	PerfTimeValue decodeTimeStart, decodeTimeEnd;
	ProviderStats& stats = providerThread->getProviderStats();

	stats.genMsgRecvCount.countStatIncr();

	if (!stats.firstGenMsgRecvTime)
	{
		stats.firstGenMsgRecvTime = perftool::common::GetTime::getTimeNano();
	}

	if (provPerfConfig.measureDecode)
	{
		decodeTimeStart = perftool::common::GetTime::getTimeNano();
	}

	if (!msgDataUtil->decodeUpdate(genericMsg.getPayload().getFieldList(), DataType::GenericMsgEnum, timeTracker, errText))
	{
		AppUtil::logError(errText);
		exit(-1);
	}

	// Store latency time from GenericMsg
	if (timeTracker.genTime)
	{
		PerfTimeValue latencyTime = perftool::common::GetTime::getTimeMicro();
		stats.genMsgLatencyRecords.updateLatencyStats(timeTracker.genTime, latencyTime, 1);
	}

	// Store decoding time from GenericMsg
	if (provPerfConfig.measureDecode)
	{
		decodeTimeEnd = perftool::common::GetTime::getTimeNano();
		stats.messageDecodeTimeRecords.updateLatencyStats(decodeTimeStart, decodeTimeEnd, 1000);
	}
}

void ProviderPerfClient::onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event)
{
	providerThread->getProviderStats().statusCount.countStatIncr();

	bool bConnectionUp = false;

	ChannelInformation channelInfo = event.getChannelInformation();
	if (channelInfo.getChannelState() == ChannelInformation::ActiveEnum)
	{
		bConnectionUp = true;
	}

	clientHandlesMutex.lock();
	std::map<UInt64, bool>::iterator it = clientHandles.find(event.getClientHandle());
	if (it != clientHandles.end())
	{
		it->second = bConnectionUp;
	}
	if (lastActiveClientHandle == event.getClientHandle() && !bConnectionUp)
	{
		lastActiveClientHandle = 0U;
	}
	clientHandlesMutex.unlock();

	cout << endl << "status for event clientHandle " << event.getHandle() << ", client clientHandle "
		<< event.getClientHandle() << endl
		<< "event channel info (onStatus)" << endl
		<< channelInfo << endl;
}

bool ProviderPerfClient::isActiveStream(UInt64 clientHandle)
{
	bool isActive = false;
	if (lastActiveClientHandle == clientHandle  ||  !clientHandles.empty())
	{
		clientHandlesMutex.lock();
		if (lastActiveClientHandle == clientHandle)
		{
			isActive = true;
		}
		else
		{
			std::map<UInt64, bool>::iterator it = clientHandles.find(clientHandle);
			if (it != clientHandles.end())
			{
				isActive = it->second;
				if (isActive)
					lastActiveClientHandle = clientHandle;
			}
		}
		clientHandlesMutex.unlock();
	}
	return isActive;
}
