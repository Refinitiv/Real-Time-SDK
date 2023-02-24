/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
*/

#include "niProvPerfConfig.h"
#include "NIProvPerf.h"
#include "directoryProvider.h"
#include "channelHandler.h"
#include "rtr/rsslGetTime.h"
#include "statistics.h"
#include "testUtils.h"
#include "rtr/rsslRDMLoginMsg.h"
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

#ifdef __cplusplus
extern "C" {
	static void signal_handler(int sig);
}
#endif

static RsslBool signal_shutdown = RSSL_FALSE;
static RsslBool testFailed = RSSL_FALSE;
static RsslTimeValue rsslProviderRuntime = 0;

static RsslBuffer applicationName = { 10, (char*)"NIProvPerf" };

static Provider provider;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

static ValueStatistics intervalMsgEncodingStats;

RsslBool showTransportDetails = RSSL_FALSE;

static RsslInt64 nsecPerTick;

RsslBool reactorConnectCount; // Used for when application uses VA Reactor instead of ETA Channel.

static void signal_handler(int sig)
{
	signal_shutdown = RSSL_TRUE;
}

extern void startProviderThreads(Provider *pProvider, RSSL_THREAD_DECLARE(threadFunction,pArg));

/* Sends a basic Login Request on the given streamID, using the RDM package. */
RsslRet sendLoginRequest(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, RsslInt32 streamId, RsslError *error)
{
	RsslRDMLoginRequest loginRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslErrorInfo errorInfo;
	RsslEncodeIterator eIter;
	RsslChannelInfo chanInfo;
	RsslChannel *pChannel = pChannelInfo->pChannel;

	if ((ret = rsslGetChannelInfo(pChannel, &chanInfo, error)) != RSSL_RET_SUCCESS)
	{
		printf("rsslGetChannelInfo() failed: %d(%s)\n", ret, error->text);
		return RSSL_RET_FAILURE;
	}

	/* Send Login Request */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed: %d\n", ret);
		return RSSL_RET_FAILURE;
	}

	if (strlen(niProvPerfConfig.username))
	{
		loginRequest.userName.data = niProvPerfConfig.username;
		loginRequest.userName.length = (RsslUInt32)strlen(niProvPerfConfig.username);
	}

	loginRequest.flags |= RDM_LG_RQF_HAS_ROLE | RDM_LG_RQF_HAS_APPLICATION_NAME;
	loginRequest.role = RDM_LOGIN_ROLE_PROV;
	loginRequest.applicationName = applicationName;

	if (!(msgBuf = rsslGetBuffer(pChannel, chanInfo.maxFragmentSize, RSSL_FALSE, error)))
	{
		printf("rsslGetBuffer() failed: (%d) %s\n", error->rsslErrorId, error->text);
		 return ret;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		 return ret;
	}

	if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRequest, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeRDMLoginMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		 return ret;
	}

	return channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0);

}

