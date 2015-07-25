
/*
 * This is the yield curve handler for the rsslConsumer application.
 * It provides functions for sending the yield curve request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * vectors containing field lists and/or arrays, decoding field entries from a response,
 * closing yield curve streams, getting the next stream id, and
 * adding/removing items to/from the item list are also provided.
 */

#include "rsslYieldCurveHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslSendMessage.h"

/* non-private stream id starting point */
#define YIELD_CURVE_STREAM_ID_START 309
#define YIELD_CURVE_BATCH_STREAM_ID_START 308

/* private stream id starting point */
#define YIELD_CURVE_PRIVATE_STREAM_ID_START (YIELD_CURVE_STREAM_ID_START + 50)
#define YIELD_CURVE_BATCH_PRIVATE_STREAM_ID_START (YIELD_CURVE_BATCH_STREAM_ID_START + 50)

/* stream yield curve item info list */
static YieldCurveItemInfo yieldCurveItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* private stream yield curve item info list */
static YieldCurveItemInfo yieldCurvePSItemInfoList[MAX_STREAM_ID_RANGE_PER_DOMAIN/2];

/* next non-private stream id for item requests */
static RsslInt32 nextStreamId = YIELD_CURVE_STREAM_ID_START;

/* next private stream id for item requests */
static RsslInt32 nextPrivateStreamId = YIELD_CURVE_PRIVATE_STREAM_ID_START;

/* number of items in non-private stream item list */
static RsslInt32 itemCount = 0;

/* number of items in private stream item list */
static RsslInt32 privateStreamItemCount = 0;

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/* Counts nesting inside containers */
static short indentCount = 0;

/* used to print the indents */
static const char* indents [] = {"", "    ", "        ", "            "};

/*
 * Publically visible method used to add a yield curve name to the request list
 * Adds an item name requested by the application to
 * the item name list.
 * itemname - Item name requested by the application
 * isPrivateStream - Flag for private stream
 */
void addYieldCurveItemName(char* itemname, RsslBool isPrivateStream)
{
	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		if (itemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(yieldCurveItemInfoList[nextStreamId - YIELD_CURVE_STREAM_ID_START].itemname, 128, "%s", itemname);
			yieldCurveItemInfoList[nextStreamId - YIELD_CURVE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			yieldCurveItemInfoList[nextStreamId - YIELD_CURVE_STREAM_ID_START].streamId = 0;
			++nextStreamId;
			++itemCount;
		}
		else
		{
			printf("Number of Yield Curve items exceeded\n");
		}
	}
	else /* private stream */
	{
		if (privateStreamItemCount < MAX_STREAM_ID_RANGE_PER_DOMAIN/2)
		{
			snprintf(yieldCurvePSItemInfoList[nextPrivateStreamId - YIELD_CURVE_PRIVATE_STREAM_ID_START].itemname, 128, "%s", itemname);
			yieldCurvePSItemInfoList[nextPrivateStreamId - YIELD_CURVE_PRIVATE_STREAM_ID_START].nameLength = (RsslUInt32)strlen(itemname);
			yieldCurvePSItemInfoList[nextPrivateStreamId - YIELD_CURVE_PRIVATE_STREAM_ID_START].streamId = 0;
			++nextPrivateStreamId;
			++privateStreamItemCount;
		}
		else
		{
			printf("Number of Private Stream Yield Curve items exceeded\n");
		}
	}
}

/*
 * Removes an entry from the item name list.
 * streamId - Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeYieldCurveItemEntry(RsslChannel *chnl, RsslInt32 streamId, RsslBool isPrivateStream)
{	
	int i;

	/* take the next streamId and increment it */
	if (!isPrivateStream) /* non-private stream */
	{
		for (i = streamId - YIELD_CURVE_STREAM_ID_START; i < itemCount - 1; i++)
		{
			strncpy(yieldCurveItemInfoList[i].itemname, yieldCurveItemInfoList[i+1].itemname, 129);
			yieldCurveItemInfoList[i].nameLength = yieldCurveItemInfoList[i+1].nameLength;
			yieldCurveItemInfoList[i].itemState = yieldCurveItemInfoList[i+1].itemState;
			yieldCurveItemInfoList[i].streamId = yieldCurveItemInfoList[i+1].streamId;
		}
		--nextStreamId;
		--itemCount;
	}
	else /* private stream */
	{
		for (i = streamId - YIELD_CURVE_PRIVATE_STREAM_ID_START; i < privateStreamItemCount - 1; i++)
		{
			strncpy(yieldCurvePSItemInfoList[i].itemname, yieldCurvePSItemInfoList[i+1].itemname, 129);
			yieldCurvePSItemInfoList[i].nameLength = yieldCurvePSItemInfoList[i+1].nameLength;
			yieldCurvePSItemInfoList[i].itemState = yieldCurvePSItemInfoList[i+1].itemState;
			yieldCurvePSItemInfoList[i].streamId = yieldCurvePSItemInfoList[i+1].streamId;
		}
		--nextPrivateStreamId;
		--privateStreamItemCount;
	}
}


