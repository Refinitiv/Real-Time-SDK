/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#include "ConsumerRoutingChannel.h"
#include "ConsumerRoutingService.h"
#include "DirectoryCallbackClient.h"
#include "rtr/rsslDataPackage.h"

using namespace refinitiv::ema::access;


ConsumerRoutingService::ConsumerRoutingService(OmmBaseImpl& ommBaseImpl) :
	Directory(ommBaseImpl),
	routingChannelList()
{
	for (UInt32 i = 0; i < _ommBaseImpl.getActiveConfig().consumerRoutingSessionSet.size(); i++)
	{
		routingChannelList.push_back(NULL);
	}
	activeServiceCount = 0;
	initialized = false;
}

ConsumerRoutingService::~ConsumerRoutingService()
{
}

ConsumerRoutingService& ConsumerRoutingService::clear()
{
	Directory::clear();

	routingChannelList.clear();
	for (UInt32 i = 0; i < _ommBaseImpl.getActiveConfig().consumerRoutingSessionSet.size(); i++)
	{
		routingChannelList.push_back(NULL);
	}
	activeServiceCount = 0;

	initialized = false;

	return *this;
}

ConsumerRoutingService::AggregationResultEnum ConsumerRoutingService::aggregateDirectoryInfo(Directory& newDirectory, RsslMapEntryActions action)
{
	bool sendUpdate = false;
	if (initialized == false)
	{
		UInt64 currentId = getId();
		// This should only be called when it's not a DELETE action.
		setService(newDirectory.getService());

		setId(currentId);
		
		// Insert the routing channel into the proper place in the routing session set.
		EmaString& routingSessionName = newDirectory._pChannel->getConsumerRoutingChannel()->name;

		for (UInt32 i = 0; i < _ommBaseImpl.getActiveConfig().consumerRoutingSessionSet.size(); i++)
		{
			if (_ommBaseImpl.getActiveConfig().consumerRoutingSessionSet[i]->name == routingSessionName)
			{
				if (routingChannelList[i] == NULL)
				{
					activeServiceCount++;

					routingChannelList[i] = newDirectory._pChannel->getConsumerRoutingChannel();
					newDirectory.setGeneratedServiceId(_id);
				}
				break;
			}
		}

		markActive();
		// Service Id will be overwritten in the ConsumerRoutingSession.
		initialized = true;
		return SuccessNewServiceEnum;
	}
	
	// This has been initialized, so we're going to either be adding another 
	switch (action)
	{
		// Watchlist will detect if the service has been given to us prior to now, so ADD_ENTRY should only happen on duplicate connections.
		case RSSL_MPEA_ADD_ENTRY:
		case RSSL_MPEA_UPDATE_ENTRY:
		{
			// Compare the aggregation.  If this fails, just return failurre
			if (compareAggregatedService(newDirectory) == FailureEnum)
				return FailureEnum;

			markActive();

			// Add or increment any added domains
			for (UInt32 i = 0; i < newDirectory._supportedDomains.size(); ++i)
			{
				DirectoryDomainType newDomainType;
				newDomainType.domain = (UInt32)newDirectory._supportedDomains[i]->domain;

				Int64 found = _supportedDomains.search(const_cast<DirectoryDomainType*>(&newDomainType), DirectoryDomainType::compare);
				if (found == -1)
				{
					DirectoryDomainType* insertDomain = new DirectoryDomainType();
					insertDomain->domain = (UInt32)newDirectory._supportedDomains[i]->domain;
					insertDomain->count = 1;

					_supportedDomains.insert_sorted(const_cast<DirectoryDomainType*>(insertDomain), DirectoryDomainType::compare);
					sendUpdate = true;
				}
				else
				{
					if (newDirectory._supportedDomains[i]->count == 0 && _supportedDomains[(UInt32)found]->count != 0)
					{
						_supportedDomains[(UInt32)found]->count--;
					}
					else
					{
						_supportedDomains[(UInt32)found]->count++;
					}
				}
			}

			if (newDirectory._service.flags & RDM_SVCF_HAS_STATE)
			{
				RsslUInt oldAcceptingRequests = _service.state.acceptingRequests;
				RsslUInt oldServiceState = _service.state.serviceState;

				if ((newDirectory._service.state.flags | RDM_SVC_STF_HAS_ACCEPTING_REQS) != 0)
				{
					_service.state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
					if (newDirectory._service.state.acceptingRequests == 0)
					{
						bool acceptingRequests = false;
						for (UInt32 i = 0; i < routingChannelList.size(); i++)
						{
							if (routingChannelList[i] == NULL)
								continue;

							DirectoryPtr* chnlDirectoryPtr = routingChannelList[i]->serviceByName.find(&newDirectory._name);

							if (chnlDirectoryPtr == NULL)
							{
								const char* temp = "Failed to create chnlDirectoryPtr in aggregregateDirectoryInfo. Out of memory.";
								throwMeeException(temp);
							}
							Directory* pDirectory = *chnlDirectoryPtr;

							if (pDirectory->_service.state.acceptingRequests == 1)
							{
								acceptingRequests = true;
								break;
							}
						}

						if (acceptingRequests == false)
							_service.state.acceptingRequests = 0;

					}
					else
					{
						// Doesn't matter what the previous value(s) were, we have at least one service now accepting requests.
						_service.state.acceptingRequests = newDirectory._service.state.acceptingRequests;
					}

					if (oldAcceptingRequests != _service.state.acceptingRequests)
					{
						sendUpdate = true;
					}
				}

				if (newDirectory._service.state.serviceState == 0)
				{
					bool serviceStateUp = false;
					for (UInt32 i = 0; i < routingChannelList.size(); i++)
					{
						if (routingChannelList[i] == NULL)
							continue;

						DirectoryPtr* chnlDirectoryPtr = routingChannelList[i]->serviceByName.find(&newDirectory._name);

						if (chnlDirectoryPtr == NULL)
						{
							// Service is not in this channel, so continue;
							continue;
						}

						Directory* pDirectory = *chnlDirectoryPtr;

						if (pDirectory->_service.state.serviceState == 1)
						{
							serviceStateUp = true;
							break;
						}
					}

					if (serviceStateUp == false)
						_service.state.serviceState = 0;
				}
				else
				{
					// Doesn't matter what the previous value(s) were, we have at least one service now up.
					_service.state.serviceState = newDirectory._service.state.serviceState;
				}

				if (oldServiceState != _service.state.serviceState)
				{
					sendUpdate = true;
				}
			}
		
			int oldActiveServiceCount = activeServiceCount;
			EmaString& routingSessionName = newDirectory._pChannel->getConsumerRoutingChannel()->name;

			for (UInt32 i = 0; i < _ommBaseImpl.getActiveConfig().consumerRoutingSessionSet.size(); i++)
			{
				if (_ommBaseImpl.getActiveConfig().consumerRoutingSessionSet[i]->name == routingSessionName)
				{
					if (routingChannelList[i] == NULL)
					{
						routingChannelList[i] = newDirectory._pChannel->getConsumerRoutingChannel();
						newDirectory.setGeneratedServiceId(_id);
						activeServiceCount++;
					}
					break;
				}
			}

			// If we're adding this to an empty channel list, that means that all channels were removed previously, so the user has gotten a DELETE update prior.  Add this to the add list.
			if (oldActiveServiceCount == 0)
			{
				return SuccessNewServiceEnum;
			}
			break;
		}
		case RSSL_MPEA_DELETE_ENTRY:
		{
			// If this is currently in the routing channel list, remove it
			Int64 pos = routingChannelList.getPositionOf(newDirectory._pChannel->getConsumerRoutingChannel());

			if (pos != -1)
			{
				activeServiceCount--;
				routingChannelList[(UInt32)pos] = NULL;
			}
			else
			{
				break;
			}

			// This service is going away, so decrement any capabilities, and check the service state.  
			for (UInt32 i = 0; i < newDirectory._supportedDomains.size(); ++i)
			{
				DirectoryDomainType newDomainType;
				newDomainType.domain = newDirectory._supportedDomains[i]->domain;

				Int64 found = _supportedDomains.search(const_cast<DirectoryDomainType*>(&newDomainType), DirectoryDomainType::compare);
				if (found != -1)
				{
					if (_supportedDomains[(UInt32)found]->count != 0)
					{
						_supportedDomains[i]->count--;
						if (_supportedDomains[i]->count == 0)
							sendUpdate = true;

					}
				}
			}

			// Check to see if this was the last channel with accepting requests.  If there are any currently active channels, leave acceptinRequests as 1.
			RsslUInt oldAcceptingRequests = _service.state.acceptingRequests;
			RsslUInt oldServiceState = _service.state.serviceState;
			bool acceptingRequests = false;
			for (UInt32 i = 0; i < routingChannelList.size(); i++)
			{
				if (routingChannelList[i] == NULL)
					continue;

				DirectoryPtr* chnlDirectoryPtr = routingChannelList[i]->serviceByName.find(&newDirectory._name);

				if (chnlDirectoryPtr == NULL)
				{
					// Channel does not have this service, continue;
					continue;
				}
				Directory* pDirectory = *chnlDirectoryPtr;

				if (pDirectory->_service.state.acceptingRequests == 1)
				{
					acceptingRequests = true;
					break;
				}
			}

			if (acceptingRequests == false)
				_service.state.acceptingRequests = 0;

			if (oldAcceptingRequests != _service.state.acceptingRequests)
			{
				sendUpdate = true;
			}

			bool serviceStateUp = false;
			for (UInt32 i = 0; i < routingChannelList.size(); i++)
			{
				if (routingChannelList[i] == NULL)
					continue;

				DirectoryPtr* chnlDirectoryPtr = routingChannelList[i]->serviceByName.find(&newDirectory._name);

				if (chnlDirectoryPtr == NULL)
				{
					// Channel does not have this service, continue;
					continue;
				}
				Directory* pDirectory = *chnlDirectoryPtr;

				if (pDirectory->_service.state.serviceState == 1)
				{
					serviceStateUp = true;
					break;
				}
			}

			if (serviceStateUp == false)
				_service.state.serviceState = 0;

			if (oldServiceState != _service.state.serviceState)
			{
				sendUpdate = true;
			}

			if (activeServiceCount == 0)
			{
				// Mark the service as deleted here so we will ignore it when trying to route.
				markDeleted();
				return SuccessAndDeleteEnum;
			}
			break;
		}
	}

	if (sendUpdate == true)
		return SuccessWithUpdateEnum;
	else
		return SuccessNoUpdateEnum;

}

