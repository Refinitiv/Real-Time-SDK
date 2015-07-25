/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "consumerThreads.h"
#include "getTime.h"
#include "testUtils.h"
#include "marketPriceDecoder.h"
#include "marketPriceEncoder.h"
#include "marketByOrderDecoder.h"
#include "marketByOrderEncoder.h"
#include "xmlItemListParser.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#define LATENCY_RANDOM_ARRAY_SET_COUNT 20

//uncomment the following line for debugging only - this will greatly affect performance
//#define ENABLE_XML_TRACE

static const RsslInt32 
			LOGIN_STREAM_ID = 1,
			DIRECTORY_STREAM_ID = 2,
			FIELD_DICTIONARY_STREAM_ID = 3,
			ENUM_TYPE_DICTIONARY_STREAM_ID = 4,
			ITEM_STREAM_ID_START = 5;

static RsslBuffer applicationName = { 12, (char*)"upacConsPerf" } ;

RsslPostUserInfo postUserInfo;

RsslBool shutdownThreads = RSSL_FALSE;

RsslRet RTR_C_INLINE decodePayload(RsslDecodeIterator* dIter, RsslMsg *msg, ConsumerThread* pConsumerThread)
{
	RsslRet ret;

	switch(msg->msgBase.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			if ((ret = decodeMPUpdate(dIter, msg, pConsumerThread)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMPUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		case RSSL_DMT_MARKET_BY_ORDER:
			if ((ret = decodeMBOUpdate(dIter, msg, pConsumerThread) != RSSL_RET_SUCCESS))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMBOUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		default:
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"decodePayload(): Unhandled domain type %s(%d)", 
					rsslDomainTypeToString(msg->msgBase.domainType), msg->msgBase.domainType);
			return RSSL_RET_FAILURE;
	}
}

RsslRet sendMessage(ConsumerThread *pConsumerThread, RsslBuffer *msgBuf)
{
	RsslUInt32 bytesWritten, uncompBytesWritten;
	RsslRet ret;

	ret = rsslWrite(pConsumerThread->pChannel, msgBuf, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompBytesWritten, &pConsumerThread->threadRsslError);

	/* call flush and write again */
	while (rtrUnlikely(ret == RSSL_RET_WRITE_CALL_AGAIN))
	{
		if (rtrUnlikely((ret = rsslFlush(pConsumerThread->pChannel, &pConsumerThread->threadRsslError)) < RSSL_RET_SUCCESS))
		{
			rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
			return ret;
		}
		ret = rsslWrite(pConsumerThread->pChannel, msgBuf, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompBytesWritten, &pConsumerThread->threadRsslError);
	}

	if (ret > RSSL_RET_SUCCESS || ret == RSSL_RET_WRITE_FLUSH_FAILED && pConsumerThread->pChannel->state == RSSL_CH_STATE_ACTIVE)
	{
		FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds);
	}
	else if (ret < RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/* Sends a basic Login Request on the given streamID, using the RDM package. */
RsslRet sendLoginRequest(ConsumerThread *pConsumerThread, RsslInt32 streamId)
{
	RsslRDMLoginRequest loginRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	/* Send Login Request */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMLoginRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (strlen(consPerfConfig.username))
	{
		loginRequest.userName.data = consPerfConfig.username;
		loginRequest.userName.length = (RsslUInt32)strlen(consPerfConfig.username);
	}

	loginRequest.flags |= RDM_LG_RQF_HAS_ROLE | RDM_LG_RQF_HAS_APPLICATION_NAME;
	loginRequest.applicationName = applicationName;

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		 return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	return RSSL_RET_SUCCESS;
}

/* Sends a basic Directory Request on the given streamID, using the RDM package. */
RsslRet sendDirectoryRequest(ConsumerThread *pConsumerThread, RsslInt32 streamId)
{
	RsslRDMDirectoryRequest directoryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	/* Send Directory Request */
	if ((ret = rsslInitDefaultRDMDirectoryRequest(&directoryRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMDirectoryRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet sendDictionaryRequests(ConsumerThread *pConsumerThread, RsslUInt16 serviceId, 
		RsslInt32 fieldDictionaryStreamId, RsslInt32 enumTypeDictionaryStreamId)
{
	RsslRDMDictionaryRequest dictionaryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;

	rsslClearRDMDictionaryRequest(&dictionaryRequest);
	dictionaryRequest.flags = RDM_DC_RQF_STREAMING;
	dictionaryRequest.serviceId = serviceId;


	/* Field dictionary request. */
	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
		return pConsumerThread->threadRsslError.rsslErrorId;

	dictionaryRequest.dictionaryName.data = (char*)"RWFFld";
	dictionaryRequest.dictionaryName.length = 6;
	dictionaryRequest.rdmMsgBase.streamId = fieldDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	/* Enumerated types dictionary request. */
	if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 128, RSSL_FALSE, &pConsumerThread->threadRsslError)))
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		return pConsumerThread->threadRsslError.rsslErrorId;
	}

	dictionaryRequest.dictionaryName.data = (char*)"RWFEnum";
	dictionaryRequest.dictionaryName.length = 7;
	dictionaryRequest.rdmMsgBase.streamId = enumTypeDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
		 return ret;

	if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet sendItemRequestBurst(ConsumerThread *pConsumerThread, RsslRDMService *pService, RsslQueue *pRequestQueue, RsslUInt32 itemBurstCount, RsslQueue *pWaitingForRefreshQueue)
{
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;
	RsslRequestMsg requestMsg;
	RsslUInt32 i;
	RsslQos *pQos;

	assert(pService);
	assert(itemBurstCount);

	/* Use a QoS from the service, if one is given. */
	if ((pService->flags & RDM_SVCF_HAS_INFO) && (pService->info.flags & RDM_SVC_IFF_HAS_QOS ) && (pService->info.qosCount > 0))
		pQos = &pService->info.qosList[0];
	else
		pQos = NULL;

	for(i = 0; i < itemBurstCount; ++i)
	{
		RsslQueueLink *pLink;
		ItemRequest *pItemRequest;

	   	pLink = rsslQueuePeekFront(pRequestQueue);
		if (!pLink) return RSSL_RET_SUCCESS;

		pItemRequest = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, link, pLink);
		assert(pItemRequest->requestState == ITEM_NOT_REQUESTED);
		if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 512, RSSL_FALSE, &pConsumerThread->threadRsslError)))
		{
			if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
			{
				pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return pConsumerThread->threadRsslError.rsslErrorId;
			}
		}

		/* Desired service has been found, so add it to the msgKey. */
		pItemRequest->msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		pItemRequest->msgKey.serviceId = (RsslUInt16)pService->serviceId;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);
		if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, pConsumerThread->threadErrorInfo.rsslError.text);
			return ret;
		}

		/* Encode request msg. */
		rsslClearRequestMsg(&requestMsg);
		if (!consPerfConfig.requestSnapshots && pItemRequest->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
			requestMsg.flags |= RSSL_RQMF_STREAMING;

		if (pQos)
		{
			requestMsg.flags |= RSSL_RQMF_HAS_QOS;
			requestMsg.qos = *pQos;
		}

		if (pItemRequest->itemInfo.itemFlags & ITEM_IS_GEN_MSG)
			requestMsg.flags |= RSSL_RQMF_PRIVATE_STREAM;

		requestMsg.msgBase.streamId = pItemRequest->itemInfo.StreamId;
		requestMsg.msgBase.domainType = pItemRequest->itemInfo.attributes.domainType;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

		requestMsg.msgBase.msgKey = pItemRequest->msgKey;

		if ((ret = rsslEncodeMsg(&eIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					(char*)"rsslEncodeMsg() failed: %d.", ret);
			return ret;
		}

		msgBuf->length = rsslGetEncodedBufferLength(&eIter);
		

		if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
			return ret;

		/* Requests has been made, move the link. */
		pItemRequest->requestState = ITEM_WAITING_FOR_REFRESH;
	   	rsslQueueRemoveFirstLink(pRequestQueue);
		rsslQueueAddLinkToBack(pWaitingForRefreshQueue, pLink);


		countStatIncr(&pConsumerThread->stats.requestCount);
	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendPostBurst(ConsumerThread *pConsumerThread, RsslRDMService *pService, RotatingQueue *pPostItemQueue, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslUInt32 i;
	RsslUInt encodeStartTime;
	RsslInt32 latencyUpdateNumber;

	assert(pService);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(pPostItemQueue))
		return RSSL_RET_SUCCESS;

	latencyUpdateNumber = (consPerfConfig.latencyPostsPerSec > 0 && rotatingQueueGetCount(pPostItemQueue)) ?
		latencyRandomArrayGetNext(pRandomArray, &pConsumerThread->randArrayIter) : -1; 


	for(i = 0; i < itemBurstCount; ++i)
	{
		ItemRequest *pPostItem;
		RsslQueueLink *pLink = rotatingQueueNext(pPostItemQueue);

		pPostItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, postQueueLink, pLink);

		assert(pPostItem->itemInfo.itemFlags & ITEM_IS_POST);

		if (latencyUpdateNumber == i)
			encodeStartTime = getTimeMicro();
		else
			encodeStartTime = 0;


		if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 
						estimateItemPostBufferLength(&pPostItem->itemInfo), RSSL_FALSE, 
						&pConsumerThread->threadRsslError)))
		{
			if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
			{
				countStatAdd(&pConsumerThread->stats.postOutOfBuffersCount, itemBurstCount - i);
				pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return pConsumerThread->threadRsslError.rsslErrorId;
			}
		}


		if ((ret = encodeItemPost(pConsumerThread->pChannel, &pPostItem->itemInfo, msgBuf,
							&postUserInfo, encodeStartTime)) != RSSL_RET_SUCCESS)
		{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"encodeItemPost() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
		}

		if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
			return ret;

		countStatIncr(&pConsumerThread->stats.postSentCount);

	}

	return RSSL_RET_SUCCESS;
}

