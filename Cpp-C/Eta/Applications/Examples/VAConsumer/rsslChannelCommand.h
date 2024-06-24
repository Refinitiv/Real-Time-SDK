/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_CHANNEL_COMMAND_H
#define _RTR_RSSL_CHANNEL_COMMAND_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslRDMMsg.h"

#include "rsslVACacheHandler.h"
#include "simpleTunnelMsgHandler.h"

#include <stdlib.h>

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BUFFER_LENGTH 128
#define MAX_NUM_GROUP_ID 10
#define MAX_NUM_CAPABILITIES 256
/*
 * Store information associated with
 * an item request.
 */
typedef struct
{
	char itemNameString[128];
	RsslBuffer itemName;
	RsslInt32 streamId;
	RsslState itemState;
	RsslInt8 groupIdIndex;
	RsslPayloadEntryHandle cacheEntryHandle;
} ItemRequest;

/* defines the maximum range of streamIds within a domain */
#define MAX_STREAM_ID_RANGE_PER_DOMAIN 100

#define CHAN_CMD_MAX_ITEMS				MAX_STREAM_ID_RANGE_PER_DOMAIN/2

/* non-private stream id starting point */
#define MARKETPRICE_STREAM_ID_START 6
#define MARKETPRICE_BATCH_STREAM_ID_START 5

#define MARKET_BY_ORDER_STREAM_ID_START 107
#define MARKET_BY_ORDER_BATCH_STREAM_ID_START 106

#define MARKET_BY_PRICE_STREAM_ID_START 208
#define MARKET_BY_PRICE_BATCH_STREAM_ID_START 207

#define YIELD_CURVE_STREAM_ID_START 309
#define YIELD_CURVE_BATCH_STREAM_ID_START 308

#define SYMBOL_LIST_STREAM_ID_START 500

/* private stream id starting point */
#define MARKETPRICE_PRIVATE_STREAM_ID_START (MARKETPRICE_STREAM_ID_START + 50)
#define MARKETPRICE_BATCH_PRIVATE_STREAM_ID_START (MARKETPRICE_BATCH_STREAM_ID_START + 50)

#define MARKET_BY_ORDER_PRIVATE_STREAM_ID_START (MARKET_BY_ORDER_STREAM_ID_START + 50)
#define MARKET_BY_ORDER_BATCH_PRIVATE_STREAM_ID_START (MARKET_BY_ORDER_BATCH_STREAM_ID_START + 50)

#define MARKET_BY_PRICE_PRIVATE_STREAM_ID_START (MARKET_BY_PRICE_STREAM_ID_START + 50)
#define MARKET_BY_PRICE_BATCH_PRIVATE_STREAM_ID_START (MARKET_BY_PRICE_BATCH_STREAM_ID_START + 50) 

#define YIELD_CURVE_PRIVATE_STREAM_ID_START (YIELD_CURVE_STREAM_ID_START + 50)
#define YIELD_CURVE_BATCH_PRIVATE_STREAM_ID_START (YIELD_CURVE_BATCH_STREAM_ID_START + 50) 
/*
 * Contains information associated with each open channel
 * in the rsslVAConsumer.
 */
