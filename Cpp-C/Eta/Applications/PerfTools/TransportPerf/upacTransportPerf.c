/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "transportPerfConfig.h"
#include "statistics.h"
#include "upacTransportPerf.h"
#include "getTime.h" 
#include "testUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#ifdef WIN32
#define getpid _getpid
#else
#include <unistd.h>
#endif


#ifdef __cplusplus
extern "C" {
	static void signal_handler(int sig);
}
#endif

static RsslBool signal_shutdown = RSSL_FALSE;
static fd_set	readfds;
static fd_set	exceptfds;
static RsslInt64 runTime = 0;

static RsslServer *rsslSrvr = NULL;		// used if we do an rsslBind

static RsslInt64 nsecPerTick;

static RsslInt32 sessionHandlerCount;
static SessionHandler *sessionHandlerList;

static ResourceUsageStats resourceStats;
static ValueStatistics cpuUsageStats;
static ValueStatistics memUsageStats;
static ValueStatistics totalLatencyStats;

static RsslUInt32 currentRuntimeSec = 0, intervalSeconds = 0;

RsslUInt64 totalMsgSentCount = 0;
RsslUInt64 totalBytesSent = 0;
RsslUInt64 totalMsgReceivedCount = 0;
RsslUInt64 totalBytesReceived = 0;

static const RsslUInt32 TEST_PROTOCOL_TYPE = 88;

static RsslUInt32 outBytesTotal = 0;
static RsslUInt32 uncompOutBytesTotal = 0;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

static void signal_handler(int sig)
{
	signal_shutdown = RSSL_TRUE;
}


static void printSummaryStats(FILE *file);

RsslRet processMsg(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslBuffer *pMsgBuf)
{
	SessionHandler *pHandler = (SessionHandler*)pChanHandler->pUserSpec;
	TransportSession *pSession = (TransportSession*)pChannelInfo->pUserSpec;
	TimeValue timeTracker;

	countStatIncr(&pHandler->transportThread.msgsReceived);
	countStatAdd(&pHandler->transportThread.bytesReceived, pMsgBuf->length);

	if (pMsgBuf->length < 16)
	{
		printf("Error: Message was too small to be valid(length %u).\n", pMsgBuf->length);
		return RSSL_RET_FAILURE;
	}

	if (pSession->receivedFirstSequenceNumber)
	{
		RsslUInt64 recvSequenceNumber;

		memcpy(&recvSequenceNumber, pMsgBuf->data, 8);

		if (pSession->recvSequenceNumber != recvSequenceNumber)
		{
			printf("Error: Received out-of-order sequence number(%llu instead of %llu).\n", recvSequenceNumber, pSession->recvSequenceNumber);
			return RSSL_RET_FAILURE;
		}

		++pSession->recvSequenceNumber;

		memcpy(&timeTracker, pMsgBuf->data + 8, 8);

		if (timeTracker)
		{
			timeRecordSubmit(&pHandler->latencyRecords, timeTracker, getTimeNano(), 1000);
		}
		return RSSL_RET_SUCCESS;
	}
	else
	{
		memcpy(&pSession->recvSequenceNumber, pMsgBuf->data, 8);
		pSession->receivedFirstSequenceNumber = RSSL_TRUE;
		++pSession->recvSequenceNumber;
		return RSSL_RET_SUCCESS;
	}
}

