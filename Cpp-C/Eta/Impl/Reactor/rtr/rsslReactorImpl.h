/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2022 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_REACTOR_IMPL_H
#define _RTR_RSSL_REACTOR_IMPL_H

#include "rtr/rsslNotifier.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslWatchlist.h"
#include "rtr/rsslReactorEventQueue.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslVAUtils.h"

#include "rtr/rsslQueue.h"
#include "rtr/rsslBindThread.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslReactorUtils.h"
#include "rtr/tunnelManager.h"
#include "rtr/rsslRestClientImpl.h"
#include "rtr/rtratomic.h"
#include "rtr/rsslReactorTokenMgntImpl.h"
#include "rtr/rsslJsonConverter.h"
#include "rtr/rsslHashTable.h"
#include "rtr/wlItem.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define RSSL_REACTOR_DEFAULT_URL_LENGHT 2084;
#define RSSL_REACTOR_RDM_SERVICE_INFO_INIT_BUFFER_SIZE 9216
#define RSSL_REACTOR_WSB_STARTING_SERVER_INDEX -1 /* indicates the RsslReactorWarmStandbyGroup.startingActiveServer */
#define RSSL_REACTOR_WORKER_ERROR_INFO_MAX_POOL_SIZE 16

#ifdef _WIN32
#ifdef _WIN64
#define RSSL_REACTOR_SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#define RSSL_REACTOR_POINTER_PRINT_TYPE	"0x%p"
#else
#define RSSL_REACTOR_SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#define RSSL_REACTOR_POINTER_PRINT_TYPE	"0x%p"
#endif
#else
#define RSSL_REACTOR_SOCKET_PRINT_TYPE "%d"  /* Linux */
#define RSSL_REACTOR_POINTER_PRINT_TYPE	"%p"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RsslReactorImpl RsslReactorImpl;

typedef struct _RsslReactorWarmStandByHandlerImpl RsslReactorWarmStandByHandlerImpl;

typedef struct _RsslReactorChannelImpl RsslReactorChannelImpl;

typedef enum
{
	RSSL_RC_CHST_INIT = 0,
	RSSL_RC_CHST_LOGGED_IN = 1,
	RSSL_RC_CHST_HAVE_DIRECTORY = 2,
	RSSL_RC_CHST_HAVE_RWFFLD = 3,
	RSSL_RC_CHST_HAVE_RWFENUM = 4,
	RSSL_RC_CHST_READY = 5,
	RSSL_RC_CHST_RECONNECTING = 6,
	RSSL_RC_CHST_INIT_FAIL = 7
} RsslReactorChannelSetupState;

/* RsslReactorChannelInfoImplState
* - Represents states for token management and requesting service discovery */
typedef enum
{
	RSSL_RC_CHINFO_IMPL_ST_STOP_REQUESTING = -6,
	RSSL_RC_CHINFO_IMPL_ST_INVALID_CONNECTION_TYPE = -5,
	RSSL_RC_CHINFO_IMPL_ST_MEM_ALLOCATION_FAILURE = -4,
	RSSL_RC_CHINFO_IMPL_ST_PARSE_RESP_FAILURE = -3,
	RSSL_RC_CHINFO_IMPL_ST_REQUEST_FAILURE = -2,
	RSSL_RC_CHINFO_IMPL_ST_BUFFER_TOO_SMALL = -1,
	RSSL_RC_CHINFO_IMPL_ST_INIT = 0,
	RSSL_RC_CHINFO_IMPL_ST_REQ_AUTH_TOKEN = 1,
	RSSL_RC_CHINFO_IMPL_ST_RECEIVED_AUTH_TOKEN = 2,
	RSSL_RC_CHINFO_IMPL_ST_QUERYING_SERVICE_DISOVERY = 3,
	RSSL_RC_CHINFO_IMPL_ST_ASSIGNED_HOST_PORT = 4,
	RSSL_RC_CHINFO_IMPL_ST_REQ_LOGIN_MSG = 5,
	RSSL_RC_CHINFO_IMPL_ST_RECEIVED_LOGIN_MSG = 6,
	RSSL_RC_CHINFO_IMPL_ST_DONE = 7
} RsslReactorChannelInfoImplState;

/* RsslReactorPackedBufferImpl
*  - Keeps track the length of the packed buffer */
typedef struct
{
	RsslHashLink hashLink;
	RsslUInt32 totalSize;
	RsslUInt32 remainingSize;

} RsslReactorPackedBufferImpl;

RTR_C_INLINE void rsslClearReactorPackedBufferImpl(RsslReactorPackedBufferImpl* pReactorPackedBufferImpl)
{
	memset(pReactorPackedBufferImpl, 0, sizeof(RsslReactorPackedBufferImpl));
}

/* RsslReactorConnectInfoImpl
* - Handles a channel information including token management */
typedef struct
{
	RsslReactorConnectInfo base;

	RsslInt		lastTokenUpdatedTime; /* Keeps track the last updated time in ms of token information from the token session */

	RsslReactorChannelInfoImplState	reactorChannelInfoImplState; /* Keeping track the state of this session */
	RsslReactorTokenMgntEventType	reactorTokenMgntEventType; /* Specify an event type for sending to the Reactor */

	RsslReactorDiscoveryTransportProtocol transportProtocol; /* This is used to keep the transport protocol for requesting service discovery */
	RsslBool	userSetConnectionInfo; /* True when user specifies connectionInfo.unified fields address and serviceName. See RsslConnectionInfo */
	RsslUInt32	reconnectEndpointAttemptCount; /* Attempts counter of connecting to the channel using this endpoint */
	RsslBool startedSessionManagement; /* Indicates that this conenctionInfo has started session management */
} RsslReactorConnectInfoImpl;

RTR_C_INLINE void rsslClearReactorConnectInfoImpl(RsslReactorConnectInfoImpl* pReactorConnectInfoImpl)
{
	memset(pReactorConnectInfoImpl, 0, sizeof(RsslReactorConnectInfoImpl));
	rsslClearReactorConnectInfo(&pReactorConnectInfoImpl->base);
}

typedef enum
{
	RSSL_RC_DEBUG_INFO_UNKNOWN = 0x00, /* Unknown */
	RSSL_RC_DEBUG_INFO_ACCEPT_CHANNEL= 0x01, /* Server acceps a channel */
	RSSL_RC_DEBUG_INFO_CHANNEL_UP = 0x02, /* Channel Up */
	RSSL_RC_DEBUG_INFO_CHANNEL_DOWN = 0x04, /* Channel Down */
	RSSL_RC_DEBUG_INFO_CHANNEL_SESSION_STARTUP_DONE = 0x08, /* Chennel start up session executed */
	RSSL_RC_DEBUG_INFO_CHANNEL_CONNECTING_PERFORMED = 0x10, /* Channel connection is go on*/
	RSSL_RC_DEBUG_INFO_CHANNEL_CLOSE = 0x20, /* Channel close. */
	RSSL_RC_DEBUG_INFO_CHANNEL_RECONNECTING = 0x40, /* Channel reconnecting. */
	RSSL_RC_DEBUG_INFO_RECEIVE_TUNNEL_REQUEST = 0x80, /* Receive tunnel stream request */
	RSSL_RC_DEBUG_INFO_ACCEPT_TUNNEL_REQUEST = 0x100, /* Accept tunnel stream request */
	RSSL_RC_DEBUG_INFO_SUBMIT_TUNNEL_STREAM_RESP = 0x200, /* Submit a response to open tunnel stream request */
	RSSL_RC_DEBUG_INFO_TUNNEL_STREAM_ESTABLISHED = 0x400, /* Accept tunnel stream request */
	RSSL_RC_DEBUG_INFO_HANDLE_TUNNEL_CLOSE = 0x800, /* Receives close tunnel stream requst. */
	RSSL_RC_DEBUG_INFO_REJECT_TUNNEL_REQUEST = 0x2000, /* Receive tunnel stream request */
}RsslReactorChannelDebugInfoState;

typedef enum
{
	RSSL_RWSB_STATE_INIT                              = 0x00,
	RSSL_RWSB_STATE_RECEIVED_PRIMARY_LOGIN_RESP       = 0x01,
	RSSL_RWSB_STATE_RECEIVED_PRIMARY_DIRECTORY_RESP   = 0x02,
	RSSL_RWSB_STATE_RECEIVED_SECONDARY_DIRECTORY_RESP = 0x04,
	RSSL_RWSB_STATE_CONNECTING_TO_A_STARTING_SERVER   = 0x08,
	RSSL_RWSB_STATE_MOVE_TO_CHANNEL_LIST              = 0x10,
	RSSL_RWSB_STATE_CLOSING_STANDBY_CHANNELS          = 0x20,
	RSSL_RWSB_STATE_MOVE_TO_NEXT_WSB_GROUP            = 0x40,
	RSSL_RWSB_STATE_RECEIVED_PRIMARY_FIELD_DICT_RESP  = 0x80,
	RSSL_RWSB_STATE_RECEIVED_PRIMARY_ENUM_DICT_RESP   = 0x100,
	RSSL_RWSB_STATE_CLOSING                           = 0x200,
	RSSL_RWSB_STATE_INACTIVE                          = 0x400,
	RSSL_RWSB_STATE_MOVED_TO_CHANNEL_LIST			  = 0x800

}RsslReactorWarmStandByHandlerState;

typedef enum
{
	RSSL_RWSB_STATE_CHANNEL_INIT = 0,
	RSSL_RWSB_STATE_CHANNEL_DOWN = 1,
	RSSL_RWSB_STATE_CHANNEL_DOWN_RECONNECTING = 2,
	RSSL_RWSB_STATE_CHANNEL_UP = 3,
	RSSL_RWSB_STATE_CHANNEL_READY = 4,
}RsslReactorWarmStandByHandlerChannelState;

/* RsslReactorWSRecoveryMsgInfo
- This is used to generate status message when a channel is active
*/
typedef struct
{
	RsslQueueLink		queueLink;

	/* For populating RsslStatusMsg */
	RsslUInt8			domainType;
	RsslContainerType	containerType;
	RsslInt32			streamId;
	RsslMsgKey			msgKey;
	RsslState			rsslState;
	RsslUInt16			flags;

	/* From the RsslStreamInfo structure. */
	void* pUserSpec;
	RsslBuffer serviceName;

} RsslReactorWSRecoveryMsgInfo;

RTR_C_INLINE void rsslClearReactorWSRecoveryMsgInfo(RsslReactorWSRecoveryMsgInfo* pWSRecoveryMsgInfo)
{
	memset(pWSRecoveryMsgInfo, 0, sizeof(RsslReactorWSRecoveryMsgInfo));
}

/* RsslReactorDictionaryVersion
* - Represents Refinitiv Data Dictionary version
*/
typedef struct
{
	RsslUInt32 major;
	RsslUInt32 minor;

} RsslReactorDictionaryVersion;

RTR_C_INLINE void rsslClearReactorDictionaryVersion(RsslReactorDictionaryVersion* pDictionaryVersion)
{
	memset(pDictionaryVersion, 0, sizeof(RsslReactorDictionaryVersion));
}

/* RsslReactorSubmitMsgOptionsImpl
 * - Represents submit messages to fanout later across all connetions. */
typedef struct
{
	RsslReactorSubmitMsgOptions base;
	RsslQueueLink			    submitMsgLink; /* Keeps in the RsslQueue of RsslWarmStandByHandlerImpl */
	RsslInt64					submitTimeStamp; /* This is used to check whether this message has been submited. */
	char						*pMemoryLocation; /* Keeps memory location to free up later */
	RsslUInt32					allocationLength; /* Keeps the allocated buffer length. */
	char						*pReqMsgExtendedHeaders; /* Keeps memory location for RequestMsg.extendedHeader */
	RsslBool					copyFromMsgCopy;

}RsslReactorSubmitMsgOptionsImpl;

