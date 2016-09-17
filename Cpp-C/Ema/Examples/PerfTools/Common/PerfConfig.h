///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef _PERF_CONFIG_H
#define _PERF_CONFIG_H

#include "Ema.h"

#define MAX_CONS_THREADS 8
using namespace thomsonreuters::ema::access;
// Provides common configuration options. 
class PerfConfig
{
public:
	PerfConfig( char* summaryFileName );

	virtual ~PerfConfig();
	virtual void clearPerfConfig() = 0;		// Use Defaults.

	Int32			ticksPerSec;		// Main loop ticks per second.  See -tps 
	Int32			threadCount;		// Number of threads that handle connections.  See -threads 
	long			*threadBindList;	// CPU ID list for threads that handle connections.  See -threads 

	EmaString		summaryFilename;	// Name of the summary log file. See -summaryFile. 

	long			mainThreadCpu;
	long			emaThreadCpu;
	bool			useUserDispatch;

};

#endif // _PERF_CONFIG_H
