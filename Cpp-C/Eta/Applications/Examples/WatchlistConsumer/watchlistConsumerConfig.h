/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef WATCHLIST_CONSUMER_CONFIG_H
#define WATCHLIST_CONSUMER_CONFIG_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMsgKey.h"
#include "tunnelStreamHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of items supported by the application. */
#define MAX_ITEMS 128

/* Maximum stored item length. */
#define MAX_ITEM_NAME_LEN 255

/* Enumerates stream IDs the application uses when opening streams. */
enum
{
	LOGIN_STREAM_ID				= 1,	/* Used when requesting a login. */
	DIRECTORY_STREAM_ID			= 2,	/* Used when requesting a source directory. */
	FIELD_DICTIONARY_STREAM_ID	= 3,	/* Used when requesting the field dictionary. */
	ENUM_DICTIONARY_STREAM_ID	= 4,	/* Used when requesting the enumerated types dictionary. */
	ITEMS_MIN_STREAM_ID			= 5		/* All item requests start from this ID. */
};

/* Represents an item stream.*/
typedef struct
{
	RsslUInt8	domainType;		/* Domain type of the item (e.g. MarketPrice, SymbolList) */
	RsslBuffer	name;			/* Name of the item. */
	RsslInt32	streamId;		/* Stream ID to use when requesting the item. */
	char		_nameMem[MAX_ITEM_NAME_LEN]; /* Storage for item name. */
	RsslBool	symbolListData;	/* For symbol list requests, indicates whether to request
								 * data streams. */
} ItemInfo;

/* Contains configurable options for the application. */
typedef struct
{
	RsslConnectionTypes	connectionType;					/* Type of RSSL transport to use. */
	RsslConnectionTypes encryptedConnectionType;		/* Encrypted protocol when connectionType is RSSL_CONN_TYPE_ENCRYPTED */
	char				interface[255];					/* Address of network interface to use. */

	/* Socket configuration settings, when using a socket connection. */
	char				hostName[255];					/* Host to connect to. */
	char				port[255];						/* Port to connect to. */

	/* Multicast configuration settings, when using a multicast connection. */
	char				sendAddress[255];				/* Send address. */
	char				recvAddress[255];				/* Receive address. */
	char				sendPort[255];					/* Send Port. */
	char				recvPort[255];					/* Receive port. */
	char				unicastPort[255];				/* Unicast port. */

	/* Proxy configuration settings */
	char				proxyHost[255];					/* Proxy host name */
	char				proxyPort[255];					/* Proxy port */
	char				proxyUserName[255];				/* Proxy user name */
	char				proxyPasswd[255];				/* Proxy password */
	char				proxyDomain[255];				/* Proxy domain */

	char				libsslName[255];
	char				libcryptoName[255];
	char				libcurlName[255];
	char				sslCAStore[255];
	/* Host-Stat message settings, when using a multicast connection. */
	RsslBool			enableHostStatMessages;			/* Whether to configure transport to publish
														 * host stat messages. */
	char				hsmAddress[255];				/* HSM Address. */
	char				hsmPort[255];					/* HSM Port. */
	char				hsmInterface[255];				/* HSM Interface name. */
	RsslUInt16			hsmInterval;					/* HSM publishing interval (seconds). */

	RsslBuffer			userName;						/* Username to use when logging in. */
	RsslBuffer			serviceName;					/* Service name to use when requesting items. */
	RsslBuffer			authenticationToken;			/* Authentication token used for logging in */
	RsslBuffer			authenticationExtended;			/* Extended Authentication information used for logging in */
	RsslBuffer			appId;					/* Application ID */
	ItemInfo			itemList[MAX_ITEMS];			/* The list of items to request. */
	ItemInfo			providedItemList[MAX_ITEMS];	/* Stores any items opened by the provider.
														 * May occur when requesting symbol list with
														 * data streams. */
	RsslUInt32			itemCount;						/* Number of items in itemList. */
	RsslUInt32			providedItemCount;				/* Number of items in providedItemList. */
	RsslBool			setView;						/* Whether to set a view on MarketPrice requests.*/
	RsslBool			post;							/* Whether to send on-stream posts. */
	RsslBool			offPost;						/* Whether to send off-stream posts. */
	RsslUInt32			runTime;						/* Running time of the application. */

	RsslBool			isTunnelStreamMessagingEnabled;	/* Whether to open a tunnel stream for
														 * exchanging some basic messages. */
	RsslBuffer			tunnelStreamServiceName;		/* Service name to use for opening a tunnel stream. */
	RsslUInt8			tunnelStreamDomainType;			/* Domain type to use for opening a tunnel stream. */
	RsslBool			useAuthentication;				/* Whether to use authentication when
														 * opening a tunnel stream. */

	char			_userNameMem[255];
	char			_serviceNameMem[255];
	char			_tunnelStreamServiceNameMem[255];
	char 			_authenticationTokenMem[1024];
	char			_authenticationExtendedMem[1024];
	char 			_appIdMem[255];
} WatchlistConsumerConfig;
extern WatchlistConsumerConfig watchlistConsumerConfig;

/* Initializes the configuration, parsing command-line options. */
void watchlistConsumerConfigInit(int argc, char **argv);

/* Cleans up resources associated with the configuration. */
void watchlistConsumerConfigCleanup();

/* Gets item information based on the stream ID. */
ItemInfo *getItemInfo(RsslInt32 streamId);

/* Adds an item to the list of provided items. */
ItemInfo *addProvidedItemInfo(RsslInt32 streamId, const RsslMsgKey *pMsgKey, RsslUInt8 domainType);

/* Removes an item from the list of provided items. */
void removeProvidedItemInfo(ItemInfo *pItem);

/* Prins usage text. */
static void printUsageAndExit(int argc, char **argv);

/* Returns whether or not XML tracing is enabled */ 
RsslBool isXmlTracingEnabled();


static RsslReactorCallbackRet channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent
*pConnEvent);

static RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent
 *pConnEvent);

 static RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDictionaryMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet msgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent);


#ifdef __cplusplus
}
#endif

#endif
