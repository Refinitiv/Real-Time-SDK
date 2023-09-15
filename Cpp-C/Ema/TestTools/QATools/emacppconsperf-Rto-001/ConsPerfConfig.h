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

const size_t MAX_CONS_THREADS = (MAX_THREADS);

const refinitiv::ema::access::UInt32	LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
const refinitiv::ema::access::Int32		ALWAYS_SEND_LATENCY_POSTMSG = (-1);
const refinitiv::ema::access::Int32		ALWAYS_SEND_LATENCY_GENMSG = (-1);

using namespace refinitiv::ema::access;
// Provides configuration options for the consumer. 
class ConsPerfConfig : public PerfConfig
{
public:
	ConsPerfConfig();
	~ConsPerfConfig();
	void clearPerfConfig();		// Use Defaults.

	UInt32			steadyStateTime;		// Time application runs befor exiting.  See -steadyStateTime.
	UInt32			delaySteadyStateCalc;	// Time before the latency is calculated. See -delaySteadyStateCalc.

	EmaString		itemFilename;		// File of names to use when requesting items. See -itemFile.
	EmaString		msgFilename;		// File of data to use for message payloads. See -msgFile.

	bool			logLatencyToFile;	// Whether to log update latency information to a file. See -latencyFile.
	EmaString		latencyLogFilename;	// Name of the latency log file. See -latencyFile.

	EmaString		statsFilename;		// Name of the statistics log file. See -statsFile.
	UInt32			writeStatsInterval;	// Controls how often statistics are written. See -writeStatsInterval.
	bool			displayStats;		// Controls whether stats appear on the screen. See -noDisplayStats.

	Int32			itemRequestsPerSec;			// Rate at which the consumer will send out item requests. See -requestRate.
	bool			requestSnapshots;			// Whether to request all items as snapshots. See -snapshot.

	EmaString		username;				// Username used when logging in. See -uname.
// APIQA:
	EmaString		clientId;				// ClientId used when logging in. See -clientId.
	EmaString		password;				// Username used when logging in. See -password.
	EmaString		clientSecret;			// Username used when logging in. See -clientSecret.
// END APIQA
	EmaString		serviceName;			// Name of service to request items from. See -serviceName.
	bool			useServiceId;
	Int32			itemRequestCount;			// Number of items to request. See -itemCount.
	Int32			commonItemCount;			// Number of items common to all connections, if
												// using multiple connections. See -commonItemCount.

	Int32			postsPerSec;				// Number of posts to send per second. See -postingRate.
	Int32			latencyPostsPerSec;			// Number of latency posts to send per second. See -postingLatencyRate.

	Int32			genMsgsPerSec;				// Number of generic messages to send per second. See -genericMsgRate.
	Int32			latencyGenMsgsPerSec;		// Number of latency generic messages to send per second. See -genericMsgLatencyRate.

	long			apiThreadBindList[MAX_CONS_THREADS];	// CPU ID list for threads that handle connections.  See -apiThreads.

	EmaString		consumerName;				// Name of the Consumer component in EmaConfig.xml. See -consumerName.

// APIQA:
	EmaString		proxyHostName;				// Proxy host name. See -ph.
	EmaString		proxyPort;					// Proxy port. See -pp.
	EmaString		proxyLogin;					// Proxy user name. See -plogin.
	EmaString		proxyPassword;				// Proxy password. See -ppasswd.
	EmaString		proxyDomain;				// Proxy domain. See -pdomain.
// END APIQA

	enum WebsocketProtocol
	{
		NoWebSocketEnum,
		WebSocketJSONEnum,
		WebSocketRWFEnum
	} websocketProtocol;
};

#endif // _CONS_PERF_CONFIG_H