RTR_C_INLINE void rsslClearReactorSubmitMsgOptionsImpl(RsslReactorSubmitMsgOptionsImpl* pReactorSubmitMsgOptionsImpl)
{
	rsslClearReactorSubmitMsgOptions(&pReactorSubmitMsgOptionsImpl->base);
	memset(&pReactorSubmitMsgOptionsImpl->submitMsgLink, 0, sizeof(RsslQueueLink));
	pReactorSubmitMsgOptionsImpl->submitTimeStamp = 0;
	pReactorSubmitMsgOptionsImpl->pMemoryLocation = NULL;
	pReactorSubmitMsgOptionsImpl->allocationLength = 0;
	pReactorSubmitMsgOptionsImpl->pReqMsgExtendedHeaders = NULL;
	pReactorSubmitMsgOptionsImpl->copyFromMsgCopy = RSSL_FALSE;
}

RTR_C_INLINE void _reactorFreeSubmitMsgOptions(RsslReactorSubmitMsgOptionsImpl* pSubmitMsgOptionImpl)
{
	if (pSubmitMsgOptionImpl)
	{
		RsslReactorSubmitMsgOptions* pSubmitMsgOptions = (RsslReactorSubmitMsgOptions*)pSubmitMsgOptionImpl;

		if (pSubmitMsgOptions->pServiceName && pSubmitMsgOptions->pServiceName->data)
		{
			free(pSubmitMsgOptions->pServiceName->data);
			free(pSubmitMsgOptions->pServiceName);
		}

		if (pSubmitMsgOptionImpl->pMemoryLocation)
		{
			free(pSubmitMsgOptionImpl->pMemoryLocation);
		}

		if (pSubmitMsgOptionImpl->pReqMsgExtendedHeaders)
		{
			free(pSubmitMsgOptionImpl->pReqMsgExtendedHeaders);
		}

		if (pSubmitMsgOptions->pRsslMsg)
		{
			if (pSubmitMsgOptionImpl->copyFromMsgCopy)
			{
				rsslReleaseCopiedMsg(pSubmitMsgOptions->pRsslMsg);
			}
			else
			{
				free(pSubmitMsgOptions->pRsslMsg);
			}
		}

		free(pSubmitMsgOptionImpl);
	}
}

typedef struct
{
	RsslReactorWarmStandbyChannelInfo base;
	RsslUInt32  maxNumberOfSocket;     /*!< Keeps track memory allocation size of socketIdList and oldSocketIdList */
} RsslReatorWarmStandbyChInfoImpl;

RTR_C_INLINE void rsslClearReactorWarmStandbyChInfoImpl(RsslReatorWarmStandbyChInfoImpl* pReactorWarmStandbyChInfoImpl)
{
	memset(pReactorWarmStandbyChInfoImpl, 0, sizeof(RsslReatorWarmStandbyChInfoImpl));
}

typedef struct
{
	RsslReactorConnectInfoImpl			reactorConnectInfoImpl;
	RsslReactorPerServiceBasedOptions  perServiceBasedOptions;
	RsslBool							isActiveChannelConfig;

} RsslReactorWarmStandbyServerInfoImpl;

RTR_C_INLINE void rsslClearReactorWarmStandbyServerInfoImpl(RsslReactorWarmStandbyServerInfoImpl* pReactorWarmStandbyServerInfoImpl)
{
	memset(&pReactorWarmStandbyServerInfoImpl->reactorConnectInfoImpl, 0, sizeof(RsslReactorWarmStandbyServerInfoImpl));
	rsslClearReactorConnectInfo(&pReactorWarmStandbyServerInfoImpl->reactorConnectInfoImpl.base);
	pReactorWarmStandbyServerInfoImpl->isActiveChannelConfig = RSSL_TRUE;
}

typedef struct
{
	RsslHashLink					  hashLink;		 /* Link for RsslReactorWarmStandByGroupImpl.pActiveServiceConfig by service name. */
	RsslQueueLink					  queueLink;	/* Queue link for RsslReactorWarmStandByGroupImpl.pActiveServiceConfigList */
	RsslBool						  isStartingServerConfig; /* This is used to indicate that this service config belongs to the starting server configuration. */
	RsslUInt32						  standByServerListIndex; /* Keeps the index of the standby server list in a RsslReactorWarmStandByGroup. */
} RsslReactorWSBServiceConfigImpl;

RTR_C_INLINE void rsslClearReactorWSByServiceConfigImpl(RsslReactorWSBServiceConfigImpl* pReactorWarmStandbyServiceConfigImpl)
{
	rsslHashLinkInit(&pReactorWarmStandbyServiceConfigImpl->hashLink);
	rsslInitQueueLink(&pReactorWarmStandbyServiceConfigImpl->queueLink);
	pReactorWarmStandbyServiceConfigImpl->isStartingServerConfig = RSSL_FALSE;
	pReactorWarmStandbyServiceConfigImpl->standByServerListIndex = 0;
}

typedef struct
{
	RsslQueueLink					  queueLink; /* For _serviceList */
	RsslQueueLink					  updateQLink; /* For _updateServiceList */
	RsslHashLink					  hashLink;		 /* Link for RsslReactorWarmStandByGroupImpl _perServiceById. */
	RsslReactorChannel				  *pReactorChannel;		 /* Keeps the channel that provides this service as the active mode. */
	RsslUInt						  serviceID;
	RsslRDMServiceInfo				  rdmServiceInfo;
	RsslRDMServiceState				  rdmServiceState;
	char							  *pMemoryLocation; /* Keeps memory location to free up later */
	RsslUInt32						  allocationLength; /* Keeps the allocated buffer length. */
	RsslBuffer						  directoryMemBuffer; /* Keeps track to the current location of memory */
	RsslUInt32						  updateServiceFilter; /* This is used as indication to create source directory response for a service using the RsslRDMServiceFlags flags. */
	RsslUInt32						  serviceAction; /* Uses RsslMapEntryActions to indicate an action for this service. */

} RsslReactorWarmStandbyServiceImpl;

RTR_C_INLINE void rsslClearReactorWarmStandbyServiceImpl(RsslReactorWarmStandbyServiceImpl* pReactorWarmStandbyServiceImpl)
{
	rsslInitQueueLink(&pReactorWarmStandbyServiceImpl->queueLink);
	rsslInitQueueLink(&pReactorWarmStandbyServiceImpl->updateQLink);
	rsslHashLinkInit(&pReactorWarmStandbyServiceImpl->hashLink);
	pReactorWarmStandbyServiceImpl->pReactorChannel = NULL;
	pReactorWarmStandbyServiceImpl->serviceID = 0;
	rsslClearRDMServiceInfo(&pReactorWarmStandbyServiceImpl->rdmServiceInfo);
	rsslClearRDMServiceState(&pReactorWarmStandbyServiceImpl->rdmServiceState);
	pReactorWarmStandbyServiceImpl->pMemoryLocation = NULL;
	pReactorWarmStandbyServiceImpl->allocationLength = 0;
	rsslClearBuffer(&pReactorWarmStandbyServiceImpl->directoryMemBuffer);
	pReactorWarmStandbyServiceImpl->updateServiceFilter = 0;
	pReactorWarmStandbyServiceImpl->serviceAction = 0;
}

typedef struct
{
	/* From RsslReactorWarmStandbyGroup*/
	RsslReactorWarmStandbyServerInfoImpl      startingActiveServer; /*!< The active server to which client should to connect.
														   Reactor chooses a server from the standByServerList instead if this parameter is not configured.*/
	RsslReactorWarmStandbyServerInfoImpl*     standbyServerList;    /*!< A list of standby servers. */
	RsslUInt32                            standbyServerCount;   /*!< The number of standby servers. */
	RsslBool                              downloadConnectionConfig;  /* Specifies whether to download connection configurations from a provider by
														   setting the DownloadConnectionConfig element with the login request. */
	RsslReactorWarmStandbyMode        warmStandbyMode; /*!< Specifies a warm standby mode. */
	/* End */

	RsslHashTable               _perServiceById; /* This hash table provides mapping between service ID and ReactorChannel. */
	RsslQueue	                _serviceList; /* Keeps a list of full services which belong to this group. */
	RsslQueue	                _updateServiceList; /* Keeps a list of update service list which belong to this group.  */
	RsslHashTable               *pActiveServiceConfig; /* This is used to select an active service from a server. */
	RsslQueue	                *pActiveServiceConfigList; /* Keeps a list of active service configuration which belongs to this group. */
	RsslBool                  sendQueueReqForAll; /* Keeps track whether all reactor channels have been created for this warmstandby group. */
	RsslUInt32                sendReqQueueCount; /* Keeps track the number of channels that the requests has been sent. */
	RsslInt32                 currentStartingServerIndex; /* Keeps track of the current starting server index. RSSL_REACTOR_WSB_STARTING_SERVER_INDEX for RsslReactorWarmStandbyGroup.startingActiveServer 
												  and non-negative values represent an index of RsslReactorWarmStandbyGroup.standbyServerList */
	RsslUInt32                numOfClosingStandbyServers; /* Keeps track of number of standby servers being closed. */
	RsslInt32				  downloadConfigActiveServer; /* Select an active server from downloadConnectionConfig */
	RsslInt32				  streamId; /* Stream ID for the source directory. */
	RsslBool				  isStartingServerIsDown; /* Indicate whether the starting server is down. */
}RsslReactorWarmStandbyGroupImpl;

RTR_C_INLINE void rsslClearReactorWarmStandByGroupImpl(RsslReactorWarmStandbyGroupImpl* pReactorWarmStandByGroupImpl)
{
	rsslClearReactorWarmStandbyServerInfoImpl(&pReactorWarmStandByGroupImpl->startingActiveServer);
	pReactorWarmStandByGroupImpl->standbyServerList = NULL;
	pReactorWarmStandByGroupImpl->standbyServerCount = 0;
	pReactorWarmStandByGroupImpl->downloadConnectionConfig = RSSL_FALSE;
	pReactorWarmStandByGroupImpl->warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;
	memset(&pReactorWarmStandByGroupImpl->_perServiceById, 0, sizeof(RsslHashTable));
	rsslInitQueue(&pReactorWarmStandByGroupImpl->_serviceList);
	rsslInitQueue(&pReactorWarmStandByGroupImpl->_updateServiceList);
	pReactorWarmStandByGroupImpl->pActiveServiceConfig = NULL;
	pReactorWarmStandByGroupImpl->pActiveServiceConfigList = NULL;
	pReactorWarmStandByGroupImpl->sendQueueReqForAll = RSSL_FALSE;
	pReactorWarmStandByGroupImpl->sendReqQueueCount = 0;
	pReactorWarmStandByGroupImpl->currentStartingServerIndex = RSSL_REACTOR_WSB_STARTING_SERVER_INDEX;
	pReactorWarmStandByGroupImpl->downloadConfigActiveServer = RSSL_REACTOR_WSB_STARTING_SERVER_INDEX;
	pReactorWarmStandByGroupImpl->numOfClosingStandbyServers = 0;
	pReactorWarmStandByGroupImpl->streamId = 0;
	pReactorWarmStandByGroupImpl->isStartingServerIsDown = RSSL_FALSE;
}

typedef struct
{
	RsslQueueLink					tokenSessionLink; /* Keeps in the RsslQueue of RsslReactorTokenSessionImpl */
	RsslReactorTokenSessionImpl* pSessionImpl;
	RsslUInt32 credentialArrayIndex;
	rtr_atomic_val hasBeenActivated;
	RsslReactorChannelImpl* pChannelImpl;
}RsslReactorTokenChannelInfo;

RTR_C_INLINE void rsslClearReactorTokenChannelInfo(RsslReactorTokenChannelInfo* pTokenChannelImpl)
{
	memset((void*)pTokenChannelImpl, 0, sizeof(RsslReactorTokenChannelInfo));
	rsslInitQueueLink(&pTokenChannelImpl->tokenSessionLink);
}

typedef struct
{
	RsslUInt32 numOfDispatchCall;
	RsslUInt16 numOfCloseChannelCall;
	RsslReactorChannelDebugInfoState debugInfoState;
} RsslReactorChannelDebugInfo;

