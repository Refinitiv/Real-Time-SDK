/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslElementList.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rtr/rsslDataUtils.h"

RsslRet RTR_FASTCALL _rsslCompleteElementSet(RsslEncodeIterator *pIter, RsslEncodingLevel *_levelInfo, RsslElementList *pElementList);


RSSL_API RsslRet rsslEncodeElementListInit(
					RsslEncodeIterator		*pIter,
					RsslElementList			*pElementList,
					const RsslLocalElementSetDefDb	*pSetDb,
					RsslUInt16				setEncodingMaxSize )
{
	RsslEncodingLevel *_levelInfo;
	RsslUInt16 setId;

	/* Assertions */
	RSSL_ASSERT(pElementList && pIter, Invalid parameters or parameters passed in as NULL );
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);
	
	if (++pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_ELEMENT_LIST,RSSL_EIS_NONE,pElementList, pIter->_curBufPtr);

	/* make sure required elements can be encoded */
	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 1, pIter->_endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	/* store flags as UInt8 */
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElementList->flags);

	/* check for List Info */
	if (rsslElementListHasInfo(pElementList))
	{
		RsslUInt8 infoLen;
		char	  *startPos;

		/* make sure that required elements can be encoded */
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 3, pIter->_endBufPtr))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		/* save info length position */
		startPos = pIter->_curBufPtr;

		/* move past it */
		pIter->_curBufPtr += 1;

		/* put element list number into element list */
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pElementList->elementListNum);

		/* encode the length into the element list */
		infoLen = (RsslUInt8)(pIter->_curBufPtr - startPos - 1);
		rwfPut8(startPos, infoLen);
	}

	/* check for set data */
	if (rsslElementListCheckHasSetData(pElementList))
	{
		/* we have element list set data */
		/* make sure set id and set length can be encoded */
		/* setID (2), setData (2) */
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4, pIter->_endBufPtr))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		/* check for/encode optional element list set id */
		if (rsslElementListCheckHasSetId(pElementList))
		{
			pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pElementList->setId);
			setId = pElementList->setId;
		}
		else
			setId = 0;

		if ( pSetDb && setId <= RSSL_ELEMENT_SET_MAX_LOCAL_ID && pSetDb->definitions[setId].setId != RSSL_ELEMENT_SET_BLANK_ID)
			_levelInfo->_elemListSetDef = &pSetDb->definitions[setId];
		else 
		if ( pIter->_pGlobalElemListSetDb && setId > RSSL_ELEMENT_SET_MAX_LOCAL_ID  && setId <= pIter->_pGlobalElemListSetDb->maxSetId && pIter->_pGlobalElemListSetDb->definitions[setId] != NULL)
			_levelInfo->_elemListSetDef = pIter->_pGlobalElemListSetDb->definitions[setId];

		/* check for element list data after the set data */
		if (rsslElementListCheckHasStandardData(pElementList))
		{
			/* if have one set data and field list data, need length in front of the set */
			if (pElementList->encSetData.data != 0)
			{
				/* the set data is already encoded */
				/* make sure that the set data can be encoded */
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, pElementList->encSetData.length + 4, pIter->_endBufPtr))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}
				/* ensure it does not overrun rb15 len */
				if (pElementList->encSetData.length > RWF_MAX_U15)
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_INVALID_DATA;
				}

				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElementList->encSetData);

				/* save bytes for field list data count */
				_levelInfo->_countWritePtr = pIter->_curBufPtr;
				_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
				pIter->_curBufPtr += 2;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* the set data needs to be encoded */
				/* save state and return */
				if (_levelInfo->_elemListSetDef)
				{
					pIter->_curBufPtr = _rsslSetupU15Mark(&_levelInfo->_internalMark,
						setEncodingMaxSize, pIter->_curBufPtr);

					/* If the set actually has no entries, just complete the set. */
					if (_levelInfo->_elemListSetDef->count > 0) _levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					else _rsslCompleteElementSet(pIter, _levelInfo, pElementList);

					return RSSL_RET_SUCCESS;
				}
				else
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_SET_DEF_NOT_PROVIDED;
				}
			}
		}
		else
		{
			/* dont need length in front of set data */
			/* encode set data if it exists */
			if (pElementList->encSetData.data != 0)
			{
				/* make sure that set data can be encoded */
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr,pElementList->encSetData.length, pIter->_endBufPtr))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}

				/* dont need a length in front of set data */
				pIter->_curBufPtr = _rsslEncodeCopyData(pIter->_curBufPtr, &pElementList->encSetData);

				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* dont need length in front of set data */
				/* save size pointer in case of rewind */									
			
				if (_levelInfo->_elemListSetDef)
				{
					_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
					_levelInfo->_internalMark._sizeBytes = 0;

					/* If the set actually has no entries, turn around and complete the set. */
					if (_levelInfo->_elemListSetDef->count > 0) _levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					else _rsslCompleteElementSet(pIter, _levelInfo, pElementList);

					return RSSL_RET_SUCCESS;
				}
				else
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_SET_DEF_NOT_PROVIDED;
				}
			}
		}
	}
	else if (rsslElementListCheckHasStandardData(pElementList))
	{
		/* we only have data */
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		_levelInfo->_countWritePtr = pIter->_curBufPtr;
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
		pIter->_curBufPtr += 2;
	}

	if (_levelInfo->_encodingState == RSSL_EIS_NONE)
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslCompleteElementSet(RsslEncodeIterator *pIter, RsslEncodingLevel *_levelInfo, RsslElementList *pElementList)
{
	RsslRet ret;

	/* Set definition completed. Write the length, and move on to standard data if any. */
	/* length may not be encoded in certain cases */
	if (_levelInfo->_internalMark._sizeBytes != 0)
	{
		if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return (ret);
		}

		if (rsslElementListCheckHasStandardData((RsslElementList*)_levelInfo->_listType))
		{
			/* save space for data count */

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
			{
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_BUFFER_TOO_SMALL;
			}

			/* Reset entry count.  Only Standard Entries actually count towards it. */
			_levelInfo->_currentCount = 0;
			_levelInfo->_countWritePtr = pIter->_curBufPtr;
			pIter->_curBufPtr += 2;
			_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
		}
		else
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
	}
	else
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;

	return RSSL_RET_SUCCESS;	
}