RsslRet processMsgReflect(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslBuffer *pInBuf)
{
	SessionHandler *pHandler = (SessionHandler*)pChanHandler->pUserSpec;
	TransportSession *pSession = (TransportSession*)pChannelInfo->pUserSpec;
	RsslChannel *chnl = pSession->pChannelInfo->pChannel;
	RsslRet ret;
	RsslError error;
	RsslBuffer *pOutBuffer;
	RsslUInt32 outBytes, uncompOutBytes;

	countStatIncr(&pHandler->transportThread.msgsReceived);
	countStatAdd(&pHandler->transportThread.bytesReceived, pInBuf->length);

	if ((pOutBuffer = rsslGetBuffer(chnl, pInBuf->length, RSSL_FALSE, &error)) == NULL)
	{
		if (error.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
			return error.rsslErrorId;

		ret = rsslFlush(chnl, &error);
		if(rtrUnlikely(ret < RSSL_RET_SUCCESS))
			return ret;

		if ((pOutBuffer = rsslGetBuffer(chnl, pInBuf->length, RSSL_FALSE, &error)) == NULL)
		{
			printf("Getbuffer failed after attempting to flush at %s:%d\n", __FILE__, __LINE__);
			return error.rsslErrorId;
		}
	}

	if (pOutBuffer->length < pInBuf->length)
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	memcpy((void*)pOutBuffer->data, (void*)pInBuf->data, pInBuf->length);
	
	pOutBuffer->length = pInBuf->length;
	
	ret = rsslWrite(chnl, pOutBuffer, RSSL_HIGH_PRIORITY, transportThreadConfig.writeFlags, &outBytes, &uncompOutBytes, &error);
	/* call flush and write again */
	while (rtrUnlikely(ret == RSSL_RET_WRITE_CALL_AGAIN))
	{
		if (rtrUnlikely((ret = rsslFlush(chnl, &error)) < RSSL_RET_SUCCESS))
		{
			printf("rsslFlush() failed with return code %d - <%s>\n", ret, error.text);
			return ret;
		}
		ret = rsslWrite(chnl, pOutBuffer, RSSL_HIGH_PRIORITY, transportThreadConfig.writeFlags, &outBytes, &uncompOutBytes, &error);
	}

	if (ret >= RSSL_RET_SUCCESS)
	{
		countStatAdd(&pHandler->transportThread.bytesSent, outBytes);
		countStatIncr(&pHandler->transportThread.msgsSent);
		if(ret > 0)
			channelHandlerRequestFlush(pChanHandler, pSession->pChannelInfo);
		return ret;
	}

	switch(ret)
	{
		case RSSL_RET_WRITE_FLUSH_FAILED:
			if (chnl->state == RSSL_CH_STATE_ACTIVE)
			{
				countStatAdd(&pHandler->transportThread.bytesSent, outBytes);
				countStatIncr(&pHandler->transportThread.msgsSent);
				channelHandlerRequestFlush(pChanHandler, pSession->pChannelInfo);
				return 1;
			}
			/* Otherwise treat as error, fall through to default. */
		default:
			printf("rsslWrite() failed: %s(%s)\n", rsslRetCodeToString(error.rsslErrorId), 
					error.text);
			return ret;
	}
}


RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo)
{
	SessionHandler *pHandler = (SessionHandler*)pChanHandler->pUserSpec;
	TransportSession *pSession = (TransportSession*)pChannelInfo->pUserSpec;
	RsslRet ret;
	RsslError error;
	RsslChannelInfo channelInfo;
	RsslUInt32 count;

	if (transportPerfConfig.highWaterMark > 0)
	{
		if ((ret = rsslIoctl(pChannelInfo->pChannel, RSSL_HIGH_WATER_MARK, &transportPerfConfig.highWaterMark, &error)) != RSSL_RET_SUCCESS)
		{
			printf("Failed to set high water mark: %s (%s)\n", rsslRetCodeToString(ret), error.text);
			return RSSL_RET_FAILURE;
		}
	}

	/* Record first connection time. */
	if (!pHandler->transportThread.connectTime)
		pHandler->transportThread.connectTime = getTimeNano();

	/* Print component version. */
	if ((ret = rsslGetChannelInfo(pChannelInfo->pChannel, &channelInfo, &error)) != RSSL_RET_SUCCESS)
	{
		printf("rsslGetChannelInfo() failed: %d\n", ret);
		return 0;
	} 

	printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
			"  maxFragmentSize: %u\n"
			"  maxOutputBuffers: %u\n"
			"  guaranteedOutputBuffers: %u\n"
			"  numInputBuffers: %u\n"
			"  pingTimeout: %u\n"
			"  clientToServerPings: %s\n"
			"  serverToClientPings: %s\n"
			"  sysSendBufSize: %u\n"
			"  sysSendBufSize: %u\n"			
			"  compressionType: %s\n"
			"  compressionThreshold: %u\n"			
			"  ComponentInfo: ", 
			pChannelInfo->pChannel->socketId,
			channelInfo.maxFragmentSize,
			channelInfo.maxOutputBuffers, channelInfo.guaranteedOutputBuffers,
			channelInfo.numInputBuffers,
			channelInfo.pingTimeout,
			channelInfo.clientToServerPings == RSSL_TRUE ? "true" : "false",
			channelInfo.serverToClientPings == RSSL_TRUE ? "true" : "false",
			channelInfo.sysSendBufSize, channelInfo.sysRecvBufSize,			
			compressionTypeToString(channelInfo.compressionType),
			channelInfo.compressionThreshold			
			);
	if (channelInfo.componentInfoCount == 0)
		printf("(No component info)");
	else
		for(count = 0; count < channelInfo.componentInfoCount; ++count)
		{
			printf("%.*s", 
					channelInfo.componentInfo[count]->componentVersion.length,
					channelInfo.componentInfo[count]->componentVersion.data);
			if (count < channelInfo.componentInfoCount - 1)
				printf(", ");
		}
	printf ("\n\n");

	if (transportThreadConfig.totalBuffersPerPack > 1
			&& (transportThreadConfig.msgSize + 8) * transportThreadConfig.totalBuffersPerPack > channelInfo.maxFragmentSize)
	{
		printf("Error(Channel "SOCKET_PRINT_TYPE"): MaxFragmentSize %u is too small for packing buffer size %u\n",
				pChannelInfo->pChannel->socketId, channelInfo.maxFragmentSize, 
				(transportThreadConfig.msgSize + 8) * transportThreadConfig.totalBuffersPerPack);
		exit(-1);
	}

	pSession->timeActivated = getTimeNano();

	return RSSL_RET_SUCCESS;
}

void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError)
{
	SessionHandler *pHandler = (SessionHandler*)pChanHandler->pUserSpec;
	TransportSession *pSession = (TransportSession*)pChannelInfo->pUserSpec;

	/* If the channel was active and this is a client, we won't attempt to reconnect,
	 * so stop the test. */
	if (!signal_shutdown && transportPerfConfig.appType == APPTYPE_CLIENT
			&& pSession->timeActivated)
	{
		signal_shutdown = RSSL_TRUE;
	}

	if(pError)
		printf("Channel Closed: %s(%s)\n", rsslRetCodeToString(pError->rsslErrorId), pError->text);
	else
		printf("Channel Closed.\n");

	if (pSession)
		transportSessionDestroy(&pHandler->transportThread, pSession);

	RSSL_MUTEX_LOCK(&pHandler->handlerLock);
	--pHandler->openChannelsCount;
	RSSL_MUTEX_UNLOCK(&pHandler->handlerLock);

	/* Record last disconnection time. */
	pHandler->transportThread.disconnectTime = getTimeNano();
}


