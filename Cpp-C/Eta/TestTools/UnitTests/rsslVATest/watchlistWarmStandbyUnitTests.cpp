/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2021 LSEG. All rights reserved.                    --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

class WatchlistWarmStandbyTestParameters;

using namespace std;

void warmStandbyTest_LoginBased_ActiveAndStandbyServers(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_LoginBased_SwitchingStandbyServerToActive(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_ServiceBased_ActiveAndStandbyServices(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ChannelDown(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ServiceDown(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_LoginBased_SwitchingFromGroupList_To_ChannelList(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_ServiceBased_SelectingActiveService_Per_Server(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_SubmitGenericMsg(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_SubmitPostMsg(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_ProviderCloseItemStreams(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_SubmitPrivateStream(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ChannelDown(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ServiceDown(WatchlistWarmStandbyTestParameters parameters);
void warmStandbyTest_AggregateSourceDirectoryResponse(WatchlistWarmStandbyTestParameters parameters, RsslBool isSameService);
void warmStandbyTest_FailOverFromOneWSBGroup_ToAnotherWSBGroup(WatchlistWarmStandbyTestParameters parameters);

class WatchlistWarmStandbyTestParameters
{
public:
	RsslConnectionTypes connectionType;
	RsslReactorWarmStandbyMode warmStandbyMode;
	RsslBool multiLoginMsg;

	WatchlistWarmStandbyTestParameters(RsslConnectionTypes conType, RsslReactorWarmStandbyMode wsbMode, RsslBool multiLogin)
	{
		connectionType = conType;
		warmStandbyMode = wsbMode;
		multiLoginMsg = multiLogin;
	}

	friend ostream &operator<<(ostream &out, const WatchlistWarmStandbyTestParameters& params)
	{
		out << "ConnectionType = " << params.connectionType << ", Warm standby mode = " << params.warmStandbyMode << ", multiple Login mode = " << params.multiLoginMsg << endl;
		return out;
	}
};

class WatchlistWarmStandbyUnitTest : public ::testing::TestWithParam<WatchlistWarmStandbyTestParameters> {
public:

	static void SetUpTestCase()
	{
		WtfInitOpts wtfInitOpts;
		wtfClearInitOpts(&wtfInitOpts);
		wtfInitOpts.numberOfServer = 4;
		wtfInit(&wtfInitOpts, 0);
	}

	virtual void SetUp()
	{
		/* Creates four servers by default. */
		wtfBindServer(GetParam().connectionType, const_cast<char*>("14011"));
		wtfBindServer(GetParam().connectionType, const_cast<char*>("14012"));
		wtfBindServer(GetParam().connectionType, const_cast<char*>("14013"));
		wtfBindServer(GetParam().connectionType, const_cast<char*>("14014"));
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}

	virtual void TearDown()
	{
		wtfCloseServer(0);
		wtfCloseServer(1);
		wtfCloseServer(2);
		wtfCloseServer(3);
	}
};

INSTANTIATE_TEST_SUITE_P(
	TestingWarmStandbyUnitTests,
	WatchlistWarmStandbyUnitTest,
	::testing::Values(
		
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_SOCKET, RSSL_RWSB_MODE_LOGIN_BASED, RSSL_FALSE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWSB_MODE_LOGIN_BASED, RSSL_FALSE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_SOCKET, RSSL_RWSB_MODE_SERVICE_BASED, RSSL_FALSE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWSB_MODE_SERVICE_BASED, RSSL_FALSE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_SOCKET, RSSL_RWSB_MODE_LOGIN_BASED, RSSL_TRUE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWSB_MODE_LOGIN_BASED, RSSL_TRUE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_SOCKET, RSSL_RWSB_MODE_SERVICE_BASED, RSSL_TRUE),
		WatchlistWarmStandbyTestParameters(RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWSB_MODE_SERVICE_BASED, RSSL_TRUE)
		
	));

TEST_P(WatchlistWarmStandbyUnitTest, LoginBasedActiveAndStandbyServers)
{
	warmStandbyTest_LoginBased_ActiveAndStandbyServers(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, LoginBasedSwitchingStandbyServerToActive)
{
	warmStandbyTest_LoginBased_SwitchingStandbyServerToActive(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, ServiceBasedActiveAndStandbyServices)
{
	warmStandbyTest_ServiceBased_ActiveAndStandbyServices(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, ServiceBasedSwitchingActiveAndStandbyServices_By_ChannelDown)
{
	warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ChannelDown(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, ServiceBasedSwitchingActiveAndStandbyServices_By_ServiceDown)
{
	warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ServiceDown(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, LoginBased_SwitchingFromGroupList_To_ChannelList)
{
	warmStandbyTest_LoginBased_SwitchingFromGroupList_To_ChannelList(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, ServiceBased_SelectingActiveService_Per_Server)
{
	warmStandbyTest_ServiceBased_SelectingActiveService_Per_Server(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, SubmitGenericMsg)
{
	warmStandbyTest_SubmitGenericMsg(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, SubmitPostMsg)
{
	warmStandbyTest_SubmitPostMsg(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, ProviderCloseItemStreams)
{
	warmStandbyTest_ProviderCloseItemStreams(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, SubmitPrivateStream)
{
	warmStandbyTest_SubmitPrivateStream(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, DifferentServiceForActiveAndStanbyServer_ChannelDown)
{
	warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ChannelDown(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, DifferentServiceForActiveAndStanbyServer_ServiceDown)
{
	warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ServiceDown(GetParam());
}

TEST_P(WatchlistWarmStandbyUnitTest, AggregateSourceDirectoryResponse_WithDifferentService_ForBothServers)
{
	warmStandbyTest_AggregateSourceDirectoryResponse(GetParam(), RSSL_FALSE);
}

TEST_P(WatchlistWarmStandbyUnitTest, AggregateSourceDirectoryResponse_WithSameService_ForBothServers)
{
	warmStandbyTest_AggregateSourceDirectoryResponse(GetParam(), RSSL_TRUE);
}

TEST_P(WatchlistWarmStandbyUnitTest, FailOverFromWSBToAnotherWSB)
{
	warmStandbyTest_FailOverFromOneWSBGroup_ToAnotherWSBGroup(GetParam());
}

void warmStandbyTest_LoginBased_ActiveAndStandbyServers(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = {256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_LOGIN_BASED)
	{
		cout << "\tSkip testing service based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);
	
	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the active server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer should receive no more message. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Both active and standby server receives the close message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_LoginBased_SwitchingStandbyServerToActive(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslRDMLoginRefresh loginRefresh;
	RsslReactorSubmitMsgOptions submitOpts;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_LOGIN_BASED)
	{
		cout << "\tSkip testing service based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Closes the channel of active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE;

	/* Provider should now accept. */
	wtfAccept(0);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	pEvent = wtfGetEvent();
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY;

	wtfDispatch(WTF_TC_PROVIDER, 200, 0, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	wtfSendDefaultSourceDirectory(&connOpts, &pEvent->rdmMsg.pRdmMsg->directoryMsg.request, 0);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a update message from the active server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer should receive no more message. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Both active and standby server receives the close message */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_ServiceBased_ActiveAndStandbyServices(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_SERVICE_BASED)
	{
		cout << "\tSkip testing login based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both active and standby server receives the close message */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ChannelDown(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslRDMLoginRefresh loginRefresh;
	RsslReactorSubmitMsgOptions submitOpts;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_SERVICE_BASED)
	{
		cout << "\tSkip testing login based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_ACTIVE;

	/* Provider should now accept. */
	wtfAccept(0);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	wtfSendDefaultSourceDirectory(&connOpts, &pEvent->rdmMsg.pRdmMsg->directoryMsg.request, 0);

	wtfDispatch(WTF_TC_CONSUMER, 200);
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_STANDBY;

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both active and standby server receives the close message */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_ServiceBased_SwitchingActiveAndStandbyServices_By_ServiceDown(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;

	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService service1;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_SERVICE_BASED)
	{
		cout << "\tSkip testing login based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider sends directory update to indicate service down. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = RDM_DIRECTORY_SERVICE_STATE_FILTER;
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE;
	service1.state.serviceState = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 100, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_STANDBY;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_ACTIVE;

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both active and standby server receives the close message */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_LoginBased_SwitchingFromGroupList_To_ChannelList(WatchlistWarmStandbyTestParameters parameters)
{
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	RsslReactorConnectInfo connectionServer;
	WtfEvent *pEvent;

	RsslReactorSubmitMsgOptions opts;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;

	ASSERT_TRUE(wtfStartTest());

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_LOGIN_BASED)
	{
		cout << "\tSkip testing service based warm standby." << endl;
		return;
	}

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server but this test case switches to a server in the connection list. */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14018"); /* Invalid server */
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14019"); /* Invalid server */
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer);

	connectionServer.rsslConnectOptions.connectionType = parameters.connectionType;
	connectionServer.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		connectionServer.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		connectionServer.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	connOpts.connectionCount = 1;
	connOpts.reactorConnectionList = &connectionServer;
	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 500;
	connOpts.reconnectMaxDelay = 2000;

	/* Setup a single connection from the connection list by switching from a starting server and a standby server. */
	wtfSetupConnectionServerFromConnectionList(&connOpts, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives only one request message from consumer side. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));
	
	wtfDispatch(WTF_TC_CONSUMER, 400);
	/* Consumer should receive no more event from the server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the single server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more event from the server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submit close item request */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerStreamId;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives the close message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_ServiceBased_SelectingActiveService_Per_Server(WatchlistWarmStandbyTestParameters parameters)
{
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslReactorPerServiceBasedOptions perServiceBasedOption;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	rsslClearReactorPerServiceBasedOptions(&perServiceBasedOption);

	if (parameters.warmStandbyMode != RSSL_RWSB_MODE_SERVICE_BASED)
	{
		cout << "\tSkip testing login based warm standby." << endl;
		return;
	}

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer.perServiceBasedOptions.serviceNameCount = 1;
	standbyServer.perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	wtfFinishTest();
}

void warmStandbyTest_SubmitGenericMsg(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslGenericMsg genericMsg, *pGenericMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer		genericItemName = { 6, const_cast<char*>("GEN1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	WtfWarmStandbyMode warmStandbyMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_SERVICE_BASED_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_SERVICE_BASED_STANDBY;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[0]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[1]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submits generic message to the warmstandby channel */
	rsslClearGenericMsg(&genericMsg);

	genericMsg.msgBase.streamId = consumerStreamId;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	genericMsg.flags |= (RSSL_PSMF_HAS_MSG_KEY);
	genericMsg.msgBase.msgKey.flags |= (RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID);
	genericMsg.msgBase.msgKey.name = genericItemName;
	genericMsg.msgBase.msgKey.nameType = 1;
	genericMsg.msgBase.msgKey.serviceId = service1Id;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Only the active server receives the generic message on data domain */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
	ASSERT_TRUE(pGenericMsg->msgBase.streamId == providerStreamId);
	ASSERT_TRUE(pGenericMsg->flags == genericMsg.flags);
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.flags == genericMsg.msgBase.msgKey.flags);
	ASSERT_TRUE(rsslBufferIsEqual(&pGenericMsg->msgBase.msgKey.name, &genericItemName));
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.serviceId == service1Id);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_SubmitPostMsg(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslPostMsg postMsg, *pPostMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer		postItemName = { 7, const_cast<char*>("POST1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	WtfWarmStandbyMode warmStandbyMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	RsslUInt32 postID = 555;

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_SERVICE_BASED_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_SERVICE_BASED_STANDBY;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[0]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[1]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Submits post message to the warmstandby channel */
	rsslClearPostMsg(&postMsg);

	postMsg.flags |= (RSSL_PSMF_HAS_MSG_KEY | RSSL_PSMF_HAS_POST_ID);
	postMsg.postId = postID;
	postMsg.msgBase.streamId = consumerStreamId;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.msgKey.flags |= (RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID);
	postMsg.msgBase.msgKey.name = postItemName;
	postMsg.msgBase.msgKey.nameType = 1;
	postMsg.msgBase.msgKey.serviceId = service1Id;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Both active and standby servers receive the generic message on data domain */
	wtfDispatch(WTF_TC_PROVIDER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerStreamId);
	ASSERT_TRUE(pPostMsg->flags == postMsg.flags);
	ASSERT_TRUE(pPostMsg->postId == postID);
	ASSERT_TRUE(pPostMsg->msgBase.msgKey.flags == postMsg.msgBase.msgKey.flags);
	ASSERT_TRUE(rsslBufferIsEqual(&pPostMsg->msgBase.msgKey.name,&postItemName));
	ASSERT_TRUE(pPostMsg->msgBase.msgKey.serviceId == service1Id);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerStreamId);
	ASSERT_TRUE(pPostMsg->flags == postMsg.flags);
	ASSERT_TRUE(pPostMsg->postId == postID);
	ASSERT_TRUE(pPostMsg->msgBase.msgKey.flags == postMsg.msgBase.msgKey.flags);
	ASSERT_TRUE(rsslBufferIsEqual(&pPostMsg->msgBase.msgKey.name, &postItemName));
	ASSERT_TRUE(pPostMsg->msgBase.msgKey.serviceId == service1Id);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_ProviderCloseItemStreams(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslStatusMsg statusMsg, *pStatusMsg;
	RsslCloseMsg *pCloseMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer *pWtfTestServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer		itemName2 = { 7, const_cast<char*>("ITEM2.N") };
	RsslInt32		firstConStreamID = 5;
	RsslInt32		secondConStreamID = 6;
	RsslInt32		providerStreamId;
	RsslInt32		providerStreamId2;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	WtfWarmStandbyMode warmStandbyMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	RsslBuffer expectedStatusText = { 31, const_cast<char*>("Service for this item was lost.") };

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
		warmStandbyMode[0] = WTF_WSBM_SERVICE_BASED_ACTIVE;
		warmStandbyMode[1] = WTF_WSBM_SERVICE_BASED_STANDBY;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request first market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = firstConStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives the first request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[0]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == warmStandbyMode[1]);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == firstConStreamID);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Request second market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = secondConStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives the second request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName2));
	providerStreamId2 = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName2));
	providerStreamId2 = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId2;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId2;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == secondConStreamID);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* The active provider closes the first item stream */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerStreamId;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
	statusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	statusMsg.msgBase.msgKey.name = itemName1;
	statusMsg.flags |= RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE,0);

	/* Consumer receives the close message for the first item stream from the active channel */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == firstConStreamID);
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->flags == (RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY));
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The standby server receives a close message from closing the item. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerStreamId);

	/* The standby provider closes the second item stream */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerStreamId2;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
	statusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	statusMsg.msgBase.msgKey.name = itemName2;
	statusMsg.flags |= RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_CONSUMER, 100);
	
	/* Consumer should receive no status message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Closes the channel of active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	
	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		/* Consumer receives a status message when it switches to the standby server. */
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->flags == (RSSL_STMF_HAS_STATE ));
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->state.text, &expectedStatusText));

		ASSERT_TRUE(pEvent = wtfGetEvent());

		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->flags == (RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY));
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->msgBase.msgKey.name, &itemName2));

		wtfDispatch(WTF_TC_PROVIDER, 100, 1);

		/* Provider side receives consumer connection status */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	}
	else
	{
		/* Consumer receives status messages when it switches to the standby server. */

		/* Service down status message. */
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->flags == (RSSL_STMF_HAS_STATE));
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

		ASSERT_TRUE(pEvent = wtfGetEvent());

		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

		/* Fand out closed recoverable message */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->flags == (RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY));
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->msgBase.msgKey.name, &itemName2));

		wtfDispatch(WTF_TC_PROVIDER, 100, 1);

		/* Provider side receives consumer connection status */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	}

	

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* No more event */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_SubmitPrivateStream(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer		itemName2 = { 7, const_cast<char*>("ITEM2.N") };
	RsslInt32		firstConStreamID = 5;
	RsslInt32		secondConStreamID = 6;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request first market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = firstConStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_PRIVATE_STREAM;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Only the active provider receives the private stream request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/*Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ChannelDown(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg *pStatusMsg;
	RsslRefreshMsg refreshMsg, *pRefreshMsg;
	RsslRDMDirectoryUpdate* pDirectoryUpdate;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer		itemName2 = { 7, const_cast<char*>("ITEM2.N") };
	RsslInt32		firstConStreamID = 5;
	RsslInt32		secondConStreamID = 6;
	RsslInt32		providerStreamId;
	RsslInt32		providerStreamId2;

	char			mpDataBodyBuf[256];
	char			mpDataBodyBuf2[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	RsslBuffer		mpDataBody2 = { 256, mpDataBodyBuf2 };
	RsslUInt32		mpDataBodyLen2 = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	RsslBuffer expectedStatusText = { 31, const_cast<char*>("Service for this item was lost.") };
	RsslBuffer expectedStatusText2 = { 28, const_cast<char*>("No matching service present.") };
	RsslBuffer expectedStatusText3 = { 16, const_cast<char*>("Channel is down.") };
	WtfMarketPriceItem marketPriceItem;
	WtfMarketPriceItem marketPriceItem2;

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService2Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_TRUE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request first market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = firstConStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* The starting provider receives the first request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The standby server doesn't receive the request message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only for service ID 1. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == firstConStreamID);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Request second market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = secondConStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service2Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 200);

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		/* Receives a status message as the service is not active on the starting server. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->state.text, &expectedStatusText2));
	}

	/* The starting server doesn't receive the request message. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		/* The standby provider receives the second request. */
		wtfDispatch(WTF_TC_PROVIDER, 400, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName2));
		providerStreamId2 = pRequestMsg->msgBase.streamId;

		/* The standby provider sends a refresh message with payload. */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
		refreshMsg.msgBase.streamId = providerStreamId2;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service2Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		mpDataBody2.length = mpDataBodyLen2;

		wtfInitMarketPriceItemFields(&marketPriceItem2);
		ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody2, &marketPriceItem2) == RSSL_RET_SUCCESS);
		refreshMsg.msgBase.encDataBody = mpDataBody2;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);
	}

	wtfDispatch(WTF_TC_CONSUMER, 200);

	/* Consumer doesn't receive any message for login based. */
	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		ASSERT_FALSE(pEvent = wtfGetEvent());
	}
	else
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
		ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service2Id);
		ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody2));
	}

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

	if(parameters.multiLoginMsg == RSSL_FALSE)
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags == RDM_SVCF_NONE);

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		/* Checks the closed recover stream state for the first item */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == firstConStreamID);
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->state.text, &expectedStatusText));

		/* Consumer channel FD change. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

		wtfDispatch(WTF_TC_PROVIDER, 400, 1);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

		/* The new active server receives the second item request and send a response.*/
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName2));
		providerStreamId2 = pRequestMsg->msgBase.streamId;

		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
		refreshMsg.msgBase.streamId = providerStreamId2;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service2Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		mpDataBody2.length = mpDataBodyLen2;

		wtfInitMarketPriceItemFields(&marketPriceItem2);
		ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody2, &marketPriceItem2, 1) == RSSL_RET_SUCCESS);
		refreshMsg.msgBase.encDataBody = mpDataBody2;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

		wtfDispatch(WTF_TC_CONSUMER, 200);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == secondConStreamID);
		ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
		ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service2Id);
		ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody2));
	}
	else
	{

		/* Checks the closed recover stream state for the first item */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == firstConStreamID);
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
		ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->state.text, &expectedStatusText));

		/* Consumer channel FD change. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

		wtfDispatch(WTF_TC_PROVIDER, 400, 1);

		/* There is no active service to change its status. */
		ASSERT_FALSE(pEvent = wtfGetEvent());
	}

	/* Closes the channel of the standby server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);


	/* Checks for login status message. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.text, &expectedStatusText3));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

	if (parameters.multiLoginMsg == RSSL_FALSE)
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[1].serviceId);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags == RDM_SVCF_NONE);

	/* Checks the Open, Suspect for the item */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == secondConStreamID);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->state.text, &expectedStatusText));


	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	wtfDispatch(WTF_TC_CONSUMER, 200);

	/* No more event on consumer side */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 200);

	/* No more event on provider side */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();

	/* Adds 5 seconds delay to account for slow machine. */
	wtfsleep(5000);
}

