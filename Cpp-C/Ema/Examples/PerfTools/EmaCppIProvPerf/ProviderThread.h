/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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

#include "IProvPerfConfig.h"
#include "PerfMessageData.h"
#include "ProviderPerfClient.h"
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

const EmaString providerThreadNameBase = "Perf_Provider_";

class ProviderThreadState {
public:
	ProviderThreadState(const IProvPerfConfig&);
	void reset() {
		currentUpdatesItem = NULL;
		currentGenericsItem = NULL;
	}

	UInt32 getCurrentTick() const { return currentTick; }
	void incrementCurrentTick() {
		if (++currentTick >= (UInt32)provConfig.ticksPerSec)
			currentTick = 0;
	}

	UInt32 getUpdatesPerTick() const { return updatesPerTick; }
	UInt32 getUpdatesPerTickRemainder() const { return updatesPerTickRemainder; }
	ProvItemInfo* getCurrentUpdatesItem() const { return currentUpdatesItem; }
	void setCurrentUpdatesItem(ProvItemInfo* item) { currentUpdatesItem = item; }

	UInt32 getGenericsPerTick() const { return genericsPerTick; }
	UInt32 getGenericsPerTickRemainder() const { return genericsPerTickRemainder; }
	ProvItemInfo* getCurrentGenericsItem() const { return currentGenericsItem; }
	void setCurrentGenericsItem(ProvItemInfo* item) { currentGenericsItem = item; }

private:
	const IProvPerfConfig& provConfig;

	UInt32	currentTick;					/* Current tick out of ticks per second. */

	// Sending UpdateMsg-es
	UInt32	updatesPerTick;					/* Updates per tick */
	UInt32	updatesPerTickRemainder;		/* Updates per tick (remainder) */
	ProvItemInfo* currentUpdatesItem;		/* Current item in updates rotating list */

	// Sending GenericMsg-es
	UInt32	genericsPerTick;				/* Number of Generic per tick */
	UInt32	genericsPerTickRemainder;		/* Generics per tick (remainder) */
	ProvItemInfo* currentGenericsItem;		/* Current item in Generics rotating list */

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
	CountStat				postMsgCount;			/* Counts posts received. */
	CountStat				outOfBuffersCount;		/* Counts updates not sent due to lack
													 * of output buffers. */
	CountStat				statusCount;			/* Number of item status messages received. */

	ValueStatistics			intervalMsgEncodingStats;	/* Time of encoding message */

	PerfTimeValue			inactiveTime;			/* Time at which channel became inactive/down */

	PerfTimeValue			firstGenMsgSentTime;		/* Time at which first generic message sent. */
	PerfTimeValue			firstGenMsgRecvTime;		/* Time at which first generic message received. */

	ValueStatistics			genMsgLatencyStats;			/* Gen Msg latency statistics. */
	ValueStatistics			intervalGenMsgLatencyStats;	/* Gen Msg latency statistics (recorded by stats thread). */

	CountStat				genMsgSentCount;			/* Counts generic messages sent. */
	CountStat				genMsgRecvCount;			/* Counts generic messages received. */
	CountStat				latencyGenMsgSentCount;		/* Counts latency generic messages sent. */
	CountStat				packedMsgCount;				/* Counts packed messages sent. */

	ValueStatistics			tunnelStreamBufUsageStats;	/* Tunnel Buffer Usage statistics. */

	LatencyCollection		genMsgLatencyRecords;		/* Collection of timestamp information(for gen msgs), collected periodically by the main thread. */
	LatencyCollection		messageEncodeTimeRecords;	/* List of time-records for measurement of encoding time */
	LatencyCollection		messageDecodeTimeRecords;	/* Time spent decoding msgs. */

};  // class ProviderStats


class ProviderThread
{
public:
	ProviderThread(IProvPerfConfig&, PerfMessageData*, Int32 provIndex);
	~ProviderThread();

	void providerThreadInit();

	void start();

	void stop();

	void run();

	void sendBurstMessages();

	void sendPackedMsg(const Msg* msg, ProvItemInfo* itemInfo);

	void sendRefreshMessages();
	void sendUpdateMessages();
	void sendGenericMessages();

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

	void addRefreshItem(ProvItemInfo* itemInfo) { refreshItems.add(itemInfo); }

	void addClosedClientHandle(UInt64);

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
	IProvPerfConfig& provPerfConfig;
	PerfMessageData* perfMessageData;	// Pre-encoded messages data

	ProviderPerfClient* providerClient;
	OmmProvider* provider;

	PackedMsg* packedMsg;
	Int32 packCountCurrent;

	ProviderThreadState provThreadState;

	ProviderStats		stats;			// Statistics, collected periodically by the main thread.

	FILE* statsFile;					// File for logging stats for this connection.
	FILE* latencyLogFile;				// File for logging latency for this connection.

	LatencyRandomArray* latencyUpdateRandomArray;	// Determines when to send latency in UpdateMsg
	LatencyRandomArray* latencyGenericRandomArray;	// Determines when to send latency in GenericMsg

	// The item's lists
	RefreshItems refreshItems;						// The list of items that are requested by consumer for sending in Refreshes
	EmaList< ProvItemInfo* > updateItems;			// The list of items that are requested by consumer for sending in Updates

	perftool::common::Mutex	listClosedClientHandlesMutex;
	EmaVector< UInt64 > listClosedClientHandles;	// The array of client handles that closed (a client sent the close msg). All their items should be removed.

	// Refresh Message. Fill up payload for MarketPrice.
	void prepareRefreshMessageMarketPrice(RefreshMsg& refreshMsg);

	// Refresh Message. Fill up payload for MarketByOrder.
	void prepareRefreshMessageMarketByOrder(RefreshMsg& refreshMsg);

	// Update Message. Fill up payload for MarketPrice.
	void prepareUpdateMessageMarketPrice(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime = 0);

	// Update Message. Fill up payload for MarketByOrder.
	void prepareUpdateMessageMarketByOrder(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime = 0);

	// Generic Message. Fill up payload for MarketPrice.
	void prepareGenericMessageMarketPrice(GenericMsg& genericMsg, PerfTimeValue latencyStartTime = 0);

	// Generic Message. Fill up payload for MarketByOrder.
	void prepareGenericMessageMarketByOrder(GenericMsg& genericMsg, PerfTimeValue latencyStartTime = 0);

	void clean();

	// Removes all the items from updateItems array when their stream closes.
	void processClosedHandles();

};  // class ProviderThread

#endif  // _PROVIDER_THREAD_H_
