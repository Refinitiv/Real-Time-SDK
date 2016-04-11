/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the queue messaging handler for the UPA Value Add consumer application.
 * It handles any queue message delivery.
 */

#include <time.h>
#ifdef _WIN32
#else
#include <sys/time.h>
#endif


#include "rsslVAQueueCons.h"
#include "QConsSimpleQueueMsgHandler.h"

#define TUNNEL_STREAM_STREAM_ID 1000
#define QUEUE_MESSAGING_STREAM_ID 2000
#define QUEUE_MSG_DOMAIN 199
#define QUEUE_MSG_FREQUENCY 2

static RsslTunnelStreamOpenOptions tunnelStreamOpenOptions;
static RsslTunnelStreamSubmitMsgOptions tunnelStreamSubmitOptions;
static RsslInt64 identifier = 0;
static RsslInt msgSampleOrderId = 0;

#ifdef __cplusplus
extern "C" {
#endif
RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamStatusEvent *pEvent);
RsslReactorCallbackRet tunnelStreamQueueMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamQueueMsgEvent *pEvent);
RsslReactorCallbackRet tunnelStreamMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);
#ifdef __cplusplus
};
#endif

/* Publically visable - Used by the Consumer to open a tunnel stream once the Consumer's channel
 * is opened and the desired service identified. */
