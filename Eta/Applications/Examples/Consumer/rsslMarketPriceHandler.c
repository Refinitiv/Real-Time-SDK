
/*
 * This is the market price handler for the rsslConsumer application.
 * It provides functions for sending the market price request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * a field entry from a response, closing market price streams, getting
 * the next stream id, and adding/removing items to/from the item list
 * are also provided.
 */

#include "rsslMarketPriceHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslSendMessage.h"

/* non-private stream id starting point */
#define MARKETPRICE_STREAM_ID_START 6
#define MARKETPRICE_BATCH_STREAM_ID_START 5

/* private stream id starting point */
#define MARKETPRICE_PRIVATE_STREAM_ID_START (MARKETPRICE_STREAM_ID_START + 50)
#define MARKETPRICE_BATCH_PRIVATE_STREAM_ID_START (MARKETPRICE_BATCH_STREAM_ID_START + 50)

/* non-private stream market price item info list */
static MarketPriceItemInfo marketPriceItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* private stream market price item info list */
static MarketPriceItemInfo marketPricePSItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* next non-private stream id for item requests */
static RsslInt32 nextStreamId = MARKETPRICE_STREAM_ID_START;

/* next private stream id for item requests */
static RsslInt32 nextPrivateStreamId = MARKETPRICE_PRIVATE_STREAM_ID_START;

/* number of items in non-private stream item list */
static RsslInt32 itemCount = 0;

/* number of items in private stream item list */
static RsslInt32 privateStreamItemCount = 0;

/* Send a view request to the provider */
static RsslBool viewRequest = RSSL_FALSE;

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/*
 * Publically visable - used to add a market price item to the request list
 * Adds an item name requested by the application to
 * the item name list.
 * itemname - Item name requested by the application
 * isPrivateStream - Flag for private stream
 */
void addMarketPriceItemName(char* itemname, RsslBool isPrivateStream)
{
	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		if (itemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(marketPriceItemInfoList[nextStreamId - MARKETPRICE_STREAM_ID_START].itemname, 128, "%s", itemname);
			marketPriceItemInfoList[nextStreamId - MARKETPRICE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			marketPriceItemInfoList[nextStreamId - MARKETPRICE_STREAM_ID_START].streamId = 0;
			++nextStreamId;
			++itemCount;
		}
		else
		{
			printf("Number of Market Price items exceeded\n");
		}
	}
	else /* private stream */
	{
		if (privateStreamItemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(marketPricePSItemInfoList[nextPrivateStreamId - MARKETPRICE_PRIVATE_STREAM_ID_START].itemname, 128, "%s", itemname);
			marketPricePSItemInfoList[nextPrivateStreamId - MARKETPRICE_PRIVATE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			marketPricePSItemInfoList[nextPrivateStreamId - MARKETPRICE_PRIVATE_STREAM_ID_START].streamId = 0;
			++nextPrivateStreamId;
			++privateStreamItemCount;
		}
		else
		{
			printf("Number of Private Stream Market Price items exceeded\n");
		}
	}
}

/*
 * Removes an entry from the item name list.
 * streamId - Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeMarketPriceItemEntry(RsslChannel *chnl, RsslInt32 streamId, RsslBool isPrivateStream)
{
	int i;

	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		for (i = streamId - MARKETPRICE_STREAM_ID_START; i < itemCount - 1; i++)
		{
			strncpy(marketPriceItemInfoList[i].itemname, marketPriceItemInfoList[i+1].itemname, 129);
			marketPriceItemInfoList[i].nameLength = marketPriceItemInfoList[i+1].nameLength;
			marketPriceItemInfoList[i].streamId = marketPriceItemInfoList[i+1].streamId;
			marketPriceItemInfoList[i].itemState = marketPriceItemInfoList[i+1].itemState;
		}
		--nextStreamId;
		--itemCount;
	}
	else /* private stream */
	{
		for (i = streamId - MARKETPRICE_PRIVATE_STREAM_ID_START; i < privateStreamItemCount - 1; i++)
		{
			strncpy(marketPricePSItemInfoList[i].itemname, marketPricePSItemInfoList[i+1].itemname, 129);
			marketPricePSItemInfoList[i].nameLength = marketPricePSItemInfoList[i+1].nameLength;
			marketPricePSItemInfoList[i].streamId = marketPricePSItemInfoList[i+1].streamId;
			marketPricePSItemInfoList[i].itemState = marketPricePSItemInfoList[i+1].itemState;
		}
		--nextPrivateStreamId;
		--privateStreamItemCount;
	}
}

