/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_ITEM_ENCODE_H
#define _RTR_RSSL_ITEM_ENCODE_H

#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ITEM_INFO_STRLEN 128

/* reasons an item request is rejected */
typedef enum {
	ITEM_COUNT_REACHED				= 1,
	INVALID_SERVICE_ID				= 2,
	ITEM_ALREADY_OPENED				= 3,
	STREAM_ALREADY_IN_USE			= 4,
	QOS_NOT_SUPPORTED				= 5,
	KEY_ENC_ATTRIB_NOT_SUPPORTED	= 6
} RsslItemRejectReason;



RsslRet encodeItemCloseStatus(RsslReactorChannel* chnl, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslInt32 streamId);
RsslRet encodeItemRequestReject(RsslReactorChannel* chnl, RsslInt32 streamId, RsslItemRejectReason reason, RsslBuffer* msgBuf, RsslUInt8 domain);

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
		default:
			return "Unknown reason";
	}
}


#ifdef __cplusplus
}
#endif

#endif