RTR_C_INLINE void rsslClearReactorChannelDebugInfo(RsslReactorChannelDebugInfo* pReactorChannelImplDebugInfo)
{
	memset(pReactorChannelImplDebugInfo, 0, sizeof(RsslReactorChannelDebugInfo));
}

/* RsslReactorChannelImpl 
 * - Handles a channel associated with the RsslReactor */
struct _RsslReactorChannelImpl
{
	RsslReactorChannel reactorChannel;

	RsslReactorImpl *pParentReactor;
	RsslUInt32 initializationTimeout;
	RsslInt64 initializationStartTimeMs;

	/* Reactor side only */
	RsslQueueLink reactorQueueLink;
	RsslQueue *reactorParentQueue;
	RsslReactorEventQueue eventQueue;
	RsslInt64 lastPingReadMs;
	RsslNotifierEvent *pNotifierEvent;
	RsslRet readRet;				/* Last return code from rsslRead on this channel. Helps determine whether data can still be read from this channel. */
	RsslRet writeRet;				/* Last return from rsslWrite() for this channel. Helps determine whether we should request a flush. */
	RsslBool requestedFlush;		/* Indicates whether flushing is signaled for this channel */
	RsslWatchlist *pWatchlist;
	RsslBool	wlDispatchEventQueued;
	RsslBool	tunnelDispatchEventQueued;

	RsslRDMMsg rdmMsg;				/* The typed message that has been decoded */
	RsslReactorChannelSetupState channelSetupState;
	RsslBuffer *pWriteCallAgainBuffer; /* Used when WRITE_CALL_AGAIN is returned from an internal rsslReactorSubmit() call. */
	RsslBuffer *pWriteCallAgainUserBuffer; /* Used when WRITE_CALL_AGAIN is returned for writing JSON buffer from an rsslReactorSubmit() call by users' applications.*/
	RsslBuffer *pUserBufferWriteCallAgain; /* Keeps the user's buffer to ensure that user passes it again. */

	RsslReactorChannelRole channelRole;

	/* When a consumer connection is using the downloadDictionaries feature, store the streamID's used for requesting field & enum type dictionaries. */
	RsslInt32 rwfFldStreamId;
	RsslInt32 rwfEnumStreamId;

	/* Worker thread only */
	RsslQueueLink workerLink;
	RsslQueue *workerParentList;
	RsslInt64 lastPingSentMs;
	RsslErrorInfo channelWorkerCerr;
	RsslInt64 lastRequestedExpireTime;
	RsslInt64 nextExpireTime;
	RsslNotifierEvent *pWorkerNotifierEvent;

	/* Reconnection logic */
	RsslInt32 reconnectMinDelay;
	RsslInt32 reconnectMaxDelay;
	RsslInt32 reconnectDelay;
	RsslInt32 reconnectAttemptLimit;
	RsslInt32 reconnectAttemptCount;
	RsslInt64 lastReconnectAttemptMs;

	RsslInt32 connectionListCount;
	RsslInt32 connectionListIter;
	RsslReactorConnectInfoImpl *connectionOptList;				/* List of connection objects */
	RsslReactorConnectInfoImpl *currentConnectionOpts;  /* Pointer to the current connection info. Actual location is either in the connectionOptList or the warm standby config. This is assigned in one of three places.  
														Either the initial rsslReactorConnect call for a non-warm standby or WSB active, in the reconnection logic once the next connection location has been determined, or 
														during the standby connection initialization for WSB. */
	TunnelManager *pTunnelManager;

	/* Support session management and RDP service discovery. */
	RsslUInt32				httpStausCode; /* the latest HTTP status code */
	RsslRestHandle			*pRestHandle; /* This is used to request the endpoints from RDP service discovery */

	/* This is original login request information */
	RsslBuffer				userName;
	RsslUInt32				flags;
	RsslUInt8				userNameType;

	RsslUInt32 connectionDebugFlags; /*!< Set of RsslDebugFlags for calling the user-set debug callbacks */

	/* Keeps aggregated values of connection statistics */
	RsslReactorChannelStatistic		*pChannelStatistic;
	RsslReactorChannelStatisticFlags statisticFlags;

	/* This is used for token session management */
	RsslReactorTokenChannelInfo		*pTokenSessionList; /* List of RsslReactorTokenSessionImpl pointers for this channel if token management is enabled. This is either a size of 1 or a size of the number of configured oAUth credentials */
	RsslReactorTokenChannelInfo		*pCurrentTokenSession; /* The current token session */

	RsslBool						hasConnected;

	RsslBuffer						temporaryURL; /* Temporary URL for redirect */
	RsslUInt32						temporaryURLBufLength;

	/* For Websocket connections */
	RsslBool						sendWSPingMessage; /* This is used to force sending ping message even though some messages is flushed to network. */
	RsslHashTable					packedBufferHashTable; /* The hash table to keep track of packed buffers */

	/* For Warm Standby by feature */
	RsslReactorWarmStandByHandlerImpl *pWarmStandByHandlerImpl; /* Keeps a list of RsslChannel(s) for connected server(s). */
	RsslQueueLink					  warmstandbyChannelLink; /* Keeps in the RsslQueue of RsslWarmStandByHandlerImpl */
	RsslBool                          isActiveServer; /* This indicates whether this channel is used to connect with the active server. */
	RsslUInt32						  standByServerListIndex; /* Keeps the index of the standby server list in a RsslReactorWarmStandByGroup. */
	RsslBool						  isStartingServerConfig; /* This is used to indicate that this channel uses the starting server configuration. */
	RsslInt64						  lastSubmitOptionsTime; /* Keeps the timestamp when handling the last submit options. */
	RsslQueue						  *pWSRecoveryMsgList; /* Keeps a list of recovery status messages to notify application when this channel becomes active. */
	RsslReactorChannelDebugInfo*      pChannelDebugInfo; /* Provides addtional debugging information for channel and tunnel stream. */

	/* For multi-credentials */
	RsslBool						  deepCopyRole;			/* This RsslReactorChannelImpl contains a deepy copy of the channel role credentials */
	RsslBool			inLoginCredentialCallback;
	RsslBool			doNotNotifyWorkerOnCredentialChange;
	

};

RTR_C_INLINE void rsslClearReactorChannelImpl(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pInfo)
{
	memset(pInfo, 0, sizeof(RsslReactorChannelImpl));
	pInfo->pParentReactor = pReactorImpl;
	pInfo->nextExpireTime = RCIMPL_TIMER_UNSET;
	pInfo->lastRequestedExpireTime = RCIMPL_TIMER_UNSET;
}

typedef enum
{
	RSSL_REACTOR_WS_MAX_NUM_BUFFERS = 0x01,
	RSSL_REACTOR_WS_NUM_GUARANTEED_BUFFERS = 0x02,
	RSSL_REACTOR_WS_HIGH_WATER_MARK = 0x04,
	RSSL_REACTOR_WS_SYSTEM_READ_BUFFERS = 0x08,
	RSSL_REACTOR_WS_SYSTEM_WRITE_BUFFERS = 0x10,
	RSSL_REACTOR_WS_PRIORITY_FLUSH_ORDER = 0x20,
	RSSL_REACTOR_WS_COMPRESSION_THRESHOLD = 0x40,
	RSSL_REACTOR_WS_TRACE = 0x80
} RsslReactorWSIoctlCodes;

/* RsslWarmStandByHandlerImpl
 * - Handles a list of RsslChannels that is used to handle multiple connections for the warm stand by feature. */
struct _RsslReactorWarmStandByHandlerImpl
{
	RsslQueueLink reactorQueueLink;
	RsslQueue rsslChannelQueue; /* Keeps a list of reactor channel */
	RsslQueue submitMsgQueue;
	RsslQueue freeSubmitMsgQueue;
	RsslReactorChannelImpl* pActiveReactorChannel; /* Returns the active server channel for the login based */
	RsslReactorChannelImpl* pNextActiveReactorChannel; /* Keeps the next active channel for the login based */
	RsslReactorChannelImpl mainReactorChannelImpl; /* The customer facing ReactorChannel to represent the warm standby feature. */
	RsslReactorChannelImpl *pStartingReactorChannel; /* The starting reactor channel from rsslReactorConnect() */
	RsslUInt32 mainChannelState; /* The state is defined in RsslReactorWarmStandByHandlerChannelState */
	RsslReactorWarmStandbyGroupImpl *warmStandbyGroupList;
	RsslUInt32 warmStandbyGroupCount;
	RsslUInt32 currentWSyGroupIndex; /* Represents the current warm standby group index. */
	RsslUInt32 warmStandByHandlerState; /* The state is defined in RsslReactorWarmStandByHandlerState */
	RsslReactorImpl			*pReactorImpl; /* The reactor component for this handler. */
	RsslRDMLoginRefresh		rdmLoginRefresh; /* Keeps login response of the first active server. */
	RsslState				rdmLoginState;			/*!< The current state of the login stream. */
	RsslBool		 hasConnectionList; /* This is used to indicate if a connection list is specified. */
	RsslReatorWarmStandbyChInfoImpl  wsbChannelInfoImpl; /* Provider warm standby channel info. */
	RsslMutex					warmStandByHandlerMutex;
	RsslReactorDictionaryVersion	rdmFieldVersion; /* keeps RDM dictionary version of primary sever */
	RsslReactorDictionaryVersion	rdmEnumTypeVersion; /* keeps RDM enumerated type version of primary sever */
	RsslReactorChannelImpl			*pReadMsgChannel; /* keeps track which channel reads the current processing message. */
	RsslBool						queuedRecoveryMessage; /* This is used to indicate for queing recovery message when moving to another WSB group. */
	/* For applying ioctl */
	RsslUInt32				ioCtlCodes; /* Keeps ioctl codes for all channels. Defined in RsslReactorWSIoctlCodes */
	RsslTraceOptions traceOptions; /* Keeps the trace options. */
	RsslUInt32		 traceFileNameLength; /* Keeps track of allocated buffer length for the trace file name. */
	RsslUInt32		maxNumBuffers;
	RsslUInt32		numGuaranteedBuffers;
	RsslUInt32		highWaterMark;
	RsslUInt32		systemReadBuffers;
	RsslUInt32		systemWriteBuffers;
	RsslUInt32		priorityFlushOrder;
	RsslUInt32		compressionThresHold;
};

RTR_C_INLINE void rsslClearReactorWarmStandByHandlerImpl(RsslReactorWarmStandByHandlerImpl* pReactorWarmStandByHandlerImpl)
{
	memset(pReactorWarmStandByHandlerImpl, 0, sizeof(RsslReactorWarmStandByHandlerImpl));
	rsslInitQueueLink(&pReactorWarmStandByHandlerImpl->reactorQueueLink);
	rsslInitQueue(&pReactorWarmStandByHandlerImpl->rsslChannelQueue);
	rsslInitQueue(&pReactorWarmStandByHandlerImpl->submitMsgQueue);
	rsslInitQueue(&pReactorWarmStandByHandlerImpl->freeSubmitMsgQueue);
	pReactorWarmStandByHandlerImpl->mainChannelState = RSSL_RWSB_STATE_CHANNEL_INIT;
	pReactorWarmStandByHandlerImpl->warmStandByHandlerState = RSSL_RWSB_STATE_INIT;
	rsslClearRDMLoginRefresh(&pReactorWarmStandByHandlerImpl->rdmLoginRefresh);
	rsslClearReactorWarmStandbyChInfoImpl(&pReactorWarmStandByHandlerImpl->wsbChannelInfoImpl);
	RSSL_MUTEX_INIT(&pReactorWarmStandByHandlerImpl->warmStandByHandlerMutex);
}

