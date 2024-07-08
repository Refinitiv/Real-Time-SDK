/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* Implements a basic hash table using the RsslQueue. */

#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslQueue.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef RsslQueueLink HashTableLink;

typedef struct {
	RsslUInt32	queueCount;	/* The number of bins in the table. */
	RsslQueue	*queueList;	/* Hash table bins. */

	/* Function used for distributing elements among bins. */
	RsslUInt32	(*keyHashFunction)(void *key);

	/* Function for comparing elements within bins. */
	RsslBool	(*keyCompareFunction)(void *key, HashTableLink *pLink);
} HashTable;

/* Initializes a hash table. */
RTR_C_INLINE RsslRet hashTableInit(HashTable *pTable, RsslUInt32 queueCount, RsslUInt32 (*keyHashFunction)(void*), RsslBool (*keyCompareFunction)(void*,HashTableLink*))
{
	RsslUInt64 queueListSize = queueCount * sizeof(RsslQueue);
	RsslUInt32 i;

	if (queueListSize > (size_t)-1) /* overflow */
		return RSSL_RET_FAILURE;

	pTable->queueCount = queueCount;

	pTable->queueList = (RsslQueue*)malloc((size_t)queueListSize);

	for (i = 0; i < queueCount; ++i)
	{
		rsslInitQueue(&pTable->queueList[i]);
	}

	if (!pTable->queueList)
		return RSSL_RET_FAILURE;

	pTable->keyHashFunction = keyHashFunction;
	pTable->keyCompareFunction = keyCompareFunction;

	return RSSL_RET_SUCCESS;
}

/* Cleans up a hash table. */
RTR_C_INLINE RsslRet hashTableCleanup(HashTable *pTable)
{
	free(pTable->queueList);
	pTable->queueList = NULL;
	return RSSL_RET_SUCCESS;
}

/* An example integer distribution function. */
RTR_C_INLINE RsslUInt32 intHashFunction(void *key)
{
	return *((RsslUInt32*)key);
}

/* Add an element to the hash table. */
RTR_C_INLINE void hashTableInsertLink(HashTable *pTable, HashTableLink *pLink, void *key)
{
	RsslUInt32 queueListLocation = pTable->keyHashFunction(key) % pTable->queueCount;
	RsslQueue *pQueue = &pTable->queueList[queueListLocation];

	rsslQueueAddLinkToBack(pQueue, pLink);
}

/* Remove an element from a hash table. */
RTR_C_INLINE void hashTableRemoveLink(HashTable *pTable, void *key)
{
	RsslUInt32 queueListLocation = pTable->keyHashFunction(key) % pTable->queueCount;
	RsslQueue *pQueue = &pTable->queueList[queueListLocation];
	RsslQueueLink *pLink;

	for(pLink = rsslQueuePeekFront(pQueue); pLink; pLink = rsslQueuePeekNext(pQueue, pLink))
	{
		if (pTable->keyCompareFunction(key, pLink))
		{
			rsslQueueRemoveLink(pQueue, pLink);
			return;
		}
	}
}

/* Find an element in the hash table that matches the given key. */
RTR_C_INLINE HashTableLink *hashTableFind(HashTable *pTable, void *key)
{
	RsslUInt32 queueListLocation = pTable->keyHashFunction(key) % pTable->queueCount;
	RsslQueue *pQueue = &pTable->queueList[queueListLocation];
	RsslQueueLink *pLink;

	for(pLink = rsslQueuePeekFront(pQueue); pLink; pLink = rsslQueuePeekNext(pQueue, pLink))
	{
		if (pTable->keyCompareFunction(key, pLink))
			return pLink;
	}

	return NULL;
}

/* Objects that are part of hash tables include the HashTableLink as part of their structure.
 * This convenience macro can be used to cast from a HashTableLink into the full object. */
#define HASH_TABLE_LINK_TO_OBJECT(__objectType, __link, __pLink) ((__objectType*)((char*)__pLink - offsetof(__objectType, __link)))

#ifdef __cplusplus
}
#endif

#endif
