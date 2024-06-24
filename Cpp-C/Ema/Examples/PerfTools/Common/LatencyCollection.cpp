///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|              Copyright (C) 2021 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

#include "LatencyCollection.h"

LatencyCollection::LatencyCollection() :
	latencyList1(LATENCY_CAPACITY),
	latencyList2(LATENCY_CAPACITY),
	pWriteListPtr(&latencyList1),
	pReadListPtr(&latencyList2)
{
}

void LatencyCollection::updateLatencyStats(PerfTimeValue startTime, PerfTimeValue endTime, PerfTimeValue tick)
{
	TimeRecord ldata;

	ldata.startTime = startTime;
	ldata.endTime = endTime;
	ldata.ticks = tick;

	statsLatencyMutex.lock();
	pWriteListPtr->push_back(ldata); // Submit Time record.
	statsLatencyMutex.unlock();
}

void LatencyCollection::getLatencyTimeRecords(LatencyRecords** pUpdateLatencyList)
{
	statsLatencyMutex.lock();

	// pass the current write list pointer so the data can be read
	// and swap read and write pointers
	*pUpdateLatencyList = pWriteListPtr;
	pWriteListPtr = pReadListPtr;
	pReadListPtr = *pUpdateLatencyList;

	statsLatencyMutex.unlock();
}
