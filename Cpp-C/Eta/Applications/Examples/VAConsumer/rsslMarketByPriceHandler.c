/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/


/*
 * This is the market by price handler for the rsslVAConsumer application.
 * It provides functions for sending the market by price request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * a map containing field list contents and decoding field entries from a 
 * response are also provided.
 */

#include "rsslMarketByPriceHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslVASendMessage.h"
#include "rsslVACacheHandler.h"

#include "rtr/rsslPayloadEntry.h"

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/*
 * Removes an entry from the item name list.
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure containing the Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeMarketByPriceItemEntry(ChannelCommand *pCommand, ItemRequest *pRequest, RsslBool isPrivateStream)
{
	int i;

	if (pRequest->cacheEntryHandle)
	{
		rsslPayloadEntryDestroy(pRequest->cacheEntryHandle);
		pRequest->cacheEntryHandle = 0;
	}

	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		for (i = pRequest->streamId - MARKET_BY_PRICE_STREAM_ID_START; i < pCommand->marketByPriceItemCount - 1; i++)
		{
			strncpy(pCommand->marketByPriceItems[i].itemNameString, pCommand->marketByPriceItems[i+1].itemNameString, 128);
			pCommand->marketByPriceItems[i].itemName = pCommand->marketByPriceItems[i+1].itemName;
			pCommand->marketByPriceItems[i].streamId = pCommand->marketByPriceItems[i+1].streamId;
			pCommand->marketByPriceItems[i].itemState = pCommand->marketByPriceItems[i+1].itemState;
			pCommand->marketByPriceItems[i].cacheEntryHandle = pCommand->marketByPriceItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableMarketByPriceStreamId;
		--pCommand->marketByPriceItemCount;
	}
	else /* private stream */
	{
		for (i = pRequest->streamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START; i < pCommand->privateStreamMarketByPriceItemCount - 1; i++)
		{
			strncpy(pCommand->marketByPricePSItems[i].itemNameString, pCommand->marketByPricePSItems[i+1].itemNameString, 128);
			pCommand->marketByPricePSItems[i].itemName = pCommand->marketByPricePSItems[i+1].itemName;
			pCommand->marketByPricePSItems[i].streamId = pCommand->marketByPricePSItems[i+1].streamId;
			pCommand->marketByPricePSItems[i].itemState = pCommand->marketByPricePSItems[i+1].itemState;
			pCommand->marketByPricePSItems[i].cacheEntryHandle = pCommand->marketByPricePSItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableMarketByPricePrivateStreamId;
		--pCommand->privateStreamMarketByPriceItemCount;
	}
}

/*
 * Encodes the market by price item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the market by price item close
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemClose(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg = RSSL_INIT_CLOSE_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Close an item stream for a specific stream id for Market By Price Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * pRequest - Item request structure containing the stream id of the item close
 */
