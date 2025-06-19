/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

void watchlistRecoveryTest_OneItem_ClosedRecover();
void watchlistRecoveryTest_OneItem_RequestTimeout(RsslBool singleOpen);
void watchlistRecoveryTest_TwoItems_RequestTimeout();
void watchlistRecoveryTest_OneItem_RequestTimeout_Multipart();
void watchlistRecoveryTest_OneItem_ClearCache();
void watchlistRecoveryTest_OneItem_QosChange();
void watchlistRecoveryTest_TwoItems_OpenWindow();
void watchlistRecoveryTest_OneItem_ServiceNameChange();
void watchlistRecoveryTest_OneItem_ClosedRecover_PrivateStream();
void watchlistRecoveryTest_OneItem_ClosedRecover_SingleOpenOff();
void watchlistRecoveryTEST_FrivateStreamWithPayloadAndExtHeader();
void watchlistRecoveryTest_OneItem_AllowSuspectDataOff();
void watchlistRecoveryTest_OneItem_GroupMerge();
void watchlistRecoveryTest_TwoItems_ClosedRecoverFromServiceState();
void watchlistRecoveryTest_OneItem_ClosedRecoverFromGroupState();
void watchlistRecoveryTest_OneItem_LoginClosedRecover(RsslBool singleOpen);
void watchlistRecoveryTest_OneItem_LoginClosed();
void watchlistRecoveryTest_OneItem_LoginClosed_Directory();
void watchlistRecoveryTest_OneItem_LoginCloseRequest(RsslBool useRsslMsg);
void watchlistRecoveryTest_OneItem_DirectoryClosedRecover();
void watchlistRecoveryTest_OneItem_QosRange();
void watchlistRecoveryTest_OneItem_NoCapability();
void watchlistRecoveryTest_OneItem_NoQosRequest();
void watchlistRecoveryTest_OneItem_StaticQos();
void watchlistRecoveryTest_OneItem_ServiceDeleted(RsslBool singleOpen);
void watchlistRecoveryTest_OneItem_Redirect();
void watchlistRecoveryTest_OneItem_RedirectToPrivateStream();
void watchlistRecoveryTest_OneItem_RedirectToPrivateStream_WithKey();
void watchlistRecoveryTest_OneItem_Dictionary();
void watchlistRecoveryTest_LoginCredentialsUpdate();
void watchlistRecoveryTest_UnknownStream();
void watchlistRecoveryTest_OneItem_Disconnect(RsslBool singleOpen);
void watchlistRecoveryTest_LoginAuthenticationUpdate();
void watchlistRecoveryTest_TwoItems_CloseStateSecondInCallback();
void watchlistRecoveryTest_OneItem_DisconnectChangeQoS();
void watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseInCallback();
void watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseAndRerequestInCallback();
void watchlistRecoveryTest_OneItem_ServiceDeleted_ItemCloseAndRerequestInDirectoryCallback();


// Global variables for callbacks
bool sendClose = false;
bool resendRequest = false;

class WatchlistRecoveryTest : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		wtfInit(NULL);
	}

	virtual void SetUp()
	{
		sendClose = false;
		resendRequest = false;
		wtfClearCallbacks();
		wtfBindServer(RSSL_CONN_TYPE_SOCKET);
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}

	virtual void TearDown()
	{
		sendClose = false;
		resendRequest = false;
		wtfClearCallbacks();
		wtfCloseServer();
	}
};

TEST_F(WatchlistRecoveryTest, OneItem_Disconnect_SingleOpen)
{
	watchlistRecoveryTest_OneItem_Disconnect(RSSL_TRUE);
}

TEST_F(WatchlistRecoveryTest, OneItem_Disconnect)
{
	watchlistRecoveryTest_OneItem_Disconnect(RSSL_FALSE);
}

TEST_F(WatchlistRecoveryTest, UnknownStream)
{
	watchlistRecoveryTest_UnknownStream();
}

TEST_F(WatchlistRecoveryTest, OneItem_ClosedRecover)
{
	watchlistRecoveryTest_OneItem_ClosedRecover();
}

TEST_F(WatchlistRecoveryTest, OneItem_RequestTimeout_SingleOpen)
{
	watchlistRecoveryTest_OneItem_RequestTimeout(RSSL_TRUE);
}

TEST_F(WatchlistRecoveryTest, OneItem_RequestTimeout)
{
	watchlistRecoveryTest_OneItem_RequestTimeout(RSSL_FALSE);
}

TEST_F(WatchlistRecoveryTest, TwoItems_RequestTimeout)
{
	watchlistRecoveryTest_TwoItems_RequestTimeout();
}

TEST_F(WatchlistRecoveryTest, OneItem_RequestTimeout_Multipart)
{
	watchlistRecoveryTest_OneItem_RequestTimeout_Multipart();
}

TEST_F(WatchlistRecoveryTest, OneItem_ClearCache)
{
	watchlistRecoveryTest_OneItem_ClearCache();
}

TEST_F(WatchlistRecoveryTest, OneItem_QosChange)
{
	watchlistRecoveryTest_OneItem_QosChange();
}

TEST_F(WatchlistRecoveryTest, TwoItems_OpenWindow)
{
	watchlistRecoveryTest_TwoItems_OpenWindow();
}

TEST_F(WatchlistRecoveryTest, OneItem_ServiceNameChange)
{
	watchlistRecoveryTest_OneItem_ServiceNameChange();
}

TEST_F(WatchlistRecoveryTest, OneItem_ClosedRecover_PrivateStream)
{
	watchlistRecoveryTest_OneItem_ClosedRecover_PrivateStream();
}

TEST_F(WatchlistRecoveryTest, OneItem_ClosedRecover_SingleOpenOff)
{
	watchlistRecoveryTest_OneItem_ClosedRecover_SingleOpenOff();
}

TEST_F(WatchlistRecoveryTest, PrivateStreamWithPayloadAndExtHeader)
{
	watchlistRecoveryTEST_FrivateStreamWithPayloadAndExtHeader();
}

TEST_F(WatchlistRecoveryTest, OneItem_AllowSuspectDataOff)
{
	watchlistRecoveryTest_OneItem_AllowSuspectDataOff();
}

TEST_F(WatchlistRecoveryTest, TwoItems_ClosedRecoverFromServiceState)
{
	watchlistRecoveryTest_TwoItems_ClosedRecoverFromServiceState();
}

TEST_F(WatchlistRecoveryTest, OneItem_GroupMerge)
{
	watchlistRecoveryTest_OneItem_GroupMerge();
}

TEST_F(WatchlistRecoveryTest, OneItem_LoginClosedRecover_SingleOpen)
{
	watchlistRecoveryTest_OneItem_LoginClosedRecover(RSSL_TRUE);
}

TEST_F(WatchlistRecoveryTest, OneItem_LoginClosedRecover)
{
	watchlistRecoveryTest_OneItem_LoginClosedRecover(RSSL_FALSE);
}

TEST_F(WatchlistRecoveryTest, OneItem_LoginClosed)
{
	watchlistRecoveryTest_OneItem_LoginClosed();
}

TEST_F(WatchlistRecoveryTest, OneItem_LoginCloseRequest_RDMMsg)
{
	watchlistRecoveryTest_OneItem_LoginCloseRequest(RSSL_FALSE);
}

TEST_F(WatchlistRecoveryTest, OneItem_LoginCloseRequest_RsslMsg)
{
	watchlistRecoveryTest_OneItem_LoginCloseRequest(RSSL_TRUE);
}

TEST_F(WatchlistRecoveryTest, OneItem_ClosedRecoverFromGroupState)
{
	watchlistRecoveryTest_OneItem_ClosedRecoverFromGroupState();
}

TEST_F(WatchlistRecoveryTest, OneItem_QosRange)
{
	watchlistRecoveryTest_OneItem_QosRange();
}

TEST_F(WatchlistRecoveryTest, OneItem_NoCapability)
{
	watchlistRecoveryTest_OneItem_NoCapability();
}

TEST_F(WatchlistRecoveryTest, OneItem_NoQosRequest)
{
	watchlistRecoveryTest_OneItem_NoQosRequest();
}

TEST_F(WatchlistRecoveryTest, OneItem_StaticQos)
{
	watchlistRecoveryTest_OneItem_StaticQos();
}

TEST_F(WatchlistRecoveryTest, OneItem_ServiceDeleted_SingleOpen)
{
	watchlistRecoveryTest_OneItem_ServiceDeleted(RSSL_TRUE);
}

TEST_F(WatchlistRecoveryTest, OneItem_ServiceDeleted)
{
	watchlistRecoveryTest_OneItem_ServiceDeleted(RSSL_FALSE);
}

TEST_F(WatchlistRecoveryTest, OneItem_Redirect)
{
	watchlistRecoveryTest_OneItem_Redirect();
}

TEST_F(WatchlistRecoveryTest, OneItem_RedirectToPrivateStream)
{
	watchlistRecoveryTest_OneItem_RedirectToPrivateStream();
}

