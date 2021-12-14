/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#include "postHandler.h"

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#else
#endif

static RsslUInt32 onStreamPostId, offStreamPostId;
static RsslUInt32 onStreamSeqNum, offStreamSeqNum;
static MarketPriceItem onStreamMPItemInfo;
static MarketPriceItem offStreamMPItemInfo;
static PostItemInfo onStreamItemInfo;
static PostItemInfo offStreamItemInfo;

//APIQA
static int multipartCount = 0;
static const int numMultipartMsgs = 9;
//End APIQA

#define RDNDISPLAY_FID 2
#define RDN_EXCHID_FID 4
#define DIVPAYDATE_FID 38
#define TRDPRC_1_FID 6
#define BID_FID 22
#define ASK_FID 25
#define ACVOL_1_FID 32
#define NETCHNG_1_FID 11
#define ASK_TIME_FID 267
#define PERATIO_FID 36
#define SALTIME_FID 379

/* Initializes post handler data. */
void postHandlerInit()
{
	onStreamPostId = 0;
	onStreamSeqNum = 0;

	offStreamPostId = 0;
	offStreamSeqNum = 0;

	initMPItemFields(&onStreamMPItemInfo);
	initMPItemFields(&offStreamMPItemInfo);

	onStreamItemInfo.IsRefreshComplete = RSSL_FALSE;
	onStreamItemInfo.itemData = &onStreamMPItemInfo;

	offStreamItemInfo.IsRefreshComplete = RSSL_FALSE;
	offStreamItemInfo.itemData = &offStreamMPItemInfo;

}

/* Initializes data to be sent in posts. */
void initMPItemFields(MarketPriceItem* mpItem)
{
	RsslBuffer tempBuffer;
	mpItem->RDNDISPLAY = 100;
	mpItem->RDN_EXCHID = 155;
	tempBuffer.data = (char *)"06/18/2013";
	tempBuffer.length = (RsslUInt32)strlen("06/18/2013");
	rsslDateStringToDate(&mpItem->DIVPAYDATE, &tempBuffer);
	mpItem->TRDPRC_1 = 1.00;
	mpItem->BID = 0.99;
	mpItem->ASK = 1.03;
	mpItem->ACVOL_1 = 100000;
	mpItem->NETCHNG_1 = 2.15;
	rsslDateTimeLocalTime(&mpItem->ASK_TIME);
	mpItem->PERATIO = 5.00;
	mpItem->SALTIME = mpItem->ASK_TIME;
	mpItem->SALTIME.time.second--;
}

// APIQA
static RsslRet sendOnStreamMultipartPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslBool postWithMsg);
static RsslRet sendOffStreamMultipartPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslBool postWithMsg);
static RsslRet encodeMultipartPostWithMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum,
	RsslBuffer *itemName, PostItemInfo* itemInfo);
// END APIQA

/* Sends a post on an existing stream. */
RsslRet sendOnStreamPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslBool postWithMsg)
{
	RsslUInt32 i;
	RsslInt32 streamId = -1;

	// APIQA
	if (watchlistConsumerConfig.enablePostMultipart)
	{
		RsslRet ret = sendOnStreamMultipartPostMsg(pReactor, pReactorChannel, postWithMsg);
		if (ret == RSSL_RET_SUCCESS)
			multipartCount++;
		return ret;
	}
	// END APIQA

	++onStreamPostId;
	++onStreamSeqNum;

	/* find a stream id of the first market price item */
	for (i = 0; i < watchlistConsumerConfig.itemCount; i++)
	{
		if (watchlistConsumerConfig.itemList[i].domainType == RSSL_DMT_MARKET_PRICE)
		{
			streamId = watchlistConsumerConfig.itemList[i].streamId;
			break;
		}
	}

	if (streamId == -1)
	{
		/* no items available to post on */
		printf("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");
		return RSSL_RET_SUCCESS;
	}

	if (postWithMsg)
		return encodePostWithMsg(pReactor, pReactorChannel, streamId, onStreamPostId, onStreamSeqNum, &watchlistConsumerConfig.itemList[i].name, &onStreamItemInfo);
	else
		return encodePostWithData(pReactor, pReactorChannel, streamId, onStreamPostId, onStreamSeqNum, &watchlistConsumerConfig.itemList[i].name, &onStreamItemInfo);
}