RsslRet sendGenMsgBurst(ConsumerThread *pConsumerThread, RsslRDMService *pService, RotatingQueue *pGenMsgItemQueue, LatencyRandomArray *pRandomArray, RsslUInt32 itemBurstCount)
{
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslUInt32 i;
	RsslUInt encodeStartTime;
	RsslInt32 latencyGenMsgNumber;

	assert(pService);
	assert(itemBurstCount);

	if (!rotatingQueueGetCount(pGenMsgItemQueue))
		return RSSL_RET_SUCCESS;

	latencyGenMsgNumber = (consPerfConfig.latencyGenMsgsPerSec > 0 && rotatingQueueGetCount(pGenMsgItemQueue)) ?
		latencyRandomArrayGetNext(pRandomArray, &pConsumerThread->randArrayIter) : -1; 


	for(i = 0; i < itemBurstCount; ++i)
	{
		ItemRequest *pGenMsgItem;
		RsslQueueLink *pLink = rotatingQueueNext(pGenMsgItemQueue);

		pGenMsgItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemRequest, genMsgQueueLink, pLink);

		assert(pGenMsgItem->itemInfo.itemFlags & ITEM_IS_GEN_MSG);

		if (latencyGenMsgNumber == i)
		{
			countStatIncr(&pConsumerThread->stats.latencyGenMsgSentCount);
			encodeStartTime = getTimeMicro();
		}
		else
			encodeStartTime = 0;


		if (!(msgBuf = rsslGetBuffer(pConsumerThread->pChannel, 
						estimateItemGenMsgBufferLength(&pGenMsgItem->itemInfo), RSSL_FALSE, 
						&pConsumerThread->threadRsslError)))
		{
			if (pConsumerThread->threadRsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
			{
				countStatAdd(&pConsumerThread->stats.genMsgOutOfBuffersCount, itemBurstCount - i);
				pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return pConsumerThread->threadRsslError.rsslErrorId;
			}
		}


		if ((ret = encodeItemGenMsg(pConsumerThread->pChannel, &pGenMsgItem->itemInfo, msgBuf, encodeStartTime)) != RSSL_RET_SUCCESS)
		{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"encodeItemPost() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
		}

		if ( sendMessage(pConsumerThread, msgBuf) != RSSL_RET_SUCCESS)
			return ret;

		countStatIncr(&pConsumerThread->stats.genMsgSentCount);
		if (!pConsumerThread->stats.firstGenMsgSentTime)
			pConsumerThread->stats.firstGenMsgSentTime = getTimeNano();

	}

	return RSSL_RET_SUCCESS;
}

