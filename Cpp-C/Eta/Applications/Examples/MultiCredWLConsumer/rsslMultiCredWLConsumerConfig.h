/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2023 Refinitiv. All rights reserved.
*/

#ifndef WATCHLIST_CONSUMER_CONFIG_H
#define WATCHLIST_CONSUMER_CONFIG_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMsgKey.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslReactor.h"

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
} ItemInfo;

/* Contains configurable options for the application. */
typedef struct
{
	RsslReactorConnectOptions		connectionOpts;
	RsslReactorOMMConsumerRole		consumerRole;

	char				libsslName[255];
	char				libcryptoName[255];
	char				libcurlName[255];
	char				sslCAStore[255];

	char				configJsonFileName[255];
	

	RsslBuffer			serviceName;					/* Service name to use when requesting items. */
	RsslBuffer			tokenURLV1;						/* Authentication Token V1 URL location */
	RsslBuffer			tokenURLV2;						/* Authentication Token V2 URL location */
	RsslBuffer			serviceDiscoveryURL;			/* Service Discovery URL location */
	ItemInfo			itemList[MAX_ITEMS];			/* The list of items to request. */
	
	RsslUInt32			itemCount;						/* Number of items in itemList. */
	RsslUInt32			runTime;						/* Running time of the application. */

	RsslInt32			reconnectLimit;					/* Number of reconnect atttempts. */


	RsslBool			restEnableLog;					/* Enable Rest request/response logging.*/
	FILE				*restOutputStreamName;			/* Set output stream for Rest request/response logging.*/
	RsslBool			restEnableLogCallback;			/* Enable Rest request/response logging via callback.*/

	char			_serviceNameMem[255];
	char 			_appIdMem[255];
	char			_tokenUrlV1[255];
	char			_tokenUrlV2[255];
	char			_serviceDiscoveryUrl[255];

	/* Proxy configuration settings for Rest requests */
	char			restProxyHost[255];					/* Proxy host name */
	char			restProxyPort[255];					/* Proxy port */
	char			restProxyUserName[255];				/* Proxy user name */
	char			restProxyPasswd[255];				/* Proxy password */
	char			restProxyDomain[255];				/* Proxy domain */

} WatchlistConsumerConfig;

typedef struct
{
	RsslReactorLoginRequestMsgCredential requestMsgCredential;
	RsslBuffer loginName;
	RsslInt8 loginArrayIndex;

	char _nameBuffer[255];
	char _userNameBuffer[255];
	char _authTokenExtendedBuffer[255];
}LoginRequestCredential;


typedef struct
{
	RsslReactorOAuthCredential oAuthCredential;
	RsslBuffer credentialName;
	RsslInt8 oAuthArrayIndex;

	char _nameBuffer[255];
	char _usernameBuffer[255];
	char _passwordBuffer[255];
	char _clientIdBuffer[255];
	char _clientSecretBuffer[255];
	char _tokenScopeBuffer[255];
	char _clientJWKBuffer[2048];
	char _audienceBuffer[255];
}OAuthRequestCredential;

typedef struct
{
	RsslReactorConnectInfo connectionInfo;
	RsslBuffer connectionName;

	char _nameBuffer[255];
	char _hostBuffer[255];
	char _portBuffer[255];
	char _interfaceBuffer[255];
	char _proxyHostBuffer[255];
	char _proxyPortBuffer[255];
	char _proxyUsernameBuffer[255];
	char _proxyPasswordBuffer[255];
	char _proxyDomainBuffer[255];
	char _caStoreBuffer[255];
	char _locationBuffer[255];
}ConnectionInfoConfig;

typedef struct
{
	RsslReactorWarmStandbyGroup warmStandbyGroup;
	ConnectionInfoConfig startingActiveConnectionInfo;
	ConnectionInfoConfig* standbyConnectionInfoList;
	RsslBuffer name;

	char _nameBuffer[255];
}WarmStandbyGroupConfig;

extern WatchlistConsumerConfig watchlistConsumerConfig;

/* Initializes the configuration, parsing command-line options. */
void watchlistConsumerConfigInit(int argc, char **argv);

void clearLoginRequestCredential(LoginRequestCredential* credentials);

void clearOAuthRequestCredential(OAuthRequestCredential* credentials);

void clearConnectionInfoConfig(ConnectionInfoConfig* config);

/* Clears the WarmStandbyGroupConfig*/
void clearWarmStandbyGroupConfig(WarmStandbyGroupConfig* config);

/* Gets item information based on the stream ID. */
ItemInfo *getItemInfo(RsslInt32 streamId);

/* Prins usage text. */
static void printUsageAndExit(int argc, char **argv);

/* Returns whether or not XML tracing is enabled */ 
RsslBool isXmlTracingEnabled();

void clearAllocatedMemory();


static RsslReactorCallbackRet channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent
*pConnEvent);

static RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent
 *pConnEvent);

 static RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDictionaryMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet msgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent);

 static RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorJsonConversionEvent *pEvent);

 static RsslRet serviceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent);

 static RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent);

 RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor* pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent);

 RsslReactorCallbackRet loginMsgEventCallback(RsslReactor* pReactor, RsslReactorChannel* pChannel, RsslReactorLoginRenewalEvent* pLoginCredentialEvent);


#ifdef __cplusplus
}
#endif

#endif
