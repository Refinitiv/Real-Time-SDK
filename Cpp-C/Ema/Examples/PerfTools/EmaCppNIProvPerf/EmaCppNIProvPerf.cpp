///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|        Copyright (C) 2021-2022 Refinitiv.    All rights reserved.         --
///*|-----------------------------------------------------------------------------

#include "EmaCppNIProvPerf.h"
#include "AppUtil.h"
#include "CtrlBreakHandler.h"
#include "ThreadBinding.h"
#include "xmlMsgDataParser.h"

#include <iostream>
#include <cstring>
#include <cmath>

using namespace refinitiv::ema::access;
using namespace perftool::common;
using namespace std;

EmaString EmaCppNIProvPerf::logText = "";

void EmaCppNIProvPerf::exitOnMissingArgument(char** argv, int argPos)
{
	logText = "Config error: ";
	logText += argv[argPos];
	logText += " missing argument.\n";
	logText += "Run '";
	logText += argv[0];
	logText += " -?' to see usage.\n";

	AppUtil::logError(logText);
	return;
}

void EmaCppNIProvPerf::exitConfigError(char** argv)
{
	logText = "Run '";
	logText += argv[0];
	logText += " -?' to see usage.\n";

	AppUtil::logError(logText);
	return;
}

void EmaCppNIProvPerf::exitWithUsage()
{
	logText = "Options:\n";
	logText += "  -?                                   Shows this usage.\n\n";

	logText += "  -providerName <name>                 Name of the NiProvider component in config file EmaConfig.xml that will be used to configure connection.\n\n";

	logText += "  -itemCount <count>                   Number of items to publish.\n";
	logText += "  -tickRate <ticks per second>         Ticks per second.\n";
	logText += "  -updateRate <updates per second>     Update rate per second.\n";
	logText += "  -latencyUpdateRate <updates/sec>     Latency update rate(can specify \"all\" to send latency in every update).\n";
	logText += "  -refreshBurstSize <count>            Number of refreshes to send in a burst(controls granularity of time-checking).\n\n";

	logText += "  -serviceName <name>                  Service Name of the provider\n";
	logText += "  -serviceId <ID>                      Service ID of the provider\n\n";

	logText += "  -itemFile <file name>                Name of the file to get items from for publishing.\n";
	logText += "  -msgFile <file name>                 Name of the file that specifies the data content in messages.\n";
	logText += "  -summaryFile <filename>              Name of file for logging summary info.\n";
	logText += "  -statsFile <filename>                Base name of file for logging periodic statistics.\n";
	logText += "  -writeStatsInterval <sec>            Controls how often stats are written to the file.\n";
	logText += "  -noDisplayStats                      Stop printout of stats to screen.\n\n";

	logText += "  -runTime <sec>                       Runtime of the application, in seconds.\n\n";

	logText += "  -mainThread <CpuId>                  CPU of the application's main thread that collects & prints stats.\n";
	logText += "  -threads <thread list>               List of threads, by their bound CPU. Comma-separated list. -1 means do not bind.\n";
	logText += "                                        (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1).\n\n";
	logText += "  -apiThreads <thread list>            List of Api threads in ApiDispatch mode, by their bound CPU.\n";
	logText += "                                        Comma-separated list. -1 means do not bind.\n";
	logText += "                                        Must match the count of listed in -threads option.\n";
	logText += "                                        (e.g. \"-apiThreads 0,1\" creates two threads bound to CPU's 0 and 1)\n";
	logText += "  -workerThreads <thread list>         List of Reactor worker threads, by their bound CPU.\n";
	logText += "                                        Comma-separated list. -1 means do not bind.\n";
	logText += "                                        Must match the count of listed in -threads option.\n";
	logText += "                                        (e.g. \"-workerThreads 0,1\" creates two threads bound to CPU's 0 and 1)\n\n";

	logText += "  -useUserDispatch <1 Or 0>            Value 1 will use UserDispatch, 0 will use ApiDispatch.\n";
	logText += "  -preEnc                              Use Pre-Encoded updates.\n";
	logText += "  -measureEncode                       Measure encoding time of messages.\n";
	logText += "  -measureDecode                       Measure dencoding time of messages.\n";
	logText += "  -nanoTime                            Use nanosecond precision for latency information instead of microsecond.\n";

	AppUtil::logError(logText);
	return;
}

