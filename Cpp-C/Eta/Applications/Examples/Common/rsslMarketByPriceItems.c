/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "rsslMarketByPriceItems.h"
#include "rsslSendMessage.h"

/* 
 * This handles the generation of example market by price data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a market by price message and payload.
 */

#define MAX_MARKET_BY_PRICE_ITEM_LIST_SIZE 100

/* item information list */
static RsslMarketByPriceItem marketByPriceItemList[MAX_MARKET_BY_PRICE_ITEM_LIST_SIZE];

/*
 * Create a local set definition.
 * Set definitions are used to reduce the size of messages with a
 * large amount of repetitive data.  For example, the set defined here
 * is for a payload that contains many FieldLists that all have an Order Price, Order Size,
 * Quote Time, and Number of Orders.  The set definition will provide the fieldIds
 * so that they need to be present on each entry.
 */

/*
 * Create an array of set definition entries. This defines
 * what entries will be encoded when using this set.
 */
static RsslFieldSetDefEntry marketByPriceSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{ORDER_PRC_FID, RSSL_DT_REAL_4RB},	/* Order Price */ 
	{ORDER_SIZE_FID, RSSL_DT_REAL},		/* Order Size */ 
	{QUOTIM_MS_FID, RSSL_DT_UINT_4},	/* Quote Time */ 
	{NO_ORD_FID, RSSL_DT_UINT_4}		/* Number of Orders */ 
};

/* 
 * Create a set definition database to store the definition.
 * This particular database is encoded with the RsslMap and is passed into 
 * rsslEncodeFieldListInit(). 
 * A database may store up to 16 sets for use. The market by price message will only use one. 
 */
static RsslLocalFieldSetDefDb marketByPriceSetDefDb =
{
	{
		/* {setId, entry count, entries array} */
    	{ 0, 4, marketByPriceSetEntries},
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 }
	},
	{0, 0}
};

/*
 * Initializes market by price handler internal structures.
 */
void initMarketByPriceItems()
{
	int i;

	/* clear item information list */
	for (i = 0; i < MAX_MARKET_BY_PRICE_ITEM_LIST_SIZE; i++)
	{
		clearMarketByPriceItem(&marketByPriceItemList[i]);
	}
}

/*
 * Initializes market by price item fields.
 * mbpItem - The item to be initialized
 */