TEST_F(WatchlistRecoveryTest, OneItem_RedirectToPrivateStream_WithKey)
{
	watchlistRecoveryTest_OneItem_RedirectToPrivateStream_WithKey();
}

TEST_F(WatchlistRecoveryTest, LoginCredentialsUpdate)
{
	watchlistRecoveryTest_LoginCredentialsUpdate();
}

TEST_F(WatchlistRecoveryTest, LoginAuthenticationUpdate)
{
	watchlistRecoveryTest_LoginAuthenticationUpdate();
}

TEST_F(WatchlistRecoveryTest, OneItem_Dictionary)
{
	watchlistRecoveryTest_OneItem_Dictionary();
}

TEST_F(WatchlistRecoveryTest, TwoItems_CloseStateSecond_CloseBothInCallback)
{
	watchlistRecoveryTest_TwoItems_CloseStateSecondInCallback();
}

TEST_F(WatchlistRecoveryTest, OneItem_DisconnectChangeQoS)
{
	watchlistRecoveryTest_OneItem_DisconnectChangeQoS();
}

TEST_F(WatchlistRecoveryTest, OneItem_DisconnectChangeQoS_CloseInCallback)
{
	watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseInCallback();
}

TEST_F(WatchlistRecoveryTest, OneItem_DisconnectChangeQoS_CloseAndRerequestInCallback)
{
	watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseAndRerequestInCallback();
}

TEST_F(WatchlistRecoveryTest, OneItem_ServiceDeleted_ItemCloseAndRerequestInDirectoryCallback)
{
	watchlistRecoveryTest_OneItem_ServiceDeleted_ItemCloseAndRerequestInDirectoryCallback();
}

void watchlistRecoveryTest_OneItem_ClosedRecover()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends closed-recover. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}


void watchlistRecoveryTest_OneItem_RequestTimeout(RsslBool singleOpen)
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;

	RsslRDMLoginStatus *pLoginStatus;
	RsslRDMLoginClose *pLoginClose;
	RsslRDMLoginRequest loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh	loginRefresh;

	RsslRDMDirectoryClose *pDirectoryClose;
	RsslRDMDirectoryRequest *pDirectoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh;
	RsslRDMService service;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());


	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.requestTimeout = 2000;
	sOpts.provideDefaultDirectory = RSSL_FALSE;
	sOpts.provideLoginRefresh = RSSL_FALSE;
	sOpts.singleOpen = singleOpen;
	wtfSetupConnection(&sOpts);

	/*** Test login request timeout. ***/
	/* Wait for login response. Consumer should get open/suspect after a couple seconds. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_TIMEOUT);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginClose = (RsslRDMLoginClose*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginClose->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.rdmMsgType == RDM_LG_MT_CLOSE);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.streamId == wtfGetProviderLoginStream());

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends refresh this time. */
	wtfInitDefaultLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer gets login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID); 

	/*** Test directory request timeout. ***/

	/* Provider receives directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);
	providerItemStream = pDirectoryRequest->rdmMsgBase.streamId;

	/* Wait for directory response. Consumer should get nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(!(pEvent = wtfGetEvent()));

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryClose = (RsslRDMDirectoryClose*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE);
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.streamId == providerItemStream);

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends refresh this time. */
	wtfInitDefaultDirectoryRefresh(&directoryRefresh, &service);
	directoryRefresh.rdmMsgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Send a login reissue now */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_NO_REFRESH;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH);


	/*** Test item request timeout. ***/

	/* Request an item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider does not respond.  Consumer should get status (Open or ClosedRecover,
	 * depending on SingleOpen setting) after a couple seconds. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == (singleOpen ? RSSL_STREAM_OPEN : RSSL_STREAM_CLOSED_RECOVER));
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_TIMEOUT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	if (singleOpen)
	{
		/* Provider receives request again. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	}
	else
		ASSERT_TRUE(!wtfGetEvent());


	wtfFinishTest();
}

void watchlistRecoveryTest_TwoItems_RequestTimeout()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;

	RsslRDMLoginStatus *pLoginStatus;
	RsslRDMLoginClose *pLoginClose;
	RsslRDMLoginRequest *pLoginRequest;
	RsslRDMLoginRefresh	loginRefresh;

	RsslRDMDirectoryClose *pDirectoryClose;
	RsslRDMDirectoryRequest *pDirectoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh;
	RsslRDMService service;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());


	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.requestTimeout = 2000;
	sOpts.provideDefaultDirectory = RSSL_FALSE;
	sOpts.provideLoginRefresh = RSSL_FALSE;
	wtfSetupConnection(&sOpts);

	/*** Test login request timeout. ***/
	/* Wait for login response. Consumer should get open/suspect after a couple seconds. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_TIMEOUT);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginClose = (RsslRDMLoginClose*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginClose->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.rdmMsgType == RDM_LG_MT_CLOSE);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.streamId == wtfGetProviderLoginStream());

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends refresh this time. */
	wtfInitDefaultLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer gets login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID); 

	/*** Test directory request timeout. ***/

	/* Provider receives directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);
	providerItemStream = pDirectoryRequest->rdmMsgBase.streamId;

	/* Wait for directory response. Consumer should get nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(!(pEvent = wtfGetEvent()));

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryClose = (RsslRDMDirectoryClose*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE);
	ASSERT_TRUE(pDirectoryClose->rdmMsgBase.streamId == providerItemStream);

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends refresh this time. */
	wtfInitDefaultDirectoryRefresh(&directoryRefresh, &service);
	directoryRefresh.rdmMsgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/*** Test item request timeout. ***/

	/* Request an item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request an item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives priority change. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider does not respond.  Consumer should get Open/Suspect after a couple seconds. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_TIMEOUT);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_TIMEOUT);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_RequestTimeout_Multipart()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg  refreshMsg, *pRefreshMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;

	WtfSetupConnectionOpts sOpts;
	RsslInt32 i;

	ASSERT_TRUE(wtfStartTest());


	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.requestTimeout = 2000;
	wtfSetupConnection(&sOpts);

	/* Request an item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends partial refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider does not respond.  Consumer should get Open/Suspect after a couple seconds. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_TIMEOUT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	/* Provider receives request again. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Refresh parts should reset the request timeout. */
	for (i = 0; i < 3; ++i)
	{
		/* Provider sends partial refresh. */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED;
		refreshMsg.msgBase.streamId = providerItemStream;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service1Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		/* Consumer receives refresh. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
		ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

		/* Wait a second. */
		wtfDispatch(WTF_TC_CONSUMER, 1500);
	}

	/* Provider sends refresh complete. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Shouldn't get anything else. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);


	wtfFinishTest();
}


void watchlistRecoveryTest_OneItem_ClearCache()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService	service1;
	RsslBuffer		itemNames[] = { { 3, const_cast<char*>("TRI") }, { 4, const_cast<char*>("GOOG") } };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[0];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Request second item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[1];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[0], &pRequestMsg->msgBase.msgKey.name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[1], &pRequestMsg->msgBase.msgKey.name));

	/* Provider re-sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service1;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service1);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives requests again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[0], &pRequestMsg->msgBase.msgKey.name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[1], &pRequestMsg->msgBase.msgKey.name));

	/* Consumer receives no more events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives no more events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_QosChange()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService		service1;
	RsslReactorSubmitMsgOptions opts;
	RsslQos			realTimeQos, delayedQos;

	rsslClearQos(&realTimeQos);
	realTimeQos.timeliness = RSSL_QOS_TIME_REALTIME;
	realTimeQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_DELAYED;
	delayedQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Change QoS on directory to suport only realtime. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();

	wtfSetService1Info(&service1);
	service1.flags = RDM_SVCF_HAS_INFO;
	service1.info.flags = RDM_SVC_IFF_HAS_QOS;
	service1.info.qosList = &realTimeQos;
	service1.info.qosCount = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &realTimeQos));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/*** Recover to No QoS ***/

	/* Provider changes QoS to nothing. */
	wtfSetService1Info(&service1);
	service1.flags = RDM_SVCF_HAS_INFO;
	service1.info.flags = RDM_SVC_IFF_HAS_QOS;
	service1.info.qosCount = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends closed-recover. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	statusMsg.state.code = RSSL_SC_NO_RESOURCES;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status from provider. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NO_RESOURCES);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives Open/Suspect status from recovery attempt. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives nothing yet. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/*** Recover to Delayed QoS ***/

	/* Provider changes QoS to delayed. */
	wtfSetService1Info(&service1);
	service1.flags = RDM_SVCF_HAS_INFO;
	service1.info.flags = RDM_SVC_IFF_HAS_QOS;
	service1.info.qosList = &delayedQos;
	service1.info.qosCount = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &delayedQos));
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_TwoItems_OpenWindow()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerItem1Stream, providerItem2Stream;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService		service1;
	RsslReactorSubmitMsgOptions opts;
	RsslQos			realTimeQos, delayedQos;

	rsslClearQos(&realTimeQos);
	realTimeQos.timeliness = RSSL_QOS_TIME_REALTIME;
	realTimeQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_REALTIME;
	delayedQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Add OpenWindow to directory. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();

	wtfSetService1Info(&service1);
	service1.flags = RDM_SVCF_HAS_LOAD;
	service1.load.flags = RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service1.load.openWindow = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 5;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Request second item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 6;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &realTimeQos));
	providerItem1Stream = pRequestMsg->msgBase.streamId;

	/* Open window is full, so provider does not receive
	 * second request. */
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItem1Stream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider receives second request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &realTimeQos));
	providerItem2Stream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(providerItem1Stream != providerItem2Stream);

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_ServiceNameChange()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService		service1;
	RsslReactorSubmitMsgOptions opts;
	RsslQos			realTimeQos, delayedQos;

	/* Test that we can handle a service name change.
	 * A provider should never do this, but let's try to make sure we do something
	 * that'll work, and at the very least, don't screw up or crash. */

	rsslClearQos(&realTimeQos);
	realTimeQos.timeliness = RSSL_QOS_TIME_REALTIME;
	realTimeQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_REALTIME;
	delayedQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &realTimeQos));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider is takin' away your name (doo-de-doo-de-doo, doo-de-doo-de-doo...). */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;
	wtfSetService1Info(&service1);
	service1.flags = RDM_SVCF_HAS_INFO;
	service1.info.flags = RDM_SVC_IFF_NONE;
	service1.info.serviceName.length = 14;
	service1.info.serviceName.data = const_cast<char*>("SecretAgentMan");

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends closed-recover. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives nothing yet. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider changes service name back. */
	wtfSetService1Info(&service1); /* Should reset name. */
	service1.flags = RDM_SVCF_HAS_INFO;
	service1.info.flags = RDM_SVC_IFF_NONE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(rsslQosIsEqual(&pRequestMsg->qos, &delayedQos));
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_ClosedRecover_PrivateStream()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslBuffer		noService = { 3, const_cast<char*>("MLB") };
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_PRIVATE_STREAM;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends closed-recover. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_PRIVATE_STREAM;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ClosedRecover/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_PRIVATE_STREAM);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider receives no more messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	wtfFinishTest();
}

