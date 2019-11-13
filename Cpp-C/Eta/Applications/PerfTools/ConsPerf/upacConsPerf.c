/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "upacConsPerf.h"
#include "statistics.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "consumerThreads.h"
#include "marketByOrderDecoder.h"
#include "marketPriceDecoder.h"
#include "getTime.h"
#include "testUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#ifdef _WIN32
#include <winsock2.h>
#include <process.h>
#define getpid _getpid
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
	static void signal_handler(int sig);
}
#endif

ConsumerThread consumerThreads[MAX_CONS_THREADS];

/* Total statistics collected from all consumer threads. 
 * This is only used when there are multiple consumer threads. */
ConsumerStats totalStats;

/* CPU & Memory Usage samples */
static ValueStatistics cpuUsageStats, memUsageStats;

TimeValue currentTime, startTime, endTime;
RsslUInt32 currentRuntimeSec = 0, intervalSeconds = 0;

RsslBool refreshRetrievalTimePrinted = RSSL_FALSE;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

static ResourceUsageStats resourceStats;

static void signal_handler(int sig)
{
	shutdownThreads = RSSL_TRUE;
}

int main(int argc, char **argv)
{
	RsslError error;
	RsslRet ret;
	RsslRet	retval = 0;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	RsslInt32 i;
	TimeValue nextTime;
	

#ifndef _WIN32
	struct timespec sleeptime;
#endif


	RsslInt32 itemListUniqueIndex;

	xmlInitParser();

	initConsPerfConfig(argc, argv);
	printConsPerfConfig(stdout);
	consumerStatsInit(&totalStats);


	if (consPerfConfig.postsPerSec || consPerfConfig.genMsgsPerSec)
	{
		if (xmlMsgDataInit(consPerfConfig.msgFilename) != RSSL_RET_SUCCESS)
			exit(-1);
	}

	/* If there are multiple connections, determine which items are
	 * to be opened on each connection. 
	 * If any items are common to all connections, they are taken from the first
	 * items in the item list.  The rest of the list is then divided to provide a unique
	 * item list for each connection. */

	itemListUniqueIndex = consPerfConfig.commonItemCount;
	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		consumerThreadInit(&consumerThreads[i], i+1);

		/* Figure out how many items each consumer should request. */
		consumerThreads[i].itemListCount = consPerfConfig.itemRequestCount
			/ consPerfConfig.threadCount;

		/* Distribute remainder. */
		if (i < consPerfConfig.itemRequestCount % consPerfConfig.threadCount)
			consumerThreads[i].itemListCount += 1;

		consumerThreads[i].itemListUniqueIndex = itemListUniqueIndex;
		itemListUniqueIndex += consumerThreads[i].itemListCount - consPerfConfig.commonItemCount;

	}

	assert(itemListUniqueIndex == consPerfConfig.itemRequestCount - 
			consPerfConfig.commonItemCount * (consPerfConfig.threadCount - 1));
	

	if (!(summaryFile = fopen(consPerfConfig.summaryFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", consPerfConfig.summaryFilename);
		exit(-1);
	}

	printConsPerfConfig(summaryFile); fflush(summaryFile);

	// set up a signal handler so we can cleanup before exit
	signal(SIGINT, signal_handler);
	
	for(i = 0; i < consPerfConfig.threadCount; ++i)
		consumerThreads[i].cpuId = consPerfConfig.threadBindList[i];

	/* Initialize RSSL */
	if (consPerfConfig.useReactor == RSSL_FALSE && consPerfConfig.useWatchlist == RSSL_FALSE) // use UPA Channel
	{
		if (rsslInitialize(consPerfConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
		{
			printf("rsslInitialize(): failed <%s>\n", error.text);
			/* WINDOWS: wait for user to enter something before exiting  */
	#ifdef _WIN32
			printf("\nPress Enter or Return key to exit application:");
			getchar();
	#endif
			exit(RSSL_RET_FAILURE);
		}
	}

	if (consPerfConfig.postsPerSec)
	{
		/* Set PostUserInfo information. */
		char tmpHostNameBlock[256];
		RsslBuffer tmpHostName = { 256, tmpHostNameBlock };

		if (gethostname(tmpHostNameBlock, sizeof(tmpHostNameBlock)) != 0)
			snprintf(tmpHostNameBlock, 256, "localhost");

		rsslClearPostUserInfo(&postUserInfo);
		postUserInfo.postUserId = getpid();
		if ((ret = rsslHostByName(&tmpHostName, &postUserInfo.postUserAddr) != RSSL_RET_SUCCESS))
		{
			printf("rsslHostByName() failed while setting postUserInfo: %d\n", ret);
			exit(-1);
		}
	}



	printf("Starting connections(%d total)...\n\n", consPerfConfig.threadCount);

	/* Spawn consumer threads */
	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		if (consPerfConfig.useReactor == RSSL_FALSE && consPerfConfig.useWatchlist == RSSL_FALSE) // use UPA Channel
		{
			if (!CHECK(RSSL_THREAD_START(&consumerThreads[i].threadId, runConsumerChannelConnection, &consumerThreads[i]) >= 0))
				exit(RSSL_RET_FAILURE);
		}
		else // use VA Reactor
		{
			if (!CHECK(RSSL_THREAD_START(&consumerThreads[i].threadId, runConsumerReactorConnection, &consumerThreads[i]) >= 0))
				exit(RSSL_RET_FAILURE);
		}
	}

	endTime = getTimeNano() + consPerfConfig.steadyStateTime * 1000000000ULL;


	/* Reset resource usage. */
	if ((ret = initResourceUsageStats(&resourceStats)) != RSSL_RET_SUCCESS)
	{
		printf("initResourceUsageStats() failed: %d\n", ret);
		exit(RSSL_RET_FAILURE);
	}

	clearValueStatistics(&cpuUsageStats);
	clearValueStatistics(&memUsageStats);

	startTime = getTimeNano();

	/* Sleep for one more second so some stats can be gathered before first printout. */
	SLEEP(1);

	/* main polling thread here */
	while(!shutdownThreads)
	{
		currentTime = getTimeNano();
		++currentRuntimeSec;
		++intervalSeconds;


		if (intervalSeconds == consPerfConfig.writeStatsInterval)
		{
			collectStats(RSSL_TRUE, consPerfConfig.displayStats, 
					currentRuntimeSec, consPerfConfig.writeStatsInterval);
			intervalSeconds = 0;
		}

		if (totalStats.imageRetrievalEndTime && consPerfConfig.requestSnapshots)
		{
			printf("Received all images for snapshot test.\n");
			break;
		}

		if(currentTime >= endTime)
		{
			if (!totalStats.imageRetrievalEndTime)
				printf("Error: Failed to receive all images within %u seconds.\n", consPerfConfig.steadyStateTime);
			else
				printf("\nSteady state time of %u seconds has expired.\n", consPerfConfig.steadyStateTime);

			break;
		}

		nextTime = currentTime + 1000000000LL;
		
#ifdef _WIN32
		currentTime = getTimeNano();
		Sleep((DWORD)((nextTime-currentTime)/1000000)); // millisecond sleep
#else
		currentTime = getTimeNano();
		sleeptime.tv_sec = 0;
		sleeptime.tv_nsec = nextTime - currentTime;
		if (sleeptime.tv_nsec >= 1000000000)
		{
			sleeptime.tv_sec = 1;
			sleeptime.tv_nsec -= 1000000000;
		}
		nanosleep(&sleeptime, 0);
#endif

	}

	consumerCleanupThreads();
	fclose(summaryFile);
	if (consPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
	{
		rsslUninitialize();
	}
	xmlCleanupParser();
	return 0;
}

void collectStats(RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec, RsslUInt32 timePassedSec)
{
	RsslRet ret;
	RsslInt32 i;
	RsslQueue latencyRecords;
	RsslBool allRefreshesRetrieved = RSSL_TRUE;

	rsslInitQueue(&latencyRecords);

	if (timePassedSec)
	{
		if ((ret = getResourceUsageStats(&resourceStats)) != RSSL_RET_SUCCESS)
		{
			printf("getResourceUsageStats() failed: %d\n", ret);
			exit(RSSL_RET_FAILURE);
		}
		updateValueStatistics(&cpuUsageStats, (double)resourceStats.cpuUsageFraction);
		updateValueStatistics(&memUsageStats, (double)resourceStats.memUsageBytes);
	}

	for(i = 0; i < consPerfConfig.threadCount; i++)
	{
		RsslQueueLink *pLink;

		RsslUInt64 refreshCount,
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

		/* Gather latency records from each thread and update statistics. */
		timeRecordQueueGet(&consumerThreads[i].latencyRecords, &latencyRecords);
		RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
		{
			TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;
			double recordEndTimeNsec = (double)pRecord->endTime/(double)pRecord->ticks * 1000.0;

			/* Make sure this latency is counted towards startup or steady-state as appropriate. */
			RsslBool latencyIsSteadyStateForClient = 
				consumerThreads[i].stats.imageRetrievalEndTime != 0
				&& recordEndTimeNsec > (double)consumerThreads[i].stats.imageRetrievalEndTime;

			updateValueStatistics(&consumerThreads[i].stats.intervalLatencyStats, latency);
			updateValueStatistics(&consumerThreads[i].stats.overallLatencyStats, latency);
			updateValueStatistics( latencyIsSteadyStateForClient ? 
					&consumerThreads[i].stats.steadyStateLatencyStats
					: &consumerThreads[i].stats.startupLatencyStats,
					latency);

			if (consPerfConfig.threadCount > 1)
			{
				/* Make sure this latency is counted towards startup or steady-state as appropriate. */
				RsslBool latencyIsSteadyStateOverall = 
					totalStats.imageRetrievalEndTime != 0
					&& recordEndTimeNsec > (double)totalStats.imageRetrievalEndTime;

				updateValueStatistics( latencyIsSteadyStateOverall ? 
						&totalStats.steadyStateLatencyStats
						: &totalStats.startupLatencyStats,
						latency);
				updateValueStatistics(&totalStats.overallLatencyStats, latency);
			}

			if (consumerThreads[i].latencyLogFile)
				fprintf(consumerThreads[i].latencyLogFile, "Upd, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
		}
		timeRecordQueueRepool(&consumerThreads[i].latencyRecords, &latencyRecords);

		/* Gather latency records for posts. */
		timeRecordQueueGet(&consumerThreads[i].postLatencyRecords, &latencyRecords);
		RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
		{
			TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;

			updateValueStatistics(&consumerThreads[i].stats.intervalPostLatencyStats, latency);
			updateValueStatistics( &consumerThreads[i].stats.postLatencyStats, latency);

			if (consPerfConfig.threadCount > 1)
				updateValueStatistics(&totalStats.postLatencyStats, latency);

			if (consumerThreads[i].latencyLogFile)
				fprintf(consumerThreads[i].latencyLogFile, "Pst, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
		}
		timeRecordQueueRepool(&consumerThreads[i].postLatencyRecords, &latencyRecords);

		/* Gather latency records for gen msgs. */
		timeRecordQueueGet(&consumerThreads[i].genMsgLatencyRecords, &latencyRecords);
		RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
		{
			TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;

			updateValueStatistics(&consumerThreads[i].stats.intervalGenMsgLatencyStats, latency);
			updateValueStatistics(&consumerThreads[i].stats.genMsgLatencyStats, latency);

			if (consPerfConfig.threadCount > 1)
				updateValueStatistics(&totalStats.genMsgLatencyStats, latency);

			if (consumerThreads[i].latencyLogFile)
				fprintf(consumerThreads[i].latencyLogFile, "Gen, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
		}
		timeRecordQueueRepool(&consumerThreads[i].genMsgLatencyRecords, &latencyRecords);

		if (consumerThreads[i].latencyLogFile)
			fflush(consumerThreads[i].latencyLogFile);

		if (consPerfConfig.measureDecode)
		{
			/* Gather time records for decoding. */
			timeRecordQueueGet(&consumerThreads[i].updateDecodeTimeRecords, &latencyRecords);

			RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
			{
				TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
				double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;

				updateValueStatistics(&consumerThreads[i].stats.intervalUpdateDecodeTimeStats, latency);
			}

			timeRecordQueueRepool(&consumerThreads[i].updateDecodeTimeRecords, &latencyRecords);
		}

		/* Collect counts. */
		startupUpdateCount = countStatGetChange(&consumerThreads[i].stats.startupUpdateCount);
		steadyStateUpdateCount = countStatGetChange(&consumerThreads[i].stats.steadyStateUpdateCount);
		statusCount = countStatGetChange(&consumerThreads[i].stats.statusCount);
		requestCount = countStatGetChange(&consumerThreads[i].stats.requestCount);
		refreshCount = countStatGetChange(&consumerThreads[i].stats.refreshCount);
		postSentCount = countStatGetChange(&consumerThreads[i].stats.postSentCount);
		postOutOfBuffersCount = countStatGetChange(&consumerThreads[i].stats.postOutOfBuffersCount);
		genMsgSentCount = countStatGetChange(&consumerThreads[i].stats.genMsgSentCount);
		genMsgRecvCount = countStatGetChange(&consumerThreads[i].stats.genMsgRecvCount);
		latencyGenMsgSentCount = countStatGetChange(&consumerThreads[i].stats.latencyGenMsgSentCount);
		latencyGenMsgRecvCount = consumerThreads[i].stats.intervalGenMsgLatencyStats.count;
		genMsgOutOfBuffersCount = countStatGetChange(&consumerThreads[i].stats.genMsgOutOfBuffersCount);

		if (consPerfConfig.threadCount > 1)
		{
			countStatAdd(&totalStats.startupUpdateCount, startupUpdateCount);
			countStatAdd(&totalStats.steadyStateUpdateCount, steadyStateUpdateCount);
			countStatAdd(&totalStats.statusCount, statusCount);
			countStatAdd(&totalStats.requestCount, requestCount);
			countStatAdd(&totalStats.refreshCount, refreshCount);
			countStatAdd(&totalStats.postSentCount, postSentCount);
			countStatAdd(&totalStats.postOutOfBuffersCount, postOutOfBuffersCount); 
			countStatAdd(&totalStats.genMsgSentCount, genMsgSentCount);
			countStatAdd(&totalStats.genMsgRecvCount, genMsgRecvCount);
			countStatAdd(&totalStats.latencyGenMsgSentCount, latencyGenMsgSentCount);
			countStatAdd(&totalStats.genMsgOutOfBuffersCount, genMsgOutOfBuffersCount);
		}

		if (writeStats)
		{
			/* Log statistics to file. */
			printCurrentTimeUTC(consumerThreads[i].statsFile);
			fprintf(consumerThreads[i].statsFile,
					", %llu, %.1f, %.1f, %.1f, %.1f, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %llu, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n",
					consumerThreads[i].stats.intervalLatencyStats.count,
					consumerThreads[i].stats.intervalLatencyStats.average,
					sqrt(consumerThreads[i].stats.intervalLatencyStats.variance), consumerThreads[i].stats.intervalLatencyStats.count ?	consumerThreads[i].stats.intervalLatencyStats.maxValue : 0.0,
					consumerThreads[i].stats.intervalLatencyStats.count ? consumerThreads[i].stats.intervalLatencyStats.minValue : 0.0,
					refreshCount,
					(startupUpdateCount + steadyStateUpdateCount)/timePassedSec,
					consumerThreads[i].stats.intervalPostLatencyStats.count,
					consumerThreads[i].stats.intervalPostLatencyStats.average,
					sqrt(consumerThreads[i].stats.intervalPostLatencyStats.variance),
					consumerThreads[i].stats.intervalPostLatencyStats.count ? consumerThreads[i].stats.intervalPostLatencyStats.maxValue : 0.0,
					consumerThreads[i].stats.intervalPostLatencyStats.count ? consumerThreads[i].stats.intervalPostLatencyStats.minValue : 0.0,
					genMsgSentCount,
					genMsgRecvCount,
					latencyGenMsgSentCount,
					latencyGenMsgRecvCount,
					consumerThreads[i].stats.intervalGenMsgLatencyStats.average,
					sqrt(consumerThreads[i].stats.intervalGenMsgLatencyStats.variance),
					latencyGenMsgRecvCount ? consumerThreads[i].stats.intervalGenMsgLatencyStats.maxValue : 0.0,
					latencyGenMsgRecvCount ? consumerThreads[i].stats.intervalGenMsgLatencyStats.minValue : 0.0,
					resourceStats.cpuUsageFraction * 100.0,
					(double)resourceStats.memUsageBytes / 1048576.0);
			fflush(consumerThreads[i].statsFile);
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

			if (consumerThreads[i].stats.intervalLatencyStats.count > 0)
			{
				printValueStatistics(stdout, "  Latency(usec)", "Msgs", &consumerThreads[i].stats.intervalLatencyStats, RSSL_FALSE);
				clearValueStatistics(&consumerThreads[i].stats.intervalLatencyStats);
			}

			if (postOutOfBuffersCount)
				printf("  - %llu posts not sent due to lack of output buffers.\n", postOutOfBuffersCount);

			if (consumerThreads[i].stats.intervalPostLatencyStats.count > 0)
			{
				printValueStatistics(stdout, "  PostLat(usec)", "Msgs", &consumerThreads[i].stats.intervalPostLatencyStats, RSSL_FALSE);
				clearValueStatistics(&consumerThreads[i].stats.intervalPostLatencyStats);
			}

			if (genMsgSentCount || genMsgRecvCount)
				printf("  GenMsgs: sent %llu, received %llu, latencies sent %llu, latencies received %llu\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
			if (genMsgOutOfBuffersCount)
				printf("  - %llu gen msgs not sent due to lack of output buffers.\n", genMsgOutOfBuffersCount);

			if (consumerThreads[i].stats.intervalGenMsgLatencyStats.count > 0)
			{
				printValueStatistics(stdout, "  GenMsgLat(usec)", "Msgs", &consumerThreads[i].stats.intervalGenMsgLatencyStats, RSSL_FALSE);
				clearValueStatistics(&consumerThreads[i].stats.intervalGenMsgLatencyStats);
			}

			if (consumerThreads[i].stats.intervalUpdateDecodeTimeStats.count > 0)
			{
				printValueStatistics(stdout, "  Update decode time (usec)", "Msgs", &consumerThreads[i].stats.intervalUpdateDecodeTimeStats, RSSL_TRUE);
				clearValueStatistics(&consumerThreads[i].stats.intervalUpdateDecodeTimeStats);
			}

			if (statusCount)
				printf("  - Received %llu status messages.\n", statusCount);
		}


		/* Get Image Retrieval time for this client. */
		if (!totalStats.imageTimeRecorded)
		{
			if (consumerThreads[i].stats.imageRetrievalEndTime)
			{
				TimeValue imageRetrievalStartTime = consumerThreads[i].stats.imageRetrievalStartTime,
						  imageRetrievalEndTime = consumerThreads[i].stats.imageRetrievalEndTime;

				/* To get the total time it took to retrieve all images, find the earliest start time
				 * and latest end time across all connections. */
				if (!totalStats.imageRetrievalStartTime 
						|| imageRetrievalStartTime < totalStats.imageRetrievalStartTime)
					totalStats.imageRetrievalStartTime = imageRetrievalStartTime;
				if (!totalStats.imageRetrievalEndTime || 
						imageRetrievalEndTime > totalStats.imageRetrievalEndTime)
					totalStats.imageRetrievalEndTime = imageRetrievalEndTime; 

			}
			/* Ignore connections that don't request anything. */
			else if (consumerThreads[i].itemListCount > 0)
			{
				allRefreshesRetrieved = RSSL_FALSE; /* Not all connections have received their images yet. */
				totalStats.imageRetrievalStartTime = 0;
				totalStats.imageRetrievalEndTime = 0;
			}
		}


		if (!consumerThreads[i].stats.imageTimeRecorded && consumerThreads[i].stats.imageRetrievalEndTime)
		{
			consumerThreads[i].stats.imageTimeRecorded = RSSL_TRUE;

			if (displayStats)
			{
				TimeValue imageRetrievalTime = consumerThreads[i].stats.imageRetrievalEndTime - 
					consumerThreads[i].stats.imageRetrievalStartTime;

				printf("  - Image retrieval time for %d images: %.3fs (%.0f images/s)\n", 
						consumerThreads[i].itemListCount,
						(double)imageRetrievalTime/1000000000.0,
						(double)(consumerThreads[i].itemListCount)/
						((double)imageRetrievalTime /1000000000.0));
			}
		}
	}


	if (!totalStats.imageTimeRecorded && allRefreshesRetrieved)
	{
		endTime = totalStats.imageRetrievalEndTime + consPerfConfig.steadyStateTime * 1000000000ULL;
		totalStats.imageTimeRecorded = RSSL_TRUE;

		if (consPerfConfig.threadCount > 1)
		{
			if (displayStats)
			{
				/* Print overall image retrieval stats. */
				TimeValue totalRefreshRetrievalTime = (totalStats.imageRetrievalEndTime - 
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
			totalStats.imageRetrievalStartTime = consumerThreads[0].stats.imageRetrievalStartTime;
			totalStats.imageRetrievalEndTime = consumerThreads[0].stats.imageRetrievalEndTime;
		}
	}
}

void consumerCleanupThreads()
{
	int i;

	shutdownThreads = RSSL_TRUE;

	printf("\nShutting down.\n\n");


	for(i = 0; i < consPerfConfig.threadCount; i++)
		RSSL_THREAD_JOIN(consumerThreads[i].threadId);

	/* If there was only one consumer thread, we didn't bother
	 * gathering cross-thread stats, so copy them from the
	 * lone consumer. */
	if (consPerfConfig.threadCount == 1)
		totalStats = consumerThreads[0].stats;
	else
		collectStats(RSSL_FALSE, RSSL_FALSE, 0, 0);

	currentTime = getTimeNano();

	printSummaryStatistics(stdout);
	printSummaryStatistics(summaryFile);

	/* Add a warning if the test failed. */
	for(i = 0; i < consPerfConfig.threadCount; i++)
	{
		if (consumerThreads[i].threadRsslError.rsslErrorId != RSSL_RET_SUCCESS)
		{
			fprintf(stdout, "ERROR: TEST FAILED due to error from Consumer Thread %d:\n"
					"  %d(%s): %s\n"
					"  At %s\n\n",
					i,
					consumerThreads[i].threadErrorInfo.rsslError.rsslErrorId,
					rsslRetCodeToString(consumerThreads[i].threadRsslError.rsslErrorId),
					consumerThreads[i].threadErrorInfo.rsslError.text,
					consumerThreads[i].threadErrorInfo.errorLocation);
			fprintf(summaryFile, "ERROR: TEST FAILED due to error from Consumer Thread %d:\n"
					"  %d(%s): %s\n"
					"  At %s\n\n",
					i,
					consumerThreads[i].threadErrorInfo.rsslError.rsslErrorId,
					rsslRetCodeToString(consumerThreads[i].threadRsslError.rsslErrorId),
					consumerThreads[i].threadErrorInfo.rsslError.text,
					consumerThreads[i].threadErrorInfo.errorLocation);

		}

		consumerThreadCleanup(&consumerThreads[i]);
	}

}


void printSummaryStatistics(FILE *file)
{
	TimeValue firstUpdateTime;
	RsslInt32 i;
	RsslUInt64 totalUpdateCount = countStatGetTotal(&totalStats.startupUpdateCount)
		+ countStatGetTotal(&totalStats.steadyStateUpdateCount);

	/* Find when the first update was received. */
	firstUpdateTime = 0;
	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		if (!firstUpdateTime || consumerThreads[i].stats.firstUpdateTime < firstUpdateTime)
			firstUpdateTime = consumerThreads[i].stats.firstUpdateTime;
	}

	for(i = 0; i < consPerfConfig.threadCount; ++i)
	{
		TimeValue imageRetrievalTime = consumerThreads[i].stats.imageRetrievalEndTime ?
			(consumerThreads[i].stats.imageRetrievalEndTime 
			- consumerThreads[i].stats.imageRetrievalStartTime) : 0;

		/* If there are multiple connections, print individual summaries. */
		if (consPerfConfig.threadCount > 1)
		{
			RsslUInt64 totalClientUpdateCount = 
				countStatGetTotal(&consumerThreads[i].stats.startupUpdateCount)
				+ countStatGetTotal(&consumerThreads[i].stats.steadyStateUpdateCount);

			fprintf(file, "\n--- CLIENT %d SUMMARY ---\n\n", i + 1);

			fprintf( file, "Startup State Statistics:\n");

			fprintf(file, 
					"  Sampling duration (sec): %.3f\n",
					consumerThreads[i].stats.imageRetrievalStartTime ?
					(((double)(consumerThreads[i].stats.imageRetrievalEndTime ? consumerThreads[i].stats.imageRetrievalEndTime : currentTime)
					 - (double)consumerThreads[i].stats.imageRetrievalStartTime )/1000000000.0) : 0.0);

			if (consumerThreads[i].stats.startupLatencyStats.count)
			{
				fprintf( file, 
						"  Latency avg (usec): %.1f\n"
						"  Latency std dev (usec): %.1f\n"
						"  Latency max (usec): %.1f\n"
						"  Latency min (usec): %.1f\n",
						consumerThreads[i].stats.startupLatencyStats.average,
						sqrt(consumerThreads[i].stats.startupLatencyStats.variance),
						consumerThreads[i].stats.startupLatencyStats.maxValue,
						consumerThreads[i].stats.startupLatencyStats.minValue);
			}
			else
				fprintf( file, "  No latency information received during startup time.\n\n");

			fprintf(file, "  Avg update rate: %.0f\n", 
					(double)countStatGetTotal(&consumerThreads[i].stats.startupUpdateCount)/
					(double)((
							(consumerThreads[i].stats.imageRetrievalEndTime ?
							consumerThreads[i].stats.imageRetrievalEndTime : currentTime)
							- consumerThreads[i].stats.firstUpdateTime
							)/1000000000.0));

			fprintf( file, "\nSteady State Statistics:\n");

			if (consumerThreads[i].stats.imageRetrievalEndTime)
			{
				fprintf(file, 
						"  Sampling duration (sec): %.3f\n",
						((double)currentTime - (double)consumerThreads[i].stats.imageRetrievalEndTime)/1000000000.0);
				if (consumerThreads[i].stats.steadyStateLatencyStats.count)
				{
					fprintf( file,
							"  Latency avg (usec): %.1f\n"
							"  Latency std dev (usec): %.1f\n"
							"  Latency max (usec): %.1f\n"
							"  Latency min (usec): %.1f\n",
							consumerThreads[i].stats.steadyStateLatencyStats.average,
							sqrt(consumerThreads[i].stats.steadyStateLatencyStats.variance),
							consumerThreads[i].stats.steadyStateLatencyStats.maxValue,
							consumerThreads[i].stats.steadyStateLatencyStats.minValue);
				}
				else
					fprintf( file, "  No latency information was received during steady-state time.\n");

				if (consPerfConfig.latencyPostsPerSec)
				{
					if (consumerThreads[i].stats.postLatencyStats.count)
					{
						fprintf( file,
								"  Posting latency avg (usec): %.1f\n"
								"  Posting latency std dev (usec): %.1f\n"
								"  Posting latency max (usec): %.1f\n"
								"  Posting latency min (usec): %.1f\n",
								consumerThreads[i].stats.postLatencyStats.average,
								sqrt(consumerThreads[i].stats.postLatencyStats.variance),
								consumerThreads[i].stats.postLatencyStats.maxValue,
								consumerThreads[i].stats.postLatencyStats.minValue);
					}
					else
						fprintf( file, "  No posting latency information was received during steady-state time.\n");
				}

				fprintf(file, "  Avg update rate: %.0f\n", 
						(double)countStatGetTotal(&consumerThreads[i].stats.steadyStateUpdateCount)
						/(double)((currentTime - consumerThreads[i].stats.imageRetrievalEndTime)/1000000000.0));
			}
			else
				fprintf( file, "  Steady state was not reached during this test.\n\n");


			fprintf(file, "\nOverall Statistics: \n");

			fprintf(file, 
					"  Sampling duration (sec): %.3f\n",
					consumerThreads[i].stats.imageRetrievalStartTime ?
					((double)currentTime
					 - (double)consumerThreads[i].stats.imageRetrievalStartTime )/1000000000.0 : 0.0);

			if (consumerThreads[i].stats.overallLatencyStats.count)
			{
				fprintf( file,
						"  Latency avg (usec): %.1f\n"
						"  Latency std dev (usec): %.1f\n"
						"  Latency max (usec): %.1f\n"
						"  Latency min (usec): %.1f\n",
						consumerThreads[i].stats.overallLatencyStats.average,
						sqrt(consumerThreads[i].stats.overallLatencyStats.variance),
						consumerThreads[i].stats.overallLatencyStats.maxValue,
						consumerThreads[i].stats.overallLatencyStats.minValue);
			}
			else
				fprintf( file, "  No latency information was received.\n");

			if (consumerThreads[i].stats.genMsgLatencyStats.count)
			{
				fprintf( file,
						"  GenMsg latency avg (usec): %.1f\n"
						"  GenMsg latency std dev (usec): %.1f\n"
						"  GenMsg latency max (usec): %.1f\n"
						"  GenMsg latency min (usec): %.1f\n",
						consumerThreads[i].stats.genMsgLatencyStats.average,
						sqrt(consumerThreads[i].stats.genMsgLatencyStats.variance),
						consumerThreads[i].stats.genMsgLatencyStats.maxValue,
						consumerThreads[i].stats.genMsgLatencyStats.minValue);
			}
			else
				fprintf( file, "  No GenMsg latency information was received.\n");

			fprintf(file, "\nTest Statistics:\n");

			fprintf(file, 
					"  Requests sent: %llu\n"
					"  Refreshes received: %llu\n"
					"  Updates received: %llu\n",
					countStatGetTotal(&consumerThreads[i].stats.requestCount),
					countStatGetTotal(&consumerThreads[i].stats.refreshCount),
					totalClientUpdateCount);

			if (consPerfConfig.postsPerSec)
			{
				fprintf(file, 
						"  Posts sent: %llu\n",
						countStatGetTotal(&consumerThreads[i].stats.postSentCount));
			}

			if (consPerfConfig.genMsgsPerSec)
				fprintf(file, "  GenMsgs sent: %llu\n", countStatGetTotal(&consumerThreads[i].stats.genMsgSentCount));
			if (countStatGetTotal(&consumerThreads[i].stats.genMsgRecvCount))
				fprintf(file, "  GenMsgs received: %llu\n", countStatGetTotal(&consumerThreads[i].stats.genMsgRecvCount));
			if (consPerfConfig.latencyGenMsgsPerSec)
				fprintf(file, "  GenMsg latencies sent: %llu\n", countStatGetTotal(&consumerThreads[i].stats.latencyGenMsgSentCount));
			if (consumerThreads[i].stats.genMsgLatencyStats.count)
				fprintf(file, "  GenMsg latencies received: %llu\n", consumerThreads[i].stats.genMsgLatencyStats.count);

			if (imageRetrievalTime)
			{
				fprintf(file,
						"  Image retrieval time (sec): %.3f\n"
						"  Avg image rate: %.0f\n",
						(double)imageRetrievalTime/1000000000.0,
						countStatGetTotal(&consumerThreads[i].stats.refreshCount)
						/((double)imageRetrievalTime/1000000000.0)); 
			}

			fprintf(file, "  Avg update rate: %.0f\n",
					(double)totalClientUpdateCount
					/(double)((currentTime - consumerThreads[i].stats.firstUpdateTime)/1000000000.0));

			if (consPerfConfig.postsPerSec)
			{
				fprintf(file, "  Avg posting rate: %.0f\n", 
						(double)countStatGetTotal(&consumerThreads[i].stats.postSentCount)/(double)((currentTime - consumerThreads[i].stats.imageRetrievalEndTime)/1000000000.0));
			}

			if (consPerfConfig.genMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg send rate: %.0f\n", 
					(double)countStatGetTotal(&consumerThreads[i].stats.genMsgSentCount)/
					(double)((currentTime - consumerThreads[i].stats.firstGenMsgSentTime)/1000000000.0));
			}
			if (countStatGetTotal(&consumerThreads[i].stats.genMsgRecvCount))
			{
				fprintf(file, "  Avg GenMsg receive rate: %.0f\n", 
					(double)countStatGetTotal(&consumerThreads[i].stats.genMsgRecvCount)/
					(double)((currentTime - consumerThreads[i].stats.firstGenMsgRecvTime)/1000000000.0));
			}
			if (consPerfConfig.latencyGenMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", 
					(double)countStatGetTotal(&consumerThreads[i].stats.latencyGenMsgSentCount)/
					(double)((currentTime - consumerThreads[i].stats.firstGenMsgSentTime)/1000000000.0));
			}
			if (consumerThreads[i].stats.genMsgLatencyStats.count)
			{
				fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", 
					(double)(consumerThreads[i].stats.genMsgLatencyStats.count)/
					(double)((currentTime - consumerThreads[i].stats.firstGenMsgRecvTime)/1000000000.0));
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
				totalStats.startupLatencyStats.average,
				sqrt(totalStats.startupLatencyStats.variance),
				totalStats.startupLatencyStats.maxValue,
				totalStats.startupLatencyStats.minValue);
	}
	else
		fprintf( file, "  No latency information received during startup time.\n\n");

	fprintf(file, "  Avg update rate: %.0f\n\n", 
			(double)countStatGetTotal(&totalStats.startupUpdateCount)
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
					totalStats.steadyStateLatencyStats.average,
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
						totalStats.postLatencyStats.average,
						sqrt(totalStats.postLatencyStats.variance),
						totalStats.postLatencyStats.maxValue,
						totalStats.postLatencyStats.minValue);
			}
			else
				fprintf( file, "  No posting latency information was received during steady-state time.\n");
		}

		fprintf(file, "  Avg update rate: %.0f\n", 
				(double)countStatGetTotal(&totalStats.steadyStateUpdateCount)
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
				totalStats.overallLatencyStats.average,
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
				totalStats.genMsgLatencyStats.average,
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
				cpuUsageStats.average * 100.0,
				memUsageStats.maxValue / 1048576.0,
				memUsageStats.minValue / 1048576.0,
				memUsageStats.average / 1048576.0
			   );
	}
	
	fprintf(file, "\nTest Statistics:\n");

	fprintf(file, 
			"  Requests sent: %llu\n"
			"  Refreshes received: %llu\n"
			"  Updates received: %llu\n",
			countStatGetTotal(&totalStats.requestCount),
			countStatGetTotal(&totalStats.refreshCount),
			totalUpdateCount);
	if (consPerfConfig.postsPerSec)
	{
		fprintf(file, 
				"  Posts sent: %llu\n",
				countStatGetTotal(&totalStats.postSentCount));
	}

	if (consPerfConfig.genMsgsPerSec)
		fprintf(file, "  GenMsgs sent: %llu\n", countStatGetTotal(&totalStats.genMsgSentCount));
	if (countStatGetTotal(&totalStats.genMsgRecvCount))
		fprintf(file, "  GenMsgs received: %llu\n", countStatGetTotal(&totalStats.genMsgRecvCount));
	if (consPerfConfig.latencyGenMsgsPerSec)
		fprintf(file, "  GenMsg latencies sent: %llu\n", countStatGetTotal(&totalStats.latencyGenMsgSentCount));
	if (totalStats.genMsgLatencyStats.count)
		fprintf(file, "  GenMsg latencies received: %llu\n", totalStats.genMsgLatencyStats.count);

	if (totalStats.imageRetrievalEndTime)
	{
		TimeValue totalRefreshRetrievalTime = (totalStats.imageRetrievalEndTime - 
				totalStats.imageRetrievalStartTime);

		fprintf(file,
				"  Image retrieval time (sec): %.3f\n"
				"  Avg image rate: %.0f\n",
				(double)totalRefreshRetrievalTime/1000000000.0,
				(double)consPerfConfig.itemRequestCount/((double)totalRefreshRetrievalTime/1000000000.0));
	}


	fprintf(file, "  Avg update rate: %.0f\n", 
			(double)totalUpdateCount/(double)((currentTime - firstUpdateTime)/1000000000.0));

	if (countStatGetTotal(&totalStats.postSentCount))
	{
		fprintf(file, "  Avg posting rate: %.0f\n", 
				(double)countStatGetTotal(&totalStats.postSentCount)/(double)((currentTime - totalStats.imageRetrievalEndTime)/1000000000.0));
	}

	//calculate the first GenMsg Sent & Recv times
	totalStats.firstGenMsgSentTime = consumerThreads[0].stats.firstGenMsgSentTime;
	totalStats.firstGenMsgRecvTime = consumerThreads[0].stats.firstGenMsgRecvTime;
	for(i = 1; i < consPerfConfig.threadCount; ++i)
	{
		if(consumerThreads[i].stats.firstGenMsgSentTime && consumerThreads[i].stats.firstGenMsgSentTime < totalStats.firstGenMsgSentTime) 
			totalStats.firstGenMsgSentTime = consumerThreads[i].stats.firstGenMsgSentTime;
		if(consumerThreads[i].stats.firstGenMsgRecvTime && consumerThreads[i].stats.firstGenMsgRecvTime < totalStats.firstGenMsgRecvTime)
			totalStats.firstGenMsgRecvTime = consumerThreads[i].stats.firstGenMsgRecvTime;
	}

	if (consPerfConfig.genMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg send rate: %.0f\n", 
			(double)countStatGetTotal(&totalStats.genMsgSentCount)/
			(double)((currentTime - totalStats.firstGenMsgSentTime)/1000000000.0));
	}
	if (countStatGetTotal(&totalStats.genMsgRecvCount))
	{
		fprintf(file, "  Avg GenMsg receive rate: %.0f\n", 
			(double)countStatGetTotal(&totalStats.genMsgRecvCount)/
			(double)((currentTime - totalStats.firstGenMsgRecvTime)/1000000000.0));
	}
	if (consPerfConfig.latencyGenMsgsPerSec)
	{
		fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", 
			(double)countStatGetTotal(&totalStats.latencyGenMsgSentCount)/
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

