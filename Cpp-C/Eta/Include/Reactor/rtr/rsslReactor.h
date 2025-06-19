/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2017,2019-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_REACTOR_H
#define _RTR_RSSL_REACTOR_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslReactorEvents.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactorCallbackReturnCodes.h"
#include "rtr/rsslReactorChannel.h"
#include "rtr/rsslTunnelStream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSSL_REACTOR_DEBUGGING_BUFFER_INIT_SIZE 1048576

/**
 *	@addtogroup VAReactorStruct
 *	@{
 */

 /**
  * @brief The Reactor.  Applications create RsslReactor objects by calling rsslCreateReactor, create connections by calling rsslReactorConnect/rsslReactorAccept and process events by calling rsslReactorDispatch.
  * @see RsslReactorChannel, rsslCreateReactor, rsslDestroyReactor, rsslReactorDispatch, rsslReactorSubmit, rsslReactorConnect, rsslReactorAccept, rsslReactorCloseChannel
  */
typedef struct
{
	RsslSocket	eventFd;		/*!< A descriptor that provides notification for events available to be processed by calling rsslReactorDispatch(). */
	void		*userSpecPtr;	/*!< A user-specified pointer associated with this RsslReactor. */
} RsslReactor;
 
/**
 * @brief Signature of a Login Message Callback function.
 * @see RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole
 */
/* Receives messages related to the Login Domain, along with a structure representing information decoded from the message. */
typedef RsslReactorCallbackRet RsslRDMLoginMsgCallback(RsslReactor*, RsslReactorChannel*, RsslRDMLoginMsgEvent*);

/**
 * @brief Signature of a Directory Message Callback function.
 * @see RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole
 */
typedef RsslReactorCallbackRet RsslRDMDirectoryMsgCallback(RsslReactor*, RsslReactorChannel*, RsslRDMDirectoryMsgEvent*);

/**
 * @brief Signature of a Dictionary Message Callback function.
 * @see RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole
 */
typedef RsslReactorCallbackRet RsslRDMDictionaryMsgCallback(RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent*);

/**
 * @brief Signature of a Default RSSL Message Callback function.
 * @see RsslReactorChannelRoleBase
 */
typedef RsslReactorCallbackRet RsslDefaultMsgCallback(RsslReactor*, RsslReactorChannel*, RsslMsgEvent*);

/**
 * @brief Signature of a Channel Event Callback function.
 * @see RsslReactorChannelRoleBase, RsslReactorChannelEvent
 */
typedef RsslReactorCallbackRet RsslReactorChannelEventCallback(RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent*);

/**
 * @brief Signature of a Authentication Token Event Callback function.
 * @see RsslReactorAuthTokenEvent
 */
typedef RsslReactorCallbackRet RsslReactorAuthTokenEventCallback(RsslReactor*, RsslReactorChannel*, RsslReactorAuthTokenEvent*);

/**
 * @brief Signature of a Login Message Renewal Callback function.
 * @see RsslReactorAuthTokenEvent
 */
typedef RsslReactorCallbackRet RsslReactorLoginMsgRenewalEventCallback(RsslReactor*, RsslReactorChannel*, RsslReactorLoginRenewalEvent*);

/**
 * @brief Signature of a Service Endpoint Event Callback function.
 * @see RsslReactorServiceEndpointEvent
 */
typedef RsslReactorCallbackRet RsslReactorServiceEndpointEventCallback(RsslReactor*, RsslReactorServiceEndpointEvent*);

/**
 * @brief Signature of an OAuth Credential Event Callback function.
 * @see RsslReactorOAuthCredentialEvent
 */
typedef RsslReactorCallbackRet RsslReactorOAuthCredentialEventCallback(RsslReactor*, RsslReactorOAuthCredentialEvent*);

/**
 * @brief Signature of an Json Conversion Event Callback function.
 * @see RsslReactorJsonConversionEvent
 */
typedef RsslReactorCallbackRet RsslReactorJsonConversionEventCallback(RsslReactor*, RsslReactorChannel*, RsslReactorJsonConversionEvent*);

/**
 * @brief Signature of a callback function to convert from a service name to a service Id.
 * @param pReactor The Reactor associated with this event. 
 * @param pServiceName The name of the service that the callback will look up to find the appropriate ID.
 * @param pServiceId An RsslUInt16 that the callback should populate with the translated ID.
 * @param pEvent An RsslReactorServiceNameToIdEvent that provides additional information for the callback.
 * @return RSSL_RET_SUCCESS if a matching ID was found, RSSL_RET_FAILURE otherwise.
 * @see RsslReactorServiceNameToIdEvent
 */
typedef RsslRet RsslReactorServiceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent);

/**
 * @brief Signature of a callback function to receive REST logging message.
 * @param pReactor The Reactor associated with this event.
 * @param pLogEvent An RsslReactorRestLoggingEvent that provides the REST logging message and additional information for the callback.
 * @return RSSL_RET_SUCCESS The callback should return success as result.
 * @see RsslReactorRestLoggingEvent
 * @see RsslCreateReactorOptions.restEnableLog, RsslCreateReactorOptions.pRestLoggingCallback
 */
typedef RsslReactorCallbackRet RsslReactorRestLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent);


/**
 * @brief Enumerated types indicating the role of a connection.
 * @see RsslReactorChannelRoleBase, RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole
 */
typedef enum 
{
	RSSL_RC_RT_INIT				= 0,	/*!< (0) Unknown role */
	RSSL_RC_RT_OMM_CONSUMER		= 1,	/*!< (1) Indicates the RsslReactorChannel represents the connection of an OMM Consumer. */
	RSSL_RC_RT_OMM_PROVIDER		= 2,	/*!< (2) Indicates the RsslReactorChannel represents the connection of an OMM Provider. */
	RSSL_RC_RT_OMM_NI_PROVIDER	= 3		/*!< (3) Indicates the RsslReactorChannel represents the connection of an OMM Provider. */
} RsslReactorChannelRoleType;

/**
 * @brief Base of ReactorChannel role structures.
 * @see RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole
 */
typedef struct
{
	RsslReactorChannelRoleType		roleType;				/*!< Type indicating the role. Populated by RsslReactorChannelRoleType. */
	RsslReactorChannelEventCallback	*channelEventCallback;	/*!< Callback function that handles RsslReactorChannelEvents.  Must be provided for all roles. */
	RsslDefaultMsgCallback			*defaultMsgCallback;	/*!< Callback function that handles RsslMsg events that aren't handled by a specific domain callback. Must be provided for all roles. */
} RsslReactorChannelRoleBase;

/**
 * @brief Available methods for automatically retrieving dictionary messages from a provider.
 * @see RsslReactorOMMConsumerRole
 */
typedef enum
{
	RSSL_RC_DICTIONARY_DOWNLOAD_NONE 			= 0,	/*!< (0) Do not automatically request dictionary messages. */
	RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE = 1		/*!< (1) Reactor searches RsslRDMDirectoryMsgs for the RWFFld and RWFEnum dictionaries.
														 * Once found, it will request the dictionaries and close their streams once all
														 * necessary data is retrieved. This option is for use with an ADS. */
} RsslDownloadDictionaryMode;

typedef struct 
{
	RsslBool 						enableWatchlist;		/*!< Enables the watchlist. */
	RsslReactorChannelEventCallback	*channelOpenCallback;	/*!< Callback function that is provided when a channel is first opened by rsslReactorConnect. This is only allowed when a watchlist is enabled and is optional. */
	RsslUInt32						itemCountHint;			/*!< Set to the number of items the application expects to request. */
	RsslBool						obeyOpenWindow;			/*!< Controls whether item requests obey the OpenWindow provided by a service. */
	RsslUInt32						maxOutstandingPosts;	/*!< Sets the maximum number of post acknowledgments that may be outstanding for the channel. */
	RsslUInt32						postAckTimeout;			/*!< Time a stream will wait for acknowledgment of a post message, in milliseconds. */
	RsslUInt32						requestTimeout;			/*!< Time a requested stream will wait for a response from the provider, in milliseconds. */
} RsslConsumerWatchlistOptions;

/**
 * @brief Structure representing the OAuth credential for authorization with the RDP token service.
 * @see RsslReactorOMMConsumerRole
 */
typedef struct
{
	RsslBuffer									userName;						/*!< The user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins */
	RsslBuffer									password;						/*!< The password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins */
	RsslBuffer									clientId;						/*!< The clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 
																				 *   oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins. */
	RsslBuffer									clientSecret;					/*!< The clientSecret, also known as the Service Account password, used to authenticate with RDP token service. 
																				 *   Mandatory for V2 Client Credentials Logins and used in conjunction with clientID. */
	RsslBuffer									tokenScope;						/*!< The scope of the generated token. Optional*/
	RsslReactorOAuthCredentialEventCallback		*pOAuthCredentialEventCallback; /*!< Callback function that receives RsslReactorOAuthCredentialEvent to specify sensitive information. Optional
																				 *   The Reactor will not deep copy the password, client secret, or client JWK if the function pointer is specified.*/
	RsslBool									takeExclusiveSignOnControl;		/*!< The exclusive sign on control value. If set to RSSL_TRUE, other applications using the same credentials will be force signed-out. Optional and only 
																				 *   used for V1 oAuth Password Credentials logins */
	void*										userSpecPtr;					/*!< user specified pointer that will be referenced in any instances of pOAuthCredentialEventCallback */
	RsslBuffer									clientJWK;						/*!< Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with RDP token service. 
																				 *   Mandatory for V2 Client Credentials with JWT Logins. */
	RsslBuffer									audience;						/*!< Specifies the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT Logins. 
																				 *   The default value is: https://login.ciam.refinitiv.com/as/token.oauth2 */
} RsslReactorOAuthCredential;