void watchlistRecoveryTEST_FrivateStreamWithPayloadAndExtHeader()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslReactorSubmitMsgOptions opts;
	RsslBuffer		requestDataBody = { 9, const_cast<char*>("Spaghetti") };
	RsslBuffer		requestDataBody2 = { 6, const_cast<char*>("Dinner") };
	RsslBuffer		requestHeader = { 6, const_cast<char*>("Turkey") };
	RsslBuffer		requestHeader2 = { 3, const_cast<char*>("Sub") };
	RsslInt32		i;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	for (i = 0; i < 3; ++i)
	{
		RsslBool testDataBody;
		RsslBool testExtHeader;

		/* Test sending a data body, then an extended header, then both. */
		switch(i)
		{
			case 0: testDataBody = RSSL_TRUE; testExtHeader = RSSL_FALSE; break;
			case 1: testDataBody = RSSL_FALSE; testExtHeader = RSSL_TRUE; break;
			case 2: testDataBody = RSSL_TRUE; testExtHeader = RSSL_TRUE; break;
			default: ASSERT_TRUE(0);
		}


		/* Request a private stream with a data body & extended header. */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 2;
		requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_PRIVATE_STREAM;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

		if (testDataBody)
		{
			requestMsg.msgBase.containerType = RSSL_DT_OPAQUE;
			requestMsg.msgBase.encDataBody = requestDataBody;
		}
		else
			requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

		if (testExtHeader)
		{
			requestMsg.flags |= RSSL_RQMF_HAS_EXTENDED_HEADER;
			requestMsg.extendedHeader = requestHeader;
		}

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives request. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM);

		if (testDataBody)
		{
			ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(rsslBufferIsEqual(&requestDataBody, &pRequestMsg->msgBase.encDataBody));
		}
		else
			ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);

		if (testExtHeader)
		{
			ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER);
			ASSERT_TRUE(rsslBufferIsEqual(&requestHeader, &pRequestMsg->extendedHeader));
		}
		else
			ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER));

		providerItemStream = pRequestMsg->msgBase.streamId;

		/* Reissue the request with a different data body & extended header. */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 2;
		requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_PRIVATE_STREAM
			| RSSL_RQMF_NO_REFRESH;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

		if (testDataBody)
		{
			requestMsg.msgBase.containerType = RSSL_DT_OPAQUE;
			requestMsg.msgBase.encDataBody = requestDataBody2;
		}
		else
			requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

		if (testExtHeader)
		{
			requestMsg.flags |= RSSL_RQMF_HAS_EXTENDED_HEADER;
			requestMsg.extendedHeader = requestHeader2;
		}

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives request. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM);

		if (testDataBody)
		{
			ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(rsslBufferIsEqual(&requestDataBody2, &pRequestMsg->msgBase.encDataBody));
		}
		else
			ASSERT_TRUE(pRequestMsg->msgBase.containerType == RSSL_DT_NO_DATA);

		if (testExtHeader)
		{
			ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER);
			ASSERT_TRUE(rsslBufferIsEqual(&requestHeader2, &pRequestMsg->extendedHeader));
		}
		else
			ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER));

		/* Close first item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 2;
		closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);
	}

	wtfFinishTest();
}


void watchlistRecoveryTest_OneItem_ClosedRecover_SingleOpenOff()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	WtfSetupConnectionOpts csOpts;
	RsslBuffer		noService = { 3, const_cast<char*>("MLB") };
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.singleOpen = RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	/* Request item from nonexistent service. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &noService;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives ClosedRecover/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends closed-recover. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ClosedRecover/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives no more messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_AllowSuspectDataOff()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslInt32		providerItemStream;
	WtfSetupConnectionOpts csOpts;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.singleOpen = RSSL_FALSE;
	csOpts.allowSuspectData = RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends open-suspect. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ClosedRecover/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives close request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);

	ASSERT_TRUE(!wtfGetEvent());
	wtfFinishTest();
}

void watchlistRecoveryTest_TwoItems_ClosedRecoverFromServiceState()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate directoryUpdate;
	RsslRDMService service1;
	RsslReactorSubmitMsgOptions opts;
	RsslBuffer		itemNames[] = { { 3, const_cast<char*>("TRI") }, { 4, const_cast<char*>("GOOG") } };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[0];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[1];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[0], &pRequestMsg->msgBase.msgKey.name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[1], &pRequestMsg->msgBase.msgKey.name));

	/* Provider sends closed-recover via directory. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.flags = RDM_SVCF_HAS_STATE; 
	service1.state.flags |= RDM_SVC_STF_HAS_STATUS;
	service1.state.status.streamState = RSSL_STREAM_CLOSED_RECOVER;
	service1.state.status.dataState = RSSL_DATA_SUSPECT;
	service1.state.status.text.data = const_cast<char*>("All items closed-recover.");
	service1.state.status.text.length = (RsslUInt32)strlen(service1.state.status.text.data);
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives requests again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	pRequestMsg = &pEvent->rsslMsg.pRsslMsg->requestMsg;
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[0], &pRequestMsg->msgBase.msgKey.name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	pRequestMsg = &pEvent->rsslMsg.pRsslMsg->requestMsg;
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[1], &pRequestMsg->msgBase.msgKey.name));

	/* Consumer receives no more events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives no more events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_GroupMerge()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate directoryUpdate;
	RsslRDMService service1;
	RsslRDMServiceGroupState	groupState;
	RsslReactorSubmitMsgOptions opts;
	unsigned char	nodeId[2] = { 0xd0, 0x0d };
	unsigned char	nodeId2[2] = { 0xf0, 0x0f };
	RsslBuffer		itemGroupId = { 2,  (char*)nodeId};
	RsslBuffer		newGroupId = { 2,  (char*)nodeId2};

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.groupId = itemGroupId;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	
	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->groupId, &itemGroupId));

	/* Provider changes group. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	wtfSetService1Info(&service1);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.flags = RDM_SVCF_NONE;
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.groupStateCount = 1;
	service1.groupStateList = &groupState;

	rsslClearRDMServiceGroupState(&groupState);
	groupState.flags = RDM_SVC_GRF_HAS_MERGED_TO_GROUP;
	groupState.group = itemGroupId;
	groupState.mergedToGroup = newGroupId;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends closed-recover to old group. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMServiceGroupState(&groupState);
	groupState.flags |= RDM_SVC_GRF_HAS_STATUS;
	groupState.group = itemGroupId;
	groupState.status.streamState = RSSL_STREAM_CLOSED_RECOVER;
	groupState.status.dataState = RSSL_DATA_SUSPECT;
	groupState.status.text.data = const_cast<char*>("Group closed-recover.");
	groupState.status.text.length = (RsslUInt32)strlen(groupState.status.text.data);
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.groupStateCount = 1;
	service1.groupStateList = &groupState;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends closed-recover to new group. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMServiceGroupState(&groupState);
	groupState.flags |= RDM_SVC_GRF_HAS_STATUS;
	groupState.group = newGroupId;
	groupState.status.streamState = RSSL_STREAM_CLOSED_RECOVER;
	groupState.status.dataState = RSSL_DATA_SUSPECT;
	groupState.status.text.data = const_cast<char*>("Group closed-recover.");
	groupState.status.text.length = (RsslUInt32)strlen(groupState.status.text.data);
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.groupStateCount = 1;
	service1.groupStateList = &groupState;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_ClosedRecoverFromGroupState()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate directoryUpdate;
	RsslRDMService service1;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	unsigned char	nodeId[2] = { 0xd0, 0x0d };
	RsslBuffer		itemGroupId = { 2,  (char*)nodeId};
	RsslRDMServiceGroupState groupState;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.groupId = itemGroupId;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	
	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->groupId, &itemGroupId));

	/* Provider sends closed-recover via group. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMServiceGroupState(&groupState);
	groupState.flags |= RDM_SVC_GRF_HAS_STATUS;
	groupState.group = itemGroupId;
	groupState.status.streamState = RSSL_STREAM_CLOSED_RECOVER;
	groupState.status.dataState = RSSL_DATA_SUSPECT;
	groupState.status.text.data = const_cast<char*>("Group closed-recover.");
	groupState.status.text.length = (RsslUInt32)strlen(groupState.status.text.data);
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.groupStateCount = 1;
	service1.groupStateList = &groupState;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_LoginClosedRecover(RsslBool singleOpen)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent			*pEvent;
	RsslRequestMsg		requestMsg, *pRequestMsg;
	RsslStatusMsg		*pStatusMsg;
	RsslInt32			providerItemStream, providerLoginStream, providerDirectoryStream;
	RsslUInt32 providerDirectoryFilter;
	RsslRDMLoginStatus	loginStatus, *pLoginStatus;
	RsslRDMLoginRequest *pLoginRequest;
	RsslRDMLoginRefresh loginRefresh, *pLoginRefresh;
	WtfSetupConnectionOpts	csOpts;
	RsslTimeValue				startTimeUsec;
	int i;
	const RsslInt32 		reconnectMinDelay = 1000, reconnectMaxDelay = 3000;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.singleOpen = singleOpen;
	csOpts.reconnectMinDelay = reconnectMinDelay;
	csOpts.reconnectMaxDelay = reconnectMaxDelay;
	wtfSetupConnection(&csOpts);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Make sure we back off on reconnecting if we don't get past login. */
	for (i = 0; i < 2; ++i)
	{

		/* Provider sends login closed recover. */
		rsslClearRDMLoginStatus(&loginStatus);
		loginStatus.rdmMsgBase.streamId = wtfGetProviderLoginStream();
		loginStatus.flags = RDM_LG_STF_HAS_STATE;
		loginStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		loginStatus.state.dataState = RSSL_DATA_SUSPECT;
		loginStatus.state.text.data = const_cast<char*>("I have no gate key.");
		loginStatus.state.text.length = (RsslUInt32)strlen(loginStatus.state.text.data);

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&loginStatus;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(!wtfGetEvent());

		/* Consumer receives Open/Suspect login status. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
		ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
		ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
		ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);

		if (i == 0)
		{
			/* First time, consumer receives item status (Open/Suspect or ClosedRecover/Suspect based on Single-Open 
			 * setting). */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
			ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
			ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
			ASSERT_TRUE(pStatusMsg->state.streamState == (singleOpen ? RSSL_STREAM_OPEN : RSSL_STREAM_CLOSED_RECOVER));
			ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
			ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		}

		/* Consumer receives channel down/reconnecting event */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

		startTimeUsec = rsslGetTimeMicro();

		/* Provider receives channel-down event. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
		/* Provider accepts reconnect. */
		wtfAcceptWithTime((time_t)(reconnectMinDelay * pow((double)2,i) + 100));

		/* Provider receives channel-up & ready events. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

		/* Test that the reconnect occurred at the expected time.
		 * It should double for each reconnection (up to the max, not accounted for here). */
		ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000.0 > reconnectMinDelay * pow((double)2,i) - 300);
		ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000.0 < reconnectMinDelay * pow((double)2,i) + 300);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Consumer receives channel-up & ready events. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Provider receives relogin. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
		ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
		providerLoginStream = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 
	}

	/*** Now, make sure we don't back off if we DO get past login
	 * (same test as above, except provider accepts the login request before closing it). 
	 * Do two attempts.  ***/

	/*** On the first attempt, test login refresh & status back-to-back
	 * (unlikely, but did encounter a timing condition here). ***/

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = pLoginRequest->userName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.state.text.data = const_cast<char*>("Oh, you mean this gate key.");
	loginRefresh.state.text.length = (RsslUInt32)strlen(loginRefresh.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Provider sends login closed recover. */
	rsslClearRDMLoginStatus(&loginStatus);
	loginStatus.rdmMsgBase.streamId = wtfGetProviderLoginStream();
	loginStatus.flags = RDM_LG_STF_HAS_STATE;
	loginStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	loginStatus.state.dataState = RSSL_DATA_SUSPECT;
	loginStatus.state.text.data = const_cast<char*>("I have no gate key.");
	loginStatus.state.text.length = (RsslUInt32)strlen(loginStatus.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer receives channel down/reconnecting event */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	startTimeUsec = rsslGetTimeMicro();

	/* Provider receives channel-down event. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
	/* Provider accepts reconnect. */
	wtfAcceptWithTime(reconnectMinDelay + 100);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Test that the reconnect occurred at the expected time.
	 * It should be the minimum delay. */
	ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000 > reconnectMinDelay - 300);
	ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000 < reconnectMinDelay + 300);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives relogin. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	providerLoginStream = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	/*** On the second attempt, exchange directory request/refresh through
	 * before sending login status. ***/

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = pLoginRequest->userName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.state.text.data = const_cast<char*>("Oh, you mean this gate key.");
	loginRefresh.state.text.length = (RsslUInt32)strlen(loginRefresh.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Provider sends login closed recover. */
	rsslClearRDMLoginStatus(&loginStatus);
	loginStatus.rdmMsgBase.streamId = wtfGetProviderLoginStream();
	loginStatus.flags = RDM_LG_STF_HAS_STATE;
	loginStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	loginStatus.state.dataState = RSSL_DATA_SUSPECT;
	loginStatus.state.text.data = const_cast<char*>("I have no gate key.");
	loginStatus.state.text.length = (RsslUInt32)strlen(loginStatus.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives Open/Suspect login status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer receives channel down/reconnecting event */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	startTimeUsec = rsslGetTimeMicro();

	/* Provider receives channel-down event. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
	/* Provider accepts reconnect. */
	wtfAcceptWithTime(reconnectMinDelay + 100);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Test that the reconnect occurred at the expected time.
	 * It should be the minimum delay, same as before. */
	ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000 > reconnectMinDelay - 300);
	ASSERT_TRUE((pEvent->base.timeUsec - startTimeUsec)/1000 < reconnectMinDelay + 300);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives relogin. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	providerLoginStream = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = pLoginRequest->userName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.state.text.data = const_cast<char*>("Oh, you mean this gate key.");
	loginRefresh.state.text.length = (RsslUInt32)strlen(loginRefresh.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	providerDirectoryStream = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 
	providerDirectoryFilter = pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter; 

	/* Provider sends directory refresh.
	 * TODO Standardize this. It's the same stuff we do in the framework connect setup. */
	{
		RsslRDMDirectoryRefresh directoryRefresh;
		RsslRDMService service;

		rsslClearRDMDirectoryRefresh(&directoryRefresh);
		directoryRefresh.rdmMsgBase.streamId = providerDirectoryStream;
		directoryRefresh.filter = providerDirectoryFilter;
		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		wtfSetService1Info(&service);

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	}

	/* Process directory(consumer receives nothing). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);

	if (singleOpen)
	{
		/* Provider receives request again. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		providerItemStream = pRequestMsg->msgBase.streamId;
	}
	else
		ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_LoginClosed()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent				*pEvent;
	RsslRequestMsg			requestMsg, *pRequestMsg;
	RsslStatusMsg			*pStatusMsg;
	RsslInt32				providerItemStream, providerLoginStream;
	RsslRDMLoginStatus		loginStatus, *pLoginStatus;
	RsslRDMLoginRequest 	loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh 	loginRefresh, *pLoginRefresh;
	RsslRDMDirectoryRequest	*pDirectoryRequest;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService			service;
	RsslInt32				directoryStreamId;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends login closed. */
	rsslClearRDMLoginStatus(&loginStatus);
	loginStatus.rdmMsgBase.streamId = wtfGetProviderLoginStream();
	loginStatus.flags = RDM_LG_STF_HAS_STATE;
	loginStatus.state.streamState = RSSL_STREAM_CLOSED;
	loginStatus.state.dataState = RSSL_DATA_SUSPECT;
	loginStatus.state.text.data = const_cast<char*>("Log in de Nile");
	loginStatus.state.text.length = (RsslUInt32)strlen(loginStatus.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives Closed login status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer receives Closed item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/*** Make sure we can log back in, and that the items are not recovered. ***/

	/* Consumer logs back in. */
	wtfInitDefaultLoginRequest(&loginRequest);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStream = pLoginRequest->rdmMsgBase.streamId;

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	directoryStreamId = pDirectoryRequest->rdmMsgBase.streamId;

	/* Provider sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = directoryStreamId;
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;
	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer & Provider receive nothing. No recovery occurs. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Request item again. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_LoginClosed_Directory()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent				*pEvent;
	RsslInt32				providerLoginStream;
	RsslRDMLoginStatus		loginStatus, *pLoginStatus;
	RsslRDMLoginRequest 	loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh 	loginRefresh, *pLoginRefresh;
	RsslRDMDirectoryRequest	*pDirectoryRequest;
	WtfSetupConnectionOpts	sOpts;

	/* Test that the directory stream is properly closed when
	 * a login close is received. The provider avoids sending a response,
	 * so we can test that the stream's timeout is properly removed. */

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.requestTimeout = 2000;
	sOpts.provideDefaultDirectory = RSSL_FALSE;
	wtfSetupConnection(&sOpts);

	/* Provider sends login closed. */
	rsslClearRDMLoginStatus(&loginStatus);
	loginStatus.rdmMsgBase.streamId = wtfGetProviderLoginStream();
	loginStatus.flags = RDM_LG_STF_HAS_STATE;
	loginStatus.state.streamState = RSSL_STREAM_CLOSED;
	loginStatus.state.dataState = RSSL_DATA_SUSPECT;
	loginStatus.state.text.data = const_cast<char*>("Log in de Nile");
	loginStatus.state.text.length = (RsslUInt32)strlen(loginStatus.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives Closed login status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_CLOSED);

	/* Ensure that no timeouts fire. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer logs back in. */
	wtfInitDefaultLoginRequest(&loginRequest);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStream = pLoginRequest->rdmMsgBase.streamId;

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);


	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_LoginCloseRequest(RsslBool useRsslMsg)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent			*pEvent;
	RsslRequestMsg		requestMsg, *pRequestMsg;
	RsslRefreshMsg		refreshMsg;
	RsslInt32			providerItemStream;
	RsslRDMLoginClose	*pLoginClose, loginClose;
	RsslCloseMsg		closeMsg;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Consumer closes login stream. */
	if (useRsslMsg)
	{
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 1;
		closeMsg.msgBase.domainType = RSSL_DMT_LOGIN;
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
	}
	else
	{
		rsslClearRDMLoginClose(&loginClose);
		loginClose.rdmMsgBase.streamId = 1;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&loginClose;
	}
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider sends refresh for item. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Provider receives login close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginClose = (RsslRDMLoginClose*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginClose->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.rdmMsgType == RDM_LG_MT_CLOSE);
	ASSERT_TRUE(pLoginClose->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer should not get the refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_DirectoryClosedRecover()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent			*pEvent;
	RsslRequestMsg		requestMsg, *pRequestMsg;
	RsslStatusMsg		*pStatusMsg;
	RsslInt32			providerItemStream, providerDirectoryStream;
	RsslUInt32 			providerDirectoryFilter;
	RsslRDMDirectoryStatus	directoryStatus;
	RsslRDMDirectoryRequest	*pDirectoryRequest;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends directory closed recover. */
	directoryStatus.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryStatus.flags = RDM_DR_STF_HAS_STATE | RDM_DR_STF_HAS_FILTER;
	directoryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	directoryStatus.filter = wtfGetProviderDirectoryFilter();
	directoryStatus.state.dataState = RSSL_DATA_SUSPECT;
	directoryStatus.state.text.data = const_cast<char*>("These are not the services you're looking for.");
	directoryStatus.state.text.length = (RsslUInt32)strlen(directoryStatus.state.text.data);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect item status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	providerDirectoryStream = pDirectoryRequest->rdmMsgBase.streamId; 
	providerDirectoryFilter = pDirectoryRequest->filter; 

	/* Provider sends directory refresh.
	 * TODO Standardize this. It's the same stuff we do in the framework connect setup. */
	{
		RsslRDMDirectoryRefresh directoryRefresh;
		RsslRDMService service;

		rsslClearRDMDirectoryRefresh(&directoryRefresh);
		directoryRefresh.rdmMsgBase.streamId = providerDirectoryStream;
		directoryRefresh.filter = providerDirectoryFilter;
		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		wtfSetService1Info(&service);

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	}

	/* Process directory(consumer receives nothing). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	pRequestMsg = &pEvent->rsslMsg.pRsslMsg->requestMsg;
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_QosRange()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	statusMsg, *pStatusMsg;
	RsslInt32		providerItemStream;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
	requestMsg.worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS));
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends closed. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Closed status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_NoCapability()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg;
	RsslStatusMsg	*pStatusMsg;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request item on some unsupported domain. 
	 * (The chosen domain is 0, so if this test ever appears to fail -- it may be because
	 * 0 was added to the default service capabilities).*/
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = 0;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
	requestMsg.worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_NoQosRequest()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	statusMsg, *pStatusMsg;
	RsslInt32		providerItemStream;

	ASSERT_TRUE(wtfStartTest());

	/* Same test as watchlistRecoveryTest_OneItem_QosRange,
	 * but request no QoS at all instead of a QoS range. */

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS));
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends closed. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_StaticQos()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	statusMsg, *pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	unsigned char	nodeId[2] = { 0xd0, 0x0d };
	RsslBuffer		itemGroupId = { 2,  (char*)nodeId};

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
	requestMsg.worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS));
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.groupId = itemGroupId;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Provider sends closed. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->groupId, &itemGroupId));

	/* Consumer receives Closed status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_ServiceDeleted(RsslBool singleOpen)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryUpdate directoryUpdate;
	RsslRDMService service1;
	RsslRefreshMsg *pRefreshMsg, refreshMsg;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.singleOpen = singleOpen;
	wtfSetupConnection(&sOpts);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider deletes service. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.action = RSSL_MPEA_DELETE_ENTRY; 
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	if (singleOpen)
	{
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	}
	else
	{
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	}

	/* Provider restores service. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	wtfSetService1Info(&service1);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Watchlist receives directory update and recovers item
	 * (no message is recevied, consumer has no directory stream open). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	
	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	if (singleOpen)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		providerItemStream = pRequestMsg->msgBase.streamId;

		/* Provider sends refresh. */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
			| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
		refreshMsg.msgBase.streamId = providerItemStream;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = service1Id;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		/* Consumer receives refresh. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	}
	else
	{
		ASSERT_TRUE(!wtfGetEvent());
	}

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_Redirect()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslBuffer		oldItemName = { 7, const_cast<char*>("RTRSY.O") };
	RsslBuffer		newItemName = { 5, const_cast<char*>("TRI.N") };
	RsslMsgKey		*pNewMsgKey;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = oldItemName;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &oldItemName));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends redirect. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
	statusMsg.state.streamState = RSSL_STREAM_REDIRECTED;
	statusMsg.state.dataState = RSSL_DATA_OK;
	statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	statusMsg.msgBase.msgKey.name = newItemName;
	statusMsg.msgBase.msgKey.serviceId = service1Id;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Redirect status, with new key info. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_REDIRECTED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	pNewMsgKey = &pStatusMsg->msgBase.msgKey;
	ASSERT_TRUE(pNewMsgKey->flags == (RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME));
	ASSERT_TRUE(pNewMsgKey->serviceId == service1Id);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(rsslBufferIsEqual(&pNewMsgKey->name, &newItemName));

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_RedirectToPrivateStream()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslBuffer		itemName = { 7, const_cast<char*>("RTRSY.O") };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends redirect. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_PRIVATE_STREAM;
	statusMsg.state.streamState = RSSL_STREAM_REDIRECTED;
	statusMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives redirect, with no key. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_PRIVATE_STREAM);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_REDIRECTED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_RedirectToPrivateStream_WithKey()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg	*pStatusMsg, statusMsg;
	RsslInt32		providerItemStream;
	RsslBuffer		oldItemName = { 7, const_cast<char*>("RTRSY.O") };
	RsslBuffer		newItemName = { 5, const_cast<char*>("TRI.N") };
	RsslMsgKey		*pNewMsgKey;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = oldItemName;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &oldItemName));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends redirect. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY | RSSL_STMF_PRIVATE_STREAM;
	statusMsg.state.streamState = RSSL_STREAM_REDIRECTED;
	statusMsg.state.dataState = RSSL_DATA_OK;
	statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	statusMsg.msgBase.msgKey.name = newItemName;
	statusMsg.msgBase.msgKey.serviceId = service1Id;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives Open/Suspect status, with new key info. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_PRIVATE_STREAM);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_REDIRECTED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	pNewMsgKey = &pStatusMsg->msgBase.msgKey;
	ASSERT_TRUE(pNewMsgKey->flags == (RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME));
	ASSERT_TRUE(pNewMsgKey->serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pNewMsgKey->name, &newItemName));

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_Dictionary()
{
	WtfSetupConnectionOpts		setupOpts;
	RsslReactorSubmitMsgOptions opts;
	WtfEvent					*pEvent;
	RsslRDMDictionaryRequest	dictionaryRequest, *pDictionaryRequest;
	RsslRDMDictionaryRefresh	dictionaryRefresh, *pDictionaryRefresh;
	RsslRDMDictionaryStatus		dictionaryStatus, *pDictionaryStatus;
	RsslInt32					providerDictStream;
	unsigned char				nodeId[2] = { 0xd0, 0x0d };
	RsslBuffer					itemGroupId = { 2,  (char*)nodeId};
	RsslBuffer					dictionaryName = { 6, const_cast<char*>("RWFFld") };

	/* Test sending the RDMFieldDictionary in pieces. */

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&setupOpts);

	/* Due to the large amount of time spent in this test checking that no unexpected
	 * messages were received, it may not complete in time when we slow it down. 
	 * Use a larger request timeout to reduce the chance of this. */
	setupOpts.requestTimeout = 120000;
	wtfSetupConnection(&setupOpts);

	/* Request dictionary. */
	rsslClearRDMDictionaryRequest(&dictionaryRequest);
	dictionaryRequest.rdmMsgBase.streamId = 2;
	dictionaryRequest.dictionaryName = dictionaryName;
	dictionaryRequest.flags = RDM_DC_RQF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&dictionaryRequest;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDictionaryRequest = (RsslRDMDictionaryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDictionaryRequest->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
	ASSERT_TRUE(pDictionaryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDictionaryRequest->serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pDictionaryRequest->dictionaryName, &dictionaryName));
	providerDictStream = pDictionaryRequest->rdmMsgBase.streamId;

	/* Provider sends closed-recover status. */
	rsslClearRDMDictionaryStatus(&dictionaryStatus);
	dictionaryStatus.rdmMsgBase.streamId = providerDictStream;
	dictionaryStatus.flags |= RDM_DC_STF_HAS_STATE;
	dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&dictionaryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives open/suspect (hasn't received dictionary yet). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDictionaryStatus = (RsslRDMDictionaryStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.rdmMsgType == RDM_DC_MT_STATUS);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE);
	ASSERT_TRUE(pDictionaryStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDictionaryStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider receives dictionary request again. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDictionaryRequest = (RsslRDMDictionaryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDictionaryRequest->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
	ASSERT_TRUE(pDictionaryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDictionaryRequest->serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pDictionaryRequest->dictionaryName, &dictionaryName));
	providerDictStream = pDictionaryRequest->rdmMsgBase.streamId;

	/* Prepare dictionary refresh. */
	rsslClearRDMDictionaryRefresh(&dictionaryRefresh);
	dictionaryRefresh.rdmMsgBase.streamId = providerDictStream;
	dictionaryRefresh.serviceId = service1Id;
	dictionaryRefresh.flags |= RDM_DC_RFF_SOLICITED;
	dictionaryRefresh.dictionaryName = dictionaryName;
	dictionaryRefresh.type = RDM_DICTIONARY_FIELD_DEFINITIONS;
	dictionaryRefresh.pDictionary = wtfGetDictionary();

	while (dictionaryRefresh.startFid <= dictionaryRefresh.pDictionary->maxFid)
	{
		/* Provider sends refresh piece. */
		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&dictionaryRefresh;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		/* Consumer receives a refresh piece. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDictionaryRefresh = (RsslRDMDictionaryRefresh*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDictionaryRefresh->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
		ASSERT_TRUE(pDictionaryRefresh->rdmMsgBase.rdmMsgType == RDM_DC_MT_REFRESH);
		ASSERT_TRUE(pDictionaryRefresh->rdmMsgBase.streamId == 2);
		ASSERT_TRUE(rsslBufferIsEqual(&pDictionaryRefresh->dictionaryName, &dictionaryName));
		ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
		ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)(int)0xfaabd00d);
	}

	/* Provider sends suspect status. */
	rsslClearRDMDictionaryStatus(&dictionaryStatus);
	dictionaryStatus.rdmMsgBase.streamId = providerDictStream;
	dictionaryStatus.flags |= RDM_DC_STF_HAS_STATE;
	dictionaryStatus.state.streamState = RSSL_STREAM_OPEN;
	dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&dictionaryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives suspect status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDictionaryStatus = (RsslRDMDictionaryStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.rdmMsgType == RDM_DC_MT_STATUS);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE);
	ASSERT_TRUE(pDictionaryStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDictionaryStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends closed-recover status. */
	rsslClearRDMDictionaryStatus(&dictionaryStatus);
	dictionaryStatus.rdmMsgBase.streamId = providerDictStream;
	dictionaryStatus.flags |= RDM_DC_STF_HAS_STATE;
	dictionaryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	dictionaryStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&dictionaryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives closed-recover status. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDictionaryStatus = (RsslRDMDictionaryStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.rdmMsgType == RDM_DC_MT_STATUS);
	ASSERT_TRUE(pDictionaryStatus->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE);
	ASSERT_TRUE(pDictionaryStatus->state.streamState == RSSL_STREAM_CLOSED_RECOVER);
	ASSERT_TRUE(pDictionaryStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)(int)0xfaabd00d);


	wtfFinishTest();
}

void watchlistRecoveryTest_LoginCredentialsUpdate()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent *pEvent;
	RsslRDMLoginRequest loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh loginRefresh, *pLoginRefresh;
	RsslRDMDirectoryRequest	*pDirectoryRequest;

	RsslInt32 providerLoginStreamId;
	RsslInt32 providerDirectoryStreamId;
	RsslUInt32 providerDirectoryFilter;

	RsslBuffer userToken = { 7, const_cast<char*>("RedPill") };
	RsslBuffer userToken2 = { 8, const_cast<char*>("BluePill") };
	RsslBuffer userToken3 = { 6, const_cast<char*>("Cookie") };
	RsslBuffer userToken4 = { 5, const_cast<char*>("Spoon") };

	WtfSetupConnectionOpts csOpts;

	/* Apart from the use of the USER_TOKEN username type, this test begins much the same
	 * way as the standard framework.  The actual test of updating it is further below. */

	ASSERT_TRUE(wtfStartTest());
	
	/* Consumer connects. */
	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.login = RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	/* Consumer submits login. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRequest.userName = userToken;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider should now accept. */
	wtfAccept();

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider Channel up, ready, & login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken));
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_USERNAME 
		| RDM_LG_RFF_HAS_USERNAME_TYPE;
	loginRefresh.userName = userToken;
	loginRefresh.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRefresh->userName, &userToken));

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter = 
		pDirectoryRequest->filter;
	ASSERT_TRUE(providerDirectoryFilter ==
				(RDM_DIRECTORY_SERVICE_INFO_FILTER
				| RDM_DIRECTORY_SERVICE_STATE_FILTER
				| RDM_DIRECTORY_SERVICE_GROUP_FILTER
				| RDM_DIRECTORY_SERVICE_LOAD_FILTER
				| RDM_DIRECTORY_SERVICE_DATA_FILTER
				| RDM_DIRECTORY_SERVICE_LINK_FILTER));
	
	/* Request should not have service ID. */
	ASSERT_TRUE(!(pDirectoryRequest->flags & RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	{
		RsslRDMDirectoryRefresh directoryRefresh;
		RsslRDMService service;

		rsslClearRDMDirectoryRefresh(&directoryRefresh);
		directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
		directoryRefresh.filter = providerDirectoryFilter;
		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		wtfSetService1Info(&service);

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	}

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	/*** Test of token changes starts here. ***/

	/* Consumer updates token. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRequest.userName = userToken2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Consumer updates token again. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE | RDM_LG_RQF_NO_REFRESH;
	loginRequest.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRequest.userName = userToken3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Consumer updates token one more time. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRequest.userName = userToken4;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives first token update. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	pLoginRequest = &pEvent->rdmMsg.pRdmMsg->loginMsg.request;
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(!(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH));
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken2));
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == providerLoginStreamId);

	/* Provider responds to first token update. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_USERNAME 
		| RDM_LG_RFF_HAS_USERNAME_TYPE;
	loginRefresh.userName = userToken2;
	loginRefresh.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Consumer receives refresh with new token. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRefresh->userName, &userToken2));

	/* Provider does not receive second token update */

	/* Provider receives final token update. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	pLoginRequest = &pEvent->rdmMsg.pRdmMsg->loginMsg.request;
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken4));
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == providerLoginStreamId);

	/* Provider responds to final token update. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_USERNAME 
		| RDM_LG_RFF_HAS_USERNAME_TYPE;
	loginRefresh.userName = userToken4;
	loginRefresh.userNameType = RDM_LOGIN_USER_TOKEN;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh with final token. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRefresh->userName, &userToken4));

	wtfFinishTest();
	
}

void watchlistRecoveryTest_LoginAuthenticationUpdate()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent *pEvent;
	RsslRDMLoginRequest loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh loginRefresh, *pLoginRefresh;
	RsslRDMDirectoryRequest	*pDirectoryRequest;

	RsslInt32 providerLoginStreamId;
	RsslInt32 providerDirectoryStreamId;
	RsslUInt32 providerDirectoryFilter;

	RsslBuffer userToken = { 7, const_cast<char*>("RedPill") };
	RsslBuffer userToken2 = { 8, const_cast<char*>("BluePill") };
	RsslBuffer userToken3 = { 6, const_cast<char*>("Cookie") };
	RsslBuffer userToken4 = { 5, const_cast<char*>("Spoon") };
	
	RsslBuffer userExt1 = { 8, const_cast<char*>("Superman")};
	RsslBuffer userExt2 = { 5, const_cast<char*>("Flash") };
	RsslBuffer userExt3 = { 6, const_cast<char*>("Batman") };
	
	RsslBuffer extResp1 = { 5, const_cast<char*>("Candy") };
	RsslBuffer extResp4 = { 6, const_cast<char*>("Steak") };
	
	RsslUInt TTReissue = 15000000;
	

	WtfSetupConnectionOpts csOpts;

	/* Apart from the use of the USER_TOKEN username type, this test begins much the same
	 * way as the standard framework.  The actual test of updating it is further below. */

	ASSERT_TRUE(wtfStartTest());
	
	/* Consumer connects. */
	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.login = RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	/* Consumer submits login. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRequest.userName = userToken;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider should now accept. */
	wtfAccept();

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider Channel up, ready, & login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken));
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_AUTHN_TT_REISSUE 
		| RDM_LG_RFF_HAS_USERNAME_TYPE | RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP;
	loginRefresh.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRefresh.authenticationExtendedResp = extResp1;
	loginRefresh.authenticationTTReissue = TTReissue;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRefresh->authenticationExtendedResp, &extResp1));
	ASSERT_TRUE(pLoginRefresh->authenticationTTReissue == TTReissue);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter = 
		pDirectoryRequest->filter;
	ASSERT_TRUE(providerDirectoryFilter ==
				(RDM_DIRECTORY_SERVICE_INFO_FILTER
				| RDM_DIRECTORY_SERVICE_STATE_FILTER
				| RDM_DIRECTORY_SERVICE_GROUP_FILTER
				| RDM_DIRECTORY_SERVICE_LOAD_FILTER
				| RDM_DIRECTORY_SERVICE_DATA_FILTER
				| RDM_DIRECTORY_SERVICE_LINK_FILTER));
	
	/* Request should not have service ID. */
	ASSERT_TRUE(!(pDirectoryRequest->flags & RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId; 

	{
		RsslRDMDirectoryRefresh directoryRefresh;
		RsslRDMService service;

		rsslClearRDMDirectoryRefresh(&directoryRefresh);
		directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
		directoryRefresh.filter = providerDirectoryFilter;
		directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.serviceList = &service;
		directoryRefresh.serviceCount = 1;

		wtfSetService1Info(&service);

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	}

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	/*** Test of token changes starts here. ***/

	/* Consumer updates token. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRequest.userName = userToken2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Consumer updates token again. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE | RDM_LG_RQF_NO_REFRESH;
	loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRequest.userName = userToken3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Consumer updates token one more time. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRequest.userName = userToken4;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives first token update. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	pLoginRequest = &pEvent->rdmMsg.pRdmMsg->loginMsg.request;
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(!(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH));
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken2));
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == providerLoginStreamId);

	/* Provider responds to first token update. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_AUTHN_TT_REISSUE 
		| RDM_LG_RFF_HAS_USERNAME_TYPE;
	loginRefresh.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.authenticationTTReissue = TTReissue;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Consumer receives refresh with new token. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE);
	ASSERT_TRUE(pLoginRefresh->authenticationTTReissue = TTReissue);

	/* Provider does not receive second token update */

	/* Provider receives final token update. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	pLoginRequest = &pEvent->rdmMsg.pRdmMsg->loginMsg.request;
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRequest->userName, &userToken4));
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == providerLoginStreamId);

	/* Provider responds to final token update. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags = RDM_LG_RFF_SOLICITED | RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP 
		| RDM_LG_RFF_HAS_USERNAME_TYPE;
	loginRefresh.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	loginRefresh.rdmMsgBase.streamId = providerLoginStreamId;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.authenticationExtendedResp = extResp4;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh with final token. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE);
	ASSERT_TRUE(pLoginRefresh->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP);
	ASSERT_TRUE(rsslBufferIsEqual(&pLoginRefresh->authenticationExtendedResp, &extResp4));

	wtfFinishTest();
	
}

void watchlistRecoveryTest_UnknownStream()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslCloseMsg	*pCloseMsg;
	RsslStatusMsg	statusMsg;
	RsslInt32		providerItemStream = 5;

	ASSERT_TRUE(wtfStartTest());

	/* Send a message on an unknown stream. */

	wtfSetupConnection(NULL);

	/* Make sure test is using a stream that isn't open. */
	ASSERT_TRUE(providerItemStream != wtfGetProviderLoginStream());
	ASSERT_TRUE(providerItemStream != wtfGetProviderDirectoryStream());

	/* Provider sends message. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives nothing(watchlist ignores
	 * the above message). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Now try a case where the provider closes the unknown stream.
	 * If a provider closes the stream at the same time a consumer
	 * reissues, the reissue might be misinterpreted as a new
	 * request.  So the watchlist will automatically close
	 * streams where any non-open state is seen. */

	/* Provider sends message(with closed state). */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives close request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);


	wtfFinishTest();
}



