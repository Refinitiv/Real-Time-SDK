/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */
 
#include "ConsumerRoutingSession.h"
#include "ConsumerRoutingChannel.h"
#include "OmmBaseImpl.h"

using namespace refinitiv::ema::access;

const EmaString channelUp("session channel up");
const EmaString channelReconnecting("session channel down reconnecting");
const EmaString channelDown("session channel down");
const EmaString phStartingFallback("Preferred host starting fallback");
const EmaString phFallbackComplete("Preferred host complete");

size_t ConsumerRoutingSession::UInt16rHasher::operator()(const UInt16& value) const
{
	return value;
}

bool ConsumerRoutingSession::UInt16Equal_To::operator()(const UInt16& x, const UInt16& y) const
{
	return x == y ? true : false;
}

size_t ConsumerRoutingSession::EmaStringPtrHasher::operator()(const EmaStringPtr& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)*s++;
	return result;
}

bool ConsumerRoutingSession::EmaStringPtrEqual_To::operator()(const EmaStringPtr& x, const EmaStringPtr& y) const
{
	return *x == *y;
}


ConsumerRoutingSession::ConsumerRoutingSession(OmmBaseImpl& consumerBaseImpl) :
	baseImpl(consumerBaseImpl),
	activeConfig(consumerBaseImpl.getActiveConfig()),
	routingChannelList(),
	pendingRequestList(consumerBaseImpl),
	aggregatedLoginInfo(),
	sentInitialLoginRefresh(false), 
	serviceList(),
	serviceById(),
	serviceByName(),
	deletedServiceList(),
	addedServiceList(),
	updatedServiceList(),
	_statusMsg()
{
	enhancedItemRecovery = false;
	serviceIdCounter = 1;
	initialLoginRefreshReceived = false;
	activeChannelCount = 0;
}

ConsumerRoutingSession::~ConsumerRoutingSession()
{
	clear();
}

//Closes a channel and removes it from the list.
void ConsumerRoutingSession::closeChannel(RsslReactorChannel* pRsslReactorChannel)
{
	if (baseImpl._pRsslReactor != NULL)
	{
		for (UInt32 i = 0; i < routingChannelList.size(); i++)
		{
			if (routingChannelList[i]->pReactorChannel == pRsslReactorChannel)
			{
				routingChannelList[i]->closeReactorChannel();
				activeChannelCount--;
				return;
			}
		}
	}
}

void ConsumerRoutingSession::closeReactorChannels()
{
	if (baseImpl._pRsslReactor != NULL)
	{
		for (UInt32 i = 0; i < routingChannelList.size(); ++i)
		{
			if (routingChannelList[i] != NULL && routingChannelList[i]->pReactorChannel != NULL)
			{
				routingChannelList[i]->closeReactorChannel();
			}
		}

	}
}

void ConsumerRoutingSession::clear()
{
	closeReactorChannels();

	for (UInt32 i = 0; i < routingChannelList.size(); ++i)
	{
		if (routingChannelList[i] != NULL)
		{
			delete routingChannelList[i];
			routingChannelList[i] = NULL;
		}
	}

	routingChannelList.clear();

	while (serviceList.size() != 0)
	{
		Directory* svc = serviceList.pop_front();
		if (svc == NULL)
			break;
		Directory::destroy(svc);
	}

	serviceList.clear();
	serviceById.clear();
	serviceByName.clear();

	deletedServiceList.clear();
	addedServiceList.clear();
	updatedServiceList.clear();
}


