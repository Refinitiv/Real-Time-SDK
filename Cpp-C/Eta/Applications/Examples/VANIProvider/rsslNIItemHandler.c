/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslNIItemHandler.h"
#include "rsslNIProvider.h"
#include "rsslVACacheHandler.h"


static RsslRet buildRefreshResponse(NIChannelCommand *chnlCommand, RsslNIItemInfo* itemInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo);
static RsslRet buildUpdateResponse(NIChannelCommand *chnlCommand, RsslNIItemInfo* itemInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo);

/*
 * This is the item handler for the rsslVANIProvider application.
 * It provides functions for sending mp(MarketPrice), mbo(MarketByOrder) refresh, update, and
 * close status message(s) to an ADH.  Functions for initializing the
 * item handler, getting the next stream id, adding items to
 * the item list, and updating item information are also provided.
 */

/*
 * Updates any item information that's currently in use.
 */
void updateItemInfo(NIChannelCommand* chnlCommand)
{
	int i;

	for (i = 0; i < 5; i++)
	{
		if (chnlCommand->marketPriceItemInfo[i].isActive)
		{

			updateMarketPriceItemFields((RsslMarketPriceItem*)chnlCommand->marketPriceItemInfo[i].itemData);
		}
		
		if (chnlCommand->marketByOrderItemInfo[i].isActive)
		{
			updateMarketByOrderItemFields((RsslMarketByOrderItem*)chnlCommand->marketByOrderItemInfo[i].itemData);

		}
	}
}

/*
 * Sends an item response to a channel.  This consists of getting
 * a message buffer, encoding the response, and sending the
 * response to the consumer.
 * chnl - The channel to send a response to
 * itemReqInfo - The item request information
 */
