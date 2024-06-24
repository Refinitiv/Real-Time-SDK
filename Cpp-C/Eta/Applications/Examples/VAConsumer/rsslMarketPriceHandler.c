/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/*
 * This is the market price handler for the rsslVAConsumer application.
 * It provides functions for sending the market price request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * a field entry from a response are also provided.
 */

#include "rsslMarketPriceHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslVASendMessage.h"
#include "rsslVACacheHandler.h"

#include "rtr/rsslPayloadEntry.h"

/* Send a view request to the provider */
static RsslBool viewRequest = RSSL_FALSE;

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/*
 * Removes an entry from the item name list.
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure containing the Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeMarketPriceItemEntry(ChannelCommand *pCommand, ItemRequest *pRequest, RsslBool isPrivateStream)
{
	int i;

	if (pRequest->cacheEntryHandle)
	{
		rsslPayloadEntryDestroy(pRequest->cacheEntryHandle);
	}

	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{

		for (i = pRequest->streamId - MARKETPRICE_STREAM_ID_START; i < pCommand->marketPriceItemCount - 1; i++)
		{
			strncpy(pCommand->marketPriceItems[i].itemNameString, pCommand->marketPriceItems[i+1].itemNameString, 128);
			pCommand->marketPriceItems[i].itemName = pCommand->marketPriceItems[i+1].itemName;
			pCommand->marketPriceItems[i].streamId = pCommand->marketPriceItems[i+1].streamId;
			pCommand->marketPriceItems[i].itemState = pCommand->marketPriceItems[i+1].itemState;
			pCommand->marketPriceItems[i].cacheEntryHandle = pCommand->marketPriceItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableMarketPriceStreamId;
		--pCommand->marketPriceItemCount;
	}
	else /* private stream */
	{
		for (i = pRequest->streamId - MARKETPRICE_PRIVATE_STREAM_ID_START; i < pCommand->privateStreamMarketPriceItemCount - 1; i++)
		{
			strncpy(pCommand->marketPricePSItems[i].itemNameString, pCommand->marketPricePSItems[i+1].itemNameString, 128);
			pCommand->marketPricePSItems[i].itemName = pCommand->marketPricePSItems[i+1].itemName;
			pCommand->marketPricePSItems[i].streamId = pCommand->marketPricePSItems[i+1].streamId;
			pCommand->marketPricePSItems[i].itemState = pCommand->marketPricePSItems[i+1].itemState;
			pCommand->marketPricePSItems[i].cacheEntryHandle = pCommand->marketPricePSItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableMarketPricePrivateStreamId;
		--pCommand->privateStreamMarketPriceItemCount;
	}
}

/*
 * Encodes the market price item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the market price item close
 *
 * This function is only used within the Market Price Handler
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
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
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
 * Close an item stream for a specific stream id for Market Price Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * pRequest - Item request structure containing the stream id of the item close
 */
static RsslRet closeMPItemStream(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, ItemRequest *pRequest)
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
			printf("\nMarket Price encodeItemClose() failed\n");
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
 * Close an item stream for a specific stream id for Market Price Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * streamId - The stream id of the item close
 */
static RsslRet closeMPItemByStreamId(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, RsslInt32 streamId)
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
			printf("\nMarket Price encodeItemClose() failed\n");
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
 * Publically visable - used to enable view requesting 
 *	Send a set of items as a view request.
 */
void setViewRequest()
{
	viewRequest = RSSL_TRUE;
}

/*
 * Publically visable - used to enable snapshot requesting 
 *	Send a set of items as a snapshot request.
 */
void setMPSnapshotRequest()
{
	snapshotRequest = RSSL_TRUE;
}  

/*
 * Encodes the View Element Entry.  This entry contains an array of FIDs
 * that the consumer wishes to receive from the provider.
 *
 * This function is only used within the Market Price Handler
 */