typedef struct
{
	RsslReactorChannel *reactorChannel;
	RsslReactorConnectOptions cOpts;
	RsslReactorConnectInfo cInfo;
	RsslReactorChannelRole *pRole;
	ItemRequest marketPriceItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest marketByOrderItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest marketByPriceItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest yieldCurveItems[CHAN_CMD_MAX_ITEMS];

	ItemRequest marketPricePSItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest marketByOrderPSItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest marketByPricePSItems[CHAN_CMD_MAX_ITEMS];
	ItemRequest yieldCurvePSItems[CHAN_CMD_MAX_ITEMS];

	ItemRequest marketPriceBatchRequest;
	ItemRequest marketByOrderBatchRequest;
	ItemRequest marketByPriceBatchRequest;
	ItemRequest yieldCurveBatchRequest;

	ItemRequest privateStreamMarketPriceBatchRequest;
	ItemRequest privateStreamMarketByOrderBatchRequest;
	ItemRequest privateStreamMarketByPriceBatchRequest;
	ItemRequest privateStreamYieldCurveBatchRequest;

	ItemRequest symbolListRequest;
	RsslBool sendSymbolList, /* User requests symbol list for this channel */
			 userSpecSymbolList, /* User requested a specific symbol list for this channel */
			 foundItemList; /* SymbolList was found in source directory response */

	/* number of items in non-private stream item list */
	RsslInt32 marketPriceItemCount, marketByOrderItemCount, marketByPriceItemCount, yieldCurveItemCount;

	/* number of items in private stream item list */
	RsslInt32 privateStreamMarketPriceItemCount, privateStreamMarketByOrderItemCount, privateStreamMarketByPriceItemCount, privateStreamYieldCurveItemCount;

	/* next non-private stream id for item requests */
	RsslInt32 nextAvailableMarketPriceStreamId, nextAvailableMarketByOrderStreamId, nextAvailableMarketByPriceStreamId, nextAvailableYieldCurveStreamId;

	/* next private stream id for item requests */
	RsslInt32 nextAvailableMarketPricePrivateStreamId, nextAvailableMarketByOrderPrivateStreamId, nextAvailableMarketByPricePrivateStreamId, nextAvailableYieldCurvePrivateStreamId; 

	char hostName[MAX_BUFFER_LENGTH];
	char port[MAX_BUFFER_LENGTH];
	char interfaceName[MAX_BUFFER_LENGTH];
	
	char loginRefreshMemoryArray[4000];
	RsslBuffer loginRefreshMemory;
	RsslRDMLoginRefresh loginRefresh;

	/* service name requested by application */
	char serviceName[MAX_BUFFER_LENGTH];
	/* service id associated with the service name requested by application */
	RsslUInt serviceId;
	/* service name found flag */
	RsslBool serviceNameFound;
	RsslBool reactorChannelReady;
	RsslBool reactorChannelClosed;
	RsslBool itemsRequested;
	RsslBool isServiceReady;
	RsslQos qos;

	/* Service information for tunnel stream. */
	RsslBool tunnelMessagingEnabled;
	char tunnelStreamServiceName[MAX_BUFFER_LENGTH];
	SimpleTunnelMsgHandler simpleTunnelMsgHandler;

	RsslDataDictionary dictionary;
	RsslBool dictionariesLoadedFromFile, dictionariesLoaded;
	RsslInt32 fieldDictionaryStreamId, enumDictionaryStreamId;

	RsslUInt			capabilities[MAX_NUM_CAPABILITIES];
	RsslUInt32			capabilitiesCount;
	RsslBuffer			groupIdBuffers[MAX_NUM_GROUP_ID];
	RsslUInt32			groupIdCount;

	RsslVACacheInfo		cacheInfo;

	/* For Posting */
	RsslBool postEnabled;
	RsslBool isRefreshComplete;
	time_t nextPostTime;
	RsslBool postWithMsg;
	RsslBool shouldOffStreamPost;
	RsslBool shouldOnStreamPost;
	RsslBool offstreamPostSent;
	RsslUInt32 nextPostId;
	RsslUInt32 nextSeqNum;
	double itemData;
	ItemRequest *pPostItem;

	/* For UserAuthn authentication login reissue */
	RsslUInt loginReissueTime; // represented by epoch time in seconds
	RsslBool canSendLoginReissue;

	/* For retrieving the channel statistics */
	time_t nextStatisticRetrivalTime;
	RsslReactorChannelStatistic	channelStatistic;
} ChannelCommand;

/*
 * initializes the ChannelCommand
 */ 
