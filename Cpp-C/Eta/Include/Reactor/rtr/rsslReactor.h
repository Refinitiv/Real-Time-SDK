/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2019. All rights reserved.
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
	RSSL_RC_DICTIONARY_DOWNLOAD_NONE 			= 0,	/*!< (0) Do not automatically reequest dictionary messages. */
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
	RsslUInt32						maxOutstandingPosts;	/*!< Sets the maximum number of post acknowledgements that may be oustanding for the channel. */
	RsslUInt32						postAckTimeout;			/*!< Time a stream will wait for acknowledgement of a post message, in milliseconds. */
	RsslUInt32						requestTimeout;			/*!< Time a requested stream will wait for a response from the provider, in milliseconds. */
} RsslConsumerWatchlistOptions;

/**
 * @brief Structure representing the OAuth credential for authorization with the EDP token service.
 * @see RsslReactorOMMConsumerRole
 */
typedef struct
{
	RsslBuffer									userName;						/*!< The user name required to authorize with the EDP token service. */
	RsslBuffer									password;						/*!< The password for user name used to get access token. */
	RsslBuffer									clientId;						/*!< A unique ID defined for an application marking the request. Optional 
																				 *	The userName variable is used if this member is not set. */
	RsslBuffer									clientSecret;					/*!< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	RsslBuffer									tokenScope;						/*!< A user can optionally limit the scope of generated token. Optional*/
	RsslReactorOAuthCredentialEventCallback		*pOAuthCredentialEventCallback; /*!< Callback function that receives RsslReactorOAuthCredentialEvent to specify sensitive information. 
																				 *   The Reactor will not copy password and client secret if the function pointer is specified.*/
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
															 * Specifies an unique ID defined for an application making a request to the EDP token service.
															 * The RsslRDMLoginRequest.userName variable is used if this member is not set. Optional.*/
	RsslReactorOAuthCredential		*pOAuthCredential;		/*!< A OAuth credential for authentication with the EDP token service. This member has higher precedence for
																authorization than the user credential specified in pLoginRequest. Optional. */
	RsslRDMLoginMsgCallback			*loginMsgCallback;		/*!< A callback function for processing RsslRDMLoginMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslRDMDirectoryRequest			*pDirectoryRequest;		/*!< A Directory Request to be sent during the setup of a Consumer-Provider session. Optional. Requires pLoginRequest to be set.*/
	RsslRDMDirectoryMsgCallback		*directoryMsgCallback;	/*!< A callback function for processing RsslRDMDirectoryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslDownloadDictionaryMode		dictionaryDownloadMode;	/*!< Indicates a method of requesting dictionaries from the Provider. If not set to RSSL_RC_DICTIONARY_DOWNLOAD_NONE, requires pLoginRequest and pDirectoryRequest to be set. */
	RsslRDMDictionaryMsgCallback	*dictionaryMsgCallback;	/*!< A callback function for processing RsslRDMDictionaryMsgs received. If not present, the received message will be passed to the defaultMsgCallback. */
	RsslConsumerWatchlistOptions 	watchlistOptions;		/*!< Options for using the watchlist. */
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
 * @brief Configuration options for creating an RsslReactor.
 * @see rsslCreateReactor
 */
typedef struct {
	RsslInt32	dispatchDecodeMemoryBufferSize;	/*!< Size of the memory buffer(in bytes) that the RsslReactor will use when decoding RsslRDMMsgs to pass to callback functions. */
	void		*userSpecPtr; 					/*!< user-specified pointer which will be set on the Reactor. */
	RsslBuffer	serviceDiscoveryURL;			/*!< Specifies a URL for the EDP-RT service discovery. The service discovery is used when the connection arguments is not specified
												 * in the RsslReactorConnectInfo.rsslConnectOptions */
	RsslBuffer	tokenServiceURL;				/*!< Specifies a URL of the token service to get an access token and a refresh token. This is used for querying EDP-RT service
												 * discovery and subscribing data from EDP-RT. */
	RsslDouble	tokenReissueRatio;				/*!< Specifies a ratio to multiply with access token validity time(seconds) for retrieving and reissuing the access token. 
												 * The valid range is between 0.01 to 0.99. */
	int			port;							/*!< @deprecated DEPRECATED: This parameter no longer has any effect. It was a port used for creating the eventFd descriptor on the RsslReactor. It was never used on Linux or Solaris platforms. */
} RsslCreateReactorOptions;

/**
 * @brief Clears an RsslCreateReactorOptions object.
 * @see RsslCreateReactorOptions
 */
RTR_C_INLINE void rsslClearCreateReactorOptions(RsslCreateReactorOptions *pReactorOpts)
{
	memset(pReactorOpts, 0, sizeof(RsslCreateReactorOptions));
	pReactorOpts->dispatchDecodeMemoryBufferSize = 65536;
	pReactorOpts->port = 55000;
	pReactorOpts->serviceDiscoveryURL.data = (char *)"https://api.refinitiv.com/streaming/pricing/v1";
	pReactorOpts->serviceDiscoveryURL.length = 46;
	pReactorOpts->tokenServiceURL.data = (char *)"https://api.refinitiv.com/auth/oauth2/beta1/token";
	pReactorOpts->tokenServiceURL.length = 49;
	pReactorOpts->tokenReissueRatio = 0.8;
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
 * @brief Cleans up an RsslReactor.  Stops the UPA Reactor if necessary and sends RsslReactorChannelEvents to all active channels indicating that they are down.
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
 * @brief Configuration options for querying EDP-RT service discovery to get service endpoints.
 * @see rsslReactorQueryServiceDiscovery
 */
typedef struct
{
	RsslBuffer                              userName; /* !< Specifies a user name for authorization with the token service. */
	RsslBuffer                              password; /* !< Specifies a password for authorization with the token service. */
	RsslBuffer                              clientId; /* !< Specifies an unique ID defined for an application making a request to the token service. Optional
														 * The userName variable is used if this member is not set. */
	RsslBuffer                              clientSecret; /* !< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	RsslBuffer								tokenScope; /* !< A user can optionally limit the scope of generated token. Optional */
	RsslReactorDiscoveryTransportProtocol   transport;  /*!< This is an optional parameter to specify the desired transport protocol to get
														 * service endpoints from the service discovery. */
	RsslReactorDiscoveryDataFormatProtocol  dataFormat; /*!< This is an optional parameter to specify the desired data format protocol to get
														 * service endpoints from the service discovery. */
	RsslReactorServiceEndpointEventCallback *pServiceEndpointEventCallback; /*!< Callback function that receives RsslReactorServiceEndpointEvents. Applications can use service
																			 * endpoint information from the callback to get an endpoint and establish a connection to the service */

	void                                    *userSpecPtr; 					/*!< user-specified pointer which will be set on the RsslReactorServiceEndpointEvent. */

	RsslBuffer                              proxyHostName; /*!< specifies a proxy server hostname. */
	RsslBuffer                              proxyPort;     /*!< specifies a proxy server port. */
	RsslBuffer                              proxyUserName; /*!< specifies a username to perform authorization with a proxy server. */
	RsslBuffer                              proxyPasswd;   /*!< specifies a password to perform authorization with a proxy server. */
	RsslBuffer                              proxyDomain;   /*!< specifies a proxy domain of the user to authenticate.
															Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols. */

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
}

/**
 * @brief Queries EDP-RT service discovery to get service endpoint information.
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
	RsslBool            enableSessionManagement; /*!< Indicates RsslReactor to request the authentication token and query an endpoint from EDP-RT service discovery if both host
												  * and port are not specified by users. The watchlist must be enable for the Reactor to send and reissue the token via the login
												  * request to keep the connection alive on behalf of users.*/
	RsslBuffer          location;                /*!< Specifies a location to get a service endpoint to establish a connection with service provider. 
												  * Defaults to "us-east" if not specified. The Reactor always uses a endpoint which provides two availability zones 
												  * for the location. */
	RsslReactorAuthTokenEventCallback *pAuthTokenEventCallback; /*!< Callback function that receives RsslReactorAuthTokenEvents. The token is requested 
	                                                             * by the Reactor for Consumer(disabling watchlist) and NiProvider applications to send login request and
																 * reissue with the token */
	RsslInt32			reissueTokenAttemptLimit;	/*!< The maximum number of times the RsllReactor will attempt to reissue the token for a channel. If set to -1, there is no limit. */
} RsslReactorConnectInfo;

RTR_C_INLINE void rsslClearReactorConnectInfo(RsslReactorConnectInfo *pInfo)
{
	rsslClearConnectOpts(&pInfo->rsslConnectOptions);
	pInfo->initializationTimeout = 60;
	pInfo->enableSessionManagement = RSSL_FALSE;
	pInfo->location.data = (char *)"us-east";
	pInfo->location.length = 7;
	pInfo->pAuthTokenEventCallback = NULL;
	pInfo->reissueTokenAttemptLimit = -1;
}

/**
 * @brief Configuraion options for creating an RsslReactor client-side connection.
 * @see rsslReactorConnect
 */
typedef struct
{
	RsslConnectOptions		rsslConnectOptions;		/*!< Options for creating the connection. */
	RsslUInt32				initializationTimeout;	/*!< Time(in seconds) to wait for successful initialization of a channel. 
													 * If initialization does not complete in time, a RsslReactorChannelEvent will be sent indicating that the channel is down. */

	RsslInt32				reconnectAttemptLimit;	/*!< The maximum number of times the RsllReactor will attempt to reconnect a channel. If set to -1, there is no limit. */
	RsslInt32				reconnectMinDelay;		/*!< The minumum time the RsslReactor will wait before attempting to reconnect, in milliseconds. */
	RsslInt32				reconnectMaxDelay;		/*!< The maximum time the RsslReactor will wait before attempting to reconnect, in milliseconds. */

	RsslReactorConnectInfo	*reactorConnectionList;	/*!< A list of connnections.  Each connection in the list will be tried with each reconnection attempt. */
	RsslUInt32				connectionCount;		/*!< The number of connections in reactorConnectionList. */

	RsslUInt32				connectionDebugFlags;	/*!< Set of RsslDebugFlags for calling the user-set debug callbacks.  These callbacks should be set with rsslSetDebugFunctions.  If set to 0, the debug callbacks will not be used. */

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
	pOpts->connectionDebugFlags = 0;
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
 * @brief Configuraion options for creating an RsslReactor server-side connection.
 * @see rsslReactorAccept
 */
typedef struct
{
	RsslAcceptOptions	rsslAcceptOptions;		/*!< Options for accepting the connection. */
	RsslUInt32			initializationTimeout;	/*!< Time(in seconds) to wait for successful initialization of a channel. 
												 * If initialization does not complete in time, a RsslReactorChannelEvent will be sent indicating that the channel is down. */
	RsslUInt32			connectionDebugFlags;	/*!< Set of RsslDebugFlags for calling the user-set debug callbacks.  These callbacks should be set with rsslSetDebugFunctions.  If set to 0, the debug callbacks will not be used. */

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
 * @return failure codees, if the RsslReactor was shut down due to a failure.
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
	RsslUInt8			writeFlags;						/*!< Options for how the message is written.  Populated by RsslWritePriorities. */
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
	RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD = 1,
	RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE = 2
} RsslReactorOAuthCredentialRenewalMode;

/**
 * @brief Configuration options for submitting OAuth credential renewal.
 * The proxy configuration is used only when there is no RsslReactorChannel specified in rsslReactorSubmitOAuthCredentialRenewal()
 * and your organization requires use of a proxy to get to the Internet.
 * @see RsslReactorAuthTokenEventCallback
 */
typedef struct
{
	RsslReactorOAuthCredentialRenewalMode		renewalMode;				/*!< Specify a mode for submitting OAuth credential renewal */
	RsslReactorAuthTokenEventCallback 			*pAuthTokenEventCallback;	/*!< Specify to get response from RsslReactorAuthTokenEventCallback when there is no RsslReactorChannel
																			 * specified in the rsslReactorSubmitOAuthCredentialRenewal() */
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
 * @param pChannel The channel to perform credential renewal. This option can be NULL to perform operation without a channel.
 * @param pReactorOAuthCredentialRenewal Options for how to perform credential renewal.
 * @param pError Error structure to be populated in the event of failure.
 * @return failure codes, if specified invalid arguments or the RsslReactor was shut down due to a failure.
 * @see RsslReactor, RsslReactorOAuthCredentialRenewalOptions, RsslReactorOAuthCredentialRenewal
 */
RSSL_VA_API RsslRet rsslReactorSubmitOAuthCredentialRenewal(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslReactorOAuthCredentialRenewalOptions *pOptions,
						RsslReactorOAuthCredentialRenewal *pReactorOAuthCredentialRenewal, RsslErrorInfo *pError);

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
