/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* This Elektron Direct Feed (EDF) example demonstrates consuming market data
 * from the feed, requesting images from the snapshot server and processing
 * updates on items of interest as they are received from the realtime feed. 
 *
 * This application provides an example mechanism for synchronizing messages
 * received from the snapshot server and realtime feed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#include <sys/types.h>
#include <sys/timeb.h>

#include "edfExampleConfig.h"
#include "realTimeSession.h"
#include "gapFillSession.h"
#include "snapshotSession.h"
#include "gapRequestSession.h"
#include "refDataSession.h"
#include "itemList.h"

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"




/* Time at which application will exit. */
static time_t endTime;

/* Data dictionary. */
extern RsslDataDictionary dictionary;

/* Dictionary file names. */
static const char *fieldDictionaryFileName = "RDMFieldDictionary";
static const char *enumTableFileName = "enumtype.def";

extern RsslBool	fieldDictionaryLoaded;
extern RsslBool	enumDictionaryLoaded;
extern RsslBool	globalSetDefDictionaryLoaded;

/* Session handling the realtime feed channel. */
static RealTimeSession* pRealTimeFeedSession;

/* Session handling the gap fill channel. */
static GapFillSession* pGapFillSession;

/* Session handling the ref data channel. */
static RefDataSession refDataSession;

/* Session handling the snapshot server channel. */
static SnapshotSession snapshotServerSession;

/* Session handling the gap request server channel. */
static GapRequestSession gapRequestServerSession;

static RsslUInt32 last_sequence_requested;

/* Initializes the EDF example. */
static void initialize();

static void initializeSnapshot();

/* Initialize connection to the reference data server */
static void initializeRefData();

/* setup XML tracing */
static void setupXMLTracing(RsslChannel* channel, char* fileName);

/* Loads the field/enumType dictionaries from files.  */
static void loadDictionary();

