/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "providerThreads.h"
#include "directoryProvider.h"
#include "niProvPerfConfig.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static RsslInt32 defaultThreadCount = 1;
static RsslInt32 defaultThreadBindList[] = { -1 };

/* Contains the global application configuration */
NIProvPerfConfig niProvPerfConfig;

static void clearNIProvPerfConfig()
{
	/* Prepare defaults. */
	niProvPerfConfig.runTime = 360;
	providerThreadConfig.threadBindList = defaultThreadBindList;

	niProvPerfConfig.highWaterMark = 0;
	niProvPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	niProvPerfConfig.guaranteedOutputBuffers = 5000;
	niProvPerfConfig.sendBufSize = 0;
	niProvPerfConfig.recvBufSize = 0;
	snprintf(niProvPerfConfig.interfaceName, sizeof(niProvPerfConfig.interfaceName), "");
	snprintf(niProvPerfConfig.hostName, sizeof(niProvPerfConfig.hostName), "%s", "localhost");
	snprintf(niProvPerfConfig.portNo, sizeof(niProvPerfConfig.portNo), "%s", "14003");
	snprintf(niProvPerfConfig.summaryFilename, sizeof(niProvPerfConfig.summaryFilename), "NIProvSummary.out");
	snprintf(niProvPerfConfig.username, sizeof(niProvPerfConfig.username), "");
	niProvPerfConfig.writeStatsInterval = 5;
	niProvPerfConfig.displayStats = RSSL_TRUE;
	niProvPerfConfig.tcpNoDelay = RSSL_TRUE;
	snprintf(niProvPerfConfig.sendAddr, sizeof(niProvPerfConfig.sendAddr), "");
	snprintf(niProvPerfConfig.recvAddr, sizeof(niProvPerfConfig.recvAddr), "");
	snprintf(niProvPerfConfig.sendPort, sizeof(niProvPerfConfig.sendPort), "");
	snprintf(niProvPerfConfig.recvPort, sizeof(niProvPerfConfig.recvPort), "");
	snprintf(niProvPerfConfig.unicastPort, sizeof(niProvPerfConfig.unicastPort), "");
	niProvPerfConfig.sAddr = RSSL_FALSE;
	niProvPerfConfig.rAddr = RSSL_FALSE;

	niProvPerfConfig.itemPublishCount = 100000;
	niProvPerfConfig.commonItemCount = 0;
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
	int iargs;

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
		   
			providerThreadConfig.threadBindList = (RsslInt32*)malloc(128 * sizeof(RsslInt32));
			pToken = strtok(argv[iargs], ",");
			while(pToken)
			{
				if (++providerThreadConfig.threadCount > 128)
				{
					printf("Config Error: Too many threads specified.\n");
					exitConfigError(argv);
				}

				sscanf(pToken, "%d", &providerThreadConfig.threadBindList[providerThreadConfig.threadCount-1]);

				pToken = strtok(NULL, ",");
			}

		}
		else if (0 == strcmp("-connType", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (0 == strcmp(argv[iargs], "socket"))
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
			else if (0 == strcmp(argv[iargs], "reliableMCast"))
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
			else
				niProvPerfConfig.connectionType = RSSL_CONN_TYPE_INIT; /* error */
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
			snprintf(directoryConfig.serviceName, sizeof(directoryConfig.serviceName), argv[iargs]);
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
			snprintf(providerThreadConfig.itemFilename, sizeof(providerThreadConfig.itemFilename), argv[iargs]);
		}
		else if (0 == strcmp("-msgFile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.msgFilename, sizeof(providerThreadConfig.msgFilename), argv[iargs]);
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
		else if (0 == strcmp("-directWrite", argv[iargs]))
		{
			providerThreadConfig.writeFlags |= RSSL_WRITE_DIRECT_SOCKET_WRITE;
		}
		else if(strcmp("-highWaterMark", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &niProvPerfConfig.highWaterMark);
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
		default:
			return "unknown";
	}

}

void printNIProvPerfConfig(FILE *file)
{
	int i;
	int threadStringPos = 0;
	char threadString[128];

	/* Build thread list */
	threadStringPos += snprintf(threadString, 128, "%d", providerThreadConfig.threadBindList[0]);
	for(i = 1; i < providerThreadConfig.threadCount; ++i)
		threadStringPos += snprintf(threadString + threadStringPos, 128 - threadStringPos, ",%d", providerThreadConfig.threadBindList[i]);
	

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
			"          Service Name: %s\n\n",
			directoryConfig.serviceId,
			directoryConfig.serviceName
		  );


}

void exitWithUsage()
{
	printf(	"Options:\n"
			"  -?                               Shows this usage\n"
			"  -connType <type>                 Type of connection(\"socket\", \"reliableMCast\")\n"
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

			"\n"
			);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}
