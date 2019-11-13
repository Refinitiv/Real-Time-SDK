/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslMap.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rtr/rsslPrimitiveDecoders.h"

RSSL_API RsslRet rsslDecodeMap(
				RsslDecodeIterator	*oIter,
				RsslMap				*oMap )
{
	char		*position;
	char		*_endBufPtr;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(oMap && oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_MAP, oMap); 

	_endBufPtr = _levelInfo->_endBufPtr;
	position = oIter->_curBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}
	else if (_endBufPtr - position < 5)
		return RSSL_RET_INCOMPLETE_DATA;

	/* extract flags, keyDataFormat and _containerType */
	position += rwfGet8(oMap->flags, position);
	position += rwfGet8(oMap->keyPrimitiveType, position);
	position += rwfGet8(oMap->containerType, position);
	/* container type needs to be scaled back */
	oMap->containerType += RSSL_DT_CONTAINER_TYPE_MIN;
	
	/* Handle legacy conversion */
	oMap->keyPrimitiveType = _rsslPrimitiveType(oMap->keyPrimitiveType);

	if (rsslMapCheckHasKeyFieldId(oMap))
	{
		position += rwfGet16(oMap->keyFieldId, position);

		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}

	if (rsslMapCheckHasSetDefs(oMap))
	{
		position = _rsslDecodeBuffer15(&oMap->encSetDefs,position);

		/* Check for buffer overflow. Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oMap->encSetDefs.data = 0;
		oMap->encSetDefs.length = 0;
	}

	if (rsslMapCheckHasSummaryData(oMap))
	{
		position = _rsslDecodeBuffer15(&oMap->encSummaryData,position);

		/* Check for buffer overflow. Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		oIter->_levelInfo[oIter->_decodingLevel+1]._endBufPtr = position;

		/* if length of summary data is 0, set summary data position to current position */
		if (oMap->encSummaryData.length == 0)
		{
			oMap->encSummaryData.data = position;
		}
	}
	else
	{
		oMap->encSummaryData.data = 0;
		oMap->encSummaryData.length = 0;
	}

	/* extract total count hint */
	if (rsslMapCheckHasTotalCountHint(oMap))
		position += RWF_GET_RESBIT_U30(&oMap->totalCountHint, position);

	position += rwfGet16(_levelInfo->_itemCount, position);

	oMap->encEntries.data = position;
	oMap->encEntries.length = (rtrUInt32)(_endBufPtr - position);

	/* Check for buffer overflow. Post check ok since no copy */
	if (position > _endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	_levelInfo->_nextEntryPtr = position; /* set entry ptr to first entry */
	oIter->_curBufPtr = rsslMapCheckHasSummaryData(oMap) ? oMap->encSummaryData.data : position;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeMapEntry(
				RsslDecodeIterator	*iIter,
				RsslMapEntry		*oMapEntry,
				void * pKeyData )
{
	char 		*position;
	RsslMap		*map;
	RsslUInt8	flags;
	RsslRet		ret;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);

	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];
	
	RSSL_ASSERT(oMapEntry && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(iIter->_levelInfo[iIter->_decodingLevel]._listType, Invalid decoding attempted);

	map = (RsslMap*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	/* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
	position = iIter->_curBufPtr = _levelInfo->_nextEntryPtr;

	if ((position + 2) > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	/* take out action/flags */
	position += rwfGet8(flags, position);

	oMapEntry->action = (flags & 0xF);
	oMapEntry->flags = (flags >> 4);

	/* get perm data */
	if ((rsslMapCheckHasPerEntryPermData(map)) && (rsslMapEntryCheckHasPermData(oMapEntry)))
	{
		position = _rsslDecodeBuffer15(&oMapEntry->permData,position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oMapEntry->permData.length = 0; 
		oMapEntry->permData.data = 0;
	}

	position = _rsslDecodeBuffer15(&oMapEntry->encKey,position);

	/* User provided storage for decoded key, so decode it for them */
	if (pKeyData)
	{
		iIter->_levelInfo[iIter->_decodingLevel]._nextEntryPtr = iIter->_curBufPtr = oMapEntry->encKey.data ? oMapEntry->encKey.data : position;
		iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = position;
		if ((ret = rsslDecodePrimitiveType(iIter, map->keyPrimitiveType, pKeyData)) < 0)
			return ret;
	}

	if (position > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	/* parse MapEntry value */
	if ((oMapEntry->action == RSSL_MPEA_DELETE_ENTRY) ||
		(map->containerType == RSSL_DT_NO_DATA))
	{
		oMapEntry->encData.data = 0;
		oMapEntry->encData.length = 0;
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = _levelInfo->_nextEntryPtr 
			= iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = position;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		/* only have data if action is not delete */
		position += rwfGetBuffer16(&oMapEntry->encData,position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

    	/* shift iterator */
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = oMapEntry->encData.data;
    	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
    	return RSSL_RET_SUCCESS;
	}
}

RSSL_API RsslRet rsslMapFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[5 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_MPF_HAS_SET_DEFS) flagStrings[flagCount++] = RSSL_OMMSTR_MPF_HAS_SET_DEFS.data;
	if (flags & RSSL_MPF_HAS_SUMMARY_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_MPF_HAS_SUMMARY_DATA.data;
	if (flags & RSSL_MPF_HAS_PER_ENTRY_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_MPF_HAS_PER_ENTRY_PERM_DATA.data;
	if (flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT) flagStrings[flagCount++] = RSSL_OMMSTR_MPF_HAS_TOTAL_COUNT_HINT.data;
	if (flags & RSSL_MPF_HAS_KEY_FIELD_ID) flagStrings[flagCount++] = RSSL_OMMSTR_MPF_HAS_KEY_FIELD_ID.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslMapEntryFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[1 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_MPEF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_MPEF_HAS_PERM_DATA.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslMapEntryActionToOmmString(RsslUInt8 action)
{
	switch(action)
	{
		case RSSL_MPEA_UPDATE_ENTRY: return RSSL_OMMSTR_MPEA_UPDATE_ENTRY.data;
		case RSSL_MPEA_ADD_ENTRY: return RSSL_OMMSTR_MPEA_ADD_ENTRY.data;
		case RSSL_MPEA_DELETE_ENTRY: return RSSL_OMMSTR_MPEA_DELETE_ENTRY.data;
		default: return NULL;
	}
}