RTR_C_INLINE RsslRet _rsslChannelCopyConnectionList(RsslReactorChannelImpl *pReactorChannel, RsslReactorConnectOptions *pOpts, 
															RsslBool *enableSessionMgnt)
{
	RsslConnectOptions *destOpts, *sourceOpts;
	RsslUInt32 i, j, k;

	if (pOpts->statisticFlags & (RSSL_RC_ST_READ | RSSL_RC_ST_WRITE | RSSL_RC_ST_PING))
	{
		pReactorChannel->pChannelStatistic = (RsslReactorChannelStatistic*)malloc(sizeof(RsslReactorChannelStatistic));
		if (pReactorChannel->pChannelStatistic == 0)
		{
			return RSSL_RET_FAILURE;
		}

		rsslClearReactorChannelStatistic(pReactorChannel->pChannelStatistic);
		pReactorChannel->statisticFlags = pOpts->statisticFlags;
	}

	if(pOpts->connectionCount != 0)
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfoImpl*)malloc(pOpts->connectionCount*sizeof(RsslReactorConnectInfoImpl));

		if(!pReactorChannel->connectionOptList)
		{
			if (pReactorChannel->pChannelStatistic)
			{
				free(pReactorChannel->pChannelStatistic);
				pReactorChannel->pChannelStatistic = NULL;
			}
			return RSSL_RET_FAILURE;
		}

		(*enableSessionMgnt) = RSSL_FALSE;
		for(i = 0; i < pOpts->connectionCount; i++)
		{
			rsslClearReactorConnectInfoImpl(&pReactorChannel->connectionOptList[i]);

			if(pOpts->reactorConnectionList[i].enableSessionManagement)
			{
				*enableSessionMgnt = RSSL_TRUE;
				pReactorChannel->connectionOptList[i].base.location.length = (RsslUInt32)strlen(pOpts->reactorConnectionList[i].location.data);
				pReactorChannel->connectionOptList[i].base.location.data = (char*)malloc((size_t)pReactorChannel->connectionOptList[i].base.location.length + (size_t)1);
				if (pReactorChannel->connectionOptList[i].base.location.data == 0)
				{
					for (k = 0; k < i; k++)
					{
						free(pReactorChannel->connectionOptList[k].base.location.data);
					}
				
					free(pReactorChannel->connectionOptList);
					pReactorChannel->connectionOptList = NULL;
					if (pReactorChannel->pChannelStatistic)
					{
						free(pReactorChannel->pChannelStatistic);
						pReactorChannel->pChannelStatistic = NULL;
					}

					return RSSL_RET_FAILURE;
				}

				memset(pReactorChannel->connectionOptList[i].base.location.data, 0, pReactorChannel->connectionOptList[i].base.location.length + (size_t)1);
				strncpy(pReactorChannel->connectionOptList[i].base.location.data, pOpts->reactorConnectionList[i].location.data, 
							pReactorChannel->connectionOptList[i].base.location.length);
			}
			else
			{
				rsslClearBuffer(&pReactorChannel->connectionOptList[i].base.location);
			}

			pReactorChannel->connectionOptList[i].base.initializationTimeout = pOpts->reactorConnectionList[i].initializationTimeout;
			pReactorChannel->connectionOptList[i].base.enableSessionManagement = pOpts->reactorConnectionList[i].enableSessionManagement;
			pReactorChannel->connectionOptList[i].base.pAuthTokenEventCallback = pOpts->reactorConnectionList[i].pAuthTokenEventCallback;
			pReactorChannel->connectionOptList[i].base.serviceDiscoveryRetryCount = pOpts->reactorConnectionList[i].serviceDiscoveryRetryCount;
			pReactorChannel->connectionOptList[i].base.loginReqIndex = pOpts->reactorConnectionList[i].loginReqIndex;
			pReactorChannel->connectionOptList[i].base.oAuthCredentialIndex = pOpts->reactorConnectionList[i].oAuthCredentialIndex;

			destOpts = &pReactorChannel->connectionOptList[i].base.rsslConnectOptions;
			sourceOpts = &pOpts->reactorConnectionList[i].rsslConnectOptions;

			if(rsslDeepCopyConnectOpts(destOpts, sourceOpts) != RSSL_RET_SUCCESS)
			{
				for(j = 0; j <= i; j++)
				{
					rsslFreeConnectOpts(&pReactorChannel->connectionOptList[j].base.rsslConnectOptions);
				}
				free(pReactorChannel->connectionOptList);
				pReactorChannel->connectionOptList = NULL;
				if (pReactorChannel->pChannelStatistic)
				{
					free(pReactorChannel->pChannelStatistic);
					pReactorChannel->pChannelStatistic = NULL;
				}
				return RSSL_RET_FAILURE;
			}

			if ((destOpts->connectionInfo.unified.address != NULL  &&  *destOpts->connectionInfo.unified.address != '\0') &&
				(destOpts->connectionInfo.unified.serviceName != NULL  &&  *destOpts->connectionInfo.unified.serviceName != '\0'))
			{
				pReactorChannel->connectionOptList[i].userSetConnectionInfo = RSSL_TRUE;
			}
		}
		pReactorChannel->connectionListCount = pOpts->connectionCount;
	}
	else
	{
		pReactorChannel->connectionOptList = (RsslReactorConnectInfoImpl*)malloc(sizeof(RsslReactorConnectInfoImpl));

		if(!pReactorChannel->connectionOptList)
		{
			if (pReactorChannel->pChannelStatistic)
			{
				free(pReactorChannel->pChannelStatistic);
				pReactorChannel->pChannelStatistic = NULL;
			}
			return RSSL_RET_FAILURE;
		}

		rsslClearReactorConnectInfoImpl(pReactorChannel->connectionOptList);

		if(rsslDeepCopyConnectOpts(&(pReactorChannel->connectionOptList->base.rsslConnectOptions), &pOpts->rsslConnectOptions) != RSSL_RET_SUCCESS)
		{
			rsslFreeConnectOpts(&pReactorChannel->connectionOptList->base.rsslConnectOptions);

			free(pReactorChannel->connectionOptList);
			pReactorChannel->connectionOptList = NULL;
			if (pReactorChannel->pChannelStatistic)
			{
				free(pReactorChannel->pChannelStatistic);
				pReactorChannel->pChannelStatistic = NULL;
			}
			return RSSL_RET_FAILURE;
		}

		pReactorChannel->connectionOptList->base.initializationTimeout = pOpts->initializationTimeout;
		pReactorChannel->connectionOptList->base.enableSessionManagement = RSSL_FALSE;
		pReactorChannel->connectionListCount = 1;
	}

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void _rsslFreeReactorWSBServiceConfigImpl(RsslReactorWarmStandbyGroupImpl* pReactorWarmStandByGroupImpl)
{
	RsslQueueLink* pLink;
	RsslReactorWSBServiceConfigImpl *pServiceConfigImpl = NULL;

	if (pReactorWarmStandByGroupImpl->warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		while ((pLink = rsslQueueRemoveFirstLink(pReactorWarmStandByGroupImpl->pActiveServiceConfigList)))
		{
			pServiceConfigImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWSBServiceConfigImpl, queueLink, pLink);

			free(pServiceConfigImpl);
		}
	}
}

RTR_C_INLINE void _rsslFreeWarmStandbyHandler(RsslReactorWarmStandByHandlerImpl *pWarmStandByHandler, RsslUInt32 count, RsslBool freeWarmStandbyHandler)
{
	RsslUInt32 i, o, m;
	RsslInt8 j;
	RsslReactorWarmStandbyGroupImpl* pReactorWarmStandByGroupImpl = NULL;
	RsslQueueLink* pLink = NULL;
	RsslReactorWarmStandbyServiceImpl* pReactorWarmStandbyServiceImpl = NULL;
	RsslReactorSubmitMsgOptionsImpl* pSubmitMsgOptionsImpl;
	
	for (i = 0; i < count; i++)
	{
		pReactorWarmStandByGroupImpl = &(pWarmStandByHandler)->warmStandbyGroupList[i];

		for (o = 0; o < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; o++)
		{
			free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o].data);
		}
		free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

		free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
		rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

		for (m = 0; m < pReactorWarmStandByGroupImpl->standbyServerCount; m++)
		{
			for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[m].perServiceBasedOptions.serviceNameCount; o++)
			{
				free(pReactorWarmStandByGroupImpl->standbyServerList[m].perServiceBasedOptions.serviceNameList[o].data);
			}
			free(pReactorWarmStandByGroupImpl->standbyServerList[m].perServiceBasedOptions.serviceNameList);

			free(pReactorWarmStandByGroupImpl->standbyServerList[m].reactorConnectInfoImpl.base.location.data);
			rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[m].reactorConnectInfoImpl.base.rsslConnectOptions);
		}

		free(pReactorWarmStandByGroupImpl->standbyServerList);

		/* Cleanup the service list */
		while ((pLink = rsslQueueRemoveFirstLink(&pReactorWarmStandByGroupImpl->_serviceList)))
		{
			pReactorWarmStandbyServiceImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandbyServiceImpl, queueLink, pLink);
			
			free(pReactorWarmStandbyServiceImpl->pMemoryLocation);
			free(pReactorWarmStandbyServiceImpl);
		}

		rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);

		if (pReactorWarmStandByGroupImpl->pActiveServiceConfigList)
		{
			_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
			free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);
		}

		if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
		{
			rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
			free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
		}
	}

	if (pWarmStandByHandler->mainReactorChannelImpl.deepCopyRole == RSSL_TRUE)
	{
		RsslReactorOMMConsumerRole* pConsRole = (RsslReactorOMMConsumerRole*)&pWarmStandByHandler->mainReactorChannelImpl.channelRole;

		if (pConsRole->pLoginRequestList != NULL)
		{
			for (j = 0; j < pConsRole->loginRequestMsgCredentialCount; j++)
			{
				if (pConsRole->pLoginRequestList[j]->loginRequestMsg)
				{
					free(pConsRole->pLoginRequestList[j]->loginRequestMsg);
					pConsRole->pLoginRequestList[j]->loginRequestMsg = NULL;
				}
			}

			free(pConsRole->pLoginRequestList);
			pConsRole->pLoginRequestList = NULL;

			/* This has already been free'd in the above */
			pConsRole->pLoginRequest = NULL;
		}

		/* We only copy the oAuth credential list, not the pOAuthCredential element */
		if (pConsRole->pOAuthCredentialList)
		{
			for (j = 0; j < pConsRole->oAuthCredentialCount; j++)
			{
				if (pConsRole->pOAuthCredentialList[j])
				{
					// Clear out all of the credential information
					if (pConsRole->pOAuthCredentialList[j]->clientId.data)
						memset((void*)(pConsRole->pOAuthCredentialList[j]->clientId.data), 0, (size_t)(pConsRole->pOAuthCredentialList[j]->clientId.length));

					if (pConsRole->pOAuthCredentialList[j]->clientSecret.data)
						memset((void*)(pConsRole->pOAuthCredentialList[j]->clientSecret.data), 0, (size_t)(pConsRole->pOAuthCredentialList[j]->clientSecret.length));

					if (pConsRole->pOAuthCredentialList[j]->password.data)
						memset((void*)(pConsRole->pOAuthCredentialList[j]->password.data), 0, (size_t)(pConsRole->pOAuthCredentialList[j]->password.length));

					if (pConsRole->pOAuthCredentialList[j]->tokenScope.data)
						memset((void*)(pConsRole->pOAuthCredentialList[j]->tokenScope.data), 0, (size_t)(pConsRole->pOAuthCredentialList[j]->tokenScope.length));

					if (pConsRole->pOAuthCredentialList[j]->userName.data)
						memset((void*)(pConsRole->pOAuthCredentialList[j]->userName.data), 0, (size_t)(pConsRole->pOAuthCredentialList[j]->userName.length));

					memset((void*)pConsRole->pOAuthCredentialList[j], 0, sizeof(RsslReactorOAuthCredential));
					free((void*)pConsRole->pOAuthCredentialList[j]);
					pConsRole->pOAuthCredentialList[j] = NULL;
				}
			}
			free(pConsRole->pOAuthCredentialList);

			pConsRole->pOAuthCredentialList = NULL;
		}
		else if (pConsRole->pOAuthCredential)
		{
			free(pConsRole->pOAuthCredential);

			pConsRole->pOAuthCredential = NULL;
		}

		pWarmStandByHandler->mainReactorChannelImpl.deepCopyRole = RSSL_FALSE;
	}


	/* Free memory from RDMLoginRefresh */
	if (pWarmStandByHandler->rdmLoginRefresh.userName.data)
	{
		free(pWarmStandByHandler->rdmLoginRefresh.userName.data);
	}

	if (pWarmStandByHandler->rdmLoginRefresh.applicationId.data)
	{
		free(pWarmStandByHandler->rdmLoginRefresh.applicationId.data);
	}

	if (pWarmStandByHandler->rdmLoginRefresh.applicationName.data)
	{
		free(pWarmStandByHandler->rdmLoginRefresh.applicationName.data);
	}

	if (pWarmStandByHandler->rdmLoginRefresh.position.data)
	{
		free(pWarmStandByHandler->rdmLoginRefresh.position.data);
	}

	/* Free trace buffer name if any */
	if (pWarmStandByHandler->traceOptions.traceMsgFileName)
	{
		free(pWarmStandByHandler->traceOptions.traceMsgFileName);
		pWarmStandByHandler->traceOptions.traceMsgFileName = NULL;
		pWarmStandByHandler->traceFileNameLength = 0;
	}

	pWarmStandByHandler->ioCtlCodes = 0;

	/* Cleanup the message queues if any*/
	while ((pLink = rsslQueueRemoveFirstLink(&pWarmStandByHandler->submitMsgQueue)))
	{
		pSubmitMsgOptionsImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorSubmitMsgOptionsImpl, submitMsgLink, pLink);
		_reactorFreeSubmitMsgOptions(pSubmitMsgOptionsImpl);
	}

	free(pWarmStandByHandler->wsbChannelInfoImpl.base.socketIdList);
	free(pWarmStandByHandler->wsbChannelInfoImpl.base.oldSocketIdList);
	free(pWarmStandByHandler->warmStandbyGroupList);

	if (freeWarmStandbyHandler)
	{
		RSSL_MUTEX_DESTROY(&pWarmStandByHandler->warmStandByHandlerMutex);
		free(pWarmStandByHandler);
	}
}

