/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

/*
 * This is the tunnel stream handler for the UPA Value Add consumer and provider applications.
 * It handles operations related to opening a tunnel stream and receiving status events for it.
 * This handler is used by the PerfTunnelMsgHandler.
 */

#include "tunnelStreamHandler.h"

#define TUNNEL_STREAM_STREAM_ID 1000
#define TUNNEL_STREAM_RECOVERY_FREQUENCY 2


void tunnelStreamHandlerClear(TunnelStreamHandler *pTunnelHandler)
{
	
	pTunnelHandler->pTunnelStream = NULL;
	pTunnelHandler->serviceId = 0;
	pTunnelHandler->isTunnelServiceFound = RSSL_FALSE;
	pTunnelHandler->isTunnelServiceUp = RSSL_FALSE;
	pTunnelHandler->tunnelServiceSupported = RSSL_FALSE;
	pTunnelHandler->tunnelStreamOpenRequested = RSSL_FALSE;
	pTunnelHandler->waitFinalStatusEvent = RSSL_FALSE;
}

void tunnelStreamHandlerClearServiceInfo(TunnelStreamHandler *pTunnelHandler)
{
	pTunnelHandler->isTunnelServiceUp = RSSL_FALSE;
	pTunnelHandler->isTunnelServiceFound = RSSL_FALSE;
	pTunnelHandler->tunnelServiceSupported = RSSL_FALSE;
}

