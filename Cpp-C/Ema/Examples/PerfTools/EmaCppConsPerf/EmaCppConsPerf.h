///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_consPerf_h_
#define __ema_consPerf_h_

#if defined(WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "ConsumerThread.h"
class ConsumerStats;
class ConsumerThread;
class ItemInfo;
class ItemRequest;
class ConsPerfConfig;
using namespace refinitiv::ema::access;
class EmaCppConsPerf {

public:

	EmaCppConsPerf() : currentTime(0), startTime(0), endTime(0) {};
	~EmaCppConsPerf();
	bool initConsPerfConfig(int argc, char *argv[]);

	void printConsPerfConfig(FILE *file);
	bool inititailizeAndRun( int argc, char *argv[] );

	// Collects test statistics from all consumer threads.
	void collectStats(bool writeStats, bool displayStats, UInt32 currentRuntimeSec, 
		UInt32 timePassedSec);

	bool shutdownThreads();
	void consumerCleanupThreads();
	void printSummaryStatistics(FILE *file);
	static void exitOnMissingArgument(char **argv, int argPos);
	static void exitWithUsage();
	static void exitConfigError(char **argv);

protected:
	ConsPerfConfig	consPerfConfig;
	ConsumerStats	totalStats;
	ResourceUsageStats	resourceStats;
	ValueStatistics cpuUsageStats;
	ValueStatistics memUsageStats;
	PerfTimeValue	currentTime;
	PerfTimeValue	startTime;
	PerfTimeValue	endTime;
	PerfTimeValue	nextTime;

	perftool::common::AppVector<ConsumerThread*>	consumerThreads;

	static FILE *summaryFile;
	static EmaString	logText;
};

#endif // __ema_consPerf_h_