RTR_C_INLINE RsslReactorWarmStandByHandlerImpl* _reactorTakeWSBChannelHandler(RsslReactorImpl *pReactorImpl,RsslQueue* pWarmstandbyChannelPool)
{
	RsslQueueLink* pLink = rsslQueueRemoveFirstLink(pWarmstandbyChannelPool);
	RsslReactorWarmStandByHandlerImpl* pReactorWarmStandByHandler;

	if (!pLink)
	{
		pReactorWarmStandByHandler = (RsslReactorWarmStandByHandlerImpl*)malloc(sizeof(RsslReactorWarmStandByHandlerImpl));

		if (!pReactorWarmStandByHandler)
		{
			return NULL;
		}

		rsslClearReactorWarmStandByHandlerImpl(pReactorWarmStandByHandler);
		pReactorWarmStandByHandler->pReactorImpl = pReactorImpl;
	}
	else
	{
		pReactorWarmStandByHandler = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorWarmStandByHandlerImpl, reactorQueueLink, pLink);

		rsslClearReactorWarmStandByHandlerImpl(pReactorWarmStandByHandler);
		pReactorWarmStandByHandler->pReactorImpl = pReactorImpl;
	}

	return pReactorWarmStandByHandler;
}

RTR_C_INLINE RsslRet _rsslChannelCopyWarmStandByGroupList(RsslReactorImpl *pReactorImpl, RsslQueue *pWSChannelPool, RsslReactorConnectOptions* pOpts, RsslReactorChannelRole* pRole,  RsslReactorWarmStandByHandlerImpl** pWarmStandByHandler, RsslBool* enableSessionMgnt, RsslErrorInfo *pError)
{
	RsslUInt32 wsGroupIndex = 0, j = 0, k = 0, l = 0, m = 0, o = 0;
	RsslReactorWarmStandbyGroupImpl* pReactorWarmStandByGroupImpl = NULL;
	RsslConnectOptions* destOpts, * sourceOpts;
	RsslErrorInfo rsslErrorInfo;
	RsslHashLink *pHashLink = NULL;
	RsslUInt32 numberOfServers = 0;

	(*pWarmStandByHandler) = NULL;
	
	if (pOpts->warmStandbyGroupCount != 0 && pOpts->reactorWarmStandbyGroupList != NULL)
	{
		if (pRole->base.roleType != RSSL_RC_RT_OMM_CONSUMER)
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "A warm standby group list was set for a non-consume role");
			return RSSL_RET_FAILURE;
		}

		/* Checks to ensure the user specified warm standby mode is valid for all groups.*/
		for (wsGroupIndex = 0; wsGroupIndex < pOpts->warmStandbyGroupCount; wsGroupIndex++)
		{
			switch (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].warmStandbyMode)
			{
				case RSSL_RWSB_MODE_SERVICE_BASED:
				{
					break;
				}
				case RSSL_RWSB_MODE_LOGIN_BASED:
				{
					break;
				}
				default: /* Invalid warm standby mode. */
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid warm standby mode specified in a warm standby group.");
					return RSSL_RET_FAILURE;
				}
			}
		}

		wsGroupIndex = 0;

		(*pWarmStandByHandler) = _reactorTakeWSBChannelHandler(pReactorImpl, pWSChannelPool);

		if ((*pWarmStandByHandler) != NULL)
		{
			(*pWarmStandByHandler)->warmStandbyGroupList = (RsslReactorWarmStandbyGroupImpl*)malloc(pOpts->warmStandbyGroupCount*sizeof(RsslReactorWarmStandbyGroupImpl));

			if ((*pWarmStandByHandler)->warmStandbyGroupList == NULL)
			{
				free((*pWarmStandByHandler));
				(*pWarmStandByHandler) = NULL;
				return RSSL_RET_FAILURE;
			}

			for (wsGroupIndex = 0; wsGroupIndex < pOpts->warmStandbyGroupCount; wsGroupIndex++)
			{
				pReactorWarmStandByGroupImpl = &(*pWarmStandByHandler)->warmStandbyGroupList[wsGroupIndex];
				rsslClearReactorWarmStandByGroupImpl(pReactorWarmStandByGroupImpl);

				if (rsslHashTableInit(&pReactorWarmStandByGroupImpl->_perServiceById, 256, rsslHashU64Sum, rsslHashU64Compare,
					RSSL_TRUE, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					/* Free previous warmstandby group if any */
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
					goto cleanUpPreviousWarmStandGroup;
				}

				if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
				{
					pReactorWarmStandByGroupImpl->pActiveServiceConfig = (RsslHashTable*)malloc(sizeof(RsslHashTable));

					if (pReactorWarmStandByGroupImpl->pActiveServiceConfig != NULL)
					{
						if (rsslHashTableInit(pReactorWarmStandByGroupImpl->pActiveServiceConfig, 256, rsslHashBufferSum, rsslHashBufferCompare,
							RSSL_TRUE, &rsslErrorInfo) != RSSL_RET_SUCCESS)
						{
							rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);

							/* Free previous warmstandby group if any */
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
							goto cleanUpPreviousWarmStandGroup;
						}
					}
					else
					{
						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
						goto cleanUpPreviousWarmStandGroup;
					}

					pReactorWarmStandByGroupImpl->pActiveServiceConfigList = (RsslQueue*)malloc(sizeof(RsslQueue));

					if (pReactorWarmStandByGroupImpl->pActiveServiceConfigList == NULL)
					{
						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);

						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
						goto cleanUpPreviousWarmStandGroup;
					}

					rsslInitQueue(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);
				}

				if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
				{
					RsslReactorWSBServiceConfigImpl* pServiceConfigImpl = NULL;

					/* Copies per service based options for the active server info */
					if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameCount != 0 && pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameList)
					{
						pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList = (RsslBuffer*)malloc(pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameCount * sizeof(RsslBuffer));

						if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList == NULL)
						{
							rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
							rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

							/* Free previous warmstandby group if any */
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
							goto cleanUpPreviousWarmStandGroup;
						}

						for (o = 0; o < pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameCount; o++)
						{
							RsslBuffer* pServiceName = &pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameList[o];
							RsslUInt32 hashCode = rsslHashBufferSum(pServiceName);
							pHashLink = rsslHashTableFind(pReactorWarmStandByGroupImpl->pActiveServiceConfig, pServiceName, &hashCode);

							if (pHashLink == NULL) /* Not found this service name */
							{
								pServiceConfigImpl = (RsslReactorWSBServiceConfigImpl*)malloc(sizeof(RsslReactorWSBServiceConfigImpl));

								if (pServiceConfigImpl == NULL)
								{
									for (m = 0; m < o; m++)
									{
										free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
									}

									free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

									rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
									rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);

									_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

									/* Free previous warmstandby group if any */
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
									goto cleanUpPreviousWarmStandGroup;
								}

								rsslClearReactorWSByServiceConfigImpl(pServiceConfigImpl);
							}

							pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o].data = (char*)malloc(pServiceName->length);
							if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o].data == NULL)
							{
								for (m = 0; m < o; m++)
								{
									free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
								}

								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

								rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
								rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);

								_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

								/* Free previous warmstandby group if any */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
								goto cleanUpPreviousWarmStandGroup;
							}

							memcpy(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o].data, pServiceName->data, pServiceName->length);
							pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o].length = pServiceName->length;

							if (pHashLink == NULL)
							{
								/* Add service config to queue link and hash table. */
								rsslQueueAddLinkToBack(pReactorWarmStandByGroupImpl->pActiveServiceConfigList, &pServiceConfigImpl->queueLink);
								rsslHashTableInsertLink(pReactorWarmStandByGroupImpl->pActiveServiceConfig, &pServiceConfigImpl->hashLink,
									&pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[o], &hashCode);
								pServiceConfigImpl->isStartingServerConfig = RSSL_TRUE;
							}
						}

						pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.perServiceBasedOptions.serviceNameCount;
					}
				}

				if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
				{
					/* If session management was turned on and neither pOauthCredential nor pOAuthCredentialList exist, error out */
					if (pRole->ommConsumerRole.pOAuthCredential == NULL && pRole->ommConsumerRole.pOAuthCredentialList == NULL)
					{
						for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
						{
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
						}

						free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);

						if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
						{
							rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						}

						_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "No oAuth credentials were specified with session management enabled.");
						goto cleanUpPreviousWarmStandGroup;
					}
					/* if a credential list is present, then check to see if the index is valid.  If the credential list is present, ignore the index */
					else if (pRole->ommConsumerRole.pOAuthCredentialList != NULL && pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex >= pRole->ommConsumerRole.oAuthCredentialCount)
					{
						for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
						{
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
						}

						free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);

						if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
						{
							rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						}

						_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid oAuth array index %d set for warm standby active.", pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex);
						goto cleanUpPreviousWarmStandGroup;
					}

					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.length = (RsslUInt32)strlen(pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.location.data);
					if (pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.length != 0)
						pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data = (char*)malloc((size_t)pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.length + (size_t)1);

					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.enableSessionManagement = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.enableSessionManagement;
					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.initializationTimeout = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.initializationTimeout;
					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.pAuthTokenEventCallback = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.pAuthTokenEventCallback;
					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.oAuthCredentialIndex;

					if (pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data == NULL)
					{
						for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
						{
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
						}

						free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);

						if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
						{
							rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						}

						_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
						goto cleanUpPreviousWarmStandGroup;
					}

					memset(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data, 0, (size_t)pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.length + (size_t)1);
					strncpy(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data, pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.location.data,
						(size_t)(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.length));

					(*enableSessionMgnt) = RSSL_TRUE;
				}
				else
				{
					rsslClearBuffer(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location);
				}

				/* For Login messages, we only need to verify that the request list exists.  If it doesn't, we handle a null ommConsumerRole.pLoginRequest at l*/
				if (pRole->ommConsumerRole.pLoginRequestList != NULL && pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.loginReqIndex >= pRole->ommConsumerRole.loginRequestMsgCredentialCount)
				{
					for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
					{
						free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
					}

					free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

					rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);

					if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
					{
						rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
					}

					_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
					free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

					/* Free previous warmstandby group if any */
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid login array index %d set for warm standby active.", pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex);
					goto cleanUpPreviousWarmStandGroup;
				}

				pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.loginReqIndex = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.loginReqIndex;
				

				destOpts = &pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions;
				sourceOpts = &pOpts->reactorWarmStandbyGroupList[wsGroupIndex].startingActiveServer.reactorConnectInfo.rsslConnectOptions;

				if (rsslDeepCopyConnectOpts(destOpts, sourceOpts) != RSSL_RET_SUCCESS)
				{
					if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount != 0)
					{
						for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
						{
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
						}

						free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);
					}

					free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);

					rsslFreeConnectOpts(destOpts);

					rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
					if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
					{
						rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
					}
					_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
					free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

					/* Free previous warmstandby group if any */
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
					goto cleanUpPreviousWarmStandGroup;
				}

				if ((destOpts->connectionInfo.unified.address != NULL && *destOpts->connectionInfo.unified.address != '\0') &&
					(destOpts->connectionInfo.unified.serviceName != NULL && *destOpts->connectionInfo.unified.serviceName != '\0'))
				{
					pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.userSetConnectionInfo = RSSL_TRUE;
				}

				++numberOfServers;

				if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerCount != 0 && pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList != NULL)
				{
					pReactorWarmStandByGroupImpl->standbyServerList = (RsslReactorWarmStandbyServerInfoImpl*)malloc(pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerCount * sizeof(RsslReactorWarmStandbyServerInfoImpl));

					if (pReactorWarmStandByGroupImpl->standbyServerList == NULL)
					{
						if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount != 0)
						{
							for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
							{
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
							}

							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);
						}

						free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);

						rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

						rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
						if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
						{
							rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
						}
						_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
						free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

						/* Free previous warmstandby group if any */
						rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
						goto cleanUpPreviousWarmStandGroup;
					}
					
					/* Copies the standby server list */
					for (j = 0; j < pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerCount; j++)
					{
						rsslClearReactorWarmStandbyServerInfoImpl(&pReactorWarmStandByGroupImpl->standbyServerList[j]);

						if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
						{
							RsslReactorWSBServiceConfigImpl *pServiceConfigImpl = NULL;

							/* Copies per service based options for the standby server info */
							if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameCount != 0 && pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameList)
							{
								pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList = (RsslBuffer*)malloc(pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameCount * sizeof(RsslBuffer));

								if (pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList == NULL)
								{
									if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount != 0)
									{
										for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
										{
											free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
										}

										free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);
									}

									free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
									rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

									for (k = 0; k < j; k++)
									{
										for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
										{
											free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
										}
										free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

										free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
										rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
									}

									free(pReactorWarmStandByGroupImpl->standbyServerList);

									rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
									if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
									{
										rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
										free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
									}
									_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

									/* Free previous warmstandby group if any */
									rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
									goto cleanUpPreviousWarmStandGroup;
								}

								for (o = 0; o < pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameCount; o++)
								{
									RsslBuffer *pServiceName = &pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameList[o];
									RsslUInt32 hashCode = rsslHashBufferSum(pServiceName);
									pHashLink = rsslHashTableFind(pReactorWarmStandByGroupImpl->pActiveServiceConfig, pServiceName, &hashCode);

									if (pHashLink == NULL) /* Not found this service name */
									{
										pServiceConfigImpl = (RsslReactorWSBServiceConfigImpl*)malloc(sizeof(RsslReactorWSBServiceConfigImpl));

										if (pServiceConfigImpl == NULL)
										{
											for (m = 0; m < o; m++)
											{
												free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[m].data);
											}
											free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

											if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount != 0)
											{
												for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
												{
													free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
												}

												free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);
											}

											free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
											rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

											for (k = 0; k < j; k++)
											{
												for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
												{
													free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
												}
												free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

												free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
												rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
											}

											free(pReactorWarmStandByGroupImpl->standbyServerList);

											rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
											if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
											{
												rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
												free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
											}
											_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
											free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

											/* Free previous warmstandby group if any */
											rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
											goto cleanUpPreviousWarmStandGroup;
										}

										rsslClearReactorWSByServiceConfigImpl(pServiceConfigImpl);
									}

									pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data = (char*)malloc(pServiceName->length);
									if (pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data == NULL)
									{
										for (m = 0; m < o; m++)
										{
											free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[m].data);
										}
										free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

										if (pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount != 0)
										{
											for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
											{
												free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
											}

											free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);
										}

										free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
										rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

										for (k = 0; k < j; k++)
										{
											for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
											{
												free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
											}
											free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

											free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
											rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
										}

										free(pReactorWarmStandByGroupImpl->standbyServerList);

										rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
										if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
										{
											rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
											free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
										}
										_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
										free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

										/* Free previous warmstandby group if any */
										rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
										goto cleanUpPreviousWarmStandGroup;
									}

									memcpy(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data, pServiceName->data, pServiceName->length);
									pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].length = pServiceName->length;

									if (pHashLink == NULL)
									{
										/* Add service config to queue link and hash table. */
										rsslQueueAddLinkToBack(pReactorWarmStandByGroupImpl->pActiveServiceConfigList, &pServiceConfigImpl->queueLink);
										rsslHashTableInsertLink(pReactorWarmStandByGroupImpl->pActiveServiceConfig, &pServiceConfigImpl->hashLink, 
											&pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o], &hashCode);

										pServiceConfigImpl->isStartingServerConfig = RSSL_FALSE;
										pServiceConfigImpl->standByServerListIndex = j;
									}
								}

								pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameCount = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].perServiceBasedOptions.serviceNameCount;
							}
						}

						if (pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
						{
							/* If session management was turned on and neither pOauthCredential nor pOAuthCredentialList exist, error out */
							if (pRole->ommConsumerRole.pOAuthCredential == NULL && pRole->ommConsumerRole.pOAuthCredentialList == NULL)
							{
								for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
								{
									free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
								}
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

								free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
								rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

								for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameCount; o++)
								{
									free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data);
								}
								free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

								for (k = 0; k < j; k++)
								{
									for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
									{
										free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
									}
									free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

									free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
									rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
								}

								free(pReactorWarmStandByGroupImpl->standbyServerList);

								rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
								if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
								{
									rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								}
								_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);
								/* Free previous warmstandby group if any */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "No oAuth credentials were specified with session management enabled.");
								goto cleanUpPreviousWarmStandGroup;
							}
							/* if a credential list is present, then check to see if the index is valid.  If the credential list is present, ignore the index */
							else if (pRole->ommConsumerRole.pOAuthCredentialList != NULL && pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.oAuthCredentialIndex >= pRole->ommConsumerRole.oAuthCredentialCount)
							{
								for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
								{
									free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
								}
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

								free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
								rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

								for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameCount; o++)
								{
									free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data);
								}
								free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

								for (k = 0; k < j; k++)
								{
									for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
									{
										free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
									}
									free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

									free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
									rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
								}

								free(pReactorWarmStandByGroupImpl->standbyServerList);

								rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
								if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
								{
									rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								}
								_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

								/* Free previous warmstandby group if any */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid oAuth array index %d set for warm standby group connection.", pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex);
								goto cleanUpPreviousWarmStandGroup;
							}

							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.length = (RsslUInt32)strlen(pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.location.data);
							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.data = (char*)malloc((size_t)pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.length + (size_t)1);

							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.initializationTimeout = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.initializationTimeout;
							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.enableSessionManagement = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.enableSessionManagement;
							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.pAuthTokenEventCallback = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.pAuthTokenEventCallback;
							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.oAuthCredentialIndex = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.oAuthCredentialIndex;


							if (pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.data == NULL)
							{
								for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
								{
									free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
								}
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

								free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
								rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

								for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameCount; o++)
								{
									free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data);
								}
								free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

								for (k = 0; k < j; k++)
								{
									for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
									{
										free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
									}
									free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

									free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
									rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
								}

								free(pReactorWarmStandByGroupImpl->standbyServerList);

								rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
								if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
								{
									rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
									free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								}
								_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

								/* Free previous warmstandby group if any */
								rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
								goto cleanUpPreviousWarmStandGroup;
							}

							memset(pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.data, 0, (size_t)pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.length + (size_t)1);
							strncpy(pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.data, pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.location.data,
								(size_t)(pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location.length));

							(*enableSessionMgnt) = RSSL_TRUE;
						}
						else
						{
							rsslClearBuffer(&pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.location);
						}

						/* For Login messages, we only need to verify that the request list exists.  If it doesn't, we handle a null ommConsumerRole.pLoginRequest at l*/
						if (pRole->ommConsumerRole.pLoginRequestList != NULL && pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.loginReqIndex >= pRole->ommConsumerRole.loginRequestMsgCredentialCount)
						{
							for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
							{
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
							}
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

							free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
							rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

							for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameCount; o++)
							{
								free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList[o].data);
							}
							free(pReactorWarmStandByGroupImpl->standbyServerList[j].perServiceBasedOptions.serviceNameList);

							for (k = 0; k < j; k++)
							{
								for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
								{
									free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
								}
								free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

								free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);
								rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
							}

							free(pReactorWarmStandByGroupImpl->standbyServerList);

							rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
							if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
							{
								rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							}
							_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

							/* Free previous warmstandby group if any */
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid login array index %d set for warm standby group connection.", pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.oAuthCredentialIndex);
							goto cleanUpPreviousWarmStandGroup;
						}

						pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.loginReqIndex = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.loginReqIndex;

						++numberOfServers;

						destOpts = &pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.base.rsslConnectOptions;
						sourceOpts = &pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerList[j].reactorConnectInfo.rsslConnectOptions;

						if (rsslDeepCopyConnectOpts(destOpts, sourceOpts) != RSSL_RET_SUCCESS)
						{
							for (k = 0; k <= j; k++)
							{
								for (o = 0; o < pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameCount; o++)
								{
									free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList[o].data);
								}
								free(pReactorWarmStandByGroupImpl->standbyServerList[k].perServiceBasedOptions.serviceNameList);

								free(pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.location.data);

								rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->standbyServerList[k].reactorConnectInfoImpl.base.rsslConnectOptions);
							}

							free(pReactorWarmStandByGroupImpl->standbyServerList);

							for (m = 0; m < pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameCount; m++)
							{
								free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList[m].data);
							}
							free(pReactorWarmStandByGroupImpl->startingActiveServer.perServiceBasedOptions.serviceNameList);

							free(pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.location.data);
							rsslFreeConnectOpts(&pReactorWarmStandByGroupImpl->startingActiveServer.reactorConnectInfoImpl.base.rsslConnectOptions);

							rsslHashTableCleanup(&pReactorWarmStandByGroupImpl->_perServiceById);
							if (pReactorWarmStandByGroupImpl->pActiveServiceConfig)
							{
								rsslHashTableCleanup(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
								free(pReactorWarmStandByGroupImpl->pActiveServiceConfig);
							}
							_rsslFreeReactorWSBServiceConfigImpl(pReactorWarmStandByGroupImpl);
							free(pReactorWarmStandByGroupImpl->pActiveServiceConfigList);

							/* Free previous warmstandby group if any */
							rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unable to copy warmstandby group list.");
							goto cleanUpPreviousWarmStandGroup;
						}

						if ((destOpts->connectionInfo.unified.address != NULL && *destOpts->connectionInfo.unified.address != '\0') &&
							(destOpts->connectionInfo.unified.serviceName != NULL && *destOpts->connectionInfo.unified.serviceName != '\0'))
						{
							pReactorWarmStandByGroupImpl->standbyServerList[j].reactorConnectInfoImpl.userSetConnectionInfo = RSSL_TRUE;
						}
					}
				}

				pReactorWarmStandByGroupImpl->standbyServerCount = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].standbyServerCount;
				pReactorWarmStandByGroupImpl->warmStandbyMode = pOpts->reactorWarmStandbyGroupList[wsGroupIndex].warmStandbyMode;

				if (numberOfServers > (*pWarmStandByHandler)->wsbChannelInfoImpl.maxNumberOfSocket)
				{
					/* Keeps with maximum number of sockets across all groups */
					(*pWarmStandByHandler)->wsbChannelInfoImpl.maxNumberOfSocket = numberOfServers;
				}
			}

			(*pWarmStandByHandler)->warmStandbyGroupCount = pOpts->warmStandbyGroupCount;
		}
		else
		{
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;

cleanUpPreviousWarmStandGroup:

	/* Free previous warmstandby group if any */
	_rsslFreeWarmStandbyHandler(*pWarmStandByHandler, wsGroupIndex, RSSL_TRUE);

	return RSSL_RET_FAILURE;
}

