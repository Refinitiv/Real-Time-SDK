/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/


#include "rsslVAMarketPriceItems.h"
#include "rsslVASendMessage.h"

/* 
 * This handles the generation of VA example market price data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a market price message and payload.
 */

#define MAX_MARKET_PRICE_ITEM_LIST_SIZE 100

/* item information list */
static RsslMarketPriceItem marketPriceItemList[MAX_MARKET_PRICE_ITEM_LIST_SIZE];

/* re-usable refresh and update messages and state text */
RsslRefreshMsg refreshMsg;
RsslUpdateMsg updateMsg;
char stateText[MAX_ITEM_INFO_STRLEN];

RsslRet encodeMPFieldList(RsslItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslBool isPrivateStream, RsslDataDictionary* dictionary);

/*
 * Initializes market price item list
 */
void initMarketPriceItems()
{
	int i;

	/* clear item information list */
	for (i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
	{
		clearMarketPriceItem(&marketPriceItemList[i]);
	}
}

/*
 * Initializes market price item fields.
 * mpItem - The item to be initialized
 */
void initMarketPriceItemFields(RsslMarketPriceItem* mpItem)
{
	RsslBuffer tempBuffer;
	mpItem->isInUse = RSSL_TRUE;
	mpItem->RDNDISPLAY = 100;
	mpItem->RDN_EXCHID = 155;
	tempBuffer.data = (char *)"10/22/2010";
	tempBuffer.length = (RsslUInt32)strlen("10/22/2010");
	rsslDateStringToDate(&mpItem->DIVPAYDATE, &tempBuffer);
	mpItem->TRDPRC_1 = 1.00;
	mpItem->BID = 0.99;
	mpItem->ASK = 1.03;
	mpItem->ACVOL_1 = 100000;
	mpItem->NETCHNG_1 = 2.15;
	rsslDateTimeLocalTime(&mpItem->ASK_TIME);
	mpItem->PERATIO = 5.00;
	mpItem->SALTIME = mpItem->ASK_TIME;
	mpItem->SALTIME.time.second--;
}

/*
 * Updates any item that's currently in use.
 */
void updateMarketPriceItemFields(RsslMarketPriceItem* mpItem)
{
	mpItem->TRDPRC_1 += 0.01;
	mpItem->BID += 0.01;
	mpItem->ASK += 0.01;
	rsslDateTimeLocalTime(&mpItem->ASK_TIME);
	mpItem->PERATIO += 0.01;
	mpItem->SALTIME = mpItem->ASK_TIME;
	mpItem->SALTIME.time.second--;
	mpItem->updateCount++;
}

/*
 * Updates the item's data from the post we got
 */
RsslRet updateMarketPriceItemFieldsFromPost(RsslMarketPriceItem* mpItem, RsslDecodeIterator* dIter, RsslErrorInfo *error)
{
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tmpReal;
	RsslRet ret;

	/* decode field list */
	if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) != RSSL_RET_SUCCESS)
	{
		sprintf(error->rsslError.text, "rsslDecodeFieldList() failed with return code: %d\n", ret);
		error->rsslError.rsslErrorId = ret;		
		return ret;
	}

	/* decode each field entry in list and apply it to the market price item */
	while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			sprintf(error->rsslError.text, "rsslDecodeFieldEntry() failed with return code: %d\n", ret);
			error->rsslError.rsslErrorId = ret;		
			return ret;
		}
		switch(fEntry.fieldId)
		{
		case RDNDISPLAY_FID:
			if ((ret = rsslDecodeUInt(dIter, &mpItem->RDNDISPLAY)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding RDNDISPLAY", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			break;

		case RDN_EXCHID_FID:
			if ((ret = rsslDecodeEnum(dIter, &mpItem->RDN_EXCHID)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding RDN_EXCHID", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			break;

		case DIVPAYDATE_FID:
			if ((ret = rsslDecodeDate(dIter, &mpItem->DIVPAYDATE)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding DIVPAYDATE", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			break;

		case TRDPRC_1_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding TRDPRC_1", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->TRDPRC_1, &tmpReal);
			break;

		case BID_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding BID", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->BID, &tmpReal);
			break;

		case ASK_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding ASK", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->ASK, &tmpReal);
			break;

		case ACVOL_1_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding ACVOL_1", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->ACVOL_1, &tmpReal);
			break;

		case NETCHNG_1_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding NETCHNG_1", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->NETCHNG_1, &tmpReal);
			break;

		case ASK_TIME_FID:
			if ((ret = rsslDecodeTime(dIter, &mpItem->ASK_TIME.time)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding ASK_TIME", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			break;

		case PERATIO_FID:
			if ((ret = rsslDecodeReal(dIter, &tmpReal)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding PERATIO", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			rsslRealToDouble(&mpItem->PERATIO, &tmpReal);
			break;

		case SALTIME_FID:
			if ((ret = rsslDecodeDateTime(dIter, &mpItem->SALTIME)) != RSSL_RET_SUCCESS)
			{
				sprintf(error->rsslError.text, "Error=%d Decoding SALTIME", ret);
				error->rsslError.rsslErrorId = ret;
				return ret;
			}
			break;

		default: 
			sprintf(error->rsslError.text, "Unknown field ID = %d in post\n", fEntry.fieldId);
			error->rsslError.rsslErrorId = RSSL_RET_FAILURE;
			return RSSL_RET_FAILURE;
			break;
		}
	}
	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the market price response.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send a market price response to
 * itemInfo - The item information
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the market price response into
 * streamId - The stream id of the market price response
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the market price response
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMarketPriceResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary)
{
	RsslRet ret;
	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	if ((ret = encodeMarketPriceResponseMsgInit(itemInfo, &encodeIter, isSolicited, streamId, isStreaming, isPrivateStream, serviceId)) != RSSL_RET_SUCCESS)
	{
		printf("Encoding Market Price message init failed: Error %d\n", ret);
		return ret;
	}

	if ((ret = encodeMPFieldList(itemInfo, &encodeIter, isPrivateStream, dictionary)) != RSSL_RET_SUCCESS)
	{
		printf("Encoding Market Price Field List failed: Error %d\n", ret);
		return ret;
	}

	if ((ret = encodeResponseMsgComplete(&encodeIter)) != RSSL_RET_SUCCESS)
	{
		printf("Encoding Market Price message complete failed: Error %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


/*
 * Builds the message header and encodes with rsslEncodeMsgInit
 * itemInfo - The item information
 * encodeIter - iterator associated with buffer for encoding message init
 * isSolicited - The response is solicited if set
 * streamId - The stream id of the market price response
 * isStreaming - Flag for streaming or snapshot
 * isPrivateStream - this item is on a private stream
 * serviceId - The service id of the market price response
 */
RsslRet encodeMarketPriceResponseMsgInit(RsslItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId)
{
	RsslRet ret = 0;
	RsslMsgBase* msgBase;
	RsslMsg* msg;

	/* set-up message */
	/* set message depending on whether refresh or update */
	if (!itemInfo->IsRefreshComplete) /* this is a refresh message */
	{
		rsslClearRefreshMsg(&refreshMsg);
		msgBase = &refreshMsg.msgBase;
		if (isStreaming)
		{
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		}
		else
		{
			refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
		}
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;
		if (isSolicited)
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}
		else
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}
		if (isPrivateStream)
		{
			refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;
		}
		sprintf(stateText, "Item Refresh Completed");
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
		/* ServiceId */
		msgBase->msgKey.serviceId = serviceId;
		/* Itemname */
		msgBase->msgKey.name.data = itemInfo->Itemname;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		/* Qos */
		refreshMsg.qos.dynamic = RSSL_FALSE;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		msg = (RsslMsg *)&refreshMsg;
	}
	else /* this is an update message */
	{
		rsslClearUpdateMsg(&updateMsg);
		msgBase = &updateMsg.msgBase;
		/* include msg key in updates for non-interactive provider streams */
		if (streamId < 0)
		{
			updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
			msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
			/* ServiceId */
			msgBase->msgKey.serviceId = serviceId;
			/* Itemname */
			msgBase->msgKey.name.data = itemInfo->Itemname;
			msgBase->msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
			msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		}
		msg = (RsslMsg *)&updateMsg;
	}

	msgBase->domainType = RSSL_DMT_MARKET_PRICE;
	msgBase->containerType = RSSL_DT_FIELD_LIST;

	/* StreamId */
	msgBase->streamId = streamId;

	/* encode message */
	if ((ret = rsslEncodeMsgInit(encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Complete the response message encoding
 */
RsslRet encodeResponseMsgComplete(RsslEncodeIterator *eIter)
{
	RsslRet ret;
	if ((ret = rsslEncodeMsgComplete(eIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return ret;
}

/*
 * Encode Market Price Field List payload from the data source into a partially
 * encoded message buffer. The encodeMarketPriceResponseMsgComplete should be
 * called after this function completes.
 * itemInfo - The item information
 * encodeIter - iterator associated with buffer which has already been encoded with rsslEncodeMsgInit
 * isPrivateStream - this item is on a private stream
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMPFieldList(RsslItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslBool isPrivateStream, RsslDataDictionary* dictionary)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslMarketPriceItem* mpItem;

	mpItem = (RsslMarketPriceItem*)itemInfo->itemData;

	/* encode field list */
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode fields */
	/* if refresh, encode refresh fields */
	if (!itemInfo->IsRefreshComplete)
	{
		/* RDNDISPLAY */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[RDNDISPLAY_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = RDNDISPLAY_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&mpItem->RDNDISPLAY)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
		/* RDN_EXCHID */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[RDN_EXCHID_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = RDN_EXCHID_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&mpItem->RDN_EXCHID)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
		/* DIVPAYDATE */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[DIVPAYDATE_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = DIVPAYDATE_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&mpItem->DIVPAYDATE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	/* TRDPRC_1 */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[TRDPRC_1_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = TRDPRC_1_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->TRDPRC_1, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* BID */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[BID_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = BID_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->BID, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* ASK */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ASK_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ASK_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->ASK, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* ACVOL_1 */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ACVOL_1_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ACVOL_1_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->ACVOL_1, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* NETCHNG_1 */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[NETCHNG_1_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = NETCHNG_1_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->NETCHNG_1, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* ASK_TIME */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ASK_TIME_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ASK_TIME_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&mpItem->ASK_TIME.time)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* encode items for private stream */
	if (isPrivateStream)
	{
		/* PERATIO */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[PERATIO_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = PERATIO_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			rsslClearReal(&tempReal);
			rsslDoubleToReal(&tempReal, &mpItem->PERATIO, RSSL_RH_EXPONENT_2);
			if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
		/* SALTIME */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[SALTIME_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = SALTIME_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&mpItem->SALTIME.time)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return ret;
}

/*
 * Gets storage for a market by order item from the list.
 * itemName - The item name of the item 
 */
RsslMarketPriceItem* getMarketPriceItem(const char* itemName)
{
	int i;
	RsslMarketPriceItem* itemInfo = NULL;

	/* first check for existing item */
	for (i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
	{
		if (!marketPriceItemList[i].isInUse)
		{
			initMarketPriceItemFields(&marketPriceItemList[i]);
			return &marketPriceItemList[i];
		}
	}

	return NULL;
}

/*
 * Frees an item 
 * itemInfo - The item information structure to free
 */
void freeMarketPriceItem(RsslMarketPriceItem* mpItem)
{
	clearMarketPriceItem(mpItem);
}