bool ConsumerRoutingSession::aggregateLoginRefreshInfo(RsslRDMLoginRefresh* pRefreshMsg)
{
	// Make a shallow copy of the old login refresh so we can crossreference the values below
	// Note that we really don't care about the strings here, so they can be cleared.
	RsslRDMLoginRefresh oldLoginRefresh = *(aggregatedLoginInfo.loginRefreshMsg.getRefreshMsg());

	// Clear the aggregated login info, and iterate through the channel list and re-aggregate everything
	aggregatedLoginInfo.loginRefreshMsg.clear();
	ConsumerRoutingSessionChannel* pRoutingChannel;
	bool changed = false;


	for (UInt32 i = 0; i < routingChannelList.size(); ++i)
	{
		pRoutingChannel = routingChannelList[i];

		// Do not aggregate if the channel is down, has not received the initial OPEN/OK, or is reconnecting.
		if (pRoutingChannel->channelClosed == false && pRoutingChannel->channelState <= OmmBaseImpl::LoginStreamOpenOkEnum && pRoutingChannel->reconnecting == false)
		{
			changed = aggregatedLoginInfo.loginRefreshMsg.aggregateForRequestRouting(pRoutingChannel->loginInfo.loginRefreshMsg.getRefreshMsg(), this);
		}
	}

	if (pRefreshMsg != NULL)
	{
		changed = aggregatedLoginInfo.loginRefreshMsg.aggregateForRequestRouting(pRefreshMsg, this);
	}

	if (initialLoginRefreshReceived == false)
	{
		initialLoginRefreshReceived = true;
		return true;
	}

	// Check for changes to the old refresh flags, if they have changed here, return true.
	if (changed)
	{
		RsslRDMLoginRefresh* pNewRefresh = aggregatedLoginInfo.loginRefreshMsg.getRefreshMsg();

		if (oldLoginRefresh.providePermissionExpressions != pNewRefresh->providePermissionExpressions)
			return true;

		if (oldLoginRefresh.supportBatchRequests != pNewRefresh->supportBatchRequests)
			return true;

		if (oldLoginRefresh.supportOMMPost != pNewRefresh->supportOMMPost)
			return true;

		if (oldLoginRefresh.supportOptimizedPauseResume != pNewRefresh->supportOptimizedPauseResume)
			return true;

		if (oldLoginRefresh.supportEnhancedSymbolList != pNewRefresh->supportEnhancedSymbolList)
			return true;

		if (oldLoginRefresh.supportViewRequests != pNewRefresh->supportViewRequests)
			return true;

		if ((oldLoginRefresh.flags & RDM_LG_RFF_RTT_SUPPORT) != (pNewRefresh->flags & RDM_LG_RFF_RTT_SUPPORT))
			return true;
	}

	return false;
	
}