RTR_C_INLINE void _rsslCleanUpPackedBufferHashTable(RsslReactorChannelImpl *pReactorChannel)
{
	if (pReactorChannel->packedBufferHashTable.queueList)
	{
		if (pReactorChannel->packedBufferHashTable.elementCount > 0)
		{
			RsslUInt32 index;
			RsslQueueLink *pLink = NULL;
			RsslReactorPackedBufferImpl *pPackedBufferImpl;
			for (index = 0; index < pReactorChannel->packedBufferHashTable.queueCount; index++)
			{
				RSSL_QUEUE_FOR_EACH_LINK(&pReactorChannel->packedBufferHashTable.queueList[index], pLink)
				{
					pPackedBufferImpl = RSSL_QUEUE_LINK_TO_OBJECT(RsslReactorPackedBufferImpl, hashLink, pLink);

					rsslHashTableRemoveLink(&pReactorChannel->packedBufferHashTable, &pPackedBufferImpl->hashLink);

					free(pPackedBufferImpl);
				}
			}
		}

		rsslHashTableCleanup(&pReactorChannel->packedBufferHashTable);
	}
}


/* All RsslReactorChannelImpl's member variables must be reset properly in rsslResetReactorChannel
   as the RsslReactorChannelImpl can be reused from the channel pool */
