///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2021-2022 Refinitiv. All rights reserved.         --
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
	providerName(""),
	packedMsgBufferSize(0),
	numberMsgInPackedMsg(0)
{
}

NIProvPerfConfig::~NIProvPerfConfig()
{
}

void NIProvPerfConfig::clearPerfConfig()
{
	// PerfConfig fields
	ticksPerSec = 1000;
	summaryFilename = defSummaryFilename;
	useUserDispatch = false;

	threadCount = 1;
	mainThreadCpu.clear();
	threadBindList[0].clear();
	apiThreadBindList[0].clear();
	workerThreadBindList[0].clear();

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

	packedMsgBufferSize = 0;
	numberMsgInPackedMsg = 0;
}
