/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/decoderTools.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslIteratorUtilsInt.h"

RSSL_API RsslRet rsslDecodeVector(RsslDecodeIterator *oIter,
								 RsslVector	*oVector)
{
	char	*position;
	char	*_endBufPtr;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(oVector && oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_VECTOR, oVector); 

	_endBufPtr = _levelInfo->_endBufPtr;
	position = oIter->_curBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}
	if (_endBufPtr - position < 4)
		return RSSL_RET_INCOMPLETE_DATA;

	_levelInfo->_endBufPtr = _endBufPtr;

	position += rwfGet8(oVector->flags, position);

	position += rwfGet8(oVector->containerType, position);
	/* container type needs to be scaled after its decoded */
	oVector->containerType += RSSL_DT_CONTAINER_TYPE_MIN;

	if (rsslVectorCheckHasSetDefs(oVector))
	{
		position = _rsslDecodeBuffer15(&oVector->encSetDefs, position);
		/* check for buffer overflow.  Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oVector->encSetDefs.data = 0;
		oVector->encSetDefs.length = 0;
	}

	if (rsslVectorCheckHasSummaryData(oVector))
	{
		position = _rsslDecodeBuffer15(&oVector->encSummaryData, position);
		/* check for buffer overflow.  Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		oIter->_levelInfo[oIter->_decodingLevel+1]._endBufPtr = position;

		// if length of summary data is 0, set summary data position to current position
		if (oVector->encSummaryData.length == 0)
		{
			oVector->encSummaryData.data = position;
		}
	}
	else
	{
		oVector->encSummaryData.data = 0;
		oVector->encSummaryData.length = 0;
	}

	/* take out total count hint if present */
	if (rsslVectorCheckHasTotalCountHint(oVector))
		position += RWF_GET_RESBIT_U30(&oVector->totalCountHint, position);

	position += rwfGet16(_levelInfo->_itemCount, position);

	oVector->encEntries.data = position;
	oVector->encEntries.length = (rtrUInt32)(_endBufPtr - position);

	/* check for buffer overflow.  Post check ok since no copy */
	if (position > _endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	_levelInfo->_nextEntryPtr = position; /* set entry ptr to first entry */
	oIter->_curBufPtr = rsslVectorCheckHasSummaryData(oVector) ? oVector->encSummaryData.data : position;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeVectorEntry(RsslDecodeIterator * iIter, 
									 RsslVectorEntry * oVectorEntry)
{
	char	*position;
	RsslVector	*vector;
	RsslUInt8	flags;
	RsslDecodingLevel *_levelInfo;
	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);
	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	RSSL_ASSERT(oVectorEntry && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(iIter->_levelInfo[iIter->_decodingLevel]._listType, Invalid decoding attempted);

	vector = (RsslVector*)_levelInfo->_listType;

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

	oVectorEntry->action = (flags & 0xF);
	oVectorEntry->flags = (flags >> 4 );

	/* get index */
	position += RWF_GET_RESBIT_U30(&oVectorEntry->index,position);

	/* get perm data */
	if ((rsslVectorCheckHasPerEntryPermData(vector)) && (rsslVectorEntryCheckHasPermData(oVectorEntry)))
	{
		position = _rsslDecodeBuffer15(&oVectorEntry->permData, position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oVectorEntry->permData.length = 0;
		oVectorEntry->permData.data = 0;
	}

	if ((oVectorEntry->action != RSSL_VTEA_CLEAR_ENTRY) && (oVectorEntry->action != RSSL_VTEA_DELETE_ENTRY) && (vector->containerType != RSSL_DT_NO_DATA))
	{
		position += rwfGetBuffer16(&oVectorEntry->encData, position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

    	/* shift iterator */
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = oVectorEntry->encData.data;
    	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		oVectorEntry->encData.data = 0;
		oVectorEntry->encData.length = 0;
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = _levelInfo->_nextEntryPtr 
			= iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = position;
		return RSSL_RET_SUCCESS;
	}

}

RSSL_API RsslRet rsslVectorFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[5 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_VTF_HAS_SET_DEFS) flagStrings[flagCount++] = RSSL_OMMSTR_VTF_HAS_SET_DEFS.data;
	if (flags & RSSL_VTF_HAS_SUMMARY_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_VTF_HAS_SUMMARY_DATA.data;
	if (flags & RSSL_VTF_HAS_PER_ENTRY_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_VTF_HAS_PER_ENTRY_PERM_DATA.data;
	if (flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT) flagStrings[flagCount++] = RSSL_OMMSTR_VTF_HAS_TOTAL_COUNT_HINT.data;
	if (flags & RSSL_VTF_SUPPORTS_SORTING) flagStrings[flagCount++] = RSSL_OMMSTR_VTF_SUPPORTS_SORTING.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslVectorEntryFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[1 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_VTEF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_VTEF_HAS_PERM_DATA.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslVectorEntryActionToOmmString(RsslUInt8 action)
{
	switch(action)
	{
		case RSSL_VTEA_UPDATE_ENTRY: return RSSL_OMMSTR_VTEA_UPDATE_ENTRY.data;
		case RSSL_VTEA_SET_ENTRY: return RSSL_OMMSTR_VTEA_SET_ENTRY.data;
		case RSSL_VTEA_CLEAR_ENTRY: return RSSL_OMMSTR_VTEA_CLEAR_ENTRY.data;
		case RSSL_VTEA_INSERT_ENTRY: return RSSL_OMMSTR_VTEA_INSERT_ENTRY.data;
		case RSSL_VTEA_DELETE_ENTRY: return RSSL_OMMSTR_VTEA_DELETE_ENTRY.data;
		default: return NULL;
	}
}
