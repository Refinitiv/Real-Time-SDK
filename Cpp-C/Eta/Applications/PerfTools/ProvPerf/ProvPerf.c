/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020, 2025 LSEG. All rights reserved.
*/

#include "ProvPerf.h"
#include "provPerfConfig.h"
#include "statistics.h"
#include "dictionaryProvider.h"
#include "rtr/rsslGetTime.h"
#include "perfTunnelMsgHandler.h"
#include "testUtils.h"
#include "rtr/rsslReactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#ifndef NO_ETA_CPU_BIND
#include "rtr/rsslBindThread.h"
#endif
//uncomment the following line for debugging only - this will greatly affect performance
//#define ENABLE_XML_TRACE

#ifdef __cplusplus
extern "C" {
	static void signal_handler(int sig);
}
#endif

static Provider provider;

static RsslBool signal_shutdown = RSSL_FALSE;
static fd_set	readfds;
static fd_set	exceptfds;
static RsslTimeValue rsslProviderRuntime = 0;

RsslTimeValue _currentTime;

static RsslServer *rsslSrvr = NULL;

static RsslInt64 nsecPerTick;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

static ValueStatistics intervalMsgEncodingStats;
RsslBool showTransportDetails = RSSL_FALSE;

static void signal_handler(int sig)
{
	signal_shutdown = RSSL_TRUE;
}

extern void startProviderThreads(Provider *pProvider, RSSL_THREAD_DECLARE(threadFunction,pArg));

