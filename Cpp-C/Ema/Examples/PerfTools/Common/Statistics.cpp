///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Statistics.h"
#include "../Common/AppUtil.h"
#include <stdlib.h>
#include <math.h>
#if defined(WIN32)
#include <windows.h>
#include <Psapi.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#if defined(Linux)
#include <unistd.h>
#else
#include <procfs.h>
#endif
#endif

using namespace perftool::common;

void ValueStatistics::printValueStatistics(FILE *file, const char *valueStatsName, const char *countUnitName, bool displayThousandths)
{
	const char *outputStr = displayThousandths ? 
		   "%s: Avg:%8.3f StdDev:%8.3f Max:%8.3f Min:%8.3f, %s: %llu\n"
		:  "%s: Avg:%6.1f StdDev:%6.1f Max:%6.0f Min:%6.0f, %s: %llu\n";

	printf(outputStr,
			valueStatsName,
			mean, 
			sqrt(variance),
			maxValue,
			minValue,
			countUnitName,
			count);
}
void ValueStatistics::updateValueStatistics(double newValue)
{
	count++;

	// Check against max & min 
	if (newValue > maxValue) maxValue = newValue;
	if (newValue < minValue) minValue = newValue;


	// Average and variance are calculated using online algorithms.
	// Average: http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
	// Variance: Devore,Farnum. "Applied Statistics for Engineers and Scientists(Second Edition)", p. 73,74
	
	mean = (newValue + mean * (count-1))/count;
	sum += newValue;
	sumOfSquares += newValue * newValue;
	variance = count > 1 ? (sumOfSquares - sum * sum / count) / (count - 1) : 0;
}
bool ResourceUsageStats::initResourceUsageStats()
{
#if defined(WIN32)
	_currentProcess = GetCurrentProcess();
#elif defined(Linux)
	_pageSize = getpagesize();
#endif

	// Set the starting point for the usage stats. 
	bool ret = getResourceUsageStats();
	return ret;
}
bool ResourceUsageStats::getResourceUsageStats()
{
	Int64 userUsec, kernelUsec;
	TimeValue currentTimeUsec;


#if defined(WIN32)
	FILETIME creationTime, exitTime, kernelTime, userTime;
	ULARGE_INTEGER tmpTime;
	PROCESS_MEMORY_COUNTERS memoryCounters;
#else
	FILE *processFile;
	UInt64 totalProgramSize;
	struct rusage myUsage;
#endif

#if defined(WIN32)

	/* GetProcessTimes gives us the user/system time in the form of 100s of nanoseconds. */
	if (!GetProcessTimes(_currentProcess, &creationTime, &exitTime, &kernelTime, &userTime))
	{
		EmaString text("GetProcessTimes() failed.\n");
		AppUtil::logError(text);
		return false;
	}

	// FILETIME is a two-part structure. To operate on the value, it must be stored into a large integer. */
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
		return false;
	}

	userUsec = myUsage.ru_utime.tv_sec; userUsec *= 1000000;
	userUsec += myUsage.ru_utime.tv_usec;

	kernelUsec = myUsage.ru_stime.tv_sec; kernelUsec *= 1000000;
	kernelUsec += myUsage.ru_stime.tv_usec;
#endif

	/* Get memory "Working Set" size */

#if defined(WIN32)

	if (!GetProcessMemoryInfo(_currentProcess, &memoryCounters, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		EmaString text("GetProcessTimes() failed.");
		AppUtil::logError(text);
		return false;
	}
	memUsageBytes = memoryCounters.WorkingSetSize;

#elif defined(Linux)

	/* On Linux, the resident set size is available in statm. */
	if (!(processFile = fopen("/proc/self/statm", "r")))
	{
		EmaString text("Failed to open /proc/self/statm\n");
		AppUtil::logError(text);
		return false;
	}

	/* The second parameter in /proc/self/statm is the resident set size, in pages. */
	if (2 != fscanf(processFile, "%llu %llu", &totalProgramSize, &memUsageBytes))
	{
		EmaString text("fscanf() failed.\n");
		AppUtil::logError(text);
		return false;
	}
	fclose(processFile);

	memUsageBytes *= _pageSize;
#endif


	/* Add up total time(user & system). */
	cpuUsageFraction = (double)(userUsec - _prevUserTimeUsec + kernelUsec - _prevKernelTimeUsec);

	/* Divide over total time to get the CPU usage as a fraction. */
	currentTimeUsec = GetTime::getTimeMicro();
	cpuUsageFraction /= ((double)(currentTimeUsec - _prevTimeUsec));

	_prevUserTimeUsec = userUsec; 
	_prevKernelTimeUsec = kernelUsec;
	_prevTimeUsec = currentTimeUsec;

	return true;
}