static RsslRet closeMBPItemStream(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, ItemRequest *pRequest)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item close */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode item close */
		if (encodeItemClose(pReactorChannel, msgBuf, pRequest->streamId) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nMarket By Price encodeItemClose() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Close an item stream for a specific stream id for Market By Price Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * streamId - The stream id of the item close
 */
static RsslRet closeMBPItemByStreamId(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, RsslInt32 streamId)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item close */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode item close */
		if (encodeItemClose(pReactorChannel, msgBuf, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nMarket By Price encodeItemClose() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Publically visable - used to enable snapshot requesting 
 *	Send a set of items as a snapshot request.
 */
void setMBPSnapshotRequest()
{
	snapshotRequest = RSSL_TRUE;
} 

/*
 * Encodes the market by price item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemRequest(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf, ChannelCommand *pCommand, ItemRequest *pRequest, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = pRequest->streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	if (snapshotRequest)
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_PRIORITY;
	}
	else
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;
	}
	if (isPrivateStream)
	{
		msg.flags |= RSSL_RQMF_PRIVATE_STREAM;
	}
	msg.priorityClass = 1;
	msg.priorityCount = 1;
	
	/* copy the QoS information */
	msg.qos = pCommand->qos;

	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.name = pRequest->itemName;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)(pCommand->serviceId);
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the batch item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the batch request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeBatchItemRequest(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf, RsslInt32 streamId, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslElementList eList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry eEntry = RSSL_INIT_ELEMENT_ENTRY;
	RsslArray elementArray = RSSL_INIT_ARRAY; 
	RsslEncodeIterator encodeIter;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	int i;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	if (snapshotRequest)
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_BATCH;
	}
	else
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_BATCH;
	}
	if (isPrivateStream)
	{
		msg.flags |= RSSL_RQMF_PRIVATE_STREAM;
	}
	msg.priorityClass = 1;
	msg.priorityCount = 1;
	
	/* copy the QoS information */
	msg.qos = pCommand->qos;

	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)(pCommand->serviceId);
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() faild with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	/* start the request message encoding */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	/* For Batch requests, the message has a payload of an element list that contains an array of the requested items */

	eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if((ret = rsslEncodeElementListInit(&encodeIter, &eList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}

	eEntry.name = RSSL_ENAME_BATCH_ITEM_LIST;
	eEntry.dataType = RSSL_DT_ARRAY;
	if((ret = rsslEncodeElementEntryInit(&encodeIter, &eEntry, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode the array of requested item names */
	elementArray.primitiveType = RSSL_DT_ASCII_STRING;
	elementArray.itemLength = 0;
	if((ret = rsslEncodeArrayInit(&encodeIter, &elementArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}

	if (!isPrivateStream) /* non-private stream */
	{
		// Start streamID at +1
		for(i = 0; i < pCommand->marketByPriceItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->marketByPriceItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else /* private stream */
	{
		// Start streamID at +1
		for(i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->marketByPricePSItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	if((ret = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMessageComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the batch item close message.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send a batch item close message to
 * msgBuf - The message buffer to encode the batch item close msg into
 * batchCloseStreamId - The stream id of the batch item close message
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeBatchItemClose(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf, RsslInt32 batchCloseStreamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg;
	RsslElementList eList;
	RsslElementEntry eEntry;
	RsslArray elementArray;
	RsslEncodeIterator encodeIter;

	RsslInt itemStreamId;
	int i;

	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	rsslClearCloseMsg(&msg);
	msg.msgBase.msgClass  = RSSL_MC_CLOSE;
	msg.msgBase.streamId = batchCloseStreamId;
	msg.flags = RSSL_CLMF_HAS_BATCH;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	/* start the batch close message encoding */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	/* For Batch close, the message has a payload of an element list that contains an array of streamIDs in which the client registers interest for close */

	rsslClearElementList(&eList);
	eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if((ret = rsslEncodeElementListInit(&encodeIter, &eList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}

	rsslClearElementEntry(&eEntry);
	eEntry.name = RSSL_ENAME_BATCH_STREAMID_LIST;
	eEntry.dataType = RSSL_DT_ARRAY;
	if((ret = rsslEncodeElementEntryInit(&encodeIter, &eEntry, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeelementEntryEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode the array of streamIDs in which the client registers interest for close */
	rsslClearArray(&elementArray);
	elementArray.primitiveType = RSSL_DT_INT;
	elementArray.itemLength = 0; /* Array will have variable length entries */
	if((ret = rsslEncodeArrayInit(&encodeIter, &elementArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode payload with streamId list */
	for(i = 0; i < pCommand->marketByPriceItemCount; i++)
	{
		itemStreamId = i + MARKET_BY_PRICE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->marketByPriceItems[i].itemState))
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, 0, (void*)(&itemStreamId))) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	for(i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
	{
		itemStreamId = i + MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->marketByPricePSItems[i].itemState))
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, 0, (void*)(&itemStreamId))) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	if((ret = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeelementEntryEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMessageComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Publically visable market by price request sending routine 
 *
 * Sends market by price item requests to a channel.  For each item, this
 * consists of getting a message buffer, encoding the item
 * request, and sending the item request to the server.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item request to
 */
RsslRet sendMarketByPriceItemRequests(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);
	int i;

	// Do not send a request if there are no items in the item list.
	if(pCommand->marketByPriceItemCount == 0 && pCommand->privateStreamMarketByPriceItemCount == 0)
		return RSSL_RET_SUCCESS;

	if(getSourceDirectoryCapabilities(pCommand, RSSL_DMT_MARKET_BY_PRICE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_MARKET_BY_PRICE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	/* send out two sets of requests - one for private stream and one for non-private stream */

	/* non-private stream request(s) */
	if (pCommand->marketByPriceItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->marketByPriceItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->marketByPriceBatchRequest.streamId = MARKET_BY_PRICE_BATCH_STREAM_ID_START;
				for (i = 0; i < pCommand->marketByPriceItemCount; i++)
					pCommand->marketByPriceItems[i].streamId = getNextAvailableMarketByPriceStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->marketByPriceBatchRequest.streamId, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nMarket By Price encodeBatchItemRequest() failed\n");
					return RSSL_RET_FAILURE;
				}

				if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
				return RSSL_RET_FAILURE;
			}
		} 
		else 
		{
			if (pCommand->marketByPriceItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests.  Sending Market By Price requests as individual request messages.\n");

			for (i = 0; i < pCommand->marketByPriceItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->marketByPriceItems[i].streamId = getNextAvailableMarketByPriceStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, pCommand, &pCommand->marketByPriceItems[i], RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nMarket By Price encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
					return RSSL_RET_FAILURE;
				}
			}
		}
	}

	/* private stream request(s) */
	if (pCommand->privateStreamMarketByPriceItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->privateStreamMarketByPriceItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->privateStreamMarketByPriceBatchRequest.streamId = MARKET_BY_PRICE_BATCH_PRIVATE_STREAM_ID_START;
				for (i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
					pCommand->marketByPricePSItems[i].streamId = getNextAvailableMarketByPricePrivateStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->privateStreamMarketByPriceBatchRequest.streamId, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nMarket By Price encodeBatchItemRequest() failed\n");
					return RSSL_RET_FAILURE;
				}

				if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
				return RSSL_RET_FAILURE;
			}
		} 
		else 
		{
			if (pCommand->privateStreamMarketByPriceItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests.  Sending Market By Price requests as individual request messages.\n");

			for (i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->marketByPricePSItems[i].streamId = getNextAvailableMarketByPricePrivateStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, pCommand, &pCommand->marketByPricePSItems[i], RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nMarket By Price encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS) 
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
					return RSSL_RET_FAILURE;
				}
			}
		}
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Publically visable market by price response processing function 
 *
 * Processes a market by price response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * field list and field entry, and calling decodeFieldEntry() to decode
 * the field entry data.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - Channel this message was received on.
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processMarketByPriceResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslBuffer mapKey = RSSL_INIT_BUFFER;
	RsslLocalFieldSetDefDb localFieldSetDefDb;  // this must be cleared using the clear function below
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;
	int i;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	ItemRequest *pRequest = NULL;
	RsslBool isBatchRequest = RSSL_FALSE;
	RsslBool isPrivateStream = RSSL_FALSE;

	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	if (msg->msgBase.streamId != 0)
	{
		/* first check if it is a response for non-private stream market by price item request */
		for(i = 0; i < pCommand->marketByPriceItemCount; ++i)
		{
			if (pCommand->marketByPriceItems[i].streamId == msg->msgBase.streamId)
			{
				pRequest = &pCommand->marketByPriceItems[i];
				break;
			}
		}
		/* then check if it is a response for private stream market by price item request */
		if (!pRequest)
		{
			for(i = 0; i < pCommand->privateStreamMarketByPriceItemCount; ++i)
			{
				if (pCommand->marketByPricePSItems[i].streamId == msg->msgBase.streamId)
				{
					pRequest = &pCommand->marketByPricePSItems[i];
					isPrivateStream = RSSL_TRUE;
					break;
				}
			}
		}
		/* then check if it is a response for market By price batch request */
		if (!pRequest)
		{
			if (msg->msgBase.streamId == pCommand->marketByPriceBatchRequest.streamId)
			{
				pRequest = &pCommand->marketByPriceBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if (msg->msgBase.streamId == pCommand->privateStreamMarketByPriceBatchRequest.streamId)
			{
				pRequest = &pCommand->privateStreamMarketByPriceBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if(pCommand->shouldOffStreamPost != RSSL_TRUE)
			{
				printf("Received market by price message on unknown stream %d\n", msg->msgBase.streamId);
				return RSSL_RET_SUCCESS;
			}
		}
	}
	else
	{
		// figure out if its an item we requested
        if (key && (key->flags & RSSL_MKF_HAS_NAME))
        {
            RsslInt32 i;

			for(i = 0; i < pCommand->marketByPriceItemCount; ++i)
			{
                if (pCommand->marketByPriceItems[i].itemName.length == key->name.length &&
                    memcmp(pCommand->marketByPriceItems[i].itemName.data, key->name.data, key->name.length) == 0)
				{
					pRequest = &pCommand->marketByPriceItems[i];
					break;
				}
			}
        }
        if (pRequest == NULL)
            return RSSL_RET_SUCCESS; // ignore items that we didnt request

	}
	rsslClearLocalFieldSetDefDb(&localFieldSetDefDb);

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			/* update our item state list if its a refresh, then process just like update */

			if (!(msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (msg->msgBase.streamId >= MARKET_BY_PRICE_PRIVATE_STREAM_ID_START)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", msg->msgBase.streamId);
					/* close stream */
					if (closeMBPItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketByPriceItemEntry(pCommand, pRequest, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			pRequest->itemState.dataState = msg->refreshMsg.state.dataState;
			pRequest->itemState.streamState = msg->refreshMsg.state.streamState;

			setGroupId(pCommand, pRequest, &msg->refreshMsg.groupId);
			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:
			/* decode market by price response */

			if (pCommand->cacheInfo.useCache)
			{
				applyMsgToCache(&pRequest->cacheEntryHandle, &pCommand->cacheInfo, msg, dIter);
			}

			/* Print descriptor of the channel this message was received from. */
			printf("\n(Channel "SOCKET_PRINT_TYPE"): ", pReactorChannel->socketId);

			/* print out item name from key if it has it */
			if (key && (key->flags & RSSL_MKF_HAS_NAME))
			{
				printf("\n%.*s", key->name.length, key->name.data);
				/* copy name back to item list */
				if (key->name.length > sizeof(pRequest->itemNameString))
				{
					printf("Unexpected name received by response: %.*s\n", key->name.length, key->name.data);
					return RSSL_RET_FAILURE;
				}
				pRequest->itemName.data = pRequest->itemNameString;
				pRequest->itemName.length = key->name.length;
				snprintf(pRequest->itemName.data, sizeof(pRequest->itemNameString), "%.*s", key->name.length, key->name.data);
			}
			else
			{
				printf("\n%.*s", pRequest->itemName.length, pRequest->itemName.data);
			}

			if (isPrivateStream) printf(" (PRIVATE STREAM)");
            printf("\nDOMAIN: %s\n", rsslDomainTypeToString(msg->msgBase.domainType));

			if (msg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				/* When displaying update information, we should also display the updateType information. */
				printf("UPDATE TYPE: %u\n", msg->updateMsg.updateType); 
			}

			if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);
			}
		
			if (pCommand->cacheInfo.useCache == RSSL_FALSE)
			{
				ret = decodeMarketMap(&((ChannelCommand*)pReactorChannel->userSpecPtr)->dictionary, RSSL_DMT_MARKET_BY_PRICE, dIter);
				if (ret != RSSL_RET_SUCCESS)
					return ret;
			}
			else
				printf("Payload cached\n");

			break;

		case RSSL_MC_STATUS:
			if (isBatchRequest)
				printf("\nReceived Batch StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);
			else
				printf("\nReceived Item StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);

			if (!(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", pRequest->streamId);
					/* close stream */
					if (closeMBPItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketByPriceItemEntry(pCommand, pRequest, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			rsslStateToString(&tempBuffer, &msg->statusMsg.state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

				/* update our state table for posting */
				pRequest->itemState.dataState = msg->refreshMsg.state.dataState;
				pRequest->itemState.streamState = msg->refreshMsg.state.streamState;
    		}

			/* redirect to private stream if indicated */
			if (msg->statusMsg.state.streamState == RSSL_STREAM_REDIRECTED && msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)
			{
				if (redirectToPrivateStream(pReactor, pCommand, pRequest) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}

			if (msg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
			{
				setGroupId(pCommand, pRequest, &msg->statusMsg.groupId);
			}
			break;

		case RSSL_MC_ACK:
			/* although this application only posts on MP (Market Price), 
			   ACK handler is provided for other domains to allow user to extend 
			   and post on MBO (Market By Order), MBP (Market By Price), SymbolList, and Yield Curve domains */
			printf("\nReceived AckMsg for stream %i \n", msg->msgBase.streamId);

           if (key && (key->flags & RSSL_MKF_HAS_NAME))
                printf("\n%.*s", key->name.length, key->name.data);
            else if (pRequest)
				printf("\n%.*s", pRequest->itemName.length, pRequest->itemName.data);

            if (isPrivateStream) printf(" (PRIVATE STREAM)");
            printf("\nDOMAIN: %s\n", rsslDomainTypeToString(msg->msgBase.domainType));

			printf("\tackId=%u\n", msg->ackMsg.ackId);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				printf("\tseqNum=%u\n", msg->ackMsg.seqNum);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
				printf("\tnakCode=%u\n", msg->ackMsg.nakCode);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				printf("\ttext=%.*s\n", msg->ackMsg.text.length, msg->ackMsg.text.data);
		
			break;

		default:
			printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
    		break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Publically visable - used to close all market by price streams
 *
 * Close all item streams.
 * pReactor - RsslReactor associated with the application
 * pCommand - the ChannelCommand to send an item close to
 */
RsslRet closeMarketByPriceItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslInt32 batchCloseStreamId;
	int i;

	if (pCommand->reactorChannelClosed)
		return RSSL_RET_SUCCESS;

	RsslReactorChannel *pReactorChannel = pCommand->reactorChannel;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

	if (!pReactorChannel)
		return RSSL_RET_SUCCESS;

	/* If provider supports batch close, will use batch close */
	if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_CLOSES) && (pCommand->marketByPriceItemCount + pCommand->privateStreamMarketByPriceItemCount) > 1)
	{
		/* get a buffer for the item close */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			batchCloseStreamId = getNextAvailableMarketByPriceStreamId(pCommand);

			/* encode batch close */
			if (encodeBatchItemClose(pReactorChannel, msgBuf, batchCloseStreamId) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nMarket By Price encodeBatchItemClose() failed\n");
				return RSSL_RET_FAILURE;
			}

			/* send batch close */
			if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		if ((pCommand->marketByPriceItemCount + pCommand->privateStreamMarketByPriceItemCount) > 1)
			printf("\nConnected Provider does not support Batch Closes. Sending Market By Price closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < pCommand->marketByPriceItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->marketByPriceItems[i].itemState))
			{
				if (closeMBPItemByStreamId(pReactor, pReactorChannel, pCommand, i + MARKET_BY_PRICE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->marketByPricePSItems[i].itemState))
			{
				if (closeMBPItemByStreamId(pReactor, pReactorChannel, pCommand, i + MARKET_BY_PRICE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Redirect a request to a private stream.
 * pReactor - RsslReactor associated with the application
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure
 */
static RsslRet redirectToPrivateStream(RsslReactor *pReactor, ChannelCommand *pCommand, ItemRequest *pRequest)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	ItemRequest *pItemRequest;

	/* add item name to private stream list */
	if (pCommand->privateStreamMarketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
	{
		pItemRequest = &pCommand->marketByPricePSItems[pCommand->privateStreamMarketByPriceItemCount];
		++pCommand->privateStreamMarketByPriceItemCount;
	}
	else
	{
		printf("Number of Private Stream Market By Price items exceeded\n");
	}

	pItemRequest->streamId = getNextAvailableMarketByPricePrivateStreamId(pCommand);

	// save the itemName for redirecting to Private Stream 
	snprintf(pItemRequest->itemNameString, 128, "%s", pRequest->itemNameString);
	pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
	pItemRequest->itemName.data = pItemRequest->itemNameString; 

	/* remove non-private stream entry from list */
	removeMarketByPriceItemEntry(pCommand, pRequest, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslReactorGetBuffer(pCommand->reactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode item request */
        if (encodeItemRequest(pCommand->reactorChannel, msgBuf, pCommand, pItemRequest, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pCommand->reactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nMarket By Price encodeItemRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send item request */
		if (sendMessage(pReactor, pCommand->reactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

