/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the main file for the rsslVAConsumer application.  It is a single-threaded
 * client application.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the UPA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#define strtok_r strtok_s
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#include "rsslConsumer.h"
#include "rsslLoginConsumer.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslYieldCurveHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslVASendMessage.h"
#include "rsslPostHandler.h"
#include "queueMsgHandler.h"

#include "rtr/rsslPayloadEntry.h"

#include "rtr/rsslReactor.h"
// APIQA: Commenting out internal code
// Start TR_INTERNAL_SRC
// #include "rtr/rsslTransportInt.h"
// End TR_INTERNAL_SRC
// END APIQA

static RsslInt32 timeToRun = 300;
static time_t rsslConsumerRuntime = 0;
static RsslBool runTimeExpired = RSSL_FALSE;
static time_t cacheTime = 0;
static time_t cacheInterval = 0;
static RsslBool onPostEnabled = RSSL_FALSE, offPostEnabled = RSSL_FALSE;
static RsslBool xmlTrace = RSSL_FALSE;
static RsslBool needFdmDictionary = RSSL_FALSE;
// APIQA:
int iBufSizeConsumer = 0;
int iFillSizeConsumer = 0;
// END APIQA

#define MAX_CHAN_COMMANDS 4
static ChannelCommand chanCommands[MAX_CHAN_COMMANDS];
static int channelCommandCount = 0;

fd_set readFds, exceptFds;
static RsslReactor *pReactor = NULL;

char userNameBlock[128];
static char traceOutputFile[128];
RsslBuffer userName = RSSL_INIT_BUFFER;

static void displayCache(ChannelCommand *pCommand);
static void displayCacheDomain(ChannelCommand *pCommand, RsslUInt8 domainType, RsslBool privateStreams, RsslInt32 itemCount, ItemRequest items[]);
static RsslRet decodeEntryFromCache(ChannelCommand *pCommand, RsslPayloadEntryHandle cacheEntryHandle, RsslUInt8 domainType);
static void sendItemRequests(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel);

static char _bufferArray[6144];

/* 
 * Prints example usage and exits. 
 */
void printUsageAndExit(char *appName)
{
	// APIQA: Added bufSize and fillSize
	printf("Usage: %s or\n%s  [-tcp [<hostname>:<port> <service name>]] [<domain>:<item name>,...] ] [-uname <LoginUsername>] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>] [-cache] [-cacheInterval <seconds>] [-bufSize <size>] [-fillSize <size>] [-tunnel] [-tsDomain <number> ] [-tsAuth] [ -qSourceName <name> ] [ -qDestName <name> ] [-tsServiceName] [-x] [-runtime]\n"
	// END APIQA
			"\n -tcp specifies a connection to open and a list of items to request:\n"
			"\n     hostname:        Hostname of provider to connect to"
			"\n     port:            Port of provider to connect to"
			"\n     service:         Name of service to request items from on this connection"
			"\n     domain:itemName: Domain and name of an item to request."
			"\n         A comma-separated list of these may be specified."
			"\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)\n"
			"\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)\n"
			"\n         Example Usage: -tcp localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl"
			"\n           (for SymbolList requests, a name can be optionally specified)\n"
			"\n -uname changes the username used when logging into the provider.\n"
			"\n -view specifies each request using a basic dynamic view.\n"
			"\n -post specifies that the application should attempt to send post messages on the first requested Market Price item.\n"
			"\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n"
			"\n -snapshot specifies each request using non-streaming.\n" 
			"\n -cache will store all open items in cache and periodically dump contents.\n"
			"\n -cacheInterval number of seconds between displaying cache contents; 0 = on exit only (default)\n"
			// APIQA: Added bufSize	and fillSize
			"\n -bufSize size of buffer to use to send a tunnel stream message\n"
			"\n -fillSize amount of bytes to write when sending a tunnel stream message (must be used together with bufSize for proper operation) \n"
			// END APIQA
			"\n -tunnel causes the consumer to open a tunnel stream that exchanges basic messages(see -qSourceName for queue messaging streams).\n"
			"\n -tsAuth causes the consumer to enable authentication when opening tunnel streams (also applies to queue messaging treams).\n"
			"\n -qSourceName specifies the source name for queue messages (if specified, configures consumer to receive queue messages)\n"
			"\n -qDestName specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)\n"
			"\n -tsServiceName specifies the name of the service to use for queue messages (if not specified, the service name specified in -c/-tcp is used)\n"
			"\n -x provides an XML trace of messages\n"
			"\n -runtime adjusts the running time of the application.\n"
			, appName, appName);

	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(-1);
}

/* 
 * configures options based on command line arguments.
 */
