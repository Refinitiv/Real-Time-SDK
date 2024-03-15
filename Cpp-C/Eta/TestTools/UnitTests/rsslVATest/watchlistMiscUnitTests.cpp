/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

/* watchlistMiscTest_SeqNumCompare tests the rsslSeqNumCompare function used internally
 * by the watchlist. */
#include "rtr/wlMsgReorderQueue.h"

void watchlistMiscTest_BigGenericMsg(RsslConnectionTypes connectionType);
void watchlistMiscTest_BigPostMsg(RsslConnectionTypes connectionType);
void watchlistMiscTest_MsgKeyInUpdates(RsslConnectionTypes connectionType);
void watchlistMiscTest_SeqNumCompare(RsslConnectionTypes connectionType);
void watchlistMiscTest_AdminRsslMsgs(RsslConnectionTypes connectionType);

class WatchlistMiscUnitTest : public ::testing::TestWithParam<RsslConnectionTypes> {
public:

	static void SetUpTestCase()
	{
		wtfInit(NULL, 50000000);
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

TEST_P(WatchlistMiscUnitTest, SeqNumCompare)
{
	watchlistMiscTest_SeqNumCompare(GetParam());
}

TEST_P(WatchlistMiscUnitTest, BigGenericMsg)
{
	watchlistMiscTest_BigGenericMsg(GetParam());
}

TEST_P(WatchlistMiscUnitTest, BigPostMsg)
{
	watchlistMiscTest_BigPostMsg(GetParam());
}

TEST_P(WatchlistMiscUnitTest, MsgKeyInUpdates)
{
	watchlistMiscTest_MsgKeyInUpdates(GetParam());
}

TEST_P(WatchlistMiscUnitTest, AdminRsslMsgs)
{
	watchlistMiscTest_AdminRsslMsgs(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
	TestingWatchlistMiscUnitTests,
	WatchlistMiscUnitTest,
	::testing::Values(
		RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET
	));

void watchlistMiscTest_BigGenericMsg(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	unsigned char	nodeId[2] = { 0xd0, 0x0d };
	RsslBuffer		itemGroupId = { 2,  (char*)nodeId};
	RsslGenericMsg	genericMsg, *pGenericMsg;
	char			*pGenericMsgBuf;
	RsslReactorChannelInfo	channelInfo;
	RsslUInt32		msgSize;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Tests watchlist internal handling of WRITE_CALL_AGAIN by sending a really big message. */

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

	if(connectionType != RSSL_CONN_TYPE_WEBSOCKET) /* the tr_json2 protocol does not support sending group ID */
		ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->groupId, &itemGroupId));

	ASSERT_TRUE(wtfGetChannelInfo(WTF_TC_CONSUMER, &channelInfo) == RSSL_RET_SUCCESS);

	/* Sixteen times the available output size should be enough. */
	msgSize = 16 * channelInfo.rsslChannelInfo.maxFragmentSize 
		* channelInfo.rsslChannelInfo.maxOutputBuffers;
	ASSERT_TRUE(msgSize > channelInfo.rsslChannelInfo.maxFragmentSize 
			* channelInfo.rsslChannelInfo.maxOutputBuffers);

	pGenericMsgBuf = (char*)malloc(msgSize);
	ASSERT_TRUE(pGenericMsgBuf);
	memset(pGenericMsgBuf, 0, msgSize);

	/* Send a generic message. */
	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.streamId = 2;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE | RSSL_GNMF_HAS_MSG_KEY;
	genericMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	genericMsg.msgBase.encDataBody.data = pGenericMsgBuf;
	genericMsg.msgBase.encDataBody.length = msgSize;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);
	
	do
	{
		wtfDispatch(WTF_TC_CONSUMER, 1);
		ASSERT_TRUE(!wtfGetEvent());
	
		wtfDispatch(WTF_TC_PROVIDER, 1);
	} while (!(pEvent = wtfGetEvent()));

	/* Provider receives generic message. */
	ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
	ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE);
	ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_HAS_MSG_KEY);
	ASSERT_TRUE(pGenericMsg->msgBase.streamId == providerItemStream);
	ASSERT_TRUE(pGenericMsg->msgBase.containerType == RSSL_DT_OPAQUE);
	ASSERT_TRUE(pGenericMsg->msgBase.encDataBody.length == msgSize);
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.serviceId == service1Id);

	free(pGenericMsgBuf);

	wtfFinishTest();
}

