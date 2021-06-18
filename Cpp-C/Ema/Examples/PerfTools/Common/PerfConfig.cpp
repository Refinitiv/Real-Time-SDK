///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "PerfConfig.h"

PerfConfig::PerfConfig( const EmaString& summaryFilename ) :
	threadCount(1), 
	ticksPerSec(1000),
	mainThreadCpu(-1),
	emaThreadCpu(-1),
	summaryFilename (summaryFilename),
	useUserDispatch(false)
{
	threadBindList[0] = -1;	
}

PerfConfig::~PerfConfig()
{
}
