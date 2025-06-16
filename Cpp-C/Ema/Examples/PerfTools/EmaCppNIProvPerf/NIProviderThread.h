///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|         Copyright (C) 2021-2022, 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#pragma once

#ifndef _PROVIDER_THREAD_H_
#define _PROVIDER_THREAD_H_

#if defined(WIN32)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#include <assert.h>

#if defined(WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <process.h>
#include <windows.h>
#else

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

#include <iostream>

#include "NIProvPerfConfig.h"
#include "NIProviderPerfClient.h"
#include "PerfMessageData.h"
#include "ProvItemInfo.h"
#include "xmlPerfMsgDataParser.h"

#include "AppUtil.h"
#include "AppVector.h"
#include "GetTime.h"
#include "LatencyCollection.h"
#include "LatencyRandomArray.h"
#include "Mutex.h"
#include "ThreadBinding.h"
#include "Statistics.h"

class ProviderPerfClient;
class NIProviderPerfClient;
class ProvItemInfo;

const refinitiv::ema::access::EmaString providerThreadNameBase = "Perf_NIProvider_";

class ProviderThreadState {
public:
	ProviderThreadState(const NIProvPerfConfig&);
	void reset() {
		currentUpdatesItem = NULL;
	}

	UInt32 getCurrentTick() const { return currentTick; }
	void incrementCurrentTick() {
		if (++currentTick >= (UInt32)niProvConfig.ticksPerSec)
			currentTick = 0;
	}

	UInt32 getUpdatesPerTick() const { return updatesPerTick; }
	UInt32 getUpdatesPerTickRemainder() const { return updatesPerTickRemainder; }
	ProvItemInfo* getCurrentUpdatesItem() const { return currentUpdatesItem; }
	void setCurrentUpdatesItem(ProvItemInfo* item) { currentUpdatesItem = item; }

private:
	const NIProvPerfConfig& niProvConfig;

	UInt32	currentTick;					/* Current tick out of ticks per second. */

	// Sending UpdateMsg-es
	UInt32	updatesPerTick;					/* Updates per tick */
	UInt32	updatesPerTickRemainder;		/* Updates per tick (remainder) */
	ProvItemInfo* currentUpdatesItem;		/* Current item in updates rotating list */

};  // class ProviderThreadState


class ProviderStats {
public:
	ProviderStats();
	ProviderStats(const ProviderStats&);
	ProviderStats& operator=(const ProviderStats&);
	~ProviderStats();

	CountStat				refreshMsgCount;		/* Counts refreshes sent. */
	CountStat				updateMsgCount;			/* Counts updates sent. */
	CountStat				itemRequestCount;		/* Counts requests received. */
	CountStat				closeMsgCount;			/* Counts closes received. */
	CountStat				statusCount;			/* Number of item status messages received. */
	CountStat				packedMsgCount;			/* Counts packed messages sent. */

	ValueStatistics			intervalMsgEncodingStats;	/* Time of encoding message */

	PerfTimeValue			inactiveTime;			/* Time at which channel became inactive/down */

	LatencyCollection		messageEncodeTimeRecords;	/* List of time-records for measurement of encoding time */
	LatencyCollection		messageDecodeTimeRecords;	/* Time spent decoding msgs. */
};  // class ProviderStats


class NIProviderThread
{
public:
	NIProviderThread(NIProvPerfConfig&, PerfMessageData*, Int32 provIndex);
	~NIProviderThread();

	void providerThreadInit();
	void providerThreadInitItems(UInt32 itemListCount, UInt32 itemListStartIndex);

	void start();

	void stop();

	void run();

	void sendBurstMessages();

	void sendRefreshMessages();
	void sendUpdateMessages();
	void sendPackedMsg(const Msg* msg, ProvItemInfo* itemInfo);

	bool isRunning() { return running; }
	bool isStopped() { return stopThread; }

	void setStopThread() { stopThread = true; }

	ProviderStats& getProviderStats() { return stats; }

	FILE* getStatsFile() { return statsFile; }
	FILE* getLatencyLogFile() { return latencyLogFile; }

	void setCpuId(const EmaString& id) { cpuId = id; }
	const EmaString& getCpuId() const { return cpuId; }

	void setApiThreadCpuId(const EmaString& id) { apiThreadCpuId = id; }
	const EmaString& getApiThreadCpuId() const { return apiThreadCpuId; }

	void setWorkerThreadCpuId(const EmaString& id) { workerThreadCpuId = id; }
	const EmaString& getWorkerThreadCpuId() const { return workerThreadCpuId; }

	Int32 getThreadIndex() { return providerThreadIndex; }

protected:
	bool				stopThread;
	bool				running;

	Int32				providerThreadIndex;
	EmaString			cpuId;				// CPU for binding Consumer thread.
	EmaString			apiThreadCpuId;		// CPU for binding EMA API internal thread when ApiDispatch mode set.
	EmaString			workerThreadCpuId;	// CPU for binding Reactor Worker thread.

#if defined(WIN32)
	static unsigned __stdcall ThreadFunc(void* pArguments);

	HANDLE					_handle;
	unsigned int			_threadId;
#else
	static void* ThreadFunc(void* pArguments);

	pthread_t				_threadId;
#endif

private:
	NIProvPerfConfig& niProvPerfConfig;
	PerfMessageData* perfMessageData;	// Pre-encoded messages data

	NIProviderPerfClient* niProviderClient;
	OmmProvider* provider;

	PackedMsg* packedMsg;
	Int32 packCountCurrent;

	ProviderThreadState provThreadState;

	ProviderStats		stats;			// Statistics, collected periodically by the main thread.

	FILE* statsFile;					// File for logging stats for this connection.
	FILE* latencyLogFile;				// File for logging latency for this connection.

	LatencyRandomArray* latencyUpdateRandomArray;	// Determines when to send latency in UpdateMsg

	// The item's lists
	EmaList< ProvItemInfo* > refreshItems;		// The list of items that are published by niprovider as Refreshes
	EmaList< ProvItemInfo* > updateItems;		// The list of items that are published by niprovider as Updates

	// Refresh Message. Fill up payload for MarketPrice.
	void prepareRefreshMessageMarketPrice(RefreshMsg& refreshMsg);

	// Refresh Message. Fill up payload for MarketByOrder.
	void prepareRefreshMessageMarketByOrder(RefreshMsg& refreshMsg);

	// Update Message. Fill up payload for MarketPrice.
	void prepareUpdateMessageMarketPrice(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime = 0);

	// Update Message. Fill up payload for MarketByOrder.
	void prepareUpdateMessageMarketByOrder(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime = 0);

	void clean();
};  // class NIProviderThread

#endif  // _PROVIDER_THREAD_H_
