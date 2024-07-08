/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "providerThreads.h"
#include "directoryProvider.h"
#include "niProvPerfConfig.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static void clearNIProvPerfConfig()
{
	/* Prepare defaults. */
	niProvPerfConfig.runTime = 360;

	niProvPerfConfig.highWaterMark = 0;
	niProvPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	niProvPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_INIT;
	niProvPerfConfig.guaranteedOutputBuffers = 5000;
	niProvPerfConfig.sendBufSize = 0;
	niProvPerfConfig.recvBufSize = 0;
	snprintf(niProvPerfConfig.interfaceName, sizeof(niProvPerfConfig.interfaceName), "%s", "");
	snprintf(niProvPerfConfig.hostName, sizeof(niProvPerfConfig.hostName), "%s", "localhost");
	snprintf(niProvPerfConfig.portNo, sizeof(niProvPerfConfig.portNo), "%s", "14003");
	snprintf(niProvPerfConfig.summaryFilename, sizeof(niProvPerfConfig.summaryFilename), "NIProvSummary.out");
	snprintf(niProvPerfConfig.username, sizeof(niProvPerfConfig.username), "%s", "");
	niProvPerfConfig.writeStatsInterval = 5;
	niProvPerfConfig.displayStats = RSSL_TRUE;
	niProvPerfConfig.tcpNoDelay = RSSL_TRUE;
	snprintf(niProvPerfConfig.sendAddr, sizeof(niProvPerfConfig.sendAddr), "%s", "");
	snprintf(niProvPerfConfig.recvAddr, sizeof(niProvPerfConfig.recvAddr), "%s", "");
	snprintf(niProvPerfConfig.sendPort, sizeof(niProvPerfConfig.sendPort), "%s", "");
	snprintf(niProvPerfConfig.recvPort, sizeof(niProvPerfConfig.recvPort), "%s", "");
	snprintf(niProvPerfConfig.unicastPort, sizeof(niProvPerfConfig.unicastPort), "%s", "");
	niProvPerfConfig.sAddr = RSSL_FALSE;
	niProvPerfConfig.rAddr = RSSL_FALSE;

	niProvPerfConfig.itemPublishCount = 100000;
	niProvPerfConfig.commonItemCount = 0;

	snprintf(niProvPerfConfig.caStore, sizeof(niProvPerfConfig.caStore), "%s", "");
	niProvPerfConfig.tlsProtocolFlags = 0;
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

void initNIProvPerfConfig(int argc, char **argv)
{
	int i;
	int iargs;
	int provThreadCount = 0;

	clearNIProvPerfConfig();
	clearProviderThreadConfig(); 
	clearDirectoryConfig(&directoryConfig);

	/* Don't need to provide an open limit.  Our encode logic will skip the OpenLimit entry if it
	 * is set to zero. */
	directoryConfig.openLimit = 0;

	snprintf(providerThreadConfig.statsFilename, sizeof(providerThreadConfig.statsFilename), "NIProvStats");

	/* Go through the argument list, and fill in configuration structures as appropriate. */
	for(iargs = 1; iargs < argc; ++iargs)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			exitWithUsage();
		}

		else if (0 == strcmp("-runTime", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &niProvPerfConfig.runTime);
		}
		else if (strcmp("-summaryFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.summaryFilename, sizeof(niProvPerfConfig.summaryFilename), "%s", argv[iargs]);
		}
		else if (strcmp("-statsFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.statsFilename, sizeof(providerThreadConfig.statsFilename), "%s", argv[iargs]);
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &niProvPerfConfig.writeStatsInterval);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			niProvPerfConfig.displayStats = RSSL_FALSE;
		}
		else if (0 == strcmp("-threads", argv[iargs]))
		{
			char *pToken;

			providerThreadConfig.threadCount = 0;

			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
		   
			pToken = strtok(argv[iargs], ",");
			while(pToken)
			{
				if (++providerThreadConfig.threadCount > MAX_PROV_THREADS)
				{
					printf("Config Error: Too many threads specified.\n");
					exitConfigError(argv);
				}

				snprintf(&(providerThreadConfig.threadBindList[providerThreadConfig.threadCount - 1][0]), MAX_LEN_CPUCOREBIND, "%s", pToken);

				pToken = strtok(NULL, ",");
			}

			if (provThreadCount > 0 && providerThreadConfig.threadCount != provThreadCount)
			{
				printf("Config Error: thread count not equal to reactor thread count.\n");
				exit(-1);
			}
			if (providerThreadConfig.threadCount < MAX_PROV_THREADS)
			{
				for (i = providerThreadConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				{
					providerThreadConfig.threadBindList[i][0] = '\0';
				}
			}
			provThreadCount = (providerThreadConfig.threadCount > 0) ? providerThreadConfig.threadCount : 0;
		}
		else if (0 == strcmp("-workerThreads", argv[iargs]))
		{
			char* pToken;

			providerThreadConfig.threadCount = 0;

			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			pToken = strtok(argv[iargs], ",");
			while (pToken)
			{
				if (++providerThreadConfig.threadCount > MAX_PROV_THREADS)
				{
					printf("Config Error: Too many reactor threads specified.\n");
					exitConfigError(argv);
				}

				snprintf(&(providerThreadConfig.threadReactorWorkerBindList[providerThreadConfig.threadCount - 1][0]), MAX_LEN_CPUCOREBIND, "%s", pToken);

				pToken = strtok(NULL, ",");
			}

			if (provThreadCount > 0 && providerThreadConfig.threadCount != provThreadCount)
			{
				printf("Config Error: reactor thread count not equal to thread count.\n");
				exit(-1);
			}
			if (providerThreadConfig.threadCount < MAX_PROV_THREADS)
			{
				for (i = providerThreadConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				{
					providerThreadConfig.threadReactorWorkerBindList[i][0] = '\0';
				}
			}
			provThreadCount = (providerThreadConfig.threadCount > 0) ? providerThreadConfig.threadCount : 0;
		}
		else if (0 == strcmp("-connType", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (0 == strcmp(argv[iargs], "socket"))
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
			else if (0 == strcmp(argv[iargs], "reliableMCast"))
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
			else if (0 == strcmp(argv[iargs], "encrypted"))
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			else
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_INIT; /* error */
		}
		else if (strcmp("-encryptedConnType", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (strcmp("socket", argv[iargs]) == 0)
				niProvPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_SOCKET;
			else if (strcmp("http", argv[iargs]) == 0)
			{
#ifdef Linux  
				printf("Config Error: Encrypted HTTP connection type not supported on Linux \"%s\".\n", argv[iargs]);
				exitConfigError(argv);
#else // HTTP connnections spported only through Windows WinInet 
				niProvPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_HTTP;
#endif
			}
			else
			{
				printf("Config Error: Unknown encrypted connection type \"%s\"\n", argv[iargs]);
				exitConfigError(argv);
			}
		}
		else if (0 == strcmp("-outputBufs", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &niProvPerfConfig.guaranteedOutputBuffers);
		}
		else if (0 == strcmp("-sendBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &niProvPerfConfig.sendBufSize);
		}
		else if (0 == strcmp("-recvBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &niProvPerfConfig.recvBufSize);
		}
		else if (0 == strcmp("-if", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.interfaceName, sizeof(niProvPerfConfig.interfaceName), "%s", argv[iargs]);
		}
		else if(strcmp("-uname", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%s", niProvPerfConfig.username);
		}
		else if (0 == strcmp("-p", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.portNo, sizeof(niProvPerfConfig.portNo), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-rp", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.recvPort, sizeof(niProvPerfConfig.recvPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-sp", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.sendPort, sizeof(niProvPerfConfig.sendPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-u", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.unicastPort, sizeof(niProvPerfConfig.unicastPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-h", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.hostName, sizeof(niProvPerfConfig.hostName), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-ra", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.recvAddr, sizeof(niProvPerfConfig.recvAddr), "%s", argv[iargs]);
			niProvPerfConfig.rAddr = RSSL_TRUE;
		}
		else if (0 == strcmp("-sa", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.sendAddr, sizeof(niProvPerfConfig.sendAddr), "%s", argv[iargs]);
			niProvPerfConfig.sAddr = RSSL_TRUE;
		}
		else if (0 == strcmp("-tcpDelay", argv[iargs]))
		{
			niProvPerfConfig.tcpNoDelay = RSSL_FALSE;
		}
		else if (0 == strcmp("-serviceName", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(directoryConfig.serviceName, sizeof(directoryConfig.serviceName), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-serviceId", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%llu", &directoryConfig.serviceId);
		}
		/* Perf Test configuration */
		else if (0 == strcmp("-itemFile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.itemFilename, sizeof(providerThreadConfig.itemFilename), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-msgFile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.msgFilename, sizeof(providerThreadConfig.msgFilename), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-itemCount", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &niProvPerfConfig.itemPublishCount);
		}
		else if(strcmp("-commonItemCount", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			/* add item count in market price handler */
			sscanf(argv[iargs], "%d", &niProvPerfConfig.commonItemCount);
		}
		else if (0 == strcmp("-updateRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.updatesPerSec);
		}
		else if (0 == strcmp("-refreshBurstSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.refreshBurstSize);
		}

		else if (0 == strcmp("-latencyUpdateRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			if (0 == strcmp("all", argv[iargs]))
				providerThreadConfig.latencyUpdatesPerSec = ALWAYS_SEND_LATENCY_UPDATE;
			else
				sscanf(argv[iargs], "%d", &providerThreadConfig.latencyUpdatesPerSec);
		}
		else if (0 == strcmp("-tickRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.ticksPerSec);
		}
		else if (0 == strcmp("-maxPackCount", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.totalBuffersPerPack);
		}
		else if (0 == strcmp("-packBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.packingBufferLength);
		}
		else if (0 == strcmp("-preEnc", argv[iargs]))
		{
			providerThreadConfig.preEncItems = RSSL_TRUE;
		}
		else if (0 == strcmp("-mcastStats", argv[iargs]))
		{
			providerThreadConfig.takeMCastStats = RSSL_TRUE;
		}
		else if (0 == strcmp("-nanoTime", argv[iargs]))
		{
			providerThreadConfig.nanoTime = RSSL_TRUE;
		}
		else if (0 == strcmp("-measureEncode", argv[iargs]))
		{
			providerThreadConfig.measureEncode = RSSL_TRUE;
		}
		else if (0 == strcmp("-directWrite", argv[iargs]))
		{
			providerThreadConfig.writeFlags |= RSSL_WRITE_DIRECT_SOCKET_WRITE;
		}
		else if(strcmp("-highWaterMark", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &niProvPerfConfig.highWaterMark);
		}
		else if (0 == strcmp("-reactor", argv[iargs]))
		{
			niProvPerfConfig.useReactor = RSSL_TRUE;
		}
		else if (0 == strcmp("-castore", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(niProvPerfConfig.caStore, sizeof(niProvPerfConfig.caStore), "%s", argv[iargs]);
		}
		else
		{
			printf("Config Error: Unrecognized option: %s\n", argv[iargs]);
			exitConfigError(argv);
		}
	}

	/* Conditions */

	if (providerThreadConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
	} 

	if (niProvPerfConfig.connectionType == RSSL_CONN_TYPE_INIT)
	{
		printf("Config Error: Unknown connectionType. Valid types are \"socket\", \"reliableMCast\"\n");
		exitConfigError(argv);
	} 

	if (niProvPerfConfig.commonItemCount > niProvPerfConfig.itemPublishCount / providerThreadConfig.threadCount)
	{
		printf("Config Error: Common item count (%d) is greater than total item count per thread (%d).\n",
				niProvPerfConfig.commonItemCount, 
				niProvPerfConfig.itemPublishCount / providerThreadConfig.threadCount);
		exitConfigError(argv);
	}

	if (niProvPerfConfig.commonItemCount > niProvPerfConfig.itemPublishCount)
	{
		printf("Config Error: Common item count is greater than total item count.\n");
		exitConfigError(argv);
	}

	initDirectoryConfig();
	providerThreadConfigInit();
}

static const char *connectionTypeToString(RsslConnectionTypes connType)
{
	switch(connType)
	{
		case RSSL_CONN_TYPE_SOCKET:
			return "socket";
		case RSSL_CONN_TYPE_RELIABLE_MCAST:
			return "reliableMCast";
		case RSSL_CONN_TYPE_ENCRYPTED:
			return "encrypted";
		default:
			return "unknown";
	}

}

#define MAXLEN_THREAD_STR 768

void printNIProvPerfConfig(FILE *file)
{
	int i;
	int threadStringPos = 0;
	char threadString[MAXLEN_THREAD_STR];

	/* Build thread list */
	threadStringPos += snprintf(threadString, MAXLEN_THREAD_STR, "%s",
		(providerThreadConfig.threadBindList[0][0] != '\0' ? providerThreadConfig.threadBindList[0] : "(non)"));
	for (i = 1; i < providerThreadConfig.threadCount && threadStringPos < MAXLEN_THREAD_STR; ++i)
		threadStringPos += snprintf(threadString + threadStringPos, MAXLEN_THREAD_STR - threadStringPos, ",%s", providerThreadConfig.threadBindList[i]);

	if (niProvPerfConfig.useReactor)
	{
		threadStringPos += snprintf(threadString + threadStringPos, MAXLEN_THREAD_STR - threadStringPos, " {%s",
			(providerThreadConfig.threadReactorWorkerBindList[0][0] != '\0' ? providerThreadConfig.threadReactorWorkerBindList[0] : "(non)"));
		if (providerThreadConfig.threadReactorWorkerBindList[0][0] != '\0')
		{
			for (i = 1; i < providerThreadConfig.threadCount && threadStringPos < MAXLEN_THREAD_STR; ++i)
				threadStringPos += snprintf(threadString + threadStringPos, MAXLEN_THREAD_STR - threadStringPos, ",%s", providerThreadConfig.threadReactorWorkerBindList[i]);
		}
		if (threadStringPos < MAXLEN_THREAD_STR)
			threadStringPos += snprintf(threadString + threadStringPos, MAXLEN_THREAD_STR - threadStringPos, "}");
	}


	fprintf(file, "--- TEST INPUTS ---\n\n");

	fprintf(file,	
			"              Run Time: %u sec\n"
			"       Connection Type: %s\n",
			niProvPerfConfig.runTime,
			connectionTypeToString(niProvPerfConfig.connectionType));

	if(niProvPerfConfig.connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST && niProvPerfConfig.sAddr)
	{
		fprintf(file,  
				"        Multicast Conn: (send %s:%s, recv %s:%s, unicast %s)\n",
				niProvPerfConfig.sendAddr, niProvPerfConfig.sendPort,
				niProvPerfConfig.recvAddr, niProvPerfConfig.recvPort,
				niProvPerfConfig.unicastPort);
	}
	else
	{
		fprintf(file, 
				"              Hostname: %s\n"
				"                  Port: %s\n",
				niProvPerfConfig.hostName,
				niProvPerfConfig.portNo);
	}


	fprintf(file,	
			"           Thread List: %s\n"
			"        Output Buffers: %u\n"
			"      Send Buffer Size: %u%s\n"
			"      Recv Buffer Size: %u%s\n"
			"       High Water Mark: %u%s\n"
			"        Interface Name: %s\n"
			"              Username: %s\n"
			"           Tcp_NoDelay: %s\n"
			"            Item Count: %d\n"
			"     Common Item Count: %d\n"
			"             Tick Rate: %u\n"
			"     Use Direct Writes: %s\n"
			"          Summary File: %s\n"
			"            Stats File: %s\n"
			"  Write Stats Interval: %u\n"
			"         Display Stats: %s\n",
			threadString,
			niProvPerfConfig.guaranteedOutputBuffers,
			niProvPerfConfig.sendBufSize, (niProvPerfConfig.sendBufSize ? " bytes" : "(use default)"),
			niProvPerfConfig.recvBufSize, (niProvPerfConfig.recvBufSize ? " bytes" : "(use default)"),
			niProvPerfConfig.highWaterMark, (niProvPerfConfig.highWaterMark > 0 ?  " bytes" : "(use default)"),
			strlen(niProvPerfConfig.interfaceName) ? niProvPerfConfig.interfaceName : "(use default)",
			strlen(niProvPerfConfig.username) ? niProvPerfConfig.username : "(use system login name)",
			(niProvPerfConfig.tcpNoDelay ? "Yes" : "No"),
			niProvPerfConfig.itemPublishCount,
			niProvPerfConfig.commonItemCount,
			providerThreadConfig.ticksPerSec,
			(providerThreadConfig.writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) ? "Yes" : "No",
			niProvPerfConfig.summaryFilename,
			providerThreadConfig.statsFilename,
			niProvPerfConfig.writeStatsInterval,
			(niProvPerfConfig.displayStats ? "Yes" : "No")
		  );

	fprintf(file,
			"           Update Rate: %d\n"
			"   Latency Update Rate: %d\n"
			"    Refresh Burst Size: %d\n"
			"             Item File: %s\n"
			"             Data File: %s\n",

			providerThreadConfig.updatesPerSec,
			providerThreadConfig.latencyUpdatesPerSec >= 0 ? providerThreadConfig.latencyUpdatesPerSec : providerThreadConfig.updatesPerSec,
			providerThreadConfig.refreshBurstSize,
			providerThreadConfig.itemFilename,
			providerThreadConfig.msgFilename);

	if (providerThreadConfig.totalBuffersPerPack > 1)
		fprintf(file,
			"               Packing: Yes(max %d per pack, %u buffer size)\n",
			providerThreadConfig.totalBuffersPerPack,
			providerThreadConfig.packingBufferLength);
	else
		fprintf(file,
			"               Packing: No\n");


	fprintf(file,
			"            Service ID: %llu\n"
			"          Service Name: %s\n",
			directoryConfig.serviceId,
			directoryConfig.serviceName
		  );

	fprintf(file,
			"           Use Reactor: %s\n\n",
			(niProvPerfConfig.useReactor ? "Yes" : "No")
		  );

	fprintf(file,
			"  Pre-Encoded Updates: %s\n" 
			"      Nanosecond Time: %s\n" 
			"       Measure Encode: %s\n"
            "      Multicast Stats: %s\n\n",
			providerThreadConfig.preEncItems ? "Yes" : "No",
			providerThreadConfig.nanoTime ? "Yes" : "No",
			providerThreadConfig.measureEncode ? "Yes" : "No",
            providerThreadConfig.takeMCastStats ? "Yes" : "No");
}

void exitWithUsage()
{
	printf(	"Options:\n"
			"  -?                               Shows this usage\n"
			"  -connType <type>                 Type of connection(\"socket\", \"reliableMCast\", \"encrypted\")\n"
			"  -encryptedConnType <type>        Encrypted connection protocol. Only used if the \"encrypted\" connection type is selected. \"http\" type is only supported on Windows. (\"socket\", \"websocket\", \"http\")\n"
			"\n"
			"Connection options(for socket-based connections):\n"
			"  -h <hostname>                    Name of host to connect to\n"
			"  -p <port number>                 Port number\n"
			"\n"
			"Connection options(for segmented multicast connections):\n"
			"  -rp <port number>                Receive port\n"
			"  -sp <port number>                Send port\n"
			"  -u  <port number>                Unicast port\n"
			"  -ra <receive Address>            Receive Address\n"
			"  -sa <send Address>               Send Address\n"
			"\n"
			"  -outputBufs <count>              Number of output buffers(configures guaranteedOutputBuffers in RsslConnectOptions)\n"
			"  -sendBufSize <size>              System Send Buffer Size(configures sysSendBufSize in RsslConnectOptions)\n"
			"  -recvBufSize <size>              System Receive Buffer Size(configures sysRecvBufSize in RsslConnectOptions)\n"
			"  -if <interface name>             Name of network interface to use\n"
			"  -tcpDelay                        Turns off tcp_nodelay in RsslConnectOptions, enabling Nagle's\n"
			"\n"
			"  -tickRate <ticks/sec>            Ticks per second\n"
			"  -itemCount <count>               Number of items to publish\n"
			"  -commonItemCount <count>         Number of items common to all providers, if using multiple connections.\n"
			"  -updateRate <updates/sec>        Update rate per second\n"
			"  -latencyUpdateRate <updates/sec> Latency update rate(can specify \"all\" to send it in every update)\n"
			"  -maxPackCount <count>            Maximum number of messages packed in a buffer(when count > 1, rsslPackBuffer() is used)\n"
			"  -packBufSize <length>            If packing, sets size of buffer to use\n"
			"  -refreshBurstSize <count>        Number of refreshes to send in a burst(controls granularity of time-checking)\n"
			"  -directWrite                     Sets direct socket write flag when using rsslWrite()\n"
			"\n"
			"  -serviceName <name>              Service Name\n"
			"  -uname <name>                    Username to use in login request\n"
			"  -serviceId <ID>                  Service ID\n"
			"\n"
			"  -itemFile <file name>            Name of the file to get items from for publishing\n"
			"  -msgFile <file name>             Name of the file that specifies the data content in messages\n"
			"  -summaryFile <filename>          Name of file for logging summary info.\n"
			"  -statsFile <filename>            Base name of file for logging periodic statistics.\n"
			"  -writeStatsInterval <sec>        Controls how often stats are written to the file.\n"
			"  -noDisplayStats                  Stop printout of stats to screen.\n"
			"\n"
			"  -runTime <sec>                   Runtime of the application, in seconds\n"

			"  -threads <thread list>           List of threads, by their bound CPU. Comma-separated list. -1 means do not bind.\n"
			"                                     (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n"
			"  -workerThreads <thread list>     List of CPU bound for Reactor worker threads. -1 means do not bind.\n"
			"  -reactor                         Use the VA Reactor instead of the ETA Channel for sending and receiving.\n"
			" \n"
			"  -castore                         File location of the certificate authority store.\n"
			"\n"
			"  -nanoTime                        Use nanosecond precision for latency information instead of microsecond.\n"
			"  -preEnc                          Use Pre-Encoded updates\n"
			"  -takeMCastStats                  Take Multicast Statistics(Warning: This enables the per-channel lock).\n"
			"  -measureEncode                   Measure encoding time of messages.\n"
			"\n"
			);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}
