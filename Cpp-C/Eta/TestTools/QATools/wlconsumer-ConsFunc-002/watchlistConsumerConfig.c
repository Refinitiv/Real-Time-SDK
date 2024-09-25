/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2023 LSEG. All rights reserved.     
*/

/*
 * This file handles configuration of the rsslWatchlistConsumer application.
 */

#include "watchlistConsumerConfig.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

WatchlistConsumerConfig watchlistConsumerConfig;

/* Defaults */
const char *defaultHostName		= "localhost";
const char *defaultPort 		= "14002";
RsslBuffer defaultServiceName	= { 11, (char*)"DIRECT_FEED" };
RsslBool xmlTrace = RSSL_FALSE;


/* Adds an item to the consumer's configured list of items to request.
 * Assumes itemName is a permanent string(usually should be taken directly from argv). */
static void addItem(char *itemName, RsslUInt8 domainType, RsslBool symbolListData)
{
	ItemInfo *pItem;

	if (watchlistConsumerConfig.itemCount == MAX_ITEMS)
	{
		printf("Config Error: Example only supports up to %d items.\n", MAX_ITEMS);
		exit(-1);
	}

	pItem = &watchlistConsumerConfig.itemList[watchlistConsumerConfig.itemCount];

	/* Copy item name. */
	pItem->name.length = (RsslUInt32)strlen(itemName);
	pItem->name.data = pItem->_nameMem;
	snprintf(pItem->name.data, MAX_ITEM_NAME_LEN, "%s", itemName);

	pItem->domainType = domainType;
	pItem->streamId = ITEMS_MIN_STREAM_ID + watchlistConsumerConfig.itemCount;
	pItem->symbolListData = (domainType == RSSL_DMT_SYMBOL_LIST) ? symbolListData : RSSL_FALSE;

	++watchlistConsumerConfig.itemCount;
}

