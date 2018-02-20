/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/decoderTools.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslIteratorUtilsInt.h"


RSSL_API RsslRet rsslDecodeSeries(RsslDecodeIterator *oIter, 
								 RsslSeries	*oSeries )
{
	char *position;
	char *_endBufPtr;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(oSeries && oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_SERIES, oSeries); 

	_endBufPtr = _levelInfo->_endBufPtr;
	position = oIter->_curBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}
	else if (_endBufPtr - position < 4)
		return RSSL_RET_INCOMPLETE_DATA;

	_levelInfo->_endBufPtr = _endBufPtr;

	/* extract flags, data format */

	position += rwfGet8(oSeries->flags, position);
	position += rwfGet8(oSeries->containerType, position);
	/* container type needs to be scaled back after decoding */
	oSeries->containerType += RSSL_DT_CONTAINER_TYPE_MIN;

	if (rsslSeriesCheckHasSetDefs(oSeries))
	{
		position = _rsslDecodeBuffer15(&oSeries->encSetDefs, position);
		/* check for buffer overflow.  Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
	}
	else
	{
		oSeries->encSetDefs.data = 0;
		oSeries->encSetDefs.length = 0;
	}

	if (rsslSeriesCheckHasSummaryData(oSeries))
	{
		position = _rsslDecodeBuffer15(&oSeries->encSummaryData, position);
		/* check for buffer overflow.  Post check ok since no copy */
		if (position > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		oIter->_levelInfo[oIter->_decodingLevel+1]._endBufPtr = position;

		/* if length of summary data is 0, set summary data position to current position */
		if (oSeries->encSummaryData.length == 0)
		{
			oSeries->encSummaryData.data = position;
		}
	}
	else
	{
		oSeries->encSummaryData.data = 0;
		oSeries->encSummaryData.length = 0;
	}

	/* extract total count hints */
	if (rsslSeriesCheckHasTotalCountHint(oSeries))
		position += RWF_GET_RESBIT_U30(&oSeries->totalCountHint, position);
	else
		oSeries->totalCountHint = 0;

	position += rwfGet16(_levelInfo->_itemCount, position);

	oSeries->encEntries.data = position;
	oSeries->encEntries.length = (rtrUInt32)(_endBufPtr - position);

	/* check for overflow */
	if (position > _endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	_levelInfo->_nextEntryPtr = position; /* set entry ptr to first entry */
	oIter->_curBufPtr = rsslSeriesCheckHasSummaryData(oSeries) ? oSeries->encSummaryData.data : position;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeSeriesEntry(RsslDecodeIterator *iIter, 
									RsslSeriesEntry	*oSeriesEntry )
{
	char *position;
	RsslSeries *series;
	RsslDecodingLevel *_levelInfo;
	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);
	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	RSSL_ASSERT(oSeriesEntry && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(iIter->_levelInfo[iIter->_decodingLevel]._listType, Invalid decoding attempted);

	series = (RsslSeries*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	/* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
	position = iIter->_curBufPtr = _levelInfo->_nextEntryPtr;
	
	if (series->containerType != RSSL_DT_NO_DATA)
	{
		position += rwfGetBuffer16(&oSeriesEntry->encData, position);
    	if (position > _levelInfo->_endBufPtr)
    		return RSSL_RET_INCOMPLETE_DATA;
    
    	/* shift iterator */
    	iIter->_curBufPtr = oSeriesEntry->encData.data;
    	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
    	_levelInfo->_nextItemPosition++;
    	return RSSL_RET_SUCCESS;
	}
	else
	{
		oSeriesEntry->encData.data = 0;
		oSeriesEntry->encData.length = 0;
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr = _levelInfo->_nextEntryPtr 
			= iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
    	return RSSL_RET_SUCCESS;
	}
}

RSSL_API RsslRet rsslSeriesFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[3 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_SRF_HAS_SET_DEFS) flagStrings[flagCount++] = RSSL_OMMSTR_SRF_HAS_SET_DEFS.data;
	if (flags & RSSL_SRF_HAS_SUMMARY_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_SRF_HAS_SUMMARY_DATA.data;
	if (flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT) flagStrings[flagCount++] = RSSL_OMMSTR_SRF_HAS_TOTAL_COUNT_HINT.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}