/* Callback for tunnel stream status events. */
/* Process a tunnel stream status event for the TunnelStreamHandler. */
RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamStatusEvent *pEvent)
{
	RsslState *pState = pEvent->pState;
	RsslBuffer tempBuffer;
	char tempData[1024];
	TunnelStreamHandler *pTunnelHandler = (TunnelStreamHandler*)pTunnelStream->userSpecPtr;
	RsslRet ret;
	RsslErrorInfo errorInfo;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	rsslStateToString(&tempBuffer, pState);
	printf("Received RsslTunnelStreamStatusEvent for stream \"%s\", with Stream ID %d and %.*s\n", pTunnelStream->name, pTunnelStream->streamId, tempBuffer.length, tempBuffer.data);

	switch(pState->streamState)
	{
		case RSSL_STREAM_OPEN:
		{
			if (pState->dataState == RSSL_DATA_OK
					&& pTunnelHandler->pTunnelStream == NULL)
			{
				if (pEvent->pAuthInfo)
				{
					printf("  Client was authenticated.\n");
				}

				pTunnelHandler->pTunnelStream = pTunnelStream;
				pTunnelHandler->processTunnelStreamOpened(pTunnelStream);
			}
			break;
		}

		case RSSL_STREAM_CLOSED:
		case RSSL_STREAM_CLOSED_RECOVER:
		default:
		{
			RsslTunnelStreamCloseOptions closeOpts;
			rsslClearTunnelStreamCloseOptions(&closeOpts);

			if (pEvent->pAuthInfo)
			{
				printf("  Client's authentication failed.\n");
			}

			/* For other stream states such as Closed & ClosedRecover, close the tunnel stream. */
			/* If we closed it already but asked for a final status event, don't need to close the tunnel stream again. */
			if (!pTunnelHandler->waitFinalStatusEvent)
				if ((ret = rsslReactorCloseTunnelStream(pTunnelStream, &closeOpts, &errorInfo)) != RSSL_RET_SUCCESS)
					printf("rsslReactorCloseTunnelStream failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
			pTunnelHandler->pTunnelStream = NULL;
			pTunnelHandler->waitFinalStatusEvent = RSSL_FALSE;
			pTunnelHandler->tunnelStreamOpenRequested = RSSL_FALSE;
			printf("Tunnel stream has closed.\n");
			break;
		}

	}

	printf("\n");

	return RSSL_RC_CRET_SUCCESS;
}

void handleTunnelStreamHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, TunnelStreamHandler *pTunnelHandler)
{
	time_t currentTime = 0;
	RsslRet ret;
	RsslTunnelStreamOpenOptions tunnelStreamOpenOptions;
	RsslErrorInfo errorInfo;

	if (pTunnelHandler->pTunnelStream != NULL)
		return;

	if (pTunnelHandler->tunnelStreamOpenRequested || !pTunnelHandler->isTunnelServiceUp || !pTunnelHandler->tunnelServiceSupported)
		return;

	/* get current time */
	time(&currentTime);
	if (currentTime < pTunnelHandler->nextRecoveryTime)
		return;

	pTunnelHandler->nextRecoveryTime = currentTime + TUNNEL_STREAM_RECOVERY_FREQUENCY;

	rsslClearTunnelStreamOpenOptions(&tunnelStreamOpenOptions);
	tunnelStreamOpenOptions.name = pTunnelHandler->consumerName;
	tunnelStreamOpenOptions.streamId = TUNNEL_STREAM_STREAM_ID;
	tunnelStreamOpenOptions.domainType = pTunnelHandler->domainType;
	tunnelStreamOpenOptions.serviceId = pTunnelHandler->serviceId;
	tunnelStreamOpenOptions.statusEventCallback = tunnelStreamStatusEventCallback;
	tunnelStreamOpenOptions.defaultMsgCallback = pTunnelHandler->defaultMsgCallback;
	tunnelStreamOpenOptions.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tunnelStreamOpenOptions.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
	tunnelStreamOpenOptions.userSpecPtr = pTunnelHandler;
	tunnelStreamOpenOptions.guaranteedOutputBuffers = pTunnelHandler->guaranteedOutputTunnelBuffers;

	if (pTunnelHandler->useAuthentication)
		tunnelStreamOpenOptions.classOfService.authentication.type = RDM_COS_AU_OMM_LOGIN;

	if ((ret = rsslReactorOpenTunnelStream(pReactorChannel, &tunnelStreamOpenOptions, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorOpenTunnelStream failed: %s(%s)\n", rsslRetCodeToString(ret), &errorInfo.rsslError.text);
		return;
	}

	pTunnelHandler->tunnelStreamOpenRequested = RSSL_TRUE;
	printf("Opened Tunnel Stream.\n");
}


/*
 * Used by the Consumer to close any tunnel streams it opened
 * for the reactor channel.
 */
void tunnelStreamHandlerCloseStreams(TunnelStreamHandler *pTunnelHandler)
{
    RsslRet ret;
	RsslErrorInfo errorInfo;

	if (pTunnelHandler->pTunnelStream)
	{
		RsslTunnelStreamCloseOptions closeOpts;
		rsslClearTunnelStreamCloseOptions(&closeOpts);
		// don't send and wait a final event
		// close the tunnel stream
//		closeOpts.finalStatusEvent = RSSL_TRUE;
//		pTunnelHandler->waitFinalStatusEvent = RSSL_TRUE;

		if ((ret = rsslReactorCloseTunnelStream(pTunnelHandler->pTunnelStream, &closeOpts, &errorInfo)) < RSSL_RET_SUCCESS)
			printf("rsslReactorCloseTunnelStream failed: %s(%s)\n", rsslRetCodeToString(ret), &errorInfo.rsslError.text);

		tunnelStreamHandlerClear(pTunnelHandler);
		printf("Tunnel stream has closed.\n");
	}
}

void tunnelStreamHandlerProcessServiceUpdate(TunnelStreamHandler *pTunnelHandler, 
											 RsslBuffer *pMatchServiceName, RsslRDMService* pService)
{
	/* Save service information for tunnel stream. */
	if (!pTunnelHandler->isTunnelServiceFound)
	{
		/* Check if the name matches the service we're looking for. */
		if (pService->flags & RDM_SVCF_HAS_INFO
			&& rsslBufferIsEqual(&pService->info.serviceName, pMatchServiceName))
		{
			pTunnelHandler->isTunnelServiceFound = RSSL_TRUE;
			pTunnelHandler->serviceId = (RsslUInt16)pService->serviceId;
		}
	}

	if (pService->serviceId == pTunnelHandler->serviceId)
	{
		/* Process the state of the tunnel stream service. */
		if (pService->action != RSSL_MPEA_DELETE_ENTRY)
		{
			RsslUInt32 i;
			pTunnelHandler->serviceId = (RsslUInt16)pService->serviceId;

			/* If info is present, check if tunnel stream support is indicated. */
			if (pService->flags & RDM_SVCF_HAS_INFO)
			{
				pTunnelHandler->tunnelServiceSupported = RSSL_FALSE;
				for(i = 0; i < pService->info.capabilitiesCount; ++i)
				{
					if (pService->info.capabilitiesList[i] == pTunnelHandler->domainType)
					{
						pTunnelHandler->tunnelServiceSupported = RSSL_TRUE;
						break;
					}
				}
			}

			/* Check state. */
			if (pService->flags & RDM_SVCF_HAS_STATE)
			{
				pTunnelHandler->isTunnelServiceUp = 
					pService->state.serviceState == 1 && 
					(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || 
					pService->state.acceptingRequests == 1);
			}
		}
		else
		{
			tunnelStreamHandlerClearServiceInfo(pTunnelHandler);
		}
	}
}

RsslInt tunnelStreamHandlerGetStreamId(void)
{
	return (RsslInt)(TUNNEL_STREAM_STREAM_ID);
}

