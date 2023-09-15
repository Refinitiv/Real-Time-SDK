/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/


/*
 * This is the main file for the rsslVANIProvider application.  It is
 * a single-threaded client application.  The application uses the
 * operating parameters entered by the user.  See readme file for
 * usage.
 *
 * The purpose of this application is to non-interactively provide
 * level 1 market price and level 2 market by order data to an 
 * Advanced Data Hub (ADH).  
 * 
 * The application implements callbacks that process information
 * received by the ADH.  It creates the RsslReactor, creates the
 * desired connections, then dispatches from the RsslReactor for events and
 * messages.  Once it has received the event indicating that the channel is 
 * ready, it begin sending refresh and update messages for the given items.

 * Reliable multicast can be used to communicate between this application and any
 * ADH on the network. Then the non-interactive provider can send one message to
 * all ADH's on the network instead of having to fan-out messages to each ADH
 * TCP/IP connection. 
 *
 * This application is intended as a basic usage example. Some of the design choices
 * were made to favor simplicity and readability over performance. This application
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

#include "rsslNIDirectoryProvider.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslRDMMsg.h"
#include "rsslNIProvider.h"
#include "rsslVASendMessage.h"
#include "rsslNILoginProvider.h"

static RsslInt32 loginStreamId = 1, sourceDirectoryStreamId = -1;
static RsslInt32 startingItemStreamId = -2;


#define MAX_CHAN_COMMANDS 1

/* default server host name */
static const char *defaultSrvrHostname = "localhost";
/* default server port number */
static const char *defaultSrvrPortNo = "14002";
/* default service name */
static const char *defaultServiceName = "DIRECT_FEED";
/* default item name */
static const char *defaultItemName = "TRI";


NIChannelCommand chnlCommand;
RsslInt32 timeToRun = 300;
RsslInt32 maxEventsInPool = 500;
time_t rsslProviderRuntime;
static RsslBool xmlTrace = RSSL_FALSE;
static char traceOutputFile[128];

char proxyHost[256];
char proxyPort[256];
char proxyUserName[128];
char proxyPasswd[128];
char proxyDomain[128];

static char libsslName[255];
static char libcryptoName[255];
static char libcurlName[255];

static char sslCAStore[255];

static RsslBool cacheCommandlineOption = RSSL_FALSE;
RsslVACacheInfo cacheInfo;

static void initializeCache(RsslBool cacheOption);
static void initializeCacheDictionary(RsslDataDictionary* dictionary);
static void uninitializeCache();

/* 
 * Prints example usage and exits. 
 */
void printUsageAndExit(char *appName)
{

	printf("Usage: %s [-tcp|-encrypted|-encryptedSocket|-encryptedHttp [<hostname>:<port> <service name>]] or [-segmentedMulticast [<sendAddress>:<sendPort>:<interfaceName> <recvAddress>:<recvPort> <unicastPort> <service name>]] [<domain>:<item name>,...] [-runtime <seconds>] [-cache]\n"
			"\n -x              specifies that XML tracing should be enabled"
			"\n -uname          specifies the user name"
			"\n -at             Specifies the Authentication Token. If this is present, the login user name type will be RDM_LOGIN_USER_AUTHN_TOKEN"
			"\n -ax             Specifies the Authentication Extended information"
			"\n -tcp 	        specifies a tcp connection to open and a list of items to request:"
			"\n     hostname:        Hostname of provider to connect to"
			"\n     port:            Port of provider to connect to"
			"\n     service:         Name of service to request items from on this connection\n"
			"\n -encryptedSocket specifies an encrypted connection to open.  Host, port, service, and items are the same as -tcp above.\n"
			"\n -encryptedHttp specifies an encrypted WinInet-based Http connection to open.  Host, port, service, and items are the same as -tcp above.  This option is only available on Windows.\n"
			"\n	-segmentedMulticast specifices a multicast connection to open:"
			"\n		sendAddress:	Multicast address that the provider will be sending data to"
			"\n		sendPort:		port on the send multicast address that the provider will be sending data to"
			"\n		interfaceName:	The interface that the prvider will be using.  This is opitional"
			"\n		recvAddress:	Multicast address that the provider will be reading data from"
			"\n		recvPort:		Port on the receive multicast address that the provider will be reading data from"
			"\n		unicastPort:	Unicast port for unicast data\n"
			"\n domain:itemName: Domain and name of an item to request."
			"\n         A comma-separated list of these may be specified."
			"\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)\n"
			"\n     Example Usage: -tcp localhost:14002 DIRECT_FEED mp:TRI,mbo:MSFT\n"
			"\n -runtime adjusts the running time of the application.\n"
			"\n -cache enables cache for the NI provider item payload data\n"
			"\n -aid Specifies the Application ID\n"
			" Options for establishing connection(s) and sending requests through a proxy server:\n"
			"   [ -ph <proxy host> ] [ -pp <proxy port> ] [ -plogin <proxy username> ] [ -ppasswd <proxy password> ] [ -pdomain <proxy domain> ] \n"
			"\n -castore specifies the filename or directory of the OpenSSL CA store\n"
			"\n -maxEventsInPool size of event pool\n"
			, appName);
	exit(-1);
}


