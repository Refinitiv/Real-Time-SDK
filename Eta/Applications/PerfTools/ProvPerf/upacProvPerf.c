/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "upacProvPerf.h"
#include "provPerfConfig.h"
#include "statistics.h"
#include "dictionaryProvider.h"
#include "getTime.h"
#include "testUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>

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
static TimeValue rsslProviderRuntime = 0;

TimeValue _currentTime;

static RsslServer *rsslSrvr = NULL;

static RsslInt64 nsecPerTick;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

RsslBool showTransportDetails = RSSL_FALSE;

static void signal_handler(int sig)
{
	signal_shutdown = RSSL_TRUE;
}

extern void startProviderThreads(Provider *pProvider, RSSL_THREAD_DECLARE(threadFunction,pArg));

RSSL_THREAD_DECLARE(runConnectionHandler, pArg)
{

	ProviderThread *pProvThread = (ProviderThread*)pArg;

	TimeValue nextTickTime;
	RsslInt32 currentTicks = 0;

	if (pProvThread->cpuId >= 0)
	{
		if (bindThread(pProvThread->cpuId) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %d.\n", pProvThread->cpuId);
			exit(-1);
		}
	}

	nextTickTime = getTimeNano() + nsecPerTick;

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

int main(int argc, char **argv)
{
	struct timeval time_interval;
	RsslError error;
	fd_set useRead;
	fd_set useExcept;
	int selRet;
	RsslRet	ret = 0;
	RsslBindOptions sopts;

	TimeValue nextTickTime;
	RsslInt32 currentTicks;
	RsslUInt32 currentRuntimeSec = 0, intervalSeconds = 0;

	TimeValue tickSetStartTime; /* Holds when a paritcular run of ticks started(basically one second's worth)*/

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

	_currentTime = getTimeNano();
	nextTickTime = _currentTime;
	currentTicks = 0;

	xmlInitParser();

	/* Initialize RSSL */
	if (rsslInitialize(providerThreadConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
	{
		printf("RsslInitialize failed: %s\n", error.text);
		exit(-1);
	}

	/* Initialize run-time */
	rsslProviderRuntime = getTimeNano() + ((RsslInt64)provPerfConfig.runTime * 1000000000LL);

	providerInit(&provider, PROVIDER_INTERACTIVE,
			processActiveChannel,
			processInactiveChannel,
			processMsg);

	startProviderThreads(&provider, runConnectionHandler);

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);


	rsslClearBindOpts(&sopts);

	sopts.guaranteedOutputBuffers = provPerfConfig.guaranteedOutputBuffers;
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

	if ((rsslSrvr = rsslBind(&sopts, &error)) == 0)
	{
		printf("rsslBind() failed: %d(%s)\n", error.rsslErrorId, error.text);
		exit(-1);
	}

	printf("Server %d bound to port %d.\n\n", rsslSrvr->socketId, rsslSrvr->portNumber);
	FD_SET(rsslSrvr->socketId,&readfds);
	FD_SET(rsslSrvr->socketId,&exceptfds);

	time_interval.tv_sec = 0; time_interval.tv_usec = 0;
	nextTickTime = getTimeNano() + nsecPerTick;
	currentTicks = 0;
	tickSetStartTime = getTimeNano();

	/* this is the main loop */
	while(1)
	{
		useRead = readfds;
		useExcept = exceptfds;

		/* select() on remaining time for this tick. If we went into the next tick, don't delay at all. */
		_currentTime = getTimeNano();
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
				RsslChannel *pChannel;
				RsslError	error;
				RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

				if ((pChannel = rsslAccept(rsslSrvr, &acceptOpts, &error)) == 0)
				{
					printf("rsslAccept: failed <%s>\n",error.text);
				}
				else
				{
					printf("Server %d accepting channel %d.\n\n", rsslSrvr->socketId, pChannel->socketId);
					sendToLeastLoadedThread(pChannel);
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
	ProviderThread *pProvThread;
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
	traceOptions.traceMsgFileName = "upacProvPerf";
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

	printf( "Channel %d active. Channel Info:\n"
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
	printf ("\n\n");

	/* Check that we can successfully pack, if packing messages. */
	if (providerThreadConfig.totalBuffersPerPack > 1
			&& providerThreadConfig.packingBufferLength > channelInfo.maxFragmentSize)
	{
		printf("Error(Channel %d): MaxFragmentSize %u is too small for packing buffer size %u\n",
				pChannelInfo->pChannel->socketId, channelInfo.maxFragmentSize, 
				providerThreadConfig.packingBufferLength);
		exit(-1);
	}



	if (ret = (printEstimatedMsgSizes(pProvThread, pProvSession)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	pProvSession->timeActivated = getTimeNano();

	return RSSL_RET_SUCCESS;
}

void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError)
{
	ProviderThread *pProvThread = (ProviderThread*)pChanHandler->pUserSpec;
	ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;

	pProvThread->stats.inactiveTime = getTimeNano();

	if(pError)
		printf("Channel Closed: %s(%s)\n", rsslRetCodeToString(pError->rsslErrorId), pError->text);
	else
		printf("Channel Closed.\n");
	

	if (pProvSession)
		providerSessionDestroy(pProvThread, pProvSession);
}

RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer)
{
	RsslRet ret = RSSL_RET_SUCCESS; RsslMsg msg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;
	ProviderThread *pProvThread = (ProviderThread*)pChannelHandler->pUserSpec;
	RsslChannel *pChannel = pChannelInfo->pChannel;
	
	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);
	
	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion, pChannel->minorVersion);

	rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

	ret = rsslDecodeMsg(&dIter, &msg);				
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%d  Size %d \n", ret, pChannel->socketId, pBuffer->length);
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
			pProvThread->stats.firstGenMsgRecvTime = getTimeNano();
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

RTR_C_INLINE void updateLatencyStats(ProviderThread *pProviderThread, TimeValue timeTracker)
{
	TimeValue currentTime;
	TimeValue unitsPerMicro;

	currentTime = getTimeMicro();
	unitsPerMicro = 1;

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
	if((ret = decodePayload(pIter, (RsslMsg*)pGenMsg, pProvThread)) 
			!= RSSL_RET_SUCCESS)
	{
		providerThreadCloseChannel(pProvThread, watchlist->pChannelInfo);
		signal_shutdown = RSSL_TRUE;
		RSSL_THREAD_RETURN();

	}
	return ret;
}
