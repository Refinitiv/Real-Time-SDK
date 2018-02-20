/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "provPerfConfig.h"
#include "loginProvider.h"
#include "providerThreads.h"
#include "directoryProvider.h"
#include "dictionaryProvider.h"
#include "rtr/rsslRDM.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

static RsslBuffer applicationName = { 12, (char*)"upacProvPerf" };
static RsslBuffer applicationId = { 3, (char*)"256" };

static void clearProvPerfConfig()
{
	provPerfConfig.runTime = 360;

	provPerfConfig.guaranteedOutputBuffers = 5000;
	provPerfConfig.maxFragmentSize = 6144;
	provPerfConfig.sendBufSize = 0;
	provPerfConfig.recvBufSize = 0;
	provPerfConfig.tcpNoDelay = RSSL_TRUE;
	provPerfConfig.highWaterMark = 0;
	snprintf(provPerfConfig.interfaceName, sizeof(provPerfConfig.interfaceName), "");
	snprintf(provPerfConfig.portNo, sizeof(provPerfConfig.portNo), "%s", "14002");
	snprintf(provPerfConfig.summaryFilename, sizeof(provPerfConfig.summaryFilename), "ProvSummary.out");
	provPerfConfig.writeStatsInterval = 5;
	provPerfConfig.displayStats = RSSL_TRUE;
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


void initProvPerfConfig(int argc, char **argv)
{
	int iargs;

	clearProvPerfConfig();
	clearProviderThreadConfig();
	clearLoginConfig();
	clearDirectoryConfig(&directoryConfig);

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
			sscanf(argv[iargs], "%u", &provPerfConfig.runTime);
		}
		else if (strcmp("-summaryFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(provPerfConfig.summaryFilename, sizeof(provPerfConfig.summaryFilename), "%s", argv[iargs]);
		}
		else if (strcmp("-latencyFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			providerThreadConfig.logLatencyToFile = RSSL_TRUE;
			snprintf(providerThreadConfig.latencyLogFilename, sizeof(providerThreadConfig.latencyLogFilename), "%s", argv[iargs]);
		}
		else if (strcmp("-statsFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.statsFilename, sizeof(providerThreadConfig.statsFilename), "%s", argv[iargs]);
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &provPerfConfig.writeStatsInterval);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			provPerfConfig.displayStats = RSSL_FALSE;
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
		else if (0 == strcmp("-outputBufs", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &provPerfConfig.guaranteedOutputBuffers);
		}
		else if (0 == strcmp("-maxFragmentSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &provPerfConfig.maxFragmentSize);
		}
		else if (0 == strcmp("-if", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(provPerfConfig.interfaceName, sizeof(provPerfConfig.interfaceName), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-p", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(provPerfConfig.portNo, sizeof(provPerfConfig.portNo), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-tcpDelay", argv[iargs]))
		{
			provPerfConfig.tcpNoDelay = RSSL_FALSE;
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
		else if (0 == strcmp("-openLimit", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%llu", &directoryConfig.openLimit);
		}


		/* Perf Test configuration */
		else if (0 == strcmp("-msgFile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(providerThreadConfig.msgFilename, sizeof(providerThreadConfig.msgFilename), argv[iargs]);
		}
		else if (0 == strcmp("-updateRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.updatesPerSec);
		}
		else if (0 == strcmp("-genericMsgRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &providerThreadConfig.genMsgsPerSec);
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
		else if (0 == strcmp("-genericMsgLatencyRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			if (0 == strcmp("all", argv[iargs]))
				providerThreadConfig.latencyGenMsgsPerSec = ALWAYS_SEND_LATENCY_GENMSG;
			else
				sscanf(argv[iargs], "%d", &providerThreadConfig.latencyGenMsgsPerSec);
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
		else if (0 == strcmp("-highWaterMark", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &provPerfConfig.highWaterMark);
		}
		else if (0 == strcmp("-sendBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &provPerfConfig.sendBufSize);
		}
		else if (0 == strcmp("-recvBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &provPerfConfig.recvBufSize);
		}
		else if (0 == strcmp("-nanoTime", argv[iargs]))
		{
			providerThreadConfig.nanoTime = RSSL_TRUE;
		}
		else if (0 == strcmp("-preEnc", argv[iargs]))
		{
			providerThreadConfig.preEncItems = RSSL_TRUE;
		}
		else if (0 == strcmp("-measureEncode", argv[iargs]))
		{
			providerThreadConfig.measureEncode = RSSL_TRUE;
		}
		else if (0 == strcmp("-directWrite", argv[iargs]))
		{
			providerThreadConfig.writeFlags |= RSSL_WRITE_DIRECT_SOCKET_WRITE;
		}
		else if (0 == strcmp("-reactor", argv[iargs]))
		{
			provPerfConfig.useReactor = RSSL_TRUE;
		}
		else
		{
			printf("Config Error: Unrecognized option: %s\n", argv[iargs]);
			exitConfigError(argv);
		}
	}


	if (providerThreadConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
	} 

	if (provPerfConfig.writeStatsInterval < 1)
	{
		printf("Config error: Write Stats Interval cannot be less than 1.\n");
		exitConfigError(argv);
	}

	loginConfig.applicationName = applicationName;
	loginConfig.applicationId = applicationId;
	setLoginConfigPosition();

	initDirectoryConfig();
	providerThreadConfigInit();
	initDictionaryProvider();
}


void printProvPerfConfig(FILE *file)
{
	int i;
	int threadStringPos = 0;
	char threadString[128];

	/* Build thread list */
	threadStringPos += snprintf(threadString, 128, "%d", providerThreadConfig.threadBindList[0]);
	for(i = 1; i < providerThreadConfig.threadCount; ++i)
		threadStringPos += snprintf(threadString + threadStringPos, 128 - threadStringPos, ",%d", providerThreadConfig.threadBindList[i]);
	


	fprintf(file, 	"--- TEST INPUTS ---\n\n");

	fprintf(file,  
			"                Run Time: %u\n"
			"                    Port: %s\n"
			"             Thread List: %s\n"
			"          Output Buffers: %u\n"
			"       Max Fragment Size: %u\n"
			"        Send Buffer Size: %u%s\n"
			"        Recv Buffer Size: %u%s\n"
			"          Interface Name: %s\n"
			"             Tcp_NoDelay: %s\n"
			"               Tick Rate: %u\n"
			"       Use Direct Writes: %s\n"
			"         High Water Mark: %d%s\n"
			"            Summary File: %s\n"
			"              Stats File: %s\n"
			"            Latency File: %s\n"
			"    Write Stats Interval: %u\n"
			"           Display Stats: %s\n",
			provPerfConfig.runTime,
			provPerfConfig.portNo,
			threadString,
			provPerfConfig.guaranteedOutputBuffers,
			provPerfConfig.maxFragmentSize,
			provPerfConfig.sendBufSize, (provPerfConfig.sendBufSize ? " bytes" : "(use default)"),
			provPerfConfig.recvBufSize, (provPerfConfig.recvBufSize ? " bytes" : "(use default)"),
			strlen(provPerfConfig.interfaceName) ? provPerfConfig.interfaceName : "(use default)",
			(provPerfConfig.tcpNoDelay ? "Yes" : "No"),
			providerThreadConfig.ticksPerSec,
			(providerThreadConfig.writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) ? "Yes" : "No",
			provPerfConfig.highWaterMark, (provPerfConfig.highWaterMark > 0 ?  " bytes" : "(use default)"),
			provPerfConfig.summaryFilename,
			providerThreadConfig.statsFilename,
			providerThreadConfig.latencyLogFilename,
			provPerfConfig.writeStatsInterval,
			(provPerfConfig.displayStats ? "Yes" : "No")
		  );

	fprintf(file, 
			"             Update Rate: %d\n"
			"     Latency Update Rate: %d\n"
			"        Generic Msg Rate: %d\n"
			"Latency Generic Msg Rate: %d\n"
			"      Refresh Burst Size: %d\n"
			"               Data File: %s\n",

			providerThreadConfig.updatesPerSec,
			providerThreadConfig.latencyUpdatesPerSec >= 0 ? providerThreadConfig.latencyUpdatesPerSec : providerThreadConfig.updatesPerSec,
			providerThreadConfig.genMsgsPerSec,
			providerThreadConfig.latencyGenMsgsPerSec >= 0 ? providerThreadConfig.latencyGenMsgsPerSec : providerThreadConfig.genMsgsPerSec,
			providerThreadConfig.refreshBurstSize,
			providerThreadConfig.msgFilename);

	if (providerThreadConfig.totalBuffersPerPack > 1)
		fprintf(file,
			"                 Packing: Yes(max %d per pack, %u buffer size)\n",
			providerThreadConfig.totalBuffersPerPack,
			providerThreadConfig.packingBufferLength);
	else
		fprintf(file,
			"                 Packing: No\n");



	fprintf(file, 
			"              Service ID: %llu\n"
			"            Service Name: %s\n"
			"               OpenLimit: %llu\n",
			directoryConfig.serviceId,
			directoryConfig.serviceName,
			directoryConfig.openLimit
		  );

	fprintf(file,
			"     Pre-Encoded Updates: %s\n"
			"         Nanosecond Time: %s\n"
			"          Measure Encode: %s\n",
			providerThreadConfig.preEncItems ? "Yes" : "No",
			providerThreadConfig.nanoTime ? "Yes" : "No",
			providerThreadConfig.measureEncode ? "Yes" : "No");

	fprintf(file,
			"             Use Reactor: %s\n\n",
			(provPerfConfig.useReactor ? "Yes" : "No")
		  );
}

void exitWithUsage()
{
	printf(	"Options:\n"
			"  -?                                   Shows this usage\n"
			"\n"
			"  -p <port number>                     Port number\n"
			"\n"
			"  -outputBufs <count>                  Number of output buffers(configures guaranteedOutputBuffers in RsslBindOptions)\n"
			"  -maxFragmentSize <size>              Max size of buffers(configures maxFragmentSize in RsslBindOptions)\n"
			"  -sendBufSize <size>                  System Send Buffer Size(configures sysSendBufSize in RsslBindOptions)\n"
			"  -recvBufSize <size>                  System Receive Buffer Size(configures sysRecvBufSize in RsslBindOptions)\n"
			"  -tcpDelay                            Turns off tcp_nodelay in RsslBindOptions, enabling Nagle's\n"
			"  -highWaterMark                       Sets the number of buffered bytes that will cause UPA to automatically flush\n"
			"  -if <interface name>                 Name of network interface to use\n"
			"\n"
			"  -tickRate <ticks per second>         Ticks per second\n"
			"  -updateRate <updates per second>     Update rate per second\n"
			"  -latencyUpdateRate <updates/sec>     Latency update rate(can specify \"all\" to send latency in every update)\n"
			"  -genericMsgRate <genMsgs per second> GenMsg rate per second\n"
			"  -genericMsgLatencyRate <genMsgs/sec> Latency genMsg rate(can specify \"all\" to send latency in every genMsg)\n"
			"  -maxPackCount <count>                Maximum number of messages packed in a buffer(when count > 1, rsslPackBuffer() is used)\n"
			"  -packBufSize <length>                If packing, sets size of buffer to use\n"
			"  -refreshBurstSize <count>            Number of refreshes to send in a burst(controls granularity of time-checking)\n"
			"  -directWrite                         Sets direct socket write flag when using rsslWrite()\n"
			"\n"
			"  -serviceName <name>                  Service Name\n"
			"  -serviceId <num>                     Service ID\n"
			"  -openLimit <count>                   Max number of items consumer may request per connection.\n"
			"\n"
			"  -msgFile <file name>                 Name of the file that specifies the data content in messages\n"
			"  -summaryFile <filename>              Name of file for logging summary info.\n"
			"  -statsFile <filename>                Base name of file for logging periodic statistics.\n"
			"  -latencyFile <filename>              Base name of file for logging latency data.\n"
			"  -writeStatsInterval <sec>            Controls how often stats are written to the file.\n"
			"  -noDisplayStats                      Stop printout of stats to screen.\n"
			"\n"
			"  -runTime <sec>                       Runtime of the application, in seconds\n"
			"  -threads <thread list>               List of threads, by their bound CPU. Comma-separated list. -1 means do not bind.\n"
			"                                        (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n"
			"\n"
			"  -preEnc                              Use Pre-Encoded updates\n"
			"  -nanoTime                            Use nanosecond precision for latency information instead of microsecond.\n"
			"  -measureEncode                       Measure encoding time of messages.\n"
			"\n"
			"  -reactor                             Use the VA Reactor instead of the UPA Channel for sending and receiving.\n"
			"\n"
			);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}
