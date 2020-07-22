/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
*/



#ifndef _RTR_RSSL_ITEM_HANDLER_H
#define _RTR_RSSL_ITEM_HANDLER_H

#include "rsslProvider.h"
#include "rsslVAItemEncode.h"
#include "rsslVAMarketByOrderItems.h"
#include "rsslVAMarketPriceItems.h"
#include "rsslVASymbolListItems.h"
#include "rsslVAMarketByPriceItems.h"

#ifdef __cplusplus
extern "C" {
#endif

static const RsslBuffer RSSL_SPECIAL_PRIVATE_STREAM_ITEM = { 6 , (char*)"RES-DS" };

/* item request information */
typedef struct {
	RsslInt32		StreamId;
	char			Itemname[MAX_ITEM_INFO_STRLEN+1];
	RsslItemInfo*   ItemInfo;
	RsslBool		IsStreamingRequest;
	RsslBool		IsPrivateStreamRequest;
	RsslBool		IncludeKeyInUpdates;
	RsslMsgKey		MsgKey;
	RsslReactorChannel*	Chnl;
	RsslBool		IsInUse;
	RsslUInt8		domainType;
} RsslItemRequestInfo;

void initItemHandler();
static RsslItemRequestInfo* getMatchingItemReqInfo(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason);
static RsslRet getNewItemReqInfo(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason, RsslItemRequestInfo** newItemRequestInfo);
static RsslRet freeItemReqInfo(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo);
static RsslRet getItemInfo(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslItemRequestInfo* itemReqInfo);
static void freeItemInfo(RsslItemInfo* itemInfo);
void updateItemInfo();
RsslRet sendSLItemUpdates(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslItemRequestInfo* itemReqInfo, RsslUInt8 responseType, RsslInt32 streamId, RsslBool isStreaming);
static RsslRet closeItemStream(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslInt32 streamId);
void closeItemChnlStreams(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel);
RsslRet processItemRequest(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet sendItemResponse(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo);
static RsslRet sendMBPItemResponse(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo);
RsslRet sendItemUpdates(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel);
void sendItemCloseStatusMsgs(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel);
static RsslRet sendItemCloseStatusMsg(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslItemRequestInfo* itemReqInfo);
static RsslRet sendItemRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslUInt8 domainType, RsslItemRejectReason reason, RsslBool isPrivateStream);
static RsslBool isItemCountReached(RsslReactorChannel* pReactorChannel);
static RsslBool isStreamInUse(RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslMsgKey* key);
RsslRet processSingleItemRequest(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* key, RsslBool isPrivateStream);
/* Function for adding new items to the symbol list */
RsslRet addSymbolListItem(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslItemRequestInfo* itemReqInfo);
/* Function for deleting items from the symbol list */
RsslRet deleteSymbolListItem(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo);

// APIQA
RsslRet sendGenericMessageLogin(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 sId);
RsslRet sendGenericMessageSource(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 sId);
RsslRet sendGenericMessageMP(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 sId);
// APIQA


/*
 * Clears the item request information.
 * itemReqInfo - The item request information to be cleared
 */
RTR_C_INLINE void clearItemReqInfo(RsslItemRequestInfo* itemReqInfo)
{
	itemReqInfo->StreamId = 0;
	itemReqInfo->Itemname[0] = '\0';
	itemReqInfo->ItemInfo = 0;
	itemReqInfo->IsStreamingRequest = RSSL_FALSE;
	itemReqInfo->IsPrivateStreamRequest = RSSL_FALSE;
	itemReqInfo->IncludeKeyInUpdates = RSSL_FALSE;
	rsslClearMsgKey(&itemReqInfo->MsgKey);
	itemReqInfo->MsgKey.name.data = itemReqInfo->Itemname;
	itemReqInfo->MsgKey.name.length = MAX_ITEM_INFO_STRLEN;
	itemReqInfo->Chnl = 0;
	itemReqInfo->IsInUse = RSSL_FALSE;
	itemReqInfo->domainType = 0;
}

#ifdef __cplusplus
}
#endif

#endif