/*
 * Encodes the yield curve item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the yield curve item close
 *
 * This function is only used within the Yield Curve Handler
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
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
static RsslRet closeYCItemStream(RsslChannel* chnl, RsslInt32 streamId)
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
			printf("\nYield Curve encodeItemClose() failed\n");
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
 * Encodes the yield curve item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the item request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Yield Curve Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeItemRequest(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId,  RsslBool isPrivateStream)
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
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
		listIndex = streamId - YIELD_CURVE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = yieldCurveItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = yieldCurveItemInfoList[listIndex].nameLength;
	}
	else /* private stream */
	{
		listIndex = streamId - YIELD_CURVE_PRIVATE_STREAM_ID_START;
		msg.msgBase.msgKey.name.data = yieldCurvePSItemInfoList[listIndex].itemname;
		msg.msgBase.msgKey.name.length = yieldCurvePSItemInfoList[listIndex].nameLength;
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
		yieldCurveItemInfoList[listIndex].streamId = streamId;
	else
		yieldCurvePSItemInfoList[listIndex].streamId = streamId;

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
 * This function is only used within the Yield Curve Handler
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
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
			itemName.data = yieldCurveItemInfoList[i].itemname;
			itemName.length = yieldCurveItemInfoList[i].nameLength;
			yieldCurveItemInfoList[i].streamId = ++streamId;
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
			itemName.data = yieldCurvePSItemInfoList[i].itemname;
			itemName.length = yieldCurvePSItemInfoList[i].nameLength;
			yieldCurvePSItemInfoList[i].streamId = ++streamId;
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
 * This function is only used within the Yield Curve Handler
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

	RsslInt32 itemStreamId;
	int i;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	rsslClearCloseMsg(&msg);
	msg.msgBase.msgClass  = RSSL_MC_CLOSE;
	msg.msgBase.streamId = batchCloseStreamId;
	msg.flags = RSSL_CLMF_HAS_BATCH;
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
		itemStreamId = i + YIELD_CURVE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&yieldCurveItemInfoList[i].itemState))
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
		itemStreamId = i + YIELD_CURVE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&yieldCurvePSItemInfoList[i].itemState))
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
 * Decodes the Primitive data types inside the Yield Curve message.
 * Returns success if decoding succeeds
 * or failure if decoding fails.
 * dIter - The decode iterator
 * dataType - RsslDataType
 * isArray - boolean used to indicate if the element is inside the Array
 */
