/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_VA_CACHE_HANDLER_H
#define _RTR_RSSL_VA_CACHE_HANDLER_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#include "rtr/rsslReactor.h"

#include "rtr/rsslPayloadCache.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	RsslBool				useCache;
	RsslPayloadCacheConfigOptions cacheOptions;
	RsslPayloadCacheHandle	cacheHandle;
	RsslPayloadCursorHandle cursorHandle;
	char					cacheDictionaryKey[128];
	RsslCacheError			cacheErrorInfo;
} RsslVACacheInfo;

RsslRet applyMsgBufferToCache(RsslUInt8 majorVersion, RsslUInt8 minorVersion, RsslPayloadEntryHandle* pCacheEntryHandle, RsslVACacheInfo* pCacheInfo, RsslBuffer *pBuffer);

RsslRet applyMsgToCache(RsslPayloadEntryHandle *pCacheEntryHandle, RsslVACacheInfo* pCacheInfo, RsslMsg *pMsg, RsslDecodeIterator *dIter);

RsslRet retrieveFromCache(RsslEncodeIterator *eIter, RsslPayloadEntryHandle cacheEntryHandle, RsslVACacheInfo *pCacheInfo);

#ifdef __cplusplus
};
#endif

#endif
