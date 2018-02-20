/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

void watchlistAggregationTest_TwoItems(); 
void watchlistAggregationTest_TwoItems_CloseBothInCallback();
#ifdef COMPILE_64BITS
void watchlistAggregationTest_TwoItems_CloseBothInCallback_multicast();
#endif
void watchlistAggregationTest_TwoItems_CloseFirstInCallback();
void watchlistAggregationTest_TwoItems_CloseSecondInCallback();
void watchlistAggregationTest_TwoItems_PostWithAck(); 
void watchlistAggregationTest_TwoItems_PostWithAck_Timeout(); 
void watchlistAggregationTest_PostWithAck_OffStream(); 
void watchlistAggregationTest_PostWithAck_OffStream_Timeout(); 
void watchlistAggregationTest_TwoItems_UnackedPosts();
void watchlistAggregationTest_TwoItems_GenericMessage(); 
void watchlistAggregationTest_ThreeItems_SvcNameAndId(); 
void watchlistAggregationTest_SnapshotBeforeStreaming(); 
void watchlistAggregationTest_SnapshotBeforeStreaming_Multipart(); 
void watchlistAggregationTest_SnapshotRefreshBeforeStreaming(); 
void watchlistReissueTest_PriorityChangeAndNewRefresh();
void watchlistAggregationTest_TwoItems_Pause(); 
void watchlistAggregationTest_LoginReissue();
void watchlistAggregationTest_TwoItems_LoginPause(); 
void watchlistAggregationTest_TwoItems_FieldViewFromMsgBuffer(); 
void watchlistAggregationTest_TwoItems_ElementViewFromMsgBuffer(); 
void watchlistAggregationTest_EmptyView(); 
void watchlistAggregationTest_TwoItems_SnapshotView(); 
void watchlistAggregationTest_SnapshotBeforeStreaming_View(); 
void watchlistAggregationTest_TwoItems_ViewOnOff();
void watchlistAggregationTest_TwoItems_ViewMixture(); 
void watchlistAggregationTest_TwoItemsInMsgBuffer_ViewMixture(); 
void watchlistAggregationTest_ThreeItemsInMsgBuffer_Batch(); 
void watchlistAggregationTest_ThreeItemsInMsgBuffer_BatchWithView(); 
void watchlistAggregationTest_ThreeItems_OnePrivate(); 

#ifdef COMPILE_64BITS
void watchlistAggregationTest_Multicast_OneItem_Reorder(RsslBool gapRecovery);
void watchlistAggregationTest_Multicast_BroadcastUnicastPermutations(RsslBool gapRecovery);
void watchlistAggregationTest_Multicast_OneItem_Reorder_MultipartSnapshot();
void watchlistAggregationTest_Multicast_OneItem_PrivateStreamOrder();
void watchlistAggregationTest_Multicast_TwoStreams_Reissue();
#endif

class WatchlistAggregationTest : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		wtfInit(NULL);
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}
};


TEST_F(WatchlistAggregationTest, EmptyView)
{
	watchlistAggregationTest_EmptyView();
}
TEST_F(WatchlistAggregationTest, TwoItems)
{
	watchlistAggregationTest_TwoItems();
}

TEST_F(WatchlistAggregationTest, TwoItems_CloseBothInCallback)
{
	watchlistAggregationTest_TwoItems_CloseBothInCallback();
}

TEST_F(WatchlistAggregationTest, TwoItems_CloseFirstInCallback)
{
	watchlistAggregationTest_TwoItems_CloseFirstInCallback();
}

TEST_F(WatchlistAggregationTest, TwoItems_CloseSecondInCallback)
{
	watchlistAggregationTest_TwoItems_CloseSecondInCallback();
}

TEST_F(WatchlistAggregationTest, TwoItems_PostWithAck)
{
	watchlistAggregationTest_TwoItems_PostWithAck();
}

TEST_F(WatchlistAggregationTest, TwoItems_PostWithAck_Timeout)
{
	watchlistAggregationTest_TwoItems_PostWithAck_Timeout();
}

TEST_F(WatchlistAggregationTest, PostWithAck_OffStream)
{
	watchlistAggregationTest_PostWithAck_OffStream();
}

TEST_F(WatchlistAggregationTest, PostWithAck_OffStream_Timeout)
{
	watchlistAggregationTest_PostWithAck_OffStream_Timeout();
}

TEST_F(WatchlistAggregationTest, TwoItems_UnackedPosts)
{
	watchlistAggregationTest_TwoItems_UnackedPosts();
}

TEST_F(WatchlistAggregationTest, TwoItems_GenericMessage)
{
	watchlistAggregationTest_TwoItems_GenericMessage();
}

TEST_F(WatchlistAggregationTest, ThreeItems_SvcNameAndId)
{
	watchlistAggregationTest_ThreeItems_SvcNameAndId();
}

TEST_F(WatchlistAggregationTest, SnapshotBeforeStreaming)
{
	watchlistAggregationTest_SnapshotBeforeStreaming();
}

TEST_F(WatchlistAggregationTest, SnapshotBeforeStreaming_Multipart)
{
	watchlistAggregationTest_SnapshotBeforeStreaming_Multipart();
}

TEST_F(WatchlistAggregationTest, SnapshotRefreshBeforeStreaming)
{
	watchlistAggregationTest_SnapshotRefreshBeforeStreaming();
}

TEST_F(WatchlistAggregationTest, PriorityChangeAndNewRefresh)
{
	watchlistReissueTest_PriorityChangeAndNewRefresh();
}

TEST_F(WatchlistAggregationTest, LoginReissue)
{
	watchlistAggregationTest_LoginReissue();
}

TEST_F(WatchlistAggregationTest, TwoItems_Pause)
{
	watchlistAggregationTest_TwoItems_Pause();
}

TEST_F(WatchlistAggregationTest, TwoItems_LoginPause)
{
	watchlistAggregationTest_TwoItems_LoginPause();
}

TEST_F(WatchlistAggregationTest, TwoItems_ViewOnOff)
{
	watchlistAggregationTest_TwoItems_ViewOnOff();
}

TEST_F(WatchlistAggregationTest, TwoItems_FieldViewFromMsgBuffer)
{
	watchlistAggregationTest_TwoItems_FieldViewFromMsgBuffer();
}

TEST_F(WatchlistAggregationTest, TwoItems_ElementViewFromMsgBuffer)
{
	watchlistAggregationTest_TwoItems_ElementViewFromMsgBuffer();
}

TEST_F(WatchlistAggregationTest, TwoItems_SnapshotView)
{
	watchlistAggregationTest_TwoItems_SnapshotView();
}

TEST_F(WatchlistAggregationTest, SnapshotBeforeStreaming_View)
{
	watchlistAggregationTest_SnapshotBeforeStreaming_View();
}

TEST_F(WatchlistAggregationTest, TwoItems_ViewMixture)
{
	watchlistAggregationTest_TwoItems_ViewMixture();
}

TEST_F(WatchlistAggregationTest, TwoItemsInMsgBuffer_ViewMixture)
{
	watchlistAggregationTest_TwoItemsInMsgBuffer_ViewMixture();
}

TEST_F(WatchlistAggregationTest, ThreeItemsInMsgBuffer_Batch)
{
	watchlistAggregationTest_ThreeItemsInMsgBuffer_Batch();
}

TEST_F(WatchlistAggregationTest, ThreeItemsInMsgBuffer_BatchWithView)
{
	watchlistAggregationTest_ThreeItemsInMsgBuffer_BatchWithView();
}

TEST_F(WatchlistAggregationTest, ThreeItems_OnePrivate)
{
	watchlistAggregationTest_ThreeItems_OnePrivate();
}

#ifdef COMPILE_64BITS
class WatchlistAggregationTest_Multicast : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		WtfInitOpts initOpts;
		wtfClearInitOpts(&initOpts);
		initOpts.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
		wtfInit(&initOpts);
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}
};

TEST_F(WatchlistAggregationTest_Multicast, EmptyView)
{
	watchlistAggregationTest_EmptyView();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems)
{
	watchlistAggregationTest_TwoItems();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_CloseBothInCallback)
{
	watchlistAggregationTest_TwoItems_CloseBothInCallback();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_CloseFirstInCallback)
{
	watchlistAggregationTest_TwoItems_CloseFirstInCallback();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_CloseSecondInCallback)
{
	watchlistAggregationTest_TwoItems_CloseSecondInCallback();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_PostWithAck)
{
	watchlistAggregationTest_TwoItems_PostWithAck();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_PostWithAck_Timeout)
{
	watchlistAggregationTest_TwoItems_PostWithAck_Timeout();
}

TEST_F(WatchlistAggregationTest_Multicast, PostWithAck_OffStream)
{
	watchlistAggregationTest_PostWithAck_OffStream();
}

TEST_F(WatchlistAggregationTest_Multicast, PostWithAck_OffStream_Timeout)
{
	watchlistAggregationTest_PostWithAck_OffStream_Timeout();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_UnackedPosts)
{
	watchlistAggregationTest_TwoItems_UnackedPosts();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_GenericMessage)
{
	watchlistAggregationTest_TwoItems_GenericMessage();
}

TEST_F(WatchlistAggregationTest_Multicast, ThreeItems_SvcNameAndId)
{
	watchlistAggregationTest_ThreeItems_SvcNameAndId();
}