static RsslRet decodePrimitive(RsslDecodeIterator *dIter, RsslDataType dataType, RsslBool isArray)
{
	RsslRet ret = 0;
	RsslReal fidRealValue = RSSL_INIT_REAL;
	RsslInt64 fidIntValue = 0;
	RsslDateTime fidDateTimeValue;
	RsslBuffer fidBufferValue;
	RsslBuffer fidDateTimeBuf;
	RsslBuffer fidRealBuf;

	switch (dataType)
	{
		case RSSL_DT_INT:
			if ((ret = rsslDecodeInt(dIter, &fidIntValue)) == RSSL_RET_SUCCESS)
				printf(""RTR_LLD"", fidIntValue);
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeInt() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_REAL:
			if ((ret = rsslDecodeReal(dIter, &fidRealValue)) == RSSL_RET_SUCCESS)
			{
				fidRealBuf.data = (char*)alloca(35);
				fidRealBuf.length = 35;
				rsslRealToString(&fidRealBuf, &fidRealValue);
				printf("%s", fidRealBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeReal() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATE:
			if ((ret = rsslDecodeDate(dIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
				printf("%s", fidDateTimeBuf.data);
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
				printf("%s", fidDateTimeBuf.data);
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
				printf("%s", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDateTime() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_ARRAY:
			if (decodeArray(dIter) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		case RSSL_DT_BUFFER:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			if((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
			{
				if (isArray)
					printf("\"");
				printf("%.*s", fidBufferValue.length, fidBufferValue.data);
				if (isArray)
					printf("\"");
			}
			else if (ret != RSSL_RET_BLANK_DATA) 
			{
				printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		default:
			printf("Unsupported data type (%d)", dataType);
			break;
	}

	if (ret == RSSL_RET_BLANK_DATA)
		printf("<blank data>");

	if(!isArray)
		printf("\n");

	return RSSL_RET_SUCCESS;

}

/*
 * Decodes the Array inside the Yield Curve message.
 * Returns success if decoding succeeds
 * or failure if decoding fails.
 * dIter - The decode iterator
 */
RsslRet decodeArray(RsslDecodeIterator *dIter)
{
	RsslRet retVal = 0;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayBuffer;
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslBool firstArrayEntry = RSSL_TRUE;
		
	printf("{ ");

	/* decode into the array structure header */
	if ((retVal = rsslDecodeArray(dIter, &rsslArray)) >= RSSL_RET_SUCCESS)
	{
		RsslRet retValArray = 0;
		/* decode each array entry  */
		while ((retValArray = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retValArray < RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeArrayEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retValArray), retValArray, rsslRetCodeInfo(retValArray));
				return retValArray;
			} 
			else 
			{
				if (firstArrayEntry)
					firstArrayEntry = RSSL_FALSE;
				else
					printf(", ");
				if (decodePrimitive(dIter,rsslArray.primitiveType, RSSL_TRUE) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}
	else
	{
		/* decoding failure tends to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeArray().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}

	printf(" }\n");

	return retVal;
}

/*
 * Decodes the Field List inside the Yield Curve message.
 * Returns success if decoding succeeds
 * or failure if decoding fails.
 * dIter - The decode iterator
 */
static RsslRet decodeFieldList(RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb)
{
	RsslRet ret = 0;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslDataType dataType = RSSL_DT_UNKNOWN;

	indentCount++;
		
	printf("\n");

	/* decode field list */
	if ((ret = rsslDecodeFieldList(dIter, &fList, fieldSetDefDb)) == RSSL_RET_SUCCESS)
	{
		RsslRet retValFL = 0;
		/* decode each field entry in list */
		while ((retValFL = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retValFL < RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeFieldEntry().  Error Text: %s\n",
						rsslRetCodeToString(retValFL), retValFL, rsslRetCodeInfo(retValFL));
				return retValFL;
			}
			else
			{
				/* get dictionary entry */
				if (!dictionary->entriesArray)
				{
					dumpHexBuffer(&fEntry.encData);
					return RSSL_RET_SUCCESS;
				}
				else
					dictionaryEntry = dictionary->entriesArray[fEntry.fieldId];

				/* return if no entry found */
				if (!dictionaryEntry) 
				{
					printf("\tFid %d not found in dictionary\n", fEntry.fieldId);
					dumpHexBuffer(&fEntry.encData);
					return RSSL_RET_SUCCESS;
				}

				/* print out fid name */
				printf("%s%-*s", indents[indentCount], 40 - indentCount * 4, dictionaryEntry->acronym.data);
				/* decode and print out fid value */
				dataType = dictionaryEntry->rwfType;

				/* decode field entry info */
				switch(dataType)
				{
					case RSSL_DT_VECTOR:
						if (decodeVector(dIter) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
						break;
					case RSSL_DT_ARRAY:
						if (decodeArray(dIter) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
						break;
					default:
						/* decode field entry info */
						if (decodePrimitive(dIter, dataType, RSSL_FALSE) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
						break;
				}
			}
		}
	}

	indentCount--;

	return ret;
}
/*
 * Decodes the Vector inside the Yield Curve message.
 * Returns success if decoding succeeds
 * or failure if decoding fails.
 * dIter - The decode iterator
 */
static RsslRet decodeVector(RsslDecodeIterator *dIter)
{
	RsslRet retVal = 0;
	RsslRet retValVector = 0;
	const char* actionString;
	RsslLocalFieldSetDefDb fieldSetDefDb;	
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslVector rsslVector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	
	/* decode contents into the vector structure */
	/* decode our vector header */
	if ((retVal = rsslDecodeVector(dIter, &rsslVector)) < RSSL_RET_SUCCESS)
	{
		/* decoding failures tend to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}

	if (retVal == RSSL_RET_NO_DATA)
	{
		printf("<no data>\n");
		return RSSL_RET_SUCCESS;
	}

	if (rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA)
	{
		if (decodeFieldList(dIter, 0) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	/* If the vector flags indicate that set definition content is present, decode the set def db */
	if (rsslVector.flags & RSSL_VTF_HAS_SET_DEFS)
	{
		/* must ensure it is the correct type - if map contents are field list, this is a field set definition db */
		if (rsslVector.containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&fieldSetDefDb);
			if ((retVal = rsslDecodeLocalFieldSetDefDb(dIter, &fieldSetDefDb)) < RSSL_RET_SUCCESS)
			{
				/* decoding failures tend to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeLocalFieldSetDefDb().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
				return retVal;
			}		
		}
	}

	indentCount++;

	printf("\n");

	/* decode each vector entry */
	/* since this succeeded, we can decode fields until we reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
	while ((retValVector = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retValVector < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retValVector), retValVector, rsslRetCodeInfo(retValVector));
			return retValVector;
		}

		/* convert the action to a string for display purposes */
		switch(vectorEntry.action)
		{
			case RSSL_VTEA_UPDATE_ENTRY:
				actionString = "RSSL_VTEA_UPDATE_ENTRY"; 
				break;
			case RSSL_VTEA_SET_ENTRY:
				actionString = "RSSL_VTEA_SET_ENTRY";
				break;
			case RSSL_VTEA_CLEAR_ENTRY:
				actionString = "RSSL_VTEA_CLEAR_ENTRY";
				break;
			case RSSL_VTEA_INSERT_ENTRY:
				actionString = "RSSL_VTEA_INSERT_ENTRY";
				break;
			case RSSL_VTEA_DELETE_ENTRY:
				actionString = "RSSL_VTEA_DELETE_ENTRY";
				break;
			default:
				actionString = "Unknown";
				break;
		}

		printf("%sINDEX: %d\n", indents[indentCount], vectorEntry.index);
		printf("%sACTION: %s", indents[indentCount], actionString);

		switch (rsslVector.containerType)
		{
			case RSSL_DT_FIELD_LIST:
				if (decodeFieldList(dIter, &fieldSetDefDb) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			case RSSL_DT_ARRAY:
				if (decodeArray(dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				// TODO move this to the primitive decoder. Array is not a container
				break;
			default:
				printf("Error: Vector contained unhandled containerType %d.\n", 
						rsslVector.containerType);
				return RSSL_RET_FAILURE;
		}
	}

	indentCount--;

	return retVal;
}

/*
 * Publically visible yield curve request sending routine 
 *
 * Sends yield curve item requests to a channel.  For each item, this
 * consists of getting a message buffer, encoding the item
 * request, and sending the item request to the server.
 * chnl - The channel to send an item request to
 */
RsslRet sendYieldCurveItemRequests(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
	int i;

	// Do not send a request if there are no items in the item list.
	if(itemCount == 0 && privateStreamItemCount == 0)
		return RSSL_RET_SUCCESS;

	/* check to see if the provider supports the yield curve domain */
	if(getSourceDirectoryCapabilities(RSSL_DMT_YIELD_CURVE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_YIELD_CURVE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as we recieve refresh and status messages */
		yieldCurveItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		yieldCurveItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
	}

	for (i = 0; i < (MAX_STREAM_ID_RANGE_PER_DOMAIN/2) ; i++)
	{
		/* initialize state management array */
		/* these will be updated as we recieve refresh and status messages */
		yieldCurvePSItemInfoList[i].itemState.dataState = RSSL_DATA_NO_CHANGE;
		yieldCurvePSItemInfoList[i].itemState.streamState = RSSL_STREAM_UNSPECIFIED;
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
				if(encodeBatchItemRequest(chnl, msgBuf, YIELD_CURVE_BATCH_STREAM_ID_START, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nYield Curve encodeBatchItemRequest() failed\n");
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
				printf("\nConnected Provider does not support Batch Requests. Sending Yield Curve requests as individual request messages.\n");

			for (i = 0; i < itemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

				if (msgBuf != NULL)
				{
					/* encode item request */
					if (encodeItemRequest(chnl, msgBuf, (i + YIELD_CURVE_STREAM_ID_START), RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nYield Curve encodeItemRequest() failed\n");
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
				if(encodeBatchItemRequest(chnl, msgBuf, YIELD_CURVE_BATCH_PRIVATE_STREAM_ID_START, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error); 
					printf("\nYield Curve encodeBatchItemRequest() failed\n");
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
				printf("\nConnected Provider does not support Batch Requests. Sending Private Stream Yield Curve requests as individual request messages.\n");

			for (i = 0; i < privateStreamItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

				if (msgBuf != NULL)
				{
					/* encode item request */
					if (encodeItemRequest(chnl, msgBuf, (i + YIELD_CURVE_PRIVATE_STREAM_ID_START), RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nYield Curve encodeItemRequest() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item request */
					if (sendMessage(chnl, msgBuf)  != RSSL_RET_SUCCESS)
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
 * Publically visible yield curve response processing function 
 *
 * Processes a yield curve response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * field list and field entry, and calling decodePrimitive() to decode
 * the entry data.
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processYieldCurveResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key;
	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslVector rsslVector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslBuffer mapKey = RSSL_INIT_BUFFER;
	RsslLocalFieldSetDefDb localFieldSetDefDb;  // this must be cleared using the clear function below
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;	
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslBool isPrivateStream = RSSL_FALSE;
	YieldCurveItemInfo *itemInfo = NULL;
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslBool isBatchRequest = RSSL_FALSE;

	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	indentCount = 0;
	rsslClearLocalFieldSetDefDb(&localFieldSetDefDb);

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	// determine which item in the info list we received a response for
	if (msg->msgBase.streamId == YIELD_CURVE_BATCH_STREAM_ID_START ||
		msg->msgBase.streamId == YIELD_CURVE_BATCH_PRIVATE_STREAM_ID_START)
	{
		isBatchRequest = RSSL_TRUE;	// we dont have info list entries for a status response to a batch request
	}
	else if (msg->msgBase.streamId >= YIELD_CURVE_PRIVATE_STREAM_ID_START)
	{
		isPrivateStream = RSSL_TRUE;
		itemInfo = &yieldCurvePSItemInfoList[msg->msgBase.streamId - YIELD_CURVE_PRIVATE_STREAM_ID_START];
	}
	else if (msg->msgBase.streamId >= YIELD_CURVE_STREAM_ID_START)
	{
		itemInfo = &yieldCurveItemInfoList[msg->msgBase.streamId - YIELD_CURVE_STREAM_ID_START];
	}

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			/* update our item state list if it's a refresh, then process just like update */

			if (!(msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", itemInfo->streamId);
					/* close stream */
					if (closeYCItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					/* remove private stream entry from list */
					removeYieldCurveItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}
			
			/* update our item state list if its a refresh, then process just like update */
			itemInfo->itemState.dataState = msg->refreshMsg.state.dataState;
			itemInfo->itemState.streamState = msg->refreshMsg.state.streamState;

			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:
			/* decode yield curve response */

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
				printf("%.*s", tempBuffer.length, tempBuffer.data);
			}

			if (decodeFieldList(dIter, &localFieldSetDefDb) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			break;
				
		case RSSL_MC_STATUS:
			if (isBatchRequest)
				printf("\nReceived Batch StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);
			else
				printf("\nReceived Item StatusMsg for stream %i \n", itemInfo->streamId);

			/* update our state table with the new state */
			if (!(msg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (isPrivateStream)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", itemInfo->streamId);
					/* close stream */
					if (closeYCItemStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeYieldCurveItemEntry(chnl, itemInfo->streamId, RSSL_TRUE);
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
    				if (redirectToYieldCurvePrivateStream(chnl, itemInfo->streamId) != RSSL_RET_SUCCESS)
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
 * Publically visible Yield Curve close function
 *
 * Close all yield curve item streams.
 * chnl - The channel to send an item close to
 */
RsslRet closeYieldCurveItemStreams(RsslChannel* chnl)
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
				printf("\nYield Curve encodeBatchItemClose() failed\n");
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
			printf("\nConnected Provider does not support Batch Closes. Sending Yield Curve closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < itemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&yieldCurveItemInfoList[i].itemState))
			{
				if (closeYCItemStream(chnl, i + YIELD_CURVE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < privateStreamItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&yieldCurvePSItemInfoList[i].itemState))
			{
				if (closeYCItemStream(chnl, i + YIELD_CURVE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/*      
 * Publically visable - used to enable snapshot requesting 
 *      Send a set of items as a snapshot request.
 */

void setYCSnapshotRequest()
{
        snapshotRequest = RSSL_TRUE;
}

/*
 * Redirect a request to a private stream.
 * streamId - The stream id to be redirected to private stream
 */
RsslRet redirectToYieldCurvePrivateStream(RsslChannel* chnl, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* add item name to private stream list */
	addYieldCurveItemName(yieldCurveItemInfoList[streamId - YIELD_CURVE_STREAM_ID_START].itemname, RSSL_TRUE);

	/* remove non-private stream entry from list */
	removeYieldCurveItemEntry(chnl, streamId, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode item request */
		if (encodeItemRequest(chnl, msgBuf, nextPrivateStreamId - 1, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nYield Curve encodeItemRequest() failed\n");
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
