/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* LatencyCollection.h
 * Collection of latency timestamps.
 * Provides the logic that fills up and reads collection.
 */

#pragma once

#ifndef _LATENCY_COLLECTION_DATA_H_
#define _LATENCY_COLLECTION_DATA_H_

#include "AppVector.h"
#include "Mutex.h"
#include "Statistics.h"

typedef perftool::common::AppVector< TimeRecord > LatencyRecords;

class LatencyCollection
{
public:
	LatencyCollection();

	void updateLatencyStats(PerfTimeValue startTime, PerfTimeValue endTime, PerfTimeValue tick);
	void getLatencyTimeRecords(LatencyRecords** pUpdateLatList);
	void clearReadLatTimeRecords() { pReadListPtr->clear(); }

private:
	perftool::common::Mutex	statsLatencyMutex;
	LatencyRecords			latencyList1;
	LatencyRecords			latencyList2;
	LatencyRecords*			pWriteListPtr;
	LatencyRecords*			pReadListPtr;

	static const refinitiv::ema::access::UInt32 LATENCY_CAPACITY = 10000;
};  // class LatencyCollection

#endif  // _LATENCY_COLLECTION_DATA_H_
