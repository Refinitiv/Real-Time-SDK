/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2021 Refinitiv. All rights reserved.
*/

/* consPerfConfig.h
 * Configures the ConsPerf application. */

#ifndef _CONS_PERF_CONFIG_H
#define _CONS_PERF_CONFIG_H

#define MAX_CONS_THREADS 8
#define MAX_LEN_CPUCOREBIND 16

#include "rtr/rsslTypes.h"
#include "rtr/rsslTransport.h"
#include "perfTunnelMsgHandler.h"

#ifdef __cplusplus
extern "C" {
#endif



/* Provides configuration options for the consumer. */
typedef struct 
{
	RsslUInt32	steadyStateTime;					/* Time application runs befor exiting.  See -steadyStateTime */
	RsslUInt32	delaySteadyStateCalc;				/* Time before the latency is calculated. See -delaySteadyStateCalc */
	RsslInt32	ticksPerSec;						/* Main loop ticks per second.  See -tps */
	RsslInt32	threadCount;						/* Number of threads that handle connections.  See -threads */
	char		threadBindList[MAX_CONS_THREADS][MAX_LEN_CPUCOREBIND];	/* CPU ID list for threads that handle connections.  See -threads */
	char		threadReactorBindList[MAX_CONS_THREADS][MAX_LEN_CPUCOREBIND];	/* CPU ID list for Reactor worker threads.  See -workerThreads */

	char		itemFilename[128];					/* File of names to use when requesting items. See -itemFile. */
	char		msgFilename[128];					/* File of data to use for message payloads. See -msgFile. */

	RsslBool	logLatencyToFile;					/* Whether to log update latency information to a file. See -latencyFile. */
	char		latencyLogFilename[128];			/* Name of the latency log file. See -latencyFile. */
	char		summaryFilename[128];				/* Name of the summary log file. See -summaryFile. */
	char		statsFilename[128];					/* Name of the statistics log file. See -statsFile. */
	RsslUInt32	writeStatsInterval;					/* Controls how often statistics are written. */
	RsslBool	displayStats;						/* Controls whether stats appear on the screen. */
	RsslBool	latencyIncludeJSONConversion;		/* All the latencies calculation should(true) or should not(false) include the time of Rwf to JSON conversion. */

	RsslInt32 itemRequestsPerSec;					/* Rate at which the consumer will send out item requests. See -rqps. */

	RsslConnectionTypes	connectionType;				/* Type of connection. See -connType */
	RsslConnectionTypes	encryptedConnectionType;	/* Cncrypted connection protocol type. See -encryptedConnType */
	char				hostName[128];				/* hostName, if using rsslConnect(). See -hostname */
	char 				portNo[32];					/* Port number. See -p */
	char				interfaceName[128];			/* Name of interface.  See -if */
	RsslUInt32			guaranteedOutputBuffers;	/* Guaranteed Output Buffers. See -outputBufs */
	RsslUInt32			numInputBuffers;			/* Input Buffers. See -inputBufs */
	RsslUInt32			sendBufSize;				/* System Send Buffer Size(-sendBufSize) */
	RsslUInt32			recvBufSize;				/* System Send Buffer Size(-recvBufSize) */
	RsslUInt32			highWaterMark;				/* sets the point which will cause ETA to automatically flush */
	RsslBool			tcpNoDelay;					/* Enable/Disable Nagle's algorithm. See -tcpDelay */
	RsslBool			requestSnapshots;			/* Whether to request all items as snapshots. See -snapshot */

	char				username[128];				/* Username used when logging in. */
// APIQA:
	RsslBool			hostNameProvided;			/* Indicates hostName or Port number option is provided */
	RsslBool			enableSessionMgnt;			/* Enables the session management to keep the session alive. See -sessionMgnt */
	char				clientId[128];				/* Client Id used when logging in. */
	char				password[128];				/* Password used when logging in. */
	char				clientSecret[128];			/* Client Secret used when logging in. */
	char				location[128];				/* Location to get an endpoint from RDP Service discovery. see -l */
// END APIQA
	char				serviceName[128];			/* Name of service to request items from. See -s. */
	RsslInt32			itemRequestCount;			/* Number of items to request. See -itemCount. */
	RsslInt32			commonItemCount;			/* Number of items common to all connections, if
													 * using multiple connections. See -commonItemCount. */

	RsslInt32			postsPerSec;				/* Number of posts to send per second. See -postingRate. */
	RsslInt32			latencyPostsPerSec;			/* Number of latency posts to send per second. See -latPostingRate. */

	RsslInt32			genMsgsPerSec;				/* Number of generic messages to send per second. See -genMsgRate. */
	RsslInt32			latencyGenMsgsPerSec;		/* Number of latency generic messages to send per second. See -latGenMsgRate. */

	RsslBool			nanoTime;					/* Whether to assume latency is nanosecond precision instead of microsecond. */
	RsslBool			measureDecode;				/* Measure time to decode latency updates (-measureDecode) */

	RsslBool			useReactor;					/* Use the VA Reactor instead of the ETA Channel for sending and receiving. */
	RsslBool			useWatchlist;				/* Use the VA Reactor watchlist instead of the ETA Channel for sending and receiving. */

	char				caStore[255];				/* Certificate authority location */
	RsslUInt32			tlsProtocolFlags;			/* Flagset of TLS protocols */

	char				protocolList[256];			/* List of supported WebSocket sub-protocols */
	RsslInt32 _requestsPerTick;
	RsslInt32 _requestsPerTickRemainder;

	RsslBool			tunnelMessagingEnabled;		/* Whether to create tunnel for sending messages. -tunnel */
	char				tunnelStreamServiceName[128];	/* Service name requested by application. -tsServiceName */
	RsslBool			tunnelUseAuthentication;	/* Whether to use authentication when opening the tunnel stream. */
	RsslUInt8			tunnelDomainType;			/* DomainType to use when opening the tunnel stream. */
	RsslUInt32			guaranteedOutputTunnelBuffers;	/* Guaranteed Output Tunnel Buffers. See -tunnelStreamOutputBufs */
	RsslBool			tunnelStreamBufsUsed;		/* Control whether to print tunnel Stream buffers usage. See -tunnelStreamBuffersUsed */
	RsslUInt32			compressionType;			/* Compression types for the client. */
	/* Warm standby configuration settings. */
	char				startingHostName[255];		/* Specify a starting server hostname for warm standby feature. See -startingHostName.*/
	char				startingPort[255];			/* Specify a starting server port for warm standby feature. See -startingPort.*/
	char				standbyHostName[255];		/* Specify a standby server hostname for warm standby feature. See -standbyHostName.*/
	char				standbyPort[255];			/* Specify a standby server port for warm standby feature. See -standbyPort.*/
	RsslReactorWarmStandbyMode	warmStandbyMode;	/* Specify a warm standby mode. See -warmStandbyMode. */

// APIQA:
	char				proxyHost[255];				/* Proxy host name. See -ph */
	char				proxyPort[255];				/* Proxy port. See -pp */
	char				proxyLogin[255];			/* Proxy user name. See -plogin */
	char				proxyPassword[255];			/* Proxy password. See -ppasswd */
	char				proxyDomain[255];			/* Proxy domain. See -pdomain */
// END APIQA
} ConsPerfConfig;

/* Contains the global application configuration */
extern ConsPerfConfig consPerfConfig;

/* Parses command-line arguments to fill in the application's configuration structures. */
void initConsPerfConfig(int argc, char **argv);

/* Prints out the configuration. */
void printConsPerfConfig(FILE *file);

/* Exits the application and prints out usage information. */
void exitWithUsage();

#ifdef __cplusplus
};
#endif

#endif
