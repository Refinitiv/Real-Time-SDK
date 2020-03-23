/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

void watchlistSymbolListTest_BigList(RsslConnectionTypes connectionType);
void watchlistSymbolListTest_TwoSymbols_FlagsFromMsgBuffer(RsslConnectionTypes connectionType);
void watchlistSymbolListTest_TwoSymbols_NonStreaming(RsslConnectionTypes connectionType);
void watchlistSymbolListTest_DataStreamMsgKey(RsslConnectionTypes connectionType);

class WatchlistSymbolListTest : public ::testing::TestWithParam<RsslConnectionTypes> {
public:

	static void SetUpTestCase()
	{
		wtfInit(NULL);
	}

	virtual void SetUp()
	{
		wtfBindServer(GetParam());
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}

	virtual void TearDown()
	{
		wtfCloseServer();
	}
};

TEST_P(WatchlistSymbolListTest, TwoSymbols_FlagsFromMsgBuffer)
{
	watchlistSymbolListTest_TwoSymbols_FlagsFromMsgBuffer(GetParam());
}

TEST_P(WatchlistSymbolListTest, TwoSymbols_NonStreaming)
{
	watchlistSymbolListTest_TwoSymbols_NonStreaming(GetParam());
}

TEST_P(WatchlistSymbolListTest, BigList)
{
	watchlistSymbolListTest_BigList(GetParam());
}

TEST_P(WatchlistSymbolListTest, DataStreamMsgKey)
{
	watchlistSymbolListTest_DataStreamMsgKey(GetParam());
}

INSTANTIATE_TEST_CASE_P(
	TestingWatchlistSymbolListTests,
	WatchlistSymbolListTest,
	::testing::Values(
		RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET
	));