/**
 * @brief Clears an RsslReactorOAuthCredential object.
 * @see RsslReactorOAuthCredential
 */
RTR_C_INLINE void rsslClearReactorOAuthCredential(RsslReactorOAuthCredential *pOAuthCredential)
{
	memset(pOAuthCredential, 0, sizeof(RsslReactorOAuthCredential));
	pOAuthCredential->tokenScope.data = (char *)"trapi.streaming.pricing.read";
	pOAuthCredential->tokenScope.length = 28;
	pOAuthCredential->takeExclusiveSignOnControl = RSSL_TRUE;
}

/**
 * @brief Structure representing the a Login message to be used with the multi-credential functionality.
 * @see RsslReactorOMMConsumerRole
 */
typedef struct
{
	RsslRDMLoginRequest* loginRequestMsg;									/*!< The initial login message.  Please note that all login messages configured in RsslReactorOMMConsumerRole.pLoginMsgList must have the same Single Open and Allow Suspect Data Validation flags and values set. */
	RsslReactorLoginMsgRenewalEventCallback* pLoginRenewalEventCallback;	/*!< Optional callback for a login message.  This is intended to be used in cases where
																				 the login userName and/or authentication extended fields in the login may change between a full channel recovery, 
																				 and will be called if Session Management is turned off for this connection, and the channel is in full reconnection.*/
	void* userSpecPtr;														/*!< Optional user specified pointer that will be referenced in any instances of pLoginRenewalEventCallback */
}
RsslReactorLoginRequestMsgCredential;

/**
 * @brief Clears an RsslReactorOAuthCredential object.
 * @see RsslReactorOAuthCredential
 */
RTR_C_INLINE void rsslClearReactorLoginRequestMsgCredential(RsslReactorLoginRequestMsgCredential* pLoginCredential)
{
	memset(pLoginCredential, 0, sizeof(RsslReactorLoginRequestMsgCredential));
}


/**
 * @brief Structure representing the role of an OMM Consumer.
 * @see RsslReactorChannelRole, RsslReactorChannelRoleBase
 */
typedef struct
{
	RsslReactorChannelRoleBase		base;					/*!< The Base Reactor Channel Role structure. */
	RsslRDMLoginRequest				*pLoginRequest;			/*!< A Login Request to be sent during the setup of a Consumer-Provider session. Optional. */
	RsslBuffer						clientId;				/*!< @deprecated This is used only for backward compatibility. All OAuth credentials should be specified in RsslReactorOAuthCredential. 
															 * Specifies an unique ID defined for an application making a request to the RDP token service. */
	RsslReactorOAuthCredential*		pOAuthCredential;		/*!< OAuth credential for authentication with the RDP token service. This member has higher precedence for
																authorization than the user credential specified in pLoginRequest. Optional. */
	RsslRDMLoginMsgCallback			*loginMsgCallback;		/*!< A callback function for processing RsslRDMLoginMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslRDMDirectoryRequest			*pDirectoryRequest;		/*!< A Directory Request to be sent during the setup of a Consumer-Provider session. Optional. Requires pLoginRequest to be set.*/
	RsslRDMDirectoryMsgCallback		*directoryMsgCallback;	/*!< A callback function for processing RsslRDMDirectoryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslDownloadDictionaryMode		dictionaryDownloadMode;	/*!< Indicates a method of requesting dictionaries from the Provider. If not set to RSSL_RC_DICTIONARY_DOWNLOAD_NONE, requires pLoginRequest and pDirectoryRequest to be set. */
	RsslRDMDictionaryMsgCallback	*dictionaryMsgCallback;	/*!< A callback function for processing RsslRDMDictionaryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslConsumerWatchlistOptions 	watchlistOptions;		/*!< Options for using the watchlist. */
	RsslReactorLoginRequestMsgCredential **pLoginRequestList;			/*!< An array of RsslRDMLoginRequest pointers to be sent during the setup of a Consumer-Provider session, if multiple connections with different credentials are
														required. This only applies if a warm standby group or multiple element connectionList is specifed, and will override what is set in pLoginRequest. Optional. */
	RsslInt8						loginRequestMsgCredentialCount; /*!< Count of the number of login credentials in pLoginRequestList */
	RsslReactorOAuthCredential** pOAuthCredentialList;		/*!< OAuth credential for authentication with the RDP token service if sesion management is specified on the warm standby or connectionList RsslReactorConnectInfo. 
															This member has higher precedence for authorization than the user credential specified in pOAuthCredential Optional. */
	RsslInt8						oAuthCredentialCount;	/*!< Count of the number of oAuth credentials in pOAuthCredentialList */
} RsslReactorOMMConsumerRole;

/**
 * @brief Clears an RsslReactorOMMConsumerRole object.
 * @see RsslReactorOMMConsumerRole
 */
RTR_C_INLINE void rsslClearOMMConsumerRole(RsslReactorOMMConsumerRole *pRole)
{
	memset(pRole, 0, sizeof(RsslReactorOMMConsumerRole));
	pRole->base.roleType = RSSL_RC_RT_OMM_CONSUMER;

	pRole->watchlistOptions.enableWatchlist = RSSL_FALSE;
	pRole->watchlistOptions.channelOpenCallback = NULL;
	pRole->watchlistOptions.itemCountHint = 0;
	pRole->watchlistOptions.obeyOpenWindow = RSSL_TRUE;
	pRole->watchlistOptions.maxOutstandingPosts = 100000;
	pRole->watchlistOptions.postAckTimeout = 15000;
	pRole->watchlistOptions.requestTimeout = 15000;
}

/**
 * @brief Structure representing the role of an OMM Provider.
 * @see RsslReactorChannelRole, RsslReactorChannelRoleBase
 */
typedef struct
{
	RsslReactorChannelRoleBase			base;						/*!< The Base Reactor Channel Role structure. */
	RsslRDMLoginMsgCallback				*loginMsgCallback;			/*!< A callback function for processing RsslRDMLoginMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslRDMDirectoryMsgCallback			*directoryMsgCallback;		/*!< A callback function for processing RsslRDMDirectoryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslRDMDictionaryMsgCallback		*dictionaryMsgCallback;		/*!< A callback function for processing RsslRDMDictionaryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslTunnelStreamListenerCallback	*tunnelStreamListenerCallback;	/*!< A callback function for accepting new tunnel streams. */
} RsslReactorOMMProviderRole;

/**
 * @brief Clears an RsslReactorOMMProviderRole object.
 * @see RsslReactorOMMProviderRole
 */
RTR_C_INLINE void rsslClearOMMProviderRole(RsslReactorOMMProviderRole *pRole)
{
	memset(pRole, 0, sizeof(RsslReactorOMMProviderRole));
	pRole->base.roleType = RSSL_RC_RT_OMM_PROVIDER;
}

/**
 * @brief Structure representing the role of an OMM Noninteractive Provider.
 * @see RsslReactorChannelRole, RsslReactorChannelRoleBase
 */
typedef struct
{
	RsslReactorChannelRoleBase	base;				/*!< The Base Reactor Channel Role structure. */
	RsslRDMLoginRequest			*pLoginRequest;		/*!< A Login Request to be sent during the setup of a Noniteractive Provider session. Optional. */
	RsslRDMLoginMsgCallback		*loginMsgCallback;	/*!< A callback function for processing RsslRDMLoginMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslRDMDirectoryRefresh		*pDirectoryRefresh;	/*!< A Directory Refresh to be sent during the setup of a Noniteractive Provider session. Optional. Requires pLoginRequest to be set.*/
} RsslReactorOMMNIProviderRole;

/**
 * @brief Clears an RsslReactorOMMNIProviderRole object.
 * @see RsslReactorOMMNIProviderRole
 */
RTR_C_INLINE void rsslClearOMMNIProviderRole(RsslReactorOMMNIProviderRole *pRole)
{
	memset(pRole, 0, sizeof(RsslReactorOMMNIProviderRole));
	pRole->base.roleType = RSSL_RC_RT_OMM_NI_PROVIDER;
}


/**
 * @brief The Reactor Channel Role structure.  The rsslReactorConnect() and rsslReactorAccept() functions expect this structure.
 * It is a group of the types of roles that are supported by RsslReactorChannels. The RsslReactorChannelRoleBase is
 * used to identify the type of role.
 * @see RsslReactorChannelRoleBase, RsslReactorOMMConsumerRole, RsslReactorOMMProviderRole, RsslReactorOMMNIProviderRole, rsslReactorConnect, rsslReactorAccept
 */