void watchlistMiscTest_BigPostMsg(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	unsigned char	nodeId[2] = { 0xd0, 0x0d };
	RsslBuffer		itemGroupId = { 2,  (char*)nodeId};
	RsslPostMsg	postMsg, *pPostMsg;
	char			*pPostMsgBuf;
	RsslReactorChannelInfo	channelInfo;
	RsslUInt32		msgSize;
	RsslUInt		ui;

	/* Tests watchlist internal handling of WRITE_CALL_AGAIN by sending a really big message. */

	RsslUInt8		domains[] = { RSSL_DMT_MARKET_PRICE, RSSL_DMT_LOGIN };
	RsslInt32		streams[] = { 2, 1 };
	RsslUInt32		domainsLength = 2;

	ASSERT_TRUE(wtfStartTest());
	wtfSetupConnection(NULL, connectionType);

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

	if (connectionType != RSSL_CONN_TYPE_WEBSOCKET) /* the tr_json2 protocol does not support sending group ID */
		ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->groupId, &itemGroupId));

	ASSERT_TRUE(wtfGetChannelInfo(WTF_TC_CONSUMER, &channelInfo) == RSSL_RET_SUCCESS);


	/* Eight times the available output size should be enough. */
	msgSize = 8 * channelInfo.rsslChannelInfo.maxFragmentSize 
		* channelInfo.rsslChannelInfo.maxOutputBuffers;
	ASSERT_TRUE(msgSize > channelInfo.rsslChannelInfo.maxFragmentSize 
			* channelInfo.rsslChannelInfo.maxOutputBuffers);

	pPostMsgBuf = (char*)malloc(msgSize);
	ASSERT_TRUE(pPostMsgBuf);
	memset(pPostMsgBuf, 0, msgSize);

	for (ui = 0; ui < domainsLength; ++ui)
	{

		/* Send a post message. */
		rsslClearPostMsg(&postMsg);
		postMsg.msgBase.streamId = streams[ui];
		postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		postMsg.flags = RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_HAS_MSG_KEY;
		postMsg.msgBase.containerType = RSSL_DT_OPAQUE;
		postMsg.msgBase.encDataBody.data = pPostMsgBuf;
		postMsg.msgBase.encDataBody.length = msgSize;

		rsslClearReactorSubmitMsgOptions(&opts);
		opts.pRsslMsg = (RsslMsg*)&postMsg;
		opts.pServiceName = &service1Name;
		wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

		do
		{
			wtfDispatch(WTF_TC_CONSUMER, 1);

			wtfDispatch(WTF_TC_PROVIDER, 1);
		} while (!(pEvent = wtfGetEvent()));

		/* Provider receives post message. */
		ASSERT_TRUE(pPostMsg = (RsslPostMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pPostMsg->msgBase.msgClass == RSSL_MC_POST);
		ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_POST_COMPLETE);
		ASSERT_TRUE(pPostMsg->flags & RSSL_PSMF_HAS_MSG_KEY);
		ASSERT_TRUE(pPostMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pPostMsg->msgBase.msgKey.serviceId == service1Id);

		if (domains[ui] == RSSL_DMT_LOGIN)
			ASSERT_TRUE(pPostMsg->msgBase.streamId == wtfGetProviderLoginStream());
		else
			ASSERT_TRUE(pPostMsg->msgBase.streamId == providerItemStream);

		ASSERT_TRUE(pPostMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pPostMsg->msgBase.containerType == RSSL_DT_OPAQUE);
		ASSERT_TRUE(pPostMsg->msgBase.encDataBody.length == msgSize);
	}

	free(pPostMsgBuf);
	wtfFinishTest();
}

void watchlistMiscTest_MsgKeyInUpdates(RsslConnectionTypes connectionType)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent		*pEvent;
	RsslRequestMsg	requestMsg, *pRequestMsg;
	RsslInt32		providerItemStream;
	RsslRefreshMsg	refreshMsg, *pRefreshMsg;
	RsslUpdateMsg	updateMsg, *pUpdateMsg;
	RsslPostMsg		postMsg, *pPostMsg;
	RsslAckMsg		ackMsg, *pAckMsg;
	RsslGenericMsg	genericMsg, *pGenericMsg;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Request first item. */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = 2;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_QOS | RSSL_RQMF_MSG_KEY_IN_UPDATES;
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

	/* Provider does not receive this flag. */
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_MSG_KEY_IN_UPDATES));

	/* Provider sends refresh without key. */
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

	/* Consumer receives refresh with key inserted by watchlist. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);

	/* Provider sends update without key. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = providerItemStream;
	updateMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update with key inserted by watchlist. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.msgClass == RSSL_MC_UPDATE);
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pUpdateMsg->flags & RSSL_UPMF_HAS_MSG_KEY);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);

	/* Provider sends generic message without key. */
	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.streamId = providerItemStream;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&genericMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives generic with key inserted by watchlist. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pGenericMsg = (RsslGenericMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pGenericMsg->msgBase.msgClass == RSSL_MC_GENERIC);
	ASSERT_TRUE(pGenericMsg->msgBase.streamId == 2);
	ASSERT_TRUE(pGenericMsg->flags & RSSL_GNMF_HAS_MSG_KEY);
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pGenericMsg->msgBase.msgKey.serviceId == service1Id);

	/*** Send a post, so we can test the AckMsg. ***/

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

	/* Consumer receives ack, with message key inserted by watchlist. */
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

	ASSERT_TRUE(pAckMsg->flags & RSSL_AKMF_HAS_MSG_KEY);
	ASSERT_TRUE(pAckMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pAckMsg->msgBase.msgKey.serviceId == service1Id);

	wtfFinishTest();
}

