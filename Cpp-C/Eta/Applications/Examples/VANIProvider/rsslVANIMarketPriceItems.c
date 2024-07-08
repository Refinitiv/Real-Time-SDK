/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rsslVANIMarketPriceItems.h"
#include "rsslVASendMessage.h"

/* 
 * This handles the generation of VA example market price data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a market price message and payload.
 */

#define MAX_MARKET_PRICE_ITEM_LIST_SIZE 100
#define MAX_ITEM_INFO_STRLEN 128

/* item information list */
static RsslMarketPriceItem marketPriceItemList[MAX_MARKET_PRICE_ITEM_LIST_SIZE];

/* re-usable refresh and update messages and state text */
static RsslRefreshMsg refreshMsg;
static RsslUpdateMsg updateMsg;
static char stateText[MAX_ITEM_INFO_STRLEN];

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
}

/*
 * Builds the message header and encodes with rsslEncodeMsgInit.
 * itemInfo - The item information
 * encodeIter - iterator associated with buffer for encoding message init
 * streamId - The stream id of the market price response
 * serviceId - The service id of the market price response
 * buildRefresh - build refresh if True; else build update
 */
RsslRet encodeMarketPriceResponseMsgInit(RsslNIItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslInt32 streamId, RsslUInt16 serviceId, RsslBool buildRefresh)
{
	RsslRet ret = 0;
	RsslMsgBase* msgBase;
	RsslMsg* msg;

	/* set-up message */
	/* set message depending on whether refresh or update */
	if (buildRefresh) /* this is a refresh message */
	{
		rsslClearRefreshMsg(&refreshMsg);
		msgBase = &refreshMsg.msgBase;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS;

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
		msg = (RsslMsg *)&updateMsg;

		updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;

		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;

		msgBase->msgKey.serviceId = serviceId;
		msgBase->msgKey.name.data = itemInfo->Itemname;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
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
 * Complete the message encoding
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
 * dictionary - The dictionary used for encoding
 * buildRefresh - if True, builds field list for refresh; else builds update field list
 */
RsslRet encodeMPFieldList(RsslMarketPriceItem *mpItem, RsslEncodeIterator *encodeIter, RsslDataDictionary* dictionary, RsslBool buildRefresh)
{
	RsslRet ret = 0;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* encode field list */
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode fields */
	/* if refresh, encode refresh fields */
	if (buildRefresh)
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

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
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

