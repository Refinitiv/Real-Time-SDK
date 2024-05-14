/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/



#include "rsslVAMarketByOrderItems.h"
#include "rsslVASendMessage.h"

/* 
 * This handles the generation of VA example market by order data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a market by order message and payload.
 */

#define MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE 100

/* item information list */
static RsslMarketByOrderItem marketByOrderItemList[MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE];

/* re-usable refresh and update messages and state text */
static RsslRefreshMsg refreshMsg;
static RsslUpdateMsg updateMsg;
static char stateText[MAX_ITEM_INFO_STRLEN];

/*
 * Create a local set definition.
 * Set definitions are used to reduce the size of messages with a
 * large amount of repetitive data.  For example, the set defined here
 * is for a payload that contains many FieldLists that all have an Order Price, Order Size,
 * and Quote Time.  The set definition will provide the fieldIds
 * so that they need to be present on each entry.
 */

/*
 * Create an array of set definition entries. This defines
 * what entries will be encoded when using this set.
 */
static RsslFieldSetDefEntry marketByOrderSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{ORDER_PRC_FID, RSSL_DT_REAL_4RB},	/* Order Price */ 
	{ORDER_SIZE_FID, RSSL_DT_REAL},		/* Order Size */ 
	{QUOTIM_MS_FID, RSSL_DT_UINT_4}		/* Quote Time */ 
};

/* 
 * Create a set definition database to store the definition.
 * This particular database is encoded with the RsslMap and is passed into 
 * rsslEncodeFieldListInit(). 
 * A database may store up to 16 sets for use. The market by order message will only use one. 
 */
