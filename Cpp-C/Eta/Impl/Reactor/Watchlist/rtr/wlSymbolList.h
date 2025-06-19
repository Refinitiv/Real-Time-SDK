/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef WL_SYMBOL_LIST_H
#define WL_SYMBOL_LIST_H

#include "rtr/wlItem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Represents a symbol list request. */
typedef struct WlSymbolListRequest
{
	WlItemRequest	itemBase;			/* Symbol list is based on item request and is aggregated
										 * and recovered the same way. */
	RsslBool		hasBehaviors;		/* SymbolListBehaviors were requested by the dataBody. */
	RsslUInt		flags;				/* RDMSymbolListDataStreamRequestFlags */
} WlSymbolListRequest;

/* Creates a symbol list request. */
RsslRet wlSymbolListRequestCreate(WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo);

RsslRet wlSymbolListRequestInit(WlSymbolListRequest *pSymbolListRequest, WlBase *pBase, 
		WlItems *pItems, WlItemRequestCreateOpts *pOpts, RsslBool hasSymbolListBehaviors,
		RsslUInt slDataStreamFlags, RsslErrorInfo *pErrorInfo);

/* Reissues a symbol list request. */
RsslRet wlSymbolListRequestReissue(WlBase *pBase, WlItems *pItems, WlSymbolListRequest *pExistingRequest,
		WlItemRequestCreateOpts *pCreateOpts, RsslErrorInfo *pErrorInfo);

/* Destroys a symbol list request. */
void wlSymbolListRequestDestroy(WlBase *pBase, WlItems *pItems, WlSymbolListRequest *pSymbolListRequest);

/* Processes a symbol list provider message. If data streams were requested, adds
 * any items indicated by the message. */
RsslRet wlProcessSymbolListMsg(WlBase *pBase, WlItems *pItems, RsslMsg *pRsslMsg,
		WlSymbolListRequest *pRequest, RsslErrorInfo *pErrorInfo);

#ifdef __cplusplus
}
#endif

#endif
