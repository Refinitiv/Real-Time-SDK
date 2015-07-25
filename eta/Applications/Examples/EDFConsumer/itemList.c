/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "itemList.h"
#include "refDataSession.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rtr/rsslRDM.h"

/* Requests to the snapshot server start at this number. */
static const RsslInt32 STREAM_ID_START = 5;

/* Offset number for stream ID from Stream_Id_Start */
static RsslInt32 Stream_Id_Offset = 0;

/* Item list instance. */
ItemList itemList;

void itemListClear(ItemList *pItemList)
{
	pItemList->itemCount = 0;
}

RsslRet itemListAdd(ItemList *pItemList, RsslInt32 feedStreamId, char *pSymbolName, RsslUInt8 domainType)
{
	Item *pItem;

	if (pItemList->itemCount == MAX_ITEMS)
	{
		printf("Error: Item list is full.\n\n");
		return RSSL_RET_FAILURE;
	}

	pItem = &pItemList->items[pItemList->itemCount];
	
	pItem->domainType = domainType;
	pItem->snapshotServerStreamId = STREAM_ID_START + Stream_Id_Offset;
	pItem->feedStreamId = feedStreamId;
	pItem->hasRefreshComplete = RSSL_FALSE;
	pItem->hasBufferedSnasphotMsgs = RSSL_FALSE;
	pItem->sequenceState = ITEM_SEQUENCE_NOT_STARTED;
	if (pSymbolName)
		snprintf(&pItem->symbolName[0], 128, "%s", pSymbolName);
	else
		pItem->symbolName[0] = '\0';
	msgQueueClear(&pItem->msgQueue);

	++pItemList->itemCount;
	++Stream_Id_Offset;

	return RSSL_RET_SUCCESS;

}

Item *itemListGetItem(ItemList *pItemList, RsslInt32 feedStreamId, RsslUInt8 domainType)
{
	unsigned int i;
	for (i = 0; i < pItemList->itemCount; ++i)
	{
		if (pItemList->items[i].feedStreamId == feedStreamId
				&& pItemList->items[i].domainType == domainType)
			return &pItemList->items[i];
	}

	return NULL;
}

Item *itemListGetItemBySnapStreamID(ItemList *pItemList, RsslInt32 snapshotServerStreamId, RsslUInt8 domainType)
{
	unsigned int i;
	for (i = 0; i < pItemList->itemCount; ++i)
	{
		if (pItemList->items[i].snapshotServerStreamId == snapshotServerStreamId
				&& pItemList->items[i].domainType == domainType)
			return &pItemList->items[i];
	}

	return NULL;
}

