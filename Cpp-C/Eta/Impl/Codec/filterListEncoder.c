/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/rsslFilterList.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"



RSSL_API RsslRet rsslEncodeFilterListInit(	RsslEncodeIterator		  *pIter, 
											RsslFilterList			  *pFilterList )
{
	char *_curBufPtr;
	char *_endBufPtr;
	RsslEncodingLevel *_levelInfo;
	RsslUInt8 flags;

	RSSL_ASSERT(pIter && pFilterList, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);

	if (!(_rsslValidAggregateDataType(pFilterList->containerType)))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;

	_curBufPtr = pIter->_curBufPtr;
	_endBufPtr = pIter->_endBufPtr;

	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_FILTER_LIST,RSSL_EIS_NONE,pFilterList, pIter->_curBufPtr);
	_levelInfo->_flags = RSSL_EIF_NONE;

	/* Make sure that required elements can be encoded */
	/* Flags (1), Opt Data Format (1), Total Count Hint (1), Count (1) */
	if (_rsslBufferOverrunEndPtr( _curBufPtr, 4, _endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	flags = (pFilterList->flags & ~RSSL_FTF_HAS_PER_ENTRY_PERM_DATA);
	_curBufPtr += rwfPut8(_curBufPtr, flags);
	/* container type needs to be scaled before we can send it */
	_curBufPtr += rwfPut8(_curBufPtr, (pFilterList->containerType - RSSL_DT_CONTAINER_TYPE_MIN));

	if (rsslFilterListCheckHasTotalCountHint(pFilterList))
	{
		_curBufPtr += rwfPut8(_curBufPtr, pFilterList->totalCountHint);
	}

	_levelInfo->_countWritePtr = _curBufPtr;
	pIter->_curBufPtr = _curBufPtr + 1;

	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

	return RSSL_RET_SUCCESS;
}

static 	RsslRet RTR_FASTCALL _rsslEncodeFilterEntryInternal(RsslEncodeIterator			*pIter,
 															 RsslFilterList				*pFilterList,
															 RsslFilterEntry			*pFilterEntry )
{
	RsslUInt8	flags = 0;
		
	RSSL_ASSERT( (pIter->_levelInfo[pIter->_encodingLevel]._currentCount + 1) != 0, Invalid encoding attempted);

	if (_rsslIteratorOverrun(pIter, 3))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	flags = pFilterEntry->flags;
	flags <<= 4;
	flags += pFilterEntry->action;

	/* store flags */
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, flags);

	/* Store id as UInt8 */
	pIter->_curBufPtr +=	rwfPut8(pIter->_curBufPtr, pFilterEntry->id );

	/* Store _containerType as UInt8 */	
	if( rsslFilterEntryCheckHasContainerType(pFilterEntry))
	{
		RSSL_ASSERT(_rsslValidAggregateDataType(pFilterEntry->containerType), Invalid entry type);
		/* since this overrides container type, check type again */
		if (!(_rsslValidAggregateDataType(pFilterEntry->containerType)))
			return RSSL_RET_UNSUPPORTED_DATA_TYPE;
		/* container type needs to be scaled before its encoded */
		pIter->_curBufPtr +=	rwfPut8( pIter->_curBufPtr, (pFilterEntry->containerType - RSSL_DT_CONTAINER_TYPE_MIN) );
	}

	/* Store perm lock */
	if(rsslFilterEntryCheckHasPermData(pFilterEntry))
	{
		pIter->_levelInfo[pIter->_encodingLevel]._flags |= RSSL_EIF_HAS_PER_ENTRY_PERM;
		/* Encode the permission expression */
		if (pFilterEntry->permData.data == 0)
		{
			/* just encode 0 bytes since none exists */
			RsslUInt8 zero = 0;
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, zero);
		}
		else
		{
			if (_rsslIteratorOverrun(pIter, 2 + pFilterEntry->permData.length))
				return(RSSL_RET_BUFFER_TOO_SMALL);
			/* ensure this does not overrun rb15 */
			if (pFilterEntry->permData.length > RWF_MAX_U15)
				return RSSL_RET_INVALID_DATA;

			pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pFilterEntry->permData);
		}
	}
	
	return RSSL_RET_SUCCESS;
}


