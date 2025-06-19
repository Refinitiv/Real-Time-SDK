/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "marketByOrderEncoder.h"
#include "itemEncoder.h"
#include "rtr/rsslGetTime.h"
#include "xmlMsgDataParser.h"
#include <assert.h>

RsslUInt32 getNextMarketByOrderUpdateEstimatedContentLength(MarketByOrderItem *mboItem)
{
	return xmlMarketByOrderMsgs.updateMsgs[mboItem->iMsg].estimatedContentLength;
}

MarketByOrderMsg *getNextMarketByOrderUpdate(MarketByOrderItem *mboItem)
{
	MarketByOrderMsg *mboMsg = &xmlMarketByOrderMsgs.updateMsgs[mboItem->iMsg++];
	if (mboItem->iMsg == xmlMarketByOrderMsgs.updateMsgCount) mboItem->iMsg = 0;
	return mboMsg;
}

RsslInt32 getMarketByOrderUpdateMsgCount()
{
	return xmlMarketByOrderMsgs.updateMsgCount;
}

RsslUInt32 getNextMarketByOrderPostEstimatedContentLength(MarketByOrderItem *mboItem)
{
	return xmlMarketByOrderMsgs.postMsgs[mboItem->iMsg].estimatedContentLength;
}

MarketByOrderMsg *getNextMarketByOrderPost(MarketByOrderItem *mboItem)
{
	MarketByOrderMsg *mboMsg = &xmlMarketByOrderMsgs.postMsgs[mboItem->iMsg++];
	if (mboItem->iMsg == xmlMarketByOrderMsgs.postMsgCount) mboItem->iMsg = 0;
	return mboMsg;
}

RsslInt32 getMarketByOrderPostMsgCount()
{
	return xmlMarketByOrderMsgs.postMsgCount;
}

RsslUInt32 getNextMarketByOrderGenMsgEstimatedContentLength(MarketByOrderItem *mboItem)
{
	return xmlMarketByOrderMsgs.genMsgs[mboItem->iMsg].estimatedContentLength;
}

MarketByOrderMsg *getNextMarketByOrderGenMsg(MarketByOrderItem *mboItem)
{
	MarketByOrderMsg *mboMsg = &xmlMarketByOrderMsgs.genMsgs[mboItem->iMsg++];
	if (mboItem->iMsg == xmlMarketByOrderMsgs.genMsgCount) mboItem->iMsg = 0;
	return mboMsg;
}

RsslInt32 getMarketByOrderGenMsgCount()
{
	return xmlMarketByOrderMsgs.genMsgCount;
}


