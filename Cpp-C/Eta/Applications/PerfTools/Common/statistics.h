/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* statistics.h
 * Provides recording and calculation of various statistics. */

#ifndef _STATISTICS_H
#define _STATISTICS_H

#include "getTime.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"

#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structure for calculating running statistics for a given value(such as latency). */
typedef struct {
	RsslUInt64		count;				/* Total number of samples. */
	double			average;			/* Current mean of samples. */
	double			variance;			/* Current variance of samples. */
	double			maxValue;			/* Highest sample value. */
	double			minValue;			/* Lowest sample value. */

	double			_sum;				/* Used in calculating variance. */
	double			_sumOfSquares;		/* Used in calculating variance. */
} ValueStatistics;

/* Clears a ValueStatistics structure. */
RTR_C_INLINE void clearValueStatistics(ValueStatistics *pStats)
{
	pStats->count = 0;
	pStats->average = 0;
	pStats->variance = 0;
	pStats->maxValue = -DBL_MAX;
	pStats->minValue = DBL_MAX;
	pStats->_sum = 0;
	pStats->_sumOfSquares = 0;
}

/* Recalculate stats based on new value. */
void updateValueStatistics(ValueStatistics *pStats, double newValue);

/* Print a line containing all calculated statistics. */
void printValueStatistics(FILE *file, const char *valueStatsName, const char *countUnitName, 
		ValueStatistics *pStats, RsslBool displayThousandths);

/*** Time Statistics. ***/

/* This functionality is used to collect individual time differences
 * for statistical calculation in a thread-safe manner -- one thread can store information
 * using timeRecordSubmit() and another can retrieve the information via timeRecordQueueGet()
 * and do any desired calculation. */

/* Stores time information. */
typedef struct {
	RsslQueueLink queueLink;
	TimeValue startTime;	/* Recorded start time. */
	TimeValue endTime;		/* Recorded end time. */
	TimeValue ticks;		/* Units per microsecond. */
} TimeRecord;

/* Stores a queue and pool of TimeRecord objects.  */
typedef struct {
	RsslMutex poolLock;
	RsslQueue pool;			/* Reusable pool of TimeRecord objects. */
	RsslQueue records;		/* Queue of submitted TimeRecord objects. */
} TimeRecordQueue;

/* Initializes an TimeRecordQueue. */
void timeRecordQueueInit(TimeRecordQueue *pRecordQueue);

/* Destroys an TimeRecordQueue. */
void timeRecordQueueCleanup(TimeRecordQueue *pRecordQueue);

/* Puts latency information into a record and adds the record to the queue. */
RsslRet timeRecordSubmit(TimeRecordQueue *pRecordQueue, TimeValue startTime, TimeValue endTime, TimeValue ticks);

/* Retrieves all time records currently in the queue. */
void timeRecordQueueGet(TimeRecordQueue *pRecordQueue, RsslQueue *pQueue);

/* Returns TimeRecord objects to the pool so they can be reused. */
void timeRecordQueueRepool(TimeRecordQueue *pRecordQueue, RsslQueue *pQueue);

/*** Count statistics ***/

/* Keeps an ongoing count that can be used to get both total and periodic counts. The count can 
 * also be collected by a different thread than the one counting. */

typedef struct
{
	RsslUInt64 currentValue;	/* The current value. */
	RsslUInt64 prevValue;		/* Value returned from the previous call to countStatGetChange. */
} CountStat;

RTR_C_INLINE void initCountStat(CountStat *pCountStat)
{
	memset(pCountStat, 0, sizeof(CountStat));
}

/* Increment the count. */
RTR_C_INLINE void countStatIncr(CountStat *pCountStat)
{
	++pCountStat->currentValue;
}

/* Add to the count. */
RTR_C_INLINE void countStatAdd(CountStat *pCountStat, RsslUInt64 addend)
{
	pCountStat->currentValue += addend;
}

/* Get the difference between the current count and that of the previous call. */
RTR_C_INLINE RsslUInt64 countStatGetChange(CountStat *pCountStat)
{
	RsslUInt64 currentValue = pCountStat->currentValue;
	currentValue -= pCountStat->prevValue;
	pCountStat->prevValue = pCountStat->currentValue;
	return currentValue;
}

/* Get the current overall count. */
RTR_C_INLINE RsslUInt64 countStatGetTotal(CountStat *pCountStat)
{
	return pCountStat->currentValue;
}

/*** Resource Statistics (CPU & Memory Usage) ***/

typedef struct {
	RsslDouble cpuUsageFraction;
	RsslUInt64 memUsageBytes;

	TimeValue _prevTimeUsec;
	RsslInt64 _prevUserTimeUsec;
	RsslInt64 _prevKernelTimeUsec;

#ifdef WIN32
	HANDLE _currentProcess;
#elif defined(Linux)
	int _pageSize;
#endif
} ResourceUsageStats;

/* Initializes ResourceUsageStats structure. */
RsslRet initResourceUsageStats(ResourceUsageStats *pStats);

/* Calculates Resource Usage since the last call to this function.  */
RsslRet getResourceUsageStats(ResourceUsageStats *pStats);

#ifdef __cplusplus
};
#endif

#endif