// This checks the following for equality:
// The itemList name is the same
// If both have a qos list, verify that the contents of the qos list is the same for both new and current
// Verify that the qosRange option is the same
ConsumerRoutingService::AggregationResultEnum ConsumerRoutingService::compareAggregatedService(Directory& newDirectory)
{
	if (newDirectory._itemList != _itemList)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString tmpString = "Received RsslRDMService name ";
			tmpString.append(_name).append(". From session channel ").append(newDirectory.getChannel()->getConsumerRoutingChannel()->name);
			tmpString.append(". With a mismatched Item List name. ");
			if (_ommBaseImpl.isInitialized() == true)
			{
				tmpString.append(" Dropping this service. ");
			}
			else
			{
				tmpString.append(" Closing this connection. ");
			}
				
			tmpString.append("Expected Item List : ").append(_itemList).append(" Received : ").append(newDirectory._itemList);
			_ommBaseImpl.getOmmLoggerClient().log("ConsumerRoutingService", OmmLoggerClient::ErrorEnum, tmpString);

			return FailureEnum;
		}
	}

	bool qosMatch = true;

	if (_service.info.flags & RDM_SVC_IFF_HAS_QOS)
	{
		if (newDirectory._service.info.flags & RDM_SVC_IFF_HAS_QOS)
		{
			if (_supportedQos.size() == newDirectory._supportedQos.size())
			{
				// Iterate through the old directory
				for (UInt32 i = 0; i < _supportedQos.size(); i++)
				{
					bool qosFound = false;
					// Iterate through the new directory
					for (UInt32 j = 0; j < _supportedQos.size(); ++j)
					{
						if (rsslQosIsEqual(&_supportedQos[i]->qos, &newDirectory._supportedQos[j]->qos) == RSSL_TRUE)
						{
							qosFound = true;
							break;
						}
					}

					if (qosFound == false)
					{
						qosMatch = false;
						break;
					}
				}
			}
			else
			{
				qosMatch = false;
			}
		}
		else
		{
			qosMatch = false;
		}
	}
	else if (newDirectory._service.info.flags & RDM_SVC_IFF_HAS_QOS)
	{
		qosMatch = false;
	}


	if (qosMatch == false)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString tmpString = "Received RsslRDMService name ";
			tmpString.append(_name).append(". From session channel: ").append(newDirectory.getChannel()->getConsumerRoutingChannel()->name).append(". With a mismatched qos list. ");
			if (_ommBaseImpl.isInitialized() == true)
			{
				tmpString.append(" Dropping this service.");
			}
			else
			{
				tmpString.append(" Closing this connection.");
			}
			_ommBaseImpl.getOmmLoggerClient().log("ConsumerRoutingService", OmmLoggerClient::ErrorEnum, tmpString);

			return FailureEnum;
		}
	}

	bool qosRangeMatch = true;

	if (_service.info.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
	{
		if (newDirectory._service.info.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
		{

			if (_service.info.supportsQosRange != newDirectory._service.info.supportsQosRange)
			{
				qosRangeMatch = false;
			}
			else
			{
				qosRangeMatch = false;
			}
		}
		else
		{
			qosRangeMatch = false;
		}
	}
	else if (newDirectory._service.info.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
	{
		qosRangeMatch = false;
	}

	if (qosRangeMatch == false)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString tmpString = "Received RsslRDMService name ";
			tmpString.append(_name).append(" From session channel: ").append(newDirectory.getChannel()->getConsumerRoutingChannel()->name);
			tmpString.append(". With a mismatched qos range configuration.");
			if (_ommBaseImpl.isInitialized() == true)
			{
				tmpString.append(" Dropping this service.");
			}
			else
			{
				tmpString.append(" Closing this connection.");
			}
			_ommBaseImpl.getOmmLoggerClient().log("ConsumerRoutingService", OmmLoggerClient::ErrorEnum, tmpString);

			return FailureEnum;
		}
	}

	return SuccessNoUpdateEnum;

}
