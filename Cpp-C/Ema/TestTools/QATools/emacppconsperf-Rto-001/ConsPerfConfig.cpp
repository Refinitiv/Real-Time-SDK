/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ConsPerfConfig.h"

ConsPerfConfig::ConsPerfConfig() : PerfConfig ( (char *) "ConsSummary.out" ), steadyStateTime(300), delaySteadyStateCalc(0),
statsFilename("ConsStats"), writeStatsInterval(5), displayStats(true), logLatencyToFile(false), 
// APIQA:
itemRequestCount(1), commonItemCount(0), itemRequestsPerSec(35000), requestSnapshots(false),
// END APIQA
serviceName("DIRECT_FEED"), useServiceId(false), itemFilename("350k.xml"),
msgFilename("MsgData.xml"), postsPerSec(0), latencyPostsPerSec(0), genMsgsPerSec(0), latencyGenMsgsPerSec(0),
consumerName(""), websocketProtocol(NoWebSocketEnum)
{
	apiThreadBindList[0] = -1;	
}
void ConsPerfConfig::clearPerfConfig()
{
	steadyStateTime = 300; 
	delaySteadyStateCalc = 0;
	threadCount = 1;
	threadBindList[0] = -1;
	apiThreadBindList[0] = -1;

	summaryFilename = "ConsSummary.out"; 

	statsFilename = "ConsStats";
	writeStatsInterval = 5; 
	displayStats = true; 
	logLatencyToFile = true; 
	serviceName = "DIRECT_FEED";
	ticksPerSec = 1000;
// APIQA:
	itemRequestCount = 1; 
// END APIQA
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
}

ConsPerfConfig::~ConsPerfConfig()
{
}
