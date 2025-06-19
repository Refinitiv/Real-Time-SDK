/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef RSSL_TYPED_MESSAGE_UTIL_H
#define RSSL_TYPED_MESSAGE_UTIL_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslRDMMsg.h"
#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TypedMessageStats */

typedef struct {
	RsslUInt64 encodesTested, copiesTested;
	RsslUInt32 smallestMsgSize , biggestMsgSize;
	RsslUInt32 smallestMemoryBufferSize, biggestMemoryBufferSize;
	RsslUInt32 smallestCopyMemoryBufferSize, biggestCopyMemoryBufferSize;
} TypedMessageStats;

RTR_C_INLINE void clearTypedMessageStats(TypedMessageStats *pStats)
{
	memset(pStats, 0, sizeof(TypedMessageStats));
	pStats->smallestMsgSize = 0xffffffff;
	pStats->biggestMsgSize = 0;
	pStats->smallestMemoryBufferSize = 0xffffffff;
	pStats->biggestMemoryBufferSize = 0;
	pStats->smallestCopyMemoryBufferSize = 0xffffffff;
	pStats->biggestCopyMemoryBufferSize = 0;
}

RTR_C_INLINE void updateTypedMessageMsgSize(TypedMessageStats *pStats, RsslUInt32 msgSize)
{
	if (msgSize < pStats->smallestMsgSize) pStats->smallestMsgSize = msgSize;
	if (msgSize > pStats->biggestMsgSize) pStats->biggestMsgSize = msgSize;
}

RTR_C_INLINE void updateTypedMessageMemoryBufferSize(TypedMessageStats *pStats, RsslUInt32 memoryBufferSize)
{
	if (memoryBufferSize < pStats->smallestMemoryBufferSize) pStats->smallestMemoryBufferSize = memoryBufferSize;
	if (memoryBufferSize > pStats->biggestMemoryBufferSize) pStats->biggestMemoryBufferSize = memoryBufferSize;
}

RTR_C_INLINE void updateTypedMessageCopyMemoryBufferSize(TypedMessageStats *pStats, RsslUInt32 memoryBufferSize)
{
	if (memoryBufferSize < pStats->smallestCopyMemoryBufferSize) pStats->smallestCopyMemoryBufferSize = memoryBufferSize;
	if (memoryBufferSize > pStats->biggestCopyMemoryBufferSize) pStats->biggestCopyMemoryBufferSize = memoryBufferSize;
}

RTR_C_INLINE void incrTestedEncodes(TypedMessageStats *pStats)
{
	++pStats->encodesTested;
}

RTR_C_INLINE void incrTestedCopies(TypedMessageStats *pStats)
{
	++pStats->copiesTested;
}

RTR_C_INLINE void printTypedMessageStats(TypedMessageStats *pStats)
{
	printf("Total messages encoded/copied: %llu/%llu\n", pStats->encodesTested, pStats->copiesTested);
	printf("Smallest/Largest Encoded Msg: %u/%u\n", pStats->smallestMsgSize, pStats->biggestMsgSize);
	printf("Smallest/Largest Decode MemoryBuffer Usage: %u/%u\n", pStats->smallestMemoryBufferSize, pStats->biggestMemoryBufferSize);
	printf("Smallest/Largest Copied MemoryBuffer Usage: %u/%u\n", pStats->smallestCopyMemoryBufferSize, pStats->biggestCopyMemoryBufferSize);
}

typedef enum {
	TEST_EACTION_ENCODE = 0, /* Encode & Decode the RDM Msg */
	TEST_EACTION_COPY = 1, /* Copy the RDM Msg */
	TEST_EACTION_CREATE_COPY = 2 /* Create a copy of the RDM Msg */
} TestWriteAction;

/* Enums indicate choice of how to send the message -- either by encoding & decoding or using a copy function. */
static TestWriteAction testWriteActions[] =
{
	TEST_EACTION_ENCODE,
	TEST_EACTION_COPY,
	TEST_EACTION_CREATE_COPY
};
static RsslUInt32 testWriteActionsCount = sizeof(testWriteActions)/sizeof(TestWriteAction);

/* writeRDMMsg
 * Encodes or copies the given RDMMsg into another RDMMsg */
void writeRDMMsg(
		RsslRDMMsg *pRDMMsgToWrite, TestWriteAction action, 
		RsslMsg *pRsslMsgToDecodeTo, RsslRDMMsg *pRDMMsgToDecodeTo,
		TypedMessageStats *pStats);

#ifdef __cplusplus
};
#endif

#endif
