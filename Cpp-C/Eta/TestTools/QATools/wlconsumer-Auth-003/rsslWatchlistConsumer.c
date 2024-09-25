/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2021-2024 LSEG. All rights reserved.
*/

/*
 * This is the main file for the rsslWatchlistConsumer application.  It is a single-threaded
 * client application that utilizes the ETA Reactor's watchlist to provide recovery of data.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the ETA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application makes use of the RDM package for easier decoding of Login & Source Directory
 * messages.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include "watchlistConsumerConfig.h"
#include "itemDecoder.h"
#include "postHandler.h"
#include "simpleTunnelMsgHandler.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslMessagePackage.h"
#include <time.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <stdlib.h>

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

static RsslReactorChannel *pConsumerChannel = NULL;
static RsslBool itemsRequested = RSSL_FALSE;
static RsslBool isConsumerChannelUp = RSSL_FALSE;

static PostServiceInfo serviceInfo;

static SimpleTunnelMsgHandler simpleTunnelMsgHandler;
static void initTunnelStreamMessaging();
RsslBool runTimeExpired = RSSL_FALSE;
RsslSocket socketIdList[2] = { 0, 0 };
RsslUInt32 socketIdListCount = 0;

/* For UserAuthn authentication login reissue */
static RsslUInt loginReissueTime; // represented by epoch time in seconds
static RsslBool canSendLoginReissue;

fd_set	readFds, exceptFds;

extern RsslDataDictionary dictionary;

// APIQA: Adding a counter
static int eventCounter = 0;
// END API QA