typedef union
{
	RsslReactorChannelRoleBase		base;				/*!< The Base Reactor Channel Role structure. */
	RsslReactorOMMConsumerRole		ommConsumerRole;	/*!< OMM Consumer */
	RsslReactorOMMProviderRole		ommProviderRole;	/*!< OMM Provider */
	RsslReactorOMMNIProviderRole	ommNIProviderRole;	/*!< OMM Noninteractive Provider */
} RsslReactorChannelRole;

/**
 * @brief Clears an RsslReactorChannelRole object.
 * @see RsslReactorChannelRole
 */
RTR_C_INLINE void rsslClearReactorChannelRole(RsslReactorChannelRole *pRole)
{
	memset(pRole, 0, sizeof(RsslReactorChannelRole));
}

/**
 * @brief Enumerated types indicates reactor debug level.
 */

typedef enum
{
	RSSL_RC_DEBUG_LEVEL_NONE = 0x0000, /* No messages will be debugged */
	RSSL_RC_DEBUG_LEVEL_CONNECTION = 0x0001, /* If applied, messages related to Connection will be debugged */
	RSSL_RC_DEBUG_LEVEL_EVENTQUEUE = 0x0002, /* if applied, the ReactorDebugger will log the number of events associated with different ReactorChannel */
	RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM = 0x0004 /* If applied, ReactorDebugger will log messages asscoated with different TunnelStream events*/
}RsslReactorDebuggerLevels;

/**
 * @brief Get an debugLevel of the reactor.
 * @param debugLevel out RsslUInt32 pointer to get debug level
 * @param pError Error structure to be populated in the event of an error.
 * @return Status of receiving debug level from reactor instance.
 * @see RsslReactorImpl, RsslReactorDebuggerLevels
 */

RSSL_VA_API RsslRet rsslReactorGetDebugLevel(RsslReactor *pReactor, RsslUInt32 *debugLevel, RsslErrorInfo *pError);

/**
 * @brief Set an debugLevel of the reactor.
 * @param debugLevel configure reactor debug level. Reactor debug level listed in RsslReactorDebuggerLevels
 * @param pError Error structure to be populated in the event of an error.
 * @return Status of setting debug level to reactor instance.
 * @see RsslReactorImpl, RsslReactorDebuggerLevels
 */

RSSL_VA_API RsslRet rsslReactorSetDebugLevel(RsslReactor *pReactor, RsslUInt32 debugLevel, RsslErrorInfo *pError);

/**
 * @brief Configuration options for creating an RsslReactor.
 * @see rsslCreateReactor
 */
typedef struct {
	RsslInt32	dispatchDecodeMemoryBufferSize;	/*!< Size of the memory buffer(in bytes) that the RsslReactor will use when decoding RsslRDMMsgs to pass to callback functions. */
    RsslInt32   maxEventsInPool;				/*!< Specifies maximum amount of the events in the RsslReactor pool. The default value -1 the maximum is not specified.>*/
    void		*userSpecPtr; 					/*!< user-specified pointer which will be set on the Reactor. */
	RsslBuffer	serviceDiscoveryURL;			/*!< Specifies a URL for the RDP service discovery. The service discovery is used when the connection arguments is not specified
												 * in the RsslReactorConnectInfo.rsslConnectOptions */
	RsslBuffer	tokenServiceURL;				/*!< @deprecated: Specifies a URL of the token service to get an access token and a refresh token. This is used for querying RDP service
												 * discovery and subscribing data from RDP. */
	RsslDouble	tokenReissueRatio;				/*!< Specifies a ratio to multiply with access token validity time(seconds) for retrieving and reissuing the access token. 
												 * The valid range is between 0.05 to 0.95. */
	RsslInt32	reissueTokenAttemptLimit;		/*!< The maximum number of times the RsllReactor will attempt to reissue the token. If set to -1, there is no limit. */
	RsslInt32	reissueTokenAttemptInterval;	/*!< The interval time for the RsslReactor will wait before attempting to reissue the token, in milliseconds. The minimum interval is 1000 milliseconds */
	RsslUInt32	restRequestTimeOut;				/*!< Specifies maximum time the request is allowed to take for token service and service discovery, in seconds. If set to 0, there is no timeout */
	int			port;							/*!< @deprecated DEPRECATED: This parameter no longer has any effect. It was a port used for creating the eventFd descriptor on the RsslReactor. It was never used on Linux or Solaris platforms. */
	RsslBuffer	cpuBindWorkerThread;			/*!< Specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format) for the internal Reactor worker thread binding. If the value is not set, then there is no limit of the binding processor cores for the Reactor worker thread.> */
	RsslBool	restEnableLog;					/*!< Enable REST interaction debug messages> */
	FILE		*restLogOutputStream;			/*!< Set output stream for REST debug message (by default is stdout)> */
	RsslBool	restEnableLogViaCallback;			/*!< Enable receiving REST logging messages via callback (pRestLoggingCallback).> */
	RsslReactorRestLoggingCallback* pRestLoggingCallback;	/*!< Specifies user callback to receive Rest logging messages.> */
	RsslBuffer	tokenServiceURL_V1;				/*!< Specifies a URL of the token service to get an access token and a refresh token for the LSEG Login V1. This is used for querying RDP service
												 * discovery and subscribing data from RDP. */
	RsslBuffer	tokenServiceURL_V2;				/*!< Specifies a URL of the token service to get an access token from the LSEG Login V2. This is used for querying RDP service
												 * discovery and subscribing data from RDP. */
	RsslProxyOpts	restProxyOptions;			/*!< Specifies proxy settings for Rest requests: service discovery and auth token service. This proxy is used when both proxyHostName and proxyPort are specified to override the proxy settings in the RsslReactorConnectOptions (RsslConnectOptions.proxyOpts) and RsslReactorServiceDiscoveryOptions (proxyHostName, proxyPort, proxyUserName, proxyPasswd, proxyDomain).> */
	RsslUInt32   debugLevel;						/*!< Configure level of debugging info> */
	RsslUInt32	 debugBufferSize;				/*!< Configure size of debug buffer> */
	RsslBool	restVerboseMode;				/*!< Enable Verbose REST debug messages> */
} RsslCreateReactorOptions;

/**
 * @brief Clears an RsslCreateReactorOptions object.
 * @see RsslCreateReactorOptions
 */
RTR_C_INLINE void rsslClearCreateReactorOptions(RsslCreateReactorOptions *pReactorOpts)
{
	memset(pReactorOpts, 0, sizeof(RsslCreateReactorOptions));
	pReactorOpts->dispatchDecodeMemoryBufferSize = 65536;
	pReactorOpts->maxEventsInPool = -1;
	pReactorOpts->port = 55000;
	pReactorOpts->serviceDiscoveryURL.data = (char *)"https://api.refinitiv.com/streaming/pricing/v1/";
	pReactorOpts->serviceDiscoveryURL.length = 47;
	pReactorOpts->tokenReissueRatio = 0.8;
	pReactorOpts->reissueTokenAttemptLimit = -1;
	pReactorOpts->reissueTokenAttemptInterval = 5000;
	pReactorOpts->restRequestTimeOut = 90;
	pReactorOpts->restEnableLog = RSSL_FALSE;
	pReactorOpts->restVerboseMode = RSSL_FALSE;
	pReactorOpts->restLogOutputStream = NULL;
	pReactorOpts->restEnableLogViaCallback = RSSL_FALSE;
	pReactorOpts->pRestLoggingCallback = NULL;
	pReactorOpts->debugLevel = RSSL_RC_DEBUG_LEVEL_NONE;
	pReactorOpts->debugBufferSize = RSSL_REACTOR_DEBUGGING_BUFFER_INIT_SIZE;
}

/**
 * @brief Creates an RsslReactor, which can then have channels added, removed, or dispatched from.
 * @param pReactorOpts Configuration options for creating the RsslReactor.
 * @param pError Error structure to be populated in the event of an error.
 * @return Pointer to the newly created RsslReactor. If the pointer is NULL, an error occurred.
 * @see RsslCreateReactorOptions, RsslErrorInfo
 */
RSSL_VA_API RsslReactor *rsslCreateReactor(RsslCreateReactorOptions *pReactorOpts, RsslErrorInfo *pError);

/**
 * @brief Cleans up an RsslReactor.  Stops the ETA Reactor if necessary and sends RsslReactorChannelEvents to all active channels indicating that they are down.
 * Once this call is made, the RsslReactor is destroyed and no further calls should be made with it.  This function is not thread-safe.
 * @param pReactorOpts Configuration options for creating the RsslReactor.
 * @param pError Error structure to be populated in the event of an error.
 * @return Pointer to the newly created RsslReactor. If the pointer is NULL, an error occurred.
 * @see RsslCreateReactorOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslDestroyReactor(RsslReactor *pReactor, RsslErrorInfo *pError);

/**
 * @brief Enumerated types indicating the transport query parameter.
 * @see RsslReactorServiceDiscoveryOptions
 */
