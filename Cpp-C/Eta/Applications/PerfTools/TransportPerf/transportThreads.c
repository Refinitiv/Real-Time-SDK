/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "transportThreads.h"
#include "testUtils.h"

#include <stdlib.h>
#include <assert.h>
#ifdef WIN32
#define getpid _getpid
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#define LATENCY_RANDOM_ARRAY_SET_COUNT 20
static LatencyRandomArray latencyRandomArray;

static RsslUInt32 outBytesTotal = 0;
static RsslUInt32 uncompOutBytesTotal = 0;
static RsslReadOutArgs readOutArgs = RSSL_INIT_READ_OUT_ARGS;

/* Contains the global TransportThread configuration. */
TransportThreadConfig transportThreadConfig;

void clearTransportThreadConfig()
{
	memset(&transportThreadConfig, 0, sizeof(TransportThreadConfig));
	transportThreadConfig.ticksPerSec = 1000;
	transportThreadConfig.totalBuffersPerPack = 1;
	transportThreadConfig.msgsPerSec = 100000;
	transportThreadConfig.msgSize = 76;
	transportThreadConfig.latencyMsgsPerSec = 10;
	transportThreadConfig.writeFlags = 0;
	transportThreadConfig.checkPings = RSSL_TRUE;
	transportThreadConfig.logLatencyToFile = RSSL_FALSE;
	snprintf(transportThreadConfig.statsFilename, sizeof(transportThreadConfig.statsFilename), "TransportStats");
}

