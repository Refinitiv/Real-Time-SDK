/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/wlBase.h"
#include "rtr/wlPostIdTable.h"
#include "rtr/rsslReactorUtils.h"

/* Hash Sum & Compare functions for the Post ID table.
 * The table is intended to be searchable both for matching acknowledgements
 * and identifying duplicate post messages(for example, a post ID with the same ID as a 
 * completed post record is a duplicate regardless of its sequence number). 
 * The RsslHashTable uses the record in the table as argument 2 when comparing. */

RsslUInt32 wlPostRecordSum(void *pKey)
{
	RsslUInt32 hashSum = 0;
	WlPostRecord *pRecord = (WlPostRecord*)pKey;

	assert(pRecord->flags & RSSL_PSMF_HAS_POST_ID);

	hashSum = pRecord->streamId + pRecord->postId;

	if (!(pRecord->flags & RSSL_PSMF_POST_COMPLETE))
	{
		/* If not complete, include sequence number. */
		assert(pRecord->flags & RSSL_PSMF_HAS_SEQ_NUM);
		hashSum += pRecord->seqNum;
	}

	return hashSum;
}

RsslBool wlPostRecordCompare(void *pKey1, void *pKey2)
{
	WlPostRecord *pRecord1 = (WlPostRecord*)pKey1;
	WlPostRecord *pRecord2 = (WlPostRecord*)pKey2;

	if (pRecord1->streamId != pRecord2->streamId)
		return RSSL_FALSE;

	if (pRecord1->postId != pRecord2->postId)
		return RSSL_FALSE;

	/* If searching for a match to an RsslPostMsg,
	 * stop on any completed post, as this indicates a duplicate Post ID is in use. */
	if (!pRecord1->fromAckMsg
			&& pRecord2->flags & RSSL_PSMF_POST_COMPLETE)
		return RSSL_TRUE;

	if ((pRecord1->flags & RSSL_PSMF_HAS_SEQ_NUM) != (pRecord2->flags & RSSL_PSMF_HAS_SEQ_NUM)
			|| (pRecord1->flags & RSSL_PSMF_HAS_SEQ_NUM) && pRecord1->seqNum != pRecord2->seqNum)
		return RSSL_FALSE;

	return RSSL_TRUE;

}

