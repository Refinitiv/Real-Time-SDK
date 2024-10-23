/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2019-2021,2024 LSEG. All rights reserved.           --
 *|-----------------------------------------------------------------------------
 */

#ifndef VA_WATCHLIST_TEST_FRAMEWORK_H
#define VA_WATCHLIST_TEST_FRAMEWORK_H 
#include "testFramework.h"
#include "rtr/rsslReactor.h" 
#include "rtr/rsslGetTime.h"
#include "gtest/gtest.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	/* Adjusts speed of time-sensitive tests.
	 * Divides all timeouts by the specified value. Useful when running in slower environments
	 * such as a debugger or valgrind where time-sensitive tests would otherwise fail. */
	float speed;

} WtfGlobalConfig;

extern WtfGlobalConfig wtfGlobalConfig;

void wtfsleep(int millisec);

RTR_C_INLINE void wtfClearGlobalConfig()
{
	wtfGlobalConfig.speed = 1.0f;
}

static const RsslInt32 WTF_DEFAULT_RECONNECT_ATTEMPT_LIMIT = -1;
static const RsslInt32 WTF_DEFAULT_RECONNECT_MIN_DELAY = 500;
static const RsslInt32 WTF_DEFAULT_RECONNECT_MAX_DELAY = 3000;

static const RsslInt32 WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID  = 1;
static const void* WTF_DEFAULT_LOGIN_USER_SPEC_PTR = (void*)0x55557777;
static const void* WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR = (void*)0x22223333;
static const RsslUInt WTF_MCAST_UPDATE_BUFFER_LIMIT = 10;
#define WTF_DIRECTORY_STREAM_ID 2 /* Used when requesting a source directory. */

extern RsslUInt32 service1DictionariesProvidedCount;
extern RsslBuffer* service1DictionariesProvidedList;
extern RsslUInt32 service1DictionariesUsedCount;
extern RsslBuffer* service1DictionariesUsedList;


RsslRDMMsg *rdmMsgCreateCopy(RsslRDMMsg *pRdmMsg);

/* Test framework for the watchlist.  Provides many convenience functions for connecting, logging
 * in, exchanging service information, and testing different payloads. */

/* Identifies the component associated with a test event or action. */
typedef enum
{
	WTF_TC_INIT,
	WTF_TC_PROVIDER,		/* Component is a Provider. */
	WTF_TC_CONSUMER,		/* Component is a Consumer. */
} WtfComponent;


/*** Events ***/

/* Event structures contain information about events received in RsslReactor callback functions
 * of each component. In addition to providing the type of event, events may contain copies
 * of information received (these copies are cleaned up automatically on calls to wtfDispatch()
 * or cleanup) */

/* Types of Events. */
typedef enum
{
	WTF_DE_INIT,
	WTF_DE_CHNL,		/* Component should receive a channel event. */
	WTF_DE_RSSL_MSG,	/* Component should receive an RsslMsg. */
	WTF_DE_RDM_MSG		/* Component should receive an RsslRDMMsg. */
} WtfEventType;

typedef struct
{
	WtfComponent		component;	/* Type of component (provider or consumer) */
	WtfEventType		type;		/* The type of event (e.g. msg event, channel event) */
	RsslTimeValue		timeUsec;	/* Time (in microseconds) at which the event occurred. */
} WtfEventBase;

typedef enum
{
	WTF_WSBM_NONE = 0,
	WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE = 1,
	WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY = 2,
	WTF_WSBM_SERVICE_BASED_ACTIVE = 3,
	WTF_WSBM_SERVICE_BASED_STANDBY = 4
}WtfWarmStandbyMode;

typedef struct
{
	/* Provider listening server. */
	RsslServer *pServer;
	WtfWarmStandbyMode warmStandbyMode;
	char serverPort[10];
	RsslConnectionTypes connType;
}WtfTestServer;

RTR_C_INLINE void wtfClearTestEventBase(WtfEventBase *pBase, WtfComponent component,
		WtfEventType type)
{
	pBase->component = component;
	pBase->type = type;
	pBase->timeUsec = 0;
}

typedef struct {
	WtfEventBase	base;
	RsslMsg			*pRsslMsg;
	void			*pUserSpec;
	RsslBuffer		*pServiceName;
	RsslBool		hasSeqNum;
	RsslBool		seqNum;
	RsslUInt64		serverIndex;
} WtfRsslMsgEvent;

