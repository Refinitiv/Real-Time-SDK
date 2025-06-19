/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */



#ifndef _RTR_RSSL_ITEM_ENCODE_H
#define _RTR_RSSL_ITEM_ENCODE_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ITEM_INFO_STRLEN 128

/* reasons an item request is rejected */
typedef enum {
	ITEM_REJECT_NONE				= 0,
	ITEM_COUNT_REACHED				= 1,
	INVALID_SERVICE_ID				= 2,
	ITEM_ALREADY_OPENED				= 3,
	STREAM_ALREADY_IN_USE			= 4,
	QOS_NOT_SUPPORTED				= 5,
	KEY_ENC_ATTRIB_NOT_SUPPORTED	= 6,
	PRIVATE_STREAM_REDIRECT			= 7,
	PRIVATE_STREAM_MISMATCH			= 8,
	BATCH_ITEM_REISSUE				= 9,
	ITEM_NOT_SUPPORTED              = 10,
	REQUEST_DECODE_ERROR			= 11
} RsslItemRejectReason;

/* batch type */
typedef enum {
	BATCH_REQUEST					= 0,
	BATCH_REISSUE					= 1,
	BATCH_CLOSE						= 2
} RsslBatchType;

/* item information. */
typedef struct {
	RsslBool		IsRefreshComplete;
	char			Itemname[MAX_ITEM_INFO_STRLEN];
	RsslUInt32		InterestCount;
	RsslUInt8		domainType;
	void*			itemData; /* Holds information about the item's data. This data will be different depending on the domain of the item. */
} RsslItemInfo;

RsslRet encodeAck(RsslChannel* chnl, RsslBuffer* ackBuf, RsslPostMsg *postMsg, RsslUInt8 nakCode, char *text);
RsslRet encodeItemCloseStatus(RsslChannel* chnl, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId);
RsslRet encodeCloseStatusToBatch(RsslChannel* chnl, RsslUInt8 domainType, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt8 dataState, RsslUInt32 numOfItemsProcessed, RsslBatchType batchType);
RsslRet encodeItemRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslItemRejectReason reason, RsslBuffer* msgBuf, RsslUInt8 domain, RsslBool isPrivateStream);

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE void clearItemInfo(RsslItemInfo* itemInfo)
{
	itemInfo->IsRefreshComplete = RSSL_FALSE;
	itemInfo->Itemname[0] = '\0';
	itemInfo->InterestCount = 0;
	itemInfo->itemData = 0;
	itemInfo->domainType = 0;
}

/*
 * Clears the item information.
 * itemInfo - The item information to be cleared
 */
RTR_C_INLINE const char* itemRejectReasonToString(RsslItemRejectReason rejectReason)
{
	switch (rejectReason)
	{
		case ITEM_COUNT_REACHED:
			return "ITEM_COUNT_REACHED";
		break;
		case INVALID_SERVICE_ID:
			return "INVALID_SERVICE_ID";
		break;
		case ITEM_ALREADY_OPENED:
			return "ITEM_ALREADY_OPENED";
		break;
		case STREAM_ALREADY_IN_USE:
			return "STREAM_ALREADY_IN_USE";
		break;
		case QOS_NOT_SUPPORTED:
			return "QOS_NOT_SUPPORTED";
		break;
		case KEY_ENC_ATTRIB_NOT_SUPPORTED:
			return "KEY_ENC_ATTRIB_NOT_SUPPORTED";
		break;
		case PRIVATE_STREAM_REDIRECT:
			return "PRIVATE_STREAM_REDIRECT";
		break;
		case PRIVATE_STREAM_MISMATCH:
			return "PRIVATE_STREAM_MISMATCH";
		break;
		case BATCH_ITEM_REISSUE:
			return "BATCH_ITEM_REISSUE";
		break;
		case ITEM_NOT_SUPPORTED:
			return "ITEM_NOT_SUPPORTED";
		break;
		case REQUEST_DECODE_ERROR:
			return "REQUEST_DECODE_ERROR";
		break;
		default:
			return "Unknown reason";
	}
}


#ifdef __cplusplus
}
#endif

#endif