typedef enum
{
	RSSL_RD_TP_INIT = 0,	/*!< (0) Unknown transport protocol */
	RSSL_RD_TP_TCP = 1,	/*!< (1) TCP transport protocol */
	RSSL_RD_TP_WEBSOCKET = 2,	/*!< (2) Websocket transport protocol */
} RsslReactorDiscoveryTransportProtocol;

/**
 * @brief Enumerated types indicating the dataformat query parameter.
 * @see RsslReactorServiceDiscoveryOptions
 */
typedef enum
{
	RSSL_RD_DP_INIT = 0,	/*!< (0) Unknown data format */
	RSSL_RD_DP_RWF = 1,	/*!< (1) Rwf data format protocol */
	RSSL_RD_DP_JSON2 = 2,	/*!< (2) tr_json2 data format protocol */
} RsslReactorDiscoveryDataFormatProtocol;

/**
 * @brief Configuration options for querying RDP service discovery to get service endpoints.
 * The proxy configuration is used only when your organization requires use of a proxy to get to the Internet.
 * @see rsslReactorQueryServiceDiscovery
 */
typedef struct
{
	RsslBuffer								userName;						/*!< The user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins.  */
	RsslBuffer								password;						/*!< The password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins   */
	RsslBuffer								clientId				;		/*!< The clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 
																			 *   oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins. */
	RsslBuffer								clientSecret;					/*!< The clientSecret, also known as the Service Account password, used to authenticate with RDP token service. 
																			 *   Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.*/
	RsslBuffer								tokenScope;						/*!< The scope of the generated token. Optional*/
	RsslBool								takeExclusiveSignOnControl;		/*!< The take exclusive sign on control value. If set to RSSL_TRUE, other applications using the same credentials will be force signed-out. Optional and only 
																			 *   used for V1 oAuth Password Credentials logins */
	RsslReactorDiscoveryTransportProtocol   transport;						/*!< This is an optional parameter to specify the desired transport protocol to get
																			 *   service endpoints from the service discovery. */
	RsslReactorDiscoveryDataFormatProtocol  dataFormat;						/*!< This is an optional parameter to specify the desired data format protocol to get
																			 *   service endpoints from the service discovery. */
	RsslReactorServiceEndpointEventCallback *pServiceEndpointEventCallback; /*!< Callback function that receives RsslReactorServiceEndpointEvents. Applications can use service
																			 *   endpoint information from the callback to get an endpoint and establish a connection to the service */

	void                                    *userSpecPtr;					/*!< user-specified pointer which will be set on the RsslReactorServiceEndpointEvent. */

	RsslBuffer                              proxyHostName;					/*!< specifies a proxy server hostname. */
	RsslBuffer                              proxyPort;						/*!< specifies a proxy server port. */
	RsslBuffer                              proxyUserName;					/*!< specifies a username to perform authorization with a proxy server. */
	RsslBuffer                              proxyPasswd;					/*!< specifies a password to perform authorization with a proxy server. */
	RsslBuffer                              proxyDomain;					/*!< specifies a proxy domain of the user to authenticate.
																			 *   Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols. */
	RsslBuffer								clientJWK;						/*!< Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with RDP token service. 
																			 *   Mandatory for V2 Client Credentials with JWT Logins. */
	RsslBuffer								audience;						/*!< Specifies the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT Logins. 
																			 *   The default value is: https://login.ciam.refinitiv.com/as/token.oauth2 */

	RsslBool                                restBlocking;                   /*!< Specifies whether to send REST blocking for authentication and service discovery requests. 
																			 *   The default value is TRUE.*/
} RsslReactorServiceDiscoveryOptions;

/**
 * @brief Clears an RsslReactorServiceDiscoveryOptions object.
 * @see RsslReactorServiceDiscoveryOptions
 */
RTR_C_INLINE void rsslClearReactorServiceDiscoveryOptions(RsslReactorServiceDiscoveryOptions *pOpts)
{
	memset(pOpts, 0, sizeof(RsslReactorServiceDiscoveryOptions));
	pOpts->tokenScope.data = (char *)"trapi.streaming.pricing.read";
	pOpts->tokenScope.length = 28;
	pOpts->takeExclusiveSignOnControl = RSSL_TRUE;
	pOpts->restBlocking = RSSL_TRUE;
}

/**
 * @brief Queries RDP service discovery to get service endpoint information.
 * @param pReactor The reactor that will handle the request.
 * @param pOpts The RsslReactorServiceDiscoveryOptions to configure options and specify the RsslReactorServiceEndpointEventCallback to receive service endpoint information.
 * @param pError Error structure to be populated in the event of failure.
 */
RSSL_VA_API RsslRet rsslReactorQueryServiceDiscovery(RsslReactor *pReactor, RsslReactorServiceDiscoveryOptions* pOpts, RsslErrorInfo *pError);

typedef struct
{
	RsslConnectOptions	rsslConnectOptions;		 /*!< Options for creating the connection. */
	RsslUInt32			initializationTimeout;	 /*!< Time(in seconds) to wait for successful initialization of a channel. 
												  * If initialization does not complete in time, a RsslReactorChannelEvent will be sent indicating that the channel is down. */
	RsslBool            enableSessionManagement; /*!< Indicates RsslReactor to request the authentication token and query an endpoint from RDP service discovery if both host
												  * and port are not specified by users. The watchlist must be enable for the Reactor to send and reissue the token via the login
												  * request to keep the connection alive on behalf of users.*/
	RsslBuffer          location;                /*!< Specifies a location to get a service endpoint to establish a connection with service provider. 
												  * Defaults to "us-east-1" if not specified. The Reactor always uses a endpoint which provides two availability zones 
												  * for the location. */
	RsslUInt32			serviceDiscoveryRetryCount;				/*!< The number of times the RsslReactor attempts to reconnect a channel before that would force the API
																 * to retry Service Discovery. Reactor will not retry to get a endpoint from the service discovery
																 * when the value is zero. */

	RsslReactorAuthTokenEventCallback *pAuthTokenEventCallback; /*!< Callback function that receives RsslReactorAuthTokenEvents. The token is requested 
	                                                             * by the Reactor for Consumer(disabling watchlist) and NiProvider applications to send login request and
																 * reissue with the token */

	RsslUInt8			loginReqIndex;					/*!< Login request array index for this channel, mapping to the login that will be used for this channel 
														 *	if RsslReactorOMMConsumerRole.pLoginRequestList has been set.  This must be less than the number set 
														 *	in RsslReactorOMMConsumerRole.pLoginRequestCount  */
	RsslUInt8			oAuthCredentialIndex;			/*!< oAuth request array index for this channel, mapping to the login that will be used for this channel if 
														 *	session management is turned on and if RsslReactorOMMConsumerRole.oAuthCredentialCount has been set.  
														 *	This must be less than the number set in RsslReactorOMMConsumerRole.oAuthCredentialCount  */

} RsslReactorConnectInfo;

RTR_C_INLINE void rsslClearReactorConnectInfo(RsslReactorConnectInfo *pInfo)
{
	rsslClearConnectOpts(&pInfo->rsslConnectOptions);
	pInfo->initializationTimeout = 60;
	pInfo->enableSessionManagement = RSSL_FALSE;
	pInfo->location.data = (char *)"us-east-1";
	pInfo->location.length = 9;
	pInfo->serviceDiscoveryRetryCount = 3;
	pInfo->pAuthTokenEventCallback = NULL;
	pInfo->loginReqIndex = 0;
	pInfo->oAuthCredentialIndex = 0;
}

/**
 * @brief Enumerated types indicating interests for channel statistics
 * @see RsslReactorConnectOptions
 */
typedef enum
{
	RSSL_RC_ST_NONE = 0x0000,	/*!< None */
	RSSL_RC_ST_READ = 0x0001,	/*!< Indicates an interest for bytes read and uncompressed bytes read statistics  */
	RSSL_RC_ST_WRITE = 0x0002,	/*!< Indicates an interest for bytes written and uncompressed bytes written statistics */
	RSSL_RC_ST_PING = 0x0004,	/*!< Indicates an interest for ping received and ping sent statistics */
} RsslReactorChannelStatisticFlags;

/**
 * @brief Configuration options for selecting a list of services to support the per service based warm standby feature.
 * @see RsslReactorWarmStandbyServerInfo
 */
typedef struct
{
    RsslUInt32                  serviceNameCount;   /*!< Number of service names to provide active services for a warm standby server. */
    RsslBuffer*                 serviceNameList;    /*!< Array of service names to provide active services for a warm standby server. */

}RsslReactorPerServiceBasedOptions;

/**
 * @brief Clears an RsslReactorPerServiceBasedOptions object.
 * @see RsslReactorPerServiceBasedOptions
 */
RTR_C_INLINE void rsslClearReactorPerServiceBasedOptions(RsslReactorPerServiceBasedOptions* pPerServiceBasedOptions)
{
    memset(pPerServiceBasedOptions, 0, sizeof(RsslReactorPerServiceBasedOptions));
}