RTR_C_INLINE void wtfRsslMsgEventInit(WtfRsslMsgEvent *pEvent, WtfComponent component,
		RsslMsg *pRsslMsg)
{
	pEvent->base.type = WTF_DE_RSSL_MSG;
	pEvent->base.component = component;
	pEvent->base.timeUsec = rsslGetTimeMicro();
	pEvent->pRsslMsg = rsslCopyMsg(pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, NULL);
	pEvent->pUserSpec = NULL;
	pEvent->pServiceName = NULL;
	pEvent->hasSeqNum = RSSL_FALSE;
	pEvent->seqNum = 0;
	pEvent->serverIndex = 0;
}

RTR_C_INLINE void wtfRsslMsgEventCleanup(WtfRsslMsgEvent *pEvent)
{
	if (pEvent->pRsslMsg)
		rsslReleaseCopiedMsg(pEvent->pRsslMsg);

	if (pEvent->pServiceName)
	{
		free(pEvent->pServiceName->data);
		free(pEvent->pServiceName);
	}
}

typedef struct {
	WtfEventBase	base;
	RsslRDMMsg		*pRdmMsg;
	void			*pUserSpec;
	RsslBuffer		*pServiceName;
	RsslUInt64		serverIndex;
} WtfRdmMsgEvent;

void wtfRdmMsgEventInit(WtfRdmMsgEvent *pEvent, WtfComponent component,
		RsslRDMMsg *pRdmMsg);

RTR_C_INLINE void wtfRdmMsgEventCleanup(WtfRdmMsgEvent *pEvent)
{
	if (pEvent->pRdmMsg)
		free(pEvent->pRdmMsg);

	if (pEvent->pServiceName)
	{
		free(pEvent->pServiceName->data);
		free(pEvent->pServiceName);
	}
}

typedef struct
{
	WtfEventBase			base;
	RsslReactorChannelEventType	channelEventType;
	RsslRet					rsslErrorId;
	int						port;
} WtfChannelEvent;

RTR_C_INLINE void wtfClearChannelEvent(WtfChannelEvent *pEvent, WtfComponent component)
{
	memset(pEvent, 0, sizeof(WtfChannelEvent));
	pEvent->base.type = WTF_DE_CHNL;
	pEvent->base.component = component;
	pEvent->base.timeUsec = rsslGetTimeMicro();
	pEvent->rsslErrorId = RSSL_RET_SUCCESS;
}

/* The set of events that can be received during a test. */
typedef union
{
	WtfEventBase		base;
	WtfRsslMsgEvent		rsslMsg;
	WtfRdmMsgEvent		rdmMsg;
	WtfChannelEvent		channelEvent;
} WtfEvent;

bool wtfStartTestInt();


void wtfFinishTestInt();

void wtfCloseChannel(WtfComponent component, RsslUInt16 serverIndex = 0);

WtfTestServer* getWtfTestServer(RsslUInt64 index = 0);

typedef struct WtfSubmitMsgOptionsEx WtfSubmitMsgOptionsEx;

/* Used to read directly from RsslChannels (no reactor). 
 * Used by wtfDispatch, and creates the same events. */
static void wtfDispatchRaw(RsslChannel *pChannel, WtfComponent component, time_t timeoutMsec);

/* Used to write directly to RsslChannels (no reactor).
 * Used by wtfSubmitMsg. */
static void wtfSubmitMsgRaw(RsslChannel *pChannel, RsslMsg *pMsg, RsslRDMMsg *pRdmMsg, WtfSubmitMsgOptionsEx *pOptsEx);

  /******************/
 /*** INTERFACES ***/
/******************/

/* These are the main interfaces to use when creating tests. 
 * The functions are described here, but the existing tests should also be able to serve as a 
 * guide for proper usage. */

typedef struct
{
	RsslConnectionTypes connectionType;
	RsslBool			useRawProvider; /* Provider server & channel do not use reactor.
										 * Used for tests which may require special behavior
										 * (such as internal extensions or not sending pings). */
	RsslUInt16          numberOfServer; /* The number of servers to bind */
} WtfInitOpts;

RTR_C_INLINE void wtfClearInitOpts(WtfInitOpts *pOpts)
{
	pOpts->connectionType = RSSL_CONN_TYPE_SOCKET;
	pOpts->useRawProvider = RSSL_FALSE;
	pOpts->numberOfServer = 1;
}

