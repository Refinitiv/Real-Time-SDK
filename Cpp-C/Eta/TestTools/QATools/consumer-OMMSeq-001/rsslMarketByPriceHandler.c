/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2020 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */


/*
 * This is the market by price handler for the rsslConsumer application.
 * It provides functions for sending the market by price request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * maps containing field lists, decoding field entries from a response,
 * closing market by price streams, getting the next stream id, and
 * adding/removing items to/from the item list are also provided.
 */

#include "rsslMarketByPriceHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslSendMessage.h"

/* non-private stream id starting point */
#define MARKET_BY_PRICE_STREAM_ID_START 208
#define MARKET_BY_PRICE_BATCH_STREAM_ID_START 207

/* private stream id starting point */
#define MARKET_BY_PRICE_PRIVATE_STREAM_ID_START (MARKET_BY_PRICE_STREAM_ID_START + 50)
#define MARKET_BY_PRICE_BATCH_PRIVATE_STREAM_ID_START (MARKET_BY_PRICE_BATCH_STREAM_ID_START + 50)

/* non-private stream market by price item info list */
static MarketByPriceItemInfo marketByPriceItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* private stream market by price item info list */
static MarketByPriceItemInfo marketByPricePSItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* next non-private stream id for item requests */
static RsslInt32 nextStreamId = MARKET_BY_PRICE_STREAM_ID_START;

/* next private stream id for item requests */
static RsslInt32 nextPrivateStreamId = MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;

/* number of items in non-private stream item list */
static RsslInt32 itemCount = 0;

/* number of items in private stream item list */
static RsslInt32 privateStreamItemCount = 0;

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/*
 * Publically visable method used to add a market by price name to the request list
 * Adds an item name requested by the application to
 * the item name list.
 * itemname - Item name requested by the application
 * isPrivateStream - Flag for private stream
 */
void addMarketByPriceItemName(char* itemname, RsslBool isPrivateStream)
{
	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		if (itemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(marketByPriceItemInfoList[nextStreamId - MARKET_BY_PRICE_STREAM_ID_START].itemname, 128, "%s", itemname);
			marketByPriceItemInfoList[nextStreamId - MARKET_BY_PRICE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			marketByPriceItemInfoList[nextStreamId - MARKET_BY_PRICE_STREAM_ID_START].streamId = 0;
			++nextStreamId;
			++itemCount;
		}
		else
		{
			printf("Number of Market By Price items exceeded\n");
		}
	}
	else /* private stream */
	{
		if (privateStreamItemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(marketByPricePSItemInfoList[nextPrivateStreamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START].itemname, 128, "%s", itemname);
			marketByPricePSItemInfoList[nextPrivateStreamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			marketByPricePSItemInfoList[nextPrivateStreamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START].streamId = 0;
			++nextPrivateStreamId;
			++privateStreamItemCount;
		}
		else
		{
			printf("Number of Private Stream Market By Price items exceeded\n");
		}
	}
}

/*
 * Removes an entry from the item name list.
 * streamId - Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeMarketByPriceItemEntry(RsslChannel *chnl, RsslInt32 streamId, RsslBool isPrivateStream)
{
	int i;

	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		for (i = streamId - MARKET_BY_PRICE_STREAM_ID_START; i < itemCount - 1; i++)
		{
			strncpy(marketByPriceItemInfoList[i].itemname, marketByPriceItemInfoList[i+1].itemname, 129);
			marketByPriceItemInfoList[i].nameLength = marketByPriceItemInfoList[i+1].nameLength;
			marketByPriceItemInfoList[i].streamId = marketByPriceItemInfoList[i+1].streamId;
			marketByPriceItemInfoList[i].itemState = marketByPriceItemInfoList[i+1].itemState;
		}
		--nextStreamId;
		--itemCount;
	}
	else /* private stream */
	{
		for (i = streamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START; i < privateStreamItemCount - 1; i++)
		{
			strncpy(marketByPricePSItemInfoList[i].itemname, marketByPricePSItemInfoList[i+1].itemname, 129);
			marketByPricePSItemInfoList[i].nameLength = marketByPricePSItemInfoList[i+1].nameLength;
			marketByPricePSItemInfoList[i].streamId = marketByPricePSItemInfoList[i+1].streamId;
			marketByPricePSItemInfoList[i].itemState = marketByPricePSItemInfoList[i+1].itemState;
		}
		--nextPrivateStreamId;
		--privateStreamItemCount;
	}
}