/**
 * @brief Enumerated types indicating warm standby modes
 * @see RsslReactorWarmStandbyGroup
 */
typedef enum
{
	RSSL_RWSB_MODE_NONE = 0,	/*!< Unknown warm standby mode. */
	RSSL_RWSB_MODE_LOGIN_BASED = 1, /*!< Per login based warm standby mode. */
	RSSL_RWSB_MODE_SERVICE_BASED = 2,  /*!< Per service based warm standby mode. */
}
RsslReactorWarmStandbyMode;

/**
 * @brief Configuration options for creating a Warm Standby server information.
 * @see RsslReactorWarmStandbyGroup
 */
typedef struct
{
	RsslReactorConnectInfo			reactorConnectInfo;

	RsslReactorPerServiceBasedOptions  perServiceBasedOptions; /*!< Per service based option for selecting a list of active services for this server.
															   The first connected server provides active services if this option is not configued in
															   and servers. */
} RsslReactorWarmStandbyServerInfo;

RTR_C_INLINE void rsslClearReactorWarmStandbyServerInfo(RsslReactorWarmStandbyServerInfo *pWarmStandByServerInfo)
{
	rsslClearReactorConnectInfo(&pWarmStandByServerInfo->reactorConnectInfo);
	rsslClearReactorPerServiceBasedOptions(&pWarmStandByServerInfo->perServiceBasedOptions);
}

/**
 * @brief Configuration options for creating a Warm Standby group.
 * @see rsslReactorConnect
 */
typedef struct
{
	RsslReactorWarmStandbyServerInfo      startingActiveServer; /*!< The active server to which client should to connect.
                                                           Reactor chooses a server from the standByServerList instead if this parameter is not configured.*/
	RsslReactorWarmStandbyServerInfo*     standbyServerList;    /*!< A list of standby servers. */
    RsslUInt32                            standbyServerCount;   /*!< The number of standby servers. */
	RsslReactorWarmStandbyMode        warmStandbyMode; /*!< Specifies a warm standby mode. */

}RsslReactorWarmStandbyGroup;

RTR_C_INLINE void rsslClearReactorWarmStandbyGroup(RsslReactorWarmStandbyGroup *pWarmStandByGroup)
{
	rsslClearReactorWarmStandbyServerInfo(&pWarmStandByGroup->startingActiveServer);
    pWarmStandByGroup->standbyServerList = NULL;
    pWarmStandByGroup->standbyServerCount = 0;
    pWarmStandByGroup->warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;
}

/**
 * @brief Configuration options for specifying a preferred host or warmstandby group.
 * @see RsslReactorConnectOptions
 */
typedef struct
{
	RsslBool   enablePreferredHostOptions;	/* !<This is used to enable the preferred host feature. */

	RsslBuffer detectionTimeSchedule;		/* !<Specifies cron time schedule to switch over to a preferred host or warmstandby group. Defaults is empty
											 * detectionTimeInterval is used instead if this member is empty. Optional. */

	RsslUInt32 detectionTimeInterval;		/* !<Specifies time interval (in second) unit to switch over to a preferred host or warmstandby group. Optional. */

	RsslUInt32 connectionListIndex;			/* !<Specifies an index in RsslReactorConnectOptions.reactorConnectionList to set as preferred host. */

	RsslUInt32 warmStandbyGroupListIndex;	/* !<Specifies an index in RsslReactorConnectOptions.reactorWarmStandbyGroupList to set as preferred warmstandby group. */

	RsslBool   fallBackWithInWSBGroup;		/* !<This is used to check whether to fallback within a WSB group instead of moving into a preferred WSB group. */

} RsslPreferredHostOptions;

/**
 * @brief Clears an RsslPreferredHostOptions object.
 * @see RsslPreferredHostOptions
 */
RTR_C_INLINE void rsslClearRsslPreferredHostOptions(RsslPreferredHostOptions* pPreferredHostOptions)
{
	pPreferredHostOptions->enablePreferredHostOptions = RSSL_FALSE;
	rsslClearBuffer(&pPreferredHostOptions->detectionTimeSchedule);
	pPreferredHostOptions->detectionTimeInterval = 0;
	pPreferredHostOptions->connectionListIndex = 0;
	pPreferredHostOptions->warmStandbyGroupListIndex = 0;
	pPreferredHostOptions->fallBackWithInWSBGroup = RSSL_FALSE;
}

/**
 * @brief Configuration options for creating an RsslReactor client-side connection.
 * @see rsslReactorConnect
 */
typedef struct
{
	RsslConnectOptions		rsslConnectOptions;		/*!< Options for creating the connection. */
	RsslUInt32				initializationTimeout;	/*!< Time(in seconds) to wait for successful initialization of a channel. 
													 * If initialization does not complete in time, a RsslReactorChannelEvent will be sent indicating that the channel is down. */

	RsslInt32				reconnectAttemptLimit;	/*!< The maximum number of times the RsllReactor will attempt to reconnect a channel. If set to -1, there is no limit. */
	RsslInt32				reconnectMinDelay;		/*!< The minimum time the RsslReactor will wait before attempting to reconnect, in milliseconds. */
	RsslInt32				reconnectMaxDelay;		/*!< The maximum time the RsslReactor will wait before attempting to reconnect, in milliseconds. */

	RsslReactorConnectInfo	*reactorConnectionList;	/*!< A list of connections.  Each connection in the list will be tried with each reconnection attempt. */
 
	RsslUInt32				connectionCount;		/*!< The number of connections in reactorConnectionList. */

	RsslReactorWarmStandbyGroup *reactorWarmStandbyGroupList; /*!< A list of warmstandby group for the warmstandby feature.
                                                                   Reactor always uses this list if specified and then moves next to the reactorConnectionList option. */
    RsslUInt32				warmStandbyGroupCount;  /*!< The number of warmstandby groups in reactorWarmStandByGroupList. */

	RsslUInt32				connectionDebugFlags;	/*!< Set of RsslDebugFlags for calling the user-set debug callbacks.  These callbacks should be set with rsslSetDebugFunctions.  If set to 0, the debug callbacks will not be used. */

	RsslUInt32				statisticFlags;			/* Specifies interests for the channel statistics defined in RsslReactorChannelStatisticFlags */

	RsslPreferredHostOptions  preferredHostOptions;		/* Specifies preferred host options to fallback for the reactorConnectionList or reactorWarmStandbyGroupList. */

} RsslReactorConnectOptions;

/**
 * @brief Clears an RsslReactorConnectOptions object.
 * @see RsslReactorConnectOptions
 */
RTR_C_INLINE void rsslClearReactorConnectOptions(RsslReactorConnectOptions *pOpts)
{
	rsslClearConnectOpts(&pOpts->rsslConnectOptions);
	pOpts->initializationTimeout = 60;
	pOpts->reconnectMinDelay = 0;
	pOpts->reconnectMaxDelay = 0;
	pOpts->reconnectAttemptLimit = 0;
	pOpts->reactorConnectionList = NULL;
	pOpts->connectionCount = 0;
	pOpts->reactorWarmStandbyGroupList = NULL;
	pOpts->warmStandbyGroupCount = 0;
	pOpts->connectionDebugFlags = 0;
	pOpts->statisticFlags = RSSL_RC_ST_NONE;
	rsslClearRsslPreferredHostOptions(&pOpts->preferredHostOptions);
}

/**
 * @brief Adds a client-side channel to the RsslReactor. Once the channel is initialized, the channelEventCallback will receive an event indicating that the channel is up.
 * @param pReactor The reactor that will handle the new connection.
 * @param pOpts The RsslReactorConnectOptions to configure options for this connection.
 * @param pRole Structure representing the role of this connection.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslReactor, RsslReactorConnectOptions, RsslReactorChannelRole, RsslErrorInfo, rsslReactorAccept, rsslReactorCloseChannel
 */
RSSL_VA_API RsslRet rsslReactorConnect(RsslReactor *pReactor, RsslReactorConnectOptions *pOpts, RsslReactorChannelRole *pRole, RsslErrorInfo *pError );

/**
 * @brief Configuration options to accept a connection on server-side for the WebSocket connection.
 * @see RsslReactorAcceptOptions, rsslReactorAccept
 */
typedef struct
{
	RsslBool		sendPingMessage; /*!< This is used to configure the Reactor to periodically send a ping message to clients. */
} RsslReactorWSocketAcceptOptions;

/**
 * @brief Configuration options for creating an RsslReactor server-side connection.
 * @see rsslReactorAccept
 */
typedef struct
{
	RsslAcceptOptions	rsslAcceptOptions;		/*!< Options for accepting the connection. */
	RsslUInt32			initializationTimeout;	/*!< Time(in seconds) to wait for successful initialization of a channel. 
												 * If initialization does not complete in time, a RsslReactorChannelEvent will be sent indicating that the channel is down. */
	RsslUInt32			connectionDebugFlags;	/*!< Set of RsslDebugFlags for calling the user-set debug callbacks.  These callbacks should be set with rsslSetDebugFunctions.  If set to 0, the debug callbacks will not be used. */

	RsslReactorWSocketAcceptOptions   wsocketAcceptOptions; /*!< This is additional accept options for WebSocket connection. */

} RsslReactorAcceptOptions;

