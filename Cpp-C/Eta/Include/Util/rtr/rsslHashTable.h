/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* Implements a basic hash table using the RsslQueue. */

#ifndef RSSL_HASH_TABLE_H
#define RSSL_HASH_TABLE_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslErrorInfo.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hash sum function prototype. */
typedef RsslUInt32 RsslHashSumFunction(void*);

/* Hash comparison function prototype. */
typedef RsslBool RsslHashCompareFunction(void*, void*);


/* Hash Table Link structure. */
typedef struct
{
	RsslQueueLink	queueLink;
	void			*pKey;
	RsslUInt32		hashSum;
} RsslHashLink;

/* Initializes a hash table link. */
RTR_C_INLINE void rsslHashLinkInit(RsslHashLink *pLink)
{
	pLink->hashSum = 0;
	pLink->pKey = NULL;
	rsslInitQueueLink(&pLink->queueLink);
}

/* Hash Table structure. */
typedef struct {
	RsslUInt32				queueCount;
	RsslUInt32				elementCount;
	RsslUInt32				thresholdCapacity;
	RsslBool				dynamicSize;
	RsslQueue				*queueList;
	RsslHashSumFunction		*keyHashFunction;
	RsslHashCompareFunction	*keyCompareFunction;
} RsslHashTable;

/* Initializes a hash table. */
RTR_C_INLINE RsslRet rsslHashTableInit(RsslHashTable *pTable, RsslUInt32 queueCount, 
		RsslHashSumFunction *keyHashFunction, RsslHashCompareFunction *keyCompareFunction,
		RsslBool dynamicSize, RsslErrorInfo *pErrorInfo);

/* Cleans up a hash table. */
RTR_C_INLINE RsslRet rsslHashTableCleanup(RsslHashTable *pTable);

/* Add an element to the hash table. */
RTR_C_INLINE void rsslHashTableInsertLink(RsslHashTable *pTable, RsslHashLink *pLink, 
		void *pKey, RsslUInt32 *pSum);

/* Remove an element from a hash table. */
RTR_C_INLINE void rsslHashTableRemoveLink(RsslHashTable *pTable, RsslHashLink *pLink);

/* Find an element in the hash table that matches the given key. */
RTR_C_INLINE RsslHashLink *rsslHashTableFind(RsslHashTable *pTable, void *pKey, RsslUInt32 *pSum);

/* Objects that are part of hash tables include the RsslHashLink as part of their structure.
 * This convenience macro can be used to cast from a RsslHashLink into the full object. */
#define RSSL_HASH_LINK_TO_OBJECT(__objectType, __link, __pLink) ((__objectType*)((char*)__pLink - offsetof(__objectType, __link)))

/* RsslUInt16 hash functions. */
RSSL_API RsslUInt32 rsslHashU16Sum(void *pKey);
RSSL_API RsslBool rsslHashU16Compare(void *pKey1, void *pKey2);

/* RsslUInt32 hash functions. */
RSSL_API RsslUInt32 rsslHashU32Sum(void *pKey);
RSSL_API RsslBool rsslHashU32Compare(void *pKey1, void *pKey2);

/* RsslUInt64 hash functions. */
RSSL_API RsslUInt32 rsslHashU64Sum(void *pKey);
RSSL_API RsslBool rsslHashU64Compare(void *pKey1, void *pKey2);

/* RsslBuffer hash functions. */
RSSL_API RsslUInt32 rsslHashBufferSum(void *pKey);
RSSL_API RsslBool rsslHashBufferCompare(void *pKey1, void *pKey2);

#define LOAD_FACTOR 0.75

#define RSSL_HASH_LINK_FROM_QUEUE_LINK(__pLink) ((RsslHashLink*)__pLink)