RsslRet itemProcessMsg(Item *pItem, RsslDecodeIterator *pIter, RsslMsg *pRsslMsg,
		RsslBool isSnapshotServerMsg)
{
	RsslRet ret;
	const RsslUInt32 *pSeqNum;
	RsslMsg *pBufferedMsg;

	/* If not reodering messages, or message has no sequence number, process now. */
	if (!(pSeqNum = rsslGetSeqNum(pRsslMsg))
			|| pItem->sequenceState == ITEM_SEQUENCE_DONE_REORDERING)
		return itemDecodeMsgData(pItem, pIter, pRsslMsg);

	/* The following logic provides an example of synchronization between the snapshot server 
	 * messages and the realtime feed messages so that this application presents the messages in 
	 * the expected order. To achieve this, messages from each stream are copied and stored when
	 * they may be out of order, and then replayed when the correct ordering is more clear. 
	 *
	 * The sequence number of each message retrieved from the snapshot server represents the 
	 * sequence of the message last applied from the realtime feed, and this is used to determine 
	 * the proper order.  So, for example, if we receive the following streams of messages with the
	 * indicated sequence number:
	 *
	 *   [Snapshot Server]: Refresh(2)
	 *   [RealTime Server]: Update(1), Update(2), Update(3), Update(4)
	 * 
	 * Assuming the refresh is a single part (i.e. a single RsslRefreshMsg, with the
	 * RSSL_RFMF_REFRESH_COMPLETE flag set), these messages will be presented by this application
	 * as:
	 *
	 *   Refresh(2), Update(3), Update(4)
	 *
	 * regardless of the order in which the refresh was received with respect to the updates.
	 * 
	 *
	 * This function accounts for multi-part refreshes as well, which may be received when
	 * consuming Level II data such as Market-by-Order items, by buffering messages
	 * from the snapshot server as needed. For example, the following streams:
	 *
	 *   [Snapshot Server]: Refresh(3), Refresh(5,complete)
	 *   [RealTime Server]: Update(1), Update(2), Update(3), Update(4), Update(5), Update(6)
	 *
	 * will be presented by this application as:
	 *
	 *   Refresh(3), Update(4), Update(5), Refresh(5,complete), Update(6)
	 *
	 * regardless of the order in which the refreshes were received with respect to the updates.
	 * 
	 * This logic is intended to be relatively simple, and does not account for:
	 *   - sequence numbers that are not in the correct order within each channel (e.g. gaps) 
	 *   - sequence number resets (indicated by messages with sequence number 0)
	 *   - unresponsive providing devices
	 */

	if (isSnapshotServerMsg)
	{
		/* Processing a message from the snapshot server. */

		if (pItem->sequenceState == ITEM_SEQUENCE_NOT_STARTED)
		{
			/* First snapshot message; discard any old messages from the realtime feed,
			 * then process the given message. */
			while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_UP_TO, *pSeqNum)))
				msgQueueCleanupRsslMsg(pBufferedMsg); 

			pItem->sequenceState = ITEM_SEQUENCE_HAVE_SNAPSHOT;
			pItem->seqNum = *pSeqNum;
		}
		else /* Message sequence started. */
		{
			/* This is a subsequent snapshot message. */

			/* If we have a snapshot message currently buffered, buffer this one too. */
			if (pItem->hasBufferedSnasphotMsgs)
				return msgQueueAdd(&pItem->msgQueue, pRsslMsg);

			/* Forward realtime messages up to this point. */
			while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_UP_TO, *pSeqNum)))
			{
				pItem->seqNum = *rsslGetSeqNum(pBufferedMsg);
				printf("  (Replaying buffered message)\n");
				if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
					return ret;
			}

			/* If this message's sequence number moved from the first snapshot message, we know we
			 * should have realtime messages. */
			if (pItem->sequenceState == ITEM_SEQUENCE_HAVE_SNAPSHOT
					&& seqNumCompare(*pSeqNum, pItem->seqNum) > 0)
			{
				pItem->seqNum = getNextSeqNum(pItem->seqNum);
				pItem->sequenceState = ITEM_SEQUENCE_REALTIME_STARTED;
			}

			/* If the realtime feed is behind, buffer this message until it catches up. */
			if (pItem->sequenceState == ITEM_SEQUENCE_REALTIME_STARTED
				   	&& seqNumCompare(pItem->seqNum, getNextSeqNum(*pSeqNum)) < 0)
			{
				pItem->hasBufferedSnasphotMsgs = RSSL_TRUE;
				return msgQueueAdd(&pItem->msgQueue, pRsslMsg);
			}
		}

		/* Now decode our mesage. */
		if ((ret = itemDecodeMsgData(pItem, pIter, pRsslMsg)) != RSSL_RET_SUCCESS)
			return ret;

		if (pRsslMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
		{
			pItem->hasRefreshComplete = RSSL_TRUE;

			/* Refresh completed; replay any buffered realtime messages. */
			while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_ALL, 0)))
			{
				pItem->seqNum = *rsslGetSeqNum(pBufferedMsg);
				printf("  (Replaying buffered message)\n");
				if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
					return ret;

				if (seqNumCompare(pItem->seqNum, *pSeqNum) >= 0)
					pItem->sequenceState = ITEM_SEQUENCE_DONE_REORDERING;
			}
		}

		return RSSL_RET_SUCCESS;
	}
	else
	{
		/* Processing a realtime message. */

		switch(pItem->sequenceState)
		{
			case ITEM_SEQUENCE_NOT_STARTED:
				/* Need to wait for snapshot server messages so we can play this in the correct order. */
				return msgQueueAdd(&pItem->msgQueue, pRsslMsg);

			case ITEM_SEQUENCE_HAVE_SNAPSHOT:
				if (seqNumCompare(*pSeqNum, pItem->seqNum) <= 0)
					return RSSL_RET_SUCCESS; /* Discard. */

				if (pItem->hasRefreshComplete)
				{
					/* Realtime data caught up to snapshot server. 
					 * This case is typically a single-part refresh (or a multipart
					 * refresh which had no interleaving updates). */
					pItem->sequenceState = ITEM_SEQUENCE_DONE_REORDERING;
					return itemDecodeMsgData(pItem, pIter, pRsslMsg);
				}
				else
					return msgQueueAdd(&pItem->msgQueue, pRsslMsg);

			case ITEM_SEQUENCE_REALTIME_STARTED:
				if (seqNumCompare(*pSeqNum, pItem->seqNum) < 0)
					return RSSL_RET_SUCCESS; /* Discard. */

				/* Processing a message from the realtime feed. */
				if (pItem->hasBufferedSnasphotMsgs)
				{
					/* Replay any snapshot server messages before this point. */
					while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_BEFORE, 
									*pSeqNum)))
					{
						if (pBufferedMsg->msgBase.msgClass == RSSL_MC_REFRESH
								&& pBufferedMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
							pItem->hasRefreshComplete = RSSL_TRUE;

						printf("  (Replaying buffered message)\n");
						if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
							return ret;
					}

					if (msgQueueHasMsgs(&pItem->msgQueue))
					{
						/* If there are still snapshot messages to play, they go after this
						 * message so play it now. */
						if ((ret = itemDecodeMsgData(pItem, pIter, pRsslMsg)) != RSSL_RET_SUCCESS)
							return ret;

						/* Now replay any snapshot server messages at the same sequence. */
						while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_UP_TO, 
										*pSeqNum)))
						{
							if (pBufferedMsg->msgBase.msgClass == RSSL_MC_REFRESH
									&& pBufferedMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
								pItem->hasRefreshComplete = RSSL_TRUE;

							printf("  (Replaying buffered message)\n");
							if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
								return ret;
						}

						if (!msgQueueHasMsgs(&pItem->msgQueue))
							pItem->hasBufferedSnasphotMsgs = RSSL_FALSE;

						if (pItem->hasRefreshComplete)
						{
							/* Refresh was completed, either before or during the replaying we just
							 * performed. */
							pItem->sequenceState = ITEM_SEQUENCE_DONE_REORDERING;
							while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_ALL, 0)))
							{
								printf("  (Replaying buffered message)\n");
								if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
									return ret;
							}
						}
					}
					else
					{
						/* No more snapshot server messages buffered.  Now, whether we buffer or play
						 * this message depends on whether we can expect more snapshot messages or not
						 * (if more are coming, we must still make sure to play this message in the right 
						 * order). */
						pItem->hasBufferedSnasphotMsgs = RSSL_FALSE;

						if (pItem->hasRefreshComplete)
						{
							/* Refresh was completed, either before or during the replaying we just
							 * performed. */
							pItem->sequenceState = ITEM_SEQUENCE_DONE_REORDERING;
							while ((pBufferedMsg = msgQueuePop(&pItem->msgQueue, POP_ALL, 0)))
							{
								printf("  (Replaying buffered message)\n");
								if ((ret = itemDecodeMsgData(pItem, pIter, pBufferedMsg)) != RSSL_RET_SUCCESS)
									return ret;
							}

							/* Now process our mesage. */
							return itemDecodeMsgData(pItem, pIter, pRsslMsg);
						}
						else
							return msgQueueAdd(&pItem->msgQueue, pRsslMsg);
					}
				}
				return RSSL_RET_SUCCESS;

		}
		return RSSL_RET_SUCCESS;
	}
}