/**
 * @brief Clears an RsslReactorAcceptOptions object.
 * @see RsslReactorAcceptOptions
 */
RTR_C_INLINE void rsslClearReactorAcceptOptions(RsslReactorAcceptOptions *pOpts)
{
	rsslClearAcceptOpts(&pOpts->rsslAcceptOptions);
	pOpts->initializationTimeout = 60;
	pOpts->connectionDebugFlags = 0;
	pOpts->wsocketAcceptOptions.sendPingMessage = RSSL_TRUE;
}

/**
 * @brief Adds a server-side channel to the RsslReactor. Once the channel is initialized, the channelEventCallback will receive an event indicating that the channel is up.
 * @param pReactor The reactor that will handle the new connection.
 * @param pServer The RsslServer that is accepting this connection. An RsslServer can be created with rsslBind().
 * @param pOpts The RsslReactorAcceptOptions to configure options for this connection.
 * @param pRole Structure representing the role of this connection.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslReactor, RsslReactorAcceptOptions, RsslReactorChannelRole, RsslErrorInfo, rsslReactorConnect, rsslReactorCloseChannel
 */
RSSL_VA_API RsslRet rsslReactorAccept(RsslReactor *pReactor, RsslServer *pServer, RsslReactorAcceptOptions *pOpts, RsslReactorChannelRole *pRole, RsslErrorInfo *pError );

/**
 * @brief Cleans up a channel and removes it from the RsslReactor. May be called inside or outside of a callback function, however the channel should no longer be used afterwards.
 * @param pReactor The reactor that handles the channel.
 * @param pChannel The channel to be closed.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslReactor, RsslReactorChannel, RsslErrorInfo, rsslReactorConnect, rsslReactorAccept
 */
RSSL_VA_API RsslRet rsslReactorCloseChannel(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslErrorInfo *pError);

/**
 * @brief Options for dispatching from an RsslReactor.
 * @see rsslReactorDispatch
 */
typedef struct
{
	RsslReactorChannel	*pReactorChannel;	/*!< If specified, only events and messages for this channel with be processed. If not specified, messages and events for all channels will be processed. */
	RsslUInt32			maxMessages;		/*!< The maximum number of events or messages to process for this call to rsslReactorDispatch(). */
} RsslReactorDispatchOptions;

/**
 * @brief Clears an RsslReactorDispatchOptions object.
 * @see RsslReactorDispatchOptions
 */
RTR_C_INLINE void rsslClearReactorDispatchOptions(RsslReactorDispatchOptions *pOpts)
{
	pOpts->pReactorChannel = NULL;
	pOpts->maxMessages = 100;
}

/**
 * @brief Process events and messages from the RsslReactor, which may be passed to the calling application via the callback functions given for that event's or message's channel.
 * @param pReactor The reactor to dispatch from.
 * @param pDispatchOpts Options for how to dispatch.
 * @param pError Error structure to be populated in the event of failure.
 * @return Value greater than RSSL_RET_SUCCESS, if dispatching succeeded and there are more messages or events to process.
 * @return RSSL_RET_SUCCESS, if dispatching succeeded and there are no more messages or events to process.
 * @return failure codes, if the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslErrorInfo, RsslReactorDispatchOptions
 */
RSSL_VA_API RsslRet rsslReactorDispatch(RsslReactor *pReactor, RsslReactorDispatchOptions *pDispatchOpts, RsslErrorInfo *pError);

/**
 * @brief Options for submitting a message
 * @see rsslReactorSubmit
 */
typedef struct
{
	RsslWritePriorities	priority;						/*!< Priority of message. Affects the order of messages sent. Populated by RsslWritePriorities. */
	RsslUInt8			writeFlags;						/*!< Options for how the message is written.  Populated by RsslWriteFlags. */
	RsslUInt32			*pBytesWritten;					/*!< Returns total number of bytes written. Optional. */
	RsslUInt32			*pUncompressedBytesWritten;		/*!< Returns total number of bytes written, before any compression. Optional. */
} RsslReactorSubmitOptions;

/**
 * @brief Clears an RsslReactorSubmitOptions object.
 * @see RsslReactorSubmitOptions
 */
RTR_C_INLINE void rsslClearReactorSubmitOptions(RsslReactorSubmitOptions *pOpts)
{
	memset(pOpts, 0, sizeof(RsslReactorSubmitOptions));
	pOpts->priority = RSSL_HIGH_PRIORITY;
}

/**
 * @brief Sends the given RsslBuffer to the given RsslReactorChannel.
 * @param pReactor The reactor handling the channel to submit the message to.
 * @param pChannel The channel to send the message to.
 * @param pBuffer The buffer to send.
 * @param pSubmitOptions Options for how to send the message.
 * @param pError Error structure to be populated in the event of failure.
 * @return Value greater than RSSL_RET_SUCCESS, if dispatching succeeded and there are more messages or events to process.
 * @return RSSL_RET_SUCCESS, if dispatching succeeded and there are no more messages or events to process.
 * @return RSSL_WRITE_CALL_AGAIN, if the buffer cannot be written at this time.
 * @return failure codes, if the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslErrorInfo, RsslReactorSubmitOptions, rsslReactorGetBuffer, rsslReactorReleaseBuffer
 */
RSSL_VA_API RsslRet rsslReactorSubmit(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslBuffer *pBuffer, RsslReactorSubmitOptions *pSubmitOptions, RsslErrorInfo *pError);

/**
  * @brief Options when using rsslReactorSubmitMsg. 
  * Provides simple methods of performing advanced item request behaviors such as batch requests and requesting views(the application may also request these behaviors by encoding them
  * per the appropriate model, however these options may be more convenient). 
  * These options are supported only when a watchlist is enabled unless indicated otherwise.
  * See rsslReactorSubmitMsg, RsslReactorSubmitMsgOptions
  **/
typedef struct
{
	void		*pUserSpec;			/*!< User-specified pointer to return as the application receives events related to this request. */
} RsslReactorRequestMsgOptions;

typedef struct
{
	RsslMsg							*pRsslMsg;			/*!< RsslMsg to submit(use only one of pRsslMsg and pRDMMsg). */
	RsslRDMMsg						*pRDMMsg;			/*!< RsslRDMMsg to submit (use only one of pRsslMsg and pRDMMsg). */
	RsslBuffer						*pServiceName;		/*!< Service name to be associated with the message, if specifying the service by name instead of by ID (watchlist enabled only). */
	RsslReactorRequestMsgOptions	requestMsgOptions;	/*!< If the submitted message is an RsslRequestMsg and a watchlist is enabled, these options may also be specified. */
	RsslUInt32						majorVersion;		/*!< Major version of encoded content, if any encoded content is present. */
	RsslUInt32						minorVersion;		/*!< Minor version of encoded content, if any encoded content is present. */
} RsslReactorSubmitMsgOptions;

RTR_C_INLINE void rsslClearReactorSubmitMsgOptions(RsslReactorSubmitMsgOptions *pOpts)
{
	memset(pOpts, 0, sizeof(RsslReactorSubmitMsgOptions));
	pOpts->majorVersion = RSSL_RWF_MAJOR_VERSION;
	pOpts->minorVersion = RSSL_RWF_MINOR_VERSION;
}

/**
  * @brief Sends an RsslMsg or RsslRDM message to the ReactorChannel.
  * When the watchlist is enabled, the message is submitted to the watchlist for processing.  If the watchlist is not enabled, the message is encoded and sent directly. */
RSSL_VA_API RsslRet rsslReactorSubmitMsg(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslReactorSubmitMsgOptions *pOptions, RsslErrorInfo *pError);

/**
 * @brief The options for opening a TunnelStream.
 * @see RsslTunnelStream
 */
typedef struct
{
	char									*name;						/*!< Name used when opening this tunnel stream. This name is transmitted to the provider. If not specified, "TunnelStream" is used. Length must be 0-255 characters. */
	RsslInt32								streamId;					/*!< Stream ID to use when opening the tunnel stream. */
	RsslUInt8								domainType;					/*!< Domain type to use when opening the tunnel stream. */
	RsslUInt16								serviceId;					/*!< Service ID to use when opening the tunnel stream. */
	RsslUInt32								responseTimeout;			/*!< Time to wait for a provider response. */
	RsslUInt32								guaranteedOutputBuffers;	/*!< Number of guaranteed output buffers that will be available for the tunnel stream. */
	RsslTunnelStreamStatusEventCallback		*statusEventCallback;		/*!< Callback for status events indicating the health of the tunnel stream. */
	RsslTunnelStreamDefaultMsgCallback		*defaultMsgCallback;		/*!< Default callback for received RSSL Messages. */
	RsslTunnelStreamQueueMsgCallback		*queueMsgCallback;			/*!< Callback for received queue messages. */
	RsslRDMLoginRequest						*pAuthLoginRequest;			/*!< Login request to send, if using authentication. */
	void									*userSpecPtr;				/*!< A user-specified pointer to be associated with the tunnel stream. */
	RsslClassOfService						classOfService;				/*!< Specifies the class of service parameters that the consumer desires to use for this tunnel stream. */
} RsslTunnelStreamOpenOptions;