bool ConsumerRoutingSession::aggregateDirectory(Directory* newDirectory, RsslMapEntryActions action)
{
	ConsumerRoutingService** service = serviceByName.find(&newDirectory->getName());

	// If the service doesn't exist, create a new ConsumerRoutingService and add it to the lists.
	if (service == NULL)
	{
		if (action == RSSL_MPEA_DELETE_ENTRY)
		{
			// We received a DELETE action for a directory we don't recognize, so just ignore it.
			return false;
		}

		ConsumerRoutingService* newService = new ConsumerRoutingService(baseImpl);
		
		// Set the serviceId to the aggregated value.
		newService->setId(serviceIdCounter);
		serviceIdCounter++;

		newService->aggregateDirectoryInfo(*newDirectory, action);


		serviceById.insert((UInt16)newService->getId(), newService);
		serviceByName.insert(&newService->getName(), newService);
		serviceList.push_back(newService);

		addedServiceList.push_back(EmaString(newService->getName()));
		return true;
	}

	ConsumerRoutingService* pService = *service;

	// If the service has already been deleted and we're bringing a new one online, clear it so we can just aggregate later.
	if (pService->isDeleted() == true)
	{
		if (action == RSSL_MPEA_DELETE_ENTRY)
		{
			// We received a DELETE action for a directory we've already deleted from our cache, so just ignore it.
			return false;
		}

		UInt64 oldSetId = pService->getId();
		pService->clear();

		pService->setId(oldSetId);
	}

	// The aggregation will mark the service as either active or deleted.
	ConsumerRoutingService::AggregationResultEnum out = pService->aggregateDirectoryInfo(*newDirectory, action);

	switch (out)
	{
		case ConsumerRoutingService::AggregationResultEnum::SuccessNewServiceEnum:
			addedServiceList.push_back(EmaString(pService->getName()));
			return true;
			break;
		case ConsumerRoutingService::AggregationResultEnum::SuccessWithUpdateEnum:
			// Only add the updated services to the updated service list if we're done with initialization
			// Otherwise, the service should already be added to the new service list.
			if (baseImpl.isInitialized() == true)
			{
				updatedServiceList.push_back(EmaString(pService->getName()));
			}

			return true;
			break;
		case ConsumerRoutingService::AggregationResultEnum::SuccessAndDeleteEnum:
			// Only add the deleted services to the deleted service list if we're done with initialization
			// Otherwise, iterate through the added service list and remove it if the name is present
			if (baseImpl.isInitialized() == true)
				deletedServiceList.push_back(EmaString(pService->getName()));
			else
			{
				for (UInt32 i = 0; i < addedServiceList.size(); i++)
				{
					if (addedServiceList[i] == newDirectory->getName())
					{
						addedServiceList.removePosition(i);
						break;
					}
				}

				// remove and delete the service from the session's hash tables and list, and delete the consumer session service.
				serviceById.erase((UInt16)pService->getId());
				serviceByName.erase(&pService->getName());
				serviceList.remove(pService);

				delete pService;
			}
			return true;
			break;
		case  ConsumerRoutingService::AggregationResultEnum::SuccessNoUpdateEnum:
			// If this is initialized, return false, because an update does not need to be sent out.
			if (baseImpl.isInitialized() == true)
			{
				return false;
			}
			else
			{
				// If the ommbaseimpl is not initialized, send true to avoid closing the channel in OmmConsumerImpl::loadDirectory()
				return true;
			}
			break;
		default:
			return false;
	}


}