RsslRet encodeMarketByOrderDataBody(RsslEncodeIterator *pIter, MarketByOrderMsg *pMboMsg,
		RsslMsgClasses msgClass, RsslUInt encodeStartTime)
{
	RsslRet ret = 0;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslFieldList fList;
	RsslFieldEntry fEntry;

	int i;

	assert(!(msgClass == RSSL_MC_REFRESH && encodeStartTime));

	/* encode map */
	rsslClearMap(&map);
	map.flags = RSSL_MPF_HAS_KEY_FIELD_ID; 
	if (pMboMsg->setDefDb) map.flags |= RSSL_MPF_HAS_SET_DEFS;
	map.containerType = RSSL_DT_FIELD_LIST;

	/* The map is keyed by the ORDER_ID field. */
	map.keyPrimitiveType = RSSL_DT_RMTES_STRING;
	map.keyFieldId = ORDER_ID_FID;

	/* If we are going to encode latency information(or this is a refresh and we need to
	 * provide the posting latency field), we are going to put it in summary data. */
	if (encodeStartTime || msgClass == RSSL_MC_REFRESH)
		map.flags |= RSSL_MPF_HAS_SUMMARY_DATA;

	if ((ret = rsslEncodeMapInit(pIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
		return ret;

	/* Encode the field set definition database into the map.
	 * The definitions are used by the field lists that the map contains. */
	if (pMboMsg->setDefDb)
	{
		if ((ret = rsslEncodeLocalFieldSetDefDb(pIter, pMboMsg->setDefDb)) < RSSL_RET_SUCCESS)
			return ret;

		if ((ret = rsslEncodeMapSetDefsComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			return ret;
	}

	rsslClearFieldEntry(&fEntry);

	if (map.flags & RSSL_MPF_HAS_SUMMARY_DATA)
	{
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(pIter, &fList, pMboMsg->setDefDb, 0)) < RSSL_RET_SUCCESS)
			return ret;


		/* Include the latency time fields in refreshes. */
		if (msgClass == RSSL_MC_REFRESH)
		{
			fEntry.fieldId = TIM_TRK_1_FID;
			fEntry.dataType = RSSL_DT_UINT;
			if ((ret = rsslEncodeFieldEntry(pIter, &fEntry, NULL)) < RSSL_RET_SUCCESS)
				return ret;

			fEntry.fieldId = TIM_TRK_2_FID;
			fEntry.dataType = RSSL_DT_UINT;
			if ((ret = rsslEncodeFieldEntry(pIter, &fEntry, NULL)) < RSSL_RET_SUCCESS)
				return ret;

			fEntry.fieldId = TIM_TRK_3_FID;
			fEntry.dataType = RSSL_DT_UINT;
			if ((ret = rsslEncodeFieldEntry(pIter, &fEntry, NULL)) < RSSL_RET_SUCCESS)
				return ret;
		}

		if (encodeStartTime)
		{
			/* Encode the latency timestamp. */

			fEntry.fieldId = (msgClass == RSSL_MC_GENERIC) ? TIM_TRK_3_FID : (msgClass == RSSL_MC_POST) ? TIM_TRK_2_FID : TIM_TRK_1_FID;
			fEntry.dataType = RSSL_DT_UINT;
			if ((ret = rsslEncodeFieldEntry(pIter, &fEntry, (void*)&encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
		}


		if ((ret = rsslEncodeFieldListComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			return ret;

		if ((ret = rsslEncodeMapSummaryDataComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			return ret;
	}



	for(i = 0; i < pMboMsg->orderCount; ++i)
	{
		int j;
		/* Encode the map entry representing each order */
		MarketOrder *order = &pMboMsg->orders[i];


		/* encode map entry */
		rsslClearMapEntry(&mapEntry);
		mapEntry.action = order->action;
		mapEntry.flags = RSSL_MPEF_NONE;
		if ((ret = rsslEncodeMapEntryInit(pIter, &mapEntry, &order->orderId, 0)) < RSSL_RET_SUCCESS)
			return ret;


		/* encode field list */
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA ;
		if (pMboMsg->setDefDb && order->setId >= 0)
		{
			if (fList.setId != 0)
				fList.flags |= RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
			else
				fList.flags |= RSSL_FLF_HAS_SET_DATA;

		}
		if ((ret = rsslEncodeFieldListInit(pIter, &fList, pMboMsg->setDefDb, 0)) < RSSL_RET_SUCCESS)
			return ret;

		/* encode fields */
		for (j = 0; j < order->fieldEntriesCount; ++j)
		{
			MarketField *entryData = &order->fieldEntries[j];
			if ((ret = rsslEncodeFieldEntry(pIter, 
							&entryData->fieldEntry, 
							(!entryData->isBlank) ? &entryData->primitive : NULL)) 
					< RSSL_RET_SUCCESS)
				return ret;
		}


		/* complete encode field list */
		if ((ret = rsslEncodeFieldListComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			return ret;

		/* complete map entry */
		if ((ret = rsslEncodeMapEntryComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			return ret;
	}

	/* complete map */
	if ((ret = rsslEncodeMapComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

MarketByOrderItem *createMarketByOrderItem()
{
	MarketByOrderItem* pMboItem = (MarketByOrderItem*)malloc(sizeof(MarketByOrderItem));
	clearMarketByOrderItem(pMboItem);
	return pMboItem;
}

void freeMarketByOrderItem(MarketByOrderItem* mboItem)
{
	free(mboItem);
}

