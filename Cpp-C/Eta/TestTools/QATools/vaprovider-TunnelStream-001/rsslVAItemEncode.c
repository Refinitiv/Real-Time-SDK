/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/



/*
 * This file contains functions that encode messages for item streams.
 * The functions here encode messages that follow a similar format among
 * the different data domains (e.g. market price, market by order).
 */

#include "rsslVAItemEncode.h"

/*
 * Encodes a close status for the item.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send close status message to
 * itemInfo - The item information
 * msgBuf - The message buffer to encode the market price close status into
 * streamId - The stream id of the market price close status
 */
RsslRet encodeItemCloseStatus(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_ITEM_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = itemInfo->domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	snprintf(stateText, 128, "Stream closed for item: %s", itemInfo->Itemname);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes a status message rejecting a request for an item.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 * msgBuf - The message buffer to encode the market price request reject into
 */
RsslRet encodeItemRequestReject(RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslItemRejectReason reason, RsslBuffer* msgBuf, RsslUInt8 domainType, RsslBool isPrivateStream)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_ITEM_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	if (isPrivateStream)
	{
		msg.flags |= RSSL_STMF_PRIVATE_STREAM;
	}
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	switch (reason)
	{
	case ITEM_COUNT_REACHED:
		msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
		sprintf(stateText, "Item request rejected for stream id %d - item count reached for this channel", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case INVALID_SERVICE_ID:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Item request rejected for stream id %d - service id invalid", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case QOS_NOT_SUPPORTED:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Item request rejected for stream id %d - QoS not supported", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case ITEM_ALREADY_OPENED:
		msg.state.code = RSSL_SC_ALREADY_OPEN;
		sprintf(stateText, "Item request rejected for stream id %d - item already open with exact same key on another stream", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case STREAM_ALREADY_IN_USE:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		sprintf(stateText, "Item request rejected for stream id %d - stream already in use with a different key", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case KEY_ENC_ATTRIB_NOT_SUPPORTED:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Item request rejected for stream id %d - this provider does not support key attribute information", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case PRIVATE_STREAM_REDIRECT:
		msg.flags |= RSSL_STMF_PRIVATE_STREAM;
		msg.state.code = RSSL_SC_NONE;
		msg.state.streamState = RSSL_STREAM_REDIRECTED;
		msg.state.dataState = RSSL_DATA_OK;
		sprintf(stateText, "Standard stream redirect to private for stream id %d - this item must be requested via private stream", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case PRIVATE_STREAM_MISMATCH:
		if (isPrivateStream)
			msg.flags |= RSSL_STMF_PRIVATE_STREAM;
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Rejected reissued request for stream id %d - private stream flag did not match previous request", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case BATCH_ITEM_REISSUE:
		if (isPrivateStream)
			msg.flags |= RSSL_STMF_PRIVATE_STREAM;
		msg.state.code = RSSL_SC_USAGE_ERROR;
		msg.state.streamState = RSSL_STREAM_CLOSED;
		sprintf(stateText, "Rejected request for stream id %d - reissue via batch request is not allowed", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case ITEM_NOT_SUPPORTED:
		msg.state.streamState = RSSL_STREAM_CLOSED;
		msg.state.code = RSSL_SC_USAGE_ERROR;
		sprintf(stateText, "Item request rejected for stream id %d - item not supported", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	default:
		break;
	}

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes a close status for the original batch stream (such as original batch request or orginal batch close).  
 * Returns success if encoding succeeds or failure if encoding fails.
 *
 * Case I) When a batch request is received, the original batch request stream is closed and
 * a new item request stream is opened for each item requested.
 * If there were any issues when processing the batch request, then the data state of the close status message 
 * to original batch stream is set to suspect.
 *
 * Case II) When a batch close is received, the original batch close stream is closed.
 *
 * pReactorChannel - The channel of the response and the channel to send close status for the orginal batch request or batch close stream to
 * domainType - the domain of the original batch message
 * msgBuf - The message buffer to encode the close status into for the orginal batch stream
 * streamId - The stream id of the close status for the original batch stream
 */
RsslRet encodeCloseStatusToBatch(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslUInt8 domainType, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt8 dataState, RsslUInt32 numOfItemsProcessed, RsslBatchType batchType)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_ITEM_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = dataState;
	msg.state.code = RSSL_SC_NONE;
	if (batchType == BATCH_REQUEST)
		snprintf(stateText, 128, "Processed %d items from Batch Request on domain %s", numOfItemsProcessed, rsslDomainTypeToString(domainType));
	else if (batchType == BATCH_CLOSE)
		snprintf(stateText, 128, "Processed %d items from Batch Close on domain %s", numOfItemsProcessed, rsslDomainTypeToString(domainType));
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/*encode message*/
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes acknowledgement message for a post message we received.
 *
 * chnl - The channel to send close status message to
 * ackBuf - The message buffer to encode the market price close status into
 * postMsg - the post message that we are acknowledging
 * nakCode - the nakCode to use in the acknowledgement
 * text - the text to put in the acknowledgement
 */
RsslRet encodeAck(RsslReactorChannel* chnl, RsslBuffer* ackBuf, RsslPostMsg *postMsg, RsslUInt8 nakCode, char *text)
{
	RsslRet ret = 0;
	RsslAckMsg ackMsg = RSSL_INIT_ACK_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	ackMsg.msgBase.msgClass = RSSL_MC_ACK;
	ackMsg.msgBase.streamId = postMsg->msgBase.streamId;
	ackMsg.msgBase.domainType = postMsg->msgBase.domainType;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.flags = RSSL_AKMF_NONE;
	ackMsg.nakCode = nakCode;
	ackMsg.ackId = postMsg->postId;
	ackMsg.seqNum = postMsg->seqNum;
	
	if (nakCode != RSSL_NAKC_NONE)
		ackMsg.flags |= RSSL_AKMF_HAS_NAK_CODE;

	if (postMsg->flags & RSSL_PSMF_HAS_SEQ_NUM)
		ackMsg.flags |= RSSL_AKMF_HAS_SEQ_NUM;

	if (text != NULL) 
	{
		ackMsg.flags |= RSSL_AKMF_HAS_TEXT;
		ackMsg.text.data = text;
		ackMsg.text.length = (RsslUInt32)strlen(text);
	}

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, ackBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&ackMsg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() of ackMsg failed with return code: %d\n", ret);
		return ret;
	}

	ackBuf->length = rsslGetEncodedBufferLength(&encodeIter);
	return RSSL_RET_SUCCESS;
}