void initTransportThreadConfig()
{
	/* Configuration checks */
	if (transportThreadConfig.latencyMsgsPerSec > transportThreadConfig.msgsPerSec)
	{
		printf("Config Error: Latency msg rate cannot be greater than total msg rate. \n\n");
		exit(-1);
	}

	if (transportThreadConfig.latencyMsgsPerSec > transportThreadConfig.ticksPerSec)
	{
		printf("Config Error: Latency msg rate cannot be greater than total ticks per second. \n\n");
		exit(-1);
	}

	if (transportThreadConfig.msgsPerSec != 0 && transportThreadConfig.msgsPerSec < transportThreadConfig.ticksPerSec)
	{
		printf("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
		exit(-1);
	}

	/* Must have room for sending both the timestamp and sequence number. */
	if (transportThreadConfig.msgSize < sizeof(TimeValue)+sizeof(RsslUInt64) )
	{
		printf("Config Error: Message size must be at least %u.\n\n", (int)sizeof(RsslUInt64));
		exit(-1);
	}

	if (transportThreadConfig.totalBuffersPerPack < 1)
	{
		printf("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
		exit(-1);
	}

	/* Determine msg rates on per-tick basis */
	transportThreadConfig._msgsPerTick = transportThreadConfig.msgsPerSec / transportThreadConfig.ticksPerSec;
	transportThreadConfig._msgsPerTickRemainder = transportThreadConfig.msgsPerSec % transportThreadConfig.ticksPerSec;

	if (transportThreadConfig.latencyMsgsPerSec > 0)
	{
		LatencyRandomArrayOptions randArrayOpts;
		randArrayOpts.totalMsgsPerSec = transportThreadConfig.msgsPerSec;
		randArrayOpts.latencyMsgsPerSec = transportThreadConfig.latencyMsgsPerSec;
		randArrayOpts.ticksPerSec = transportThreadConfig.ticksPerSec;
		randArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;
		createLatencyRandomArray(&latencyRandomArray, &randArrayOpts);
	}
}

void cleanupTransportThreadConfig()
{
	cleanupLatencyRandomArray(&latencyRandomArray);
}

void transportThreadInit(TransportThread *pThread,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg,
		RsslInt32 threadIndex)
{
	char tmpFilename[sizeof(transportThreadConfig.statsFilename) + 8];

	initCountStat(&pThread->msgsSent);
	initCountStat(&pThread->bytesSent);
	initCountStat(&pThread->msgsReceived);
	initCountStat(&pThread->bytesReceived);
	initCountStat(&pThread->outOfBuffersCount);
	clearValueStatistics(&pThread->latencyStats);

	pThread->connectTime = 0;
	pThread->disconnectTime = 0;
	pThread->threadIndex = threadIndex;

	snprintf(tmpFilename, sizeof(tmpFilename), "%s%d_%d.csv", 
			transportThreadConfig.statsFilename, threadIndex + 1, getpid());

	if (!(pThread->statsFile = fopen(tmpFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", tmpFilename);
		exit(-1);
	}

	fprintf(pThread->statsFile, "UTC, Msgs sent, Bytes sent, Msgs received, Bytes received, Latency msgs received, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), CPU usage (%%), Memory (MB)\n");

	if (transportThreadConfig.logLatencyToFile)
	{
		snprintf(tmpFilename, sizeof(tmpFilename), "%s%d_%d.csv", 
				transportThreadConfig.latencyLogFilename, threadIndex + 1, getpid());

		/* Open latency log file. */
		pThread->latencyLogFile = fopen(tmpFilename, "w");
		if (!pThread->latencyLogFile)
		{
			printf("Failed to open latency log file: %s\n", tmpFilename);
			exit(-1);
		}

		fprintf(pThread->latencyLogFile, "Send Time, Receive Time, Latency(nsec)\n");
	}

	pThread->currentTicks = 0;
	initChannelHandler(&pThread->channelHandler, processActiveChannel, processInactiveChannel, processMsg, (void*)pThread);
	latencyRandomArrayIterInit(&pThread->randArrayIter);
}

void transportThreadCleanup(TransportThread *pThread)
{
	channelHandlerCleanup(&pThread->channelHandler);
	fclose(pThread->statsFile);
	if(transportThreadConfig.logLatencyToFile)
		fclose(pThread->latencyLogFile);
}

TransportSession *transportSessionCreate(TransportThread *pHandler, RsslChannel *chnl)
{
	RsslUInt32 maxFragmentSizePad = 0;
	TransportSession *pSession;
	
	pSession = (TransportSession*)malloc(sizeof(TransportSession));
	memset(pSession, 0, sizeof(TransportSession));
	pSession->pWritingBuffer = NULL;
	pSession->packedBufferCount = 0;
	pSession->sendSequenceNumber = 0;
	pSession->recvSequenceNumber = 0;
	pSession->receivedFirstSequenceNumber = RSSL_FALSE;

	pSession->maxMsgBufSize = transportThreadConfig.totalBuffersPerPack * transportThreadConfig.msgSize;

	/* If the buffer is to be packed, add some additional bytes for each message. */
	if (transportThreadConfig.totalBuffersPerPack > 1)
		pSession->maxMsgBufSize += transportThreadConfig.totalBuffersPerPack * 8;


	pSession->sendSequenceNumber = 0;
	pSession->recvSequenceNumber = 0;
	pSession->receivedFirstSequenceNumber = RSSL_FALSE;

	pSession->pChannelInfo = channelHandlerAddChannel(&pHandler->channelHandler, chnl, (void*)pSession, transportThreadConfig.checkPings);

	return pSession;
}

void transportSessionDestroy(TransportThread *pHandler, TransportSession *pSession)
{
	free(pSession);
}

static RsslRet transportSessionSendMsg(TransportThread *pHandler, TransportSession *pSession, RsslInt32 msgsLeft, RsslBool sendLatency)
{
	RsslRet ret;
	RsslChannel *chnl = pSession->pChannelInfo->pChannel;
	RsslUInt64 currentTime;

	/* Add latency timestamp, if appropriate. */
	if (sendLatency)
		currentTime = getTimeNano();
	else
		currentTime = 0;

	if (rtrUnlikely((ret = getMsgBuffer(pSession)) < RSSL_RET_SUCCESS))
		return ret;

	assert(sizeof(currentTime) == 8 && sizeof(pSession->sendSequenceNumber == 8));

	if (pSession->pWritingBuffer->length < transportThreadConfig.msgSize)
	{
		printf("Error: transportSessionSendMsg(): Buffer length %u is too small to write next message.\n", pSession->pWritingBuffer->length);
		exit(-1);
	}

	/* Zero out message. */
	memset(pSession->pWritingBuffer->data, 0x0, transportThreadConfig.msgSize);

	/* Add seqnuence number */
	memcpy(pSession->pWritingBuffer->data, &pSession->sendSequenceNumber, 8);

	memcpy(pSession->pWritingBuffer->data + 8, &currentTime, 8);

	pSession->pWritingBuffer->length = transportThreadConfig.msgSize;

	++pSession->sendSequenceNumber;

	return writeMsgBuffer(pHandler, pSession, msgsLeft > 1);
}

RsslRet transportSessionSendMsgBurst(TransportThread *pHandler, TransportSession *pSession, RsslInt32 *pMsgsSent)
{
	RsslInt32 msgsLeft;
	RsslInt32 latencyUpdateNumber;
	RsslRet ret = RSSL_RET_SUCCESS;

	/* Determine msgs to send out. Spread the remainder out over the first ticks */
	msgsLeft = transportThreadConfig._msgsPerTick;
	if (transportThreadConfig._msgsPerTickRemainder > pHandler->currentTicks)
		++msgsLeft;

	*pMsgsSent += msgsLeft;

	latencyUpdateNumber = (transportThreadConfig.latencyMsgsPerSec > 0) ?
		latencyRandomArrayGetNext(&latencyRandomArray, &pHandler->randArrayIter) : -1; 

	for(; msgsLeft > 0; --msgsLeft)
	{
		/* Send the item. */
		ret = transportSessionSendMsg(pHandler, pSession, msgsLeft, 
				(msgsLeft - 1) == latencyUpdateNumber || transportThreadConfig.latencyMsgsPerSec == ALWAYS_SEND_LATENCY_MSG);

		if (rtrUnlikely(ret < RSSL_RET_SUCCESS) )
		{
			if (ret == RSSL_RET_BUFFER_NO_BUFFERS)
				countStatAdd(&pHandler->outOfBuffersCount, msgsLeft);
			*pMsgsSent -= msgsLeft;
			return ret;
		}
	}

	if (rtrUnlikely(++pHandler->currentTicks == transportThreadConfig.ticksPerSec))
		pHandler->currentTicks = 0;

	return ret;
}

RsslRet getMsgBuffer(TransportSession *pSession)
{
	RsslChannel *chnl = pSession->pChannelInfo->pChannel;

	if (pSession->pWritingBuffer) 
		return RSSL_RET_SUCCESS;
	else
	{
		RsslError error;
		pSession->pWritingBuffer = rsslGetBuffer(chnl, pSession->maxMsgBufSize, (transportThreadConfig.totalBuffersPerPack > 1)? RSSL_TRUE : RSSL_FALSE , &error);
		if(pSession->pWritingBuffer)
		{
			assert(pSession->pWritingBuffer->length == pSession->maxMsgBufSize);
			return RSSL_RET_SUCCESS;
		}

		if (error.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
			printf("rsslGetBuffer() failed: %s(%s)\n", 
					rsslRetCodeToString(error.rsslErrorId), 
					error.text);
		return error.rsslErrorId;
	}

}

RsslRet writeMsgBuffer(TransportThread *pHandler, TransportSession *pSession, RsslBool allowPack)
{
	RsslError error;
	RsslChannel *chnl = pSession->pChannelInfo->pChannel;

	/* Make sure we stop packing at the end of a burst of msgs
	 *   in case the next burst is for a different channel. 
	 *   (This will also prevent any latency msgs from sitting in the pack for a tick). */
	if (pSession->packedBufferCount == (transportThreadConfig.totalBuffersPerPack - 1)|| !allowPack)
	{
		RsslUInt32 outBytes;
		RsslUInt32 uncompOutBytes;
		RsslRet ret;

		/* Send the completed buffer(or if there is no packing being done, send as normal) */
		pSession->packedBufferCount = 0;

		ret = rsslWrite(chnl, pSession->pWritingBuffer, RSSL_HIGH_PRIORITY, transportThreadConfig.writeFlags, &outBytes, &uncompOutBytes, &error);
		/* call flush and write again */
		while (rtrUnlikely(ret == RSSL_RET_WRITE_CALL_AGAIN))
		{
			if (rtrUnlikely((ret = rsslFlush(chnl, &error)) < RSSL_RET_SUCCESS))
			{
				printf("rsslFlush() failed with return code %d - <%s>\n", ret, error.text);
				return ret;
			}
			ret = rsslWrite(chnl, pSession->pWritingBuffer, RSSL_HIGH_PRIORITY, transportThreadConfig.writeFlags, &outBytes, &uncompOutBytes, &error);
		}

		if (ret >= RSSL_RET_SUCCESS)
		{
			pSession->pWritingBuffer = 0;
			countStatAdd(&pHandler->bytesSent, outBytes);
			countStatIncr(&pHandler->msgsSent);
			return ret;
		}

		switch(ret)
		{
			case RSSL_RET_WRITE_FLUSH_FAILED:
				if (chnl->state == RSSL_CH_STATE_ACTIVE)
				{
					pSession->pWritingBuffer = 0;
					countStatAdd(&pHandler->bytesSent, outBytes);
					countStatIncr(&pHandler->msgsSent);
					return 1;
				}
				/* Otherwise treat as error, fall through to default. */
			default:
				printf("rsslWrite() failed: %s(%s)\n", rsslRetCodeToString(error.rsslErrorId), 
						error.text);
				return ret;
		}
	}
	else
	{
		/* Pack the buffer and continue using it. */
		++pSession->packedBufferCount;
		pSession->pWritingBuffer = rsslPackBuffer(chnl, pSession->pWritingBuffer, &error);
		if (!pSession->pWritingBuffer)
		{
			printf("rsslPackBuffer failed: %d <%s>", error.rsslErrorId, error.text);
			return RSSL_RET_FAILURE;
		}
		countStatIncr(&pHandler->msgsSent);
		return RSSL_RET_SUCCESS;
	}
}
