/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rsslVANIMarketByOrderItems.h"
#include "rsslVASendMessage.h"

/* 
 * This handles the generation of VA example market by order data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a market by order message and payload.
 */

#define MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE 100
#define MAX_ITEM_INFO_STRLEN 128

/* item information list */
static RsslMarketByOrderItem marketByOrderItemList[MAX_MARKET_BY_ORDER_ITEM_LIST_SIZE];

/* re-usable refresh and update messages and state text */
RsslRefreshMsg refreshMsg;
RsslUpdateMsg updateMsg;
char stateText[MAX_ITEM_INFO_STRLEN];

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

		snprintf(order->ORDER_ID, 128, "%d", i + 100);
		snprintf(order->MKT_MKR_ID, 128, "%d", 1+i);

		order->ORDER_PRC.hint = RSSL_RH_EXPONENT_2;
		order->ORDER_PRC.value = 3459 + 100*i;
		order->ORDER_PRC.isBlank = RSSL_FALSE;

		order->ORDER_SIZE.hint = RSSL_RH_EXPONENT0;
		order->ORDER_SIZE.value = 5+i;
		order->ORDER_SIZE.isBlank = RSSL_FALSE;

		order->ORDER_SIDE = (i >= MAX_ORDERS / 2) ? ORDER_ASK : ORDER_BID;
		order->QUOTIM_MS = 500*i;

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

		if (order->lifetime == 0)
			order->lifetime = 5+i;
		else
			--order->lifetime;
	}
}

/*
 * Encodes the market by order response.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * itemInfo - The item information
 * encodeIter - iterator associated with buffer for encoding message init
 * streamId - The stream id of the market price response
 * serviceId - The service id of the market price response
 */
RsslRet encodeMarketByOrderResponseMsgInit(RsslNIItemInfo* itemInfo, RsslEncodeIterator *encodeIter, RsslInt32 streamId, RsslUInt16 serviceId, RsslBool buildRefresh)
{
	RsslRet ret = 0;
	RsslMsgBase* msgBase;
	RsslMsg* msg;

	/* set-up message */
	/* set message depending on whether refresh or update */
	if (buildRefresh)
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
 * Encode Market Price Map payload from the data source into a partially
 * encoded message buffer. The encodeResponseMsgComplete should be
 * called after this function completes.
 * mboItem - The market by order item information
 * encodeIter - iterator associated with buffer which has already been encoded with rsslEncodeMsgInit
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeMarketByOrderMap(RsslMarketByOrderItem* mboItem, RsslEncodeIterator *encodeIter, RsslDataDictionary* dictionary, RsslBool buildRefresh)
{
	RsslRet ret = 0;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	int i;

	/* encode map */
	map.flags = RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_KEY_FIELD_ID;
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

	for(i = 0; i < MAX_ORDERS; ++i)
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
		else
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

			/* Since the field list specified RSSL_FLF_HAS_STANDARD_DATA as well, standard entries can be encoded after
			 * the set is finished. The Add actions will additionally include the ORDER_SIDE and MKT_MKR_ID fields. */

			/* encode fields */
			/* if refresh, encode refresh fields */
			if (buildRefresh || order->lifetime == (5+i))
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

	/* complete map */
	if ((ret = rsslEncodeMapComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
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