/* Sends a post on the login stream ("off-stream"). */
RsslRet sendOffStreamPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslBool postWithMsg)
{
	RsslBuffer nameBuf;

	// APIQA
	if (watchlistConsumerConfig.enablePostMultipart)
	{
		RsslRet ret = sendOffStreamMultipartPostMsg(pReactor, pReactorChannel, postWithMsg);
		if (ret == RSSL_RET_SUCCESS)
			multipartCount++;
		return ret;
	}
	// END APIQA

	++offStreamPostId;
	++offStreamSeqNum;

	rsslClearBuffer(&nameBuf);

	nameBuf.data = (char *)"OFFPOST";
	nameBuf.length = (RsslUInt32)strlen("OFFPOST");

	if (postWithMsg)
		return encodePostWithMsg(pReactor, pReactorChannel, LOGIN_STREAM_ID, offStreamPostId, offStreamSeqNum, &nameBuf, &offStreamItemInfo);
	else
		return encodePostWithData(pReactor, pReactorChannel, LOGIN_STREAM_ID, offStreamPostId, offStreamSeqNum, &nameBuf, &offStreamItemInfo);
}

/* Encodes a post with a nested message as its payload. */
static RsslRet encodePostWithMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum, 
		RsslBuffer *itemName, PostItemInfo* itemInfo)
{
	RsslPostMsg postMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslEncodeIterator encodeIter;
	RsslBuffer payloadMsgBuf;
	RsslBuffer buff;
	RsslBuffer hostName;
	char hostNameBuf[256];
	char msgBuf[256];

	buff.data = msgBuf;
	buff.length = 255;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = (RsslMsg*)&postMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;

	/* Send a post message. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_MSG;
	postMsg.msgBase.encDataBody.data = buff.data;
	postMsg.msgBase.encDataBody.length = buff.length;

	// Note: post message key not required for on-stream post
	postMsg.flags = RSSL_PSMF_POST_COMPLETE 
		| RSSL_PSMF_ACK // request ACK
		| RSSL_PSMF_HAS_POST_ID
		| RSSL_PSMF_HAS_SEQ_NUM
		| RSSL_PSMF_HAS_POST_USER_RIGHTS
	 	| RSSL_PSMF_HAS_MSG_KEY;

	postMsg.postId = postId;
	postMsg.seqNum = seqNum;

	postMsg.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
					rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
			return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();

	postMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;

	rsslClearEncodeIterator(&encodeIter);

	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, &buff)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	/* although we are encoding an RWF type inside this message, 
	   this function is used to extract a buffer to encode into.  
	   This is done because the encodeMarketPriceResp is shared by 
	   provider applications so it expects to encode the message into 
	   a stand alone buffer.  This could have been pre-encoded as well */

	rsslClearBuffer(&payloadMsgBuf);

	ret = rsslEncodeNonRWFDataTypeInit(&encodeIter, &payloadMsgBuf);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret =  encodeMarketPriceResp(pReactorChannel, &payloadMsgBuf, streamId, itemName, itemInfo);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: encodeMarketPriceResp() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret = rsslEncodeNonRWFDataTypeComplete(&encodeIter, &payloadMsgBuf, RSSL_TRUE);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}

	postMsg.msgBase.encDataBody.length = rsslGetEncodedBufferLength(&encodeIter);

	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	printf ("\nSENDING POST WITH MESSAGE:\n");
	printf ("  streamId: %d\n", postMsg.msgBase.streamId);
	printf ("  postId:   %d\n", postMsg.postId);
	printf ("  seqNum:   %d\n\n", postMsg.seqNum);

	return RSSL_RET_SUCCESS;
}

