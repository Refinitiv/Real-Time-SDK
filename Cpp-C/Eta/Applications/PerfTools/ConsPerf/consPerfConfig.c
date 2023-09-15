/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2022 Refinitiv. All rights reserved.
*/

/* This provides handling for command-line configuration of ConsPerf. */

#include "consPerfConfig.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static RsslInt32 defaultThreadCount = 1;

/* Contains the global application configuration */
ConsPerfConfig consPerfConfig;

static void clearConsPerfConfig()
{
	/* Prepare defaults. */
	consPerfConfig.steadyStateTime = 300;
	consPerfConfig.delaySteadyStateCalc = 0;
	consPerfConfig.threadCount = defaultThreadCount;
	consPerfConfig.threadBindList[0][0] = '\0';
	consPerfConfig.threadReactorBindList[0][0] = '\0';

	snprintf(consPerfConfig.summaryFilename, sizeof(consPerfConfig.summaryFilename), "ConsSummary.out");
	snprintf(consPerfConfig.statsFilename, sizeof(consPerfConfig.statsFilename), "ConsStats");
	consPerfConfig.writeStatsInterval = 5;
	consPerfConfig.displayStats = RSSL_TRUE;
	consPerfConfig.logLatencyToFile = RSSL_FALSE;
	consPerfConfig.latencyIncludeJSONConversion = RSSL_FALSE;

	consPerfConfig.sendBufSize = 0;
	consPerfConfig.recvBufSize = 0;
	consPerfConfig.highWaterMark = 0;
	consPerfConfig.tcpNoDelay = RSSL_TRUE;
	consPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	consPerfConfig.guaranteedOutputBuffers = 5000;
	consPerfConfig.numInputBuffers = 15;
	snprintf(consPerfConfig.interfaceName, sizeof(consPerfConfig.interfaceName), "%s", "");
	snprintf(consPerfConfig.hostName, sizeof(consPerfConfig.hostName), "localhost");
	snprintf(consPerfConfig.portNo, sizeof(consPerfConfig.portNo), "14002");
	snprintf(consPerfConfig.serviceName, sizeof(consPerfConfig.serviceName), "DIRECT_FEED");
	snprintf(consPerfConfig.username, sizeof(consPerfConfig.username), "%s", "");

	consPerfConfig.ticksPerSec = 1000;
	consPerfConfig.itemRequestCount = 100000;
	consPerfConfig.commonItemCount = 0;
	consPerfConfig.itemRequestsPerSec = 500000;
	consPerfConfig.requestSnapshots = RSSL_FALSE;

	consPerfConfig.nanoTime = RSSL_FALSE;

	consPerfConfig.postsPerSec = 0;
	consPerfConfig.latencyPostsPerSec = 0;
	consPerfConfig.genMsgsPerSec = 0;
	consPerfConfig.latencyGenMsgsPerSec = 0;

	snprintf(consPerfConfig.itemFilename, sizeof(consPerfConfig.itemFilename), "%s", "350k.xml");
	snprintf(consPerfConfig.msgFilename, sizeof(consPerfConfig.msgFilename), "%s", "MsgData.xml");

	snprintf(consPerfConfig.caStore, sizeof(consPerfConfig.caStore), "%s", "");
	consPerfConfig.tlsProtocolFlags = 0;

	snprintf(consPerfConfig.protocolList, sizeof(consPerfConfig.protocolList), "%s", "");

	consPerfConfig.tunnelMessagingEnabled = RSSL_FALSE;
	snprintf(consPerfConfig.tunnelStreamServiceName, sizeof(consPerfConfig.tunnelStreamServiceName), "%s", "");
	consPerfConfig.tunnelUseAuthentication = RSSL_FALSE;
	consPerfConfig.tunnelDomainType = RSSL_DMT_SYSTEM;
	consPerfConfig.guaranteedOutputTunnelBuffers = 15000;
	consPerfConfig.tunnelStreamBufsUsed = RSSL_FALSE;
	consPerfConfig.compressionType = 0;

	snprintf(consPerfConfig.startingHostName, sizeof(consPerfConfig.startingHostName), "%s", "");
	snprintf(consPerfConfig.startingPort, sizeof(consPerfConfig.startingPort), "%s", "");
	snprintf(consPerfConfig.standbyHostName, sizeof(consPerfConfig.standbyHostName), "%s", "");
	snprintf(consPerfConfig.standbyPort, sizeof(consPerfConfig.standbyPort), "%s", "");
	consPerfConfig.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	consPerfConfig.convertJSON = RSSL_FALSE;
	consPerfConfig.jsonAllocatorSize = 64 * 1024;
}