int main(int argc, char **argv)
{
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	int selRet;	
	RsslRDMService* pService = 0;
	RsslLibraryVersionInfo libVer = RSSL_INIT_LIBRARY_VERSION_INFO;
	
	RsslRet ret;
	time_t currentTime;
	time_t tmpTime;
	RsslError rsslError;
        gap_info_t gapInfo;
	RsslBool allConnectionsInitialized = RSSL_FALSE;
	RsslBool snapshotInitialized = RSSL_FALSE;

	time(&currentTime);
	tmpTime = currentTime;

	pRealTimeFeedSession = 0;
	pGapFillSession = 0;
	
	memset(&gapInfo, 0, sizeof(gap_info_t));

	// handle user input
	edfExampleConfigInit(argc, argv);

	// print out RSSL transport library version
	rsslQueryTransportLibraryVersion(&libVer);
	printf("Transport Library Version:\n \t%s\n \t%s\n \t%s\n\n", libVer.productVersion, libVer.internalVersion, libVer.productDate);

	/* initialize RSSL transport */
	if (rsslInitialize(RSSL_LOCK_NONE, &rsslError) < RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n",rsslError.text);
		exit(RSSL_RET_FAILURE);
	}

	/* initialize EDF example */
	initializeRefData(); 


	// main processing loop
	// handles processing of all requests and responses via select call
	do
	{
		struct timeval timeout;

		unsigned int j;
		

		FD_ZERO(&useRead);
		FD_ZERO(&useWrt);
		FD_ZERO(&useExcept);

		/* Set notification for the ref data server channel. */
		if (refDataSession.pRsslChannel)
		{
			RsslInt32 socketId = refDataSession.pRsslChannel->socketId;

			FD_SET(socketId, &useRead);
			FD_SET(socketId, &useExcept);

			if (refDataSessionNeedsWriteNotification(&refDataSession))
				FD_SET(socketId, &useWrt);

			if (refDataSession.state == REF_DATA_STATE_READY && snapshotInitialized == RSSL_FALSE)
			{
				printf("\nReceived config info and global set definitions from Ref Data Server.\n\n");
				initializeSnapshot();
				pService = refDataSession.pService;
				snapshotInitialized = RSSL_TRUE;
			}
		}

		/* Set notification for the snapshot server channel. */
		if (snapshotServerSession.pRsslChannel)
		{
			RsslInt32 socketId = snapshotServerSession.pRsslChannel->socketId;

			FD_SET(socketId, &useRead);
			FD_SET(socketId, &useExcept);

			if (snapshotSessionNeedsWriteNotification(&snapshotServerSession))
				FD_SET(socketId, &useWrt);

			if (snapshotServerSession.state == SNAPSHOT_STATE_SYMBOL_LIST_RECEIVED && allConnectionsInitialized == RSSL_FALSE)
			{
				printf("\nReceived symbol list from Snapshot Server.\n\n");
				initialize();
				allConnectionsInitialized = RSSL_TRUE;
			}
		}

		/* Set notification for the gap request server channel. */
		if (gapRequestServerSession.pRsslChannel)
		{
			RsslInt32 socketId = gapRequestServerSession.pRsslChannel->socketId;

			FD_SET(socketId, &useRead);
			FD_SET(socketId, &useExcept);

			if (gapRequestSessionNeedsWriteNotification(&gapRequestServerSession))
				FD_SET(socketId, &useWrt);
		}

		if (pService)
		{
			/* Set notification for the gap fill feed channel. */
			for (j = 0; (j < pService->seqMCastInfo.GapMCastChanServerCount && pGapFillSession[j].connected); j++)
			{
				RsslInt32 socketId = pGapFillSession[j].pRsslChannel->socketId;

				FD_SET(socketId, &useRead);
				FD_SET(socketId, &useExcept);
			}

			/* Set notification for the realtime feed channel. */
			for (j = 0; (j < pService->seqMCastInfo.StreamingMCastChanServerCount && pRealTimeFeedSession[j].connected); j++)
			{
				RsslInt32 socketId = pRealTimeFeedSession[j].pRsslChannel->socketId;

				FD_SET(socketId, &useRead);
				FD_SET(socketId, &useExcept);
			}
		}

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &timeout);

		if (selRet > 0)
		{
			/* Read from the ref data server. */
			if (refDataSession.pRsslChannel &&
					FD_ISSET(refDataSession.pRsslChannel->socketId, &useRead))
			{			

				do
				{
					ret = refDataSessionProcessReadReady(&refDataSession);
				} while (ret > RSSL_RET_SUCCESS);

				if (ret < RSSL_RET_SUCCESS)
					exit(-1);
			}

			/* Read from the realtime feed. */
			if (pService)
			{
				for (j = 0; (j < pService->seqMCastInfo.StreamingMCastChanServerCount &&  pRealTimeFeedSession[j].connected); j++)
				{
					if (FD_ISSET(pRealTimeFeedSession[j].pRsslChannel->socketId, &useRead))
					{
						do
						{
							ret = realTimeSessionProcessReadReady(&pRealTimeFeedSession[j]);

							/* If gap was detected in the real time data send a gap request to the gap server */
							if ( pRealTimeFeedSession[j].gapDetected )
							{
								// set the address and port
								snprintf(&gapInfo.address[0], 128, "%s", pRealTimeFeedSession[j].gapInfo.address);
								gapInfo.port =  pRealTimeFeedSession[j].gapInfo.port;

								// create and send gap fill request
								gapRequestSessionRequestFill(&gapRequestServerSession, &pRealTimeFeedSession[j].gapInfo);
								pRealTimeFeedSession[j].gapDetected = RSSL_FALSE;
								pRealTimeFeedSession[j].gapInfo.start = pRealTimeFeedSession[j].gapInfo.end+1;
							}

						} while (ret > RSSL_RET_SUCCESS);

						if (ret < RSSL_RET_SUCCESS)
							exit(-1);
					}
				}

				/* Send a gap request every 5 seconds if there are no real gaps detected. */
				if (!(currentTime % 5) && tmpTime != currentTime)
				{
					// for demonstration purposes only use the first real time session only
					if (last_sequence_requested != pRealTimeFeedSession[0].gapInfo.start)
					{
						gapInfo.start = pRealTimeFeedSession[0].gapInfo.start;
						gapInfo.end = pRealTimeFeedSession[0].gapInfo.start;
						
						snprintf(&gapInfo.address[0], 128, "%s", pRealTimeFeedSession[0].gapInfo.address);
						gapInfo.port =  pRealTimeFeedSession[0].gapInfo.port;
							
						last_sequence_requested = (RsslUInt32)pRealTimeFeedSession[0].gapInfo.start;
						gapRequestSessionRequestFill(&gapRequestServerSession, &gapInfo);
						pRealTimeFeedSession[0].gapInfo.start = pRealTimeFeedSession[0].gapInfo.end+1;
					}
					tmpTime = currentTime;
				}

				/* Read from the gap fill feed. */
				for (j = 0; (j < pService->seqMCastInfo.GapMCastChanServerCount && pGapFillSession[j].connected); j++)
				{
					if (FD_ISSET(pGapFillSession[j].pRsslChannel->socketId, &useRead))
					{
						do
						{
							ret = gapFillSessionProcessReadReady(&pGapFillSession[j]);
						} while (ret > RSSL_RET_SUCCESS);

						if (ret < RSSL_RET_SUCCESS)
							exit(-1);
					}
				}

			}

			/* Read from the snapshot server. */
			if (snapshotServerSession.pRsslChannel &&
					FD_ISSET(snapshotServerSession.pRsslChannel->socketId, &useRead))
			{
	
				do
				{
					ret = snapshotSessionProcessReadReady(&snapshotServerSession);
				} while (ret > RSSL_RET_SUCCESS);

				if (ret < RSSL_RET_SUCCESS)
					exit(-1);
			}

			/* Read from the gap request server. */
			if (gapRequestServerSession.pRsslChannel &&
					FD_ISSET(gapRequestServerSession.pRsslChannel->socketId, &useRead))
			{

				do
				{
					ret = gapRequestSessionProcessReadReady(&gapRequestServerSession);
				} while (ret > RSSL_RET_SUCCESS);

				if (ret < RSSL_RET_SUCCESS)
					exit(-1);
			}

			/* Read from the ref data server. */
			if (refDataSession.pRsslChannel &&
					FD_ISSET(refDataSession.pRsslChannel->socketId, &useRead))
			{

				do
				{
					ret = refDataSessionProcessReadReady(&refDataSession);
				} while (ret > RSSL_RET_SUCCESS);

				if (ret < RSSL_RET_SUCCESS)
					exit(-1);
			}

			/* Check if we need to flush the snapshot server. */
			if (snapshotServerSession.pRsslChannel &&
					FD_ISSET(snapshotServerSession.pRsslChannel->socketId, &useWrt))
			{
				if (snapshotSessionProcessWriteReady(&snapshotServerSession) != RSSL_RET_SUCCESS)
					exit(-1);
			}


			/* Check if we need to flush the gap request server. */
			if (gapRequestServerSession.pRsslChannel &&
					FD_ISSET(gapRequestServerSession.pRsslChannel->socketId, &useWrt))
			{
				if (gapRequestSessionProcessWriteReady(&gapRequestServerSession) != RSSL_RET_SUCCESS)
					exit(-1);
			}

			/* Check if we need to flush the ref data server. */
			if (refDataSession.pRsslChannel &&
					FD_ISSET(refDataSession.pRsslChannel->socketId, &useWrt))
			{
				if (refDataSessionProcessWriteReady(&refDataSession) != RSSL_RET_SUCCESS)
					exit(-1);
			}
		}

		time(&currentTime);
	} while (currentTime < endTime);

	printf("Runtime of %u expired. Exiting.\n\n", exampleConfig.runTime);

	/* Cleanup memory allocated for ref data server session. */
	refDataSessionCleanup(&refDataSession);

	/* Make sure dictionary is cleaned up */
	rsslDeleteDataDictionary(&dictionary);
	rsslClearDataDictionary(&dictionary);

	/* clean up global set def */
	if (globalSetDefDictionaryLoaded)
		rsslDeleteFieldSetDefDb(&globalFieldSetDefDb);

	/* uninitialize RSSL transport */
	rsslUninitialize();

	/* Cleanup memory allocated for items. */
	itemListCleanup(&itemList);

	if (pRealTimeFeedSession)
		free (pRealTimeFeedSession);

	if (pGapFillSession)
		free (pGapFillSession);

	return 0;
}

