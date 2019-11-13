/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include "rtr/rsslDataPackage.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/decoderTools.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rsslVersion.h"

RSSL_API RsslDataType rsslPrimitiveBaseType(RsslDataType dataType)
{
	return _rsslPrimitiveType(dataType);
}

RSSL_API void rsslQueryDataLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	RSSL_ASSERT(pVerInfo, Invalid parameters or parameters passed in as NULL);

	pVerInfo->productDate = rsslDeltaDate;
	pVerInfo->internalVersion = rsslVersion;
	pVerInfo->productVersion = rsslPackage;
}

RSSL_API RsslRet rsslFinishDecodeEntries(RsslDecodeIterator *pIter)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	_endOfList(pIter);
	return RSSL_RET_END_OF_CONTAINER;
}

RSSL_API RsslRet rsslEncodeNonRWFDataTypeInit(RsslEncodeIterator *pIter, RsslBuffer *pBuffer)
{
	RsslEncodingLevel *_levelInfo;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pBuffer, Invalid parameters or parameters passed in as NULL);

	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_OPAQUE,RSSL_EIS_NON_RWF_DATA,0, pIter->_curBufPtr);

	pBuffer->data = pIter->_curBufPtr;
	pBuffer->length = (rtrUInt32)(pIter->_endBufPtr - pIter->_curBufPtr);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeNonRWFDataTypeComplete(RsslEncodeIterator *pIter, RsslBuffer *pBuffer, RsslBool success)
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	/* Validations */
	RSSL_ASSERT(pBuffer && pBuffer->data, Invalid buffer);
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_OPAQUE, Invalid container type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_NON_RWF_DATA), Unexpected encoding attempted);			
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{
		/* verify no overrun */
		if (pIter->_curBufPtr != pBuffer->data)
			return RSSL_RET_INVALID_DATA;

		if (_rsslIteratorOverrun(pIter, pBuffer->length))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		pIter->_curBufPtr += pBuffer->length;
	}
	else
		pIter->_curBufPtr = _levelInfo->_containerStartPos;

	--pIter->_encodingLevel;
	
	return RSSL_RET_SUCCESS;
}