/* Encodes a MarketPrice response. */
RsslRet encodeMarketPriceResp(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf,  RsslInt32 streamId, RsslBuffer *itemName, PostItemInfo* itemInfo)
{
	RsslRet ret;
	RsslRefreshMsg refreshMsg;
	RsslUpdateMsg updateMsg;
	RsslMsgBase* msgBase;
	RsslMsg* msg;
	RsslFieldList fList;
	RsslFieldEntry fEntry;
	char stateText[256];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslReal tempReal;
	RsslEncodeIterator encodeIter;

	MarketPriceItem* mpItem = itemInfo->itemData;

	mpItem->TRDPRC_1 += 0.1;
	mpItem->BID += 0.1;
	mpItem->ASK += 0.1;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	/* set message depending on whether refresh or update */
	if (!itemInfo->IsRefreshComplete) /* this is a refresh message */
	{
		rsslClearRefreshMsg(&refreshMsg);
		msgBase = &refreshMsg.msgBase;
		msgBase->msgClass = RSSL_MC_REFRESH;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		sprintf(stateText, "Item Refresh Completed");
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
		msgBase->msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
		/* ItemName */
		msgBase->msgKey.name.data = itemName->data;
		msgBase->msgKey.name.length = itemName->length;
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		/* Qos */
		refreshMsg.qos.dynamic = RSSL_FALSE;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		msg = (RsslMsg *)&refreshMsg;
	}
	else /* this is an update message */
	{
		rsslClearUpdateMsg(&updateMsg);
		msgBase = &updateMsg.msgBase;
		msgBase->msgClass = RSSL_MC_UPDATE;
		/* include msg key in updates for non-interactive provider streams */
		if (streamId < 0)
		{
			updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
			msgBase->msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
			/* ItemName */
			msgBase->msgKey.name.data = itemName->data;
			msgBase->msgKey.name.length = itemName->length;
			msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		}
		msg = (RsslMsg *)&updateMsg;
	}

	msgBase->domainType = RSSL_DMT_MARKET_PRICE;
	msgBase->containerType = RSSL_DT_FIELD_LIST;

	/* StreamId */
	msgBase->streamId = streamId;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode field list */
	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode fields */
	/* if refresh, encode refresh fields */
	if (!itemInfo->IsRefreshComplete)
	{
		/* RDNDISPLAY */
		rsslClearFieldEntry(&fEntry);
		fEntry.fieldId = RDNDISPLAY_FID;
		fEntry.dataType = RSSL_DT_UINT;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&mpItem->RDNDISPLAY)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
		/* RDN_EXCHID */
		rsslClearFieldEntry(&fEntry);
		fEntry.fieldId = RDN_EXCHID_FID;
		fEntry.dataType = RSSL_DT_ENUM;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&mpItem->RDN_EXCHID)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
		/* DIVPAYDATE */
		rsslClearFieldEntry(&fEntry);
		fEntry.fieldId = DIVPAYDATE_FID;
		fEntry.dataType = RSSL_DT_DATE;
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&mpItem->DIVPAYDATE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}

		// APIQA:
		if ((multipartCount == (numMultipartMsgs - 1)) && !itemInfo->IsRefreshComplete)
		{
			itemInfo->IsRefreshComplete = RSSL_TRUE;
		}
		// END APIQA:
	}
	/* TRDPRC_1 */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = TRDPRC_1_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->TRDPRC_1, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* BID */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = BID_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->BID, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ASK */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = ASK_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->ASK, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ACVOL_1 */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = ACVOL_1_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->ACVOL_1, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* NETCHNG_1 */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = NETCHNG_1_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->NETCHNG_1, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ASK_TIME */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = ASK_TIME_FID;
	fEntry.dataType = RSSL_DT_TIME;
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&mpItem->ASK_TIME.time)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Encodes a post with a field list payload. */
static RsslRet encodePostWithData(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum, RsslBuffer *itemName, 
		PostItemInfo* itemInfo)
{
	RsslPostMsg postMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslEncodeIterator encodeIter;

	RsslFieldList fList;
	RsslFieldEntry fEntry;
	RsslReal tempReal;
	RsslBuffer hostName;
	char hostNameBuf[256];
	char msgBuf[256];
	RsslBuffer buff;

	MarketPriceItem* mpItem = itemInfo->itemData;

	buff.data = msgBuf;
	buff.length = 255;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = (RsslMsg*)&postMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;

	/* Send a post message. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;

	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.encDataBody.data = buff.data;
	postMsg.msgBase.encDataBody.length = buff.length;

	/* set-up message */
	postMsg.msgBase.msgClass = RSSL_MC_POST;
	postMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

	/* Note: post message key not required for on-stream post */
	postMsg.flags = RSSL_PSMF_POST_COMPLETE 
		| RSSL_PSMF_ACK // request ACK
		| RSSL_PSMF_HAS_POST_ID
		| RSSL_PSMF_HAS_SEQ_NUM
	 	| RSSL_PSMF_HAS_MSG_KEY;

	postMsg.postId = postId;
	postMsg.seqNum = seqNum;

	mpItem->TRDPRC_1 += 0.1;
	mpItem->BID += 0.1;
	mpItem->ASK += 0.1;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
				rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
		return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();

	postMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;

	rsslClearEncodeIterator(&encodeIter);

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, &buff)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	/* encode field list */
	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* TRDPRC_1 */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = TRDPRC_1_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->TRDPRC_1, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* BID */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = BID_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->BID, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ASK */
	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = ASK_FID;
	fEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&tempReal);
	rsslDoubleToReal(&tempReal, &mpItem->ASK, RSSL_RH_EXPONENT_2);
	if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	postMsg.msgBase.encDataBody.length = rsslGetEncodedBufferLength(&encodeIter);

	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	printf ("\nSENDING POST WITH DATA:\n");
	printf ("  streamId: %d\n", postMsg.msgBase.streamId);
	printf ("  postId:   %d\n", postMsg.postId);
	printf ("  seqNum:   %d\n\n", postMsg.seqNum);

	return RSSL_RET_SUCCESS;
}