/**
 * @brief Clears a RsslTunnelStreamOpenOptions object.
 * @see RsslTunnelStreamOpenOptions
 */
RTR_C_INLINE void rsslClearTunnelStreamOpenOptions(RsslTunnelStreamOpenOptions *pOptions)
{
	memset(pOptions, 0, sizeof(RsslTunnelStreamOpenOptions));
	rsslClearClassOfService(&pOptions->classOfService);
	pOptions->responseTimeout = 60;
	pOptions->guaranteedOutputBuffers = 50;
}

/**
 * @brief Opens a tunnel stream for a ReactorChannel.
 * @param pReactorChannel The ReactorChannel to open the tunnel stream.
 * @param pOptions Options for opening the tunnel stream.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if opening the tunnel stream succeeded.
 * @return failure codes, if the RsslReactor was shut down due to a failure.
 * @see RsslReactorChannel, RsslTunnelStreamOpenOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorOpenTunnelStream(RsslReactorChannel *pReactorChannel, RsslTunnelStreamOpenOptions *pOptions, RsslErrorInfo *pError);

/**
 * @brief The options for closing a TunnelStream.
 * @see RsslTunnelStream
 */
typedef struct
{
	RsslBool finalStatusEvent;		/*!< Use only when closing an open tunnel stream. If provided, a final RsslTunnelStreamStatusEvent will be provided. The RsslTunnelStream will be cleaned up after returning from that event's callback. */
} RsslTunnelStreamCloseOptions;

/**
 * @brief Clears a RsslTunnelStreamCloseOptions object.
 * @see RsslTunnelStreamCloseOptions
 */
RTR_C_INLINE void rsslClearTunnelStreamCloseOptions(RsslTunnelStreamCloseOptions *pOptions)
{
	pOptions->finalStatusEvent = RSSL_FALSE;
}

/**
 * @brief Closes a tunnel stream for a ReactorChannel.
 * @param pTunnelStream The Tunnel Stream to close.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if closing the tunnel stream succeeded.
 * @return failure codes, if the RsslReactor was shut down due to a failure.
 * @see RsslReactorChannel, RsslTunnelStream, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorCloseTunnelStream(RsslTunnelStream *pTunnelStream, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pError);

/**
 * @brief Options for accepting a tunnel stream from a consumer.
 * @see rsslReactorAcceptTunnelStream
 */
typedef struct
{
	RsslTunnelStreamStatusEventCallback	*statusEventCallback;	   	/*!< Callback for status events indicating the health of the tunnel stream. */
	RsslTunnelStreamDefaultMsgCallback	*defaultMsgCallback;		/*!< Default callback for received RSSL Messages. */
	void								*userSpecPtr;	   			/*!< A user-specified pointer to be associated with the tunnel stream. */
	RsslClassOfService					classOfService;				/*!< Specifies the class of service parameters that the provider desires to use for this tunnel stream. */
	RsslUInt32							guaranteedOutputBuffers;	/*!< Number of guaranteed output buffers that will be available for the tunnel stream. */
} RsslReactorAcceptTunnelStreamOptions;

/**
 * @brief Clears a RsslReactorAcceptTunnelStreamOptions object.
 * @see RsslReactorAcceptTunnelStreamOptions
 */
RTR_C_INLINE void rsslClearReactorAcceptTunnelStreamOptions(RsslReactorAcceptTunnelStreamOptions *pOpts)
{
	pOpts->statusEventCallback = NULL;
	pOpts->defaultMsgCallback = NULL;
	pOpts->userSpecPtr = NULL;
	rsslClearClassOfService(&pOpts->classOfService);
	pOpts->guaranteedOutputBuffers = 50;
}

/**
 * @brief Accepts a tunnel stream from a consumer.
 * @param pEvent The RsslTunnelStreamRequestEvent indicating the new tunnel stream.
 * @param pOptions Options for accepting the tunnel stream.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if accepting the tunnel stream succeeded.
 * @return failure codes, if a failure occurred.
 * @see RsslReactorAcceptTunnelStreamOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorAcceptTunnelStream(RsslTunnelStreamRequestEvent *pEvent, RsslReactorAcceptTunnelStreamOptions *pRsslReactorAcceptTunnelStreamOptions, RsslErrorInfo *pError);

/**
 * @brief Options for rejecting a tunnel stream from a consumer.
 * @see rsslReactorRejectTunnelStream
 */
typedef struct
{
	RsslState 			state;		/*!< State to send when rejecting the tunnel stream. */
	RsslClassOfService	*pCos;		/*!< If rejecting due receiving an unexpected RsslClassOfService, specify this to send the expected RsslClassOfService to the consumer. */
} RsslReactorRejectTunnelStreamOptions;

/**
 * @brief Options for rejecting a tunnel stream from a consumer.
 * @see rsslReactorRejectTunnelStream
 */
RTR_C_INLINE void rsslClearReactorRejectTunnelStreamOptions(RsslReactorRejectTunnelStreamOptions *pOpts)
{
	rsslClearState(&pOpts->state);
	pOpts->state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	pOpts->state.dataState = RSSL_DATA_SUSPECT;
	pOpts->pCos = NULL;
}

/**
 * @brief Rejects a tunnel stream from a consumer.
 * @param pEvent The RsslTunnelStreamRequestEvent indicating the new tunnel stream to reject.
 * @param pOptions Options for rejecting the tunnel stream.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if rejecting the tunnel stream succeeded.
 * @return failure codes, if a failure occurred.
 * @see RsslReactorRejectTunnelStreamOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorRejectTunnelStream(RsslTunnelStreamRequestEvent *pEvent, RsslReactorRejectTunnelStreamOptions *pRsslReactorRejectTunnelStreamOptions, RsslErrorInfo *pError);

/**
 * @brief Enumerated types indicating OAuth credential renewal mode.
 * @see RsslReactorOAuthCredentialRenewalOptions
 */
typedef enum
{
	RSSL_ROC_RT_INIT = 0x00,
	RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD = 1,			/*!< Indicates that there is either a password(V1),  clientSecret(V2) or clientJWK(V2 with JWK) in this renewal call.  
														 *	 Use this for all V2 credential types, even if there is a change in clientSecret or clientJWK. */
	RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE = 2	/*!< OAuth credential V1 only: Indicates that the password has changed. 
														 *   The associated RsslReactorOAuthCredentialRenewalOptions.newPassword and RsslReactorOAuthCredentialRenewalOptions.password need to 
														 *	 be populated. */
} RsslReactorOAuthCredentialRenewalMode;

/**
 * @brief Configuration options for submitting OAuth credential renewal.
 * The proxy configuration is used only when rsslReactorSubmitOAuthCredentialRenewal() is called outside of the RsslReactorOAuthCredentialEventCallback method.
 * and your organization requires use of a proxy to get to the Internet.
 * @see RsslReactorAuthTokenEventCallback
 */
typedef struct
{
	RsslReactorOAuthCredentialRenewalMode		renewalMode;				/*!< Specify a mode for submitting OAuth credential renewal */
	RsslReactorAuthTokenEventCallback 			*pAuthTokenEventCallback;	/*!< Specify to get response from RsslReactorAuthTokenEventCallback when rsslReactorSubmitOAuthCredentialRenewal()
																			 * is called outside of the RsslReactorOAuthCredentialEventCallback method. */
	RsslBuffer									proxyHostName;				/*!<  @brief Proxy host name. */
	RsslBuffer									proxyPort;					/*!<  @brief Proxy port. */
	RsslBuffer									proxyUserName;				/*!<  @brief User Name for authenticated proxies. */
	RsslBuffer									proxyPasswd;				/*!<  @brief Password for authenticated proxies. */
	RsslBuffer									proxyDomain;				/*!<  @brief Domain for authenticated proxies. */
} RsslReactorOAuthCredentialRenewalOptions;

/**
 * @brief Clears an RsslReactorOAuthCredentialRenewalOptions object.
 * @see RsslReactorOAuthCredentialRenewalOptions
 */
RTR_C_INLINE void rsslClearReactorOAuthCredentialRenewalOptions(RsslReactorOAuthCredentialRenewalOptions *pOpts)
{
	memset(pOpts, 0, sizeof(RsslReactorOAuthCredentialRenewalOptions));
}

/**
 * @brief Submit OAuth credential renewal with password or password change.
 * @param pReactor The reactor handling the credential renewal.
 * @param pReactorOAuthCredentialRenewal Options for how to perform credential renewal.
 * @param pError Error structure to be populated in the event of failure.
 * @return failure codes, if specified invalid arguments or the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslReactorOAuthCredentialRenewalOptions, RsslReactorOAuthCredentialRenewal
 */
RSSL_VA_API RsslRet rsslReactorSubmitOAuthCredentialRenewal(RsslReactor *pReactor, RsslReactorOAuthCredentialRenewalOptions *pOptions,
						RsslReactorOAuthCredentialRenewal *pReactorOAuthCredentialRenewal, RsslErrorInfo *pError);