/* Call before starting a series of tests. Initializes the framework, creates
 * provider & consumer reactors,  and binds the provider component's server. */
void wtfInit(WtfInitOpts *pOpts, RsslUInt32 maxOutputBufSize = 0);

void wtfBindServer(RsslConnectionTypes connectionType, char  *pServerPort = const_cast<char*>("14011"));

void wtfShutdownServer(RsslUInt16 serverIndex = 0);

void wtfRestartServer(RsslUInt16 serverIndex = 0);

void wtfCloseServer(RsslUInt16 serverIndex = 0);

/* Call after finishing a series of tests. Cleans up component reactors and resources,
 * and closes the provider server. */
void wtfCleanup();

/* Call before starting a test. Initializes the test event queue. */
#define wtfStartTest() (rssl_test_start(), wtfStartTestInt())

/* Call when finishing a test. Closes component channels. */
#define wtfFinishTest() (wtfFinishTestInt(), rssl_test_finish())

/* Retrieves an event that was received while performing an action or calling wtfDispatch(). */
WtfEvent *wtfGetEvent();

/* Uses notification & calls rsslReactorDispatch() on the given component's channel for the
 * specified time.
 * To ensure that previous events are not missed, this call will fail if events are still
 * present in the queue. */
void wtfDispatch(WtfComponent component, time_t timeoutMsec, RsslUInt64 index = 0, RsslBool forceDispatch = 0);

/* Additional options for WtfSubmitMsg beyond RsslReactorSubmitMsgOptions. */
struct WtfSubmitMsgOptionsEx
{
	RsslBool	hasSeqNum:1;		/* Indicates presence of seqNum. */
	RsslUInt32	seqNum;				/* Sequence number to send. Supported for components in raw (non-reactor) mode only
									   * as sequence numbers are an internal extension and . */
};

RTR_C_INLINE void wtfClearSubmitMsgOptionsEx(WtfSubmitMsgOptionsEx *pOptsEx)
{
	pOptsEx->hasSeqNum = RSSL_FALSE;
	pOptsEx->seqNum = 0;
}

/* Submits a message using rsslReactorSubmit(). If no events are expected,
 * set the noExpectedEvents parameter to call Dispatch automatically and verify this
 * (this should be true for most cases). */
void wtfSubmitMsg(RsslReactorSubmitMsgOptions *pOpts, WtfComponent component,
		WtfSubmitMsgOptionsEx *pOptsEx, RsslBool noEventsExpected, RsslUInt16 serverIndex = 0);

/* Gets an RsslMsg from an event. */
RTR_C_INLINE RsslMsg *wtfGetRsslMsg(WtfEvent *pEvent)
{
	if (pEvent->base.type != WTF_DE_RSSL_MSG) return NULL;
	return pEvent->rsslMsg.pRsslMsg;
}

/* Gets an RsslRDMMsg from an event. */
RTR_C_INLINE RsslRDMMsg *wtfGetRdmMsg(WtfEvent *pEvent)
{
	if (pEvent->base.type != WTF_DE_RDM_MSG) return NULL;
	return pEvent->rdmMsg.pRdmMsg;
}

/* Gets a WtfChannelEvent from an event. */
RTR_C_INLINE WtfChannelEvent *wtfGetChannelEvent(WtfEvent *pEvent)
{
	if (pEvent->base.type != WTF_DE_CHNL) return NULL;
	return &pEvent->channelEvent;
}


typedef enum
{
	WTF_CB_NONE,				/* Do not use the domain-specific callback. */
	WTF_CB_USE_DOMAIN_CB,		/* Use the domain-specific callback. */
	WTF_CB_RAISE_TO_DEFAULT_CB	/* Raise it to the default message callback. */
} WtfCallbackAction;


