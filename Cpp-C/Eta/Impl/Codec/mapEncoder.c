/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/rsslMap.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"


static void RTR_FASTCALL _rsslFinishMapInit(
					RsslMap					*pMap,
					RsslEncodeIterator		*pIter )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	/* Store Total count hint */
	if (rsslMapCheckHasTotalCountHint(pMap))
		pIter->_curBufPtr += RWF_PUT_RESBIT_U30( pIter->_curBufPtr, &pMap->totalCountHint );

	/* Store the position where the count will be stored */
	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += 2;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
}

RSSL_API RsslRet rsslEncodeMapInit(
					RsslEncodeIterator		*pIter,
					RsslMap					*pMap,
					RsslUInt16				summaryMaxSize,
					RsslUInt16				setMaxSize )
{
	RsslEncodingLevel *_levelInfo;
	RsslUInt8 flags;

	RSSL_ASSERT(pMap && pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);

	if (!(_rsslValidPrimitiveDataType(pMap->keyPrimitiveType)))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;

	if (!(_rsslValidAggregateDataType(pMap->containerType)))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;
	
	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_MAP,RSSL_EIS_NONE,pMap, pIter->_curBufPtr);
	_levelInfo->_flags = RSSL_EIF_NONE;

	/* Make sure that required elements can be encoded */
	/* Flags (1), keyPrimitiveType (1), Data Format (1), Count (2) */
	if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 5, pIter->_endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	flags = (pMap->flags & ~RSSL_MPF_HAS_PER_ENTRY_PERM_DATA);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, flags);
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMap->keyPrimitiveType );

	/* container type needs to be scaled before its encoded */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMap->containerType - RSSL_DT_CONTAINER_TYPE_MIN)  );

	if (rsslMapCheckHasKeyFieldId(pMap))
	{
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pMap->keyFieldId);
	}

	/* Check for List Set Definitions */
	if (rsslMapCheckHasSetDefs(pMap))
	{
		/* We have list set definitions */
		if (pMap->encSetDefs.data != 0)
		{
			/* The set data is already encoded. */
			/* Make sure the data can be encoded */
			if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 2 + pMap->encSetDefs.length, pIter->_endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			/* ensure it does not overrun rb15 */
			if (pMap->encSetDefs.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}
			
			pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr,&pMap->encSetDefs);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			pIter->_curBufPtr = _rsslSetupU15Mark( &_levelInfo->_internalMark,
									setMaxSize, pIter->_curBufPtr );
			if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
				_levelInfo->_internalMark2._sizeBytes = 2;
			else
				_levelInfo->_internalMark2._sizeBytes = 1;

			/* Save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SET_DEFINITIONS;
			return RSSL_RET_SUCCESS;
		}
	}

	/* Check for Summary Data */
	if (rsslMapCheckHasSummaryData(pMap))
	{
		/* We have summary data */
		if (pMap->encSummaryData.data != 0)
		{
			/* The summary data is already encoded. */
			/* Make sure the data can be encoded */
			if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 2 + pMap->encSummaryData.length, pIter->_endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			/* ensure it does not overrun rb15 */
			if (pMap->encSummaryData.length > RWF_MAX_U15)
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_INVALID_DATA;
			}

			pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr,&pMap->encSummaryData);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			pIter->_curBufPtr = _rsslSetupU15Mark( &_levelInfo->_internalMark2,
									summaryMaxSize, pIter->_curBufPtr );
			/* Save state and return */
			_levelInfo->_encodingState = RSSL_EIS_SUMMARY_DATA;
			return RSSL_RET_SUCCESS;
		}
	}

	if (_rsslIteratorOverrun(pIter, 6))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	_rsslFinishMapInit( pMap, pIter );
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMapSetDefsComplete(
					RsslEncodeIterator		*pIter,
					RsslBool				success )
{
	RsslRet	ret;
	RsslMap	*map;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong container);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_SET_DEFINITIONS, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid iterator use - check buffer);

	map = (RsslMap*)_levelInfo->_listType;

	if (success)
	{
		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark,pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(ret);
		}

		/* Check for Summary Data */
		if (rsslMapCheckHasSummaryData(map))
		{
			if (map->encSummaryData.data != 0)
			{
				/* Summary data is already encoded */
				if (_rsslIteratorOverrun(pIter, 2 + map->encSummaryData.length))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}

				/* ensure it does not overrun rb15 */
				if (map->encSummaryData.length > RWF_MAX_U15)
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_INVALID_DATA;
				}

				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr,&map->encSummaryData);
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

				/* Save state and return */
				_levelInfo->_encodingState = RSSL_EIS_SUMMARY_DATA;
				return RSSL_RET_SUCCESS;
			}
		}
		
		if (_rsslIteratorOverrun(pIter, 6))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		_rsslFinishMapInit( map, pIter );
	}
	else
	{
		RSSL_ASSERT( _levelInfo->_internalMark._sizePtr != 0, Invalid encoding attempted );
		pIter->_curBufPtr = _levelInfo->_internalMark._sizePtr + _levelInfo->_internalMark._sizeBytes;
		/* Leave state as SET_DEFINITIONS */
	}	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMapSummaryDataComplete(
					RsslEncodeIterator		*pIter,
					RsslBool				success )
{
	RsslRet	ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong container);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_SUMMARY_DATA) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid iterator use - check buffer);

	if (success)
	{
		/* check length first - if this fails, they can grow buffer and come back here because sizeMark will still be OK */
		if (_rsslIteratorOverrun(pIter, 6))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark2,pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(ret);
		}

		_rsslFinishMapInit( (RsslMap*)_levelInfo->_listType, pIter );
	}
	else
	{
		RSSL_ASSERT( _levelInfo->_internalMark2._sizePtr != 0, Invalid encoding attempted );
		pIter->_curBufPtr = _levelInfo->_internalMark2._sizePtr;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMapComplete(
					RsslEncodeIterator		*pIter,
					RsslBool				success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong container);
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
			_levelInfo->_containerStartPos[0] |= RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;
		}
	}
	else
	{
		pIter->_curBufPtr = _levelInfo->_containerStartPos;
	}

	--pIter->_encodingLevel;
	return RSSL_RET_SUCCESS;
}														 