void watchlistRecoveryTest_OneItem_Disconnect(RsslBool singleOpen)
{
	/* Test message fanout in response to disconnection. */

	RsslReactorSubmitMsgOptions opts;
	WtfEvent				*pEvent;
	RsslRequestMsg			requestMsg, *pRequestMsg;
	RsslStatusMsg			*pStatusMsg;
	RsslRDMLoginRequest		*pLoginRequest;
	RsslRDMLoginStatus		*pLoginStatus;
	RsslRDMDirectoryRequest	directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	*pDirectoryUpdate;
	RsslRDMService			*serviceList;
	WtfSetupConnectionOpts	csOpts;
	const RsslInt32 		reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
	WtfChannelEvent 		*pChannelEvent;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.singleOpen = singleOpen;
	csOpts.reconnectMinDelay = reconnectMinDelay;
	csOpts.reconnectMaxDelay = reconnectMaxDelay;
	wtfSetupConnection(&csOpts);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	/* Close channel from provider. */
	wtfCloseChannel(WTF_TC_PROVIDER);

	wtfDispatch(WTF_TC_CONSUMER, 100);



	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == (singleOpen ? RSSL_STREAM_OPEN : RSSL_STREAM_CLOSED_RECOVER));
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	/* Wait for channel to come back. */
	wtfAcceptWithTime(reconnectMinDelay * 2);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives relogin. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	/* Close channel from provider. */
	wtfCloseChannel(WTF_TC_PROVIDER);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives channel event (and nothing else; login stream not re-established). */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void _watchlistRecoveryTest_TwoItems_Callback_CloseSecondStream()
{
	RsslReactorSubmitMsgOptions opts;
	RsslCloseMsg closeMsg;

	/* TODO Use ReactorSubmitMsg instead of wtfSubmitMsg */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);


	wtfSetConsumerMsgCallbackAction(NULL);
}