/* Options for wtfSetupConnection. */
typedef struct
{
	RsslInt32	reconnectAttemptLimit;			/* reconnectAttemptLimit used when connecting. */
	RsslInt32	reconnectMinDelay;				/* reconnectMaxDelay used when connecting consumer. */
	RsslInt32	reconnectMaxDelay;				/* reconnectMaxDelay used when connecting consumer. */
	RsslBool	login;							/* Consumer logs in. */
	RsslBool	accept;							/* Automatically accept connection on provider. */
	RsslBool	provideLoginRefresh;			/* Provider automatically accepts login. */
	RsslBool	provideDefaultDirectory;		/* Provider automatically provides a directory
												 * containing Service1 when requested. */
	RsslBool	singleOpen;						/* Enables SingleOpen on watchlist. */
	RsslBool	allowSuspectData;				/* Enables AllowSuspectData on watchlist. */
	WtfCallbackAction	consumerLoginCallback;			/* Enables consumer loginMsgCallback. */
	WtfCallbackAction	consumerDirectoryCallback;		/* Enables consumer directoryMsgCallback. */
	WtfCallbackAction	consumerDictionaryCallback;		/* Enables consumer dictionaryMsgCallback. */
	WtfCallbackAction	providerDictionaryCallback;		/* Enables provider dictionaryMsgCallback. */
	RsslUInt32	postAckTimeout;					/* Sets watchlist post ack timeout. */
	RsslUInt32	requestTimeout;					/* Sets watchlist request timeout. */
	RsslBool	multicastGapRecovery;			/* Provider's login response indicates
												 * whether watchlist should recover from gaps. */
	RsslBool    provideDefaultServiceLoad;		/* Provide a default service's load for directory refresh. */
	RsslBool    provideDictionaryUsedAndProvided; /* Provides dictionary used and provided list */
	char		*pServerPort;					/* A server port to establish a connection. */
	RsslReactorConnectInfo* reactorConnectionList;	/*!< A list of connections.  Each connection in the list will be tried with each reconnection attempt. */

	RsslUInt32				connectionCount;		/*!< The number of connections in reactorConnectionList. */
	RsslPreferredHostOptions preferredHostOpts;
	RsslBool watchlistOn;						/* Turns off the watchlist, used for preferred host tests */
} WtfSetupConnectionOpts;

/* Initializes commonly used settings of WtfSetupConnectionOpts. */
static void wtfClearSetupConnectionOpts(WtfSetupConnectionOpts *pOpts)
{
	pOpts->reconnectAttemptLimit = WTF_DEFAULT_RECONNECT_ATTEMPT_LIMIT;
	pOpts->reconnectMinDelay = WTF_DEFAULT_RECONNECT_MIN_DELAY;
	pOpts->reconnectMaxDelay = WTF_DEFAULT_RECONNECT_MAX_DELAY;
	pOpts->login = RSSL_TRUE;
	pOpts->accept = RSSL_TRUE;
	pOpts->provideLoginRefresh = RSSL_TRUE;
	pOpts->provideDefaultDirectory = RSSL_TRUE;
	pOpts->singleOpen = RSSL_TRUE;
	pOpts->allowSuspectData = RSSL_TRUE;
	pOpts->consumerLoginCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->consumerDirectoryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->consumerDictionaryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->providerDictionaryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->postAckTimeout = 15000;
	pOpts->requestTimeout = 15000;
	pOpts->multicastGapRecovery = RSSL_TRUE;
	pOpts->provideDefaultServiceLoad = RSSL_FALSE;
	pOpts->provideDictionaryUsedAndProvided = RSSL_FALSE;
	pOpts->pServerPort = const_cast<char*>("14011");
	pOpts->reactorConnectionList = NULL;
	pOpts->connectionCount = 0;
	rsslClearRsslPreferredHostOptions(&pOpts->preferredHostOpts);
	pOpts->watchlistOn = RSSL_TRUE;
}

