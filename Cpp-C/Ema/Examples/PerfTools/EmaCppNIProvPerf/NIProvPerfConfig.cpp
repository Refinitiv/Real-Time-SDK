///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2021 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "NIProvPerfConfig.h"

using namespace refinitiv::ema::access;

EmaString NIProvPerfConfig::defSummaryFilename = "NIProvSummary.out";

NIProvPerfConfig::NIProvPerfConfig() : PerfConfig((char*)defSummaryFilename.c_str()),
	runTime(360),
	writeStatsInterval(5),
	displayStats(true),
	preEncItems(false),
	useServiceId(false),
	serviceId(1),
	serviceName("DIRECT_FEED"),
	itemFilename("350k.xml"),
	msgFilename("MsgData.xml"),
	logLatencyToFile(false),
	latencyLogFilename(""),
	statsFilename("NIProvStats"),
	itemCount(100000),
	refreshBurstSize(10),
	updatesPerSec(100000),
	latencyUpdatesPerSec(10),
	nanoTime(false),
	measureEncode(false),
	measureDecode(false),
	loginPosition(""),
	providerName("")
{
	apiThreadBindList[0] = -1;
}

NIProvPerfConfig::~NIProvPerfConfig()
{
}

void NIProvPerfConfig::clearPerfConfig()
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

	useServiceId = false;
	serviceId = 1;
	serviceName = "DIRECT_FEED";

	itemFilename = "350k.xml";
	msgFilename = "MsgData.xml";
	logLatencyToFile = false;
	latencyLogFilename = "";
	statsFilename = "NIProvStats";

	itemCount = 100000;
	refreshBurstSize = 10;
	updatesPerSec = 100000;
	latencyUpdatesPerSec = 10;

	nanoTime = false;
	measureEncode = false;
	measureDecode = false;
	loginPosition = "";
	providerName = "";
}
