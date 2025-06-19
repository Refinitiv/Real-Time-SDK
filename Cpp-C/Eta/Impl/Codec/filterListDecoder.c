/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslFilterList.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"

RSSL_API RsslRet rsslDecodeFilterList(RsslDecodeIterator	*oIter, 
									 RsslFilterList		*oFilterList)
{
	char	*position;
	char	*_endBufPtr;
	RsslUInt8 count;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(oFilterList && oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_FILTER_LIST, oFilterList); 

	_endBufPtr = _levelInfo->_endBufPtr;
	position = oIter->_curBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}
	else if (_endBufPtr - position < 3)
		return RSSL_RET_INCOMPLETE_DATA;

	position += rwfGet8(oFilterList->flags, position);
	 
	position += rwfGet8(oFilterList->containerType, position);
	/* needs to be scaled back after decoding */
	oFilterList->containerType += RSSL_DT_CONTAINER_TYPE_MIN;
	
	if (rsslFilterListCheckHasTotalCountHint(oFilterList))
	{
		position += rwfGet8(oFilterList->totalCountHint, position);
	}
	else
		oFilterList->totalCountHint = 0;

	position += rwfGet8(count, position);
	_levelInfo->_itemCount = count;
	
	oFilterList->encEntries.data = position;
	oFilterList->encEntries.length = (rtrUInt32)(_endBufPtr - position);

	/* Check for buffer overflow.  Post check ok since no copy */
	if (position > _endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	oIter->_curBufPtr = _levelInfo->_nextEntryPtr = position;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeFilterEntry( RsslDecodeIterator *iIter, RsslFilterEntry *oFilterEntry )
{
	char	*position;
	RsslFilterList	*filterList;
	RsslUInt8 flags;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);

	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	RSSL_ASSERT(oFilterEntry && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(iIter->_levelInfo[iIter->_decodingLevel]._listType, Invalid decoding attempted);
	
	filterList = (RsslFilterList*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	/* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
	position = iIter->_curBufPtr = _levelInfo->_nextEntryPtr;

	if ((position + 2) > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	/* take out flags */
	position += rwfGet8(flags, position);

	oFilterEntry->action = (flags & 0xF);
	oFilterEntry->flags = (flags >> 4);

	/* parse FilterEntry */
	position += rwfGet8(oFilterEntry->id, position);

	if (rsslFilterEntryCheckHasContainerType(oFilterEntry))
	{
		if ((position + 1) > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
		position += rwfGet8(oFilterEntry->containerType, position);
		/* needs to be scaled back after decoding */
		oFilterEntry->containerType += RSSL_DT_CONTAINER_TYPE_MIN;
	}
	else
		oFilterEntry->containerType = filterList->containerType;

	if ((rsslFilterListCheckHasPerEntryPermData(filterList)) && (rsslFilterEntryCheckHasPermData(oFilterEntry)))
	{
		position = _rsslDecodeBuffer15(&oFilterEntry->permData, position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oFilterEntry->permData.length = 0;
		oFilterEntry->permData.data = 0;
	}

	if ((oFilterEntry->containerType != RSSL_DT_NO_DATA) && (oFilterEntry->action != RSSL_FTEA_CLEAR_ENTRY))
	{
		position += rwfGetBuffer16(&oFilterEntry->encData, position);
		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		/* shift iterator */
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = oFilterEntry->encData.data;
    	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		oFilterEntry->encData.data = 0;
		oFilterEntry->encData.length = 0;
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = _levelInfo->_nextEntryPtr 
			= iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = position;
		return RSSL_RET_SUCCESS;
	}
	



	
}

RSSL_API const char* rsslFilterEntryActionToOmmString(RsslUInt8 action)
{
	switch(action)
	{
		case RSSL_FTEA_UPDATE_ENTRY: return RSSL_OMMSTR_FTEA_UPDATE_ENTRY.data;
		case RSSL_FTEA_SET_ENTRY: return RSSL_OMMSTR_FTEA_SET_ENTRY.data;
		case RSSL_FTEA_CLEAR_ENTRY: return RSSL_OMMSTR_FTEA_CLEAR_ENTRY.data;
		default: return NULL;
	}
}

RSSL_API RsslRet rsslFilterListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[2 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_FTF_HAS_PER_ENTRY_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_FTF_HAS_PER_ENTRY_PERM_DATA.data;
	if (flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT) flagStrings[flagCount++] = RSSL_OMMSTR_FTF_HAS_TOTAL_COUNT_HINT.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslFilterEntryFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[2 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_FTEF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_FTEF_HAS_PERM_DATA.data;
	if (flags & RSSL_FTEF_HAS_CONTAINER_TYPE) flagStrings[flagCount++] = RSSL_OMMSTR_FTEF_HAS_CONTAINER_TYPE.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}
