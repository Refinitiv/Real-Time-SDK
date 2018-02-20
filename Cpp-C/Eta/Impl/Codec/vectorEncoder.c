/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/rsslVector.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"


static void RTR_FASTCALL _rsslFinishVectorInit(
								RsslVector		*pVector,
								RsslEncodeIterator	*pIter )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	if (rsslVectorCheckHasTotalCountHint(pVector))
		pIter->_curBufPtr +=	RWF_PUT_RESBIT_U30( pIter->_curBufPtr, &pVector->totalCountHint );

	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += 2;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
}

RSSL_API RsslRet rsslEncodeVectorInit( RsslEncodeIterator	*pIter,
										RsslVector			*pVector,
										RsslUInt16			summaryMaxSize,
										RsslUInt16			setMaxSize )
{
	char *_curBufPtr;
	char *_endBufPtr;
	RsslEncodingLevel *_levelInfo;
	RsslUInt8 flags; 

	RSSL_ASSERT(pVector && pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator or buffer not properly set on iterator);

	if (!(_rsslValidAggregateDataType(pVector->containerType)))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;

	_curBufPtr = pIter->_curBufPtr;
	_endBufPtr = pIter->_endBufPtr;

	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_VECTOR,RSSL_EIS_NONE,pVector, pIter->_curBufPtr);
	_levelInfo->_flags = RSSL_EIF_NONE;

	/* Make sure required elements can be encoded */
	/* Flags (1), _containerType (1), Count (2) */
	if (_rsslBufferOverrunEndPtr( _curBufPtr, 5, _endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	flags = (pVector->flags & ~RSSL_VTF_HAS_PER_ENTRY_PERM_DATA);
	_curBufPtr += rwfPut8(_curBufPtr, flags);
	/* container type needs to be scaled before its encoded */
	_curBufPtr += rwfPut8(_curBufPtr, (pVector->containerType - RSSL_DT_CONTAINER_TYPE_MIN));

	/* check for list set definitions */
	if (rsslVectorCheckHasSetDefs(pVector))
	{
		/* We have list set definitions */
		if (pVector->encSetDefs.data != 0)
		{
			/* set data is already encoded */
			/* make sure it can be put in buffer */
			if (_rsslBufferOverrunEndPtr( _curBufPtr, 2 + pVector->encSetDefs.length, _endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			/* ensure it does not overrun rb15 */
			if (pVector->encSetDefs.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}

			_curBufPtr = _rsslEncodeBuffer15(_curBufPtr, &pVector->encSetDefs);
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
			
			/* store # of bytes for summary data so user does not pass it in again */
			if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
				_levelInfo->_internalMark2._sizeBytes = 2;
			else
				_levelInfo->_internalMark2._sizeBytes = 1;

			/* Save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SET_DEFINITIONS;

			return RSSL_RET_SUCCESS;
		}
	}

	/* Check for summary data */
	if (rsslVectorCheckHasSummaryData(pVector))
	{
		/* we have summary data */
		if (pVector->encSummaryData.data != 0)
		{
			/* the summary data is already encoded */
			/* Make sure it can be put in buffer */
			if (_rsslBufferOverrunEndPtr(_curBufPtr, 2 + pVector->encSummaryData.length, _endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}
			
			/* ensure it does not overrun rb15 length */
			if (pVector->encSummaryData.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}

			_curBufPtr = _rsslEncodeBuffer15(_curBufPtr, &pVector->encSummaryData);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			pIter->_curBufPtr = _rsslSetupU15Mark( &_levelInfo->_internalMark2,
												  summaryMaxSize, _curBufPtr );
			/* save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SUMMARY_DATA;
			return RSSL_RET_SUCCESS;
		}
	}

	pIter->_curBufPtr = _curBufPtr;

	if (_rsslIteratorOverrun(pIter, 6))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}	

	_rsslFinishVectorInit(pVector, pIter);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeVectorSetDefsComplete(
										RsslEncodeIterator		*pIter,
										RsslBool				success )
{
	RsslRet ret;
	RsslVector *vector;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_SET_DEFINITIONS, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

	vector = (RsslVector*)_levelInfo->_listType;

	if (success)
	{
		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(ret);
		}

		/* check for summary data */
		if (rsslVectorCheckHasSummaryData(vector))
		{
			if (vector->encSummaryData.data != 0)
			{
				/* summary data is already encoded */
				if (_rsslIteratorOverrun(pIter, 2 + vector->encSummaryData.length))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}
				/* ensure it does not overrun rb15 length */
				if (vector->encSummaryData.length > RWF_MAX_U15)
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_INVALID_DATA;
				}
				
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &vector->encSummaryData);
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
		_rsslFinishVectorInit(vector, pIter );
	}
	else
	{
		RSSL_ASSERT( _levelInfo->_internalMark._sizePtr != 0, Invalid encoding attempted);
		pIter->_curBufPtr = _levelInfo->_internalMark._sizePtr + _levelInfo->_internalMark._sizeBytes;
		/* Leave state as SET_DEFINITIONS */
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeVectorSummaryDataComplete(
										RsslEncodeIterator *pIter,
										RsslBool		   success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_SUMMARY_DATA) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

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
			return(ret);
		}

		_rsslFinishVectorInit((RsslVector*)_levelInfo->_listType, pIter);
	}
	else
	{
		RSSL_ASSERT(_levelInfo->_internalMark2._sizePtr != 0, Invalid encoding attempted);
		pIter->_curBufPtr = _levelInfo->_internalMark2._sizePtr;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
	}
	return RSSL_RET_SUCCESS;
}

static 	RsslRet RTR_FASTCALL _rsslEncodeVectorEntryInternal( RsslEncodeIterator	*pIter,
															RsslVector			*pVector,
															RsslVectorEntry		*pVectorEntry )
{
	RsslUInt8 flags = 0;

	/* Preliminary validations */

	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._currentCount + 1) != 0, Invalid encoding attempted);

	if (_rsslIteratorOverrun(pIter, 5))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pVectorEntry->index > RSSL_VTE_MAX_INDEX)
		return RSSL_RET_INVALID_ARGUMENT;
	
	/* Flags byte made up of flags and action */
	/* set action and flags in same 8 bit value */
	flags = pVectorEntry->flags;
	/* shifts flags by 4 bits */
	flags <<= 4;
	/* sets action bits */
	flags += pVectorEntry->action;

	/* put flags/action into packet */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, flags);

    /* Store index as UInt30_rb */
    pIter->_curBufPtr +=	RWF_PUT_RESBIT_U30( pIter->_curBufPtr, &pVectorEntry->index );

    if(rsslVectorEntryCheckHasPermData(pVectorEntry))
    {
		pIter->_levelInfo[pIter->_encodingLevel]._flags |= RSSL_EIF_HAS_PER_ENTRY_PERM;
	    /* encode perm exp as small buffer */
		if (pVectorEntry->permData.data == 0)
		{
			/* just encode 0 bytes since its empty */
			RsslUInt8 zero = 0;
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, zero);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, 2 + pVectorEntry->permData.length))
				return(RSSL_RET_BUFFER_TOO_SMALL);
			/* ensure this does not overrun rb15 */
			if (pVectorEntry->permData.length > RWF_MAX_U15)
				return RSSL_RET_INVALID_DATA;

			pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pVectorEntry->permData);
		}
	}
 	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeVectorEntry( RsslEncodeIterator	*pIter,
										RsslVectorEntry		*pVectorEntry )
{
	RsslRet ret;
	RsslVector *vector;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	
	RSSL_ASSERT(pIter && pVectorEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

	vector = (RsslVector*)_levelInfo->_listType;

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if ((ret = _rsslEncodeVectorEntryInternal(pIter, vector, pVectorEntry)) < 0)
	{	/* rollback */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		return ret;
	}

	/* encode data as small buffer */
	/* we should only have data if we are not deleting/clearing a position */
	if ((pVectorEntry->action != RSSL_VTEA_CLEAR_ENTRY) && (pVectorEntry->action != RSSL_VTEA_DELETE_ENTRY) && (vector->containerType != RSSL_DT_NO_DATA))
	{
		if (_rsslIteratorOverrun(pIter, 3 + pVectorEntry->encData.length))
		{	/* rollback */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		/* ensure this does not overrun length */
		if (pVectorEntry->encData.length > RWF_MAX_16)
		{
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_INVALID_DATA;
		}

		pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pVectorEntry->encData);

	}
	_levelInfo->_currentCount++;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeVectorEntryInit( RsslEncodeIterator	*pIter,
											RsslVectorEntry		*pVectorEntry,
											RsslUInt16			maxEncodingSize )
{
	RsslRet ret;
	RsslVector *vector;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pVectorEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

	vector = (RsslVector*)_levelInfo->_listType;

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;
	
	if ((ret = _rsslEncodeVectorEntryInternal(pIter, vector, pVectorEntry)) < 0)
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return ret;
	}

	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;

	if ((pVectorEntry->action != RSSL_VTEA_CLEAR_ENTRY) && 
		(pVectorEntry->action != RSSL_VTEA_DELETE_ENTRY) && 
		(vector->containerType != RSSL_DT_NO_DATA))
	{
		if (_rsslIteratorOverrun(pIter, 3))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		pIter->_curBufPtr = _rsslSetupU16Mark(&_levelInfo->_internalMark, 
									maxEncodingSize, pIter->_curBufPtr );
	}
	else
	{
		_levelInfo->_internalMark._sizeBytes = 0;
		_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeVectorEntryComplete( RsslEncodeIterator	*pIter,
												RsslBool			success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRY_INIT) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);

	if (success)
	{
		if (_levelInfo->_internalMark._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU16Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
			{
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_initElemStartPos = 0;
				return(ret);
			}
		}
		else
		{
			/* size bytes is 0 - this means that action was clear or df was no data */
			if (_levelInfo->_internalMark._sizePtr != pIter->_curBufPtr)
			{
				/* something was written when it shouldnt have been */
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

RSSL_API RsslRet rsslEncodeVectorComplete( RsslEncodeIterator		*pIter,
											RsslBool				success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	/* Validations */
	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_VECTOR, Invalid encoding attempted - wrong type);
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

		if (_levelInfo->_flags & RSSL_EIF_HAS_PER_ENTRY_PERM)
		{
			/* write per_entry_perm bit */
			/* flags are first byte of container */
			_levelInfo->_containerStartPos[0] |= RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;
		}
	}
	else
		pIter->_curBufPtr = _levelInfo->_containerStartPos;

	//_levelInfo->_encodingState = RSSL_EIS_COMPLETE;
	--pIter->_encodingLevel;
	
	return RSSL_RET_SUCCESS;
}						