RsslRet wlPostTableInit(WlPostTable *pTable, RsslUInt32 maxPoolSize,
		RsslUInt32 postAckTimeout, RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 ui;
	RsslRet ret;

	memset(pTable, 0, sizeof(WlPostTable));

	if ((ret = rsslHashTableInit(&pTable->records, maxPoolSize, wlPostRecordSum, wlPostRecordCompare, 
			RSSL_FALSE, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	pTable->postAckTimeout = postAckTimeout;
	rsslInitQueue(&pTable->pool);
	rsslInitQueue(&pTable->timeoutQueue);

	for (ui = 0; ui < maxPoolSize; ++ui)
	{
		WlPostRecord *pRecord = (WlPostRecord*)malloc(sizeof(WlPostRecord));
		if (!pRecord)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failure.");
			wlPostTableCleanup(pTable);
			return RSSL_RET_FAILURE;
		}
		rsslClearBuffer(&pRecord->name);
		rsslQueueAddLinkToBack(&pTable->pool, &pRecord->qlUser);
	}

	return RSSL_RET_SUCCESS;

}

void wlPostTableCleanup(WlPostTable *pTable)
{
	RsslQueueLink *pLink;

	while(pLink = rsslQueueRemoveFirstLink(&pTable->pool))
	{
		WlPostRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);
		rsslHeapBufferCleanup(&pRecord->name);
		free (pRecord);
	}

	rsslHashTableCleanup(&pTable->records);
}

WlPostRecord *wlPostTableFindRecord(WlPostTable *pTable, RsslAckMsg *pAckMsg)
{
	WlPostRecord postRecord;
	RsslHashLink *pHashLink;
	
	postRecord.postId = pAckMsg->ackId;
	postRecord.streamId = pAckMsg->msgBase.streamId;
	postRecord.fromAckMsg = RSSL_TRUE;

	if (!(pAckMsg->flags & RSSL_AKMF_HAS_SEQ_NUM))
	{
		/* Complete posts? */
		postRecord.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_POST_COMPLETE;
		if ((pHashLink = rsslHashTableFind(&pTable->records, (void*)&postRecord, NULL)))
			return RSSL_HASH_LINK_TO_OBJECT(WlPostRecord, hlTable, pHashLink);
	}
	else
	{
		postRecord.seqNum = pAckMsg->seqNum;

		/* Complete posts? */
		postRecord.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_SEQ_NUM;
		if ((pHashLink = rsslHashTableFind(&pTable->records, (void*)&postRecord, NULL)))
			return RSSL_HASH_LINK_TO_OBJECT(WlPostRecord, hlTable, pHashLink);

		/* Incomplete posts? */
		postRecord.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_HAS_SEQ_NUM;
		if ((pHashLink = rsslHashTableFind(&pTable->records, (void*)&postRecord, NULL)))
			return RSSL_HASH_LINK_TO_OBJECT(WlPostRecord, hlTable, pHashLink);
	}


	return NULL;
}

WlPostRecord *wlPostTableAddRecord(struct WlBase *pBase, WlPostTable *pTable, 
		RsslPostMsg *pPostMsg, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	RsslHashLink *pHashLink;
	WlPostRecord *pRecord, *pExistingRecord;
	RsslUInt32	hashSum;

	assert(pPostMsg->flags & RSSL_PSMF_ACK);

	if (!(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
				"Post requested acknowledgement without Post ID.");
		return NULL;
	}

	if (!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE) && !(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
				"Post part requested acknowledgement without sequence number.");
		return NULL;
	}

	if (!(pLink = rsslQueueRemoveFirstLink(&pTable->pool)))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Too many posts awaiting acknowledgement.");
		return NULL;
	}

	pRecord = RSSL_QUEUE_LINK_TO_OBJECT(WlPostRecord, qlUser, pLink);

	pRecord->flags = pPostMsg->flags;
	pRecord->postId = pPostMsg->postId;
	pRecord->seqNum = pPostMsg->seqNum;
	pRecord->streamId = pPostMsg->msgBase.streamId;
	pRecord->fromAckMsg = RSSL_FALSE;
	pRecord->domainType = pPostMsg->msgBase.domainType;

	pRecord->msgkeyflags = RSSL_MKF_NONE;
	rsslClearBuffer(&pRecord->name);
	if ((pPostMsg->flags & RSSL_PSMF_HAS_MSG_KEY) && (pPostMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME))
	{
		rsslHeapBufferCopy(&pRecord->name, &pPostMsg->msgBase.msgKey.name, &pRecord->name);
		pRecord->msgkeyflags |= RSSL_MKF_HAS_NAME;
	}
	if ((pPostMsg->flags & RSSL_PSMF_HAS_MSG_KEY) && (pPostMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID))
	{
		pRecord->serviceId = pPostMsg->msgBase.msgKey.serviceId;
		pRecord->msgkeyflags |= RSSL_MKF_HAS_SERVICE_ID;
	}

	/* Check for a completed post with the same post ID. */
	pRecord->flags = (pPostMsg->flags & ~RSSL_PSMF_HAS_SEQ_NUM) | RSSL_PSMF_POST_COMPLETE;
	if ((pHashLink = rsslHashTableFind(&pTable->records, (void*)pRecord, NULL)))
	{
		pExistingRecord = RSSL_HASH_LINK_TO_OBJECT(WlPostRecord, hlTable, pHashLink);
		if (pExistingRecord->flags & RSSL_PSMF_HAS_SEQ_NUM)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
					"Post contains duplicate information (Post exists with ID %u, SeqNum %u).",
					pExistingRecord->postId, pExistingRecord->seqNum);
		}
		else
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
					"Post contains duplicate information (Post exists with ID %u).",
				   	pExistingRecord->postId);
		}

		rsslQueueAddLinkToBack(&pTable->pool, pLink);
		return NULL;
	}

	/* Check for any incomplete post with the same ID & sequence number. */
	pRecord->flags = pPostMsg->flags;
	hashSum = wlPostRecordSum((void*)pRecord);
	if (!(pRecord->flags & RSSL_PSMF_POST_COMPLETE))
	{
		if ((pHashLink = rsslHashTableFind(&pTable->records, (void*)pRecord, &hashSum)))
		{
			pExistingRecord = RSSL_HASH_LINK_TO_OBJECT(WlPostRecord, hlTable, pHashLink);
			assert (pExistingRecord->flags & RSSL_PSMF_HAS_SEQ_NUM);

			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, 
					"Post contains duplicate information (Post part exists with ID %u, SeqNum %u).",
					pExistingRecord->postId, pExistingRecord->seqNum);

			rsslQueueAddLinkToBack(&pTable->pool, pLink);
			return NULL;
		}
	}

	rsslHashTableInsertLink(&pTable->records, &pRecord->hlTable, (void*)pRecord, &hashSum);

	/* Add timer. */
	/* Time is currently only set when dispatching, so we must get an up-to-date time. */
	pRecord->expireTime = getCurrentTimeMs(pBase->ticksPerMsec) + pTable->postAckTimeout;
	rsslQueueAddLinkToBack(&pTable->timeoutQueue, &pRecord->qlTimeout);
	pBase->watchlist.state |= RSSLWL_STF_NEED_TIMER;

	return pRecord;
}

void wlPostTableRemoveRecord(WlPostTable *pTable, WlPostRecord *pRecord)
{
	rsslHashTableRemoveLink(&pTable->records, &pRecord->hlTable);
	rsslQueueAddLinkToBack(&pTable->pool, &pRecord->qlUser);
	rsslQueueRemoveLink(&pTable->timeoutQueue, &pRecord->qlTimeout);
}
