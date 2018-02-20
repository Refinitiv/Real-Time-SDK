/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslArray.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"


RSSL_API RsslRet rsslDecodeArray( 
					    RsslDecodeIterator	*oIter,
						RsslArray			*oArray )
{
	RsslDecodingLevel *_levelInfo;
	char	*_endBufPtr;

	RSSL_ASSERT(oArray && oIter, Invalid parameters or parameters passed in as NULL);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_ARRAY, oArray); 

	_endBufPtr = _levelInfo->_endBufPtr;

	if (oIter->_curBufPtr == _endBufPtr)
	{
		_endOfList(oIter);
		return RSSL_RET_BLANK_DATA;
	}
	else if (_endBufPtr - oIter->_curBufPtr < 3)
		return RSSL_RET_INCOMPLETE_DATA;

	/* extract data type */
	oIter->_curBufPtr += rwfGet8(oArray->primitiveType, oIter->_curBufPtr);
	/* extract itemLength */
	oIter->_curBufPtr += rwfGetOptByteU16((&oArray->itemLength), oIter->_curBufPtr);
	/* extract count */
	oIter->_curBufPtr += rwfGet16(_levelInfo->_itemCount, oIter->_curBufPtr);

	oArray->encData.data = _levelInfo->_nextEntryPtr = oIter->_curBufPtr;
	oArray->encData.length = (rtrUInt32)(_endBufPtr - oIter->_curBufPtr);

	/* handle legacy types */
	oArray->primitiveType = _rsslPrimitiveType(oArray->primitiveType);

	/* Check for buffer overflow. Post check ok since no copy */
	if (oIter->_curBufPtr > _endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeArrayEntry(
							RsslDecodeIterator	*iIter,
							RsslBuffer	*oBuffer )
{
	RsslRet ret;
	RsslArray *array;
	RsslDecodingLevel *_levelInfo;
	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);
	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	RSSL_ASSERT(oBuffer && iIter, Invalid parameters or parameters passed in as NULL);

	iIter->_curBufPtr = _levelInfo->_nextEntryPtr;

	/* For an array, since the entries are always primitives(not containers), no skip logic(_nextEntryPtr) needs to be applied. */
	
	array = (RsslArray*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	if (array->itemLength)
	{
		oBuffer->length = array->itemLength;
		oBuffer->data = iIter->_curBufPtr;
		iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = _levelInfo->_nextEntryPtr += array->itemLength;
	}
	else
	{
		if ((ret = _rsslDecodeItem(iIter, array->primitiveType, oBuffer)) < 0)
			return(ret);
		iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = _levelInfo->_nextEntryPtr;
	}
	
	if (iIter->_curBufPtr > _levelInfo->_endBufPtr)
		return(RSSL_RET_INCOMPLETE_DATA);

	/* shift iterator */
	_levelInfo->_nextItemPosition++;

	return RSSL_RET_SUCCESS;
}

	




	
