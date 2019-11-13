/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/* This provides handling for command-line configuration of ConsPerf. */

#include "consPerfConfig.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static RsslInt32 defaultThreadCount = 1;
static RsslInt32 defaultThreadBindList[] = { -1 };

/* Contains the global application configuration */
ConsPerfConfig consPerfConfig;

static void clearConsPerfConfig()
{
	/* Prepare defaults. */
	consPerfConfig.steadyStateTime = 300;
	consPerfConfig.threadCount = defaultThreadCount;
	consPerfConfig.threadBindList = defaultThreadBindList;

	snprintf(consPerfConfig.summaryFilename, sizeof(consPerfConfig.summaryFilename), "ConsSummary.out");
	snprintf(consPerfConfig.statsFilename, sizeof(consPerfConfig.statsFilename), "ConsStats");
	consPerfConfig.writeStatsInterval = 5;
	consPerfConfig.displayStats = RSSL_TRUE;
	consPerfConfig.logLatencyToFile = RSSL_FALSE;

	consPerfConfig.sendBufSize = 0;
	consPerfConfig.recvBufSize = 0;
	consPerfConfig.highWaterMark = 0;
	consPerfConfig.tcpNoDelay = RSSL_TRUE;
	consPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	consPerfConfig.guaranteedOutputBuffers = 5000;
	consPerfConfig.numInputBuffers = 15;
	snprintf(consPerfConfig.interfaceName, sizeof(consPerfConfig.interfaceName), "");
	snprintf(consPerfConfig.hostName, sizeof(consPerfConfig.hostName), "localhost");
	snprintf(consPerfConfig.portNo, sizeof(consPerfConfig.portNo), "14002");
	snprintf(consPerfConfig.serviceName, sizeof(consPerfConfig.serviceName), "DIRECT_FEED");
	snprintf(consPerfConfig.username, sizeof(consPerfConfig.username), "");

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

	snprintf(consPerfConfig.caStore, sizeof(consPerfConfig.caStore), "");
	consPerfConfig.tlsProtocolFlags = 0;
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
	int iargs;

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
			consPerfConfig.threadBindList = (RsslInt32*)malloc(MAX_CONS_THREADS * sizeof(RsslInt32));

			pToken = strtok(argv[iargs++], ",");
			while(pToken)
			{
				if (++consPerfConfig.threadCount > MAX_CONS_THREADS)
				{
					printf("Config Error: Too many threads specified.\n");
					exit(-1);
				}

				sscanf(pToken, "%d", &consPerfConfig.threadBindList[consPerfConfig.threadCount-1]);

				pToken = strtok(NULL, ",");
			}
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
			else if (strcmp("http", argv[iargs]) == 0)
			{
#ifdef Linux  
				printf("Config Error: Encrypted HTTP connection type not supported on Linux.\n", argv[iargs]);
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
		else if (strcmp("-castore", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(consPerfConfig.caStore, sizeof(consPerfConfig.caStore), "%s", argv[iargs++]);
		}
		else if (strcmp("-spTLSv1.2", argv[iargs]) == 0)
		{
			++iargs; consPerfConfig.tlsProtocolFlags |= RSSL_ENC_TLSV1_2;
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

	consPerfConfig._requestsPerTick = consPerfConfig.itemRequestsPerSec 
		/ consPerfConfig.ticksPerSec;

	consPerfConfig._requestsPerTickRemainder = consPerfConfig.itemRequestsPerSec 
		% consPerfConfig.ticksPerSec;

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
		default:
			return "unknown";
	}

}


/* Print the parsed configuration options(so the user can verify intended usage) */
void printConsPerfConfig(FILE *file)
{
	int i;
	int tmpStringPos = 0;
	char tmpString[128];
	char reactorWatchlistUsageString[32];

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

	/* Build thread list */
	tmpStringPos += snprintf(tmpString, 128, "%d", consPerfConfig.threadBindList[0]);
	for(i = 1; i < consPerfConfig.threadCount; ++i)
		tmpStringPos += snprintf(tmpString + tmpStringPos, 128 - tmpStringPos, ",%d", consPerfConfig.threadBindList[i]);

	fprintf(file, "--- TEST INPUTS ---\n\n");

	fprintf(file,
		"       Steady State Time: %u\n"
		"         Connection Type: %s\n",
		consPerfConfig.steadyStateTime,
		connectionTypeToString(consPerfConfig.connectionType));

	fprintf(file,
		"                Hostname: %s\n"
		"                    Port: %s\n"
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
		"               Tick Rate: %u\n"
		" Reactor/Watchlist Usage: %s\n"
		"       CA store location: %s\n"
		"      TLS Protocol flags: %i\n",
		consPerfConfig.hostName,
		consPerfConfig.portNo,
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
		consPerfConfig.ticksPerSec,
		reactorWatchlistUsageString,
		consPerfConfig.caStore,
		consPerfConfig.tlsProtocolFlags
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
			"  -?                                   Shows this usage\n"
			"\n"
			"  -connType <type>                     Type of connection(\"socket\", \"http\", \"encrypted\")\n"
			"  -encryptedConnType <type>            Encrypted connection protocol. Only used if the \"encrypted\" connection type is selected. \"http\" type is only supported on Windows. (\"socket\", \"http\")\n"
			"  -h <hostname>                        Name of host to connect to\n"
			"  -p <port number>                     Port number to connect to\n"
			"  -if <interface name>                 Name of network interface to use\n"
			"\n"
			"  -outputBufs <count>                  Number of output buffers(configures guaranteedOutputBuffers in RsslConnectOptions)\n"
			"  -inputBufs <count>                   Number of input buffers(configures numInputBufs in RsslConnectOptions)\n"
			"  -tcpDelay                            Turns off tcp_nodelay in RsslConnectOptions, enabling Nagle's\n"
			"  -sendBufSize <size>                  System Send Buffer Size(configures sysSendBufSize in RsslConnectOptions)\n"
			"  -recvBufSize <size>                  System Receive Buffer Size(configures sysRecvBufSize in RsslConnectOptions)\n"
			"\n"
			"  -tickRate <ticks per second>         Ticks per second\n"
			"  -itemCount <count>                   Number of items to request\n"
			"  -commonItemCount <count>             Number of items common to all consumers, if using multiple connections.\n"
			"  -requestRate <items/sec>             Rate at which to request items\n"
			"  -snapshot                            Snapshot test; request all items as non-streaming\n"
			"  -postingRate <posts/sec>             Rate at which to send post messages.\n"
			"  -postingLatencyRate <posts/sec>      Rate at which to send latency post messages.\n"
			"  -genericMsgRate <genMsgs/sec>        Rate at which to send generic messages.\n"
			"  -genericMsgLatencyRate <genMsgs/sec> Rate at which to send latency generic messages.\n"
			"\n"
			"  -uname <name>                        Username to use in login request\n"
			"  -serviceName <name>                  Service Name\n"
			"\n"
			"  -itemFile <file name>                Name of the file to get item names from\n"
			"  -msgFile <file name>                 Name of the file that specifies the data content in messages\n"
			"  -summaryFile <filename>              Name of file for logging summary info.\n"
			"  -statsFile <filename>                Base name of file for logging periodic statistics.\n"
			"  -writeStatsInterval <sec>            Controls how often stats are written to the file.\n"
			"  -noDisplayStats                      Stop printout of stats to screen.\n"
			"  -latencyFile <filename>              Base name of file for logging latency.\n"
			"\n"
			"  -steadyStateTime <seconds>           Time consumer will run the steady-state portion of the test.\n"
			"                                         Also used as a timeout during the startup-state portion.\n"
			"  -threads <thread list>               list of threads(which create 1 connection each),\n"
			"                                         by their bound CPU. Comma-separated list. -1 means do not bind.\n"
			"                                         (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n"
			"  -reactor                             Use the VA Reactor instead of the UPA Channel for sending and receiving.\n"
			"  -watchlist                           Use the VA Reactor watchlist instead of the UPA Channel for sending and receiving.\n"
			"\n"
			"  -nanoTime                            Assume latency has nanosecond precision instead of microsecond.\n"
			"  -measureDecode                       Measure decode time of updates.\n"
			"\n"
			"  -castore								File location of the certificate authority store.\n"
			"  -spTLSv1.2							Specifies that TLSv1.2 can be used for an OpenSSL-based encrypted connection\n"
			"\n"
	);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}