TEST_F(WatchlistAggregationTest_Multicast, SnapshotBeforeStreaming)
{
	watchlistAggregationTest_SnapshotBeforeStreaming();
}

TEST_F(WatchlistAggregationTest_Multicast, SnapshotBeforeStreaming_Multipart)
{
	watchlistAggregationTest_SnapshotBeforeStreaming_Multipart();
}

TEST_F(WatchlistAggregationTest_Multicast, SnapshotRefreshBeforeStreaming)
{
	watchlistAggregationTest_SnapshotRefreshBeforeStreaming();
}

TEST_F(WatchlistAggregationTest_Multicast, PriorityChangeAndNewRefresh)
{
	watchlistReissueTest_PriorityChangeAndNewRefresh();
}

TEST_F(WatchlistAggregationTest_Multicast, LoginReissue)
{
	watchlistAggregationTest_LoginReissue();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_ViewOnOff)
{
	watchlistAggregationTest_TwoItems_ViewOnOff();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_FieldViewFromMsgBuffer)
{
	watchlistAggregationTest_TwoItems_FieldViewFromMsgBuffer();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_ElementViewFromMsgBuffer)
{
	watchlistAggregationTest_TwoItems_ElementViewFromMsgBuffer();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_SnapshotView)
{
	watchlistAggregationTest_TwoItems_SnapshotView();
}

TEST_F(WatchlistAggregationTest_Multicast, SnapshotBeforeStreaming_View)
{
	watchlistAggregationTest_SnapshotBeforeStreaming_View();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItems_ViewMixtures)
{
	watchlistAggregationTest_TwoItems_ViewMixture();
}

TEST_F(WatchlistAggregationTest_Multicast, TwoItemsInMsgBuffer_ViewMixture)
{
	watchlistAggregationTest_TwoItemsInMsgBuffer_ViewMixture();
}

TEST_F(WatchlistAggregationTest_Multicast, ThreeItemsInMsgBuffer_Batch)
{
	watchlistAggregationTest_ThreeItemsInMsgBuffer_Batch();
}

TEST_F(WatchlistAggregationTest_Multicast, ThreeItemsInMsgBuffer_BatchWithView)
{
	watchlistAggregationTest_ThreeItemsInMsgBuffer_BatchWithView();
}

TEST_F(WatchlistAggregationTest_Multicast, ThreeItems_OnePrivate)
{
	watchlistAggregationTest_ThreeItems_OnePrivate();
}


class WatchlistAggregationTest_Multicast_RawServer : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		WtfInitOpts initOpts;
		wtfClearInitOpts(&initOpts);
		initOpts.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
		initOpts.useRawProvider = RSSL_TRUE;
		wtfInit(&initOpts);
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}
};

TEST_F(WatchlistAggregationTest_Multicast_RawServer, TwoItems_CloseBothInCallback_multicast)
{
	watchlistAggregationTest_TwoItems_CloseBothInCallback_multicast();
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_OneItem_Reorder_GapRecovery)
{
	watchlistAggregationTest_Multicast_OneItem_Reorder(RSSL_TRUE); /* With gap recovery enabled. */
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_OneItem_Reorder)
{
	watchlistAggregationTest_Multicast_OneItem_Reorder(RSSL_FALSE); /* Without gap recovery enabled. */
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_BroadcastUnicastPermutations_GapRecovery)
{
	watchlistAggregationTest_Multicast_BroadcastUnicastPermutations(RSSL_TRUE); /* With gap recovery enabled. */
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_BroadcastUnicastPermutations)
{
	watchlistAggregationTest_Multicast_BroadcastUnicastPermutations(RSSL_FALSE); /* Without gap recovery enabled. */
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_OneItem_PrivateStreamOrder)
{
	watchlistAggregationTest_Multicast_OneItem_PrivateStreamOrder();
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_OneItem_Reorder_MultipartSnapshot)
{
	watchlistAggregationTest_Multicast_OneItem_Reorder_MultipartSnapshot();
}

TEST_F(WatchlistAggregationTest_Multicast_RawServer, Multicast_TwoStreams_Reissue)
{
	watchlistAggregationTest_Multicast_TwoStreams_Reissue();
}
#endif

void watchlistAggregationTest_TwoItems()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
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

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}


/* Callback function that closes streams 2 and 3 once a certain
 * number of messages have been received. */
static int _watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount = 0;
void _watchlistAggregationTest_TwoItems_Callback_CloseBoth()
{
	RsslReactorSubmitMsgOptions opts;
	RsslCloseMsg closeMsg;

	ASSERT_TRUE(_watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount > 0);
	--_watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount;
	if (_watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount == 0)
	{
		/* TODO Use ReactorSubmitMsg instead of wtfSubmitMsg */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 2;
		closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 3;
		closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		wtfSetConsumerMsgCallbackAction(NULL);
	}

}

void _watchlistAggregationTest_TwoItems_Callback_CloseFirst()
{
	RsslReactorSubmitMsgOptions opts;
	RsslCloseMsg closeMsg;

	/* Called when in msgCallback -- closes both streams */

	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfSetConsumerMsgCallbackAction(NULL);
}

void _watchlistAggregationTest_TwoItems_Callback_CloseSecond()
{
	RsslReactorSubmitMsgOptions opts;
	RsslCloseMsg closeMsg;

	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfSetConsumerMsgCallbackAction(NULL);
}

RsslInt32 _watchlistAggregationTest_TwoItems_OpenTwoItems()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;

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
	wtfGetEvent();

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	pEvent = wtfGetEvent();
	pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	pEvent = wtfGetEvent();
	pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent);

	return providerItemStream;

}

void watchlistAggregationTest_TwoItems_CloseBothInCallback()
{
	WtfEvent		*pEvent;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	providerItemStream = _watchlistAggregationTest_TwoItems_OpenTwoItems();

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	_watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount = 1;
	wtfSetConsumerMsgCallbackAction((void*)_watchlistAggregationTest_TwoItems_Callback_CloseBoth);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Consumer should not receive other refresh since we closed its stream */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}
#ifdef COMPILE_64BITS
void watchlistAggregationTest_TwoItems_CloseBothInCallback_multicast()
{
	WtfEvent		*pEvent;
	RsslInt32		providerItemStream;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslCloseMsg	*pCloseMsg;
	RsslReactorSubmitMsgOptions opts;
	WtfSubmitMsgOptionsEx optsEx;
	
	int i;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	for (i = 1; i <= 6; ++i)
	{
		providerItemStream = _watchlistAggregationTest_TwoItems_OpenTwoItems();

		/* Provider sends first refresh part. */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.msgBase.streamId = providerItemStream;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_CLEAR_CACHE 
			| RSSL_RFMF_HAS_QOS | RSSL_RFMF_HAS_PART_NUM;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.partNum = 0;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfClearSubmitMsgOptionsEx(&optsEx);
		optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

		/* Provider sends update */
		rsslClearUpdateMsg(&updateMsg);
		updateMsg.msgBase.streamId = 0;
		updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
		updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
		updateMsg.msgBase.msgKey.serviceId = service1Id;
		updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&updateMsg;
		optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

		/* Provider sends second refresh part (complete). */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.msgBase.streamId = providerItemStream;
		refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
			| RSSL_RFMF_HAS_QOS | RSSL_RFMF_HAS_PART_NUM;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.partNum = 1;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfClearSubmitMsgOptionsEx(&optsEx);
		optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

		_watchlistAggregationTest_TwoItems_Callback_CloseBoth_waitCount = i;
		wtfSetConsumerMsgCallbackAction((void*)_watchlistAggregationTest_TwoItems_Callback_CloseBoth);

		/* Consumer receives refresh. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

		if (i >= 2)
		{
			/* Consumer receives refresh on other stream */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
			ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
			ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
		}

		if (i >= 3)
		{
			/* Consumer receives second refresh part */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
			ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
			ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
		}

		if (i >= 4)
		{
			/* Consumer receives second refresh part on other stream*/
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
			ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
			ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
		}

		if (i >= 5)
		{
			/* Consumer receives update */
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);
		}

		if (i >= 6)
		{
			/* Consumer receives update on other stream*/
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 3);
		}


		/* Consumer receives no more messages since we closed its stream */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(!wtfGetEvent());

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);
	}

	wtfFinishTest();
}
#endif