static RsslRet RTR_FASTCALL _rsslEncodeMapEntryInternal(
					RsslEncodeIterator	*pIter,
					RsslMap				*pMap,
					RsslMapEntry		*pMapEntry,
					const void			*pKey )

{
	RsslUInt8 flags = 0;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);

	/* Flags byte made up of flags and action */
	flags = pMapEntry->flags;
	flags <<= 4;
	flags += pMapEntry->action;

	if (_rsslIteratorOverrun(pIter, 1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* Encode the flags */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, flags );

	/* Check for optional permissions expression per entry */
	if(rsslMapEntryCheckHasPermData(pMapEntry))
    {
		/* indicate that we want to set per-entry perm since the user encoded perm data */
		_levelInfo->_flags |= RSSL_EIF_HAS_PER_ENTRY_PERM;

		/* Encode the permission expression */
		if (pMapEntry->permData.data == 0)
		{
			/* Just encode 0 bytes since none exists */
			RsslUInt8 zero = 0;
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, zero );
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, 2 + pMapEntry->permData.length))
				return(RSSL_RET_BUFFER_TOO_SMALL);
			
			/* make sure this wont exceed rb15 length */
			if (pMapEntry->permData.length > RWF_MAX_U15)
				return RSSL_RET_INVALID_DATA;

			pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr,&pMapEntry->permData);
		}
    }

    /* not pre-encoded - encode it */
	if (pKey != 0)
	{
		RsslRet ret = 0; 
		RsslUInt8 prevstate = _levelInfo->_encodingState;
		_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE_U15;
		ret = _rsslEncodePrimitive(pIter, pMap->keyPrimitiveType, pKey );
		_levelInfo->_encodingState = prevstate;
		if (ret < 0)
			return ret;
	}
	else if (pMapEntry->encKey.data != 0)
	{
		/* Check for pre-encoded key */
		/* we probably dont need to check data or length as the ASSERTS should prevent it, however
		   those are currently only on debug mode */
		RSSL_ASSERT(pMapEntry->encKey.length, Blank key not allowed);
		/* Key is pre-encoded. */

        /* for buffers, etc; size need to be encoded, hence encoding it as small buffer */
		if (_rsslIteratorOverrun(pIter, 2 + pMapEntry->encKey.length))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		/* ensure length does not overrun rb15 */
		if (pMapEntry->encKey.length > RWF_MAX_U15)
			return RSSL_RET_INVALID_DATA;
		pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr,&pMapEntry->encKey);
	}
	else
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMapEntry(
					RsslEncodeIterator	*pIter,
					RsslMapEntry		*pMapEntry,
					const void			*pKeyData )
{
	RsslRet	ret;
	RsslMap	*map;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pMapEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid iterator use - check buffer);
	RSSL_ASSERT((pMapEntry->encKey.length != 0 && pMapEntry->encKey.data) || pKeyData, Entry key missing);

	map = (RsslMap*)_levelInfo->_listType;
	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if ((ret = _rsslEncodeMapEntryInternal( pIter, map, pMapEntry, pKeyData )) < 0)
	{	
		/* rollback */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		return ret;
	}

	if ((pMapEntry->action != RSSL_MPEA_DELETE_ENTRY) &&
		(map->containerType != RSSL_DT_NO_DATA))
	{
		if (_rsslIteratorOverrun(pIter, 3 + pMapEntry->encData.length))
		{
			/* rollback */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}
		
		/* if content is too big to fit in length bytes */
		if (pMapEntry->encData.length > RWF_MAX_16)
		{
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(RSSL_RET_INVALID_DATA);
		}

		pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr,&pMapEntry->encData);
	}

	_levelInfo->_currentCount++;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMapEntryInit(
					RsslEncodeIterator	*pIter,
					RsslMapEntry		*pMapEntry,
					const void			*pKeyData,
					RsslUInt16			maxEncodingSize )
{
	RsslRet	ret;
	RsslMap	*map;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pMapEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid iterator use - check buffer);
	RSSL_ASSERT((pMapEntry->encKey.length != 0 && pMapEntry->encKey.data) || pKeyData, Entry key missing);

	map = (RsslMap*)_levelInfo->_listType;

	RSSL_ASSERT(map->keyPrimitiveType != RSSL_DT_NO_DATA, Invalid key type specified);

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if ((ret = _rsslEncodeMapEntryInternal( pIter, map, pMapEntry, pKeyData )) < 0)
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return ret;
	}

	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;
	
	if ((pMapEntry->action != RSSL_MPEA_DELETE_ENTRY) &&
		(map->containerType != RSSL_DT_NO_DATA))
	{
		if (_rsslIteratorOverrun(pIter, 3))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		pIter->_curBufPtr = _rsslSetupU16Mark( &_levelInfo->_internalMark,
									maxEncodingSize, pIter->_curBufPtr );
	}
	else
	{
		/* set up mark so we know not to close it */
		_levelInfo->_internalMark._sizeBytes = 0;
		_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
	}

	return RSSL_RET_SUCCESS;
}


RSSL_API RsslRet rsslEncodeMapEntryComplete(
					RsslEncodeIterator	*pIter,
					RsslBool			success )
{
	RsslRet	ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_MAP, Invalid encoding attempted - wrong type);
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
		/* Reset the pointer */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
	}

	_levelInfo->_initElemStartPos = 0;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
	
	return RSSL_RET_SUCCESS;
}

