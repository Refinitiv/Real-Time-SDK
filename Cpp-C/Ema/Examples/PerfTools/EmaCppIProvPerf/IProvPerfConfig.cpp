///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2021 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "IProvPerfConfig.h"
#include "AppUtil.h"

using namespace refinitiv::ema::access;

EmaString IProvPerfConfig::defSummaryFilename = "IProvSummary.out";

IProvPerfConfig::IProvPerfConfig() : PerfConfig((char*)defSummaryFilename.c_str()),
	runTime(360),
	writeStatsInterval(5),
	displayStats(true),
	preEncItems(false),
	msgFilename("MsgData.xml"),
	logLatencyToFile(false),
	latencyLogFilename(""),
	statsFilename("ProvStats"),
	refreshBurstSize(10),
	updatesPerSec(100000),
	latencyUpdatesPerSec(10),
	genMsgsPerSec(0),
	latencyGenMsgsPerSec(0),
	nanoTime(false),
	measureEncode(false),
	measureDecode(false),
	loginPosition(""),
	providerName("")
{
	apiThreadBindList[0] = -1;
}

IProvPerfConfig::~IProvPerfConfig()
{
}

void IProvPerfConfig::clearPerfConfig()
{
	// PerfConfig fields
	ticksPerSec = 1000;
	mainThreadCpu = -1;
	summaryFilename = defSummaryFilename;
	useUserDispatch = false;

	threadCount = 1;
	threadBindList[0] = -1;
	apiThreadBindList[0] = -1;

	runTime = 360;
	writeStatsInterval = 5;
	displayStats = true;
	preEncItems = false;

	msgFilename = "MsgData.xml";
	logLatencyToFile = false;
	latencyLogFilename = "";
	statsFilename = "ProvStats";

	refreshBurstSize = 10;
	updatesPerSec = 100000;
	latencyUpdatesPerSec = 10;
	genMsgsPerSec = 0;
	latencyGenMsgsPerSec = 0;

	nanoTime = false;
	measureEncode = false;
	measureDecode = false;
	loginPosition = "";
	providerName = "";
}