void watchlistAggregationTest_TwoItems_CloseFirstInCallback()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	*pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	providerItemStream = _watchlistAggregationTest_TwoItems_OpenTwoItems();

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfSetConsumerMsgCallbackAction((void*)_watchlistAggregationTest_TwoItems_Callback_CloseFirst);

	/* Consumer receives refreshes. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_CloseSecondInCallback()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	*pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	providerItemStream = _watchlistAggregationTest_TwoItems_OpenTwoItems();

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfSetConsumerMsgCallbackAction((void*)_watchlistAggregationTest_TwoItems_Callback_CloseSecond);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Consumer should not receive other refresh since we closed its stream */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_PostWithAck()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslAckMsg		ackMsg, *pAckMsg;
	RsslReactorSubmitMsgOptions opts;
	RsslUInt32		ackId;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.postAckTimeout = 2000;
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
	opts.requestMsgOptions.pUserSpec = (void*)0x77777777;
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

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer sends incomplete post, with sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK | RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE));
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Provider sends acknowledgement. */
	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.streamId = providerItemStream;
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.flags = RSSL_AKMF_HAS_SEQ_NUM;
	ackMsg.ackId = 55;
	ackMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&ackMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags & RSSL_AKMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends complete post, with ack ID & sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK
		| RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Provider sends acknowledgement. */
	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.streamId = providerItemStream;
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.flags = RSSL_AKMF_HAS_SEQ_NUM;
	ackMsg.ackId = 55;
	ackMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&ackMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == RSSL_AKMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends complete post, without sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK;
	postMsg.postId = 55;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ackId = pPostMsg->postId;

	/* Provider sends acknowledgement. */
	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.streamId = providerItemStream;
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.ackId = ackId;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&ackMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends post, with ack ID & sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK
		| RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Provider sends acknowledgement. */
	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.streamId = providerItemStream;
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.flags = RSSL_AKMF_HAS_SEQ_NUM;
	ackMsg.ackId = 55;
	ackMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&ackMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == RSSL_AKMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer receives no timeout nack. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(!wtfGetEvent());

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_PostWithAck_Timeout()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslAckMsg		*pAckMsg;
	RsslReactorSubmitMsgOptions opts;
	RsslUInt32		ackId;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.postAckTimeout = 2000;
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
	opts.requestMsgOptions.pUserSpec = (void*)0x77777777;
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

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer sends incomplete post, with sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK | RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post (doesn't respond). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE));
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Consumer receives nack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == (RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT));
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pAckMsg->nakCode == RSSL_NAKC_NO_RESPONSE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends complete post, with ack ID & sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK
		| RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post(doesn't respond). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Consumer receives nack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == (RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT));
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pAckMsg->nakCode == RSSL_NAKC_NO_RESPONSE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends complete post, without sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK;
	postMsg.postId = 55;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post(doesn't respond). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ackId = pPostMsg->postId;

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == (RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT));
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->nakCode == RSSL_NAKC_NO_RESPONSE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	/* Consumer sends post, with ack ID & sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK
		| RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post(doesn't respond). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Consumer receives ack, only on the first stream. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == (RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT));
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pAckMsg->nakCode == RSSL_NAKC_NO_RESPONSE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

	ASSERT_TRUE(!wtfGetEvent());


	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}

void watchlistAggregationTest_PostWithAck_OffStream()
{
	WtfEvent		*pEvent;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslAckMsg		ackMsg, *pAckMsg;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.postAckTimeout = 2000;
	wtfSetupConnection(&sOpts);


	/* Consumer sends incomplete post, with sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 1;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK | RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE));
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Provider sends acknowledgement. */
	rsslClearAckMsg(&ackMsg);
	ackMsg.msgBase.streamId = wtfGetProviderLoginStream();
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.flags = RSSL_AKMF_HAS_SEQ_NUM;
	ackMsg.ackId = 55;
	ackMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&ackMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives ack. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == 1);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags & RSSL_AKMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName == NULL);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == WTF_DEFAULT_LOGIN_USER_SPEC_PTR);

	/* Consumer receives no timeout nack. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistAggregationTest_PostWithAck_OffStream_Timeout()
{
	WtfEvent		*pEvent;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslAckMsg		*pAckMsg;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.postAckTimeout = 2000;
	wtfSetupConnection(&sOpts);

	/* Consumer sends incomplete post, with sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK | RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post (doesn't respond). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE));
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Consumer receives nack. */
	wtfDispatch(WTF_TC_CONSUMER, 2500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pAckMsg = (RsslAckMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pAckMsg->msgBase.msgClass == RSSL_MC_ACK);
	ASSERT_TRUE(pAckMsg->msgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pAckMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pAckMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pAckMsg->flags == (RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_HAS_TEXT));
	ASSERT_TRUE(pAckMsg->ackId == 55);
	ASSERT_TRUE(pAckMsg->seqNum == 88);
	ASSERT_TRUE(pAckMsg->nakCode == RSSL_NAKC_NO_RESPONSE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName == NULL);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)WTF_DEFAULT_LOGIN_USER_SPEC_PTR);

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_UnackedPosts()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslStatusMsg	statusMsg, *pStatusMsg;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Tests that unacknowledged post messages are properly cleaned up
	 * on stream close. */

	/* Request item. */
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
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer sends incomplete post, with sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK | RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(!(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE));
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

	/* Consumer sends complete post, with ack ID & sequence number. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = 2;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_ACK
		| RSSL_PSMF_HAS_SEQ_NUM;
	postMsg.postId = 55;
	postMsg.seqNum = 88;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&postMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives post. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
	ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_POST_ID);
	ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM);
	ASSERT_TRUE(pPostMsg->postId == 55);
	ASSERT_TRUE(pPostMsg->seqNum == 88);

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


	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_GenericMessage()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslGenericMsg	genericMsg, *pGenericMsg;
	RsslBool		stream2GenericMsg, stream3GenericMsg;
	int				i;
	RsslRDMDirectoryRequest	directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMService			*serviceList;

	ASSERT_TRUE(wtfStartTest());

	/* Try sending a generic message in each direction. */

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

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refreshes. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 4);
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
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 4);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
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
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	serviceList = pDirectoryRefresh->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_ADD_ENTRY);

	/* Send a generic message on the first stream. */
	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.streamId = 2;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives generic message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
	ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE);
	ASSERT_TRUE(pGenericMsg->msgBase.streamId == providerItemStream);

	/* Provider sends generic message back. */
	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.streamId = providerItemStream;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives two generic messages(one on each request). */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	stream2GenericMsg = RSSL_FALSE;
	stream3GenericMsg = RSSL_FALSE;
	for(i = 0; i < 2; ++i)
	{
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
		ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE);

		if (pGenericMsg->msgBase.streamId == 2)
		{
			ASSERT_TRUE(!stream2GenericMsg);
			stream2GenericMsg = RSSL_TRUE;
		}
		else if (pGenericMsg->msgBase.streamId == 3)
		{
			ASSERT_TRUE(!stream3GenericMsg);
			stream3GenericMsg = RSSL_TRUE;
		}
		else
			ASSERT_TRUE(0);
	}

	ASSERT_TRUE(stream2GenericMsg && stream3GenericMsg);
	ASSERT_TRUE(!wtfGetEvent());

	/* Now try receiving a generic message on the login stream. */

	/* Provider sends generic message . */
	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.streamId = wtfGetProviderLoginStream();
	genericMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives generic message. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
	ASSERT_TRUE(pGenericMsg->msgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE);
	ASSERT_TRUE(pGenericMsg->msgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);

	/* Now try receiving a generic message on directory streams. */
	if (wtfGetConnectionType() != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		/* TODO This does not work over Multicast. */

		/* Provider sends generic message back. */
		rsslClearGenericMsg(&genericMsg);
		genericMsg.msgBase.streamId = wtfGetProviderDirectoryStream();
		genericMsg.msgBase.domainType = RSSL_DMT_SOURCE;
		genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&genericMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		/* Consumer receives generic message on both directory streams */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		stream2GenericMsg = RSSL_FALSE;
		stream3GenericMsg = RSSL_FALSE;
		for(i = 0; i < 2; ++i)
		{
			ASSERT_TRUE(pEvent = wtfGetEvent());
			ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pGenericMsg->msgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
			ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE);

			if (pGenericMsg->msgBase.streamId == 4)
			{
				ASSERT_TRUE(!stream2GenericMsg);
				stream2GenericMsg = RSSL_TRUE;
			}
			else if (pGenericMsg->msgBase.streamId == 5)
			{
				ASSERT_TRUE(!stream3GenericMsg);
				stream3GenericMsg = RSSL_TRUE;
			}
			else
				ASSERT_TRUE(0);
		}

		ASSERT_TRUE(stream2GenericMsg && stream3GenericMsg);
		ASSERT_TRUE(!wtfGetEvent());
	}

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
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

	wtfFinishTest();
}

void watchlistAggregationTest_ThreeItems_SvcNameAndId()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	requestMsg.msgBase.msgKey.serviceId = service1Id;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pEvent->rsslMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	pRequestMsg = &pEvent->rsslMsg.pRsslMsg->requestMsg;
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Request third item, using name. */
	requestMsg.msgBase.streamId = 4;
	requestMsg.msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 3);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close third item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 4;
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

	wtfFinishTest();
}

void watchlistAggregationTest_SnapshotBeforeStreaming()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerItemStream;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item as a snapshot. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item as streaming. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	
	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider receives streaming request. */
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