RTR_C_INLINE RsslRet _rsslChannelFreeConnectionList(RsslReactorChannelImpl *pReactorChannel)
{
	int i;

	if(pReactorChannel->connectionOptList)
	{
		for(i = 0; i < pReactorChannel->connectionListCount; i++)
		{
			/* Cleaning memory relating to the session management */
			if (pReactorChannel->connectionOptList[i].base.enableSessionManagement)
			{
				free(pReactorChannel->connectionOptList[i].base.location.data);
			}

			rsslFreeConnectOpts(&pReactorChannel->connectionOptList[i].base.rsslConnectOptions);
		}

		free(pReactorChannel->pChannelStatistic);
		free(pReactorChannel->temporaryURL.data);

		free(pReactorChannel->connectionOptList);
		pReactorChannel->connectionOptList = NULL;
	}
	
	return RSSL_RET_SUCCESS;

}

/* Reset reactor channel state in response to channel failure. */
RTR_C_INLINE void rsslResetReactorChannelState(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	pReactorChannel->requestedFlush = 0;
	pReactorChannel->channelSetupState = RSSL_RC_CHST_INIT;
	pReactorChannel->lastPingReadMs = 0;
	pReactorChannel->readRet = 0;
	pReactorChannel->writeRet = 0;
	pReactorChannel->pWriteCallAgainBuffer = 0;
	pReactorChannel->pWriteCallAgainUserBuffer = 0;
	pReactorChannel->pUserBufferWriteCallAgain = 0;
}


/* Fully reset reactor channel (used when channel is retrieved from pool). */
RTR_C_INLINE void rsslResetReactorChannel(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel)
{
	memset(&pReactorChannel->reactorChannel, 0, sizeof(RsslReactorChannel));
	rsslInitQueueLink(&pReactorChannel->reactorQueueLink);
	pReactorChannel->reactorParentQueue = 0;
	rsslClearReactorChannelRole(&pReactorChannel->channelRole);
	rsslInitQueueLink(&pReactorChannel->workerLink);
	pReactorChannel->workerParentList = 0;
	pReactorChannel->lastReconnectAttemptMs = 0;
	pReactorChannel->reconnectAttemptCount = 0;
	pReactorChannel->pWatchlist = NULL;

	/* Always resets the flag to clear the tunnel stream event queued indication. */
	pReactorChannel->tunnelDispatchEventQueued = RSSL_FALSE;

	pReactorChannel->connectionListCount = 0;
	pReactorChannel->connectionListIter = 0;
	pReactorChannel->connectionOptList = NULL;
	pReactorChannel->connectionDebugFlags = 0;
	pReactorChannel->reactorChannel.socketId = (RsslSocket)REACTOR_INVALID_SOCKET;
	pReactorChannel->reactorChannel.oldSocketId = (RsslSocket)REACTOR_INVALID_SOCKET;

	/* Reset all buffers for the session management */
	pReactorChannel->pRestHandle = NULL;

	/* The channel statistics */
	pReactorChannel->pChannelStatistic = NULL;
	pReactorChannel->statisticFlags = RSSL_RC_ST_NONE;

	/* The token session management */
	pReactorChannel->pTokenSessionList = NULL;
	pReactorChannel->pCurrentTokenSession = NULL;
	rsslClearBuffer(&pReactorChannel->temporaryURL);
	pReactorChannel->temporaryURLBufLength = 0;

	/* Additional WebSocket options */
	pReactorChannel->sendWSPingMessage = RSSL_FALSE; 

	rsslResetReactorChannelState(pReactorImpl, pReactorChannel);

	memset(&pReactorChannel->packedBufferHashTable, 0, sizeof(RsslHashTable));

	/* The warm stand by feature */
	pReactorChannel->pWarmStandByHandlerImpl = NULL;
	rsslInitQueueLink(&pReactorChannel->warmstandbyChannelLink);
	pReactorChannel->isActiveServer = RSSL_FALSE;
	pReactorChannel->standByServerListIndex = 0;
	pReactorChannel->isStartingServerConfig = RSSL_FALSE;
	pReactorChannel->lastSubmitOptionsTime = 0;
	pReactorChannel->pWSRecoveryMsgList = NULL;
}

/* Verify that the given RsslReactorChannel is valid for this RsslReactor */
RTR_C_INLINE RsslBool rsslReactorChannelIsValid(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pInfo, RsslErrorInfo *pError)
{
	RsslBool ret = (pInfo->pParentReactor == pReactorImpl);

	if (ret == RSSL_FALSE)
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Invalid channel.");
	return ret;
}

/* Checks that we are not in a callback already (or that's it's okay), and locks the reactor */
RsslRet reactorLockInterface(RsslReactorImpl *pReactorImpl, RsslBool allowedInCallback, RsslErrorInfo *pError);

/* Unlocks reactor */
RsslRet reactorUnlockInterface(RsslReactorImpl *pReactorImpl);

#define MAX_THREADNAME_STRLEN 16

/* Describes the starting process of the Reactor worker thread. */
typedef enum
{
	RSSL_REACTOR_WORKER_THREAD_ST_INIT = 0UL,
	RSSL_REACTOR_WORKER_THREAD_STARTED = 1UL, /* Reactor Worker thread is started. */
	RSSL_REACTOR_WORKER_THREAD_ERROR = 2UL /* Reactor Worker thread has encountered an error (not Started). */
} RsslReactorWorkerThreadStartingState;

/* RsslReactorWorker
 * The reactorWorker handles when to send pings and flushing.
 * Primary responsiblities include:
 *   - Initializing channels and signaling the reactor when channel is active
 *   - Sending periodic pings on idle channels to keep them alive
 *   - Flushing in response to requests from the reactor and signaling when finished
 *   - Processing of timer events
 */
typedef struct 
{
	RsslQueue initializingChannels;	/* Channels to call rsslInitChannel() on */
	RsslQueue activeChannels;			/* Channels currently active */
	RsslQueue inactiveChannels;			/* Channels that have failed in some way */
	RsslQueue reconnectingChannels;
	RsslQueue disposableRestHandles; /* Rest handles that needs to be cleanup */

	RsslNotifier *pNotifier; /* Notifier for workerQueue and channels */
	RsslNotifierEvent *pQueueNotifierEvent;	/* Notification for workerQueue */

	RsslInt64 lastRecordedTimeMs;

	RsslThreadId thread;
	RsslReactorEventQueue workerQueue;
	RsslUInt32 sleepTimeMs; /* Time to sleep when not flushing; should be equivalent to 1/3 of smallest ping timeout. */

	RsslErrorInfo workerCerr;
	RsslReactorEventQueueGroup activeEventQueueGroup;

	/* For sharing access token for multiple reactor channel using the same OAuth credential */
	RsslReactorTokenManagementImpl reactorTokenManagement;

	RsslQueue errorInfoPool; /* Keeps a pool of RsslErrorInfo for notifying users with the token events. */
	RsslQueue errorInfoInUsedPool; /* Keeps a pool of RsslErrorInfo in used */
	RsslMutex errorInfoPoolLock; /* The Mutual exclusive lock for the pool */

	char nameReactorWorker[MAX_THREADNAME_STRLEN]; /* Name of the reactor worker thread */
	RsslBuffer cpuBindWorkerThread; /*!< Specifies Cpu core in string format (Cpu core id or P:X C:Y T:Z format) for the worker thread binding; if the value is not set, then there is no limit of the binding processor cores for the Reactor worker thread.> */
	rtr_atomic_val threadStarted; /*!< Describes the starting process of the Reactor worker thread. see RsslReactorWorkerThreadStartingState > */
} RsslReactorWorker;

typedef enum
{
	RSSL_REACTOR_ST_INIT = 0,
	RSSL_REACTOR_ST_ACTIVE = 1, /* Reactor is active */
	RSSL_REACTOR_ST_ERROR = 2, /* Reactor has encountered an error */
	RSSL_REACTOR_ST_SHUT_DOWN = 3 /* Reactor has shutdown */
} RsslReactorState;

typedef struct
{
	char* pCurrentWrite; /* Keeps the current write position in the debugging memory. */
	RsslUInt32 remainingLength; /* Keeps the remaining buffer's length for writing. */
	RsslBuffer debuggingMemory; /* Keeps the allocation address and size*/
	RsslBuffer outputMemory; /* This is used to present debugging information.*/

} RsslReactorDebugInfoImpl;

RTR_C_INLINE void rsslClearReactorDebugInfoImpl(RsslReactorDebugInfoImpl* pReactorDebugInfo)
{
	memset(pReactorDebugInfo, 0, sizeof(RsslReactorDebugInfoImpl));
}

/* RsslReactorImpl
 * The Reactor handles reading messages and commands from the application.
 * Primary responsibilities include:
 *   - Reading messages from transport
 *   - Calling back the application via callback functions, decoding messages to the RDM structs when appropriate.
 *   - Adding and removing channels in response to network events or requests from user.
 *   - Signaling the worker thread to flush when appropriate
 */
struct _RsslReactorImpl
{
	RsslReactor reactor;								/* Public-facing base reactor object */

	RsslReactorChannelImpl *channelPoolArray;
	RsslQueue channelPool;				/* Pool of available channel structures */
	RsslQueue initializingChannels;	/* Channels waiting for worker to initialize */
	RsslQueue activeChannels;			/* Channels that are active */
	RsslQueue inactiveChannels;			/* Channels that have failed in some way */
	RsslQueue closingChannels;			/* Waiting for close from worker */
	RsslQueue reconnectingChannels;		/* Channels that have been closed, but are currently reconnecting */

	RsslThreadId thread;

	RsslReactorEventQueue reactorEventQueue;

	RsslNotifier *pNotifier; /* Notifier for reactorEventQueue and channels */
	RsslNotifierEvent *pQueueNotifierEvent; /* Notification for reactorEventQueue */