/*
 * Encodes the market by price item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the market by price item close
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Close an item stream for a specific stream id.
 * chnl - The channel to send an item close to
 * streamId - The stream id of the item close
 */
static RsslRet closeMBPItemStream(RsslChannel* chnl, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item close */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode item close */
		if (encodeItemClose(chnl, msgBuf, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nMarket Price encodeItemClose() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS) 
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
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
 * chnl - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the item request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemRequest(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;
	RsslSourceDirectoryResponseInfo* srcDirRespInfo = 0;
	RsslUInt32 listIndex;

	if (getSourceDirectoryResponseInfo(getServiceId(), &srcDirRespInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
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
	rsslCopyQos(&(msg.qos), &(srcDirRespInfo->ServiceGeneralInfo.QoS[0]));
	
	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	if (!isPrivateStream) /* non-private stream */
	{
		listIndex = streamId - MARKET_BY_PRICE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = marketByPriceItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = marketByPriceItemInfoList[listIndex].nameLength;
	}
	else /* private stream */
	{
		listIndex = streamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = marketByPricePSItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = marketByPricePSItemInfoList[listIndex].nameLength;
	}
	msg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);


	if (!isPrivateStream) 
		marketByPriceItemInfoList[listIndex].streamId = streamId;
	else
		marketByPricePSItemInfoList[listIndex].streamId = streamId;

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the batch item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the item request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeBatchItemRequest(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslElementList eList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry eEntry = RSSL_INIT_ELEMENT_ENTRY;
	RsslArray elementArray = RSSL_INIT_ARRAY; 
	RsslEncodeIterator encodeIter;
	RsslSourceDirectoryResponseInfo* srcDirRespInfo = 0;
	RsslBuffer itemName;
	int i;

	if (getSourceDirectoryResponseInfo(getServiceId(), &srcDirRespInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_HAS_BATCH;
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
	rsslCopyQos(&(msg.qos), &(srcDirRespInfo->ServiceGeneralInfo.QoS[0]));
	
	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
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
		for(i = 0; i < itemCount; i++)
		{
			itemName.data = marketByPriceItemInfoList[i].itemname;
			itemName.length = marketByPriceItemInfoList[i].nameLength;
			marketByPriceItemInfoList[i].streamId = ++streamId;
			if((ret = rsslEncodeArrayEntry(&encodeIter, &itemName, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else /* private stream */
	{
		for(i = 0; i < privateStreamItemCount; i++)
		{
			itemName.data = marketByPricePSItemInfoList[i].itemname;
			itemName.length = marketByPricePSItemInfoList[i].nameLength;
			marketByPricePSItemInfoList[i].streamId = ++streamId;
			if((ret = rsslEncodeArrayEntry(&encodeIter, &itemName, 0)) < RSSL_RET_SUCCESS)
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
 * chnl - The channel to send a batch item close message to
 * msgBuf - The message buffer to encode the batch item close msg into
 * batchCloseStreamId - The stream id of the batch item close message
 *
 * This function is only used within the Market By Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeBatchItemClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 batchCloseStreamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg;
	RsslElementList eList;
	RsslElementEntry eEntry;
	RsslArray elementArray;
	RsslEncodeIterator encodeIter;

	RsslInt itemStreamId;
	int i;

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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

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
	for(i = 0; i < itemCount; i++)
	{
		itemStreamId = i + MARKET_BY_PRICE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&marketByPriceItemInfoList[i].itemState))
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, 0, (void*)(&itemStreamId))) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	for(i = 0; i < privateStreamItemCount; i++)
	{
		itemStreamId = i + MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&marketByPricePSItemInfoList[itemStreamId].itemState))
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
 * chnl - The channel to send an item request to
 */
RsslRet sendMarketByPriceItemRequests(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
	int i;

	// Do not send a request if there are no items in the item list.
	if(itemCount == 0 && privateStreamItemCount == 0)
		return RSSL_RET_SUCCESS;

	/* check to see if the provider supports the market by price domain */
	if(getSourceDirectoryCapabilities(RSSL_DMT_MARKET_BY_PRICE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_MARKET_BY_PRICE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as we receive refresh and status messages */
		marketByPriceItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		marketByPriceItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as refresh and status messages are received */
		marketByPricePSItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		marketByPricePSItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
	}

	/* send out two sets of requests - one for private stream and one for non-private stream */

	/* non-private stream request(s) */
	if (itemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->SupportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (itemCount > 1))
		{
			msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);
		
			if (msgBuf != NULL)
			{
				if(encodeBatchItemRequest(chnl, msgBuf, MARKET_BY_PRICE_BATCH_STREAM_ID_START, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nMarket By Price encodeBatchItemRequest() failed\n");
					return RSSL_RET_FAILURE;
				}
				
				if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslGetBuffer(): Failed <%s>\n", error.text);
				return RSSL_RET_FAILURE;
			}
		} 
		else 
		{
			if (itemCount > 1)
				printf("\nConnected Provider does not support Batch Requests. Sending Market Price requests as individual request messages.\n");

			for (i = 0; i < itemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

				if (msgBuf != NULL)
				{
					/* encode item request */
					if (encodeItemRequest(chnl, msgBuf, (i + MARKET_BY_PRICE_STREAM_ID_START), RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nMarket By Price encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslGetBuffer(): Failed <%s>\n", error.text);
					return RSSL_RET_FAILURE;
				}
			}
		}
	}

	/* private stream request(s) */
	if (privateStreamItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->SupportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (privateStreamItemCount > 1))
		{
			msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);
		
			if (msgBuf != NULL)
			{
				if(encodeBatchItemRequest(chnl, msgBuf, MARKET_BY_PRICE_BATCH_PRIVATE_STREAM_ID_START, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nMarket By Price encodeBatchItemRequest() failed\n");
					return RSSL_RET_FAILURE;
				}
				
				if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslGetBuffer(): Failed <%s>\n", error.text);
				return RSSL_RET_FAILURE;
			}
		} 
		else 
		{
			if (privateStreamItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests. Sending Private Stream Market Price requests as individual request messages.\n");

			for (i = 0; i < privateStreamItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

				if (msgBuf != NULL)
				{
					/* encode item request */
					if (encodeItemRequest(chnl, msgBuf, (i + MARKET_BY_PRICE_PRIVATE_STREAM_ID_START), RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nMarket By Price encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslGetBuffer(): Failed <%s>\n", error.text);
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
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processMarketByPriceResponse(RsslChannel *chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key;
	const char* actionString;
	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslBuffer mapKey = RSSL_INIT_BUFFER;
	RsslLocalFieldSetDefDb localFieldSetDefDb;  // this must be cleared using the clear function below
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslBool isBatchRequest = RSSL_FALSE;
	RsslBool isPrivateStream = RSSL_FALSE;
	MarketByPriceItemInfo *itemInfo = NULL;

	rsslClearLocalFieldSetDefDb(&localFieldSetDefDb);
	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	// determine which item in the info list we received a response for
	if (msg->msgBase.streamId == MARKET_BY_PRICE_BATCH_STREAM_ID_START ||
		msg->msgBase.streamId == MARKET_BY_PRICE_BATCH_PRIVATE_STREAM_ID_START)
	{
		isBatchRequest = RSSL_TRUE;	// we dont have info list entries for a status response to a batch request
	}
	else if (msg->msgBase.streamId >= MARKET_BY_PRICE_PRIVATE_STREAM_ID_START)
	{
		isPrivateStream = RSSL_TRUE;
		itemInfo = &marketByPricePSItemInfoList[msg->msgBase.streamId - MARKET_BY_PRICE_PRIVATE_STREAM_ID_START];
	}
	else if (msg->msgBase.streamId >= MARKET_BY_PRICE_STREAM_ID_START)
	{
		itemInfo = &marketByPriceItemInfoList[msg->msgBase.streamId - MARKET_BY_PRICE_STREAM_ID_START];
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
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", itemInfo->streamId);
					/* close stream */
					if (closeMBPItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketByPriceItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}
			
			/* update our item state list if its a refresh, then process just like update */
			itemInfo->itemState.dataState = msg->refreshMsg.state.dataState;
			itemInfo->itemState.streamState = msg->refreshMsg.state.streamState;

			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:
			/* decode market by price response */

			/* print out item name from key if it has it */
			if (key && (key->flags & RSSL_MKF_HAS_NAME))
				printf("\n%.*s", key->name.length, key->name.data);
			else
				printf("\n%s", itemInfo->itemname);

			if (isPrivateStream) printf(" (PRIVATE STREAM)");
			printf("\n");

			printf("DOMAIN: %s\n", rsslDomainTypeToString(msg->msgBase.domainType));

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
		
			/* level 2 market by price is a map of field lists */
			if ((ret = rsslDecodeMap(dIter, &rsslMap)) == RSSL_RET_SUCCESS)
			{
				/* decode set definition database */
				if (rsslMap.flags & RSSL_MPF_HAS_SET_DEFS)
				{
					/* decode set definition - should be field set definition */
					/* this needs to be passed in when we decode each field list */
					if (rsslDecodeLocalFieldSetDefDb(dIter, &localFieldSetDefDb) != RSSL_RET_SUCCESS)
					{
						printf("rsslDecodeLocalFieldSetDefDb() failed\n");
						return RSSL_RET_FAILURE;
					}
				}

				/* decode any summary data - this should be a field list according to the domain model */ 
				if (rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA)
				{
					printf("SUMMARY DATA\n");
					if ((ret = rsslDecodeFieldList(dIter, &fList, &localFieldSetDefDb)) == RSSL_RET_SUCCESS)
					{
						/* decode each field entry in list */
						while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
						{
							if (ret == RSSL_RET_SUCCESS)
					 		{
								/* decode field entry info */
								/* tab this over before decoding this so it looks better within this domain model */
								printf("\t");
								if (decodeFieldEntry(&fEntry, dIter) != RSSL_RET_SUCCESS)
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
					}
					else
					{
						printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
						return RSSL_RET_FAILURE;
					}
					printf("\n"); /* add blank line after summary data for readability */
				}

				/* decode the map */
				while ((ret = rsslDecodeMapEntry(dIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret == RSSL_RET_SUCCESS)
					{
						/* convert the action to a string for display purposes */
						switch(mapEntry.action)
						{
							case RSSL_MPEA_UPDATE_ENTRY:
								actionString = "RSSL_MPEA_UPDATE_ENTRY"; 
								break;
							case RSSL_MPEA_ADD_ENTRY:
								actionString = "RSSL_MPEA_ADD_ENTRY";
								break;
							case RSSL_MPEA_DELETE_ENTRY:
								actionString = "RSSL_MPEA_DELETE_ENTRY";
								break;
							default:
								actionString = "Unknown";

						}
						/* print out the key */
						if (mapKey.length)
						{
							printf("PRICE POINT: %.*s\nACTION: %s\n", mapKey.length, mapKey.data, actionString);
						}
					
						/* there is not any payload in delete actions */
						if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
						{
							/* decode field list */
							if ((ret = rsslDecodeFieldList(dIter, &fList, &localFieldSetDefDb)) == RSSL_RET_SUCCESS)
							{
								/* decode each field entry in list */
								while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
								{
									if (ret == RSSL_RET_SUCCESS)
						 			{
										/* decode field entry info */
										/* tab this over before decoding this so it looks better within this domain model */
										printf("\t");
										if (decodeFieldEntry(&fEntry, dIter) != RSSL_RET_SUCCESS)
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
							}
							else
							{
								printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
								return RSSL_RET_FAILURE;
							}						
						}
						printf("\n"); /* add a space between end of entry and beginning of next entry for readability */
					}
					else
					{
						printf("rsslDecodeMapEntry() failed with return code: %d\n", ret);
						return RSSL_RET_FAILURE;
					}
				}
			}
			else
			{
				printf("rsslDecodeMap() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}

			break;

		case RSSL_MC_STATUS:
			if (isBatchRequest)
				printf("\nReceived Batch StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);
			else
				printf("\nReceived Item StatusMsg for stream %i \n", itemInfo->streamId);

			if (!(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", itemInfo->streamId);
					/* close stream */
					if (closeMBPItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketByPriceItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}
			
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			rsslStateToString(&tempBuffer, &msg->statusMsg.state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

				if (itemInfo)
				{
					/* update our state table with the new state */
					itemInfo->itemState.dataState = msg->statusMsg.state.dataState;
					itemInfo->itemState.streamState = msg->statusMsg.state.streamState;
				}
				/* redirect to private stream if indicated */
				if (msg->statusMsg.state.streamState == RSSL_STREAM_REDIRECTED &&
					(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM))
				{
    				if (redirectToMarketByPricePrivateStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}

			break;

		case RSSL_MC_ACK:
			/* although this application only posts on MP (Market Price), 
			   ACK handler is provided for other domains to allow user to extend 
			   and post on MBO (Market By Order), MBP (Market By Price), SymbolList, and Yield Curve domains */
			printf("\nReceived AckMsg for stream %i \n", msg->msgBase.streamId);

			/* print out item name from key if it has it */
			if (key && (key->flags & RSSL_MKF_HAS_NAME))
				printf("\n%.*s", key->name.length, key->name.data);
			else if(itemInfo)	
				printf("\n%s", itemInfo->itemname);

			if (isPrivateStream) printf(" (PRIVATE STREAM)");
			printf("\n");

			printf("DOMAIN: %s\n", rsslDomainTypeToString(msg->msgBase.domainType));

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
 * Publically visable Market By Price close function
 *
 * Close all market by price item streams.
 * chnl - The channel to send an item close to
 */
RsslRet closeMarketByPriceItemStreams(RsslChannel* chnl)
{
	RsslBuffer* msgBuf = 0;
	RsslError error;
	int i;
	RsslInt32 batchCloseStreamId;

	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();

	/* If provider supports batch close, will use batch close */
	if((loginInfo->SupportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_CLOSES) && (itemCount + privateStreamItemCount) > 1)
	{
		/* get a buffer for the item request */
		msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			batchCloseStreamId = nextStreamId++;

			/* encode batch close */
			if (encodeBatchItemClose(chnl, msgBuf, batchCloseStreamId) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error); 
				printf("\nMarket By Price encodeBatchItemClose() failed\n");
				return RSSL_RET_FAILURE;
			}

			/* send batch close */
			if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			printf("rsslGetBuffer(): Failed <%s>\n", error.text);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		if ((itemCount + privateStreamItemCount) > 1)
			printf("\nConnected Provider does not support Batch Closes. Sending Market By Price closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < itemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&marketByPriceItemInfoList[i].itemState))
			{
				if (closeMBPItemStream(chnl, i + MARKET_BY_PRICE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < privateStreamItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&marketByPricePSItemInfoList[i].itemState))
			{
				if (closeMBPItemStream(chnl, i + MARKET_BY_PRICE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Redirect a request to a private stream.
 * streamId - The stream id to be redirected to private stream
 */
RsslRet redirectToMarketByPricePrivateStream(RsslChannel *chnl, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* add item name to private stream list */
	addMarketByPriceItemName(marketByPriceItemInfoList[streamId - MARKET_BY_PRICE_STREAM_ID_START].itemname, RSSL_TRUE);

	/* remove non-private stream entry from list */
	removeMarketByPriceItemEntry(chnl, streamId, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode item request */
		if (encodeItemRequest(chnl, msgBuf, nextPrivateStreamId - 1, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nMarket By Price encodeItemRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send item request */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}