void printUsageAndExit(int argc, char **argv)
{
	printf("Usage: %s"
		" or %s [-c <Connection Type> ] [-ec <encrypted protocol> ] [-if <Interface Name>] [ -u <Login UserName> ] [ -passwd <Login password> ] [ -clientId <Client ID> ] [ -sessionMgnt ] [ -takeExclusiveSignOnControl <true/false> ] [ -l <Location name> ] [ -query ] [-s <ServiceName>] [ -mp <MarketPrice ItemName> ] [ -mbo <MarketByOrder ItemName> ] [ -mbp <MarketByPrice ItemName> ] [ -yc <YieldCurve ItemName> ] [ -sl <SymbolList ItemName> ] [ -view ] [-x] [ -runTime <TimeToRun> ] [-rtt]\n"
			" -c           Specifies connection type. Valid arguments are socket, webSocket, http, encrypted, and reliableMCast.\n"
			" -ec          Specifies the encrypted transport protocol. Valid arguments are socket, webSocket, and http.  Http is only supported on Windows Platforms.\n"
			" -if          Specifies the address of a specific network interface to use.\n"
			" -clientId    Specifies an unique ID for application making the request to RDP token service, or the client Id for LSEG login version 2 (mandatory).\n"
			" -clientSecret Specifies the client secret associated with the client id.\n"
			" -jwkFile		Specifies the file location of the private JWK for use with V2 JWT authentication. \n"
			" -audience		Specifies the audience claim for V2 JWT authentication.\n"
			" -sessionMgnt Enables session management in the Reactor.\n"
			" -l           Specifies a location to get an endpoint from service endpoint information. Defaults to us-east.\n"
			" -takeExclusiveSignOnControl Specifies true or false to set the exclusive sign on control to force sign-out for the same credentials.\n"
			" -query       Quries RDP service discovery to get an endpoint according the specified connection type and location.\n"
			" -mp          For each occurance, requests item using Market Price domain.\n"
			" -mbo         For each occurance, requests item on the Market By Order domain.\n"
			" -mbp         For each occurance, requests item on the Market By Price domain.\n"
			" -yc          For each occurance, requests item on the Yield Curve domain.\n"
			" -sl          For each occurance, requests item on the Symbol List domain.\n"
			" -sld         For each occurance, requests item on the Symbol List domain and data streams for items on that list.\n"
			" -post        Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream)\n"
			" -offpost     Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n"
			" -x           Enables tracing of messages sent to and received from the channel.\n"
			" -runTime     Adjusts the running time of the application.\n"
			" -at	       Specifies the Authentication Token. If this is present, the login user name type will be RDM_LOGIN_USER_AUTHN_TOKEN.\n"
			" -ax          Specifies the Authentication Extended information.\n"
			" -aid	       Specifies the Application ID.\n"
			" -pl	       Specifies list of supported websocket sub-protocols, white space or ',' delineated.\n"
			" -spTLSv1.2 Specifies that TLSv1.2 can be used for an OpenSSL-based encrypted connection\n"
			" -spTLSv1.3 Specifies that TLSv1.3 can be used for an OpenSSL-based encrypted connection\n"
			"\n"
			" Connection options for socket, http, and encrypted connection types:\n"
			"   [ -h <Server Hostname> ] [ -p <Port> ]\n"
			" Options for specifying starting and standby providers for Warm standby feature:\n"
			"   [ -startingHostName <Starting Server Hostname> ] [ -startingPort <Starting Server Port> ] [ -standbyHostName <Standby Server Hostname> ] [ -standbyPort <Standby Server Port> ] [ -warmStandbyMode <login/service> ]\n"
			"\n"
			" Connection options for the reliable multicast connection type; all must be specified:\n"
			"   [ -sa <Send Address> ] [ -ra <Receive Address> ] [ -sp <Send Port> ] [ -rp <Receive Port> ] [ -up <Unicast Port> ]\n"
			"\n"
			" Options for publishing Host Stat Message options on reliable multicast connections; -hsmAddr and -hsmPort must be specified to enable:\n"
			"   [ -hsmAddr <Address> ] [ -hsmPort <Port> ] [ -hsmInterface <Interface Name> ] [ -hsmInterval <Seconds> ] \n"
			"\n"
			" Options for establishing a connection and sending requests through a proxy server:\n"
			"   [ -ph <proxy host> ] [ -pp <proxy port> ] [ -plogin <proxy username> ] [ -ppasswd <proxy password> ] [ -pdomain <proxy domain> ] \n"
			"\n"
			" Options for tunnel stream messaging:\n"
			"   [-tunnel] [-tsDomain <domain> ] [-tsAuth] [-tsServiceName]\n"
			"\n"
			"   -tunnel Opens a tunnel stream to the provider and begins exchanging messages.\n"
			"   -tsDomain Specifies the domain type to use when opening the tunnel stream.\n"
			"   -tsAuth causes the consumer to enable authentication when opening tunnel streams.\n"
			"   -tsServiceName specifies the name of the service to use for tunnel stream messages (if not specified, the service specified by -s is used).\n"
			"   -tsAuth  Causes the consumer to use authentication when opening tunnel streams.\n"
			"\n"
			" -rtt Turns on the Round Trip Time monitoring feature in the login stream"
			"\n"
			"-restEnableLog enable REST logging message"
			"\n"
			"-restVerbose enable verbose REST logging message"
			"\n"
			"-restLogFileName set REST logging output stream"
			"\n"
			"-restEnableLogViaCallback enable receiving REST logging messages via callback. 0 - disabled, 1 - enabled from the start, 2 - enabled after initialization stage."
			"\n"
			"-tokenURLV1 URL of token service V1\n"
			"-tokenURLV2 URL of token service V2\n"
			"-tokenScope scope for the login token\n"
			"-serviceDiscoveryURL URL the service discovery\n"
			"-restProxyHost <proxy host> Proxy host name. Used for Rest requests only: service discovery, auth\n"
			"-restProxyPort <proxy port> Proxy port. Used for Rest requests only: service discovery, auth\n"
			"-restProxyUserName <proxy username> Proxy user name. Used for Rest requests only: service discovery, auth\n"
			"-restProxyPasswd <proxy password> Proxy password. Used for Rest requests only: service discovery, auth\n"
			"-restProxyDomain <proxy domain> Proxy domain of the user. Used for Rest requests only: service discovery, auth"
			"\n"
			"-jsonOutputBufferSize <size>   Size of the buffer that the converter will allocate for its output buffer. The conversion fails if the size is not large enough"
			"-jsonTokenIncrementSize <increment> Number of json token increment size for parsing JSON messages"
			"\n"
		, argv[0], argv[0]);
	exit(-1);
}