	RsslBuffer memoryBuffer;

	RsslInt64 lastRecordedTimeMs;

	RsslInt32 channelCount;			/* Total number of channels in use. */

	RsslInt32 maxEventsInPool; /* To control size of memory */

	/* Used on each interface in the reactor to ensure thread-safety and that calling interfaces in callbacks is prevented. */
	RsslMutex interfaceLock; /* Ensures function calls are thread-safe */
	RsslBool inReactorFunction; /* Ensures functions are not called inside callbacks */

	RsslReactorEventQueueGroup activeEventQueueGroup;

	RsslReactorWorker reactorWorker;						/* The reactor's worker */
	RsslInt32 dispatchDecodeMemoryBufferSize;					/* The size to allocate for the temporary decoding block. This is used for decoding to the RDM structures. */
	RsslReactorState state;

	RsslInt64 ticksPerMsec;

	/* For RDP token management and service discovery */
	RsslBuffer			serviceDiscoveryURL; /* Used the memory location from the serviceDiscoveryURLBuffer */
	RsslBuffer			serviceDiscoveryURLBuffer;
	RsslBuffer			tokenServiceURLV1; /* Used the memory location from the tokenServiceURLBuffer */
	RsslBuffer			tokenServiceURLBufferV1;
	RsslBuffer			tokenServiceURLV2; /* Used the memory location from the tokenServiceURLBuffer */
	RsslBuffer			tokenServiceURLBufferV2;
	RsslBuffer			accessTokenRespBuffer;
	RsslBuffer			tokenInformationBuffer;
	RsslBuffer			serviceDiscoveryRespBuffer;
	RsslBuffer			argumentsAndHeaders;
	RsslRestClient*			pRestClient;
	RsslBool			registeredRsslRestClientEventFd;
	RsslNotifierEvent *pWorkerNotifierEvent; /* This is used for RsslRestClient to receive notification */
	RsslBuffer			restServiceEndpointRespBuf; /* This is memory allocation for RsslRestServiceEndpointResp */
	RsslRestServiceEndpointResp	restServiceEndpointResp; /* This is used for querying service discovery by users */
	RsslDouble			tokenReissueRatio; /* User defined ration multiply with the expires_in field to retrieve and reissue the access token */
	RsslInt32			reissueTokenAttemptLimit; /* User defined the number of attempt limit*/
	RsslInt32			reissueTokenAttemptInterval; /* User defined the number of attempt interval in milliseconds */

	RsslReactorTokenSessionImpl	*pTokenSessionForCredentialRenewalCallback; /* This is set before calling the callback to get user's credential */
	RsslBool			doNotNotifyWorkerOnCredentialChange;
	RsslBool			rsslWorkerStarted;
	RsslUInt32			restRequestTimeout; /* Keeps the request timeout */

	
	RsslBool			jsonConverterInitialized; 	/* This is used to indicate whether the RsslJsonConverter is initialized */
	RsslJsonConverter	*pJsonConverter; 
	RsslDataDictionary	**pDictionaryList; /* Creates a list of pointer to pointer with the size of 1. */
	RsslReactorServiceNameToIdCallback	*pServiceNameToIdCallback; /* Sets a callback specified by users */
	RsslReactorJsonConversionEventCallback	*pJsonConversionEventCallback; /* Sets a callback specified by users to receive JSON error message. */
	void				*userSpecPtr; /* Users's closure for callback functions */
	RsslErrorInfo		*pJsonErrorInfo; /* Place holder for JSON error messages */
	RsslBool			closeChannelFromFailure; /* This is used to indicate whether to close the channel from dispatching */
	RsslBool			restEnableLog;	/* Enable REST interaction debug messages */
	FILE				*restLogOutputStream;	/* Set output stream for REST debug message (by default is stdout) */
	RsslBool			restEnableLogViaCallback;	/* Enable of invoking a callback specified by user to receive Rest logging message (pRestLoggingCallback). */
	RsslReactorRestLoggingCallback	*pRestLoggingCallback;	/* Sets a callback specified by users to receive Rest logging message. */
	RsslQueue warmstandbyChannelPool;	/* Pool of available RsslReactorWarmStandByHandlerImpl structures */
	RsslQueue closingWarmstandbyChannel;    /* Keeps a list RsslReactorWarmStandByHandlerImpl being closed. */

	RsslInt32		    debugLevel; /*Configure level of debugging info*/
	RsslReactorDebugInfoImpl* pReactorDebugInfo;	/* Keeps debug info pre reactor. */
	RsslUInt32			debugBufferSize;			/* Configure size of debug buffer  */
	RsslMutex			debugLock;					/* Ensures reactor debug function calls are thread-safe */
};

RTR_C_INLINE void rsslClearReactorImpl(RsslReactorImpl *pReactorImpl)
{
	memset(pReactorImpl, 0, sizeof(RsslReactorImpl));
	pReactorImpl->closeChannelFromFailure = RSSL_TRUE;
}

RTR_C_INLINE RsslBool isReactorDebugLevelEnabled(RsslReactorImpl *pReactorImpl, RsslReactorDebuggerLevels debugLevel)
{
	return (pReactorImpl->debugLevel & debugLevel);
}

RTR_C_INLINE RsslBool isReactorDebugEnabled(RsslReactorImpl *pReactorImpl)
{
	return (pReactorImpl->debugLevel != RSSL_RC_DEBUG_LEVEL_NONE);
}

void _assignConnectionArgsToRequestArgs(RsslConnectOptions *pConnOptions, RsslRestRequestArgs* pRestRequestArgs);

/* Populates the request for v1 token handling */
RsslRestRequestArgs* _reactorCreateTokenRequestV1(RsslReactorImpl *pReactorImpl, RsslBuffer *pTokenServiceURL, RsslBuffer *pUserName, RsslBuffer *password, RsslBuffer *pNewPassword,
	RsslBuffer *pClientID, RsslBuffer *pClientSecret, RsslBuffer *pTokenScope, RsslBool takeExclusiveSignOnContorl, RsslBuffer *pPostDataBodyBuf, void *pUserSpecPtr, RsslErrorInfo *pError);

/* Populates the request for v2 token handling */
RsslRestRequestArgs* _reactorCreateTokenRequestV2(RsslReactorImpl* pReactorImpl, RsslBuffer* pTokenServiceURL, RsslBuffer* pClientId, RsslBuffer* pClientSecret, RsslBuffer* pTokenScope, RsslBuffer* pHeaderAndDataBodyBuf, void* pUserSpecPtr, RsslErrorInfo* pError);


RsslRestRequestArgs* _reactorCreateRequestArgsForServiceDiscovery(RsslReactorImpl *pReactorImpl, RsslBuffer *pServiceDiscoveryURL, RsslReactorDiscoveryTransportProtocol transport,
																RsslReactorDiscoveryDataFormatProtocol dataFormat, RsslBuffer *pTokenType, RsslBuffer *pAccessToken,
																RsslBuffer *pArgsAndHeaderBuf, void *pUserSpecPtr, RsslErrorInfo* pError);

RsslRet _reactorGetAccessTokenAndServiceDiscovery(RsslReactorChannelImpl* pReactorChannelImpl, RsslBool *queryConnectInfo, RsslErrorInfo* pError);

RsslBuffer* getHeaderValue(RsslQueue *pHeaders, RsslBuffer* pHeaderName);

void _cumulativeValue(RsslUInt* destination, RsslUInt32 value);

/* Checks whether the ReactorChannel handles the warm standby feature at its current state */
RsslBool _reactorHandlesWarmStandby(RsslReactorChannelImpl *pReactorChannelImpl);

RsslReactorErrorInfoImpl *rsslReactorGetErrorInfoFromPool(RsslReactorWorker *pReactorWoker);

void rsslReactorReturnErrorInfoToPool(RsslReactorErrorInfoImpl *pReactorErrorInfo, RsslReactorWorker *pReactorWoker);

RsslBool _reactorHandlesWarmStandby(RsslReactorChannelImpl *pReactorChannelImpl);

RsslBool _isActiveServiceForWSBChannelByID(RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannel, RsslUInt serviceId);

RsslBool _isActiveServiceForWSBChannelByName(RsslReactorWarmStandbyGroupImpl *pWarmStandByGroupImpl, RsslReactorChannelImpl *pReactorChannel, const RsslBuffer* pServiceName);

RsslBool isRsslChannelActive(RsslReactorChannelImpl *pReactorChannelImpl);

RsslRet copyWlItemRequest(RsslReactorImpl *pReactorImpl, RsslReactorSubmitMsgOptionsImpl* pDestinationImpl, WlItemRequest *pItemRequest);

RsslRet _reactorChannelGetTokenSession(RsslReactorChannelImpl* pReactorChannel,
	RsslReactorOAuthCredential* pOAuthCredential, RsslReactorTokenChannelInfo* pTokenChannelImpl, RsslBool setMutex, RsslErrorInfo* pError);

RsslRet _reactorChannelGetTokenSessionList(RsslReactorChannelImpl* pReactorChannel,
	 RsslBool setMutex, RsslErrorInfo* pError);

/* Setup and start the worker thread (Should be called from rsslCreateReactor) */
RsslRet _reactorWorkerStart(RsslReactorImpl *pReactorImpl, RsslCreateReactorOptions *pReactorOptions, rtr_atomic_val reactorIndex, RsslErrorInfo *pError);

/* Cleanup all reactor resources(it is assumed that there will be no more use of this reactor so all memory can be cleaned up */
void _reactorWorkerCleanupReactor(RsslReactorImpl *pReactorImpl);

/* Write reactor thread function */
RSSL_THREAD_DECLARE(runReactorWorker, pArg);

/* Estimate encoded message size. */
RsslUInt32 _reactorMsgEncodedSize(RsslMsg *pMsg);

void _clearRoleSensitiveData(RsslReactorOMMConsumerRole* pRole);

/**
* @brief Print out the given input argument to the output stream.
*  Method will send RestLoggingEvent and Reactor will invoke pRestLoggingCallback if enabled
*  otherwise print out to the output stream.
* @param pRestRequestArgs specifies REST request data.
* @param pError Error structure to be populated in the event of an error.
* @see RsslReactorImpl.restEnableLog, RsslReactorImpl.restEnableLogViaCallback, RsslReactorImpl.pRestLoggingCallback.
*/
RsslRet rsslRestRequestDump(RsslReactorImpl* pReactorImpl, RsslRestRequestArgs* pRestRequestArgs, RsslError* pError);

/**
* @brief Print out the given input argument to the output stream.
*  Method will send RestLoggingEvent and Reactor will invoke pRestLoggingCallback if enabled
*  otherwise print out to the output stream.
* @param pRestResponseArgs specifies REST response data.
* @param pError Error structure to be populated in the event of an error.
* @see RsslReactorImpl.restEnableLog, RsslReactorImpl.restEnableLogViaCallback, RsslReactorImpl.pRestLoggingCallback.
*/
RsslRet rsslRestResponseDump(RsslReactorImpl* pReactorImpl, RsslRestResponse* pRestResponseArgs, RsslError* pError);
RsslRet _getCurrentTimestamp(RsslBuffer* timeStamp);

RSSL_VA_API RsslRet rsslReactorCreateRestClient(RsslReactorImpl* pRsslReactorImpl, RsslErrorInfo* pError);

void _writeDebugInfo(RsslReactorImpl* pReactorDebugInfo, const char* debugText, ...);

RsslRet _initReactorAndChannelDebugInfo(RsslReactorImpl* pReactorDebugInfo, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

RsslRet _initReactorDebugInfo(RsslReactorImpl *pReactorImpl, RsslErrorInfo *pError);

RsslRet _initReactorChannelDebugInfo(RsslReactorImpl *pReactorImpl, RsslReactorChannelImpl *pReactorChannel, RsslErrorInfo *pError);

void _cleanupReactorChannelDebugInfo(RsslReactorChannelImpl *pReactorChannel);

#ifdef __cplusplus
};
#endif

#endif