#ifdef __cplusplus
extern "C" {
#endif
RSSL_THREAD_DECLARE(runConnectionHandler, pArg)
{

	SessionHandler *pHandler = (SessionHandler*)pArg;
	TransportThread *pTransportThread = &pHandler->transportThread;
	TimeValue currentTime, nextTickTime;
	struct timeval time_interval;
	RsslInt32 tickSetMsgCount = 0; /* Holds the number of messages sent in one run of ticks */
	RsslInt32 currentTicks = 0;

	RsslQueueLink *pLink;

	if (pHandler->cpuId >= 0)
	{
		if (bindThread(pHandler->cpuId) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %d.\n", pHandler->cpuId);
			exit(-1);
		}
	}

	if (transportPerfConfig.appType == APPTYPE_CLIENT)
	{
		do
		{
			RsslRet ret;
			RsslChannel *pChannel;
			TransportSession *pSession;

			if (!(pChannel = startConnection()))
			{
				SLEEP(1);
				continue;
			}

			if (!(pSession = transportSessionCreate(pTransportThread, pChannel)))
			{
				printf("providerSessionInit() failed\n");
				exit(-1);
			}

			if (pChannel->state == RSSL_CH_STATE_ACTIVE)
				break;

			do
			{
				ret = channelHandlerWaitForChannelInit(&pTransportThread->channelHandler, 
						pSession->pChannelInfo, 100000);
			} while (!signal_shutdown && ret == RSSL_RET_CHAN_INIT_IN_PROGRESS);

			if (ret < RSSL_RET_SUCCESS)
			{
				SLEEP(1);
				continue;
			}
			else
				break; /* Successful initialization. */

		} while (!signal_shutdown);
	}

	currentTime = getTimeNano();
	nextTickTime = currentTime + nsecPerTick;

	time_interval.tv_sec = 0; time_interval.tv_usec = 0;
	
	/* this is the main loop */
	while(!signal_shutdown)
	{
		for(currentTicks = 0; currentTicks < transportThreadConfig.ticksPerSec && !signal_shutdown; ++currentTicks)
		{
			if (!transportPerfConfig.busyRead)
			{
				channelHandlerReadChannels(&pTransportThread->channelHandler, nextTickTime);

				/* We've reached the next tick. Send a burst of messages out */
				nextTickTime += nsecPerTick;
				if(pHandler->role & ROLE_WRITER)
				{
					RSSL_QUEUE_FOR_EACH_LINK(&pTransportThread->channelHandler.activeChannelList, pLink)
					{
						ChannelInfo *pChannelInfo;
						TransportSession *pSession;
						RsslRet ret;

						pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
						pSession = (TransportSession*)pChannelInfo->pUserSpec;

						/* The application corrects for ticks that don't finish before the time 
						 * that the next message burst should start.  But don't do this correction 
						 * for new channels. */
						if (rtrUnlikely(nextTickTime < pSession->timeActivated)) continue; 

						/* Send burst of messages */
						ret = transportSessionSendMsgBurst(&pHandler->transportThread, pSession, &tickSetMsgCount);

						if (rtrUnlikely(ret < RSSL_RET_SUCCESS))
						{
							switch(ret)
							{
								case RSSL_RET_BUFFER_NO_BUFFERS:
									channelHandlerRequestFlush(&pTransportThread->channelHandler, pChannelInfo);
									break;
								default:
									printf("Failure while writing message bursts: %s\n",
											rsslRetCodeToString(ret));
									channelHandlerCloseChannel(&pTransportThread->channelHandler, pChannelInfo, NULL);
									break;
							}
						}
						else if (rtrUnlikely(ret > RSSL_RET_SUCCESS))
						{
							/* Need to flush */
							channelHandlerRequestFlush(&pTransportThread->channelHandler, pChannelInfo);
						}
					}
				}
			}
			else
			{
				ChannelInfo *pChannelInfo;

				RSSL_QUEUE_FOR_EACH_LINK(&pTransportThread->channelHandler.activeChannelList, pLink)
				{
					pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
					channelHandlerReadChannel(&pTransportThread->channelHandler, pChannelInfo);
				}

				RSSL_QUEUE_FOR_EACH_LINK(&pTransportThread->channelHandler.initializingChannelList, pLink)
				{
					pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
					channelHandlerInitializeChannel(&pTransportThread->channelHandler, pChannelInfo);
				}
			}


			do
			{
				/* Check if there are any new connections. */

				RSSL_MUTEX_LOCK(&pHandler->handlerLock);
				pLink = rsslQueueRemoveFirstLink(&pHandler->newChannelsList);
				RSSL_MUTEX_UNLOCK(&pHandler->handlerLock);

				if (pLink)
				{
					NewChannel *pNewChannel = RSSL_QUEUE_LINK_TO_OBJECT(NewChannel, queueLink, pLink);
					TransportSession *pSession;

					if (!(pSession = transportSessionCreate(&pHandler->transportThread, pNewChannel->pChannel)))
					{
						printf("transportSessionCreate() failed\n");
						exit(-1);
					}

					free(pNewChannel);
				}
			} while (pLink);
		}

		channelHandlerCheckPings(&pTransportThread->channelHandler);

	}

	/* Shutdown */
	RSSL_QUEUE_FOR_EACH_LINK(&pTransportThread->channelHandler.activeChannelList, pLink)
	{
		ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
		channelHandlerCloseChannel(&pTransportThread->channelHandler, pChannelInfo, NULL);
	}

	return RSSL_THREAD_RETURN();
}
#ifdef __cplusplus
};
#endif