static RsslLocalFieldSetDefDb marketByOrderSetDefDb =
{
	{
		/* {setId, entry count, entries array} */
    	{ 0, 3, marketByOrderSetEntries},
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
 * Initializes market by order handler internal structures.
 */
void initMarketByOrderItems()
{
	int i;

	/* clear item information list */
	for (i = 0; i < MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE; i++)
	{
		clearMarketByOrderItem(&marketByOrderItemList[i]);
	}
}

/*
 * Initializes market by order item fields.
 * mboItem - The item to be initialized
 */
void initMarketByOrderItemFields(RsslMarketByOrderItem* mboItem)
{
	int i;

    mboItem->isInUse = RSSL_TRUE;
	
	/* Initializes fields for the orders associated with the item. */
	for(i = 0; i < MAX_ORDERS; ++i)
	{
		RsslOrderInfo *order = &mboItem->orders[i];

		snprintf(order->ORDER_ID, MAX_ORDER_ID_STRLEN, "%d", i + 100);
		snprintf(order->MKT_MKR_ID, MAX_MKT_MKR_ID_STRLEN, "%d", 1+i);

		order->ORDER_PRC.hint = RSSL_RH_EXPONENT_2;
		order->ORDER_PRC.value = 3459 + 100*i;
		order->ORDER_PRC.isBlank = RSSL_FALSE;

		order->ORDER_SIZE.hint = RSSL_RH_EXPONENT0;
		order->ORDER_SIZE.value = 5+i;
		order->ORDER_SIZE.isBlank = RSSL_FALSE;

		order->ORDER_SIDE = (i >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID;
		order->QUOTIM_MS = 500*i;

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
void updateMarketByOrderItemFields(RsslMarketByOrderItem* itemInfo)
{
	int i;

	for (i = 0; i < MAX_ORDERS; ++i)
	{
		RsslOrderInfo *order = &itemInfo->orders[i];

		order->ORDER_PRC.value += 1;
		order->QUOTIM_MS += 1000;

		order->MKOASK_VOL.value += 1;
		order->MKOBID_VOL.value += 1;

		if (order->lifetime == 0)
			order->lifetime = 5+i;
		else
			--order->lifetime;
	}
	itemInfo->updateCount++;
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
RsslRet encodeMarketByOrderResponseMsgInit(RsslItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId)
{
	RsslRet ret = 0;
	RsslMsgBase* msgBase;
	RsslMsg* msg;
	int currencyEnum = USD_ENUM, marketStatusIndEnum = BBO_ENUM;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

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
		msg = (RsslMsg *)&updateMsg;
	}

	msgBase->domainType = RSSL_DMT_MARKET_BY_ORDER;
	msgBase->containerType = RSSL_DT_MAP;

	/* StreamId */
	msgBase->streamId = streamId;

	if ((ret = rsslEncodeMsgInit(encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encode Market By Order Map payload from the data source into a partially
 * encoded message buffer. The encodeMarketPriceResponseMsgComplete should be
 * called after this function completes.
 * itemInfo - The item information
 * encodeIter - iterator associated with buffer which has already been encoded with rsslEncodeMsgInit
 * isPrivateStream - this item is on a private stream
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMBOMap(RsslItemInfo* itemInfo, RsslEncodeIterator* encodeIter, RsslBool isPrivateStream, RsslDataDictionary* dictionary)
{
	RsslRet ret = 0;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslMarketByOrderItem *mboItem;
	int i;
	int currencyEnum = USD_ENUM, marketStatusIndEnum = BBO_ENUM;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

	mboItem = (RsslMarketByOrderItem*)itemInfo->itemData;

	/* encode map */
	if (!itemInfo->IsRefreshComplete) /* refresh - has summary data */
	{
		map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
	}
	else /* update - no summary data */
	{
	map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_KEY_FIELD_ID;
	}
	map.containerType = RSSL_DT_FIELD_LIST;

	/* The map is keyed by the ORDER_ID field. */
	map.keyPrimitiveType = RSSL_DT_BUFFER;
	map.keyFieldId = ORDER_ID_FID;

	if ((ret = rsslEncodeMapInit(encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode the field set definition database into the map.
	 * The definitions are used by the field lists that the map contains. */
	if ((ret = rsslEncodeLocalFieldSetDefDb(encodeIter, &marketByOrderSetDefDb)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeLocalFieldSetDefDb() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeMapSetDefsComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapSetDefsComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* Encode summary data for refresh */
	if (!itemInfo->IsRefreshComplete) /* this is a refresh message */
	{
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(encodeIter, &fList, &marketByOrderSetDefDb, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* CURRENCY */
		fEntry.fieldId = CURRENCY_FID;
		fEntry.dataType = RSSL_DT_ENUM;
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&currencyEnum)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		/* MARKET STATUS INDICATOR */
		fEntry.fieldId = MKT_STATUS_IND_FID;
		fEntry.dataType = RSSL_DT_ENUM;
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&marketStatusIndEnum)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		/* ACTIVE DATE */
		fEntry.fieldId = ACTIVE_DATE_FID;
		fEntry.dataType = RSSL_DT_DATE;
		rsslDateTimeGmtTime(&dateTime);
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&dateTime.date)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeMapSummaryDataComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeMapSummaryDataComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}

	for(i = 0; i < MAX_ORDERS; ++i)
	{
		/* Encode the map entry representing each order */
		RsslOrderInfo *order = &mboItem->orders[i];
		RsslBuffer tmpBuf;

		if (order->lifetime != 0)
		{

			/* encode map entry */
			rsslClearMapEntry(&mapEntry);
			if (order->lifetime == 5+i)
    			mapEntry.action = RSSL_MPEA_ADD_ENTRY;
			else
    			mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
			mapEntry.flags = RSSL_MPEF_NONE;
			tmpBuf.data = order->ORDER_ID;
			tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
			if ((ret = rsslEncodeMapEntryInit(encodeIter, &mapEntry, &tmpBuf, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
				return ret;
			}

			/* encode field list */
			rsslClearFieldList(&fList);
			fList.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
			fList.setId = 0;
			if ((ret = rsslEncodeFieldListInit(encodeIter, &fList, &marketByOrderSetDefDb, 0)) < RSSL_RET_SUCCESS)
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
				if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->ORDER_PRC)) < RSSL_RET_SUCCESS)
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
				if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->ORDER_SIZE)) < RSSL_RET_SUCCESS)
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

				/* This encoding completes the encoding of the ORDER_PRC, ORDER_SIZE, QUOTIM_MS set. When a set is completed,
				 * a success code, RSSL_RET_SET_COMPLETE, is returned to indicate this.  This may be used to ensure that
				 * the set is being used as intended. */
				if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->QUOTIM_MS)) != RSSL_RET_SET_COMPLETE)
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
					if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->MKOASK_VOL)) < RSSL_RET_SUCCESS)
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
					if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->MKOBID_VOL)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
						return ret;
					}
				}
			}

			/* Since the field list specified RSSL_FLF_HAS_STANDARD_DATA as well, standard entries can be encoded after
			 * the set is finished. The Add actions will additionally include the ORDER_SIDE and MKT_MKR_ID fields. */

			/* encode fields */
			/* if refresh, encode refresh fields */
			if (!itemInfo->IsRefreshComplete || order->lifetime == (5+i))
			{
				/* ORDER_SIDE */
				rsslClearFieldEntry(&fEntry);
				dictionaryEntry = dictionary->entriesArray[ORDER_SIDE_FID];
				if (dictionaryEntry)
				{
					fEntry.fieldId = ORDER_SIDE_FID;
					fEntry.dataType = dictionaryEntry->rwfType;
					if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&order->ORDER_SIDE)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
						return ret;
					}
				}

				/* MKT_MKR_ID */
				rsslClearFieldEntry(&fEntry);
				dictionaryEntry = dictionary->entriesArray[MKT_MKR_ID_FID];
				if (dictionaryEntry)
				{
					fEntry.fieldId = MKT_MKR_ID_FID;
					fEntry.dataType = dictionaryEntry->rwfType;
					tmpBuf.data = order->MKT_MKR_ID;
					tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
					if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, (void*)&tmpBuf)) < RSSL_RET_SUCCESS)
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

			/* complete map entry */
			if ((ret = rsslEncodeMapEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
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
		RsslOrderInfo *order = &mboItem->orders[i];
		RsslBuffer tmpBuf;

		if (order->lifetime == 0)
		{
			/* Delete the order */
			rsslClearMapEntry(&mapEntry);
			mapEntry.flags = RSSL_MPEF_NONE;
			mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
			tmpBuf.data = order->ORDER_ID;
			tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
			if ((ret = rsslEncodeMapEntry(encodeIter, &mapEntry, &tmpBuf)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* complete map */
	if ((ret = rsslEncodeMapComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;

}

/*
 * Encodes the market by order response.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send a market price response to
 * itemInfo - The item information
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the market by order response into
 * streamId - The stream id of the market by order response
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the market by order response
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMarketByOrderResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary)
{
	RsslRet ret;
	RsslEncodeIterator encodeIter;

	rsslClearEncodeIterator(&encodeIter);

	if ((ret = encodeMarketByOrderResponseMsgInit(itemInfo, &encodeIter, isSolicited, streamId, isStreaming, isPrivateStream, serviceId)) != RSSL_RET_SUCCESS)
	{
		printf("encodeMarketByOrderResponseMsgInit failed with return code: %d\n", ret);
		return ret;
	}

	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	if ((ret = encodeMBOMap(itemInfo, &encodeIter, isPrivateStream, dictionary)) != RSSL_RET_SUCCESS)
	{
		printf("encodeMBOMap() failed with return code: %d\n", ret);
		return ret;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return ret;
}

/*
 * Gets storage for a market by order item from the list.
 * itemName - The item name of the item 
 */
RsslMarketByOrderItem* getMarketByOrderItem(const char* itemName)
{
	int i;
	RsslMarketByOrderItem* itemInfo = NULL;

	/* first check for existing item */
	for (i = 0; i < MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE; i++)
	{
		if (!marketByOrderItemList[i].isInUse)
		{
			initMarketByOrderItemFields(&marketByOrderItemList[i]);
			return &marketByOrderItemList[i];
		}
	}

	return NULL;
}

/*
 * Frees an item information structure.
 * itemInfo - The item information structure to free
 */
void freeMarketByOrderItem(RsslMarketByOrderItem* mboItem)
{
	clearMarketByOrderItem(mboItem);
}
