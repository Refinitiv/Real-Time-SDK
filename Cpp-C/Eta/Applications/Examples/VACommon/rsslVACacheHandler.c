/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the cache handler for the rsslVAProvider application.
 */

#include "rsslVACacheHandler.h"

#include "rtr/rsslPayloadEntry.h"


/*
 * Apply buffer containing full encoded message to cache
 */
RsslRet applyMsgBufferToCache(RsslUInt8 majorVersion, RsslUInt8 minorVersion, RsslPayloadEntryHandle* pCacheEntryHandle, RsslVACacheInfo* pCacheInfo, RsslBuffer *pBuffer)
{
	RsslDecodeIterator dIter;
	RsslMsg msg;
	RsslRet ret;

	if (*pCacheEntryHandle == 0)
	{
		/* allocate cache entry */
		*pCacheEntryHandle = rsslPayloadEntryCreate(pCacheInfo->cacheHandle, &pCacheInfo->cacheErrorInfo);
		if (*pCacheEntryHandle == 0)
		{
			printf("Failed to create cache entry.\n\tError (%d): %s\n",
					pCacheInfo->cacheErrorInfo.rsslErrorId, pCacheInfo->cacheErrorInfo.text);

			return RSSL_RET_FAILURE;
		}
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, majorVersion, minorVersion);

	if((ret = rsslSetDecodeIteratorBuffer(&dIter, pBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("Failed to set iterator on data buffer: Error (%d)", ret);
		return ret;
	}

	rsslClearMsg(&msg);
	if ((ret = rsslDecodeMsg(&dIter, &msg)) != RSSL_RET_SUCCESS)
	{
		printf("Failed to decode message: Error (%d)", ret);
		return ret;
	}

	if ((ret = rsslPayloadEntryApply(*pCacheEntryHandle,
			&dIter,
			&msg,
			&pCacheInfo->cacheErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("Failed to apply payload data to cache.\n\tError (%d): %s\n",
				pCacheInfo->cacheErrorInfo.rsslErrorId, pCacheInfo->cacheErrorInfo.text);

		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Apply partially decoded message to cache
 */
RsslRet applyMsgToCache(RsslPayloadEntryHandle *pCacheEntryHandle, RsslVACacheInfo* pCacheInfo, RsslMsg *pMsg, RsslDecodeIterator *dIter)
{
	RsslRet ret;
	RsslDecodeIterator cacheDecodeIter;

	if (*pCacheEntryHandle == 0)
	{
		/* allocate cache entry */
		*pCacheEntryHandle = rsslPayloadEntryCreate(pCacheInfo->cacheHandle, &pCacheInfo->cacheErrorInfo);
		if (*pCacheEntryHandle == 0)
		{
			printf("Failed to create cache entry.\n\tError (%d): %s\n",
					pCacheInfo->cacheErrorInfo.rsslErrorId, pCacheInfo->cacheErrorInfo.text);

			return RSSL_RET_FAILURE;
		}
	}

	rsslClearDecodeIterator(&cacheDecodeIter);
	rsslSetDecodeIteratorRWFVersion(&cacheDecodeIter, dIter->_majorVersion, dIter->_minorVersion);
	rsslSetDecodeIteratorBuffer(&cacheDecodeIter, &pMsg->msgBase.encDataBody);

	if ((ret = rsslPayloadEntryApply(*pCacheEntryHandle,
			&cacheDecodeIter,
			pMsg,
			&pCacheInfo->cacheErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("Failed to apply payload data to cache.\n\tError (%d): %s\n",
				pCacheInfo->cacheErrorInfo.rsslErrorId, pCacheInfo->cacheErrorInfo.text);

		return ret;
	}


	return RSSL_RET_SUCCESS;
}

/*
 * Retrieval from cache
 */
RsslRet retrieveFromCache(RsslEncodeIterator *eIter, RsslPayloadEntryHandle cacheEntryHandle, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;

	if (cacheEntryHandle)
	{
		// retrieve payload from cache
		rsslCacheErrorClear(&pCacheInfo->cacheErrorInfo);

		ret = rsslPayloadEntryRetrieve(cacheEntryHandle,
				eIter,
				pCacheInfo->cursorHandle,
				&pCacheInfo->cacheErrorInfo);

		return ret;
	}
	else
	{
		printf("Error: no cache entry handle\n");
		return RSSL_RET_FAILURE;
	}

}
