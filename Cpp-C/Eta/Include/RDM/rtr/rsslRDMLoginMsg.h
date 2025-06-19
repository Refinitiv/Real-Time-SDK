/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017,2019-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RSSL_RDM_LOGIN_MSG_H
#define _RSSL_RDM_LOGIN_MSG_H

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include "rtr/rsslRDMMsgBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMLogin
 *	@{
 */
 
/**
 * @brief Warm Standby Information for the RDM Login Consumer Connection Status.
 * @see RsslRDMLoginConsumerConnectionStatus, rsslClearRDMLoginWarmStandbyInfo
 */
typedef struct
{
	RsslMapEntryActions action;		/*!< Action associated with this information. */
	RsslUInt warmStandbyMode;		/*!< The desired Warm Standby Mode. Populated by RDMLoginServerTypes */
} RsslRDMLoginWarmStandbyInfo;

/**
 * @brief Clears an RsslRDMLoginWarmStandbyInfo.
 * @see RsslRDMLoginWarmStandbyInfo, RsslRDMLoginConsumerConnectionStatus
 */
RTR_C_INLINE void rsslClearRDMLoginWarmStandbyInfo(RsslRDMLoginWarmStandbyInfo *pInfo)
{
	pInfo->action = RSSL_MPEA_ADD_ENTRY;
	pInfo->warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
}

/**
 * @brief The RDM Login Server Info Flags.
 * @see RsslRDMServerInfo, RsslRDMLoginRefresh
 */
typedef enum {
	RDM_LG_SIF_NONE 			= 0x00,	/*!< (0x00) No flags set */
	RDM_LG_SIF_HAS_LOAD_FACTOR 	= 0x01,	/*!< (0x01) Indicates presence of the loadFactor member. */
	RDM_LG_SIF_HAS_TYPE 		= 0x02	/*!< (0x02) Indicates presence of the serverType member. */
} RsslRDMServerInfoFlags;

/**
 * @brief Information about an available Server.  A list of RsslRDMServerInfo is used by an RsslRDMLoginRefresh
 * to list servers available for connecting to, and whether to use them as Standby servers.
 * @see RsslRDMServerInfoFlags, RsslRDMLoginRefresh, rsslClearRDMServerInfo
 */
typedef struct {
	RsslUInt32	flags;			/*!< The Server Info Flags. Populated by RsslRDMServerInfoFlags. */
	RsslInt32	serverIndex;	/*!< An index for the server. */
	RsslBuffer	hostname;		/*!< The server's hostName to connect to. */
	RsslUInt	port;			/*!< The server's port to connect to. */
	RsslUInt	loadFactor;		/*!< The server's current Load Factor. */
	RsslEnum	serverType;		/*!< Type indicating how the server is expected to be used. Populated by RDMLoginServerTypes */
} RsslRDMServerInfo;

/**
 * @brief Clears an RsslRDMServerInfo.
 * @see RsslRDMServerInfo
 */
RTR_C_INLINE void rsslClearRDMServerInfo(RsslRDMServerInfo *pServerInfo)
{
	pServerInfo->flags = RDM_LG_SIF_NONE;
	pServerInfo->serverIndex = 0;
	rsslClearBuffer(&pServerInfo->hostname);
	pServerInfo->port = 0;
	pServerInfo->loadFactor = 65535;
	pServerInfo->serverType = RDM_LOGIN_SERVER_TYPE_STANDBY;
}

/**
 * @brief The RDM Login Warm Standby modes.
 * @see RsslRDMLoginRefresh
 */
typedef enum {
	RDM_LG_WSBM_NONE = 0x00,			/*!< (0x00) No modes set */
	RDM_LG_WSBM_LOGIN_BASED = 0x01,		/*!< (0x01) Indicates whether provider supports login based warm standby. */
	RDM_LG_WSBM_SERVICE_BASED = 0x02	/*!< (0x02) Indicates whether provider supports service based warm standby. */
} RsslRDMLoginWarmStandbyModes;

/** 
 * @brief The types of RDM Login Messages.  When the rdmMsgBase's domainType is RSSL_DMT_LOGIN, 
 * the rdmMsgType member may be set to one of these to indicate the specific RsslRDMLoginMsg class.
 * @see RsslRDMLoginMsg, RsslRDMMsgBase, RsslRDMLoginRequest, RsslRDMLoginClose, RsslRDMLoginConsumerConnectionStatus, RsslRDMLoginRefresh, RsslRDMLoginStatus
 */
typedef enum {
	RDM_LG_MT_UNKNOWN						= 0,	/*!< (0) Unknown */
	RDM_LG_MT_REQUEST						= 1,	/*!< (1) Login Request */ 
	RDM_LG_MT_CLOSE							= 2,	/*!< (2) Login Close */
	RDM_LG_MT_CONSUMER_CONNECTION_STATUS	= 3,	/*!< (3) Login Consumer Connection Status */
	RDM_LG_MT_REFRESH						= 4,	/*!< (4) Login Refresh */
	RDM_LG_MT_STATUS						= 5,	/*!< (5) Login Status */
	RDM_LG_MT_POST							= 6, 	/*!< (6) Indicates an off-stream Post Message. */
	RDM_LG_MT_ACK							= 7, 	/*!< (7) Incidates an off-stream Post Acknowledgement. */
	RDM_LG_MT_RTT							= 8 	/*!< (8) Login RTT message. */
} RsslRDMLoginMsgType;

/**
 * @brief The RDM Login Request Flags
 * @see RsslRDMLoginRequest
 */
typedef enum {
	RDM_LG_RQF_NONE						= 0x000000,	/*!< (0x0000) No flags set */
	RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA	= 0x000001,	/*!< (0x0001) Indicates presence of the allowSuspectData member. */
	RDM_LG_RQF_HAS_APPLICATION_ID		= 0x000002,	/*!< (0x0002) Indicates presence of the applicationId member */
	RDM_LG_RQF_HAS_APPLICATION_NAME		= 0x000004,	/*!< (0x0004) Indicates presence of the applicationName member */
	RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG	= 0x000008,	/*!< (0x0008) Indicates presence of the downloadConnectionConfig member */	
	RDM_LG_RQF_HAS_INSTANCE_ID			= 0x000010,	/*!< (0x0010) Indicates presence of the instanceId member */
	RDM_LG_RQF_HAS_PASSWORD				= 0x000020,	/*!< (0x0020) Indicates presence of the password member */
	RDM_LG_RQF_HAS_POSITION				= 0x000040,	/*!< (0x0040) Indicates presence of the position member */
	RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR	= 0x000080,	/*!< (0x0080) Indicates presence of the providePermissionExpressions member */
	RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE = 0x000100,	/*!< (0x0100) Indicates presence of the providePermissionProfile member */
	RDM_LG_RQF_HAS_ROLE					= 0x000200,	/*!< (0x0200) Indicates presence of the role member */
	RDM_LG_RQF_HAS_SINGLE_OPEN			= 0x000400,	/*!< (0x0400) Indicates presence of the singleOpen member */
	RDM_LG_RQF_HAS_USERNAME_TYPE		= 0x000800,	/*!< (0x0800) Indicates presence of the userNameType member */
	RDM_LG_RQF_NO_REFRESH				= 0x001000,	/*!< (0x1000) Indicates the Consumer or Noninteractive provider does not require a refresh. */
	RDM_LG_RQF_PAUSE_ALL						= 0x002000,	/*!< (0x2000) Used by a Consumer to request that all open items on a channel be paused. Support for this request is indicated by the supportOptimizedPauseResume member of the RsslRDMLoginRefresh */
	RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD	= 0x004000, /*!< (0x4000) Indiactes presence of the supportProviderDictionaryDownload member */
	RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN	= 0x008000,	/*!< (0x8000) Indicates presence of the applicationAuthenticationToken member */
	RDM_LG_RQF_HAS_AUTHN_EXTENDED		= 0x010000,	/*!< (0x010000) Indicates presence of extended authentication data. */
	RDM_LG_RQF_RTT_SUPPORT				= 0x020000	/*!< (0x020000) Indicates that the consumer supports the RTT feature. */
} RsslRDMLoginRequestFlags;

/**
 * @brief The RDM Login Request.  Used by an OMM Consumer or OMM Noninteractive Provider
 *  to request a login. 
 * @see RsslRDMMsgBase, RsslRDMLoginMsg, rsslClearRDMLoginRequest, rsslInitDefaultRDMLoginRequest
 */
typedef struct {
	RsslRDMMsgBase rdmMsgBase;					/*!< The Base RDM Message. */
	RsslUInt32	flags; 							/*!< The Login Request flags. Populated from RsslRDMLoginRequestFlags. */
	RsslBuffer	applicationId;					/*!< The DACS Application ID. */
	RsslBuffer	applicationName;				/*!< The ApplicationName. */
	RsslUInt	downloadConnectionConfig;		/*!< Indicates whether the Consumer desires connection information. If available, a list of servers will be present in the serverList member of the RsslRDMLoginRefresh. */
	RsslBuffer	instanceId; 					/*!< The InstanceId. */
	RsslBuffer	password;						/*!< The Password. */
	RsslBuffer	position;						/*!< The DACS Position. */
	RsslUInt	allowSuspectData;				/*!< Indicates whether the Consumer desires allowing suspect streamState on its items. */
	RsslUInt	providePermissionExpressions;	/*!< Indicates whether the Consumer desires permission expression information to be sent with responses. */
	RsslUInt	providePermissionProfile;		/*!< Indicates whether the Consumer desires the permission profile. */
	RsslUInt	role;							/*!< Indicates the role of the application.  Populated by RDMLoginRoleTypes */
	RsslUInt	singleOpen;						/*!< Indicates whether the Consumer desires for the Provider to handle recovery on its behalf. */
	RsslUInt	supportProviderDictionaryDownload;	/*!< Indicates whether the Consumer supports dictionary download. */
	RsslBuffer	userName;						/*!< The UserName being used to login.  For UserAuthn Authentication, this should contain Authentication Token */
	RsslUInt8	userNameType;					/*!< The type of the userName.  Populated by RDMLoginUserIdTypes */
	RsslBuffer  applicationAuthorizationToken;	/*!< The ApplicationAuthorizationToken. */
	RsslBuffer 	authenticationExtended;			/*!< Additional data required for UserAuthn Authentication */
	char defaultUsername[256];					/*!< Provides memory space for the userName when using the rsslInitDefaultRDMLoginRequest() function. */
	char defaultPosition[256];					/*!< Provides memory space for the position when using the rsslInitDefaultRDMLoginRequest() function. */
} RsslRDMLoginRequest;

/**
 * @brief Clears an RsslRDMLoginRequest.
 * @see RsslRDMLoginRequest, rsslInitDefaultRDMLoginRequest
 */
RTR_C_INLINE void rsslClearRDMLoginRequest(RsslRDMLoginRequest *pRequest)
{
	rsslClearRDMMsgBase(&pRequest->rdmMsgBase);
	pRequest->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pRequest->rdmMsgBase.rdmMsgType = RDM_LG_MT_REQUEST;
	pRequest->flags = RDM_LG_RQF_NONE;
	pRequest->userNameType = RDM_LOGIN_USER_NAME;
	rsslClearBuffer(&pRequest->userName);
	rsslClearBuffer(&pRequest->applicationId);
	rsslClearBuffer(&pRequest->applicationName);
	rsslClearBuffer(&pRequest->applicationAuthorizationToken);
	rsslClearBuffer(&pRequest->authenticationExtended);
	rsslClearBuffer(&pRequest->position);
	rsslClearBuffer(&pRequest->password);
	rsslClearBuffer(&pRequest->instanceId);
	pRequest->providePermissionProfile = 1;
	pRequest->providePermissionExpressions = 1;
	pRequest->singleOpen = 1;
	pRequest->supportProviderDictionaryDownload = 0;
	pRequest->allowSuspectData = 1;
	pRequest->role = RDM_LOGIN_ROLE_CONS;
	pRequest->downloadConnectionConfig = 0;
	
}

/**
 * @brief Initializes an RsslRDMLoginRequest, clearing it and filling in a typical userName, applicationName and position. 
 * NOTE: On Windows it is recommended that rsslInitialize() is called before calling this function to ensure that position is correctly set.
 * @see RsslRDMLoginRequest, rsslInitDefaultRDMLoginRequest
 */
RTR_C_INLINE RsslRet rsslInitDefaultRDMLoginRequest(RsslRDMLoginRequest *pRequest, RsslInt32 streamId)
{
	RsslUInt32 ipAddress = 0;
	/* size = 256 - 4 to avoid -Wformat-truncation GCC warning when snprinf("%s/net") */
	char tmpHostNameBlock[256 - 4];
	RsslBuffer tmpHostName = { sizeof(tmpHostNameBlock), tmpHostNameBlock };

	rsslClearRDMLoginRequest(pRequest);

	pRequest->rdmMsgBase.streamId = streamId;

	/* Set username to system name */
	pRequest->userName.data = pRequest->defaultUsername;
	pRequest->userName.length = sizeof(pRequest->defaultUsername);
	if (rsslGetUserName(&pRequest->userName) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	/* Set application name to "eta" */
	pRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_NAME;
	pRequest->applicationName.data = (char *)"eta";
	pRequest->applicationName.length = 3;

	/* Specify an application ID */
	pRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
	pRequest->applicationId.data = (char *)"256"; 
	pRequest->applicationId.length = 3;


	/* Set position to current IP address. */
	pRequest->flags |= RDM_LG_RQF_HAS_POSITION;

	if (gethostname(tmpHostNameBlock, sizeof(tmpHostNameBlock)) != 0)
		snprintf(tmpHostNameBlock, sizeof(tmpHostNameBlock), "localhost");

	if (rsslHostByName(&tmpHostName, &ipAddress) == RSSL_RET_SUCCESS)
	{
		rsslIPAddrUIntToString(ipAddress, tmpHostNameBlock);
		snprintf(pRequest->defaultPosition, sizeof(pRequest->defaultPosition), "%s/net", tmpHostNameBlock);
		pRequest->position.data = pRequest->defaultPosition;
		pRequest->position.length = (RsslUInt32)strlen(pRequest->defaultPosition);
	}
	else
	{
		snprintf(pRequest->defaultPosition, sizeof(pRequest->defaultPosition), "localhost");
		pRequest->position.data = pRequest->defaultPosition;
		pRequest->position.length = (RsslUInt32)strlen(pRequest->defaultPosition);
	}
	
	return RSSL_RET_SUCCESS;
}

/**
 * @brief The RDM Login Refresh Flags
 * @see RsslRDMLoginRefresh
 */
typedef enum
{
	RDM_LG_RFF_NONE                      		= 0x00000000,	/*!< (0x00000000) No flags set. */
	RDM_LG_RFF_CLEAR_CACHE               		= 0x00000001,	/*!< (0x00000001) Indicates whether the receiver of the refresh should clear any associated cache information. */
	RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA    		= 0x00000002,	/*!< (0x00000002) Indicates presence of the allowSuspectData member. */
	RDM_LG_RFF_HAS_APPLICATION_ID        		= 0x00000004,	/*!< (0x00000004) Indicates presence of the applicationId member. */
	RDM_LG_RFF_HAS_APPLICATION_NAME      		= 0x00000008,	/*!< (0x00000008) Indicates presence of the applicationName member. */
	RDM_LG_RFF_HAS_CONN_CONFIG           		= 0x00000010,	/*!< (0x00000010) Indicates presence of the numStandbyServers and serverList members. */
	RDM_LG_RFF_HAS_POSITION              		= 0x00000020,	/*!< (0x00000020) Indicates presence of the position member. */
	RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR     		= 0x00000040,	/*!< (0x00000040) Indicates presence of the providePermissionExpressions member. */
	RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE  		= 0x00000080,	/*!< (0x00000080) Indicates presence of the providePermissionProfile member. */
	RDM_LG_RFF_HAS_SEQ_NUM               		= 0x00000100,	/*!< (0x00000100) Indicates presence of the sequenceNumber member. */
	RDM_LG_RFF_HAS_SINGLE_OPEN           		= 0x00000200,	/*!< (0x00000200) Indicates presence of the singleOpen member. */
	RDM_LG_RFF_HAS_SUPPORT_BATCH         		= 0x00000400,	/*!< (0x00000400) Indicates presence of the supportBatchRequests member. */
	RDM_LG_RFF_HAS_SUPPORT_POST          		= 0x00000800,	/*!< (0x00000800) Indicates presence of the supportOMMPost member. */
	RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE     		= 0x00001000,	/*!< (0x00001000) Indicates presence of the supportOptimizedPauseResume member. */
	RDM_LG_RFF_HAS_SUPPORT_STANDBY       		= 0x00002000,	/*!< (0x00002000) Indicates presence of the supportStandby member. */
	RDM_LG_RFF_HAS_SUPPORT_VIEW          		= 0x00004000,	/*!< (0x00004000) Indicates presence of the supportViewRequests member. */
	RDM_LG_RFF_HAS_USERNAME              		= 0x00008000,	/*!< (0x00008000) Indicates presence of the userName member. */
	RDM_LG_RFF_HAS_USERNAME_TYPE         		= 0x00010000,	/*!< (0x00010000) Indicates presence of the userNameType member. */
	RDM_LG_RFF_SOLICITED                 		= 0x00020000,	/*!< (0x00020000) Indicates whether this refresh is being provided in response to a request. */
	RDM_LG_RFF_HAS_SUPPORT_ENH_SL				= 0x00040000,	/*!< (0x00040000) Indicates presence of the supportEnhancedSymbolList member. */
	RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD	= 0x00080000,	/*!< (0x00080000) Indicates presence of the supportProviderDictionaryDownload member. */
	RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL			= 0x00100000,	/*!< (0x00100000) Indicates presence of the sequenceRetryInterval member. */
	RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT				= 0x00200000,	/*!< (0x00200000) Indicates presence of the updateBufferLimit member. */
	RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY				= 0x00400000,	/*!< (0x00400000) Indicates presence of the sequenceNumberRecovery member. */
	RDM_LG_RFF_HAS_AUTHN_TT_REISSUE				= 0x00800000,	/*!< (0x00800000) Indicates presence of the authentication time to reissue member */
	RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP			= 0x01000000,	/*!< (0x01000000) Indicates presence of the authentication extended response buffer */
	RDM_LG_RFF_HAS_AUTHN_ERROR_CODE				= 0x02000000,	/*!< (0x02000000) Indicates presence of the authenication error code */
	RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT				= 0x04000000,	/*!< (0x04000000) Indicates presence of the authentication error text member */
	RDM_LG_RFF_RTT_SUPPORT						= 0x08000000,	/*!< (0x04000000) Indicates RTT feature support. */
	RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE			= 0x10000000	/*!< (0x10000000) Indicates presence of the supportStandbyMode member. */
} RsslRDMLoginRefreshFlags;

/**
 * @brief The RDM Login Refresh. Used by an OMM Provider to accept or reject a login request and provide information about features supported.
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMServerInfo, rsslClearRDMLoginRefresh
 */
typedef struct {
	RsslRDMMsgBase				rdmMsgBase;						/*!< The Base RDM Message. */
	RsslUInt32					flags;							/*!< The RDM Login Refresh flags. Populated by RsslRDMLoginRefreshFlags. */
	RsslUInt32					sequenceNumber;					/*!< The sequence number of this message. */
	RsslState					state;							/*!< The current state of the login stream. */

	RsslBuffer					userName;						/*!< The userName that was used when sending the Login Request. */
	RsslUInt8					userNameType;					/*!< The type of the userName that was used with the Loginn Request.Populated by RDMLoginUserIdTypes */
	RsslUInt					allowSuspectData;				/*!< Indicates whether a suspect streamState is allowed for items. */
	RsslBuffer					applicationId;					/*!< The DACS Application ID. */
	RsslBuffer					applicationName;				/*!< The ApplicationName. */
	RsslUInt32					serverCount;					/*!< The number of servers present in the serverList member. */
	RsslRDMServerInfo 			*serverList;					/*!< An array describing the list of servers present in the Connection Config. */
	RsslUInt					numStandbyServers;				/*!< The number of Standby Servers the Consumer is recommended to connect to, if serverList is provided. */
	RsslBuffer					position;						/*!< The DACS Position. */
	RsslUInt					providePermissionExpressions;	/*!< Indicates whether the Provider will send permission expression information with its responses. */
	RsslUInt					providePermissionProfile;		/*!< Indicates whether the permission profile is provided. */
	RsslUInt					singleOpen;						/*!< Indicates whether recovery of items will be handled on the Consumer's behalf. */
	RsslUInt					supportEnhancedSymbolList;		/*!< Indicates support for Enhanced Symbol List features. Populated by RDMEnhancedSymbolListSupportFlags. */
	RsslUInt					supportBatchRequests;			/*!< Indicates whether the Provider supports Batch Requests, Batch Reissues, and/or Batch Closes. Populated by RDMLoginBatchSupportFlags. */
	RsslUInt					supportProviderDictionaryDownload;	/*!< Indicates whether the Provider supports Dictionary Download. */
	RsslUInt					supportOMMPost;					/*!< Indicates whether the Provider supports OMM Posting. */
	RsslUInt					supportOptimizedPauseResume;	/*!< Indicates whether the Provider supports Optimized Pause & Resume. */
	RsslUInt					supportStandby;					/*!< Indicates whether the Provider may be used for Warm Standby. */
	RsslUInt					supportViewRequests;			/*!< Indicates whether the Provider supports Requests with Dynamic View information. */
	RsslUInt					sequenceRetryInterval;			/*!< For ADS multicast configuration of a watchlist-enabled ETA Reactor. Configures the time (in seconds) the Reactor will delay recovery of items when a gap is detected. */
	RsslUInt					updateBufferLimit;				/*!< For ADS multicast configuration of a watchlist-enabled ETA Reactor. Configures the number of multicast messages the Reactor will buffer on each stream for reordering againts the item's refresh. */
	RsslUInt					sequenceNumberRecovery;			/*!< For ADS multicast configuration of a watchlist-enabled ETA Reactor. Configures whether the Reactor recovers from a gap detected in an item stream. */
	RsslUInt					authenticationTTReissue;		/*!< For UserAuthn Authentication, this indicates the time(in epoch) that the token expires on */
	RsslBuffer					authenticationExtendedResp;		/*!< For UserAuthn Authentication, this is extra data that may need to be used by the client application from the token generator */
	RsslUInt					authenticationErrorCode;		/*!< For UserAuthn Authentication, this is any error information from the token generator */
	RsslBuffer					authenticationErrorText;		/*!< For UserAuthn Authentication, this is the text associated with the authentication error code */
	RsslUInt					supportStandbyMode;				/*!< Indicates whether the Warm Standby modes supported by Provider. Populated by RsslRDMLoginWarmStandbyModes. */
} RsslRDMLoginRefresh;

/**
 * @brief Clears an RsslRDMLoginRefresh.
 * @see RsslRDMLoginRefresh
 */
RTR_C_INLINE void rsslClearRDMLoginRefresh(RsslRDMLoginRefresh *pRefresh)
{
	rsslClearRDMMsgBase(&pRefresh->rdmMsgBase);
	pRefresh->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pRefresh->rdmMsgBase.rdmMsgType = RDM_LG_MT_REFRESH;
	rsslClearState(&pRefresh->state);
	pRefresh->state.streamState = RSSL_STREAM_OPEN;
	pRefresh->state.dataState = RSSL_DATA_OK;
	pRefresh->state.code = RSSL_SC_NONE;
	pRefresh->flags = RDM_LG_RFF_NONE;
	rsslClearBuffer(&pRefresh->userName);
	pRefresh->userNameType = RDM_LOGIN_USER_NAME;
	rsslClearBuffer(&pRefresh->applicationId);
	rsslClearBuffer(&pRefresh->applicationName);
	rsslClearBuffer(&pRefresh->position);
	rsslClearBuffer(&pRefresh->authenticationExtendedResp);
	rsslClearBuffer(&pRefresh->authenticationErrorText);
	pRefresh->authenticationTTReissue = 0;
	pRefresh->authenticationErrorCode = 0;
	pRefresh->providePermissionProfile = 1;
	pRefresh->providePermissionExpressions = 1;
	pRefresh->singleOpen = 1;
	pRefresh->supportEnhancedSymbolList = RDM_SYMBOL_LIST_SUPPORT_NAMES_ONLY;
	pRefresh->allowSuspectData = 1;
	pRefresh->supportOMMPost = 0;
	pRefresh->supportStandby = 0;
	pRefresh->supportBatchRequests = 0;
	pRefresh->supportProviderDictionaryDownload = 0;
	pRefresh->supportViewRequests = 0;
	pRefresh->supportOptimizedPauseResume = 0;
	pRefresh->numStandbyServers = 0;
	pRefresh->serverCount = 0;
	pRefresh->serverList = NULL;
	pRefresh->sequenceNumber = 0;
	pRefresh->sequenceRetryInterval = 5;
	pRefresh->updateBufferLimit = 100;
	pRefresh->sequenceNumberRecovery = 1;
	pRefresh->supportStandbyMode = 0;
}

/**
 * @brief The RDM Login Close.  Used by an OMM Consumer or OMM Noninteractive Provider to close a login.
 * @see RsslRDMMsgBase, RsslRDMLoginMsg, rsslClearRDMLoginClose
 */
typedef struct {
	RsslRDMMsgBase rdmMsgBase;	/*!< The Base RDM Message. */
} RsslRDMLoginClose;

/**
 * @brief Clears an RsslRDMLoginClose.
 * @see RsslRDMLoginClose
 */
RTR_C_INLINE void rsslClearRDMLoginClose(RsslRDMLoginClose *pClose)
{
	rsslClearRDMMsgBase(&pClose->rdmMsgBase);
	pClose->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pClose->rdmMsgBase.rdmMsgType = RDM_LG_MT_CLOSE;
}


/**
 * @brief The RDM Login Status Flags
 * @see RsslRDMLoginStatus
 */
typedef enum {
	RDM_LG_STF_NONE					= 0x00,	/*!< (0x00) No flags set. */
	RDM_LG_STF_HAS_STATE			= 0x01, /*!< (0x01) Indicates presence of the state member. */
	RDM_LG_STF_HAS_USERNAME			= 0x02,	/*!< (0x02) Indicates presence of the userName member. */
	RDM_LG_STF_HAS_USERNAME_TYPE	= 0x04,	/*!< (0x04) Indicates presence of the userNameType member. */
	RDM_LG_STF_HAS_AUTHN_ERROR_CODE = 0x08, /*!< (0x08) Indicates presence of the Authentication error code member */
	RDM_LG_STF_HAS_AUTHN_ERROR_TEXT = 0x10	/*!< (0x10) Indicates presence of the Authentication error text member */
} RsslRDMLoginStatusFlags;

/**
 * @brief The RDM Login Status. Used by an OMM Provider to indicate changes to the Login stream.
 * @see RsslRDMMsgBase, RsslRDMMsg, rsslClearRDMLoginStatus
 */
typedef struct {
	RsslRDMMsgBase	rdmMsgBase;		/*!< The base RDM Message. */
	RsslUInt32 		flags;			/*!< The RDM Login Status flags. Populated by RsslRDMLoginStatusFlags. */
	RsslState		state;			/*!< The current state of the login stream. */
	RsslBuffer		userName;		/*!< The userName that was used when sending the Login Request. */
	RsslUInt8		userNameType;	/*!< The type of the userName that was used with the Loginn Request.Populated by RDMLoginUserIdTypes */
	RsslUInt		authenticationErrorCode;	/*!< For UserAuthn Authentication, this is any error information from the token generator */
	RsslBuffer		authenticationErrorText;	/*!< For UserAuthn Authentication, this is the text associated with the authentication error code */

} RsslRDMLoginStatus;

/**
* @brief Clears an RsslRDMLoginStatus.
* @see RsslRDMLoginStatus
*/
RTR_C_INLINE void rsslClearRDMLoginStatus(RsslRDMLoginStatus *pStatus)
{
	rsslClearRDMMsgBase(&pStatus->rdmMsgBase);
	pStatus->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pStatus->rdmMsgBase.rdmMsgType = RDM_LG_MT_STATUS;
	pStatus->flags = RDM_LG_STF_NONE;
	rsslClearState(&pStatus->state);
	pStatus->state.streamState = RSSL_STREAM_OPEN;
	pStatus->state.dataState = RSSL_DATA_OK;
	pStatus->state.code = RSSL_SC_NONE;
	rsslClearBuffer(&pStatus->userName);
	pStatus->userNameType = RDM_LOGIN_USER_NAME;
	pStatus->authenticationErrorCode = 0;
	rsslClearBuffer(&pStatus->authenticationErrorText);
}

/**
 * @brief The RDM Login Consumer Connection Status Flags
 * @see RsslRDMLoginConsumerConnectionStatus, RsslRDMLoginWarmStandbyInfo
 */
typedef enum {
	RDM_LG_CCSF_NONE = 0x00,					/*!< (0x00) No flags set */
	RDM_LG_CCSF_HAS_WARM_STANDBY_INFO = 0x01	/*!< (0x01) Indicates presence of Warm Standby information. */
} RsslRDMLoginConsumerConnectionStatusFlags;

/**
 * @brief The RDM Login Consumer Connection Status.  Used by an OMM Consumer to send Warm Standby information.
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMLoginWarmStandbyInfo, rsslClearRDMLoginConsumerConnectionStatus
 */
typedef struct {
	RsslRDMMsgBase					rdmMsgBase;			/*!< The Base RDM Message. */
	RsslUInt32						flags;				/*!< The Consumer Connection Status flags.  Populated by RsslRDMLoginConsumerConnectionStatusFlags. */
	RsslRDMLoginWarmStandbyInfo		warmStandbyInfo;	/*!< Warm Standby Information.  Presence indicated by RDM_LG_CCSF_HAS_WARM_STANDBY_INFO */
} RsslRDMLoginConsumerConnectionStatus;

/**
 * @brief Clears an RsslRDMLoginConsumerConnectionStatus.
 * @see RsslRDMLoginConsumerConnectionStatus
 */
RTR_C_INLINE void rsslClearRDMLoginConsumerConnectionStatus(RsslRDMLoginConsumerConnectionStatus *pConsumerStatus)
{
	rsslClearRDMMsgBase(&pConsumerStatus->rdmMsgBase);
	pConsumerStatus->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pConsumerStatus->rdmMsgBase.rdmMsgType = RDM_LG_MT_CONSUMER_CONNECTION_STATUS;
	pConsumerStatus->flags = RDM_LG_CCSF_NONE;
	rsslClearRDMLoginWarmStandbyInfo(&pConsumerStatus->warmStandbyInfo);
}



/**
* @brief The RDM Login RTT Flags
* @see RsslRDMLoginRTT
*/
typedef enum {
	RDM_LG_RTT_NONE = 0x00,	/*!< (0x00) No flags set. */
	RDM_LG_RTT_HAS_TCP_RETRANS = 0x01, /*!< (0x01) Indicates presence of the TCP Retransmission count member. */
	RDM_LG_RTT_HAS_LATENCY = 0x02,	/*!< (0x02) Indicates presence of the last latency member. */
} RsslRDMLoginRTTFlags;

/**
* @brief The RDM Login RTT. Used by an OMM Provider and OMM Consumer to test round trip latency for the connection..
* @see RsslRDMMsgBase, RsslRDMMsg, rsslClearRDMLoginStatus
*/
typedef struct {
	RsslRDMMsgBase	rdmMsgBase;		/*!< The base RDM Message. */
	RsslUInt32 		flags;			/*!< The RDM Login RTT flags. Populated by RsslRDMLoginRTTFlags. */
	RsslUInt		ticks;			/*!< Required Tick count, provided by rsslGetTicks. */
	RsslUInt		lastLatency;	/*!< Optional last latency measurement from the provider. */
	RsslUInt		tcpRetrans;		/*!< Optional TCP Retransmit count for the current connection. */
} RsslRDMLoginRTT;

/**
* @brief Clears an RsslRDMLoginRTT.
* @see RsslRDMLoginRTT
*/
RTR_C_INLINE void rsslClearRDMLoginRTT(RsslRDMLoginRTT *pRTT)
{
	rsslClearRDMMsgBase(&pRTT->rdmMsgBase);
	pRTT->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	pRTT->rdmMsgBase.rdmMsgType = RDM_LG_MT_RTT;
	pRTT->flags = RDM_LG_RTT_NONE;
	pRTT->lastLatency = 0;
	pRTT->ticks = 0;
	pRTT->tcpRetrans = 0;
}

/**
 * @brief The RDM Login Msg.  The Login Message encoder and decoder functions expect this type.
 * It is a group of the classes of message that the Login domain supports.  Any 
 * specific message class may be cast to an RsslRDMLoginMsg, and an RsslRDMLoginMsg may be cast 
 * to any specific message class.  The RsslRDMMsgBase contains members common to each class
 * that may be used to identify the class of message.
 * @see RsslRDMLoginMsgType, RsslRDMMsgBase, RsslRDMLoginRequest, RsslRDMLoginClose, RsslRDMLoginConsumerConnectionStatus, RsslRDMLoginRefresh, RsslRDMLoginStatus, rsslClearRDMLoginMsg
 */
typedef union
{
	RsslRDMMsgBase 							rdmMsgBase;					/*!< The base RDM Message. */
	RsslRDMLoginRequest						request;					/*!< Login Request */
	RsslRDMLoginClose						close;						/*!< Login Close */
	RsslRDMLoginConsumerConnectionStatus	consumerConnectionStatus;	/*!< Login Consumer Connection Status */
	RsslRDMLoginRefresh						refresh;					/*!< Login Refresh */
	RsslRDMLoginStatus						status;						/*!< Login Status */
	RsslRDMLoginRTT							RTT;						/*!< Login RTT */
} RsslRDMLoginMsg;

/**
 * @brief Clears an RsslRDMLoginMsg.
 * @see RsslRDMLoginMsg
 */
RTR_C_INLINE void rsslClearRDMLoginMsg(RsslRDMLoginMsg *pLoginMsg)
{
	memset(pLoginMsg, 0, sizeof(RsslRDMLoginMsg));
	pLoginMsg->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
}

/**
 * @brief Encodes an RsslRDMLoginMsg.
 * @param pEncodeIter The Encode Iterator
 * @param pLoginMsg The RDM Login Message to Encode
 * @param pBytesWritten Returns the total number of bytes used to encode the message.
 * @see RsslRDMLoginMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMLoginMsg(RsslEncodeIterator *pEncodeIter, RsslRDMLoginMsg *pLoginMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError);


/**
 * @brief Decodes an RsslRDMLoginMsg.
 * @param pEncodeIter The Decode Iterator
 * @param pLoginMsg The RDM Login Message to be populated
 * @param pMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the decoded message.
 * @param pError An Error Structure, which will be populated if the decoding fails.
 * @return RSSL_RET_SUCCESS, if the message was succesfully decoded and correctly followed the RDM.
 * @return RSSL_RET_FAILURE, if the message was not successfully decoded or did  follow the RDM.  Information about the error will be stored in pError.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginMsg
 */
RSSL_VA_API RsslRet rsslDecodeRDMLoginMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMLoginMsg *pLoginMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError);

/**
 *	@addtogroup VARDMLoginHelper
 *	@{
 */

/**
 * @brief Fully copies an RsslRDMLoginMsg.
 * @param pNewMsg The resulting copy of the RDM Login Message
 * @param pOldMsg The RDM Login Message to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginMsg
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginMsg(RsslRDMLoginMsg *pNewMsg, RsslRDMLoginMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMLoginRequest.
 * @param pNewRequest The resulting copy of the RDM Login Request
 * @param pOldRequest The RDM Login Request to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginRequest
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginRequest(RsslRDMLoginRequest *pNewRequest, RsslRDMLoginRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMLoginRefresh.
 * @param pNewRefresh The resulting copy of the RDM Login Refresh
 * @param pOldRefresh The RDM Login Refresh to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginRefresh
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginRefresh(RsslRDMLoginRefresh *pNewRefresh, RsslRDMLoginRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer);


/**
 * @brief Fully copies an RsslRDMLoginClose.
 * @param pNewClose The resulting copy of the RDM Login Close
 * @param pOldClose The RDM Login Close to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginClose
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginClose(RsslRDMLoginClose *pNewClose, RsslRDMLoginClose *pOldClose, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMLoginConsumerConnectionStatus.
 * @param pNewConnectionStatus The resulting copy of the RDM Login ConsumerConnectionStatus
 * @param pOldConnectionStatus The RDM Login ConsumerConnectionStatus to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginConsumerConnectionStatus
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginConsumerConnectionStatus(RsslRDMLoginConsumerConnectionStatus *pNewConnectionStatus, RsslRDMLoginConsumerConnectionStatus *pOldConnectionStatus, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMLoginStatus.
 * @param pNewStatus The resulting copy of the RDM Login Status
 * @param pOldStatus The RDM Login Status to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMLoginStatus
 */
RSSL_VA_API RsslRet rsslCopyRDMLoginStatus(RsslRDMLoginStatus *pNewStatus, RsslRDMLoginStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer);

/**
* @brief Fully copies an RsslRDMLoginRTT.
* @param pNewStatus The resulting copy of the RDM Login RTT
* @param pOldStatus The RDM Login RTT to be copied
* @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
* @return RSSL_RET_SUCCESS, if the message was succesfully copied.
* @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data.
* @see RsslRDMLoginStatus
*/
RSSL_VA_API RsslRet rsslCopyRDMLoginRTT(RsslRDMLoginRTT *pNewRTT, RsslRDMLoginRTT *pOldRTT, RsslBuffer *pNewMemoryBuffer);

/**
 *	@}
 */

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