static void initializeRefData()
{
	RsslConnectOptions copts;
	time(&endTime);
	endTime += exampleConfig.runTime;

	refDataSessionClear(&refDataSession);

	// connect to ref data server
	printf("Connecting to ref data server %s:%s\n", exampleConfig.refDataServerAddr,
			exampleConfig.refDataServerPort);
	rsslClearConnectOpts(&copts);
	copts.connectionType = RSSL_CONN_TYPE_SOCKET;
	copts.connectionInfo.segmented.recvServiceName = exampleConfig.refDataServerPort;
	copts.connectionInfo.segmented.recvAddress = exampleConfig.refDataServerAddr;
	if (refDataSessionConnect(&refDataSession, &copts) != RSSL_RET_SUCCESS)
		exit(-1);

	if (exampleConfig.xmlRefDataTrace)
	{
		setupXMLTracing(refDataSession.pRsslChannel, exampleConfig.refDataTraceOutputFile);
	}


	// load dictionary
	loadDictionary();
}

static void initializeSnapshot()
{
	RsslConnectOptions copts;
	unsigned int i;
	char address[128];
	char port[128];

	RsslRDMService* pService = refDataSession.pService;

	printf("\n\nSetting up other connections\n\n");

	if (pService->flags & RDM_SVCF_HAS_SEQ_MCAST)
	{
		if((pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_SNAPSHOT_SERV) && 
				(pService->seqMCastInfo.snapshotServer.address.length > 0))
		{
			printf("Snapshot Server:\n\t%.*s "RTR_LLU"\n\n", pService->seqMCastInfo.snapshotServer.address.length,
				pService->seqMCastInfo.snapshotServer.address.data,
				pService->seqMCastInfo.snapshotServer.port);
		}	
	}

	snapshotSessionClear(&snapshotServerSession);
	gapRequestSessionClear(&gapRequestServerSession);

	// connect to snapshot server
	rsslClearConnectOpts(&copts);
	copts.connectionType = RSSL_CONN_TYPE_SOCKET;
	if (exampleConfig.snapshotServerPort[0])
	{
		copts.connectionInfo.segmented.recvServiceName = exampleConfig.snapshotServerPort;
	}
	else
	{
		snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.snapshotServer.port);
		copts.connectionInfo.segmented.recvServiceName = &port[0];
	}
	if (exampleConfig.snapshotServerAddr[0])
	{
		copts.connectionInfo.segmented.recvAddress = exampleConfig.snapshotServerAddr;
	}
	else
	{
		snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.snapshotServer.address.length, pService->seqMCastInfo.snapshotServer.address.data);
		copts.connectionInfo.segmented.recvAddress = &address[0];
	}
	printf("Connecting to snapshot server %s:%s\n", copts.connectionInfo.segmented.recvAddress, copts.connectionInfo.segmented.recvServiceName);
	if (snapshotSessionConnect(&snapshotServerSession, &copts, pService) != RSSL_RET_SUCCESS)
		exit(-1);

	// Initialize the memory for Streaming Real Time Sessions
	if (pService->seqMCastInfo.StreamingMCastChanServerCount != 0)
	{
		pRealTimeFeedSession = (RealTimeSession*)malloc(sizeof(RealTimeSession) * pService->seqMCastInfo.StreamingMCastChanServerCount);
		for (i = 0; i < pService->seqMCastInfo.StreamingMCastChanServerCount; i++)
			realTimeSessionClear(&pRealTimeFeedSession[i]);
	}

	if (pService->seqMCastInfo.GapMCastChanServerCount != 0)
	{
		pGapFillSession = (GapFillSession*)malloc(sizeof(GapFillSession) * pService->seqMCastInfo.GapMCastChanServerCount);

		for (i = 0; i < pService->seqMCastInfo.GapMCastChanServerCount; i++)
			gapFillSessionClear(&pGapFillSession[i]);
	}
}