void watchlistRecoveryTest_TwoItems_CloseStateSecondInCallback()
{
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslStatusMsg statusMsg, *pStatusMsg;
	RsslInt32		providerFirstItemStream, providerSecondItemStream;

	RsslReactorSubmitMsgOptions opts;
	RsslBuffer		itemNames[] = { { 3, const_cast<char*>("TRI") }, { 4, const_cast<char*>("GOOG") } };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[0];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemNames[1];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	providerFirstItemStream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[0], &pRequestMsg->msgBase.msgKey.name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	providerSecondItemStream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&itemNames[1], &pRequestMsg->msgBase.msgKey.name));

	/* Provider sends status message with CLOSED/SUSPECT for first request. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerFirstItemStream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	// Set callback behavior to close the SECOND item upon next message
	wtfSetConsumerMsgCallbackAction((void*)_watchlistRecoveryTest_TwoItems_Callback_CloseSecondStream);

	/* Consumer receives status message for the first item. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);

	/* Consumer receives no more events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistRecoveryTest_OneItem_DisconnectChangeQoS()
{
	/* Test message fanout in response to disconnection. */

	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg			requestMsg, * pRequestMsg;
	RsslStatusMsg* pStatusMsg;
	RsslRDMLoginRequest* pLoginRequest;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginRefresh* pLoginRefresh;
	RsslRDMDirectoryRequest	directoryRequest;
	RsslRDMDirectoryRequest* pDirectoryRequest;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMDirectoryRefresh* pDirectoryRefresh;
	RsslRDMDirectoryUpdate* pDirectoryUpdate;
	RsslRDMService* serviceList;
	WtfSetupConnectionOpts	csOpts;
	const RsslInt32 		reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
	WtfChannelEvent* pChannelEvent;
	RsslRDMService			service;
	RsslUInt32 providerLoginStream;
	RsslUInt32 directoryStreamId;
	RsslQos			delayedQos;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_DELAYED;
	delayedQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.reconnectMinDelay = reconnectMinDelay;
	csOpts.reconnectMaxDelay = reconnectMaxDelay;
	wtfSetupConnection(&csOpts);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request first item with a real time/tick by tick qos */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	/* Close channel from provider. */
	wtfCloseChannel(WTF_TC_PROVIDER);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	/* Wait for channel to come back. */
	wtfAcceptWithTime(reconnectMinDelay * 2);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStream = pLoginRequest->rdmMsgBase.streamId;

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	directoryStreamId = pDirectoryRequest->rdmMsgBase.streamId;

	/* Provider sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = directoryStreamId;
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	// Change the service info to send a delayed qos.
	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;
	wtfSetService1Info(&service);
	service.info.qosList = &delayedQos;
	service.info.qosCount = 1;


	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);	
	
	/* Consumer receives item status then directory refresh. */	
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}


