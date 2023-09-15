///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
///*|-----------------------------------------------------------------------------

#include "PerfConfig.h"
#include <cstdio>

PerfConfig::PerfConfig( const EmaString& summaryFilename ) :
	threadCount(1), 
	ticksPerSec(1000),
	summaryFilename (summaryFilename),
	useUserDispatch(false)
{
}

PerfConfig::~PerfConfig()
{
}

void PerfConfig::getThreadListAsString(const EmaString* threads, char* destStr, size_t len)
{
	int i;
	int thStrPos = 0;

	if ( threads[0].empty() )
	{
		snprintf(destStr, len, "-1");
	}
	else
	{
		thStrPos = snprintf(destStr, len, "%s", threads[0].c_str());
		for (i = 1; i < threadCount; ++i)
		{
			thStrPos += snprintf(destStr + thStrPos, len - thStrPos, ",%s", threads[i].c_str());
		}
	}
	return;
}