void initMarketByPriceItemFields(RsslMarketByPriceItem* mbpItem)
{
	int i;
	RsslBuffer tempBuf = RSSL_INIT_BUFFER;

    mbpItem->isInUse = RSSL_TRUE;
	
	/* Initializes fields for the orders associated with the item. */
	for(i = 0; i < MAX_ORDERS; ++i)
	{
		RsslPriceInfo *order = &mbpItem->orders[i];

		snprintf(order->MKT_MKR_ID, MAX_MKT_MKR_ID_STRLEN, "MarketMaker%d", 1+i);

		order->ORDER_PRC.hint = RSSL_RH_EXPONENT_2;
		order->ORDER_PRC.value = 3459 + 100*i;
		order->ORDER_PRC.isBlank = RSSL_FALSE;
		tempBuf.data = order->PRICE_POINT;
		tempBuf.length = MAX_PRICE_POINT_STRLEN;
		rsslRealToString(&tempBuf, &order->ORDER_PRC);
		strcat(order->PRICE_POINT, (i >= MAX_ORDERS / 2) ? "a" : "b");

		order->ORDER_SIZE.hint = RSSL_RH_EXPONENT0;
		order->ORDER_SIZE.value = 5+i;
		order->ORDER_SIZE.isBlank = RSSL_FALSE;

		order->ORDER_SIDE = (i >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID;
		order->QUOTIM_MS = 500*i;
		order->NO_ORD = 1+i;

		order->MKOASK_VOL.hint = RSSL_RH_EXPONENT0;
		order->MKOASK_VOL.value = 2+i;
		order->MKOASK_VOL.isBlank = RSSL_FALSE;
		order->MKOBID_VOL.hint = RSSL_RH_EXPONENT0;
		order->MKOBID_VOL.value = 3+i;
		order->MKOBID_VOL.isBlank = RSSL_FALSE;

		order->lifetime = 5 + i;
	}
}

/*
 * Updates any item that's currently in use.
 */
void updateMarketByPriceItemFields(RsslMarketByPriceItem* itemInfo)
{
	int i;

	for (i = 0; i < MAX_ORDERS; ++i)
	{
		RsslPriceInfo *order = &itemInfo->orders[i];

		order->NO_ORD += 1;
		order->QUOTIM_MS += 1000;

		order->MKOASK_VOL.value += 1;
		order->MKOBID_VOL.value += 1;

		if (order->lifetime == 0)
			order->lifetime = 5+i;
		else
			--order->lifetime;
	}
}

/*
 * Encodes the market by price refresh.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a market by price refresh to
 * itemInfo - The item information
 * isSolicited - The refresh is solicited if set
 * msgBuf - The message buffer to encode the market by price refresh into
 * streamId - The stream id of the market by price refresh
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the market by price refresh
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMarketByPriceRefresh(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary, int multiPartNo)
{
	RsslRet ret = 0;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	char stateText[MAX_ITEM_INFO_STRLEN];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslEncodeIterator encodeIter;
	RsslMarketByPriceItem *mbpItem;
	int currencyEnum = USD_ENUM, marketStatusIndEnum = BBO_ENUM;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	RsslPriceInfo *order;
	RsslBuffer tmpBuf;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	mbpItem = (RsslMarketByPriceItem*)itemInfo->itemData;
	order = &mbpItem->orders[multiPartNo];

	/* set-up message */
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
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
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
	}
	else
	{
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
	}
	if (isPrivateStream)
	{
		refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;
	}
	/* multi-part refresh complete when multiPartNo hits max */
	if (multiPartNo == MAX_ORDERS - 1)
	{
		refreshMsg.flags |= RSSL_RFMF_REFRESH_COMPLETE;
		snprintf(stateText, 128, "Item Refresh Completed");
	}
	else
	{
		snprintf(stateText, 128, "Item Refresh In Progress");
	}
	refreshMsg.state.text.data = stateText;
	refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
	/* ServiceId */
	refreshMsg.msgBase.msgKey.serviceId = serviceId;
	/* Itemname */
	refreshMsg.msgBase.msgKey.name.data = itemInfo->Itemname;
	refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
	refreshMsg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	/* Qos */
	refreshMsg.qos.dynamic = RSSL_FALSE;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;

	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;

	/* StreamId */
	refreshMsg.msgBase.streamId = streamId;

	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg *)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map */
	if (multiPartNo == 0) /* only first part of multi-part refresh has summary data */
	{
		map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
	}
	else
	{
		map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_KEY_FIELD_ID;
	}
	map.containerType = RSSL_DT_FIELD_LIST;

	/* The map is keyed by the ORDER_ID field. */
	map.keyPrimitiveType = RSSL_DT_BUFFER;
	map.keyFieldId = ORDER_ID_FID;
	map.totalCountHint = MAX_ORDERS;

	if ((ret = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode the field set definition database into the map.
	 * The definitions are used by the field lists that the map contains. */
	if ((ret = rsslEncodeLocalFieldSetDefDb(&encodeIter, &marketByPriceSetDefDb)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeLocalFieldSetDefDb() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeMapSetDefsComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapSetDefsComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode summary data for refresh */
	if (multiPartNo == 0) /* only first part of multi-part refresh has summary data */
	{
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, &marketByPriceSetDefDb, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* CURRENCY */
		fEntry.fieldId = CURRENCY_FID;
		fEntry.dataType = RSSL_DT_ENUM;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&currencyEnum)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		/* MARKET STATUS INDICATOR */
		fEntry.fieldId = MKT_STATUS_IND_FID;
		fEntry.dataType = RSSL_DT_ENUM;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&marketStatusIndEnum)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		/* ACTIVE DATE */
		fEntry.fieldId = ACTIVE_DATE_FID;
		fEntry.dataType = RSSL_DT_DATE;
		rsslDateTimeGmtTime(&dateTime);
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&dateTime.date)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeMapSummaryDataComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeMapSummaryDataComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* Encode the map entry representing each order */

	/* encode map entry */
	rsslClearMapEntry(&mapEntry);
    mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	mapEntry.flags = RSSL_MPEF_NONE;
	tmpBuf.data = order->PRICE_POINT;
	tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
	if ((ret = rsslEncodeMapEntryInit(&encodeIter, &mapEntry, &tmpBuf, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode field list */
	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
	fList.setId = 0;
	if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, &marketByPriceSetDefDb, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

    /* encode fields */

	/* ORDER_PRC */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ORDER_PRC_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ORDER_PRC_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_PRC)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* ORDER_SIZE */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ORDER_SIZE_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ORDER_SIZE_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_SIZE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}


	/* QUOTIM_MS */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[QUOTIM_MS_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = QUOTIM_MS_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->QUOTIM_MS)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* NO_ORD */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[NO_ORD_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = NO_ORD_FID;
		fEntry.dataType = dictionaryEntry->rwfType;

		/* This encoding completes the encoding of the ORDER_PRC, ORDER_SIZE, QUOTIM_MS, NO_ORD set. When a set is completed,
			* a success code, RSSL_RET_SET_COMPLETE, is returned to indicate this.  This may be used to ensure that
			* the set is being used as intended. */
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->NO_ORD)) != RSSL_RET_SET_COMPLETE)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* encode items for private stream */
	if (isPrivateStream)
	{
		/* MKOASK_VOL */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[MKOASK_VOL_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = MKOASK_VOL_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->MKOASK_VOL)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}

		/* MKOBID_VOL */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[MKOBID_VOL_FID];
		if (dictionaryEntry)
		{
			fEntry.fieldId = MKOBID_VOL_FID;
			fEntry.dataType = dictionaryEntry->rwfType;
			if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->MKOBID_VOL)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* Since the field list specified RSSL_FLF_HAS_STANDARD_DATA as well, standard entries can be encoded after
		* the set is finished. The Add actions will additionally include the ORDER_SIDE and MKT_MKR_ID fields. */ 

	/* ORDER_SIDE */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ORDER_SIDE_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ORDER_SIDE_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_SIDE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete map entry */
	if ((ret = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete map */
	if ((ret = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the market by price update.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a market by price update to
 * itemInfo - The item information
 * isSolicited - The update is solicited if set
 * msgBuf - The message buffer to encode the market by price update into
 * streamId - The stream id of the market by price update
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the market by price update
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMarketByPriceUpdate(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary)
{
	RsslRet ret = 0;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslEncodeIterator encodeIter;
	RsslMarketByPriceItem *mbpItem;
	int i;
	int currencyEnum = USD_ENUM, marketStatusIndEnum = BBO_ENUM;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	mbpItem = (RsslMarketByPriceItem*)itemInfo->itemData;

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_UPDATE;
	msg.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
	msg.msgBase.containerType = RSSL_DT_MAP;

	/* StreamId */
	msg.msgBase.streamId = streamId;

	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, &msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map */
	map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_KEY_FIELD_ID;
	map.containerType = RSSL_DT_FIELD_LIST;

	/* The map is keyed by the ORDER_ID field. */
	map.keyPrimitiveType = RSSL_DT_BUFFER;
	map.keyFieldId = ORDER_ID_FID;

	if ((ret = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode the field set definition database into the map.
	 * The definitions are used by the field lists that the map contains. */
	if ((ret = rsslEncodeLocalFieldSetDefDb(&encodeIter, &marketByPriceSetDefDb)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeLocalFieldSetDefDb() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeMapSetDefsComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapSetDefsComplete() failed with return code: %d\n", ret);
		return ret;
	}

	for(i = 0; i < MAX_ORDERS; ++i)
	{
		/* Encode the map entry representing each order */
		RsslPriceInfo *order = &mbpItem->orders[i];
		RsslBuffer tmpBuf;

		if (order->lifetime != 0)
		{

			/* encode map entry */
			rsslClearMapEntry(&mapEntry);
			if (order->lifetime == 5+i && itemInfo->IsRefreshComplete)
    			mapEntry.action = RSSL_MPEA_ADD_ENTRY;
			else
    			mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
			mapEntry.flags = RSSL_MPEF_NONE;
			tmpBuf.data = order->PRICE_POINT;
			tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
			if ((ret = rsslEncodeMapEntryInit(&encodeIter, &mapEntry, &tmpBuf, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
				return ret;
			}

			/* encode field list */
			rsslClearFieldList(&fList);
			fList.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
			fList.setId = 0;
			if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, &marketByPriceSetDefDb, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
				return ret;
			}

            /* encode fields */

			/* ORDER_PRC */
			rsslClearFieldEntry(&fEntry);
			dictionaryEntry = dictionary->entriesArray[ORDER_PRC_FID];
			if (dictionaryEntry)
			{
				fEntry.fieldId = ORDER_PRC_FID;
				fEntry.dataType = dictionaryEntry->rwfType;
				if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_PRC)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}

			/* ORDER_SIZE */
			rsslClearFieldEntry(&fEntry);
			dictionaryEntry = dictionary->entriesArray[ORDER_SIZE_FID];
			if (dictionaryEntry)
			{
				fEntry.fieldId = ORDER_SIZE_FID;
				fEntry.dataType = dictionaryEntry->rwfType;
				if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_SIZE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}


			/* QUOTIM_MS */
			rsslClearFieldEntry(&fEntry);
			dictionaryEntry = dictionary->entriesArray[QUOTIM_MS_FID];
			if (dictionaryEntry)
			{
				fEntry.fieldId = QUOTIM_MS_FID;
				fEntry.dataType = dictionaryEntry->rwfType;
				if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->QUOTIM_MS)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}

			/* NO_ORD */
			rsslClearFieldEntry(&fEntry);
			dictionaryEntry = dictionary->entriesArray[NO_ORD_FID];
			if (dictionaryEntry)
			{
				fEntry.fieldId = NO_ORD_FID;
				fEntry.dataType = dictionaryEntry->rwfType;

				/* This encoding completes the encoding of the ORDER_PRC, ORDER_SIZE, QUOTIM_MS, NO_ORD set. When a set is completed,
				 * a success code, RSSL_RET_SET_COMPLETE, is returned to indicate this.  This may be used to ensure that
				 * the set is being used as intended. */
				if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->NO_ORD)) != RSSL_RET_SET_COMPLETE)
				{
					printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}

			/* encode items for private stream */
			if (isPrivateStream)
			{
				/* MKOASK_VOL */
				rsslClearFieldEntry(&fEntry);
				dictionaryEntry = dictionary->entriesArray[MKOASK_VOL_FID];
				if (dictionaryEntry)
				{
					fEntry.fieldId = MKOASK_VOL_FID;
					fEntry.dataType = dictionaryEntry->rwfType;
					if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->MKOASK_VOL)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
						return ret;
					}
				}

				/* MKOBID_VOL */
				rsslClearFieldEntry(&fEntry);
				dictionaryEntry = dictionary->entriesArray[MKOBID_VOL_FID];
				if (dictionaryEntry)
				{
					fEntry.fieldId = MKOBID_VOL_FID;
					fEntry.dataType = dictionaryEntry->rwfType;
					if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->MKOBID_VOL)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
						return ret;
					}
				}
			}

			/* Since the field list specified RSSL_FLF_HAS_STANDARD_DATA as well, standard entries can be encoded after
			 * the set is finished. The Add actions will additionally include the ORDER_SIDE and MKT_MKR_ID fields. */ 

			/* encode fields */
			if (order->lifetime == (5+i) && itemInfo->IsRefreshComplete)
			{
				/* ORDER_SIDE */
				rsslClearFieldEntry(&fEntry);
				dictionaryEntry = dictionary->entriesArray[ORDER_SIDE_FID];
				if (dictionaryEntry)
				{
					fEntry.fieldId = ORDER_SIDE_FID;
					fEntry.dataType = dictionaryEntry->rwfType;
					if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&order->ORDER_SIDE)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
						return ret;
					}
				}
			}

			/* complete encode field list */
			if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
				return ret;
			}

			/* complete map entry */
			if ((ret = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* Adds the RSSL_MPEA_DELETE_ENTRY action after others if any */
	for (i = 0; i < MAX_ORDERS; ++i)
	{
		/* Encode the map entry representing each order */
		RsslPriceInfo *order = &mbpItem->orders[i];
		RsslBuffer tmpBuf;

		if (order->lifetime == 0)
		{
			/* Delete the order */
			rsslClearMapEntry(&mapEntry);
			mapEntry.flags = RSSL_MPEF_NONE;
			mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
			tmpBuf.data = order->PRICE_POINT;
			tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
			if ((ret = rsslEncodeMapEntry(&encodeIter, &mapEntry, &tmpBuf)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* complete map */
	if ((ret = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Gets storage for a market by price item from the list.
 * itemName - The item name of the item 
 */
RsslMarketByPriceItem* getMarketByPriceItem(const char* itemName)
{
	int i;
	RsslMarketByPriceItem* itemInfo = NULL;

	/* first check for existing item */
	for (i = 0; i < MAX_MARKET_BY_PRICE_ITEM_LIST_SIZE; i++)
	{
		if (!marketByPriceItemList[i].isInUse)
		{
			initMarketByPriceItemFields(&marketByPriceItemList[i]);
			return &marketByPriceItemList[i];
		}
	}

	return NULL;
}

/*
 * Frees an item information structure.
 * itemInfo - The item information structure to free
 */
void freeMarketByPriceItem(RsslMarketByPriceItem* mbpItem)
{
	clearMarketByPriceItem(mbpItem);
}