void warmStandbyTest_DifferentServiceForActiveAndStanbyServer_ServiceDown(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg refreshMsg, *pRefreshMsg;
	RsslUpdateMsg updateMsg, *pUpdateMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamID = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	RsslBuffer expectedStatusText = { 31, const_cast<char*>("Service for this item was lost.") };
	WtfMarketPriceItem marketPriceItem;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService service1;

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService2Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request first market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamID;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* The starting provider receives the first request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The standby server doesn't receive the request message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only for service ID 1. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamID);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Provider sends directory update to indicate service down. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = RDM_DIRECTORY_SERVICE_STATE_FILTER;
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE;
	service1.state.serviceState = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* This is no effect as the service state doesn't affect existing streams */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider doesn't receive consumer status message and change its service to standby mode for service based. */
	wtfDispatch(WTF_TC_PROVIDER, 200);
	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
	}
	else
	{
		/* The active provider sends a update message with payload for the login based only.*/
		rsslClearUpdateMsg(&updateMsg);
		updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
		updateMsg.msgBase.streamId = providerStreamId;
		updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
		updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		updateMsg.msgBase.msgKey.serviceId = service1Id;
		mpDataBody.length = mpDataBodyLen;

		wtfUpdateMarketPriceItemFields(&marketPriceItem);
		ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
		updateMsg.msgBase.encDataBody = mpDataBody;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&updateMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


		wtfDispatch(WTF_TC_CONSUMER, 400);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamID);
		ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
		ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));
	}

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer should receive no more message. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_AggregateSourceDirectoryResponse(WatchlistWarmStandbyTestParameters parameters, RsslBool isSameService)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRDMDirectoryUpdate *pDirectoryUpdate;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;

	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService service1, service2;

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = isSameService ? RDM_DIRECTORY_SERVICE_TYPE_STANDBY : RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		standbyServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = parameters.warmStandbyMode;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	if (isSameService)
	{
		wtfSetService1Info(&rdmService[0]);
		wtfSetService1Info(&rdmService[1]);
	}
	else
	{
		wtfSetService1Info(&rdmService[0]);
		wtfSetService2Info(&rdmService[1]);
	}

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_TRUE, parameters.connectionType, parameters.multiLoginMsg);

	/* The active server sends directory update to indicate service down for service ID 1. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = RDM_DIRECTORY_SERVICE_STATE_FILTER;
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.action = RSSL_MPEA_UPDATE_ENTRY;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE;
	service1.state.serviceState = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	if (isSameService)
	{
		/* Consumer should receive no source directory message. */
		ASSERT_FALSE(pEvent = wtfGetEvent());

		/* Provider receives consumer status message for the standy service to provide service ID 1 as active*/
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);

		if (parameters.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
		{
			/* Consumer status for the starting server */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
			ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

			/* Consumer status for the standby server */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
			ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
		}

		/* The standby server sends directory update to indicate service down for service ID 1. */
		rsslClearRDMDirectoryUpdate(&directoryUpdate);
		directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
		directoryUpdate.filter = RDM_DIRECTORY_SERVICE_STATE_FILTER;
		rsslClearRDMService(&service1);
		service1.serviceId = service1Id;
		service1.action = RSSL_MPEA_UPDATE_ENTRY;
		directoryUpdate.serviceCount = 1;
		directoryUpdate.serviceList = &service1;

		/* Change state filter's service state. */
		service1.flags |= RDM_SVCF_HAS_STATE;
		service1.state.serviceState = 0;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

		wtfDispatch(WTF_TC_CONSUMER, 600);
	}

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

	if (parameters.multiLoginMsg == RSSL_FALSE)
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == service1.action);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags == RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.serviceState == service1.state.serviceState);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.flags == service1.state.flags);

	if (isSameService == RSSL_FALSE)
	{
		if (parameters.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
		{
			wtfDispatch(WTF_TC_PROVIDER, 400, 0);
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == service1Id);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
			ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
		}

		/* The standby server sends directory update to indicate service down for service ID 2. */
		rsslClearRDMDirectoryUpdate(&directoryUpdate);
		directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
		directoryUpdate.filter = RDM_DIRECTORY_SERVICE_STATE_FILTER;
		rsslClearRDMService(&service2);
		service2.serviceId = service2Id;
		service2.action = RSSL_MPEA_UPDATE_ENTRY;
		directoryUpdate.serviceCount = 1;
		directoryUpdate.serviceList = &service2;

		/* Change state filter's service state. */
		service2.flags |= RDM_SVCF_HAS_STATE;
		service2.state.serviceState = 0;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

		wtfDispatch(WTF_TC_CONSUMER, 400);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

		if (parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

		ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == service2.action);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[1].serviceId);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags == RDM_SVCF_HAS_STATE);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.serviceState == service2.state.serviceState);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.flags == service2.state.flags);
	}

	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		/* Consumer status for the server to provide service as standby as well. */
		wtfDispatch(WTF_TC_PROVIDER, 400, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	}

	/* The active server sends directory update to delete the service ID 1. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.action = RSSL_MPEA_DELETE_ENTRY;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 200);

	if (isSameService == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

		if (parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

		ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[0].serviceId);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == service1.action);

		/* The standby server sends directory update to delete the service ID 2. */
		rsslClearRDMDirectoryUpdate(&directoryUpdate);
		directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
		rsslClearRDMService(&service2);
		service2.serviceId = service2Id;
		service2.action = RSSL_MPEA_DELETE_ENTRY;
		directoryUpdate.serviceCount = 1;
		directoryUpdate.serviceList = &service2;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

		wtfDispatch(WTF_TC_CONSUMER, 200);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);
		
		if (parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

		ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[1].serviceId);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == service2.action);
	}
	else
	{
		/* The standby server sends directory update to delete the service ID 1. */
		rsslClearRDMDirectoryUpdate(&directoryUpdate);
		directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
		rsslClearRDMService(&service1);
		service1.serviceId = service1Id;
		service1.action = RSSL_MPEA_DELETE_ENTRY;
		directoryUpdate.serviceCount = 1;
		directoryUpdate.serviceList = &service1;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

		wtfDispatch(WTF_TC_CONSUMER, 200);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == WTF_DIRECTORY_STREAM_ID);

		if (parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)WTF_DEFAULT_DIRECTORY_USER_SPEC_PTR);

		ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == rdmService[0].serviceId);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == service1.action);
	}

	/* Consumer should receive no more message. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void warmStandbyTest_FailOverFromOneWSBGroup_ToAnotherWSBGroup(WatchlistWarmStandbyTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg refreshMsg, refreshMsg2, *pRefreshMsg;
	RsslUpdateMsg updateMsg, *pUpdateMsg;
	RsslStatusMsg *pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[2];
	RsslReactorWarmStandbyServerInfo standbyServer[2];

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslBuffer expectedStatusText3 = { 16, const_cast<char*>("Channel is down.") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	WtfMarketPriceItem marketPriceItem;
	RsslRDMLoginRefresh loginRefresh;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslBuffer activeUserName = { 10, const_cast<char*>("activeUser") };
	RsslBuffer standbyUserName = { 11, const_cast<char*>("standbyUser") };


	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;
	}
	else
	{
		warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
		warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	{
		/* Setup first warm standby group */
		rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

		if (parameters.multiLoginMsg)
		{
			reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
		}

		if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
		}

		rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

		standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
		standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
		standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
		standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
		standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

		if (parameters.multiLoginMsg)
		{
			standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
		}

		if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			standbyServer[0].reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
		}

		reactorWarmstandByGroup[0].standbyServerCount = 1;
		reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
		reactorWarmstandByGroup[0].warmStandbyMode = parameters.warmStandbyMode;
	}

	{
		/* Setup second warm standby group */
		rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

		if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
		}

		rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

		standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = parameters.connectionType;
		standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
		standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
		standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
		standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

		if (parameters.multiLoginMsg)
		{
			standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
		}

		if (parameters.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			standbyServer[1].reactorConnectInfo.rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
		}

		reactorWarmstandByGroup[1].standbyServerCount = 1;
		reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
		reactorWarmstandByGroup[1].warmStandbyMode = parameters.warmStandbyMode;
	}

	connOpts.warmStandbyGroupCount = 2;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup[0];
	connOpts.reconnectAttemptLimit = 1; /* Reconnect only once and move on to another warm standby group. */

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, parameters.connectionType, parameters.multiLoginMsg);

	/* Request first market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the starting server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more event from the server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Closes the channel of active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	/* Shutdown the starting server */
	wtfCloseServer(0);

	wtfDispatch(WTF_TC_CONSUMER, 400);
	/* Consumer channel FD change. */

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);
	
	wtfDispatch(WTF_TC_PROVIDER, 200);
	if (parameters.warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	}
	else
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == service1Id);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	}

	/* Closes the channel of the last server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	/* Shutdown the last server */
	wtfCloseServer(1);

	wtfDispatch(WTF_TC_PROVIDER, 200);

	wtfDispatch(WTF_TC_CONSUMER, 400);