int main(int argc, char **argv)
{
	int i;
	struct timeval time_interval;
	RsslError error;
	fd_set useRead;
	fd_set useExcept;
	int selRet;

	TimeValue currentTime, nextTickTime;
	RsslInt32 currentTicks;

	RsslQueue latencyRecords;

	/* Read in configuration and echo it. */
	initTransportPerfConfig(argc, argv);
	printTransportPerfConfig(stdout);

	if (!(summaryFile = fopen(transportPerfConfig.summaryFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", transportPerfConfig.summaryFilename);
		exit(-1);
	}

	printTransportPerfConfig(summaryFile); fflush(summaryFile);

	// set up a signal handler so we can cleanup before exit
	signal(SIGINT, signal_handler);

	/* Determine update rates on per-tick basis */
	nsecPerTick = 1000000000LL/(RsslInt64)transportThreadConfig.ticksPerSec;

	currentTime = getTimeNano();
	nextTickTime = currentTime;
	currentTicks = 0;

	sessionHandlerCount = transportPerfConfig.threadCount;
	sessionHandlerList = (SessionHandler*)malloc(sessionHandlerCount * sizeof(SessionHandler));
	assert(sessionHandlerList);

	/* Initialize RSSL */
	/* Multicast statistics are retrieved via rsslGetChannelInfo(), so set the per-channel-lock 
	 * when taking them. */
	if (rsslInitialize(transportPerfConfig.takeMCastStats ? RSSL_LOCK_GLOBAL_AND_CHANNEL :
					(transportPerfConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE),
					 &error) != RSSL_RET_SUCCESS)
	{
		printf("RsslInitialize failed: %s\n", error.text);
		exit(-1);
	}
	/* Initialize run-time */
	initRuntime();

	for (i = 0; i < sessionHandlerCount; ++i)
	{
		sessionHandlerInit(&sessionHandlerList[i]);
		sessionHandlerList[i].cpuId = transportPerfConfig.threadBindList[i];
		if(transportPerfConfig.reflectMsgs)
		{
			sessionHandlerList[i].role = ROLE_REFLECTOR;
			sessionHandlerList[i].active = RSSL_TRUE;
			transportThreadInit(&sessionHandlerList[i].transportThread,
					processActiveChannel,
					processInactiveChannel,
					processMsgReflect, i);
		}
		else
		{
			sessionHandlerList[i].role = (transportTestRole)(ROLE_WRITER | ROLE_READER);
			sessionHandlerList[i].active = RSSL_TRUE;
			transportThreadInit(&sessionHandlerList[i].transportThread,
					processActiveChannel,
					processInactiveChannel,
					processMsg, i);
		}

		sessionHandlerList[i].transportThread.channelHandler.pUserSpec = &sessionHandlerList[i];

			
		
		if (!CHECK(RSSL_THREAD_START(&sessionHandlerList[i].threadId, runConnectionHandler, &sessionHandlerList[i]) >= 0))
			exit(-1);
	}

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);

	if (transportPerfConfig.appType == APPTYPE_SERVER)
	{
		if ((rsslSrvr = bindRsslServer(&error)) == NULL)
		{
			printf("Bind failed: %s\n", error.text);
			exit(-1);
		}
	}
	

	rsslInitQueue(&latencyRecords);

	clearValueStatistics(&cpuUsageStats);
	clearValueStatistics(&memUsageStats);
	clearValueStatistics(&totalLatencyStats);

	time_interval.tv_sec = 0; time_interval.tv_usec = 0;
	nextTickTime = getTimeNano() + nsecPerTick;
	currentTicks = 0;

	if (initResourceUsageStats(&resourceStats) != RSSL_RET_SUCCESS)
	{
		printf("initResourceUsageStats() failed.\n");
		exit(-1);
	}

	/* this is the main loop */
	while(1)
	{
		useRead = readfds;
		useExcept = exceptfds;

#ifdef WIN32
		/* Windows does not allow select() to be called with empty file descriptor sets. */
		if (transportPerfConfig.appType == APPTYPE_CLIENT)
		{
			currentTime = getTimeNano();
			selRet = 0;
			Sleep( (DWORD)((currentTime < nextTickTime) ? (nextTickTime - currentTime)/1000000 : 0));
		}
		else
#endif
		{
			/* select() on remaining time for this tick. If we went into the next tick, don't delay at all. */
			currentTime = getTimeNano();
			time_interval.tv_usec = (long)((currentTime > nextTickTime) ? 0 : ((nextTickTime - currentTime)/1000));


			selRet = select(FD_SETSIZE, &useRead, NULL, &useExcept, &time_interval);
		}

		if (selRet == 0)
		{
			/* We've reached the next tick. */
			nextTickTime += nsecPerTick;

			if (++currentTicks == transportThreadConfig.ticksPerSec)
			{
				++currentRuntimeSec;
				++intervalSeconds;
				if (intervalSeconds == transportPerfConfig.writeStatsInterval)
				{
					collectStats(RSSL_TRUE, transportPerfConfig.displayStats, currentRuntimeSec, transportPerfConfig.writeStatsInterval);
					intervalSeconds = 0;
				}
				currentTicks = 0;
			}
		}
		else if (selRet > 0) /* messages received, create new client session or read from channel */
		{
			if ((rsslSrvr != NULL) && (rsslSrvr->socketId != -1) && (FD_ISSET(rsslSrvr->socketId,&useRead)))
			{
				RsslChannel *chnl;
				RsslError	error;
				RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

				if ((chnl = rsslAccept(rsslSrvr, &acceptOpts, &error)) == 0)
				{
					printf("rsslAccept: failed <%s>\n",error.text);
				}
				else
				{
					printf("Server "SOCKET_PRINT_TYPE" accepting channel "SOCKET_PRINT_TYPE".\n\n", rsslSrvr->socketId, chnl->socketId);
					sendToLeastLoadedThread(chnl);
				}
			}
		}
		else if (selRet < 0)
		{
			/* continue */
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;
#else
			if (errno == EINTR)
				continue;
#endif
			
			perror("select");
			cleanUpAndExit();
		}

		/* Handle run-time */
		handleRuntime(currentTime);
	}
}

void collectStats(RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec, RsslUInt32 timePassedSec)
{
	RsslInt64 intervalMsgSentCount = 0, intervalBytesSent = 0,
			  intervalMsgReceivedCount = 0, intervalBytesReceived = 0,
			  intervalOutOfBuffersCount = 0;
	ValueStatistics intervalLatencyStats;
	RsslRet ret;
	RsslInt32 i;

	if (timePassedSec)
	{
		if ((ret = getResourceUsageStats(&resourceStats)) != RSSL_RET_SUCCESS)
		{
			printf("getResourceUsageStats() failed: %d\n", ret);
			exit(-1);
		}
		updateValueStatistics(&cpuUsageStats, resourceStats.cpuUsageFraction);
		updateValueStatistics(&memUsageStats, (double)resourceStats.memUsageBytes);
	}

	for(i = 0; i < sessionHandlerCount; ++i)
	{
		RsslQueueLink *pLink;
		RsslQueue latencyRecords;

		rsslInitQueue(&latencyRecords);

		timeRecordQueueGet(&sessionHandlerList[i].latencyRecords, &latencyRecords);

		clearValueStatistics(&intervalLatencyStats);


		RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
		{
			TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;
			if (pRecord->startTime > pRecord->endTime) 
			{	// if the start time is after the end time, then there is probably an issue with timing on the machine	
				static RsslBool badFound = RSSL_FALSE;
				if (!badFound) 
				{
					const char *errStr = "Warning: negative latency calculated. The clocks do not appear to be synchronized. (start=%llu, end=%llu)\n";
					printf(errStr, pRecord->startTime, pRecord->endTime);
					fprintf(summaryFile, errStr, pRecord->startTime, pRecord->endTime);
				}

				badFound = RSSL_TRUE;
			}
			updateValueStatistics(&intervalLatencyStats, latency);
			updateValueStatistics(&sessionHandlerList[i].transportThread.latencyStats, latency);
			updateValueStatistics(&totalLatencyStats, latency);

			if (transportThreadConfig.logLatencyToFile)
				fprintf(sessionHandlerList[i].transportThread.latencyLogFile, "%llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, pRecord->endTime - pRecord->startTime);
		}

		timeRecordQueueRepool(&sessionHandlerList[i].latencyRecords, &latencyRecords);

		if (transportThreadConfig.logLatencyToFile)
			fflush(sessionHandlerList[i].transportThread.latencyLogFile);

		intervalMsgSentCount = countStatGetChange(&sessionHandlerList[i].transportThread.msgsSent);
		intervalBytesSent = countStatGetChange(&sessionHandlerList[i].transportThread.bytesSent);
		intervalMsgReceivedCount = countStatGetChange(&sessionHandlerList[i].transportThread.msgsReceived);
		intervalBytesReceived = countStatGetChange(&sessionHandlerList[i].transportThread.bytesReceived);
		intervalOutOfBuffersCount = countStatGetChange(&sessionHandlerList[i].transportThread.outOfBuffersCount);

		totalMsgSentCount += intervalMsgSentCount;
		totalBytesSent += intervalBytesSent;
		totalMsgReceivedCount += intervalMsgReceivedCount;
		totalBytesReceived += intervalBytesReceived;

		if (writeStats)
		{
			TransportThread *pThread = &sessionHandlerList[i].transportThread;

			printCurrentTimeUTC(pThread->statsFile);
			fprintf(pThread->statsFile, ", %llu, %llu, %llu, %llu, %llu, %.3f, %.3f, %.3f, %.3f, %.2f, %.2f\n", 
					intervalMsgSentCount,
					intervalBytesSent,
					intervalMsgReceivedCount,
					intervalBytesReceived,
					intervalLatencyStats.count,
					intervalLatencyStats.average,
					sqrt(intervalLatencyStats.variance),
					intervalLatencyStats.count ?
					intervalLatencyStats.maxValue : 0.0,
					intervalLatencyStats.count ?
					intervalLatencyStats.minValue : 0.0,
					resourceStats.cpuUsageFraction * 100.0f, (double)resourceStats.memUsageBytes / 1048576.0);

			fflush(pThread->statsFile);
		}

		if (displayStats)
		{
			if (transportPerfConfig.threadCount == 1)
				printf("%03u:\n", currentRuntimeSec);
			else
				printf("%03u: Thread %d:\n", currentRuntimeSec, i + 1);

			printf("  Sent: MsgRate: %8.0f, DataRate:%8.3fMBps\n",
					(double)intervalMsgSentCount / (double)timePassedSec,
					(double)intervalBytesSent / (double)(1024*1024) / (double)timePassedSec);

			printf("  Recv: MsgRate: %8.0f, DataRate:%8.3fMBps\n",
					(double)intervalMsgReceivedCount / (double)timePassedSec,
					(double)intervalBytesReceived / (double)(1024*1024) / (double)timePassedSec);

			if (intervalOutOfBuffersCount > 0)
			{
				printf("  %lld messages not sent due to lack of output buffers.\n", intervalOutOfBuffersCount);
			}

			if (intervalLatencyStats.count > 0)
				printValueStatistics(stdout, "  Latency (usec)", "Msgs", &intervalLatencyStats, RSSL_TRUE);

			if(transportPerfConfig.takeMCastStats)
			{
				RsslUInt64 intervalMcastPacketsSent = 0, intervalMcastPacketsReceived = 0, intervalMcastRetransSent = 0, intervalMcastRetransReceived = 0;
				RsslChannelInfo chnlInfo;
				RsslQueueLink *pLink;
				SessionHandler *pHandler = &sessionHandlerList[i];

				RSSL_QUEUE_FOR_EACH_LINK(&pHandler->transportThread.channelHandler.activeChannelList, pLink)
				{
					RsslError error;
					ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);

					if (pChannelInfo->pChannel->state != RSSL_CH_STATE_ACTIVE)
						continue;

					if (rsslGetChannelInfo(pChannelInfo->pChannel, &chnlInfo, &error) != RSSL_RET_SUCCESS)
					{
						printf ("rsslGetChannelInfo() failed. errorId = %d (%s)\n", error.rsslErrorId, error.text);
						continue;
					}

					intervalMcastPacketsSent = chnlInfo.multicastStats.mcastSent 
						- pHandler->prevMCastStats.mcastSent;

					intervalMcastPacketsReceived = chnlInfo.multicastStats.mcastRcvd 
						- pHandler->prevMCastStats.mcastRcvd;

					intervalMcastRetransSent = chnlInfo.multicastStats.retransPktsSent 
						- pHandler->prevMCastStats.retransPktsSent;

					intervalMcastRetransReceived = chnlInfo.multicastStats.retransPktsRcvd 
						- pHandler->prevMCastStats.retransPktsRcvd;

					pHandler->prevMCastStats = chnlInfo.multicastStats;

				}

				printf("  Multicast: Pkts Sent: %llu, Pkts Received: %llu, : Retrans sent: %llu, Retrans received: %llu\n",
						intervalMcastPacketsSent, intervalMcastPacketsReceived,  intervalMcastRetransSent, intervalMcastRetransReceived);
			}

			printf("  CPU: %6.2f%% Mem: %8.2fMB\n",
					resourceStats.cpuUsageFraction * 100.0,
					(double)resourceStats.memUsageBytes / 1048576.0 );
		}
	}

}


static RsslServer* bindRsslServer(RsslError* error)
{
	RsslServer* srvr;
	RsslBindOptions sopts = RSSL_INIT_BIND_OPTS;
	

	sopts.guaranteedOutputBuffers = transportPerfConfig.guaranteedOutputBuffers;

	if(strlen(transportPerfConfig.interfaceName)) sopts.interfaceName = transportPerfConfig.interfaceName;
	sopts.majorVersion = 0;
	sopts.minorVersion = 0;
	sopts.protocolType = TEST_PROTOCOL_TYPE;
	sopts.tcp_nodelay = transportPerfConfig.tcpNoDelay;
	sopts.connectionType = transportPerfConfig.connectionType;
	sopts.maxFragmentSize = transportPerfConfig.maxFragmentSize;
	sopts.compressionType = transportPerfConfig.compressionType;
	sopts.compressionLevel = transportPerfConfig.compressionLevel;
	sopts.serviceName = transportPerfConfig.portNo;
	sopts.sysSendBufSize = transportPerfConfig.sendBufSize;
	sopts.sysRecvBufSize = transportPerfConfig.recvBufSize;
	if (transportPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED) 
	{
		sopts.encryptionOpts.serverCert = transportPerfConfig.serverCert;
		sopts.encryptionOpts.serverPrivateKey = transportPerfConfig.serverKey;
	}
	if ((srvr = rsslBind(&sopts, error)) == 0)
		return NULL;

	printf("\nServer "SOCKET_PRINT_TYPE" bound on port %d\n", srvr->socketId, srvr->portNumber);
	FD_SET(srvr->socketId,&readfds);
	FD_SET(srvr->socketId,&exceptfds);
	return srvr;
}

/* Gives a channel to a provider thread that has the fewest open channels. */
static RsslRet sendToLeastLoadedThread(RsslChannel *chnl)
{

	SessionHandler *pConnHandler;
	RsslInt32 i, minProviderConnCount, connHandlerIndex;
	NewChannel *pNewChannel;


	minProviderConnCount = 0x7fffffff;
	for(i = 0; i < sessionHandlerCount; ++i)
	{
		SessionHandler *pTmpHandler = &sessionHandlerList[i];
		RSSL_MUTEX_LOCK(&pTmpHandler->handlerLock);
		if (pTmpHandler->openChannelsCount < minProviderConnCount)
		{
			minProviderConnCount = pTmpHandler->openChannelsCount;
			pConnHandler = pTmpHandler;
			connHandlerIndex = i;
		}
		RSSL_MUTEX_UNLOCK(&pTmpHandler->handlerLock);
	}


	pNewChannel = (NewChannel*)malloc(sizeof(NewChannel)); assert(pNewChannel);
	pNewChannel->pChannel = chnl;
	RSSL_MUTEX_LOCK(&pConnHandler->handlerLock);
	++pConnHandler->openChannelsCount;
	rsslQueueAddLinkToBack(&pConnHandler->newChannelsList, &pNewChannel->queueLink);
	RSSL_MUTEX_UNLOCK(&pConnHandler->handlerLock);

	if (rsslSrvr)
		printf("Server "SOCKET_PRINT_TYPE": ", rsslSrvr->socketId);
	printf("New Channel "SOCKET_PRINT_TYPE" passed connection to handler %d\n\n", chnl->socketId, connHandlerIndex);

	return RSSL_RET_SUCCESS;
}

static RsslChannel* startConnection()
{
	RsslConnectOptions copts;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	RsslChannel* chnl;
	RsslError error;
	
	rsslClearConnectOpts(&copts);
	
	if (transportPerfConfig.sAddr || transportPerfConfig.rAddr)
	{
		printf("\nAttempting segmented connect to server (send %s:%s,  recv %s:%s) unicastPort %s...\n", 
			transportPerfConfig.sendAddr, transportPerfConfig.sendPort, transportPerfConfig.recvAddr, transportPerfConfig.recvPort, transportPerfConfig.unicastPort);
		copts.connectionInfo.segmented.recvAddress = transportPerfConfig.recvAddr;
		copts.connectionInfo.segmented.recvServiceName = transportPerfConfig.recvPort;
		copts.connectionInfo.segmented.sendAddress = transportPerfConfig.sendAddr;
		copts.connectionInfo.segmented.sendServiceName = transportPerfConfig.sendPort;
		copts.connectionInfo.segmented.interfaceName = transportPerfConfig.interfaceName;
		copts.connectionInfo.unified.unicastServiceName = transportPerfConfig.unicastPort;		
	}
	else
	{
		printf("\nAttempting to connect to server %s:%s...\n", transportPerfConfig.hostName, transportPerfConfig.portNo);
		copts.connectionInfo.unified.address = transportPerfConfig.hostName;
		copts.connectionInfo.unified.serviceName = transportPerfConfig.portNo;
		copts.connectionInfo.unified.interfaceName = transportPerfConfig.interfaceName;
	}

	copts.guaranteedOutputBuffers = transportPerfConfig.guaranteedOutputBuffers;
	copts.sysSendBufSize = transportPerfConfig.sendBufSize;
	copts.sysRecvBufSize = transportPerfConfig.recvBufSize;
	copts.majorVersion = 0;
	copts.minorVersion = 0;
	copts.protocolType = TEST_PROTOCOL_TYPE;
	copts.connectionType = transportPerfConfig.connectionType;
	copts.tcp_nodelay = transportPerfConfig.tcpNoDelay;
	copts.compressionType = transportPerfConfig.compressionType;
	if (transportPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		copts.encryptionOpts.encryptedProtocol = transportPerfConfig.encryptedConnectionType;
		copts.encryptionOpts.openSSLCAStore = transportPerfConfig.caStore;
	}

	if(copts.connectionType == RSSL_CONN_TYPE_SEQ_MCAST)
	{
		copts.seqMulticastOpts.maxMsgSize = transportPerfConfig.maxFragmentSize;
	}

	if ( (chnl = rsslConnect(&copts,&error)) == 0)
	{
		printf("rsslConnect() failed: %d(%s)\n", error.rsslErrorId, error.text);
		return NULL;
	}

	return chnl;
}

static void initRuntime()
{
	TimeValue currentTime = 0;

	/* get current time */
	currentTime = getTimeNano();
	
	runTime = currentTime + ((RsslInt64)transportPerfConfig.runTime * 1000000000LL);
}

static void handleRuntime(RsslInt64 currentTime)
{
	RsslRet	ret = 0;
	RsslInt32 i;

	/* get current time */

	if (currentTime >= runTime)
	{
		printf("\nRun time of %u seconds has expired.\n\n", transportPerfConfig.runTime);
		signal_shutdown = RSSL_TRUE; /* Tell other threads to shutdown. */
	}

	if (signal_shutdown == RSSL_TRUE)
	{
		for (i = 0; i < sessionHandlerCount; ++i)
		{
			if ((ret = RSSL_THREAD_JOIN(sessionHandlerList[i].threadId)) < 0)
				printf("Failed to join thread: %d\n", ret);
		}

		cleanUpAndExit();
	}

}

void cleanUpAndExit()
{
	RsslError error;

	printf("Shutting down.\n\n");

	/* if we did a bind, clean it up */
	if (rsslSrvr)
	{
		FD_CLR(rsslSrvr->socketId, &readfds);
		FD_CLR(rsslSrvr->socketId, &exceptfds);
		rsslCloseServer(rsslSrvr, &error);
	}

	rsslUninitialize();

	collectStats(RSSL_FALSE, RSSL_FALSE, 0, 0);
	printSummaryStats(stdout);
	printSummaryStats(summaryFile);

	fclose(summaryFile);

	if(sessionHandlerList)
	{
		RsslInt32 i;
		for(i = 0; i < sessionHandlerCount; ++i)
		{
			sessionHandlerCleanup(&sessionHandlerList[i]);
		}
		free(sessionHandlerList);
	}

	cleanupTransportThreadConfig();

	exit(0);
}

static void printSummaryStats(FILE *file)
{
	RsslInt32 i;
	TimeValue earliestConnectTime, latestDisconnectTime;
	double connectedTime;

	earliestConnectTime = 0;
	latestDisconnectTime = 0;
	for(i = 0; i < transportPerfConfig.threadCount; ++i)
	{
		if (!earliestConnectTime || sessionHandlerList[i].transportThread.connectTime
				< earliestConnectTime)
			earliestConnectTime = sessionHandlerList[i].transportThread.connectTime;

		if (!latestDisconnectTime || sessionHandlerList[i].transportThread.disconnectTime
				> latestDisconnectTime)
			latestDisconnectTime = sessionHandlerList[i].transportThread.disconnectTime;
	}

	connectedTime = ((double)latestDisconnectTime - (double)earliestConnectTime)
		/ 1000000000.0;

	if (transportPerfConfig.threadCount > 1)
	{
		for(i = 0; i < transportPerfConfig.threadCount; ++i)
		{
			TransportThread *pThread = &sessionHandlerList[i].transportThread;

			double threadConnectedTime = ((double)sessionHandlerList[i].transportThread.disconnectTime 
					- (double)sessionHandlerList[i].transportThread.connectTime) / 1000000000.0;

			fprintf( file, "--- THREAD %d SUMMARY ---\n\n", i + 1);

			fprintf(file, "Statistics: \n");

			if (pThread->latencyStats.count)
			{
				fprintf( file,
						"  Latency avg (usec): %.3f\n"
						"  Latency std dev (usec): %.3f\n"
						"  Latency max (usec): %.3f\n"
						"  Latency min (usec): %.3f\n",
						pThread->latencyStats.average,
						sqrt(pThread->latencyStats.variance),
						pThread->latencyStats.maxValue,
						pThread->latencyStats.minValue);
			}
			else
				fprintf( file, "  No latency information was received.\n\n");

			fprintf( file,
					"  Sampling duration(sec): %.2f\n"
					"  Msgs Sent: %llu\n"
					"  Msgs Received: %llu\n"
					"  Data Sent (MB): %.2f\n"
					"  Data Received (MB): %.2f\n"
					"  Avg. Msg Sent Rate: %.0f\n"
					"  Avg. Msg Recv Rate: %.0f\n"
					"  Avg. Data Sent Rate (MB): %.2f\n"
					"  Avg. Data Recv Rate (MB): %.2f\n\n",
					threadConnectedTime,
					countStatGetTotal(&pThread->msgsSent),
					countStatGetTotal(&pThread->msgsReceived),
					(double)countStatGetTotal(&pThread->bytesSent) / 1048576.0,
					(double)countStatGetTotal(&pThread->bytesReceived)/ 1048576.0,
					threadConnectedTime ? (double)countStatGetTotal(&pThread->msgsSent)/ threadConnectedTime : 0,
					threadConnectedTime ? (double)countStatGetTotal(&pThread->msgsReceived)/ threadConnectedTime : 0,
					threadConnectedTime ? (double)countStatGetTotal(&pThread->bytesSent) / 1048576.0 / threadConnectedTime : 0,
					threadConnectedTime ? (double)countStatGetTotal(&pThread->bytesReceived) / 1048576.0 / threadConnectedTime : 0);
		}

	}


	fprintf( file, "--- OVERALL SUMMARY ---\n\n");

	fprintf(file, "Statistics: \n");

	if (totalLatencyStats.count)
	{
		fprintf( file,
				"  Latency avg (usec): %.3f\n"
				"  Latency std dev (usec): %.3f\n"
				"  Latency max (usec): %.3f\n"
				"  Latency min (usec): %.3f\n",
				totalLatencyStats.average,
				sqrt(totalLatencyStats.variance),
				totalLatencyStats.maxValue,
				totalLatencyStats.minValue);
	}
	else
		fprintf( file, "  No latency information was received.\n\n");

	fprintf( file,
			"  Sampling duration(sec): %.2f\n"
			"  Msgs Sent: %llu\n"
			"  Msgs Received: %llu\n"
			"  Data Sent (MB): %.2f\n"
			"  Data Received (MB): %.2f\n"
			"  Avg. Msg Sent Rate: %.0f\n"
			"  Avg. Msg Recv Rate: %.0f\n"
			"  Avg. Data Sent Rate (MB): %.2f\n"
			"  Avg. Data Recv Rate (MB): %.2f\n",
			connectedTime,
			totalMsgSentCount,
			totalMsgReceivedCount,
			(double)totalBytesSent / 1048576.0,
			(double)totalBytesReceived / 1048576.0,
			connectedTime ? (double)totalMsgSentCount / connectedTime : 0,
			connectedTime ? (double)totalMsgReceivedCount / connectedTime : 0,
			connectedTime ? (double)totalBytesSent / 1048576.0 / connectedTime : 0,
			connectedTime ? (double)totalBytesReceived / 1048576.0 / connectedTime : 0);

	if (cpuUsageStats.count)
	{
		assert(memUsageStats.count);
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
	else
		printf("No CPU/Mem statistics taken.\n\n");

	fprintf(file, "  Process ID: %d\n", getpid());
}