static RsslRet encodeViewElementRequest(RsslEncodeIterator* encodeIter)
{
	RsslRet ret = 0;
	RsslElementEntry eEntry = RSSL_INIT_ELEMENT_ENTRY;
	RsslArray viewArray = RSSL_INIT_ARRAY;
	/* These values are taken from the RDMFieldDictionary.
	 * 22 = BID
	 * 25 = ASK
	 * 30 = BIDSIZE
	 * 31 = ASKSIZE
	 * 1025 = QUOTIM
	 */
	RsslUInt viewList[5] = {22, 25, 30, 31, 1025};
	RsslUInt fdList;
	int viewListCount = 5;
	int i;

	eEntry.name = RSSL_ENAME_VIEW_TYPE;
	eEntry.dataType = RSSL_DT_UINT;
	fdList = RDM_VIEW_TYPE_FIELD_ID_LIST;
	if((ret = rsslEncodeElementEntry(encodeIter, &eEntry, &fdList)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	rsslClearElementEntry(&eEntry);
	eEntry.name = RSSL_ENAME_VIEW_DATA;
	eEntry.dataType = RSSL_DT_ARRAY;
	if((ret = rsslEncodeElementEntryInit(encodeIter, &eEntry, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryInit() failed with return code: %d\n", ret);
		return ret;
	}

	viewArray.primitiveType = RSSL_DT_UINT;
	viewArray.itemLength = 2;

	if((ret = rsslEncodeArrayInit(encodeIter, &viewArray)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
		return ret;
	}

	for(i = 0; i < viewListCount; i++)
	{
		if((ret = rsslEncodeArrayEntry(encodeIter, 0, &viewList[i])) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	if ((ret = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

 /* complete encoding of complex element entry.  */
	 if ((ret = rsslEncodeElementEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	 return RSSL_RET_SUCCESS;
}



/*
 * Encodes the item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * pRequest - Item request structure
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemRequest(RsslReactorChannel* pReactorChannel, RsslBuffer *msgBuf, ItemRequest *pRequest, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;
	RsslElementList eList = RSSL_INIT_ELEMENT_LIST;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = pRequest->streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	if((viewRequest == RSSL_TRUE) && (loginInfo->supportViewRequests == RSSL_TRUE))
		msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	else
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
	if((loginInfo->supportViewRequests == RSSL_TRUE)  && (viewRequest == RSSL_TRUE))
		msg.flags |= RSSL_RQMF_HAS_VIEW;
	msg.priorityClass = 1;
	msg.priorityCount = 1;

	/* copy the QoS information */
	msg.qos = pCommand->qos;
	
	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.name = pRequest->itemName;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)pCommand->serviceId;
	
	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	if(viewRequest == RSSL_TRUE) 
	{
		if (loginInfo->supportViewRequests == RSSL_TRUE)
		{
			eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
			if ((ret = rsslEncodeElementListInit(&encodeIter, &eList, 0, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
				return ret;
			}

			encodeViewElementRequest(&encodeIter);

			if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
				return ret;
			}
		}
		else
		{
			printf("\nConnected Provider does not support Dynamic View requests.  Disabling View functionality.\n");
			viewRequest = RSSL_FALSE;
		}
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
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
 * This function is only used within the Market Price Handler
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
	int i;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	if (snapshotRequest)
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_BATCH;
	}
	else
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_BATCH;
	} 
	if((loginInfo->supportViewRequests == RSSL_TRUE) && (viewRequest == RSSL_TRUE))
	{
		msg.flags |= RSSL_RQMF_HAS_VIEW;
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
	msg.msgBase.msgKey.serviceId = (RsslUInt16)pCommand->serviceId;
	
	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
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
		for(i = 0; i < pCommand->marketPriceItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->marketPriceItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else /* private stream */
	{
		// Start streamID at +1
		for(i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->marketPricePSItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
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

	/* 
	 *	Encode a view request into the list.
	 */
	if((loginInfo->supportViewRequests == RSSL_TRUE) && (viewRequest == RSSL_TRUE) )
		encodeViewElementRequest(&encodeIter);


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
 * This function is only used within the Market Price Handler
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
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
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
	for(i = 0; i < pCommand->marketPriceItemCount; i++)
	{
		itemStreamId = i + MARKETPRICE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->marketPriceItems[i].itemState))
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, 0, (void*)(&itemStreamId))) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	for(i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
	{
		itemStreamId = i + MARKETPRICE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->marketPricePSItems[i].itemState))
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

#include "rtr/rsslReactor.h"
/*
 * Publically visable market price request function 
 *
 * Sends item requests to a channel.  For each item, this
 * consists of getting a message buffer, encoding the item
 * request, and sending the item request to the server.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item request to
 */
RsslRet sendMarketPriceItemRequests(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);
	int i;

	// Do not send a request if there are no items in the item list.
	if (pCommand->marketPriceItemCount == 0 && pCommand->privateStreamMarketPriceItemCount == 0)
		return RSSL_RET_SUCCESS;

	if(getSourceDirectoryCapabilities(pCommand, RSSL_DMT_MARKET_PRICE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_MARKET_PRICE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	/* send out two sets of requests - one for private stream and one for non-private stream */

	/* non-private stream request(s) */
	if (pCommand->marketPriceItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->marketPriceItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->marketPriceBatchRequest.streamId = MARKETPRICE_BATCH_STREAM_ID_START;
				for (i = 0; i < pCommand->marketPriceItemCount; i++)
					pCommand->marketPriceItems[i].streamId = getNextAvailableMarketPriceStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->marketPriceBatchRequest.streamId, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nMarket Price encodeBatchItemRequest() failed\n");
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
			if (pCommand->marketPriceItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests.  Sending Market Price requests as individual request messages.\n");

			for (i = 0; i < pCommand->marketPriceItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->marketPriceItems[i].streamId = getNextAvailableMarketPriceStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, &pCommand->marketPriceItems[i], RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nMarket Price encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);\
					return RSSL_RET_FAILURE;
				}
			}
		}
	}

	/* private stream request(s) */
	if (pCommand->privateStreamMarketPriceItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->privateStreamMarketPriceItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->privateStreamMarketPriceBatchRequest.streamId = MARKETPRICE_BATCH_PRIVATE_STREAM_ID_START;
				for (i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
					pCommand->marketPricePSItems[i].streamId = getNextAvailableMarketPricePrivateStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->privateStreamMarketPriceBatchRequest.streamId, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nMarket Price encodeBatchItemRequest() failed\n");
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
			if (pCommand->privateStreamMarketPriceItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests.  Sending Market Price requests as individual request messages.\n");

			for (i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->marketPricePSItems[i].streamId = getNextAvailableMarketPricePrivateStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, &pCommand->marketPricePSItems[i], RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nMarket Price encodeItemRequest() failed\n");
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
 * Publically visable market price response handler 
 *
 * Processes a market price response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * field list and field entry, and calling decodeFieldEntry() to decode
 * the field entry data.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - Channel this message was received on.
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processMarketPriceResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg *msg, RsslDecodeIterator* dIter)
{
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslMsgKey* key = 0;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;
	ItemRequest *pRequest = NULL;
	RsslBool isBatchRequest = RSSL_FALSE;
	RsslBool isPrivateStream = RSSL_FALSE;
	int i;
	
	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	if (msg->msgBase.streamId != 0)
	{
		/* first check if it is a response for non-private stream market price item request */
		for(i = 0; i < pCommand->marketPriceItemCount; ++i)
		{
			if (pCommand->marketPriceItems[i].streamId == msg->msgBase.streamId)
			{
				pRequest = &pCommand->marketPriceItems[i];
				break;
			}
		}
		/* then check if it is a response for private stream market price item request */
		if (!pRequest)
		{
			for(i = 0; i < pCommand->privateStreamMarketPriceItemCount; ++i)
			{
				if (pCommand->marketPricePSItems[i].streamId == msg->msgBase.streamId)
				{
					pRequest = &pCommand->marketPricePSItems[i];
					isPrivateStream = RSSL_TRUE;
					break;
				}
			}
		}
		/* then check if it is a response for market price batch request */
		if (!pRequest)
		{
			if (msg->msgBase.streamId == pCommand->marketPriceBatchRequest.streamId)
			{
				pRequest = &pCommand->marketPriceBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if (msg->msgBase.streamId == pCommand->privateStreamMarketPriceBatchRequest.streamId)
			{
				pRequest = &pCommand->privateStreamMarketPriceBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if(pCommand->shouldOffStreamPost != RSSL_TRUE)
			{
				printf("Received market price message on unknown stream %d\n", msg->msgBase.streamId);
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

			for(i = 0; i < pCommand->marketPriceItemCount; ++i)
			{
                if (pCommand->marketPriceItems[i].itemName.length == key->name.length &&
                    memcmp(pCommand->marketPriceItems[i].itemName.data, key->name.data, key->name.length) == 0)
				{
					pRequest = &pCommand->marketPriceItems[i];
					break;
				}
			}
        }
        if (pRequest == NULL)
            return RSSL_RET_SUCCESS; // ignore items that we didnt request

	}
	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:

			/* update our item state list if its a refresh, then process just like update */
			if (!(msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", pRequest->streamId);
					if (closeMPItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketPriceItemEntry(pCommand, pRequest, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			pRequest->itemState.dataState = msg->refreshMsg.state.dataState;
			pRequest->itemState.streamState = msg->refreshMsg.state.streamState;

			setGroupId(pCommand, pRequest, &msg->refreshMsg.groupId);
			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:

			/* decode market price response */
			if (pCommand->cacheInfo.useCache)
				applyMsgToCache(&pRequest->cacheEntryHandle, &pCommand->cacheInfo, msg, dIter);

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
			else if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);
			}

			/* decode field list */
			if (pCommand->cacheInfo.useCache == RSSL_FALSE)
			{
				if ((ret = decodeMarketPriceFieldList(&((ChannelCommand*)pReactorChannel->userSpecPtr)->dictionary,
						dIter)) != RSSL_RET_SUCCESS)
					return ret;
			}
			else
				printf("Payload cached\n");

			break;

		case RSSL_MC_STATUS:
			if (isBatchRequest)
				printf("\nReceived Batch StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);
			else
				printf("\nReceived Item StatusMsg for stream %i \n", pRequest->streamId);

			/* update our state table for posting */
			if (!(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", msg->msgBase.streamId);
					if (closeMPItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketPriceItemEntry(pCommand, pRequest, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			rsslStateToString(&tempBuffer, &msg->statusMsg.state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

				if(pRequest)
				{
					pRequest->itemState.dataState = msg->statusMsg.state.dataState;
					pRequest->itemState.streamState = msg->statusMsg.state.streamState;
				}
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
 * Publically visable - used to close all market price streams
 *
 * Close all item streams.
 * pReactor - RsslReactor associated with the application
 * pCommand - the ChannelCommand to send an item close to
 */
RsslRet closeMarketPriceItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand)
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
	if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_CLOSES) && (pCommand->marketPriceItemCount + pCommand->privateStreamMarketPriceItemCount) > 1)
	{
		/* get a buffer for the item close */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			batchCloseStreamId = getNextAvailableMarketPriceStreamId(pCommand);

			/* encode batch close */
			if (encodeBatchItemClose(pReactorChannel, msgBuf, batchCloseStreamId) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nMarket Price encodeBatchItemClose() failed\n");
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
		if ((pCommand->marketPriceItemCount + pCommand->privateStreamMarketPriceItemCount) > 1)
			printf("\nConnected Provider does not support Batch Closes. Sending Market Price closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < pCommand->marketPriceItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->marketPriceItems[i].itemState))
			{
				if (closeMPItemByStreamId(pReactor, pReactorChannel, pCommand, i + MARKETPRICE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->marketPricePSItems[i].itemState))
			{
				if (closeMPItemByStreamId(pReactor, pReactorChannel, pCommand, i + MARKETPRICE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
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
	if (pCommand->privateStreamMarketPriceItemCount < CHAN_CMD_MAX_ITEMS)
	{
		pItemRequest = &pCommand->marketPricePSItems[pCommand->privateStreamMarketPriceItemCount];
		++pCommand->privateStreamMarketPriceItemCount;
	}
	else
	{
		printf("Number of Private Stream Market Price items exceeded\n");
	}

	pItemRequest->streamId = getNextAvailableMarketPricePrivateStreamId(pCommand);

	// save the itemName for redirecting to Private Stream 
	snprintf(pItemRequest->itemNameString, 128, "%s", pRequest->itemNameString);
	pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
	pItemRequest->itemName.data = pItemRequest->itemNameString; 

	/* remove non-private stream entry from list */
	removeMarketPriceItemEntry(pCommand, pRequest, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslReactorGetBuffer(pCommand->reactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode item request */
        if (encodeItemRequest(pCommand->reactorChannel, msgBuf, pItemRequest, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			printf("\nMarket Price encodeItemRequest() failed\n");
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

/*
 * decodeMarketPriceFieldList
 */
RsslRet decodeMarketPriceFieldList(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter)
{
	RsslRet ret;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;

	if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) == RSSL_RET_SUCCESS)
	{
		/* decode each field entry in list */
		while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret == RSSL_RET_SUCCESS)
			{
				/* decode field entry info */
				if (decodeFieldEntry(dictionary, &fEntry, dIter) != RSSL_RET_SUCCESS)
				{
					printf("\ndecodeFieldEntry() failed\n");
					return RSSL_RET_FAILURE;
				}
			}
			else
			{
				printf("rsslDecodeFieldEntry() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}
		}
		if (ret == RSSL_RET_END_OF_CONTAINER)
			return RSSL_RET_SUCCESS;
	}
	else
	{
		printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
		return RSSL_RET_FAILURE;
	}

	return ret;
}

/*
 * Publically visible - used by all non-admin domain handlers to output field lists
 *
 * Decodes the field entry data and prints out the field entry data
 * with help of the dictionary.  Returns success if decoding succeeds
 * or failure if decoding fails.
 * pReactorChannel - The channel the message was received on
 * fEntry - The field entry data
 * dIter - The decode iterator
 */
RsslRet decodeFieldEntry(RsslDataDictionary *dictionary, RsslFieldEntry* fEntry, RsslDecodeIterator *dIter)
{
	RsslRet ret = 0;
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslUInt64 fidUIntValue = 0;
	RsslInt64 fidIntValue = 0;
	RsslFloat tempFloat = 0;
	RsslDouble tempDouble = 0;
	RsslReal fidRealValue = RSSL_INIT_REAL;
	RsslEnum fidEnumValue;
	RsslFloat fidFloatValue = 0;
	RsslDouble fidDoubleValue = 0;
	RsslQos fidQosValue = RSSL_INIT_QOS; 
	RsslDateTime fidDateTimeValue;
	RsslState fidStateValue;
	RsslBuffer fidBufferValue;
	RsslBuffer fidDateTimeBuf;
	RsslBuffer fidRealBuf;
	RsslBuffer fidStateBuf;
	RsslBuffer fidQosBuf;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* get dictionary entry */
	if (!dictionary->isInitialized)
	{
		dumpHexBuffer(&fEntry->encData);
		return RSSL_RET_SUCCESS;
	}
	else
		dictionaryEntry = dictionary->entriesArray[fEntry->fieldId];

	/* return if no entry found */
	if (!dictionaryEntry) 
    {
		printf("\tFid %d not found in dictionary\n", fEntry->fieldId);
		dumpHexBuffer(&fEntry->encData);
		return RSSL_RET_SUCCESS;
    }

	/* print out fid name */
	printf("\t%-20s", dictionaryEntry->acronym.data);
	/* decode and print out fid value */
	dataType = dictionaryEntry->rwfType;
	switch (dataType)
	{
		case RSSL_DT_UINT:
			if ((ret = rsslDecodeUInt(dIter, &fidUIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLU "\n", fidUIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_INT:
			if ((ret = rsslDecodeInt(dIter, &fidIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLD "\n", fidIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeInt() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_FLOAT:
			if ((ret = rsslDecodeFloat(dIter, &fidFloatValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f\n", fidFloatValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeFloat() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DOUBLE:
			if ((ret = rsslDecodeDouble(dIter, &fidDoubleValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f\n", fidDoubleValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDouble() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_REAL:
			if ((ret = rsslDecodeReal(dIter, &fidRealValue)) == RSSL_RET_SUCCESS)
			{
				fidRealBuf.data = (char*)alloca(35);
				fidRealBuf.length = 35;
				rsslRealToString(&fidRealBuf, &fidRealValue);
				printf("%s\n", fidRealBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeReal() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_ENUM:
			if ((ret = rsslDecodeEnum(dIter, &fidEnumValue)) == RSSL_RET_SUCCESS)
			{
				RsslEnumType *pEnumType = getFieldEntryEnumType(dictionaryEntry, fidEnumValue);
				if (pEnumType)
    				printf("%.*s(%d)\n", pEnumType->display.length, pEnumType->display.data, fidEnumValue);
				else
    				printf("%d\n", fidEnumValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeEnum() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATE:
			if ((ret = rsslDecodeDate(dIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDate() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_TIME:
			if ((ret = rsslDecodeTime(dIter, &fidDateTimeValue.time)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_TIME, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeTime() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATETIME:
			if ((ret = rsslDecodeDateTime(dIter, &fidDateTimeValue)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(50);
				fidDateTimeBuf.length = 50;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATETIME, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDateTime() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_QOS:
			if((ret = rsslDecodeQos(dIter, &fidQosValue)) == RSSL_RET_SUCCESS) {
				fidQosBuf.data = (char*)alloca(100);
				fidQosBuf.length = 100;
				rsslQosToString(&fidQosBuf, &fidQosValue);
				printf("%s\n", fidQosBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeQos() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_STATE:
			if((ret = rsslDecodeState(dIter, &fidStateValue)) == RSSL_RET_SUCCESS) {
				int stateBufLen = 80;
				if (fidStateValue.text.data)
					stateBufLen += fidStateValue.text.length;
				fidStateBuf.data = (char*)alloca(stateBufLen);
				fidStateBuf.length = stateBufLen;
				rsslStateToString(&fidStateBuf, &fidStateValue);
				printf("%.*s\n", fidStateBuf.length, fidStateBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeState() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		
		/*For an example of array decoding, see fieldListEncDec.c*/
		case RSSL_DT_ARRAY:
		break;
		case RSSL_DT_BUFFER:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			if((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
			{
				printf("%.*s\n", fidBufferValue.length, fidBufferValue.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA) 
			{
				printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		default:
			printf("Unsupported data type (%d) for fid value\n", dataType);
			break;
	}
	if (ret == RSSL_RET_BLANK_DATA)
	{
		printf("<blank data>\n");
	}

	return RSSL_RET_SUCCESS;
}

/* 
 * this function is used while posting to query the first 
 * requested market price item, if any.
 * pCommand - the ChannelCommand to send an item request to
 */
ItemRequest* getFirstMarketPriceItem(ChannelCommand *pCommand)
{
	int i; 
	ItemRequest *pRequest;

	for (i = 0; i < pCommand->marketPriceItemCount; i++)
	{
		pRequest = &pCommand->marketPriceItems[i];
		/* iterate over state list, when we find first OPEN state return that items information */
		if ((pRequest->itemState.dataState == RSSL_DATA_OK) && (pRequest->itemState.streamState == RSSL_STREAM_OPEN))
		{
			return pRequest;
		}
	}

	/* no suitable items were found */
	return NULL;

}