int main(int argc, char **argv)
{
	RsslReactor							*pReactor;

	RsslRet								ret;
	RsslCreateReactorOptions			reactorOpts;
	RsslReactorConnectOptions			reactorConnectOpts;
	RsslReactorConnectInfo				reactorConnectInfo;
	RsslErrorInfo						rsslErrorInfo;
	RsslReactorServiceDiscoveryOptions	serviceDiscoveryOpts;
	RsslReactorJsonConverterOptions		jsonConverterOptions;

	RsslReactorOMMConsumerRole			consumerRole;
	RsslRDMLoginRequest					loginRequest;
	RsslRDMDirectoryRequest				dirRequest;
	RsslReactorOAuthCredential			oauthCredential;

	time_t								stopTime;
	time_t								currentTime;
	time_t								nextPostTime;

	RsslBool							postWithMsg = RSSL_TRUE;

	RsslInitializeExOpts		initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
	RsslReactorWarmStandbyGroup			reactorWarmStandbyGroup;
	RsslReactorWarmStandbyServerInfo	standbyServerInfo;

	watchlistConsumerConfigInit(argc, argv);

	itemDecoderInit();
	postHandlerInit();
	initTunnelStreamMessaging();

	/* handle service updates for channel posting purposes */
	clearPostServiceInfo(&serviceInfo);

	stopTime = time(NULL);

	if (stopTime < 0)
	{
		printf("time() failed.\n");
		exit(-1);
	}

	nextPostTime = stopTime + POST_MESSAGE_FREQUENCY;
	stopTime += watchlistConsumerConfig.runTime;

	initOpts.jitOpts.libsslName = watchlistConsumerConfig.libsslName;
	initOpts.jitOpts.libcryptoName = watchlistConsumerConfig.libcryptoName;
	initOpts.jitOpts.libcurlName = watchlistConsumerConfig.libcurlName;
	initOpts.rsslLocking = RSSL_LOCK_GLOBAL_AND_CHANNEL;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitializeEx(&initOpts, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitializeEx(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		exit(-1);
	}


	/* Prepare a default login request(Use 1 as the Login Stream ID). 
	 * This function sets login request parameters according to what a consumer
	 * application would normally set. */
	if (rsslInitDefaultRDMLoginRequest(&loginRequest, LOGIN_STREAM_ID) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		exit(-1);
	}


	rsslClearReactorServiceDiscoveryOptions(&serviceDiscoveryOpts);
	/* Set login userName if specified. Otherwise, rsslInitDefaultRDMLoginRequest()
	 * will set it to the user's system login name. */
	if (watchlistConsumerConfig.userName.length)
	{
		loginRequest.userName = watchlistConsumerConfig.userName;
		
		if(watchlistConsumerConfig.queryEndpoint) 
			serviceDiscoveryOpts.userName = loginRequest.userName;
	}

	/* Set login password if specified. */
	if (watchlistConsumerConfig.password.length)
	{
		loginRequest.password = watchlistConsumerConfig.password;

		if(watchlistConsumerConfig.queryEndpoint) 
			serviceDiscoveryOpts.password = loginRequest.password;
	}

	/* Set client Id if specified. */
	if (watchlistConsumerConfig.clientId.length)
	{
		if (watchlistConsumerConfig.queryEndpoint)
			serviceDiscoveryOpts.clientId = watchlistConsumerConfig.clientId;
	}

	/* Set client Secret if specified. */
	if (watchlistConsumerConfig.clientSecret.length)
	{
		if (watchlistConsumerConfig.queryEndpoint)
			serviceDiscoveryOpts.clientSecret = watchlistConsumerConfig.clientSecret;
	}

	/* Set JWK if specified. */
	if (watchlistConsumerConfig.clientJWK.length)
	{
		if (watchlistConsumerConfig.queryEndpoint)
			serviceDiscoveryOpts.clientJWK = watchlistConsumerConfig.clientJWK;
	}

	/* Set Audience if specified. */
	if (watchlistConsumerConfig.audience.length)
	{
		if (watchlistConsumerConfig.queryEndpoint)
			serviceDiscoveryOpts.audience = watchlistConsumerConfig.audience;
	}
	
	/* If the authentication Token is specified, set it and authenticationExtended(if present) to the loginRequest */
	if (watchlistConsumerConfig.authenticationToken.length)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
		loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		loginRequest.userName = watchlistConsumerConfig.authenticationToken;
		
		if(watchlistConsumerConfig.authenticationExtended.length)
		{
			loginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
			loginRequest.authenticationExtended = watchlistConsumerConfig.authenticationExtended;
		}
	}
	
	if (watchlistConsumerConfig.appId.length)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
		loginRequest.applicationId = watchlistConsumerConfig.appId;
	}

	if (watchlistConsumerConfig.RTTSupport == RSSL_TRUE)
	{
		loginRequest.flags |= RDM_LG_RQF_RTT_SUPPORT;
	}

	/* Setup consumer role for connection. */
	rsslClearOMMConsumerRole(&consumerRole);
	consumerRole.pLoginRequest = &loginRequest;
	consumerRole.base.channelEventCallback = channelEventCallback;
	consumerRole.base.defaultMsgCallback = msgCallback;
	consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	consumerRole.watchlistOptions.channelOpenCallback = channelOpenCallback;

	consumerRole.loginMsgCallback = loginMsgCallback;

	/* Prepare a default directory request. */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, DIRECTORY_STREAM_ID) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		exit(-1);
	}

	consumerRole.pDirectoryRequest = &dirRequest;
	consumerRole.directoryMsgCallback = directoryMsgCallback;
	consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;

	if (watchlistConsumerConfig.clientId.data)
	{
		rsslClearReactorOAuthCredential(&oauthCredential);
		oauthCredential.clientId = watchlistConsumerConfig.clientId;
		if (watchlistConsumerConfig.clientSecret.data)
			oauthCredential.clientSecret = watchlistConsumerConfig.clientSecret;
		if (watchlistConsumerConfig.clientJWK.data)
			oauthCredential.clientJWK = watchlistConsumerConfig.clientJWK;
		if (watchlistConsumerConfig.audience.data)
			oauthCredential.audience = watchlistConsumerConfig.audience;
		oauthCredential.takeExclusiveSignOnControl = watchlistConsumerConfig.takeExclusiveSignOnControl;
		if (watchlistConsumerConfig.tokenScope.data)
			oauthCredential.tokenScope = watchlistConsumerConfig.tokenScope;
		consumerRole.pOAuthCredential = &oauthCredential;
		
		/* Specified the RsslReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
		consumerRole.pOAuthCredential->pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
	}


	/* Create Reactor. */
	rsslClearCreateReactorOptions(&reactorOpts);
	
	if (watchlistConsumerConfig.tokenURLV1.data != NULL)
	{
		reactorOpts.tokenServiceURL_V1 = watchlistConsumerConfig.tokenURLV1;
	}

	if (watchlistConsumerConfig.tokenURLV2.data != NULL)
	{
		reactorOpts.tokenServiceURL_V2 = watchlistConsumerConfig.tokenURLV2;
	}

	if (watchlistConsumerConfig.serviceDiscoveryURL.data != NULL)
	{
		reactorOpts.serviceDiscoveryURL = watchlistConsumerConfig.serviceDiscoveryURL;
	}

	if (watchlistConsumerConfig.restEnableLog || watchlistConsumerConfig.restEnableLogViaCallback > 0)
	{
		reactorOpts.restEnableLog = watchlistConsumerConfig.restEnableLog;
		reactorOpts.restLogOutputStream = watchlistConsumerConfig.restOutputStreamName;

		reactorOpts.restVerboseMode = watchlistConsumerConfig.restVerboseMode;
	}

	if (watchlistConsumerConfig.restEnableLogViaCallback > 0)
	{
		reactorOpts.pRestLoggingCallback = restLoggingCallback;
	}

	if (watchlistConsumerConfig.restEnableLogViaCallback == 1)
	{
		reactorOpts.restEnableLogViaCallback = RSSL_TRUE;
	}

	if (watchlistConsumerConfig.restProxyHost[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyHostName = watchlistConsumerConfig.restProxyHost;
	}

	if (watchlistConsumerConfig.restProxyPort[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPort = watchlistConsumerConfig.restProxyPort;
	}

	if (watchlistConsumerConfig.restProxyUserName[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyUserName = watchlistConsumerConfig.restProxyUserName;
	}

	if (watchlistConsumerConfig.restProxyPasswd[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPasswd = watchlistConsumerConfig.restProxyPasswd;
	}

	if (watchlistConsumerConfig.restProxyDomain[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyDomain = watchlistConsumerConfig.restProxyDomain;
	}

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	rsslClearReactorConnectOptions(&reactorConnectOpts);
	rsslClearReactorConnectInfo(&reactorConnectInfo);
	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);
	if(watchlistConsumerConfig.location.length == 0) // Use the default location from the Reactor if not specified
	{
		watchlistConsumerConfig.location.length =
                (RsslUInt32)snprintf(watchlistConsumerConfig._locationMem, 255, "%.*s", reactorConnectInfo.location.length, reactorConnectInfo.location.data);
            watchlistConsumerConfig.location.data = watchlistConsumerConfig._locationMem;					
	}

	if (watchlistConsumerConfig.queryEndpoint)
	{
		if (watchlistConsumerConfig.clientId.length)
			serviceDiscoveryOpts.clientId = watchlistConsumerConfig.clientId;

		if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			if (watchlistConsumerConfig.encryptedConnectionType == RSSL_CONN_TYPE_SOCKET)
			{
				serviceDiscoveryOpts.transport = RSSL_RD_TP_TCP;
			}
			else if (watchlistConsumerConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				serviceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;
			}
			else
			{
				printf("Error: Invalid encrypted connection type %d for querying RDP service discovery", watchlistConsumerConfig.encryptedConnectionType);
				exit(-1);
			}
		}
		else
		{
			printf("Error: Invalid connection type %d for querying RDP service discovery", watchlistConsumerConfig.connectionType);
			exit(-1);
		}

		serviceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

		/* Note: If RsslCreateReactorOptions.restProxyOptions are set when creating the Reactor, */
		/* the serviceDiscoveryOpts proxy settings will not take affect for service discovery done in application: */
		/* proxyHostName, proxyPort, proxyUserName, proxyPasswd, proxyDomain. */

		if(rsslReactorQueryServiceDiscovery(pReactor, &serviceDiscoveryOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error: %s\n", rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}

	/* Setup connection options. */
	if (watchlistConsumerConfig.connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		reactorConnectInfo.rsslConnectOptions.connectionType = watchlistConsumerConfig.connectionType;
		if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && watchlistConsumerConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
			reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = watchlistConsumerConfig.encryptedConnectionType;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.hostName;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.port;
		reactorConnectInfo.rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
		reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyHostName = watchlistConsumerConfig.proxyHost;
		reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPort = watchlistConsumerConfig.proxyPort;
		reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyUserName = watchlistConsumerConfig.proxyUserName;
		reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPasswd = watchlistConsumerConfig.proxyPasswd;
		reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyDomain = watchlistConsumerConfig.proxyDomain;
		reactorConnectInfo.enableSessionManagement = watchlistConsumerConfig.enableSessionMgnt;
		reactorConnectInfo.location = watchlistConsumerConfig.location;
		reactorConnectInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = watchlistConsumerConfig.sslCAStore;
		if (watchlistConsumerConfig.tlsProtocol != RSSL_ENC_NONE)
		{
			reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = watchlistConsumerConfig.tlsProtocol;
		}
		if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET || watchlistConsumerConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = watchlistConsumerConfig.protocolList;
		}

		if (watchlistConsumerConfig.warmStandbyMode != RSSL_RWSB_MODE_NONE)
		{
			rsslClearReactorWarmStandbyGroup(&reactorWarmStandbyGroup);
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = watchlistConsumerConfig.connectionType;
			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && watchlistConsumerConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
				reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = watchlistConsumerConfig.encryptedConnectionType;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.startingHostName;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.startingPort;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyHostName = watchlistConsumerConfig.proxyHost;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPort = watchlistConsumerConfig.proxyPort;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyUserName = watchlistConsumerConfig.proxyUserName;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPasswd = watchlistConsumerConfig.proxyPasswd;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyDomain = watchlistConsumerConfig.proxyDomain;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.enableSessionManagement = watchlistConsumerConfig.enableSessionMgnt;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.location = watchlistConsumerConfig.location;
			reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = watchlistConsumerConfig.sslCAStore;
			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET || watchlistConsumerConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				reactorWarmStandbyGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = watchlistConsumerConfig.protocolList;
			}

			rsslClearReactorWarmStandbyServerInfo(&standbyServerInfo);
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionType = watchlistConsumerConfig.connectionType;
			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && watchlistConsumerConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
				standbyServerInfo.reactorConnectInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = watchlistConsumerConfig.encryptedConnectionType;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.standbyHostName;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.standbyPort;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyHostName = watchlistConsumerConfig.proxyHost;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPort = watchlistConsumerConfig.proxyPort;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyUserName = watchlistConsumerConfig.proxyUserName;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyPasswd = watchlistConsumerConfig.proxyPasswd;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.proxyOpts.proxyDomain = watchlistConsumerConfig.proxyDomain;
			standbyServerInfo.reactorConnectInfo.enableSessionManagement = watchlistConsumerConfig.enableSessionMgnt;
			standbyServerInfo.reactorConnectInfo.location = watchlistConsumerConfig.location;
			standbyServerInfo.reactorConnectInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = watchlistConsumerConfig.sslCAStore;
			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_WEBSOCKET || watchlistConsumerConfig.encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				standbyServerInfo.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = watchlistConsumerConfig.protocolList;
			}

			reactorWarmStandbyGroup.standbyServerCount = 1;
			reactorWarmStandbyGroup.standbyServerList = &standbyServerInfo;
			reactorWarmStandbyGroup.warmStandbyMode = watchlistConsumerConfig.warmStandbyMode;
			reactorConnectOpts.warmStandbyGroupCount = 1;
			reactorConnectOpts.reactorWarmStandbyGroupList = &reactorWarmStandbyGroup;
		}
	}
	else
	{
		/* Segmented multicast connection. */
		reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.sendAddress = watchlistConsumerConfig.sendAddress;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.recvAddress = watchlistConsumerConfig.recvAddress;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.sendServiceName = watchlistConsumerConfig.sendPort;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.recvServiceName = watchlistConsumerConfig.recvPort;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.unicastServiceName = watchlistConsumerConfig.unicastPort;
		reactorConnectInfo.rsslConnectOptions.multicastOpts.flags = RSSL_MCAST_FILTERING_ON;

		if (strlen(watchlistConsumerConfig.interface))
			reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.interfaceName = watchlistConsumerConfig.interface;

		if (watchlistConsumerConfig.enableHostStatMessages)
		{
			/* Configure transport to publish Host Statistics Messages. */
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmMultAddress = watchlistConsumerConfig.hsmAddress;
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmPort = watchlistConsumerConfig.hsmPort;
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmInterval = watchlistConsumerConfig.hsmInterval;
			if (strlen(watchlistConsumerConfig.hsmInterface))
				reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmInterface = watchlistConsumerConfig.hsmInterface;
		}

	}

	reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	/* Use one connection, with an unlimited number of reconnect attempts. */
	reactorConnectOpts.reactorConnectionList = &reactorConnectInfo;
	reactorConnectOpts.connectionCount = 1;
	reactorConnectOpts.reconnectAttemptLimit = -1;
	reactorConnectOpts.reconnectMinDelay = 500;
	reactorConnectOpts.reconnectMaxDelay = 3000;

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	FD_SET(pReactor->eventFd, &readFds);
	FD_SET(pReactor->eventFd, &exceptFds);

	/* Connect. */
	if ((ret = rsslReactorConnect(pReactor, &reactorConnectOpts, 
					(RsslReactorChannelRole*)&consumerRole, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorConnect() failed: %d(%s)\n", 
				ret, rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	jsonConverterOptions.pDictionary = &dictionary;
	jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdCallback;
	jsonConverterOptions.pJsonConversionEventCallback = jsonConversionEventCallback;

	if (watchlistConsumerConfig.jsonOutputBufferSize > 0)
	{
		jsonConverterOptions.outputBufferSize = watchlistConsumerConfig.jsonOutputBufferSize;
	}
	if (watchlistConsumerConfig.jsonTokenIncrementSize > 0)
	{
		jsonConverterOptions.jsonTokenIncrementSize = watchlistConsumerConfig.jsonTokenIncrementSize;
	}

	if (rsslReactorInitJsonConverter(pReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error initializing RWF/JSON Converter: %s\n", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	if (watchlistConsumerConfig.restEnableLogViaCallback == 2)  // enabled after initialization stage
	{
		RsslInt value = 1;

		if (rsslReactorIoctl(pReactor, RSSL_RIC_ENABLE_REST_CALLBACK_LOGGING, &value, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error initialization Rest callback logging: %s\n", rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}

	/* Dispatch until application stops. */
	do
	{
		struct timeval 				selectTime;
		RsslReactorDispatchOptions	dispatchOpts;
		RsslUInt32					index;

		FD_ZERO(&readFds);
		FD_ZERO(&exceptFds);
		FD_SET(pReactor->eventFd, &readFds);
		FD_SET(pReactor->eventFd, &exceptFds);

		if (pConsumerChannel)
		{
			if (pConsumerChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				if (pConsumerChannel->pRsslChannel && pConsumerChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
				{
					FD_SET(pConsumerChannel->socketId, &readFds);
					FD_SET(pConsumerChannel->socketId, &exceptFds);
				}
			}
			else if (pConsumerChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				for (index = 0; index < socketIdListCount; index++)
				{
					FD_SET(socketIdList[index], &readFds);
					FD_SET(socketIdList[index], &exceptFds);
				}
			}
		}

		selectTime.tv_sec = 1; selectTime.tv_usec = 0;
		ret = select(FD_SETSIZE, &readFds, NULL, &exceptFds, &selectTime);

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
			exit(-1);
		}

		rsslClearReactorDispatchOptions(&dispatchOpts);
		
		/* Call rsslReactorDispatch().  This will handle any events that have occurred on its channels.
		 * If there are events or messages for the application to process, they will be delivered
		 * through the callback functions given by the consumerRole object. 
		 * A return value greater than RSSL_RET_SUCCESS indicates there may be more to process. */
		while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
			exit(-1);
		}

		if ((currentTime = time(NULL)) < 0)
		{
			printf("time() failed.\n");
		}

		if (pConsumerChannel && !runTimeExpired)
		{
			if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
				handleSimpleTunnelMsgHandler(pReactor, pConsumerChannel, &simpleTunnelMsgHandler);

			/* Handle posting, if configured. */
			if (currentTime >= nextPostTime)
			{
				nextPostTime = currentTime + POST_MESSAGE_FREQUENCY;

				if (watchlistConsumerConfig.post && isConsumerChannelUp && serviceInfo.isServiceFound && serviceInfo.isServiceUp)
					sendOnStreamPostMsg(pReactor, pConsumerChannel, postWithMsg);

				if (watchlistConsumerConfig.offPost && isConsumerChannelUp && serviceInfo.isServiceFound && serviceInfo.isServiceUp)
					sendOffStreamPostMsg(pReactor, pConsumerChannel, postWithMsg);

				if (postWithMsg)
					postWithMsg = RSSL_FALSE;
				else
					postWithMsg = RSSL_TRUE;
			}
		}

		if (currentTime >= stopTime)
		{
			if (!runTimeExpired)
			{
				runTimeExpired = RSSL_TRUE;
				printf("Run time expired.\n");
				if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
					printf("Waiting for tunnel stream to close...\n\n");

				/* Close tunnel streams if any are open. */
				simpleTunnelMsgHandlerCloseStreams(&simpleTunnelMsgHandler);
			}

			/* Wait for tunnel streams to close before closing channel. */
			if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream == NULL)
				break;
			else if (currentTime >= stopTime + 10)
			{
				printf("Tunnel stream still open after ten seconds, giving up.\n");
				break;
			}
		}

		// send login reissue if login reissue time has passed
		if (canSendLoginReissue == RSSL_TRUE &&
			currentTime >= (time_t)loginReissueTime)
		{
			RsslReactorSubmitMsgOptions submitMsgOpts;
			RsslErrorInfo rsslErrorInfo;

			// APIQA: Changing token to new one on reissue
			loginRequest.userName = watchlistConsumerConfig.authenticationToken2;
			// END APIQA

			rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
			submitMsgOpts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
			if ((ret = rsslReactorSubmitMsg(pReactor,pConsumerChannel,&submitMsgOpts,&rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("Login reissue failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			}
			else
			{
				printf("Login reissue sent\n");
			}
			canSendLoginReissue = RSSL_FALSE;
		}
	} while(ret >= RSSL_RET_SUCCESS);

	/* Clean up and exit. */
	clearPostServiceInfo(&serviceInfo);

	if (pConsumerChannel)
	{
		if ((ret = rsslReactorCloseChannel(pReactor, pConsumerChannel, &rsslErrorInfo))
				!= RSSL_RET_SUCCESS)
		{
			printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}

	if ((ret = rsslDestroyReactor(pReactor, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	if (reactorOpts.restLogOutputStream)
		fclose(reactorOpts.restLogOutputStream);

	rsslUninitialize();

	itemDecoderCleanup();
	
	exit(0);
}

/* When requesting symbol list items, this application will also request data streams from
 * those items.  This function demonstrates how to encode the item request payload to request 
 * data streams. */
RsslRet encodeSymbolListDataStreamsPayload(RsslBuffer *pDataBody, RsslUInt dataStreamFlags)
{
        RsslEncodeIterator eIter;
        RsslElementList eList, eBehaviorsList;
        RsslElementEntry eEntry, eDataStreamsEntry;
        RsslRet ret;

        rsslClearEncodeIterator(&eIter);
        rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
        rsslSetEncodeIteratorBuffer(&eIter, pDataBody);

        rsslClearElementList(&eList);
        eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode symbol list behaviors element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_SYMBOL_LIST_BEHAVIORS;
        eEntry.dataType = RSSL_DT_ELEMENT_LIST;
        if ((ret = rsslEncodeElementEntryInit(&eIter, &eEntry, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        rsslClearElementList(&eBehaviorsList);
        eBehaviorsList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eBehaviorsList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode data streams element. */
        rsslClearElementEntry(&eDataStreamsEntry);
        eDataStreamsEntry.name = RSSL_ENAME_DATA_STREAMS;
        eDataStreamsEntry.dataType = RSSL_DT_UINT;
        if ((ret = rsslEncodeElementEntry(&eIter, &eDataStreamsEntry, &dataStreamFlags)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementEntryComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        pDataBody->length = rsslGetEncodedBufferLength(&eIter);

        return RSSL_RET_SUCCESS;
}

/* Demonstrates encoding of a field list view in a request payload. */
RsslRet encodeViewPayload(RsslBuffer *pDataBody, RsslInt *fieldIdList, RsslUInt32 fieldIdCount)
{
        RsslEncodeIterator eIter;
        RsslElementList eList;
        RsslElementEntry eEntry;
        RsslRet ret;
        RsslUInt viewType;
        RsslArray rsslArray;

        RsslUInt32 i;

        rsslClearEncodeIterator(&eIter);
        rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
        rsslSetEncodeIteratorBuffer(&eIter, pDataBody);

        rsslClearElementList(&eList);
        eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode view type (field ID list) into the ViewType element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_VIEW_TYPE;
        eEntry.dataType = RSSL_DT_UINT;
        viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
        if ((ret = rsslEncodeElementEntry(&eIter, &eEntry, &viewType)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode the fields desired into the ViewData element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_VIEW_DATA;
        eEntry.dataType = RSSL_DT_ARRAY;
        viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
        if ((ret = rsslEncodeElementEntryInit(&eIter, &eEntry, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        rsslClearArray(&rsslArray);
        rsslArray.primitiveType = RSSL_DT_INT;
        rsslArray.itemLength = 2;
        if ((ret = rsslEncodeArrayInit(&eIter, &rsslArray)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementArrayInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

		/* Encode each desired field into the array. */
        for (i = 0; i < fieldIdCount; ++i)
        {
                if ((ret = rsslEncodeArrayEntry(&eIter, NULL, &fieldIdList[i])) != RSSL_RET_SUCCESS)
                {
                        printf("rsslEncodeArrayEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                        return ret;
                }
        }

        if ((ret = rsslEncodeArrayComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeArrayComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementEntryComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        pDataBody->length = rsslGetEncodedBufferLength(&eIter);

        return RSSL_RET_SUCCESS;
}

// APIQA: Adding ability to send item PAUSE 
RsslRet reissueItem(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, int streamId, int shouldPause)
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
//	RsslUInt32 itemListIndex;

	char dataBodyBuf[512];
	RsslBuffer dataBody = { 512, dataBodyBuf };
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	if (shouldPause == 1)
	{
		requestMsg.flags |= RSSL_RQMF_PAUSE;
		requestMsg.flags |= RSSL_RQMF_NO_REFRESH;
	}
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.domainType =
		watchlistConsumerConfig.itemList[streamId].domainType;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;
	requestMsg.msgBase.msgKey.name = watchlistConsumerConfig.itemList[streamId].name;
	requestMsg.msgBase.streamId = watchlistConsumerConfig.itemList[streamId].streamId;

	if (shouldPause == 1)
		printf("APIQA: Sending PAUSE on item stream: %d \n", watchlistConsumerConfig.itemList[streamId].streamId);
	else
		printf("APIQA: Sending ITEM RESUME on item stream: %d \n", watchlistConsumerConfig.itemList[streamId].streamId);

	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
		&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
			ret, rsslErrorInfo.rsslError.text);
	}
	return ret;
}
// END APIQA 

/* Requests the desired items.  */
RsslRet requestItems(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslUInt32 itemListIndex;

	if (itemsRequested)
		return RSSL_RET_SUCCESS;

	itemsRequested = RSSL_TRUE;


	for(itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount; ++itemListIndex)
	{
		char dataBodyBuf[512];
		RsslBuffer dataBody = { 512, dataBodyBuf };

		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
		requestMsg.flags = RSSL_RQMF_STREAMING;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		requestMsg.msgBase.domainType =
			watchlistConsumerConfig.itemList[itemListIndex].domainType;

		rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

		submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
		submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;


		if (requestMsg.msgBase.domainType == RSSL_DMT_SYMBOL_LIST && 
				watchlistConsumerConfig.itemList[itemListIndex].symbolListData)
		{
			/* Encode the message payload with a request for data streams of the items
			 * received in the symbol list response. This will cause items present in
			 * response messages to be automatically opened on behalf of the application. */
			RsslUInt dataStreamFlags = RDM_SYMBOL_LIST_DATA_STREAMS;

			requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
			if (encodeSymbolListDataStreamsPayload(&dataBody, dataStreamFlags) != RSSL_RET_SUCCESS)
			{
				printf("encodeSymbolListDataStreamsPayload() failed.");
				exit(-1);
			}
			requestMsg.msgBase.encDataBody = dataBody;
		}

		/* If desired, request a view for Market Price items. */
		if (requestMsg.msgBase.domainType == RSSL_DMT_MARKET_PRICE && watchlistConsumerConfig.setView)
		{
			RsslInt fieldIdList[] = { 22, 25, 30, 31, 1025 };
			RsslUInt32 fieldIdCount = 5;

			requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
			requestMsg.flags |= RSSL_RQMF_HAS_VIEW;
			if (encodeViewPayload(&dataBody, fieldIdList, fieldIdCount) != RSSL_RET_SUCCESS)
			{
				printf("encodeViewPayload() failed.");
				exit(-1);
			}
			requestMsg.msgBase.encDataBody = dataBody;

		}


		requestMsg.msgBase.msgKey.name = watchlistConsumerConfig.itemList[itemListIndex].name;
		requestMsg.msgBase.streamId = watchlistConsumerConfig.itemList[itemListIndex].streamId;
		if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
						&rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
					ret, rsslErrorInfo.rsslError.text);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/* Requests the field dictionary and enumerated types dictionary. */
RsslRet requestDictionaries(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;

	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = RDM_DICTIONARY_NORMAL;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;

	requestMsg.msgBase.msgKey.name = fieldDictionaryName;
	requestMsg.msgBase.streamId = FIELD_DICTIONARY_STREAM_ID;
	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	requestMsg.msgBase.msgKey.name = enumDictionaryName;
	requestMsg.msgBase.streamId = ENUM_DICTIONARY_STREAM_ID;
	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}


/* Prints out information from an RsslState. */
static void printRsslState(const RsslState *pState)
{
	char stateString[256];
	RsslBuffer stateBuffer = { 256, stateString };

	if (rsslStateToString(&stateBuffer, (RsslState*)pState) != RSSL_RET_SUCCESS)
	{ printf("(rsslStateToString() failed)\n"); }

	printf("%.*s\n", stateBuffer.length, stateBuffer.data);
}

/* Prints information about a stream. */
static void printStreamInfo(RsslBuffer *pItemName, RsslUInt8 domainType, RsslInt32 streamId,
		const char *msgClass)
{
	if (pItemName && pItemName->data)
		printf(	"Name: %-20.*s   ", pItemName->length, pItemName->data);

	printf(	"Domain: %s\n", rsslDomainTypeToString(domainType));

	printf( "Stream: %-20d Msg Class: %s\n", streamId, msgClass);


}

/* Callback function for login message responses. */
static RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pMsgEvent)
{
	RsslRDMLoginMsg *pLoginMsg = pMsgEvent->pRDMLoginMsg;

	if (!pLoginMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Login message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REFRESH:
		{
			RsslRDMLoginRefresh *pLoginRefresh = &pLoginMsg->refresh;

			printStreamInfo(NULL, RSSL_DMT_LOGIN, pLoginMsg->rdmMsgBase.streamId, "RDM_LG_MT_REFRESH");
			printRsslState(&pLoginRefresh->state);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME)
				printf("  UserName: %.*s\n", pLoginRefresh->userName.length, pLoginRefresh->userName.data);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
				printf("  AuthenticationTTReissue: %llu\n", pLoginRefresh->authenticationTTReissue);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
				printf("  AuthenticationExtendedResp: %.*s\n", pLoginRefresh->authenticationExtendedResp.length, pLoginRefresh->authenticationExtendedResp.data);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginRefresh->authenticationErrorCode);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginRefresh->authenticationErrorText.length, pLoginRefresh->authenticationErrorText.data);

			// get login reissue time from authenticationTTReissue
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
			{
				loginReissueTime = pLoginRefresh->authenticationTTReissue;
				canSendLoginReissue = watchlistConsumerConfig.enableSessionMgnt ? RSSL_FALSE : RSSL_TRUE;
			}

			break;
		}

		case RDM_LG_MT_STATUS:
		{
			RsslRDMLoginStatus *pLoginStatus = &pLoginMsg->status;

			printStreamInfo(NULL, RSSL_DMT_LOGIN, pLoginMsg->rdmMsgBase.streamId, "RDM_LG_MT_STATUS");

			if (pLoginStatus->flags & RDM_LG_STF_HAS_STATE)
				printRsslState(&pLoginStatus->state);

			if (pLoginStatus->flags & RDM_LG_STF_HAS_USERNAME)
				printf("  UserName: %.*s\n", pLoginStatus->userName.length, pLoginStatus->userName.data);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginStatus->authenticationErrorCode);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginStatus->authenticationErrorText.length, pLoginStatus->authenticationErrorText.data);

			break;
		}
		case RDM_LG_MT_RTT:
			printf("\nReceived Login RTT Msg\n");
			printf("	Ticks: %llu\n", pLoginMsg->RTT.ticks);
			if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_LATENCY)
				printf("	Last Latency: %llu\n", pLoginMsg->RTT.lastLatency);
			if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_TCP_RETRANS)
				printf("	TCP Retransmissions: %llu\n", pLoginMsg->RTT.tcpRetrans);
			if (pMsgEvent->flags & RSSL_RDM_LG_LME_RTT_RESPONSE_SENT)
				printf("RTT Response sent to provider.\n");
			break;

		default:
			printf("\n  Received Unhandled Login Msg Type: %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	printf("\n");

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for directory message responses. 
 * As many of the behaviors normally performed by the application in response to directory
 * messages(e.g. item recovery or status fanout) are automatically handled by the watchlist, 
 * these messages are primarily informational. */
static RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent* pMsgEvent)
{
	RsslUInt32 i;
	RsslRDMDirectoryMsg *pDirectoryMsg = pMsgEvent->pRDMDirectoryMsg;
	RsslRDMService *serviceList = NULL; 
	RsslUInt32 serviceCount = 0;

	if (!pDirectoryMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Directory message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
		{
			RsslRDMDirectoryRefresh *pDirectoryRefresh = &pDirectoryMsg->refresh;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_REFRESH");
			printRsslState(&pDirectoryRefresh->state);

			serviceList = pDirectoryRefresh->serviceList;
			serviceCount = pDirectoryRefresh->serviceCount;

			break;
		}
		case RDM_DR_MT_UPDATE:
		{
			RsslRDMDirectoryUpdate *pDirectoryUpdate = &pDirectoryMsg->update;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_UPDATE");

			serviceList = pDirectoryUpdate->serviceList;
			serviceCount = pDirectoryUpdate->serviceCount;

			break;
		}
		case RDM_DR_MT_STATUS:
		{
			RsslRDMDirectoryStatus *pDirectoryStatus = &pDirectoryMsg->status;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_STATUS");

			if (pDirectoryStatus->flags & RDM_DR_STF_HAS_STATE)
				printRsslState(&pDirectoryMsg->status.state);

			break;
		}

		default:
			printf("\n  Received Unhandled Source Directory Msg Type: %d\n", pDirectoryMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	/* Refresh and update messages contain updates to service information. */
	if (serviceList)
	{
		for (i = 0; i <  serviceCount; ++i)
		{
			RsslRDMService *pService = &serviceList[i];
			char tmpBuf[128];

			/* Print service ID (and service name, if present). */
			if (pService->flags & RDM_SVCF_HAS_INFO)
			{
				snprintf(tmpBuf, 128, "%llu(%.*s)", pService->serviceId, 
						pService->info.serviceName.length, pService->info.serviceName.data);

			}
			else
				snprintf(tmpBuf, 128, "%llu", pService->serviceId);

			printf("  Service: %-20s Action: %s\n", tmpBuf, mapActionToString(pService->action));

			if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
				tunnelStreamHandlerProcessServiceUpdate(&simpleTunnelMsgHandler.tunnelStreamHandler,
				&watchlistConsumerConfig.tunnelStreamServiceName, pService);

			/* handle service updates for channel posting purposes */
			processPostServiceUpdate(&serviceInfo, &watchlistConsumerConfig.serviceName, pService);
		}
	}

	printf("\n");

	if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
	{
		if (!simpleTunnelMsgHandler.tunnelStreamHandler.isTunnelServiceFound)
			printf("  Directory response does not contain service name for tunnel streams: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
		else if (!simpleTunnelMsgHandler.tunnelStreamHandler.tunnelServiceSupported)
			printf("  Service in use for tunnel streams does not support them: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
	}

	/* handle service updates for channel posting purposes */
	if (watchlistConsumerConfig.post || watchlistConsumerConfig.offPost)
	{
		if (!serviceInfo.isServiceFound)
			printf("  Directory response does not contain service name to send post messages: %s\n\n",
				watchlistConsumerConfig.serviceName.data);
		else if (!serviceInfo.isServiceUp)
			printf("  Service in use to send post messages is down: %s\n\n",
				watchlistConsumerConfig.serviceName.data);
	}


	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for dictionary message responses. */
static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDictionaryMsgEvent* pMsgEvent)
{
	RsslRDMDictionaryMsg *pDictionaryMsg = pMsgEvent->pRDMDictionaryMsg;

	if (!pDictionaryMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Dictionary message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REFRESH:
		{
			RsslRDMDictionaryRefresh *pDictionaryRefresh = &pDictionaryMsg->refresh;

			printStreamInfo(&pDictionaryRefresh->dictionaryName,RSSL_DMT_DICTIONARY, 
					pDictionaryRefresh->rdmMsgBase.streamId, "RDM_DC_MT_REFRESH");

			printRsslState(&pDictionaryRefresh->state);

			decodeDictionaryDataBody(pChannel, pDictionaryRefresh);
			if (fieldDictionaryLoaded && enumDictionaryLoaded)
				requestItems(pReactor, pChannel);

			break;
		}

		case RDM_DC_MT_STATUS:
		{
			RsslRDMDictionaryStatus *pDictionaryStatus = &pDictionaryMsg->status;
			RsslBuffer *pDictionaryName = NULL;

			switch(pDictionaryMsg->rdmMsgBase.streamId)
			{
				case FIELD_DICTIONARY_STREAM_ID:
					pDictionaryName = &fieldDictionaryName; break;
				case ENUM_DICTIONARY_STREAM_ID:
					pDictionaryName = &enumDictionaryName; break;
				default:
					printf("Error: Received dictionary status message on unknown stream %d\n",
							pDictionaryMsg->rdmMsgBase.streamId);
					return RSSL_RC_CRET_SUCCESS;

			}

			printStreamInfo(pDictionaryName, RSSL_DMT_DICTIONARY, 
					pDictionaryStatus->rdmMsgBase.streamId, "RDM_DC_MT_STATUS");

			if (pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE)
				printRsslState(&pDictionaryStatus->state);

			break;
		}

		default:
			printf("\n  Received Unhandled Dictionary Msg Type: %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	printf("\n");

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for authentication credential events, particularily credential renewal */
static RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor* pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent)
{
	RsslReactorOAuthCredentialRenewalOptions renewalOptions;
	RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
	RsslErrorInfo rsslError;

	rsslClearReactorOAuthCredentialRenewalOptions(&renewalOptions);
	renewalOptions.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;

	rsslClearReactorOAuthCredentialRenewal(&reactorOAuthCredentialRenewal);

	if (watchlistConsumerConfig.password.length != 0)
		reactorOAuthCredentialRenewal.password = watchlistConsumerConfig.password; /* Specified password as needed */
	else if (watchlistConsumerConfig.clientSecret.length != 0)
		reactorOAuthCredentialRenewal.clientSecret = watchlistConsumerConfig.clientSecret;
	else
		reactorOAuthCredentialRenewal.clientJWK = watchlistConsumerConfig.clientJWK;

	rsslReactorSubmitOAuthCredentialRenewal(pReactor, &renewalOptions, &reactorOAuthCredentialRenewal, &rsslError);

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for message responses. 
 * Messages regarding items opened by the application are handled here. 
 *
 * This callback handles provider-driven streams, which this application will receive
 * when requesting data with any symbol list request. As new item streams are received,
 * it will store the stream's information for display.
 *
 **/
static RsslReactorCallbackRet msgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent)
{
	const RsslState *pState = NULL;
	RsslMsg *pRsslMsg = pMsgEvent->pRsslMsg;
	RsslBuffer *pItemName = NULL;
	ItemInfo *pItem = NULL;

	if (!pRsslMsg)
	{
		printf("Message Error.\n");
		if (pMsgEvent->pErrorInfo)
			printf("	Error text: %s\n\n", pMsgEvent->pErrorInfo->rsslError.text);
		return RSSL_RC_CRET_SUCCESS;
	}

	/* Get stored item information associated with this stream, if any. 
	 * This may be used to print out item information such as the name. */
	pItem = getItemInfo(pRsslMsg->msgBase.streamId);

	if (pMsgEvent->pSeqNum || pMsgEvent->pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->pSeqNum);

		if (pMsgEvent->pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->pFTGroupId);
		printf("\n");
	}

	switch(pRsslMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;

				/* Store new item information, if not currently stored and stream is 
				 * provider-driven. */
				if (!pItem && pRsslMsg->msgBase.streamId < 0)
					pItem = addProvidedItemInfo(pRsslMsg->msgBase.streamId,
							&pRsslMsg->msgBase.msgKey, pRsslMsg->msgBase.domainType);

			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printRsslState(&pRsslMsg->refreshMsg.state);
			pState = &pRsslMsg->refreshMsg.state;


			/* Decode data body according to its domain. */
			decodeDataBody(pChannel, pRsslMsg);
			break;


		case RSSL_MC_UPDATE:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			/* Decode data body according to its domain. */
			decodeDataBody(pChannel, pRsslMsg);

			// APIQA: Sending reissues 
			eventCounter++;
			if (eventCounter == 5)
			{
				reissueItem(pReactor, pChannel, 0, 1);
			}
			if (eventCounter == 10)
			{
				reissueItem(pReactor, pChannel, 1, 1);
			}
			if (eventCounter == 25)
			{
				reissueItem(pReactor, pChannel, 0, 0);
			}
			if (eventCounter == 30)
			{
				reissueItem(pReactor, pChannel, 1, 0);
			}
			// END APIQA: Sending reissues 
			break;

		case RSSL_MC_STATUS:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;

				/* Store new item information, if not currently stored and stream is 
				 * provider-driven. */
				if (!pItem && pRsslMsg->msgBase.streamId < 0)
					pItem = addProvidedItemInfo(pRsslMsg->msgBase.streamId,
							&pRsslMsg->msgBase.msgKey, pRsslMsg->msgBase.domainType);
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			if (pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				printRsslState(&pRsslMsg->statusMsg.state);
				pState = &pRsslMsg->statusMsg.state;
			}

			printf("\n");
			break;

		case RSSL_MC_ACK:

			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printf("  ackId:   %u\n", pRsslMsg->ackMsg.ackId);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				printf("  seqNum:  %u\n", pRsslMsg->ackMsg.seqNum);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
				printf("  nakCode: %u\n", pRsslMsg->ackMsg.nakCode);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				printf("  text:    %.*s\n", pRsslMsg->ackMsg.text.length, pRsslMsg->ackMsg.text.data);
			printf("\n");
			break;

		default:
			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printf("  Error: Unhandled message class.\n\n");
			break;
	}

	/* Check state of any provider-driven streams.
	 * If the state indicates the item was closed, remove it from our list. */
	if (pState && pItem && pRsslMsg->msgBase.streamId < 0 && pState->streamState != RSSL_STREAM_OPEN)
		removeProvidedItemInfo(pItem);

	return RSSL_RC_CRET_SUCCESS;
}

static void clearConnection(RsslReactorChannel *pReactorChannel)
{
	if (pReactorChannel == NULL)
	{
		printf("Error in clearConnection (). pReactorChannel is not SET.");
		return;
	}

	if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
	{
		FD_CLR(pReactorChannel->socketId, &readFds);
		FD_CLR(pReactorChannel->socketId, &exceptFds);
	}
	else if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
	{
		RsslUInt32 index;

		for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
		{
			FD_CLR(socketIdList[index], &readFds);
			FD_CLR(socketIdList[index], &exceptFds);
		}
	}
}

/* Callback for when the channel is first opened by the application.
 * If dictionaries are loaded, items will immediately be requested.
 * Otherwise, dictionaries must be retrieved before requesting items. 
 * Note that at the channel is not yet up and should only be used
 * to open requests. */
RsslReactorCallbackRet channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	if (fieldDictionaryLoaded && enumDictionaryLoaded)
		requestItems(pReactor, pReactorChannel);
	else
		requestDictionaries(pReactor, pReactorChannel);

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for channel events. */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* Save the channel on our info structure. */
			pConsumerChannel = pReactorChannel;
			if (pConsumerChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				FD_SET(pReactorChannel->socketId, &readFds);
				FD_SET(pReactorChannel->socketId, &exceptFds);
			}
			else if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					socketIdList[index] = pConsumerChannel->pWarmStandbyChInfo->socketIdList[index];
				}

				socketIdListCount = pConsumerChannel->pWarmStandbyChInfo->socketIdCount;
			}
			printf("Channel "SOCKET_PRINT_TYPE" is up!\n\n", pReactorChannel->socketId);

			RsslChannelInfo rsslChannelInfo;
			RsslError rsslError;
			rsslGetChannelInfo(pReactorChannel->pRsslChannel, &rsslChannelInfo, &rsslError);
			switch (rsslChannelInfo.encryptionProtocol)
			{
			case RSSL_ENC_TLSV1_2:
				printf("Encryption protocol: TLSv1.2\n\n");
				break;
			case RSSL_ENC_TLSV1_3:
				printf("Encryption protocol: TLSv1.3\n\n");
				break;
			default:
				printf("Encryption protocol: unknown\n\n");
			}

			if (isXmlTracingEnabled() == RSSL_TRUE) 
			{
				RsslTraceOptions traceOptions;
				char traceOutputFile[128];
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				snprintf(traceOutputFile, 128, "rsslWatchlistConsumer");
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ | RSSL_TRACE_DUMP;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			isConsumerChannelUp = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
			return RSSL_RC_CRET_SUCCESS;
		case RSSL_RC_CET_FD_CHANGE:
		{
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				FD_CLR(pReactorChannel->oldSocketId, &readFds);
				FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
				FD_SET(pReactorChannel->socketId, &readFds);
				FD_SET(pReactorChannel->socketId, &exceptFds);
			}
			else if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					socketIdList[index] = pConsumerChannel->pWarmStandbyChInfo->socketIdList[index];
				}

				socketIdListCount = pConsumerChannel->pWarmStandbyChInfo->socketIdCount;
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			RsslErrorInfo rsslErrorInfo;

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				printf("Channel "SOCKET_PRINT_TYPE" down.\n", pReactorChannel->socketId);
				clearConnection(pReactorChannel);
			}
			else
				printf("Channel down.\n");

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			pConsumerChannel = NULL;
			rsslReactorCloseChannel(pReactor, pReactorChannel, &rsslErrorInfo);
			exit(-1);

			isConsumerChannelUp = RSSL_FALSE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			RsslChannel *pRsslChannel = pReactorChannel->pRsslChannel;
			char hostName[512];

			memset(hostName, 0, 512);

			if (pRsslChannel != NULL && pRsslChannel->hostname != NULL)
			{
				memcpy(hostName, pRsslChannel->hostname, strlen(pRsslChannel->hostname));
			}

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				printf("Channel "SOCKET_PRINT_TYPE" down. Reconnecting hostname %s\n", pReactorChannel->socketId, hostName);
				clearConnection(pReactorChannel);
			}
			else
				printf("Channel down. Reconnecting hostname %s\n",  hostName);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			socketIdListCount = 0;
			isConsumerChannelUp = RSSL_FALSE;
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
			exit(-1);
		}
	}


	return RSSL_RC_CRET_SUCCESS;
}

/* Initialize TunnelStreamHandler object, in which we store information about that tunnel stream
 * we will open. */
static void initTunnelStreamMessaging()
{
	simpleTunnelMsgHandlerInit(&simpleTunnelMsgHandler, (char*)"WatchlistConsumer", 
			watchlistConsumerConfig.tunnelStreamDomainType,
			watchlistConsumerConfig.useAuthentication, RSSL_FALSE);
}

RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pEndPointEvent)
{
	RsslUInt32 index;
	RsslReactorServiceEndpointInfo *pServiceEndpointInfo;
	char *endPoint = NULL;
	char *port = NULL;

	if (pEndPointEvent->pErrorInfo != NULL)
	{
		printf("Error requesting Service Discovery Endpoint Information: %s\n", pEndPointEvent->pErrorInfo->rsslError.text);
		exit(-1);
	}

	for(index = 0; index < pEndPointEvent->serviceEndpointInfoCount; index++)
	{
		pServiceEndpointInfo = &pEndPointEvent->serviceEndpointInfoList[index];
		if(pServiceEndpointInfo->locationCount >= 2) // Get an endpoint that provides auto failover for the specified location
		{
			if (strncmp(watchlistConsumerConfig.location.data, pServiceEndpointInfo->locationList[0].data, watchlistConsumerConfig.location.length) == 0 )
			{
				endPoint = pServiceEndpointInfo->endPoint.data;
				port = pServiceEndpointInfo->port.data;
				break;
			}
		}
		else if (pServiceEndpointInfo->locationCount > 0) // Try to get backups and keep looking for main case
		{
			if (endPoint == NULL && port == NULL) // keep only the first item met
			{
				if (strncmp(watchlistConsumerConfig.location.data, pServiceEndpointInfo->locationList[0].data, watchlistConsumerConfig.location.length) == 0)
				{
					endPoint = pServiceEndpointInfo->endPoint.data;
					port = pServiceEndpointInfo->port.data;
				}
			}
		}
	}

	if (endPoint != NULL && port != NULL)
	{
		snprintf(watchlistConsumerConfig.hostName, 255, "%s", pServiceEndpointInfo->endPoint.data);
		snprintf(watchlistConsumerConfig.port, 255, "%s", pServiceEndpointInfo->port.data);
	}

	return RSSL_RC_CRET_SUCCESS;
}

void clearPostServiceInfo(PostServiceInfo *serviceInfo)
{
	serviceInfo->isServiceFound = RSSL_FALSE;
	serviceInfo->isServiceUp = RSSL_FALSE;
	serviceInfo->serviceId = 0;
}

void processPostServiceUpdate(PostServiceInfo *serviceInfo, RsslBuffer *pMatchServiceName, RsslRDMService* pService)
{
	/* Save service information for tunnel stream. */
	if (!serviceInfo->isServiceFound)
	{
		/* Check if the name matches the service we're looking for. */
		if (pService->flags & RDM_SVCF_HAS_INFO
			&& rsslBufferIsEqual(&pService->info.serviceName, pMatchServiceName))
		{
			serviceInfo->isServiceFound = RSSL_TRUE;
			serviceInfo->serviceId = (RsslUInt16)pService->serviceId;
		}
	}

	if (pService->serviceId == serviceInfo->serviceId)
	{
		/* Process the state of the tunnel stream service. */
		if (pService->action != RSSL_MPEA_DELETE_ENTRY)
		{
			/* Check state. */
			if (pService->flags & RDM_SVCF_HAS_STATE)
			{
				serviceInfo->isServiceUp =
					pService->state.serviceState == 1 &&
					(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) ||
						pService->state.acceptingRequests == 1);
			}
		}
		else
		{
			clearPostServiceInfo(serviceInfo);
		}
	}
}

RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorJsonConversionEvent *pEvent)
{
	if (pEvent->pError)
	{
		printf("Error Id: %d, Text: %s\n", pEvent->pError->rsslError.rsslErrorId, pEvent->pError->rsslError.text);
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslRet serviceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	if (serviceInfo.isServiceFound && rsslBufferIsEqual(&watchlistConsumerConfig.serviceName, pServiceName))
	{
		*pServiceId = (RsslUInt16)serviceInfo.serviceId;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent)
{
	if (pLogEvent && pLogEvent->pRestLoggingMessage && pLogEvent->pRestLoggingMessage->data)
	{
		FILE* pOutputStream = watchlistConsumerConfig.restOutputStreamName;
		if (!pOutputStream)
			pOutputStream = stdout;

		fprintf(pOutputStream, "{restLoggingCallback}: %s", pLogEvent->pRestLoggingMessage->data);
		fflush(pOutputStream);
	}
	return RSSL_RC_CRET_SUCCESS;
}
