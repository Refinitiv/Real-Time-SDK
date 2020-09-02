///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#ifndef _CONS_PERF_CONFIG_H
#define _CONS_PERF_CONFIG_H

#include "Ema.h"
#include "../Common/PerfConfig.h"

#define MAX_CONS_THREADS 8
using namespace rtsdk::ema::access;
// Provides configuration options for the consumer. 
class ConsPerfConfig : public PerfConfig
{
public:
	ConsPerfConfig();
	~ConsPerfConfig();
	void clearPerfConfig();		// Use Defaults.

	UInt32			steadyStateTime;	// Time application runs befor exiting.  See -steadyStateTime 

	EmaString		itemFilename;	// File of names to use when requesting items. See -itemFile. 
	EmaString		msgFilename;	// File of data to use for message payloads. See -msgFile. 

	bool			logLatencyToFile;	// Whether to log update latency information to a file. See -latencyFile. 
	EmaString		latencyLogFilename;	// Name of the latency log file. See -latencyFile. 

	EmaString		statsFilename;	// Name of the statistics log file. See -statsFile. 
	UInt32			writeStatsInterval;	// Controls how often statistics are written. 
	bool			displayStats;	// Controls whether stats appear on the screen. 

	Int32			itemRequestsPerSec;		// Rate at which the consumer will send out item requests. See -rqps. 
	bool			requestSnapshots;			// Whether to request all items as snapshots. See -snapshot 

	EmaString		username;				// Username used when logging in. 
	EmaString		serviceName;			// Name of service to request items from. See -s. 
	bool			useServiceId;
	Int32			itemRequestCount;			// Number of items to request. See -itemCount. 
	Int32			commonItemCount;			// Number of items common to all connections, if
												// using multiple connections. See -commonItemCount. 

	bool			useUserDispatch;
	Int32			postsPerSec;				// Number of posts to send per second. See -postingRate. 
	Int32			latencyPostsPerSec;			// Number of latency posts to send per second. See -latPostingRate. 

	Int32			genMsgsPerSec;				// Number of generic messages to send per second. See -genMsgRate. 
	Int32			latencyGenMsgsPerSec;		// Number of latency generic messages to send per second. See -latGenMsgRate. 

	Int32			_requestsPerTick;
	Int32			_requestsPerTickRemainder;
	long			*apiThreadBindList;	// CPU ID list for threads that handle connections.  See -threads 

	enum WebsocketProtocol
	{
		NoWebSocketEnum,
		WebSocketJSONEnum,
		WebSocketRWFEnum
	} websocketProtocol;
};

#endif // _CONS_PERF_CONFIG_H
