///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

#pragma once

#ifndef _PERF_UTILS_H_
#define _PERF_UTILS_H_

#include "NIProvPerfConfig.h"
#include "NIProviderThread.h"

class PerfUtils
{
public:
	static bool initializeItems(NIProvPerfConfig const& niProvPerfConfig, EmaList< ProvItemInfo* >& items, UInt32 itemListCount, UInt32 itemListStartIndex);
};  // class PerfUtils

#endif // !_PERF_UTILS_H_