RTR_C_INLINE void initChannelCommand(ChannelCommand *pCommand)
{
	int i;
	memset(pCommand, 0, sizeof(ChannelCommand));
	rsslClearReactorConnectOptions(&pCommand->cOpts);
	rsslClearReactorConnectInfo(&pCommand->cInfo);
	rsslClearRDMLoginRefresh(&pCommand->loginRefresh);
	rsslClearReactorChannelStatistic(&pCommand->channelStatistic);
	pCommand->loginRefreshMemory.data = pCommand->loginRefreshMemoryArray;
	pCommand->loginRefreshMemory.length = sizeof(pCommand->loginRefreshMemoryArray);

	pCommand->dictionariesLoaded = pCommand->dictionariesLoadedFromFile;
	if (!pCommand->dictionariesLoaded)
	{
		rsslDeleteDataDictionary(&pCommand->dictionary);
		rsslClearDataDictionary(&pCommand->dictionary);
	}


	pCommand->nextAvailableMarketPriceStreamId = MARKETPRICE_STREAM_ID_START;
	pCommand->nextAvailableMarketPricePrivateStreamId = MARKETPRICE_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableMarketByOrderStreamId = MARKET_BY_ORDER_STREAM_ID_START;
	pCommand->nextAvailableMarketByOrderPrivateStreamId = MARKET_BY_ORDER_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableMarketByPriceStreamId = MARKET_BY_PRICE_STREAM_ID_START;
	pCommand->nextAvailableMarketByPricePrivateStreamId = MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableYieldCurveStreamId = YIELD_CURVE_STREAM_ID_START;
	pCommand->nextAvailableYieldCurvePrivateStreamId = YIELD_CURVE_PRIVATE_STREAM_ID_START;
	pCommand->nextPostTime = 0;
	pCommand->postWithMsg = RSSL_TRUE;
	pCommand->nextPostId = 1;
	pCommand->nextSeqNum = 1;
	pCommand->itemData = 12.00;
	pCommand->isRefreshComplete = RSSL_FALSE;
	pCommand->postEnabled = RSSL_FALSE;
	pCommand->shouldOffStreamPost = RSSL_FALSE;
	pCommand->shouldOnStreamPost = RSSL_FALSE;
	pCommand->offstreamPostSent = RSSL_FALSE;
	pCommand->foundItemList = RSSL_FALSE;
	pCommand->fieldDictionaryStreamId = 0;
	pCommand->enumDictionaryStreamId = 0;
	pCommand->serviceNameFound = RSSL_FALSE;
	pCommand->reactorChannelReady = RSSL_FALSE;
	pCommand->reactorChannelClosed = RSSL_FALSE;
	pCommand->itemsRequested = RSSL_FALSE;
	pCommand->isServiceReady = RSSL_FALSE;
	pCommand->cacheInfo.useCache = RSSL_FALSE;
	pCommand->cacheInfo.cacheHandle = 0;
	pCommand->cacheInfo.cacheDictionaryKey[0] = 0;
	rsslCacheErrorClear(&pCommand->cacheInfo.cacheErrorInfo);
	for (i = 0; i < MAX_NUM_GROUP_ID; i++)
	{
		pCommand->groupIdBuffers[i].data = 0;
		pCommand->groupIdBuffers[i].length = 0;
	}
	pCommand->groupIdCount = 0;
	for (i = 0; i < MAX_NUM_CAPABILITIES; i++)
	{
		pCommand->capabilities[i] = 0;
	}
	pCommand->capabilitiesCount = 0;

	pCommand->tunnelMessagingEnabled = RSSL_FALSE;
	snprintf(pCommand->tunnelStreamServiceName, sizeof(pCommand->tunnelStreamServiceName), "%s", "");
}

/*
 * Cleans up a ChannelCommand
 */
RTR_C_INLINE void cleanupChannelCommand(ChannelCommand *pCommand)
{
	rsslDeleteDataDictionary(&pCommand->dictionary);
}

/*
 * Get the group id assignment for an item
 * Returns a group id index [0, MAX_NUM_GROUP_ID-1) if item belongs to a group.
 * Returns -1 if item is not in a group.
 */
RTR_C_INLINE RsslInt8 getGroupIdIndex(ChannelCommand *pCommand, RsslBuffer *pGroupIdBuffer)
{
	RsslUInt8 i;
	for (i = 0; i < pCommand->groupIdCount; i++)
	{
		if (rsslBufferIsEqual(pGroupIdBuffer, &pCommand->groupIdBuffers[i]))
			return i;
	}

	return -1;
}

/*
 * Assign the item to a group based on the group id buffer
 * Returns the index [0, MAX_NUM_GROUP_ID-1) of the assigned group
 */