// This callback is for the Msg callback for the following test
RsslReactorCallbackRet closeMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent)
{
	RsslCloseMsg			closeMsg;
	RsslReactorSubmitMsgOptions opts;
	if (sendClose)
	{
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = pEvent->pRsslMsg->msgBase.streamId;
		closeMsg.msgBase.domainType = pEvent->pRsslMsg->msgBase.domainType;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);
	}
	return RSSL_RC_CRET_SUCCESS;
}

void watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseInCallback()
{
	/* Test message fanout in response to disconnection. */

	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg			requestMsg, * pRequestMsg;
	RsslStatusMsg* pStatusMsg;
	RsslRDMLoginRequest* pLoginRequest;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginRefresh* pLoginRefresh;
	RsslRDMDirectoryRequest	directoryRequest;
	RsslRDMDirectoryRequest* pDirectoryRequest;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMDirectoryRefresh* pDirectoryRefresh;
	RsslRDMDirectoryUpdate* pDirectoryUpdate;
	RsslRDMService* serviceList;
	WtfSetupConnectionOpts	csOpts;
	const RsslInt32 		reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
	WtfChannelEvent* pChannelEvent;
	RsslRDMService			service;
	RsslUInt32 providerLoginStream;
	RsslUInt32 directoryStreamId;
	RsslQos			delayedQos;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_DELAYED;
	delayedQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.reconnectMinDelay = reconnectMinDelay;
	csOpts.reconnectMaxDelay = reconnectMaxDelay;
	csOpts.consumerDefaultCallbackFunction = closeMsgCallback;
	wtfSetupConnection(&csOpts);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request first item with a real time/tick by tick qos */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	/* Close channel from provider. */
	wtfCloseChannel(WTF_TC_PROVIDER);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	/* Wait for channel to come back. */
	wtfAcceptWithTime(reconnectMinDelay * 2);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStream = pLoginRequest->rdmMsgBase.streamId;

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	directoryStreamId = pDirectoryRequest->rdmMsgBase.streamId;

	/* Provider sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = directoryStreamId;
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	// Change the service info to send a delayed qos.
	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;
	wtfSetService1Info(&service);
	service.info.qosList = &delayedQos;
	service.info.qosCount = 1;


	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives item status then directory refresh. */
	sendClose = true;
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request item again with a delayed/conflated qos */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
	requestMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TIME_CONFLATED);

	// Make sure nothing else gets sent
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