#if !defined(_WIN32)
	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);
#endif

	/* Checks for login status message. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.status.state.text, &expectedStatusText3));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);


	/* Consumer should receive no more event from the server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 1000);

	wtfDispatch(WTF_TC_CONSUMER, 5000);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

#if defined(_WIN32)
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
#endif

	wtfDispatch(WTF_TC_CONSUMER, 8000);

	wtfAcceptWithTime(5000, 2);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request for the second group. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(wtfGetProviderLoginStream() == pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId);

	if (parameters.multiLoginMsg == RSSL_TRUE)
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (connOpts.consumerLoginCallback == WTF_CB_USE_DOMAIN_CB)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
		if(parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}
	else
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.streamId == 1);
		if (parameters.multiLoginMsg == RSSL_FALSE)
			ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	if (connOpts.reactorWarmStandbyGroupList[1].warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);
	}

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	{
		/* Filter should contain at least Info, State, and Group filters. */
		RsslInt32 providerDirectoryStreamId;
		RsslUInt32 providerDirectoryFilter =
			pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
		ASSERT_TRUE(providerDirectoryFilter ==
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

		providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

		if (connOpts.provideDefaultDirectory)
		{
			RsslRDMDirectoryRefresh directoryRefresh;

			rsslClearRDMDirectoryRefresh(&directoryRefresh);
			directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
			directoryRefresh.filter = providerDirectoryFilter;
			directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

			directoryRefresh.serviceList = &rdmService[0];
			directoryRefresh.serviceCount = 1;

			if (connOpts.provideDefaultServiceLoad)
			{
				rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
				rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
				rdmService[0].load.openLimit = 0xffffffffffffffffULL;
				rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
				rdmService[0].load.openWindow = 0xffffffffffffffffULL;
				rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
				rdmService[0].load.loadFactor = 65535;
			}

			rsslClearReactorSubmitMsgOptions(&submitOpts);
			submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
			wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);
		}
	}

	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	if (connOpts.reactorWarmStandbyGroupList[1].warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		wtfDispatch(WTF_TC_CONSUMER, 400);

		wtfDispatch(WTF_TC_PROVIDER, 400, 2);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
		providerStreamId = pRequestMsg->msgBase.streamId;

		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
		refreshMsg.msgBase.streamId = providerStreamId;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service1Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		mpDataBody.length = mpDataBodyLen;

		wtfInitMarketPriceItemFields(&marketPriceItem);
		ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
		refreshMsg.msgBase.encDataBody = mpDataBody;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);
	}

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* This starting server of the second group receives the item request. */

	if (connOpts.reactorWarmStandbyGroupList[1].warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
		providerStreamId = pRequestMsg->msgBase.streamId;

		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
		refreshMsg.msgBase.streamId = providerStreamId;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service1Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		mpDataBody.length = mpDataBodyLen;

		wtfInitMarketPriceItemFields(&marketPriceItem);
		ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
		refreshMsg.msgBase.encDataBody = mpDataBody;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);
	}

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE); 

	/* Consumer receives a refresh message from the active server only. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 3);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	if (connOpts.reactorWarmStandbyGroupList[1].warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED)
	{
		/* Provider receives a generic message on the login domain to indicate warm standby mode. */
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_STANDBY);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);
	}

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	{
		/* Filter should contain at least Info, State, and Group filters. */
		RsslInt32 providerDirectoryStreamId;
		RsslUInt32 providerDirectoryFilter =
			pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
		ASSERT_TRUE(providerDirectoryFilter ==
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

		providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

		if (connOpts.provideDefaultDirectory)
		{
			RsslRDMDirectoryRefresh directoryRefresh;

			rsslClearRDMDirectoryRefresh(&directoryRefresh);
			directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
			directoryRefresh.filter = providerDirectoryFilter;
			directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

			directoryRefresh.serviceList = &rdmService[1];
			directoryRefresh.serviceCount = 1;

			if (connOpts.provideDefaultServiceLoad)
			{
				rdmService[1].flags |= RDM_SVCF_HAS_LOAD;
				rdmService[1].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
				rdmService[1].load.openLimit = 0xffffffffffffffffULL;
				rdmService[1].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
				rdmService[1].load.openWindow = 0xffffffffffffffffULL;
				rdmService[1].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
				rdmService[1].load.loadFactor = 65535;
			}

			rsslClearReactorSubmitMsgOptions(&submitOpts);
			submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
			wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);
		}
	}

	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	if (connOpts.reactorWarmStandbyGroupList[1].warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED)
	{
		wtfDispatch(WTF_TC_CONSUMER, 400);

		wtfDispatch(WTF_TC_PROVIDER, 400, 3, 1);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
		ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	}
	else
	{
		/* Provider receives item request. */
		wtfDispatch(WTF_TC_PROVIDER, 100, 3);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
		ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
		ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
		providerStreamId = pRequestMsg->msgBase.streamId;
	}

	/* No more message.*/
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}