/* Used to validate the application's configuration of socket or multicast transport. */
typedef enum
{
	/* Multicast transport options. */
	CFG_HAS_SEND_ADDR			= 0x0001,
	CFG_HAS_RECV_ADDR			= 0x0002,
	CFG_HAS_SEND_PORT			= 0x0004,
	CFG_HAS_RECV_PORT			= 0x0008,
	CFG_HAS_UNICAST_PORT		= 0x0010,
	CFG_REQUIRED_MCAST_OPTS = 
		( CFG_HAS_SEND_ADDR | CFG_HAS_RECV_ADDR | CFG_HAS_SEND_PORT | CFG_HAS_RECV_PORT
		  | CFG_HAS_UNICAST_PORT),
	CFG_ALL_MCAST_OPTS = CFG_REQUIRED_MCAST_OPTS,

	/* Host Stat Message options for multicast connections. */
	CFG_HAS_HSM_ADDR			= 0x0020,
	CFG_HAS_HSM_PORT			= 0x0040,
	CFG_HAS_HSM_INTERFACE		= 0x0080,
	CFG_HAS_HSM_INTERVAL		= 0x0100,
	CFG_REQUIRED_HSM_OPTS		= (CFG_HAS_HSM_ADDR | CFG_HAS_HSM_PORT),
	CFG_ALL_HSM_OPTS			= 
		(CFG_HAS_HSM_ADDR | CFG_HAS_HSM_PORT | CFG_HAS_HSM_INTERFACE | CFG_HAS_HSM_INTERVAL),

	/* Socket transport options. */
	CFG_HAS_HOSTNAME			= 0x0200,
	CFG_HAS_PORT				= 0x0400,
	CFG_ALL_SOCKET_OPTS			= (CFG_HAS_HOSTNAME | CFG_HAS_PORT),

	/* Warm standby options. */
	CFG_HAS_STARTING_HOSTNAME	= 0x0800,
	CFG_HAS_STARTING_PORT		= 0x1000,
	CFG_HAS_STANDBY_HOSTNAME	= 0x2000,
	CFG_HAS_STANDBY_PORT		= 0x4000,
	CFG_ALL_WMSTANDBY_OPTS		= (CFG_HAS_STARTING_HOSTNAME | CFG_HAS_STARTING_PORT | CFG_HAS_STANDBY_HOSTNAME | CFG_HAS_STANDBY_PORT)

} ConfigConnOptions;