// This callback is for the Msg callback for the following test
RsslReactorCallbackRet closeAndRerequestMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent)
{
	RsslCloseMsg			closeMsg;
	RsslRequestMsg			requestMsg;
	RsslReactorSubmitMsgOptions opts;
	if (sendClose)
	{
		printf("Sending item close\n");
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = pEvent->pRsslMsg->msgBase.streamId;
		closeMsg.msgBase.domainType = pEvent->pRsslMsg->msgBase.domainType;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		/* Request item again with a delayed/conflated qos */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 3;
		requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
		requestMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);
	}
	return RSSL_RC_CRET_SUCCESS;
}

void watchlistRecoveryTest_OneItem_DisconnectChangeQoS_CloseAndRerequestInCallback()
{
	/* Test message fanout in response to disconnection. */

	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg			requestMsg, * pRequestMsg;
	RsslStatusMsg* pStatusMsg;
	RsslRDMLoginRequest* pLoginRequest;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginRefresh* pLoginRefresh;
	RsslRDMDirectoryRequest	directoryRequest;
	RsslRDMDirectoryRequest* pDirectoryRequest;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMDirectoryRefresh* pDirectoryRefresh;
	RsslRDMDirectoryUpdate* pDirectoryUpdate;
	RsslRDMService* serviceList;
	WtfSetupConnectionOpts	csOpts;
	const RsslInt32 		reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
	WtfChannelEvent* pChannelEvent;
	RsslRDMService			service;
	RsslUInt32 providerLoginStream;
	RsslUInt32 directoryStreamId;
	RsslQos			delayedQos;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_DELAYED;
	delayedQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.reconnectMinDelay = reconnectMinDelay;
	csOpts.reconnectMaxDelay = reconnectMaxDelay;
	csOpts.consumerDefaultCallbackFunction = closeAndRerequestMsgCallback;
	wtfSetupConnection(&csOpts);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request first item with a real time/tick by tick qos */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	/* Close channel from provider. */
	wtfCloseChannel(WTF_TC_PROVIDER);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	/* Wait for channel to come back. */
	wtfAcceptWithTime(reconnectMinDelay * 2);

	/* Provider receives channel-up & ready events. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer receives channel-up & ready events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStream = pLoginRequest->rdmMsgBase.streamId;

	/* Provider sends login response. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = providerLoginStream;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	directoryStreamId = pDirectoryRequest->rdmMsgBase.streamId;

	/* Provider sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = directoryStreamId;
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	// Change the service info to send a delayed qos.
	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;
	wtfSetService1Info(&service);
	service.info.qosList = &delayedQos;
	service.info.qosCount = 1;


	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives item status then directory refresh. */
	sendClose = true;
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TIME_CONFLATED);

	// Make sure nothing else gets sent
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}


