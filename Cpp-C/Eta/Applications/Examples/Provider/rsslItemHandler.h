

#ifndef _RTR_RSSL_ITEM_HANDLER_H
#define _RTR_RSSL_ITEM_HANDLER_H

#include "rsslProvider.h"
#include "rsslItemEncode.h"
#include "rsslMarketByOrderItems.h"
#include "rsslMarketByPriceItems.h"
#include "rsslMarketPriceItems.h"
#include "rsslSymbolListItems.h"

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
	RsslChannel*	Chnl;
	RsslBool		IsInUse;
	RsslUInt8		domainType;
} RsslItemRequestInfo;

void initItemHandler();
static RsslItemRequestInfo* getMatchingItemReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason);
static RsslRet getNewItemReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason, RsslItemRequestInfo** newItemRequestInfo);
static RsslRet freeItemReqInfo(RsslItemRequestInfo* itemReqInfo);
static RsslRet getItemInfo(RsslItemRequestInfo* itemReqInfo, RsslUInt8 domainType, RsslItemInfo** itemInfo);
static void freeItemInfo(RsslItemInfo* itemInfo);
void updateItemInfo();
RsslRet sendSLItemUpdates(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo, RsslUInt8 responseType, RsslInt32 streamId, RsslBool isStreaming);
static RsslRet closeItemStream(RsslChannel *chnl, RsslInt32 streamId);
void closeItemChnlStreams(RsslChannel* chnl);
RsslRet processItemRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet sendItemResponse(RsslItemRequestInfo* itemReqInfo);
static RsslRet sendMBPItemResponse(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo);
RsslRet sendItemUpdates(RsslChannel* chnl);
void sendItemCloseStatusMsgs(RsslChannel* chnl);
static RsslRet sendItemCloseStatusMsg(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo);
static RsslRet sendItemRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslUInt8 domainType, RsslItemRejectReason reason, RsslBool isPrivateStream);
static RsslBool isItemCountReached(RsslChannel* chnl);
static RsslBool isStreamInUse(RsslChannel* chnl, RsslInt32 streamId, RsslMsgKey* key);
/* Function for adding new items to the symbol list */
RsslRet addSymbolListItem(RsslItemRequestInfo* itemReqInfo);
/* Function for deleting items from the symbol list */
RsslRet deleteSymbolListItem(RsslItemRequestInfo* itemReqInfo);

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

