/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/* itemEncoder.h
 * Provides encoding of messages containing item data, in the forms of refreshes, updates, 
 * posts, and generic messages. */

#ifndef _ITEM_ENCODER_H
#define _ITEM_ENCODER_H

#include "marketPriceEncoder.h"
#include "marketByOrderEncoder.h"

#include "channelHandler.h"
#include "rotatingQueue.h"
#include "hashTable.h"

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"


#define NUM_CLIENT_SESSIONS 5


#ifdef __cplusplus
extern "C" {
#endif

/* Reasons an item request is rejected */
typedef enum {
	ITEM_COUNT_REACHED				= 1,	/* Consumer crossed the limit of allowed requests */
	INVALID_SERVICE_ID				= 2,  	/* Consumer used wrong service ID */
	ITEM_ALREADY_OPENED				= 3,	/* Same item requested more than once */
	STREAM_ALREADY_IN_USE			= 4,	/* Multiple item requests on the same stream. */
	QOS_NOT_SUPPORTED				= 5,	/* Unexpected QoS */
	DOMAIN_NOT_SUPPORTED			= 6		/* Unsupported domain */
} ItemRejectReason;


typedef enum {
	ITEM_IS_STREAMING_REQ		= 0x04,		/* Provider should send updates */
	ITEM_IS_SOLICITED			= 0x10,		/* Item was requested(not published) */
	ITEM_IS_POST				= 0x20,		/* Consumer should send posts */
	ITEM_IS_GEN_MSG				= 0x40,		/* Consumer should send generic messages */
	ITEM_IS_PRIVATE				= 0x80		/* Item should be requested on private stream */
} ItemFlags;

/* Stucture containing the attributes that uniquely identify an item. */
typedef struct
{
	RsslMsgKey	*pMsgKey;	/* Key for this item */
	RsslUInt8	domainType;	/* Domain of this item */
} RsslItemAttributes;

/* Contains information about a particular item. */
typedef struct {
	RsslQueueLink		watchlistLink;
	HashTableLink		itemAttributesTableLink;	/* Link for item attributes table in watchlist */
	HashTableLink		itemStreamIdTableLink;		/* Link for stream ID table in watchlist */
	RotatingQueue		*myQueue;					/* pointer to the queue this item is in. So we know which queue to remove it from */
	RsslInt32			StreamId;					/* Item's Stream ID */
	void*				itemData; 					/* Holds information about the item's data. This data will be different depending on the domain of the item. */
	RsslUInt8			itemFlags;					/* See ItemFlags struct */
	RsslItemAttributes	attributes;					/* Attributes that uniquely identify this item */
} ItemInfo;

/* Estimate an appropriate buffer size for the refresh message. */
RsslUInt32 estimateItemRefreshBufferLength(ItemInfo *itemInfo, RsslUInt32 protocol);

/* Encode a refresh message for the item. */
RsslRet encodeItemRefresh(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime);

/* Estimate an appropriate buffer size for the next update message. */
RsslUInt32 estimateItemUpdateBufferLength(ItemInfo *itemInfo, RsslUInt32 protocol);

/* Encode an update message for the item. */
RsslRet encodeItemUpdate(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime);

/* Estimate an appropriate buffer size for the next post message. */
RsslUInt32 estimateItemPostBufferLength(ItemInfo *itemInfo, RsslUInt32 protocol);

/* Encode a post message for the item. */
RsslRet encodeItemPost(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime);

/* Create a post message for the item. Used when VA Reactor Watchlist is enabled. */
RsslRet createItemPost(RsslChannel* chnl, ItemInfo* itemInfo, RsslPostMsg* pPostMsg, RsslBuffer* msgBuf,
					   RsslPostUserInfo *pPostUserInfo, RsslUInt encodeStartTime);

/* Estimate an appropriate buffer size for the next generic message. */
RsslUInt32 estimateItemGenMsgBufferLength(ItemInfo *itemInfo, RsslUInt32 protocol);

/* Encode a generic message for the item. */
RsslRet encodeItemGenMsg(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslUInt encodeStartTime);

/* Create a generic message for the item. Used when VA Reactor Watchlist is enabled. */
RsslRet createItemGenMsg(RsslChannel* chnl,
						 ItemInfo* itemInfo, RsslGenericMsg* pGenericMsg, RsslBuffer* msgBuf, RsslUInt encodeStartTime);

/* Encode a close status for the item. */
RsslRet encodeItemCloseStatus(RsslChannel* chnl, ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId);

/* Encode a close status for the item, with a particular rejection reason. */
RsslRet encodeItemRequestReject(RsslChannel* chnl, RsslInt32 streamId, ItemRejectReason reason, RsslBuffer* msgBuf, RsslUInt8 domain);

/* Clears an ItemInfo structure. */
RTR_C_INLINE void clearItemInfo(ItemInfo* itemInfo)
{
	itemInfo->itemFlags = 0;
	itemInfo->StreamId = 0;
	itemInfo->attributes.pMsgKey = NULL;
	itemInfo->itemData = 0;
	itemInfo->attributes.domainType = 0;
	itemInfo->myQueue = NULL;
}

RTR_C_INLINE const char* itemRejectReasonToString(ItemRejectReason rejectReason)
{
	switch (rejectReason)
	{
		case ITEM_COUNT_REACHED:
			return "ITEM_COUNT_REACHED";
		break;
		case INVALID_SERVICE_ID:
			return "INVALID_SERVICE_ID";
		break;
		case ITEM_ALREADY_OPENED:
			return "ITEM_ALREADY_OPENED";
		break;
		case STREAM_ALREADY_IN_USE:
			return "STREAM_ALREADY_IN_USE";
		break;
		case QOS_NOT_SUPPORTED:
			return "QOS_NOT_SUPPORTED";
		break;
		case DOMAIN_NOT_SUPPORTED:
			return "DOMAIN_NOT_SUPPORTED";
		break;
		default:
			return "Unknown reason";
	}
}


#ifdef __cplusplus
}
#endif

#endif