RTR_C_INLINE RsslInt8 setGroupId(ChannelCommand *pCommand, ItemRequest *pItem, RsslBuffer *pGroupIdBuffer)
{
	RsslInt8 groupIdIndex;

	if (pGroupIdBuffer->length == 0 )
	{
		printf("setGroupId failure: Group Id buffer is empty\n");
		return -1;
	}

	groupIdIndex = getGroupIdIndex(pCommand, pGroupIdBuffer);
	if (groupIdIndex < 0)
	{
		if (pCommand->groupIdCount >= MAX_NUM_GROUP_ID)
		{
			printf("setGroupId failure: Exceeded maximum number of Group Ids (%d) supported by application\n", MAX_NUM_GROUP_ID);
			return -1;
		}

		groupIdIndex = pCommand->groupIdCount;
		pCommand->groupIdCount++;

		pCommand->groupIdBuffers[groupIdIndex].data = (char*) malloc(pGroupIdBuffer->length);
		pCommand->groupIdBuffers[groupIdIndex].length = pGroupIdBuffer->length;

		memcpy(pCommand->groupIdBuffers[groupIdIndex].data, pGroupIdBuffer->data, pGroupIdBuffer->length);
	}

	pItem->groupIdIndex = groupIdIndex;

	return groupIdIndex;
}

/*
 * Set state of a group of items (matching the groupIdIndex) or all items (groupIdIndex == -1)
 */
RTR_C_INLINE void setItemStates(ChannelCommand* pCommand, RsslInt8 groupIdIndex, RsslState *pState)
{
	int i;

	for(i = 0; i < pCommand->marketPriceItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketPriceItems[i].groupIdIndex)
		{
			pCommand->marketPriceItems[i].itemState.streamState = pState->streamState;
			pCommand->marketPriceItems[i].itemState.dataState = pState->dataState;
			pCommand->marketPriceItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketPriceItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketPricePSItems[i].groupIdIndex)
		{
			pCommand->marketPricePSItems[i].itemState.streamState = pState->streamState;
			pCommand->marketPricePSItems[i].itemState.dataState = pState->dataState;
			pCommand->marketPricePSItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->marketByOrderItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketByOrderItems[i].groupIdIndex)
		{
			pCommand->marketByOrderItems[i].itemState.streamState = pState->streamState;
			pCommand->marketByOrderItems[i].itemState.dataState = pState->dataState;
			pCommand->marketByOrderItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketByOrderItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketByOrderPSItems[i].groupIdIndex)
		{
			pCommand->marketByOrderPSItems[i].itemState.streamState = pState->streamState;
			pCommand->marketByOrderPSItems[i].itemState.dataState = pState->dataState;
			pCommand->marketByOrderPSItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->marketByPriceItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketByPriceItems[i].groupIdIndex)
		{
			pCommand->marketByPriceItems[i].itemState.streamState = pState->streamState;
			pCommand->marketByPriceItems[i].itemState.dataState = pState->dataState;
			pCommand->marketByPriceItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->privateStreamMarketByPriceItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->marketByPricePSItems[i].groupIdIndex)
		{
			pCommand->marketByPricePSItems[i].itemState.streamState = pState->streamState;
			pCommand->marketByPricePSItems[i].itemState.dataState = pState->dataState;
			pCommand->marketByPricePSItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->yieldCurveItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->yieldCurveItems[i].groupIdIndex)
		{
			pCommand->yieldCurveItems[i].itemState.streamState = pState->streamState;
			pCommand->yieldCurveItems[i].itemState.dataState = pState->dataState;
			pCommand->yieldCurveItems[i].itemState.code = pState->code;
		}
	}
	for(i = 0; i < pCommand->privateStreamYieldCurveItemCount; i++)
	{
		if (groupIdIndex == -1 || groupIdIndex == pCommand->yieldCurvePSItems[i].groupIdIndex)
		{
			pCommand->yieldCurvePSItems[i].itemState.streamState = pState->streamState;
			pCommand->yieldCurvePSItems[i].itemState.dataState = pState->dataState;
			pCommand->yieldCurvePSItems[i].itemState.code = pState->code;
		}
	}
}

