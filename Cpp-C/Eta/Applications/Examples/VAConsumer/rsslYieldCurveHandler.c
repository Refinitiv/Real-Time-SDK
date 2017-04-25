/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/


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
#include "rsslVASendMessage.h"
#include "rsslVACacheHandler.h"

#include "rtr/rsslPayloadEntry.h"

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE;

/* Counts nesting inside containers */
static short indentCount = 0;

/* used to print the indents */
static const char* indents [] = {"", "    ", "        ", "            "};

/* Decodes and prints the Yield Curve payload. */
RsslRet decodeYieldCurvePayload(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb)
{
	indentCount = 0;
	return decodeYieldCurveFieldList(dictionary, dIter, fieldSetDefDb);
}

/*
 * Removes an entry from the item name list.
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure containing the Stream id of item to remove
 * isPrivateStream - Flag for private stream
 */
static void removeYieldCurveItemEntry(ChannelCommand *pCommand, ItemRequest *pRequest, RsslBool isPrivateStream)
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
		for (i = pRequest->streamId - YIELD_CURVE_STREAM_ID_START; i < pCommand->yieldCurveItemCount - 1; i++)
		{
			strncpy(pCommand->yieldCurveItems[i].itemNameString, pCommand->yieldCurveItems[i+1].itemNameString, 128);
			pCommand->yieldCurveItems[i].itemName = pCommand->yieldCurveItems[i+1].itemName;
			pCommand->yieldCurveItems[i].streamId = pCommand->yieldCurveItems[i+1].streamId;
			pCommand->yieldCurveItems[i].itemState = pCommand->yieldCurveItems[i+1].itemState;
			pCommand->yieldCurveItems[i].cacheEntryHandle = pCommand->yieldCurveItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableYieldCurveStreamId;
		--pCommand->yieldCurveItemCount;
	}
	else /* private stream */
	{
		for (i = pRequest->streamId - YIELD_CURVE_PRIVATE_STREAM_ID_START; i < pCommand->privateStreamYieldCurveItemCount - 1; i++)
		{
			strncpy(pCommand->yieldCurvePSItems[i].itemNameString, pCommand->yieldCurvePSItems[i+1].itemNameString, 128);
			pCommand->yieldCurvePSItems[i].itemName = pCommand->yieldCurvePSItems[i+1].itemName;
			pCommand->yieldCurvePSItems[i].streamId = pCommand->yieldCurvePSItems[i+1].streamId;
			pCommand->yieldCurvePSItems[i].itemState = pCommand->yieldCurvePSItems[i+1].itemState;
			pCommand->yieldCurvePSItems[i].cacheEntryHandle = pCommand->yieldCurvePSItems[i+1].cacheEntryHandle;
		}
		--pCommand->nextAvailableYieldCurvePrivateStreamId;
		--pCommand->privateStreamYieldCurveItemCount;
	}
}


/*
 * Encodes the yield curve item close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the yield curve item close
 *
 * This function is only used within the Yield Curve Handler
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
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
 * Close an item stream for a specific stream id for Yield Curve Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * pRequest - Item request structure containing the stream id of the item close
 */
static RsslRet closeYCItemStream(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, ItemRequest *pRequest)
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
			printf("\nYield Curve encodeItemClose() failed\n");
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
 * Close an item stream for a specific stream id for Yield Curve Domain.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item close to
 * pCommand - the ChannelCommand to send an item close to
 * streamId - The stream id of the item close
 */
static RsslRet closeYCItemByStreamId(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, ChannelCommand *pCommand, RsslInt32 streamId)
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
			printf("\nYield Curve encodeItemClose() failed\n");
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
void setYCSnapshotRequest()
{
	snapshotRequest = RSSL_TRUE;
} 

