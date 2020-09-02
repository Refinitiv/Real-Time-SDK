///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef _STATISTICS_H
#define _STATISTICS_H
// Simple statistics include mean, minimum, maximum, and variance.
// The standard deviation can be obtained by taking the square root of the variance.

#if defined(WIN32)
#include <Winsock2.h>
typedef SOCKET DUMMY_SOCKET;
#define SOCK_ERR WSAGetLastError()
#else
#include <sys/socket.h>
typedef int DUMMY_SOCKET;
#define SOCK_ERR errno
#endif

#undef maxValue
#undef minValue
#include "../Common/GetTime.h"
#include <float.h>
#include "Ema.h"
#include <stdio.h>
using namespace rtsdk::ema::access;

struct ResourceUsageStats {
	double cpuUsageFraction;
	UInt64 memUsageBytes;

	PerfTimeValue _prevTimeUsec;
	Int64 _prevUserTimeUsec;
	Int64 _prevKernelTimeUsec;
	ResourceUsageStats() : cpuUsageFraction(0), memUsageBytes(0), _prevTimeUsec(0), _prevUserTimeUsec(0), _prevKernelTimeUsec(0) {};
	bool initResourceUsageStats();
	bool getResourceUsageStats();
	ResourceUsageStats& operator=(const ResourceUsageStats& other)
	{
		cpuUsageFraction = other.cpuUsageFraction;
		memUsageBytes = other.memUsageBytes;
		_prevTimeUsec = other._prevTimeUsec;
		_prevUserTimeUsec = other._prevUserTimeUsec;
		_prevKernelTimeUsec = other._prevKernelTimeUsec;
		return *this;
	};
	bool operator==(const ResourceUsageStats& other) const
	{
		if(cpuUsageFraction == other.cpuUsageFraction && memUsageBytes == other.memUsageBytes
			&& _prevTimeUsec == other._prevTimeUsec && _prevKernelTimeUsec == other._prevKernelTimeUsec)
			return true;
		return false;
	};
#if defined(WIN32)
	HANDLE _currentProcess;
#elif defined(Linux)
	int _pageSize;
#endif
};

struct ValueStatistics
{
	UInt64	count;//number of values
	double				mean; //mean value
	double				minValue;  //minumum value
	double				maxValue;  //maximum value
	double				variance;  //variance (the standard deviation is the square root of the variance)
	double				sum;  //running sum of values (used to compute variance)
	double				sumOfSquares;  //running sum of squared values (used to compute variance)
	 ValueStatistics() : count(0), mean(0.0), maxValue(-DBL_MAX), minValue(DBL_MAX), variance(0.0), sum(0.0), sumOfSquares(0.0) {}
	void clearValueStatistics();
	void updateValueStatistics(double newValue);
	void printValueStatistics(FILE *file, const char *valueStatsName, const char *countUnitName, bool displayThousandths);

	ValueStatistics& operator=(const ValueStatistics& other)
	{
		count = other.count;
		mean = other.mean;
		minValue = other.minValue;
		maxValue = other.maxValue;
		variance = other.variance;
		sum = other.sum;
		sumOfSquares = other.sumOfSquares;
		return *this;
	};
	bool operator==(const ValueStatistics& other) const
	{
		if(count == other.count && mean == other.mean && minValue == other.minValue && maxValue == other.maxValue && variance == other.variance)
			return true;
		return false;
	};
};

struct TimeRecord
{
	PerfTimeValue startTime;	// Recorded start time.
	PerfTimeValue endTime;		// Recorded end time. 
	PerfTimeValue ticks;		// Units per microsecond.
	TimeRecord() : startTime(0), endTime(0), ticks(0) {};
	TimeRecord& operator=(const TimeRecord& other)
	{
		startTime = other.startTime;
		endTime = other.endTime;
		ticks = other.ticks;
		return *this;
	};
	bool operator==(const TimeRecord& other) const
	{
		if(startTime == other.startTime && endTime == other.endTime && ticks == other.ticks)
			return true;
		return false;
	};
};

struct CountStat
{
	UInt64 currentValue;	// The current value. 
	UInt64 prevValue;		// Value returned from the previous call to countStatGetChange.
	CountStat() : currentValue( 0 ), prevValue( 0 ) {};

	void countStatIncr();
	void countStatAdd( UInt64 addend );
	UInt64 countStatGetChange();
	UInt64 countStatGetTotal();
	CountStat& operator=(const CountStat& other)
	{
		currentValue = other.currentValue;
		prevValue = other.prevValue;
		return *this;
	}
	bool operator==(const CountStat& other) const
	{
		if(currentValue == other.currentValue && prevValue == other.prevValue )
			return true;
		return false;
	}
};

// Resource Statistics - TO DO

// Increment the count.
inline void CountStat::countStatIncr()
{
	++currentValue;
}
// Add to the count.
inline void CountStat::countStatAdd( UInt64 addend )
{
	currentValue += addend;
}
// Get the difference between the current count and that of the previous call.
inline UInt64 CountStat::countStatGetChange( )
{
	UInt64 tmpCurrentValue = currentValue;
	tmpCurrentValue -= prevValue;
	prevValue = currentValue;
	return tmpCurrentValue;
}
// Get the current overall count. 
inline UInt64 CountStat::countStatGetTotal( )
{
	return currentValue;
}
inline void ValueStatistics::clearValueStatistics()
{
	count = 0;
	mean = 0;
	variance = 0;
	maxValue = -DBL_MAX;
	minValue = DBL_MAX;		
	sum = 0;
	sumOfSquares = 0;
}
#endif // _STATISTICS_H
