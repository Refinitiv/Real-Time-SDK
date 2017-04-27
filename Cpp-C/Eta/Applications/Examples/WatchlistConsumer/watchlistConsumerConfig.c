/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
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
			" or %s [-c <Connection Type> ] [-if <Interface Name>] [ -u <Login UserName> ] [-s <ServiceName>] [ -mp <MarketPrice ItemName> ] [ -mbo <MarketByOrder ItemName> ] [ -mbp <MarketByPrice ItemName> ] [ -yc <YieldCurve ItemName> ] [ -sl <SymbolList ItemName> ] [ -view ] [-x] [ -runTime <TimeToRun> ]\n"
			" -c       Specifies connection type. Valid arguments are socket, http, encrypted, and reliableMCast.\n"
			" -if      Specifies the address of a specific network interface to use.\n"
			" -mp      For each occurance, requests item using Market Price domain.\n"
			" -mbo     For each occurance, requests item on the Market By Order domain.\n"
			" -mbp     For each occurance, requests item on the Market By Price domain.\n"
			" -yc      For each occurance, requests item on the Yield Curve domain.\n"
			" -sl      For each occurance, requests item on the Symbol List domain.\n"
			" -sld     For each occurance, requests item on the Symbol List domain and data streams for items on that list.\n"
			" -post    Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream)\n"
			" -offpost Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n"
			" -x       Enables tracing of messages sent to and received from the channel.\n"
			" -runTime Adjusts the running time of the application.\n"
			" -at	   Specifies the Authentication Token. If this is present, the login user name type will be RDM_LOGIN_USER_AUTHN_TOKEN.\n"
			" -ax      Specifies the Authentication Extended information.\n"
			" -aid	   Specifies the Application ID.\n"
			"\n"
			" Connection options for socket, http, and encrypted connection types:\n"
			"   [ -h <Server Hostname> ] [ -p <Port> ]\n"
			"\n"
			" Connection options for the reliable multicast connection type; all must be specified:\n"
			"   [ -sa <Send Address> ] [ -ra <Receive Address> ] [ -sp <Send Port> ] [ -rp <Receive Port> ] [ -up <Unicast Port> ]\n"
			"\n"
			" Options for publishing Host Stat Message options on reliable multicast connections; -hsmAddr and -hsmPort must be specified to enable:\n"
			"   [ -hsmAddr <Address> ] [ -hsmPort <Port> ] [ -hsmInterface <Interface Name> ] [ -hsmInterval <Seconds> ] \n"
			"\n"
			" Options for tunnel stream messaging/queue messaging:\n"
			"   [-tunnel] [-tsDomain <domain> ] [-tsAuth] [ -qSourceName <name> ] [ -qDestName <name> ] [-tsServiceName]\n"
			"\n"
			"   -tunnel Opens a tunnel stream to the provider and begins exchanging messages.\n"
			"   -tsDomain Specifies the domain type to use when opening the tunnel stream.\n"
			"   -tsAuth causes the consumer to enable authentication when opening tunnel streams (also applies to queue messaging treams).\n"
			"   -qSourceName  Opens a tunnel stream to the provider, and opens a queue with the provided name.\n"
			"   -qDestName  Specifies destination names for sending queue messages. May specify this multiple times.\n"
			"   -tsServiceName specifies the name of the service to use for queue messages (if not specified, the service specified by -s is used).\n"
			"   -tsAuth  Causes the consumer to use authentication when opening tunnel streams.\n"
			, argv[0], argv[0]);
	exit(-1);
}

/* Used to validate the application's configuration of socket or multicast transport. */
typedef enum
{
	/* Multicast transport options. */
	CFG_HAS_SEND_ADDR			= 0x001,
	CFG_HAS_RECV_ADDR			= 0x002,
	CFG_HAS_SEND_PORT			= 0x004,
	CFG_HAS_RECV_PORT			= 0x008,
	CFG_HAS_UNICAST_PORT		= 0x010,
	CFG_REQUIRED_MCAST_OPTS = 
		( CFG_HAS_SEND_ADDR | CFG_HAS_RECV_ADDR | CFG_HAS_SEND_PORT | CFG_HAS_RECV_PORT
		  | CFG_HAS_UNICAST_PORT),
	CFG_ALL_MCAST_OPTS = CFG_REQUIRED_MCAST_OPTS,

	/* Host Stat Message options for multicast connections. */
	CFG_HAS_HSM_ADDR			= 0x020,
	CFG_HAS_HSM_PORT			= 0x040,
	CFG_HAS_HSM_INTERFACE		= 0x080,
	CFG_HAS_HSM_INTERVAL		= 0x100,
	CFG_REQUIRED_HSM_OPTS		= (CFG_HAS_HSM_ADDR | CFG_HAS_HSM_PORT),
	CFG_ALL_HSM_OPTS			= 
		(CFG_HAS_HSM_ADDR | CFG_HAS_HSM_PORT | CFG_HAS_HSM_INTERFACE | CFG_HAS_HSM_INTERVAL),

	/* Socket transport options. */
	CFG_HAS_HOSTNAME			= 0x200,
	CFG_HAS_PORT				= 0x400,
	CFG_ALL_SOCKET_OPTS			= (CFG_HAS_HOSTNAME | CFG_HAS_PORT)

} ConfigConnOptions;

