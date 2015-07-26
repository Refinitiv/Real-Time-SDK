/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "niProvPerfConfig.h"
#include "upacNIProvPerf.h"
#include "directoryProvider.h"
#include "channelHandler.h"
#include "getTime.h"
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

#ifdef __cplusplus
extern "C" {
	static void signal_handler(int sig);
}
#endif

static RsslBool signal_shutdown = RSSL_FALSE;
static RsslBool testFailed = RSSL_FALSE;
static TimeValue rsslProviderRuntime = 0;

static RsslBuffer applicationName = { 14, (char*)"upacNIProvPerf" };


static Provider provider;

/* Logs summary information, such as application inputs and final statistics. */
static FILE *summaryFile = NULL;

RsslBool showTransportDetails = RSSL_FALSE;

static RsslInt64 nsecPerTick;

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

RSSL_THREAD_DECLARE(runNIProvConnection, pArg)
{

	ProviderThread *pProviderThread = (ProviderThread*)pArg;
	RsslError error;

	TimeValue nextTickTime;
	RsslInt32 currentTicks = 0;
	RsslConnectOptions copts;

	if (pProviderThread->cpuId >= 0)
	{
		if (bindThread(pProviderThread->cpuId) != RSSL_RET_SUCCESS)
		{
			printf("Error: Failed to bind thread to core %d.\n", pProviderThread->cpuId);
			exit(-1);
		}
	}

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
		copts.connectionInfo.unified.address = niProvPerfConfig.hostName;
		copts.connectionInfo.unified.serviceName = niProvPerfConfig.portNo;
		copts.connectionInfo.unified.interfaceName = niProvPerfConfig.interfaceName;
		copts.tcp_nodelay = niProvPerfConfig.tcpNoDelay;
		copts.connectionType = RSSL_CONN_TYPE_SOCKET;
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



	nextTickTime = getTimeNano() + nsecPerTick;

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
			processMsg);

	// set up a signal handler so we can cleanup before exit
	signal(SIGINT, signal_handler);

	/* Determine update rates on per-tick basis */
	nsecPerTick = 1000000000LL/(RsslInt64)providerThreadConfig.ticksPerSec;

	xmlInitParser();

	/* Initialize RSSL */
	if (rsslInitialize(providerThreadConfig.threadCount > 1 ? RSSL_LOCK_GLOBAL : RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize() failed.\n");
		exit(-1);
	}

	/* Initialize runtime timer */
	rsslProviderRuntime = getTimeNano() + ((RsslInt64)niProvPerfConfig.runTime * 1000000000LL);

	startProviderThreads(&provider, runNIProvConnection);

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
		if (getTimeNano() >= rsslProviderRuntime)
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
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%d  Size %d \n", ret, chnl->socketId, pBuffer->length);
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

	if (ret = (printEstimatedMsgSizes(pProviderThread, pProvSession)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	pProvSession->timeActivated = getTimeNano();

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

	rsslUninitialize();
	xmlCleanupParser();
	printf("Exiting.\n");
	exit(0);
}