/**
 * @brief Configuration options for submitting Login credential.
 * @see RsslReactorLoginMsgRenewalEventCallback
 */
typedef struct
{
	RsslBuffer									userName;				/*!<  @brief The user name or authentication token for the new login credentials. Corresponds to the configured userName on the initial RsslRDMLoginRequest */
	RsslBuffer									authenticationExtended; /*!<  @brief The extended authentation for the new login credentials. Corresponds to the configured authenticationExtended on the initial RsslRDMLoginRequest */
} RsslReactorLoginCredentialRenewalOptions;

/**
 * @brief Clears an RsslReactorLoginCredentialRenewalOptions object.
 * @see RsslReactorLoginCredentialRenewalOptions
 */
RTR_C_INLINE void rsslClearReactorLoginCredentialRenewalOptions(RsslReactorLoginCredentialRenewalOptions* pOpts)
{
	memset(pOpts, 0, sizeof(RsslReactorLoginCredentialRenewalOptions));
}

/**
 * @brief Submit Login Msg renewal with updated credentials. This function can only be called during a RsslReactorLoginMsgRenewalEventCallback function. Any further updates to the login message should be done through rsslSubmitMsg.
 * @param pReactor The reactor handling the login renewal.
 * @param pReactorChannel the channel that will have it's login updated.
 * @param pOptions new Login request options.
 * @param pError Error structure to be populated in the event of failure.
 * @return failure codes, if specified invalid arguments or the RsslReactor was shut down due to a failure.
 * @see RsslReactor
 */
RSSL_VA_API RsslRet rsslReactorSubmitLoginCredentialRenewal(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorLoginCredentialRenewalOptions* pOptions, RsslErrorInfo* pError);
/**
 * @brief This structure is used to retrieve connection statistics from the rsslReactorChannelStatistic() method.
 * @see RsslReactorChannelStatisticFlags
 */
typedef struct
{
	RsslUInt							bytesRead;					/*!< Returns the aggregated number of bytes read */
	RsslUInt							uncompressedBytesRead;		/*!< Returns the aggregated number of uncompressed bytes read */
	RsslUInt							pingReceived;				/*!< Returns the aggregated number of ping received */
	RsslUInt							pingSent;					/*!< Returns the aggregated number of ping sent */
	RsslUInt							bytesWritten;				/*!< Returns the aggregated number of bytes written */
	RsslUInt							uncompressedBytesWritten;	/*!< Returns the aggregated number of uncompressed bytes written */
} RsslReactorChannelStatistic;

/**
 * @brief Clears an RsslReactorChannelStatistic object.
 * @see RsslReactorChannelStatistic
 */
RTR_C_INLINE void rsslClearReactorChannelStatistic(RsslReactorChannelStatistic *pStatistic)
{
	memset(pStatistic, 0, sizeof(RsslReactorChannelStatistic));
}

/**
 * @brief Retrieves channel statistics for the specified RsslReactorChannel. The statistics in passed in RsslReactorChannel is reset after this calls.
 * @param pReactor The reactor handling the RsslReactorChannel.
 * @param pReactorChannel The channel to retrieve channel statistics.
 * @param pRsslReactorChannelStatistic The passed in RsslReactorChannelStatistic to populate channel statistics.
 * @param pError Error structure to be populated in the event of failure.
 * @return failure codes, if specified invalid arguments or the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslReactorChannelStatistic
 */
RSSL_VA_API RsslRet rsslReactorRetrieveChannelStatistic(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslReactorChannelStatistic *pRsslReactorChannelStatistic, RsslErrorInfo *pError);

/**
 * @brief Configuration options for initializing JSON converter.
 * @see rsslReactorInitJsonConverter
 */
typedef struct {
	RsslDataDictionary						*pDictionary;					/*!< the RsslDataDictionary to initialize the RWF/JSON converter. */
	void									*userSpecPtr; 					/*!< user-specified pointer which will be retrieved in the callback function. */
	RsslInt32								defaultServiceId;				/*!< Specify a default service ID for a request if both service name and ID are not set. A service ID must be in between 0 and 65535. */
	RsslReactorServiceNameToIdCallback		*pServiceNameToIdCallback;		/*!< Callback function that handles conversion from service name to ID. */
	RsslReactorJsonConversionEventCallback	*pJsonConversionEventCallback;	/*!< Callback function that receives RsslReactorJsonConversionEvent when the JSON converter failed to convert message. */
	RsslBool								jsonExpandedEnumFields;			/*!< Expand enumerated values in field entries to their display values for JSON protocol. */
	RsslBool								catchUnknownJsonKeys;			/*!< When converting from JSON to RWF, catch unknown JSON keys. */
	RsslBool								catchUnknownJsonFids;			/*!< When converting from JSON to RWF, catch unknown JSON field IDs. */
	RsslBool								closeChannelFromFailure;		/*!< Closes the channel when the Reactor failed to parse JSON message or received JSON error message. */
	RsslUInt32								outputBufferSize;				/*!< Size of the buffer that the converter will allocate for its output buffer. The conversion fails if the size is not large enough */
	RsslUInt32								jsonTokenIncrementSize;				/*!< Number of json token increment size for parsing JSON messages. */
	RsslBool								sendJsonConvError;				/*!< Enable sending json conversion error>*/
} RsslReactorJsonConverterOptions;

/**
 * @brief Clears an RsslReactorJsonConverterOptions object.
 * @see RsslReactorJsonConverterOptions
 */
RTR_C_INLINE void rsslClearReactorJsonConverterOptions(RsslReactorJsonConverterOptions *pReactorJsonConverterOptions)
{
	memset(pReactorJsonConverterOptions, 0, sizeof(RsslReactorJsonConverterOptions));
	pReactorJsonConverterOptions->defaultServiceId = -1;
	pReactorJsonConverterOptions->catchUnknownJsonFids = RSSL_TRUE;
	pReactorJsonConverterOptions->closeChannelFromFailure = RSSL_TRUE;
	pReactorJsonConverterOptions->outputBufferSize = 65535;
	pReactorJsonConverterOptions->jsonTokenIncrementSize = 500;
}

/**
 * @brief Initializes an RsslReactor to be able to convert payloads to and from RWF and JSON protocol
 * @param pReactor The reactor that will handle the request.
 * @param pDictionary the RsslDataDictionary to initialize the RWF/JSON converter
 * @param pError Error structure to be populated in the event of an error.
 * @return RSSL_RET_SUCCESS, if initialization succeeded
 * @return failure codes, if the RsslReactor was shut down due to a failure.
 * @see RsslReactorJsonConverterOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorInitJsonConverter(RsslReactor *pReactor, RsslReactorJsonConverterOptions *pReactorJsonConverterOptions, RsslErrorInfo *pError);

/**
 * @brief Reactor IOCtl codes
 * @see RsslReactorRestLoggingCallback
 * @see rsslReactorIoctl
 */
typedef enum {
	RSSL_RIC_ENABLE_REST_LOGGING = 1,			/*!< (1) Enable or disable REST interaction debug messages. (RsslReactorImpl.restEnableLog) */
	RSSL_RIC_ENABLE_REST_CALLBACK_LOGGING = 2,	/*!< (2) Enable or disable invoking a callback specified by user to receive Rest logging message. (RsslReactorImpl.restEnableLogViaCallback) */
} RsslReactorIoctlCodes;

/**
 * @brief Allows changing some I/O values programmatically
 *
 * Typical use:<BR>
 * If an I/O value needs to be changed, this is used
 *
 * @param pReactor The reactor that will handle the request.
 * @param code Code of I/O Option to change
 * @param value Value to change Option to
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 * @see RsslReactorIoctlCodes, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorIoctl(RsslReactor* pReactor, RsslReactorIoctlCodes code, void* value, RsslErrorInfo* pError);

/**
 * @brief This structure is used to retrieve per Reactor debugging information from the rsslReactorGetDebugInfo() method.
 */
typedef struct
{
	RsslBuffer							debugInfoBuffer;					/*!< Returns the debugging information as text per Reactor. This is read only buffer.*/
} RsslReactorDebugInfo;

/**
 * @brief Clears an RsslReactorDebugInfo object.
 * @see RsslReactorDebugInfo
 */
RTR_C_INLINE void rsslClearReactorDebugInfo(RsslReactorDebugInfo* pReactorDebugInfo)
{
	memset(pReactorDebugInfo, 0, sizeof(RsslReactorDebugInfo));
}

/**
 * @brief Retrieves per Reactor debugging information. The debugging information is cleared after this calls.
 * @param pReactor The reactor to get debugging information.
 * @param pReactorDebugInfo The passed in pReactorDebugInfo to retrieve debugging information.
 * @param pError Error structure to be populated in the event of failure.
 * @return failure codes, if specified invalid arguments or the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslReactorDebugInfo
 */
RSSL_VA_API RsslRet rsslReactorGetDebugInfo(RsslReactor* pReactor, RsslReactorDebugInfo* pReactorDebugInfo, RsslErrorInfo* pError);

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
