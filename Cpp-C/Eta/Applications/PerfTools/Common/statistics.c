/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "statistics.h"
#include <stdlib.h>
#include <math.h>
#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#if defined(Linux)
#include <unistd.h>
#else
#include <procfs.h>
#endif
#endif

void updateValueStatistics(ValueStatistics *pStats, double newValue)
{
	double prevAverage = pStats->average;

	++pStats->count;

	/* Check against max & min */
	if (newValue > pStats->maxValue) pStats->maxValue = newValue;
	if (newValue < pStats->minValue) pStats->minValue = newValue;

	/* Average and variance are calculated using online algorithms.
	 * - Average: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average 
	 * - Variance: Devore,Farnum. "Applied Statistics for Engineers and Scientists(Second Edition)", p. 73,74 */

	pStats->average = (newValue + pStats->average * (pStats->count-1))/pStats->count;

	pStats->_sum += newValue;
	pStats->_sumOfSquares += newValue * newValue;
	pStats->variance = pStats->count > 1 ? 
		(pStats->_sumOfSquares - pStats->_sum * pStats->_sum / pStats->count) / (pStats->count - 1) : 0;
}

void printValueStatistics(FILE *file, const char *valueStatsName, const char *countUnitName, 
		ValueStatistics *pStats, RsslBool displayThousandths)
{
	const char *outputStr = displayThousandths ? 
		   "%s: Avg:%8.3f StdDev:%8.3f Max:%8.3f Min:%8.3f, %s: %llu\n"
		:  "%s: Avg:%6.1f StdDev:%6.1f Max:%6.0f Min:%6.0f, %s: %llu\n";

	printf(outputStr,
			valueStatsName,
			pStats->average, 
			sqrt(pStats->variance),
			pStats->maxValue,
			pStats->minValue,
			countUnitName,
			pStats->count);
}

void timeRecordQueueInit(TimeRecordQueue *pRecordQueue)
{
	int i;
	RSSL_MUTEX_INIT(&pRecordQueue->poolLock);
	rsslInitQueue(&pRecordQueue->pool);
	rsslInitQueue(&pRecordQueue->records);

	/* Preload latency pool */
	for(i = 0; i < 1000; ++i)
	{
		TimeRecord *pRecord = (TimeRecord *)malloc(sizeof(TimeRecord));
		rsslQueueAddLinkToBack(&pRecordQueue->pool, &pRecord->queueLink);
	}
}

void timeRecordQueueCleanup(TimeRecordQueue *pRecordQueue)
{
	RsslQueueLink *pLink;

	RSSL_MUTEX_DESTROY(&pRecordQueue->poolLock);

	while (pLink = rsslQueueRemoveFirstLink(&pRecordQueue->pool))
	{
		TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
		free(pRecord);
	}

	while (pLink = rsslQueueRemoveFirstLink(&pRecordQueue->records))
	{
		TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
		free(pRecord);
	}
}

RsslRet timeRecordSubmit(TimeRecordQueue *pRecordQueue, RsslTimeValue startTime, RsslTimeValue endTime, RsslTimeValue ticks)
{
	RsslQueueLink *pLink;
	TimeRecord *pRecord;

	RSSL_MUTEX_LOCK(&pRecordQueue->poolLock);
	pLink = rsslQueueRemoveFirstLink(&pRecordQueue->pool);

	if (pLink)
		pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
	else
	{
		pRecord = (TimeRecord*)malloc(sizeof(TimeRecord));

		if (!pRecord)
			return RSSL_RET_FAILURE;
	}

	pRecord->ticks = ticks;
	pRecord->startTime = startTime;
	pRecord->endTime = endTime;

	rsslQueueAddLinkToBack(&pRecordQueue->records, &pRecord->queueLink);

	RSSL_MUTEX_UNLOCK(&pRecordQueue->poolLock);

	return RSSL_RET_SUCCESS;

}

void timeRecordQueueGet(TimeRecordQueue *pRecordQueue, RsslQueue *pQueue)
{
	RSSL_MUTEX_LOCK(&pRecordQueue->poolLock);
	rsslQueueAppend(pQueue, &pRecordQueue->records);
	RSSL_MUTEX_UNLOCK(&pRecordQueue->poolLock);
}

void timeRecordQueueRepool(TimeRecordQueue *pRecordQueue, RsslQueue *pQueue)
{
	RSSL_MUTEX_LOCK(&pRecordQueue->poolLock);
	rsslQueueAppend(&pRecordQueue->pool, pQueue);
	RSSL_MUTEX_UNLOCK(&pRecordQueue->poolLock);
}

