/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef _Ema_Cpp_IProv_Perf_
#define _Ema_Cpp_IProv_Perf_

#if defined(WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <cstdio>

#include "EmaString.h"
#include "IProvPerfConfig.h"
#include "AppVector.h"
#include "Mutex.h"
#include "AppUtil.h"
#include "Statistics.h"
#include "ProviderThread.h"


class EmaCppIProvPerf
{
public:
	EmaCppIProvPerf() : perfMessageData(NULL), summaryFile(NULL) {};

	bool inititailizeAndRun(int argc, char* argv[]);

protected:
	bool initIProvPerfConfig(int argc, char* argv[]);
	void printIProvPerfConfig(FILE* file);

	bool monitorStatistics();
	bool collectStats(bool writeStats, bool displayStats,
			refinitiv::ema::access::UInt32 currentRuntimeSec,
			refinitiv::ema::access::UInt32 timePassedSec);
	void providerPrintSummaryStats(FILE*);

	bool shutdownThreads();
	void cleanupThreads();

private:
	static void exitOnMissingArgument(char** argv, int argPos);
	static void exitWithUsage();
	static void exitConfigError(char** argv);

	IProvPerfConfig	provPerfConfig;
	PerfMessageData* perfMessageData;

	ProviderStats totalStats;
	ResourceUsageStats	resourceStats;
	ValueStatistics cpuUsageStats;
	ValueStatistics memUsageStats;

	static refinitiv::ema::access::EmaString		logText;

	perftool::common::AppVector<ProviderThread*>	providerThreads;

	FILE* summaryFile;				// Logs summary information, such as application inputs and final statistics.
};

#endif // _Ema_Cpp_IProv_Perf_