// This callback is for the Msg callback for the following test
RsslReactorCallbackRet serviceDeletedDirectoryCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pEvent)
{
	RsslRequestMsg			requestMsg;
	RsslCloseMsg			closeMsg;
	RsslReactorSubmitMsgOptions opts;
	if (sendClose)
	{
		printf("Sending item close\n");
		// Close stream Id 3.
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 3;
		closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		/* Request item again with a delayed/conflated qos */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 3;
		requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
		requestMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	}
	return RSSL_RC_CRET_SUCCESS;
}
void watchlistRecoveryTest_OneItem_ServiceDeleted_ItemCloseAndRerequestInDirectoryCallback()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslCloseMsg* pCloseMsg;
	RsslStatusMsg* pStatusMsg;
	RsslInt32		providerItemStream;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryUpdate directoryUpdate;
	RsslRDMDirectoryRefresh* pDirectoryRefresh;
	RsslRDMDirectoryUpdate* pDirectoryUpdate;
	RsslRDMService service1;
	RsslRDMService* serviceList;
	RsslRefreshMsg* pRefreshMsg, refreshMsg;
	WtfSetupConnectionOpts sOpts;

	RsslQos			delayedQos;

	rsslClearQos(&delayedQos);
	delayedQos.timeliness = RSSL_QOS_TIME_DELAYED;
	delayedQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;


	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.consumerDirectoryCallbackFunction = serviceDeletedDirectoryCallback;
	wtfSetupConnection(&sOpts);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider deletes service. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.action = RSSL_MPEA_DELETE_ENTRY;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	
	sendClose = true;

	/* Watchlist receives directory update, which will close and reopen the item */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Provider receives CloseMsg. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == 4);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	
	sendClose = false;

	/* Provider restores service. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	wtfSetService1Info(&service1);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;
	service1.info.qosCount = 1;
	service1.info.qosList = &delayedQos;
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory update status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);


	wtfFinishTest();
}