RSSL_THREAD_DECLARE(runNIProvChannelConnection, pArg)
{

	ProviderThread *pProviderThread = (ProviderThread*)pArg;
	RsslError error;
	RsslErrorInfo rsslErrorInfo;

	RsslTimeValue nextTickTime;
	RsslInt32 currentTicks = 0;
	RsslConnectOptions copts;

#ifndef NO_ETA_CPU_BIND
	if (pProviderThread->cpuId.length > 0 && pProviderThread->cpuId.data != NULL)
	{
		if (rsslBindThread(pProviderThread->cpuId.data, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %s: %s.\n", pProviderThread->cpuId.data, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}
#endif

	/* Configure connection options. */
	rsslClearConnectOpts(&copts);
	copts.guaranteedOutputBuffers = niProvPerfConfig.guaranteedOutputBuffers;
	copts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	copts.minorVersion = RSSL_RWF_MINOR_VERSION;
	copts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	copts.sysSendBufSize = niProvPerfConfig.sendBufSize;
	copts.sysRecvBufSize = niProvPerfConfig.recvBufSize;
	if (niProvPerfConfig.sAddr || niProvPerfConfig.rAddr)
	{
		if (niProvPerfConfig.connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
		{
			printf("Error: Attempting non-multicast segmented connection.\n");
			exit(-1);
		}

		copts.connectionInfo.segmented.recvAddress = niProvPerfConfig.recvAddr;
		copts.connectionInfo.segmented.recvServiceName = niProvPerfConfig.recvPort;
		copts.connectionInfo.segmented.sendAddress = niProvPerfConfig.sendAddr;
		copts.connectionInfo.segmented.sendServiceName = niProvPerfConfig.sendPort;
		copts.connectionInfo.segmented.interfaceName = niProvPerfConfig.interfaceName;
		copts.connectionInfo.unified.unicastServiceName = niProvPerfConfig.unicastPort;		
		copts.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
	}
	else
	{
		copts.connectionType = niProvPerfConfig.connectionType;
		if (niProvPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && niProvPerfConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
		{
			copts.encryptionOpts.encryptedProtocol = niProvPerfConfig.encryptedConnectionType;

			if (niProvPerfConfig.tlsProtocolFlags != 0)
				copts.encryptionOpts.encryptionProtocolFlags = niProvPerfConfig.tlsProtocolFlags;
			copts.encryptionOpts.openSSLCAStore = niProvPerfConfig.caStore;
		}
		copts.connectionInfo.unified.address = niProvPerfConfig.hostName;
		copts.connectionInfo.unified.serviceName = niProvPerfConfig.portNo;
		copts.connectionInfo.unified.interfaceName = niProvPerfConfig.interfaceName;
		copts.tcp_nodelay = niProvPerfConfig.tcpNoDelay;
	}

	/* Setup connection. */

	do
	{
		ProviderSession *pProvSession;
		RsslChannel *pChannel;
		RsslRet ret;

		if (niProvPerfConfig.sAddr || niProvPerfConfig.rAddr)
			printf("\nAttempting segmented connect to server %s:%s  %s:%s unicastPort %s...\n", 
					niProvPerfConfig.sendAddr, niProvPerfConfig.sendPort, niProvPerfConfig.recvAddr, niProvPerfConfig.recvPort, niProvPerfConfig.unicastPort);
		else
			printf("\nAttempting unified connect to server %s:%s...\n", 
					niProvPerfConfig.hostName, niProvPerfConfig.portNo) ;

		if (!(pChannel = rsslConnect(&copts, &error)))
		{
			printf("rsslConnect() failed: %s(%s)\n", rsslRetCodeToString(error.rsslErrorId),
					error.text);
			SLEEP(1);
			continue;
		}

		if (!(pProvSession = providerSessionCreate(pProviderThread, pChannel)))
		{
			printf("providerSessionInit() failed\n");
			exit(-1);
		}
		
		do
		{
			ret = channelHandlerWaitForChannelInit(&pProviderThread->channelHandler, 
					pProvSession->pChannelInfo, 100000);
		} while (!signal_shutdown && ret == RSSL_RET_CHAN_INIT_IN_PROGRESS);

		if (ret < RSSL_RET_SUCCESS)
		{
			SLEEP(1);
			continue;
		}
		else
			break; /* Successful initialization. */

	} while (!signal_shutdown);



	nextTickTime = rsslGetTimeNano() + nsecPerTick;

	/* this is the main loop */
	while(rtrLikely(!signal_shutdown))
	{
		for (currentTicks = 0; currentTicks < providerThreadConfig.ticksPerSec; ++currentTicks)
		{
			providerThreadRead(pProviderThread, nextTickTime);

			nextTickTime += nsecPerTick;

			providerThreadSendMsgBurst(pProviderThread, nextTickTime);
		}

		providerThreadCheckPings(pProviderThread);

	}

	return RSSL_THREAD_RETURN();
}

/* 
 * Processes events about the state of an RsslReactorChannel.
 */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	ProviderSession *pProvSession = (ProviderSession *)pReactorChannel->userSpecPtr;
	ProviderThread *pProviderThread = pProvSession->pProviderThread;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorChannelInfo reactorChannelInfo;
	RsslUInt32 count;
	RsslRet ret;

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorConnect() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. This will drive the process of setting up the connection
			 * by exchanging the Login messages. 
			 * The application will receive the response messages in the appropriate callback
			 * function we specified. */

            // set the high water mark if configured
            if (niProvPerfConfig.highWaterMark > 0)
            {
				if (rsslReactorChannelIoctl(pReactorChannel, RSSL_HIGH_WATER_MARK, &niProvPerfConfig.highWaterMark, &rsslErrorInfo) != RSSL_RET_SUCCESS)
                {
					printf("rsslReactorChannelIoctl() of RSSL_HIGH_WATER_MARK failed <%s>\n", rsslErrorInfo.rsslError.text);
					exit(-1);
                }
            }

			/* Set file descriptor. */
			FD_SET(pReactorChannel->socketId, &pProviderThread->readfds);
			FD_SET(pReactorChannel->socketId, &pProviderThread->exceptfds);

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

			RSSL_MUTEX_LOCK(&pProviderThread->newClientSessionsLock);
			++pProviderThread->clientSessionsCount;
			pProvSession->pChannelInfo->pChannel = pReactorChannel->pRsslChannel;
			pProvSession->pChannelInfo->pReactorChannel = pReactorChannel;
			pProvSession->pChannelInfo->pReactor = pReactor;
			rsslQueueAddLinkToBack(&pProviderThread->channelHandler.activeChannelList, &pProvSession->pChannelInfo->queueLink);
			pProvSession->timeActivated = rsslGetTimeNano();
			RSSL_MUTEX_LOCK(&pProviderThread->newClientSessionsLock);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			RsslReactorCallbackRet cbRet = RSSL_RC_CRET_SUCCESS;

			if (ret = (printEstimatedMsgSizes(pProviderThread, pProvSession)) != RSSL_RET_SUCCESS)
			{
				printf("printEstimatedMsgSizes() failed: %d\n", ret);
				return RSSL_RC_CRET_SUCCESS;
			}

			if (niProvPerfConfig.itemPublishCount > 0)
			{
				RsslInt32 itemListUniqueIndex;
				RsslInt32 itemListCount;
				RsslInt32 itemListCountRemainder;

				/* If there are multiple connections, determine which items are
					* to be published on this connection.
					* If any items are common to all connections, they are taken from the first
					* items in the item list.  The rest of the list is then divided to provide a unique
					* item list for each connection. */

				/* Determine where items unique to this connection start. */
				itemListUniqueIndex = niProvPerfConfig.commonItemCount;
				itemListUniqueIndex += ((niProvPerfConfig.itemPublishCount - niProvPerfConfig.commonItemCount)
					/ providerThreadConfig.threadCount) * (pProviderThread->providerIndex);

				/* Account for remainder. */
				itemListCount = niProvPerfConfig.itemPublishCount / providerThreadConfig.threadCount;
				itemListCountRemainder = niProvPerfConfig.itemPublishCount % providerThreadConfig.threadCount;

				if (pProviderThread->providerIndex < itemListCountRemainder)
				{
					/* This provider publishes an extra item */
					itemListCount += 1;

					/* Shift index by one for each provider before this one, since they publish extra items too. */
					itemListUniqueIndex += pProviderThread->providerIndex;
				}
				else
					/* Shift index by one for each provider that publishes an extra item. */
					itemListUniqueIndex += itemListCountRemainder;

				if ((ret = providerSessionAddPublishingItems(pProviderThread, pProvSession, 
							niProvPerfConfig.commonItemCount, itemListUniqueIndex, itemListCount - niProvPerfConfig.commonItemCount, 
							(RsslUInt16)directoryConfig.serviceId)) != RSSL_RET_SUCCESS)
				{
					printf("providerSessionAddPublishingItems() failed\n");
					return RSSL_RC_CRET_FAILURE;
				}
				else
				{
					printf("Created publishing list.\n");
				}

				/* send the first burst of refreshes */
				if (rotatingQueueGetCount(&pProvSession->refreshItemList) != 0)
					ret = sendRefreshBurst(pProviderThread, pProvSession);

				if (ret < RSSL_RET_SUCCESS)
					cbRet = RSSL_RC_CRET_FAILURE;
			}

			reactorConnectCount++;

			return cbRet;
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
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			/* The channel has failed and has gone down.  Print the error, close the channel, and reconnect later. */

			printf("Connection down: Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			signal_shutdown = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			printf("Connection down, reconnecting.  Channel fd="SOCKET_PRINT_TYPE"\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &pProviderThread->readfds);
				FD_CLR(pReactorChannel->socketId, &pProviderThread->exceptfds);
			}

			// only allow one connect
			if (reactorConnectCount == providerThreadConfig.threadCount)
			{
				signal_shutdown = RSSL_TRUE;
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("Unknown connection event!\n");
			RSSL_RC_CRET_FAILURE;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

/*
 * Processes the information contained in Login responses.
 * Copies the refresh so we can use it to know what features are
 * supported(for example, whether posting is supported
 * by the provider).
 */
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent)
{
	RsslState *pState = 0;
	RsslBuffer tempBuffer = { 1024, (char*)alloca(1024) };
	ProviderThread *pProviderThread = (ProviderThread *)pReactorChannel->userSpecPtr;
	RsslRDMLoginMsg *pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;
	RsslReactorCallbackRet cbRet = RSSL_RC_CRET_SUCCESS;

	if (!pLoginMsg)
	{
		RsslErrorInfo *pError = pLoginMsgEvent->baseMsgEvent.pErrorInfo;
		printf("loginMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_FAILURE;
	}

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REFRESH:
			pState = &pLoginMsg->refresh.state;

			printf(	"Received login refresh.\n");
			if (pLoginMsg->refresh.flags &
					RDM_LG_RFF_HAS_APPLICATION_NAME)
				printf( "  ApplicationName: %.*s\n",
						pLoginMsg->refresh.applicationName.length,
						pLoginMsg->refresh.applicationName.data);
			printf("\n");

			break;

		case RDM_LG_MT_STATUS:
			printf("Received login status message.\n");
			if (pLoginMsg->status.flags & RDM_LG_STF_HAS_STATE)
				pState = &pLoginMsg->status.state;
			break;

		default:
			printf("Received unhandled login message class: %d.\n", pLoginMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	if (pState && pState->streamState != RSSL_STREAM_OPEN)
	{
		printf("Login stream closed.\n");
		cbRet = RSSL_RC_CRET_FAILURE;
	}

	return cbRet;
}

/*
 * Processes all RSSL messages that aren't processed by 
 * any domain-specific callback functions.  Responses for
 * items requested by the function are handled here. 
 */
RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent)
{
	RsslMsg *pMsg = pMsgEvent->pRsslMsg;

	if (!pMsg)
	{
		/* The message is not present because an error occurred while decoding it.  Print 
		 * the error and close the channel. If desired, the un-decoded message buffer 
		 * is available in pMsgEvent->pRsslMsgBuffer. */

		RsslErrorInfo *pError = pMsgEvent->pErrorInfo;
		printf("defaultMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_FAILURE;
	}

	printf("Received message with unhandled domain: %d\n", pMsg->msgBase.domainType);

	return RSSL_RC_CRET_SUCCESS;
}

RSSL_THREAD_DECLARE(runNIProvReactorConnection, pArg)
{
	ProviderThread *pProviderThread = (ProviderThread*)pArg;
	ProviderSession *pProvSession;

	RsslTimeValue nextTickTime;
	RsslInt32 currentTicks = 0;
	RsslCreateReactorOptions reactorOpts;
	RsslReactorConnectOptions cOpts;
	RsslReactorConnectInfo cInfo;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorDispatchOptions dispatchOptions;
	RsslRet ret = 0;

	FD_ZERO(&pProviderThread->readfds);
	FD_ZERO(&pProviderThread->exceptfds);
	FD_ZERO(&pProviderThread->wrtfds);

	rsslClearCreateReactorOptions(&reactorOpts);
	rsslClearReactorConnectOptions(&cOpts);
	rsslClearReactorConnectInfo(&cInfo);

	/* The Cpu core id for the internal Reactor worker thread. */
	reactorOpts.cpuBindWorkerThread = pProviderThread->cpuReactorWorkerId;

	/* Create an RsslReactor which will manage our channels. */
	if (!(pProviderThread->pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

#ifndef NO_ETA_CPU_BIND
	// Cpu core bind for the NI provider thread thread.
	// The application should invoke rsslBindThread() after rsslInitialize() has invoked.
	// rsslInitialize analyzed Cpu Topology.
	if (pProviderThread->cpuId.length > 0 && pProviderThread->cpuId.data != NULL)
	{
		if (rsslBindThread(pProviderThread->cpuId.data, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %s: %s\n", pProviderThread->cpuId.data, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}
#endif

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors 
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pProviderThread->pReactor->eventFd, &pProviderThread->readfds);

	/* Configure connection options. */
	cInfo.rsslConnectOptions.guaranteedOutputBuffers = niProvPerfConfig.guaranteedOutputBuffers;
	cInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	cInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	cInfo.rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	cInfo.rsslConnectOptions.sysSendBufSize = niProvPerfConfig.sendBufSize;
	cInfo.rsslConnectOptions.sysRecvBufSize = niProvPerfConfig.recvBufSize;
	if (niProvPerfConfig.sAddr || niProvPerfConfig.rAddr)
	{
		if (niProvPerfConfig.connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
		{
			printf("Error: Attempting non-multicast segmented connection.\n");
			exit(-1);
		}

		cInfo.rsslConnectOptions.connectionInfo.segmented.recvAddress = niProvPerfConfig.recvAddr;
		cInfo.rsslConnectOptions.connectionInfo.segmented.recvServiceName = niProvPerfConfig.recvPort;
		cInfo.rsslConnectOptions.connectionInfo.segmented.sendAddress = niProvPerfConfig.sendAddr;
		cInfo.rsslConnectOptions.connectionInfo.segmented.sendServiceName = niProvPerfConfig.sendPort;
		cInfo.rsslConnectOptions.connectionInfo.segmented.interfaceName = niProvPerfConfig.interfaceName;
		cInfo.rsslConnectOptions.connectionInfo.unified.unicastServiceName = niProvPerfConfig.unicastPort;		
		cInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
	}
	else
	{
		cInfo.rsslConnectOptions.connectionType = niProvPerfConfig.connectionType;
		if (niProvPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && niProvPerfConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
		{
			cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = niProvPerfConfig.encryptedConnectionType;

			if (niProvPerfConfig.tlsProtocolFlags != 0)
				cInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = niProvPerfConfig.tlsProtocolFlags;
			cInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = niProvPerfConfig.caStore;
		}
		cInfo.rsslConnectOptions.connectionInfo.unified.address = niProvPerfConfig.hostName;
		cInfo.rsslConnectOptions.connectionInfo.unified.serviceName = niProvPerfConfig.portNo;
		cInfo.rsslConnectOptions.connectionInfo.unified.interfaceName = niProvPerfConfig.interfaceName;
		cInfo.rsslConnectOptions.tcp_nodelay = niProvPerfConfig.tcpNoDelay;
	}

	// set NIProvider role information
	rsslClearOMMNIProviderRole(&pProviderThread->niProviderRole);
	pProviderThread->niProviderRole.base.channelEventCallback = channelEventCallback;
	pProviderThread->niProviderRole.base.defaultMsgCallback = defaultMsgCallback;
	pProviderThread->niProviderRole.loginMsgCallback = loginMsgCallback;

	/* Initialize the default login request(Use 1 as the Login Stream ID). */
	if (rsslInitDefaultRDMLoginRequest(&pProviderThread->loginRequest, 1) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		exit(-1);
	}
	pProviderThread->loginRequest.flags |= RDM_LG_RQF_HAS_ROLE;
	pProviderThread->loginRequest.role = RDM_LOGIN_ROLE_PROV;
	/* If a username was specified, change username on login request. */
	if (strlen(niProvPerfConfig.username) > 0)
	{
		pProviderThread->loginRequest.userName.data = niProvPerfConfig.username;
		pProviderThread->loginRequest.userName.length = (rtrUInt32)strlen(niProvPerfConfig.username);
	}
	pProviderThread->niProviderRole.pLoginRequest = &pProviderThread->loginRequest;

	/* Initialize the default directory refresh(Use -1 as the Directory Stream ID). */
	rsslClearRDMDirectoryRefresh(&pProviderThread->directoryRefresh);
	pProviderThread->directoryRefresh.flags = RDM_DR_RFF_HAS_SERVICE_ID | RDM_DR_RFF_CLEAR_CACHE;
	pProviderThread->directoryRefresh.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	pProviderThread->directoryRefresh.rdmMsgBase.streamId = -1;

	pProviderThread->directoryRefresh.serviceList = &service;
	pProviderThread->directoryRefresh.serviceCount = 1;
	
	pProviderThread->niProviderRole.pDirectoryRefresh = &pProviderThread->directoryRefresh;

	// connect via Reactor
	if (niProvPerfConfig.sAddr || niProvPerfConfig.rAddr)
		printf("\nAttempting segmented connect to server %s:%s  %s:%s unicastPort %s...\n", 
				niProvPerfConfig.sendAddr, niProvPerfConfig.sendPort, niProvPerfConfig.recvAddr, niProvPerfConfig.recvPort, niProvPerfConfig.unicastPort);
	else
		printf("\nAttempting unified connect to server %s:%s...\n", 
				niProvPerfConfig.hostName, niProvPerfConfig.portNo) ;

	// create provider session here and link to provider thread
	if (!(pProvSession = providerSessionCreate(pProviderThread, NULL)))
	{
		printf("providerSessionInit() failed\n");
		exit(-1);
	}
	pProvSession->pProviderThread = pProviderThread;

	cOpts.reconnectAttemptLimit = -1;
	cOpts.reconnectMaxDelay = 5000;
	cOpts.reconnectMinDelay = 1000;
	cInfo.rsslConnectOptions.userSpecPtr = pProvSession;
	cOpts.reactorConnectionList = &cInfo;
	cOpts.connectionCount = 1;

    if ((ret = rsslReactorConnect(pProviderThread->pReactor, &cOpts, (RsslReactorChannelRole *)&pProviderThread->niProviderRole, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
    {
		printf("rsslReactorConnect failed with return code: %d error = %s", ret,  rsslErrorInfo.rsslError.text);
		exit(-1);
    }		

	nextTickTime = rsslGetTimeNano() + nsecPerTick;

	rsslClearReactorDispatchOptions(&dispatchOptions);

	/* this is the main loop */
	while(rtrLikely(!signal_shutdown))
	{
		RsslTimeValue currentTime;

		for (currentTicks = 0; currentTicks < providerThreadConfig.ticksPerSec; ++currentTicks)
		{
			/* Loop on select(), looking for channels with available data, until stopTimeNsec is reached. */
			do
			{
				int selRet;
				struct timeval time_interval;
				fd_set useRead;
				fd_set useExcept;
				fd_set useWrt;
#ifdef WIN32
				/* Windows does not allow select() to be called with empty file descriptor sets. */
				if (pProviderThread->readfds.fd_count == 0)
				{
					currentTime = rsslGetTimeNano();
					selRet = 0;
					Sleep((DWORD)((currentTime < nextTickTime) ? (nextTickTime - currentTime)/1000000 : 0));
				}
				else
#endif
				{
					useRead = pProviderThread->readfds;
					useWrt = pProviderThread->wrtfds;
					useExcept = pProviderThread->exceptfds;

					currentTime = rsslGetTimeNano();
					time_interval.tv_usec = (long)((currentTime < nextTickTime) ? (nextTickTime - currentTime)/1000 : 0);
					time_interval.tv_sec = 0;

					selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
				}

				if (selRet == 0)
					continue;
				else if (selRet > 0)
				{
					while ((ret = rsslReactorDispatch(pProviderThread->pReactor, &dispatchOptions, &rsslErrorInfo)) > RSSL_RET_SUCCESS) {}
					if (ret < RSSL_RET_SUCCESS)
					{
						printf("rsslReactorDispatch failed with return code: %d error = %s", ret,  rsslErrorInfo.rsslError.text);
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

			if (pProvSession->pChannelInfo &&
				pProvSession->pChannelInfo->pReactorChannel &&
				pProvSession->pChannelInfo->pReactorChannel->pRsslChannel &&
				pProvSession->pChannelInfo->pReactorChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				providerThreadSendMsgBurst(pProviderThread, nextTickTime);
			}
		}
	}

	return RSSL_THREAD_RETURN();
}

int main(int argc, char **argv)
{
	RsslError error;
	RsslUInt32 intervalSeconds = 0, currentRuntimeSec = 0;

	/* Read in configuration and echo it. */
	initNIProvPerfConfig(argc, argv);
	printNIProvPerfConfig(stdout);

	if (!(summaryFile = fopen(niProvPerfConfig.summaryFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", niProvPerfConfig.summaryFilename);
		exit(-1);
	}

	printNIProvPerfConfig(summaryFile); fflush(summaryFile);

	providerInit(&provider, PROVIDER_NONINTERACTIVE,
			processActiveChannel,
			processInactiveChannel,
			processMsg, NULL);

	// set up a signal handler so we can cleanup before exit
	signal(SIGINT, signal_handler);

	/* Determine update rates on per-tick basis */
	nsecPerTick = 1000000000LL/(RsslInt64)providerThreadConfig.ticksPerSec;

	xmlInitParser();

	/* Initialize RSSL */
	if (niProvPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
	{
		if (rsslInitialize(providerThreadConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
		{
			printf("rsslInitialize() failed.\n");
			exit(-1);
		}
	}

	/* Initialize runtime timer */
	rsslProviderRuntime = rsslGetTimeNano() + ((RsslInt64)niProvPerfConfig.runTime * 1000000000LL);

	if (niProvPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
	{
		startProviderThreads(&provider, runNIProvChannelConnection);
	}
	else // use ETA VA Reactor
	{
		reactorConnectCount = 0;
		startProviderThreads(&provider, runNIProvReactorConnection);
	}

	/* this is the main loop */
	while(!signal_shutdown)
	{
		SLEEP(1);
		++currentRuntimeSec;
		++intervalSeconds;

		if (intervalSeconds == niProvPerfConfig.writeStatsInterval)
		{
			providerCollectStats(&provider, RSSL_TRUE, niProvPerfConfig.displayStats, 
					currentRuntimeSec, niProvPerfConfig.writeStatsInterval);
			intervalSeconds = 0;
		}
		
		/* Handle runtime. */
		if (rsslGetTimeNano() >= rsslProviderRuntime)
		{
			printf("\nRun time of %u seconds has expired.\n\n", niProvPerfConfig.runTime);
			signal_shutdown = RSSL_TRUE; /* Tell other threads to shutdown. */
		}
	}

	cleanUpAndExit();
}

RsslRet processMsg(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslBuffer* pBuffer)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;
	RsslChannel *chnl = pChannelInfo->pChannel;
	ProviderThread *pProviderThread = (ProviderThread*)pChannelHandler->pUserSpec;
	
	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);
	
	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, chnl->majorVersion, chnl->minorVersion);

	rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

	ret = rsslDecodeMsg(&dIter, &msg);				
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", ret, chnl->socketId, pBuffer->length);
		cleanUpAndExit();
	}

	switch ( msg.msgBase.domainType )
	{
		case RSSL_DMT_LOGIN:
		{
			RsslRDMLoginMsg loginMsg;
			char memoryChar[1024];
			RsslBuffer memoryBuffer = { sizeof(memoryChar), memoryChar };
			RsslErrorInfo errorInfo;
			RsslState *pState = NULL;

			if ((ret = rsslDecodeRDMLoginMsg(&dIter, &msg, &loginMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeRDMLoginMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
				ret = RSSL_RET_FAILURE;
				break;
			}

			switch(loginMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_LG_MT_REFRESH:
					pState = &loginMsg.refresh.state;

					printf(	"Received login refresh.\n");
					if (loginMsg.refresh.flags &
							RDM_LG_RFF_HAS_APPLICATION_NAME)
						printf( "  ApplicationName: %.*s\n",
								loginMsg.refresh.applicationName.length,
								loginMsg.refresh.applicationName.data);
					printf("\n");

					break;

				case RDM_LG_MT_STATUS:
					printf("Received login status message.\n");
					if (loginMsg.status.flags & RDM_LG_STF_HAS_STATE)
						pState = &loginMsg.status.state;
					break;

				default:
					printf("Received unhandled login message class: %d.\n", msg.msgBase.msgClass);
					break;
			}

			if (pState)
			{
				if (pState->streamState == RSSL_STREAM_OPEN)
				{
					ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;

					ret = publishDirectoryRefresh(pChannelHandler, pChannelInfo, -1);

					if (ret < RSSL_RET_SUCCESS)
					{
						printf("publishDirectoryRefresh() failed: %d.\n", ret);
						return ret;
					}

					if (niProvPerfConfig.itemPublishCount > 0)
					{
						RsslInt32 itemListUniqueIndex;
						RsslInt32 itemListCount;
						RsslInt32 itemListCountRemainder;

						/* If there are multiple connections, determine which items are
						 * to be published on this connection.
						 * If any items are common to all connections, they are taken from the first
						 * items in the item list.  The rest of the list is then divided to provide a unique
						 * item list for each connection. */

						/* Determine where items unique to this connection start. */
						itemListUniqueIndex = niProvPerfConfig.commonItemCount;
						itemListUniqueIndex += ((niProvPerfConfig.itemPublishCount - niProvPerfConfig.commonItemCount)
							/ providerThreadConfig.threadCount) * (pProviderThread->providerIndex);

						/* Account for remainder. */
						itemListCount = niProvPerfConfig.itemPublishCount / providerThreadConfig.threadCount;
						itemListCountRemainder = niProvPerfConfig.itemPublishCount % providerThreadConfig.threadCount;

						if (pProviderThread->providerIndex < itemListCountRemainder)
						{
							/* This provider publishes an extra item */
							itemListCount += 1;

							/* Shift index by one for each provider before this one, since they publish extra items too. */
							itemListUniqueIndex += pProviderThread->providerIndex;
						}
						else
							/* Shift index by one for each provider that publishes an extra item. */
							itemListUniqueIndex += itemListCountRemainder;

						if ((ret = providerSessionAddPublishingItems(pProviderThread, (ProviderSession*)pChannelInfo->pUserSpec, 
									niProvPerfConfig.commonItemCount, itemListUniqueIndex, itemListCount - niProvPerfConfig.commonItemCount, 
									(RsslUInt16)directoryConfig.serviceId)) != RSSL_RET_SUCCESS)
						{
							printf("providerSessionAddPublishingItems() failed\n");
							return ret;
						}
						else
						{
							printf("Created publishing list.\n");
							ret = RSSL_RET_SUCCESS;
						}

						/* send the first burst of refreshes */
						if (rotatingQueueGetCount(&pProvSession->refreshItemList) != 0)
							ret = sendRefreshBurst(pProviderThread, pProvSession);
						else
							ret = RSSL_RET_SUCCESS;

						if (ret < RSSL_RET_SUCCESS)
							return ret;
					}

					if (ret > RSSL_RET_SUCCESS)
					{
						/* Need to flush */
						providerThreadRequestChannelFlush(pProviderThread, pChannelInfo);
					}

				}
				else
				{
					printf("Login stream closed.\n");
					ret = RSSL_RET_FAILURE;
				}
			}
			else 
				ret = RSSL_RET_SUCCESS;
			break;
		}
		default:
			printf("Received message with unhandled domain: %d\n", msg.msgBase.domainType);
			break;
	}


	if (ret < RSSL_RET_SUCCESS) 
	{
		signal_shutdown = RSSL_TRUE;
		return ret;
	}
	else
	{
		/* Typically these requests result in a response, so call for a flush to make sure it gets out.*/
		providerThreadRequestChannelFlush(pProviderThread, pChannelInfo);
		return ret;
	}
}

RsslRet processActiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo)
{
	ProviderThread *pProviderThread = (ProviderThread*)pChanHandler->pUserSpec;
	ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;
	RsslRet ret;
	RsslError error;
	RsslChannelInfo channelInfo;
	RsslUInt32 count;

	if (niProvPerfConfig.highWaterMark > 0)
	{
		if (rsslIoctl(pChannelInfo->pChannel, RSSL_HIGH_WATER_MARK, &niProvPerfConfig.highWaterMark, &error) != RSSL_RET_SUCCESS)
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

	if (ret = (printEstimatedMsgSizes(pProviderThread, pProvSession)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	pProvSession->timeActivated = rsslGetTimeNano();

	/* Send Login Request */
	ret = sendLoginRequest(pChanHandler, pChannelInfo, 1, &error);
	if (ret < RSSL_RET_SUCCESS)
	{
		printf("sendLoginRequest() failed: %d\n", ret);
		exit(-1);
	}
	else if (ret > RSSL_RET_SUCCESS)
	{
		/* Need to flush */
		providerThreadRequestChannelFlush(pProviderThread, pChannelInfo);
	}

	return RSSL_RET_SUCCESS;
}

void processInactiveChannel(ChannelHandler *pChanHandler, ChannelInfo *pChannelInfo, RsslError *pError)
{
	ProviderThread *pProviderThread = (ProviderThread*)pChanHandler->pUserSpec;
	ProviderSession *pProvSession = (ProviderSession*)pChannelInfo->pUserSpec;

	/* If the channel was active, then the test was started so we won't attempt to reconnect,
	 * so stop the test. */
	if (!signal_shutdown && pProvSession->timeActivated)
	{
		signal_shutdown = RSSL_TRUE;
		testFailed = RSSL_TRUE;
	}

	if(pError)
		printf("Channel Closed: %s(%s)\n", rsslRetCodeToString(pError->rsslErrorId), pError->text);
	else
		printf("Channel Closed.\n");

	if (pProvSession)
	{
		providerSessionDestroy(pProviderThread, pProvSession);
		pChannelInfo->pUserSpec = NULL;
	}
}

void cleanUpAndExit()
{
	providerWaitForThreads(&provider);
	providerCollectStats(&provider, RSSL_FALSE, RSSL_FALSE, 0, 0);
	providerPrintSummaryStats(&provider, stdout);
	providerPrintSummaryStats(&provider, summaryFile);

	if (testFailed)
	{
		fprintf(stdout, "TEST FAILED. An error occurred during this test.\n");
		fprintf(summaryFile, "TEST FAILED. An error occurred during this test.\n");
	}

	fclose(summaryFile);

	providerCleanup(&provider);

	providerThreadConfigCleanup();

	if (niProvPerfConfig.useReactor == RSSL_FALSE) // use ETA Channel
	{
		rsslUninitialize();
	}

	xmlCleanupParser();
	printf("Exiting.\n");
	exit(0);
}
