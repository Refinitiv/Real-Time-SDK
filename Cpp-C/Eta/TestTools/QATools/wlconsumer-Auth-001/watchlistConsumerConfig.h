/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef WATCHLIST_CONSUMER_CONFIG_H
#define WATCHLIST_CONSUMER_CONFIG_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMsgKey.h"
#include "rtr/rsslDataUtils.h"
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
	RsslEncryptionProtocolTypes tlsProtocol;			/* Bitmap flag set defining the TLS version(s) to be used by this connection. See RsslEncryptionProtocolTypes */
	char				interface[255];					/* Address of network interface to use. */

	/* Socket configuration settings, when using a socket connection. */
	char				hostName[255];					/* Host to connect to. */
	char				port[255];						/* Port to connect to. */

	/* Warm standby configuration settings. */
	char				startingHostName[255];
	char				startingPort[255];

	char				standbyHostName[255];
	char				standbyPort[255];
	RsslReactorWarmStandbyMode		warmStandbyMode;


	/* WebSocket configuration settings, when using a websocket connection. */
	char				protocolList[255];				/* List of supported WebSocket sub-protocols */

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
	RsslBuffer			password;						/* Password to use when logging in. */
	RsslBuffer			serviceName;					/* Service name to use when requesting items. */
	RsslBuffer			authenticationToken;			/* Authentication token used for logging in */
// APIQA: Added 2nd token
	RsslBuffer			authenticationToken2;			/* Authentication token 2 used for a token renewal */
// END APIQA
	RsslBuffer			authenticationExtended;			/* Extended Authentication information used for logging in */
	RsslBuffer			appId;							/* Application ID */
	RsslBuffer			tokenURLV1;						/* Authentication Token V1 URL location */
	RsslBuffer			tokenURLV2;						/* Authentication Token V2 URL location */
	RsslBuffer			serviceDiscoveryURL;			/* Service Discovery URL location */
	RsslBuffer			tokenScope;						/* Optional token scope */
	RsslBool			RTTSupport;						/* Enable the RTT feature on this reactor channel */
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

	RsslBool			enableSessionMgnt;				/* Enables the session management to keep the session alive */
	RsslBuffer			clientId;						/* Unique ID defined for application making request to RDP token service, or client ID */  
	RsslBuffer			clientSecret;					/* Client secret with associated clientId */
	RsslBuffer			clientJWK;						/* Client JWK with associated clientId */
	RsslBuffer			audience;						/* Optional audience claim for use with JWT */
	RsslBuffer			location;						/* Location to get an endpoint from RDP Service discovery */
	RsslBool			queryEndpoint;					/* Queries the RDP service discovery in application for the specified connection type and location. */
	RsslBool			takeExclusiveSignOnControl;		/* The exclusive sign on control to force sign-out for the same credentials.*/

	RsslBool			restEnableLog;					/* Enable Rest request/response logging.*/
	RsslBool			restVerboseMode;				/* Enable verbose Rest request/response logging.*/
	FILE				*restOutputStreamName;			/* Set output stream for Rest request/response logging.*/
	RsslUInt			restEnableLogViaCallback;		/* Enable Rest request/response logging via callback. 0 - disabled, 1 - enabled from the start, 2 - enabled after initialization stage. */

	RsslUInt32			jsonOutputBufferSize;			/* JSON Converter output buffer size. */
	RsslUInt32			jsonTokenIncrementSize;			/* JSON Converter number of json token increment size for parsing JSON messages. */

	char			_userNameMem[255];
	char			_passwordMem[255];
	char			_serviceNameMem[255];
	char			_tunnelStreamServiceNameMem[255];
	char 			_authenticationTokenMem[1024];
// APIQA:  Added 2nd token
	char 			_authenticationToken2Mem[1024];
// END APIQA
	char			_authenticationExtendedMem[1024];
	char 			_appIdMem[255];
	char 			_clientIdMem[255];
	char 			_clientSecretMem[255];
	char			_clientJwkMem[2048];
	char			_audienceMem[255];
	char 			_locationMem[255];
	char			_tokenUrlV1[255];
	char			_tokenUrlV2[255];
	char			_serviceDiscoveryUrl[255];
	char			_tokenScope[255];

	/* Proxy configuration settings for Rest requests */
	char			restProxyHost[255];					/* Proxy host name */
	char			restProxyPort[255];					/* Proxy port */
	char			restProxyUserName[255];				/* Proxy user name */
	char			restProxyPasswd[255];				/* Proxy password */
	char			restProxyDomain[255];				/* Proxy domain */

} WatchlistConsumerConfig;
extern WatchlistConsumerConfig watchlistConsumerConfig;

/* Initializes the configuration, parsing command-line options. */
void watchlistConsumerConfigInit(int argc, char **argv);

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

 static RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pEndPointEvent); 

 static RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorJsonConversionEvent *pEvent);

 static RsslRet serviceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent);

 static RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent);
 static RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor* pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent);
#ifdef __cplusplus
}
#endif

#endif