void watchlistConsumerConfigInit(int argc, char **argv)
{
	int i;
	int configFlags = 0;
	FILE* pFile;		// For JWK file
	int readSize = 0;
	int wsConfigFlags = 0;
	char warmStandbyMode[255];

	memset(&watchlistConsumerConfig, 0, sizeof(WatchlistConsumerConfig));

	watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	watchlistConsumerConfig.encryptedConnectionType = RSSL_CONN_TYPE_INIT;

	watchlistConsumerConfig.tlsProtocol = RSSL_ENC_NONE;

	/* Set defaults. */
	snprintf(watchlistConsumerConfig.interface, 255, "%s", "");

	watchlistConsumerConfig.serviceName = defaultServiceName;
	rsslClearBuffer(&watchlistConsumerConfig.userName);
	watchlistConsumerConfig.itemCount = 0;
	watchlistConsumerConfig.providedItemCount = 0;
	watchlistConsumerConfig.setView = RSSL_FALSE;
	watchlistConsumerConfig.runTime = 300;

	watchlistConsumerConfig.enableHostStatMessages = RSSL_FALSE;
	watchlistConsumerConfig.takeExclusiveSignOnControl = RSSL_TRUE;
	snprintf(watchlistConsumerConfig.hsmAddress, 255, "%s", "");
	snprintf(watchlistConsumerConfig.hsmPort, 255, "%s", "");
	snprintf(watchlistConsumerConfig.hsmInterface, 255, "%s", "");
	watchlistConsumerConfig.hsmInterval = 5;

	snprintf(watchlistConsumerConfig.proxyHost, 255, "%s", "");
	snprintf(watchlistConsumerConfig.proxyPort, 255, "%s", "");
	snprintf(watchlistConsumerConfig.proxyUserName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.proxyPasswd, 255, "%s", "");
	snprintf(watchlistConsumerConfig.proxyDomain, 255, "%s", "");
	//API QA
	snprintf(watchlistConsumerConfig.tokenServiceUrl, 255, "");
	//END API QA

	snprintf(watchlistConsumerConfig.libsslName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.libcryptoName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.libcurlName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.sslCAStore, 255, "%s", "");

	snprintf(watchlistConsumerConfig._clientJwkMem, 2048, "%s", "");
	snprintf(watchlistConsumerConfig._audienceMem, 255, "%s", "");


	snprintf(watchlistConsumerConfig._tokenUrlV1, 255, "%s", "");
	snprintf(watchlistConsumerConfig._tokenUrlV2, 255, "%s", "");
	snprintf(watchlistConsumerConfig._serviceDiscoveryUrl, 255, "%s", "");
	snprintf(watchlistConsumerConfig._tokenScope, 255, "%s", "");

	snprintf(watchlistConsumerConfig.restProxyHost, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyPort, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyUserName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyPasswd, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyDomain, 255, "%s", "");

	snprintf(watchlistConsumerConfig.protocolList, 255, "tr_json2");

	watchlistConsumerConfig.RTTSupport = RSSL_FALSE;
	watchlistConsumerConfig.restEnableLog = RSSL_FALSE;
	watchlistConsumerConfig.restVerboseMode = RSSL_FALSE;
	watchlistConsumerConfig.restOutputStreamName = NULL;
	watchlistConsumerConfig.restEnableLogViaCallback = 0U;


	watchlistConsumerConfig.tunnelStreamDomainType = RSSL_DMT_SYSTEM;

	snprintf(warmStandbyMode, 255, "login");

	/* Set login based warm standby as default. */
	watchlistConsumerConfig.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Use default values for JSON Converter buffers. */
	watchlistConsumerConfig.jsonOutputBufferSize = 0;
	watchlistConsumerConfig.jsonTokenIncrementSize = 0;

	for(i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], "-c"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);

			if (0 == strcmp(argv[i], "socket"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
			else if (0 == strcmp(argv[i], "webSocket"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
			else if (0 == strcmp(argv[i], "http"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_HTTP;
			else if (0 == strcmp(argv[i], "encrypted"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			else if (0 == strcmp(argv[i], "reliableMCast"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
			else
			{
				printf("Unknown connection type specified: %s\n", argv[i]);
				printUsageAndExit(argc, argv);
			}
		}
		else if (0 == strcmp(argv[i], "-ec"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);

			if (0 == strcmp(argv[i], "socket"))
				watchlistConsumerConfig.encryptedConnectionType = RSSL_CONN_TYPE_SOCKET;
			else if (0 == strcmp(argv[i], "webSocket"))
				watchlistConsumerConfig.encryptedConnectionType = RSSL_CONN_TYPE_WEBSOCKET;
			else if (0 == strcmp(argv[i], "http"))
			{
#ifdef LINUX
				printf("WinInet HTTP connection not supported on this platform.\n");
				printUsageAndExit(argc, argv);
#endif
				watchlistConsumerConfig.encryptedConnectionType = RSSL_CONN_TYPE_HTTP;
			}
			else
			{
				printf("Unknown connection type specified: %s\n", argv[i]);
				printUsageAndExit(argc, argv);
			}
		}
		else if (strcmp("-libsslName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libsslName, 255, "%s", argv[i]);
		}
		else if (strcmp("-libcryptoName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libcryptoName, 255, "%s", argv[i]);
		}
		else if (strcmp("-libcurlName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libcurlName, 255, "%s", argv[i]);
		}
		else if (strcmp("-castore", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.sslCAStore, 255, "%s", argv[i]);
		}
		else if (strcmp("-spTLSv1.2", argv[i]) == 0)
		{
			watchlistConsumerConfig.tlsProtocol |= RSSL_ENC_TLSV1_2;
		}
		else if (strcmp("-spTLSv1.3", argv[i]) == 0)
		{
			watchlistConsumerConfig.tlsProtocol |= RSSL_ENC_TLSV1_3;
		}
		else if (0 == strcmp(argv[i], "-h"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.hostName, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_HOSTNAME;
		}
		else if (0 == strcmp(argv[i], "-p"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.port, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_PORT;
		}
		else if (0 == strcmp(argv[i], "-startingHostName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.startingHostName, 255, "%s", argv[i]);
			wsConfigFlags |= CFG_HAS_STARTING_HOSTNAME;
		}
		else if (0 == strcmp(argv[i], "-startingPort"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.startingPort, 255, "%s", argv[i]);
			wsConfigFlags |= CFG_HAS_STARTING_PORT;
		}
		else if (0 == strcmp(argv[i], "-standbyHostName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.standbyHostName, 255, "%s", argv[i]);
			wsConfigFlags |= CFG_HAS_STANDBY_HOSTNAME;
		}
		else if (0 == strcmp(argv[i], "-standbyPort"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.standbyPort, 255, "%s", argv[i]);
			wsConfigFlags |= CFG_HAS_STANDBY_PORT;
		}
		else if (0 == strcmp(argv[i], "-warmStandbyMode"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(warmStandbyMode, 255, "%s", argv[i]);

			if (0 == strcmp(argv[i], "login"))
				watchlistConsumerConfig.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;
			else if (0 == strcmp(argv[i], "service"))
				watchlistConsumerConfig.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;
			else
			{
				printf("Unknown warm standby mode specified: %s\n", argv[i]);
				printUsageAndExit(argc, argv);
			}
		}
		else if (0 == strcmp(argv[i], "-ph"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.proxyHost, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-pp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.proxyPort, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-plogin"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.proxyUserName, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-ppasswd"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.proxyPasswd, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-pdomain"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.proxyDomain, 255, "%s", argv[i]);
		}
		// API QA
		else if (0 == strcmp(argv[i], "-tokenServiceUrl"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.tokenServiceUrl, 255, "%s", argv[i]);
		}
		// END API QA
		else if (0 == strcmp(argv[i], "-pl"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.protocolList, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-sa"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.sendAddress, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_SEND_ADDR;
		}
		else if (0 == strcmp(argv[i], "-ra"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.recvAddress, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_RECV_ADDR;
		}
		else if (0 == strcmp(argv[i], "-sp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.sendPort, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_SEND_PORT;
		}
		else if (0 == strcmp(argv[i], "-rp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.recvPort, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_RECV_PORT;
		}
		else if (0 == strcmp(argv[i], "-up"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.unicastPort, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_UNICAST_PORT;
		}
		else if (0 == strcmp(argv[i], "-hsmAddr"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.hsmAddress, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_HSM_ADDR;
		}
		else if (0 == strcmp(argv[i], "-hsmPort"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.hsmPort, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_HSM_PORT;
		}
		else if (0 == strcmp(argv[i], "-hsmInterface"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.hsmInterface, 255, "%s", argv[i]);
			configFlags |= CFG_HAS_HSM_INTERFACE;
		}
		else if (0 == strcmp(argv[i], "-hsmInterval"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.hsmInterval = atoi(argv[i]);
			configFlags |= CFG_HAS_HSM_INTERVAL;
		}
		else if (0 == strcmp(argv[i], "-if"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.interface, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-s"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.serviceName.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._serviceNameMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.serviceName.data = watchlistConsumerConfig._serviceNameMem;
		}
		else if (0 == strcmp(argv[i], "-u"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.userName.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._userNameMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.userName.data = watchlistConsumerConfig._userNameMem;
		}
		else if (0 == strcmp(argv[i], "-passwd"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.password.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._passwordMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.password.data = watchlistConsumerConfig._passwordMem;
		}
		else if (0 == strcmp(argv[i], "-at"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.authenticationToken.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._authenticationTokenMem, 1024, "%s", argv[i]);
			watchlistConsumerConfig.authenticationToken.data = watchlistConsumerConfig._authenticationTokenMem;
		}
		else if (0 == strcmp(argv[i], "-ax"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.authenticationExtended.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._authenticationExtendedMem, 1024, "%s", argv[i]);
			watchlistConsumerConfig.authenticationExtended.data = watchlistConsumerConfig._authenticationExtendedMem;
		}
		else if (0 == strcmp(argv[i], "-aid"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.appId.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._appIdMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.appId.data = watchlistConsumerConfig._appIdMem;
		}
		else if (0 == strcmp(argv[i], "-rtt"))
		{
			watchlistConsumerConfig.RTTSupport = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-mp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_PRICE, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-mbo"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_BY_ORDER, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-mbp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_BY_PRICE, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-yc"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_YIELD_CURVE, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-sl"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_SYMBOL_LIST, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-sld"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_SYMBOL_LIST, RSSL_TRUE);
		}
		else if (0 == strcmp(argv[i], "-view"))
		{
			watchlistConsumerConfig.setView = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-post"))
		{
			watchlistConsumerConfig.post = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-offpost"))
		{
			watchlistConsumerConfig.offPost = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-tunnel"))
		{
			watchlistConsumerConfig.isTunnelStreamMessagingEnabled = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-tsDomain"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tunnelStreamDomainType = (RsslUInt8)atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-tsServiceName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tunnelStreamServiceName.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._tunnelStreamServiceNameMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.tunnelStreamServiceName.data = watchlistConsumerConfig._tunnelStreamServiceNameMem;
		}
		else if (0 == strcmp(argv[i], "-tsAuth"))
		{
			watchlistConsumerConfig.useAuthentication = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-runTime"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.runTime = atoi(argv[i]);
		}
		else if (strcmp("-x", argv[i]) == 0)
		{
			xmlTrace = RSSL_TRUE;
		}
		else if (strcmp("-sessionMgnt", argv[i]) == 0)
		{
			watchlistConsumerConfig.enableSessionMgnt = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-clientId"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.clientId.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._clientIdMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.clientId.data = watchlistConsumerConfig._clientIdMem;
		}
		else if (0 == strcmp(argv[i], "-clientSecret"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.clientSecret.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._clientSecretMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.clientSecret.data = watchlistConsumerConfig._clientSecretMem;
		}
		else if (0 == strcmp(argv[i], "-jwkFile"))
		{
			/* As this is an example program showing API, this handling of the JWK is not secure. */
			if (++i == argc) printUsageAndExit(argc, argv);
			pFile = fopen(argv[i], "rb");
			if (pFile == NULL)
			{
				printf("Cannot load jwk file.\n");
				printUsageAndExit(argc, argv);
			}
			/* Read the JWK contents into a pre-allocated buffer*/
			readSize = (int)fread(watchlistConsumerConfig._clientJwkMem, sizeof(char), 2048, pFile);
			if (readSize == 0)
			{
				printf("Cannot load jwk file.\n");
				printUsageAndExit(argc, argv);
			}
			watchlistConsumerConfig.clientJWK.data = watchlistConsumerConfig._clientJwkMem;
			watchlistConsumerConfig.clientJWK.length = readSize;

			fclose(pFile);
		}
		else if (0 == strcmp(argv[i], "-audience"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.audience.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._audienceMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.audience.data = watchlistConsumerConfig._audienceMem;
		}
		else if (0 == strcmp(argv[i], "-tokenURLV1"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tokenURLV1.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._tokenUrlV1, 255, "%s", argv[i]);
			watchlistConsumerConfig.tokenURLV1.data = watchlistConsumerConfig._tokenUrlV1;
		}
		else if (0 == strcmp(argv[i], "-tokenURLV2"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tokenURLV2.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._tokenUrlV2, 255, "%s", argv[i]);
			watchlistConsumerConfig.tokenURLV2.data = watchlistConsumerConfig._tokenUrlV2;
		}
		else if (0 == strcmp(argv[i], "-tokenScope"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tokenScope.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._tokenScope, 255, "%s", argv[i]);
			watchlistConsumerConfig.tokenScope.data = watchlistConsumerConfig._tokenScope;
		}
		else if (0 == strcmp(argv[i], "-serviceDiscoveryURL"))
		{
		if (++i == argc) printUsageAndExit(argc, argv);
		watchlistConsumerConfig.serviceDiscoveryURL.length =
			(RsslUInt32)snprintf(watchlistConsumerConfig._serviceDiscoveryUrl, 255, "%s", argv[i]);
		watchlistConsumerConfig.serviceDiscoveryURL.data = watchlistConsumerConfig._serviceDiscoveryUrl;
		}
		else if (0 == strcmp(argv[i], "-l"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.location.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._locationMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.location.data = watchlistConsumerConfig._locationMem;
		}
		else if (strcmp("-query", argv[i]) == 0)
		{
			watchlistConsumerConfig.queryEndpoint = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-takeExclusiveSignOnControl"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			if (RTR_STRNICMP(argv[i], "true", 4) == 0)
			{
				watchlistConsumerConfig.takeExclusiveSignOnControl = RSSL_TRUE;
			}
			else if (RTR_STRNICMP(argv[i], "false", 5) == 0)
			{
				watchlistConsumerConfig.takeExclusiveSignOnControl = RSSL_FALSE;
			}
		}
		else if (strcmp("-restEnableLog", argv[i]) == 0)
		{
			watchlistConsumerConfig.restEnableLog = RSSL_TRUE;
		}
		else if (strcmp("-restVerbose", argv[i]) == 0)
		{
			watchlistConsumerConfig.restVerboseMode = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-restLogFileName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.restOutputStreamName = fopen(argv[i], "w");
			if (!watchlistConsumerConfig.restOutputStreamName)
			{
				printf("Error: Unable to open the specified file name %s\n", argv[i]);
				printUsageAndExit(argc, argv);
			}
		}
		else if (strcmp("-restEnableLogViaCallback", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.restEnableLogViaCallback = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-jsonOutputBufferSize"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.jsonOutputBufferSize = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-jsonTokenIncrementSize"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.jsonTokenIncrementSize = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyHost"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyHost, sizeof(watchlistConsumerConfig.restProxyHost), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyPort"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyPort, sizeof(watchlistConsumerConfig.restProxyPort), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyUserName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyUserName, sizeof(watchlistConsumerConfig.restProxyUserName), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyPasswd"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyPasswd, sizeof(watchlistConsumerConfig.restProxyPasswd), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyDomain"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyDomain, sizeof(watchlistConsumerConfig.restProxyDomain), "%s", argv[i]);
		}
		else
		{
			printf("Error: Unrecognized option: %s\n", argv[i]);
			printUsageAndExit(argc, argv);
		}
	}

	/* Checks whether warm standby options are specified. */
	if (wsConfigFlags != CFG_ALL_WMSTANDBY_OPTS)
	{
		watchlistConsumerConfig.warmStandbyMode = RSSL_RWSB_MODE_NONE;
	}

	if (!watchlistConsumerConfig.enableSessionMgnt)
	{
		if (strlen(watchlistConsumerConfig.hostName) == 0)
			snprintf(watchlistConsumerConfig.hostName, 255, "%s", defaultHostName);

		if (strlen(watchlistConsumerConfig.port) == 0)
			snprintf(watchlistConsumerConfig.port, 255, "%s", defaultPort);
	}

	if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		/* Make sure no socket connection options were specified. */
		if (configFlags & CFG_ALL_SOCKET_OPTS)
		{
			printf("Config Error: Do not specify -h or -p when using a multicast connection.\n");
			printUsageAndExit(argc, argv);
		}

		/* Make sure all required multicast options were specified. */
		if ((configFlags & CFG_REQUIRED_MCAST_OPTS) != CFG_REQUIRED_MCAST_OPTS)
		{
			printf("Config Error: Missing multicast options. Specify all of -ra, -sa, -rp, -sp, and -up when using a multicast connection.\n");
			printUsageAndExit(argc, argv);
		}

		/* If Host Stat Message options were specified, make sure all required ones were specified. */
		if (configFlags & CFG_ALL_HSM_OPTS)
		{
			if ((configFlags & CFG_REQUIRED_HSM_OPTS) != CFG_REQUIRED_HSM_OPTS)
			{
				printf("Config Error: Both -hsmAddr and -hsmPort must be specified when enabling Host Stat Messages.\n");
				printUsageAndExit(argc, argv);
			}
			watchlistConsumerConfig.enableHostStatMessages = RSSL_TRUE;
		}

		printf( "Config:\n"
				"  Send Address: %s\n"
				"  Receive Address: %s\n"
				"  Send Port: %s\n"
				"  Receive Port: %s\n"
				"  Unicast Port: %s\n" ,
				watchlistConsumerConfig.sendAddress,
				watchlistConsumerConfig.recvAddress,
				watchlistConsumerConfig.sendPort,
				watchlistConsumerConfig.recvPort,
				watchlistConsumerConfig.unicastPort);

		if (watchlistConsumerConfig.enableHostStatMessages)
		{
			printf(
					"  HSM Address: %s\n"
					"  HSM Port: %s\n"
					"  HSM Interface: %s\n"
					"  HSM Interval: %u\n",
					watchlistConsumerConfig.hsmAddress,
					watchlistConsumerConfig.hsmPort,
					watchlistConsumerConfig.hsmInterface,
					watchlistConsumerConfig.hsmInterval);
		}
	}
	else
	{
		/* Make sure no multicast connection options were specified. */
		if (configFlags & CFG_ALL_MCAST_OPTS)
		{
			printf("Config Error: Do not specify -ra, -sa, -rp, -sp, or -up when using a non-multicast connection.\n");
			printUsageAndExit(argc, argv);
		}

		/* No HSM options should be set for non-multicast connections. */
		if (configFlags & CFG_ALL_HSM_OPTS)
		{
			printf("Config Error: Do not specify -hsmAddr, -hsmPort, -hsmInterface, or -hsmInterval when using a non-multicast connection.\n");
			printUsageAndExit(argc, argv);
		}

		if (watchlistConsumerConfig.warmStandbyMode == RSSL_RWSB_MODE_NONE)
		{
			printf("Config:\n"
				"  Hostname: %s\n"
				"  Port: %s\n",
				watchlistConsumerConfig.hostName,
				watchlistConsumerConfig.port);
		}
		else
		{
			printf("Config:\n"
				"  Warm standby mode: %s\n"
				"  Starting Hostname: %s\n"
				"  Starting Port: %s\n"
				"  Standby Hostname: %s\n"
				"  Standby Port: %s\n",
				warmStandbyMode,
				watchlistConsumerConfig.startingHostName,
				watchlistConsumerConfig.startingPort,
				watchlistConsumerConfig.standbyHostName,
				watchlistConsumerConfig.standbyPort);
		}
	}

	if (watchlistConsumerConfig.itemCount == 0 && !watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
	{
		addItem((char*)"TRI.N", RSSL_DMT_MARKET_PRICE, RSSL_FALSE);
	}

	printf( "  Interface: %s\n", watchlistConsumerConfig.interface);
	if(watchlistConsumerConfig.authenticationToken.length)
	{
		printf( " AuthenticationToken: %s\n", watchlistConsumerConfig.authenticationToken.data);
	}
	else
	{
		printf(	"  UserName: %s\n", 
			watchlistConsumerConfig.userName.length ? watchlistConsumerConfig.userName.data : "(use system login name)");
	}
	
	printf(	"  ServiceName: %s\n"
			"  Run Time: %us\n",
			watchlistConsumerConfig.serviceName.data,
			watchlistConsumerConfig.runTime);

	if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
	{
		/* If no service specific to tunnel stream messaging was provided, use the same service used 
		 * elsewhere. */
		if (watchlistConsumerConfig.tunnelStreamServiceName.data == NULL)
			watchlistConsumerConfig.tunnelStreamServiceName = watchlistConsumerConfig.serviceName;

		printf("\n");
		printf("  TunnelStream Service Name: %s\n", watchlistConsumerConfig.tunnelStreamServiceName.data);
	}

	printf("\n");
}

ItemInfo *getItemInfo(RsslInt32 streamId)
{
	if (streamId > 0)
	{
		int itemLocation = streamId - ITEMS_MIN_STREAM_ID;
		if (streamId >= ITEMS_MIN_STREAM_ID && streamId < (RsslInt32)watchlistConsumerConfig.itemCount
				+ ITEMS_MIN_STREAM_ID)
			return &watchlistConsumerConfig.itemList[streamId - ITEMS_MIN_STREAM_ID];
		else
			return NULL;
	}
	else if (streamId < 0)
	{
		RsslUInt32 i;

		for(i = 0; i < watchlistConsumerConfig.providedItemCount; ++i)
		{
			if (watchlistConsumerConfig.providedItemList[i].streamId == streamId)
				return &watchlistConsumerConfig.providedItemList[i];
		}
		return NULL;
	}
	else
		return NULL;
}

ItemInfo *addProvidedItemInfo(RsslInt32 streamId, const RsslMsgKey *pMsgKey, RsslUInt8 domainType)
{
	ItemInfo *pItem = NULL;
	RsslUInt32 i;

	/* Check if item is already present. */
	for(i = 0; i < watchlistConsumerConfig.providedItemCount; ++i)
	{
		if (watchlistConsumerConfig.providedItemList[i].streamId == streamId)
		{
			pItem = &watchlistConsumerConfig.providedItemList[i];
			break;
		}
	}

	/* Add new item. */
	if (!pItem)
	{
		if (watchlistConsumerConfig.providedItemCount == MAX_ITEMS)
		{
			printf("Too many provided items.\n");
			return NULL;
		}
		pItem = &watchlistConsumerConfig.providedItemList[watchlistConsumerConfig.providedItemCount];
		++watchlistConsumerConfig.providedItemCount;
	}

	if (pMsgKey->flags & RSSL_MKF_HAS_NAME)
	{
		pItem->name.length = pMsgKey->name.length;
		if (pItem->name.length > MAX_ITEM_NAME_LEN)
		{
			printf("Error: Adding item name that is too long.\n");
			exit(-1);
		}
		pItem->name.data = pItem->_nameMem;
		strncpy(pItem->name.data, pMsgKey->name.data, pItem->name.length);
	}
	else
	{
		pItem->name.data = NULL;
		pItem->name.length = 0;
	}

	pItem->streamId = streamId;
	pItem->domainType = domainType;

	return pItem;
}

void removeProvidedItemInfo(ItemInfo *pItem)
{
	RsslUInt32 i;
	for(i = 0; i < watchlistConsumerConfig.providedItemCount; ++i)
	{
		if (watchlistConsumerConfig.providedItemList[i].streamId == pItem->streamId)
		{
			/* Clean up item. */
			pItem = &watchlistConsumerConfig.providedItemList[i];

			/* Swap in information from the last item in the list. */
			if (watchlistConsumerConfig.providedItemCount > 1)
				*pItem = watchlistConsumerConfig.providedItemList[watchlistConsumerConfig.providedItemCount - 1];

			--watchlistConsumerConfig.providedItemCount;
			return;
		}
	}
}

RsslBool isXmlTracingEnabled()
{
	return xmlTrace;
}
