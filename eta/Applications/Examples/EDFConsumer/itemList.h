/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "msgQueue.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"

#define MAX_ITEMS 10

/* Tracks the sequencing state of the item stream while synchronizing the
 * playback of the snapshot server messages with those from the realtime feed. */
typedef enum
{
	ITEM_SEQUENCE_NOT_STARTED,		/* No snapshot server messages received. Any
									 * realtime messages have been buffered. */
	ITEM_SEQUENCE_HAVE_SNAPSHOT,	/* We have received our first snapshot message. */
	ITEM_SEQUENCE_REALTIME_STARTED,	/* We have determined the starting point for
									 * sequencing realtime messages. */
	ITEM_SEQUENCE_DONE_REORDERING	/* Image from snapshot server has been
									 * replayed in order with realtime updates; no
									 * more reordering is needed. */
} ItemSequenceState;

/* Represents an item for which the consumer wants to get data. */
typedef struct
{
	RsslUInt8	domainType;					/* Domain type of the item (e.g. MarketPrice, 
											 * MarketByOrder, MarketByPrice) */
	RsslInt32	snapshotServerStreamId;		/* Stream ID used to request this item from the
											 * snapshot server. */
	RsslInt32	feedStreamId;				/* Real-Time feed's Stream ID of this item. */
	RsslBool	hasRefreshComplete;			/* Indicates whether a complete refresh has
											 * been received from the snapshot server. */
	ItemSequenceState	sequenceState;		/* Current state of reodering snapshot vs.
											 * realtime messages. */
	RsslUInt32	seqNum;						/* Sequence number used for reordering. */
	RsslBool	hasBufferedSnasphotMsgs;	/* Indicates if msgQueue contains
											 * messages from the snapshot server instead
											 * of realtime messages. */
	MsgQueue	msgQueue;					/* Queue where messages are buffered so that
											 * they can be played in the correct order. */
	char		symbolName[128];
	RsslUInt32	realTimeChannelId;		/* Index into the source directory connection information for real time streams */
	RsslUInt32	gapChannelId;			/* Index into the source directory connection information for gap fill streams */
} Item;

/* Represents a list of items for which the consumer wants to get data. */
typedef struct
{
	RsslUInt32	itemCount;					/* Number of items in the items array. */
	Item		items[MAX_ITEMS];			/* Items in which the consumer is interested. */
} ItemList;

/* The global item list. */
extern ItemList itemList;

/* Clears the item list. */
void itemListClear(ItemList *pItemList);

/* Adds an item to the list. */
RsslRet itemListAdd(ItemList *pItemList, RsslInt32 feedStreamId, char *pSymbolName, RsslUInt8 domainType);

/* Sets that the item has received a complete refresh for an item. */
RTR_C_INLINE void itemSetRefreshComplete(Item *pItem)
{ pItem->hasRefreshComplete = RSSL_TRUE; }

/* Gets an item structure from the list of requested items. */
Item *itemListGetItem(ItemList *pItemList, RsslInt32 feedStreamId, RsslUInt8 domainType);

/* Gets an item structure from the list of requested items based on our snapshot server stream ID. */
Item *itemListGetItemBySnapStreamID(ItemList *pItemList, RsslInt32 snapshotServerStreamId, RsslUInt8 domainType);

/* Processes a single message intended for the given item. */
RsslRet itemProcessMsg(Item *pItem, RsslDecodeIterator *pIter, RsslMsg *pRsslMsg,
		RsslBool isSnapshotServerMsg);

/* Free any resources associated with items in the item list. */
void itemListCleanup(ItemList *pItemList);

/* Decodes data of a message according to domain (e.g. MarketPrice, MarketByOrder, MarketByPrice). */
RsslRet itemDecodeMsgData(Item *pItem, RsslDecodeIterator *pIter, RsslMsg *pRsslMsg);

#endif