void exitConfigError(char **argv)
{
	printf("Run '%s -?' to see usage.\n\n", argv[0]);
	exit(-1);
}

void exitMissingArgument(char **argv, int arg)
{
	printf("Config error: %s missing argument.\n"
			"Run '%s -?' to see usage.\n\n", argv[arg], argv[0]);
	exit(-1);
}


void initConsPerfConfig(int argc, char **argv)
{
	int i;
	int iargs;
	int consThreadCount = 0;

	clearConsPerfConfig();

	iargs = 1;

	while(iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			exitWithUsage();
		}
		else if(strcmp("-uname", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs++], "%s", consPerfConfig.username);
		}
		else if(strcmp("-h", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs++], "%s", consPerfConfig.hostName);
		}
		else if(strcmp("-p", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs++], "%s", consPerfConfig.portNo);
		}
		else if(strcmp("-if", argv[iargs]) == 0 || strcmp("-i", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs++], "%s", consPerfConfig.interfaceName);
		}
		else if (0 == strcmp("-threads", argv[iargs]))
		{
			char *pToken;

			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			consPerfConfig.threadCount = 0;

			pToken = strtok(argv[iargs++], ",");
			while(pToken)
			{
				if (++consPerfConfig.threadCount > MAX_CONS_THREADS)
				{
					printf("Config Error: Too many threads specified.\n");
					exit(-1);
				}

				snprintf(&(consPerfConfig.threadBindList[consPerfConfig.threadCount - 1][0]), MAX_LEN_CPUCOREBIND, "%s", pToken);

				pToken = strtok(NULL, ",");
			}

			if (consThreadCount > 0 && consPerfConfig.threadCount != consThreadCount)
			{
				printf("Config Error: thread count not equal to reactor thread count.\n");
				exit(-1);
			}
			if (consPerfConfig.threadCount < MAX_CONS_THREADS)
			{
				for (i = consPerfConfig.threadCount; i < MAX_CONS_THREADS; ++i)
				{
					consPerfConfig.threadBindList[i][0] = '\0';
				}
			}
			consThreadCount = (consPerfConfig.threadCount > 0) ? consPerfConfig.threadCount : 0;
		}
		else if (0 == strcmp("-workerThreads", argv[iargs]))
		{
			char* pToken;

			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			consPerfConfig.threadCount = 0;

			pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++consPerfConfig.threadCount > MAX_CONS_THREADS)
				{
					printf("Config Error: Too many reactor threads specified.\n");
					exit(-1);
				}

				snprintf(&(consPerfConfig.threadReactorBindList[consPerfConfig.threadCount - 1][0]), MAX_LEN_CPUCOREBIND, "%s", pToken);

				pToken = strtok(NULL, ",");
			}

			if (consThreadCount > 0 && consPerfConfig.threadCount != consThreadCount)
			{
				printf("Config Error: reactor thread count not equal to thread count.\n");
				exit(-1);
			}
			if (consPerfConfig.threadCount < MAX_CONS_THREADS)
			{
				for (i = consPerfConfig.threadCount; i < MAX_CONS_THREADS; ++i)
				{
					consPerfConfig.threadReactorBindList[i][0] = '\0';
				}
			}
			consThreadCount = (consPerfConfig.threadCount > 0) ? consPerfConfig.threadCount : 0;
		}
		else if(strcmp("-tcpDelay", argv[iargs]) == 0)
		{
			++iargs;
			consPerfConfig.tcpNoDelay = RSSL_FALSE;
		}
		else if(strcmp("-serviceName", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.serviceName, 128, "%s", argv[iargs++]);
		}
		else if(strcmp("-itemCount", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.itemRequestCount = atoi(argv[iargs++]);
		}
		else if(strcmp("-commonItemCount", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.commonItemCount = atoi(argv[iargs++]);
		}
		else if (strcmp("-itemFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.itemFilename, sizeof(consPerfConfig.itemFilename), "%s", argv[iargs++]);
		}
		else if (strcmp("-msgFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.msgFilename, sizeof(consPerfConfig.msgFilename), "%s", argv[iargs++]);
		}
		else if (strcmp("-latencyFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.logLatencyToFile = RSSL_TRUE;
			snprintf(consPerfConfig.latencyLogFilename, sizeof(consPerfConfig.latencyLogFilename), "%s", argv[iargs++]);
		}
		else if (strcmp("-summaryFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.summaryFilename, sizeof(consPerfConfig.summaryFilename), "%s", argv[iargs++]);
		}
		else if (strcmp("-statsFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.statsFilename, sizeof(consPerfConfig.statsFilename), "%s", argv[iargs++]);
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs++], "%u", &consPerfConfig.writeStatsInterval);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			++iargs;
			consPerfConfig.displayStats = RSSL_FALSE;
		}
		else if(strcmp("-connType", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if(strcmp("socket", argv[iargs]) == 0)
				consPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
			else if(strcmp("http", argv[iargs]) == 0)
				consPerfConfig.connectionType = RSSL_CONN_TYPE_HTTP;
			else if(strcmp("encrypted", argv[iargs]) == 0)
				consPerfConfig.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			else if((strcmp("websocket", argv[iargs]) == 0) || (strcmp("webSocket", argv[iargs]) == 0) || (strcmp("WebSocket", argv[iargs]) == 0))
				consPerfConfig.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
			else
			{
				printf("Config Error: Unknown connection type \"%s\"\n", argv[iargs]);
				exitConfigError(argv);
			}

			iargs++;
		}
		else if (strcmp("-encryptedConnType", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (strcmp("socket", argv[iargs]) == 0)
				consPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_SOCKET;
			else if (strcmp("websocket", argv[iargs]) == 0)
				consPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_WEBSOCKET;
			else if (strcmp("http", argv[iargs]) == 0)
			{
#ifdef Linux  
				printf("Config Error: Encrypted HTTP connection type not supported on Linux \"%s\".\n", argv[iargs]);
				exitConfigError(argv);
#else // HTTP connnections spported only through Windows WinInet 
				consPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_HTTP;
#endif
			}
			else
			{
				printf("Config Error: Unknown encrypted connection type \"%s\"\n", argv[iargs]);
				exitConfigError(argv);
			}

			iargs++;
		}
		else if(strcmp("-outputBufs", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.guaranteedOutputBuffers = atoi(argv[iargs++]);
		}
		else if(strcmp("-inputBufs", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.numInputBuffers = atoi(argv[iargs++]);
		}
		else if (0 == strcmp("-sendBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.sendBufSize = atoi(argv[iargs++]);
		}
		else if (0 == strcmp("-recvBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.recvBufSize = atoi(argv[iargs++]);
		}
		else if(strcmp("-steadyStateTime", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.steadyStateTime = atoi(argv[iargs++]);
		}
		else if (strcmp("-delaySteadyStateCalc", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.delaySteadyStateCalc = atoi(argv[iargs++]);
		}
		else if(strcmp("-requestRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.itemRequestsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-snapshot", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.requestSnapshots = RSSL_TRUE;
		}
		else if(strcmp("-nanoTime", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.nanoTime = RSSL_TRUE;
		}
		else if(strcmp("-measureDecode", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.measureDecode = RSSL_TRUE;
		}
		else if(strcmp("-postingRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.postsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-postingLatencyRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.latencyPostsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-genericMsgRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.genMsgsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-genericMsgLatencyRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.latencyGenMsgsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-tickRate", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.ticksPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-highWaterMark", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.highWaterMark = atoi(argv[iargs++]);
		}
		else if(strcmp("-reactor", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.useReactor = RSSL_TRUE;
		}
		else if(strcmp("-watchlist", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.useWatchlist = RSSL_TRUE;
		}
		else if (strcmp("-pl", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.protocolList, sizeof(consPerfConfig.protocolList), "%s", argv[iargs++]);
		}
		else if (strcmp("-castore", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.caStore, sizeof(consPerfConfig.caStore), "%s", argv[iargs++]);
		}
		else if (strcmp("-spTLSv1.2", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.tlsProtocolFlags |= RSSL_ENC_TLSV1_2;
		}
		else if (strcmp("-tunnel", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.tunnelMessagingEnabled = RSSL_TRUE;
		}
		else if (strcmp("-tunnelAuth", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.tunnelUseAuthentication = RSSL_TRUE;
		}
		else if (strcmp("-tunnelStreamOutputBufs", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.guaranteedOutputTunnelBuffers = atoi(argv[iargs++]);
		}
		else if (strcmp("-tunnelStreamBuffersUsed", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.tunnelStreamBufsUsed = RSSL_TRUE;
		}
		else if (strcmp("-calcRWFJSONConversionLatency", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.latencyIncludeJSONConversion = RSSL_TRUE;
		}
		else if (strcmp("-compressionType", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.compressionType = atoi(argv[iargs++]);
		}
		else if (strcmp("-startingHostName", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.startingHostName, 255, "%s", argv[iargs++]);
		}
		else if (strcmp("-startingPort", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.startingPort, 255, "%s", argv[iargs++]);
		}
		else if (strcmp("-standbyHostName", argv[iargs]) == 0)
		{
		++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
		snprintf(consPerfConfig.standbyHostName, 255, "%s", argv[iargs++]);
		}
		else if (strcmp("-standbyPort", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.standbyPort, 255, "%s", argv[iargs++]);
		}
		else if (strcmp("-warmStandbyMode", argv[iargs]) == 0)
		{
			char warmStandbyModeStr[255];
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(warmStandbyModeStr, 255, "%s", argv[iargs++]);

			if (0 == strcmp(&warmStandbyModeStr[0], "login"))
				consPerfConfig.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;
			else if (0 == strcmp(&warmStandbyModeStr[0], "service"))
				consPerfConfig.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;
			else
			{
				printf("Unknown warm standby mode specified: %s\n", warmStandbyModeStr);
				exitConfigError(argv);
			}
		}
		else if (strcmp("-addConversionOverhead", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.convertJSON = RSSL_TRUE;
		}
		else if (strcmp("-jsonAllocatorSize", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			consPerfConfig.jsonAllocatorSize = atoi(argv[iargs++]);
		}
		else
		{
			printf("Config Error: Unrecognized option: %s\n", argv[iargs]);
			exitConfigError(argv);
		}
	}

	if (consPerfConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
	} 

	if (consPerfConfig.itemRequestsPerSec < consPerfConfig.ticksPerSec)
	{
		printf("Config Error: Item Request Rate cannot be less than tick rate.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.postsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.postsPerSec != 0)
	{
		printf("Config Error: Post Rate cannot be less than tick rate(unless it is zero).\n");
		exitConfigError(argv);
	}
	if (consPerfConfig.genMsgsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.genMsgsPerSec != 0)
	{
		printf("Config Error: Generic Message Rate cannot be less than tick rate(unless it is zero).\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.latencyPostsPerSec > consPerfConfig.postsPerSec)
	{
		printf("Config Error: Latency Post Rate cannot be greater than total posting rate.\n");
		exitConfigError(argv);
	}
	if (consPerfConfig.latencyGenMsgsPerSec > consPerfConfig.genMsgsPerSec)
	{
		printf("Config Error: Latency Generic Message Rate cannot be greater than total generic message rate.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.latencyPostsPerSec > consPerfConfig.ticksPerSec)
	{
		printf("Config Error: Latency Post Rate cannot be greater than tick rate.\n");
		exitConfigError(argv);
	}
	if (consPerfConfig.latencyGenMsgsPerSec > consPerfConfig.ticksPerSec)
	{
		printf("Config Error: Latency Generic Message Rate cannot be greater than tick rate.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.postsPerSec && consPerfConfig.requestSnapshots)
	{
		printf("Config Error: Configured to post while requesting snapshots.\n");
		exitConfigError(argv);
	}
	if (consPerfConfig.genMsgsPerSec && consPerfConfig.requestSnapshots)
	{
		printf("Config Error: Configured to send generic messages while requesting snapshots.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.postsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.postsPerSec != 0)
	{
		printf("Config Error: Post Rate cannot be less than tick rate(unless it is zero).\n");
		exitConfigError(argv);
	}
	if (consPerfConfig.genMsgsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.genMsgsPerSec != 0)
	{
		printf("Config Error: Generic Message Rate cannot be less than tick rate(unless it is zero).\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.commonItemCount > consPerfConfig.itemRequestCount / consPerfConfig.threadCount)
	{
		printf("Config Error: Common item count (%d) is greater than total item count per thread (%d).\n",
				consPerfConfig.commonItemCount, 
				consPerfConfig.itemRequestCount / consPerfConfig.threadCount);
		exitConfigError(argv);
	}

	if (consPerfConfig.writeStatsInterval < 1)
	{
		printf("Config error: Write Stats Interval cannot be less than 1.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.tunnelMessagingEnabled == RSSL_TRUE && consPerfConfig.useReactor != RSSL_TRUE && consPerfConfig.useWatchlist != RSSL_TRUE)
	{
		printf("\nConfig error: Should add -reactor or -watchlist to create and use special tunnel streams.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.delaySteadyStateCalc < 0 || consPerfConfig.delaySteadyStateCalc > 30000)
	{
		printf("\nConfig error: Time before the latency is calculated should not be less than 0 or greater than 30000.\n");
		exitConfigError(argv);
	}

	if ((consPerfConfig.delaySteadyStateCalc / 1000) > consPerfConfig.steadyStateTime)
	{
		printf("\nConfig Error: Time before the latency is calculated should be less than Steady State Time.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.latencyIncludeJSONConversion == RSSL_TRUE && (consPerfConfig.useReactor == RSSL_TRUE || consPerfConfig.useWatchlist == RSSL_TRUE))
	{
		printf("\nConfig error: Should not combine -calcRWFJSONConversionLatency and -reactor or -watchlist.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.convertJSON == RSSL_TRUE && (consPerfConfig.useReactor == RSSL_TRUE || consPerfConfig.useWatchlist == RSSL_TRUE))
	{
		printf("\nConfig error: Should not combine -addConversionOverhead and -reactor or -watchlist.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.latencyIncludeJSONConversion == RSSL_TRUE
		&& !(consPerfConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET
			|| consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && consPerfConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET))
	{
		printf("\nConfig error: -calcRWFJSONConversionLatency enables for WebSocket connection only.\n");
		exitConfigError(argv);
	}

	if (consPerfConfig.convertJSON == RSSL_TRUE
		&& !(consPerfConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET
			|| consPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && consPerfConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET))
	{
		printf("\nConfig error: -addConversionOverhead enables for WebSocket connection only.\n");
		exitConfigError(argv);
	}

	/* Checks whether the warm standby feature is enabled. */
	if (consPerfConfig.startingHostName[0] != '\0' && consPerfConfig.startingPort[0] != '\0' && consPerfConfig.standbyHostName[0] != '\0' && consPerfConfig.standbyPort[0] != '\0')
	{
		if (consPerfConfig.useWatchlist == RSSL_FALSE)
		{
			printf("\nConfig error: -watchlist must be specified in order to use the warm standby feature.\n");
			exitConfigError(argv);
		}
	}
	else
	{
		consPerfConfig.warmStandbyMode = RSSL_RWSB_MODE_NONE;
	}

	consPerfConfig._requestsPerTick = consPerfConfig.itemRequestsPerSec 
		/ consPerfConfig.ticksPerSec;

	consPerfConfig._requestsPerTickRemainder = consPerfConfig.itemRequestsPerSec 
		% consPerfConfig.ticksPerSec;

	/* If service not specified for tunnel stream, use the service given for other items instead. */
	if (consPerfConfig.tunnelMessagingEnabled == RSSL_TRUE && consPerfConfig.tunnelStreamServiceName[0] == '\0')
	{
		snprintf(consPerfConfig.tunnelStreamServiceName, sizeof(consPerfConfig.tunnelStreamServiceName), "%s", consPerfConfig.serviceName);
	}
}


static const char *connectionTypeToString(RsslConnectionTypes connType)
{
	switch(connType)
	{
		case RSSL_CONN_TYPE_SOCKET:
			return "socket";
		case RSSL_CONN_TYPE_HTTP:
			return "http";
		case RSSL_CONN_TYPE_ENCRYPTED:
			return "encrypted";
		case RSSL_CONN_TYPE_WEBSOCKET:
			return "websocket";
		default:
			return "unknown";
	}

}

#define MAXLEN_THREAD_STR 768

/* Print the parsed configuration options(so the user can verify intended usage) */
void printConsPerfConfig(FILE *file)
{
	int i;
	int tmpStringPos = 0;
	char tmpString[MAXLEN_THREAD_STR];
	char reactorWatchlistUsageString[32];
	char warmStandbyModeStr[32];

	if (consPerfConfig.useWatchlist)
	{
		strcpy(reactorWatchlistUsageString, "Reactor and Watchlist");
	}
	else if (consPerfConfig.useReactor)
	{
		strcpy(reactorWatchlistUsageString, "Reactor");
	}
	else
	{
		strcpy(reactorWatchlistUsageString, "None");
	}

	if (consPerfConfig.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		strcpy(warmStandbyModeStr, "Login");
	}
	else if (consPerfConfig.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		strcpy(warmStandbyModeStr, "Service");
	}
	else
	{
		strcpy(warmStandbyModeStr, "None");
	}

	/* Build thread list */
	tmpStringPos += snprintf(tmpString, MAXLEN_THREAD_STR, "%s",
		(consPerfConfig.threadBindList[0][0] != '\0' ? consPerfConfig.threadBindList[0] : "(non)"));
	for(i = 1; i < consPerfConfig.threadCount && tmpStringPos < MAXLEN_THREAD_STR; ++i)
		tmpStringPos += snprintf(tmpString + tmpStringPos, MAXLEN_THREAD_STR - tmpStringPos, ",%s", consPerfConfig.threadBindList[i]);

	if (consPerfConfig.useReactor || consPerfConfig.useWatchlist)
	{
		tmpStringPos += snprintf(tmpString + tmpStringPos, MAXLEN_THREAD_STR - tmpStringPos, " {%s",
			(consPerfConfig.threadReactorBindList[0][0] != '\0' ? consPerfConfig.threadReactorBindList[0] : "(non)"));
		if (consPerfConfig.threadReactorBindList[0][0] != '\0')
		{
			for (i = 1; i < consPerfConfig.threadCount && tmpStringPos < MAXLEN_THREAD_STR; ++i)
				tmpStringPos += snprintf(tmpString + tmpStringPos, MAXLEN_THREAD_STR - tmpStringPos, ",%s", consPerfConfig.threadReactorBindList[i]);
		}
		if (tmpStringPos < MAXLEN_THREAD_STR)
			tmpStringPos += snprintf(tmpString + tmpStringPos, MAXLEN_THREAD_STR - tmpStringPos, "}");
	}

	fprintf(file, "--- TEST INPUTS ---\n\n");

	fprintf(file,
		"       Steady State Time: %u\n"
		" Delay Steady State Time: %u\n"
		"         Connection Type: %s\n",
		consPerfConfig.steadyStateTime,
		consPerfConfig.delaySteadyStateCalc,
		connectionTypeToString(consPerfConfig.connectionType));

	fprintf(file,
		"                Hostname: %s\n"
		"                    Port: %s\n"
		"     Enable warm standby: %s\n"
		"       Warm standby mode: %s\n"
		"Starting server HostName: %s\n"
		"    Starting server Port: %s\n"
		" Standby server HostName: %s\n"
		"     Standby server Port: %s\n"
		"                 Service: %s\n"
		"             Thread List: %s\n"
		"          Output Buffers: %u\n"
		"           Input Buffers: %u\n"
		"        Send Buffer Size: %u%s\n"
		"        Recv Buffer Size: %u%s\n"
		"         High Water Mark: %u%s\n"
		"          Interface Name: %s\n"
		"             Tcp_NoDelay: %s\n"
		"                Username: %s\n"
		"              Item Count: %d\n"
		"       Common Item Count: %d\n"
		"            Request Rate: %d\n"
		"       Request Snapshots: %s\n"
		"            Posting Rate: %d\n"
		"    Latency Posting Rate: %d\n"
		"        Generic Msg Rate: %d\n"
		"Latency Generic Msg Rate: %d\n"
		"               Item File: %s\n"
		"               Data File: %s\n"
		"            Summary File: %s\n"
		"              Stats File: %s\n"
		"        Latency Log File: %s\n"
		"  Latency Show JSON Conv: %s\n"
		"      Use JSON Converter: %s\n"
		"     JSON Allocator Size: %u\n"
		"               Tick Rate: %u\n"
		" Reactor/Watchlist Usage: %s\n"
		"       CA store location: %s\n"
		"      TLS Protocol flags: %i\n"
		"        WS Protocol List: %s\n"
		"          Tunnel Enabled: %s\n"
		"   Tunnel Authentication: %s\n"
		"   Output Tunnel Buffers: %u\n"
		" Print Usage Tunnel Bufs: %s\n"
		"        Compression type: %u\n"
		,
		consPerfConfig.hostName,
		consPerfConfig.portNo,
		consPerfConfig.warmStandbyMode == RSSL_RWSB_MODE_NONE ? "No" : "Yes",
		warmStandbyModeStr,
		consPerfConfig.startingHostName,
		consPerfConfig.startingPort,
		consPerfConfig.standbyHostName,
		consPerfConfig.standbyPort,
		consPerfConfig.serviceName,
		tmpString,
		consPerfConfig.guaranteedOutputBuffers,
		consPerfConfig.numInputBuffers,
		consPerfConfig.sendBufSize, (consPerfConfig.sendBufSize ? " bytes" : "(use default)"),
		consPerfConfig.recvBufSize, (consPerfConfig.recvBufSize ? " bytes" : "(use default)"),
		consPerfConfig.highWaterMark, (consPerfConfig.highWaterMark > 0 ? " bytes" : "(use default)"),
		strlen(consPerfConfig.interfaceName) ? consPerfConfig.interfaceName : "(use default)",
		(consPerfConfig.tcpNoDelay ? "Yes" : "No"),
		strlen(consPerfConfig.username) ? consPerfConfig.username : "(use system login name)",
		consPerfConfig.itemRequestCount,
		consPerfConfig.commonItemCount,
		consPerfConfig.itemRequestsPerSec,
		consPerfConfig.requestSnapshots ? "Yes" : "No",
		consPerfConfig.postsPerSec,
		consPerfConfig.latencyPostsPerSec,
		consPerfConfig.genMsgsPerSec,
		consPerfConfig.latencyGenMsgsPerSec,
		consPerfConfig.itemFilename,
		consPerfConfig.msgFilename,
		consPerfConfig.summaryFilename,
		consPerfConfig.statsFilename,
		consPerfConfig.logLatencyToFile ? consPerfConfig.latencyLogFilename : "(none)",
		(consPerfConfig.latencyIncludeJSONConversion ? "Yes" : "No"),
		(consPerfConfig.convertJSON ? "Yes" : "No"),
		consPerfConfig.jsonAllocatorSize,
		consPerfConfig.ticksPerSec,
		reactorWatchlistUsageString,
		consPerfConfig.caStore,
		consPerfConfig.tlsProtocolFlags,
		consPerfConfig.protocolList,
		(consPerfConfig.tunnelMessagingEnabled ? "Yes" : "No"),
		(consPerfConfig.tunnelUseAuthentication ? "Yes" : "No"),
		consPerfConfig.guaranteedOutputTunnelBuffers,
		(consPerfConfig.tunnelStreamBufsUsed ? "Yes" : "No"),
		consPerfConfig.compressionType
	  );

	fprintf(file,
			"      Nanosecond Latency: %s\n"
			"          Measure Decode: %s\n\n",
			consPerfConfig.nanoTime ? "Yes" : "No",
			consPerfConfig.measureDecode ? "Yes" : "No"
		   );
}

void exitWithUsage()
{

	printf(	"Options:\n"
			"  -?                                    Shows this usage\n"
			"\n"
			"  -connType <type>                      Type of connection(\"socket\", \"websocket\", \"http\", \"encrypted\")\n"
			"  -encryptedConnType <type>             Encrypted connection protocol. Only used if the \"encrypted\" connection type is selected. \"http\" type is only supported on Windows. (\"socket\", \"websocket\", \"http\")\n"
			"  -h <hostname>                         Name of host to connect to\n"
			"  -p <port number>                      Port number to connect to\n"
			"  -if <interface name>                  Name of network interface to use\n"
			"  -pl \"<list>\"                         List of desired WS sub-protocols in order of preference(',' | white space delineated)\n"
			"\n"
			"  -outputBufs <count>                   Number of output buffers(configures guaranteedOutputBuffers in RsslConnectOptions)\n"
			"  -inputBufs <count>                    Number of input buffers(configures numInputBufs in RsslConnectOptions)\n"
			"  -tcpDelay                             Turns off tcp_nodelay in RsslConnectOptions, enabling Nagle's\n"
			"  -sendBufSize <size>                   System Send Buffer Size(configures sysSendBufSize in RsslConnectOptions)\n"
			"  -recvBufSize <size>                   System Receive Buffer Size(configures sysRecvBufSize in RsslConnectOptions)\n"
			"\n"
			"  -tickRate <ticks per second>          Ticks per second\n"
			"  -itemCount <count>                    Number of items to request\n"
			"  -commonItemCount <count>              Number of items common to all consumers, if using multiple connections.\n"
			"  -requestRate <items/sec>              Rate at which to request items\n"
			"  -snapshot                             Snapshot test; request all items as non-streaming\n"
			"  -postingRate <posts/sec>              Rate at which to send post messages.\n"
			"  -postingLatencyRate <posts/sec>       Rate at which to send latency post messages.\n"
			"  -genericMsgRate <genMsgs/sec>         Rate at which to send generic messages.\n"
			"  -genericMsgLatencyRate <genMsgs/sec>  Rate at which to send latency generic messages.\n"
			"  -calcRWFJSONConversionLatency         Enable calculation of time which spent on rwf-json conversion for WebSocket Transport + RWF.\n"
			"  -addConversionOverhead                Enable rwf-json conversion for WebSocket Transport + RWF. This setting is disabled by default.\n"
			"  -jsonAllocatorSize                    Size of cJSON custom allocator buffer.\n"
			"\n"
			"  -uname <name>                         Username to use in login request\n"
			"  -serviceName <name>                   Service Name\n"
			"\n"
			"  -itemFile <file name>                 Name of the file to get item names from\n"
			"  -msgFile <file name>                  Name of the file that specifies the data content in messages\n"
			"  -summaryFile <filename>               Name of file for logging summary info.\n"
			"  -statsFile <filename>                 Base name of file for logging periodic statistics.\n"
			"  -writeStatsInterval <sec>             Controls how often stats are written to the file.\n"
			"  -noDisplayStats                       Stop printout of stats to screen.\n"
			"  -latencyFile <filename>               Base name of file for logging latency.\n"
			"\n"
			"  -steadyStateTime <seconds>            Time consumer will run the steady-state portion of the test.\n"
			"                                         Also used as a timeout during the startup-state portion.\n"
			"  -delaySteadyStateCalc <mili sec>      Time consumer will wait before calculate the latency.\n"
			"  -threads <thread list>                list of threads(which create 1 connection each),\n"
			"                                        by their bound CPU. Comma-separated list. -1 means do not bind.\n"
			"                                         (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n"
			"  -workerThreads <thread list>          list of CPU bound for Reactor worker threads. -1 means do not bind.\n"
			"  -reactor                              Use the VA Reactor instead of the ETA Channel for sending and receiving.\n"
			"  -watchlist                            Use the VA Reactor watchlist instead of the ETA Channel for sending and receiving.\n"
			"\n"
			"  -nanoTime                             Assume latency has nanosecond precision instead of microsecond.\n"
			"  -measureDecode                        Measure decode time of updates.\n"
			"\n"
			"  -castore                              File location of the certificate authority store.\n"
			"  -spTLSv1.2                            Specifies that TLSv1.2 can be used for an OpenSSL-based encrypted connection\n"
			"\n"
			"  -tunnel                               Causes the consumer to open a tunnel stream that exchanges basic messages. Require using -reactor or -watchlist.\n"
			"  -tunnelAuth                           Causes the consumer to enable authentication when opening tunnel streams.\n"
			"  -tunnelStreamOutputBufs <count>       Number of output tunnel buffers (configures guaranteedOutputBuffers in RsslTunnelStreamOpenOptions).\n"
			"  -tunnelStreamBuffersUsed              Print stats of buffers used by tunnel stream. This setting is disabled by default.\n"
			"  -compressionType <compression type>   Specify a compression type (configures compressionType in RsslConnectOptions).\n"
			"\n"
			"  -startingHostName <Starting hostname> Specify a starting server hostname for enabling warm standby feature.\n"
			"  -startingPort <Starting port>         Specify a starting server port for enabling warm standby feature.\n"
			"  -standbyHostName <Standby hostname>   Specify a standby server hostname for enabling warm standby feature.\n"
			"  -standbyPort <Standby port>           Specify a standby server port for enabling warm standby feature.\n"
			"  -warmStandbyMode <login/service>      Specify a warm standby mode. Defaults to login based.\n"
			"\n"
	);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}
