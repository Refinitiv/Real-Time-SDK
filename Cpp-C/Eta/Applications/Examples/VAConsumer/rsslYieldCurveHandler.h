/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_YIELD_CURVE_HANDLER_H
#define _RTR_RSSL_YIELD_CURVE_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rsslConsumer.h"

/* yield curve item information */
typedef struct {
	char		itemname[MAX_ITEM_NAME_STRLEN+1];
	RsslState	itemState;
} YieldCurveItemInfo;

/* called by rsslConsumer to enable snapshot requesting for yield curve items */
void setYCSnapshotRequest();
/* called when it is time to send yield curve requests */
RsslRet sendYieldCurveItemRequests(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel);
/* called to process yield curve responses */
RsslRet processYieldCurveResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter);
/* called to close yield curve streams */
RsslRet closeYieldCurveItemStreams(RsslReactor *pReactor, ChannelCommand *pCommand);
/* called to redirect a request to a private stream */
static RsslRet redirectToPrivateStream(RsslReactor *pReactor, ChannelCommand *pCommand, ItemRequest *pRequest);
/* called to decode Vector */
static RsslRet decodeVector(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter);
/* called to decode Field List */
RsslRet decodeYieldCurveFieldList(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb);
/* called to decode Array */
static RsslRet decodeArray(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter);
/* called to decode Primitive */
static RsslRet decodePrimitive(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslDataType dataType, RsslBool isArray);
/* Decodes and prints the Yield Curve payload. */
RsslRet decodeYieldCurvePayload(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb);


#ifdef __cplusplus
};
#endif

#endif