/* Options for wtfSetupWarmStandbyConnection. */
typedef struct
{
	RsslInt32	reconnectAttemptLimit;			/* reconnectAttemptLimit used when connecting. */
	RsslInt32	reconnectMinDelay;				/* reconnectMaxDelay used when connecting consumer. */
	RsslInt32	reconnectMaxDelay;				/* reconnectMaxDelay used when connecting consumer. */
	RsslBool	login;							/* Consumer logs in. */
	RsslBool	accept;							/* Automatically accept connection on provider. */
	RsslBool	provideLoginRefresh;			/* Provider automatically accepts login. */
	RsslBool	provideDefaultDirectory;		/* Provider automatically provides a directory
												 * containing Service1 when requested. */
	RsslBool	singleOpen;						/* Enables SingleOpen on watchlist. */
	RsslBool	allowSuspectData;				/* Enables AllowSuspectData on watchlist. */
	WtfCallbackAction	consumerLoginCallback;			/* Enables consumer loginMsgCallback. */
	WtfCallbackAction	consumerDirectoryCallback;		/* Enables consumer directoryMsgCallback. */
	WtfCallbackAction	consumerDictionaryCallback;		/* Enables consumer dictionaryMsgCallback. */
	WtfCallbackAction	providerDictionaryCallback;		/* Enables provider dictionaryMsgCallback. */
	RsslUInt32	postAckTimeout;					/* Sets watchlist post ack timeout. */
	RsslUInt32	requestTimeout;					/* Sets watchlist request timeout. */
	RsslBool	multicastGapRecovery;			/* Provider's login response indicates
												 * whether watchlist should recover from gaps. */
	RsslBool    provideDefaultServiceLoad;		/* Provide a default service's load for directory refresh. */
	RsslBool    provideDictionaryUsedAndProvided; /* Provides dictionary used and provided list */
	RsslReactorWarmStandbyGroup *reactorWarmStandbyGroupList; /*!< A list of warmstandby group for the warmstandby feature.
															   Reactor always uses this list if specified and then the reactorConnectionList option. */
	RsslUInt32				warmStandbyGroupCount;  /*!< The number of warmstandby groups in reactorWarmStandByGroupList. */

	RsslReactorConnectInfo	*reactorConnectionList;	/*!< A list of connections.  Each connection in the list will be tried with each reconnection attempt. */

	RsslUInt32				connectionCount;		/*!< The number of connections in reactorConnectionList. */
	RsslPreferredHostOptions preferredHostOpts;
} WtfSetupWarmStandbyOpts;

/* Initializes commonly used settings of WtfSetupWarmStandbyConnectionOpts. */
static void wtfClearSetupWarmStandbyConnectionOpts(WtfSetupWarmStandbyOpts *pOpts)
{
	pOpts->reconnectAttemptLimit = WTF_DEFAULT_RECONNECT_ATTEMPT_LIMIT;
	pOpts->reconnectMinDelay = WTF_DEFAULT_RECONNECT_MIN_DELAY;
	pOpts->reconnectMaxDelay = WTF_DEFAULT_RECONNECT_MAX_DELAY;
	pOpts->login = RSSL_TRUE;
	pOpts->accept = RSSL_TRUE;
	pOpts->provideLoginRefresh = RSSL_TRUE;
	pOpts->provideDefaultDirectory = RSSL_TRUE;
	pOpts->singleOpen = RSSL_TRUE;
	pOpts->allowSuspectData = RSSL_TRUE;
	pOpts->consumerLoginCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->consumerDirectoryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->consumerDictionaryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->providerDictionaryCallback = WTF_CB_USE_DOMAIN_CB;
	pOpts->postAckTimeout = 15000;
	pOpts->requestTimeout = 15000;
	pOpts->multicastGapRecovery = RSSL_TRUE;
	pOpts->provideDefaultServiceLoad = RSSL_FALSE;
	pOpts->provideDictionaryUsedAndProvided = RSSL_FALSE;
	pOpts->reactorWarmStandbyGroupList = NULL;
	pOpts->warmStandbyGroupCount = 0;
	pOpts->reactorConnectionList = NULL;
	pOpts->connectionCount = 0;
	rsslClearRsslPreferredHostOptions(&pOpts->preferredHostOpts);
}

typedef struct
{
	RsslUInt32 warmStandbyMode;

}WtfWarmStandbyExpectedMode;

/*** Connections ***/

/* Perform a standard connect attempt. (Most tests connect in the same way,
 * options are provided for minor changes). */
void wtfSetupConnection(WtfSetupConnectionOpts *pOpts, RsslConnectionTypes connectionType = RSSL_CONN_TYPE_SOCKET);

void wtfSetupConnectionList(WtfSetupConnectionOpts* pOpts, RsslConnectionTypes connectionType, RsslBool noEventsExpected, RsslUInt16 serverIndex = 0);

void wtfSetupWarmStandbyConnection(WtfSetupWarmStandbyOpts *pOpts, WtfWarmStandbyExpectedMode* pExpectedWarmStandbyMode, 
	RsslRDMService *pActiveServerService, RsslRDMService* pStandByServerService, RsslBool sendDirectoryRequest = RSSL_FALSE, RsslConnectionTypes connectionType = RSSL_CONN_TYPE_SOCKET, RsslBool multiLogin = RSSL_FALSE);

void wtfSetupConnectionServerFromConnectionList(WtfSetupWarmStandbyOpts *pOpts, RsslConnectionTypes connectionType = RSSL_CONN_TYPE_SOCKET, RsslBool multiLogin = RSSL_FALSE);