void watchlistSymbolListTest_TwoSymbols_FlagsFromMsgBuffer(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslInt32		providerSymbolListStream, providerItem1Stream, providerItem2Stream;
	RsslInt32		consumerItem1Stream, consumerItem2Stream;

	WtfSymbolAction symbolList[2];
	RsslBuffer		listName = { 17, const_cast<char*>("SOONG-TYPE DROIDS") };
	/* B4 is from Nemesis, he doesn't count. */
	RsslBuffer		itemNames[2] = { { 4, const_cast<char*>("Data") }, { 4, const_cast<char*>("Lore") } }; 

	char			slDataBodyBuf[256];
	RsslBuffer		slDataBody = { 256, slDataBodyBuf };
	RsslUInt32		slDataBodyLen = 256;

	RsslInt		view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;

	RsslInt		providerView1List[] = {2, 6, 7};
	RsslUInt32		providerView1Count = 3;

	char			viewBodyBuf[256];
	RsslBuffer		viewDataBody = { 256, viewBodyBuf };
	RsslUInt32		viewDataBodyLen = 256;

	/* Consumer requests a symbol list that contains two items and specifies it wants data streams.  
	 * These items should therefore be automatically requested by the watchlist. In this test,
	 * closing the symbol list does NOT automatically close the items. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Request symbol list. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = listName;

	slDataBody.length = slDataBodyLen;
	wtfConsumerEncodeSymbolListRequestBehaviors(&slDataBody, RDM_SYMBOL_LIST_DATA_STREAMS);
	requestMsg.msgBase.encDataBody = slDataBody;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &listName));
	providerSymbolListStream = pRequestMsg->msgBase.streamId;


	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerSymbolListStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	symbolList[0].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[0].itemName = itemNames[0];
	symbolList[1].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[1].itemName = itemNames[1];
	slDataBody.length = slDataBodyLen;
	wtfProviderEncodeSymbolListDataBody(&slDataBody, symbolList, 2);
	refreshMsg.msgBase.encDataBody = slDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	wtfConsumerDecodeSymbolListDataBody(&pRefreshMsg->msgBase.encDataBody, symbolList, 2);

	/* Provider receives requests for items(should come in order of list). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[0]));
	providerItem1Stream = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[1]));
	providerItem2Stream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(providerItem1Stream != providerItem2Stream);

	/* Provider sends refreshes. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItem1Stream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME;
	refreshMsg.msgBase.msgKey.name = itemNames[0];
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	refreshMsg.msgBase.msgKey.name = itemNames[1];
	refreshMsg.msgBase.streamId = providerItem2Stream;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refreshes. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId < 0);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.msgKey.name, &itemNames[0]));
	consumerItem1Stream = pRefreshMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId < 0);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.msgKey.name, &itemNames[1]));
	consumerItem2Stream = pRefreshMsg->msgBase.streamId;
	ASSERT_TRUE(consumerItem2Stream != consumerItem1Stream);

	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer adds a view to first request. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerItem1Stream;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.msgBase.msgKey.name = itemNames[0];
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view1List, 0, view1Count);
	requestMsg.msgBase.encDataBody = viewDataBody;

	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request with view. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItem1Stream);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);

	/* Consumer closes symbol list. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close of symbol list stream. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerSymbolListStream);

	/* Providers receives nothing else. */
	ASSERT_TRUE(!(pEvent = wtfGetEvent()));

	/* Consumer closes symbol item 1. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerItem1Stream;
	closeMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Consumer closes symbol item 2. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = consumerItem2Stream;
	closeMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close of items. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItem1Stream);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItem2Stream);

	wtfFinishTest();
}

void watchlistSymbolListTest_TwoSymbols_NonStreaming(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslInt32		providerSymbolListStream, providerItem1Stream, providerItem2Stream;
	RsslInt32		consumerItem1Stream, consumerItem2Stream;

	WtfSymbolAction symbolList[2];
	RsslBuffer		listName = { 17, const_cast<char*>("SOONG-TYPE DROIDS") };
	/* B4 is from Nemesis, he doesn't count. */
	RsslBuffer		itemNames[2] = { { 4, const_cast<char*>("Data") }, { 4, const_cast<char*>("Lore") } }; 

	char			slDataBodyBuf[256];
	RsslBuffer		slDataBody = { 256, slDataBodyBuf };
	RsslUInt32		slDataBodyLen = 256;

	/* Consumer requests a symbol list that contains two items and specifies it wants data streams.  
	 * These items should therefore be automatically requested by the watchlist. In this test,
	 * closing the symbol list does NOT automatically close the items. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Request symbol list. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = listName;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	slDataBody.length = slDataBodyLen;
	wtfConsumerEncodeSymbolListRequestBehaviors(&slDataBody, RDM_SYMBOL_LIST_DATA_SNAPSHOTS);
	requestMsg.msgBase.encDataBody = slDataBody;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &listName));
	providerSymbolListStream = pRequestMsg->msgBase.streamId;


	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerSymbolListStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	symbolList[0].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[0].itemName = itemNames[0];
	symbolList[1].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[1].itemName = itemNames[1];
	slDataBody.length = slDataBodyLen;
	wtfProviderEncodeSymbolListDataBody(&slDataBody, symbolList, 2);
	refreshMsg.msgBase.encDataBody = slDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	wtfConsumerDecodeSymbolListDataBody(&pRefreshMsg->msgBase.encDataBody, symbolList, 2);

	/* Provider receives requests for items(should come in order of list). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[0]));
	providerItem1Stream = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[1]));
	providerItem2Stream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(providerItem1Stream != providerItem2Stream);

	/* Provider sends refreshes. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItem1Stream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME;
	refreshMsg.msgBase.msgKey.name = itemNames[0];
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	/* Send response as Open state, to make sure the consumer converts properly. */
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	refreshMsg.msgBase.msgKey.name = itemNames[1];
	refreshMsg.msgBase.streamId = providerItem2Stream;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refreshes. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId < 0);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.msgKey.name, &itemNames[0]));
	consumerItem1Stream = pRefreshMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId < 0);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.msgKey.name, &itemNames[1]));
	consumerItem2Stream = pRefreshMsg->msgBase.streamId;
	ASSERT_TRUE(consumerItem2Stream != consumerItem1Stream);

	/* Since refresh was sent as open, provider should get closes. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItem1Stream);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItem2Stream);

	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer closes symbol list. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close of symbol list stream. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerSymbolListStream);

	/* Providers receives nothing else. */
	ASSERT_TRUE(!(pEvent = wtfGetEvent()));

	wtfFinishTest();
}

