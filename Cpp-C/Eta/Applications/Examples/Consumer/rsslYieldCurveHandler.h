/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_YIELD_CURVE_HANDLER_H
#define _RTR_RSSL_YIELD_CURVE_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* yield curve item information */
typedef struct {
	char		itemname[MAX_ITEM_NAME_STRLEN+1];
	RsslUInt32	nameLength;
	RsslInt32	streamId;
	RsslState	itemState;
} YieldCurveItemInfo;

/* called by rsslConsumer to enable snapshot requesting for yield curve items */
void setYCSnapshotRequest();
/* called from rsslConsumer to add a yield curve item */
void addYieldCurveItemName(char* itemname, RsslBool isPrivateStream);
/* called when it is time to send yield curve requests */
RsslRet sendYieldCurveItemRequests(RsslChannel* chnl);
/* called to process yield curve responses */
RsslRet processYieldCurveResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* called to close yield curve streams */
RsslRet closeYieldCurveItemStreams(RsslChannel* chnl);
/* called to redirect a request to a private stream */
RsslRet redirectToYieldCurvePrivateStream(RsslChannel* chnl, RsslInt32 streamId);
/* called to decode Vector */
static RsslRet decodeVector(RsslDecodeIterator *dIter);
/* called to decode Field List */
static RsslRet decodeFieldList(RsslDecodeIterator *dIter, RsslLocalFieldSetDefDb *fieldSetDefDb);
/* called to decode Array */
static RsslRet decodeArray(RsslDecodeIterator *dIter);
/* called to decode Primitive */
static RsslRet decodePrimitive(RsslDecodeIterator *dIter, RsslDataType dataType, RsslBool isArray);


#ifdef __cplusplus
};
#endif

#endif