// This will return true if it is able to match the request to a channel
// This will return false if it is unable to match a channel.
bool ConsumerRoutingSession::matchRequestToSessionChannel(SingleItem& item)
{
	ConsumerRoutingSessionChannel* pOldChannel = item.sessionChannel;

	if (item.getServiceListName().length() != 0)
	{
		Directory* pDirectory = NULL;
		ServiceList** pServiceListPtr = baseImpl.getActiveConfig().serviceListByName.find(&(item.getServiceListName()));
		bool foundDirectory = false;   // This will be set to true if any directory is found, so the request can go to the pending lists.
		UInt32 serviceListIndex = item.currentServiceListIndex;

		if (pServiceListPtr == NULL)
		{
			return false;
		}

		EmaVector<EmaString>& concreteServiceList = (*pServiceListPtr)->concreteServiceList();

		ConsumerRoutingSessionChannel* pOldChannel = item.sessionChannel;

		item.sessionChannel = NULL;
		bool matchedService = false;

		if (item.sessionChannelItemClosedList == NULL)
		{
			item.sessionChannelItemClosedList = new EmaVector<bool*>();
			item.closedListSize = concreteServiceList.size();

			for (UInt32 i = 0; i < routingChannelList.size(); ++i)
			{
				bool* pDirectoryFlagList = (bool*)malloc(sizeof(bool) * concreteServiceList.size());
				memset((void*)pDirectoryFlagList, 0, (sizeof(bool) * concreteServiceList.size()));
				item.sessionChannelItemClosedList->push_back(pDirectoryFlagList);
			}
		}

		// Iterate through the concrete service list, if there is a match, set the directory here and continue.
		for (UInt32 i = 0; i < concreteServiceList.size(); ++i)
		{
			ConsumerRoutingService** pRoutingServicePtr = baseImpl.getConsumerRoutingSession()->serviceByName.find(&concreteServiceList[serviceListIndex]);
			if (!pRoutingServicePtr)
			{
				// Increment the service list index here.  If it's equal to the size of the concreteServiceList, set the index to 0
				serviceListIndex++;
				if (serviceListIndex == concreteServiceList.size())
					serviceListIndex = 0;
				continue;
			}

			ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

			if (pRoutingService->isDeleted())
			{
				// We have seen this service before, but it currently isn't active, so we continue
				foundDirectory = true;
				serviceListIndex++;
				if (serviceListIndex == concreteServiceList.size())
					serviceListIndex = 0;
				continue;
			}

			for (UInt32 j = 0; j < pRoutingService->routingChannelList.size(); j++)
			{
				// Skip any channels that are NULL here.
				if (pRoutingService->routingChannelList[j] == NULL || pRoutingService->routingChannelList[j]->channelClosed == true)
					continue;

				// Skip this directory if it has already been marked as closed by this service on this session channel.
				if (item.sessionChannelItemClosedList != NULL && ((*(item.sessionChannelItemClosedList))[j])[serviceListIndex] == true)
					continue;

				// Skip this channel if the request is currently on the same service name as the consumer routing service and it's the same channel
				if (item.getDirectory() != NULL && pRoutingService->getName() == item.getDirectory()->getName() && pOldChannel == pRoutingService->routingChannelList[j])
					continue;

				pDirectory = *pRoutingService->routingChannelList[j]->serviceByName.find(&concreteServiceList[serviceListIndex]);

				if (pDirectory->serviceMatch(item.getReqMsg()) == true)
				{
					item.sessionChannel = pRoutingService->routingChannelList[j];
					item.currentServiceListIndex = serviceListIndex;
					item.setDirectory(pDirectory);
					item.setServiceName((EmaString&)pDirectory->getName());
					pRoutingService->routingChannelList[j]->routedRequestList.addItem(&item);
					matchedService = true;
					break;
				}
			}

			if (matchedService == true)
			{
				break;
			}

			// Increment the service list index here.  If it's equal to the size of the concreteServiceList, set the index to 0
			serviceListIndex++;
			if (serviceListIndex == concreteServiceList.size())
				serviceListIndex = 0;
		}

		if (item.sessionChannel == NULL)
		{
			return false;
		}
		else
			return true;
	}
	else if (item.getServiceName().length() != 0)
	{
		ConsumerRoutingService** pRoutingServicePtr = baseImpl.getConsumerRoutingSession()->serviceByName.find(&item.getServiceName());
		if (!pRoutingServicePtr)
		{
			return false;
		}

		ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

		if (pRoutingService->isDeleted())
		{
			return false;
		}

		if (item.sessionChannelItemClosedList == NULL)
		{
			item.sessionChannelItemClosedList = new EmaVector<bool*>();

			item.closedListSize = 1;

			for (UInt32 i = 0; i < routingChannelList.size(); ++i)
			{
				bool* pDirectoryFlag = (bool*)malloc(sizeof(bool));
				*pDirectoryFlag = false;
				item.sessionChannelItemClosedList->push_back(pDirectoryFlag);
			}
		}

		ConsumerRoutingSessionChannel* pOldChannel = item.sessionChannel;

		item.sessionChannel = NULL;

		for (UInt32 i = 0; i < pRoutingService->routingChannelList.size(); i++)
		{
			// Skip any channels that are NULL here.
			if (pRoutingService->routingChannelList[i] == NULL || pOldChannel == pRoutingService->routingChannelList[i])
				continue;

			// Skip this directory if it has already been marked as closed by this service on this session channel.
			// Since this does not have a serviceList, the bool pointer will be a singleton bool
			if (*((*(item.sessionChannelItemClosedList))[i]) == true)
				continue;

			Directory* pDirectory = *pRoutingService->routingChannelList[i]->serviceByName.find(&item.getServiceName());

			if (pDirectory->serviceMatch(item.getReqMsg()) == true)
			{
				item.sessionChannel = pRoutingService->routingChannelList[i];
				item.setDirectory(pDirectory);
				pRoutingService->routingChannelList[i]->routedRequestList.addItem(&item);
				break;
			}
		}

		if (item.sessionChannel == NULL)
		{
			return false;
		}
		else
			return true;
	}
	else if (item.hasServiceId())
	{
		ConsumerRoutingService** pRoutingServicePtr = baseImpl.getConsumerRoutingSession()->serviceById.find((UInt16)item.getServiceId());
		if (!pRoutingServicePtr)
		{
			return false;
		}

		ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

		if (pRoutingService->isDeleted())
		{
			return false;
		}

		if (item.sessionChannelItemClosedList == NULL)
		{
			item.sessionChannelItemClosedList = new EmaVector<bool*>();
			item.closedListSize = 1;

			for (UInt32 i = 0; i < routingChannelList.size(); ++i)
			{
				bool* pDirectoryFlag = (bool*)malloc(sizeof(bool));
				*pDirectoryFlag = false;
				item.sessionChannelItemClosedList->push_back(pDirectoryFlag);
			}
		}

		item.sessionChannel = NULL;

		for (UInt32 i = 0; i < pRoutingService->routingChannelList.size(); i++)
		{
			// Skip any channels that are NULL or match the old channel here.
			if (pRoutingService->routingChannelList[i] == NULL || pOldChannel == pRoutingService->routingChannelList[i])
				continue;

			Directory** pDirectoryPtr =  pRoutingService->routingChannelList[i]->serviceByName.find(&pRoutingService->getName());

			if (pDirectoryPtr == NULL)
				continue;

			Directory* pDirectory = *pDirectoryPtr;

			if (pDirectory->serviceMatch(item.getReqMsg()) == true)
			{
				item.sessionChannel = pRoutingService->routingChannelList[i];
				item.setDirectory(pDirectory);
				pRoutingService->routingChannelList[i]->routedRequestList.addItem(&item);
				break;
			}
		}


		if (item.sessionChannel == NULL)
		{
			return false;
		}
		else
			return true;
	}

	return false;
}

