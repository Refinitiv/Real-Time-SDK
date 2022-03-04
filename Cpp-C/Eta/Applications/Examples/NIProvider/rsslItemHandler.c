/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



/*
 * This is the item handler for the rsslNIProvider application.
 * It provides functions for sending mp(MarketPrice), mbo(MarketByOrder) refresh, update, and
 * close status message(s) to an ADH.  Functions for initializing the
 * item handler, getting the next stream id, adding items to
 * the item list, and updating item information are also provided.
 */

#include "rsslMarketPriceItems.h"
#include "rsslMarketByOrderItems.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryProvider.h"
#include "rsslSendMessage.h"
#include "rsslItemHandler.h"

#define ITEM_STREAM_ID_START 4 /* starting stream ID */

/* item information list */
static RsslItemInfo itemInfoList[MAX_ITEM_LIST_SIZE+ITEM_STREAM_ID_START];

/* next stream id for items */
static RsslInt32 nextStreamId = ITEM_STREAM_ID_START;

/* number of items in item list */
static RsslInt32 itemCount = 0;

/*
 * Initializes item handler internal structures.
 */
void initItemHandler()
{
	int i;

	/* clear item information list */
	for (i = 0; i < MAX_ITEM_LIST_SIZE+ITEM_STREAM_ID_START; i++)
	{
		clearItemInfo(&itemInfoList[i]);
	}
}

/*
 * Resets the refresh complete flag for all items.
 */
void resetRefreshComplete()
{
	int i;

	/* reset the refresh complete flag */
	for (i = 0; i < MAX_ITEM_LIST_SIZE+ITEM_STREAM_ID_START; i++)
	{
		itemInfoList[i].IsRefreshComplete = RSSL_FALSE;
	}
}

/*
 * Adds an item name requested by the application to
 * the item name list.
 * itemname - Item name requested by the application
 */
void addItemName(const char* itemname, RsslUInt8 domainType)
{
	RsslInt32 streamId = nextStreamId++;

	snprintf(itemInfoList[streamId].Itemname, 128, "%s", itemname);
	itemInfoList[streamId].InterestCount++;

	switch(domainType)
	{
	case RSSL_DMT_MARKET_PRICE:
		itemInfoList[streamId].domainType = RSSL_DMT_MARKET_PRICE;
		itemInfoList[streamId].itemData = (void*)getMarketPriceItem(itemname);
		break;
	case RSSL_DMT_MARKET_BY_ORDER:
		itemInfoList[streamId].domainType = RSSL_DMT_MARKET_BY_ORDER;
		itemInfoList[streamId].itemData = (void*)getMarketByOrderItem(itemname);
		break;
	default:
		printf("\nUnknown domain\n");
	}

	itemCount++;
}


/*
 * Updates any item information that's currently in use.
 */
void updateItemInfo()
{
	int i;

	for (i = ITEM_STREAM_ID_START; i < ITEM_STREAM_ID_START+itemCount; i++)
	{
		if (itemInfoList[i].InterestCount > 0)
		{
			switch(itemInfoList[i].domainType)
			{
			case RSSL_DMT_MARKET_PRICE:
				updateMarketPriceItemFields((RsslMarketPriceItem*)itemInfoList[i].itemData);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				updateMarketByOrderItemFields((RsslMarketByOrderItem*)itemInfoList[i].itemData);
				break;
			default:
				printf("\nUnknown domain\n");
			}
		}
	}
}

/*
 * Sends an item response to a channel.  This consists of getting
 * a message buffer, encoding the response, and sending the
 * response to the server.
 * chnl - The channel to send a response to
 * itemInfo - The item information
 * streamId - The stream id of the item response
 */

static RsslRet sendItemResponse(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* Encode the message with data appopriate for the domain */
		switch(itemInfo->domainType)
		{
		case RSSL_DMT_MARKET_PRICE:
			/* encode market price response */
			if (encodeMarketPriceResponse(chnl, itemInfo, msgBuf, RSSL_FALSE, streamId, RSSL_TRUE, RSSL_FALSE, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("\nencodeMarketPriceResponse() failed\n");
				return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			/* encode market by order response */
			if (encodeMarketByOrderResponse(chnl, itemInfo, msgBuf, RSSL_FALSE, streamId, RSSL_TRUE, RSSL_FALSE, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("\nencodeMarketByOrderResponse() failed\n");
				return RSSL_RET_FAILURE;
			}
			break;
		default:
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nReceived unhandled domain %u for item\n", itemInfo->domainType);
			return RSSL_RET_FAILURE;
			break;
		}

		/* set refresh complete flag if this is a refresh */
		if (itemInfo->IsRefreshComplete == RSSL_FALSE)
		{
			itemInfo->IsRefreshComplete = RSSL_TRUE;
		}

		/* send item response */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item update(s) for a channel.  This consists
 * of finding all item information for this channel, and sending
 * the responses to the channel.
 * chnl - The channel to send item update(s) to
 */
RsslRet sendItemUpdates(RsslChannel* chnl)
{
	int i;

	for (i = ITEM_STREAM_ID_START; i < ITEM_STREAM_ID_START+itemCount; i++)
	{
		if (itemInfoList[i].InterestCount > 0)
		{
			if (sendItemResponse(chnl, &itemInfoList[i], -i) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item close status message for a channel.
 * chnl - The channel to send close status message to
 * itemInfo - The item information
 * streamId - The stream id of the item close status
 */
static RsslRet sendItemCloseStatusMsg(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item close status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode item close status */
		if (encodeItemCloseStatus(chnl, itemInfo, msgBuf, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeItemCloseStatus() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close status */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Sends the item close status message(s) for a channel.
 * This consists of finding all item information for this channel
 * and sending the close status messages to the channel.
 * chnl - The channel to send close status message(s) to
 */
void sendItemCloseStatusMsgs(RsslChannel* chnl)
{
	int i;

	for (i = ITEM_STREAM_ID_START; i < ITEM_STREAM_ID_START+itemCount; i++)
	{
		if (itemInfoList[i].InterestCount > 0)
		{
			sendItemCloseStatusMsg(chnl, &itemInfoList[i], -i);
		}
	}
}