void watchlistConsumerConfigInit(int argc, char **argv)
{
	int i;
	int configFlags = 0;

	memset(&watchlistConsumerConfig, 0, sizeof(WatchlistConsumerConfig));

	/* Set defaults. */
	snprintf(watchlistConsumerConfig.hostName, 255, "%s", defaultHostName);
	snprintf(watchlistConsumerConfig.port, 255, "%s", defaultPort);
	snprintf(watchlistConsumerConfig.interface, 255, "");

	watchlistConsumerConfig.serviceName = defaultServiceName;
	rsslClearBuffer(&watchlistConsumerConfig.userName);
	watchlistConsumerConfig.itemCount = 0;
	watchlistConsumerConfig.providedItemCount = 0;
	watchlistConsumerConfig.setView = RSSL_FALSE;
	watchlistConsumerConfig.runTime = 300;

	watchlistConsumerConfig.enableHostStatMessages = RSSL_FALSE;
	snprintf(watchlistConsumerConfig.hsmAddress, 255, "");
	snprintf(watchlistConsumerConfig.hsmPort, 255, "");
	snprintf(watchlistConsumerConfig.hsmInterface, 255, "");
	watchlistConsumerConfig.hsmInterval = 5;


	watchlistConsumerConfig.tunnelStreamDomainType = RSSL_DMT_SYSTEM;

	for(i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], "-c"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);

			if (0 == strcmp(argv[i], "socket"))
				watchlistConsumerConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
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
		else if (0 == strcmp(argv[i], "-qSourceName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.isQueueMessagingEnabled = RSSL_TRUE;
			watchlistConsumerConfig.queueSourceName.length = 
				(RsslUInt32)snprintf(watchlistConsumerConfig._queueSourceNameMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.queueSourceName.data = watchlistConsumerConfig._queueSourceNameMem;
		}
		else if (0 == strcmp(argv[i], "-qDestName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);

			if (watchlistConsumerConfig.queueDestNameCount == MAX_DEST_NAMES)
			{
				printf("Config error: Example only supports %d queue destination names.\n",
						MAX_DEST_NAMES);
				printUsageAndExit(argc, argv);
			}

			watchlistConsumerConfig.queueDestNameList[watchlistConsumerConfig.queueDestNameCount].length 
				= (RsslUInt32)snprintf(
						watchlistConsumerConfig._queueDestNameMem[watchlistConsumerConfig.queueDestNameCount]
						, 255, "%s", argv[i]);
			watchlistConsumerConfig.queueDestNameList[watchlistConsumerConfig.queueDestNameCount].data = 
				watchlistConsumerConfig._queueDestNameMem[watchlistConsumerConfig.queueDestNameCount];
			++watchlistConsumerConfig.queueDestNameCount;
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
		else
		{
			printf("Config Error: Unknown option %s\n", argv[i]);
			printUsageAndExit(argc, argv);
		}

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

		printf( "Config:\n"
				"  Hostname: %s\n"
				"  Port: %s\n",
				watchlistConsumerConfig.hostName,
				watchlistConsumerConfig.port);
	}

	if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled
			&& watchlistConsumerConfig.isQueueMessagingEnabled)
	{
			printf("Config Error: Both tunnel stream messaging and queue messaging are enabled.\n");
			printUsageAndExit(argc, argv);
	}

	if (watchlistConsumerConfig.queueSourceName.data == NULL 
			&& watchlistConsumerConfig.queueDestNameCount > 0)
	{
			printf("Config Error: Queue Messaging destinations specified but no source name.\n");
			printUsageAndExit(argc, argv);
	}

	if (watchlistConsumerConfig.itemCount == 0 && !watchlistConsumerConfig.isTunnelStreamMessagingEnabled
			&& !watchlistConsumerConfig.isQueueMessagingEnabled)
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

	if (watchlistConsumerConfig.isQueueMessagingEnabled || watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
	{
		/* If no service specific to queue messaging was provided, use the same service used 
		 * elsewhere. */
		if (watchlistConsumerConfig.tunnelStreamServiceName.data == NULL)
			watchlistConsumerConfig.tunnelStreamServiceName = watchlistConsumerConfig.serviceName;

		printf("\n");
		printf("  TunnelStream Service Name: %s\n", watchlistConsumerConfig.tunnelStreamServiceName.data);
	}

	if (watchlistConsumerConfig.isQueueMessagingEnabled)
	{
		RsslUInt32 i;

		printf("  Queue Messaging: SourceName: %s Destination Names:", 
				watchlistConsumerConfig.queueSourceName.data);

		for (i = 0; i < watchlistConsumerConfig.queueDestNameCount; ++i)
			printf(" %s", watchlistConsumerConfig.queueDestNameList[i].data);

	}

	printf("\n");
}

void watchlistConsumerConfigCleanup()
{
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