void wtfSendDefaultSourceDirectory(WtfSetupWarmStandbyOpts *pOpts, RsslRDMDirectoryRequest *pRDMDirectoryRequest,RsslUInt16 serverIndex = 0);

/* Accepts the connection for the provider (waits for notification; same as
 * calling wtfAcceptWithTime(5000). */
void wtfAccept(RsslUInt64 serverIndex = 0);

/* Accepts the connection for the provider (waits for notification up to the specified time). */
void wtfAcceptWithTime(time_t timeoutMsec, RsslUInt64 serverIndex = 0);

/* Gets channel information (wraps around rsslReactorGetChannelInfo). */
RsslRet wtfGetChannelInfo(WtfComponent component, RsslReactorChannelInfo *pChannelInfo, RsslUInt16 serverIndex = 0);

/* Returns the currently-used connection type for the test. */
RsslConnectionTypes wtfGetConnectionType();

/* Returns the default login stream ID used by the consumer when logging in. */
RsslInt32 wtfConsumerGetDefaultLoginStreamId();

/* Initializes a default login request for the consumer
 * (wraps around rsslInitDefaultRDMLoginRequest) */
void wtfInitDefaultLoginRequest(RsslRDMLoginRequest *pLoginRequest);

/* Initializes a default login refresh for a provider to send. */
void wtfInitDefaultLoginRefresh(RsslRDMLoginRefresh *pLoginRefresh, bool supportWarmStandby = false);

/* Retrieves the channel of the component. */
RsslReactorChannel *wtfGetChannel(WtfComponent component, RsslUInt16 serverIndex = 0);

/* Sets an function containing "extra actions" to perform while in the msgCallback. */
void wtfSetConsumerMsgCallbackAction(void (*function));


/*** Login ***/

/* The username typically used on login. */
extern RsslBuffer	loginUserName;

/* Gets the stream ID on which the provider received the login request. */
RsslInt32 wtfGetProviderLoginStream();

/*** Directory & Services ***/

/* Gets the stream ID on which the provider received the directory requests. */
RsslInt32 wtfGetProviderDirectoryStream();

/* Gets the filter the provider received on the directory requests (when the consumer is using
 * the watchlist, this should contain all the filters). */
RsslUInt32 wtfGetProviderDirectoryFilter();

/* Initializes a directory refresh. */
void wtfInitDefaultDirectoryRefresh(RsslRDMDirectoryRefresh *pDirectoryRefresh,
		RsslRDMService *pSingleService);

/* Most tests operate using a single default service, "Service1." */

/* Sets a RsslRDMService structure to the information used for Service 1.
 * Tests can use this as the base information and modify the state of the service. */
void wtfSetService1Info(RsslRDMService *pService);

/* The name of Service1. */
extern RsslBuffer	service1Name;

/* The ID of Service1. */
extern RsslUInt16	service1Id;

/* Most tests operate using a single default service, "Service2." */

/* Sets a RsslRDMService structure to the information used for Service 2.
 * Tests can use this as the base information and modify the state of the service. */
void wtfSetService2Info(RsslRDMService *pService);

/* The name of Service2. */
extern RsslBuffer	service2Name;

/* The ID of Service2. */
extern RsslUInt16	service2Id;

extern RsslBuffer activeUserName;
extern RsslBuffer standbyUserName;

/*** Views ***/

/* Decodes a view payload and tests it against the expected list of contained elements. */
void wtfProviderTestView(RsslRequestMsg *pRequestMsg, void *elemList, 
		RsslUInt32 elemCount, RsslUInt viewType, RsslUInt16 serverIndex = 0);

/* Returns a dictionary object populated by RDMFieldDictionary and enumtype.def. */
RsslDataDictionary *wtfGetDictionary();

/*** Symbol Lists. ***/

/* Describes a symbol list action to encode or decode. Used with wtfProviderEncodeSymbolList()
 * and wtfConsumerDecodeSymbolListDataBody() to test symbol list payloads. */
typedef struct
{
	RsslMapEntryActions	action;
	RsslBuffer 			itemName;
} WtfSymbolAction;

/* Encodes a symbol list payload. */
void wtfProviderEncodeSymbolListDataBody(RsslBuffer *pBuffer, WtfSymbolAction *symbolList, 
		RsslUInt32 symbolCount, RsslUInt16 serverIndex = 0);