RSSL_API RsslRet rsslEncodeElementEntry( 
								RsslEncodeIterator		*pIter,
								RsslElementEntry		*pElement,
								const void				*pData )
{
	RsslRet ret = 0;
	RsslUInt32 encSize = 0;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pElement, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ELEMENT_LIST, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES || _levelInfo->_encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if (_levelInfo->_encodingState == RSSL_EIS_SET_DATA)
	{
		RsslElementList *pElementList = (RsslElementList*)_levelInfo->_listType;
		const RsslElementSetDef *pDef = _levelInfo->_elemListSetDef;

		/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
		const RsslElementSetDefEntry *pEncoding = &pDef->pEntries[_levelInfo->_currentCount];

		RSSL_ASSERT(pDef, Invalid parameters or parameters passed in as NULL);

		/* Validate name (if present) */
		if (pElement->name.data && !rsslBufferIsEqual(&pElement->name, &pEncoding->name))
			return RSSL_RET_INVALID_DATA;
  
		/* Encode item according to set type */
		if (pData)
		{
			/* Validate type - only require it to be primitive if not pre-encoded */
			if (pElement->dataType != _rsslPrimitiveType(pEncoding->dataType))
				return RSSL_RET_INVALID_DATA;

			if ((ret = _rsslEncodeSet(pIter, pEncoding->dataType, pData)) < 0)
			{
				/* likely nothing to roll back, but be safe */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				return ret;
			}
		}
		else if (pElement->encData.length && pElement->encData.data)/* Encoding pre-encoded data from the field entry */
		{
			if (_rsslIteratorOverrun(pIter, (pElement->encData.length + 3))) /* Account for ob16 length byte */
				return(RSSL_RET_BUFFER_TOO_SMALL);

			/* if the dataType is primitive we need to make sure the data is length specified */
			if ((pElement->dataType < RSSL_DT_SET_PRIMITIVE_MIN) || (pElement->dataType > RSSL_DT_CONTAINER_TYPE_MIN)) 
			{
				/* ensure doesnt overrun buf16 length */
				if (pElement->encData.length > RWF_MAX_16)
				{
					/* nothing to roll back */
					return RSSL_RET_INVALID_DATA;
				}
				pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pElement->encData);
			}
			else
			{
				MemCopyByInt(pIter->_curBufPtr, &pElement->encData.data, pElement->encData.length ),
					pIter->_curBufPtr += pElement->encData.length;
			}
		}
		else /* blank */
		{
			if ((ret = _rsslEncodeBlank(pIter, pEncoding->dataType)) != RSSL_RET_SUCCESS) 
			{
				/* likely nothing to roll back but be safe */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				return ret;
			}
		}

		if (++_levelInfo->_currentCount < pDef->count)
			return RSSL_RET_SUCCESS;

		/* Set is complete. */
		if ((ret = _rsslCompleteElementSet(pIter, _levelInfo, pElementList)) < 0)
		{
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(ret);
		}
		
		return RSSL_RET_SET_COMPLETE;
	}

	/* Encoding standard entries. */
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	
	if (pElement->name.length > RWF_MAX_U15)
		return RSSL_RET_INVALID_DATA;

	if (pData)
	{
		RSSL_ASSERT(pElement->name.data, Missing element name);

		encSize = pElement->name.length + 2;
		if (pElement->name.length > 0x80)
			encSize++;

		if (_rsslIteratorOverrun(pIter, encSize))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		/* ensure name does not overrun rb15 */
		if (pElement->name.length > RWF_MAX_U15)
			return RSSL_RET_INVALID_DATA;
		/* store element name as buffer 15 */
		_levelInfo->_initElemStartPos = pIter->_curBufPtr;
		pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElement->name);

		/* store data type */
		pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElement->dataType);
		if (pElement->dataType != RSSL_DT_NO_DATA)
		{
			_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE;
			ret = _rsslEncodePrimitive(pIter, pElement->dataType, pData);
		}
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

		if (ret < 0)
		{
			/* rollback */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return ret;
		}
		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
	else if (pElement->encData.length && pElement->encData.data)
	{
		if (pElement->encData.length > RWF_MAX_16)
			return RSSL_RET_INVALID_DATA;

		encSize = pElement->name.length + pElement->encData.length + 3;
		if (pElement->name.length > 0x80)
			encSize++;
		if (pElement->encData.length >= 0xFE)
			encSize += 2;

		if (_rsslIteratorOverrun(pIter, encSize))
			return(RSSL_RET_BUFFER_TOO_SMALL);

		/* store element name as buffer 15 */
		pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElement->name);

		/* store datatype */
		pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElement->dataType);

		/* copy encoded data */
		if (pElement->dataType != RSSL_DT_NO_DATA)
			pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pElement->encData);

		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		RsslUInt8 zero = 0;
		encSize = pElement->name.length + 3;
		if (pElement->name.length > 0x80)
			encSize++;

		if (_rsslIteratorOverrun(pIter, encSize))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		/* ensure name does not overrun rb15 */
		if (pElement->name.length > RWF_MAX_U15)
			return RSSL_RET_INVALID_DATA;

		/* store element name as buffer 15 */
		pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElement->name);

		/* store datatype */
		pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElement->dataType);

		/* copy encoded data */
		if (pElement->dataType != RSSL_DT_NO_DATA)
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, zero);
		
		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
}