static RsslRet printEstimatedPostMsgSizes(ConsumerThread *pConsumerThread)
{
	MarketPriceItem *mpItem;
	MarketByOrderItem *mboItem;
	RsslBuffer testBuffer;
	RsslRet ret;
	RsslInt32 i;
	RsslMsgKey msgKey;

	ItemInfo *pItemInfo;

	assert(pConsumerThread->pChannel);

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	rsslClearMsgKey(&msgKey);
	msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msgKey.name.data = (char*)"RDT0";
	msgKey.name.length = 4;
	msgKey.serviceId = 0;


	/* Market Price */
	if (xmlMsgDataHasMarketPrice)
	{

		mpItem = createMarketPriceItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_PRICE;
		pItemInfo->itemData = (void*)mpItem;

		if (consPerfConfig.postsPerSec)
		{
			printf("Approximate message sizes:\n");

			for (i = 0; i < getMarketPricePostMsgCount(); ++i)
			{
				testBuffer.length = estimateItemPostBufferLength(pItemInfo);
				testBuffer.data = (char*)malloc(testBuffer.length);
				if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
					return RSSL_RET_FAILURE;
				}
				printf(	"  MarketPrice PostMsg %d: \n"
						"         estimated length: %u bytes\n" 
						"    approx encoded length: %u bytes\n", 
						i+1,
						estimateItemPostBufferLength(pItemInfo),
						testBuffer.length);
				free(testBuffer.data);
			}
			assert(mpItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

			freeMarketPriceItem(mpItem);
		}
	}

	/* Market By Order */

	if (xmlMsgDataHasMarketByOrder)
	{
		mboItem = createMarketByOrderItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		pItemInfo->itemData = (void*)mboItem;

		for (i = 0; i < getMarketByOrderPostMsgCount(); ++i)
		{
			testBuffer.length = estimateItemPostBufferLength(pItemInfo);
			testBuffer.data = (char *)malloc(testBuffer.length);
			if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
				return RSSL_RET_FAILURE;
			}
			printf(	"  MarketByOrder PostMsg %d:\n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1, 
					estimateItemPostBufferLength(pItemInfo),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mboItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketByOrderItem(mboItem);

		printf("\n");

	}

	free(pItemInfo);

	printf("\n");

	return RSSL_RET_SUCCESS;
}

static RsslRet printEstimatedGenMsgSizes(ConsumerThread *pConsumerThread)
{
	MarketPriceItem *mpItem;
	MarketByOrderItem *mboItem;
	RsslBuffer testBuffer;
	RsslRet ret;
	RsslInt32 i;
	RsslMsgKey msgKey;

	ItemInfo *pItemInfo;

	assert(pConsumerThread->pChannel);

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	rsslClearMsgKey(&msgKey);
	msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msgKey.name.data = (char*)"RDT0";
	msgKey.name.length = 4;
	msgKey.serviceId = 0;


	/* Market Price */
	if (xmlMsgDataHasMarketPrice)
	{

		mpItem = createMarketPriceItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_PRICE;
		pItemInfo->itemData = (void*)mpItem;

		if (consPerfConfig.genMsgsPerSec)
		{
			printf("Approximate message sizes:\n");

			for (i = 0; i < getMarketPriceGenMsgCount(); ++i)
			{
				testBuffer.length = estimateItemGenMsgBufferLength(pItemInfo);
				testBuffer.data = (char*)malloc(testBuffer.length);
				if ((ret = encodeItemGenMsg(pConsumerThread->pChannel, pItemInfo, &testBuffer, 0)) < RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							(char*)"printEstimatedGenMsgSizes: encodeItemGenMsg() failed: %s", rsslRetCodeToString(ret));
					return RSSL_RET_FAILURE;
				}
				printf(	"  MarketPrice GenMsg %d: \n"
						"         estimated length: %u bytes\n" 
						"    approx encoded length: %u bytes\n", 
						i+1,
						estimateItemGenMsgBufferLength(pItemInfo),
						testBuffer.length);
				free(testBuffer.data);
			}
			assert(mpItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

			freeMarketPriceItem(mpItem);
		}
	}

	/* Market By Order */

	if (xmlMsgDataHasMarketByOrder)
	{
		mboItem = createMarketByOrderItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		pItemInfo->itemData = (void*)mboItem;

		for (i = 0; i < getMarketByOrderPostMsgCount(); ++i)
		{
			testBuffer.length = estimateItemPostBufferLength(pItemInfo);
			testBuffer.data = (char *)malloc(testBuffer.length);
			if ((ret = encodeItemPost(pConsumerThread->pChannel, pItemInfo, &testBuffer, &postUserInfo, 0)) < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"printEstimatedPostMsgSizes: encodeItemPost() failed: %s", rsslRetCodeToString(ret));
				return RSSL_RET_FAILURE;
			}
			printf(	"  MarketByOrder PostMsg %d:\n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1, 
					estimateItemPostBufferLength(pItemInfo),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mboItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketByOrderItem(mboItem);

		printf("\n");

	}

	free(pItemInfo);

	printf("\n");

	return RSSL_RET_SUCCESS;
}

RSSL_THREAD_DECLARE(runConsumerConnection, threadStruct) 
{
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;

	RsslBuffer *msgBuf=0;
	RsslRet	readret;

	RsslMsg msg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;

	ConsumerThread* pConsumerThread = (ConsumerThread*)threadStruct;
	
	RsslConnectOptions copts = RSSL_INIT_CONNECT_OPTS;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	RsslChannelInfo channelInfo;

	int selRet;
	RsslInt32 i;
	RsslUInt32 count;

	RsslQueue requestQueue, waitingForRefreshQueue, refreshCompleteQueue;
	RotatingQueue postItemQueue, genMsgItemQueue;
	
	struct timeval time_interval;
	
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	RsslRet ret = 0;
	RsslError closeError;

	/* Store information about the desired service once we find it. */
	RsslRDMService *pDesiredService = NULL;
	RsslBuffer serviceName;
	RsslBool loggedIn = RSSL_FALSE;

	RsslBool haveMarketPricePostItems = RSSL_FALSE, haveMarketByOrderPostItems = RSSL_FALSE;
	RsslBool haveMarketPriceGenMsgItems = RSSL_FALSE, haveMarketByOrderGenMsgItems = RSSL_FALSE;

	RsslInt32 postItemCount = 0;
	RsslInt32 genMsgItemCount = 0;

	RsslInt64 nsecPerTick;
	TimeValue currentTime, nextTickTime;

	RsslInt32 postsPerTick, postsPerTickRemainder;
	RsslInt32 genMsgsPerTick, genMsgsPerTickRemainder;
	RsslInt32 currentTicks = 0;
	LatencyRandomArray postLatencyRandomArray, genMsgLatencyRandomArray;

	XmlItemInfoList *pXmlItemInfoList;
	RsslInt32 xmlItemListIndex;

#ifdef ENABLE_XML_TRACE
	RsslError error;
	RsslTraceOptions traceOptions;
#endif
	
	nsecPerTick = 1000000000 / consPerfConfig.ticksPerSec;
	postsPerTick = consPerfConfig.postsPerSec / consPerfConfig.ticksPerSec;
	postsPerTickRemainder = consPerfConfig.postsPerSec % consPerfConfig.ticksPerSec;
	genMsgsPerTick = consPerfConfig.genMsgsPerSec / consPerfConfig.ticksPerSec;
	genMsgsPerTickRemainder = consPerfConfig.genMsgsPerSec % consPerfConfig.ticksPerSec;

	if (consPerfConfig.latencyPostsPerSec)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = consPerfConfig.postsPerSec;
		randomArrayOpts.latencyMsgsPerSec = consPerfConfig.latencyPostsPerSec;
		randomArrayOpts.ticksPerSec = consPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(&postLatencyRandomArray, &randomArrayOpts);
	}

	if (consPerfConfig.genMsgsPerSec)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = consPerfConfig.genMsgsPerSec;
		randomArrayOpts.latencyMsgsPerSec = consPerfConfig.latencyGenMsgsPerSec;
		randomArrayOpts.ticksPerSec = consPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(&genMsgLatencyRandomArray, &randomArrayOpts);
	}
	
	if (pConsumerThread->cpuId >= 0)
	{
		if (bindThread(pConsumerThread->cpuId) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %d.\n", pConsumerThread->cpuId);
			exit(-1);
		}
	}

	serviceName.data = consPerfConfig.serviceName;
	serviceName.length = (RsslUInt32)strlen(serviceName.data);

	rsslInitQueue(&requestQueue);
	rsslInitQueue(&waitingForRefreshQueue);
	rsslInitQueue(&refreshCompleteQueue);
	initRotatingQueue(&postItemQueue);
	initRotatingQueue(&genMsgItemQueue);

	/* Get names of other items from file. */
	if (!(pXmlItemInfoList = createXmlItemList(consPerfConfig.itemFilename, consPerfConfig.itemRequestCount)))
	{
		printf("Failed to load item list from file '%s'.\n", consPerfConfig.itemFilename);
		exit(-1);
	}

	pConsumerThread->itemRequestList = 
		(ItemRequest*)malloc(sizeof(ItemRequest)*pConsumerThread->itemListCount);

	/* Shift item info list pointer so we can use streamID's for direct lookup. */
	pConsumerThread->itemRequestList -= ITEM_STREAM_ID_START;


	/* Should be able to store any item name that the XML parser can store. */
	assert(sizeof(pConsumerThread->itemRequestList[ITEM_STREAM_ID_START].itemNameChar) >= sizeof(pXmlItemInfoList->itemInfoList[0].name));

	if (pXmlItemInfoList->itemInfoCount < consPerfConfig.itemRequestCount)
	{
		printf("Error: Item file contained %d items, but consumer wants %d items.\n", pXmlItemInfoList->itemInfoCount, 
				consPerfConfig.itemRequestCount);
		exit(-1);
	}

	if (consPerfConfig.postsPerSec && pXmlItemInfoList->postItemCount == 0)
	{
		printf("Error: Configured for posting but no posting items found in item file.\n");
		exit(-1);
	}
	if (consPerfConfig.genMsgsPerSec && pXmlItemInfoList->genMsgItemCount == 0)
	{
		printf("Error: Configured for sending generic messages but no generic message items found in item file.\n");
		exit(-1);
	}

	/* Copy item information from the XML list. */
	xmlItemListIndex = 0;
	for(i = 0; i < pConsumerThread->itemListCount; ++i)
	{
		RsslInt32 streamId = i + ITEM_STREAM_ID_START;

		/* If there are multiple consumer connections, each consumer must build its
		 * list of items from the items that are common to all consumers (if any)
		 * and those unique to that consumer. */

		/* Once we have filled our list with the common items,
		 * start using the range of items unique to this consumer thread. */
		if (xmlItemListIndex == consPerfConfig.commonItemCount
				&& xmlItemListIndex < pConsumerThread->itemListUniqueIndex)
			xmlItemListIndex = pConsumerThread->itemListUniqueIndex;

		clearItemRequest(&pConsumerThread->itemRequestList[streamId]);
		pConsumerThread->itemRequestList[streamId].itemInfo.StreamId = streamId;
		pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType = pXmlItemInfoList->itemInfoList[xmlItemListIndex].domainType;

		strncpy(pConsumerThread->itemRequestList[streamId].itemNameChar, pXmlItemInfoList->itemInfoList[xmlItemListIndex].name, pXmlItemInfoList->itemInfoList[xmlItemListIndex].nameLength);
		pConsumerThread->itemRequestList[streamId].msgKey.flags = RSSL_MKF_HAS_NAME;
		pConsumerThread->itemRequestList[streamId].msgKey.name.length = pXmlItemInfoList->itemInfoList[xmlItemListIndex].nameLength;
		pConsumerThread->itemRequestList[streamId].msgKey.name.data = pConsumerThread->itemRequestList[streamId].itemNameChar;
		pConsumerThread->itemRequestList[streamId].itemInfo.attributes.pMsgKey = &pConsumerThread->itemRequestList[streamId].msgKey;

		rsslQueueAddLinkToBack(&requestQueue, &pConsumerThread->itemRequestList[streamId].link);

		if (!pXmlItemInfoList->itemInfoList[xmlItemListIndex].isSnapshot)
		{
			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_STREAMING_REQ;
		}

		if (pXmlItemInfoList->itemInfoList[xmlItemListIndex].isPost && consPerfConfig.postsPerSec)
		{
			void *pItemData;

			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_POST;

			/* Get item info data from appropriate pool */
			switch(pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pItemData = (void*)createMarketPriceItem();
					haveMarketPricePostItems = RSSL_TRUE;
					break;
				case RSSL_DMT_MARKET_BY_ORDER:
					pItemData = (void*)createMarketByOrderItem();
					haveMarketByOrderPostItems = RSSL_TRUE;
					break;
				default:
					pItemData = 0;
					break;
			}

			pConsumerThread->itemRequestList[streamId].itemInfo.itemData = pItemData;
		}

		if (consPerfConfig.postsPerSec > 0)
		{
			if (haveMarketPricePostItems && (!xmlMsgDataHasMarketPrice || xmlMarketPriceMsgs.postMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketPrice posting data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}
			else if (haveMarketByOrderPostItems && (!xmlMsgDataHasMarketByOrder || xmlMarketByOrderMsgs.postMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketByOrder posting data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}
		}

		if (pXmlItemInfoList->itemInfoList[xmlItemListIndex].isGenMsg && consPerfConfig.genMsgsPerSec)
		{
			void *pItemData;

			pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags |= ITEM_IS_GEN_MSG;

			/* Get item info data from appropriate pool */
			switch(pConsumerThread->itemRequestList[streamId].itemInfo.attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pItemData = (void*)createMarketPriceItem();
					haveMarketPriceGenMsgItems = RSSL_TRUE;
					break;
				case RSSL_DMT_MARKET_BY_ORDER:
					pItemData = (void*)createMarketByOrderItem();
					haveMarketByOrderGenMsgItems = RSSL_TRUE;
					break;
				default:
					pItemData = 0;
					break;
			}

			pConsumerThread->itemRequestList[streamId].itemInfo.itemData = pItemData;
		}

		if (consPerfConfig.genMsgsPerSec > 0)
		{
			if (haveMarketPriceGenMsgItems && (!xmlMsgDataHasMarketPrice || xmlMarketPriceMsgs.genMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketPrice generic message data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}
			else if (haveMarketByOrderGenMsgItems && (!xmlMsgDataHasMarketByOrder || xmlMarketByOrderMsgs.genMsgCount == 0))
			{
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Error: No MarketByOrder generic message data in file: %s", consPerfConfig.msgFilename);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}
		}

		++xmlItemListIndex;
	}

	/* Did the index stop where we expected it to? */
	assert(
			/* No unique items */
			pConsumerThread->itemListCount == consPerfConfig.commonItemCount && 
			xmlItemListIndex == consPerfConfig.commonItemCount
			||
			xmlItemListIndex == pConsumerThread->itemListUniqueIndex 
			+ pConsumerThread->itemListCount - consPerfConfig.commonItemCount);

	destroyXmlItemList(pXmlItemInfoList);

	/* Load dictionary from file if possible. */
	if (rsslLoadFieldDictionary("RDMFieldDictionary", pConsumerThread->pDictionary, &errorText) < 0)
	{
		printf("Unable to load field dictionary: %s.\n"
				"Will request dictionaries from provider.\n\n" , errorText.data);
	}
	else if (rsslLoadEnumTypeDictionary("enumtype.def", pConsumerThread->pDictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s\n", errorText.data);
		printf("Unable to load enum type dictionary: %s.\n\n"
				"Will request dictionaries from provider.\n" , errorText.data);
	}
	else
	{
		/* Dictionary successfully loaded. */
		pConsumerThread->dictionaryStateFlags =
			(DictionaryStateFlags)(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT);
	}
	
	FD_ZERO(&(pConsumerThread->readfds));
	FD_ZERO(&(pConsumerThread->exceptfds));
	FD_ZERO(&(pConsumerThread->wrtfds));
	
	/* Connect to RSSL server */
	if(strlen(consPerfConfig.interfaceName)) copts.connectionInfo.unified.interfaceName = consPerfConfig.interfaceName;

	copts.guaranteedOutputBuffers = consPerfConfig.guaranteedOutputBuffers;
	copts.numInputBuffers = consPerfConfig.numInputBuffers;
	copts.sysSendBufSize = consPerfConfig.sendBufSize;
	copts.sysRecvBufSize = consPerfConfig.recvBufSize;
	copts.connectionInfo.unified.address = consPerfConfig.hostName;
	copts.connectionInfo.unified.serviceName = consPerfConfig.portNo;
	copts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	copts.minorVersion = RSSL_RWF_MINOR_VERSION;
	copts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	copts.connectionType = consPerfConfig.connectionType;

	if(consPerfConfig.connectionType == RSSL_CONN_TYPE_SOCKET)
	{
		copts.tcpOpts.tcp_nodelay = consPerfConfig.tcpNoDelay;
	}

		copts.connectionInfo.unified.address = consPerfConfig.hostName;
		copts.connectionInfo.unified.serviceName = consPerfConfig.portNo;

	do
	{
		RsslError error;
		RsslChannel *pChannel;

		if (shutdownThreads)
			return 0;

			printf("\nAttempting to connect to server %s:%s...\n", consPerfConfig.hostName, consPerfConfig.portNo);

		if (!CHECK_RSSLERROR((pChannel = rsslConnect(&copts,&error)), error))
		{
			SLEEP(1);
			continue;
		}

		FD_SET(pChannel->socketId,&(pConsumerThread->readfds));
		FD_SET(pChannel->socketId,&(pConsumerThread->exceptfds));

		assert(pChannel->state == RSSL_CH_STATE_INITIALIZING || pChannel->state == RSSL_CH_STATE_ACTIVE);

		/* Wait for channel to become active.  This finalizes the three-way handshake. */
		while(pChannel->state == RSSL_CH_STATE_INITIALIZING)
		{
			if (shutdownThreads)
				return 0;

			ret = rsslInitChannel(pChannel, &inProg, &error);

			switch (ret)
			{
				case RSSL_RET_CHAN_INIT_IN_PROGRESS:
					if (inProg.flags & RSSL_IP_FD_CHANGE)
					{
						FD_CLR(inProg.oldSocket,&pConsumerThread->readfds);
						FD_CLR(inProg.oldSocket,&pConsumerThread->exceptfds);
						FD_SET(pChannel->socketId,&pConsumerThread->readfds);
						FD_SET(pChannel->socketId,&pConsumerThread->exceptfds);
					}
					break;
				case RSSL_RET_SUCCESS:
					/* Channel is now active. */
					assert(pChannel->state == RSSL_CH_STATE_ACTIVE);
					break;
				default:
					printf("rsslInitChannel() failed: %s(%s)\n", rsslRetCodeToString(ret),
							error.text);
					FD_CLR(pChannel->socketId,&pConsumerThread->readfds);
					FD_CLR(pChannel->socketId,&pConsumerThread->exceptfds);
					break;
			}

			if (ret < RSSL_RET_SUCCESS)
				break;
		}

		if (pChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			/* Channel failed. Close and retry. */
			rsslCloseChannel(pChannel, &closeError);
			SLEEP(1);
			continue;
		}

		pConsumerThread->pChannel = pChannel;

	} while (!pConsumerThread->pChannel);

	consumerThreadInitPings(pConsumerThread);

	printEstimatedPostMsgSizes(pConsumerThread);
	printEstimatedGenMsgSizes(pConsumerThread);

#ifdef ENABLE_XML_TRACE
	rsslClearTraceOptions(&traceOptions);
	traceOptions.traceMsgFileName = "upacConsPerf";
	traceOptions.traceMsgMaxFileSize = 1000000000;
	traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
	rsslIoctl(pConsumerThread->pChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &error);
#endif

	if (consPerfConfig.highWaterMark > 0)
	{
		if (rsslIoctl(pConsumerThread->pChannel, RSSL_HIGH_WATER_MARK, &consPerfConfig.highWaterMark, &pConsumerThread->threadRsslError) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			shutdownThreads = RSSL_TRUE;
			return RSSL_THREAD_RETURN();
		}
	}

	if ((ret = rsslGetChannelInfo(pConsumerThread->pChannel, &channelInfo, &pConsumerThread->threadRsslError)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_THREAD_RETURN();
	} 

	printf( "Channel %d active. Channel Info:\n"
			"  maxFragmentSize: %u\n"
			"  maxOutputBuffers: %u\n"
			"  guaranteedOutputBuffers: %u\n"
			"  numInputBuffers: %u\n"
			"  pingTimeout: %u\n"
			"  clientToServerPings: %s\n"
			"  serverToClientPings: %s\n"
			"  sysSendBufSize: %u\n"
			"  sysSendBufSize: %u\n"			
			"  compressionType: %s\n"
			"  compressionThreshold: %u\n"			
			"  ComponentInfo: ", 
			pConsumerThread->pChannel->socketId,
			channelInfo.maxFragmentSize,
			channelInfo.maxOutputBuffers, channelInfo.guaranteedOutputBuffers,
			channelInfo.numInputBuffers,
			channelInfo.pingTimeout,
			channelInfo.clientToServerPings == RSSL_TRUE ? "true" : "false",
			channelInfo.serverToClientPings == RSSL_TRUE ? "true" : "false",
			channelInfo.sysSendBufSize, channelInfo.sysRecvBufSize,			
			channelInfo.compressionType == RSSL_COMP_ZLIB ? "zlib" : "none",
			channelInfo.compressionThreshold			
			);

	if (channelInfo.componentInfoCount == 0)
		printf("(No component info)");
	else
		for(count = 0; count < channelInfo.componentInfoCount; ++count)
		{
			printf("%.*s", 
					channelInfo.componentInfo[count]->componentVersion.length,
					channelInfo.componentInfo[count]->componentVersion.data);
			if (count < channelInfo.componentInfoCount - 1)
				printf(", ");
		}
	printf ("\n\n");

	if ( (ret = sendLoginRequest(pConsumerThread, LOGIN_STREAM_ID)) != RSSL_RET_SUCCESS)
	{
		rsslCloseChannel(pConsumerThread->pChannel, &closeError);
		shutdownThreads = RSSL_TRUE;
		return RSSL_THREAD_RETURN();
	}

	currentTime = getTimeNano();
	nextTickTime = currentTime + nsecPerTick;
	time_interval.tv_sec = 0;

	while(1)
	{

		if(shutdownThreads == RSSL_TRUE)
		{
			rsslCloseChannel(pConsumerThread->pChannel, &closeError);
			return RSSL_THREAD_RETURN();
		}
				
		useRead = pConsumerThread->readfds;
		useExcept = pConsumerThread->exceptfds;
		useWrt = pConsumerThread->wrtfds;

		currentTime = getTimeNano();
		time_interval.tv_usec = (long)((currentTime > nextTickTime) ? 0 : ((nextTickTime - currentTime)/1000));

		selRet = select(FD_SETSIZE,&useRead,&useWrt,&useExcept,&time_interval);

		if (selRet > 0)
		{
			if (pConsumerThread->pChannel != NULL && FD_ISSET(pConsumerThread->pChannel->socketId, &useRead))
			{
				do{
					if ((msgBuf = rsslRead(pConsumerThread->pChannel,&readret,&pConsumerThread->threadRsslError)) != 0)
					{	
						pConsumerThread->receivedPing = RSSL_TRUE;

						/* clear decode iterator */
						rsslClearDecodeIterator(&dIter);
		
						/* set version info */
						rsslSetDecodeIteratorRWFVersion(&dIter, pConsumerThread->pChannel->majorVersion, pConsumerThread->pChannel->minorVersion);

						rsslSetDecodeIteratorBuffer(&dIter, msgBuf);

						ret = rsslDecodeMsg(&dIter, &msg);	

						if (ret != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									(char*)"rsslDecodeMsg() failed: %d(%s)", ret, rsslRetCodeToString(ret));
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}

						switch(msg.msgBase.domainType)
						{
							case RSSL_DMT_LOGIN:
							{
								char memoryChar[1024];
								RsslBuffer memoryBuffer = { 4000, memoryChar };
								RsslRDMLoginMsg loginMsg;

								if ((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
								{
									rsslCloseChannel(pConsumerThread->pChannel, &closeError);
									shutdownThreads = RSSL_TRUE;
									return RSSL_THREAD_RETURN();
								} 

								switch(loginMsg.rdmMsgBase.rdmMsgType)
								{
									case RDM_LG_MT_REFRESH:

										printf(	"Received login refresh.\n");
										if (loginMsg.refresh.flags &
												RDM_LG_RFF_HAS_APPLICATION_NAME)
											printf( "  ApplicationName: %.*s\n",
												loginMsg.refresh.applicationName.length,
												loginMsg.refresh.applicationName.data);
										printf("\n");

										if(loginMsg.refresh.state.streamState != RSSL_STREAM_OPEN)
										{
											printf("Error: StreamState: %s, Login failed: %.*s\n", rsslStreamStateToString(loginMsg.refresh.state.streamState), 
												loginMsg.refresh.state.text.length, loginMsg.refresh.state.text.data);
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}
										if (consPerfConfig.postsPerSec &&
												(!(loginMsg.refresh.flags & RDM_LG_RFF_HAS_SUPPORT_POST)
												 || !loginMsg.refresh.supportOMMPost))
										{
											rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
													(char*)"Provider for this connection does not support posting.");
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}
										break;
									default:
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
												(char*)"Received unhandled RDMLoginMsgType %d", loginMsg.rdmMsgBase.rdmMsgType);
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
								}

								if (!loggedIn)
								{
									if ( (ret = sendDirectoryRequest(pConsumerThread, DIRECTORY_STREAM_ID)) != RSSL_RET_SUCCESS)
									{
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									}

									loggedIn = RSSL_TRUE;
								}
								break;
							}

							case RSSL_DMT_SOURCE:
							{
								char memoryChar[16384];
								RsslBuffer memoryBuffer = { 16384, memoryChar };
								RsslRDMDirectoryMsg directoryMsg;
								RsslRDMService *pMsgServiceList;
								RsslUInt32 msgServiceCount, iMsgServiceList;
								RsslBool foundServiceName = RSSL_FALSE;

								printf("Received source directory response.\n\n");

								/* Found our service already, ignore the message. */
								if (pDesiredService)
									break;

								if ((ret = rsslDecodeRDMDirectoryMsg(&dIter, &msg, &directoryMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
								{
									rsslCloseChannel(pConsumerThread->pChannel, &closeError);
									shutdownThreads = RSSL_TRUE;
									return RSSL_THREAD_RETURN();
								}

								/* Copy the directory message.  If our desired service is present inside, we will want to keep it. */
								if ((ret = rsslCopyRDMDirectoryMsg(&pConsumerThread->directoryMsgCopy, &directoryMsg, &pConsumerThread->directoryMsgCopyMemory)) != RSSL_RET_SUCCESS)
								{
									rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
											(char*)"rsslCopyRDMDirectoryMsg failed: %d(%s)", ret, rsslRetCodeToString(ret));
									rsslCloseChannel(pConsumerThread->pChannel, &closeError);
									shutdownThreads = RSSL_TRUE;
									return RSSL_THREAD_RETURN();
								}

								switch(pConsumerThread->directoryMsgCopy.rdmMsgBase.rdmMsgType)
								{
									case RDM_DR_MT_REFRESH:
										pMsgServiceList = pConsumerThread->directoryMsgCopy.refresh.serviceList;
										msgServiceCount = pConsumerThread->directoryMsgCopy.refresh.serviceCount;
										break;
									case RDM_DR_MT_UPDATE:
										pMsgServiceList = pConsumerThread->directoryMsgCopy.update.serviceList;
										msgServiceCount = pConsumerThread->directoryMsgCopy.update.serviceCount;
										break;
									default:
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
												(char*)"Error: Received unhandled directory message type %d.", directoryMsg.rdmMsgBase.rdmMsgType);
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();

								}

								/* Search service list for our desired service. */
								for(iMsgServiceList = 0; iMsgServiceList < msgServiceCount; ++iMsgServiceList)
								{
									RsslRDMService *pService = &pMsgServiceList[iMsgServiceList];

									if ( /* Check for matching service name. */
											pService->flags & RDM_SVCF_HAS_INFO
											&& rsslBufferIsEqual(&serviceName, &pService->info.serviceName) )
									{
										foundServiceName = RSSL_TRUE;

										if ( /* Wait for service to come up at least once before we begin using it. */
												pService->flags & RDM_SVCF_HAS_STATE
												&& pService->state.serviceState
												&& (!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) || pService->state.acceptingRequests))
										{
											pDesiredService = pService;

											if (pConsumerThread->dictionaryStateFlags != (DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
												if ( (ret = sendDictionaryRequests(pConsumerThread, (RsslUInt16)pDesiredService->serviceId,
																FIELD_DICTIONARY_STREAM_ID, ENUM_TYPE_DICTIONARY_STREAM_ID)) != RSSL_RET_SUCCESS)
												{
													rsslCloseChannel(pConsumerThread->pChannel, &closeError);
													shutdownThreads = RSSL_TRUE;
													return RSSL_THREAD_RETURN();
												}

											break;
										}
									}

								}

								if (!pDesiredService)
								{
									if (foundServiceName)
										printf("Service %.*s is currently down.\n\n", serviceName.length, serviceName.data);
									else
										printf("Service %.*s not found in message.\n\n", serviceName.length, serviceName.data);
								}
								else
									printf("Service %.*s is up.\n\n", serviceName.length, serviceName.data);

								break;
							}

							case RSSL_DMT_DICTIONARY:
							{
								char memoryChar[1024];
								RsslBuffer memoryBuffer = { 1024, memoryChar };
								RsslRDMDictionaryMsg dictionaryMsg;


								if ((ret = rsslDecodeRDMDictionaryMsg(&dIter, &msg, &dictionaryMsg, &memoryBuffer, &pConsumerThread->threadErrorInfo)) != RSSL_RET_SUCCESS)
								{
									rsslCloseChannel(pConsumerThread->pChannel, &closeError);
									shutdownThreads = RSSL_TRUE;
									return RSSL_THREAD_RETURN();
								}
								
								switch(dictionaryMsg.rdmMsgBase.rdmMsgType)
								{
									case RDM_DC_MT_REFRESH:
										if (dictionaryMsg.rdmMsgBase.streamId == FIELD_DICTIONARY_STREAM_ID)
										{
											if ((ret = rsslDecodeFieldDictionary(&dIter, pConsumerThread->pDictionary, RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
											{
												rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
														(char*)"rsslDecodeFieldDictionary() failed: %.*s", errorText.length, errorText.data);
												rsslCloseChannel(pConsumerThread->pChannel, &closeError);
												shutdownThreads = RSSL_TRUE;
												return RSSL_THREAD_RETURN();
											}

											if (dictionaryMsg.refresh.flags & RDM_DC_RFF_IS_COMPLETE)
											{
												printf("Field dictionary downloaded.\n\n");
												pConsumerThread->dictionaryStateFlags = (DictionaryStateFlags)(pConsumerThread->dictionaryStateFlags | DICTIONARY_STATE_HAVE_FIELD_DICT);
											}

										}
										else if (dictionaryMsg.rdmMsgBase.streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
										{
											if ((ret = rsslDecodeEnumTypeDictionary(&dIter, pConsumerThread->pDictionary, RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
											{
												rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
														(char*)"rsslDecodeEnumTypeDictionary() failed: %.*s", errorText.length, errorText.data);
												rsslCloseChannel(pConsumerThread->pChannel, &closeError);
												shutdownThreads = RSSL_TRUE;
												return RSSL_THREAD_RETURN();
											}

											if (dictionaryMsg.refresh.flags & RDM_DC_RFF_IS_COMPLETE)
											{
												printf("Enumerated Types downloaded.\n\n");
												pConsumerThread->dictionaryStateFlags = (DictionaryStateFlags)(pConsumerThread->dictionaryStateFlags | DICTIONARY_STATE_HAVE_ENUM_DICT);
											}

										}

										if (pConsumerThread->dictionaryStateFlags == (DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
											printf("Dictionary download complete.\n");
													
										break;
									default:
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
												(char*)"Received unhandled dictionary message type %d.", dictionaryMsg.rdmMsgBase.rdmMsgType);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
								}
								break;
							}

							default:
							{
								ItemInfo *pItemInfo;
								{
									if (msg.msgBase.streamId < ITEM_STREAM_ID_START || msg.msgBase.streamId 
											>= ITEM_STREAM_ID_START + consPerfConfig.commonItemCount + pConsumerThread->itemListCount)
									{
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
												(char*)"Error: Received message with unexpected Stream ID: %d  Class: %s(%u) Domain: %s(%u)",
												msg.msgBase.streamId,
												rsslMsgClassToString(msg.msgBase.msgClass), msg.msgBase.msgClass,
												rsslDomainTypeToString(msg.msgBase.domainType), msg.msgBase.domainType);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									}

									pItemInfo = &pConsumerThread->itemRequestList[msg.msgBase.streamId].itemInfo;

									if (pItemInfo->attributes.domainType != msg.msgBase.domainType)
									{
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
												(char*)"Error: Received message with domain type %u vs. expected type %u",
												msg.msgBase.domainType, pItemInfo->attributes.domainType);
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										shutdownThreads = RSSL_TRUE;
										return RSSL_THREAD_RETURN();
									}
								}

								switch(msg.msgBase.msgClass)
								{
									case RSSL_MC_UPDATE:
									{

										countStatIncr(pConsumerThread->stats.imageRetrievalEndTime ?
												&pConsumerThread->stats.steadyStateUpdateCount :
												&pConsumerThread->stats.startupUpdateCount);


										if (!pConsumerThread->stats.firstUpdateTime)
											pConsumerThread->stats.firstUpdateTime = getTimeNano();

										if((ret = decodePayload(&dIter, &msg, pConsumerThread)) 
												!= RSSL_RET_SUCCESS)
										{
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}
										break;
									}

									case RSSL_MC_REFRESH:
									{
										RsslInt32 streamId = msg.msgBase.streamId;

										countStatIncr(&pConsumerThread->stats.refreshCount);

										if((ret = decodePayload(&dIter, &msg, pConsumerThread)) 
												!= RSSL_RET_SUCCESS)
										{
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}

										if(!pConsumerThread->stats.imageRetrievalEndTime && rsslQueueGetElementCount(&waitingForRefreshQueue))
										{
											if (rsslIsFinalState(&msg.refreshMsg.state))
											{
												if (pItemInfo)
												{
													RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
													rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
															(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(msg.refreshMsg.state.streamState),
															pName->length, pName->data);
												}
												else
												{
													if (msg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
														rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
																(char*)"Received unexpected final state %s in refresh for item: %.*s", rsslStreamStateToString(msg.refreshMsg.state.streamState),
																msg.msgBase.msgKey.name.length, msg.msgBase.msgKey.name.data);
													else
														rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
																(char*)"Received unexpected final state %s in refresh for unknown item", rsslStreamStateToString(msg.refreshMsg.state.streamState));
												}
												rsslCloseChannel(pConsumerThread->pChannel, &closeError);
												shutdownThreads = RSSL_TRUE;
												return RSSL_THREAD_RETURN();
											}

											if (msg.refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE
													&& msg.refreshMsg.state.dataState == RSSL_DATA_OK)
											{
												{
													/* Received a complete refresh. */
													if ( pConsumerThread->itemRequestList[streamId].requestState == ITEM_WAITING_FOR_REFRESH)
													{
														pConsumerThread->itemRequestList[streamId].requestState = ITEM_HAS_REFRESH;
														rsslQueueRemoveLink(&waitingForRefreshQueue, &pConsumerThread->itemRequestList[streamId].link);
														rsslQueueAddLinkToBack(&refreshCompleteQueue, &pConsumerThread->itemRequestList[streamId].link);

														if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_STREAMING_REQ)
														{
															if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_POST && consPerfConfig.postsPerSec)
															{
																pConsumerThread->itemRequestList[streamId].itemInfo.myQueue = &postItemQueue;
																rotatingQueueInsert(&postItemQueue, &pConsumerThread->itemRequestList[streamId].postQueueLink);
															}
															if (pConsumerThread->itemRequestList[streamId].itemInfo.itemFlags & ITEM_IS_GEN_MSG && consPerfConfig.genMsgsPerSec)
															{
																pConsumerThread->itemRequestList[streamId].itemInfo.myQueue = &genMsgItemQueue;
																rotatingQueueInsert(&genMsgItemQueue, &pConsumerThread->itemRequestList[streamId].genMsgQueueLink);
															}
														}

														if (rsslQueueGetElementCount(&refreshCompleteQueue) == pConsumerThread->itemListCount)
														{
															pConsumerThread->stats.imageRetrievalEndTime = getTimeNano();
														}
													}
												}
											}
										}
										
										break;
									}

									case RSSL_MC_STATUS:
										countStatIncr(&pConsumerThread->stats.statusCount);

										/* Stop if an item is unexpectedly closed. */
										if ((msg.statusMsg.flags & RSSL_STMF_HAS_STATE)
												&& rsslIsFinalState(&msg.statusMsg.state))
										{
											if (pItemInfo)
											{
												RsslBuffer *pName = &pItemInfo->attributes.pMsgKey->name;
												rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
														(char*)"Received unexpected final state %s for item: %.*s", rsslStreamStateToString(msg.statusMsg.state.streamState),
														pName->length, pName->data);
											}
											else
											{
												if (msg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
													rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
															(char*)"Received unexpected final state %s for item: %.*s", rsslStreamStateToString(msg.statusMsg.state.streamState),
															msg.msgBase.msgKey.name.length, msg.msgBase.msgKey.name.data);
												else
													rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
															(char*)"Received unexpected final state %s for unknown item", rsslStreamStateToString(msg.statusMsg.state.streamState));
											}
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}

										break;
									case RSSL_MC_GENERIC:
										countStatIncr(&pConsumerThread->stats.genMsgRecvCount);
										if (!pConsumerThread->stats.firstGenMsgRecvTime)
											pConsumerThread->stats.firstGenMsgRecvTime = getTimeNano();
										if((ret = decodePayload(&dIter, &msg, pConsumerThread)) 
												!= RSSL_RET_SUCCESS)
										{
											rsslCloseChannel(pConsumerThread->pChannel, &closeError);
											shutdownThreads = RSSL_TRUE;
											return RSSL_THREAD_RETURN();
										}
										break;
									default:
										rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
												(char*)"Received unhandled msg class %u on stream ID %d.", msg.msgBase.msgClass, msg.msgBase.streamId);
										rsslCloseChannel(pConsumerThread->pChannel, &closeError);
										return RSSL_THREAD_RETURN();
								}
								break;
							}
						}

					}
					else
					{
						if (readret < 0)

						switch(readret)
						{
							case RSSL_RET_READ_PING:
								pConsumerThread->receivedPing = RSSL_TRUE;
								break;
							case RSSL_RET_READ_WOULD_BLOCK:
								break;
							case RSSL_RET_READ_FD_CHANGE:
								printf("\nrsslRead() FD Change - Old FD: %d New FD: %d\n", pConsumerThread->pChannel->oldSocketId, pConsumerThread->pChannel->socketId);
								FD_CLR(pConsumerThread->pChannel->oldSocketId, &pConsumerThread->readfds);
								FD_CLR(pConsumerThread->pChannel->oldSocketId, &pConsumerThread->exceptfds);
								FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->readfds);
								FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->exceptfds);
								break;
							default:
								rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
								rsslCloseChannel(pConsumerThread->pChannel, &closeError);
								shutdownThreads = RSSL_TRUE;
								return RSSL_THREAD_RETURN();

						}
					}

					if (shutdownThreads)
					{
						rsslCloseChannel(pConsumerThread->pChannel, &closeError);
						return RSSL_THREAD_RETURN();
					}

				} while(readret > 0);

			}

			if (FD_ISSET(pConsumerThread->pChannel->socketId, &useWrt))
			{
				if (rtrUnlikely((ret = rsslFlush(pConsumerThread->pChannel, &pConsumerThread->threadRsslError)) < RSSL_RET_SUCCESS))
				{
					rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
					rsslCloseChannel(pConsumerThread->pChannel, &closeError);
					shutdownThreads = RSSL_TRUE;
					return RSSL_THREAD_RETURN();
				}

				if (ret == RSSL_RET_SUCCESS)
					FD_CLR(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds);
			}
		}
		else if (selRet == 0)
		{
			if (consumerThreadCheckPings(pConsumerThread))
			{
				rsslCloseChannel(pConsumerThread->pChannel, &closeError);
				shutdownThreads = RSSL_TRUE;
				return RSSL_THREAD_RETURN();
			}

			currentTime = getTimeNano();
			nextTickTime += nsecPerTick;
			
			if (pDesiredService)
			{
				/* Send some item requests(assuming dictionaries are ready). */
				if (rsslQueueGetElementCount(&requestQueue)
						&& pConsumerThread->dictionaryStateFlags == 
						(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT))
				{
					RsslInt32 requestBurstCount;

					requestBurstCount = consPerfConfig._requestsPerTick;
					if (currentTicks > consPerfConfig._requestsPerTickRemainder)
						++requestBurstCount;

					if (!pConsumerThread->stats.imageRetrievalStartTime)
						pConsumerThread->stats.imageRetrievalStartTime = getTimeNano();

					if (sendItemRequestBurst(pConsumerThread, pDesiredService, &requestQueue, requestBurstCount, &waitingForRefreshQueue) < RSSL_RET_SUCCESS)
					{
						rsslCloseChannel(pConsumerThread->pChannel, &closeError);
						shutdownThreads = RSSL_TRUE;
						return RSSL_THREAD_RETURN();
					}
				}

				if (pConsumerThread->stats.imageRetrievalEndTime)
				{
					if (rotatingQueueGetCount(&postItemQueue))
					{
						if (sendPostBurst(pConsumerThread, pDesiredService, &postItemQueue, &postLatencyRandomArray,
									postsPerTick + ((currentTicks < postsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
					if (rotatingQueueGetCount(&genMsgItemQueue))
					{
						if (sendGenMsgBurst(pConsumerThread, pDesiredService, &genMsgItemQueue, &genMsgLatencyRandomArray,
									genMsgsPerTick + ((currentTicks < genMsgsPerTickRemainder ) ? 1 : 0))
								< RSSL_RET_SUCCESS)
						{
							rsslCloseChannel(pConsumerThread->pChannel, &closeError);
							shutdownThreads = RSSL_TRUE;
							return RSSL_THREAD_RETURN();
						}
					}
				}
			}

			if (++currentTicks == consPerfConfig.ticksPerSec)
				currentTicks = 0;

		}
		else
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;
#else
			if (errno == EINTR)
			{
				continue;
			}
#endif

			perror("select");
			exit(-1);
		}


	}
}

void consumerThreadInitPings(ConsumerThread* pConsumerThread)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	/* set time client should receive next message/ping from server */
	pConsumerThread->nextReceivePingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout;
	pConsumerThread->nextSendPingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout/3;
}


RsslBool consumerThreadCheckPings(ConsumerThread* pConsumerThread)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	// handle server pings
	// nextReceivePingTime will be zero until we connect
	if (pConsumerThread->nextReceivePingTime != 0)
	{
		if (currentTime >= pConsumerThread->nextReceivePingTime)
		{
			/* check if client received message from server since last time */
			if (pConsumerThread->receivedPing)
			{
				/* set time client should receive next message/ping from server */
				pConsumerThread->nextReceivePingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout;
				pConsumerThread->receivedPing = RSSL_FALSE;
			}
			else
			{	// timed out
				rsslSetErrorInfo(&pConsumerThread->threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
						(char*)"Ping timeout.");
				return RSSL_TRUE;
			}
		}

		if (currentTime >= pConsumerThread->nextSendPingTime)
		{
			RsslRet ret;

			ret = rsslPing(pConsumerThread->pChannel, &pConsumerThread->threadRsslError);

			if (ret > RSSL_RET_SUCCESS) 
				FD_SET(pConsumerThread->pChannel->socketId, &pConsumerThread->wrtfds); /* Call for flushing if bytes remain */
			else if (ret < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfoLocation(&pConsumerThread->threadErrorInfo, __FILE__, __LINE__);
				return RSSL_TRUE;
			}

			/* set time to send next ping to client */
			pConsumerThread->nextSendPingTime = currentTime + (time_t)pConsumerThread->pChannel->pingTimeout/3;
		}
	}
	return RSSL_FALSE;
}

void consumerThreadInit(ConsumerThread *pConsumerThread, RsslInt32 consThreadId)
{

	char tmpFilename[sizeof(consPerfConfig.statsFilename) + 8];

	timeRecordQueueInit(&pConsumerThread->latencyRecords);
	timeRecordQueueInit(&pConsumerThread->postLatencyRecords);
	timeRecordQueueInit(&pConsumerThread->genMsgLatencyRecords);

	consumerStatsInit(&pConsumerThread->stats);

	pConsumerThread->pChannel = NULL;

	pConsumerThread->cpuId = -1;
	pConsumerThread->latStreamId = 0;
	pConsumerThread->pDictionary = NULL;
	pConsumerThread->dictionaryStateFlags = DICTIONARY_STATE_NONE;

	FD_ZERO(&pConsumerThread->readfds);
	FD_ZERO(&pConsumerThread->exceptfds);
	FD_ZERO(&pConsumerThread->wrtfds);

	pConsumerThread->receivedPing = RSSL_FALSE;
	pConsumerThread->nextSendPingTime = 0;
	pConsumerThread->nextReceivePingTime = 0;
	pConsumerThread->itemListUniqueIndex = 0;
	pConsumerThread->itemListCount = 0;

	latencyRandomArrayIterInit(&pConsumerThread->randArrayIter);

	snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
			consPerfConfig.statsFilename, consThreadId);

	/* Open stats file. */
	if (!(pConsumerThread->statsFile = fopen(tmpFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", tmpFilename);
		exit(-1);
	}

	if (consPerfConfig.logLatencyToFile)
	{
		snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
				consPerfConfig.latencyLogFilename, consThreadId);

		/* Open latency log file. */
		pConsumerThread->latencyLogFile = fopen(tmpFilename, "w");
		if (!pConsumerThread->latencyLogFile)
		{
			printf("Failed to open latency log file: %s\n", tmpFilename);
			exit(-1);
		}

		fprintf(pConsumerThread->latencyLogFile, "Message type, Send time, Receive time, Latency (usec)\n");
	}

	fprintf(pConsumerThread->statsFile, "UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate (msg/sec), Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsgs received, GenMsg Latencies sent, GenMsg latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");

	pConsumerThread->threadRsslError.rsslErrorId = RSSL_RET_SUCCESS;
	pConsumerThread->directoryMsgCopyMemory.data = (char*)malloc(16384);
	pConsumerThread->directoryMsgCopyMemory.length = 16384;
	pConsumerThread->directoryMsgCopyMemoryOrig = pConsumerThread->directoryMsgCopyMemory;
	pConsumerThread->pDictionary = (RsslDataDictionary*)malloc(sizeof(RsslDataDictionary));
	rsslClearDataDictionary(pConsumerThread->pDictionary);
}

void consumerThreadCleanup(ConsumerThread *pConsumerThread)
{
	timeRecordQueueCleanup(&pConsumerThread->latencyRecords);
	timeRecordQueueCleanup(&pConsumerThread->postLatencyRecords);
	timeRecordQueueCleanup(&pConsumerThread->genMsgLatencyRecords);
	if (pConsumerThread->statsFile)
		fclose(pConsumerThread->statsFile);
	if (pConsumerThread->latencyLogFile)
		fclose(pConsumerThread->latencyLogFile);

	free(pConsumerThread->directoryMsgCopyMemoryOrig.data);
	if (pConsumerThread->pDictionary)
	{
		rsslDeleteDataDictionary(pConsumerThread->pDictionary);
		free(pConsumerThread->pDictionary);
	}

	if (pConsumerThread->itemRequestList)
	{
		pConsumerThread->itemRequestList += ITEM_STREAM_ID_START;
		free(pConsumerThread->itemRequestList);
	}
}
