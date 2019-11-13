/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "itemEncoder.h"
#include <assert.h>

RsslUInt32 estimateItemRefreshBufferLength(ItemInfo *itemInfo)
{
	RsslUInt32 bufferSize = 64;

	if (itemInfo->attributes.pMsgKey->flags & RSSL_MKF_HAS_NAME)
		bufferSize += itemInfo->attributes.pMsgKey->name.length;

	if (itemInfo->attributes.pMsgKey->flags & RSSL_MKF_HAS_ATTRIB)
		bufferSize += itemInfo->attributes.pMsgKey->encAttrib.length;

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			bufferSize += xmlMarketPriceMsgs.refreshMsg.estimatedContentLength;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			bufferSize += xmlMarketByOrderMsgs.refreshMsg.estimatedContentLength;
			break;
		default:
			assert(0);
			break;
	}

	return bufferSize;
}

RsslRet encodeItemRefresh(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime) 
{
	RsslRet ret;
	RsslRefreshMsg refreshMsg;
	RsslEncodeIterator encodeIter;

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	rsslClearRefreshMsg(&refreshMsg);

	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
	if (itemInfo->itemFlags & ITEM_IS_SOLICITED)
		refreshMsg.flags |= RSSL_RFMF_SOLICITED;
	if (itemInfo->itemFlags & ITEM_IS_PRIVATE)
		refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;

	refreshMsg.msgBase.msgKey = *itemInfo->attributes.pMsgKey;

	refreshMsg.msgBase.streamId = itemInfo->StreamId;

	/* Images for snapshot requests should use the non-streaming state. */
	refreshMsg.state.streamState = (itemInfo->itemFlags & ITEM_IS_STREAMING_REQ) ? 
		RSSL_STREAM_OPEN : RSSL_STREAM_NON_STREAMING;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;

	refreshMsg.qos.dynamic = RSSL_FALSE;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;

	refreshMsg.msgBase.domainType = itemInfo->attributes.domainType;

	if (pPostUserInfo)
	{
		refreshMsg.flags |= RSSL_RFMF_HAS_POST_USER_INFO;
		refreshMsg.postUserInfo = *pPostUserInfo;
	}

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketPriceDataBody(&encodeIter, &xmlMarketPriceMsgs.refreshMsg, RSSL_MC_REFRESH, 
							encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			refreshMsg.msgBase.containerType = RSSL_DT_MAP;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketByOrderDataBody(&encodeIter, &xmlMarketByOrderMsgs.refreshMsg, RSSL_MC_REFRESH, 
							encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

RsslUInt32 estimateItemUpdateBufferLength(ItemInfo *itemInfo)
{
	RsslUInt32 bufferSize = 64;

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			bufferSize += getNextMarketPriceUpdateEstimatedContentLength(
					(MarketPriceItem*)itemInfo->itemData);
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			bufferSize += getNextMarketByOrderUpdateEstimatedContentLength(
					(MarketByOrderItem*)itemInfo->itemData);
			break;
		default:
			assert(0);
			break;
	}

	return bufferSize;
}

RsslRet encodeItemUpdate(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime)
{
	RsslRet ret;
	RsslUpdateMsg updateMsg;
	RsslEncodeIterator encodeIter;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = itemInfo->StreamId;

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	updateMsg.msgBase.domainType = itemInfo->attributes.domainType;

	if (pPostUserInfo)
	{
		updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
		updateMsg.postUserInfo = *pPostUserInfo;
	}

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketPriceDataBody(&encodeIter, 
							getNextMarketPriceUpdate((MarketPriceItem*)itemInfo->itemData),
							RSSL_MC_UPDATE, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			updateMsg.msgBase.containerType = RSSL_DT_MAP;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketByOrderDataBody(&encodeIter, 
							getNextMarketByOrderUpdate((MarketByOrderItem*)itemInfo->itemData),
							RSSL_MC_UPDATE, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

RsslUInt32 estimateItemPostBufferLength(ItemInfo *itemInfo)
{
	RsslUInt32 bufferSize = 64;

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			bufferSize += getNextMarketPricePostEstimatedContentLength(
					(MarketPriceItem*)itemInfo->itemData);
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			bufferSize += getNextMarketByOrderPostEstimatedContentLength(
					(MarketByOrderItem*)itemInfo->itemData);
			break;
		default:
			assert(0);
			break;
	}

	return bufferSize;
}

RsslRet encodeItemPost(RsslChannel* chnl, 
		ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslPostUserInfo *pPostUserInfo, 
		RsslUInt encodeStartTime)
{
	RsslRet ret;
	RsslPostMsg postMsg;
	RsslUpdateMsg updateMsg;
	RsslEncodeIterator encodeIter;

	assert(pPostUserInfo);

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	/* Prepare post message. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = itemInfo->StreamId;
	postMsg.flags = RSSL_PSMF_POST_COMPLETE;
	postMsg.postUserInfo = *pPostUserInfo;
	postMsg.msgBase.domainType = itemInfo->attributes.domainType;
	postMsg.msgBase.containerType = RSSL_DT_MSG;

	/* Prepare update message. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
	updateMsg.postUserInfo = *pPostUserInfo;
	updateMsg.msgBase.domainType = itemInfo->attributes.domainType;

	/* Encode post. */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&postMsg, 0)) < RSSL_RET_SUCCESS)
		return ret;

	/* Encode update. */
	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketPriceDataBody(&encodeIter, 
							getNextMarketPricePost((MarketPriceItem*)itemInfo->itemData),
							RSSL_MC_POST, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			updateMsg.msgBase.containerType = RSSL_DT_MAP;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketByOrderDataBody(&encodeIter, 
							getNextMarketByOrderPost((MarketByOrderItem*)itemInfo->itemData),
							RSSL_MC_POST, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

RsslRet createItemPost(RsslChannel* chnl, ItemInfo* itemInfo, RsslPostMsg* pPostMsg, RsslBuffer* msgBuf,
					   RsslPostUserInfo *pPostUserInfo, RsslUInt encodeStartTime)
{
	RsslRet ret;
	RsslUpdateMsg updateMsg;
	RsslEncodeIterator encodeIter;

	assert(pPostUserInfo);

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	/* Prepare post message. */
	rsslClearPostMsg(pPostMsg);
	pPostMsg->msgBase.streamId = itemInfo->StreamId;
	pPostMsg->flags = RSSL_PSMF_POST_COMPLETE;
	pPostMsg->postUserInfo = *pPostUserInfo;
	pPostMsg->msgBase.domainType = itemInfo->attributes.domainType;
	pPostMsg->msgBase.containerType = RSSL_DT_MSG;

	/* Prepare update message. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
	updateMsg.postUserInfo = *pPostUserInfo;
	updateMsg.msgBase.domainType = itemInfo->attributes.domainType;

	/* Encode update. */
	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketPriceDataBody(&encodeIter, 
							getNextMarketPricePost((MarketPriceItem*)itemInfo->itemData),
							RSSL_MC_POST, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			updateMsg.msgBase.containerType = RSSL_DT_MAP;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&updateMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketByOrderDataBody(&encodeIter, 
							getNextMarketByOrderPost((MarketByOrderItem*)itemInfo->itemData),
							RSSL_MC_POST, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	// set encodedDataBody on PostMsg
	pPostMsg->msgBase.encDataBody.data = msgBuf->data;
	pPostMsg->msgBase.encDataBody.length = msgBuf->length;

	return RSSL_RET_SUCCESS;
}

RsslUInt32 estimateItemGenMsgBufferLength(ItemInfo *itemInfo)
{
	RsslUInt32 bufferSize = 64;

	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			bufferSize += getNextMarketPriceGenMsgEstimatedContentLength(
					(MarketPriceItem*)itemInfo->itemData);
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			bufferSize += getNextMarketByOrderGenMsgEstimatedContentLength(
					(MarketByOrderItem*)itemInfo->itemData);
			break;
		default:
			assert(0);
			break;
	}

	return bufferSize;
}

RsslRet encodeItemGenMsg(RsslChannel* chnl, ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslUInt encodeStartTime)
{
	RsslRet ret;
	RsslGenericMsg genMsg;
	RsslEncodeIterator encodeIter;

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	/* Prepare generic message. */
	rsslClearGenericMsg(&genMsg);
	genMsg.msgBase.streamId = itemInfo->StreamId;
	genMsg.msgBase.domainType = itemInfo->attributes.domainType;

	/* Encode generic message. */
	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			genMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&genMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketPriceDataBody(&encodeIter, 
							getNextMarketPriceGenMsg((MarketPriceItem*)itemInfo->itemData),
							RSSL_MC_GENERIC, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			genMsg.msgBase.containerType = RSSL_DT_MAP;
			if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&genMsg, 0)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = encodeMarketByOrderDataBody(&encodeIter, 
							getNextMarketByOrderGenMsg((MarketByOrderItem*)itemInfo->itemData),
							RSSL_MC_GENERIC, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

RsslRet createItemGenMsg(RsslChannel* chnl, ItemInfo* itemInfo, RsslGenericMsg* pGenericMsg,
						 RsslBuffer* msgBuf, RsslUInt encodeStartTime)
{
	RsslRet ret;
	RsslEncodeIterator encodeIter;

	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;

	/* Prepare generic message. */
	rsslClearGenericMsg(pGenericMsg);
	pGenericMsg->msgBase.streamId = itemInfo->StreamId;
	pGenericMsg->msgBase.domainType = itemInfo->attributes.domainType;

	/* Encode generic message. */
	switch(itemInfo->attributes.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			pGenericMsg->msgBase.containerType = RSSL_DT_FIELD_LIST;
			if ((ret = encodeMarketPriceDataBody(&encodeIter, 
							getNextMarketPriceGenMsg((MarketPriceItem*)itemInfo->itemData),
							RSSL_MC_GENERIC, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			pGenericMsg->msgBase.containerType = RSSL_DT_MAP;
			if ((ret = encodeMarketByOrderDataBody(&encodeIter, 
							getNextMarketByOrderGenMsg((MarketByOrderItem*)itemInfo->itemData),
							RSSL_MC_GENERIC, encodeStartTime)) < RSSL_RET_SUCCESS)
				return ret;
			break;
		default:
			assert(0);
			break;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	// set encodedDataBody on GenericMsg
	pGenericMsg->msgBase.encDataBody.data = msgBuf->data;
	pGenericMsg->msgBase.encDataBody.length = msgBuf->length;

	return RSSL_RET_SUCCESS;
}

RsslRet encodeItemCloseStatus(RsslChannel* chnl, ItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[128];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = itemInfo->attributes.domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	snprintf(stateText, 128, "Stream closed" );
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

RsslRet encodeItemRequestReject(RsslChannel* chnl, RsslInt32 streamId, ItemRejectReason reason, RsslBuffer* msgBuf, RsslUInt8 domainType)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[128];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	switch (reason)
	{
	case ITEM_COUNT_REACHED:
		msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
		snprintf(stateText, 128, "Item request rejected for stream id %d - item count reached for this channel", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case INVALID_SERVICE_ID:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		snprintf(stateText, 128, "Item request rejected for stream id %d - service id invalid", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case QOS_NOT_SUPPORTED:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		snprintf(stateText, 128, "Item request rejected for stream id %d - QoS not supported", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case ITEM_ALREADY_OPENED:
		msg.state.code = RSSL_SC_ALREADY_OPEN;
		snprintf(stateText, 128, "Item request rejected for stream id %d - item already open with exact same key on another stream", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case STREAM_ALREADY_IN_USE:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		snprintf(stateText, 128, "Item request rejected for stream id %d - stream already in use with a different key", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case DOMAIN_NOT_SUPPORTED:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		snprintf(stateText, 128, "Item request rejected for stream id %d - this provider does not support domain %u", streamId, domainType);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	default:
		break;
	}

	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