/* Decodes a symbol list payload. */
void wtfConsumerDecodeSymbolListDataBody(RsslBuffer *pBuffer, WtfSymbolAction *symbolList, 
		RsslUInt32 symbolCount, RsslUInt16 serverIndex = 0);

typedef struct
{
	RsslUInt	viewType;				/* Type of view to encode, if any. */
	RsslInt		*viewFieldList;			/* For field ID views. */
	RsslBuffer	*viewElemList;			/* For element name views. */
	RsslUInt32	viewCount;				/* Number of elements in viewFieldList/viewElemList. */
	RsslBuffer	*batchItemList;			/* List of item names. */
	RsslUInt32	batchItemCount;			/* Number of names in batchItemList. */
	RsslUInt	slDataStreamFlags;		/* Symbol list data stream flags to encode, if any. */
} WtfConsumerRequestPayloadOptions;

RTR_C_INLINE void wtfClearConsumerRequestPayloadOptions(WtfConsumerRequestPayloadOptions *pOpts)
{
	memset(pOpts, 0, sizeof(WtfConsumerRequestPayloadOptions));
}

/* Encodes the given requestable payload data. Similar to other functions but this
 * allows testing combinations. */
void wtfConsumerEncodeRequestPayload(RsslBuffer *pBuffer, WtfConsumerRequestPayloadOptions *pOpts);

/* Encodes Symbol List Request Behaviors (recommend using wtfConsumerEncodeRequestPayload) */
void wtfConsumerEncodeSymbolListRequestBehaviors(RsslBuffer *pBuffer, RsslUInt slDataStreamFlags);

/* Encodes a batch request(recommend using wtfConsumerEncodeRequestPayload) */
void wtfConsumerEncodeBatchRequest(RsslBuffer *pBuffer, RsslBuffer* itemNameList, RsslUInt32 itemCount);

/* Encodes a view request(recommend using wtfConsumerEncodeRequestPayload) */
void wtfConsumerEncodeViewRequest(RsslUInt32 viewType, RsslBuffer *pBuffer, RsslInt* fidIdList, RsslBuffer* eItemNameList, RsslUInt32 itemCount);

/*** Encode MarketPrice payload. ***/

#define WTF_MP_RDNDISPLAY_FID 2
#define WTF_MP_RDN_EXCHID_FID 4
#define WTF_MP_DIVPAYDATE_FID 38
#define WTF_MP_TRDPRC_1_FID 6
#define WTF_MP_BID_FID 22
#define WTF_MP_ASK_FID 25
#define WTF_MP_ACVOL_1_FID 32
#define WTF_MP_NETCHNG_1_FID 11
#define WTF_MP_ASK_TIME_FID 267
#define WTF_MP_PERATIO_FID 36

typedef struct {
	RsslUInt		RDNDISPLAY;
	RsslEnum		RDN_EXCHID;
	RsslDate		DIVPAYDATE;
	RsslDouble		TRDPRC_1;
	RsslDouble		BID;
	RsslDouble		ASK;
	RsslDouble		ACVOL_1;
	RsslDouble		NETCHNG_1;
	RsslDateTime	ASK_TIME;
	RsslDouble		PERATIO;
} WtfMarketPriceItem;

RTR_C_INLINE void wtfClearMarketPriceItem(WtfMarketPriceItem* itemInfo)
{
	itemInfo->RDNDISPLAY = 0;
	itemInfo->RDN_EXCHID = 0;
	rsslClearDate(&itemInfo->DIVPAYDATE);
	itemInfo->TRDPRC_1 = 0;
	itemInfo->BID = 0;
	itemInfo->ASK = 0;
	itemInfo->ACVOL_1 = 0;
	itemInfo->NETCHNG_1 = 0;
	rsslClearDateTime(&itemInfo->ASK_TIME);
	itemInfo->PERATIO = 0;
}

void wtfInitMarketPriceItemFields(WtfMarketPriceItem* mpItem);

void wtfUpdateMarketPriceItemFields(WtfMarketPriceItem* mpItem);

RsslRet wtfProviderEncodeMarketPriceDataBody(RsslBuffer *pBuffer, WtfMarketPriceItem *marketPriceItem,
	RsslUInt16 index = 0);

RsslRet wtfProviderDecodeMarketPriceDataBody(RsslBuffer *pBuffer, WtfMarketPriceItem *marketPriceItem,
	RsslUInt16 index = 0);

#ifdef __cplusplus
};
#endif

#endif