void watchlistAggregationTest_SnapshotBeforeStreaming_Multipart()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslInt32		providerItemStream;
	
	/* Similar to the watchlistAggregationTest_SnapshotRefreshBeforeStreaming,
	 * but tests cases regarding multipart refreshes, such as interleaving updates
	 * and unsolicited refreshes. */

	/* NOTE: For now, the behavior for unsolicited refreshes in these cases is that they
	 * should be treated like updates.  This may change in the future, especially with regard
	 * to multicast. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item as a snapshot. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item as streaming. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends UNSOLICTED refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer does NOT see this. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	
	/* Provider sends refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends UNSOLICTED refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer DOES see this refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED));
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);


	/* Provider sends final refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider receives streaming request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends UNSOLICITED refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer does NOT see this since first part hasn't arrived. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends UNSOLICTED refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer DOES see this refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED));
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);


	/* Provider sends final refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	wtfFinishTest();
}

void watchlistAggregationTest_SnapshotRefreshBeforeStreaming()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerItemStream;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Tests fulfilling a snapshot request, then streaming request.
	 * Modeled after rssl3302. */

	/* Request first item as a snapshot. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Request item as streaming. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_STREAMING;

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
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	wtfFinishTest();
}

void watchlistReissueTest_PriorityChangeAndNewRefresh()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;

	ASSERT_TRUE(wtfStartTest());

	/* Test priority change and reissue for new refresh. */

	wtfSetupConnection(NULL);

	/* Consumer requests item. */
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

	/* Consumer reissues item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_NO_REFRESH
		| RSSL_RQMF_HAS_PRIORITY;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.priorityClass = 5;
	requestMsg.priorityCount = 6;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 5);
	ASSERT_TRUE(pRequestMsg->priorityCount == 6);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Consumer reissues with new image. */
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
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Consumer closes item. */
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

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_Pause()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;

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

	/* Request second item as paused. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_PAUSE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority, not paused. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_PAUSE));

	/* Reissue first item as paused. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_NO_REFRESH 
		| RSSL_RQMF_PAUSE;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives pause request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PAUSE);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority, still paused. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PAUSE);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
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

	wtfFinishTest();
}

void watchlistAggregationTest_LoginReissue()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent			*pEvent;
	RsslRDMLoginRequest	loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh	loginRefresh, *pLoginRefresh;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Send relogin. */
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
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(!(pLoginRequest->flags & RDM_LG_RQF_PAUSE_ALL));
	ASSERT_TRUE(!(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH));

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends login refresh. */
	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.flags |= RDM_LG_RFF_SOLICITED;
	loginRefresh.userName = loginUserName;
	loginRefresh.rdmMsgBase.streamId = wtfGetProviderLoginStream();
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
	loginRefresh.supportOptimizedPauseResume = 1;
	loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
	loginRefresh.supportViewRequests = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRefresh = (RsslRDMLoginRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pLoginRefresh->rdmMsgBase.streamId == 1);
	ASSERT_TRUE(pLoginRefresh->flags & RDM_LG_RFF_SOLICITED);
	ASSERT_TRUE(!wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_LoginPause()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent			*pEvent;
	RsslRequestMsg		requestMsg, *pRequestMsg;
	RsslInt32			providerItemStream;
	RsslCloseMsg		closeMsg, *pCloseMsg;
	RsslRDMLoginRequest	loginRequest, *pLoginRequest;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.requestTimeout = 2000;
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

	/* Request second item as paused. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_PAUSE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority, not paused. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_PAUSE));

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refreshes. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	/* Send login reissue to pause. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_PAUSE_ALL | RDM_LG_RQF_NO_REFRESH;
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
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_PAUSE_ALL);
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH);

	/* Reissue first item as paused. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_NO_REFRESH 
		| RSSL_RQMF_PAUSE;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider should not receive anything (item is already considered paused). */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	/*** Wait for timeout to ensure that consumer receives no messages (UPAC-500). ***/
	wtfDispatch(WTF_TC_CONSUMER, 3000);
	ASSERT_TRUE(!wtfGetEvent());

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority, still paused. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PAUSE);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Send login reissue to resume. */
	wtfInitDefaultLoginRequest(&loginRequest);
	loginRequest.flags |= RDM_LG_RQF_NO_REFRESH;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives login request to resume. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.streamId == wtfGetProviderLoginStream());
	ASSERT_TRUE(!(pLoginRequest->flags & RDM_LG_RQF_PAUSE_ALL));
	ASSERT_TRUE(pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH);

	/*** Wait for timeout to ensure that consumer receives no messages (UPAC-500). ***/
	wtfDispatch(WTF_TC_CONSUMER, 3000);
	ASSERT_TRUE(!wtfGetEvent());

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
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

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_ViewOnOff()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt		view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;
	RsslInt		view1_1List[] = {3, 22};
	RsslUInt32		view1_1Count = 2;
	RsslInt		view2List[] = {6, 7};
	RsslUInt32		view2Count = 2;

	RsslInt		providerView1List[] = {2, 6, 7};
	RsslUInt32		providerView1Count = 3;

	RsslInt		providerView1_1List[] = {3, 22};
	RsslUInt32		providerView1_1Count = 2;

	char			viewBodyBuf[256];
	RsslBuffer		viewDataBody = { 256, viewBodyBuf };
	RsslUInt32		viewDataBodyLen = 256;


	/* Test how aggregation of items can remove/restore
	 * a view. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
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

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Request second item without view. */
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

	/* Provider receives request. View should be removed. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW));
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Provider sends refresh, satisfying view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh on both streams (unsolicited on one). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);

	/* Consumer requests snapshot with view. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 4;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view2List, 0, view2Count);
	requestMsg.msgBase.encDataBody = viewDataBody;

	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. View should still not be there.
	 * (priority doesn't change since the new request is a snapshot)
	 */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW));
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Provider sends refresh, satisfying view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refreshes on all streams (unsolicited on first two). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_SOLICITED));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 4);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives priority change and restored view. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH); 
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, view1List, view1Count, RDM_VIEW_TYPE_FIELD_ID_LIST);

	/* Provider sends refresh, satisfying view (as unsolicited, image was not requested). */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh (as unsolicited). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

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



	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_FieldViewFromMsgBuffer()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt			view1List[] = {2, 6, 7, 2};
	RsslUInt32		view1Count = 4;
	RsslInt			view1_1List[] = {3, 22};
	RsslUInt32		view1_1Count = 2;
	RsslInt			view2List[] = {6, 7};
	RsslUInt32		view2Count = 2;

	RsslInt		providerView1List[] = {2, 6, 7};
	RsslUInt32		providerView1Count = 3;

	RsslInt		providerView1_1List[] = {3, 22};
	RsslUInt32		providerView1_1Count = 2;


	RsslInt		providerView2List[] = {6, 7};
	RsslUInt32		providerView2Count = 2;
	RsslInt		combinedViewList[] = {3, 6, 7, 22};
	RsslUInt32		combinedViewCount = 4;

        char                    view1BodyBuf[256];
        RsslBuffer              view1DataBody = { 256, view1BodyBuf };
        RsslUInt32              view1DataBodyLen = 256;

        char                    view1_1BodyBuf[256];
        RsslBuffer              view1_1DataBody = { 256, view1_1BodyBuf };
        RsslUInt32              view1_1DataBodyLen = 256;

        char                    view2BodyBuf[256];
        RsslBuffer              view2DataBody = { 256, view2BodyBuf };
        RsslUInt32              view2DataBodyLen = 256;

	view1DataBody.length = view1DataBodyLen;
	view1_1DataBody.length = view1_1DataBodyLen;
	view2DataBody.length = view2DataBodyLen;


	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &view1DataBody, view1List, 0, view1Count);
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &view1_1DataBody, view1_1List, 0, view1_1Count);
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &view2DataBody, view2List, 0, view2Count);


	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = view1DataBody;

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
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Reissue first item with new view. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = view1_1DataBody;

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
	wtfProviderTestView(pRequestMsg, providerView1_1List, providerView1_1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.encDataBody = view2DataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Second request shouldn't arrive yet. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Provider receives request -- same stream, need refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH)); /* View Change needs image */
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, combinedViewList, combinedViewCount,
			RDM_VIEW_TYPE_FIELD_ID_LIST);

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives priority change -- although closing the item changes the view,
	 * the watchlist expects us to satisfy the previous request before
	 * requesting another view. */
	/* View flag should still be there, just no actual payload. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE((pRequestMsg->flags & RSSL_RQMF_NO_REFRESH)); 
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, NULL, 0, 0);

	/* Provider sends refresh, satisfying second view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
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

	wtfFinishTest();
}

void watchlistAggregationTest_EmptyView()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt			view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;

	char			dataBodyBuf[256];
	RsslBuffer		dataBody = { 256, dataBodyBuf };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &dataBody, view1List, 0, view1Count);

	/* Request item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = dataBody;

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
	wtfProviderTestView(pRequestMsg, view1List, view1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Request second item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = dataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE((pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);

	wtfProviderTestView(pRequestMsg, NULL, 0, 0); /* No change in view. */
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	/* Encode empty view. */
	dataBody.length = 256;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &dataBody, NULL, 0, 0);

	/* Reissue first item with new view. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = dataBody;

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
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, NULL, 0, 0); /* No change in view. */

	/* Provider sends refresh, satisfying new view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Reissue second item with empty view. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = dataBody;

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
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, NULL, 0, RDM_VIEW_TYPE_FIELD_ID_LIST); /* Empty view. */



	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_ElementViewFromMsgBuffer()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslBuffer		view1List[]
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }, { 16, const_cast<char*>("William Hartnell")}};
	RsslUInt32		view1Count = 3;

	RsslBuffer		view1_1List[]
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		view1_1Count = 2;
	RsslBuffer		view2List[]
		= {{ 9, const_cast<char*>("Tom Baker") }, { 10, const_cast<char*>("Matt Smith") }, { 10, const_cast<char*>("Matt Smith") }};
	RsslUInt32		view2Count = 3;

	RsslBuffer		providerView1List[] 
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }, { 16, const_cast<char*>("William Hartnell")}};
	RsslUInt32		providerView1Count = 3;
	RsslBuffer		providerView1_1List[] 
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		providerView1_1Count = 2;
	RsslBuffer		providerView2List[]
		= {{ 10, const_cast<char*>("Matt Smith") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		providerView2Count = 2;
	RsslBuffer		combinedViewList[]
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 10, const_cast<char*>("Matt Smith") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		combinedViewCount = 3;


        char                    view1BodyBuf[256];
        RsslBuffer              view1DataBody = { 256, view1BodyBuf };
        RsslUInt32              view1DataBodyLen = 256;

        char                    view1_1BodyBuf[256];
        RsslBuffer              view1_1DataBody = { 256, view1_1BodyBuf };
        RsslUInt32              view1_1DataBodyLen = 256;

        char                    view2BodyBuf[256];
        RsslBuffer              view2DataBody = { 256, view2BodyBuf };
        RsslUInt32              view2DataBodyLen = 256;

	view1DataBody.length = view1DataBodyLen;
	view1_1DataBody.length = view1_1DataBodyLen;
	view2DataBody.length = view2DataBodyLen;


	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_ELEMENT_NAME_LIST, &view1DataBody, 0, view1List, view1Count);
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_ELEMENT_NAME_LIST, &view1_1DataBody, 0, view1_1List, view1_1Count);
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_ELEMENT_NAME_LIST, &view2DataBody, 0, view2List, view2Count);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = view1DataBody;

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
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH)); /* View Change needs image */
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count, 
			RDM_VIEW_TYPE_ELEMENT_NAME_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Reissue first item with new view. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = view1_1DataBody;

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
	wtfProviderTestView(pRequestMsg, providerView1_1List, providerView1_1Count,
			RDM_VIEW_TYPE_ELEMENT_NAME_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;


	/* Request second item. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.msgBase.encDataBody = view2DataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Second request shouldn't arrive yet. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Provider receives request -- same stream, need refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH)); /* View Change needs image */
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	wtfProviderTestView(pRequestMsg, combinedViewList, combinedViewCount, 
			RDM_VIEW_TYPE_ELEMENT_NAME_LIST);

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives priority change -- although closing the item changes the view,
	 * the watchlist expects us to satisfy the previous request before
	 * requesting another view. */
	/* View flag should still be there, just no actual payload. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);
	wtfProviderTestView(pRequestMsg, NULL, 0, 0);

	/* Provider sends refresh, satisfying second view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;
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

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItemsInMsgBuffer_ViewMixture()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt			view1List[] = {2, 3};
	RsslUInt32		view1Count = 2;
	RsslInt		providerView1List[] = {2, 3};
	RsslUInt32		providerView1Count = 2;

	RsslBuffer		eview1List[]
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		eview1Count = 2;

        char                    viewBodyBuf[256];
        RsslBuffer              viewDataBody = { 256, viewBodyBuf };
        RsslUInt32              viewDataBodyLen = 256;

        char                    eviewBodyBuf[256];
        RsslBuffer              eviewDataBody = { 256, eviewBodyBuf };
        RsslUInt32              eviewDataBodyLen = 256;

	viewDataBody.length = viewDataBodyLen;
	eviewDataBody.length = eviewDataBodyLen;


	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_ELEMENT_NAME_LIST, &eviewDataBody, 0, eview1List, eview1Count);
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view1List, 0, view1Count);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.encDataBody = viewDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request with field view. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item with element view. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	requestMsg.msgBase.encDataBody = eviewDataBody;
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Second request is rejected for trying to mix views. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_USAGE_ERROR);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

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

	wtfFinishTest();

}

void watchlistAggregationTest_TwoItems_SnapshotView()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt		view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;
	RsslInt		view1_1List[] = {3, 22};
	RsslUInt32		view1_1Count = 2;
	RsslInt		view2List[] = {6, 7};
	RsslUInt32		view2Count = 2;

	RsslInt		providerView1List[] = {2, 6, 7};
	RsslUInt32		providerView1Count = 3;

	RsslInt		providerView1_1List[] = {3, 22};
	RsslUInt32		providerView1_1Count = 2;


	RsslInt		providerView2List[] = {6, 7};
	RsslUInt32		providerView2Count = 2;
	RsslInt		combinedViewList[] = {3, 6, 7, 22};
	RsslUInt32		combinedViewCount = 4;

	char			viewBodyBuf[256];
	RsslBuffer		viewDataBody = { 256, viewBodyBuf };
	RsslUInt32		viewDataBodyLen = 256;


	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view1List, 0, view1Count);
	requestMsg.msgBase.encDataBody = viewDataBody;

	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);


	wtfFinishTest();
}

void watchlistAggregationTest_SnapshotBeforeStreaming_View()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerItemStream;
	
	RsslInt		view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;

	RsslInt		view2List[] = {6, 7};
	RsslUInt32		view2Count = 2;

	char			viewBodyBuf[256];
	RsslBuffer		viewDataBody = { 256, viewBodyBuf };
	RsslUInt32		viewDataBodyLen = 256;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item as a snapshot. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	requestMsg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_VIEW;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	opts.requestMsgOptions.pUserSpec = (void*)(int)0xfaabd00d;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view1List, 0, view1Count);
	requestMsg.msgBase.encDataBody = viewDataBody;


	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	wtfProviderTestView(pRequestMsg, view1List, view1Count, RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item as streaming. */
	requestMsg.msgBase.streamId = 3;
	requestMsg.flags |= RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_FIELD_ID_LIST, &viewDataBody, view2List, 0, view2Count);
	requestMsg.msgBase.encDataBody = viewDataBody;


	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives nothing. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());
	
	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)(int)0xfaabd00d);

	/* Provider receives streaming request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	wtfProviderTestView(pRequestMsg, view2List, view2Count, RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void watchlistAggregationTest_TwoItems_ViewMixture()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslInt		view1List[] = {2, 3};
	RsslUInt32		view1Count = 2;
	RsslInt		providerView1List[] = {2, 3};
	RsslUInt32		providerView1Count = 2;

	RsslBuffer		eview1List[]
		= {{ 11, const_cast<char*>("Jon Pertwee") }, { 9, const_cast<char*>("Tom Baker") }};
	RsslUInt32		eview1Count = 2;

	char			viewBodyBuf[256];
	RsslBuffer		viewDataBody = { 256, viewBodyBuf };
	RsslUInt32		viewDataBodyLen = 256;


	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
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

	/* Provider receives request with field view. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	wtfProviderTestView(pRequestMsg, providerView1List, providerView1Count,
			RDM_VIEW_TYPE_FIELD_ID_LIST);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item with element view. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;

	viewDataBody.length = viewDataBodyLen;
	wtfConsumerEncodeViewRequest(RDM_VIEW_TYPE_ELEMENT_NAME_LIST, &viewDataBody, 0, eview1List, view1Count);
	requestMsg.msgBase.encDataBody = viewDataBody;

	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Second request is rejected for trying to mix views. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_USAGE_ERROR);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends refresh, satisfying first view. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
		| RSSL_RFMF_HAS_QOS;
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
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

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

	wtfFinishTest();
}

void watchlistAggregationTest_ThreeItemsInMsgBuffer_Batch()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslInt32		providerTriStream, providerDjiStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslBuffer		itemList[] = {{5, const_cast<char*>("TRI.N")}, {5, const_cast<char*>("TRI.N")}, {4, const_cast<char*>(".DJI")}};
	RsslUInt32		itemCount = 3;

	char                    bodyBuf[256];
	RsslBuffer              dataBody = { 256, bodyBuf };
	RsslUInt32              dataBodyLen = 256;

	RsslUInt8 domains[] = { RSSL_DMT_MARKET_PRICE, RSSL_DMT_SYMBOL_LIST, RSSL_DMT_DICTIONARY };
	RsslUInt32 domainCount = 3;
	RsslUInt32 ui;
	WtfSetupConnectionOpts csOpts;

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.consumerDictionaryCallback = (WtfCallbackAction)RSSL_FALSE;
	csOpts.providerDictionaryCallback = (WtfCallbackAction)RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	for (ui = 0; ui < domainCount; ++ui)
	{
		dataBody.length = dataBodyLen;

		wtfConsumerEncodeBatchRequest(&dataBody, itemList, itemCount);

		/* Request items. */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 2;
		requestMsg.msgBase.domainType = domains[ui];
		requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;


		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_BATCH;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		requestMsg.msgBase.encDataBody = dataBody;

		if (domains[ui] == RSSL_DMT_DICTIONARY)
			requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;


		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		opts.requestMsgOptions.pUserSpec = (void*)0x77777777;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		/* Consumer receives close ack for batch. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
		ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
		ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);

		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(!wtfGetEvent());

		/* Provider receives request for TRI.N. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
		ASSERT_TRUE(pRequestMsg->priorityClass == 1);
		ASSERT_TRUE(pRequestMsg->priorityCount == 2);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemList[0]));
		providerTriStream = pRequestMsg->msgBase.streamId;

		/* Provider receives request for .DJI. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemList[2]));
		providerDjiStream = pRequestMsg->msgBase.streamId;

		/* Provider sends refreshes */
		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.msgBase.streamId = providerTriStream;
		refreshMsg.msgBase.domainType = domains[ui];
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
			| RSSL_RFMF_HAS_QOS;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		rsslClearRefreshMsg(&refreshMsg);
		refreshMsg.msgBase.streamId = providerDjiStream;
		refreshMsg.msgBase.domainType = domains[ui];
		refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE 
			| RSSL_RFMF_HAS_QOS;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&refreshMsg;
		wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

		/* Consumer receives refreshes. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
		ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);
		ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
		ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 4);
		ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);
		ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
		ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
		ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 5);
		ASSERT_TRUE(pEvent->rsslMsg.pUserSpec == (void*)0x77777777);
		ASSERT_TRUE(pEvent->rsslMsg.pServiceName != NULL);
		ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

		/* Close first item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 3;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives request -- same stream, no refresh, updated priority. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
		ASSERT_TRUE(pRequestMsg->priorityClass == 1);
		ASSERT_TRUE(pRequestMsg->priorityCount == 1);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerTriStream);

		/* Close second item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 4;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerTriStream);

		/* Close third item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 5;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerDjiStream);

	}

	wtfFinishTest();

}

void watchlistAggregationTest_ThreeItemsInMsgBuffer_BatchWithView()
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerTriStream, providerDjiStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
	RsslStatusMsg	*pStatusMsg;
	RsslBuffer		itemList[] = {{5, const_cast<char*>("TRI.N")}, {5, const_cast<char*>("TRI.N")}, {4, const_cast<char*>(".DJI")}};
	RsslUInt32		itemCount = 3;

	RsslInt			view1List[] = {2, 6, 7};
	RsslUInt32		view1Count = 3;

	char                    bodyBuf[256];
	RsslBuffer              dataBody = { 256, bodyBuf };
	RsslUInt32              dataBodyLen = 256;

	RsslUInt8 domains[] = { RSSL_DMT_MARKET_PRICE, RSSL_DMT_SYMBOL_LIST, RSSL_DMT_DICTIONARY };
	RsslUInt32 domainCount = 3;
	RsslUInt32 ui;
	WtfSetupConnectionOpts csOpts;

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.consumerDictionaryCallback = (WtfCallbackAction)RSSL_FALSE;
	csOpts.providerDictionaryCallback = (WtfCallbackAction)RSSL_FALSE;
	wtfSetupConnection(&csOpts);

	for (ui = 0; ui < domainCount; ++ui)
	{
		WtfConsumerRequestPayloadOptions payloadOpts;

		dataBody.length = dataBodyLen;

		wtfClearConsumerRequestPayloadOptions(&payloadOpts);
		payloadOpts.batchItemList = itemList;
		payloadOpts.batchItemCount = itemCount;
		payloadOpts.viewFieldList = view1List;
		payloadOpts.viewCount = view1Count;
		payloadOpts.viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
		wtfConsumerEncodeRequestPayload(&dataBody, &payloadOpts);

		/* Request items. */
		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.streamId = 2;
		requestMsg.msgBase.domainType = domains[ui];
		requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

		requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_BATCH | RSSL_RQMF_HAS_VIEW;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		requestMsg.msgBase.encDataBody = dataBody;

		if (domains[ui] == RSSL_DMT_DICTIONARY)
			requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;


		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&requestMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

		/* Consumer receives close ack for batch. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
		ASSERT_TRUE(pStatusMsg->msgBase.streamId == 2);
		ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
		ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_CLOSED);
		ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(!wtfGetEvent());

		/* Provider receives request for TRI.N. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
		ASSERT_TRUE(pRequestMsg->priorityClass == 1);
		ASSERT_TRUE(pRequestMsg->priorityCount == 2);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemList[0]));
		providerTriStream = pRequestMsg->msgBase.streamId;
		wtfProviderTestView(pRequestMsg, view1List, view1Count,
				RDM_VIEW_TYPE_FIELD_ID_LIST);

		/* Provider receives request for .DJI. */
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
		ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemList[2]));
		wtfProviderTestView(pRequestMsg, view1List, view1Count,
				RDM_VIEW_TYPE_FIELD_ID_LIST);
		providerDjiStream = pRequestMsg->msgBase.streamId;

		/* Close first item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 3;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives request -- same stream, no refresh, updated priority. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
		ASSERT_TRUE(pRequestMsg->priorityClass == 1);
		ASSERT_TRUE(pRequestMsg->priorityCount == 1);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerTriStream);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_VIEW);

		/* Close second item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 4;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerTriStream);

		/* Close third item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 5;
		closeMsg.msgBase.domainType = domains[ui];

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerDjiStream);

	}

	wtfFinishTest();

}

void watchlistAggregationTest_ThreeItems_OnePrivate()
{
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream, providerPrivateItemStream;
	RsslCloseMsg	closeMsg, *pCloseMsg;
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

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Request third item as a private stream. */
	requestMsg.msgBase.streamId = 4;
	requestMsg.flags |= RSSL_RQMF_PRIVATE_STREAM;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives private request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId != providerItemStream);
	providerPrivateItemStream = pRequestMsg->msgBase.streamId;

	/* Close first item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 1);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Close private stream. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 4;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerPrivateItemStream);

	/* Close second item. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives close. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
	ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);

	wtfFinishTest();
}

#ifdef COMPILE_64BITS
void watchlistAggregationTest_Multicast_OneItem_Reorder(RsslBool gapRecovery)
{
	/* Produces an out-of-sync unicast/broadcast stream to test that watchlist properly buffers 
	 * and fowards unicast and broadcast messages according to our expected ordering. */

	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;
	WtfSubmitMsgOptionsEx optsEx;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.multicastGapRecovery = gapRecovery;
	wtfSetupConnection(&sOpts);

	wtfClearSubmitMsgOptionsEx(&optsEx);

	/* Request item. */
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


	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends multicast update broadcast. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 1;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer does not receive it. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends multicast update broadcast (this one should be buffered). */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer does not receive it yet. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends multicast update broadcast (this one should be buffered). */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer does not receive it yet. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends multicast update broadcast (this one should be buffered). */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer does not receive it yet. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends first refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 1;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

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

	/* Consumer receives nothing else. */
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends second refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update (2), and refresh (2). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider sends final refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update (3), refresh (3), and update (4). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	wtfFinishTest();
}
#endif