RsslRet itemDecodeMsgData(Item *pItem, RsslDecodeIterator *pIter, RsslMsg *pRsslMsg)
{
	RsslRet ret = RSSL_RET_FAILURE;
	rsslSetDecodeIteratorBuffer(pIter, &pRsslMsg->msgBase.encDataBody);

	rsslSetDecodeIteratorGlobalFieldListSetDB(pIter, &globalFieldSetDefDb);

	switch(pRsslMsg->msgBase.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			if ((ret = processMarketPriceResponse(pRsslMsg, pIter)) != RSSL_RET_SUCCESS)
				return ret;

			printf("\n");
			return RSSL_RET_SUCCESS;

		case RSSL_DMT_MARKET_BY_ORDER:
			if ((ret = processMarketByOrderResponse(pRsslMsg, pIter)) != RSSL_RET_SUCCESS)
				return ret;

			printf("\n");
			return RSSL_RET_SUCCESS;

		case RSSL_DMT_MARKET_BY_PRICE:
			if ((ret = processMarketByPriceResponse(pRsslMsg, pIter)) != RSSL_RET_SUCCESS)
				return ret;
			
			printf("\n");
			return RSSL_RET_SUCCESS;

		default:
			printf("  Received unhandled message with domain %d(%s).\n\n",
					pRsslMsg->msgBase.domainType,
					rsslDomainTypeToString(pRsslMsg->msgBase.domainType));
			return RSSL_RET_FAILURE;

	}
}

void itemListCleanup(ItemList *pItemList)
{
	RsslUInt32 i;

	for(i = 0; i < pItemList->itemCount; ++i)
	{
		RsslMsg *pBufferedMsg;

		/* Free any messages still queued. */
		while ((pBufferedMsg = msgQueuePop(&pItemList->items[i].msgQueue, POP_ALL, 0)))
			msgQueueCleanupRsslMsg(pBufferedMsg); 
	}

}

