/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include <assert.h>

//#define WATCHLIST_TEST_FRAMEWORK_DEBUG

#define DISPATCH_EVENT_MAX 32

WtfGlobalConfig wtfGlobalConfig;
#define MCAST_FT_GROUP 0x02
#define MCAST_SEQNUM 0x04

typedef struct
{
	/* Connection type that this test series will use. */
	RsslConnectionTypes connType;

	/* Uses an RSSL-based Provider instead of a Reactor-based one. Used by multicast tests
	 * to send sequence numbers. */
	RsslBool useRawProvider;
} WtfConfig;

typedef enum
{
	WTF_ST_NONE,		/* Uninitialized state. */
	WTF_ST_INITIALIZED,	/* Framework initialized (Reactors started). */
	WTF_ST_CLEANUP		/* Framework is cleaning up (Reactors are being shut down). */
} WtfState;

typedef struct
{
	/* Configuration for the current series of tests. */
	WtfConfig config;

	/* Current framework state. */
	WtfState state;

	/* List of events received from callbacks from rsslReactorDispatch(). */
	WtfEvent eventList[DISPATCH_EVENT_MAX];
	RsslUInt32 eventCount; /* Number of events in eventList. */
	RsslUInt32 eventPos; /* When retrieving events, current position in eventList. */

	/* Reactor for provider channels. */
	RsslReactor *pProvReactor;

	/* Provider listening server. */
	RsslServer *pServer;

	/* Reactor for consumer channels. */
	RsslReactor *pConsReactor;

	/* Current provider channel. */
	RsslReactorChannel *pProvReactorChannel;

	/* Current provider channel (if using raw provider). */
	RsslChannel *pRawProvChannel;

	/* Current consumer channel. */
	RsslReactorChannel *pConsReactorChannel;

	/* Notes whether the consumer channel has received RSSL_RC_CET_CHANNEL_UP. */
	RsslBool consReactorChannelUp;

	/* Provider Reactor connection config. */
	RsslReactorOMMProviderRole	ommProviderRole;

	/* Consumer Reactor connection config. */
	RsslReactorOMMConsumerRole	ommConsumerRole;

	/* Stores login stream ID the provider received when the framework setup the login message
	 * exchange. */
	RsslInt32 providerLoginStreamId;

	/* Stores directory stream ID the provider received when the framework setup the directory
	 * message exchange. */
	RsslInt32 providerDirectoryStreamId;

	/* Stores directory filter the provider received when the framework setup the directory message
	 * exchange. */
	RsslUInt32 providerDirectoryFilter;

	void (*consumerMsgCallbackAction)();
	WtfCallbackAction consumerLoginCallbackAction;
	WtfCallbackAction consumerDirectoryCallbackAction;
	WtfCallbackAction consumerDictionaryCallbackAction;
	WtfCallbackAction providerDictionaryCallbackAction;

} WatchlistTestFramework;

/* Represents the currently-used framework. */
WatchlistTestFramework wtf;



RsslBuffer loginUserName = { 15, const_cast<char*>("watchlistTester")};

RsslBuffer service1Name = { 8, const_cast<char*>("service1") };
RsslUInt16 service1Id = 1;

static RsslUInt	service1CapabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE, 
	RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_SYMBOL_LIST };
static RsslUInt32 service1CapabilitiesCount = 4;

/* Note: QoS is purposefully ordered worst to best here, to test service QoS sorting. 
 * Make sure the sorting is still tested before changing this. */
static RsslQos service1QosList[] = {{RSSL_QOS_TIME_DELAYED, RSSL_QOS_RATE_JIT_CONFLATED, 0, 0, 0},
	{RSSL_QOS_TIME_REALTIME, RSSL_QOS_RATE_TICK_BY_TICK, 0, 0, 0}};
static RsslUInt32 service1QosCount = 2;

/* Unicast ports for multicast channels.
 * These are incrmented during each connection, to workaround occasional cases where the Reactor 
 * didn't close the channel before the next test started. */
static RsslUInt16 unicastProvPort = 20000;
static RsslUInt16 unicastConsPort = 30000;


RsslDataDictionary dataDictionary;

RsslRDMMsg *rdmMsgCreateCopy(RsslRDMMsg *pRdmMsg)
{

	RsslUInt32 length = 128 + sizeof(RsslRDMMsg);

	for(;;)
	{
		RsslRDMMsg *pNewRdmMsg;
		RsslRet ret;
		char *pData;
		RsslBuffer memoryBuffer;

		pData = (char*)malloc(length);
		pNewRdmMsg = (RsslRDMMsg*)pData;
		memoryBuffer.data = pData + sizeof(RsslRDMMsg);
		memoryBuffer.length = length - sizeof(RsslRDMMsg);

		ret = rsslCopyRDMMsg(pNewRdmMsg, pRdmMsg, &memoryBuffer);

		if (ret == RSSL_RET_SUCCESS) 
			return pNewRdmMsg;
		switch(ret)
		{
			case RSSL_RET_BUFFER_TOO_SMALL:
				free(pData);
				length *= 2;
				break;
			default:
				free(pData);
				return NULL;

		}
	}
}

void wtfRdmMsgEventInit(WtfRdmMsgEvent *pEvent, WtfComponent component,
		RsslRDMMsg *pRdmMsg)
{
	pEvent->base.type = WTF_DE_RDM_MSG;
	pEvent->base.component = component;
	pEvent->base.timeUsec = rsslGetTimeMicro();
	pEvent->pRdmMsg = rdmMsgCreateCopy(pRdmMsg);
	pEvent->pUserSpec = NULL;
	pEvent->pServiceName = NULL;
}

RsslBuffer *createBufferCopy(const RsslBuffer *pBuffer)
{
	RsslBuffer *pNewBuffer = (RsslBuffer*)malloc(sizeof(RsslBuffer));
	pNewBuffer->length = pBuffer->length;
	pNewBuffer->data = (char*)malloc(pBuffer->length);
	memcpy(pNewBuffer->data, pBuffer->data, pBuffer->length);
	return pNewBuffer;
}

/* Default Msg Callback. */
static RsslReactorCallbackRet msgCallback(RsslReactor* pReactor, 
		RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent)
{
	WtfComponent component = *(WtfComponent*)&pReactor->userSpecPtr;
	WtfRsslMsgEvent *pRsslMsgEvent;
	EXPECT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);

	/* Buffer is not modified by the watchlist, and so should not be forwarded. */
	if (component == WTF_TC_CONSUMER)
	  EXPECT_EQ(NULL, pEvent->pRsslMsgBuffer);

	/* Copy message. */
	pRsslMsgEvent = &wtf.eventList[wtf.eventCount].rsslMsg;
	wtfRsslMsgEventInit(pRsslMsgEvent, component, pEvent->pRsslMsg);

	if (pEvent->pStreamInfo)
	{
		pRsslMsgEvent->pUserSpec = pEvent->pStreamInfo->pUserSpec;

		if (pEvent->pStreamInfo->pServiceName)
			pRsslMsgEvent->pServiceName = createBufferCopy(pEvent->pStreamInfo->pServiceName);
	}

	if (pEvent->pSeqNum)
	{
		pRsslMsgEvent->hasSeqNum = RSSL_TRUE;
		pRsslMsgEvent->seqNum = *pEvent->pSeqNum;
	}

	++wtf.eventCount;


	if (wtf.consumerMsgCallbackAction)
		wtf.consumerMsgCallbackAction();

	return RSSL_RC_CRET_SUCCESS;
}