RsslRet serviceNameToIdCallback(RsslBuffer* pServiceName, void* userSecPtr, RsslUInt16* pServiceId)
{
	ProviderThread *pProvThread = (ProviderThread*)userSecPtr;

	if (strncmp(&directoryConfig.serviceName[0], pServiceName->data, pServiceName->length) == 0)
	{
		*pServiceId = (RsslUInt16)directoryConfig.serviceId;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

RsslRet serviceNameToIdReactorCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	ProviderThread *pProvThread = (ProviderThread*)pEvent->pUserSpec;

	if (strncmp(&directoryConfig.serviceName[0], pServiceName->data, pServiceName->length) == 0)
	{
		*pServiceId = (RsslUInt16)directoryConfig.serviceId;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}


RSSL_THREAD_DECLARE(runChannelConnectionHandler, pArg)
{

	ProviderThread *pProvThread = (ProviderThread*)pArg;
	RsslErrorInfo rsslErrorInfo;

	RsslTimeValue nextTickTime;
	RsslInt32 currentTicks = 0;

#ifndef NO_ETA_CPU_BIND
	if (pProvThread->cpuId.length > 0 && pProvThread->cpuId.data != NULL)
	{
		if (rsslBindThread(pProvThread->cpuId.data, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %s: %s.\n", pProvThread->cpuId.data, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}
#endif

	nextTickTime = rsslGetTimeNano() + nsecPerTick;

	pProvThread->rjcSess.options.pDictionary = pProvThread->pDictionary;
	pProvThread->rjcSess.options.defaultServiceId = (RsslUInt16)directoryConfig.serviceId;
	pProvThread->rjcSess.options.userSpecPtr = (void*)pProvThread;
	pProvThread->rjcSess.options.pServiceNameToIdCallback = serviceNameToIdCallback;

	if (rjcSessionInitialize(&(pProvThread->rjcSess), &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("RWF/JSON Converter failed: %s\n", rsslErrorInfo.rsslError.text);
		cleanUpAndExit();
	}

	/* this is the main loop */
	while(rtrLikely(!signal_shutdown))
	{
		for (currentTicks = 0; currentTicks < providerThreadConfig.ticksPerSec; ++currentTicks)
		{
			providerThreadRead(pProvThread, nextTickTime);

			nextTickTime += nsecPerTick;

			providerThreadSendMsgBurst(pProvThread, nextTickTime);

			providerThreadReceiveNewChannels(pProvThread);

		}

		providerThreadCheckPings(pProvThread);

	}

	return RSSL_THREAD_RETURN();
}

RSSL_THREAD_DECLARE(runReactorConnectionHandler, pArg)
{
	ProviderThread *pProvThread = (ProviderThread*)pArg;

	RsslTimeValue nextTickTime;
	RsslInt32 currentTicks = 0;
	RsslTimeValue currentTime;
	RsslRet ret;
	int selRet;
	struct timeval time_interval;
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorDispatchOptions dispatchOptions;
	RsslCreateReactorOptions reactorOpts;
	RsslReactorJsonConverterOptions jsonConverterOptions;

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);
	rsslClearReactorDispatchOptions(&dispatchOptions);

	// create reactor
	rsslClearCreateReactorOptions(&reactorOpts);

	/* The Cpu core id for the internal Reactor worker thread. */
	reactorOpts.cpuBindWorkerThread = pProvThread->cpuReactorWorkerId;

	if (!(pProvThread->pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Reactor creation failed: %s\n", rsslErrorInfo.rsslError.text);
		cleanUpAndExit();
	}

#ifndef NO_ETA_CPU_BIND
	// Cpu core bind for the provider thread thread.
	// The application should invoke rsslBindThread() after rsslInitialize() has invoked.
	// rsslInitialize analyzed Cpu Topology.
	if (pProvThread->cpuId.length > 0 && pProvThread->cpuId.data != NULL)
	{
		if (rsslBindThread(pProvThread->cpuId.data, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %s: %s\n", pProvThread->cpuId.data, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}
#endif

	jsonConverterOptions.pDictionary = pProvThread->pDictionary;
	jsonConverterOptions.defaultServiceId = (RsslUInt16)directoryConfig.serviceId;
	jsonConverterOptions.userSpecPtr = (void*)pProvThread;
	jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdReactorCallback;
	if (rsslReactorInitJsonConverter(pProvThread->pReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error initializing RWF/JSON converter: %s\n", rsslErrorInfo.rsslError.text);
		cleanUpAndExit();
	}

	FD_ZERO(&pProvThread->readfds);
	FD_ZERO(&pProvThread->wrtfds);
	FD_ZERO(&pProvThread->exceptfds);

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pProvThread->pReactor->eventFd, &pProvThread->readfds);

	nextTickTime = rsslGetTimeNano() + nsecPerTick;

	/* this is the main loop */
	while(rtrLikely(!signal_shutdown))
	{
		/* Loop on select(), looking for channels with available data, until stopTimeNsec is reached. */
		do
		{
#ifdef WIN32
			/* Windows does not allow select() to be called with empty file descriptor sets. */
			if (pProvThread->readfds.fd_count == 0)
			{
				currentTime = rsslGetTimeNano();
				selRet = 0;
				Sleep((DWORD)((currentTime < nextTickTime) ? (nextTickTime - currentTime)/1000000 : 0));
			}
			else
#endif
			{
				useRead = pProvThread->readfds;
				useWrt = pProvThread->wrtfds;
				useExcept = pProvThread->exceptfds;

				currentTime = rsslGetTimeNano();
				time_interval.tv_usec = (long)((currentTime < nextTickTime) ? (nextTickTime - currentTime)/1000 : 0);
				time_interval.tv_sec = 0;

				selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
			}

			if (selRet == 0)
			{
				break;
			}
			else if (selRet > 0)
			{	
				while ((ret = rsslReactorDispatch(pProvThread->pReactor, &dispatchOptions, &rsslErrorInfo)) > RSSL_RET_SUCCESS) {}
				if (ret < RSSL_RET_SUCCESS)
				{
					printf("rsslReactorDispatch failed with return code: %d error = %s\n", ret,  rsslErrorInfo.rsslError.text);
					exit(-1);
				}
			}
#ifdef WIN32
			else if (WSAGetLastError() != WSAEINTR)
#else 
			else if (errno != EINTR)
#endif
			{
				perror("select");
				exit(-1);
			}
		} while (currentTime < nextTickTime);

		nextTickTime += nsecPerTick;

		providerThreadSendMsgBurst(pProvThread, nextTickTime);
	}

	return RSSL_THREAD_RETURN();
}

RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pChannelEvent)
{
	ProviderSession *pProvSession = (ProviderSession *)pReactorChannel->userSpecPtr;
	ProviderThread *pProviderThread = pProvSession->pProviderThread;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorChannelInfo reactorChannelInfo;
	RsslUInt32 count;
	RsslRet ret;

	switch(pChannelEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorAccept() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. */
			FD_SET(pReactorChannel->socketId, &pProviderThread->readfds);
			FD_SET(pReactorChannel->socketId, &pProviderThread->exceptfds);

#ifdef ENABLE_XML_TRACE
			{
				RsslError error;
				RsslTraceOptions traceOptions;
				rsslClearTraceOptions(&traceOptions);
				traceOptions.traceMsgFileName = "ProvPerf";
				traceOptions.traceMsgMaxFileSize = 1000000000;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
				rsslIoctl(pReactorChannel->pRsslChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &error);
			}
#endif

			if (provPerfConfig.highWaterMark > 0)
			{
				if (rsslReactorChannelIoctl(pReactorChannel, RSSL_HIGH_WATER_MARK, &provPerfConfig.highWaterMark, &rsslErrorInfo) != RSSL_RET_SUCCESS)
                {
					printf("rsslReactorChannelIoctl() of RSSL_HIGH_WATER_MARK failed <%s>\n", rsslErrorInfo.rsslError.text);
					exit(-1);
                }
			}

			if ((ret = rsslReactorGetChannelInfo(pReactorChannel, &reactorChannelInfo, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorGetChannelInfo() failed: %d\n", ret);
				return RSSL_RC_CRET_SUCCESS;
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
					pReactorChannel->socketId,
					reactorChannelInfo.rsslChannelInfo.maxFragmentSize,
					reactorChannelInfo.rsslChannelInfo.maxOutputBuffers, reactorChannelInfo.rsslChannelInfo.guaranteedOutputBuffers,
					reactorChannelInfo.rsslChannelInfo.numInputBuffers,
					reactorChannelInfo.rsslChannelInfo.pingTimeout,
					reactorChannelInfo.rsslChannelInfo.clientToServerPings == RSSL_TRUE ? "true" : "false",
					reactorChannelInfo.rsslChannelInfo.serverToClientPings == RSSL_TRUE ? "true" : "false",
					reactorChannelInfo.rsslChannelInfo.sysSendBufSize, reactorChannelInfo.rsslChannelInfo.sysRecvBufSize,			
					reactorChannelInfo.rsslChannelInfo.compressionType == RSSL_COMP_ZLIB ? "zlib" : "none",
					reactorChannelInfo.rsslChannelInfo.compressionThreshold			
					);

			if (reactorChannelInfo.rsslChannelInfo.componentInfoCount == 0)
				printf("(No component info)");
			else
				for(count = 0; count < reactorChannelInfo.rsslChannelInfo.componentInfoCount; ++count)
				{
					printf("%.*s", 
							reactorChannelInfo.rsslChannelInfo.componentInfo[count]->componentVersion.length,
							reactorChannelInfo.rsslChannelInfo.componentInfo[count]->componentVersion.data);
					if (count < reactorChannelInfo.rsslChannelInfo.componentInfoCount - 1)
						printf(", ");
				}

			switch (reactorChannelInfo.rsslChannelInfo.encryptionProtocol)
			{
			case RSSL_ENC_TLSV1_2:
				printf("\n  Encryption protocol: TLSv1.2");
				break;
			case RSSL_ENC_TLSV1_3:
				printf("\n  Encryption protocol: TLSv1.3");
				break;
			default:
				printf("\n  Encryption protocol: unknown");
			}

			printf ("\n\n");

			/* Check that we can successfully pack, if packing messages. */
			if (providerThreadConfig.totalBuffersPerPack > 1
					&& providerThreadConfig.packingBufferLength > reactorChannelInfo.rsslChannelInfo.maxFragmentSize)
			{
				printf("Error(Channel "SOCKET_PRINT_TYPE"): MaxFragmentSize %u is too small for packing buffer size %u\n",
						pReactorChannel->socketId, reactorChannelInfo.rsslChannelInfo.maxFragmentSize, 
						providerThreadConfig.packingBufferLength);
				exit(-1);
			}


			pProvSession->pChannelInfo->pChannel = pReactorChannel->pRsslChannel;
			pProvSession->pChannelInfo->pReactorChannel = pReactorChannel;
			pProvSession->pChannelInfo->pReactor = pReactor;
			rsslQueueAddLinkToBack(&pProviderThread->channelHandler.activeChannelList, &pProvSession->pChannelInfo->queueLink);
			pProvSession->timeActivated = rsslGetTimeNano();

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			if (ret = (printEstimatedMsgSizes(pProviderThread, pProvSession)) != RSSL_RET_SUCCESS)
			{
				printf("printEstimatedMsgSizes() failed: %d\n", ret);
				return RSSL_RC_CRET_SUCCESS;
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			/* The file descriptor representing the RsslReactorChannel has been changed.
			 * Update our file descriptor sets. */
			printf("Fd change: "SOCKET_PRINT_TYPE" to "SOCKET_PRINT_TYPE"\n", pReactorChannel->oldSocketId, pReactorChannel->socketId);
			FD_CLR(pReactorChannel->oldSocketId, &pProviderThread->readfds);
			FD_CLR(pReactorChannel->oldSocketId, &pProviderThread->exceptfds);
			FD_SET(pReactorChannel->socketId, &pProviderThread->readfds);
			FD_SET(pReactorChannel->socketId, &pProviderThread->exceptfds);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("	Error text: %s\n", pChannelEvent->pError->rsslError.text);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			pProviderThread->stats.inactiveTime = rsslGetTimeNano();

			printf("Channel Closed.\n");

			FD_CLR(pReactorChannel->socketId, &pProviderThread->readfds);
			FD_CLR(pReactorChannel->socketId, &pProviderThread->exceptfds);

			--pProviderThread->clientSessionsCount;
			if (pProvSession->pChannelInfo->pReactorChannel && rsslQueueGetElementCount(&pProviderThread->channelHandler.activeChannelList) > 0)
			{
				rsslQueueRemoveLink(&pProviderThread->channelHandler.activeChannelList, &pProvSession->pChannelInfo->queueLink);
			}
	
			if (pProvSession)
			{
				providerSessionDestroy(pProviderThread, pProvSession);
			}

			if (rsslReactorCloseChannel(pReactor, pReactorChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorCloseChannel() failed: %s\n", rsslErrorInfo.rsslError.text);
				cleanUpAndExit();
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		default:
			printf("Unknown channel event!\n");
			cleanUpAndExit();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent)
{
	processLoginRequestReactor(pReactor, pReactorChannel, pLoginMsgEvent->pRDMLoginMsg);

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent)
{
	processDirectoryRequestReactor(pReactor, pReactorChannel, pDirectoryMsgEvent->pRDMDirectoryMsg);

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsgEvent *pDictionaryMsgEvent)
{
	processDictionaryRequestReactor(pReactor, pReactorChannel, pDictionaryMsgEvent->pRDMDictionaryMsg);

	return RSSL_RC_CRET_SUCCESS;
}

RsslRet processItemMsg(RsslReactorChannel *pReactorChannel, RsslMsg *pMsg)
{
	ProviderSession *pProvSession = (ProviderSession *)pReactorChannel->userSpecPtr;
	ProviderThread *pProvThread = pProvSession->pProviderThread;
	RsslDecodeIterator dIter;

	//printf("processItemMsg Received message in TunnelStream with stream ID %d, class %u(%s) and domainType %u(%s)\n\n",
	//	pMsg->msgBase.streamId,
	//	pMsg->msgBase.msgClass, rsslMsgClassToString(pMsg->msgBase.msgClass),
	//	pMsg->msgBase.domainType, rsslDomainTypeToString(pMsg->msgBase.domainType));

	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);

	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);

	switch (pMsg->msgBase.domainType)
	{
	case RSSL_DMT_MARKET_PRICE:
		if (xmlMsgDataHasMarketPrice)
			processItemRequest(pProvThread, pProvSession, pMsg, &dIter);
		else
			sendItemRequestReject(pProvThread, pProvSession,
				pMsg->msgBase.streamId, pMsg->msgBase.domainType, DOMAIN_NOT_SUPPORTED);
		break;
	case RSSL_DMT_MARKET_BY_ORDER:
		if (xmlMsgDataHasMarketByOrder)
			processItemRequest(pProvThread, pProvSession, pMsg, &dIter);
		else
			sendItemRequestReject(pProvThread, pProvSession,
				pMsg->msgBase.streamId, pMsg->msgBase.domainType, DOMAIN_NOT_SUPPORTED);
		break;
	default:
		sendItemRequestReject(pProvThread, pProvSession,
			pMsg->msgBase.streamId, pMsg->msgBase.domainType, DOMAIN_NOT_SUPPORTED);
		break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent)
{

	processItemMsg(pReactorChannel, pMsgEvent->pRsslMsg);

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent *pEvent, RsslErrorInfo *pErrorInfo)
{
	ProviderSession *pProvSession = (ProviderSession *)pEvent->pReactorChannel->userSpecPtr;
	PerfTunnelMsgHandler *pPerfTunnelMsgHandler = &pProvSession->perfTunnelMsgHandler;

	perfTunnelMsgHandlerProcessNewStream(pPerfTunnelMsgHandler, pEvent);

	if (pPerfTunnelMsgHandler->tunnelStreamHandler.tunnelStreamOpenRequested == RSSL_TRUE)
	{
		perfTunnelMsgHandlerAddCallbackProcessItemMsg(pPerfTunnelMsgHandler, processItemMsg);
	}

	return RSSL_RC_CRET_SUCCESS;
}

static RsslRet acceptReactorConnection(RsslServer *pRsslSrvr, RsslErrorInfo *pRsslErrorInfo)
{
	RsslReactorAcceptOptions aopts;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorOMMProviderRole providerRole;

	ProviderThread *pProvThread = NULL;
	ProviderSession *pProvSession = NULL;
	RsslInt32 i, minProviderConnCount, connHandlerIndex;
	int ret;

	if (!providerThreadConfig.threadCount)
	{
		printf("Provider thread count need to be greater than zero.\n");
		return RSSL_RET_FAILURE;
	}
	// find least loaded reactor thread
	minProviderConnCount = 0x7fffffff;
	for(i = 0; i < providerThreadConfig.threadCount; ++i)
	{
		ProviderThread *pTmpProvThread = &provider.providerThreadList[i];
		RsslInt32 connCount = providerThreadGetConnectionCount(pTmpProvThread);
		if (connCount < minProviderConnCount)
		{
			minProviderConnCount = connCount;
			pProvThread = pTmpProvThread;
			connHandlerIndex = i;
		}
		else
		{
			printf("Provider connection count greater than max value 0x7fffffff.\n");
			return RSSL_RET_FAILURE;
		}
	}

	// create provider session here and link to provider thread
	if (!(pProvSession = providerSessionCreate(pProvThread, NULL)))
	{
		printf("providerSessionInit() failed\n");
		exit(-1);
	}
	pProvSession->pProviderThread = pProvThread;
	++pProvThread->clientSessionsCount;

	// initialize provider role
	rsslClearOMMProviderRole(&providerRole);

	providerRole.base.channelEventCallback = channelEventCallback;
	providerRole.base.defaultMsgCallback = defaultMsgCallback;
	providerRole.loginMsgCallback = loginMsgCallback;
	providerRole.directoryMsgCallback = directoryMsgCallback;
	providerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	providerRole.tunnelStreamListenerCallback = tunnelStreamListenerCallback;

	// waiting until provider thread initializes
	while (pProvThread->pReactor == NULL) {}

	printf("Accepting new Reactor connection...\n");

	rsslClearReactorAcceptOptions(&aopts);
	aopts.rsslAcceptOptions.userSpecPtr = pProvSession;

	if ((ret = rsslReactorAccept(pProvThread->pReactor, pRsslSrvr, &aopts, (RsslReactorChannelRole*)&providerRole, &rsslErrorInfo))
			!= RSSL_RET_SUCCESS)
	{
		providerSessionDestroy(pProvThread, pProvSession);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

int main(int argc, char **argv)
{
	struct timeval time_interval;
	RsslError error;
	fd_set useRead;
	fd_set useExcept;
	int selRet;
	RsslRet	ret = 0;
	RsslBindOptions sopts;
	RsslErrorInfo rsslErrorInfo;

	RsslTimeValue nextTickTime;
	RsslInt32 currentTicks;
	RsslUInt32 currentRuntimeSec = 0, intervalSeconds = 0;

	RsslTimeValue tickSetStartTime; /* Holds when a paritcular run of ticks started(basically one second's worth)*/

	/* Read in configuration and echo it. */
	initProvPerfConfig(argc, argv);
	printProvPerfConfig(stdout);

	if (!(summaryFile = fopen(provPerfConfig.summaryFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", provPerfConfig.summaryFilename);
		exit(-1);
	}

	printProvPerfConfig(summaryFile); fflush(summaryFile);

	// set up a signal handler so we can cleanup before exit
	signal(SIGINT, signal_handler);

	nsecPerTick = 1000000000LL/(RsslInt64)providerThreadConfig.ticksPerSec;

	_currentTime = rsslGetTimeNano();
	nextTickTime = _currentTime;
	currentTicks = 0;

	xmlInitParser();

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);

	/* Initialize RSSL */
	if (provPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
	{
		if (rsslInitialize(providerThreadConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
		{
			printf("RsslInitialize failed: %s\n", error.text);
			exit(-1);
		}
	}
	else // use ETA VA Reactor
	{
		/* The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
		if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &error) != RSSL_RET_SUCCESS)
		{
			printf("RsslInitialize failed: %s\n", error.text);
			exit(-1);
		}
	}

	/* Initialize run-time */
	rsslProviderRuntime = rsslGetTimeNano() + ((RsslInt64)provPerfConfig.runTime * 1000000000LL);

	providerInit(&provider, PROVIDER_INTERACTIVE,
			processActiveChannel,
			processInactiveChannel,
			processMsg, convertMsg);

	if (provPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
	{
		startProviderThreads(&provider, runChannelConnectionHandler);
	}
	else // use ETA VA Reactor
	{
		startProviderThreads(&provider, runReactorConnectionHandler);
	}


	rsslClearBindOpts(&sopts);

	sopts.guaranteedOutputBuffers = provPerfConfig.guaranteedOutputBuffers;
	sopts.maxOutputBuffers = provPerfConfig.maxOutputBuffers;
	sopts.serviceName = provPerfConfig.portNo;
	if(strlen(provPerfConfig.interfaceName)) sopts.interfaceName = provPerfConfig.interfaceName;
	sopts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	sopts.minorVersion = RSSL_RWF_MINOR_VERSION;
	sopts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	sopts.tcp_nodelay = provPerfConfig.tcpNoDelay;
	sopts.sysSendBufSize = provPerfConfig.sendBufSize;
	sopts.sysRecvBufSize = provPerfConfig.recvBufSize;
	sopts.connectionType = RSSL_CONN_TYPE_SOCKET;
	sopts.maxFragmentSize = provPerfConfig.maxFragmentSize;
	sopts.wsOpts.protocols = provPerfConfig.protocolList;
	sopts.compressionType = provPerfConfig.compressionType;
	sopts.compressionLevel = provPerfConfig.compressionLevel;

	sopts.connectionType = provPerfConfig.connType;

	if (provPerfConfig.connType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		sopts.encryptionOpts.serverCert = provPerfConfig.serverCert;
		sopts.encryptionOpts.serverPrivateKey = provPerfConfig.serverKey;
		sopts.encryptionOpts.cipherSuite = provPerfConfig.cipherSuite;
	}

	if ((rsslSrvr = rsslBind(&sopts, &error)) == 0)
	{
		printf("rsslBind() failed: %d(%s)\n", error.rsslErrorId, error.text);
		exit(-1);
	}

	printf("Server "SOCKET_PRINT_TYPE" bound to port %d.\n\n", rsslSrvr->socketId, rsslSrvr->portNumber);
	FD_SET(rsslSrvr->socketId,&readfds);
	FD_SET(rsslSrvr->socketId,&exceptfds);

	time_interval.tv_sec = 0; time_interval.tv_usec = 0;
	nextTickTime = rsslGetTimeNano() + nsecPerTick;
	currentTicks = 0;
	tickSetStartTime = rsslGetTimeNano();

	/* this is the main loop */
	while(1)
	{
		useRead = readfds;
		useExcept = exceptfds;

		/* select() on remaining time for this tick. If we went into the next tick, don't delay at all. */
		_currentTime = rsslGetTimeNano();
		time_interval.tv_usec = (long)((_currentTime > nextTickTime) ? 0 : ((nextTickTime - _currentTime)/1000));
		selRet = select(FD_SETSIZE, &useRead, NULL, &useExcept, &time_interval);

		if (selRet == 0)
		{
			/* We've reached the next tick. */
			nextTickTime += nsecPerTick;
			++currentTicks;

			if (currentTicks == providerThreadConfig.ticksPerSec)
			{
				++currentRuntimeSec;
				++intervalSeconds;
				currentTicks = 0;
			}

			if (intervalSeconds == provPerfConfig.writeStatsInterval)
			{
				providerCollectStats(&provider, RSSL_TRUE, provPerfConfig.displayStats, currentRuntimeSec,
						provPerfConfig.writeStatsInterval);
				intervalSeconds = 0;
			}

		}
		else if (selRet > 0)
		{
			if ((rsslSrvr != NULL) && (rsslSrvr->socketId != -1) && (FD_ISSET(rsslSrvr->socketId,&useRead)))
			{
				if (provPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
				{
					RsslChannel *pChannel;
					RsslError	error;
					RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

					if ((pChannel = rsslAccept(rsslSrvr, &acceptOpts, &error)) == 0)
					{
						printf("rsslAccept: failed <%s>\n",error.text);
					}
					else
					{
						printf("Server "SOCKET_PRINT_TYPE" accepting channel "SOCKET_PRINT_TYPE".\n\n", rsslSrvr->socketId, pChannel->socketId);
						sendToLeastLoadedThread(pChannel);
					}
				}
				else // use ETA VA Reactor
				{
					if (acceptReactorConnection(rsslSrvr, &rsslErrorInfo) != RSSL_RET_SUCCESS)
					{
						printf("acceptReactorConnection: failed <%s>\n", rsslErrorInfo.rsslError.text);
					}
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
			exit(1);
		}

		/* Handle run-time */
		if (_currentTime >= rsslProviderRuntime)
		{
			printf("\nRun time of %u seconds has expired.\n", provPerfConfig.runTime);
			signal_shutdown = RSSL_TRUE; /* Tell other threads to shutdown. */
		}

		if (signal_shutdown == RSSL_TRUE)
		{
			cleanUpAndExit();
		}
	}
}

/* Gives a channel to a provider thread that has the fewest open channels. */
static RsslRet sendToLeastLoadedThread(RsslChannel *pChannel)
{
	ProviderThread *pProvThread = NULL;
	RsslInt32 i, minProviderConnCount, connHandlerIndex;

	minProviderConnCount = 0x7fffffff;
	for(i = 0; i < providerThreadConfig.threadCount; ++i)
	{
		ProviderThread *pTmpProvThread = &provider.providerThreadList[i];
		RsslInt32 connCount = providerThreadGetConnectionCount(pTmpProvThread);
		if (connCount < minProviderConnCount)
		{
			minProviderConnCount = connCount;
			pProvThread = pTmpProvThread;
			connHandlerIndex = i;
		}
	}

	providerThreadSendNewChannel(pProvThread, pChannel);

	return RSSL_RET_SUCCESS;
}

RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo)
{
	ProviderThread *pProvThread = (ProviderThread*)pChanHandler->pUserSpec;
	ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;
	RsslError error;
	RsslRet ret;
	RsslChannelInfo channelInfo;
	RsslUInt32 count;

#ifdef ENABLE_XML_TRACE
	RsslTraceOptions traceOptions;
	rsslClearTraceOptions(&traceOptions);
	traceOptions.traceMsgFileName = "ProvPerf";
	traceOptions.traceMsgMaxFileSize = 1000000000;
	traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
	rsslIoctl(pChannelInfo->pChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &error);
#endif

	if (provPerfConfig.highWaterMark > 0)
	{
		if (rsslIoctl(pChannelInfo->pChannel, RSSL_HIGH_WATER_MARK, &provPerfConfig.highWaterMark, &error) != RSSL_RET_SUCCESS)
		{
			printf("rsslIoctl() of RSSL_HIGH_WATER_MARK failed <%s>\n", error.text);
			exit(-1);
		}
	}

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
			channelInfo.compressionType == RSSL_COMP_ZLIB ? "zlib" : "none",
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

	switch (channelInfo.encryptionProtocol)
	{
	case RSSL_ENC_TLSV1_2:
		printf("\n  Encryption protocol: TLSv1.2");
		break;
	case RSSL_ENC_TLSV1_3:
		printf("\n  Encryption protocol: TLSv1.3");
		break;
	default:
		printf("\n  Encryption protocol: unknown");
	}

	printf ("\n\n");

	/* Check that we can successfully pack, if packing messages. */
	if (providerThreadConfig.totalBuffersPerPack > 1
			&& providerThreadConfig.packingBufferLength > channelInfo.maxFragmentSize)
	{
		printf("Error(Channel "SOCKET_PRINT_TYPE"): MaxFragmentSize %u is too small for packing buffer size %u\n",
				pChannelInfo->pChannel->socketId, channelInfo.maxFragmentSize, 
				providerThreadConfig.packingBufferLength);
		exit(-1);
	}



	if (ret = (printEstimatedMsgSizes(pProvThread, pProvSession)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	pProvSession->timeActivated = rsslGetTimeNano();

	return RSSL_RET_SUCCESS;
}

void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError)
{
	ProviderThread *pProvThread = (ProviderThread*)pChanHandler->pUserSpec;
	ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;

	pProvThread->stats.inactiveTime = rsslGetTimeNano();

	if(pError)
		printf("Channel Closed: %s(%s)\n", rsslRetCodeToString(pError->rsslErrorId), pError->text);
	else
		printf("Channel Closed.\n");
	

	if (pProvSession)
		providerSessionDestroy(pProvThread, pProvSession);
}

RsslBuffer *convertMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer)
{
	ProviderThread *pProvThread = (ProviderThread*)pChannelHandler->pUserSpec;
	RsslChannel *pChannel = pChannelInfo->pChannel;
	RsslErrorInfo eInfo;
	RsslBuffer *pMsgBuffer;

	pMsgBuffer = 0;
				/* convert message to JSON */
	if ((pMsgBuffer = rjcMsgConvertToJson(&(pProvThread->rjcSess), pChannel, pBuffer, &eInfo)) == NULL)
		fprintf(stderr, "convertMsg(): Failed to convert RWF > JSON %s\n", 
						(eInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS ?
							"Out of pool buffers" : eInfo.rsslError.text));

	return pMsgBuffer;
}

RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer)
{
	RsslRet ret = RSSL_RET_SUCCESS; RsslMsg msg = RSSL_INIT_MSG;
	RsslRet	cRet = 0;
	RsslDecodeIterator dIter;
	ProviderThread *pProvThread = (ProviderThread*)pChannelHandler->pUserSpec;
	RsslChannel *pChannel = pChannelInfo->pChannel;
	RsslBuffer	*origBuffer = 0;
	RsslBuffer decodedMsg = RSSL_INIT_BUFFER;
	RsslInt16	numConverted = 0;
	

	do {
		if (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
		{
			RsslErrorInfo	pErr;

			origBuffer = pBuffer;

			if((cRet = rjcMsgConvertFromJson(&(pProvThread->rjcSess), pChannel, &decodedMsg, 
										 (numConverted == 0 ?  origBuffer : NULL), &pErr)) == RSSL_RET_FAILURE)
			{
				ret = cRet;
				printf("Error in Json Conversion, fd="SOCKET_PRINT_TYPE" error: %s\n", 
																pChannel->socketId, pErr.rsslError.text);
				break;
			}
			numConverted++;

			if (cRet == RSSL_RET_SUCCESS && decodedMsg.length > 0)
				pBuffer = &decodedMsg;
		
			if (cRet == RSSL_RET_END_OF_CONTAINER)
				break;

			if (cRet != RSSL_RET_SUCCESS)
				continue;
		}
		/* clear decode iterator */
		rsslClearDecodeIterator(&dIter);
		
		/* set version info */
		rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion, pChannel->minorVersion);

		rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

		ret = rsslDecodeMsg(&dIter, &msg);				
		if (ret != RSSL_RET_SUCCESS)
		{
			printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", ret, pChannel->socketId, pBuffer->length);
			cleanUpAndExit();
		}

		switch ( msg.msgBase.domainType )
		{
			case RSSL_DMT_LOGIN:
				ret = processLoginRequest(pChannelHandler, pChannelInfo, &msg, &dIter);
				break;
			case RSSL_DMT_SOURCE:
				ret = processDirectoryRequest(pChannelHandler, pChannelInfo, &msg, &dIter);
				break;
			case RSSL_DMT_DICTIONARY:
				ret = processDictionaryRequest(pChannelHandler, pChannelInfo, &msg, &dIter); 
				break; 
			case RSSL_DMT_MARKET_PRICE: 
				if (xmlMsgDataHasMarketPrice) 
					ret = processItemRequest(pProvThread, (ProviderSession*)pChannelInfo->pUserSpec, &msg, &dIter);
				else
					ret = sendItemRequestReject(pProvThread, (ProviderSession*)pChannelInfo->pUserSpec, 
							msg.msgBase.streamId, msg.msgBase.domainType, DOMAIN_NOT_SUPPORTED);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				if (xmlMsgDataHasMarketByOrder)
					ret = processItemRequest(pProvThread, (ProviderSession*)pChannelInfo->pUserSpec, &msg, &dIter);
				else
					ret = sendItemRequestReject(pProvThread, (ProviderSession*)pChannelInfo->pUserSpec, 
							msg.msgBase.streamId, msg.msgBase.domainType, DOMAIN_NOT_SUPPORTED);
				break;
			default:
				ret = sendItemRequestReject(pProvThread, (ProviderSession*)pChannelInfo->pUserSpec, 
						msg.msgBase.streamId, msg.msgBase.domainType, DOMAIN_NOT_SUPPORTED);
				break;
		}

		if (ret < RSSL_RET_SUCCESS) 
		{
			printf("Failed to process request from domain %d: %d\n", msg.msgBase.domainType, ret);
		}
		else if (ret > RSSL_RET_SUCCESS)
		{
			/* The function sent a message and indicated that we need to flush. */
			providerThreadRequestChannelFlush(pProvThread, pChannelInfo);
		}

	} while (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && cRet != RSSL_RET_END_OF_CONTAINER);

	return ret;
}

void cleanUpAndExit()
{
	printf("\nShutting down.\n\n");

	providerWaitForThreads(&provider);

	/* Collect final stats before writing summary. */
	providerCollectStats(&provider, RSSL_FALSE, RSSL_FALSE, 0, 0);

	providerPrintSummaryStats(&provider, stdout);
	providerPrintSummaryStats(&provider, summaryFile);
	fclose(summaryFile);

	providerCleanup(&provider);

	/* if we did a bind, clean it up */
	if (rsslSrvr)
	{
		RsslError error;
		FD_CLR(rsslSrvr->socketId, &readfds);
		FD_CLR(rsslSrvr->socketId, &exceptfds);
		rsslCloseServer(rsslSrvr, &error);
	}

	providerThreadConfigCleanup();

	rsslUninitialize();
	xmlCleanupParser();
	printf("Exiting.\n");
	exit(0);
}

static RsslRet processItemRequest(ProviderThread *pProvThread, ProviderSession *pProvSession, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	ItemInfo* itemInfo = NULL;
	RsslMsgKey* key = 0;
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslItemAttributes attribs;
	RsslChannel *pChannel = pProvSession->pChannelInfo->pChannel;

	attribs.domainType = msg->msgBase.domainType;
	attribs.pMsgKey = &msg->msgBase.msgKey;


	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REQUEST:
		countStatIncr(&pProvThread->itemRequestCount);

		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		/* check if item count reached */
		if ((pProvSession->openItemsCount >= directoryConfig.openLimit))
		{
			sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, ITEM_COUNT_REACHED);
			break;
		}
		/* check if service id correct */
		if (key->serviceId != directoryConfig.serviceId)
		{
			sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, INVALID_SERVICE_ID);
			break;
		}
		/* check if QoS supported */
		if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_WORST_QOS &&
			((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsInRange(&((RsslRequestMsg *)msg)->qos, &((RsslRequestMsg *)msg)->worstQos, &directoryConfig.qos))
			{
				sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, QOS_NOT_SUPPORTED);
				break;
			}
		}
		else if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsEqual(&((RsslRequestMsg *)msg)->qos, &directoryConfig.qos))
			{
				sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, QOS_NOT_SUPPORTED);
				break;
			}
		}

		/* check if item already opened with exact same key and domain.
		 * If we find one, check the StreamId.
		 * If the streamId matches, it is a reissue.
		 * If the streamId does not match, reject the redundant request. */
		itemInfo = findAlreadyOpenItem(pProvSession, msg, &attribs);
		if (itemInfo && itemInfo->StreamId != msg->msgBase.streamId)
		{
			sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, ITEM_ALREADY_OPENED);
			break;
		}

		/* check if stream already in use with a different key */
		if (isStreamInUse(pProvSession, msg->msgBase.streamId, key))
		{
			sendItemRequestReject(pProvThread, pProvSession, msg->msgBase.streamId, attribs.domainType, STREAM_ALREADY_IN_USE);
			break;
		}

		if (!itemInfo)
		{
			RsslItemAttributes attributes;

			/* New request */
			/* get item info structure */
			attributes.pMsgKey = key;
			attributes.domainType = msg->msgBase.domainType;
			itemInfo = createItemInfo(pProvThread, pProvSession, &attributes, msg->msgBase.streamId);

			if (!itemInfo)
			{
				printf("Failed to get storage for item.\n");
				return RSSL_RET_FAILURE;
			}

			/* get StreamId */
			itemInfo->StreamId = msg->requestMsg.msgBase.streamId;

			hashTableInsertLink(&pProvSession->itemAttributesTable, &itemInfo->itemAttributesTableLink, &itemInfo->attributes);
			hashTableInsertLink(&pProvSession->itemStreamIdTable, &itemInfo->itemStreamIdTableLink, &itemInfo->StreamId);

			itemInfo->itemFlags |= ITEM_IS_SOLICITED;
		}
		else
		{
			/* else it was a reissue */
			if (!(msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH))
			{
				/* Move item back to refresh queue. */
				if (itemInfo->myQueue)
				{
					rotatingQueueRemove(itemInfo->myQueue, &itemInfo->watchlistLink);
					itemInfo->myQueue = &pProvSession->refreshItemList;
					rotatingQueueInsert(&pProvSession->refreshItemList, &itemInfo->watchlistLink);
				}
			}


		}

		/* get IsStreamingRequest */
		if (msg->requestMsg.flags & RSSL_RQMF_STREAMING)
		{
			itemInfo->itemFlags |= ITEM_IS_STREAMING_REQ;
		}

		/* check if the request is for a private stream */
		if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
		{
			itemInfo->itemFlags |= ITEM_IS_PRIVATE;
		}

		break;

	case RSSL_MC_POST:
	{
		countStatIncr(&pProvThread->postMsgCount);
		return reflectPostMsg(pProvThread, dIter, pProvSession, &msg->postMsg);
	}

	case RSSL_MC_GENERIC:
	{
		countStatIncr(&pProvThread->stats.genMsgRecvCount);
		if (!pProvThread->stats.firstGenMsgRecvTime)
			pProvThread->stats.firstGenMsgRecvTime = rsslGetTimeNano();
		return processGenMsg(pProvThread, dIter, pProvSession, &msg->genericMsg);
	}

	case RSSL_MC_CLOSE:
		/* close item stream */
		countStatIncr(&pProvThread->closeMsgCount);
		closeItemStream(pProvThread, pProvSession, msg->msgBase.streamId);
		break;

	default:
		printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return ret;
}

static ItemInfo *findAlreadyOpenItem(ProviderSession *pProvSession, RsslMsg* msg, RsslItemAttributes* attribs)
{
	HashTableLink *pLink = hashTableFind(&pProvSession->itemAttributesTable, attribs);
	return (pLink ? HASH_TABLE_LINK_TO_OBJECT(ItemInfo, itemAttributesTableLink, pLink) : NULL);
}

static RsslBool isStreamInUse(ProviderSession *pProvSession, RsslInt32 streamId, RsslMsgKey* key)
{
	HashTableLink *pLink;
	ItemInfo *itemInfo;

	pLink = hashTableFind(&pProvSession->itemStreamIdTable, &streamId);
	if (!pLink) return RSSL_FALSE;
	
	itemInfo = HASH_TABLE_LINK_TO_OBJECT(ItemInfo, itemStreamIdTableLink, pLink);

	return ((rsslCompareMsgKeys(itemInfo->attributes.pMsgKey, key) != RSSL_RET_SUCCESS
		 ) ? RSSL_TRUE : RSSL_FALSE);
}

static RsslRet sendItemRequestReject(ProviderThread *pProvThread, ProviderSession* pProvSession, RsslInt32 streamId, RsslUInt8 domainType, ItemRejectReason reason)
{
	RsslRet ret;

	if ((ret = getItemMsgBuffer(pProvThread, pProvSession, 128) < RSSL_RET_SUCCESS))
		return ret;

	/* encode request reject status */
	if ((ret = encodeItemRequestReject(pProvSession->pChannelInfo->pChannel, streamId, reason, pProvSession->pWritingBuffer, domainType)) < RSSL_RET_SUCCESS)
	{
		printf("\nencodeItemRequestReject() failed: %d\n", ret);
		return RSSL_RET_FAILURE;
	}

	printf("\nRejecting Item Request with streamId=%d and domain %s.  Reason: %s\n", streamId,  rsslDomainTypeToString(domainType), itemRejectReasonToString(reason));

	if (perfTunnelMsgHandlerGetStreamId() == streamId && domainType == RSSL_DMT_SYSTEM && provPerfConfig.useReactor == RSSL_FALSE)
	{
		printf("The Item Request for TunnelStream. It requires to use -reactor in command line.\n");
	}

	/* send request reject status */
	return sendItemMsgBuffer(pProvThread, pProvSession, RSSL_TRUE);
}


static RsslRet sendItemCloseStatusMsg(ProviderThread *pProvThread, ProviderSession* pProvSession, ItemInfo* itemInfo)
{
	RsslRet ret;
	RsslChannel *pChannel = pProvSession->pChannelInfo->pChannel;

	/* get a buffer for the close status */
	if ((ret = getItemMsgBuffer(pProvThread, pProvSession, 128) < RSSL_RET_SUCCESS))
		return ret;

	/* encode close status */
	if ((ret = encodeItemCloseStatus(pChannel, itemInfo, pProvSession->pWritingBuffer, itemInfo->StreamId)) < RSSL_RET_SUCCESS)
	{
		printf("\nencodeItemCloseStatus() failed: %d\n", ret);
		return ret;
	}

	/* send close status */
	return sendItemMsgBuffer(pProvThread, pProvSession, RSSL_TRUE);
}

static RsslRet reflectPostMsg(ProviderThread *pProvThread, RsslDecodeIterator *pIter, ProviderSession *pProvSession, RsslPostMsg *pPostMsg)
{
	RsslRet ret;
	RsslChannel *pChannel = pProvSession->pChannelInfo->pChannel;

	RsslMsg msgToReflect;
	RsslEncodeIterator eIter;


	switch(pPostMsg->msgBase.containerType)
	{
		case RSSL_DT_MSG:
			if ((ret = rsslDecodeMsg(pIter, &msgToReflect)) != RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			/* It's a container(e.g. field list). Add an update header for reflecting. */
			rsslClearUpdateMsg(&msgToReflect.updateMsg);
			msgToReflect.updateMsg.msgBase.containerType = pPostMsg->msgBase.containerType;
			msgToReflect.updateMsg.msgBase.domainType = pPostMsg->msgBase.domainType;
			msgToReflect.updateMsg.msgBase.encDataBody = pPostMsg->msgBase.encDataBody;
			break;
	}

	/* get a buffer for the response */
	if (rtrUnlikely((ret = getItemMsgBuffer(pProvThread, pProvSession, 128 + msgToReflect.msgBase.encDataBody.length)) 
				< RSSL_RET_SUCCESS))
		return ret;

	/* Add the post user info from the post message to the nested message and re-encode. */
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pProvSession->pWritingBuffer);

	/* Add stream ID of PostMsg to nested message. */
	msgToReflect.msgBase.streamId = pPostMsg->msgBase.streamId;

	/* Add PostUserInfo of PostMsg to nested message. */
	switch(msgToReflect.msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			msgToReflect.refreshMsg.postUserInfo = pPostMsg->postUserInfo;
			msgToReflect.refreshMsg.flags |= RSSL_RFMF_HAS_POST_USER_INFO;
			msgToReflect.refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;
			break;
		case RSSL_MC_UPDATE:
			msgToReflect.updateMsg.postUserInfo = pPostMsg->postUserInfo;
			msgToReflect.updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
			break;
		case RSSL_MC_STATUS:
			msgToReflect.statusMsg.postUserInfo = pPostMsg->postUserInfo;
			msgToReflect.statusMsg.flags |= RSSL_STMF_HAS_POST_USER_INFO;
			break;
		default:
			printf("Error: Unhandled message class in post: %s(%u)\n", 
					rsslMsgClassToString(msgToReflect.msgBase.msgClass),
					msgToReflect.msgBase.msgClass);
			return RSSL_RET_FAILURE;
	}

	/* Other header members & data body should be properly set, so re-encode. */
	if (ret = rsslEncodeMsg(&eIter, &msgToReflect) != RSSL_RET_SUCCESS)
		return ret;

	pProvSession->pWritingBuffer->length = rsslGetEncodedBufferLength(&eIter);

	return sendItemMsgBuffer(pProvThread, pProvSession, RSSL_FALSE);
}

RTR_C_INLINE void updateLatencyStats(ProviderThread *pProviderThread, RsslTimeValue timeTracker)
{
	RsslTimeValue currentTime;
	RsslTimeValue unitsPerMicro;

	currentTime = providerThreadConfig.nanoTime ? rsslGetTimeNano() : rsslGetTimeMicro();
	unitsPerMicro = providerThreadConfig.nanoTime ? 1000 : 1;

	timeRecordSubmit(&pProviderThread->genMsgLatencyRecords, timeTracker, currentTime, unitsPerMicro);
}

RsslRet decodeMPUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ProviderThread* pProviderThread)
{
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;

	RsslDataType dataType;
	RsslPrimitive primitive;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	RsslUInt genMsgTimeTracker = 0;

	/* decode field list */
	if ((ret = rsslDecodeFieldList(pIter, &fList, 0)) == RSSL_RET_SUCCESS)
	{
		while ((ret = rsslDecodeFieldEntry(pIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
		{	
			if (ret != RSSL_RET_SUCCESS) return ret;

			/* get dictionary entry */
			dictionaryEntry = getDictionaryEntry(pProviderThread->pDictionary,
					fEntry.fieldId);

			if  (!dictionaryEntry)
			{
				printf("Error: Decoded field ID %d not present in dictionary.\n", fEntry.fieldId);
				return RSSL_RET_FAILURE;
			}

			/* decode and print out fid value */
			dataType = dictionaryEntry->rwfType;
			switch (dataType)
			{
				case RSSL_DT_INT:
					if((ret = rsslDecodeInt(pIter, &primitive.intType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_UINT:
					if((ret = rsslDecodeUInt(pIter, &primitive.uintType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_FLOAT:
					if ((ret = rsslDecodeFloat(pIter, &primitive.floatType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_DOUBLE:
					if ((ret = rsslDecodeDouble(pIter, &primitive.doubleType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_REAL:
					if ((ret = rsslDecodeReal(pIter, &primitive.realType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_DATE:
					if ((ret = rsslDecodeDate(pIter, &primitive.dateType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_TIME:
					if ((ret = rsslDecodeTime(pIter, &primitive.timeType)) < RSSL_RET_SUCCESS )
						return ret;
					break;
				case RSSL_DT_DATETIME:
					if ((ret = rsslDecodeDateTime(pIter, &primitive.dateTimeType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_QOS:
					if ((ret = rsslDecodeQos(pIter, &primitive.qosType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_STATE:
					if ((ret = rsslDecodeState(pIter, &primitive.stateType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_ENUM:
				{
					RsslEnumType *pEnumTypeInfo;
					if ((ret = rsslDecodeEnum(pIter, &primitive.enumType)) < RSSL_RET_SUCCESS )
						return ret;

					if (ret == RSSL_RET_BLANK_DATA)
						break;

					pEnumTypeInfo = getFieldEntryEnumType(dictionaryEntry, primitive.enumType);
					if (pEnumTypeInfo)
						primitive.bufferType = pEnumTypeInfo->display;
					break;
				}
				case RSSL_DT_BUFFER:
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_RMTES_STRING:
					if ((ret = rsslDecodeBuffer(pIter, &primitive.bufferType)) < RSSL_RET_SUCCESS )
						return ret;
					break;
				default:
					printf("Error: Unhandled data type %s(%u) in field with ID %u.\n", 
							rsslDataTypeToString(dataType), dataType, fEntry.fieldId);
					return RSSL_RET_FAILURE;
			}

			if (msg->msgBase.msgClass == RSSL_MC_GENERIC && ret != RSSL_RET_BLANK_DATA)
			{
				if(fEntry.fieldId == TIM_TRK_3_FID)
					genMsgTimeTracker = primitive.uintType;
			}
		}
	}
	else
	{
		return ret;
	}
	
	if(genMsgTimeTracker)
		updateLatencyStats(pProviderThread, genMsgTimeTracker);

	return RSSL_RET_SUCCESS;
}


RsslRet decodeMBOUpdate(RsslDecodeIterator* pIter, RsslMsg* msg, ProviderThread* pProviderThread)
{
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslMap Map = RSSL_INIT_MAP;
	RsslMapEntry mEntry = RSSL_INIT_MAP_ENTRY;
	RsslRet ret = 0;
	
	RsslPrimitive key;
	RsslDataType dataType;
	RsslPrimitive primitive;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	RsslUInt genMsgTimeTracker = 0;
	
	/* decode field list */
			
	if((ret = rsslDecodeMap(pIter, &Map)) == RSSL_RET_SUCCESS)
	{
		if(rsslMapCheckHasSetDefs(&Map) == RSSL_TRUE)
		{
			rsslClearLocalFieldSetDefDb(&pProviderThread->fListSetDef);
			pProviderThread->fListSetDef.entries.data = pProviderThread->setDefMemory;
			pProviderThread->fListSetDef.entries.length = sizeof(pProviderThread->setDefMemory);
			if((ret = rsslDecodeLocalFieldSetDefDb(pIter, &pProviderThread->fListSetDef)) != RSSL_RET_SUCCESS)
				return ret;
		}
		
		if(rsslMapCheckHasSummaryData(&Map) == RSSL_TRUE)
		{
			if((ret = rsslDecodeFieldList(pIter, &fList, &pProviderThread->fListSetDef)) == RSSL_RET_SUCCESS)
			{
				while ((ret = rsslDecodeFieldEntry(pIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
				{	
					if (ret != RSSL_RET_SUCCESS) return ret;

					/* get dictionary entry */
					dictionaryEntry = getDictionaryEntry(pProviderThread->pDictionary,
							fEntry.fieldId);

					if  (!dictionaryEntry)
					{
						printf("Error: Decoded field ID %d not present in dictionary.\n", fEntry.fieldId);
						return RSSL_RET_FAILURE;
					}

					/* decode and print out fid value */
					dataType = dictionaryEntry->rwfType;

					switch (dataType)
					{
						case RSSL_DT_INT:
							if((ret = rsslDecodeInt(pIter, &primitive.intType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_UINT:
							if((ret = rsslDecodeUInt(pIter, &primitive.uintType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_FLOAT:
							if ((ret = rsslDecodeFloat(pIter, &primitive.floatType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_DOUBLE:
							if ((ret = rsslDecodeDouble(pIter, &primitive.doubleType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_REAL:
							if ((ret = rsslDecodeReal(pIter, &primitive.realType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_DATE:
							if ((ret = rsslDecodeDate(pIter, &primitive.dateType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_TIME:
							if ((ret = rsslDecodeTime(pIter, &primitive.timeType)) < RSSL_RET_SUCCESS )
								return ret;
							break;
						case RSSL_DT_DATETIME:
							if ((ret = rsslDecodeDateTime(pIter, &primitive.dateTimeType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_QOS:
							if ((ret = rsslDecodeQos(pIter, &primitive.qosType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_STATE:
							if ((ret = rsslDecodeState(pIter, &primitive.stateType)) < RSSL_RET_SUCCESS)
								return ret;
							break;
						case RSSL_DT_ENUM:
							{
								RsslEnumType *pEnumTypeInfo;
								if ((ret = rsslDecodeEnum(pIter, &primitive.enumType)) < RSSL_RET_SUCCESS )
									return ret;

								if (ret == RSSL_RET_BLANK_DATA)
									break;

								pEnumTypeInfo = getFieldEntryEnumType(dictionaryEntry, primitive.enumType);
								if (pEnumTypeInfo)
									primitive.bufferType = pEnumTypeInfo->display;
								break;
							}
						case RSSL_DT_BUFFER:
						case RSSL_DT_ASCII_STRING:
						case RSSL_DT_UTF8_STRING:
						case RSSL_DT_RMTES_STRING:
							if ((ret = rsslDecodeBuffer(pIter, &primitive.bufferType)) < RSSL_RET_SUCCESS )
								return ret;
							break;
						default:
							printf("Error: Unhandled data type %s(%u) in field with ID %u.\n", 
									rsslDataTypeToString(dataType), dataType, fEntry.fieldId);
							return RSSL_RET_FAILURE;
					}

					if (msg->msgBase.msgClass == RSSL_MC_GENERIC && ret != RSSL_RET_BLANK_DATA)
					{
						if(fEntry.fieldId == TIM_TRK_3_FID)
							genMsgTimeTracker = primitive.uintType;
					}
				}
			}
			else
			{
				return ret;
			}
		}

		while((ret = rsslDecodeMapEntry(pIter, &mEntry, &key)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (mEntry.action != RSSL_MPEA_DELETE_ENTRY)
			{
				if((ret = rsslDecodeFieldList(pIter, &fList, &pProviderThread->fListSetDef)) == RSSL_RET_SUCCESS)
				{
					while ((ret = rsslDecodeFieldEntry(pIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
					{	
						if (ret != RSSL_RET_SUCCESS) return ret;

						/* get dictionary entry */
						dictionaryEntry = getDictionaryEntry(pProviderThread->pDictionary,
								fEntry.fieldId);

						if  (!dictionaryEntry)
						{
							printf("Error: Decoded field ID %d not present in dictionary.\n", fEntry.fieldId);
							return RSSL_RET_FAILURE;
						}

						/* decode and print out fid value */
						dataType = dictionaryEntry->rwfType;
						
						switch (dataType)
						{
							case RSSL_DT_INT:
								if((ret = rsslDecodeInt(pIter, &primitive.intType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_UINT:
								if((ret = rsslDecodeUInt(pIter, &primitive.uintType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_FLOAT:
								if ((ret = rsslDecodeFloat(pIter, &primitive.floatType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_DOUBLE:
								if ((ret = rsslDecodeDouble(pIter, &primitive.doubleType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_REAL:
								if ((ret = rsslDecodeReal(pIter, &primitive.realType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_DATE:
								if ((ret = rsslDecodeDate(pIter, &primitive.dateType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_TIME:
								if ((ret = rsslDecodeTime(pIter, &primitive.timeType)) < RSSL_RET_SUCCESS )
									return ret;
								break;
							case RSSL_DT_DATETIME:
								if ((ret = rsslDecodeDateTime(pIter, &primitive.dateTimeType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_QOS:
								if ((ret = rsslDecodeQos(pIter, &primitive.qosType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_STATE:
								if ((ret = rsslDecodeState(pIter, &primitive.stateType)) < RSSL_RET_SUCCESS)
									return ret;
								break;
							case RSSL_DT_ENUM:
								{
									RsslEnumType *pEnumTypeInfo;
									if ((ret = rsslDecodeEnum(pIter, &primitive.enumType)) < RSSL_RET_SUCCESS )
										return ret;

									if (ret == RSSL_RET_BLANK_DATA)
										break;

									pEnumTypeInfo = getFieldEntryEnumType(dictionaryEntry, primitive.enumType);
									if (pEnumTypeInfo)
										primitive.bufferType = pEnumTypeInfo->display;
									break;
								}
							case RSSL_DT_BUFFER:
							case RSSL_DT_ASCII_STRING:
							case RSSL_DT_UTF8_STRING:
							case RSSL_DT_RMTES_STRING:
								if ((ret = rsslDecodeBuffer(pIter, &primitive.bufferType)) < RSSL_RET_SUCCESS )
									return ret;
								break;
							default:
								printf("Error: Unhandled data type %s(%u) in field with ID %u.\n", 
										rsslDataTypeToString(dataType), dataType, fEntry.fieldId);
								return RSSL_RET_FAILURE;
						}
					}
				}
				else
				{
					return ret;
				}
			}
		}
	}
	else
	{
		return ret;
	}
	
	if(genMsgTimeTracker)
		updateLatencyStats(pProviderThread, genMsgTimeTracker);

	return RSSL_RET_SUCCESS;
}


RsslRet RTR_C_INLINE decodePayload(RsslDecodeIterator* dIter, RsslMsg *msg, ProviderThread* pProviderThread)
{
	RsslRet ret;
	RsslErrorInfo errorInfo;

	switch(msg->msgBase.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			if ((ret = decodeMPUpdate(dIter, msg, pProviderThread)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMPUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		case RSSL_DMT_MARKET_BY_ORDER:
			if ((ret = decodeMBOUpdate(dIter, msg, pProviderThread) != RSSL_RET_SUCCESS))
			{
				rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						(char*)"decodeMBOUpdate() failed: %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}
			return RSSL_RET_SUCCESS;
		default:
			rsslSetErrorInfo(&errorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
					(char*)"decodePayload(): Unhandled domain type %s(%d)", 
					rsslDomainTypeToString(msg->msgBase.domainType), msg->msgBase.domainType);
			return RSSL_RET_FAILURE;
	}
}

static RsslRet processGenMsg(ProviderThread *pProvThread, RsslDecodeIterator *pIter, ProviderSession *watchlist, RsslGenericMsg *pGenMsg)
{
	RsslRet ret = 0;
	RsslTimeValue decodeTimeStart;
	if (providerThreadConfig.measureDecode)
		decodeTimeStart = rsslGetTimeNano();

	if((ret = decodePayload(pIter, (RsslMsg*)pGenMsg, pProvThread)) 
			!= RSSL_RET_SUCCESS)
	{
		providerThreadCloseChannel(pProvThread, watchlist->pChannelInfo);
		signal_shutdown = RSSL_TRUE;
		RSSL_THREAD_RETURN();

	}

	if (providerThreadConfig.measureDecode)
	{
		RsslTimeValue decodeTimeEnd = rsslGetTimeNano();
		timeRecordSubmit(&pProvThread->updateDecodeTimeRecords, decodeTimeStart, decodeTimeEnd, 1000);
	}

	return ret;
}