void watchlistSymbolListTest_BigList(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerSymbolListStream;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService			service1;

	WtfSymbolAction 	symbolList[1000];
	RsslBuffer			symbolListName = { 7, const_cast<char*>("NUMB3RS") };
	char				itemNames[1000][5];
	const RsslUInt32	symbolCount = 1000;

	char			dataBodyBuf[6000];
	RsslBuffer		dataBody = { 6000, dataBodyBuf };
	RsslUInt32		dataBodyLen = 6000;

	RsslUInt32 i;
	RsslUInt32		providerRequests, consumerRefreshes;

	/* Fill symbol list with names. */
	for (i = 0; i < symbolCount; ++i)
	{
		symbolList[i].itemName.data = itemNames[i];
		symbolList[i].itemName.length = snprintf(symbolList[i].itemName.data, 5, "%d", i);
		symbolList[i].action = RSSL_MPEA_ADD_ENTRY;
	}


	/* Test a large symbol list.  Modeled after rssl3304. */
	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Set the open window so we can better control this loop. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.flags = RDM_SVCF_HAS_LOAD; 
	service1.load.flags = RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service1.load.openWindow = 32;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	/* Request symbol list. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = symbolListName;

	dataBody.length = dataBodyLen;
	wtfConsumerEncodeSymbolListRequestBehaviors(&dataBody, RDM_SYMBOL_LIST_DATA_STREAMS);
	requestMsg.msgBase.encDataBody = dataBody;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &symbolListName));
	providerSymbolListStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerSymbolListStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	dataBody.length = dataBodyLen;
	wtfProviderEncodeSymbolListDataBody(&dataBody, symbolList, symbolCount);
	refreshMsg.msgBase.encDataBody = dataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	wtfConsumerDecodeSymbolListDataBody(&pRefreshMsg->msgBase.encDataBody, symbolList, symbolCount);

	/* Exchange requests/refreshes. */
	providerRequests = 0;
	consumerRefreshes = 0;
	while(consumerRefreshes < symbolCount)
	{
		wtfDispatch(WTF_TC_PROVIDER, 100);

		/* Make sure we get at least one. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		
		do
		{
			RsslInt32 providerItemStream;

			/* Provider receives request. */
			ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
			ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
			ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
			ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
			ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
			ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
			ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
			ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
			ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &symbolList[providerRequests].itemName));
			providerItemStream = pRequestMsg->msgBase.streamId;

			/* Provider sends refresh. */
			rsslClearRefreshMsg(&refreshMsg);
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
				| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
			refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
			refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
			refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME;
			refreshMsg.msgBase.msgKey.serviceId = service1Id;
			refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
			refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
			refreshMsg.state.streamState = RSSL_STREAM_OPEN;
			refreshMsg.state.dataState = RSSL_DATA_OK;

			refreshMsg.msgBase.msgKey.name = symbolList[providerRequests].itemName;
			refreshMsg.msgBase.streamId = providerItemStream;

			rsslClearReactorSubmitMsgOptions(&opts);
			opts.pRsslMsg = (RsslMsg*)&refreshMsg;
			wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

			++providerRequests;
		} while (pEvent = wtfGetEvent());

		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(!wtfGetEvent());

		wtfDispatch(WTF_TC_CONSUMER, 100);


		/* Make sure we get at least one. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		do 
		{
			/* Consumer receives refresh. */
			ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
			ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pRefreshMsg->msgBase.streamId < 0);
			ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
			ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
			ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
			ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.msgKey.name, &symbolList[consumerRefreshes].itemName));
			++consumerRefreshes;
		} while((pEvent = wtfGetEvent()));

		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(!wtfGetEvent());
	}

	ASSERT_TRUE(providerRequests == symbolCount);
	ASSERT_TRUE(consumerRefreshes == symbolCount);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistSymbolListTest_DataStreamMsgKey(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslStatusMsg	statusMsg, *pStatusMsg;
	RsslInt32		providerSymbolListStream, providerItem1Stream, providerItem2Stream;
	RsslInt32		consumerItem1Stream, consumerItem2Stream;

	WtfSymbolAction symbolList[2];
	RsslBuffer		listName = { 17, const_cast<char*>("SOONG-TYPE DROIDS") };
	/* B4 is from Nemesis, he doesn't count. */
	RsslBuffer		itemNames[2] = { { 4, const_cast<char*>("Data") }, { 4, const_cast<char*>("Lore") } }; 

	char			slDataBodyBuf[256];
	RsslBuffer		slDataBody = { 256, slDataBodyBuf };
	RsslUInt32		slDataBodyLen = 256;

	/* Tests that the watchlist will provide the MsgKey in the event that the provider
	 * does not send it. Since the provider is not actually opening the stream, it may not
	 * realize that it needs to send the key.  In this case, the watchlist should provide it. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Request symbol list. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = listName;

	slDataBody.length = slDataBodyLen;
	wtfConsumerEncodeSymbolListRequestBehaviors(&slDataBody, RDM_SYMBOL_LIST_DATA_STREAMS);
	requestMsg.msgBase.encDataBody = slDataBody;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &listName));
	providerSymbolListStream = pRequestMsg->msgBase.streamId;


	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerSymbolListStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	symbolList[0].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[0].itemName = itemNames[0];
	symbolList[1].action = RSSL_MPEA_ADD_ENTRY;
	symbolList[1].itemName = itemNames[1];
	slDataBody.length = slDataBodyLen;
	wtfProviderEncodeSymbolListDataBody(&slDataBody, symbolList, 2);
	refreshMsg.msgBase.encDataBody = slDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_SYMBOL_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	wtfConsumerDecodeSymbolListDataBody(&pRefreshMsg->msgBase.encDataBody, symbolList, 2);

	/* Provider receives requests for items(should come in order of list). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[0]));
	providerItem1Stream = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemNames[1]));
	providerItem2Stream = pRequestMsg->msgBase.streamId;
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(providerItem1Stream != providerItem2Stream);

	/* Provider sends status messages for each. */

	/* Send Open/Suspect on the first item. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItem1Stream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Send a private stream redirect on the second item. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItem2Stream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_PRIVATE_STREAM;
	statusMsg.state.streamState = RSSL_STREAM_REDIRECTED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives Open/Suspect Status, with key. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	consumerItem1Stream = pStatusMsg->msgBase.streamId;
	ASSERT_TRUE(consumerItem1Stream < 0);

	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY);
	ASSERT_TRUE(pStatusMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->msgBase.msgKey.name, &itemNames[0]));

	/* Consumer receives private stream redirect, with key. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_PRIVATE_STREAM);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_REDIRECTED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	consumerItem2Stream = pStatusMsg->msgBase.streamId;
	ASSERT_TRUE(consumerItem2Stream < 0);

	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY);
	ASSERT_TRUE(pStatusMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pStatusMsg->msgBase.msgKey.name, &itemNames[1]));

	ASSERT_TRUE(consumerItem1Stream != consumerItem2Stream);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends another Open/Suspect on first stream. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = providerItem1Stream;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&statusMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives, without key. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerItem1Stream);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(!(pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY));

	wtfFinishTest();
}