/* Channel Event Callback. */
static RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, 
		RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	WtfComponent component = *(WtfComponent*)&pReactor->userSpecPtr;
	WtfChannelEvent *pChannelEvent;

	EXPECT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);

	/* Copy message. */
	pChannelEvent = &wtf.eventList[wtf.eventCount].channelEvent;
	wtfClearChannelEvent(pChannelEvent, component);
	pChannelEvent->channelEventType = pEvent->channelEventType;
		
	++wtf.eventCount;

	if (wtf.state == WTF_ST_CLEANUP)
	{
	  EXPECT_EQ(pEvent->channelEventType, RSSL_RC_CET_CHANNEL_DOWN);
	  return RSSL_RC_CRET_SUCCESS;
	}

	if (pEvent->pError)
		pChannelEvent->rsslErrorId = pEvent->pError->rsslError.rsslErrorId;

	switch(pEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_OPENED: 
			/* Set consumer channel. */
			switch(component)
			{
				case WTF_TC_CONSUMER:
				  wtf.pConsReactorChannel = pReactorChannel;
				  break;
				case WTF_TC_PROVIDER:
				  EXPECT_TRUE(0) << "unexpected event type: provider received channel open"
								 << std::endl;
				  break;
			}
			break;

		case RSSL_RC_CET_CHANNEL_UP:
			/* Set provider channel. */
			switch(component)
			{
				case WTF_TC_PROVIDER:
				  wtf.pProvReactorChannel = pReactorChannel;
				  break;
				case WTF_TC_CONSUMER:
				  wtf.consReactorChannelUp = RSSL_TRUE;
				  break;
			}
			break;

		case RSSL_RC_CET_CHANNEL_READY:
			break;

		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
			if (component == WTF_TC_CONSUMER)
			  wtf.consReactorChannelUp = RSSL_FALSE;
			break;

		case RSSL_RC_CET_CHANNEL_DOWN:
			/* Reset channel. */
			switch(component) {
			case WTF_TC_PROVIDER:
			  wtfCloseChannel(WTF_TC_PROVIDER);
			  wtf.pProvReactorChannel = NULL;
			  break;
			case WTF_TC_CONSUMER:
			  wtfCloseChannel(WTF_TC_CONSUMER);
			  wtf.pConsReactorChannel = NULL;
			  wtf.consReactorChannelUp = RSSL_FALSE;
			  break;
			}
			break;
		default:
		  EXPECT_TRUE(0) << "unhandled channel event" << std::endl;
		  break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Login Callback. */
static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, 
		RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent *pInfo)
{
	WtfComponent component = *(WtfComponent*)&pReactor->userSpecPtr;
	WtfRdmMsgEvent *pRdmMsgEvent;
	EXPECT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);

	/* When provided as RDM messages, buffer & msg are not modified, and so should not be 
	 * forwarded. */
	if (component == WTF_TC_CONSUMER)
	{
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsgBuffer);
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsg);

		if (wtf.consumerLoginCallbackAction == WTF_CB_RAISE_TO_DEFAULT_CB)
			return RSSL_RC_CRET_RAISE;
	}

	/* Copy Login message. */
	pRdmMsgEvent = &wtf.eventList[wtf.eventCount].rdmMsg;
	wtfRdmMsgEventInit(pRdmMsgEvent, component, (RsslRDMMsg*)pInfo->pRDMLoginMsg);
	if (pInfo->baseMsgEvent.pStreamInfo)
	{	
		pRdmMsgEvent->pUserSpec = pInfo->baseMsgEvent.pStreamInfo->pUserSpec;

		if (pInfo->baseMsgEvent.pStreamInfo->pServiceName)
		{
			pRdmMsgEvent->pServiceName = createBufferCopy(
					pInfo->baseMsgEvent.pStreamInfo->pServiceName);
		}
	}

	++wtf.eventCount;
	return RSSL_RC_CRET_SUCCESS;
}

/* Directory Callback. */
static RsslReactorCallbackRet directoryMsgCallback(RsslReactor* pReactor, 
		RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent *pInfo)
{
	WtfComponent component = *(WtfComponent*)&pReactor->userSpecPtr;
	WtfRdmMsgEvent *pRdmMsgEvent;
	EXPECT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);

	/* When provided as RDM messages, buffer & msg are not modified, and so should not be 
	 * forwarded. */
	if (component == WTF_TC_CONSUMER)
	{
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsgBuffer);
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsg);

		if (wtf.consumerDirectoryCallbackAction == WTF_CB_RAISE_TO_DEFAULT_CB)
			return RSSL_RC_CRET_RAISE;
	}

	/* Copy Directory message. */
	pRdmMsgEvent = &wtf.eventList[wtf.eventCount].rdmMsg;
	wtfRdmMsgEventInit(pRdmMsgEvent, component, (RsslRDMMsg*)pInfo->pRDMDirectoryMsg);
	if (pInfo->baseMsgEvent.pStreamInfo)
	{
		pRdmMsgEvent->pUserSpec = pInfo->baseMsgEvent.pStreamInfo->pUserSpec;

		if (pInfo->baseMsgEvent.pStreamInfo->pServiceName)
		{
			pRdmMsgEvent->pServiceName = createBufferCopy(
					pInfo->baseMsgEvent.pStreamInfo->pServiceName);
		}
	}


	++wtf.eventCount;
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, 
		RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent *pInfo)
{
	WtfComponent component = *(WtfComponent*)&pReactor->userSpecPtr;
	WtfRdmMsgEvent *pRdmMsgEvent;
	EXPECT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);

	/* When provided as RDM messages, buffer & msg are not modified, and so should not be 
	 * forwarded. */
	if (component == WTF_TC_CONSUMER)
	{
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsgBuffer);
		EXPECT_TRUE(!pInfo->baseMsgEvent.pRsslMsg);

		if (wtf.consumerDictionaryCallbackAction == WTF_CB_RAISE_TO_DEFAULT_CB)
			return RSSL_RC_CRET_RAISE;
	}
	else
	{
		if (wtf.providerDictionaryCallbackAction == WTF_CB_RAISE_TO_DEFAULT_CB)
			return RSSL_RC_CRET_RAISE;
	}

	/* Copy Dictionary message. */
	pRdmMsgEvent = &wtf.eventList[wtf.eventCount].rdmMsg;
	wtfRdmMsgEventInit(pRdmMsgEvent, component, (RsslRDMMsg*)pInfo->pRDMDictionaryMsg);
	if (pInfo->baseMsgEvent.pStreamInfo)
	{
		pRdmMsgEvent->pUserSpec = pInfo->baseMsgEvent.pStreamInfo->pUserSpec;

		if (pInfo->baseMsgEvent.pStreamInfo)
		{
			pRdmMsgEvent->pUserSpec = pInfo->baseMsgEvent.pStreamInfo->pUserSpec;

			if (pInfo->baseMsgEvent.pStreamInfo->pServiceName)
			{
				pRdmMsgEvent->pServiceName = createBufferCopy(
						pInfo->baseMsgEvent.pStreamInfo->pServiceName);
			}
		}
	}

	++wtf.eventCount;
	return RSSL_RC_CRET_SUCCESS;
}