RsslRet initResourceUsageStats(ResourceUsageStats *pStats)
{
	RsslRet ret;
	memset(pStats, 0, sizeof(ResourceUsageStats));

#ifdef WIN32
	pStats->_currentProcess = GetCurrentProcess();
#elif defined(Linux)
	pStats->_pageSize = getpagesize();
#endif

	/* Set the starting point for the usage stats. */
	if ((ret = getResourceUsageStats(pStats)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet getResourceUsageStats(ResourceUsageStats *pStats)
{

	RsslInt64 userUsec, kernelUsec;
	RsslTimeValue currentTimeUsec;


#ifdef WIN32
	FILETIME creationTime, exitTime, kernelTime, userTime;
	ULARGE_INTEGER tmpTime;
	PROCESS_MEMORY_COUNTERS memoryCounters;
#else
	FILE *processFile;
	RsslUInt64 totalProgramSize;
	struct rusage myUsage;

#if defined(Linux)
#else /* Solaris */
	psinfo_t psInfo;
#endif

#endif

#ifdef WIN32

	/* GetProcessTimes gives us the user/system time in the form of 100s of nanoseconds. */
	if (!GetProcessTimes(pStats->_currentProcess, &creationTime, &exitTime, &kernelTime, &userTime))
	{
		printf("GetProcessTimes() failed.\n");
		return RSSL_RET_FAILURE;
	}

	/* FILETIME is a two-part structure. To operate on the value, it must be stored into a large integer. */
	tmpTime.HighPart = userTime.dwHighDateTime;
	tmpTime.LowPart = userTime.dwLowDateTime;
	userUsec = tmpTime.QuadPart / 10;
	
	tmpTime.HighPart = kernelTime.dwHighDateTime;
	tmpTime.LowPart = kernelTime.dwLowDateTime;
	kernelUsec = tmpTime.QuadPart / 10;

#else

	if (getrusage(RUSAGE_SELF, &myUsage) < 0)
	{
		perror("getrusage");
		return RSSL_RET_FAILURE;
	}

	userUsec = myUsage.ru_utime.tv_sec; userUsec *= 1000000;
	userUsec += myUsage.ru_utime.tv_usec;

	kernelUsec = myUsage.ru_stime.tv_sec; kernelUsec *= 1000000;
	kernelUsec += myUsage.ru_stime.tv_usec;
#endif

	/* Get memory "Working Set" size */

#ifdef WIN32

	if (!GetProcessMemoryInfo(pStats->_currentProcess, &memoryCounters, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		printf("GetProcessTimes() failed.\n");
		return RSSL_RET_FAILURE;
	}
	pStats->memUsageBytes = memoryCounters.WorkingSetSize;

#elif defined(Linux)

	/* On Linux, the resident set size is available in statm. */
	if (!(processFile = fopen("/proc/self/statm", "r")))
	{
		printf("Failed to open /proc/self/statm\n");
		return RSSL_RET_FAILURE;
	}

	/* The second parameter in /proc/self/statm is the resident set size, in pages. */
	if (2 != fscanf(processFile, "%llu %llu", &totalProgramSize, &pStats->memUsageBytes))
	{
		printf("fscanf() failed.\n");
		return RSSL_RET_FAILURE;
	}
	fclose(processFile);

	pStats->memUsageBytes *= pStats->_pageSize;


#else /* Solaris */

	/* On solaris, the resident set size is available in psinfo. */
	if (!(processFile = fopen("/proc/self/psinfo", "r")))
	{
		printf("Failed to open /proc/self/psinfo\n");
		return RSSL_RET_FAILURE;
	}
	if (fread(&psInfo, sizeof(psinfo_t), 1, processFile) < 0)
	{
		printf("Failed to read /proc/self/psinfo\n");
		return RSSL_RET_FAILURE;
	}
	fclose(processFile);

	pStats->memUsageBytes = psInfo.pr_rssize * 1024;

#endif


	/* Add up total time(user & system). */
	pStats->cpuUsageFraction = (RsslDouble)(userUsec - pStats->_prevUserTimeUsec + kernelUsec - pStats->_prevKernelTimeUsec);

	/* Divide over total time to get the CPU usage as a fraction. */
	currentTimeUsec = rsslGetTimeMicro();
	pStats->cpuUsageFraction /= ((RsslDouble)(currentTimeUsec - pStats->_prevTimeUsec));

	pStats->_prevUserTimeUsec = userUsec; 
	pStats->_prevKernelTimeUsec = kernelUsec;
	pStats->_prevTimeUsec = currentTimeUsec;

	return RSSL_RET_SUCCESS;
}