// This will process and handle sending login status messages when channel events happen
// Note: all channel close operations happen after this call.
void ConsumerRoutingSession::processChannelEvent(ConsumerRoutingSessionChannel* pSessionChannel, RsslReactorChannelEvent* pEvent)
{
	EmaVector<Item*>& loginItems = baseImpl.getLoginCallbackClient().getLoginItems();
	LoginCallbackClient& loginClient = baseImpl.getLoginCallbackClient();
	RsslStatusMsg statusMsg;
	RsslBuffer tmpBuffer = RSSL_INIT_BUFFER;
	int reconnectingCount = 0;

	// This will clear the statusMsg structure.
	pSessionChannel->loginInfo.loginRefreshMsg.populate(statusMsg, tmpBuffer);
	statusMsg.flags |= RSSL_STMF_HAS_STATE;

	switch (pEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
			// Do not send the login if the initial login refresh has been sent
			if (sentInitialLoginRefresh)
				return;

			// Stream state is OPEN. The data State is either OK or SUSPECT.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			if (sentInitialLoginRefresh == true)
				statusMsg.state.dataState = RSSL_DATA_OK;
			else
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

			statusMsg.state.text.data = (char*)channelUp.c_str();
			statusMsg.state.text.length = channelUp.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			pSessionChannel->sentChannelUpStatus = true;

			break;
		case RSSL_RC_CET_CHANNEL_READY:
			// Do not send the login we have already sent a channel up status
			if (pSessionChannel->sentChannelUpStatus == true)
				return;

			// Stream state is OPEN. The data State is either OK or SUSPECT.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			if(sentInitialLoginRefresh == true)
				statusMsg.state.dataState = RSSL_DATA_OK;
			else
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

			statusMsg.state.text.data = (char*)channelUp.c_str();
			statusMsg.state.text.length = channelUp.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			pSessionChannel->sentChannelUpStatus = true;

			break;
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:

			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// Only send SUSPECT if all channels are reconnecting.  Also, if they are all reconnecting, set sentInitialLoginRefresh to false.
			for (UInt32 i = 0; i < pSessionChannel->pRoutingSession->routingChannelList.size(); i++)
			{
				if (pSessionChannel->pRoutingSession->routingChannelList[i]->channelClosed == false && pSessionChannel->pRoutingSession->routingChannelList[i]->reconnecting == true)
				{
					++reconnectingCount;
				}
			}

			// If the ommConsumer is currently initializing, send SUSPECT unless we've sent a login OpenOk to everything.
			if ((!baseImpl.isInitialized() && baseImpl.getState() < OmmBaseImpl::LoginStreamOpenOkEnum) || reconnectingCount == pSessionChannel->pRoutingSession->activeChannelCount)
			{	 
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

				// Only clear this if initialization has succeded.
				if (baseImpl.isInitialized())
				{
					sentInitialLoginRefresh = false;
				}
			}
			else
			{
				statusMsg.state.dataState = RSSL_DATA_OK;
			}

			pSessionChannel->sentChannelUpStatus = false;

			statusMsg.state.text.data = (char*)channelReconnecting.c_str();
			statusMsg.state.text.length = channelReconnecting.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);
			break;
		case RSSL_RC_CET_CHANNEL_DOWN:
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// If this is the last channel, then send CLOSED/SUSPECT.
			if (pSessionChannel->pRoutingSession->activeChannelCount == 1)
			{
				statusMsg.state.streamState = RSSL_STREAM_CLOSED;
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			}
			else
			{
				// Only send SUSPECT if all remaining channels are reconnecting
				// The channel has not been closed yet, so the total active should be one less.
				for (UInt32 i = 0; i < pSessionChannel->pRoutingSession->routingChannelList.size(); i++)
				{
					if (pSessionChannel->pRoutingSession->routingChannelList[i]->channelClosed == false && pSessionChannel->pRoutingSession->routingChannelList[i]->reconnecting == true)
					{
						++reconnectingCount;
					}
				}

				// If the current channel is reconnecting, then this will be equal to activeChannelCount.
				// If the current channel is not reconnecting, then the count of all possible reconnecting channels is activeChannelCount -1.
				if (reconnectingCount >= pSessionChannel->pRoutingSession->activeChannelCount - 1)
				{
					statusMsg.state.dataState = RSSL_DATA_SUSPECT;
					sentInitialLoginRefresh = false;
				}
				else
				{
					statusMsg.state.dataState = RSSL_DATA_OK;
				}
			}

			pSessionChannel->sentChannelUpStatus = false;

			statusMsg.state.text.data = (char*)channelDown.c_str();
			statusMsg.state.text.length = channelDown.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);
			break;
		case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
			// The channel just got a DOWN_RECONNECTING call, so we need to preserve the state here.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// Only send SUSPECT if all channels are reconnecting.  Also, if they are all reconnecting, set sentInitialLoginRefresh to false.
			for (UInt32 i = 0; i < pSessionChannel->pRoutingSession->routingChannelList.size(); i++)
			{
				if (pSessionChannel->pRoutingSession->routingChannelList[i]->channelClosed == false && pSessionChannel->pRoutingSession->routingChannelList[i]->reconnecting == true)
				{
					++reconnectingCount;
				}
			}

			if (reconnectingCount == pSessionChannel->pRoutingSession->activeChannelCount)
			{
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			}
			else
			{
				statusMsg.state.dataState = RSSL_DATA_OK;
			}

			statusMsg.state.text.data = (char*)phFallbackComplete.c_str();
			statusMsg.state.text.length = phFallbackComplete.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			// This means that preferred host has ended, so set inPreferredHost to false.
			pSessionChannel->inPreferredHost = false;
			break;
		case RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK:
			// This can only happen on a channel that's in a good state, so it's always OPEN/OK

			statusMsg.state.streamState = RSSL_STREAM_OPEN;
			statusMsg.state.dataState = RSSL_DATA_OK;

			statusMsg.state.text.data = (char*)phStartingFallback.c_str();
			statusMsg.state.text.length = phStartingFallback.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			pSessionChannel->sentChannelUpStatus = true;

			// This means that preferred host has started, so set inPreferredHost to true.
			pSessionChannel->inPreferredHost = true;

			break;
		default:
			break;
	}
}