///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#ifndef _PERF_CONFIG_H
#define _PERF_CONFIG_H

#include <cstddef>
#include "Ema.h"

const size_t MAX_THREADS = 8;

using namespace refinitiv::ema::access;
// Provides common configuration options.
class PerfConfig
{
public:
	PerfConfig( const EmaString& summaryFileName );

	virtual ~PerfConfig();
	virtual void clearPerfConfig() = 0;		// Use Defaults.

	Int32			ticksPerSec;		// Main loop ticks per second.  See -tps

	Int32			threadCount;		// Number of threads that handle connections.  See -threads.
	EmaString		threadBindList[MAX_THREADS];	// CPU ID list for threads that handle connections.  See -threads.

	EmaString		mainThreadCpu;		// CPU for binding the main thread of the app.  See -mainThread.
	EmaString		apiThreadBindList[MAX_THREADS];		// CPU ID list for API threads that dispatched messages, when ApiDispatch.  See -apiThreads.
	EmaString		workerThreadBindList[MAX_THREADS];	// CPU ID list for Reactor worker threads.  See -workerThreads.

	EmaString		summaryFilename;	// Name of the summary log file. See -summaryFile.

	bool			useUserDispatch;		/* Configures that the application is responsible for calling dispatch method to dispatch all received messages
											* otherwise (false/ApiDispatch) EMA creates a second internal thread over which to dispatch received messages */

	void getThreadListAsString(const EmaString* threads, char* destStr, size_t len);
};

#endif // _PERF_CONFIG_H