typedef struct
{
	RsslMsgClasses	msgClass;				/* Class of the message. May be refresh or update,
											 * and for this particular test refreshes are
											 * sent unicast and updates broadcast. */
	RsslUInt32		seqNum;					/* Sequence number to associate with the message. */
	int				isRefreshComplete;		/* For refreshes, indicates whether this is
											 * the final refresh part. */
} OrderedMsg;


#define MAX_ORDERED_MSGS 16
/* Structure representing series of unicast & broadcast messages with which to test
 * reorderings. */
typedef struct
{
	RsslRefreshMsg	sampleRefreshPart;		/* Sample refresh provider will send. */
	RsslRefreshMsg	sampleRefreshComplete;	/* Sample final refresh provider send. */
	RsslUpdateMsg	sampleUpdate;			/* Sample update provider will send. */
	RsslUInt32		inUcMsgCount;			/* Number of messages in inUcMsgs. */
	OrderedMsg		*inUcMsgs;				/* Ordered list of unicast messages. */
	RsslUInt32		inBcMsgCount;			/* Number of messages in inBcMsgs. */
	OrderedMsg		*inBcMsgs;				/* Ordered list of unicast messages. */
	RsslUInt32		expectedMsgCount;		/* Number of messages in expectedMsgs. */
	OrderedMsg		*expectedMsgs;			/* Messages (in order) that the consumer expects. */

	/* Used internally while testing. */
	RsslUInt32		_inMsgCount;				/* Number of messages in _inMsgs. */
	OrderedMsg		_inMsgs[MAX_ORDERED_MSGS];	/* Specific ordering of messages to test. */
	RsslInt32		_testId;					/* Test ID, to aid setting debugger breakpoints. */
} ReorderTest;