void handleConfig(int argc, char **argv, NIChannelCommand *pCommand)
{
	RsslInt32 i;
	RsslInt32 streamId = -2;


    /* Check usage and retrieve operating parameters */
	i = 1;

	snprintf(proxyHost, sizeof(proxyHost), "%s", "");
	snprintf(proxyPort, sizeof(proxyPort), "%s", "");
	snprintf(proxyUserName, sizeof(proxyUserName), "%s", "");
	snprintf(proxyPasswd, sizeof(proxyPasswd), "%s", "");
	snprintf(proxyDomain, sizeof(proxyDomain), "%s", "");

	snprintf(libcryptoName, sizeof(libcryptoName), "%s", "");
	snprintf(libsslName, sizeof(libsslName), "%s", "");
	snprintf(libcurlName, sizeof(libcurlName), "%s", "");
	snprintf(sslCAStore, sizeof(sslCAStore), "%s", "");

	while(i < argc)
	{
		if (strcmp("-?", argv[i]) == 0)
		{
			printUsageAndExit(argv[0]);
		}
		if (strcmp("-libsslName", argv[i]) == 0)
		{
			i += 2;
			snprintf(libsslName, 255, "%s", argv[i - 1]);
		}
		else if (strcmp("-libcryptoName", argv[i]) == 0)
		{
			i += 2;
			snprintf(libcryptoName, 255, "%s", argv[i - 1]);
		}
		else if (strcmp("-libcurlName", argv[i]) == 0)
		{
			i += 2;
			snprintf(libcurlName, 255, "%s", argv[i - 1]);
		}
		else if (strcmp("-castore", argv[i]) == 0)
		{
			i += 2;
			snprintf(sslCAStore, 255, "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.encryptionOpts.openSSLCAStore = sslCAStore;
		}
		else if(strcmp("-uname", argv[i]) == 0)
		{
			i += 2;
			snprintf(pCommand->username.data, MAX_BUFFER_LENGTH, "%s", argv[i-1]);
			pCommand->username.length = (RsslUInt32)strlen(pCommand->username.data);
		}
		else if(strcmp("-at", argv[i]) == 0)
		{
			i += 2;
			snprintf(pCommand->authenticationToken.data, MAX_AUTHN_LENGTH, "%s", argv[i-1]);
			pCommand->authenticationToken.length = (RsslUInt32)strlen(pCommand->authenticationToken.data);
		}
		else if(strcmp("-ax", argv[i]) == 0)
		{
			i += 2;
			snprintf(pCommand->authenticationExtended.data, MAX_AUTHN_LENGTH, "%s", argv[i-1]);
			pCommand->authenticationExtended.length = (RsslUInt32)strlen(pCommand->authenticationExtended.data);
		}
		else if(strcmp("-aid", argv[i]) == 0)
		{
			i += 2;
			snprintf(pCommand->applicationId.data, MAX_BUFFER_LENGTH, "%s", argv[i-1]);
			pCommand->applicationId.length = (RsslUInt32)strlen(pCommand->applicationId.data);
		}
		else if (strcmp("-ph", argv[i]) == 0)
		{
			i += 2;
			snprintf(proxyHost, sizeof(proxyHost), "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.proxyOpts.proxyHostName = proxyHost;
		}
		else if (strcmp("-pp", argv[i]) == 0)
		{
			i += 2;
			snprintf(proxyPort, sizeof(proxyPort), "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.proxyOpts.proxyPort = proxyPort;
		}
		else if (strcmp("-plogin", argv[i]) == 0)
		{
			i += 2;
			snprintf(proxyUserName, sizeof(proxyUserName), "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.proxyOpts.proxyUserName = proxyUserName;
		}
		else if (strcmp("-ppasswd", argv[i]) == 0)
		{
			i += 2;
			snprintf(proxyPasswd, sizeof(proxyPasswd), "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.proxyOpts.proxyPasswd = proxyPasswd;
		}
		else if (strcmp("-pdomain", argv[i]) == 0)
		{
			i += 2;
			snprintf(proxyDomain, sizeof(proxyDomain), "%s", argv[i - 1]);
			pCommand->cOpts.rsslConnectOptions.proxyOpts.proxyDomain = proxyDomain;
		}
		else if(strcmp("-x", argv[i]) == 0)
		{
			i++;
			xmlTrace = RSSL_TRUE;
			snprintf(traceOutputFile, 128, "RsslNIVAProvider");
		}
		else if(strcmp("-maxEventsInPool", argv[i]) == 0)
		{
			i += 2;
			maxEventsInPool = atoi(argv[i-1]);
		}
		else if (strcmp("-tcp", argv[i]) == 0)
		{

			char *pToken, *pToken2, *pSaveToken, *pSaveToken2;
				
				
			/* Syntax:
				*  -tcp hostname:port SERVICE_NAME mp:TRI,mp:.DJI
				*/
				

			/* Hostname */
			if (++i >= argc) { printf("Error: -tcp: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok(argv[i], ":");
			if (!pToken) { printf("Error: -tcp: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->hostName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->hostName.length = (RsslUInt32)strlen(pCommand->hostName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName.data;

			/* Port */
			pToken = strtok(NULL, ":");
			if (!pToken) { printf("Error: -tcp: Missing serviceName.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->port.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->port.length = (RsslUInt32)strlen(pCommand->port.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port.data;

			/* Item Service Name */
			i++;
			pToken = argv[i];
			if (!pToken) { printf("Error: -tcp: Missing item service name.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->serviceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->serviceName.length = (RsslUInt32)strlen(pCommand->serviceName.data);

			/* Item List */
			if (++i >= argc) { printf("Error: -tcp: Missing item.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok_r(argv[i], ",", &pSaveToken);

			while(pToken)
			{
				RsslNIItemInfo *pItemInfo;
				/* domain */
				pToken2 = strtok_r(pToken, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -tcp: Missing item.\n"); printUsageAndExit(argv[0]); }

				if (0 == strcmp(pToken2, "mp"))
				{
					if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketPriceItemInfo[pCommand->marketPriceItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_PRICE;
						++pCommand->marketPriceItemCount;
					}
					else
					{
						printf("Number of items for Market Price domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else if (0 == strcmp(pToken2, "mbo"))
				{
					if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketByOrderItemInfo[pCommand->marketByOrderItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_BY_ORDER;
						++pCommand->marketByOrderItemCount;
					}
					else
					{
						printf("Number of items for Market By Order domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else
				{
					printf("Unknown item domain: %s\n", pToken2);
					printUsageAndExit(argv[0]);
				}

				/* name */
				pToken2 = strtok_r(NULL, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -tcp: Missing item name.\n"); printUsageAndExit(argv[0]); }
				snprintf(pItemInfo->Itemname, 128, "%s", pToken2);
					
				pItemInfo->isActive = RSSL_TRUE;

				pItemInfo->streamId = streamId--;

				pToken = strtok_r(NULL, ",", &pSaveToken);
			}
				
			i++;
		}
		else if (strcmp("-encryptedSocket", argv[i]) == 0)
		{

			char *pToken, *pToken2, *pSaveToken, *pSaveToken2;


			/* Syntax:
			*  -encryptedSocket hostname:port SERVICE_NAME mp:TRI,mp:.DJI
			*/
			pCommand->cOpts.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			pCommand->cOpts.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;

			/* Hostname */
			if (++i >= argc) { printf("Error: -encryptedSocket: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok(argv[i], ":");
			if (!pToken) { printf("Error: -encryptedSocket: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->hostName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->hostName.length = (RsslUInt32)strlen(pCommand->hostName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName.data;

			/* Port */
			pToken = strtok(NULL, ":");
			if (!pToken) { printf("Error: -encryptedSocket: Missing serviceName.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->port.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->port.length = (RsslUInt32)strlen(pCommand->port.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port.data;

			/* Item Service Name */
			i++;
			pToken = argv[i];
			if (!pToken) { printf("Error: -encryptedSocket: Missing item service name.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->serviceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->serviceName.length = (RsslUInt32)strlen(pCommand->serviceName.data);

			/* Item List */
			if (++i >= argc) { printf("Error: -encryptedSocket: Missing item.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok_r(argv[i], ",", &pSaveToken);

			while (pToken)
			{
				RsslNIItemInfo *pItemInfo;
				/* domain */
				pToken2 = strtok_r(pToken, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -encryptedSocket: Missing item.\n"); printUsageAndExit(argv[0]); }

				if (0 == strcmp(pToken2, "mp"))
				{
					if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketPriceItemInfo[pCommand->marketPriceItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_PRICE;
						++pCommand->marketPriceItemCount;
					}
					else
					{
						printf("Number of items for Market Price domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else if (0 == strcmp(pToken2, "mbo"))
				{
					if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketByOrderItemInfo[pCommand->marketByOrderItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_BY_ORDER;
						++pCommand->marketByOrderItemCount;
					}
					else
					{
						printf("Number of items for Market By Order domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else
				{
					printf("Unknown item domain: %s\n", pToken2);
					printUsageAndExit(argv[0]);
				}

				/* name */
				pToken2 = strtok_r(NULL, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -encryptedSocket: Missing item name.\n"); printUsageAndExit(argv[0]); }
				snprintf(pItemInfo->Itemname, 128, "%s", pToken2);

				pItemInfo->isActive = RSSL_TRUE;

				pItemInfo->streamId = streamId--;

				pToken = strtok_r(NULL, ",", &pSaveToken);
			}

			i++;
		}
		else if (strcmp("-encryptedHttp", argv[i]) == 0)
		{

			char *pToken, *pToken2, *pSaveToken, *pSaveToken2;
#ifdef LINUX
			printf("Error: -encryptedHttp: WinInet HTTP connection not supported on Linux.\n"); 
			printUsageAndExit(argv[0]);
#endif

			/* Syntax:
			*  -encryptedSocket hostname:port SERVICE_NAME mp:TRI,mp:.DJI
			*/
			pCommand->cOpts.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			pCommand->cOpts.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_HTTP;

			/* Hostname */
			if (++i >= argc) { printf("Error: -encryptedSocket: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok(argv[i], ":");
			if (!pToken) { printf("Error: -encryptedSocket: Missing hostname.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->hostName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->hostName.length = (RsslUInt32)strlen(pCommand->hostName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName.data;

			/* Port */
			pToken = strtok(NULL, ":");
			if (!pToken) { printf("Error: -encryptedSocket: Missing serviceName.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->port.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->port.length = (RsslUInt32)strlen(pCommand->port.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port.data;

			/* Item Service Name */
			i++;
			pToken = argv[i];
			if (!pToken) { printf("Error: -encryptedSocket: Missing item service name.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->serviceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->serviceName.length = (RsslUInt32)strlen(pCommand->serviceName.data);

			/* Item List */
			if (++i >= argc) { printf("Error: -encryptedSocket: Missing item.\n"); printUsageAndExit(argv[0]); }
			pToken = strtok_r(argv[i], ",", &pSaveToken);

			while (pToken)
			{
				RsslNIItemInfo *pItemInfo;
				/* domain */
				pToken2 = strtok_r(pToken, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -encryptedSocket: Missing item.\n"); printUsageAndExit(argv[0]); }

				if (0 == strcmp(pToken2, "mp"))
				{
					if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketPriceItemInfo[pCommand->marketPriceItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_PRICE;
						++pCommand->marketPriceItemCount;
					}
					else
					{
						printf("Number of items for Market Price domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else if (0 == strcmp(pToken2, "mbo"))
				{
					if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
					{
						pItemInfo = &pCommand->marketByOrderItemInfo[pCommand->marketByOrderItemCount];
						pItemInfo->domainType = RSSL_DMT_MARKET_BY_ORDER;
						++pCommand->marketByOrderItemCount;
					}
					else
					{
						printf("Number of items for Market By Order domain exceeded CHAN_CMD_MAX_ITEMS (%d)\n", CHAN_CMD_MAX_ITEMS);
						printUsageAndExit(argv[0]);
					}
				}
				else
				{
					printf("Unknown item domain: %s\n", pToken2);
					printUsageAndExit(argv[0]);
				}

				/* name */
				pToken2 = strtok_r(NULL, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -encryptedSocket: Missing item name.\n"); printUsageAndExit(argv[0]); }
				snprintf(pItemInfo->Itemname, 128, "%s", pToken2);

				pItemInfo->isActive = RSSL_TRUE;

				pItemInfo->streamId = streamId--;

				pToken = strtok_r(NULL, ",", &pSaveToken);
			}

			i++;
		}
		else if (strcmp("-segmentedMulticast", argv[i]) == 0)
		{


			char *pToken, *pToken2, *pSaveToken, *pSaveToken2;
				
			/* Syntax:
				*  -segmentedmulticast hostname:port SERVICE_NAME mp:TRI,mp:.DJI
				*/
				 
				pCommand->cOpts.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;

			/* Hostname */
			i++;
			pToken = strtok(argv[i], ":");
			if (!pToken) { printf("Error: -segmentedMulticast: Missing sendAddress.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->hostName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->hostName.length = (RsslUInt32)strlen(pCommand->hostName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.sendAddress = pCommand->hostName.data;

			/* Port */
			pToken = strtok(NULL, ":");
			if (!pToken) { printf("Error: -segmentedMulticast: Missing sendPort.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->port.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->port.length = (RsslUInt32)strlen(pCommand->port.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.sendServiceName = pCommand->port.data;
				
			/* Interface name */
			pToken = strtok(NULL, ":");
			if (!pToken)
			{
				pCommand->interfaceName.length = 0;
				pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.interfaceName = NULL;
			}
			else
			{
				snprintf(pCommand->interfaceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
				pCommand->interfaceName.length = (RsslUInt32)strlen(pCommand->interfaceName.data);
				pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.interfaceName = pCommand->interfaceName.data;
			}

			/* Recv Hostname */
			i++;
			pToken = strtok(argv[i], ":");
			if (!pToken) { printf("Error: -segmentedMulticast: Missing recvAddress.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->recvHostName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->recvHostName.length = (RsslUInt32)strlen(pCommand->recvHostName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.recvAddress = pCommand->recvHostName.data;
				
			/* Recv Port */
			pToken = strtok(NULL, ":");
			if (!pToken) { printf("Error: -segmentedMulticast: Missing recvPort.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->recvPort.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->recvPort.length = (RsslUInt32)strlen(pCommand->recvPort.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.recvServiceName = pCommand->recvPort.data;
				
			i++;
			pToken = argv[i];
			if (!pToken) { printf("Error: -segmentedMulticast: Missing unicast Port.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->unicastServiceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->unicastServiceName.length = (RsslUInt32)strlen(pCommand->unicastServiceName.data);
			pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.unicastServiceName = pCommand->unicastServiceName.data;
				

			/* Item Service Name */
			i++;
			pToken = argv[i];
			if (!pToken) { printf("Error: -segmentedMulticast: Missing Service Name.\n"); printUsageAndExit(argv[0]); }
			snprintf(pCommand->serviceName.data, MAX_BUFFER_LENGTH, "%s", pToken);
			pCommand->serviceName.length = (RsslUInt32)strlen(pCommand->serviceName.data);

			/* Item List */
			i++;
			pToken = strtok_r(argv[i], ",", &pSaveToken);
			if (!pToken) { printf("Error: -segmentedMulticast: Missing Items.\n"); printUsageAndExit(argv[0]); }

			while(pToken)
			{
				RsslNIItemInfo *pItemInfo;
				/* domain */
				pToken2 = strtok_r(pToken, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -segmentedMulticast: Missing Items.\n"); printUsageAndExit(argv[0]); }

				if (0 == strcmp(pToken2, "mp") && pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
				{
					pItemInfo = &pCommand->marketPriceItemInfo[pCommand->marketPriceItemCount];
					pItemInfo->domainType = RSSL_DMT_MARKET_PRICE;
					++pCommand->marketPriceItemCount;
				}
				else if (0 == strcmp(pToken2, "mbo") && pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
				{
					pItemInfo = &pCommand->marketByOrderItemInfo[pCommand->marketByOrderItemCount];
					pItemInfo->domainType = RSSL_DMT_MARKET_BY_ORDER;
					++pCommand->marketByOrderItemCount;
				}
				else
				{
					printf("Unknown item domain: %s\n", pToken2);
					printUsageAndExit(argv[0]);
				}

				/* name */
				pToken2 = strtok_r(NULL, ":", &pSaveToken2);
				if (!pToken2) { printf("Error: -segmentedMulticast: Missing Item name.\n"); printUsageAndExit(argv[0]); }
				snprintf(pItemInfo->Itemname, 128, "%s", pToken2);
					
				pItemInfo->isActive = RSSL_TRUE;

				pItemInfo->streamId = streamId--;

				pToken = strtok_r(NULL, ",", &pSaveToken);
					
			}
				
			i++;
				
			printf("sendAddress: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.sendAddress);
			printf("SendPort: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.sendServiceName);
			printf("RecvAddress: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.recvAddress);
			printf("RecvPort: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.recvServiceName);
			printf("UnicastPort: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.unicastServiceName);
			printf("InterfaceName: %s\n", pCommand->cOpts.rsslConnectOptions.connectionInfo.segmented.interfaceName);
		}
		else if(strcmp("-runtime", argv[i]) == 0)
		{
			i += 2;
			timeToRun = atoi(argv[i-1]);
		}
		else if (strcmp("-cache", argv[i]) == 0)
		{
			i += 1;
			cacheCommandlineOption = RSSL_TRUE;
		}
		else
		{
			printf("Error: Unrecognized option: %s\n\n", argv[i]);
			printUsageAndExit(argv[0]);
		}

	}
}

fd_set readFds, writeFds, exceptFds;

RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	NIChannelCommand *chanCommand;
	
#ifdef _WIN32
			int rcvBfrSize = 65535;
			int sendBfrSize = 65535;
			RsslErrorInfo rsslErrorInfo;
#endif

	chanCommand = (NIChannelCommand*)pReactorChannel->userSpecPtr;

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
			printf("Connection up!\n");
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);
			chanCommand->reactorChannel = pReactorChannel;

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
				RsslErrorInfo rsslErrorInfo;
				RsslTraceOptions traceOptions;

				rsslClearTraceOptions(&traceOptions);
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			break;
		case RSSL_RC_CET_WARNING:
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("	Error text: %s\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		case RSSL_RC_CET_FD_CHANGE:
			printf("Fd change: "SOCKET_PRINT_TYPE" to "SOCKET_PRINT_TYPE"\n", pReactorChannel->oldSocketId, pReactorChannel->socketId);
			FD_CLR(pReactorChannel->oldSocketId, &readFds);
			FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);
			break;
		case RSSL_RC_CET_CHANNEL_DOWN:
			printf("Connection down: Channel fd="SOCKET_PRINT_TYPE" Requesting reconnect.\n", pReactorChannel->socketId);
			chanCommand = (NIChannelCommand*)pReactorChannel->userSpecPtr;
			recoverConnection(pReactor, pReactorChannel, chanCommand);
			break;
		case RSSL_RC_CET_CHANNEL_READY:
			printf("Connection is ready, starting publishing\n");
			chanCommand->startWrite = RSSL_TRUE;
			break;
		default:
			printf("Unknown connection event!\n");
			return RSSL_RC_CRET_SUCCESS;
	}

	if (pConnEvent->pError)
		printf("	Error text: %s\n", pConnEvent->pError->rsslError.text);
		
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactor *pReactor = 0;


/*
 * Processes a response from a channel.  This consists of
 * performing a high level decode of the message and then
 * calling the applicable specific function for further
 * processing.
 * chnl - The channel of the response
 * buffer - The message buffer containing the response
 */
static RsslReactorCallbackRet processResponse(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pEvent)
{
	NIChannelCommand* chnlCommand = (NIChannelCommand*)pChannel->userSpecPtr;
	
	if(!pEvent->pRsslMsg)
	{
		printf("processResponse error: %s(%s)\n", pEvent->pErrorInfo->rsslError.text, pEvent->pErrorInfo->errorLocation);
		recoverConnection(pReactor, pChannel, chnlCommand);
		return RSSL_RC_CRET_SUCCESS;
	}

	printf("Unhandled Domain Type: %d\n", pEvent->pRsslMsg->msgBase.domainType);

	return RSSL_RC_CRET_SUCCESS;
}

void recoverConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, NIChannelCommand *pCommand)
{
	time_t currentTime;
	RsslErrorInfo rsslErrorInfo;
	RsslUInt i;
	time(&currentTime);
	pCommand->reconnect = RSSL_TRUE;

	if (pChannel->socketId != REACTOR_INVALID_SOCKET)
	{
		FD_CLR(pChannel->socketId, &readFds);
		FD_CLR(pChannel->socketId, &exceptFds);
	}

	if (rsslReactorCloseChannel(pReactor, pChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %s\n", rsslErrorInfo.rsslError.text);
	}
	rsslClearRDMLoginRefresh(&pCommand->loginRefresh);
	pCommand->loginRefreshBuffer.data = pCommand->loginRefreshArray;
	pCommand->loginRefreshBuffer.length = LOGIN_ARRAY_LENGTH;

	for(i = 0; i < pCommand->marketPriceItemCount; i++)
	{
		pCommand->marketPriceItemInfo[i].IsRefreshComplete = RSSL_FALSE;
	}

	for(i = 0; i < pCommand->marketByOrderItemCount; i++)
	{
		pCommand->marketByOrderItemInfo[i].IsRefreshComplete = RSSL_FALSE;
	}

	pCommand->startWrite = RSSL_FALSE;

	pCommand->canSendLoginReissue = RSSL_FALSE;
}


/*** MAIN ***/
int main(int argc, char **argv)
{
	RsslErrorInfo error;
	RsslError rsslErr;
	RsslCreateReactorOptions reactorOpts;
	RsslReactorDispatchOptions dispatchOpts;
	RsslInt32 j;

	RsslReactorOMMNIProviderRole role;
	RsslDataDictionary dictionary = RSSL_INIT_DATA_DICTIONARY;
	
	char err[128];
	RsslBuffer errBuf = {128, &err[0]};
	
	int selRet;

	time_t currentTime = 0;
	RsslRet ret;

	RsslInitializeExOpts initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	rsslInitNIChannelCommand(&chnlCommand);

	handleConfig(argc, argv, &chnlCommand);

	initializeCache(cacheCommandlineOption);
	
	initOpts.jitOpts.libcryptoName = libcryptoName;
	initOpts.jitOpts.libsslName = libsslName;
	initOpts.jitOpts.libcurlName = libcurlName;
	initOpts.rsslLocking = RSSL_LOCK_GLOBAL_AND_CHANNEL;

	/* Initialize run-time */
	initRuntime();

	rsslInitializeEx(&initOpts, &rsslErr);

	/* Setup role */
	rsslClearOMMNIProviderRole((RsslReactorOMMNIProviderRole*)&role);
	role.pLoginRequest = (RsslRDMLoginRequest*)&chnlCommand.loginRequest;
	role.loginMsgCallback = processLoginResponse;
	role.pDirectoryRefresh = &chnlCommand.directoryRefresh;
	role.base.channelEventCallback = channelEventCallback;
	role.base.defaultMsgCallback = processResponse;

	printf("Connections:\n");

	chnlCommand.pRole = (RsslReactorChannelRole*)&role;
	chnlCommand.cOpts.rsslConnectOptions.guaranteedOutputBuffers = 500;
	chnlCommand.cOpts.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	chnlCommand.cOpts.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	chnlCommand.cOpts.rsslConnectOptions.userSpecPtr = (void*)&chnlCommand;

//	printf("	%s:%s:%s\n", chnlCommand.hostName, chnlCommand.port, chnlCommand.serviceName);

	printf("		MarketPriceItems:");
	for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
	{
		RsslNIItemInfo *pItem = &chnlCommand.marketPriceItemInfo[j];
		if (pItem->isActive)
		printf(" %.*s", (int)sizeof(pItem->Itemname), pItem->Itemname);
	}

	printf("		MarketByOrderItems:");
	for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
	{
		RsslNIItemInfo *pItem = &chnlCommand.marketByOrderItemInfo[j];
		if (pItem->isActive)
		printf(" %.*s", (int)sizeof(pItem->Itemname), pItem->Itemname);
	}


	printf("\n");

	printf("\n");
	
	/* Load local dictionary */
	if(rsslLoadFieldDictionary("RDMFieldDictionary", &dictionary, &errBuf) < RSSL_RET_SUCCESS)
	{
		printf("Dictionary error: %s\n", errBuf.data);
		exit(-1);
	}
	
	chnlCommand.dictionary = &dictionary;
	
	initializeCacheDictionary(&dictionary);

	rsslClearCreateReactorOptions(&reactorOpts);
	reactorOpts.dispatchDecodeMemoryBufferSize = 1024;
	reactorOpts.maxEventsInPool = maxEventsInPool;
	
	setupLoginRequest(&chnlCommand, 1);

	setupDirectoryResponseMsg(&chnlCommand, -1);
	
	if (!(pReactor = rsslCreateReactor(&reactorOpts, &error)))
	{
		printf("Error: %s", error.rsslError.text);
		exit(-1);
	}
	
	

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	FD_SET(pReactor->eventFd, &readFds);

	if (rsslReactorConnect(pReactor, &chnlCommand.cOpts, chnlCommand.pRole, &error) != RSSL_RET_SUCCESS)
		printf("Error rsslReactorConnect(): %s\n", error.rsslError.text);
		
	rsslClearReactorDispatchOptions(&dispatchOpts);
	//dispatchOpts.pReactorChannel = &chnlCommand.reactorChannel;

	printf("Connection started!\n");

	while ( RSSL_TRUE )
	{
		struct timeval selectTime;
		int dispatchCount = 0;
		fd_set useReadFds = readFds, useWriteFds = writeFds, useExceptFds = exceptFds;
		selectTime.tv_sec = 1; selectTime.tv_usec = 0;

		handleRuntime();

		selRet = select(FD_SETSIZE, &useReadFds, &useWriteFds, &useExceptFds, &selectTime);

		if (selRet == 0 ) /* no messages received, send updates and continue */
		{
			// Update items
			updateItemInfo(&chnlCommand);

			/* Send market price updates for each connected channel */
			if (chnlCommand.startWrite == RSSL_TRUE)
			{
				if ((chnlCommand.reactorChannel != NULL) && (chnlCommand.reactorChannel->socketId != -1))
				{
					if (sendItemUpdates(pReactor,chnlCommand.reactorChannel) != RSSL_RET_SUCCESS)
						recoverConnection(pReactor, chnlCommand.reactorChannel, &chnlCommand);
				}
			}
			else
			{
				if (cacheInfo.useCache)
					cacheItemData(&chnlCommand);
			}
		}
		else if (selRet > 0)
		{
			RsslRet ret;
			while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &error)) > 0);

			if(ret < 0)
			{
				printf("dispatch error! %i\n %s\n %s\n", ret, error.rsslError.text, error.errorLocation);
				/* Reactor has shutdown. Clean up and exit. */
				cleanUpAndExit();
			}
		}
	
		/* get current time */
		if ((currentTime = time(NULL)) < 0)
		{
			printf("time() failed.\n");
		}

		// send login reissue if login reissue time has passed
		if (chnlCommand.canSendLoginReissue == RSSL_TRUE &&
			currentTime >= (RsslInt)chnlCommand.loginReissueTime)
		{
			RsslReactorSubmitMsgOptions submitMsgOpts;
			RsslErrorInfo rsslErrorInfo;

			rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
			submitMsgOpts.pRDMMsg = (RsslRDMMsg*)chnlCommand.pRole->ommNIProviderRole.pLoginRequest;
			if ((ret = rsslReactorSubmitMsg(pReactor,chnlCommand.reactorChannel,&submitMsgOpts,&rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("Login reissue failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			}
			else
			{
				printf("Login reissue sent\n");
			}
			chnlCommand.canSendLoginReissue = RSSL_FALSE;
		}
	}
}

/*
 * Initializes the run-time for the rsslVANIProvider.
 */
static void initRuntime()
{
	time_t currentTime = 0;
	
	/* get current time */
	time(&currentTime);

	rsslProviderRuntime = currentTime + (time_t)timeToRun;

}

/*
 * Handles the run-time for the rsslVANIProvider.  Closes all streams
 * for the NIP after run-time has elapsed.
 */
static RsslBool shutdownCalled = RSSL_FALSE;
static void handleRuntime()
{
	RsslErrorInfo error;
	time_t currentTime = 0;
	RsslRet	retval = 0;

	/* get current time */
	time(&currentTime); 

	if(chnlCommand.reconnect == RSSL_TRUE)
	{
		chnlCommand.reconnect = RSSL_FALSE;
		printf("Reconnecting in 10 seconds...\n");
		chnlCommand.timeToReconnect = 10;
	}

	if (chnlCommand.timeToReconnect > 0)
	{
		--chnlCommand.timeToReconnect;

		if (chnlCommand.timeToReconnect == 0)
		{
			if (rsslReactorConnect(pReactor, &chnlCommand.cOpts, chnlCommand.pRole, &error) != RSSL_RET_SUCCESS)
			{
				printf("Error rsslReactorConnect(): %s\n", error.rsslError.text);
				chnlCommand.reconnect = RSSL_TRUE;
			}
		}
	}

	if(currentTime > rsslProviderRuntime)
	{
		if( chnlCommand.reactorChannel != NULL && rsslReactorCloseChannel(pReactor, chnlCommand.reactorChannel, &error) != RSSL_RET_SUCCESS)
		{
			printf("Error rsslReactorCloseChannel(): %s\n", error.rsslError.text);
		}

		cleanUpAndExit();
	}

}

/*
 * Initialization for cache use
 */
static void initializeCache(RsslBool cacheOption)
{
	RsslRet ret;

	if (cacheOption)
	{
		if ((ret = rsslPayloadCacheInitialize()) != RSSL_RET_SUCCESS)
		{
			printf("rsslPayloadCacheInitialize() failed: %d (%s). Cache will be disabled.", ret, rsslRetCodeToString(ret));
			cacheInfo.useCache = RSSL_FALSE;
			return;
		}

		cacheInfo.useCache = RSSL_TRUE;
		cacheInfo.cacheOptions.maxItems = 1000;
		rsslCacheErrorClear(&cacheInfo.cacheErrorInfo);
		snprintf(cacheInfo.cacheDictionaryKey, sizeof(cacheInfo.cacheDictionaryKey), "cacheDictionary%d", 1);

		cacheInfo.cacheHandle = rsslPayloadCacheCreate(&cacheInfo.cacheOptions,
				&cacheInfo.cacheErrorInfo);
		if (cacheInfo.cacheHandle == 0)
		{
			printf("Error: Failed to create cache.\n\tError (%d): %s\n",
					cacheInfo.cacheErrorInfo.rsslErrorId, cacheInfo.cacheErrorInfo.text);
			cacheInfo.useCache = RSSL_FALSE;
		}
	}
	else
	{
		cacheInfo.useCache = RSSL_FALSE;
		cacheInfo.cacheHandle = 0;
	}
}

/*
 * Bind the application dictionary to the cache
 */
static void initializeCacheDictionary(RsslDataDictionary* dictionary)
{
	if (cacheInfo.useCache && cacheInfo.cacheHandle)
	{
		if (dictionary)
		{
			if (rsslPayloadCacheSetDictionary(cacheInfo.cacheHandle, dictionary, cacheInfo.cacheDictionaryKey, &cacheInfo.cacheErrorInfo)
					!= RSSL_RET_SUCCESS)
			{
				printf("Error: Failed to bind RDM Field dictionary to cache.\n\tError (%d): %s\n",
						cacheInfo.cacheErrorInfo.rsslErrorId, cacheInfo.cacheErrorInfo.text);
				cacheInfo.useCache = RSSL_FALSE;
			}
		}
		else
		{
			printf("Error: No RDM Field dictionary for cache.\n");
			cacheInfo.useCache = RSSL_FALSE;
		}
	}
}

/*
 * cleanup cache prior to shutdown
 */
static void uninitializeCache()
{
	if (cacheInfo.cacheHandle)
		rsslPayloadCacheDestroy(cacheInfo.cacheHandle);
	cacheInfo.cacheHandle = 0;

	rsslPayloadCacheUninitialize();
}

/*
 * Access to the cache info for the NI provider application
 */
RsslVACacheInfo* getCacheInfo()
{
	return &cacheInfo;
}


/*
 * Cleans up and exits the application.
 */
void cleanUpAndExit()
{
	RsslErrorInfo error;

	printf("Exiting.\n");

	rsslDestroyReactor(pReactor, &error);

	uninitializeCache();

	rsslCleanupNIChannelCommand(&chnlCommand);

	rsslUninitialize();
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(0);
}

