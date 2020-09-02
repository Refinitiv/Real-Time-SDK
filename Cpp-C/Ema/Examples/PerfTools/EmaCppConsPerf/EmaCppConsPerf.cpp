///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "EmaCppConsPerf.h"
#include "../Common/AppUtil.h"
#include "../Common/CtrlBreakHandler.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;
using namespace perftool::common;

FILE *EmaCppConsPerf::summaryFile = NULL;
EmaString EmaCppConsPerf::logText = "";

EmaCppConsPerf::~EmaCppConsPerf()
{
	if( summaryFile )
		fclose( summaryFile );
}

bool EmaCppConsPerf::initConsPerfConfig(int argc, char *argv[])
{
	int iargs = 1;
	while(iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			exitWithUsage();
			return false;
		}
		else if(strcmp("-uname", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.username = argv[iargs++];
		}
		else if(strcmp("-threads", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			char *pToken;
			Int32 consThreadCount = ( consPerfConfig.threadCount > 0) ? consPerfConfig.threadCount  : 0;
			consPerfConfig.threadCount = 0;
			if(consPerfConfig.threadBindList)
				delete [] consPerfConfig.threadBindList;
			consPerfConfig.threadBindList = new long[MAX_CONS_THREADS];

			pToken = strtok(argv[iargs++], ",");
			while(pToken)
			{
				if (++consPerfConfig.threadCount > MAX_CONS_THREADS)
				{
					logText = "Config Error: Too many threads specified.";
					AppUtil::logError(logText);
					if( consPerfConfig.threadBindList )
					{
						delete [] consPerfConfig.threadBindList;
						consPerfConfig.threadBindList = NULL;
					}
					return false;
				}
				sscanf(pToken, "%ld", &consPerfConfig.threadBindList[consPerfConfig.threadCount-1]);
				pToken = strtok(NULL, ",");
			}
			if( consThreadCount > 0 && consPerfConfig.threadCount != consThreadCount )
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				if( consPerfConfig.threadBindList )
				{
					delete [] consPerfConfig.threadBindList;
					consPerfConfig.threadBindList = NULL;
				}
			if( consPerfConfig.apiThreadBindList )
				{
					delete [] consPerfConfig.apiThreadBindList;
					consPerfConfig.apiThreadBindList = NULL;
				}
				return false;
			}
			if( consPerfConfig.threadCount < MAX_CONS_THREADS )
			{
				for( int i = consPerfConfig.threadCount; i < MAX_CONS_THREADS; ++i )
					consPerfConfig.threadBindList[i] = -1;
			}
		}
		else if(strcmp("-apiThreads", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			char *pToken;
			Int32 consThreadCount = ( consPerfConfig.threadCount > 0) ? consPerfConfig.threadCount  : 0;
			consPerfConfig.threadCount = 0;
			if(consPerfConfig.apiThreadBindList)
				delete [] consPerfConfig.apiThreadBindList;
			consPerfConfig.apiThreadBindList = new long[MAX_CONS_THREADS];

			pToken = strtok(argv[iargs++], ",");
			while(pToken)
			{
				if (++consPerfConfig.threadCount > MAX_CONS_THREADS)
				{
					logText = "Config Error: Too many api threads specified.";
					AppUtil::logError(logText);
					if( consPerfConfig.apiThreadBindList )
					{
						delete [] consPerfConfig.apiThreadBindList;
						consPerfConfig.apiThreadBindList = NULL;
					}
					return false;
				}
	
				sscanf(pToken, "%ld", &consPerfConfig.apiThreadBindList[consPerfConfig.threadCount-1]);
				pToken = strtok(NULL, ",");
			}
			if( consThreadCount > 0  && consPerfConfig.threadCount != consThreadCount )
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				if( consPerfConfig.threadBindList )
				{
					delete [] consPerfConfig.threadBindList;
					consPerfConfig.threadBindList = NULL;
				}
			if( consPerfConfig.apiThreadBindList )
				{
					delete [] consPerfConfig.apiThreadBindList;
					consPerfConfig.apiThreadBindList = NULL;
				}
				return false;
			}
			if( consPerfConfig.threadCount < MAX_CONS_THREADS )
			{
				for( int i = consPerfConfig.threadCount; i < MAX_CONS_THREADS; ++i )
					consPerfConfig.threadBindList[i] = -1;
			}
		}
		else if(strcmp("-mainThread", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			int i = atoi(argv[iargs++]);
			consPerfConfig.mainThreadCpu = i;
		}
		else if(strcmp("-serviceName", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.serviceName = argv[iargs++];
		}
		else if(strcmp("-useServiceId", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			int i = atoi(argv[iargs++]);
			consPerfConfig.useServiceId = ( i == 1 ) ? true : false;
		}
		else if(strcmp("-useUserDispatch", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			int i = atoi(argv[iargs++]);
			consPerfConfig.useUserDispatch = ( i == 1 ) ? true : false;
		}
		else if(strcmp("-itemCount", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.itemRequestCount = atoi(argv[iargs++]);
		}
		else if(strcmp("-commonItemCount", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.commonItemCount = atoi(argv[iargs++]);
		}
		else if(strcmp("-itemFile", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.itemFilename = argv[iargs++];
		}
		else if(strcmp("-msgFile", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.msgFilename = argv[iargs++];
		}
		else if(strcmp("-latencyFile", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.logLatencyToFile = true;
			consPerfConfig.latencyLogFilename = argv[iargs++];
		}
		else if(strcmp("-summaryFile", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.summaryFilename = argv[iargs++];
		}
		else if(strcmp("-statsFile", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.statsFilename = argv[iargs++];
		}
		else if(strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.writeStatsInterval = atoi(argv[iargs++]);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			++iargs;
			consPerfConfig.displayStats = false;
		}
		else if(strcmp("-steadyStateTime", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.steadyStateTime = atoi(argv[iargs++]);
		}
		else if(strcmp("-requestRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.itemRequestsPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-snapshot", argv[iargs]) == 0)
		{
			++iargs;
			consPerfConfig.requestSnapshots = true;
		}
		else if(strcmp("-postingRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.postsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-postingLatencyRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.latencyPostsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-genericMsgRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.genMsgsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-genericMsgLatencyRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.latencyGenMsgsPerSec = atoi(argv[iargs++]);
		}
		else if(strcmp("-tickRate", argv[iargs]) == 0)
		{
			++iargs; 
			if (iargs == argc) 
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			consPerfConfig.ticksPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-websocket", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			if (strcmp("rssl.json.v2", argv[iargs]) == 0)
			{
				consPerfConfig.websocketProtocol = ConsPerfConfig::WebSocketJSONEnum;
			}
			else if (strcmp("rssl.rwf", argv[iargs]) == 0)
			{
				consPerfConfig.websocketProtocol = ConsPerfConfig::WebSocketRWFEnum;
			}
			else
			{
				printf("Config Error: Unknown websocket protocol \"%s\"\n", argv[iargs]);
				exitConfigError(argv);
				return false;
			}
			iargs++;
		}
		else
		{
			logText = "Invalid Config ";
			logText += argv[iargs];
			AppUtil::logError(logText);
			exitWithUsage();
			return false;		
		}
	}
	
	if( consPerfConfig.useUserDispatch && consPerfConfig.apiThreadBindList[0] != -1 )
	{
		AppUtil::logError("Config Error: -apiThreads cannot be used when user dispacth is used. ");
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.ticksPerSec < 1)
	{
		AppUtil::logError("Config Error: Tick rate cannot be less than 1. ");
		exitConfigError(argv); return false;
	} 

	if (consPerfConfig.itemRequestsPerSec < consPerfConfig.ticksPerSec)
	{
		logText = "Config Error: Item Request Rate cannot be less than tick rate.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.postsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.postsPerSec != 0)
	{
		logText = "Config Error: Post Rate cannot be less than tick rate(unless it is zero).";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}
	if (consPerfConfig.genMsgsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.genMsgsPerSec != 0)
	{
		logText = "Config Error: Generic Message Rate cannot be less than tick rate(unless it is zero).";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.latencyPostsPerSec > consPerfConfig.postsPerSec)
	{
		logText = "Config Error: Latency Post Rate cannot be greater than total posting rate.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}
	if (consPerfConfig.latencyGenMsgsPerSec > consPerfConfig.genMsgsPerSec)
	{
		logText = "Config Error: Latency Generic Message Rate cannot be greater than total generic message rate.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.latencyPostsPerSec > consPerfConfig.ticksPerSec)
	{
		logText = "Config Error: Latency Post Rate cannot be greater than tick rate.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}
	if (consPerfConfig.latencyGenMsgsPerSec > consPerfConfig.ticksPerSec)
	{
		logText = "Config Error: Latency Generic Message Rate cannot be greater than tick rate.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.postsPerSec && consPerfConfig.requestSnapshots)
	{
		logText = "Config Error: Configured to post while requesting snapshots.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}
	if (consPerfConfig.genMsgsPerSec && consPerfConfig.requestSnapshots)
	{
		logText = "Config Error: Configured to send generic messages while requesting snapshots."; 
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.postsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.postsPerSec != 0)
	{
		logText = "Config Error: Post Rate cannot be less than tick rate(unless it is zero).";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}
	if (consPerfConfig.genMsgsPerSec < consPerfConfig.ticksPerSec && consPerfConfig.genMsgsPerSec != 0)
	{
		logText = "Config Error: Generic Message Rate cannot be less than tick rate(unless it is zero).";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.commonItemCount > consPerfConfig.itemRequestCount / consPerfConfig.threadCount)
	{
		logText = "Config Error: Common item count ";
		logText.append(consPerfConfig.commonItemCount);
		logText += " is greater than total item count per thread ";
		logText.append( consPerfConfig.itemRequestCount / consPerfConfig.threadCount );
		logText += ".";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	if (consPerfConfig.writeStatsInterval < 1)
	{
		logText = "Config Error: Write Stats Interval cannot be less than 1.";
		AppUtil::logError(logText);
		exitConfigError(argv); return false;
	}

	consPerfConfig._requestsPerTick = consPerfConfig.itemRequestsPerSec / consPerfConfig.ticksPerSec;

	consPerfConfig._requestsPerTickRemainder = consPerfConfig.itemRequestsPerSec % consPerfConfig.ticksPerSec;
	
	return true;
}
void EmaCppConsPerf::exitConfigError(char **argv)
{
	logText ="Run '";
	logText += argv[0];
	logText += " -?' to see usage.\n";

	AppUtil::logError(logText);
}

void EmaCppConsPerf::exitOnMissingArgument(char **argv, int argPos)
{
	logText = "Config error: ";
	logText += argv[argPos];
	logText += " missing argument.\n";
	logText += "Run '";
	logText += argv[0];
	logText += " -?' to see usage.\n";
	
	AppUtil::logError(logText);
}
void EmaCppConsPerf::exitWithUsage()
{
	logText = "Options:\n";
	logText += "  -?                                    Shows this usage\n";
	logText += "  -tickRate <ticks per second>          Ticks per second\n";
	logText += "   -itemCount <count>                   Number of items to request\n";
	logText += "   -commonItemCount <count>             Number of items common to all consumers, if using multiple connections.\n";
	logText += "   -requestRate <items/sec>             Rate at which to request items\n";
	logText += "   -snapshot                            Snapshot test; request all items as non-streaming\n";
	logText += "   -postingRate <posts/sec>             Rate at which to send post messages.\n";
	logText += "   -postingLatencyRate <posts/sec>      Rate at which to send latency post messages.\n";
	logText += "   -genericMsgRate <genMsgs/sec>        Rate at which to send generic messages.\n";
	logText += "   -genericMsgLatencyRate <genMsgs/sec> Rate at which to send latency generic messages.\n\n";
	logText += "   -uname <name>                        Username to use in login request\n";
	logText += "   -serviceName <name>                  Service Name\n";
	logText += "   -useServiceId <1 Or 0>               Value 1 will use serviceId in the Requests\n\n";
	logText += "   -useUserDispatch <1 Or 0>            Value 1 will use UserDispatch \n";
	logText += "   -itemFile <file name>                Name of the file to get item names from\n";
	logText += "   -msgFile <file name>                 Name of the file that specifies the data content in messages\n";
	logText += "   -summaryFile <filename>              Name of file for logging summary info.\n";
	logText += "   -statsFile <filename>                Base name of file for logging periodic statistics.\n";
	logText += "   -writeStatsInterval <sec>            Controls how often stats are written to the file.\n";
	logText += "   -noDisplayStats                      Stop printout of stats to screen.\n";
	logText += "   -latencyFile <filename>              Base name of file for logging latency.\n\n";
	logText += "   -steadyStateTime <seconds>           Time consumer will run the steady-state portion of the test.\n";
	logText += "                                          Also used as a timeout during the startup-state portion.\n";
	logText += "    -apiThreads <thread list>			list of Api threads in ApiDispatch mode (which create 1 connection each),\n";
	logText += "                                          by their bound CPU. Comma-separated list. -1 means do not bind.\n";
	logText += "                                         Must match the count of listed in -threads option.\n";
	logText += "                                          (e.g. \"-apiThreads 0,1 \" creates two threads bound to CPU's 0 and 1)\n";
	logText += "   -mainThread <CpuId>                  CPU of the main thread of the app that collects & prints stats. \n";
	logText += "   -threads <thread list>               list of threads(which create 1 connection each),\n";
	logText += "                                          by their bound CPU. Comma-separated list. -1 means do not bind.\n";
	logText += "                                          (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n";
	logText += "   -websocket <protocol>                Using websocket connection with specified tunnel protocol: \"rssl.json.v2\" or \"rssl.rwf\".\n";

	AppUtil::logError(logText);
}
void EmaCppConsPerf::printConsPerfConfig(FILE *file)
{
	// NOTE: For EMA unlike ETA the following parameters are contolled by EMA for priniting
	//        The application does not have interface to access it and print its values
	//        Ema prints them via log file.
	// Unless the transport config paramaters are passed via programatic config and logger turned off.
	// 1. consPerfConfig.connectionType
	// 2. Hostname, Port
	int i;
	int tmpStringPos = 0;
	int tmpApiThreadListStringPos = 0;
	char tmpString[128];
	char tmpApiThreadListString[128];
	char mainThread[128];
	// Build thread list 
	snprintf(mainThread, 128, "%d", consPerfConfig.mainThreadCpu);
	tmpStringPos += snprintf(tmpString, 128, "%d", consPerfConfig.threadBindList[0]);
	tmpApiThreadListStringPos += snprintf(tmpApiThreadListString, 128, "%d", consPerfConfig.apiThreadBindList[0]);
	for(i = 1; i < consPerfConfig.threadCount; ++i)
	{
		tmpStringPos += snprintf(tmpString + tmpStringPos, 128 - tmpStringPos, ",%d", consPerfConfig.threadBindList[i]);
		if( !consPerfConfig.useUserDispatch )
			tmpApiThreadListStringPos += snprintf(tmpApiThreadListString + tmpApiThreadListStringPos, 128 - tmpStringPos, ",%d", consPerfConfig.apiThreadBindList[i]);

	}

	fprintf(file, "--- TEST INPUTS ---\n\n");
	fprintf(file,
	"       Steady State Time: %u\n",
	consPerfConfig.steadyStateTime);
	fprintf(file,
		"                 Service: %s\n"
		"         useUserDispatch: %s\n"
		"              mainThread: %s\n"
		"             Thread List: %s\n"
		"          ApiThread List: %s\n"
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
		"               Tick Rate: %u\n",
		consPerfConfig.serviceName.c_str(),
		(consPerfConfig.useUserDispatch) ? "1" : "0",
		mainThread,
		tmpString,
		tmpApiThreadListString,
		strlen(consPerfConfig.username.c_str()) ? consPerfConfig.username.c_str() : "(use system login name)",
		consPerfConfig.itemRequestCount,
		consPerfConfig.commonItemCount,
		consPerfConfig.itemRequestsPerSec,
		consPerfConfig.requestSnapshots ? "Yes" : "No",
		consPerfConfig.postsPerSec,
		consPerfConfig.latencyPostsPerSec,
		consPerfConfig.genMsgsPerSec,
		consPerfConfig.latencyGenMsgsPerSec,
		consPerfConfig.itemFilename.c_str(),
		consPerfConfig.msgFilename.c_str(),
		consPerfConfig.summaryFilename.c_str(),
		consPerfConfig.statsFilename.c_str(),
		consPerfConfig.logLatencyToFile ? consPerfConfig.latencyLogFilename.c_str() : "(none)",
		consPerfConfig.ticksPerSec);
}
void EmaCppConsPerf::printSummaryStatistics(FILE *file)
{
	PerfTimeValue firstUpdateTime;
	Int32 i;
	UInt64 totalUpdateCount = totalStats.startupUpdateCount.countStatGetTotal()
		+ totalStats.steadyStateUpdateCount.countStatGetTotal();

	// Find when the first update was received. 
	firstUpdateTime = 0;
	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		if (!firstUpdateTime || consumerThreads[i]->stats.firstUpdateTime < firstUpdateTime)
			firstUpdateTime = consumerThreads[i]->stats.firstUpdateTime;
	}

	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		PerfTimeValue imageRetrievalTime = consumerThreads[i]->stats.imageRetrievalEndTime ?
			(consumerThreads[i]->stats.imageRetrievalEndTime 
			- consumerThreads[i]->stats.imageRetrievalStartTime) : 0;

		// If there are multiple connections, print individual summaries. 
		if (consPerfConfig.threadCount > 1)
		{
			UInt64 totalClientUpdateCount = 
				consumerThreads[i]->stats.startupUpdateCount.countStatGetTotal()
				+ consumerThreads[i]->stats.steadyStateUpdateCount.countStatGetTotal();

			fprintf(file, "\n--- CLIENT %d SUMMARY ---\n\n", i + 1);

			fprintf( file, "Startup State Statistics:\n");

			fprintf(file, 
					"  Sampling duration (sec): %.3f\n",
					consumerThreads[i]->stats.imageRetrievalStartTime ?
					(((double)(consumerThreads[i]->stats.imageRetrievalEndTime ? consumerThreads[i]->stats.imageRetrievalEndTime : currentTime)
					 - (double)consumerThreads[i]->stats.imageRetrievalStartTime )/1000000000.0) : 0.0);

			if (consumerThreads[i]->stats.startupLatencyStats.count)
			{
				fprintf( file, 
						"  Latency avg (usec): %.1f\n"
						"  Latency std dev (usec): %.1f\n"
						"  Latency max (usec): %.1f\n"
						"  Latency min (usec): %.1f\n",
						consumerThreads[i]->stats.startupLatencyStats.mean,
						sqrt(consumerThreads[i]->stats.startupLatencyStats.variance),
						consumerThreads[i]->stats.startupLatencyStats.maxValue,
						consumerThreads[i]->stats.startupLatencyStats.minValue);
			}
			else
				fprintf( file, "  No latency information received during startup time.\n\n");

			fprintf(file, "  Avg update rate: %.0f\n", 
					(double) consumerThreads[i]->stats.startupUpdateCount.countStatGetTotal()/
					(double)((
							(consumerThreads[i]->stats.imageRetrievalEndTime ?
							consumerThreads[i]->stats.imageRetrievalEndTime : currentTime)
							- consumerThreads[i]->stats.firstUpdateTime
							)/1000000000.0));

			fprintf( file, "\nSteady State Statistics:\n");

			if (consumerThreads[i]->stats.imageRetrievalEndTime)
			{
				fprintf(file, 
						"  Sampling duration (sec): %.3f\n",
						((double)currentTime - (double)consumerThreads[i]->stats.imageRetrievalEndTime)/1000000000.0);
				if (consumerThreads[i]->stats.steadyStateLatencyStats.count)
				{
					fprintf( file,
							"  Latency avg (usec): %.1f\n"
							"  Latency std dev (usec): %.1f\n"
							"  Latency max (usec): %.1f\n"
							"  Latency min (usec): %.1f\n",
							consumerThreads[i]->stats.steadyStateLatencyStats.mean,
							sqrt(consumerThreads[i]->stats.steadyStateLatencyStats.variance),
							consumerThreads[i]->stats.steadyStateLatencyStats.maxValue,
							consumerThreads[i]->stats.steadyStateLatencyStats.minValue);
				}
				else
					fprintf( file, "  No latency information was received during steady-state time.\n");

				if (consPerfConfig.latencyPostsPerSec)
				{
					if (consumerThreads[i]->stats.postLatencyStats.count)
					{
						fprintf( file,
								"  Posting latency avg (usec): %.1f\n"
								"  Posting latency std dev (usec): %.1f\n"
								"  Posting latency max (usec): %.1f\n"
								"  Posting latency min (usec): %.1f\n",
								consumerThreads[i]->stats.postLatencyStats.mean,
								sqrt(consumerThreads[i]->stats.postLatencyStats.variance),
								consumerThreads[i]->stats.postLatencyStats.maxValue,
								consumerThreads[i]->stats.postLatencyStats.minValue);
					}
					else
						fprintf( file, "  No posting latency information was received during steady-state time.\n");
				}

				fprintf(file, "  Avg update rate: %.0f\n", 
						(double) consumerThreads[i]->stats.steadyStateUpdateCount.countStatGetTotal()
						/(double)((currentTime - consumerThreads[i]->stats.imageRetrievalEndTime)/1000000000.0));
			}
			else
				fprintf( file, "  Steady state was not reached during this test.\n\n");


			fprintf(file, "\nOverall Statistics: \n");

			fprintf(file, 
					"  Sampling duration (sec): %.3f\n",
					consumerThreads[i]->stats.imageRetrievalStartTime ?
					((double)currentTime
					 - (double)consumerThreads[i]->stats.imageRetrievalStartTime )/1000000000.0 : 0.0);

			if (consumerThreads[i]->stats.overallLatencyStats.count)
			{
				fprintf( file,
						"  Latency avg (usec): %.1f\n"
						"  Latency std dev (usec): %.1f\n"
						"  Latency max (usec): %.1f\n"
						"  Latency min (usec): %.1f\n",
						consumerThreads[i]->stats.overallLatencyStats.mean,
						sqrt(consumerThreads[i]->stats.overallLatencyStats.variance),
						consumerThreads[i]->stats.overallLatencyStats.maxValue,
						consumerThreads[i]->stats.overallLatencyStats.minValue);
			}
			else
				fprintf( file, "  No latency information was received.\n");

			if (consumerThreads[i]->stats.genMsgLatencyStats.count)
			{
				fprintf( file,
						"  GenMsg latency avg (usec): %.1f\n"
						"  GenMsg latency std dev (usec): %.1f\n"
						"  GenMsg latency max (usec): %.1f\n"
						"  GenMsg latency min (usec): %.1f\n",
						consumerThreads[i]->stats.genMsgLatencyStats.mean,
						sqrt(consumerThreads[i]->stats.genMsgLatencyStats.variance),
						consumerThreads[i]->stats.genMsgLatencyStats.maxValue,
						consumerThreads[i]->stats.genMsgLatencyStats.minValue);
			}
			else
				fprintf( file, "  No GenMsg latency information was received.\n");

			fprintf(file, "\nTest Statistics:\n");

			fprintf(file, 
					"  Requests sent: %llu\n"
					"  Refreshes received: %llu\n"
					"  Updates received: %llu\n",
					consumerThreads[i]->stats.requestCount.countStatGetTotal(),
					consumerThreads[i]->stats.refreshCount.countStatGetTotal(),
					totalClientUpdateCount);

			if (consPerfConfig.postsPerSec)
			{
				fprintf(file, 
						"  Posts sent: %llu\n",
						consumerThreads[i]->stats.postSentCount.countStatGetTotal());
			}

			if (consPerfConfig.genMsgsPerSec)
				fprintf(file, "  GenMsgs sent: %llu\n", consumerThreads[i]->stats.genMsgSentCount.countStatGetTotal());
			if (consumerThreads[i]->stats.genMsgRecvCount.countStatGetTotal())
				fprintf(file, "  GenMsgs received: %llu\n", consumerThreads[i]->stats.genMsgRecvCount.countStatGetTotal());
			if (consPerfConfig.latencyGenMsgsPerSec)
				fprintf(file, "  GenMsg latencies sent: %llu\n", consumerThreads[i]->stats.latencyGenMsgSentCount.countStatGetTotal());
			if (consumerThreads[i]->stats.genMsgLatencyStats.count)
				fprintf(file, "  GenMsg latencies received: %llu\n", consumerThreads[i]->stats.genMsgLatencyStats.count);

			if (imageRetrievalTime)
			{
				fprintf(file,
						"  Image retrieval time (sec): %.3f\n"
						"  Avg image rate: %.0f\n",
						(double)imageRetrievalTime/1000000000.0,
						consumerThreads[i]->stats.refreshCount.countStatGetTotal()
						/((double)imageRetrievalTime/1000000000.0)); 
			}

			fprintf(file, "  Avg update rate: %.0f\n",
					(double)totalClientUpdateCount
					/(double)((currentTime - consumerThreads[i]->stats.firstUpdateTime)/1000000000.0));

			if (consPerfConfig.postsPerSec)
			{
				fprintf(file, "  Avg posting rate: %.0f\n", 
						(double) consumerThreads[i]->stats.postSentCount.countStatGetTotal()/(double)((currentTime - consumerThreads[i]->stats.imageRetrievalEndTime)/1000000000.0));
			}

			if (consPerfConfig.genMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg send rate: %.0f\n", 
					(double) consumerThreads[i]->stats.genMsgSentCount.countStatGetTotal()/
					(double)((currentTime - consumerThreads[i]->stats.firstGenMsgSentTime)/1000000000.0));
			}
			if ( consumerThreads[i]->stats.genMsgRecvCount.countStatGetTotal())
			{
				fprintf(file, "  Avg GenMsg receive rate: %.0f\n", 
					(double) consumerThreads[i]->stats.genMsgRecvCount.countStatGetTotal()/
					(double)((currentTime - consumerThreads[i]->stats.firstGenMsgRecvTime)/1000000000.0));
			}
			if (consPerfConfig.latencyGenMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", 
					(double) consumerThreads[i]->stats.latencyGenMsgSentCount.countStatGetTotal()/
					(double)((currentTime - consumerThreads[i]->stats.firstGenMsgSentTime)/1000000000.0));
			}
			if (consumerThreads[i]->stats.genMsgLatencyStats.count)
			{
				fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", 
					(double)(consumerThreads[i]->stats.genMsgLatencyStats.count)/
					(double)((currentTime - consumerThreads[i]->stats.firstGenMsgRecvTime)/1000000000.0));
			}
		}	
	}

	fprintf( file, "\n--- OVERALL SUMMARY ---\n\n");

	fprintf( file, "Startup State Statistics:\n");

	fprintf(file, 
			"  Sampling duration (sec): %.3f\n",
			(totalStats.imageRetrievalStartTime ? 
			 ((double)(totalStats.imageRetrievalEndTime ? totalStats.imageRetrievalEndTime : currentTime)
			- (double)(totalStats.imageRetrievalStartTime))/1000000000.0 : 0.0));

	if (totalStats.startupLatencyStats.count)
	{
		fprintf( file, 
				"  Latency avg (usec): %.1f\n"
				"  Latency std dev (usec): %.1f\n"
				"  Latency max (usec): %.1f\n"
				"  Latency min (usec): %.1f\n",
				totalStats.startupLatencyStats.mean,
				sqrt(totalStats.startupLatencyStats.variance),
				totalStats.startupLatencyStats.maxValue,
				totalStats.startupLatencyStats.minValue);
	}
	else
		fprintf( file, "  No latency information received during startup time.\n\n");

	fprintf(file, "  Avg update rate: %.0f\n\n", 
			(double) totalStats.startupUpdateCount.countStatGetTotal()
			/(double)( ((totalStats.imageRetrievalEndTime ? totalStats.imageRetrievalEndTime : currentTime)
					- firstUpdateTime)/1000000000.0));

	fprintf( file, "Steady State Statistics:\n");

	if (totalStats.imageRetrievalEndTime)
	{

		fprintf(file, 
				"  Sampling duration (sec): %.3f\n",
				((double)currentTime - (double)totalStats.imageRetrievalEndTime)/1000000000.0);

		if (totalStats.steadyStateLatencyStats.count)
		{
			fprintf( file,
					"  Latency avg (usec): %.1f\n"
					"  Latency std dev (usec): %.1f\n"
					"  Latency max (usec): %.1f\n"
					"  Latency min (usec): %.1f\n",
					totalStats.steadyStateLatencyStats.mean,
					sqrt(totalStats.steadyStateLatencyStats.variance),
					totalStats.steadyStateLatencyStats.maxValue,
					totalStats.steadyStateLatencyStats.minValue);
		}
		else
			fprintf( file, "  No latency information was received during steady-state time.\n");

		if (consPerfConfig.latencyPostsPerSec)
		{
			if (totalStats.postLatencyStats.count)
			{
				fprintf( file,
						"  Posting latency avg (usec): %.1f\n"
						"  Posting latency std dev (usec): %.1f\n"
						"  Posting latency max (usec): %.1f\n"
						"  Posting latency min (usec): %.1f\n",
						totalStats.postLatencyStats.mean,
						sqrt(totalStats.postLatencyStats.variance),
						totalStats.postLatencyStats.maxValue,
						totalStats.postLatencyStats.minValue);
			}
			else
				fprintf( file, "  No posting latency information was received during steady-state time.\n");
		}

		fprintf(file, "  Avg update rate: %.0f\n", 
				(double) totalStats.steadyStateUpdateCount.countStatGetTotal()
				/(double)((currentTime - totalStats.imageRetrievalEndTime)/1000000000.0));

		fprintf(file, "\n");

	}
	else
		fprintf( file, "  Steady state was not reached during this test.\n\n");
		

	fprintf(file, "Overall Statistics: \n");

	fprintf(file, 
			"  Sampling duration (sec): %.3f\n",
			(totalStats.imageRetrievalStartTime ? 
			((double)currentTime
			 - (double)totalStats.imageRetrievalStartTime)/1000000000.0 : 0.0));

	if (totalStats.overallLatencyStats.count)
	{
		fprintf( file,
				"  Latency avg (usec): %.1f\n"
				"  Latency std dev (usec): %.1f\n"
				"  Latency max (usec): %.1f\n"
				"  Latency min (usec): %.1f\n",
				totalStats.overallLatencyStats.mean,
				sqrt(totalStats.overallLatencyStats.variance),
				totalStats.overallLatencyStats.maxValue,
				totalStats.overallLatencyStats.minValue);
	}
	else
		fprintf( file, "  No latency information was received.\n");

	if (totalStats.genMsgLatencyStats.count)
	{
		fprintf( file,
				"  GenMsg latency avg (usec): %.1f\n"
				"  GenMsg latency std dev (usec): %.1f\n"
				"  GenMsg latency max (usec): %.1f\n"
				"  GenMsg latency min (usec): %.1f\n",
				totalStats.genMsgLatencyStats.mean,
				sqrt(totalStats.genMsgLatencyStats.variance),
				totalStats.genMsgLatencyStats.maxValue,
				totalStats.genMsgLatencyStats.minValue);
	}
	else
		fprintf( file, "  No GenMsg latency information was received.\n");

	if (cpuUsageStats.count)
	{
		fprintf( file,
				"  CPU/Memory samples: %llu\n"
				"  CPU Usage max (%%): %.2f\n"
				"  CPU Usage min (%%): %.2f\n"
				"  CPU Usage avg (%%): %.2f\n"
				"  Memory Usage max (MB): %.2f\n"
				"  Memory Usage min (MB): %.2f\n"
				"  Memory Usage avg (MB): %.2f\n",
				cpuUsageStats.count,
				cpuUsageStats.maxValue * 100.0,
				cpuUsageStats.minValue * 100.0,
				cpuUsageStats.mean * 100.0,
				memUsageStats.maxValue / 1048576.0,
				memUsageStats.minValue / 1048576.0,
				memUsageStats.mean / 1048576.0
			   );
	}
	
	fprintf(file, "\nTest Statistics:\n");

	fprintf(file, 
			"  Requests sent: %llu\n"
			"  Refreshes received: %llu\n"
			"  Updates received: %llu\n",
			totalStats.requestCount.countStatGetTotal(),
			totalStats.refreshCount.countStatGetTotal(),
			totalUpdateCount);
	if (consPerfConfig.postsPerSec)
	{
		fprintf(file, 
				"  Posts sent: %llu\n",
				totalStats.postSentCount.countStatGetTotal());
	}

	if (consPerfConfig.genMsgsPerSec)
		fprintf(file, "  GenMsgs sent: %llu\n", totalStats.genMsgSentCount.countStatGetTotal());
	if (totalStats.genMsgRecvCount.countStatGetTotal())
		fprintf(file, "  GenMsgs received: %llu\n", totalStats.genMsgRecvCount.countStatGetTotal());
	if (consPerfConfig.latencyGenMsgsPerSec)
		fprintf(file, "  GenMsg latencies sent: %llu\n", totalStats.latencyGenMsgSentCount.countStatGetTotal());
	if (totalStats.genMsgLatencyStats.count)
		fprintf(file, "  GenMsg latencies received: %llu\n", totalStats.genMsgLatencyStats.count);

	if (totalStats.imageRetrievalEndTime)
	{
		PerfTimeValue totalRefreshRetrievalTime = (totalStats.imageRetrievalEndTime - 
				totalStats.imageRetrievalStartTime);

		fprintf(file,
				"  Image retrieval time (sec): %.3f\n"
				"  Avg image rate: %.0f\n",
				(double)totalRefreshRetrievalTime/1000000000.0,
				(double)consPerfConfig.itemRequestCount/((double)totalRefreshRetrievalTime/1000000000.0));
	}


	fprintf(file, "  Avg update rate: %.0f\n", 
			(double)totalUpdateCount/(double)((currentTime - firstUpdateTime)/1000000000.0));

	if (totalStats.postSentCount.countStatGetTotal())
	{
		fprintf(file, "  Avg posting rate: %.0f\n", 
				(double) totalStats.postSentCount.countStatGetTotal()/(double)((currentTime - totalStats.imageRetrievalEndTime)/1000000000.0));
	}

	//calculate the first GenMsg Sent & Recv times
	totalStats.firstGenMsgSentTime = consumerThreads[0]->stats.firstGenMsgSentTime;
	totalStats.firstGenMsgRecvTime = consumerThreads[0]->stats.firstGenMsgRecvTime;
	for(i = 1; i < consPerfConfig.threadCount; ++i)
	{
		if(consumerThreads[i]->stats.firstGenMsgSentTime && consumerThreads[i]->stats.firstGenMsgSentTime < totalStats.firstGenMsgSentTime) 
			totalStats.firstGenMsgSentTime = consumerThreads[i]->stats.firstGenMsgSentTime;
		if(consumerThreads[i]->stats.firstGenMsgRecvTime && consumerThreads[i]->stats.firstGenMsgRecvTime < totalStats.firstGenMsgRecvTime)
			totalStats.firstGenMsgRecvTime = consumerThreads[i]->stats.firstGenMsgRecvTime;
	}

	if (consPerfConfig.genMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg send rate: %.0f\n", 
			(double) totalStats.genMsgSentCount.countStatGetTotal()/
			(double)((currentTime - totalStats.firstGenMsgSentTime)/1000000000.0));
	}
	if ( totalStats.genMsgRecvCount.countStatGetTotal())
	{
		fprintf(file, "  Avg GenMsg receive rate: %.0f\n", 
			(double) totalStats.genMsgRecvCount.countStatGetTotal()/
			(double)((currentTime - totalStats.firstGenMsgRecvTime)/1000000000.0));
	}
	if (consPerfConfig.latencyGenMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", 
			(double) totalStats.latencyGenMsgSentCount.countStatGetTotal()/
			(double)((currentTime - totalStats.firstGenMsgSentTime)/1000000000.0));
	}
	if (totalStats.genMsgLatencyStats.count)
	{
		fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", 
			(double)(totalStats.genMsgLatencyStats.count)/
			(double)((currentTime - totalStats.firstGenMsgRecvTime)/1000000000.0));
	}

	fprintf(file, "\n");
}

void EmaCppConsPerf::consumerCleanupThreads()
{
	const UInt64 ctSize = consumerThreads.size();
	UInt64 count =0;
	while(1)
	{
		count = 0;
		for( UInt64 i = 0; i < ctSize; ++i )
		{
			consumerThreads[i]->stopThread = true;
			if( !(consumerThreads[i]->running))				
				count++;
		}
		if( count == ctSize)
			break;
	}

	if (consPerfConfig.threadCount == 1)
		totalStats = consumerThreads[0]->stats;
	else
		collectStats(false, false, 0, 0);

	currentTime = perftool::common::GetTime::getTimeNano();

	printSummaryStatistics(stdout);
	printSummaryStatistics(summaryFile);

	for( UInt64 i = 0; i < ctSize; ++i )
	{
		if( !consumerThreads[i]->testPassed )
		{
			fprintf(stdout, "ERROR: TEST FAILED due to error from %s%d: Location: %s \n",
					BASECONSUMER_NAME, 
					consumerThreads[i]->consumerThreadIndex,
					consumerThreads[i]->failureLocation.c_str());
			fprintf(summaryFile, "ERROR: TEST FAILED due to error from %s%d: Location: %s \n",
					BASECONSUMER_NAME, 
					consumerThreads[i]->consumerThreadIndex,
					consumerThreads[i]->failureLocation.c_str());
		}
		if( consumerThreads[i] )
		{
			delete consumerThreads[i];
			consumerThreads[i] = NULL;
		}
	}

}
bool EmaCppConsPerf::shutdownThreads()
{
	int count = 0;
	const UInt64 ctSize = consumerThreads.size();
	for( UInt64 i = 0; i < ctSize; ++i )
	{
		if( !(consumerThreads[i]->running))
			count++;
	}
		if(count == ctSize)
			return true;
	return false;
}
bool EmaCppConsPerf::inititailizeAndRun( int argc, char *argv[])
{
	if(initConsPerfConfig(argc, argv) == false)
		return false;
	printConsPerfConfig(stdout);
	if( consPerfConfig.mainThreadCpu != -1)
	{
		bindThisThread("Main Thread", consPerfConfig.mainThreadCpu);
		printAllThreadBinding();
	}	

	// If there are multiple connections, determine which items are
	 // to be opened on each connection. 
	 // If any items are common to all connections, they are taken from the first
	 // items in the item list.  The rest of the list is then divided to provide a unique
	 // item list for each connection.
	Int32 itemListUniqueIndex = consPerfConfig.commonItemCount;
	Int32 i = 0;

	for( i = 0; i < consPerfConfig.threadCount; ++i)
	{
		ConsumerThread *pconsumerThread = new ConsumerThread( consPerfConfig );
		consumerThreads.push_back( pconsumerThread );
		consumerThreads[i]->consumerThreadInit(consPerfConfig, i+1);

		// Figure out how many items each consumer should request. 
		consumerThreads[i]->itemListCount = consPerfConfig.itemRequestCount
			/ consPerfConfig.threadCount;

		// Distribute remainder. 
		if (i < consPerfConfig.itemRequestCount % consPerfConfig.threadCount)
			consumerThreads[i]->itemListCount += 1;

		consumerThreads[i]->itemListUniqueIndex = itemListUniqueIndex;
		itemListUniqueIndex += consumerThreads[i]->itemListCount - consPerfConfig.commonItemCount;
		if(consumerThreads[i]->initialize() == false)
			return false;
		consumerThreads[i]->cpuId = consPerfConfig.threadBindList[i];
		consumerThreads[i]->apiThreadCpuId = consPerfConfig.apiThreadBindList[i];
	}

	assert(itemListUniqueIndex == consPerfConfig.itemRequestCount - 
			consPerfConfig.commonItemCount * (consPerfConfig.threadCount - 1));

	if (!(summaryFile = fopen(consPerfConfig.summaryFilename.c_str(), "w")))
	{
		logText = "Error: Failed to open file '";
		logText += consPerfConfig.summaryFilename;
		logText += "'.";
		AppUtil::logError(logText);
		return false;
	}

	printConsPerfConfig(summaryFile); fflush(summaryFile);

	// Reset resource usage. 
	if (resourceStats.initResourceUsageStats() == false)
	{
		logText = "initResourceUsageStats() failed:";
		AppUtil::logError(logText);
		consumerCleanupThreads();
		return false;
	}

	// Spawn consumer threads 
	EmaString consumerThreadName;

	const UInt64 ctSize = consumerThreads.size();
	for( i = 0; i < ctSize; ++i )
	{
		consumerThreadName = BASECONSUMER_NAME;
		if( consumerThreads[i]->cpuId != -1)
			firstThreadSnapshot();

		consumerThreads[i]->start();
		if(consumerThreads[i]->cpuId != -1)
		{
			consumerThreadName += consumerThreads[i]->consumerThreadIndex;
			AppUtil::sleep( 1000 );
			secondThreadSnapshot(consumerThreadName, consumerThreads[i]->cpuId);
			printAllThreadBinding();
		}		
	}

	UInt32 currentRuntimeSec = 0;
	UInt32 intervalSeconds = 0;

	endTime = perftool::common::GetTime::getTimeMilli() + consPerfConfig.steadyStateTime * 1000;
	

	startTime = perftool::common::GetTime::getTimeMilli();

	// Sleep for one more second so some stats can be gathered before first printout.
	AppUtil::sleep( 1000 );
	while ( !shutdownThreads() )
	{		
		currentTime = perftool::common::GetTime::getTimeMilli();
		++currentRuntimeSec;
		++intervalSeconds;


		if (intervalSeconds == consPerfConfig.writeStatsInterval)
		{
			collectStats(true, consPerfConfig.displayStats, 
					currentRuntimeSec, consPerfConfig.writeStatsInterval);
			intervalSeconds = 0;
		}

		if (totalStats.imageRetrievalEndTime && consPerfConfig.requestSnapshots)
		{
			AppUtil::log("Received all images for snapshot test.\n");
			break;
		}

		if(currentTime >= endTime)
		{
			if (!totalStats.imageRetrievalEndTime)
			{
				logText = "Error: Failed to receive all images within ";
				logText += consPerfConfig.steadyStateTime;
				logText += "seconds.";
				AppUtil::logError(logText);
			}
			else
				AppUtil::log("\nSteady state time of %u seconds has expired.\n", consPerfConfig.steadyStateTime);

			break;
		}
		if(CtrlBreakHandler::isTerminated() )
			break;

		nextTime = currentTime + 1000;
		AppUtil::sleep( nextTime - currentTime );	
	}

	consumerCleanupThreads();

	return true;
}

void EmaCppConsPerf::collectStats(bool writeStats, bool displayStats, UInt32 currentRuntimeSec, 
		UInt32 timePassedSec)
{
	Int32 i;
	bool allRefreshesRetrieved = false;
	LatencyRecords *pUpdateLateList = NULL;
	// LatencyRecords	*pPostLateList = NULL;

	if (timePassedSec)
	{
		if (resourceStats.getResourceUsageStats() == false)
		{
			logText = "getResourceUsageStats() failed:";
			AppUtil::logError(logText);
			return;
		}
		cpuUsageStats.updateValueStatistics( (double)resourceStats.cpuUsageFraction );
		memUsageStats.updateValueStatistics( (double)resourceStats.memUsageBytes );
	}

	for(i = 0; i < consPerfConfig.threadCount; i++)
	{
		UInt64 refreshCount,
				   startupUpdateCount,
				   steadyStateUpdateCount,
				   requestCount,
				   statusCount,
				   postSentCount,
				   postOutOfBuffersCount,
				   genMsgSentCount,
				   genMsgRecvCount,
				   latencyGenMsgSentCount,
				   latencyGenMsgRecvCount,
				   genMsgOutOfBuffersCount;

		// Gather latency records from each thread and update statistics.

		consumerThreads[i]->getLatencyTimeRecords(&pUpdateLateList);
		UInt64 updateLateListSize = (pUpdateLateList == NULL ) ? 0 : pUpdateLateList->size();
		for (UInt64 l = 0; l  < updateLateListSize; ++l)
		{
			TimeRecord *pRecord = &(*pUpdateLateList)[l];
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;
			double recordEndTimeNsec = (double)pRecord->endTime/(double)pRecord->ticks * 1000.0;

			// Make sure this latency is counted towards startup or steady-state as appropriate.
			bool latencyIsSteadyStateForClient = 
					consumerThreads[i]->stats.imageRetrievalEndTime != 0
					&& recordEndTimeNsec > (double)consumerThreads[i]->stats.imageRetrievalEndTime;

			consumerThreads[i]->stats.intervalLatencyStats.updateValueStatistics( latency);
			consumerThreads[i]->stats.overallLatencyStats.updateValueStatistics( latency);
			if( latencyIsSteadyStateForClient )
				consumerThreads[i]->stats.steadyStateLatencyStats.updateValueStatistics( latency );
			else
				consumerThreads[i]->stats.startupLatencyStats.updateValueStatistics( latency );
	
			if (consPerfConfig.threadCount > 1)
			{
				// Make sure this latency is counted towards startup or steady-state as appropriate. 
				bool latencyIsSteadyStateOverall = 
					totalStats.imageRetrievalEndTime != 0
					&& recordEndTimeNsec > (double)totalStats.imageRetrievalEndTime;

				if( latencyIsSteadyStateOverall ) 
					totalStats.steadyStateLatencyStats.updateValueStatistics( latency );
				else
					totalStats.startupLatencyStats.updateValueStatistics( latency );
				totalStats.overallLatencyStats.updateValueStatistics( latency);
			}

			if (consumerThreads[i]->latencyLogFile)
				fprintf(consumerThreads[i]->latencyLogFile, "Upd, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
		}
		if (pUpdateLateList )
			consumerThreads[i]->clearReadLatTimeRecords( pUpdateLateList );

		if (consumerThreads[i]->latencyLogFile)
			fflush(consumerThreads[i]->latencyLogFile);

		// Collect counts.
		startupUpdateCount =  consumerThreads[i]->stats.startupUpdateCount.countStatGetChange() ;
		steadyStateUpdateCount = consumerThreads[i]->stats.steadyStateUpdateCount.countStatGetChange();
		statusCount = consumerThreads[i]->stats.statusCount.countStatGetChange();
		requestCount = consumerThreads[i]->stats.requestCount.countStatGetChange();
		refreshCount = consumerThreads[i]->stats.refreshCount.countStatGetChange();
		postSentCount = consumerThreads[i]->stats.postSentCount.countStatGetChange();
		postOutOfBuffersCount = consumerThreads[i]->stats.postOutOfBuffersCount.countStatGetChange();
		genMsgSentCount = consumerThreads[i]->stats.genMsgSentCount.countStatGetChange();
		genMsgRecvCount = consumerThreads[i]->stats.genMsgRecvCount.countStatGetChange();
		latencyGenMsgSentCount = consumerThreads[i]->stats.latencyGenMsgSentCount.countStatGetChange();
		latencyGenMsgRecvCount = consumerThreads[i]->stats.intervalGenMsgLatencyStats.count;
		genMsgOutOfBuffersCount = consumerThreads[i]->stats.genMsgOutOfBuffersCount.countStatGetChange();

		if (consPerfConfig.threadCount > 1)
		{
			totalStats.startupUpdateCount.countStatAdd( startupUpdateCount);
			totalStats.steadyStateUpdateCount.countStatAdd( steadyStateUpdateCount);
			totalStats.statusCount.countStatAdd( statusCount);
			totalStats.requestCount.countStatAdd( requestCount);
			totalStats.refreshCount.countStatAdd( refreshCount);
			totalStats.postSentCount.countStatAdd( postSentCount);
			totalStats.postOutOfBuffersCount.countStatAdd( postOutOfBuffersCount); 
			totalStats.genMsgSentCount.countStatAdd( genMsgSentCount);
			totalStats.genMsgRecvCount.countStatAdd( genMsgRecvCount);
			totalStats.latencyGenMsgSentCount.countStatAdd( latencyGenMsgSentCount);
			totalStats.genMsgOutOfBuffersCount.countStatAdd( genMsgOutOfBuffersCount);
		}
		
		if (writeStats)
		{
			/* Log statistics to file. */
			AppUtil::printCurrentTimeUTC(consumerThreads[i]->statsFile);
			fprintf(consumerThreads[i]->statsFile,
					", %llu, %.1f, %.1f, %.1f, %.1f, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %llu, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n",
					consumerThreads[i]->stats.intervalLatencyStats.count,
					consumerThreads[i]->stats.intervalLatencyStats.mean,
					sqrt(consumerThreads[i]->stats.intervalLatencyStats.variance), consumerThreads[i]->stats.intervalLatencyStats.count ?	consumerThreads[i]->stats.intervalLatencyStats.maxValue : 0.0,
					consumerThreads[i]->stats.intervalLatencyStats.count ? consumerThreads[i]->stats.intervalLatencyStats.minValue : 0.0,
					refreshCount,
					(startupUpdateCount + steadyStateUpdateCount)/timePassedSec,
					consumerThreads[i]->stats.intervalPostLatencyStats.count,
					consumerThreads[i]->stats.intervalPostLatencyStats.mean,
					sqrt(consumerThreads[i]->stats.intervalPostLatencyStats.variance),
					consumerThreads[i]->stats.intervalPostLatencyStats.count ? consumerThreads[i]->stats.intervalPostLatencyStats.maxValue : 0.0,
					consumerThreads[i]->stats.intervalPostLatencyStats.count ? consumerThreads[i]->stats.intervalPostLatencyStats.minValue : 0.0,
					genMsgSentCount,
					genMsgRecvCount,
					latencyGenMsgSentCount,
					latencyGenMsgRecvCount,
					consumerThreads[i]->stats.intervalGenMsgLatencyStats.mean,
					sqrt(consumerThreads[i]->stats.intervalGenMsgLatencyStats.variance),
					latencyGenMsgRecvCount ? consumerThreads[i]->stats.intervalGenMsgLatencyStats.maxValue : 0.0,
					latencyGenMsgRecvCount ? consumerThreads[i]->stats.intervalGenMsgLatencyStats.minValue : 0.0,
					resourceStats.cpuUsageFraction * 100.0,
					(double)resourceStats.memUsageBytes / 1048576.0);
			fflush(consumerThreads[i]->statsFile);
		}

		if (displayStats)
		{
			if (consPerfConfig.threadCount == 1)
				printf("%03u: ", currentRuntimeSec);
			else
				printf("%03u: Client %d:\n  ", currentRuntimeSec, i + 1);

			printf("Images: %6llu, Posts: %6llu, UpdRate: %8llu, CPU: %6.2f%%, Mem: %6.2fMB\n", 
					refreshCount,
					postSentCount,
					(startupUpdateCount + steadyStateUpdateCount)/timePassedSec,
					resourceStats.cpuUsageFraction * 100.0,
					(double)resourceStats.memUsageBytes / 1048576.0);

			if (consumerThreads[i]->stats.intervalLatencyStats.count > 0)
			{
				consumerThreads[i]->stats.intervalLatencyStats.printValueStatistics(stdout, "  Latency(usec)", "Msgs",  false);
				consumerThreads[i]->stats.intervalLatencyStats.clearValueStatistics();
			}

			if (postOutOfBuffersCount)
				printf("  - %llu posts not sent due to lack of output buffers.\n", postOutOfBuffersCount);

			if (consumerThreads[i]->stats.intervalPostLatencyStats.count > 0)
			{
				consumerThreads[i]->stats.intervalPostLatencyStats.printValueStatistics(stdout, "  PostLat(usec)", "Msgs",  false);
				consumerThreads[i]->stats.intervalPostLatencyStats.clearValueStatistics();
			}

			if (genMsgSentCount || genMsgRecvCount)
				printf("  GenMsgs: sent %llu, received %llu, latencies sent %llu, latencies received %llu\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
			if (genMsgOutOfBuffersCount)
				printf("  - %llu gen msgs not sent due to lack of output buffers.\n", genMsgOutOfBuffersCount);

			if (consumerThreads[i]->stats.intervalGenMsgLatencyStats.count > 0)
			{
				consumerThreads[i]->stats.intervalGenMsgLatencyStats.printValueStatistics(stdout, "  GenMsgLat(usec)", "Msgs", false);
				consumerThreads[i]->stats.intervalGenMsgLatencyStats.clearValueStatistics();
			}

			if (statusCount)
				printf("  - Received %llu status messages.\n", statusCount);
		}

		// Get Image Retrieval time for this client.
		if (!totalStats.imageTimeRecorded)
		{
			if (consumerThreads[i]->stats.imageRetrievalEndTime)
			{
				PerfTimeValue imageRetrievalStartTime = consumerThreads[i]->stats.imageRetrievalStartTime,
						  imageRetrievalEndTime = consumerThreads[i]->stats.imageRetrievalEndTime;

				// To get the total time it took to retrieve all images, find the earliest start time
				// and latest end time across all connections. 
				if (!totalStats.imageRetrievalStartTime 
						|| imageRetrievalStartTime < totalStats.imageRetrievalStartTime)
					totalStats.imageRetrievalStartTime = imageRetrievalStartTime;
				if (!totalStats.imageRetrievalEndTime || 
						imageRetrievalEndTime > totalStats.imageRetrievalEndTime)
					totalStats.imageRetrievalEndTime = imageRetrievalEndTime; 

			}
			// Ignore connections that don't request anything. 
			else if (consumerThreads[i]->itemListCount > 0)
			{
				allRefreshesRetrieved = false; // Not all connections have received their images yet.
				totalStats.imageRetrievalStartTime = 0;
				totalStats.imageRetrievalEndTime = 0;
			}
		}

		if (!consumerThreads[i]->stats.imageTimeRecorded && consumerThreads[i]->stats.imageRetrievalEndTime)
		{
			consumerThreads[i]->stats.imageTimeRecorded = true;

			if (displayStats)
			{
				PerfTimeValue imageRetrievalTime = consumerThreads[i]->stats.imageRetrievalEndTime - 
					consumerThreads[i]->stats.imageRetrievalStartTime;

				printf("  - Image retrieval time for %d images: %.3fs (%.0f images/s)\n", 
						consumerThreads[i]->itemListCount,
						(double)imageRetrievalTime/1000000000.0,
						(double)(consumerThreads[i]->itemListCount)/
						((double)imageRetrievalTime /1000000000.0));
			}
		}
	}

	if (!totalStats.imageTimeRecorded && allRefreshesRetrieved)
	{
		endTime = totalStats.imageRetrievalEndTime + consPerfConfig.steadyStateTime * 1000000000ULL;
		totalStats.imageTimeRecorded = true;

		if (consPerfConfig.threadCount > 1)
		{
			if (displayStats)
			{
				/* Print overall image retrieval stats. */
				PerfTimeValue totalRefreshRetrievalTime = (totalStats.imageRetrievalEndTime - 
						totalStats.imageRetrievalStartTime);

				printf("\nOverall image retrieval time for %d images: %.3fs (%.0f Images/s).\n\n", 
						consPerfConfig.itemRequestCount,
						(double)totalRefreshRetrievalTime/1000000000.0,
						(double)(consPerfConfig.itemRequestCount)/
						((double)totalRefreshRetrievalTime /1000000000.0)
					  );
			}

		}
		else
		{
			totalStats.imageRetrievalStartTime = consumerThreads[0]->stats.imageRetrievalStartTime;
			totalStats.imageRetrievalEndTime = consumerThreads[0]->stats.imageRetrievalEndTime;
		}
	}
}

int main( int argc, char* argv[] )
{
	
	EmaCppConsPerf emaConsumerPerf ;
	// If there are multiple connections, determine which items are
	 // to be opened on each connection. 
	 // If any items are common to all connections, they are taken from the first
	 // items in the item list.  The rest of the list is then divided to provide a unique
	 // item list for each connection.
	emaConsumerPerf.inititailizeAndRun( argc, argv );

	return 0;
}