static RsslRet sendItemResponse(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslNIItemInfo* itemInfo)
{
	RsslErrorInfo error;
	RsslRet ret;
	RsslBuffer* msgBuf = 0;
	NIChannelCommand *chnlCommand;
	RsslVACacheInfo *pCacheInfo;
	
	chnlCommand = (NIChannelCommand*)chnl->userSpecPtr;
	pCacheInfo = getCacheInfo();
	
	/* get a buffer for the response */
	msgBuf = rsslReactorGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{

		/* Encode the message with data appopriate for the domain */
		switch(itemInfo->domainType)
		{
		case RSSL_DMT_MARKET_PRICE:
		case RSSL_DMT_MARKET_BY_ORDER:
			if (itemInfo->IsRefreshComplete)
				ret = buildUpdateResponse(chnlCommand, itemInfo, msgBuf, pCacheInfo);
			else
			{
				ret = buildRefreshResponse(chnlCommand, itemInfo, msgBuf, pCacheInfo);
				itemInfo->IsRefreshComplete = RSSL_TRUE;
			}

			if (ret != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(chnl, msgBuf, &error);
				printf("\nFailed to build item response message: Error %d\n", ret);
				return RSSL_RET_FAILURE;
			}
			break;
		default:
			rsslReactorReleaseBuffer(chnl, msgBuf, &error);
			printf("\nReceived unhandled domain %u for item\n", itemInfo->domainType);
			return RSSL_RET_FAILURE;
			break;
		}

		/* send item response */
		if (sendMessage(pReactor, chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", error.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item update(s) for a channel.  This consists
 * of finding all request information for this channel, and sending
 * the responses to the channel.
 * chnl - The channel to send update(s) to
 */
RsslRet sendItemUpdates(RsslReactor *pReactor, RsslReactorChannel* chnl)
{
	NIChannelCommand* chnlCommand = (NIChannelCommand*)chnl->userSpecPtr;
	int i;
	int itemCount = 0;

	for (i = 0; i < 5; i++)
	{
		if (chnlCommand->marketPriceItemInfo[i].isActive)
		{
			if (sendItemResponse(pReactor, chnl, &chnlCommand->marketPriceItemInfo[i]) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			itemCount++;
		}
		
		if(chnlCommand->marketByOrderItemInfo[i].isActive)
		{
			if (sendItemResponse(pReactor, chnl, &chnlCommand->marketByOrderItemInfo[i]) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			itemCount++;
		}
	}

	printf("Sent %i items.\n", itemCount);

	return RSSL_RET_SUCCESS;
}

/*
 * Cache the item data for a channel without sending (channel down scenario)
 * chnlCommand - channel and item information for this connection
 */
RsslRet cacheItemData(NIChannelCommand* chnlCommand)
{
	int i;
	int itemCount = 0;
	RsslBuffer msgBuf;
	RsslVACacheInfo *pCacheInfo;
	char msgArray[500];

	msgBuf.data = msgArray;
	msgBuf.length = sizeof(msgArray);
	pCacheInfo = getCacheInfo();

	for (i = 0; i < 5; i++)
	{
		if (chnlCommand->marketPriceItemInfo[i].isActive)
		{
			itemCount++;
			msgBuf.length = sizeof(msgArray);

			if (chnlCommand->marketPriceItemInfo[i].cacheEntryHandle &&
				rsslPayloadEntryGetDataType(chnlCommand->marketPriceItemInfo[i].cacheEntryHandle) != RSSL_DT_UNKNOWN)
			{
				if (buildUpdateResponse(chnlCommand, &chnlCommand->marketPriceItemInfo[i], &msgBuf, pCacheInfo) != RSSL_RET_SUCCESS)
					printf("cacheItemData: Failed to cache update\n");
			}
			else
			{
				if (buildRefreshResponse(chnlCommand, &chnlCommand->marketPriceItemInfo[i], &msgBuf, pCacheInfo) != RSSL_RET_SUCCESS)
					printf("cacheItemData: Failed to cache refresh\n");
			}
		}

		if(chnlCommand->marketByOrderItemInfo[i].isActive)
		{
			itemCount++;
			msgBuf.length = sizeof(msgArray);

			if (chnlCommand->marketByOrderItemInfo[i].cacheEntryHandle &&
				rsslPayloadEntryGetDataType(chnlCommand->marketByOrderItemInfo[i].cacheEntryHandle) != RSSL_DT_UNKNOWN)
			{
				if (buildUpdateResponse(chnlCommand, &chnlCommand->marketByOrderItemInfo[i], &msgBuf, pCacheInfo) != RSSL_RET_SUCCESS)
					printf("cacheItemData: Failed to cache update\n");
			}
			else
			{
				if (buildRefreshResponse(chnlCommand, &chnlCommand->marketByOrderItemInfo[i], &msgBuf, pCacheInfo) != RSSL_RET_SUCCESS)
					printf("cacheItemData: Failed to cache refresh\n");
			}
		}
	}

	printf("Cached %i items.\n", itemCount);

	return RSSL_RET_SUCCESS;
}


/*
 * Builds a refresh response message into the given message buffer.
 * chnlCommand - the channel information for this publisher's connection
 * itemReqInfo - the request for the item from the client
 * msgBuf - the buffer where the response will be encoded
 * pCacheInfo - cache information to support optional use of cache while encoding message
 */
static RsslRet buildRefreshResponse(NIChannelCommand *chnlCommand, RsslNIItemInfo* itemInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;
	RsslEncodeIterator eIter;
	RsslBool applyToCache = RSSL_FALSE;

	rsslClearEncodeIterator(&eIter);
	if (chnlCommand->reactorChannel)
		rsslSetEncodeIteratorRWFVersion(&eIter, chnlCommand->reactorChannel->majorVersion, chnlCommand->reactorChannel->minorVersion);
	else
		rsslSetEncodeIteratorRWFVersion(&eIter, chnlCommand->cOpts.rsslConnectOptions.majorVersion, chnlCommand->cOpts.rsslConnectOptions.minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	if (itemInfo->domainType == RSSL_DMT_MARKET_PRICE)
		ret = encodeMarketPriceResponseMsgInit(itemInfo, &eIter, itemInfo->streamId, chnlCommand->serviceId, RSSL_TRUE);
	else if (itemInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
		ret = encodeMarketByOrderResponseMsgInit(itemInfo, &eIter, itemInfo->streamId, chnlCommand->serviceId, RSSL_TRUE);
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nFailed to encode response message init\n");
		return ret;
	}

	if (pCacheInfo->useCache && itemInfo->cacheEntryHandle && rsslPayloadEntryGetDataType(itemInfo->cacheEntryHandle) != RSSL_DT_UNKNOWN)
	{
		/* If item is cached, encode payload from cache entry */
		printf("Encoding refresh for item %s from cache.\n", itemInfo->Itemname);
		if ((ret = retrieveFromCache(&eIter, itemInfo->cacheEntryHandle, pCacheInfo)) != RSSL_RET_SUCCESS)
		{
			printf("Error %d retrieving payload from cache\n", ret);
			return ret;
		}
	}
	else
	{
		/* If not cached, encode data from (simulated) data source */
		if (itemInfo->domainType == RSSL_DMT_MARKET_PRICE)
			ret = encodeMPFieldList((RsslMarketPriceItem*)(itemInfo->itemData), &eIter, chnlCommand->dictionary, RSSL_TRUE);
		else if (itemInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
			ret = encodeMarketByOrderMap((RsslMarketByOrderItem*)(itemInfo->itemData), &eIter, chnlCommand->dictionary, RSSL_TRUE);
		else
			ret = RSSL_RET_FAILURE;

		if (ret != RSSL_RET_SUCCESS)
		{
			printf("Error encoding response payload\n");
			return ret;
		}
		applyToCache = RSSL_TRUE;
	}

	if ((ret = encodeResponseMsgComplete(&eIter)) != RSSL_RET_SUCCESS)
	{
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	if (pCacheInfo->useCache && applyToCache)
	{
		printf("Applying refresh for item %s to cache.\n", itemInfo->Itemname);
		applyMsgBufferToCache(eIter._majorVersion, eIter._minorVersion, &itemInfo->cacheEntryHandle, getCacheInfo(), msgBuf);
	}

	return ret;
}

/*
 * Builds an update response message into the given message buffer.
 * chnlCommand - the channel information for this publisher's connection
 * itemReqInfo - the request for the item from the client
 * msgBuf - the buffer where the response will be encoded
 * pCacheInfo - cache information to support optional use of cache while encoding message
 */
static RsslRet buildUpdateResponse(NIChannelCommand *chnlCommand, RsslNIItemInfo* itemInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;
	RsslEncodeIterator eIter;

	rsslClearEncodeIterator(&eIter);
	if (chnlCommand->reactorChannel)
		rsslSetEncodeIteratorRWFVersion(&eIter, chnlCommand->reactorChannel->majorVersion, chnlCommand->reactorChannel->minorVersion);
	else
		rsslSetEncodeIteratorRWFVersion(&eIter, chnlCommand->cOpts.rsslConnectOptions.majorVersion, chnlCommand->cOpts.rsslConnectOptions.minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	if (itemInfo->domainType == RSSL_DMT_MARKET_PRICE)
		ret = encodeMarketPriceResponseMsgInit(itemInfo, &eIter, itemInfo->streamId, chnlCommand->serviceId, RSSL_FALSE);
	else if (itemInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
		ret = encodeMarketByOrderResponseMsgInit(itemInfo, &eIter, itemInfo->streamId, chnlCommand->serviceId, RSSL_FALSE);
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nFailed to encode response message init\n");
		return ret;
	}

	if (itemInfo->domainType == RSSL_DMT_MARKET_PRICE)
		ret = encodeMPFieldList((RsslMarketPriceItem*)(itemInfo->itemData), &eIter, chnlCommand->dictionary, RSSL_FALSE);
	else if (itemInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
		ret = encodeMarketByOrderMap((RsslMarketByOrderItem*)(itemInfo->itemData), &eIter, chnlCommand->dictionary, RSSL_FALSE);
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("Error encoding Update payload\n");
		return RSSL_RET_FAILURE;
	}

	if ((ret = encodeResponseMsgComplete(&eIter)) != RSSL_RET_SUCCESS)
	{
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	if (pCacheInfo->useCache)
	{
		applyMsgBufferToCache(eIter._majorVersion, eIter._minorVersion, &itemInfo->cacheEntryHandle, getCacheInfo(), msgBuf);
	}

	return ret;
}