RTR_C_INLINE RsslRet rsslHashTableInit(RsslHashTable *pTable, RsslUInt32 queueCount, 
		RsslHashSumFunction *keyHashFunction, RsslHashCompareFunction *keyCompareFunction,
		RsslBool dynamicSize, RsslErrorInfo *pErrorInfo)
{
	RsslUInt64 queueListSize = queueCount * sizeof(RsslQueue);
	RsslUInt32 i;


	if (queueListSize > (size_t)-1) /* overflow */
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Hash table queue size too large.");
		return RSSL_RET_FAILURE;
	}

	memset(pTable, 0, sizeof(RsslHashTable));

	pTable->queueCount = queueCount;
	pTable->elementCount = 0;
	pTable->thresholdCapacity = (RsslUInt32)((double)queueCount * LOAD_FACTOR);
	pTable->dynamicSize = dynamicSize;

	pTable->queueList = (RsslQueue*)malloc((size_t)queueListSize);

	if (!pTable->queueList)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failure.");
		return RSSL_RET_FAILURE;
	}

	for (i = 0; i < queueCount; ++i)
	{
		rsslInitQueue(&pTable->queueList[i]);
	}

	pTable->keyHashFunction = keyHashFunction;
	pTable->keyCompareFunction = keyCompareFunction;

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet rsslHashTableCleanup(RsslHashTable *pTable)
{
	free(pTable->queueList);
	pTable->queueList = NULL;
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet rsslHashTableResize(RsslHashTable *pTable)
{
	RsslUInt32 newQueueCount = pTable->queueCount * 2 + 1;
        RsslUInt64 queueListSize = newQueueCount * sizeof(RsslQueue);
	RsslQueue* rsslOldQueue = (RsslQueue*)pTable->queueList;
        RsslUInt32 i;

	/* calculate new treshold */
        pTable->thresholdCapacity = (RsslUInt32)((double)newQueueCount * LOAD_FACTOR);

        if (queueListSize > (size_t)-1) /* overflow */
                return RSSL_RET_FAILURE;

	/* allocate new table */	
	pTable->queueList = (RsslQueue*)malloc((size_t)queueListSize);

        if (!pTable->queueList)
                return RSSL_RET_FAILURE;

	/* initialize queue list */
        for (i = 0; i < newQueueCount; ++i)
        {
                rsslInitQueue(&pTable->queueList[i]);
        }

	/* iterate through old hash table to move all the items */
        for (i = 0; i < pTable->queueCount; ++i)
        {
		RsslQueueLink* pQueueLink = rsslQueueRemoveFirstLink(&rsslOldQueue[i]);
		while (pQueueLink)
		{
			RsslQueue* pQueue;
			RsslHashLink *pHashLink = RSSL_HASH_LINK_FROM_QUEUE_LINK(pQueueLink);
			RsslUInt32 queueListLocation;

			/* figure out a new position */
		        queueListLocation =  pHashLink->hashSum % newQueueCount;
		        pQueue = &pTable->queueList[queueListLocation];

			/* add the link to new queue */
		        rsslQueueAddLinkToBack(pQueue, &pHashLink->queueLink);

			/* go to next link */
			pQueueLink = rsslQueueRemoveFirstLink(&rsslOldQueue[i]);
		}
        }

	/* update new size */
	pTable->queueCount = newQueueCount;
		
	/* free old table */
	free(rsslOldQueue);

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void rsslHashTableInsertLink(RsslHashTable *pTable, RsslHashLink *pLink, 
		void *pKey, RsslUInt32 *pSum)
{
	RsslUInt32 queueListLocation;
	RsslQueue *pQueue;

	pLink->hashSum = (pSum ? *pSum : pTable->keyHashFunction(pKey));
	pLink->pKey = pKey;

	queueListLocation =  pLink->hashSum % pTable->queueCount;
	pQueue = &pTable->queueList[queueListLocation];

	rsslQueueAddLinkToBack(pQueue, &pLink->queueLink);

	pTable->elementCount++;

	if (pTable->dynamicSize && pTable->elementCount > pTable->thresholdCapacity)
		rsslHashTableResize(pTable);
}



RTR_C_INLINE void rsslHashTableRemoveLink(RsslHashTable *pTable, RsslHashLink *pLink)
{
	RsslUInt32 queueListLocation = pLink->hashSum % pTable->queueCount;
	rsslQueueRemoveLink(&pTable->queueList[queueListLocation], &pLink->queueLink);
	pTable->elementCount--;
}

RTR_C_INLINE RsslHashLink *rsslHashTableFind(RsslHashTable *pTable, void *pKey, RsslUInt32 *pSum)
{
	RsslUInt32 queueListLocation = (pSum ? *pSum : pTable->keyHashFunction(pKey)) 
		% pTable->queueCount;
	RsslQueue *pQueue = &pTable->queueList[queueListLocation];
	RsslQueueLink *pLink;

	for(pLink = rsslQueuePeekFront(pQueue); pLink; pLink = rsslQueuePeekNext(pQueue, pLink))
	{
		RsslHashLink *pHashLink = RSSL_HASH_LINK_FROM_QUEUE_LINK(pLink);
		
		if (pTable->keyCompareFunction(pKey, pHashLink->pKey))
			return pHashLink;
	}

	return NULL;
}


#ifdef __cplusplus
}
#endif

#endif
