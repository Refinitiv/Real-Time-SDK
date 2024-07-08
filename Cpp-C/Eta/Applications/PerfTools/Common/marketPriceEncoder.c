/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "marketPriceEncoder.h"
#include "itemEncoder.h"
#include "rtr/rsslGetTime.h"
#include "xmlMsgDataParser.h"

#include <assert.h>

RsslUInt32 getNextMarketPriceUpdateEstimatedContentLength(MarketPriceItem *mpItem)
{
	return xmlMarketPriceMsgs.updateMsgs[mpItem->iMsg].estimatedContentLength;
}

MarketPriceMsg *getNextMarketPriceUpdate(MarketPriceItem *mpItem)
{
	MarketPriceMsg *mpMsg = &xmlMarketPriceMsgs.updateMsgs[mpItem->iMsg++];
	if (mpItem->iMsg == xmlMarketPriceMsgs.updateMsgCount) mpItem->iMsg = 0;
	return mpMsg;
}

RsslInt32 getMarketPriceUpdateMsgCount()
{
	return xmlMarketPriceMsgs.updateMsgCount;
}

RsslUInt32 getNextMarketPricePostEstimatedContentLength(MarketPriceItem *mpItem)
{
	return xmlMarketPriceMsgs.postMsgs[mpItem->iMsg].estimatedContentLength;
}

MarketPriceMsg *getNextMarketPricePost(MarketPriceItem *mpItem)
{
	MarketPriceMsg *mpMsg = &xmlMarketPriceMsgs.postMsgs[mpItem->iMsg++];
	if (mpItem->iMsg == xmlMarketPriceMsgs.postMsgCount) mpItem->iMsg = 0;
	return mpMsg;
}

RsslInt32 getMarketPricePostMsgCount()
{
	return xmlMarketPriceMsgs.postMsgCount;
}

RsslUInt32 getNextMarketPriceGenMsgEstimatedContentLength(MarketPriceItem *mpItem)
{
	return xmlMarketPriceMsgs.genMsgs[mpItem->iMsg].estimatedContentLength;
}

MarketPriceMsg *getNextMarketPriceGenMsg(MarketPriceItem *mpItem)
{
	MarketPriceMsg *mpMsg = &xmlMarketPriceMsgs.genMsgs[mpItem->iMsg++];
	if (mpItem->iMsg == xmlMarketPriceMsgs.genMsgCount) mpItem->iMsg = 0;
	return mpMsg;
}

RsslInt32 getMarketPriceGenMsgCount()
{
	return xmlMarketPriceMsgs.genMsgCount;
}


RsslRet encodeMarketPriceDataBody(RsslEncodeIterator *pIter, MarketPriceMsg *mpMsg,
		RsslMsgClasses msgClass, RsslUInt encodeStartTime)
{
	RsslFieldList fList;
	RsslFieldEntry fEntry;
	RsslInt32 i;
	RsslRet ret;

	assert(!(msgClass == RSSL_MC_REFRESH && encodeStartTime));

	/* encode field list */
	rsslClearFieldList(&fList);
	rsslClearFieldEntry(&fEntry);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(pIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
		return ret;

	for(i = 0; i < mpMsg->fieldEntriesCount; ++i)
	{
		if ((ret = rsslEncodeFieldEntry(pIter, 
						&mpMsg->fieldEntries[i].fieldEntry, 
						(!mpMsg->fieldEntries[i].isBlank) ? &mpMsg->fieldEntries[i].primitive : NULL)) 
				< RSSL_RET_SUCCESS)
			return ret;
	}

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

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

MarketPriceItem *createMarketPriceItem()
{
	MarketPriceItem* pMpItem = (MarketPriceItem*)malloc(sizeof(MarketPriceItem));
	clearMarketPriceItem(pMpItem);
	return pMpItem;
}

void freeMarketPriceItem(MarketPriceItem* mpItem)
{
	free(mpItem);
}