void parseCommandLine(int argc, char **argv)
{

	RsslInt32 i;
	RsslBool cacheOption = RSSL_FALSE;

	/* Check usage and retrieve operating parameters */
	{
		ChannelCommand *pCommand = NULL;
		RsslBool hasQueueServiceName = RSSL_FALSE;
		RsslBool useTunnelStreamAuthentication = RSSL_FALSE;
		RsslUInt8 tunnelStreamDomainType = RSSL_DMT_SYSTEM;

		i = 1;

		while(i < argc)
		{
			if(strcmp("-uname", argv[i]) == 0)
			{
				i += 2;
				userName.length = snprintf(userNameBlock, sizeof(userNameBlock), "%s", argv[i-1]);
				userName.data = userNameBlock;
			}
			else if(strcmp("-view", argv[i]) == 0)
			{
				i++;
				setViewRequest();
			}
			else if (strcmp("-post", argv[i]) == 0)
			{
				i++;
				onPostEnabled = RSSL_TRUE;
			}
			else if (strcmp("-offpost", argv[i]) == 0)
			{
				i++;
				offPostEnabled = RSSL_TRUE;
			}
			else if(strcmp("-snapshot", argv[i]) == 0)
			{
				i++;
				setMPSnapshotRequest();
				setMBOSnapshotRequest();
				setMBPSnapshotRequest();
				setSLSnapshotRequest();
				setYCSnapshotRequest();
			} 
			else if (strcmp("-cache", argv[i]) == 0)
			{
				i++;
				cacheOption = RSSL_TRUE;
			}
			else if (strcmp("-cacheInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				cacheInterval = atoi(argv[i-1]);
			}
			// APIQA
			else if (strcmp("-bufSize", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				iBufSizeConsumer = atoi(argv[i-1]);
			}
			else if (strcmp("-fillSize", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				iFillSizeConsumer = atoi(argv[i-1]);
			}
			// END APIQA
			else if ((strcmp("-c", argv[i]) == 0) || (strcmp("-tcp", argv[i]) == 0))
			{
				char *pToken, *pToken2, *pSaveToken, *pSaveToken2;

				RsslUInt8 itemDomain;

				if (channelCommandCount == MAX_CHAN_COMMANDS)
				{
					printf("Too many connections requested.\n");
					printUsageAndExit(argv[0]);
				}

				pCommand = &chanCommands[channelCommandCount];
				hasQueueServiceName = RSSL_FALSE;
				useTunnelStreamAuthentication = RSSL_FALSE;
				tunnelStreamDomainType = RSSL_DMT_SYSTEM;

				queueMsgHandlerInit(&pCommand->queueMsgHandler, (char*)"VAConsumer", RSSL_DMT_SYSTEM, RSSL_FALSE);
				simpleTunnelMsgHandlerInit(&pCommand->simpleTunnelMsgHandler, (char*)"VAConsumer", RSSL_DMT_SYSTEM, RSSL_FALSE, RSSL_FALSE);


				/* Syntax:
				 *  -tcp hostname:port:SERVICE_NAME mp:TRI,mp:.DJI
				 */

				i += 1;
				if (i >= argc) printUsageAndExit(argv[0]);

				/* Hostname */
				pToken = strtok(argv[i], ":");
				if (!pToken) { printf("Error: Missing hostname.\n"); printUsageAndExit(argv[0]); }
				snprintf(pCommand->hostName, MAX_BUFFER_LENGTH, pToken);
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName;

				/* Port */
				pToken = strtok(NULL, ":");
				if (!pToken) { printf("Error: Missing port.\n"); printUsageAndExit(argv[0]); }
				snprintf(pCommand->port, MAX_BUFFER_LENGTH, pToken);
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port;

				pToken = strtok(NULL, ":");
				if (pToken) { printf("Error: extra input after <hostname>:<port>.\n"); printUsageAndExit(argv[0]); }

				i += 1;

				/* Item Service Name */
				pToken = argv[i];
				snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, pToken);

				i += 1;
				if (i < argc)
				{
					if (argv[i][0] != '-')
					{
						/* Item List */
						pToken = strtok_r(argv[i], ",", &pSaveToken);

						while(pToken)
						{
							ItemRequest *pItemRequest;
							/* domain */
							pToken2 = strtok_r(pToken, ":", &pSaveToken2);
							if (!pToken2) { printf("Error: Missing domain.\n"); printUsageAndExit(argv[0]); }

							if (0 == strcmp(pToken2, "mp"))
							{
								if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPriceItems[pCommand->marketPriceItemCount];
									++pCommand->marketPriceItemCount;
								}
								else
								{
									printf("Number of Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbo"))
							{
								if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderItems[pCommand->marketByOrderItemCount];
									++pCommand->marketByOrderItemCount;
								}
								else
								{
									printf("Number of Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbp"))
							{
								if (pCommand->marketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPriceItems[pCommand->marketByPriceItemCount];
									++pCommand->marketByPriceItemCount;
								}
								else
								{
									printf("Number of Market By Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "yc"))
							{
								if (pCommand->yieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurveItems[pCommand->yieldCurveItemCount];
									++pCommand->yieldCurveItemCount;
								}
								else
								{
									printf("Number of Yield Curve items exceeded\n");
								}
							}
							else if(0 == strcmp(pToken2, "sl"))
							{
								itemDomain = RSSL_DMT_SYMBOL_LIST;
								pItemRequest = &pCommand->symbolListRequest;
								pCommand->sendSymbolList = RSSL_TRUE;
							}
							else if (0 == strcmp(pToken2, "mpps"))
							{
								if (pCommand->privateStreamMarketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPricePSItems[pCommand->privateStreamMarketPriceItemCount];
									++pCommand->privateStreamMarketPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbops"))
							{
								if (pCommand->privateStreamMarketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderPSItems[pCommand->privateStreamMarketByOrderItemCount];
									++pCommand->privateStreamMarketByOrderItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "ycps"))
							{
								if (pCommand->privateStreamYieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurvePSItems[pCommand->privateStreamYieldCurveItemCount];
									++pCommand->privateStreamYieldCurveItemCount;
								}
								else
								{
									printf("Number of Private Stream Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbpps"))
							{
								if (pCommand->privateStreamMarketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPricePSItems[pCommand->privateStreamMarketByPriceItemCount];
									++pCommand->privateStreamMarketByPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Price items exceeded\n");
								}
							}
							else
							{
								printf("Error: Unknown item domain: %s\n", pToken2);
								printUsageAndExit(argv[0]);
							}

							if (pItemRequest)
							{
								pItemRequest->cacheEntryHandle = 0;
								pItemRequest->streamId = 0;
								pItemRequest->groupIdIndex = -1;
							}

							/* name */
							pToken2 = strtok_r(NULL, ":", &pSaveToken2);
							if (!pToken2) 
							{ 
								if(itemDomain != RSSL_DMT_SYMBOL_LIST)
								{
									printf("Error: Missing item name.\n"); 
									printUsageAndExit(argv[0]);
								}
								else
								{
									/* No specific name given for the symbol list request.
									 * A name will be retrieved from the directory
									 * response if available. */
									pItemRequest->itemName.data = NULL;
									pItemRequest->itemName.length = 0;
								}
							}
							else
							{
								snprintf(pItemRequest->itemNameString, 128, "%s", pToken2);
								pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
								pItemRequest->itemName.data = pItemRequest->itemNameString;

								/* A specific name was specified for the symbol list request. */
								if(itemDomain == RSSL_DMT_SYMBOL_LIST)
									pCommand->userSpecSymbolList = RSSL_TRUE;
							}
							pToken = strtok_r(NULL, ",", &pSaveToken);
						}

						i += 1;
					}
				}

				++channelCommandCount;
			}
			else if (strcmp("-x", argv[i]) == 0)
			{
				i++;
				xmlTrace = RSSL_TRUE;
				snprintf(traceOutputFile, 128, "RsslVAConsumer\0");
			}
			else if (strcmp("-runtime", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				timeToRun = atoi(argv[i-1]);
				if (timeToRun == 0)
					timeToRun = 5;
			}
			else if (strcmp("-tsServiceName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsServiceName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				hasQueueServiceName = RSSL_TRUE;
				snprintf(pCommand->tunnelStreamServiceName, sizeof(pCommand->tunnelStreamServiceName), argv[i-1]);
			}
			else if (strcmp("-tunnel", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tunnel specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}
				pCommand->tunnelMessagingEnabled = RSSL_TRUE;

				i += 1;
			}
			else if (strcmp("-tsDomain", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsDomain specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tunnelStreamDomainType = atoi(argv[i-1]);
			}
			else if (strcmp("-qSourceName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-qSourceName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);

				pCommand->queueMessagingEnabled = RSSL_TRUE;
				needFdmDictionary = RSSL_TRUE;

				pCommand->queueMsgHandler.sourceName.data = argv[i-1];
				pCommand->queueMsgHandler.sourceName.length = (RsslUInt32)strlen(argv[i-1]);
			}
			else if (strcmp("-tsAuth", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsAuth specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				useTunnelStreamAuthentication = RSSL_TRUE;
				i += 1;
			}
			else if (strcmp("-qDestName", argv[i]) == 0)
			{
				int destNameCount = pCommand->queueMsgHandler.destNameCount;

				if (pCommand == NULL)
				{
					printf("-qDestName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				if (destNameCount == MAX_DEST_NAMES)
				{
					printf("Error: Example only supports %d queue destination names.\n", MAX_DEST_NAMES);
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				pCommand->queueMsgHandler.destNames[destNameCount].data = argv[i-1];
				pCommand->queueMsgHandler.destNames[destNameCount].length = (RsslUInt32)strlen(argv[i-1]);

				++pCommand->queueMsgHandler.destNameCount;
			}
			else if (strcmp("-?", argv[i]) == 0)
			{
				printUsageAndExit(argv[0]);
			}
			else
			{
				printf("Unknown option: %s\n", argv[i]);
				printUsageAndExit(argv[0]);
			}

			/* Check channel-specific options. */
			if (pCommand != NULL && (i >= argc || strcmp("-c", argv[i]) == 0 || strcmp("-tcp", argv[i]) == 0))
			{
				/* Only allow one of tunnel messaging or queue messaging to be enabled. */
				if (pCommand->tunnelMessagingEnabled && pCommand->queueMessagingEnabled)
				{
					printf("Error: Both tunnel stream messaging and queue messaging enabled.\n");
					printUsageAndExit(argv[0]);
				}

				/* Ensure queue source is specified if queue destinations are specified. */
				if (!pCommand->queueMessagingEnabled && pCommand->queueMsgHandler.destNameCount > 0)
				{
					printf("Error: Destination queue specified without source queue.\n");
					printUsageAndExit(argv[0]);
				}

				/* If service not specified for queue messaging, use the service given for other items instead. */
				if ((pCommand->tunnelMessagingEnabled || pCommand->queueMessagingEnabled) && hasQueueServiceName == RSSL_FALSE)
				{
					snprintf(pCommand->tunnelStreamServiceName, sizeof(pCommand->tunnelStreamServiceName), pCommand->serviceName);
				}

				pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.useAuthentication = useTunnelStreamAuthentication;
				pCommand->queueMsgHandler.tunnelStreamHandler.useAuthentication = useTunnelStreamAuthentication;
				pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.domainType = tunnelStreamDomainType;
				pCommand->queueMsgHandler.tunnelStreamHandler.domainType = tunnelStreamDomainType;
			}
		}

	}
	// APIQA:  setting buffer size and fillSize here 
	setBufSize(iBufSizeConsumer);
	setFillSize(iFillSizeConsumer);
	// END APIQA: 

	/* If no connections were specified on the command line,
	 * the default will be to create two connections to localhost:14002. */ 
	if (!channelCommandCount)
	{
		ChannelCommand *pCommand;
		RsslReactorConnectInfo *pInfo;

		channelCommandCount = 2;

		/* 1st Connection*/
		pCommand = &chanCommands[0];
		pInfo = &pCommand->cInfo;

		/* 1st Connection: Connect to localhost:14002 */
		snprintf(pCommand->hostName, MAX_BUFFER_LENGTH, "%s", "localhost"); 
		pInfo->rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName;
		snprintf(pCommand->port, MAX_BUFFER_LENGTH, "%s", "14002"); 
		pInfo->rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port;

		/* 1st Connection: Service name is DIRECT_FEED */
		snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", "DIRECT_FEED");

		/* 1st Connection: Request Item TRI.N */
		pCommand->marketPriceItemCount = 1;
		pCommand->marketPriceItems[0].itemName.data = (char *)"TRI.N"; 
		pCommand->marketPriceItems[0].itemName.length = 5;

		/* 2nd Connection*/ 
		pCommand = &chanCommands[1];
		pInfo = &pCommand->cInfo;

		/* 2nd Connection: Connect to localhost:14002 */
		snprintf(pCommand->hostName, MAX_BUFFER_LENGTH, "%s", "localhost"); 
		pInfo->rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName;
		snprintf(pCommand->port, MAX_BUFFER_LENGTH, "%s", "14002"); 
		pInfo->rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port;


		/* 2nd Connection: Service name is DIRECT_FEED */
		snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", "DIRECT_FEED");
		pInfo->rsslConnectOptions.userSpecPtr = &chanCommands[0];

		/* 2nd Connection: Request Item TRI.N & .DJI*/
		pCommand->marketPriceItemCount = 2;
		pCommand->marketPriceItems[0].itemName.data = (char *)"TRI.N"; 
		pCommand->marketPriceItems[0].itemName.length = 5;
		pCommand->marketPriceItems[1].itemName.data = (char *)".DJI"; 
		pCommand->marketPriceItems[1].itemName.length = 4;
	}

	if (cacheOption)
	{
		ChannelCommand *pCommand;
		for (i = 0; i < channelCommandCount; i++)
		{
			pCommand = &chanCommands[i];
			pCommand->cacheInfo.useCache = RSSL_TRUE;
			pCommand->cacheInfo.cursorHandle = 0;
			pCommand->cacheInfo.cacheOptions.maxItems = 100000;
			pCommand->cacheInfo.cacheHandle = rsslPayloadCacheCreate(&pCommand->cacheInfo.cacheOptions, &pCommand->cacheInfo.cacheErrorInfo);
			if (pCommand->cacheInfo.cacheHandle == 0)
			{
				printf("Error: Failed to create cache on channel %s:%s.\n\tError (%d): %s\n",
						pCommand->hostName, pCommand->port,
						pCommand->cacheInfo.cacheErrorInfo.rsslErrorId, pCommand->cacheInfo.cacheErrorInfo.text);
				pCommand->cacheInfo.useCache = RSSL_FALSE;
			}
			snprintf(pCommand->cacheInfo.cacheDictionaryKey, sizeof(pCommand->cacheInfo.cacheDictionaryKey), "cacheDictionary%d", i);
		}
	}

}

/* 
 * Closes a channel in the RsslReactor
 */
void closeConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, ChannelCommand *pCommand)
{
	RsslErrorInfo rsslErrorInfo;

	if (pChannel->socketId != REACTOR_INVALID_SOCKET)
	{
		FD_CLR(pChannel->socketId, &readFds);
		FD_CLR(pChannel->socketId, &exceptFds);
	}

	if (rsslReactorCloseChannel(pReactor, pChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %s\n", rsslErrorInfo.rsslError.text);
	}

	pCommand->reactorChannelReady = RSSL_FALSE;
}

/* 
 * Processes events about the state of an RsslReactorChannel.
 */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorConnect() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. This will drive the process of setting up the connection
			 * by exchanging the Login, Directory, and (if not already loaded)Dictionary messages. 
			 * The application will receive the response messages in the appropriate callback
			 * function we specified. */

#ifdef _WIN32
			int rcvBfrSize = 65535;
			int sendBfrSize = 65535;
			RsslErrorInfo rsslErrorInfo;
#endif

			printf("Connection up! Channel fd=%d\n\n", pReactorChannel->socketId);

			/* Set file descriptor. */
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);

			/* Save the channel on our info structure. */
			pCommand->reactorChannel = pReactorChannel;

			if (!pCommand->dictionariesLoaded)
			{
				rsslDeleteDataDictionary(&pCommand->dictionary);
				rsslClearDataDictionary(&pCommand->dictionary);
			}

#ifdef _WIN32
			/* WINDOWS: change size of send/receive buffer since it's small by default */
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
#endif
			if (xmlTrace) 
			{
				RsslTraceOptions traceOptions;
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			pCommand->reactorChannelReady = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			/* The file descriptor representing the RsslReactorChannel has been changed.
			 * Update our file descriptor sets. */
			printf("Fd change: %d to %d\n", pReactorChannel->oldSocketId, pReactorChannel->socketId);
			FD_CLR(pReactorChannel->oldSocketId, &readFds);
			FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			/* The channel has failed and has gone down.  Print the error, close the channel, and reconnect later. */

			printf("Connection down: Channel fd=%d.\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			/* It is important to make sure that no more interface calls are made using the channel after
			 * calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
			 * to call it inside callback functions. */
			closeConnection(pReactor, pReactorChannel, pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			printf("Connection down, reconnecting.  Channel fd=%d\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &readFds);
				FD_CLR(pReactorChannel->socketId, &exceptFds);
			}

			clearChannelCommand(pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd=%d.\n", pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("Unknown connection event!\n");
			cleanUpAndExit(-1);
		}
	}


	return RSSL_RC_CRET_SUCCESS;
}

static void sendItemRequests(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	ChannelCommand *pCommand;

	if (!pReactorChannel)
		return;

	pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;

	if (pCommand->itemsRequested == RSSL_TRUE)
		return;

	if (pCommand->serviceNameFound != RSSL_TRUE || pCommand->reactorChannelReady != RSSL_TRUE)
		return;

	if (sendMarketPriceItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendMarketByOrderItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendMarketByPriceItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendYieldCurveItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendSymbolListRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	/* Start posting, if the provider supports it */
	if ((onPostEnabled||offPostEnabled) && !pCommand->postEnabled)
	{
		RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

		if (loginInfo->supportOMMPost == RSSL_TRUE)
		{
			pCommand->postEnabled = RSSL_TRUE;
			pCommand->shouldOnStreamPost = onPostEnabled;
			pCommand->shouldOffStreamPost = offPostEnabled;
			/* This sets up our basic timing so post messages will be sent periodically */
			initPostHandler(pCommand);
		}
		else
		{
			/* provider does not support posting, disable it */
			pCommand->postEnabled = RSSL_FALSE;
			printf("\nConnected Provider does not support OMM Posting.  Disabling Post functionality.\n");
		}
	}

	pCommand->itemsRequested = RSSL_TRUE;
}

/*** MAIN ***/
int main(int argc, char **argv)
{
	RsslRet ret;
	RsslCreateReactorOptions reactorOpts;
	RsslErrorInfo rsslErrorInfo;
	int i;

	RsslReactorOMMConsumerRole consumerRole;
	RsslRDMLoginRequest loginRequest;
	RsslRDMDirectoryRequest dirRequest;
	RsslReactorDispatchOptions dispatchOpts;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}

	if ((ret = rsslPayloadCacheInitialize()) != RSSL_RET_SUCCESS)
	{
		printf("rsslPayloadCacheInitialize() failed: %d (%s)", ret,rsslRetCodeToString(ret));
		exitApp(-1);
	}

	for(i = 0; i < MAX_CHAN_COMMANDS; ++i)
		initChannelCommand(&chanCommands[i]);

	/* Initialize parameters from config. */
	parseCommandLine(argc, argv);

	/* If queue messaging is in use, load FDM Dictionary */
	if (needFdmDictionary && loadFdmDictionary() == RSSL_FALSE)
		cleanUpAndExit(-1);
		

	/* Initialize run-time */
	initRuntime();

	/* Initialize the default login request(Use 1 as the Login Stream ID). */
	if (rsslInitDefaultRDMLoginRequest(&loginRequest, 1) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		cleanUpAndExit(-1);
	}
	
	/* If a username was specified, change username on login request. */
	if (userName.length)
		loginRequest.userName = userName;

	/* Initialize the default directory request(Use 2 as the Directory Stream Id) */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, 2) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		cleanUpAndExit(-1);
	}

	/* Setup callback functions and use them on all connections*/
	rsslClearOMMConsumerRole(&consumerRole);
	consumerRole.loginMsgCallback = loginMsgCallback;
	consumerRole.directoryMsgCallback = directoryMsgCallback;
	consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	consumerRole.base.channelEventCallback = channelEventCallback;
	consumerRole.base.defaultMsgCallback = defaultMsgCallback;

	/* Set the messages to send when the channel is up */
	consumerRole.pLoginRequest = &loginRequest;
	consumerRole.pDirectoryRequest = &dirRequest;

	printf("Connections:\n");

	/* Print out a summary of the connections and desired items. */
	for(i = 0; i < channelCommandCount; ++i)
	{
		RsslInt32 j;

		ChannelCommand *pCommand = &chanCommands[i];

		printf("	%s:%s %s\n", pCommand->hostName, pCommand->port, pCommand->serviceName);

		printf("		MarketPriceItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketPriceItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		MarketByOrderItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByOrderItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		MarketByPriceItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByPriceItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		YieldCurveItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->yieldCurveItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}

		printf("\n");

		printf("		MarketPriceItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketPricePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		MarketByOrderItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByOrderPSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		MarketByPriceItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByPricePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		YieldCurveItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->yieldCurvePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}

		printf("\n");
	}

	/* Initialize connection options and try to load dictionaries. */
	for(i = 0; i < channelCommandCount; ++i)
	{
		ChannelCommand *pCommand = &chanCommands[i];
		RsslReactorConnectOptions *pOpts = &pCommand->cOpts;
		RsslReactorConnectInfo *pInfo = &pCommand->cInfo;

		loadDictionary(pCommand);
		pCommand->pRole = (RsslReactorChannelRole*)&consumerRole;
		pInfo->rsslConnectOptions.guaranteedOutputBuffers = 500;
		pInfo->rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		pInfo->rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		pInfo->rsslConnectOptions.userSpecPtr = &chanCommands[i];
		pInfo->initializationTimeout = 5;
		pOpts->reactorConnectionList = pInfo;
		pOpts->connectionCount = 1;
		pOpts->reconnectAttemptLimit = -1;
		pOpts->reconnectMaxDelay = 5000;
		pOpts->reconnectMinDelay = 1000;

	}

	printf("\n");

	/* Create an RsslReactor which will manage our channels. */

	rsslClearCreateReactorOptions(&reactorOpts);

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors 
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pReactor->eventFd, &readFds);

	/* Add the desired connections to the reactor. */
	for(i = 0; i < channelCommandCount; ++i)
	{
		ChannelCommand *pCommand = &chanCommands[i];
		if (!isDictionaryLoaded(pCommand))
			consumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;

		printf("Adding connection to %s:%s...\n", pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address, pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName );

		if (rsslReactorConnect(pReactor, &pCommand->cOpts, pCommand->pRole, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error rsslReactorConnect(): %s\n", rsslErrorInfo.rsslError.text);
		}

		printf("\n");

	}

	rsslClearReactorDispatchOptions(&dispatchOpts);

	/* Main loop. The loop
	 * calls select() to wait for notification, then calls rsslReactorDispatch(). */
	do
	{
		struct timeval selectTime;
		int dispatchCount = 0;
		fd_set useReadFds = readFds, useExceptFds = exceptFds;
		selectTime.tv_sec = 1; selectTime.tv_usec = 0;

		handleRuntime();

		if (!runTimeExpired)
		{
			for (i = 0; i < channelCommandCount; ++i)
			{
				sendItemRequests(pReactor, chanCommands[i].reactorChannel);
				if (chanCommands[i].tunnelMessagingEnabled)
					handleSimpleTunnelMsgHandler(pReactor, chanCommands[i].reactorChannel, &chanCommands[i].simpleTunnelMsgHandler);
				if (chanCommands[i].queueMessagingEnabled)
					handleQueueMsgHandler(pReactor, chanCommands[i].reactorChannel, &chanCommands[i].queueMsgHandler);
			}

			if (onPostEnabled ||offPostEnabled)
			{
				for(i = 0; i < channelCommandCount; ++i)
				{
					if (handlePosts(pReactor, &chanCommands[i]) != RSSL_RET_SUCCESS)
						cleanUpAndExit(-1);
				}
			}
		}

		ret = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);

		if (ret < 0)
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;

			printf("Error: select: %d\n", WSAGetLastError());
#else
			if (errno == EINTR)
				continue;

			perror("select");
#endif
			cleanUpAndExit(-1);
		}

		
		/* Call rsslReactorDispatch().  This will handle any events that have occurred on its channels.
		 * If there are events or messages for the application to process, they will be delivered
		 * through the callback functions given by the consumerRole object. 
		 * A return value greater than RSSL_RET_SUCCESS indicates there may be more to process. */
		while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
			cleanUpAndExit(-1);
		}

	} while(ret >= RSSL_RET_SUCCESS);

	cleanUpAndExit(-1);
}

/*
 * Initializes the run-time.
 */
static void initRuntime()
{
	time_t currentTime = 0;
	
	/* get current time */
	time(&currentTime);

	rsslConsumerRuntime = currentTime + (time_t)timeToRun;

	cacheTime = currentTime + (time_t)cacheInterval;
}

/*
 * Handles time-related events.
 * Calls rsslReactorConnect() for any channels that we want to recover.
 * Exits the application if the run-time has expired.
 */
static void handleRuntime()
{
	time_t currentTime = 0;
	RsslRet	retval = 0;
	int i;
	RsslState state;

	/* get current time */
	time(&currentTime); 

	if (currentTime >= cacheTime && cacheInterval > 0)
	{
		for (i = 0; i < channelCommandCount; ++i)
		{
			if (chanCommands[i].cacheInfo.useCache)
				displayCache(&chanCommands[i]);
		}

		cacheTime = currentTime + (time_t)cacheInterval;
	}

	if (currentTime >= rsslConsumerRuntime)
	{
		RsslBool tunnelStreamsOpen = RSSL_FALSE;

		if (!runTimeExpired)
		{
			RsslBool waitingTunnelStreamFree = RSSL_FALSE;
			printf("rsslVAConsumer run-time expired.\n\n");
			runTimeExpired = RSSL_TRUE;

			state.streamState = RSSL_STREAM_CLOSED;
			state.dataState = RSSL_DATA_SUSPECT;
			state.code = 0;
			for(i = 0; i < channelCommandCount; ++i)
			{
				closeOffstreamPost(pReactor, &chanCommands[i]);

				/* for each connection, close all item streams */
				closeMarketPriceItemStreams(pReactor, &chanCommands[i]);
				closeMarketByOrderItemStreams(pReactor, &chanCommands[i]);
				closeMarketByPriceItemStreams(pReactor, &chanCommands[i]);
				closeYieldCurveItemStreams(pReactor, &chanCommands[i]);

				if (chanCommands[i].simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL
						|| chanCommands[i].queueMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
				{
					if (waitingTunnelStreamFree == RSSL_FALSE)
					{
						waitingTunnelStreamFree = RSSL_TRUE;
						printf("Waiting for tunnel streams to close...\n");
					}

					simpleTunnelMsgHandlerCloseStreams(&chanCommands[i].simpleTunnelMsgHandler);
					queueMsgHandlerCloseStreams(&chanCommands[i].queueMsgHandler);
				}

				setItemStates(&chanCommands[i], -1, &state);
			}
		}

		/* If any tunnel streams are open, wait for them to close before quitting. */
		for(i = 0; i < channelCommandCount; ++i)
		{
			if (chanCommands[i].simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL
					|| chanCommands[i].queueMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
			{
				tunnelStreamsOpen = RSSL_TRUE;
				break;
			}
		}

		if (!tunnelStreamsOpen || currentTime >= rsslConsumerRuntime + 10)
		{
			if (tunnelStreamsOpen)
				printf("Tunnel streams still open after ten seconds, giving up.\n");
			cleanUpAndExit(0);
		}
	}
}

/*
 * Processes all RSSL messages that aren't processed by 
 * any domain-specific callback functions.  Responses for
 * items requested by the function are handled here. 
 */
static RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent)
{
	RsslRet ret = 0;
	RsslDecodeIterator dIter;
	RsslReactorCallbackRet cret = RSSL_RC_CRET_SUCCESS;
	RsslMsg *pMsg = pMsgEvent->pRsslMsg;
	ChannelCommand *pCommand = (ChannelCommand*)pChannel->userSpecPtr;

	if (!pMsg)
	{
		/* The message is not present because an error occurred while decoding it.  Print 
		 * the error and close the channel. If desired, the un-decoded message buffer 
		 * is available in pMsgEvent->pRsslMsgBuffer. */

		RsslErrorInfo *pError = pMsgEvent->pErrorInfo;
		printf("defaultMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		closeConnection(pReactor, pChannel, pCommand);
		return RSSL_RC_CRET_SUCCESS;
	}

	/* Clear the iterator and set it to the message payload. */
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion, pChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);

	switch ( pMsg->msgBase.domainType )
	{
		case RSSL_DMT_MARKET_PRICE:
			if (processMarketPriceResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			if (processMarketByOrderResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_MARKET_BY_PRICE:
			if (processMarketByPriceResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_YIELD_CURVE:
			if (processYieldCurveResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_SYMBOL_LIST:
			if (processSymbolListResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		default:
			printf("Unhandled Domain Type %d received on channel %d\n", pMsg->msgBase.domainType, pChannel->socketId);
			break;
	}

	return cret;
}


/*
 * Cleans up and exits the application.
 */
void cleanUpAndExit(int code)
{
	RsslErrorInfo rsslErrorInfo;
	int i;
	unsigned int n;

	if (pReactor && rsslDestroyReactor(pReactor, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error cleaning up reactor: %s\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}

	for (i = 0; i < channelCommandCount; ++i)
	{
		if (chanCommands[i].cacheInfo.useCache)
			displayCache(&chanCommands[i]);
	}

	for (i = 0; i < channelCommandCount; ++i)
	{
		if (chanCommands[i].cacheInfo.useCache && chanCommands[i].cacheInfo.cacheHandle)
			rsslPayloadCacheDestroy(chanCommands[i].cacheInfo.cacheHandle);

		for (n=0; n < chanCommands[i].groupIdCount; n++)
		{
			if (chanCommands[i].groupIdBuffers[n].data)
				free (chanCommands[i].groupIdBuffers[n].data);
		}
	}

	cleanupFdmDictionary();

	rsslPayloadCacheUninitialize();
	rsslUninitialize();

	exitApp(code);
}

void exitApp(int code)
{
	printf("Exiting.\n");
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(code);
}

void dumpHexBuffer(const RsslBuffer * buffer)
{
      unsigned int i;
      char * position = buffer->data;
      for (i = 0; i < buffer->length; i++, position++)
      {
            if (i % 32 == 0)
            {
                  if (i != 0)
                  {
                        printf("\n");
                  }
            }
            else if ((i != 0) && (i % 2 == 0))
            {
                  printf(" ");
            }
            printf("%2.2X", (unsigned char) *position);
      }
}

static void displayCache(ChannelCommand *pCommand)
{
	printf("\nStarting Cache Display ");

	if (pCommand->reactorChannel)
		printf("(Channel %d):\n", pCommand->reactorChannel->socketId);
	else
		printf(":\n");

	if (pCommand->cacheInfo.cacheHandle)
		printf("Total Items in Cache: " RTR_LLU "\n", rsslPayloadCacheGetEntryCount(pCommand->cacheInfo.cacheHandle));

	if (!pCommand->dictionary.isInitialized)
	{
		printf("\tDictionary for decoding cache entries is not available\n");
		return;
	}

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_PRICE, RSSL_FALSE, pCommand->marketPriceItemCount, pCommand->marketPriceItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_PRICE, RSSL_TRUE, pCommand->privateStreamMarketPriceItemCount, pCommand->marketPricePSItems);

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_ORDER, RSSL_FALSE, pCommand->marketByOrderItemCount, pCommand->marketByOrderItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_ORDER, RSSL_TRUE, pCommand->privateStreamMarketByOrderItemCount, pCommand->marketByOrderPSItems);

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_PRICE, RSSL_FALSE, pCommand->marketByPriceItemCount, pCommand->marketByPriceItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_PRICE, RSSL_TRUE, pCommand->privateStreamMarketByPriceItemCount, pCommand->marketByPricePSItems);

	displayCacheDomain(pCommand, RSSL_DMT_YIELD_CURVE, RSSL_FALSE, pCommand->yieldCurveItemCount, pCommand->yieldCurveItems);
	displayCacheDomain(pCommand, RSSL_DMT_YIELD_CURVE, RSSL_TRUE, pCommand->privateStreamYieldCurveItemCount, pCommand->yieldCurvePSItems);

	printf("\nCache Display Complete\n");
}

/*
 * Display contents of cache for a given list of items from a given domain
 */
static void displayCacheDomain(ChannelCommand *pCommand, RsslUInt8 domainType, RsslBool privateStreams, RsslInt32 itemCount, ItemRequest items[])
{
	int i;
	RsslRet ret;
	char tempData[128];
	RsslBuffer tempBuffer;

	printf("Retrieving cache data for %d %s %s items:\n", itemCount,
			rsslDomainTypeToString(domainType),
			privateStreams ? "Private Stream" : "");
	for (i = 0; i < itemCount; i++)
	{
		if (items[i].cacheEntryHandle)
		{
			printf("\n%.*s\n", items[i].itemName.length, items[i].itemName.data);

			tempBuffer.data = tempData;
			tempBuffer.length = sizeof(tempData);
			rsslStateToString(&tempBuffer, &items[i].itemState);
			printf("%.*s\n", tempBuffer.length, tempBuffer.data);

			if ((ret = decodeEntryFromCache(pCommand, items[i].cacheEntryHandle, domainType)) != RSSL_RET_SUCCESS)
				printf("Error decoding cache content: %d\n", ret);
		}
		else
		{
			printf("\n%.*s\tno data in cache\n", items[i].itemName.length, items[i].itemName.data);
		}
	}
}

static RsslRet decodeEntryFromCache(ChannelCommand *pCommand, RsslPayloadEntryHandle cacheEntryHandle, RsslUInt8 domainType)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	RsslCacheError cacheError;
	RsslBuffer buffer;
	RsslLocalFieldSetDefDb localFieldSetDefDb;
	RsslUInt8 majorVersion;
	RsslUInt8 minorVersion;

	buffer.data = _bufferArray;
	buffer.length = sizeof(_bufferArray);

	if (pCommand->reactorChannel)
	{
		majorVersion = pCommand->reactorChannel->majorVersion;
		minorVersion = pCommand->reactorChannel->minorVersion;
	}
	else
	{
		majorVersion = pCommand->cOpts.rsslConnectOptions.majorVersion;
		minorVersion = pCommand->cOpts.rsslConnectOptions.minorVersion;
	}

	rsslClearLocalFieldSetDefDb(&localFieldSetDefDb);

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, majorVersion, minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, &buffer);

	if (rsslPayloadEntryRetrieve(cacheEntryHandle, &eIter, 0, &cacheError) != RSSL_RET_SUCCESS)
	{
		printf("Failed retrieving cache entry.\n\tError (%d): %s\n", cacheError.rsslErrorId, cacheError.text);
		return RSSL_RET_FAILURE;
	}
	else
	{
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorBuffer(&dIter, &buffer);
		rsslSetDecodeIteratorRWFVersion(&dIter, majorVersion, minorVersion);

		switch (domainType)
		{
		case RSSL_DMT_MARKET_PRICE:
			ret = decodeMarketPriceFieldList(&pCommand->dictionary, &dIter);
			break;

		case RSSL_DMT_MARKET_BY_ORDER:
			ret = decodeMarketMap(&pCommand->dictionary, RSSL_DMT_MARKET_BY_ORDER, &dIter);
			break;

		case RSSL_DMT_MARKET_BY_PRICE:
			ret = decodeMarketMap(&pCommand->dictionary, RSSL_DMT_MARKET_BY_PRICE, &dIter);
			break;

		case RSSL_DMT_YIELD_CURVE:
			ret = decodeYieldCurveFieldList(&pCommand->dictionary, &dIter, &localFieldSetDefDb);
			break;

		default:
			break;
		}
		if (ret > RSSL_RET_SUCCESS)
			ret = RSSL_RET_SUCCESS;
	}

	return ret;
}