RsslRet openTunnelStream(RsslReactor *pReactor, RsslReactorChannel *pChnl, ChannelStorage *pCommand, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	if (pCommand->sourceName.data == NULL)
        return RSSL_RET_SUCCESS;

	rsslClearTunnelStreamOpenOptions(&tunnelStreamOpenOptions);
	tunnelStreamOpenOptions.name = (char*)"QueueConsumer";
    tunnelStreamOpenOptions.streamId = TUNNEL_STREAM_STREAM_ID;
    tunnelStreamOpenOptions.domainType = pCommand->tunnelStreamDomain;
    tunnelStreamOpenOptions.serviceId = (RsslUInt16)pCommand->queueServiceId;
	tunnelStreamOpenOptions.statusEventCallback = tunnelStreamStatusEventCallback;
	tunnelStreamOpenOptions.queueMsgCallback = tunnelStreamQueueMsgCallback;
	tunnelStreamOpenOptions.defaultMsgCallback = tunnelStreamMsgCallback;

	/* Queue messaging requires a reliable stream, bidirectional flow control, and queue persistence. */
	tunnelStreamOpenOptions.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
	tunnelStreamOpenOptions.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;
	tunnelStreamOpenOptions.classOfService.guarantee.type = RDM_COS_GU_PERSISTENT_QUEUE;

	if (pCommand->useAuthentication)
		tunnelStreamOpenOptions.classOfService.authentication.type = RDM_COS_AU_OMM_LOGIN;

	if ((ret = rsslReactorOpenTunnelStream(pChnl, &tunnelStreamOpenOptions, pErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorOpenTunnelStream failed: %s(%s)\n", rsslRetCodeToString(ret), pErrorInfo->rsslError.text);
		return ret;
	}

	printf("Opened Tunnel Stream.\n");

	return RSSL_RET_SUCCESS;
}

/* Callback for tunnel stream status events. */
/* Process a tunnel stream status event for the QueueMsgHandler. */
RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamStatusEvent *pEvent)
{
	RsslState *pState = pEvent->pState;
	RsslRDMQueueRequest queueRequest;
	RsslBuffer tempBuffer;
	char tempData[1024];
	ChannelStorage *pCommand = (ChannelStorage*)pEvent->pReactorChannel->userSpecPtr;
	RsslRet ret;
	RsslErrorInfo errorInfo;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	rsslStateToString(&tempBuffer, pState);
	printf("Received RsslTunnelStreamStatusEvent for stream \"%s\", with Stream ID %d and %.*s\n\n", pTunnelStream->name, pTunnelStream->streamId, tempBuffer.length, tempBuffer.data);

	switch(pState->streamState)
	{
		case RSSL_STREAM_OPEN:
		{
			if (pState->dataState == RSSL_DATA_OK
					&& pCommand->pHandlerTunnelStream == NULL)
			{
				/* New tunnel stream established. Send a QueueRequest to open our queue stream. */
				time_t currentTime = 0;

				/* get current time */
				time(&currentTime);

				pCommand->nextQueueMsgTime = currentTime + (time_t)QUEUE_MSG_FREQUENCY;

				pCommand->pHandlerTunnelStream = pTunnelStream;

				// open a queue message sub-stream for this tunnel stream
				rsslClearRDMQueueRequest(&queueRequest);
				queueRequest.rdmMsgBase.streamId = QUEUE_MESSAGING_STREAM_ID;
				queueRequest.rdmMsgBase.domainType = QUEUE_MSG_DOMAIN;
				queueRequest.sourceName =  pCommand->sourceName;

				rsslClearTunnelStreamSubmitMsgOptions(&tunnelStreamSubmitOptions);
				tunnelStreamSubmitOptions.pRDMMsg = (RsslRDMMsg *)&queueRequest;
				if ((ret = rsslTunnelStreamSubmitMsg(pTunnelStream, &tunnelStreamSubmitOptions, &errorInfo)) != RSSL_RET_SUCCESS)
					printf("rsslTunnelStreamSubmitMsg failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
			}
			break;
		}

		case RSSL_STREAM_CLOSED:
		case RSSL_STREAM_CLOSED_RECOVER:
		default:
		{
			RsslTunnelStreamCloseOptions closeOptions;
			rsslClearTunnelStreamCloseOptions(&closeOptions);
			if ((ret = rsslReactorCloseTunnelStream(pTunnelStream, &closeOptions, &errorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslReactorCloseTunnelStream failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);

			pCommand->tunnelStreamOpenRequested = RSSL_FALSE;
			pCommand->pHandlerTunnelStream = NULL;
			pCommand->isQueueStreamOpen = RSSL_FALSE;
			break;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for received tunnel stream messages. */
/* Processes a queue message event. */
RsslReactorCallbackRet tunnelStreamQueueMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamQueueMsgEvent *pEvent)
{
	RsslState *pState;
	RsslBuffer tempBuffer;
	ChannelStorage *pCommand = (ChannelStorage*)pTunnelStream->pReactorChannel->userSpecPtr;
	char tempData[1024];

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch (pEvent->pQueueMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_QMSG_MT_DATA:
        {
			RsslRDMQueueData *pQueueData = (RsslRDMQueueData *)pEvent->pQueueMsg;

			printf("Received Msg on stream %d to %.*s from %.*s, ID: " RTR_LLU "\n",
					pQueueData->rdmMsgBase.streamId, pQueueData->destName.length, pQueueData->destName.data, 
					pQueueData->sourceName.length, pQueueData->sourceName.data, pQueueData->identifier);

            switch (pQueueData->containerType)
            {
                case  RSSL_DT_OPAQUE:
                {
					printf(" Payload: %.*s\n\n", pQueueData->encDataBody.length, pQueueData->encDataBody.data);
					break;
                }
                default:
                    break;
            }                
            break;
        }

		case RDM_QMSG_MT_DATA_EXPIRED:
        {
			RsslRDMQueueDataExpired *pQueueDataExpired = (RsslRDMQueueDataExpired *)pEvent->pQueueMsg;

			printf("Received Msg on stream %d to %.*s from %.*s, ID: " RTR_LLU " (Undeliverable Message with code: %d(%s))\n",
					pQueueDataExpired->rdmMsgBase.streamId, pQueueDataExpired->destName.length, pQueueDataExpired->destName.data, 
					pQueueDataExpired->sourceName.length, pQueueDataExpired->sourceName.data, 
					pQueueDataExpired->identifier, 
					pQueueDataExpired->undeliverableCode, rsslRDMQueueUndeliverableCodeToString(pQueueDataExpired->undeliverableCode));

            switch (pQueueDataExpired->containerType)
            {
                case  RSSL_DT_OPAQUE:
                {
					printf(" Payload: %.*s\n\n", pQueueDataExpired->encDataBody.length, pQueueDataExpired->encDataBody.data);
					break;
                }
                default:
                    break;
            }                
            break;
        }
		case RDM_QMSG_MT_ACK:
		{
			RsslRDMQueueAck *pQueueAck = (RsslRDMQueueAck *)pEvent->pQueueMsg;
			printf("Received persistence Ack for submitted message with ID: " RTR_LLU "\n\n", pQueueAck->identifier);
			break;
		}
		case RDM_QMSG_MT_REFRESH:
		{
			RsslRDMQueueRefresh *pQueueRefresh = (RsslRDMQueueRefresh *)pEvent->pQueueMsg;
			pState = &pQueueRefresh->state;
			rsslStateToString(&tempBuffer, pState);
			printf("Received QueueRefresh on stream %d for sourceName %.*s with %.*s\n\n", pQueueRefresh->rdmMsgBase.streamId, pQueueRefresh->sourceName.length, pQueueRefresh->sourceName.data, tempBuffer.length, tempBuffer.data);

			if (pState->streamState == RSSL_STREAM_OPEN
					&& pState->dataState == RSSL_DATA_OK)
				pCommand->isQueueStreamOpen = RSSL_TRUE;
			else
				pCommand->isQueueStreamOpen = RSSL_FALSE;

			break;
		}
		case RDM_QMSG_MT_STATUS:
		{
			RsslRDMQueueStatus *pQueueStatus = (RsslRDMQueueStatus *)pEvent->pQueueMsg;
			if (pQueueStatus->flags & RDM_QMSG_STF_HAS_STATE)
			{
				pState = &pQueueStatus->state;
				rsslStateToString(&tempBuffer, pState);
				printf("Received QueueStatus on stream %d with %.*s\n\n", pQueueStatus->rdmMsgBase.streamId, tempBuffer.length, tempBuffer.data);

				if (pState->streamState == RSSL_STREAM_OPEN
						&& pState->dataState == RSSL_DATA_OK)
					pCommand->isQueueStreamOpen = RSSL_TRUE;
				else
					pCommand->isQueueStreamOpen = RSSL_FALSE;

			}
			else
			{
				printf("Received QueueStatus on stream %d\n\n", pQueueStatus->rdmMsgBase.streamId);
			}
			break;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for received tunnel stream messages. 
 * This application only performs queue messaging. Return RSSL_RC_CRET_RAISE from 
 * tunnelStreamQueueMsgCallback to invoke this callback. */
RsslReactorCallbackRet tunnelStreamMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	RsslMsg *pRsslMsg = pEvent->pRsslMsg;

	printf("Received RsslMsg with Stream ID %d, class %u(%s) and domainType %u(%s)\n\n",
			pRsslMsg->msgBase.streamId, 
			pRsslMsg->msgBase.msgClass, rsslMsgClassToString(pRsslMsg->msgBase.msgClass),
			pRsslMsg->msgBase.domainType, rsslDomainTypeToString(pRsslMsg->msgBase.domainType));

	return RSSL_RC_CRET_SUCCESS; 
}



RsslRet handleQueueMessaging(RsslReactor *pReactor, ChannelStorage *pCommand)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	if (currentTime >= pCommand->nextQueueMsgTime)
	{
		pCommand->nextQueueMsgTime = currentTime + (time_t)QUEUE_MSG_FREQUENCY;
		if (pCommand->pHandlerTunnelStream == NULL)
		{
			sendOpenTunnelStreamRequest(pReactor, pCommand->reactorChannel);
		}
		else if (pCommand->isQueueStreamOpen)
		{
			sendQueueMsg(pCommand->reactorChannel);
		}
	}

	return RSSL_RET_SUCCESS;
}

static void sendOpenTunnelStreamRequest(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	ChannelStorage *pCommand;
	RsslErrorInfo rsslErrorInfo;

	if (!pReactorChannel)
		return;

	pCommand = (ChannelStorage*)pReactorChannel->userSpecPtr;

	if (pCommand->tunnelStreamOpenRequested || !pCommand->isQueueServiceUp || !pCommand->queueServiceSupportsMessaging)
		return;

	if (openTunnelStream(pReactor, pReactorChannel, pCommand, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	pCommand->tunnelStreamOpenRequested = RSSL_TRUE;
}


/*
 * Encode and send a queue message through the ReactorChannel.
 * This message will contain an element list as its payload.
 * The element list contains two elements named BUY ORDER #
 * and SELL ORDER #. The order number is incremented for each
 * message sent.
 */
static void sendQueueMsg(RsslReactorChannel *pChnl)
{
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret, ret2;
	ChannelStorage *pCommand = (ChannelStorage*)pChnl->userSpecPtr;
	RsslTunnelStreamGetBufferOptions bufferOpts;
	RsslBuffer *pBuffer;
	RsslTunnelStreamSubmitOptions submitOpts;
	RsslRDMQueueData queueData;
	RsslEncodeIterator eIter;
	RsslUInt32 i;

	for (i = 0; i < pCommand->destNameCount; ++i)
	{
		/* Get a buffer from the tunnel stream to put our content in */
		rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
		bufferOpts.size = 1024;
		if ((pBuffer = rsslTunnelStreamGetBuffer(pCommand->pHandlerTunnelStream, &bufferOpts, &rsslErrorInfo)) == NULL)
		{
			printf("rsslTunnelStreamGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			return;
		}

		// initialize the QueueData encoding
		rsslClearRDMQueueData(&queueData);
		queueData.rdmMsgBase.streamId = QUEUE_MESSAGING_STREAM_ID;
		queueData.identifier = ++identifier;
		queueData.rdmMsgBase.domainType = QUEUE_MSG_DOMAIN;
		queueData.sourceName = pCommand->sourceName;
		queueData.destName = pCommand->destNames[i];
		queueData.timeout = RDM_QMSG_TC_INFINITE;
		queueData.containerType = RSSL_DT_OPAQUE;

		/* Populate payload with simple message */
		queueData.encDataBody.data = (char*)"Hello World!";
		queueData.encDataBody.length = 12;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChnl->majorVersion, pChnl->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

		/* Since we are communicating with a Queue Provider, encode the content into a Queue Message */
		if ((ret = rsslEncodeRDMQueueMsg(&eIter, (RsslRDMQueueMsg*)&queueData, &pBuffer->length, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMQueueMsg(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}


		// submit the encoded data buffer of QueueData to the tunnel stream
		rsslClearTunnelStreamSubmitOptions(&submitOpts);
		submitOpts.containerType = RSSL_DT_MSG;
		if ((ret = rsslTunnelStreamSubmit(pCommand->pHandlerTunnelStream, pBuffer, &submitOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslTunnelStreamSubmit(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		printf("Submitted message to %.*s with ID %lld\n\n", pCommand->destNames[i].length, pCommand->destNames[i].data, identifier);
	}
}

/*
 * Used by the Consumer to close any tunnel streams it opened
 * for the reactor channel.
 */
RsslRet closeTunnelStreams(RsslReactorChannel *pChnl, RsslErrorInfo *pErrorInfo)
{
    RsslRet ret;
	ChannelStorage *pCommand = NULL;
	
	if (pChnl)
		pCommand = (ChannelStorage*)pChnl->userSpecPtr;

	if (pCommand && pCommand->pHandlerTunnelStream)
	{
		RsslTunnelStreamCloseOptions closeOptions;
		rsslClearTunnelStreamCloseOptions(&closeOptions);
		closeOptions.finalStatusEvent = RSSL_TRUE;
		if ((ret =  rsslReactorCloseTunnelStream(pCommand->pHandlerTunnelStream, &closeOptions, pErrorInfo)) < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorCloseTunnelStream failed with return code: %d\n", ret);
		}
	}

    return RSSL_RET_SUCCESS;
}