static void initialize()
{
	RsslConnectOptions copts;
	int i;
	unsigned int j;
	RsslBool foundItem;
	char address[128];
	char port[128];

	RsslRDMService* pService = refDataSession.pService;

	itemListClear(&itemList);

	printf("Searching for the items in the symbol list:\n");

	// Match symbol names in the symbol list
	for (j = 0; j < MAX_ITEMS && strlen(exampleConfig.itemList[j].symbolName) > 0; j++)
	{
		foundItem = RSSL_FALSE;	
		for (i = 0; snapshotServerSession.symbolListEntry[i] != 0 && foundItem == RSSL_FALSE; i++)
		{
			if (strcmp(snapshotServerSession.symbolListEntry[i]->name, exampleConfig.itemList[j].symbolName) == 0)
			{
				foundItem = RSSL_TRUE;
				// Copy the real time stream and gap fill connection information
				itemListAdd(&itemList, snapshotServerSession.symbolListEntry[i]->id, exampleConfig.itemList[j].symbolName, exampleConfig.itemList[j].domainType);
				if (snapshotServerSession.symbolListEntry[i]->streamingChannels[0].domain == exampleConfig.itemList[j].domainType)
				{
					exampleConfig.itemList[j].realTimeChannelId = (RsslUInt32)snapshotServerSession.symbolListEntry[i]->streamingChannels[0].channelId;
				}
				else
				{
					exampleConfig.itemList[j].realTimeChannelId = (RsslUInt32)snapshotServerSession.symbolListEntry[i]->streamingChannels[1].channelId;
				}

				if (snapshotServerSession.symbolListEntry[i]->gapChannels[0].domain == exampleConfig.itemList[j].domainType)
				{
					exampleConfig.itemList[j].gapChannelId = (RsslUInt32)snapshotServerSession.symbolListEntry[i]->gapChannels[0].channelId;
				}
				else
				{
					exampleConfig.itemList[j].gapChannelId = (RsslUInt32)snapshotServerSession.symbolListEntry[i]->gapChannels[1].channelId;
				}
				printf("Item %s found in symbol list\n", exampleConfig.itemList[j].symbolName);
			}
		}
		if (!foundItem)
			printf("Item %s NOT found in symbol list\n", exampleConfig.itemList[j].symbolName);
	}

	printf("\n\nSetting up other connections\n\n");

	if (pService->flags & RDM_SVCF_HAS_SEQ_MCAST)
	{
		if ((pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_GAP_REC_SERV) && 
				(pService->seqMCastInfo.gapRecoveryServer.address.length > 0))
		{
			printf("Gap Recovery Server:\n\t%.*s "RTR_LLU"\n\n", pService->seqMCastInfo.gapRecoveryServer.address.length,
				pService->seqMCastInfo.gapRecoveryServer.address.data,
				pService->seqMCastInfo.gapRecoveryServer.port);
		}	

		if ((pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_REF_DATA_SERV) && 
				(pService->seqMCastInfo.refDataServer.address.length > 0))
		{
			printf("Reference Data Server:\n\t%.*s "RTR_LLU"\n\n",  pService->seqMCastInfo.refDataServer.address.length,
				pService->seqMCastInfo.refDataServer.address.data,
				pService->seqMCastInfo.refDataServer.port);
		}	

		// Connect to all the channels only if the verbose option was specified on the command line
		// otherwise connect only to the channels that provide the items of interest.
		if ((pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_SMC_SERV) && (exampleConfig.realTimeDataVerboseOutput == RSSL_TRUE))
		{
			printf("Streaming Multicast Server(s):\n");

			// When verbose mode is sellected connect to all the real time streams.
			for (j = 0; j < pService->seqMCastInfo.StreamingMCastChanServerCount; j++)
			{
				snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.StreamingMCastChanServerList[j].port);
				snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.StreamingMCastChanServerList[j].address.length, 
					pService->seqMCastInfo.StreamingMCastChanServerList[j].address.data);

				rsslClearConnectOpts(&copts);

				copts.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;

				if (j == 0)
				{
					copts.connectionInfo.segmented.recvServiceName = 
						exampleConfig.realTimeDataPort[0] ? exampleConfig.realTimeDataPort : &port[0];
					copts.connectionInfo.segmented.recvAddress = 
						exampleConfig.realTimeDataAddr[0] ? exampleConfig.realTimeDataAddr : &address[0];
				}
				else
				{
					copts.connectionInfo.segmented.recvServiceName = &port[0];
					copts.connectionInfo.segmented.recvAddress =  &address[0];
				}
				if (exampleConfig.hasRealTimeInterface)
					copts.connectionInfo.segmented.interfaceName = exampleConfig.realTimeDataIntf;

				snprintf(&pRealTimeFeedSession[j].gapInfo.address[0], 128, "%s", copts.connectionInfo.segmented.recvAddress);
				pRealTimeFeedSession[j].gapInfo.port = atoi(copts.connectionInfo.segmented.recvServiceName);

				printf("\nConnecting to real-time data server %s:%s\n", copts.connectionInfo.segmented.recvAddress,
						copts.connectionInfo.segmented.recvServiceName);
				if (realTimeSessionConnect(&pRealTimeFeedSession[j], &copts) != RSSL_RET_SUCCESS)
					exit(-1);

				// mark the session as connected
				pRealTimeFeedSession[j].connected = RSSL_TRUE;

				// setup tracing if enabled
				if (exampleConfig.xmlRealTimeTrace)
				{
					char tmp[128];

					snprintf(&tmp[0], 128, "%s%d_", exampleConfig.realTimeTraceOutputFile, j);
					setupXMLTracing(pRealTimeFeedSession[j].pRsslChannel, &tmp[0]);
				}
			}

			printf("\n");
		}
		else if ((pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_SMC_SERV) && (exampleConfig.realTimeDataVerboseOutput == RSSL_FALSE))	
		{
			// We have to find the channels we have to connect to based on the item requested and the symbol list information
			// about the channels
			// we do not want to connect to all the channels received from the Source Directory.
			printf("Streaming Multicast Server(s):\n");

			for (i = 0; exampleConfig.itemList[i].domainType != 0; i++)
			{
				// get the index from the symbol list of the channel information in the source directory message
				int index = exampleConfig.itemList[i].realTimeChannelId;

				// check if this is a duplicate connection
				RsslBool duplicateConnection = RSSL_FALSE;
				for (j = i - 1; j >= 0; j--)
				{
					if (index == exampleConfig.itemList[j].realTimeChannelId)
					{
						duplicateConnection = RSSL_TRUE;
						break;
					}
				}

				if (!duplicateConnection)
				{
					snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.StreamingMCastChanServerList[index].port);
					snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.StreamingMCastChanServerList[index].address.length, 
							pService->seqMCastInfo.StreamingMCastChanServerList[index].address.data);

					rsslClearConnectOpts(&copts);

					copts.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;

					if (j == 0)
					{
						copts.connectionInfo.segmented.recvServiceName =
							exampleConfig.realTimeDataPort[0] ? exampleConfig.realTimeDataPort : &port[0];
						copts.connectionInfo.segmented.recvAddress =
							exampleConfig.realTimeDataAddr[0] ? exampleConfig.realTimeDataAddr : &address[0];
					}
					else
					{
						copts.connectionInfo.segmented.recvServiceName = &port[0];
						copts.connectionInfo.segmented.recvAddress =  &address[0];
					}
					if (exampleConfig.hasRealTimeInterface)
						copts.connectionInfo.segmented.interfaceName = exampleConfig.realTimeDataIntf;

					snprintf(&pRealTimeFeedSession[i].gapInfo.address[0], 128, "%s", copts.connectionInfo.segmented.recvAddress);
					pRealTimeFeedSession[i].gapInfo.port = atoi(copts.connectionInfo.segmented.recvServiceName);

					printf("\nConnecting to real-time data server %s:%s\n", copts.connectionInfo.segmented.recvAddress,
							copts.connectionInfo.segmented.recvServiceName);
					if (realTimeSessionConnect(&pRealTimeFeedSession[i], &copts) != RSSL_RET_SUCCESS)
						exit(-1);

					// mark the session as connected
					pRealTimeFeedSession[i].connected = RSSL_TRUE;

					// setup tracing if enabled
					if (exampleConfig.xmlRealTimeTrace)
					{
						char tmp[128];

						snprintf(&tmp[0], 128, "%s%d_", exampleConfig.realTimeTraceOutputFile, i);
						setupXMLTracing(pRealTimeFeedSession[i].pRsslChannel, &tmp[0]);
					}
				}
                        }
                        printf("\n");
		}
			
		// connect to gap fill servers
		if (pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_GMC_SERV && (exampleConfig.gapFillServerVerboseOutput == RSSL_FALSE))
		{
			printf("Gap Fill Server(s):\n");

			for (i = 0; exampleConfig.itemList[i].domainType != 0; i++)
			{
				// get the index from the symbol list of the channel information in the source directory message
				int index = exampleConfig.itemList[i].gapChannelId;

				// check if this is a duplicate connection
				RsslBool duplicateConnection = RSSL_FALSE;
				for (j = i - 1; j >= 0; j--)
				{
					if (index == exampleConfig.itemList[j].gapChannelId)
					{
						duplicateConnection = RSSL_TRUE;
						break;
					}
				}

				if (!duplicateConnection)
				{
					snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.GapMCastChanServerList[index].port);
					snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.GapMCastChanServerList[index].address.length, 
							pService->seqMCastInfo.GapMCastChanServerList[index].address.data);

					rsslClearConnectOpts(&copts);

					copts.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;

					if (j == 0)
					{
						copts.connectionInfo.segmented.recvServiceName = 
							exampleConfig.gapFillServerPort[0] ? exampleConfig.gapFillServerPort : &port[0];
						copts.connectionInfo.segmented.recvAddress = 
							exampleConfig.gapFillServerAddr[0] ? exampleConfig.gapFillServerAddr : &address[0];
					}
					else
					{
						copts.connectionInfo.segmented.recvServiceName = &port[0];
						copts.connectionInfo.segmented.recvAddress =  &address[0];
					}

					if (exampleConfig.hasGapFillInterface)
						copts.connectionInfo.segmented.interfaceName = exampleConfig.gapFillIntf;


					printf("\nConnecting to gap fill data server %s:%s\n", copts.connectionInfo.segmented.recvAddress,
							copts.connectionInfo.segmented.recvServiceName);
					if (gapFillSessionConnect(&pGapFillSession[i], &copts) != RSSL_RET_SUCCESS)
						exit(-1);

					// mark the session as connected
					pGapFillSession[i].connected = RSSL_TRUE;
				}
			}
			printf("\n");
		}
		else if (pService->seqMCastInfo.flags & RDM_SVC_SMF_HAS_GMC_SERV && (exampleConfig.gapFillServerVerboseOutput == RSSL_TRUE))
		{
			printf("Gap Fill Server(s):\n");

			// When verbose mode is sellected connect to all the gap fill servers.
			for (j = 0; j < pService->seqMCastInfo.GapMCastChanServerCount; j++)
			{
				snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.GapMCastChanServerList[j].port);
				snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.GapMCastChanServerList[j].address.length, 
					pService->seqMCastInfo.GapMCastChanServerList[j].address.data);

				rsslClearConnectOpts(&copts);

				copts.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;

				if (j == 0)
				{
					copts.connectionInfo.segmented.recvServiceName = 
						exampleConfig.gapFillServerPort[0] ? exampleConfig.gapFillServerPort : &port[0];
					copts.connectionInfo.segmented.recvAddress = 
						exampleConfig.gapFillServerAddr[0] ? exampleConfig.gapFillServerAddr : &address[0];
				}
				else
				{
					copts.connectionInfo.segmented.recvServiceName = &port[0];
					copts.connectionInfo.segmented.recvAddress =  &address[0];
				}
				if (exampleConfig.hasGapFillInterface)
					copts.connectionInfo.segmented.interfaceName = exampleConfig.gapFillIntf;

				printf("\nConnecting to gap fill data server %s:%s\n", copts.connectionInfo.segmented.recvAddress,
						copts.connectionInfo.segmented.recvServiceName);
				if (gapFillSessionConnect(&pGapFillSession[j], &copts) != RSSL_RET_SUCCESS)
					exit(-1);

				// mark the session as connected
				pGapFillSession[j].connected = RSSL_TRUE;

			}

			printf("\n");
		}
	}	

	// Handle error case where there is a problem with the source directory message.
	// Try to use the command line information if specified
	if ((pService->seqMCastInfo.StreamingMCastChanServerCount == 0) && (strlen(&exampleConfig.realTimeDataPort[0]) > 0))
	{
		printf("There was an issue with the Source Directory message.\nWill try to use the command line arguments\n");

		pService->seqMCastInfo.StreamingMCastChanServerCount = 1;
		pRealTimeFeedSession = (RealTimeSession*)malloc(sizeof(RealTimeSession) * pService->seqMCastInfo.StreamingMCastChanServerCount);

		realTimeSessionClear(&pRealTimeFeedSession[0]);
		rsslClearConnectOpts(&copts);

		copts.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;

		copts.connectionInfo.segmented.recvServiceName = exampleConfig.realTimeDataPort;
		copts.connectionInfo.segmented.recvAddress = exampleConfig.realTimeDataAddr;

		if (exampleConfig.hasRealTimeInterface)
			copts.connectionInfo.segmented.interfaceName = exampleConfig.realTimeDataIntf;

		snprintf(&pRealTimeFeedSession[0].gapInfo.address[0], 128, "%s", copts.connectionInfo.segmented.recvAddress);
		pRealTimeFeedSession[0].gapInfo.port = atoi(copts.connectionInfo.segmented.recvServiceName);

		printf("\nConnecting to real-time data server %s:%s\n", copts.connectionInfo.segmented.recvAddress,
				copts.connectionInfo.segmented.recvServiceName);

		if (realTimeSessionConnect(&pRealTimeFeedSession[0], &copts) != RSSL_RET_SUCCESS)
			exit(-1);

		// mark the session as connected
		pRealTimeFeedSession[0].connected = RSSL_TRUE;

		// setup tracing if enabled
		if (exampleConfig.xmlRealTimeTrace)
		{
			char tmp[128];

			snprintf(&tmp[0], 128, "%s%d_", exampleConfig.realTimeTraceOutputFile, 0);
			setupXMLTracing(pRealTimeFeedSession[0].pRsslChannel, &tmp[0]);
		}
	}

	// connect to gap request server
	rsslClearConnectOpts(&copts);
	copts.connectionType = RSSL_CONN_TYPE_SOCKET;
	if (exampleConfig.gapRequestServerPort[0])
	{
		copts.connectionInfo.segmented.recvServiceName = exampleConfig.gapRequestServerPort;
	}
	else
	{
		snprintf(&port[0], 128, RTR_LLU, pService->seqMCastInfo.gapRecoveryServer.port);
		copts.connectionInfo.segmented.recvServiceName = &port[0];
	}
	if (exampleConfig.gapRequestServerAddr[0])
	{
		copts.connectionInfo.segmented.recvAddress = exampleConfig.gapRequestServerAddr;
	}
	else
	{
		snprintf(&address[0], 128, "%.*s", pService->seqMCastInfo.gapRecoveryServer.address.length, pService->seqMCastInfo.gapRecoveryServer.address.data);
		copts.connectionInfo.segmented.recvAddress = &address[0];
	}
	printf("Connecting to gap recovery server %s:%s\n", copts.connectionInfo.segmented.recvAddress, copts.connectionInfo.segmented.recvServiceName);
	if (gapRequestSessionConnect(&gapRequestServerSession, &copts) != RSSL_RET_SUCCESS)
		exit(-1);

	/* Send requests for items */

	snapshotSessionRequestItems(&snapshotServerSession);

}

static void loadDictionary()
{
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	rsslClearDataDictionary(&dictionary);
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("\nUnable to load field dictionary.\n\tError Text: %s\nWill request it from Reference Data Server\n\n", errorText.data);
		fieldDictionaryLoaded = RSSL_FALSE;
	}
	else
		fieldDictionaryLoaded = RSSL_TRUE;
			
	if (rsslLoadEnumTypeDictionary(enumTableFileName, &dictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary.\n\tError Text: %s\nWill request it from Reference Data Server\n\n", errorText.data);
		enumDictionaryLoaded = RSSL_FALSE;
	}
	else
		enumDictionaryLoaded = RSSL_TRUE;

	globalSetDefDictionaryLoaded = RSSL_FALSE;
}

static void setupXMLTracing(RsslChannel* pChannel, char* fileName)
{
	RsslError error;
	int debugFlags = 0x2C0;
	RsslTraceOptions traceOptions;

	rsslClearTraceOptions(&traceOptions);
	traceOptions.traceMsgFileName = fileName;
	traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
	traceOptions.traceMsgMaxFileSize = 100000000;
	rsslIoctl(pChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &error);
}