void wtfInit(WtfInitOpts *pOpts, RsslUInt32 maxOutputBufSize)
{
	RsslCreateReactorOptions	reactorOpts;
	RsslErrorInfo				rsslErrorInfo;
	char 						errorString[256];
	RsslBuffer					errorText = { 256, errorString };
	WtfInitOpts					defaultOpts;
	RsslReactorJsonConverterOptions jsonConverterOptions;

	wtf.state = WTF_ST_NONE;
	wtf.pServer = NULL;
	wtf.pProvReactor = NULL;
	wtf.pConsReactor = NULL;
	wtf.pProvReactorChannel = NULL;
	wtf.pRawProvChannel = NULL;
	wtf.pConsReactorChannel = NULL;
	wtf.consReactorChannelUp = RSSL_FALSE;
	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslErrorInfo.rsslError);

#ifdef WIN32
	/* Server might have been closed recently. Give windows a chance to release it. */
	Sleep(200);
#endif

	if (!pOpts)
	{
		pOpts = &defaultOpts;
		wtfClearInitOpts(pOpts);
	}

	wtf.config.useRawProvider = pOpts->useRawProvider;
	wtf.config.connType = pOpts->connectionType;

	rsslClearCreateReactorOptions(&reactorOpts);
	reactorOpts.userSpecPtr = (void*)WTF_TC_PROVIDER;
	wtf.pProvReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo);

	reactorOpts.userSpecPtr = (void*)WTF_TC_CONSUMER;
	wtf.pConsReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo);

	rsslClearDataDictionary(&dataDictionary);
	std::string dictionaryName("RDMFieldDictionary");
	ASSERT_EQ(rsslLoadFieldDictionary(dictionaryName.c_str(), &dataDictionary, &errorText),
			  RSSL_RET_SUCCESS) << "failed to load field dictionary from file named " <<
	  dictionaryName << std::endl;
	dictionaryName = "enumtype.def";
	ASSERT_EQ(rsslLoadEnumTypeDictionary(dictionaryName.c_str(), &dataDictionary, &errorText),
			  RSSL_RET_SUCCESS) << "failed to load enum dictionary from file named " <<
	  dictionaryName << std::endl;

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);

	jsonConverterOptions.pDictionary = &dataDictionary;
	jsonConverterOptions.defaultServiceId = 1;

	if (maxOutputBufSize != 0)
	{
		jsonConverterOptions.outputBufferSize = maxOutputBufSize;
	}

	ASSERT_TRUE(rsslReactorInitJsonConverter(wtf.pProvReactor, &jsonConverterOptions, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslReactorInitJsonConverter(wtf.pConsReactor, &jsonConverterOptions, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	wtf.state = WTF_ST_INITIALIZED;
}

void wtfCleanup()
{
	RsslErrorInfo rsslErrorInfo;

	assert(wtf.pProvReactor);
	assert(wtf.pConsReactor);

	wtf.state = WTF_ST_CLEANUP;

	rsslDestroyReactor(wtf.pProvReactor, &rsslErrorInfo); wtf.pProvReactor = NULL;
	rsslDestroyReactor(wtf.pConsReactor, &rsslErrorInfo); wtf.pConsReactor = NULL;

	rsslDeleteDataDictionary(&dataDictionary);

	wtf.pProvReactorChannel = NULL;
	wtf.pConsReactorChannel = NULL;

	rsslUninitialize();
}

void wtfBindServer(RsslConnectionTypes connectionType)
{
	RsslBindOptions				bindOpts;
	RsslErrorInfo				rsslErrorInfo;

	/* Bind Provider. */
	rsslClearBindOpts(&bindOpts); bindOpts.serviceName = const_cast<char*>("14011");

	if(connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		bindOpts.wsOpts.protocols = const_cast<char*>("rssl.json.v2");

	wtf.pServer = rsslBind(&bindOpts, &rsslErrorInfo.rsslError);
	bindOpts.pingTimeout = bindOpts.minPingTimeout = 30;
}

void wtfCloseServer()
{
	RsslErrorInfo				rsslErrorInfo;
	rsslCloseServer(wtf.pServer, &rsslErrorInfo.rsslError);
	wtf.pServer = NULL;
}

void wtfConnect(WtfSetupConnectionOpts *pOpts)
{
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorConnectOptions connectOpts;

	rsslClearReactorConnectOptions(&connectOpts);
	connectOpts.rsslConnectOptions.connectionType = wtf.config.connType;
	connectOpts.reconnectAttemptLimit = pOpts->reconnectAttemptLimit;
	connectOpts.reconnectMinDelay = pOpts->reconnectMinDelay;
	connectOpts.reconnectMaxDelay = pOpts->reconnectMaxDelay;


	connectOpts.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");

	if (connectOpts.rsslConnectOptions.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		connectOpts.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}


	if ((ret = rsslReactorConnect(wtf.pConsReactor, &connectOpts, 
					(RsslReactorChannelRole*)&wtf.ommConsumerRole, &rsslErrorInfo) != RSSL_RET_SUCCESS))
	{
		printf("wtfConnect: rsslReactorConnect() failed: %d(%s -- %s)\n",
				ret,
				rsslErrorInfo.errorLocation,
				rsslErrorInfo.rsslError.text);
		ASSERT_TRUE(0);
	}
}

void wtfAccept()
{
	wtfAcceptWithTime(5000);
}

void wtfAcceptWithTime(time_t timeoutMsec)
{
	/* Wait for server descriptor trigger before accepting. 
	* If using a Reactor for the provider, also checks reactor descriptor. */

	time_t currentTimeUsec, stopTimeUsec;

	ASSERT_TRUE(wtf.pServer != NULL);

	currentTimeUsec = rsslGetTimeMicro();

	stopTimeUsec =  (time_t)(timeoutMsec / wtfGlobalConfig.speed);
	stopTimeUsec *= 1000;
	stopTimeUsec += currentTimeUsec;

	do
	{
		int selectRet;
		struct timeval selectTime;
		fd_set readFds;
		time_t timeoutUsec;

		FD_ZERO(&readFds);

		if (!wtf.config.useRawProvider)
			FD_SET(wtf.pProvReactor->eventFd, &readFds);

		FD_SET(wtf.pServer->socketId, &readFds);

		timeoutUsec = (currentTimeUsec < stopTimeUsec) ? 
			(stopTimeUsec - currentTimeUsec) : (time_t)0;

		selectTime.tv_sec = (long)timeoutUsec/1000000;
		selectTime.tv_usec = (long)(timeoutUsec - selectTime.tv_sec * 1000000);
		selectRet = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);

		ASSERT_TRUE(selectRet > 0);

		if (!wtf.config.useRawProvider && FD_ISSET(wtf.pProvReactor->eventFd, &readFds))
		{
			/* Reactor descriptor triggered. Make sure we didn't get anything. */
			wtfDispatch(WTF_TC_PROVIDER, 100);
			ASSERT_TRUE(!wtfGetEvent());
		}
		else if (FD_ISSET(wtf.pServer->socketId, &readFds))
		{
			/* Server descriptor triggered, ready to accept. */
			break;
		}
		currentTimeUsec = rsslGetTimeMicro();

	} while(currentTimeUsec <= stopTimeUsec);

	if (currentTimeUsec > stopTimeUsec)
	{
		printf("wtfAccept: No accept notification.\n" );
		ASSERT_TRUE(0);
	}

	if (!wtf.config.useRawProvider)
	{
		RsslReactorAcceptOptions	acceptOpts;
		RsslErrorInfo rsslErrorInfo;

		rsslClearReactorAcceptOptions(&acceptOpts);

		ASSERT_TRUE(!wtf.pProvReactorChannel);
		ASSERT_TRUE(rsslReactorAccept(wtf.pProvReactor, wtf.pServer, &acceptOpts, 
					(RsslReactorChannelRole*)&wtf.ommProviderRole, &rsslErrorInfo) == 
				RSSL_RET_SUCCESS);
	}
	else
	{
		/* Raw provider; connect directly. */
		RsslAcceptOptions acceptOpts;
		RsslError rsslError;

		rsslClearAcceptOpts(&acceptOpts);

		assert(!wtf.pRawProvChannel);
		ASSERT_TRUE((wtf.pRawProvChannel = rsslAccept(wtf.pServer, &acceptOpts, &rsslError)));
	}
}

void wtfCloseChannel(WtfComponent component)
{
	RsslErrorInfo rsslErrorInfo;

	switch(component)
	{
		case WTF_TC_CONSUMER: 
			ASSERT_TRUE(rsslReactorCloseChannel(wtf.pConsReactor, wtf.pConsReactorChannel, 
						&rsslErrorInfo) == RSSL_RET_SUCCESS);
			wtf.pConsReactorChannel = NULL; 
			break;
		case WTF_TC_PROVIDER: 
			if (wtf.config.useRawProvider)
			{
				ASSERT_TRUE(rsslCloseChannel(wtf.pRawProvChannel, &rsslErrorInfo.rsslError)
						== RSSL_RET_SUCCESS);
				wtf.pRawProvChannel = NULL;
			}
			else
			{
				ASSERT_TRUE(rsslReactorCloseChannel(wtf.pProvReactor, wtf.pProvReactorChannel, 
							&rsslErrorInfo) == RSSL_RET_SUCCESS);
				wtf.pProvReactorChannel = NULL;
			}
			break;
		default: assert(0); break;
	}
}

void wtfCleanupEvents()
{
	RsslUInt32 i;
	for(i = 0; i < wtf.eventCount; ++i)
	{
		/* Clean up any received msg event memory. */
		switch(wtf.eventList[i].base.type)
		{
			case WTF_DE_RSSL_MSG: wtfRsslMsgEventCleanup(&wtf.eventList[i].rsslMsg); break;
			case WTF_DE_RDM_MSG: wtfRdmMsgEventCleanup(&wtf.eventList[i].rdmMsg); break;
		}
	}
	wtf.eventCount = 0;
	wtf.eventPos = 0;
}

void wtfDispatch(WtfComponent component, time_t timeoutMsec)
{
	/* Event requires dispatching. */

	int selectRet;
	time_t currentTimeUsec, stopTimeUsec;

	RsslReactorChannel *pChannel;
	RsslReactor *pReactor;
	RsslRet lastDispatchRet = 0;

	/* Ensure no events were missed from previous calls to wtfDispatch. */
	EXPECT_EQ(wtf.eventPos, wtf.eventCount) << "wtfDispatch with component "
											<< (component == WTF_TC_PROVIDER ? "provider" :
												(component == WTF_TC_CONSUMER ? "consumer" :
												 "unknown")) << std::endl;
	wtfCleanupEvents();

	switch(component)
	{
		case WTF_TC_PROVIDER: 
		{
			if (!wtf.config.useRawProvider)
			{
				pReactor = wtf.pProvReactor; 
				break;
			}
			else
			{
				wtfDispatchRaw(wtf.pRawProvChannel, WTF_TC_PROVIDER, timeoutMsec);
				return;
			}
		}
		case WTF_TC_CONSUMER: pReactor = wtf.pConsReactor; break;
		default: assert(0); break;
	}

	currentTimeUsec = rsslGetTimeMicro();

	
	stopTimeUsec =  (time_t)(timeoutMsec / wtfGlobalConfig.speed);
	stopTimeUsec *= 1000;
	stopTimeUsec += currentTimeUsec;

	do
	{
		/* Channel events can result in opening/closing the channel. So reset this each time. */
		switch(component)
		{
			case WTF_TC_PROVIDER: pChannel = wtf.pProvReactorChannel; break;
			case WTF_TC_CONSUMER: 
				/* Watchlist gives consumer channel before it is ready.
				 * Do not select() on it if it isn't ready. */
				pChannel = wtf.consReactorChannelUp ? wtf.pConsReactorChannel: NULL; break;
			default: assert(0); break;
		}

		if (lastDispatchRet == 0)
		{
			struct timeval selectTime;
			fd_set readFds;
			time_t timeoutUsec;

			FD_ZERO(&readFds);

			FD_SET(pReactor->eventFd, &readFds);

			if (pChannel)
				FD_SET(pChannel->socketId, &readFds);

			timeoutUsec = (currentTimeUsec < stopTimeUsec) ? 
				(stopTimeUsec - currentTimeUsec) : (time_t)0;

			selectTime.tv_sec = (long)timeoutUsec/1000000;
			selectTime.tv_usec = (long)(timeoutUsec - selectTime.tv_sec * 1000000);
			selectRet = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);
		}
		else
			selectRet = 1;


		if (selectRet > 0)
		{
			RsslErrorInfo rsslErrorInfo;
			RsslReactorDispatchOptions dispatchOpts;

			rsslClearReactorDispatchOptions(&dispatchOpts);
			lastDispatchRet = rsslReactorDispatch(pReactor, &dispatchOpts, 
					&rsslErrorInfo);

			if (lastDispatchRet < 0)
			{
				printf("rsslReactorDispatch() failed: %d(%s -- %s)\n",
						lastDispatchRet,
						rsslErrorInfo.errorLocation,
						rsslErrorInfo.rsslError.text);
				ASSERT_TRUE(0);
			}
		}

		currentTimeUsec = rsslGetTimeMicro();

	} while(currentTimeUsec < stopTimeUsec);
}

static void wtfDispatchRaw(RsslChannel *pChannel, WtfComponent component, time_t timeoutMsec)
{
	int selectRet;
	time_t currentTimeUsec, stopTimeUsec;
	struct timeval selectTime;
	fd_set readFds;
	time_t timeoutUsec;
	RsslInProgInfo inProg;

	currentTimeUsec = rsslGetTimeMicro();
	stopTimeUsec =  (time_t)(timeoutMsec / wtfGlobalConfig.speed);
	stopTimeUsec *= 1000;
	stopTimeUsec += currentTimeUsec;

	do
	{
		FD_ZERO(&readFds);
		FD_SET(pChannel->socketId, &readFds);

		timeoutUsec = (currentTimeUsec < stopTimeUsec) ? 
			(stopTimeUsec - currentTimeUsec) : (time_t)0;

		selectTime.tv_sec = (long)timeoutUsec/1000000;
		selectTime.tv_usec = (long)(timeoutUsec - selectTime.tv_sec * 1000000);
		selectRet = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);

		if (selectRet > 0)
		{
			RsslError rsslError;
			RsslRet ret;
			WtfEvent *pEvent;

			switch(pChannel->state)
			{
				case RSSL_CH_STATE_INITIALIZING:

					ASSERT_TRUE((ret = rsslInitChannel(pChannel, &inProg, &rsslError)) 
							>= RSSL_RET_SUCCESS);

					switch (ret)
					{
						case RSSL_RET_SUCCESS:
							/* Channel is up. */
							ASSERT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);
							pEvent = &wtf.eventList[wtf.eventCount++];
							wtfClearChannelEvent(&pEvent->channelEvent, component);
							pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_UP;
							pEvent->channelEvent.rsslErrorId = RSSL_RET_SUCCESS;

							/* Channel is ready. */
							ASSERT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);
							pEvent = &wtf.eventList[wtf.eventCount++];
							wtfClearChannelEvent(&pEvent->channelEvent, component);
							pEvent->channelEvent.channelEventType = RSSL_RC_CET_CHANNEL_READY;
							pEvent->channelEvent.rsslErrorId = RSSL_RET_SUCCESS;
							break;

						case RSSL_RET_CHAN_INIT_IN_PROGRESS:
						case RSSL_RET_READ_FD_CHANGE:
							break;

						default:
							ASSERT_TRUE(0);
							break;
					}



					break;

				case RSSL_CH_STATE_ACTIVE:
				{
					RsslBuffer *pBuffer;

					pBuffer = rsslRead(pChannel, &ret, &rsslError);

					if (pBuffer)
					{
						RsslMsg msg;
						RsslDecodeIterator dIter;

						RsslBuffer tmpMem;
						RsslRDMMsg rdmMsg;
						RsslErrorInfo rsslErrorInfo;

						tmpMem.length = 6000;
						tmpMem.data = (char*)alloca(6000);

						rsslClearDecodeIterator(&dIter);
						rsslSetDecodeIteratorBuffer(&dIter, pBuffer);
						rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion,
								pChannel->minorVersion);

						/* Decodes RsslMsg (and RsslRDMMsg, if appropriate domain). */
						ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &msg, &rdmMsg, &tmpMem,
									&rsslErrorInfo) == RSSL_RET_SUCCESS);

						ASSERT_TRUE(wtf.eventCount < DISPATCH_EVENT_MAX);
						pEvent = &wtf.eventList[wtf.eventCount++];

						/* Raw dispatches will always produce RDMMsgs for admin domains. */
						switch(msg.msgBase.domainType)
						{
							case RSSL_DMT_LOGIN:
							case RSSL_DMT_SOURCE:
							case RSSL_DMT_DICTIONARY:
							{
								wtfRdmMsgEventInit(&pEvent->rdmMsg, component, &rdmMsg);
								break;
							}

							default:
								wtfRsslMsgEventInit(&pEvent->rsslMsg, component, &msg);
								break;
						}
					}
					break;
				}
				default:
					ASSERT_TRUE(0); 
					break;
			}
		}

		currentTimeUsec = rsslGetTimeMicro();
	} while (currentTimeUsec < stopTimeUsec);
}



