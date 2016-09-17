///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "PerfConfig.h"

PerfConfig::PerfConfig( char* summaryFilename ) : 
	threadCount(1), 
	threadBindList(0), 
	ticksPerSec(1000),
	mainThreadCpu(-1),
	emaThreadCpu(-1),
	summaryFilename (summaryFilename),
	useUserDispatch(false)
{
	threadBindList = new long[1];
	threadBindList[0] = -1;	
}

PerfConfig::~PerfConfig()
{
	if( threadBindList )
		delete [] threadBindList;
}