/* multi step element list encoding */

RSSL_API RsslRet rsslEncodeElementEntryInit(
							RsslEncodeIterator	*pIter,
							RsslElementEntry	*pElement,
							RsslUInt16			encodingMaxSize )
{
	RsslUInt32 encSize = 0;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	
	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ELEMENT_LIST, Invalid encoding attempted);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES || _levelInfo->_encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pElement && pElement->name.data, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	/* Set data. */
	if (_levelInfo->_encodingState == RSSL_EIS_SET_DATA)
	{
		const RsslElementSetDef *pDef = _levelInfo->_elemListSetDef;
		/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
		const RsslElementSetDefEntry *pEncoding = &pDef->pEntries[_levelInfo->_currentCount];

		RSSL_ASSERT(pDef, Invalid parameters or parameters passed in as NULL);

		RSSL_ASSERT(_rsslValidAggregateDataType(_levelInfo->_containerType), Invalid container type /* When encoding set data, this should never be used on anything but a container. */);
		if (!_rsslValidAggregateDataType(_levelInfo->_containerType))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_UNEXPECTED_ENCODER_CALL;
		}

		/* Validate name (if present) */
		if (pElement->name.data && !rsslBufferIsEqual(&pElement->name, &pEncoding->name))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_INVALID_DATA;
		}
  
		/* Validate type */
		if (pElement->dataType != _rsslPrimitiveType(pEncoding->dataType))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_INVALID_DATA;
		}

		if (_rsslIteratorOverrun(pIter, ((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_INIT;

		/* need to use mark 2 here because mark 1 is used to length specify 
		all the set data */
		pIter->_curBufPtr = _rsslSetupU16Mark(&_levelInfo->_internalMark2,
			encodingMaxSize, pIter->_curBufPtr);

		return RSSL_RET_SUCCESS;
	}

	/* Standard data. */
	/* name length, plus format, plus size */
	encSize = pElement->name.length + 2;
	if (pElement->name.length > 0x80)
		++encSize;

	if (encodingMaxSize == 0 || encodingMaxSize >= 0xFE)
		encSize += 3;
	else
		++encSize;

	if (_rsslIteratorOverrun(pIter, encSize))
	{
		_levelInfo->_encodingState = RSSL_EIS_ENTRY_WAIT_COMPLETE;
		return RSSL_RET_BUFFER_TOO_SMALL;
	}
	/* ensure name does not overrun rb15 */
	if (pElement->name.length > RWF_MAX_U15)
	{
		_levelInfo->_encodingState = RSSL_EIS_ENTRY_WAIT_COMPLETE;
		return RSSL_RET_INVALID_DATA;
	}

	/* store element name as buffer 15 */
	pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElement->name);
	/* store data type */
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElement->dataType);

	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;
	
	if (pElement->dataType != RSSL_DT_NO_DATA)
	{
		pIter->_curBufPtr = _rsslSetupU16Mark(&_levelInfo->_internalMark,
									encodingMaxSize, pIter->_curBufPtr);
	}
	else
	{
		_levelInfo->_internalMark._sizeBytes = 0;
		_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeElementEntryComplete(
										RsslEncodeIterator		*pIter,
										RsslBool				success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ELEMENT_LIST, Invalid encoding attempted);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_WAIT_COMPLETE || _levelInfo->_encodingState == RSSL_EIS_ENTRY_WAIT_COMPLETE || _levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	/* Set data. */
	if (_levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_WAIT_COMPLETE)
	{
		RsslElementList *pElementList = (RsslElementList*)_levelInfo->_listType;
		const RsslElementSetDef *pDef = _levelInfo->_elemListSetDef;
		if (success)
		{
			if (_levelInfo->_internalMark2._sizeBytes > 0)
			{
				/* need to use mark 2 here because mark 1 is used to length specify all the set data */
				if ((ret = _rsslFinishU16Mark(&_levelInfo->_internalMark2, pIter->_curBufPtr)) < 0)
				{
					/* roll back */
					pIter->_curBufPtr = _levelInfo->_initElemStartPos;
					_levelInfo->_initElemStartPos = 0;
					return(ret);
				}
			}
			_levelInfo->_initElemStartPos = 0;

			if (++_levelInfo->_currentCount < pDef->count)
			{
				_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
				return RSSL_RET_SUCCESS;
			}

			/* Set is complete. */
			if ((ret = _rsslCompleteElementSet(pIter, _levelInfo, pElementList)) < 0)
			{
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_initElemStartPos = 0;
				return(ret);
			}

			return RSSL_RET_SET_COMPLETE;
		}
		else
		{
			/* reset the pointer */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_initElemStartPos = 0;
			_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
			return RSSL_RET_SUCCESS;
		}
	}

	/* Standard data. */
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

RSSL_API RsslRet rsslEncodeElementListComplete(
									RsslEncodeIterator  *pIter,
									RsslBool			success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ELEMENT_LIST, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
				(_levelInfo->_encodingState == RSSL_EIS_SET_DATA) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{
		if (rsslElementListCheckHasStandardData((RsslElementList*)_levelInfo->_listType))
		{
			RSSL_ASSERT(_levelInfo->_countWritePtr != 0, Invalid encoding attempted);
			rwfPut16(_levelInfo->_countWritePtr, _levelInfo->_currentCount);
		}
	}
	else
		pIter->_curBufPtr = _levelInfo->_containerStartPos;

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}

/* Element list set definitions */
RSSL_API RsslRet rsslEncodeLocalElementSetDefDb(
													RsslEncodeIterator			*pIter, 
													RsslLocalElementSetDefDb	*pELSetDb )
{
	RsslUInt8	flags = 0;
	RsslUInt8	defCount;
	int			defs;
	RsslEncodingLevel *_levelInfo;

	/* assertions */
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);
	RSSL_ASSERT(pELSetDb, Invalid parameters or parameters passed in as NULL);

	if (pIter->_encodingLevel + 1 >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel+1];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_ELEMENT_LIST,RSSL_EIS_SET_DEFINITIONS,pELSetDb, pIter->_curBufPtr);

	/* make sure that required elements can be encoded */
	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* store flags as uint8 */
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, flags);

	_levelInfo->_countWritePtr = pIter->_curBufPtr; /* store count position */
	pIter->_curBufPtr++;		/* skip count byte */
	/* go through defs and encode them */
	defCount = 0;
	for (defs=0; defs<= RSSL_ELEMENT_SET_MAX_LOCAL_ID; defs++)
	{
		if (pELSetDb->definitions[defs].setId != RSSL_ELEMENT_SET_BLANK_ID)
		{
			int i;
			const RsslElementSetDef *pSetDef = &pELSetDb->definitions[defs];

			RSSL_ASSERT(pSetDef->setId == defs, Invalid set definition);

			/* make sure required elements fit */
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pSetDef->setId < 0x80) ? 2 : 3), pIter->_endBufPtr))
				return(RSSL_RET_BUFFER_TOO_SMALL);

			pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pSetDef->setId);
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pSetDef->count);

			for (i = 0; i< pSetDef->count; i++)
			{
				RsslElementSetDefEntry *pElementEnc = &pSetDef->pEntries[i];
				RSSL_ASSERT(pElementEnc, Invalid parameters or parameters passed in as NULL);

				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, (((pElementEnc->name.length < 0x80) ? 2 : 3) + pElementEnc->name.length), pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pElementEnc->name);
				pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pElementEnc->dataType);
			}
			++defCount;
		}
		
	}
	rwfPut8(_levelInfo->_countWritePtr, defCount);

	return RSSL_RET_SUCCESS;
}