/*
 * Clears a ChannelCommand.
 */
RTR_C_INLINE void clearChannelCommand(ChannelCommand *pCommand)
{
	RsslState state;
	int i;

	pCommand->dictionariesLoaded = pCommand->dictionariesLoadedFromFile;
    if (!pCommand->dictionariesLoaded)
    {
        /* Will request dictionary when channel is connected. */
		pCommand->fieldDictionaryStreamId = 0;
		pCommand->enumDictionaryStreamId = 0;
    }

	rsslClearRDMLoginRefresh(&pCommand->loginRefresh);
	pCommand->loginRefreshMemory.data = pCommand->loginRefreshMemoryArray;
	pCommand->loginRefreshMemory.length = sizeof(pCommand->loginRefreshMemoryArray);
	pCommand->nextAvailableMarketPriceStreamId = MARKETPRICE_STREAM_ID_START;
	pCommand->nextAvailableMarketPricePrivateStreamId = MARKETPRICE_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableMarketByOrderStreamId = MARKET_BY_ORDER_STREAM_ID_START;
	pCommand->nextAvailableMarketByOrderPrivateStreamId = MARKET_BY_ORDER_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableMarketByPriceStreamId = MARKET_BY_PRICE_STREAM_ID_START;
	pCommand->nextAvailableMarketByPricePrivateStreamId = MARKET_BY_PRICE_PRIVATE_STREAM_ID_START;
	pCommand->nextAvailableYieldCurveStreamId = YIELD_CURVE_STREAM_ID_START;
	pCommand->nextAvailableYieldCurvePrivateStreamId = YIELD_CURVE_PRIVATE_STREAM_ID_START;
    pCommand->postWithMsg = RSSL_TRUE;
    pCommand->nextPostTime = 0;
	pCommand->nextPostId = 1;
	pCommand->nextSeqNum = 1;
	pCommand->itemData = 12.00;
	pCommand->postEnabled = RSSL_FALSE;
	pCommand->isRefreshComplete = RSSL_FALSE;
	pCommand->offstreamPostSent = RSSL_FALSE;
	pCommand->shouldOffStreamPost = RSSL_FALSE;
	pCommand->shouldOnStreamPost = RSSL_FALSE;
	pCommand->serviceNameFound = RSSL_FALSE;
	pCommand->reactorChannelReady = RSSL_FALSE;
	pCommand->reactorChannelClosed = RSSL_FALSE;
	pCommand->itemsRequested = RSSL_FALSE;
	pCommand->sendSymbolList = RSSL_FALSE;
	pCommand->userSpecSymbolList = RSSL_FALSE;
	pCommand->foundItemList = RSSL_FALSE;
	pCommand->canSendLoginReissue = RSSL_FALSE;

	state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	state.dataState = RSSL_DATA_SUSPECT;
	state.code = 0;
	setItemStates(pCommand, -1, &state);
	for (i = 0; i < MAX_NUM_CAPABILITIES; i++)
	{
		pCommand->capabilities[i] = 0;
	}
	pCommand->capabilitiesCount = 0;

	tunnelStreamHandlerClearServiceInfo(&pCommand->simpleTunnelMsgHandler.tunnelStreamHandler);

}


RTR_C_INLINE RsslInt32 getNextAvailableMarketPriceStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketPriceStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableMarketPricePrivateStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketPricePrivateStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableMarketByOrderStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketByOrderStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableYieldCurveStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableYieldCurveStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableMarketByOrderPrivateStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketByOrderPrivateStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableMarketByPriceStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketByPriceStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableMarketByPricePrivateStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableMarketByPricePrivateStreamId++;
}

RTR_C_INLINE RsslInt32 getNextAvailableYieldCurvePrivateStreamId(ChannelCommand *pChannelCommand)
{
	return pChannelCommand->nextAvailableYieldCurvePrivateStreamId++;
}

#ifdef __cplusplus
};
#endif

#endif