WtfEvent *wtfGetEvent()
{ 
	return (wtf.eventPos < wtf.eventCount) ? &wtf.eventList[wtf.eventPos++] : NULL; 
}

bool wtfStartTestInt()
{
  if (wtf.state == WTF_ST_INITIALIZED) {
	wtf.eventCount = 0;
	wtf.eventPos = 0;
	wtf.consReactorChannelUp = RSSL_FALSE;
	return true;
  }
  return false;
}

void wtfFinishTestInt()
{
	ASSERT_TRUE(wtf.eventCount == wtf.eventPos);

	wtfCleanupEvents();

	if (wtf.pConsReactorChannel)
		wtfCloseChannel(WTF_TC_CONSUMER);

	if (wtf.pProvReactorChannel || wtf.pRawProvChannel)
		wtfCloseChannel(WTF_TC_PROVIDER);
}

void wtfInitDefaultLoginRequest(RsslRDMLoginRequest *pLoginRequest)
{
	rsslInitDefaultRDMLoginRequest(pLoginRequest, WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	pLoginRequest->userName = loginUserName;
}

void wtfInitDefaultLoginRefresh(RsslRDMLoginRefresh *pLoginRefresh)
{
	rsslClearRDMLoginRefresh(pLoginRefresh);
	pLoginRefresh->flags |= RDM_LG_RFF_SOLICITED;
	pLoginRefresh->userName = loginUserName;
	pLoginRefresh->rdmMsgBase.streamId = wtf.providerLoginStreamId;
	pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	pLoginRefresh->supportOptimizedPauseResume = 1;
	pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	pLoginRefresh->supportViewRequests = 1;
}

void wtfInitDefaultDirectoryRefresh(RsslRDMDirectoryRefresh *pDirectoryRefresh,
		RsslRDMService *pSingleService)
{
	rsslClearRDMDirectoryRefresh(pDirectoryRefresh);
	pDirectoryRefresh->rdmMsgBase.streamId = wtf.providerDirectoryStreamId;
	pDirectoryRefresh->filter = wtf.providerDirectoryFilter;
	pDirectoryRefresh->flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	pDirectoryRefresh->serviceList = pSingleService;
	pDirectoryRefresh->serviceCount = 1;

	wtfSetService1Info(pSingleService);
}

void wtfSetupConnection(WtfSetupConnectionOpts *pOpts, RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions submitOpts;
	WtfEvent *pEvent;
	RsslRDMLoginRequest loginRequest;
	RsslRDMLoginRefresh loginRefresh;

	WtfSetupConnectionOpts defaultCalOpts;

	if (pOpts == NULL)
	{
		wtfClearSetupConnectionOpts(&defaultCalOpts);
		pOpts = &defaultCalOpts;
	}

	rsslClearOMMConsumerRole(&wtf.ommConsumerRole);
	wtf.ommConsumerRole.base.defaultMsgCallback = msgCallback;
	wtf.ommConsumerRole.base.channelEventCallback = channelEventCallback;

	wtf.consumerLoginCallbackAction = pOpts->consumerLoginCallback;
	wtf.consumerDirectoryCallbackAction = pOpts->consumerDirectoryCallback;
	wtf.consumerDictionaryCallbackAction = pOpts->consumerDictionaryCallback;
	wtf.providerDictionaryCallbackAction = pOpts->providerDictionaryCallback;

	if (pOpts->consumerLoginCallback != WTF_CB_NONE)
		wtf.ommConsumerRole.loginMsgCallback = loginMsgCallback;
	if (pOpts->consumerDirectoryCallback != WTF_CB_NONE)
		wtf.ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	if (pOpts->consumerDictionaryCallback != WTF_CB_NONE)
		wtf.ommConsumerRole.dictionaryMsgCallback = dictionaryMsgCallback;

	rsslClearOMMProviderRole(&wtf.ommProviderRole);
	wtf.ommProviderRole.base.defaultMsgCallback = msgCallback;
	wtf.ommProviderRole.base.channelEventCallback = channelEventCallback;
	wtf.ommProviderRole.loginMsgCallback = loginMsgCallback;
	wtf.ommProviderRole.directoryMsgCallback = directoryMsgCallback;

	if (pOpts->providerDictionaryCallback != WTF_CB_NONE)
		wtf.ommProviderRole.dictionaryMsgCallback = dictionaryMsgCallback;
	
	/* Consumer connects. */
	wtf.ommConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	wtf.ommConsumerRole.watchlistOptions.channelOpenCallback = channelEventCallback;
	wtf.ommConsumerRole.watchlistOptions.requestTimeout = pOpts->requestTimeout;
	wtf.ommConsumerRole.watchlistOptions.postAckTimeout = pOpts->postAckTimeout;

	/* wtfDispatch() multiplies times less than 1 second. So set
	 * requestTimeout/postAckTimeout accordingly. */
	wtf.ommConsumerRole.watchlistOptions.requestTimeout = 
		(RsslUInt32)((float)wtf.ommConsumerRole.watchlistOptions.requestTimeout / wtfGlobalConfig.speed);
	wtf.ommConsumerRole.watchlistOptions.postAckTimeout = 
		(RsslUInt32)((float)wtf.ommConsumerRole.watchlistOptions.postAckTimeout / wtfGlobalConfig.speed);

	wtf.config.connType = connectionType;
	wtfConnect(pOpts);

	/* Consumer receives channel open. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_OPENED);

	if (!pOpts->login)
		return;

	/* Consumer submits login. */
	wtfInitDefaultLoginRequest(&loginRequest);

	if (!pOpts->singleOpen)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
		loginRequest.singleOpen = 0;
	}
	if (!pOpts->allowSuspectData)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA;
		loginRequest.allowSuspectData = 0;
	}

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	submitOpts.requestMsgOptions.pUserSpec = (void*)WTF_DEFAULT_LOGIN_USER_SPEC_PTR;
	wtfSubmitMsg(&submitOpts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	if (!pOpts->accept)
		return;

	/* Provider should now accept. */
	wtfAccept();

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200);
	while(!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);


	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	wtf.providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	if (!pOpts->provideLoginRefresh)
		return;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pOpts->consumerLoginCallback == WTF_CB_USE_DOMAIN_CB)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1); 
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}
	else
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.streamId == 1); 
		ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	wtf.providerDirectoryFilter = 
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter; 
	ASSERT_TRUE(wtf.providerDirectoryFilter ==
				(RDM_DIRECTORY_SERVICE_INFO_FILTER
				| RDM_DIRECTORY_SERVICE_STATE_FILTER
				| RDM_DIRECTORY_SERVICE_GROUP_FILTER
				| RDM_DIRECTORY_SERVICE_LOAD_FILTER
				| RDM_DIRECTORY_SERVICE_DATA_FILTER
				| RDM_DIRECTORY_SERVICE_LINK_FILTER));
	
	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags 
				& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags 
				& RDM_DR_RQF_STREAMING));

	wtf.providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	if (pOpts->provideDefaultDirectory)
	{
		RsslRDMDirectoryRefresh directoryRefresh;
		RsslRDMService service;

		rsslClearRDMDirectoryRefresh(&directoryRefresh);
		directoryRefresh.rdmMsgBase.streamId = wtf.providerDirectoryStreamId;
		directoryRefresh.filter = wtf.providerDirectoryFilter;
		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		wtfSetService1Info(&service);

		rsslClearReactorSubmitMsgOptions(&submitOpts);
		submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
		wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	}

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(wtf.eventCount == 0);
	
}