void watchlistMiscTest_SeqNumCompare(RsslConnectionTypes connectionType)
{
	/* Tests that the sequence number comparison function works correctly on our supported 
	 * platforms. */
	RsslUInt32 seqNum1;
	RsslUInt32 seqNum2;

	/* Test "before" and "after" cases.  This test ensures that unsigned subtraction has the intended 
	 * behavior. */
	seqNum1 = 2;
	seqNum2 = 3;
	ASSERT_TRUE(rsslSeqNumCompare(seqNum1, seqNum2) < 0 );
	ASSERT_TRUE(rsslSeqNumCompare(seqNum2, seqNum1) > 0);

	/* Test "equal case. */
	seqNum1 = 3;
	seqNum2 = 3;
	ASSERT_TRUE(rsslSeqNumCompare(seqNum1, seqNum2) == 0);

	/* Test "before" and "after" cases with wraparound. */
	seqNum1 = 0xffffffff;
	seqNum2 = 1;
	ASSERT_TRUE(rsslSeqNumCompare(seqNum1, seqNum2) < 0 );
	ASSERT_TRUE(rsslSeqNumCompare(seqNum2, seqNum1) > 0);


}

/* Converts an RDM msg into an RsslMsg, using the given buffer for holding encoded data. */
static void _wtfRDMMsgToRsslMsg(RsslBuffer *pBuffer, RsslRDMMsg *pRdmMsg, RsslMsg *pRsslMsg)
{
	RsslEncodeIterator			eIter;
	RsslDecodeIterator			dIter;
	RsslErrorInfo				rsslErrorInfo;

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	rsslSetEncodeIteratorBuffer(&eIter, pBuffer);
	ASSERT_TRUE(rsslEncodeRDMMsg(&eIter, pRdmMsg, &pBuffer->length,
				&rsslErrorInfo) == RSSL_RET_SUCCESS);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	rsslSetDecodeIteratorBuffer(&dIter, pBuffer);
	ASSERT_TRUE(rsslDecodeMsg(&dIter, pRsslMsg) == RSSL_RET_SUCCESS);
}

void watchlistMiscTest_AdminRsslMsgs(RsslConnectionTypes connectionType)
{

	/* Test that login & directory messages can be submitted as RsslMsgs to the watchlist. */
	WtfEvent					*pEvent;
	RsslRDMLoginRequest			loginRequest, *pLoginRequest;
	RsslRDMLoginRefresh			loginRefresh;
	RsslRDMDirectoryRequest 	directoryRequest, *pDirectoryRequest;
	RsslRDMDirectoryRefresh		directoryRefresh, *pDirectoryRefresh;
	RsslRDMService				service;
	RsslMsg 					rsslMsg;
	char						tmpMem[1024];
	RsslBuffer					tmpBuffer = { 1024, tmpMem };

	WtfSetupConnectionOpts sopts;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&sopts);
	sopts.login = RSSL_FALSE;
	sopts.provideLoginRefresh = RSSL_FALSE;
	sopts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&sopts, connectionType);

	/* Nothing happening. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	wtfAccept();

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer sends login request. */
	tmpBuffer.length = 1024;
	wtfInitDefaultLoginRequest(&loginRequest);
	_wtfRDMMsgToRsslMsg(&tmpBuffer, (RsslRDMMsg*)&loginRequest, &rsslMsg);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = &rsslMsg;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);


	/* Provider channel up. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginRequest = (RsslRDMLoginRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginRequest->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	ASSERT_TRUE(pLoginRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1); 
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider receives directory request from watchlist. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRequest = (RsslRDMDirectoryRequest*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRequest->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);
	ASSERT_TRUE(pDirectoryRequest->flags & RDM_DR_RQF_STREAMING);

	/* Provider sends directory response. */
	wtfInitDefaultDirectoryRefresh(&directoryRefresh, &service);
	directoryRefresh.rdmMsgBase.streamId = pDirectoryRequest->rdmMsgBase.streamId;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer requests directory. */
	tmpBuffer.length = 1024;
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 3);
	_wtfRDMMsgToRsslMsg(&tmpBuffer, (RsslRDMMsg*)&directoryRequest, &rsslMsg);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = &rsslMsg;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 3);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}