void orderedMsgSet(OrderedMsg *pMsg, RsslMsgClasses msgClass, RsslUInt32 seqNum, RsslBool isRefreshComplete)
{
	assert(!isRefreshComplete || msgClass == RSSL_MC_REFRESH);

	pMsg->msgClass = msgClass;
	pMsg->seqNum = seqNum;
	pMsg->isRefreshComplete = isRefreshComplete;
}


/* Recursively sets up permutations of unicast/broadcast messages for testing.
 * The ordering is set up by placing the unicast messages in the various possible
 * positions in the array(though still in increasing order).  Once we've placed all unicast
 * messages, the remaining cells are filled with broadcast messages, and at that point
 * the ordering is ready so a test is run. */
void setupReorderTest(ReorderTest *pTest, RsslUInt32 nextUcPos, RsslUInt32 ucMsgsLeft)
{
	RsslUInt32 totalMsgs = pTest->inBcMsgCount + pTest->inUcMsgCount;

	if (ucMsgsLeft == 0)
	{
		/* All unicast messages are in position, so fill in the broadcast messages and
		 * run the test. */
		RsslUInt32 i, bcPos;
		RsslMsg rsslMsg;
		RsslRequestMsg requestMsg, *pRequestMsg;
		RsslCloseMsg closeMsg, *pCloseMsg;
		RsslInt32 providerItemStream, consumerItemStream;
		WtfEvent *pEvent;
		RsslReactorSubmitMsgOptions opts;
		WtfSubmitMsgOptionsEx optsEx;

		assert(pTest->_inMsgCount == pTest->inUcMsgCount);

		/* Fill remaining slots with broadcast messages. */
		for (i = 0, bcPos = 0; bcPos < pTest->inBcMsgCount; ++i)
		{
			assert(i < pTest->inBcMsgCount + pTest->inUcMsgCount);
			if (pTest->_inMsgs[i].msgClass == 0)
			{
				pTest->_inMsgs[i] = pTest->inBcMsgs[bcPos];
				++pTest->_inMsgCount;
				++bcPos;
			}
		}

		/* Run the test. */

		/* Request item. */
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

		/* Provider receives request. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
		ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
		ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);

		providerItemStream = pRequestMsg->msgBase.streamId;
		consumerItemStream = 2;

		/* If enabled, prints the unicast/broadcast permutations tested. */
//#define REORDER_TEST_PRINT
#ifdef REORDER_TEST_PRINT
		//printf("%5d:", pTest->_testId);

		for(i = 0; i < pTest->_inMsgCount; ++i)
		{
			OrderedMsg *pOMsg = &pTest->_inMsgs[i];
			switch(pOMsg->msgClass)
			{
				case RSSL_MC_REFRESH:
					if (pOMsg->isRefreshComplete)
						snprintf(outBuf, sizeof(outBuf), "RC%u", pOMsg->seqNum);
					else
						snprintf(outBuf, sizeof(outBuf), "R%u", pOMsg->seqNum);
					break;

				case RSSL_MC_UPDATE:
					snprintf(outBuf, sizeof(outBuf), "U%u", pOMsg->seqNum);
					break;

				default:
					ASSERT_TRUE(0);
					break;
			}

			//printf("%6s ", outBuf);
		}

		//printf("\n");
#endif

		/* Send each each message according to the specified order. */
		for(i = 0; i < pTest->_inMsgCount; ++i)
		{
			OrderedMsg *pMsg = &pTest->_inMsgs[i];
			switch(pMsg->msgClass)
			{
				case RSSL_MC_REFRESH:
					rsslMsg.refreshMsg = pMsg->isRefreshComplete ?
						pTest->sampleRefreshComplete
						: pTest->sampleRefreshPart;
					rsslMsg.msgBase.streamId = providerItemStream;
					break;
				case RSSL_MC_UPDATE:
					rsslMsg.updateMsg = pTest->sampleUpdate;
					rsslMsg.msgBase.streamId = 0;
					break;
				default:
					ASSERT_TRUE(0);
					break;
			}

			rsslClearReactorSubmitMsgOptions(&opts);
			wtfClearSubmitMsgOptionsEx(&optsEx);
			opts.pRsslMsg = &rsslMsg;
			optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = pMsg->seqNum;
			wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);
		}

		wtfDispatch(WTF_TC_CONSUMER, pTest->_inMsgCount * 40);

		/* Check that the consumer received the correct messages and in the expected order. */
		for(i = 0; i < pTest->expectedMsgCount; ++i)
		{
			WtfEvent *pEvent = wtfGetEvent();
			while (!pEvent)
			{
				wtfDispatch(WTF_TC_CONSUMER, pTest->_inMsgCount * 120);
				pEvent = wtfGetEvent();
			}
			OrderedMsg *pMsg = &pTest->expectedMsgs[i];
			RsslMsg *pRsslMsg;

			ASSERT_TRUE(pEvent);
			ASSERT_TRUE((pRsslMsg = wtfGetRsslMsg(pEvent)));
			ASSERT_TRUE(pRsslMsg->msgBase.msgClass == pMsg->msgClass);
			ASSERT_TRUE(pRsslMsg->msgBase.streamId == consumerItemStream);
			ASSERT_TRUE(pEvent->rsslMsg.hasSeqNum 
					&& pEvent->rsslMsg.seqNum == pMsg->seqNum);


			if (pMsg->msgClass == RSSL_MC_REFRESH)
			{
				if (pMsg->isRefreshComplete)
					ASSERT_TRUE(pRsslMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE);
				else
					ASSERT_TRUE(!(pRsslMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE));
			}
		}

		ASSERT_TRUE(!wtfGetEvent());

		++pTest->_testId;

		/* Close item. */
		rsslClearCloseMsg(&closeMsg);
		closeMsg.msgBase.streamId = 2;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&closeMsg;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		/* Provider receives close. */
		wtfDispatch(WTF_TC_PROVIDER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pCloseMsg = (RsslCloseMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pCloseMsg->msgBase.msgClass == RSSL_MC_CLOSE);
		ASSERT_TRUE(pCloseMsg->msgBase.streamId == providerItemStream);
		
		/* Test is finished. */

		/* Remove broadcast messages for next test. */
		for (i = 0, bcPos = 0; bcPos < pTest->inBcMsgCount; ++i)
		{
			assert(i < pTest->inBcMsgCount + pTest->inUcMsgCount);
			if (pTest->_inMsgs[i].msgClass == RSSL_MC_UPDATE)
			{
				pTest->_inMsgs[i].msgClass = (RsslMsgClasses)0;
				--pTest->_inMsgCount;
				++bcPos;
			}
		}

		assert(pTest->_inMsgCount == pTest->inUcMsgCount);
	}
	else
	{
		/* Unicast messages still need to be positioned.  Choose a position for the next
		 * one and recurse. */
		RsslUInt32 i;

		/* Loop through open cells so that this messages is tested in each possible
		 * position (being sure to leave room for other unicast messages that still need to
		 * be added as well). */
		for (i = nextUcPos; i <= totalMsgs - ucMsgsLeft; ++i)
		{

			pTest->_inMsgs[i] = pTest->inUcMsgs[pTest->inUcMsgCount - ucMsgsLeft];
			++pTest->_inMsgCount;

			setupReorderTest(pTest, i + 1, ucMsgsLeft - 1);

			pTest->_inMsgs[i].msgClass = (RsslMsgClasses)0;
			--pTest->_inMsgCount;
		}

	}
}

