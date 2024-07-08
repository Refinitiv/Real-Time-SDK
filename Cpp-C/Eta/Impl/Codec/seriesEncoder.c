/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/encoderTools.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslIteratorUtilsInt.h"


static void RTR_FASTCALL _rsslFinishSeriesInit(	RsslSeries		*pSeries,
											RsslEncodeIterator	*pIter )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	/* store count hint */
	if (rsslSeriesCheckHasTotalCountHint(pSeries))
		pIter->_curBufPtr += RWF_PUT_RESBIT_U30(pIter->_curBufPtr, &pSeries->totalCountHint);

	/* store the count position */
	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += 2;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
}


RSSL_API RsslRet rsslEncodeSeriesInit( RsslEncodeIterator	*pIter,
									  RsslSeries			*pSeries,
									  RsslUInt16			summaryMaxSize,
									  RsslUInt16			setMaxSize )
{
    char *_curBufPtr;
	char *_endBufPtr;
	RsslEncodingLevel *_levelInfo;

	RSSL_ASSERT(pSeries && pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);
	if (!(_rsslValidAggregateDataType(pSeries->containerType)))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;

	_curBufPtr = pIter->_curBufPtr;
	_endBufPtr = pIter->_endBufPtr;

	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_SERIES,RSSL_EIS_NONE,pSeries, pIter->_curBufPtr);

	/* Make sure required elements can be encoded */
	/* Flags (1), _containerType (1), Count (2) */
	if (_rsslBufferOverrunEndPtr(_curBufPtr, 4, _endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	_curBufPtr += rwfPut8(_curBufPtr, pSeries->flags);
	/* container type needs to be scaled before its encoded */
	_curBufPtr += rwfPut8(_curBufPtr, (pSeries->containerType - RSSL_DT_CONTAINER_TYPE_MIN));

	/* check if we have list set definitions */
	if (rsslSeriesCheckHasSetDefs(pSeries))
	{
		/* we have definitions */
		if (pSeries->encSetDefs.data != 0)
		{
			/* the set data is already encoded */
			/* make sure it fits */
			if (_rsslBufferOverrunEndPtr(_curBufPtr, 2 + pSeries->encSetDefs.length, _endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}
			/* ensure does not overrun rb15 */
			if (pSeries->encSetDefs.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}

			_curBufPtr = _rsslEncodeBuffer15(_curBufPtr, &pSeries->encSetDefs);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			pIter->_curBufPtr = _rsslSetupU15Mark(&_levelInfo->_internalMark, 
													setMaxSize, _curBufPtr);
			
			if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
				_levelInfo->_internalMark2._sizeBytes = 2;
			else
				_levelInfo->_internalMark2._sizeBytes = 1;

			/* save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SET_DEFINITIONS;
			return RSSL_RET_SUCCESS;
		}
	}

	/* check for summary data */
	if (rsslSeriesCheckHasSummaryData(pSeries))
	{
		/* we have summary data */
		if (pSeries->encSummaryData.data != 0)
		{
			/* this is already encoded */
			/* make sure it fits */
			if (_rsslBufferOverrunEndPtr(_curBufPtr, 2 + pSeries->encSummaryData.length, _endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}
			/* ensure this does not overrun rb15 length */
			if (pSeries->encSummaryData.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}

			_curBufPtr = _rsslEncodeBuffer15(_curBufPtr, &pSeries->encSummaryData);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			pIter->_curBufPtr = _rsslSetupU15Mark(&_levelInfo->_internalMark2,
													summaryMaxSize, _curBufPtr);
			/* save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SUMMARY_DATA;
			return RSSL_RET_SUCCESS;
		}
	}

	pIter->_curBufPtr = _curBufPtr;

	if (_rsslIteratorOverrun(pIter, 4))
	{	
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;	
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	_rsslFinishSeriesInit(pSeries, pIter);
	return RSSL_RET_SUCCESS;
}
                                                        
RSSL_API RsslRet rsslEncodeSeriesSetDefsComplete(RsslEncodeIterator		*pIter,
												RsslBool				success )
{
	RsslRet ret;
	RsslSeries *series;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_SET_DEFINITIONS, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted - check buffer);

	series = (RsslSeries*)_levelInfo->_listType;

	if (success)
	{
		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return (ret);
		}

		/* check for summary data */
		if (rsslSeriesCheckHasSummaryData(series))
		{
			if (series->encSummaryData.data != 0)
			{
				/* summary data is already encoded */
				if (_rsslIteratorOverrun(pIter, 2 + series->encSummaryData.length))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}
				/* ensure this does not overrun rb15 */
				if (series->encSummaryData.length > RWF_MAX_U15)
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return  RSSL_RET_INVALID_DATA;
				}

				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &series->encSummaryData);
			}
			else
			{
				/* we already stored length in the INIT call */
				RSSL_ASSERT(_levelInfo->_internalMark2._sizePtr == 0, Invalid encoding attempted);

				if (_rsslIteratorOverrun(pIter, _levelInfo->_internalMark2._sizeBytes))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}

				_levelInfo->_internalMark2._sizePtr = pIter->_curBufPtr;
				pIter->_curBufPtr += _levelInfo->_internalMark2._sizeBytes;

				/* save state and return */
				_levelInfo->_encodingState = RSSL_EIS_SUMMARY_DATA;
				return RSSL_RET_SUCCESS;
			}
		}
		if (_rsslIteratorOverrun(pIter, 6))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		_rsslFinishSeriesInit(series, pIter);
	}
	else
	{
		RSSL_ASSERT(_levelInfo->_internalMark._sizePtr != 0, Invalid encoding attempted);
		pIter->_curBufPtr = _levelInfo->_internalMark._sizePtr + _levelInfo->_internalMark._sizeBytes;
		/* Leave state as SET_DEFINITIONS */
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeSeriesSummaryDataComplete(RsslEncodeIterator		*pIter,
													RsslBool				success )
{
	RsslRet	ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_SUMMARY_DATA) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted - wrong type);

	if (success)
	{
		/* check length first - if this fails, they can grow buffer and come back here because sizeMark will still be OK */
		if (_rsslIteratorOverrun(pIter, 6))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark2, pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return (ret);
		}

		_rsslFinishSeriesInit( (RsslSeries*)_levelInfo->_listType, pIter);
	}
	else
	{
		RSSL_ASSERT(_levelInfo->_internalMark2._sizePtr != 0, Invalid encoding attempted);
		pIter->_curBufPtr = _levelInfo->_internalMark2._sizePtr;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeSeriesEntry( RsslEncodeIterator	*pIter,
										RsslSeriesEntry		*pSeriesEntry )
{
	RsslSeries *series;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pSeriesEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid iterator use - check buffer);

	series = (RsslSeries*)_levelInfo->_listType;

	if (series->containerType != RSSL_DT_NO_DATA)
	{
		if (_rsslIteratorOverrun(pIter, 3 + pSeriesEntry->encData.length))
		{
			/* no need for rollback because iterator was never moved*/
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}
		/* ensure this does not overrun buffer 16 length */
		if (pSeriesEntry->encData.length > RWF_MAX_16)
		{
			/* no need for rollback because iterator was never moved */
			return RSSL_RET_INVALID_DATA;
		}
		pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pSeriesEntry->encData);
	}

	_levelInfo->_currentCount++;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet     rsslEncodeSeriesEntryInit(RsslEncodeIterator		*pIter,
												RsslSeriesEntry			*pSeriesEntry,
												RsslUInt16				maxEncodingSize )
{
	RsslSeries	*series;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pSeriesEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted - check buffer);

	series = (RsslSeries*)_levelInfo->_listType;

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;

	if (series->containerType != RSSL_DT_NO_DATA)
	{
		if (_rsslIteratorOverrun(pIter, 3))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		pIter->_curBufPtr = _rsslSetupU16Mark(&_levelInfo->_internalMark,
									maxEncodingSize, pIter->_curBufPtr);
	}
	else
	{
		_levelInfo->_internalMark._sizeBytes = 0;
		_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet     rsslEncodeSeriesEntryComplete(RsslEncodeIterator  *pIter,
													RsslBool			success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRY_INIT) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1 ) != 0, Invalid encoding attempted);

	if (success)
	{
		if (_levelInfo->_internalMark._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU16Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
			{
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_initElemStartPos = 0;
				return (ret);
			}
		}
		else
		{
			/* size bytes is 0 - this means that action was clear or df was no data */
			if (_levelInfo->_internalMark._sizePtr != pIter->_curBufPtr)
			{
				/* something was written when it shouldnt have been */
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_initElemStartPos = 0;
				return RSSL_RET_INVALID_DATA;
			}
			_levelInfo->_internalMark._sizePtr = 0;
		}

		_levelInfo->_currentCount++;
	}
	else
	{
		/* reset the pointer */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
	}

	_levelInfo->_initElemStartPos = 0;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeSeriesComplete( RsslEncodeIterator	*pIter,
											RsslBool			success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_SERIES, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
				(_levelInfo->_encodingState == RSSL_EIS_SET_DEFINITIONS) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{
		RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
		RSSL_ASSERT(_levelInfo->_countWritePtr != 0, Invalid encoding attempted);
		rwfPut16(_levelInfo->_countWritePtr, _levelInfo->_currentCount);
	}
	else
	{
		pIter->_curBufPtr = _levelInfo->_containerStartPos;
	}

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}