bool EmaCppNIProvPerf::initNIProvPerfConfig(int argc, char* argv[])
{
	int iargs = 1;
	Int32 provThreadCount = 0;

	while (iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			exitWithUsage();
			return false;
		}

		else if (0 == strcmp("-runTime", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.runTime = atoi(argv[iargs++]);
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.writeStatsInterval = atoi(argv[iargs++]);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			++iargs;
			niProvPerfConfig.displayStats = false;
		}
		else if (strcmp("-preEnc", argv[iargs]) == 0)
		{
			++iargs;
			niProvPerfConfig.preEncItems = true;
		}
		else if (strcmp("-nanoTime", argv[iargs]) == 0)
		{
			++iargs;
			niProvPerfConfig.nanoTime = true;
		}
		else if (strcmp("-measureEncode", argv[iargs]) == 0)
		{
			++iargs;
			niProvPerfConfig.measureEncode = true;
		}
		else if (strcmp("-measureDecode", argv[iargs]) == 0)
		{
			++iargs;
			niProvPerfConfig.measureDecode = true;
		}

		else if (strcmp("-refreshBurstSize", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.refreshBurstSize = atoi(argv[iargs++]);
		}
		else if (strcmp("-tickRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.ticksPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-updateRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.updatesPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-latencyUpdateRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			if (0 == strcmp("all", argv[iargs]))
				niProvPerfConfig.latencyUpdatesPerSec = ALWAYS_SEND_LATENCY_UPDATE;
			else
				niProvPerfConfig.latencyUpdatesPerSec = atoi(argv[iargs]);
			++iargs;
		}

		else if (strcmp("-serviceName", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.serviceName = argv[iargs++];
		}
		else if (strcmp("-serviceId", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.useServiceId = true;
			niProvPerfConfig.serviceId = atoi(argv[iargs++]);
		}

		else if (strcmp("-mainThread", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.mainThreadCpu = argv[iargs++];
		}
		else if (strcmp("-threads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			niProvPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++niProvPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				niProvPerfConfig.threadBindList[niProvPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && niProvPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = niProvPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				niProvPerfConfig.threadBindList[i].clear();

			provThreadCount = (niProvPerfConfig.threadCount > 0) ? niProvPerfConfig.threadCount : 0;
		}
		else if (strcmp("-apiThreads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			niProvPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++niProvPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many api threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				niProvPerfConfig.apiThreadBindList[niProvPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && niProvPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = niProvPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				niProvPerfConfig.apiThreadBindList[i].clear();

			provThreadCount = (niProvPerfConfig.threadCount > 0) ? niProvPerfConfig.threadCount : 0;
		}
		else if (strcmp("-workerThreads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			niProvPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++niProvPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many worker threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				niProvPerfConfig.workerThreadBindList[niProvPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && niProvPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = niProvPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				niProvPerfConfig.workerThreadBindList[i].clear();

			provThreadCount = (niProvPerfConfig.threadCount > 0) ? niProvPerfConfig.threadCount : 0;
		}
		else if (0 == strcmp("-useUserDispatch", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			int i = atoi(argv[iargs++]);
			niProvPerfConfig.useUserDispatch = (i == 1) ? true : false;
		}
		else if (0 == strcmp("-itemCount", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.itemCount = atoi(argv[iargs++]);
		}

		else if (0 == strcmp("-summaryFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.summaryFilename = argv[iargs++];
		}
		else if (0 == strcmp("-itemFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.itemFilename = argv[iargs++];
		}
		else if (0 == strcmp("-msgFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.msgFilename = argv[iargs++];
		}
		else if (0 == strcmp("-statsFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.statsFilename = argv[iargs++];
		}

		else if (0 == strcmp("-providerName", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			niProvPerfConfig.providerName = argv[iargs++];
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

	// set Position for login
	//
	// The Data Access Control System position. If the
	// server is authenticating with the Data Access
	// Control System, the consumer application might
	// need to pass in a valid position.
	// If present, this
	// should match whatever was sent in the request or
	// be set to the IP address of the connected client.
	//
	refinitiv::ema::domain::login::Login::LoginReq loginReq;
	if (loginReq.hasPosition())
	{
		niProvPerfConfig.loginPosition = loginReq.getPosition();
	}
	else {
		niProvPerfConfig.loginPosition.set("localhost");
	}

	/* Conditions */

	if (niProvPerfConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.writeStatsInterval < 1)
	{
		printf("Config error: Write Stats Interval cannot be less than 1.\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.latencyUpdatesPerSec > niProvPerfConfig.updatesPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total update rate.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.latencyUpdatesPerSec > niProvPerfConfig.ticksPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total ticks per second.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE
		&& niProvPerfConfig.preEncItems)
	{
		printf("Config Error: -preEnc has no effect when always sending latency update, since it must be encoded.\n\n");
		exitConfigError(argv);
		return false;
	}
	if (niProvPerfConfig.latencyUpdatesPerSec == 0 && niProvPerfConfig.measureEncode)
	{
		printf("Config Error: Measuring message encoding time when latency update rate is zero. Message encoding time is only recorded for latency updates.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.updatesPerSec != 0 && niProvPerfConfig.updatesPerSec < niProvPerfConfig.ticksPerSec)
	{
		printf("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
		exitConfigError(argv);
		return false;
	}

	if (niProvPerfConfig.threadCount == 0)
	{
		AppUtil::logError("Config Error: -threads must be contain list of Cpu.");
		exitConfigError(argv);
		return false;
	}

	return true;
}


void EmaCppNIProvPerf::printNIProvPerfConfig(FILE* file)
{
	char threadListStr[128] = "-1";
	char apiThreadListStr[128] = "-1";
	char workerThreadListStr[128] = "-1";
	EmaString providerName;

	// Build thread list 
	niProvPerfConfig.getThreadListAsString(niProvPerfConfig.threadBindList, threadListStr, 128);
	niProvPerfConfig.getThreadListAsString(niProvPerfConfig.workerThreadBindList, workerThreadListStr, 128);
	if ( !niProvPerfConfig.useUserDispatch )
		niProvPerfConfig.getThreadListAsString(niProvPerfConfig.apiThreadBindList, apiThreadListStr, 128);

	if (!niProvPerfConfig.providerName.empty())
		providerName = niProvPerfConfig.providerName;
	else
		providerName = providerThreadNameBase + "1";

	fprintf(file, "--- TEST INPUTS ---\n\n");

	fprintf(file,
		"                Run Time: %u\n"
		"           Provider Name: %s\n"
		"         useUserDispatch: %s\n"
		"        mainThread CpuId: %s\n"
		"             Thread List: %s\n"
		"         Api thread List: %s\n"
		"      Worker thread List: %s\n"
		"            Summary File: %s\n"
		"              Stats File: %s\n"
		"    Write Stats Interval: %u\n"
		"           Display Stats: %s\n"
		,
		niProvPerfConfig.runTime,
		providerName.c_str(),
		niProvPerfConfig.useUserDispatch ? "Yes" : "No",
		niProvPerfConfig.mainThreadCpu.empty() ? "-1" : niProvPerfConfig.mainThreadCpu.c_str(),
		threadListStr,
		apiThreadListStr,
		workerThreadListStr,
		niProvPerfConfig.summaryFilename.c_str(),
		niProvPerfConfig.statsFilename.c_str(),
		niProvPerfConfig.writeStatsInterval,
		niProvPerfConfig.displayStats ? "Yes" : "No"
	);

	fprintf(file,
		"              Item Count: %d\n"
		"               Tick Rate: %d\n"
		"             Update Rate: %d\n"
		"     Latency Update Rate: %d\n"
		"      Refresh Burst Size: %d\n"
		"               Item File: %s\n"
		"               Data File: %s\n",

		niProvPerfConfig.itemCount,
		niProvPerfConfig.ticksPerSec,
		niProvPerfConfig.updatesPerSec,
		niProvPerfConfig.latencyUpdatesPerSec >= 0 ? niProvPerfConfig.latencyUpdatesPerSec : niProvPerfConfig.updatesPerSec,
		niProvPerfConfig.refreshBurstSize,
		niProvPerfConfig.itemFilename.c_str(),
		niProvPerfConfig.msgFilename.c_str());

	if (niProvPerfConfig.useServiceId)
	{
		fprintf(file,
		"              Service ID: %lu\n",
		niProvPerfConfig.serviceId);
	}

	fprintf(file,
		"            Service Name: %s\n"
		"     Pre-Encoded Updates: %s\n"
		"         Nanosecond Time: %s\n"
		"          Measure Encode: %s\n",
		niProvPerfConfig.serviceName.c_str(),
		niProvPerfConfig.preEncItems ? "Yes" : "No",
		niProvPerfConfig.nanoTime ? "Yes" : "No",
		niProvPerfConfig.measureEncode ? "Yes" : "No");

	return;
}

bool EmaCppNIProvPerf::inititailizeAndRun(int argc, char* argv[])
{
	if (initNIProvPerfConfig(argc, argv) == false)
		return false;
	printNIProvPerfConfig(stdout);

	if (!(summaryFile = fopen(niProvPerfConfig.summaryFilename, "w")))
	{
		logText = "Error: Failed to open file '";
		logText += niProvPerfConfig.summaryFilename;
		logText += "'.";
		AppUtil::logError(logText);
		exit(-1);
	}

	printNIProvPerfConfig(summaryFile); fflush(summaryFile);


	if ( !niProvPerfConfig.mainThreadCpu.empty() && !niProvPerfConfig.mainThreadCpu.caseInsensitiveCompare("-1") )
	{
		RsslError rsslError;
		RsslRet retCode = rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);
		if ( !bindThisThread("Main Thread", niProvPerfConfig.mainThreadCpu) )
		{
			rsslUninitialize();
			return false;
		}
		printAllThreadBinding();
	}

	// Initializes pre-encoded messages data, helpers to access to template messages file.
	perfMessageData = new PerfMessageData(niProvPerfConfig.msgFilename, niProvPerfConfig.preEncItems);

	// Creates and initializes threads
	Int32 i;

	UInt32 itemListUniqueIndex = 0;
	UInt32 itemListCountBase = niProvPerfConfig.itemCount / niProvPerfConfig.threadCount;
	Int32 itemListCountRemainder = niProvPerfConfig.itemCount % niProvPerfConfig.threadCount;
	UInt32 itemListCount;

	for (i = 0; i < niProvPerfConfig.threadCount; ++i)
	{
		NIProviderThread* pProviderThread = new NIProviderThread(niProvPerfConfig, perfMessageData, (i+1));
		providerThreads.push_back(pProviderThread);

		pProviderThread->providerThreadInit();

		pProviderThread->setCpuId(niProvPerfConfig.threadBindList[i]);
		pProviderThread->setApiThreadCpuId(niProvPerfConfig.apiThreadBindList[i]);
		pProviderThread->setWorkerThreadCpuId(niProvPerfConfig.workerThreadBindList[i]);

		// initialize item's lists by template file (350.xml)
		itemListCount = itemListCountBase;
		if (i < itemListCountRemainder)
			itemListCount++;

		pProviderThread->providerThreadInitItems(itemListCount, itemListUniqueIndex);

		itemListUniqueIndex += itemListCount;
	}

	// Spawn threads and assigns to specific CPU
	const UInt64 ptSize = providerThreads.size();
	for (i = 0; i < ptSize; ++i)
	{
		// Start the thread
		providerThreads[i]->start();
	}

	// Registers Ctrl+C handler in the handler's chain after EMA.
	// OmmBaseImplMap is used to uninitilalize for internal EMA objects.
	AppUtil::sleep(20);
	CtrlBreakHandler::registerAction();

	// Main application thread: collect and print statistics
	monitorStatistics();

	cleanupThreads();
	return true;
}

bool EmaCppNIProvPerf::shutdownThreads()
{
	UInt64 count = 0;
	const UInt64 ctSize = providerThreads.size();
	for (UInt64 i = 0; i < ctSize; ++i)
	{
		NIProviderThread* pProviderThread = providerThreads[i];
		if (!pProviderThread->isRunning() || pProviderThread->isStopped())
			count++;
	}
	if (count == ctSize)
		return true;
	return false;
}

void EmaCppNIProvPerf::cleanupThreads()
{
	printf("\ncleanupThreads.\n\n");

	// Waits until all threads stop executing
	const UInt64 ctSize = providerThreads.size();
	UInt64 i;

	if (!shutdownThreads())
	{
		for (i = 0; i < ctSize; ++i)
		{
			providerThreads[i]->setStopThread();
		}

		PerfTimeValue startTime = perftool::common::GetTime::getTimeMilli();
		PerfTimeValue currentTime = startTime;
		
		bool threadRunning = true;
		while (threadRunning && currentTime < startTime + 5000)
		{
			AppUtil::sleep(200);

			threadRunning = false;
			for (i = 0; i < ctSize; ++i)
			{
				if (providerThreads[i]->isRunning()) {
					threadRunning = true;
					break;
				}
			}
			currentTime = perftool::common::GetTime::getTimeMilli();
		}
	}

	/* Collect final stats before writing summary. */
	collectStats(RSSL_FALSE, RSSL_FALSE, 0, 0);

	providerPrintSummaryStats(stdout);
	providerPrintSummaryStats(summaryFile);
	fclose(summaryFile);

	for (i = 0; i < ctSize; ++i)
	{
		delete providerThreads[i];
		providerThreads[i] = NULL;
	}

	if (perfMessageData != NULL)
		delete perfMessageData;
	perfMessageData = NULL;
}

bool EmaCppNIProvPerf::monitorStatistics()
{
	PerfTimeValue startTime = perftool::common::GetTime::getTimeMilli();
	PerfTimeValue endProviderRunTime = startTime + niProvPerfConfig.runTime * 1000ULL;

	PerfTimeValue currentTime = startTime;

	const UInt64 writeStatsIntervalMs = niProvPerfConfig.writeStatsInterval * 1000ULL;

	UInt64 nextTimeMs = startTime + writeStatsIntervalMs;

	UInt64 sleepIntervalMs = 0;

	// Reset resource usage. 
	if (resourceStats.initResourceUsageStats() == false)
	{
		logText = "initResourceUsageStats() failed:";
		AppUtil::logError(logText);
		return false;
	}

	// Sleep for one more second so some stats can be gathered before first printout.
	AppUtil::sleep(1000);

	// Main loop: gathering statistics
	while (!shutdownThreads())
	{
		currentTime = perftool::common::GetTime::getTimeMilli();

		if (currentTime >= nextTimeMs)
		{
			nextTimeMs += writeStatsIntervalMs;

			if (!collectStats(true, niProvPerfConfig.displayStats,
				(UInt32)((currentTime - startTime) / 1000), niProvPerfConfig.writeStatsInterval))
			{
				break;
			}

			currentTime = perftool::common::GetTime::getTimeMilli();
		}

		if (currentTime >= endProviderRunTime)
		{
			AppUtil::log("\nRun time of %u seconds has expired.\n", niProvPerfConfig.runTime);
			break;
		}

		sleepIntervalMs = (nextTimeMs > currentTime ? (nextTimeMs - currentTime) : 0U);
		AppUtil::sleepUI(sleepIntervalMs);

		if (CtrlBreakHandler::isTerminated())
			break;
	}

	return true;
}

bool EmaCppNIProvPerf::collectStats(bool writeStats, bool displayStats, UInt32 currentRuntimeSec,
	UInt32 timePassedSec)
{
	UInt64 refreshMsgCount,
		updateMsgCount,
		itemRequestCount,
		closeMsgCount,
		statusCount;

	const double OneMegaByte = 1048576.0;

	if (timePassedSec)
	{
		if (resourceStats.getResourceUsageStats() == false)
		{
			logText = "getResourceUsageStats() failed:";
			AppUtil::logError(logText);
			return false;
		}
		cpuUsageStats.updateValueStatistics( resourceStats.cpuUsageFraction );
		memUsageStats.updateValueStatistics( (double)resourceStats.memUsageBytes );
	}

	for (Int32 i = 0; i < niProvPerfConfig.threadCount; ++i)
	{
		NIProviderThread* providerThread = providerThreads[i];
		ProviderStats& stats = providerThread->getProviderStats();

		// Collect counts.
		refreshMsgCount = stats.refreshMsgCount.countStatGetChange();
		updateMsgCount = stats.updateMsgCount.countStatGetChange();
		itemRequestCount = stats.itemRequestCount.countStatGetChange();
		closeMsgCount = stats.closeMsgCount.countStatGetChange();
		statusCount = stats.statusCount.countStatGetChange();

		if (niProvPerfConfig.measureEncode)
		{
			LatencyRecords* pUpdateEncodedMeasurements = NULL;
			UInt64 updateEncListSize = 0;

			stats.messageEncodeTimeRecords.getLatencyTimeRecords(&pUpdateEncodedMeasurements);
			updateEncListSize = (pUpdateEncodedMeasurements == NULL) ? 0 : pUpdateEncodedMeasurements->size();
			for (UInt64 l = 0; l < updateEncListSize; ++l)
			{
				TimeRecord* pRecord = &(*pUpdateEncodedMeasurements)[l];
				double encodingTime = (double)(pRecord->endTime - pRecord->startTime) / (double)pRecord->ticks;

				stats.intervalMsgEncodingStats.updateValueStatistics(encodingTime);
			}

			stats.messageEncodeTimeRecords.clearReadLatTimeRecords();
		}

		/* Add the new counts to the provider's total. */
		totalStats.refreshMsgCount.countStatAdd(refreshMsgCount);
		totalStats.updateMsgCount.countStatAdd(updateMsgCount);
		totalStats.itemRequestCount.countStatAdd(itemRequestCount);
		totalStats.closeMsgCount.countStatAdd(closeMsgCount);
		totalStats.statusCount.countStatAdd(statusCount);

		if (writeStats)
		{
			/* Write stats to the stats file. */
			FILE* statsFile = providerThread->getStatsFile();
			printCurrentTimeUTC(statsFile);

//							case PROVIDER_NONINTERACTIVE:
			fprintf(statsFile, ", %llu, %llu, %.2f, %.2f\n",
				refreshMsgCount,
				updateMsgCount,
				resourceStats.cpuUsageFraction * 100.0f,
				(double)resourceStats.memUsageBytes / OneMegaByte);
			fflush(statsFile);
		}

		if (displayStats)
		{
			if (niProvPerfConfig.threadCount == 1)
				printf("%03u: ", currentRuntimeSec);
			else
				printf("%03u: Thread %d:\n  ", currentRuntimeSec, (i + 1));

			printf("UpdRate: %8llu, CPU: %6.2f%%, Mem: %6.2fMB\n",
				updateMsgCount / timePassedSec,
				resourceStats.cpuUsageFraction * 100.0f, (double)resourceStats.memUsageBytes / OneMegaByte);

			// if an non-interactive provider
			if (itemRequestCount > 0 || refreshMsgCount > 0)
				printf("  - Sent %llu images (total: %llu)\n",
					refreshMsgCount, totalStats.refreshMsgCount.countStatGetTotal());

			closeMsgCount = totalStats.closeMsgCount.countStatGetChange();
			if (closeMsgCount > 0)
				printf("  - Received %llu closes.\n", closeMsgCount);

			if (statusCount > 0)
				printf("  - Received %llu status messages.\n", statusCount);

			if (stats.intervalMsgEncodingStats.count > 0)
			{
				stats.intervalMsgEncodingStats.printValueStatistics(stdout, "Update Encode Time (usec)", "Msgs", true);
				stats.intervalMsgEncodingStats.clearValueStatistics();
			}

			// TODO: if (providerThreadConfig.takeMCastStats)
			// Mcast statistics could be here but we can't access to a low-level data
		}

	}

	return true;
}

void EmaCppNIProvPerf::providerPrintSummaryStats(FILE* file)
{
	RsslInt32 i;
	PerfTimeValue statsTime;
	PerfTimeValue currentTime = perftool::common::GetTime::getTimeNano();
	
	if (niProvPerfConfig.threadCount > 1)
	{
		totalStats.inactiveTime = providerThreads[0]->getProviderStats().inactiveTime;

		for (i = 0; i < niProvPerfConfig.threadCount; ++i)
		{
			NIProviderThread* pProviderThread = providerThreads[i];
			ProviderStats& stats = pProviderThread->getProviderStats();

			statsTime = (stats.inactiveTime && stats.inactiveTime < currentTime) ? stats.inactiveTime : currentTime;
			if (stats.inactiveTime && stats.inactiveTime < totalStats.inactiveTime)
				totalStats.inactiveTime = stats.inactiveTime;
			fprintf(file, "\n--- THREAD %d SUMMARY ---\n\n", i + 1);

			fprintf(file, "Overall Statistics: \n");

			//case PROVIDER_NONINTERACTIVE:
			fprintf(file,
				"  Images sent: %llu\n"
				"  Updates sent: %llu\n\n",
				totalStats.refreshMsgCount.countStatGetTotal(),
				totalStats.updateMsgCount.countStatGetTotal());
		}
	}
	else
	{
		totalStats = providerThreads[0]->getProviderStats();
		statsTime = (totalStats.inactiveTime && totalStats.inactiveTime < currentTime) ? totalStats.inactiveTime : currentTime;
	}

	fprintf(file, "\n--- OVERALL SUMMARY ---\n\n");

	fprintf(file, "Overall Statistics: \n");

	//case PROVIDER_NONINTERACTIVE:
	fprintf(file,
		"  Images sent: %llu\n"
		"  Updates sent: %llu\n",
		totalStats.refreshMsgCount.countStatGetTotal(),
		totalStats.updateMsgCount.countStatGetTotal());

	if (cpuUsageStats.count)
	{
		assert(memUsageStats.count);
		fprintf(file,
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

	fprintf(file, "\n");
	return;
}

int main(int argc, char* argv[])
{
	EmaCppNIProvPerf emaNIProvPerf;
	// If there are multiple connections, determine which items are
	 // to be opened on each connection. 
	 // If any items are common to all connections, they are taken from the first
	 // items in the item list.  The rest of the list is then divided to provide a unique
	 // item list for each connection.
	emaNIProvPerf.inititailizeAndRun(argc, argv);
	
	std::cout << "main(). Finish!" << std::endl;
	return 0;
}