/*
 * Encodes the item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the item close
 *
 * This function is only used within the Market Price Handler
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
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
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
static RsslRet closeMPItemStream(RsslChannel* chnl, RsslInt32 streamId)
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

/* this function is used while posting to query the first 
 * requested market price item, if any.
 * It will populate the passed in buffer with the name
 * and length information and return the streamId 
 * associated with the stream.  
 * If mpItemName->length is 0 and streamId is returned
 * as 0, this indicates that there is no valid name 
 * available. 
 */
RsslInt32 getFirstMarketPriceItem(RsslBuffer *mpItemName)
{
	int i;

	/* try non-private stream items first */
	for (i = 0; i < itemCount; i++)
	{
		/* iterate over state list, when we find first OPEN state return that items information */
		if ((marketPriceItemInfoList[i].itemState.dataState == RSSL_DATA_OK) && (marketPriceItemInfoList[i].itemState.streamState == RSSL_STREAM_OPEN))
		{
			mpItemName->data = marketPriceItemInfoList[i].itemname;
			mpItemName->length = marketPriceItemInfoList[i].nameLength;
			return (i + MARKETPRICE_STREAM_ID_START);
		}
	}

	/* now try private stream items */
	for (i = 0; i < privateStreamItemCount; i++)
	{
		/* iterate over state list, when we find first OPEN state return that items information */
		if ((marketPricePSItemInfoList[i].itemState.dataState == RSSL_DATA_OK) && (marketPricePSItemInfoList[i].itemState.streamState == RSSL_STREAM_OPEN))
		{
			mpItemName->data = marketPricePSItemInfoList[i].itemname;
			mpItemName->length = marketPricePSItemInfoList[i].nameLength;
			return (i + MARKETPRICE_PRIVATE_STREAM_ID_START);
		}
	}

	/* no suitable items were found */
	mpItemName->length = 0;
	mpItemName->data = 0;
	return 0;

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

	viewArray.primitiveType = RSSL_DT_INT;
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
 * chnl - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the item request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Market Price Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemRequest(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;
	RsslElementList eList = RSSL_INIT_ELEMENT_LIST;
	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
	RsslSourceDirectoryResponseInfo* srcDirRespInfo = 0;
	RsslUInt32	listIndex;

	if (getSourceDirectoryResponseInfo(getServiceId(), &srcDirRespInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	if((viewRequest == RSSL_TRUE) && (loginInfo->SupportViewRequests == RSSL_TRUE))
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
	if((loginInfo->SupportViewRequests == RSSL_TRUE)  && (viewRequest == RSSL_TRUE))
		msg.flags |= RSSL_RQMF_HAS_VIEW;
	msg.priorityClass = 1;
	msg.priorityCount = 1;

	/* copy the QoS information */
	rsslCopyQos(&(msg.qos), &(srcDirRespInfo->ServiceGeneralInfo.QoS[0]));
	
	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	if (!isPrivateStream) /* non-private stream */
	{
		listIndex = streamId - MARKETPRICE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = marketPriceItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = marketPriceItemInfoList[listIndex].nameLength;
	}
	else /* private stream */
	{
		listIndex = streamId - MARKETPRICE_PRIVATE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = marketPricePSItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = marketPricePSItemInfoList[listIndex].nameLength;
	}
	msg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	if(viewRequest == RSSL_TRUE) 
	{
		if (loginInfo->SupportViewRequests == RSSL_TRUE)
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


	if (!isPrivateStream) 
		marketPriceItemInfoList[listIndex].streamId = streamId;
	else
		marketPricePSItemInfoList[listIndex].streamId = streamId;

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
 * This function is only used within the Market Price Handler
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

	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
	
	if (getSourceDirectoryResponseInfo(getServiceId(), &srcDirRespInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

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
	if((loginInfo->SupportViewRequests == RSSL_TRUE) && (viewRequest == RSSL_TRUE))
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
			itemName.data = marketPriceItemInfoList[i].itemname;
			itemName.length = marketPriceItemInfoList[i].nameLength;
			marketPriceItemInfoList[i].streamId = ++streamId;
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
			itemName.data = marketPricePSItemInfoList[i].itemname;
			itemName.length = marketPricePSItemInfoList[i].nameLength;
			marketPricePSItemInfoList[i].streamId = ++streamId;
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

	/* 
	 *	Encode a view request into the list.
	 */
	if((loginInfo->SupportViewRequests == RSSL_TRUE) && (viewRequest == RSSL_TRUE) )
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
 * chnl - The channel to send a batch item close message to
 * msgBuf - The message buffer to encode the batch item close msg into
 * batchCloseStreamId - The stream id of the batch item close message
 *
 * This function is only used within the Market Price Handler
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
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
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
		itemStreamId = i + MARKETPRICE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&marketPriceItemInfoList[i].itemState))
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
		itemStreamId = i + MARKETPRICE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&marketPricePSItemInfoList[i].itemState))
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
 * Publically visable market price request function 
 *
 * Sends item requests to a channel.  For each item, this
 * consists of getting a message buffer, encoding the item
 * request, and sending the item request to the server.
 * chnl - The channel to send an item request to
 */
RsslRet sendMarketPriceItemRequests(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
	int i;

	/* Do not send a request if there are no items in the item list. */
	if(itemCount == 0 && privateStreamItemCount == 0)
		return RSSL_RET_SUCCESS;

	/* check to see if the provider supports the market price domain */
	if(getSourceDirectoryCapabilities(RSSL_DMT_MARKET_PRICE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_MARKET_PRICE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as refresh and status messages are received */
		marketPriceItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		marketPriceItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as refresh and status messages are received */
		marketPricePSItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		marketPricePSItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
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
				if(encodeBatchItemRequest(chnl, msgBuf, MARKETPRICE_BATCH_STREAM_ID_START, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nMarket Price encodeBatchItemRequest() failed\n");
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
					if (encodeItemRequest(chnl, msgBuf, (i + MARKETPRICE_STREAM_ID_START), RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nMarket Price encodeItemRequest() failed\n");
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
				if(encodeBatchItemRequest(chnl, msgBuf, MARKETPRICE_BATCH_PRIVATE_STREAM_ID_START, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nMarket Price encodeBatchItemRequest() failed\n");
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
					if (encodeItemRequest(chnl, msgBuf, (i + MARKETPRICE_PRIVATE_STREAM_ID_START), RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nMarket Price encodeItemRequest() failed\n");
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
 * Publically visable market price response handler 
 *
 * Processes a market price response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * field list and field entry, and calling decodeFieldEntry() to decode
 * the field entry data.
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processMarketPriceResponse(RsslChannel *chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslBool isBatchRequest = RSSL_FALSE;
	RsslBool isPrivateStream = RSSL_FALSE;
	MarketPriceItemInfo *itemInfo = NULL;
	char postUserAddrString[16];

	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	// determine which item in the info list we received a response for
	if (msg->msgBase.streamId == MARKETPRICE_BATCH_STREAM_ID_START ||
		msg->msgBase.streamId == MARKETPRICE_BATCH_PRIVATE_STREAM_ID_START)
	{
		isBatchRequest = RSSL_TRUE;	// we dont have info list entries for a status response to a batch request
	}
	else if (msg->msgBase.streamId >= MARKETPRICE_PRIVATE_STREAM_ID_START)
	{
		isPrivateStream = RSSL_TRUE;
		itemInfo = &marketPricePSItemInfoList[msg->msgBase.streamId - MARKETPRICE_PRIVATE_STREAM_ID_START];
	}
	else if (msg->msgBase.streamId >= MARKETPRICE_STREAM_ID_START)
	{
		itemInfo = &marketPriceItemInfoList[msg->msgBase.streamId - MARKETPRICE_STREAM_ID_START];
	}

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			if (!(msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", msg->msgBase.streamId);
					/* close stream */
					if (closeMPItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketPriceItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			/* update our item state list if its a refresh, then process just like update */
			itemInfo->itemState.dataState = msg->refreshMsg.state.dataState;
			itemInfo->itemState.streamState = msg->refreshMsg.state.streamState;

			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:

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

				/* The Visible Publisher Identity (VPI) can be found within the RsslPostUserInfo. 
				 * This will provide both the publisher ID and publisher address. Consumer can obtain the information from the msg - The partially decoded message.
				 */
				if (msg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
				{
					rsslIPAddrUIntToString(msg->updateMsg.postUserInfo.postUserAddr, postUserAddrString);
					printf("Received UpdateMsg for stream %i ", itemInfo->streamId);
					printf("from publisher with user ID: \"%u\" at user address: \"%s\"\n", msg->updateMsg.postUserInfo.postUserId, postUserAddrString);
				}
			}
			else if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);

				/* The Visible Publisher Identity (VPI) can be found within the RsslPostUserInfo. 
				 * This will provide both the publisher ID and publisher address. Consumer can obtain the information from the msg - The partially decoded message.
				 */
				if (msg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
				{
					rsslIPAddrUIntToString(msg->refreshMsg.postUserInfo.postUserAddr, postUserAddrString);
					printf("\nReceived RefreshMsg for stream %i ", itemInfo->streamId);
					printf("from publisher with user ID: \"%u\" at user address: \"%s\"\n", msg->refreshMsg.postUserInfo.postUserId, postUserAddrString);
				}
			}


			/* decode market price response */
			/* decode field list */
			if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) != RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}
			/* decode each field entry in list */
			while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret != RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeFieldEntry() failed with return code: %d\n", ret);
					return RSSL_RET_FAILURE;
				}
				/* decode field entry info */
				if (decodeFieldEntry(&fEntry, dIter) != RSSL_RET_SUCCESS)
				{
					printf("\ndecodeFieldEntry() failed\n");
					return RSSL_RET_FAILURE;
				}
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
					if (closeMPItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeMarketPriceItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
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
				/* The Visible Publisher Identity (VPI) can be found within the RsslPostUserInfo. 
				 * This will provide both the publisher ID and publisher address. Consumer can obtain the information from the msg - The partially decoded message.
				 */
				if (msg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO)
				{
					rsslIPAddrUIntToString(msg->statusMsg.postUserInfo.postUserAddr, postUserAddrString);
					printf("Received StatusMsg for stream %i ", itemInfo->streamId);
					printf("from publisher with user ID: \"%u\" at user address: \"%s\"\n", msg->statusMsg.postUserInfo.postUserId, postUserAddrString);
				}

				/* redirect to private stream if indicated */
				if (msg->statusMsg.state.streamState == RSSL_STREAM_REDIRECTED &&
					(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM))
				{
    				if (redirectToMarketPricePrivateStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}
			break;

		case RSSL_MC_ACK:
			printf("\nReceived AckMsg for stream %i \n", msg->msgBase.streamId);

			/* print out item name from key if it has it */
			/* if we sent an off stream post, we may not have created an itemInfo for it */
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
 * Publically visable - used by all non-admin domain handlers to output field lists
 *
 * Decodes the field entry data and prints out the field entry data
 * with help of the dictionary.  Returns success if decoding succeeds
 * or failure if decoding fails.
 * fEntry - The field entry data
 * dIter - The decode iterator
 */
RsslRet decodeFieldEntry(RsslFieldEntry* fEntry, RsslDecodeIterator *dIter)
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
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* get dictionary entry */
	if (!dictionary->entriesArray)
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
				printf(""RTR_LLU"\n", fidUIntValue);
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
				printf(""RTR_LLD"\n", fidIntValue);
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
 * Publically visable - used to close all market price streams
 *
 * Close all item streams.
 * chnl - The channel to send an item close to
 */
RsslRet closeMarketPriceItemStreams(RsslChannel* chnl)
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
				printf("\nMarket Price encodeBatchItemClose() failed\n");
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
			printf("\nConnected Provider does not support Batch Closes. Sending Market Price closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < itemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&marketPriceItemInfoList[i].itemState))
			{
				if (closeMPItemStream(chnl, i + MARKETPRICE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < privateStreamItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&marketPricePSItemInfoList[i].itemState))
			{
				if (closeMPItemStream(chnl, i + MARKETPRICE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
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
RsslRet redirectToMarketPricePrivateStream(RsslChannel *chnl, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* add item name to private stream list */
	addMarketPriceItemName(marketPriceItemInfoList[streamId - MARKETPRICE_STREAM_ID_START].itemname, RSSL_TRUE);

	/* remove non-private stream entry from list */
	removeMarketPriceItemEntry(chnl, streamId, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode item request */
		if (encodeItemRequest(chnl, msgBuf, nextPrivateStreamId - 1, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nMarket Price encodeItemRequest() failed\n");
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