/* Runs a test of the given message order. 
 * Calls setupReorderTest to wrapping positions. */
void testReorder(ReorderTest *pTest)
{
	assert(pTest->inUcMsgCount + pTest->inBcMsgCount <= MAX_ORDERED_MSGS);
	setupReorderTest(pTest, 0, pTest->inUcMsgCount);
}
#ifdef COMPILE_64BITS
void watchlistAggregationTest_Multicast_BroadcastUnicastPermutations(RsslBool gapRecovery)
{
	/* Try sending unicast and broadcast messages in varying order with respect to each other,
	 * and verify that they are still provided to the application in
	 * the correct order. */

	WtfSubmitMsgOptionsEx optsEx;
	WtfSetupConnectionOpts sOpts;
	OrderedMsg 			oneRefreshList[1];
	OrderedMsg			twoRefreshList[2];
	OrderedMsg			twoRefreshListSameSeq[2];
	OrderedMsg			threeRefreshList[3];
	OrderedMsg			fiveUpdateList[5];
	OrderedMsg			expectedMsgList[16];

	ReorderTest reorderTest;

	memset(&reorderTest, 0, sizeof(ReorderTest));

	/* Set up partial refresh (part number left off for simplicity; partNum gaps
	 * are tested elsewhere). */
	rsslClearRefreshMsg(&reorderTest.sampleRefreshPart);
	reorderTest.sampleRefreshPart.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE
		| RSSL_RFMF_HAS_QOS | RSSL_RFMF_SOLICITED;
	reorderTest.sampleRefreshPart.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	reorderTest.sampleRefreshPart.msgBase.containerType = RSSL_DT_NO_DATA;
	reorderTest.sampleRefreshPart.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	reorderTest.sampleRefreshPart.msgBase.msgKey.serviceId = service1Id;
	reorderTest.sampleRefreshPart.msgBase.msgKey.identifier = 5;
	reorderTest.sampleRefreshPart.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	reorderTest.sampleRefreshPart.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	reorderTest.sampleRefreshPart.state.streamState = RSSL_STREAM_OPEN;
	reorderTest.sampleRefreshPart.state.dataState = RSSL_DATA_OK;

	/* Set up completed refresh (it's like the part, just with a complete flag). */
	rsslClearRefreshMsg(&reorderTest.sampleRefreshComplete);
	reorderTest.sampleRefreshComplete = reorderTest.sampleRefreshPart;
	reorderTest.sampleRefreshComplete.flags |= RSSL_RFMF_REFRESH_COMPLETE;

	/* Setup update. */
	rsslClearUpdateMsg(&reorderTest.sampleUpdate);
	reorderTest.sampleUpdate.msgBase.streamId = 0;
	reorderTest.sampleUpdate.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	reorderTest.sampleUpdate.flags = RSSL_UPMF_HAS_MSG_KEY;
	reorderTest.sampleUpdate.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	reorderTest.sampleUpdate.msgBase.msgKey.serviceId = service1Id;
	reorderTest.sampleUpdate.msgBase.msgKey.identifier = 5;
	reorderTest.sampleUpdate.msgBase.containerType = RSSL_DT_NO_DATA;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	sOpts.multicastGapRecovery = gapRecovery;
	wtfSetupConnection(&sOpts);

	wtfClearSubmitMsgOptionsEx(&optsEx);

	/* Setup refresh lists. */
	orderedMsgSet(&oneRefreshList[0], RSSL_MC_REFRESH, 3, RSSL_TRUE);

	orderedMsgSet(&twoRefreshList[0], RSSL_MC_REFRESH, 3, RSSL_FALSE);
	orderedMsgSet(&twoRefreshList[1], RSSL_MC_REFRESH, 5, RSSL_TRUE);

	orderedMsgSet(&twoRefreshListSameSeq[0], RSSL_MC_REFRESH, 4, RSSL_FALSE);
	orderedMsgSet(&twoRefreshListSameSeq[1], RSSL_MC_REFRESH, 4, RSSL_TRUE);

	orderedMsgSet(&threeRefreshList[0], RSSL_MC_REFRESH, 2, RSSL_FALSE);
	orderedMsgSet(&threeRefreshList[1], RSSL_MC_REFRESH, 4, RSSL_FALSE);
	orderedMsgSet(&threeRefreshList[2], RSSL_MC_REFRESH, 5, RSSL_TRUE);

	/* Setup update list. */
	orderedMsgSet(&fiveUpdateList[0], RSSL_MC_UPDATE, 2, RSSL_FALSE);
	orderedMsgSet(&fiveUpdateList[1], RSSL_MC_UPDATE, 3, RSSL_FALSE);
	orderedMsgSet(&fiveUpdateList[2], RSSL_MC_UPDATE, 4, RSSL_FALSE);
	orderedMsgSet(&fiveUpdateList[3], RSSL_MC_UPDATE, 5, RSSL_FALSE);
	orderedMsgSet(&fiveUpdateList[4], RSSL_MC_UPDATE, 6, RSSL_FALSE);

	/* Level I Test (single refresh). */

	/* Setup expected msg list. */
	orderedMsgSet(&expectedMsgList[0], RSSL_MC_REFRESH, 3, RSSL_TRUE);
	orderedMsgSet(&expectedMsgList[1], RSSL_MC_UPDATE, 4, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[2], RSSL_MC_UPDATE, 5, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[3], RSSL_MC_UPDATE, 6, RSSL_FALSE);

	reorderTest.inUcMsgs = oneRefreshList;
	reorderTest.inUcMsgCount = 1;
	reorderTest.inBcMsgs = fiveUpdateList;
	reorderTest.inBcMsgCount = 5;
	reorderTest.expectedMsgs = expectedMsgList;
	reorderTest.expectedMsgCount = 4;

	testReorder(&reorderTest);

	/* Level II Test (two-part refresh). */

	/* Setup expected msg list. */
	orderedMsgSet(&expectedMsgList[0], RSSL_MC_REFRESH, 3, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[1], RSSL_MC_UPDATE, 4, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[2], RSSL_MC_UPDATE, 5, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[3], RSSL_MC_REFRESH, 5, RSSL_TRUE);
	orderedMsgSet(&expectedMsgList[4], RSSL_MC_UPDATE, 6, RSSL_FALSE);

	reorderTest.inUcMsgs = twoRefreshList;
	reorderTest.inUcMsgCount = 2;
	reorderTest.inBcMsgs = fiveUpdateList;
	reorderTest.inBcMsgCount = 5;
	reorderTest.expectedMsgs = expectedMsgList;
	reorderTest.expectedMsgCount = 5;

	testReorder(&reorderTest);

	/* Level II Test (two-part refresh, both have same sequence number). */

	/* Setup expected msg list. */
	orderedMsgSet(&expectedMsgList[0], RSSL_MC_REFRESH, 4, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[1], RSSL_MC_REFRESH, 4, RSSL_TRUE);
	orderedMsgSet(&expectedMsgList[2], RSSL_MC_UPDATE, 5, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[3], RSSL_MC_UPDATE, 6, RSSL_FALSE);

	reorderTest.inUcMsgs = twoRefreshListSameSeq;
	reorderTest.inUcMsgCount = 2;
	reorderTest.inBcMsgs = fiveUpdateList;
	reorderTest.inBcMsgCount = 5;
	reorderTest.expectedMsgs = expectedMsgList;
	reorderTest.expectedMsgCount = 4;

	testReorder(&reorderTest);

	/* Level II Test (three-part refresh). */

	/* Setup expected msg list. */
	orderedMsgSet(&expectedMsgList[0], RSSL_MC_REFRESH, 2, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[1], RSSL_MC_UPDATE, 3, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[2], RSSL_MC_UPDATE, 4, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[3], RSSL_MC_REFRESH, 4, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[4], RSSL_MC_UPDATE, 5, RSSL_FALSE);
	orderedMsgSet(&expectedMsgList[5], RSSL_MC_REFRESH, 5, RSSL_TRUE);
	orderedMsgSet(&expectedMsgList[6], RSSL_MC_UPDATE, 6, RSSL_FALSE);

	reorderTest.inUcMsgs = threeRefreshList;
	reorderTest.inUcMsgCount = 3;
	reorderTest.inBcMsgs = fiveUpdateList;
	reorderTest.inBcMsgCount = 5;
	reorderTest.expectedMsgs = expectedMsgList;
	reorderTest.expectedMsgCount = 7;

	testReorder(&reorderTest);

	wtfFinishTest();
}