void wtfSetService1Info(RsslRDMService *pService)
{
	rsslClearRDMService(pService);
	pService->flags |= RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
	pService->serviceId = service1Id;

	pService->info.serviceName = service1Name;

	pService->info.capabilitiesCount = service1CapabilitiesCount;
	pService->info.capabilitiesList = service1CapabilitiesList;

	pService->info.flags |= RDM_SVC_IFF_HAS_QOS;
	pService->info.qosCount = service1QosCount;
	pService->info.qosList = service1QosList;
}

RsslInt32 wtfGetProviderLoginStream() { return wtf.providerLoginStreamId; }
RsslInt32 wtfGetProviderDirectoryStream() { return wtf.providerDirectoryStreamId; }
RsslUInt32 wtfGetProviderDirectoryFilter() { return wtf.providerDirectoryFilter; }
RsslDataDictionary *wtfGetDictionary() { return &dataDictionary; }

void wtfProviderTestView(RsslRequestMsg *pRequestMsg, void *elemList, 
		RsslUInt32 elemCount, RsslUInt viewType)
{
	RsslDecodeIterator dIter;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslRet ret;
	RsslBool foundViewType = RSSL_FALSE, foundView = RSSL_FALSE;

	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);
	if (viewType)
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_ELEMENT_LIST);
	else 
	{
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		return;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, wtf.pProvReactorChannel->majorVersion,
			wtf.pProvReactorChannel->minorVersion);

	rsslSetDecodeIteratorBuffer(&dIter, &pRequestMsg->msgBase.encDataBody);

	ASSERT_TRUE((ret = rsslDecodeElementList(&dIter, &elementList, NULL)) == RSSL_RET_SUCCESS);

	while((ret = rsslDecodeElementEntry(&dIter, &elementEntry)) !=
			RSSL_RET_END_OF_CONTAINER)
	{
		ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

		if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VIEW_TYPE))
		{
			RsslUInt decodedViewType;

			ASSERT_TRUE(!foundViewType);
			ASSERT_TRUE(elementEntry.dataType == RSSL_DT_UINT);
			ASSERT_TRUE((ret = rsslDecodeUInt(&dIter, &decodedViewType)) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodedViewType == viewType);
			foundViewType = RSSL_TRUE;
		}
		else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_VIEW_DATA))
		{
			RsslArray rsslArray;
			RsslBuffer arrayEntry;
			RsslInt fieldId;
			RsslBuffer name;
			RsslUInt32 elemPos = 0;

			switch(viewType)
			{
				case RDM_VIEW_TYPE_FIELD_ID_LIST:
				ASSERT_TRUE(!foundView);
				ASSERT_TRUE((ret = rsslDecodeArray(&dIter, &rsslArray)) == RSSL_RET_SUCCESS);
				ASSERT_TRUE(rsslArray.primitiveType == RSSL_DT_INT);

				while ((ret = rsslDecodeArrayEntry(&dIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

					if (!elemList) ASSERT_TRUE(0); /* Field found but list is expected to be empty. */

					ASSERT_TRUE((ret = rsslDecodeInt(&dIter, &fieldId)) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(fieldId == ((RsslInt*)elemList)[elemPos]);
					++elemPos;
					ASSERT_TRUE(elemPos <= elemCount);
				}
				break;

				case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
				ASSERT_TRUE(!foundView);
				ASSERT_TRUE((ret = rsslDecodeArray(&dIter, &rsslArray)) == RSSL_RET_SUCCESS);
				ASSERT_TRUE(rsslArray.primitiveType == RSSL_DT_ASCII_STRING);

				while ((ret = rsslDecodeArrayEntry(&dIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

					if (!elemList) ASSERT_TRUE(0); /* Field found but list is expected to be empty. */

					ASSERT_TRUE((ret = rsslDecodeBuffer(&dIter, &name)) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslBufferIsEqual(&name, &((RsslBuffer*)elemList)[elemPos]));
					++elemPos;
					ASSERT_TRUE(elemPos <= elemCount);
				}
				break;

				default: 
				assert(0);
				break;
			}

			ASSERT_TRUE(elemPos == elemCount);
			foundView = RSSL_TRUE;
		}
	}

	ASSERT_TRUE(foundViewType);
	ASSERT_TRUE(foundView);
}



void wtfProviderEncodeSymbolListDataBody(RsslBuffer *pBuffer, WtfSymbolAction *symbolList, 
		RsslUInt32 symbolCount)
{
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslUInt32 ui;
	RsslEncodeIterator eIter;

	assert(wtf.pProvReactorChannel);

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, wtf.pProvReactorChannel->majorVersion, 
			wtf.pProvReactorChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

	rsslClearMap(&map);
	map.containerType = RSSL_DT_NO_DATA;
	map.keyPrimitiveType = RSSL_DT_ASCII_STRING;
	map.totalCountHint = symbolCount;

	ASSERT_TRUE(rsslEncodeMapInit(&eIter, &map, 0, 0) == RSSL_RET_SUCCESS);

	for (ui = 0; ui < symbolCount; ++ui)
	{
		rsslClearMapEntry(&mapEntry);
		mapEntry.action = symbolList[ui].action;

		ASSERT_TRUE(rsslEncodeMapEntry(&eIter, &mapEntry, &symbolList[ui].itemName) 
				== RSSL_RET_SUCCESS);
	}

	ASSERT_TRUE(rsslEncodeMapComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	pBuffer->length = rsslGetEncodedBufferLength(&eIter);
	
}

void wtfConsumerDecodeSymbolListDataBody(RsslBuffer *pBuffer, WtfSymbolAction *symbolList, 
		RsslUInt32 symbolCount)
{
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslUInt32 ui;
	RsslDecodeIterator dIter;

	assert(wtf.pProvReactorChannel);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, wtf.pProvReactorChannel->majorVersion, 
			wtf.pProvReactorChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, pBuffer);

	rsslClearMap(&map);

	ASSERT_TRUE(rsslDecodeMap(&dIter, &map) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(map.keyPrimitiveType == RSSL_DT_ASCII_STRING
			|| map.keyPrimitiveType == RSSL_DT_RMTES_STRING
			|| map.keyPrimitiveType == RSSL_DT_BUFFER);
	ASSERT_TRUE(map.containerType == RSSL_DT_NO_DATA);

	for (ui = 0; ui < symbolCount; ++ui)
	{
		RsslBuffer itemName;
		rsslClearMapEntry(&mapEntry);
		mapEntry.action = symbolList[ui].action;

		ASSERT_TRUE(rsslDecodeMapEntry(&dIter, &mapEntry, &itemName) 
				== RSSL_RET_SUCCESS);
	}

	ASSERT_TRUE(rsslDecodeMapEntry(&dIter, &mapEntry, NULL) == RSSL_RET_END_OF_CONTAINER);
}

static void wtfSubmitMsgRaw(RsslChannel *pChannel, RsslMsg *pMsg, RsslRDMMsg *pRdmMsg, WtfSubmitMsgOptionsEx *pOptsEx)
{
	RsslBuffer			*pBuffer;
	RsslEncodeIterator	eIter;
	RsslError			rsslError;
	RsslErrorInfo		rsslErrorInfo;
	RsslWriteInArgs		wiArgs;
	RsslWriteOutArgs	woArgs;

	assert(pMsg || pRdmMsg);
	assert(!(pMsg && pRdmMsg));
	assert(pOptsEx);

	ASSERT_TRUE((pBuffer = rsslGetBuffer(pChannel, 2000, RSSL_FALSE, 
					&rsslError)));

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

	if (pMsg)
		ASSERT_TRUE(rsslEncodeMsg(&eIter, pMsg) == RSSL_RET_SUCCESS);
	else
		ASSERT_TRUE(rsslEncodeRDMMsg(&eIter, pRdmMsg, &pBuffer->length, &rsslErrorInfo) 
				== RSSL_RET_SUCCESS);

	rsslClearWriteInArgs(&wiArgs);
	rsslClearWriteOutArgs(&woArgs);
	/* Try to write directly. If it fails, may need to add more code to handle it. */
	wiArgs.writeInFlags = RSSL_WRITE_IN_DIRECT_SOCKET_WRITEN;

	if (pOptsEx->hasSeqNum)
	{
		wiArgs.writeInFlags |= RSSL_WRITE_IN_SEQNUM;
		wiArgs.seqNum = pOptsEx->seqNum;
	}

	ASSERT_TRUE(rsslWriteEx(pChannel, pBuffer, &wiArgs, &woArgs, &rsslError) == RSSL_RET_SUCCESS);

}

void wtfSubmitMsg(RsslReactorSubmitMsgOptions *pOpts, WtfComponent component,
		WtfSubmitMsgOptionsEx *pOptsEx, RsslBool noEventsExpected)
{
	RsslErrorInfo rsslErrorInfo;
	RsslReactor *pReactor;
	RsslReactorChannel *pChannel;
	WtfSubmitMsgOptionsEx exOpts;

	if (!pOptsEx)
	{
		wtfClearSubmitMsgOptionsEx(&exOpts);
		pOptsEx = &exOpts;
	}

	if (pOptsEx->hasSeqNum)
		assert(component == WTF_TC_PROVIDER && wtf.config.useRawProvider);

	switch(component)
	{
		case WTF_TC_PROVIDER: 
			if (!wtf.config.useRawProvider)
			{
				pReactor = wtf.pProvReactor; 
				pChannel = wtf.pProvReactorChannel; 
				break;
			}
			else
			{
				wtfSubmitMsgRaw(wtf.pRawProvChannel, pOpts->pRsslMsg, pOpts->pRDMMsg, pOptsEx);
				return;
			}

		case WTF_TC_CONSUMER: pReactor = wtf.pConsReactor; pChannel = wtf.pConsReactorChannel; break;
		default: assert(0); break;
	}

	EXPECT_GE(rsslReactorSubmitMsg(pReactor, pChannel, pOpts, &rsslErrorInfo), RSSL_RET_SUCCESS) <<
	  "rsslErrorInfo on channel " << rsslErrorInfo.rsslError.channel <<
	  " with rssl error id " << rsslErrorInfo.rsslError.rsslErrorId <<
	  " with system error id " << rsslErrorInfo.rsslError.sysError <<	  
	  " and text " << rsslErrorInfo.rsslError.text << std::endl;

	if (noEventsExpected)
	{
		wtfDispatch(component, 30);
		ASSERT_TRUE(!wtfGetEvent());
	}
}

RsslConnectionTypes wtfGetConnectionType() { return wtf.config.connType; }

RsslReactorChannel *wtfGetChannel(WtfComponent component)
{
	switch(component)
	{
		case WTF_TC_CONSUMER: return wtf.pConsReactorChannel;
		case WTF_TC_PROVIDER: return wtf.pProvReactorChannel;
		default: printf("wtfGetChannel: Unknown component type %d", component); abort(); return NULL;
	}
}

void wtfSetConsumerMsgCallbackAction(void (*function))
{
	wtf.consumerMsgCallbackAction = (void (*)())function;
}

RsslRet wtfGetChannelInfo(WtfComponent component, RsslReactorChannelInfo *pChannelInfo)
{
	RsslReactorChannel *pReactorChannel;
	RsslErrorInfo rsslErrorInfo;

	pReactorChannel = wtfGetChannel(component);


	return rsslReactorGetChannelInfo(pReactorChannel, pChannelInfo, &rsslErrorInfo);
}

static void wtfConsumerEncodeSLBehaviorsElement(RsslEncodeIterator *pIter, RsslUInt slDataStreamFlags)
{
	RsslElementList behaviorsEList;
	RsslElementEntry eEntry, behaviorsEEntry;

	rsslClearElementEntry(&eEntry);

	eEntry.name = RSSL_ENAME_SYMBOL_LIST_BEHAVIORS;
	eEntry.dataType = RSSL_DT_ELEMENT_LIST;
	ASSERT_TRUE(rsslEncodeElementEntryInit(pIter, &eEntry, 0) == RSSL_RET_SUCCESS);

	rsslClearElementList(&behaviorsEList);
	behaviorsEList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ASSERT_TRUE(rsslEncodeElementListInit(pIter, &behaviorsEList, 0, 0) == RSSL_RET_SUCCESS);

	behaviorsEEntry.name = RSSL_ENAME_DATA_STREAMS;
	behaviorsEEntry.dataType = RSSL_DT_UINT;
	ASSERT_TRUE(rsslEncodeElementEntry(pIter, &behaviorsEEntry, &slDataStreamFlags) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslEncodeElementListComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslEncodeElementEntryComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
}

static void wtfConsumerEncodeViewElements(RsslEncodeIterator *pIter, RsslUInt viewType, RsslInt* fidIdList, RsslBuffer* eItemNameList, RsslUInt32 itemCount)
{
	RsslElementEntry eEntry;
	RsslArray elementArray;
	RsslUInt32 i;
	RsslUInt type = viewType;

	assert(wtf.pConsReactorChannel);

	eEntry.name = RSSL_ENAME_VIEW_DATA;
	eEntry.dataType = RSSL_DT_ARRAY;
	ASSERT_TRUE(rsslEncodeElementEntryInit(pIter, &eEntry, 0) == RSSL_RET_SUCCESS);

	rsslClearArray(&elementArray);

	switch (viewType)
	{
		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST:
			elementArray.primitiveType = RSSL_DT_ASCII_STRING;
			elementArray.itemLength = 0;
			ASSERT_TRUE(rsslEncodeArrayInit(pIter, &elementArray) == RSSL_RET_SUCCESS);

			for(i = 0; i < itemCount; i++)
			{
				ASSERT_TRUE(rsslEncodeArrayEntry(pIter, &eItemNameList[i], 0) ==  RSSL_RET_SUCCESS);
			}
			break;
		case RDM_VIEW_TYPE_FIELD_ID_LIST:
			elementArray.primitiveType = RSSL_DT_INT;
			elementArray.itemLength = 2;
			ASSERT_TRUE(rsslEncodeArrayInit(pIter, &elementArray) == RSSL_RET_SUCCESS);

			for(i = 0; i < itemCount; i++)
			{
				ASSERT_TRUE(rsslEncodeArrayEntry(pIter, 0, &fidIdList[i]) ==  RSSL_RET_SUCCESS);
			}
			break;
	}


	ASSERT_TRUE(rsslEncodeArrayComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslEncodeElementEntryComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	/* encode ViewType */
	rsslClearElementEntry(&eEntry);

	eEntry.name = RSSL_ENAME_VIEW_TYPE;
	eEntry.dataType = RSSL_DT_UINT;

	ASSERT_TRUE(rsslEncodeElementEntry(pIter, &eEntry, &type) == RSSL_RET_SUCCESS);

}

static void wtfConsumerEncodeBatchElement(RsslEncodeIterator *pIter, RsslBuffer* itemNameList, RsslUInt32 itemCount)
{
	RsslElementEntry eEntry;
	RsslArray elementArray;
	RsslUInt32 i;

	assert(wtf.pConsReactorChannel);

	rsslClearElementEntry(&eEntry);
	eEntry.name = RSSL_ENAME_BATCH_ITEM_LIST;
	eEntry.dataType = RSSL_DT_ARRAY;
	ASSERT_TRUE(rsslEncodeElementEntryInit(pIter, &eEntry, 0) == RSSL_RET_SUCCESS);

	rsslClearArray(&elementArray);

	elementArray.primitiveType = RSSL_DT_ASCII_STRING;
	elementArray.itemLength = 0;
	ASSERT_TRUE(rsslEncodeArrayInit(pIter, &elementArray) == RSSL_RET_SUCCESS);

	for(i = 0; i < itemCount; i++)
	{
		ASSERT_TRUE(rsslEncodeArrayEntry(pIter, &itemNameList[i], 0) ==  RSSL_RET_SUCCESS);
	}

	ASSERT_TRUE(rsslEncodeArrayComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslEncodeElementEntryComplete(pIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
}

void wtfConsumerEncodeRequestPayload(RsslBuffer *pBuffer, WtfConsumerRequestPayloadOptions *pOpts)
{
	RsslElementList eList;
	RsslEncodeIterator eIter;

	assert(wtf.pConsReactorChannel);

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, wtf.pConsReactorChannel->majorVersion,
			wtf.pConsReactorChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

	rsslClearElementList(&eList);
	eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ASSERT_TRUE(rsslEncodeElementListInit(&eIter, &eList, 0, 0) == RSSL_RET_SUCCESS);

	/* Encode symbol list data stream flags. */
	if (pOpts->slDataStreamFlags)
		wtfConsumerEncodeSLBehaviorsElement(&eIter, pOpts->slDataStreamFlags);

	if (pOpts->viewType)
		wtfConsumerEncodeViewElements(&eIter, pOpts->viewType, pOpts->viewFieldList,
				pOpts->viewElemList, pOpts->viewCount);

	if (pOpts->batchItemList)
		wtfConsumerEncodeBatchElement(&eIter, pOpts->batchItemList, pOpts->batchItemCount);

	ASSERT_TRUE(rsslEncodeElementListComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	pBuffer->length = rsslGetEncodedBufferLength(&eIter);
}

void wtfConsumerEncodeViewRequest(RsslUInt32 viewType, RsslBuffer *pBuffer, RsslInt* fidIdList, RsslBuffer* eItemNameList, RsslUInt32 itemCount)
{
	WtfConsumerRequestPayloadOptions opts;
	wtfClearConsumerRequestPayloadOptions(&opts);

	opts.viewFieldList = fidIdList;
	opts.viewElemList = eItemNameList;
	opts.viewCount = itemCount;
	opts.viewType = viewType;

	wtfConsumerEncodeRequestPayload(pBuffer, &opts);
}


void wtfConsumerEncodeBatchRequest(RsslBuffer *pBuffer, RsslBuffer* itemNameList, RsslUInt32 itemCount)
{
	WtfConsumerRequestPayloadOptions opts;
	wtfClearConsumerRequestPayloadOptions(&opts);

	opts.batchItemList = itemNameList;
	opts.batchItemCount = itemCount;

	wtfConsumerEncodeRequestPayload(pBuffer, &opts);
}

void wtfConsumerEncodeSymbolListRequestBehaviors(RsslBuffer *pBuffer, RsslUInt slDataStreamFlags)
{
	WtfConsumerRequestPayloadOptions opts;
	wtfClearConsumerRequestPayloadOptions(&opts);

	opts.slDataStreamFlags = slDataStreamFlags;

	wtfConsumerEncodeRequestPayload(pBuffer, &opts);
}

