///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2020-2022 LSEG. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "ConsPerfConfig.h"

ConsPerfConfig::ConsPerfConfig() : PerfConfig ( (char *) "ConsSummary.out" ), steadyStateTime(300), delaySteadyStateCalc(0),
statsFilename("ConsStats"), writeStatsInterval(5), displayStats(true), logLatencyToFile(false), 
itemRequestCount(100000), commonItemCount(0), itemRequestsPerSec(35000), requestSnapshots(false),
serviceName("DIRECT_FEED"), useServiceId(false), itemFilename("350k.xml"),
msgFilename("MsgData.xml"), postsPerSec(0), latencyPostsPerSec(0), genMsgsPerSec(0), latencyGenMsgsPerSec(0),
consumerName(""), websocketProtocol(NoWebSocketEnum),
securityProtocol(OmmConsumerConfig::ENC_NONE)
{
}
void ConsPerfConfig::clearPerfConfig()
{
	steadyStateTime = 300; 
	delaySteadyStateCalc = 0;

	threadCount = 1;
	mainThreadCpu.clear();
	threadBindList[0].clear();
	apiThreadBindList[0].clear();
	workerThreadBindList[0].clear();

	summaryFilename = "ConsSummary.out"; 

	statsFilename = "ConsStats";
	writeStatsInterval = 5; 
	displayStats = true; 
	logLatencyToFile = true; 
	serviceName = "DIRECT_FEED";
	ticksPerSec = 1000;
	itemRequestCount = 100000; 
	commonItemCount = 0; 
	itemRequestsPerSec = 35000; 
	requestSnapshots = false;
	postsPerSec = 0; 
	latencyPostsPerSec = 0; 
	genMsgsPerSec = 0; 
	latencyGenMsgsPerSec = 0;
	itemFilename = "350k.xml";
	msgFilename = "MsgData.xml";
	useServiceId = false;
	useUserDispatch = false;
	consumerName = "";
	websocketProtocol = NoWebSocketEnum;

	securityProtocol = OmmConsumerConfig::ENC_NONE;
}

ConsPerfConfig::~ConsPerfConfig()
{
}