RSSL_API RsslRet rsslEncodeFilterEntry(RsslEncodeIterator			*pIter,
 										RsslFilterEntry				*pFilterEntry )
{
	RsslRet	ret;
	RsslFilterList *filterList;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && pFilterEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

	filterList = (RsslFilterList*)_levelInfo->_listType;

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if ((ret = _rsslEncodeFilterEntryInternal(pIter, filterList, pFilterEntry)) < 0)
	{
		/* rollback */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		return ret;
	}

	if ((((filterList->containerType != RSSL_DT_NO_DATA) && !(pFilterEntry->flags & RSSL_FTEF_HAS_CONTAINER_TYPE)) ||
		 ((pFilterEntry->containerType != RSSL_DT_NO_DATA) && (pFilterEntry->flags & RSSL_FTEF_HAS_CONTAINER_TYPE))) &&
		(pFilterEntry->action != RSSL_FTEA_CLEAR_ENTRY))
	{
		if (_rsslIteratorOverrun(pIter, 3 + pFilterEntry->encData.length))
		{
			/* rollback */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}
		/* ensure this does not overrun buffer 16 length */
		if (pFilterEntry->encData.length > RWF_MAX_16)
		{
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_INVALID_DATA;
		}

		pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pFilterEntry->encData);
	}

	_levelInfo->_currentCount++;

	return RSSL_RET_SUCCESS;	
}


RSSL_API RsslRet rsslEncodeFilterEntryInit(	 RsslEncodeIterator			*pIter,
 											 RsslFilterEntry			*pFilterEntry,
											 RsslUInt16					maxEncodingSize )
{
	RsslRet	ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslFilterList *filterList;

	RSSL_ASSERT(pIter && pFilterEntry, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_FILTER_LIST, Invalid encoding attempted - wrong container);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);

	filterList = (RsslFilterList*)_levelInfo->_listType;

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if ((ret = _rsslEncodeFilterEntryInternal(pIter, filterList, pFilterEntry)) < 0)
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;	
		return ret;
	}
	
	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;

	if ((((filterList->containerType != RSSL_DT_NO_DATA) && !(pFilterEntry->flags & RSSL_FTEF_HAS_CONTAINER_TYPE)) ||
		 ((pFilterEntry->containerType != RSSL_DT_NO_DATA) && (pFilterEntry->flags & RSSL_FTEF_HAS_CONTAINER_TYPE))) &&
		(pFilterEntry->action != RSSL_FTEA_CLEAR_ENTRY))
	{
		if (_rsslIteratorOverrun(pIter, 3))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return RSSL_RET_BUFFER_TOO_SMALL;	
		}

		pIter->_curBufPtr = _rsslSetupU16Mark( &_levelInfo->_internalMark,
										maxEncodingSize, pIter->_curBufPtr);
	}
	else
	{
		_levelInfo->_internalMark._sizeBytes = 0;
		_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeFilterEntryComplete(RsslEncodeIterator	*pIter,
												RsslBool			success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
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
		/* reset the pointer */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
	}

	_levelInfo->_initElemStartPos = 0;
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeFilterListComplete(RsslEncodeIterator		*pIter,
												RsslBool				success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslUInt8 count = (RsslUInt8)_levelInfo->_currentCount;

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{	
		RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
		RSSL_ASSERT(_levelInfo->_countWritePtr != 0, Invalid encoding attempted);
		rwfPut8(_levelInfo->_countWritePtr, count);

		if (_levelInfo->_flags & RSSL_EIF_HAS_PER_ENTRY_PERM)
		{
			/* write per_entry_perm bit */
			/* flags are first byte of container */
			_levelInfo->_containerStartPos[0] |= RSSL_FTF_HAS_PER_ENTRY_PERM_DATA;
		}
	}
	else
	{
		pIter->_curBufPtr = _levelInfo->_containerStartPos;
	}

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}




