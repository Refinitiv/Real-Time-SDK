/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* latencyRandomArray.h
 * Generates a randomized array that can be used to determine which message in a burst
 * should contain latency information. 
 * Create the array using createLatencyRandomArray(). Iterate over the array using
 * latencyRandomArrayGetNext().
 */

#ifndef _LATENCY_RANDOM_ARRAY_H
#define _LATENCY_RANDOM_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Iterator to use with a randomized latency array. */
typedef int LatencyRandomArrayIter;

/* Initializes a LatencyRandomArrayIter. */
void latencyRandomArrayIterInit(LatencyRandomArrayIter *pIter);

typedef struct
{
	int *_array;		/* Pointer to the array of values. */
	int _valueCount;	/* Total number of values in the array. */
} LatencyRandomArray;


typedef struct
{
	int	totalMsgsPerSec;	/* Total messages sent per second */
	int	latencyMsgsPerSec;	/* Total number of latency messages sent per second */
	int	ticksPerSec;		/* Bursts of messages sent per second by the user. */
	int	arrayCount;			/* Increasing this adds more random values to use 
							 * (each individual array contains one second's worth of 
							 * values).	*/
} LatencyRandomArrayOptions;

/* Clears a LatencyRandomArrayOptions struct. */
void clearLatencyRandomArrayOptions(LatencyRandomArrayOptions *pOpts);

/* Creates a LatencyRandomArray. */
void createLatencyRandomArray(LatencyRandomArray *pRandomArray, LatencyRandomArrayOptions *pOpts);

/* Iterate over the LatencyRandomArray. The value returned indicates which message in the tick
 * should contain latency information.  If the value is -1, no latency message should be
 * sent in that tick. The iteration starts over when the end of the array is reached. */
int latencyRandomArrayGetNext(LatencyRandomArray *pRandomArray, LatencyRandomArrayIter *pIter);

/* Cleans up a LatencyRandomArray. */
void cleanupLatencyRandomArray(LatencyRandomArray *pRandomArray);

#ifdef __cplusplus
};
#endif

#endif
