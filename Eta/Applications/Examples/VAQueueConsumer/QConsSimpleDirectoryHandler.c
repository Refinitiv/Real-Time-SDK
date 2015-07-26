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

#include "QConsSimpleDirectoryHandler.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "rsslVAQueueCons.h"

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
	RsslUInt32 c;
	RsslInt8 groupIdIndex = -1;
	ChannelStorage *pCommand = (ChannelStorage*)pChannel->userSpecPtr;
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

			tmpServiceNameBuffer.data = pCommand->queueServiceName;
			tmpServiceNameBuffer.length = (RsslUInt32)strlen(pCommand->queueServiceName);
			if (rsslBufferIsEqual(&pService->info.serviceName, &tmpServiceNameBuffer))
			{
				pCommand->queueServiceId = pService->serviceId;
				pCommand->queueServiceNameFound = RSSL_TRUE;

				/* Check state of service. */
				if (pService->flags & RDM_SVCF_HAS_STATE)
				{
					printf("\tQueue Service State: %s\n\n", pService->state.serviceState == 1 ? "Up" : "Down");

					pCommand->isQueueServiceUp = 
						pService->state.serviceState == 1 && 
						(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || 
						 pService->state.acceptingRequests == 1);
				}

				/* Check if service supports the domain we will use for messaging. */
				for (c = 0; c < pService->info.capabilitiesCount; c++)
				{
					if (pService->info.capabilitiesList[c] == pCommand->tunnelStreamDomain)
					{
						pCommand->queueServiceSupportsMessaging = RSSL_TRUE;
						break;
					}
				}

				if (c == pService->info.capabilitiesCount)
				{
					printf("Service for queue messaging does not support domain %d\n", pCommand->tunnelStreamDomain);
					pCommand->queueServiceSupportsMessaging = RSSL_FALSE;
				}
			}
		}

		/* recover if service name entered by user cannot be found */
		if (!pCommand->queueServiceNameFound)
			printf("\nSource directory response does not contain service name for queue messaging: %s\n", pCommand->queueServiceName);

		break;
	}

	case RDM_DR_MT_UPDATE:
		printf("\nReceived Source Directory Update\n");
		pUpdate = &pDirectoryMsg->update;

		for (i = 0; i <  pUpdate->serviceCount; ++i)
		{
			RsslBuffer tmpServiceNameBuffer;

			pService = &pUpdate->serviceList[i];

			printf("\tserviceName: %.*s Id: %d\n",
					pService->info.serviceName.length, pService->info.serviceName.data, pService->serviceId);

			/* check if name matches service name entered by user. */
			tmpServiceNameBuffer.data = pCommand->serviceName;
			tmpServiceNameBuffer.length = (RsslUInt32)strlen(pCommand->serviceName);
			if (rsslBufferIsEqual(&pService->info.serviceName, &tmpServiceNameBuffer)
					|| (pService->serviceId == pCommand->serviceId))
			{
				pCommand->serviceId = pService->serviceId;

				if (pService->flags & RDM_SVCF_HAS_INFO)
				{
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
						
					}
				}
			

			}

			if (rsslBufferIsEqual(&pService->info.serviceName, &tmpServiceNameBuffer)
					|| (pService->serviceId == pCommand->queueServiceId))
			{
				pCommand->queueServiceId = pService->serviceId;
				pCommand->queueServiceNameFound = RSSL_TRUE;

				if (pService->flags & RDM_SVCF_HAS_INFO)
				{
					/* Check if service supports the domain we will use for messaging. */
					for (c = 0; c < pService->info.capabilitiesCount; c++)
					{
						if (pService->info.capabilitiesList[c] == pCommand->tunnelStreamDomain)
						{
							pCommand->queueServiceSupportsMessaging = RSSL_TRUE;
							break;
						}
					}

					if (c == pService->info.capabilitiesCount)
					{
						printf("Service for queue messaging does not support domain %d\n", pCommand->tunnelStreamDomain);
						pCommand->queueServiceSupportsMessaging = RSSL_FALSE;
					}
				}

				/* Check state of service. */
				if (pService->flags & RDM_SVCF_HAS_STATE)
				{
					printf("\tQueue Service State: %s\n\n", pService->state.serviceState == 1 ? "Up" : "Down");

					pCommand->isQueueServiceUp = 
						pService->state.serviceState == 1 && 
						(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || 
						 pService->state.acceptingRequests == 1);
				}

			}
		}

		if (!pCommand->queueServiceNameFound)
			printf("\nSource directory response does not contain service name for queue messaging: %s\n", pCommand->queueServiceName);

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
RsslBool getSourceDirectoryCapabilities(ChannelStorage *pCommand, RsslUInt32 domainId)
{
	RsslUInt32 i;

	for(i = 0; i < pCommand->capabilitiesCount; i++)
	{
		if (domainId == pCommand->capabilities[i])
			return RSSL_TRUE;
	}

	return RSSL_FALSE;
}





