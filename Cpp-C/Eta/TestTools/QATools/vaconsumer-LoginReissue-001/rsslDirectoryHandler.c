/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the implementation of the callback for Directory responses
 * received by the rsslVAConsumer application.
 */

#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslVASendMessage.h"
#include "rtr/rsslRDMDirectoryMsg.h"

static void mergeGroups(ChannelCommand *pCommand, RsslBuffer *pGroupIdBuffer, RsslBuffer *pMergeToGroupIdBuffer);
static void mergeItemGroups(ChannelCommand *pCommand, RsslInt8 groupIdIndex, RsslInt8 mergeToGroupIdIndex);
// APIQA:  adding a counter
static int eventCounter = 0;
extern RsslBuffer authnToken;
extern RsslBuffer appId;
// END APIQA:

/*
 * Processes information contained in Directory responses.
 * Searches the refresh for the service the application
 * wants to use for this connection and stores the information
 * so we can request items from that service.
 */
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent)
{
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslRDMService *pService;
	RsslUInt32 i;
	RsslUInt32 n;
	RsslUInt32 c;
	RsslInt8 groupIdIndex = -1;
	ChannelCommand *pCommand = (ChannelCommand*)pChannel->userSpecPtr;
	RsslRDMDirectoryMsg *pDirectoryMsg = pDirectoryMsgEvent->pRDMDirectoryMsg;
	RsslRDMDirectoryRefresh *pRefresh;
	RsslRDMDirectoryUpdate *pUpdate;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	if (!pDirectoryMsg)
	{
		RsslErrorInfo *pError = pDirectoryMsgEvent->baseMsgEvent.pErrorInfo;
		printf("processDirectoryResponse: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		closeConnection(pReactor, pChannel, pCommand);
		return RSSL_RC_CRET_SUCCESS;
	}

 	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_DR_MT_REFRESH:
	{
		pRefresh = &pDirectoryMsg->refresh;
		printf("\nReceived Source Directory Response: ");
		rsslStateToString(&tempBuffer, &pRefresh->state);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		for (i = 0; i <  pRefresh->serviceCount; ++i)
		{
			RsslBuffer tmpServiceNameBuffer;

			pService = &pRefresh->serviceList[i];


			printf("Received serviceName: %.*s\n\n", pService->info.serviceName.length, pService->info.serviceName.data);

			/* check if name matches service name entered by user. 
			 * if it does, store the service ID and QoS. This will
			 * be used to make item requests later. */
			tmpServiceNameBuffer.data = pCommand->serviceName;
			tmpServiceNameBuffer.length = (RsslUInt32)strlen(pCommand->serviceName);
			if (rsslBufferIsEqual(&pService->info.serviceName, &tmpServiceNameBuffer))
			{
				pCommand->serviceId = pService->serviceId;
				pCommand->serviceNameFound = RSSL_TRUE;

				if (pService->info.qosCount)
					pCommand->qos = pService->info.qosList[0];
				else
				{
					pCommand->qos.timeliness = RSSL_QOS_TIME_REALTIME;
					pCommand->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
				}

				if (pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST
						&& !pCommand->userSpecSymbolList)
				{
					/* User did not specify a symbol list name because
					 * they wanted to use the source directory's ItemList. */
					pCommand->foundItemList = RSSL_TRUE;
					setSymbolListName(pCommand, pService->info.itemList);
				}

				if (pService->flags & RDM_SVCF_HAS_STATE)
				{
					printf("\tService State: %s\n\n", pService->state.serviceState == 1 ? "Up" : "Down");

					pCommand->isServiceReady = 
						pService->state.serviceState == 1 && 
						(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || 
						 pService->state.acceptingRequests == 1);
				}

				/* For service of interest, track available capabilities this
				 * application might use.
				 */
				pCommand->capabilitiesCount = 0;
				for (c = 0; c < pService->info.capabilitiesCount; c++)
				{
					if (pService->info.capabilitiesList[c] <= RSSL_DMT_MAX_RESERVED)
					{
						pCommand->capabilities[pCommand->capabilitiesCount] = pService->info.capabilitiesList[c];
						pCommand->capabilitiesCount++;
					}
				}
			}

			tmpServiceNameBuffer.data = pCommand->tunnelStreamServiceName;
			tmpServiceNameBuffer.length = (RsslUInt32)strlen(pCommand->tunnelStreamServiceName);

			if (pCommand->tunnelMessagingEnabled)
				tunnelStreamHandlerProcessServiceUpdate(&pCommand->simpleTunnelMsgHandler.tunnelStreamHandler,
					&tmpServiceNameBuffer, pService);
		}

		/* recover if service name entered by user cannot be found */
		if (!pCommand->serviceNameFound)
			printf("\nSource directory response does not contain service name: %s\n", pCommand->serviceName);
		else if (getSourceDirectoryCapabilities(pCommand, RSSL_DMT_DICTIONARY) == RSSL_FALSE
				&& !isDictionaryLoaded(pCommand))
			printf("\nService does not provide dictionaries for decoding item streams.\n");

		if (pCommand->tunnelMessagingEnabled) 
		{
			if (!pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.isTunnelServiceFound)
				printf("\nSource directory response does not contain service name for tunnel stream: %s\n", pCommand->tunnelStreamServiceName);
			else if (!pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.tunnelServiceSupported)
				printf("\nService in use for tunnel streams does not support them: %s\n", pCommand->tunnelStreamServiceName);
		}

		break;
	}

	case RDM_DR_MT_UPDATE:
		printf("\nReceived Source Directory Update\n");
		pUpdate = &pDirectoryMsg->update;

		for (i = 0; i <  pUpdate->serviceCount; ++i)
		{
			RsslBuffer tmpServiceNameBuffer;

			pService = &pUpdate->serviceList[i];

			printf("\tserviceName: %.*s Id: " RTR_LLU "\n",
					pService->info.serviceName.length, pService->info.serviceName.data, pService->serviceId);

			/* check if name matches service name entered by user. */
			tmpServiceNameBuffer.data = pCommand->serviceName;
			tmpServiceNameBuffer.length = (RsslUInt32)strlen(pCommand->serviceName);
			if (rsslBufferIsEqual(&pService->info.serviceName, &tmpServiceNameBuffer)
					|| (pService->serviceId == pCommand->serviceId))
			{
				pCommand->serviceId = pService->serviceId;
				pCommand->serviceNameFound = RSSL_TRUE;

				if (pService->flags & RDM_SVCF_HAS_INFO)
				{
				    RsslUInt32 limit;
					if (pService->info.qosCount)
						pCommand->qos = pService->info.qosList[0];
					else
					{
						pCommand->qos.timeliness = RSSL_QOS_TIME_REALTIME;
						pCommand->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
					}

					if ((pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST)
							&& !pCommand->userSpecSymbolList)
					{
						/* User did not specify a symbol list name because
						 * they wanted to use the source directory's ItemList. */
						pCommand->foundItemList = RSSL_TRUE;
						setSymbolListName(pCommand, pService->info.itemList);
					}

					/* For service of interest, track available capabilities this
					 * application might use.
					 */
					pCommand->capabilitiesCount = 0;
					limit = (pService->info.capabilitiesCount < MAX_NUM_CAPABILITIES ?
							 pService->info.capabilitiesCount : MAX_NUM_CAPABILITIES);
					for (c = 0; c < limit; c++)
					{
						if (pService->info.capabilitiesList[c] <= RSSL_DMT_MAX_RESERVED)
						{
							pCommand->capabilities[pCommand->capabilitiesCount] = pService->info.capabilitiesList[c];
							pCommand->capabilitiesCount++;
						}
					}
				}

				if (pService->flags & RDM_SVCF_HAS_STATE)
				{
					printf("\tService State: %s\n", pService->state.serviceState == 1 ? "Up" : "Down");

					pCommand->isServiceReady = 
						pService->state.serviceState == 1 && 
						(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || 
						 pService->state.acceptingRequests == 1);

					if (pService->state.flags & RDM_SVC_STF_HAS_STATUS)
					{
						/* status change for all items on service */
						tempBuffer.length = 1024;
						rsslStateToString(&tempBuffer, &pService->state.status);
						printf("\tService Status: %.*s\n", tempBuffer.length, tempBuffer.data);
						setItemStates(pCommand, -1, &pService->state.status);
					}
				}

				for (n = 0; n < pService->groupStateCount; n++)
				{
					/* status or merges applied to item groups */
					if (pService->groupStateList[n].flags & RDM_SVC_GRF_HAS_STATUS)
					{
						groupIdIndex = getGroupIdIndex(pCommand, &pService->groupStateList[n].group);
						if (groupIdIndex < 0)
							continue; /* We don't have any items with this group ID. */

						tempBuffer.length = 1024;
						rsslStateToString(&tempBuffer, &pService->groupStateList[n].status);
						printf("\tItem Group (index %d) Status: %.*s\n", groupIdIndex,
								tempBuffer.length, tempBuffer.data);

						if (groupIdIndex != -1)
							setItemStates(pCommand, groupIdIndex, &pService->groupStateList[n].status);
					}
					if (pService->groupStateList[n].flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
					{
						printf("\tItem Group Merge\n");
						mergeGroups(pCommand, &pService->groupStateList[n].group, &pService->groupStateList[n].mergedToGroup);
					}
				}

			}

			if (pCommand->tunnelMessagingEnabled)
				tunnelStreamHandlerProcessServiceUpdate(&pCommand->simpleTunnelMsgHandler.tunnelStreamHandler,
					&tmpServiceNameBuffer, pService);
		}

		if (!pCommand->serviceNameFound)
			printf("\nSource directory response does not contain service name: %s\n", pCommand->serviceName);
		else if (getSourceDirectoryCapabilities(pCommand, RSSL_DMT_DICTIONARY) == RSSL_FALSE
				&& !isDictionaryLoaded(pCommand))
			printf("\nService does not provide dictionaries for decoding item streams.\n");


		if (pCommand->tunnelMessagingEnabled) 
		{
			if (!pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.isTunnelServiceFound)
				printf("\nSource directory response does not contain service name for tunnel stream: %s\n", pCommand->tunnelStreamServiceName);
			else if (!pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.tunnelServiceSupported)
				printf("\nService in use for tunnel streams does not support them: %s\n", pCommand->tunnelStreamServiceName);
		}
		//APIQA: Send a RESUME ALL
		eventCounter++;
		if (eventCounter == 1)
		{
			//APIQA: Send login message with PAUSE ALL
			RsslRDMLoginRequest loginRequest;
			RsslReactorSubmitMsgOptions submitMsgOpts;
			RsslErrorInfo rsslErrorInfo;
			RsslRet ret = 0;
			if (rsslInitDefaultRDMLoginRequest(&loginRequest, 1) != RSSL_RET_SUCCESS)
			{
				printf("APIQA: rsslInitDefaultRDMLoginRequest() failed\n");
				break;
			}
			loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
			loginRequest.userName = authnToken;
			loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
			loginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
			loginRequest.applicationId = appId;
			rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
			submitMsgOpts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
			if ((ret = rsslReactorSubmitMsg(pReactor,pChannel,&submitMsgOpts,&rsslErrorInfo)) != RSSL_RET_SUCCESS )
			{
				printf("APIQA: rsslReactorSubmitMsg failed when attempting to send RESUME ALL:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			}
			printf("APIQA: sending RESUME ALL\n");
		}
		// END APIQA: Send a RESUME ALL 

		break;

	case RDM_DR_MT_CLOSE:
		printf("\nReceived Source Directory Close\n");
		break;

	case RDM_DR_MT_STATUS:
	{
		RsslRDMDirectoryStatus *pStatus = &pDirectoryMsg->status;
		printf("\nReceived Source Directory StatusMsg\n");
		if (pStatus->flags & RDM_DR_STF_HAS_STATE)
    	{
			rsslStateToString(&tempBuffer, &pStatus->state);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    	}
		break;
	}

	default:
		printf("\nReceived Unhandled Source Directory Msg Type: %d\n", pDirectoryMsg->rdmMsgBase.rdmMsgType);
    	break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Checks to see if the specified domainID appears in
the source directory's list of supported domains*/
RsslBool getSourceDirectoryCapabilities(ChannelCommand *pCommand, RsslUInt32 domainId)
{
	RsslUInt32 i;

	for(i = 0; i < pCommand->capabilitiesCount; i++)
	{
		if (domainId == pCommand->capabilities[i])
			return RSSL_TRUE;
	}

	return RSSL_FALSE;
}

static void mergeItemGroups(ChannelCommand *pCommand, RsslInt8 groupIdIndex, RsslInt8 mergeToGroupIdIndex)
{
	int i;

	printf("Merging from Item Group index [%d] to [%d]\n", groupIdIndex, mergeToGroupIdIndex);

	for(i = 0; i < pCommand->marketPriceItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketPriceItems[i].groupIdIndex)
		{
			pCommand->marketPriceItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketPricePSItems[i].groupIdIndex)
		{
			pCommand->marketPricePSItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->marketByOrderItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketByOrderItems[i].groupIdIndex)
		{
			pCommand->marketByOrderItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketByOrderItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketByOrderPSItems[i].groupIdIndex)
		{
			pCommand->marketByOrderPSItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->marketByPriceItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketByPriceItems[i].groupIdIndex)
		{
			pCommand->marketByPriceItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
	{
		if (groupIdIndex == pCommand->marketByPricePSItems[i].groupIdIndex)
		{
			pCommand->marketByPricePSItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->yieldCurveItemCount; i++)
	{
		if (groupIdIndex == pCommand->yieldCurveItems[i].groupIdIndex)
		{
			pCommand->yieldCurveItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
	for(i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
	{
		if (groupIdIndex == pCommand->yieldCurvePSItems[i].groupIdIndex)
		{
			pCommand->yieldCurvePSItems[i].groupIdIndex = mergeToGroupIdIndex;
		}
	}
}

static void mergeGroups(ChannelCommand *pCommand, RsslBuffer *pGroupIdBuffer, RsslBuffer *pMergeToGroupIdBuffer)
{
	RsslInt8 groupIdIndex;
	RsslInt8 mergeToGroupIdIndex;

	groupIdIndex = getGroupIdIndex(pCommand, pGroupIdBuffer);
	if (groupIdIndex < 0)
		return; /* We don't have any items with this group ID. */

	mergeToGroupIdIndex = getGroupIdIndex(pCommand, pMergeToGroupIdBuffer);
	if (mergeToGroupIdIndex < 0)
	{
		if (pCommand->groupIdCount >= MAX_NUM_GROUP_ID)
		{
			printf("mergeGroups failure: Exceeded maximum number of Group Ids (%d) supported by application\n", MAX_NUM_GROUP_ID);
			return;
		}

		mergeToGroupIdIndex = pCommand->groupIdCount;
		pCommand->groupIdCount++;

		pCommand->groupIdBuffers[mergeToGroupIdIndex].data = (char*) malloc(pMergeToGroupIdBuffer->length);
		pCommand->groupIdBuffers[mergeToGroupIdIndex].length = pMergeToGroupIdBuffer->length;

		memcpy(pCommand->groupIdBuffers[mergeToGroupIdIndex].data, pMergeToGroupIdBuffer->data, pMergeToGroupIdBuffer->length);
	}

	mergeItemGroups(pCommand, groupIdIndex, mergeToGroupIdIndex);
}