// APIQA
static RsslRet sendOnStreamMultipartPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslBool postWithMsg)
{
	RsslUInt32 i;
	RsslInt32 streamId = -1;

	++onStreamPostId;
	++onStreamSeqNum;

	/* find a stream id of the first market price item */
	for (i = 0; i < watchlistConsumerConfig.itemCount; i++)
	{
		if (watchlistConsumerConfig.itemList[i].domainType == RSSL_DMT_MARKET_PRICE)
		{
			streamId = watchlistConsumerConfig.itemList[i].streamId;
			break;
		}
	}

	if (streamId == -1)
	{
		/* no items available to post on */
		printf("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");
		return RSSL_RET_SUCCESS;
	}

	return encodeMultipartPostWithMsg(pReactor, pReactorChannel, streamId, onStreamPostId, onStreamSeqNum, &watchlistConsumerConfig.itemList[i].name, &onStreamItemInfo);
}

/* Sends a post on the login stream ("off-stream"). */
static RsslRet sendOffStreamMultipartPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslBool postWithMsg)
{
	RsslBuffer nameBuf;

	++offStreamPostId;
	++offStreamSeqNum;

	rsslClearBuffer(&nameBuf);

	nameBuf.data = (char *)"OFFPOST";
	nameBuf.length = (RsslUInt32)strlen("OFFPOST");

	return encodeMultipartPostWithMsg(pReactor, pReactorChannel, LOGIN_STREAM_ID, offStreamPostId, offStreamSeqNum, &nameBuf, &offStreamItemInfo);
}

/* Encodes a post with a nested message as its payload. */
static RsslRet encodeMultipartPostWithMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel,
	RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum,
	RsslBuffer *itemName, PostItemInfo* itemInfo)
{
	RsslPostMsg postMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslEncodeIterator encodeIter;
	RsslBuffer payloadMsgBuf;
	RsslBuffer buff;
	RsslBuffer hostName;
	char hostNameBuf[256];
	char msgBuf[256];

	buff.data = msgBuf;
	buff.length = 255;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = (RsslMsg*)&postMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;

	/* Send a post message. */
	rsslClearPostMsg(&postMsg);
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_MSG;
	postMsg.msgBase.encDataBody.data = buff.data;
	postMsg.msgBase.encDataBody.length = buff.length;

	// Note: post message key not required for on-stream post
	// APIQA VAS Comment: If sending multiple parts, don't send post complete and increment part number
	postMsg.flags =
		  RSSL_PSMF_ACK // request ACK
		| RSSL_PSMF_HAS_POST_ID
		| RSSL_PSMF_HAS_SEQ_NUM
		| RSSL_PSMF_HAS_POST_USER_RIGHTS
		| RSSL_PSMF_HAS_MSG_KEY;

	postMsg.postId = postId;
	postMsg.seqNum = seqNum;

	// APIQA VAS Comment: If sending multiple parts, don't send post complete and increment part number
	if ((multipartCount < numMultipartMsgs) && !itemInfo->IsRefreshComplete)
	{
		postMsg.flags |= RSSL_PSMF_HAS_PART_NUM;
		postMsg.partNum = multipartCount;
	}
	if ((multipartCount == (numMultipartMsgs - 1)) && !itemInfo->IsRefreshComplete)
	{
		postMsg.flags |= RSSL_PSMF_POST_COMPLETE;
	}

	postMsg.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
			rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
		return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();

	postMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;

	rsslClearEncodeIterator(&encodeIter);

	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, &buff)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

	/* although we are encoding an RWF type inside this message,
	   this function is used to extract a buffer to encode into.
	   This is done because the encodeMarketPriceResp is shared by
	   provider applications so it expects to encode the message into
	   a stand alone buffer.  This could have been pre-encoded as well */

	rsslClearBuffer(&payloadMsgBuf);

	ret = rsslEncodeNonRWFDataTypeInit(&encodeIter, &payloadMsgBuf);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret = encodeMarketPriceResp(pReactorChannel, &payloadMsgBuf, streamId, itemName, itemInfo);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: encodeMarketPriceResp() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret = rsslEncodeNonRWFDataTypeComplete(&encodeIter, &payloadMsgBuf, RSSL_TRUE);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}

	postMsg.msgBase.encDataBody.length = rsslGetEncodedBufferLength(&encodeIter);

	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
		&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
			ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	printf("\nSENDING MULTIPLART POST WITH MESSAGE:\n");
	printf("  streamId: %d\n", postMsg.msgBase.streamId);
	printf("  postId:   %d\n", postMsg.postId);
	printf("  seqNum:   %d\n\n", postMsg.seqNum);

	return RSSL_RET_SUCCESS;
}
// END OF APIQA