/*
 * Encodes the yield curve item request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * pCommand - the ChannelCommand to send an item request to
 * pRequest - Item request structure
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Yield Curve Handler
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
 * chnl - The channel to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * streamId - The stream id of the item request
 * isPrivateStream - Flag for private stream request
 *
 * This function is only used within the Yield Curve Handler
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
		for(i = 0; i < pCommand->yieldCurveItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->yieldCurveItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else /* private stream */
	{
		// Start streamID at +1
		for(i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, &pCommand->yieldCurvePSItems[i].itemName, 0)) < RSSL_RET_SUCCESS)
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
 * This function is only used within the Yield Curve Handler
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
	msg.msgBase.domainType = RSSL_DMT_YIELD_CURVE;
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
	for(i = 0; i < pCommand->yieldCurveItemCount; i++)
	{
		itemStreamId = i + YIELD_CURVE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->yieldCurveItems[i].itemState))
		{
			if((ret = rsslEncodeArrayEntry(&encodeIter, 0, (void*)(&itemStreamId))) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	for(i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
	{
		itemStreamId = i + YIELD_CURVE_PRIVATE_STREAM_ID_START;

		/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
		if (!rsslIsFinalState(&pCommand->yieldCurvePSItems[i].itemState))
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
static RsslRet decodePrimitive(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslDataType dataType, RsslBool isArray)
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
				printf(RTR_LLD, fidIntValue);
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
			if (decodeArray(dictionary, dIter) != RSSL_RET_SUCCESS)
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
RsslRet decodeArray(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter)
{
	RsslRet retVal = 0;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayBuffer;
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
				if (decodePrimitive(dictionary, dIter, rsslArray.primitiveType, RSSL_TRUE) != RSSL_RET_SUCCESS)
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
RsslRet decodeYieldCurveFieldList(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb)
{
	RsslRet ret = 0;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
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
						if (decodeVector(dictionary, dIter) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
						break;
					case RSSL_DT_ARRAY:
						if (decodeArray(dictionary, dIter) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
						break;
					default:
						/* decode field entry info */
						if (decodePrimitive(dictionary, dIter, dataType, RSSL_FALSE) != RSSL_RET_SUCCESS)
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
static RsslRet decodeVector(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter)
{
	RsslRet retVal = 0;
	RsslRet retValVector = 0;
	const char* actionString;
	RsslLocalFieldSetDefDb fieldSetDefDb;	
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslVector rsslVector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
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
		if (decodeYieldCurveFieldList(dictionary, dIter, 0) != RSSL_RET_SUCCESS)
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
				if (decodeYieldCurveFieldList(dictionary, dIter, &fieldSetDefDb) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			case RSSL_DT_ARRAY:
				if (decodeArray(dictionary, dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				// TODO move this to the primitive decoder. Array is not a container
				break;
			default:
				printf("Error: Vector contained unhandled containerType %d.\n", 
						rsslVector.containerType);
				break;
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
RsslRet sendYieldCurveItemRequests(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);
	int i;

	// Do not send a request if there are no items in the item list.
	if(pCommand->yieldCurveItemCount == 0 && pCommand->privateStreamYieldCurveItemCount == 0)
		return RSSL_RET_SUCCESS;

	/* check to see if the provider supports the yield curve domain */
	if(getSourceDirectoryCapabilities(pCommand, RSSL_DMT_YIELD_CURVE) == RSSL_FALSE)
	{
		printf("RSSL_DMT_YIELD_CURVE is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	/* send out two sets of requests - one for private stream and one for non-private stream */

	/* non-private stream request(s) */
	if (pCommand->yieldCurveItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->yieldCurveItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->yieldCurveBatchRequest.streamId = YIELD_CURVE_BATCH_STREAM_ID_START;
				for (i = 0; i < pCommand->yieldCurveItemCount; i++)
					pCommand->yieldCurveItems[i].streamId = getNextAvailableYieldCurveStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->yieldCurveBatchRequest.streamId, RSSL_FALSE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nYield Curve encodeBatchItemRequest() failed\n");
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
			if (pCommand->yieldCurveItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests.  Sending Yield Curve requests as individual request messages.\n");
			for (i = 0; i < pCommand->yieldCurveItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->yieldCurveItems[i].streamId = getNextAvailableYieldCurveStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, pCommand, &pCommand->yieldCurveItems[i], RSSL_FALSE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nYield Curve encodeItemRequest() failed\n");
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
	if (pCommand->privateStreamYieldCurveItemCount > 0)
	{
		/* If there is only one item in the itemList, it is a waste of bandwidth to send a batch request */
		if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) && (pCommand->privateStreamYieldCurveItemCount > 1))
		{
			msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
		
			if (msgBuf != NULL)
			{
				pCommand->privateStreamYieldCurveBatchRequest.streamId = YIELD_CURVE_BATCH_PRIVATE_STREAM_ID_START;
				for (i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
					pCommand->yieldCurvePSItems[i].streamId = getNextAvailableYieldCurvePrivateStreamId(pCommand);

				if(encodeBatchItemRequest(pReactorChannel, msgBuf, pCommand->privateStreamYieldCurveBatchRequest.streamId, RSSL_TRUE) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
					printf("\nYield Curve encodeBatchItemRequest() failed\n");
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
			if (pCommand->privateStreamYieldCurveItemCount > 1)
				printf("\nConnected Provider does not support Batch Requests. Sending Private Stream Yield Curve requests as individual request messages.\n");

			for (i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
			{
				/* get a buffer for the item request */
				msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

				if (msgBuf != NULL)
				{
					pCommand->yieldCurvePSItems[i].streamId = getNextAvailableYieldCurvePrivateStreamId(pCommand);

					/* encode item request */
					if (encodeItemRequest(pReactorChannel, msgBuf, pCommand, &pCommand->yieldCurvePSItems[i], RSSL_TRUE) != RSSL_RET_SUCCESS)
					{
						rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
						printf("\nYield Curve encodeItemRequest() failed\n");
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
 * Publically visible yield curve response processing function 
 *
 * Processes a yield curve response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * field list and field entry, and calling decodePrimitive() to decode
 * the entry data.
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processYieldCurveResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
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
	int i;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	ItemRequest *pRequest = NULL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslBool isBatchRequest = RSSL_FALSE;
	RsslBool isPrivateStream = RSSL_FALSE;

	tempBuffer.data = tempData;
	tempBuffer.length = sizeof(tempData);

	indentCount = 0;

	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	if (msg->msgBase.streamId != 0)
	{
		/* first check if it is a response for non-private stream yield curve item request */
		for(i = 0; i < pCommand->yieldCurveItemCount; ++i)
		{
			if (pCommand->yieldCurveItems[i].streamId == msg->msgBase.streamId)
			{
				pRequest = &pCommand->yieldCurveItems[i];
				break;
			}
		}
		/* then check if it is a response for private stream yield curve item request */
		if (!pRequest)
		{
			for(i = 0; i < pCommand->privateStreamYieldCurveItemCount; ++i)
			{
				if (pCommand->yieldCurvePSItems[i].streamId == msg->msgBase.streamId)
				{
					pRequest = &pCommand->yieldCurvePSItems[i];
					isPrivateStream = RSSL_TRUE;
					break;
				}
			}
		}
		/* then check if it is a response for yield curve batch request */
		if (!pRequest)
		{
			if (msg->msgBase.streamId == pCommand->yieldCurveBatchRequest.streamId)
			{
				pRequest = &pCommand->yieldCurveBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if (msg->msgBase.streamId == pCommand->privateStreamYieldCurveBatchRequest.streamId)
			{
				pRequest = &pCommand->privateStreamYieldCurveBatchRequest;
				isBatchRequest = RSSL_TRUE;
			}
			else if(pCommand->shouldOffStreamPost != RSSL_TRUE)
			{
				printf("Received yield curve message on unknown stream %d\n", msg->msgBase.streamId);
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

			for(i = 0; i < pCommand->yieldCurveItemCount; ++i)
			{
                if (pCommand->yieldCurveItems[i].itemName.length == key->name.length &&
                    memcmp(pCommand->yieldCurveItems[i].itemName.data, key->name.data, key->name.length) == 0)
				{
					pRequest = &pCommand->yieldCurveItems[i];
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
			/* update our item state list if it's a refresh, then process just like update */

			if (!(msg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM)) /* non-private stream */
			{
				/* check if this response should be on private stream but is not */
				/* if this is the case, close the stream */
				if (msg->msgBase.streamId >= YIELD_CURVE_PRIVATE_STREAM_ID_START)
				{
					printf("\nReceived non-private response for stream %i that should be private - closing stream\n", msg->msgBase.streamId);
					/* close stream */
					if (closeYCItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;

					/* remove private stream entry from list */
					removeYieldCurveItemEntry(pCommand, pRequest, RSSL_TRUE);
					return RSSL_RET_SUCCESS;
				}
			}

			pRequest->itemState.dataState = msg->refreshMsg.state.dataState;
			pRequest->itemState.streamState = msg->refreshMsg.state.streamState;

			setGroupId(pCommand, pRequest, &msg->refreshMsg.groupId);
			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:
			/* decode yield curve response */

			if (pCommand->cacheInfo.useCache)
				applyMsgToCache(&pRequest->cacheEntryHandle, &pCommand->cacheInfo, msg, dIter);

			/* Print descriptor of the channel this message was received from. */
			printf("\n(Channel %d): ", pReactorChannel->socketId);

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
				printf("%.*s", tempBuffer.length, tempBuffer.data);
			}

			if (pCommand->cacheInfo.useCache == RSSL_FALSE)
			{
				if (decodeYieldCurveFieldList(&((ChannelCommand*)pReactorChannel->userSpecPtr)->dictionary, dIter, &localFieldSetDefDb) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
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
					if (closeYCItemStream(pReactor, pReactorChannel, pCommand, pRequest) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					/* remove private stream entry from list */
					removeYieldCurveItemEntry(pCommand, pRequest, RSSL_TRUE);
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
 * Publically visable - used to close all yield curve streams
 *
 * Close all item streams.
 * pReactor - RsslReactor associated with the application
 * pCommand - the ChannelCommand to send an item close to
 */
RsslRet closeYieldCurveItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslInt32 batchCloseStreamId;
	int i;

	RsslReactorChannel *pReactorChannel = pCommand->reactorChannel;
	RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

	if (!pReactorChannel)
		return RSSL_RET_SUCCESS;

	/* If provider supports batch close, will use batch close */
	if((loginInfo->supportBatchRequests & RDM_LOGIN_BATCH_SUPPORT_CLOSES) && (pCommand->yieldCurveItemCount + pCommand->privateStreamYieldCurveItemCount) > 1)
	{
		/* get a buffer for the item close */
		msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			batchCloseStreamId = getNextAvailableYieldCurveStreamId(pCommand);

			/* encode batch close */
			if (encodeBatchItemClose(pReactorChannel, msgBuf, batchCloseStreamId) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
				printf("\nYield Curve encodeBatchItemClose() failed\n");
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
		if ((pCommand->yieldCurveItemCount + pCommand->privateStreamYieldCurveItemCount) > 1)
			printf("\nConnected Provider does not support Batch Closes. Sending Yield Curve closes as individual close messages.\n");

		/* non-private streams */
		for (i = 0; i < pCommand->yieldCurveItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->yieldCurveItems[i].itemState))
			{
				if (closeYCItemByStreamId(pReactor, pReactorChannel, pCommand, i + YIELD_CURVE_STREAM_ID_START) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}

		/* private streams */
		for (i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
		{
			/* we only want to close a stream if it was not already closed (e.g. rejected by provider, closed via refresh or status, or redirected) */
			if (!rsslIsFinalState(&pCommand->yieldCurvePSItems[i].itemState))
			{
				if (closeYCItemByStreamId(pReactor, pReactorChannel, pCommand, i + YIELD_CURVE_PRIVATE_STREAM_ID_START) != RSSL_RET_SUCCESS)
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
	if (pCommand->privateStreamYieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
	{
		pItemRequest = &pCommand->yieldCurvePSItems[pCommand->privateStreamYieldCurveItemCount];
		++pCommand->privateStreamYieldCurveItemCount;
	}
	else
	{
		printf("Number of Private Stream Yield Curve items exceeded\n");
	}

	pItemRequest->streamId = getNextAvailableYieldCurvePrivateStreamId(pCommand);

	// save the itemName for redirecting to Private Stream 
	snprintf(pItemRequest->itemNameString, 128, "%s", pRequest->itemNameString);
	pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
	pItemRequest->itemName.data = pItemRequest->itemNameString; 

	/* remove non-private stream entry from list */
	removeYieldCurveItemEntry(pCommand, pRequest, RSSL_FALSE);

	/* get a buffer for the item request */
	msgBuf = rsslReactorGetBuffer(pCommand->reactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode item request */
        if (encodeItemRequest(pCommand->reactorChannel, msgBuf, pCommand, pItemRequest, RSSL_TRUE) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pCommand->reactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nMarket By Order encodeItemRequest() failed\n");
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
