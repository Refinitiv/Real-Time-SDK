///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
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
	Int32			threadCount;		// Number of threads that handle connections.  See -threads 
	long			threadBindList[MAX_THREADS];	// CPU ID list for threads that handle connections.  See -threads 

	EmaString		summaryFilename;	// Name of the summary log file. See -summaryFile. 

	long			mainThreadCpu;
	long			emaThreadCpu;
	bool			useUserDispatch;		/* Configures that the application is responsible for calling dispatch method to dispatch all received messages
											* otherwise (false/ApiDispatch) EMA creates a second internal thread over which to dispatch received messages */
};

#endif // _PERF_CONFIG_H
