///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2021-2022 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#include "EmaCppIProvPerf.h"
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

EmaString EmaCppIProvPerf::logText = "";

void EmaCppIProvPerf::exitOnMissingArgument(char** argv, int argPos)
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

void EmaCppIProvPerf::exitConfigError(char** argv)
{
	logText = "Run '";
	logText += argv[0];
	logText += " -?' to see usage.\n";

	AppUtil::logError(logText);
	return;
}

void EmaCppIProvPerf::exitWithUsage()
{
	logText = "Options:\n";
	logText += "  -?                                   Shows this usage.\n\n";

	logText += "  -providerName <name>                 Name of the IProvider component in config file EmaConfig.xml that will be used to configure connection.\n\n";

	logText += "  -tickRate <ticks per second>         Ticks per second.\n";
	logText += "  -updateRate <updates per second>     Update rate per second.\n";
	logText += "  -latencyUpdateRate <updates/sec>     Latency update rate(can specify \"all\" to send latency in every update).\n";
	logText += "  -genericMsgRate <genMsgs per second> GenMsg rate per second.\n";
	logText += "  -genericMsgLatencyRate <genMsgs/sec> Latency genMsg rate(can specify \"all\" to send latency in every genMsg).\n";
	logText += "  -packedMsgsRate <updates per second> PackedMsg rate per second.\n";
	logText += "  -refreshBurstSize <count>            Number of refreshes to send in a burst(controls granularity of time-checking).\n\n";

	logText += "  -packBufSize <byte>                  Size of buffer for PackedMsg.\n";
	logText += "  -maxPackCount <count>                Amount of packed  Update Messages into PackedMsg.\n\n";

	logText += "  -msgFile <file name>                 Name of the file that specifies the data content in messages.\n";
	logText += "  -summaryFile <filename>              Name of file for logging summary info.\n";
	logText += "  -statsFile <filename>                Base name of file for logging periodic statistics.\n";
	logText += "  -latencyFile <filename>              Base name of file for logging latency data.\n";
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

bool EmaCppIProvPerf::initIProvPerfConfig(int argc, char* argv[])
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
			provPerfConfig.runTime = atoi(argv[iargs++]);
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.writeStatsInterval = atoi(argv[iargs++]);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			++iargs;
			provPerfConfig.displayStats = false;
		}
		else if (strcmp("-preEnc", argv[iargs]) == 0)
		{
			++iargs;
			provPerfConfig.preEncItems = true;
		}
		else if (strcmp("-nanoTime", argv[iargs]) == 0)
		{
			++iargs;
			provPerfConfig.nanoTime = true;
		}
		else if (strcmp("-measureEncode", argv[iargs]) == 0)
		{
			++iargs;
			provPerfConfig.measureEncode = true;
		}
		else if (strcmp("-measureDecode", argv[iargs]) == 0)
		{
			++iargs;
			provPerfConfig.measureDecode = true;
		}

		else if (strcmp("-refreshBurstSize", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.refreshBurstSize = atoi(argv[iargs++]);
		}
		else if (strcmp("-tickRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.ticksPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-updateRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.updatesPerSec = atoi(argv[iargs++]);
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
				provPerfConfig.latencyUpdatesPerSec = ALWAYS_SEND_LATENCY_UPDATE;
			else
				provPerfConfig.latencyUpdatesPerSec = atoi(argv[iargs]);
			++iargs;
		}
		else if (0 == strcmp("-maxPackCount", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.numberMsgInPackedMsg = atoi(argv[iargs++]);
		}
		else if (strcmp("-genericMsgRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.genMsgsPerSec = atoi(argv[iargs++]);
		}
		else if (strcmp("-genericMsgLatencyRate", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			if (0 == strcmp("all", argv[iargs]))
				provPerfConfig.latencyGenMsgsPerSec = ALWAYS_SEND_LATENCY_GENMSG;
			else
				provPerfConfig.latencyGenMsgsPerSec = atoi(argv[iargs]);
			++iargs;
		}

		else if (strcmp("-mainThread", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.mainThreadCpu = argv[iargs++];
		}
		else if (strcmp("-threads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			provPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++provPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				provPerfConfig.threadBindList[provPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && provPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = provPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				provPerfConfig.threadBindList[i].clear();

			provThreadCount = (provPerfConfig.threadCount > 0) ? provPerfConfig.threadCount : 0;
		}
		else if (strcmp("-apiThreads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			provPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++provPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many api threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				provPerfConfig.apiThreadBindList[provPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && provPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = provPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				provPerfConfig.apiThreadBindList[i].clear();

			provThreadCount = (provPerfConfig.threadCount > 0) ? provPerfConfig.threadCount : 0;
		}
		else if (strcmp("-workerThreads", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}

			provPerfConfig.threadCount = 0;

			char* pToken = strtok(argv[iargs++], ",");
			while (pToken)
			{
				if (++provPerfConfig.threadCount > MAX_PROV_THREADS)
				{
					logText = "Config Error: Too many worker threads specified.";
					AppUtil::logError(logText);
					return false;
				}
				provPerfConfig.workerThreadBindList[provPerfConfig.threadCount - 1] = pToken;
				pToken = strtok(NULL, ",");
			}
			if (provThreadCount > 0 && provPerfConfig.threadCount != provThreadCount)
			{
				logText = "Config Error: worker thread count not equal to api thread count.";
				AppUtil::logError(logText);
				return false;
			}
			for (int i = provPerfConfig.threadCount; i < MAX_PROV_THREADS; ++i)
				provPerfConfig.workerThreadBindList[i].clear();

			provThreadCount = (provPerfConfig.threadCount > 0) ? provPerfConfig.threadCount : 0;
		}
		else if (strcmp("-useUserDispatch", argv[iargs]) == 0)
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			int i = atoi(argv[iargs++]);
			provPerfConfig.useUserDispatch = (i == 1) ? true : false;
		}

		else if (0 == strcmp("-summaryFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.summaryFilename = argv[iargs++];
		}
		else if (0 == strcmp("-msgFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.msgFilename = argv[iargs++];
		}
		else if (0 == strcmp("-latencyFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.logLatencyToFile = true;
			provPerfConfig.latencyLogFilename = argv[iargs++];
		}
		else if (0 == strcmp("-statsFile", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.statsFilename = argv[iargs++];
		}

		else if (0 == strcmp("-providerName", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.providerName = argv[iargs++];
		}

		else if (0 == strcmp("-packBufSize", argv[iargs]))
		{
			++iargs;
			if (iargs == argc)
			{
				exitOnMissingArgument(argv, iargs - 1);
				return false;
			}
			provPerfConfig.packedMsgBufferSize = atoi(argv[iargs++]);
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
		provPerfConfig.loginPosition = loginReq.getPosition();
	}
	else {
		provPerfConfig.loginPosition.set("localhost");
	}

	/* Conditions */

	if (provPerfConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.writeStatsInterval < 1)
	{
		printf("Config error: Write Stats Interval cannot be less than 1.\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyUpdatesPerSec > provPerfConfig.updatesPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total update rate.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyGenMsgsPerSec > provPerfConfig.genMsgsPerSec)
	{
		printf("Config Error: Latency genMsg rate cannot be greater than total genMsg rate.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyUpdatesPerSec > provPerfConfig.ticksPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total ticks per second.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyGenMsgsPerSec > provPerfConfig.ticksPerSec)
	{
		printf("Config Error: Latency genMsg rate cannot be greater than total ticks per second.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE
		&& provPerfConfig.preEncItems)
	{
		printf("Config Error: -preEnc has no effect when always sending latency update, since it must be encoded.\n\n");
		exitConfigError(argv);
		return false;
	}
	if (provPerfConfig.latencyUpdatesPerSec == 0 && provPerfConfig.measureEncode)
	{
		printf("Config Error: Measuring message encoding time when latency update rate is zero. Message encoding time is only recorded for latency updates.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyGenMsgsPerSec == ALWAYS_SEND_LATENCY_GENMSG
		&& provPerfConfig.preEncItems)
	{
		printf("Config Error: -preEnc has no effect when always sending latency genMsg, since it must be encoded.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.latencyGenMsgsPerSec == 0 && provPerfConfig.measureEncode)
	{
		printf("Config Error: Measuring message encoding time when latency genMsg rate is zero. Message encoding time is only recorded for latency genMsgs.\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.updatesPerSec != 0 && provPerfConfig.updatesPerSec < provPerfConfig.ticksPerSec)
	{
		printf("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.genMsgsPerSec != 0 && provPerfConfig.genMsgsPerSec < provPerfConfig.ticksPerSec)
	{
		printf("Config Error: GenMsg rate cannot be less than total ticks per second(unless it is zero).\n\n");
		exitConfigError(argv);
		return false;
	}

	if (provPerfConfig.threadCount == 0)
	{
		AppUtil::logError("Config Error: -threads must be contain list of Cpu.");
		exitConfigError(argv);
		return false;
	}

	return true;
}


void EmaCppIProvPerf::printIProvPerfConfig(FILE* file)
{
	char threadListStr[128] = "-1";
	char apiThreadListStr[128] = "-1";
	char workerThreadListStr[128] = "-1";
	EmaString providerName;

	// Build thread list 
	provPerfConfig.getThreadListAsString(provPerfConfig.threadBindList, threadListStr, 128);
	provPerfConfig.getThreadListAsString(provPerfConfig.workerThreadBindList, workerThreadListStr, 128);
	if ( !provPerfConfig.useUserDispatch )
		provPerfConfig.getThreadListAsString(provPerfConfig.apiThreadBindList, apiThreadListStr, 128);

	if (!provPerfConfig.providerName.empty())
		providerName = provPerfConfig.providerName;
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
		"        Latency Log File: %s\n"
		"    Write Stats Interval: %u\n"
		"           Display Stats: %s\n"
		,
		provPerfConfig.runTime,
		providerName.c_str(),
		provPerfConfig.useUserDispatch ? "Yes" : "No",
		provPerfConfig.mainThreadCpu.empty() ? "-1" : provPerfConfig.mainThreadCpu.c_str(),
		threadListStr,
		apiThreadListStr,
		workerThreadListStr,
		provPerfConfig.summaryFilename.c_str(),
		provPerfConfig.statsFilename.c_str(),
		( !provPerfConfig.latencyLogFilename.empty() ) ? provPerfConfig.latencyLogFilename.c_str() : "(none)",
		provPerfConfig.writeStatsInterval,
		provPerfConfig.displayStats ? "Yes" : "No"
	);

	fprintf(file,
		"               Tick Rate: %d\n"
		"             Update Rate: %d\n"
		"     Latency Update Rate: %d\n"
		"        Generic Msg Rate: %d\n"
		"Latency Generic Msg Rate: %d\n"
		"      Refresh Burst Size: %d\n"
		"               Data File: %s\n",

		provPerfConfig.ticksPerSec,
		provPerfConfig.updatesPerSec,
		provPerfConfig.latencyUpdatesPerSec >= 0 ? provPerfConfig.latencyUpdatesPerSec : provPerfConfig.updatesPerSec,
		provPerfConfig.genMsgsPerSec,
		provPerfConfig.latencyGenMsgsPerSec >= 0 ? provPerfConfig.latencyGenMsgsPerSec : provPerfConfig.genMsgsPerSec,
		provPerfConfig.refreshBurstSize,
		provPerfConfig.msgFilename.c_str());

	if (provPerfConfig.numberMsgInPackedMsg > 1)
	{
		fprintf(file,
		"               Number In Packed Msg: %d\n",
		provPerfConfig.numberMsgInPackedMsg);
	}

	fprintf(file,
		"     Pre-Encoded Updates: %s\n"
		"         Nanosecond Time: %s\n"
		"          Measure Encode: %s\n",
		provPerfConfig.preEncItems ? "Yes" : "No",
		provPerfConfig.nanoTime ? "Yes" : "No",
		provPerfConfig.measureEncode ? "Yes" : "No");

	return;
}

bool EmaCppIProvPerf::inititailizeAndRun(int argc, char* argv[])
{
	if (initIProvPerfConfig(argc, argv) == false)
		return false;
	printIProvPerfConfig(stdout);

	if (!(summaryFile = fopen(provPerfConfig.summaryFilename, "w")))
	{
		logText = "Error: Failed to open file '";
		logText += provPerfConfig.summaryFilename;
		logText += "'.";
		AppUtil::logError(logText);
		exit(-1);
	}

	printIProvPerfConfig(summaryFile); fflush(summaryFile);


	if ( !provPerfConfig.mainThreadCpu.empty() && !provPerfConfig.mainThreadCpu.caseInsensitiveCompare("-1") )
	{
		RsslError rsslError;
		RsslRet retCode = rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);
		if ( !bindThisThread("Main Thread", provPerfConfig.mainThreadCpu) )
		{
			rsslUninitialize();
			return false;
		}
		printAllThreadBinding();
	}

	// Initializes pre-encoded messages data, helpers to access to template messages file.
	perfMessageData = new PerfMessageData(provPerfConfig.msgFilename, provPerfConfig.preEncItems);

	// Creates and initializes threads
	Int32 i;
	for (i = 0; i < provPerfConfig.threadCount; ++i)
	{
		ProviderThread* pProviderThread = new ProviderThread(provPerfConfig, perfMessageData, (i+1));
		providerThreads.push_back(pProviderThread);

		pProviderThread->providerThreadInit();

		pProviderThread->setCpuId(provPerfConfig.threadBindList[i]);
		pProviderThread->setApiThreadCpuId(provPerfConfig.apiThreadBindList[i]);
		pProviderThread->setWorkerThreadCpuId(provPerfConfig.workerThreadBindList[i]);
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

bool EmaCppIProvPerf::shutdownThreads()
{
	UInt64 count = 0;
	const UInt64 ctSize = providerThreads.size();
	for (UInt64 i = 0; i < ctSize; ++i)
	{
		ProviderThread* pProviderThread = providerThreads[i];
		if (!pProviderThread->isRunning() || pProviderThread->isStopped())
			count++;
	}
	if (count == ctSize)
		return true;
	return false;
}

void EmaCppIProvPerf::cleanupThreads()
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

bool EmaCppIProvPerf::monitorStatistics()
{
	PerfTimeValue startTime = perftool::common::GetTime::getTimeMilli();
	PerfTimeValue endProviderRunTime = startTime + provPerfConfig.runTime * 1000ULL;

	PerfTimeValue currentTime = startTime;

	const UInt64 writeStatsIntervalMs = provPerfConfig.writeStatsInterval * 1000ULL;

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

			if (!collectStats(true, provPerfConfig.displayStats,
				(UInt32)((currentTime - startTime) / 1000), provPerfConfig.writeStatsInterval))
			{
				break;
			}

			currentTime = perftool::common::GetTime::getTimeMilli();
		}

		if (currentTime >= endProviderRunTime)
		{
			AppUtil::log("\nRun time of %u seconds has expired.\n", provPerfConfig.runTime);
			break;
		}

		sleepIntervalMs = (nextTimeMs > currentTime ? (nextTimeMs - currentTime) : 0U);
		AppUtil::sleepUI(sleepIntervalMs);

		if (CtrlBreakHandler::isTerminated())
			break;
	}

	return true;
}

bool EmaCppIProvPerf::collectStats(bool writeStats, bool displayStats, UInt32 currentRuntimeSec,
	UInt32 timePassedSec)
{
	UInt64 refreshMsgCount,
		updateMsgCount,
		packedMsgCount,
		itemRequestCount,
		closeMsgCount,
		postMsgCount,
		outOfBuffersCount,
		statusCount,
		genMsgRecvCount,
		genMsgSentCount,
		latencyGenMsgRecvCount,
		latencyGenMsgSentCount;

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

	for (Int32 i = 0; i < provPerfConfig.threadCount; ++i)
	{
		ProviderThread* providerThread = providerThreads[i];
		ProviderStats& stats = providerThread->getProviderStats();

		/* Gather latency records for gen msgs. */
		{
			LatencyRecords* latencyRecords = NULL;
			UInt64 latencyListSize = 0;
			FILE* latencyLogFile = providerThread->getLatencyLogFile();

			stats.genMsgLatencyRecords.getLatencyTimeRecords(&latencyRecords);
			latencyListSize = (latencyRecords == NULL) ? 0 : latencyRecords->size();
			for (UInt64 l = 0; l < latencyListSize; ++l)
			{
				TimeRecord* pRecord = &(*latencyRecords)[l];
				double latency = (double)(pRecord->endTime - pRecord->startTime) / (double)pRecord->ticks;

				stats.intervalGenMsgLatencyStats.updateValueStatistics(latency);
				stats.genMsgLatencyStats.updateValueStatistics(latency);

				if (provPerfConfig.threadCount > 1)
					totalStats.genMsgLatencyStats.updateValueStatistics(latency);

				if (latencyLogFile)
					fprintf(latencyLogFile, "Gen, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
			}
			stats.genMsgLatencyRecords.clearReadLatTimeRecords();

			if (latencyLogFile)
				fflush(latencyLogFile);
		}

		// Collect counts.
		refreshMsgCount = stats.refreshMsgCount.countStatGetChange();
		updateMsgCount = stats.updateMsgCount.countStatGetChange();
		packedMsgCount = stats.packedMsgCount.countStatGetChange();
		itemRequestCount = stats.itemRequestCount.countStatGetChange();
		closeMsgCount = stats.closeMsgCount.countStatGetChange();
		postMsgCount = stats.postMsgCount.countStatGetChange();
		outOfBuffersCount = stats.outOfBuffersCount.countStatGetChange();
		statusCount = stats.statusCount.countStatGetChange();
		genMsgRecvCount = stats.genMsgRecvCount.countStatGetChange();
		genMsgSentCount = stats.genMsgSentCount.countStatGetChange();
		latencyGenMsgRecvCount = stats.intervalGenMsgLatencyStats.count;
		latencyGenMsgSentCount = stats.latencyGenMsgSentCount.countStatGetChange();

		if (provPerfConfig.measureEncode)
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
		totalStats.postMsgCount.countStatAdd(postMsgCount);
		totalStats.outOfBuffersCount.countStatAdd(outOfBuffersCount);
		totalStats.statusCount.countStatAdd(statusCount);
		totalStats.genMsgRecvCount.countStatAdd(genMsgRecvCount);
		totalStats.genMsgSentCount.countStatAdd(genMsgSentCount);
		totalStats.latencyGenMsgSentCount.countStatAdd(latencyGenMsgSentCount);
		totalStats.packedMsgCount.countStatAdd(packedMsgCount);

		if (writeStats)
		{
			/* Write stats to the stats file. */
			FILE* statsFile = providerThread->getStatsFile();
			printCurrentTimeUTC(statsFile);

//							case PROVIDER_INTERACTIVE:
			fprintf(statsFile, ", %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n",
				itemRequestCount,
				refreshMsgCount,
				updateMsgCount,
				postMsgCount,
				genMsgSentCount,
				genMsgRecvCount,
				latencyGenMsgSentCount,
				latencyGenMsgRecvCount,
				stats.intervalGenMsgLatencyStats.mean,
				sqrt(stats.intervalGenMsgLatencyStats.variance),
				latencyGenMsgRecvCount ? stats.intervalGenMsgLatencyStats.maxValue : 0.0,
				latencyGenMsgRecvCount ? stats.intervalGenMsgLatencyStats.minValue : 0.0,
				resourceStats.cpuUsageFraction * 100.0f,
				(double)resourceStats.memUsageBytes / OneMegaByte);
			fflush(statsFile);
		}

		if (displayStats)
		{
			if (provPerfConfig.threadCount == 1)
				printf("%03u: ", currentRuntimeSec);
			else
				printf("%03u: Thread %d:\n  ", currentRuntimeSec, (i + 1));

			printf("UpdRate: %8llu, CPU: %6.2f%%, Mem: %6.2fMB\n",
				updateMsgCount / timePassedSec,
				resourceStats.cpuUsageFraction * 100.0f, (double)resourceStats.memUsageBytes / OneMegaByte);

			// if an interactive provider
			if (itemRequestCount > 0 || refreshMsgCount > 0)
				printf("  - Received %llu item requests (total: %llu), sent %llu images (total: %llu)\n",
					itemRequestCount, totalStats.itemRequestCount.countStatGetTotal(), refreshMsgCount, totalStats.refreshMsgCount.countStatGetTotal());
			if (postMsgCount > 0)
				printf("  Posting: received %llu, reflected %llu\n", postMsgCount, postMsgCount);
			if (genMsgRecvCount > 0 || genMsgSentCount > 0)
				printf("  GenMsgs: sent %llu, received %llu, latencies sent %llu, latencies received %llu\n",
					genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
			if (stats.intervalGenMsgLatencyStats.count > 0)
			{
				stats.intervalGenMsgLatencyStats.printValueStatistics(stdout, "  GenMsgLat(usec)", "Msgs", false);
				stats.intervalGenMsgLatencyStats.clearValueStatistics();
			}
			if (packedMsgCount)
				printf("  Average update messages packed per message: %llu\n", updateMsgCount/packedMsgCount);

			closeMsgCount = totalStats.closeMsgCount.countStatGetChange();
			if (closeMsgCount > 0)
				printf("  - Received %llu closes.\n", closeMsgCount);

			outOfBuffersCount = totalStats.outOfBuffersCount.countStatGetChange();
			if (outOfBuffersCount > 0)
				printf("  - Stopped %llu updates due to lack of output buffers.\n", outOfBuffersCount);

			if (statusCount > 0)
				printf("  - Received %llu status messages.\n", statusCount);

			if (stats.intervalMsgEncodingStats.count > 0)
			{
				stats.intervalMsgEncodingStats.printValueStatistics(stdout, "Update Encode Time (usec)", "Msgs", true);
				stats.intervalMsgEncodingStats.clearValueStatistics();
			}
		}

	}

	return true;
}

void EmaCppIProvPerf::providerPrintSummaryStats(FILE* file)
{
	RsslInt32 i;
	PerfTimeValue statsTime;
	PerfTimeValue currentTime = perftool::common::GetTime::getTimeNano();
	
	if (provPerfConfig.threadCount > 1)
	{
		totalStats.inactiveTime = providerThreads[0]->getProviderStats().inactiveTime;
		totalStats.firstGenMsgSentTime = providerThreads[0]->getProviderStats().firstGenMsgSentTime;
		totalStats.firstGenMsgRecvTime = providerThreads[0]->getProviderStats().firstGenMsgRecvTime;

		for (i = 0; i < provPerfConfig.threadCount; ++i)
		{
			ProviderThread* pProviderThread = providerThreads[i];
			ProviderStats& stats = pProviderThread->getProviderStats();

			statsTime = (stats.inactiveTime && stats.inactiveTime < currentTime) ? stats.inactiveTime : currentTime;
			if (stats.inactiveTime && stats.inactiveTime < totalStats.inactiveTime)
				totalStats.inactiveTime = stats.inactiveTime;
			if (stats.firstGenMsgSentTime && stats.firstGenMsgSentTime < totalStats.firstGenMsgSentTime)
				totalStats.firstGenMsgSentTime = stats.firstGenMsgSentTime;
			if (stats.firstGenMsgRecvTime && stats.firstGenMsgRecvTime < totalStats.firstGenMsgRecvTime)
				totalStats.firstGenMsgRecvTime = stats.firstGenMsgRecvTime;
			fprintf(file, "\n--- THREAD %d SUMMARY ---\n\n", i + 1);

			fprintf(file, "Overall Statistics: \n");

			//case PROVIDER_INTERACTIVE:
			if (stats.genMsgLatencyStats.count)
			{
				fprintf(file,
					"  GenMsg latency avg (usec): %.1f\n"
					"  GenMsg latency std dev (usec): %.1f\n"
					"  GenMsg latency max (usec): %.1f\n"
					"  GenMsg latency min (usec): %.1f\n",
					stats.genMsgLatencyStats.mean,
					sqrt(stats.genMsgLatencyStats.variance),
					stats.genMsgLatencyStats.maxValue,
					stats.genMsgLatencyStats.minValue);
			}
			else {
				fprintf(file, "  No GenMsg latency information was received.\n");
			}
			if (provPerfConfig.genMsgsPerSec)
				fprintf(file, "  GenMsgs sent: %llu\n", stats.genMsgSentCount.countStatGetTotal());
			if (stats.genMsgRecvCount.countStatGetTotal())
				fprintf(file, "  GenMsgs received: %llu\n", stats.genMsgRecvCount.countStatGetTotal());
			if (provPerfConfig.latencyGenMsgsPerSec)
				fprintf(file, "  GenMsg latencies sent: %llu\n", stats.latencyGenMsgSentCount.countStatGetTotal());
			if (stats.genMsgLatencyStats.count)
				fprintf(file, "  GenMsg latencies received: %llu\n", stats.genMsgLatencyStats.count);
			if (provPerfConfig.genMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg send rate: %.0f\n", stats.genMsgSentCount.countStatGetTotal() /
					(double)((statsTime - stats.firstGenMsgSentTime) / 1000000000.0));
			}
			if (stats.genMsgRecvCount.countStatGetTotal())
			{
				fprintf(file, "  Avg GenMsg receive rate: %.0f\n", stats.genMsgRecvCount.countStatGetTotal() /
					(double)((statsTime - stats.firstGenMsgRecvTime) / 1000000000.0));
			}
			if (provPerfConfig.latencyGenMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", stats.latencyGenMsgSentCount.countStatGetTotal() /
					(double)((statsTime - stats.firstGenMsgSentTime) / 1000000000.0));
			}
			if (stats.genMsgLatencyStats.count)
			{
				fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", stats.genMsgLatencyStats.count /
					(double)((statsTime - stats.firstGenMsgRecvTime) / 1000000000.0));
			}
			fprintf(file, "  Image requests received: %llu\n", stats.itemRequestCount.countStatGetTotal());
			if (provPerfConfig.updatesPerSec)
				fprintf(file, "  Updates sent: %llu\n", stats.updateMsgCount.countStatGetTotal());
			if (stats.postMsgCount.countStatGetTotal())
			{
				fprintf(file, "  Posts received: %llu\n", stats.postMsgCount.countStatGetTotal());
				fprintf(file, "  Posts reflected: %llu\n", stats.postMsgCount.countStatGetTotal());
			}

		}
	}
	else
	{
		totalStats = providerThreads[0]->getProviderStats();
		statsTime = (totalStats.inactiveTime && totalStats.inactiveTime < currentTime) ? totalStats.inactiveTime : currentTime;
	}

	fprintf(file, "\n--- OVERALL SUMMARY ---\n\n");

	fprintf(file, "Overall Statistics: \n");

//	case PROVIDER_INTERACTIVE:
	if (totalStats.genMsgLatencyStats.count)
	{
		fprintf(file,
			"  GenMsg latency avg (usec): %.1f\n"
			"  GenMsg latency std dev (usec): %.1f\n"
			"  GenMsg latency max (usec): %.1f\n"
			"  GenMsg latency min (usec): %.1f\n",
			totalStats.genMsgLatencyStats.mean,
			sqrt(totalStats.genMsgLatencyStats.variance),
			totalStats.genMsgLatencyStats.maxValue,
			totalStats.genMsgLatencyStats.minValue);
	}
	else {
		fprintf(file, "  No GenMsg latency information was received.\n");
	}
	if (provPerfConfig.genMsgsPerSec)
		fprintf(file, "  GenMsgs sent: %llu\n", totalStats.genMsgSentCount.countStatGetTotal());
	if (totalStats.genMsgRecvCount.countStatGetTotal())
		fprintf(file, "  GenMsgs received: %llu\n", totalStats.genMsgRecvCount.countStatGetTotal());
	if (provPerfConfig.latencyGenMsgsPerSec)
		fprintf(file, "  GenMsg latencies sent: %llu\n", totalStats.latencyGenMsgSentCount.countStatGetTotal());
	if (totalStats.genMsgLatencyStats.count)
		fprintf(file, "  GenMsg latencies received: %llu\n", totalStats.genMsgLatencyStats.count);
	if (provPerfConfig.genMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg send rate: %.0f\n", totalStats.genMsgSentCount.countStatGetTotal() /
			(double)((statsTime - totalStats.firstGenMsgSentTime) / 1000000000.0));
	}
	if (totalStats.genMsgRecvCount.countStatGetTotal())
	{
		fprintf(file, "  Avg GenMsg receive rate: %.0f\n", totalStats.genMsgRecvCount.countStatGetTotal() /
			(double)((statsTime - totalStats.firstGenMsgRecvTime) / 1000000000.0));
	}
	if (provPerfConfig.latencyGenMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", totalStats.latencyGenMsgSentCount.countStatGetTotal() /
			(double)((statsTime - totalStats.firstGenMsgSentTime) / 1000000000.0));
	}
	if (totalStats.genMsgLatencyStats.count)
	{
		fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", totalStats.genMsgLatencyStats.count /
			(double)((statsTime - totalStats.firstGenMsgRecvTime) / 1000000000.0));
	}
	fprintf(file, "  Image requests received: %llu\n", totalStats.itemRequestCount.countStatGetTotal());
	if (provPerfConfig.updatesPerSec)
		fprintf(file, "  Updates sent: %llu\n", totalStats.updateMsgCount.countStatGetTotal());
	if (totalStats.postMsgCount.countStatGetTotal())
	{
		fprintf(file, "  Posts received: %llu\n", totalStats.postMsgCount.countStatGetTotal());
		fprintf(file, "  Posts reflected: %llu\n", totalStats.postMsgCount.countStatGetTotal());
	}

	if (totalStats.packedMsgCount.countStatGetTotal())
	{
		fprintf(file, "  Packed msg send: %llu\n", totalStats.packedMsgCount.countStatGetTotal());
	}

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
	EmaCppIProvPerf emaIProvPerf;
	// If there are multiple connections, determine which items are
	 // to be opened on each connection. 
	 // If any items are common to all connections, they are taken from the first
	 // items in the item list.  The rest of the list is then divided to provide a unique
	 // item list for each connection.
	emaIProvPerf.inititailizeAndRun(argc, argv);
	
	std::cout << "main(). Finish!" << std::endl;
	return 0;
}