void watchlistAggregationTest_Multicast_OneItem_Reorder_MultipartSnapshot()
{
	/* Tests buffering of multiple unicast messages. See UPAC-44. */

	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;
	WtfSubmitMsgOptionsEx optsEx;
	WtfSetupConnectionOpts sOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sOpts);
	wtfSetupConnection(&sOpts);

	wtfClearSubmitMsgOptionsEx(&optsEx);

	/* Request item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_NONE;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 5;

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
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_STREAMING));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends unsolicited refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_HAS_PART_NUM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 1;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Provider sends multicast updates broadcast with sequence nums 2 and 5. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends solicited refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 1;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE));
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Consumer receives nothing else. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends multicast updates broadcast with sequence nums 5 and 6
	 * (these are out of order and should be discarded). */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 6;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Provider sends solicited refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Provider sends solicited refresh part (complete). */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update 2. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends multicast updates broadcast with sequence nums 3, 4, and 5. Update 4
	 * should trigger the rest of the refresh parts and close the stream. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update 3 and 4. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	/* Consumer receives refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	ASSERT_TRUE(!(pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE));
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Consumer receives refresh (complete). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_SOLICITED);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	wtfFinishTest();
}

void watchlistAggregationTest_Multicast_OneItem_PrivateStreamOrder()
{
	/* Tests that watchlist assumes that private streams follow a single stream sequence
	 * and that no reordering is done (no messages are buffered like public streams). */

	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;
	WtfSubmitMsgOptionsEx optsEx;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL);

	wtfClearSubmitMsgOptionsEx(&optsEx);

	/* Request item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_PRIVATE_STREAM;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 5;

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
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer does not receive it. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends first refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_PRIVATE_STREAM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 3;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_PRIVATE_STREAM);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);

	/* Provider sends second refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_PRIVATE_STREAM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_PRIVATE_STREAM);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 6;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);


	/* Provider sends final refresh part. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_PRIVATE_STREAM;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.partNum = 2;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 7;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_PRIVATE_STREAM);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Provider sends update. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 8;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);


	wtfFinishTest();
}

void watchlistAggregationTest_Multicast_TwoStreams_Reissue()
{
	/* Test that reissuing a stream for a new image 
	 * does not cause other consumers to miss updates (due to buffering). */

	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslInt32		providerItemStream;
	RsslReactorSubmitMsgOptions opts;
	WtfSubmitMsgOptionsEx optsEx;

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
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 5;

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
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.identifier == 5);
	providerItemStream = pRequestMsg->msgBase.streamId;

	/* Request second item. */
	requestMsg.msgBase.streamId = 3;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request -- same stream, no refresh, updated priority. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY);
	ASSERT_TRUE(pRequestMsg->priorityClass == 1);
	ASSERT_TRUE(pRequestMsg->priorityCount == 2);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.identifier == 5);
	ASSERT_TRUE(pRequestMsg->msgBase.streamId == providerItemStream);

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 4;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Consumer receives refresh on each stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 3);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));

	/* Consumer re-requests first stream. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_IDENTIFIER;
	requestMsg.msgBase.msgKey.identifier = 5;

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
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.identifier == 5);
	providerItemStream = pRequestMsg->msgBase.streamId;


	/* Provider sends multicast update broadcast. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 0;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	updateMsg.msgBase.msgKey.identifier = 5;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfClearSubmitMsgOptionsEx(&optsEx);
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Provider sends refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerItemStream;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.msgBase.msgKey.identifier = 5;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	optsEx.hasSeqNum = RSSL_TRUE; optsEx.seqNum = 5;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, &optsEx, RSSL_TRUE);

	/* Order of these events may depend on implementation, but at the very
	 * least stream 3 must get an update and stream 2 must get the refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives update on stream 3. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 3);

	/* Consumer receives refresh on stream 2. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rsslMsg.pServiceName);
	ASSERT_TRUE(rsslBufferIsEqual(pEvent->rsslMsg.pServiceName, &service1Name));


	wtfFinishTest();
}
#endif
