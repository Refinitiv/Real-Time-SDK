/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/rsslFieldList.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"

RsslRet RTR_FASTCALL _rsslCompleteFieldSet(RsslEncodeIterator *pIter, RsslEncodingLevel *_levelInfo, RsslFieldList *pFieldList);


RSSL_API RsslRet rsslEncodeFieldListInit(
				RsslEncodeIterator	*pIter,
				RsslFieldList		*pFieldList,
				const RsslLocalFieldSetDefDb	*pSetDb,
				RsslUInt16			setEncodingMaxSize )
{
	RsslEncodingLevel *_levelInfo;
	RsslUInt16 setId;

	/* Assertions */
	RSSL_ASSERT(pFieldList && pIter, Invalid parameters or parameters passed in as NULL);

	_levelInfo = &pIter->_levelInfo[++pIter->_encodingLevel]; if (pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_FIELD_LIST,RSSL_EIS_NONE,pFieldList, pIter->_curBufPtr);

	/* Make sure that required elements can be encoded */
	if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 1, pIter->_endBufPtr ))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	/* Store flags as UInt8 */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pFieldList->flags);

	/* Check for field list info */
	if (rsslFieldListCheckHasInfo(pFieldList))
	{
		RsslUInt8	infoLen;
		char		*startPos;

		/* Make sure that required elements can be encoded */
		if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 5, pIter->_endBufPtr ))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		/* Save info length position */
		startPos = pIter->_curBufPtr;

		/* Skip info length so it can be encoded later */
		pIter->_curBufPtr += 1;

		/* Put dictionary id and field list number (range check first) */
		if (pFieldList->dictionaryId > 16383 || pFieldList->dictionaryId < -16384)
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return RSSL_RET_INVALID_DATA;
		}
		pIter->_curBufPtr += rwfPutResBitI15( pIter->_curBufPtr, pFieldList->dictionaryId );
		pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pFieldList->fieldListNum );

			/* encode the field list info length */
		infoLen = (RsslUInt8)(pIter->_curBufPtr - startPos - 1);
		rwfPut8(startPos, infoLen);
	}

	/* Check for set data */
	if (rsslFieldListCheckHasSetData(pFieldList))
	{
		/* We have Field List Set Data */
		/* Make sure that the setid and set length can be encoded.
		 * Always check worst case. SetId (2) SetDataLen (2) */
		if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 4, pIter->_endBufPtr ))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		/* Check for/encode optional Field List Set Id */
		if (rsslFieldListCheckHasSetId(pFieldList))
		{
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, pFieldList->setId );
			setId = pFieldList->setId;
		}
		else
			setId = 0;

		if ( pSetDb && setId <= RSSL_FIELD_SET_MAX_LOCAL_ID && pSetDb->definitions[setId].setId != RSSL_ELEMENT_SET_BLANK_ID)
			_levelInfo->_fieldListSetDef = &pSetDb->definitions[setId];
		else 
		if(pIter->_pGlobalFieldListSetDb && setId > RSSL_FIELD_SET_MAX_LOCAL_ID && pIter->_pGlobalFieldListSetDb->definitions[setId] != NULL)
			_levelInfo->_fieldListSetDef = pIter->_pGlobalFieldListSetDb->definitions[setId];


		/* Check for Field List Data after the Set Data */
		if (rsslFieldListCheckHasStandardData(pFieldList))
		{
			/* If have set data and field list data, need length in
			 * front of the set */
			if (pFieldList->encSetData.data != 0)
			{
				/* The set data is already encoded. */
				/* Make sure that the set data can be encoded. The length was
				 * included in the previous check.
				 */
				if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr,
									pFieldList->encSetData.length + 2, pIter->_endBufPtr ))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}

				/* ensure it doesnt overrun rb15 */
				if (pFieldList->encSetData.length > RWF_MAX_U15)
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return RSSL_RET_INVALID_DATA;
				}

				pIter->_curBufPtr = _rsslEncodeBuffer15( pIter->_curBufPtr, &pFieldList->encSetData );

				/* Save bytes for field list data count */
				_levelInfo->_countWritePtr = pIter->_curBufPtr;
				_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
				pIter->_curBufPtr += 2;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* The set data need to be encoded. */
				/* Save state and return */
				if (_levelInfo->_fieldListSetDef)
				{
					pIter->_curBufPtr = _rsslSetupU15Mark( &_levelInfo->_internalMark,
						setEncodingMaxSize, pIter->_curBufPtr );

					/* If the set actually has no entries, just complete the set. */
					if (_levelInfo->_fieldListSetDef->count > 0)
						_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					else _rsslCompleteFieldSet(pIter, _levelInfo, pFieldList);

					return RSSL_RET_SUCCESS;
				}
				else
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_SET_DEF_NOT_PROVIDED);
				}
			}
		}
		else
		{
			/* Don't need a length in front of set data. */
			/* Encode set data if it exists */
			if (pFieldList->encSetData.data != 0)
			{
				/* Make sure that the set data can be encoded. */
				if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr,
									pFieldList->encSetData.length, pIter->_endBufPtr ))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_BUFFER_TOO_SMALL);
				}

				/* Don't need a length in front of set data. */
				pIter->_curBufPtr = _rsslEncodeCopyData( pIter->_curBufPtr, &pFieldList->encSetData );

				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* Don't need a length in front of set data. */
				/* Save the size pointer in case we need to rewind */
				if (_levelInfo->_fieldListSetDef)
				{
					_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
					_levelInfo->_internalMark._sizeBytes = 0;
					
					/* If the set actually has no entries, just complete the set. */
					if (_levelInfo->_fieldListSetDef->count > 0)
						_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					else
						_rsslCompleteFieldSet(pIter, _levelInfo, pFieldList);
					return RSSL_RET_SUCCESS;
				}
				else
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return(RSSL_RET_SET_DEF_NOT_PROVIDED);
				}
			}
		}
	}
	else if (rsslFieldListCheckHasStandardData(pFieldList))
	{
		/* We only have field list data */
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
		{
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}

		_levelInfo->_countWritePtr = pIter->_curBufPtr;
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
		pIter->_curBufPtr += 2;
	}

	if (_levelInfo->_encodingState == RSSL_EIS_NONE)
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeFieldListComplete(
				RsslEncodeIterator	*pIter,
				RsslBool			success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_FIELD_LIST, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
				(_levelInfo->_encodingState == RSSL_EIS_SET_DATA) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{
		if (_levelInfo->_encodingState == RSSL_EIS_ENTRIES)
		{
			RSSL_ASSERT(_levelInfo->_countWritePtr != 0, Invalid encoding attempted);
			rwfPut16(_levelInfo->_countWritePtr, _levelInfo->_currentCount);
		}
	}
	else
	{
		pIter->_curBufPtr = _levelInfo->_containerStartPos;
	}

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslCompleteFieldSet(RsslEncodeIterator *pIter, RsslEncodingLevel *_levelInfo, RsslFieldList *pFieldList)
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

		if (rsslFieldListCheckHasStandardData(pFieldList))
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

/* Simple, one-step, Field List Field encoding */

RSSL_API RsslRet rsslEncodeFieldEntry(
				RsslEncodeIterator	*pIter,
				RsslFieldEntry		*pField,
				const void			*pData )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pField, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_FIELD_LIST, Invalid encoding attempted - wrong container);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES || _levelInfo->_encodingState == RSSL_EIS_SET_DATA , Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if (_levelInfo->_encodingState == RSSL_EIS_SET_DATA)
	{
		RsslFieldList *pFieldList = (RsslFieldList*)_levelInfo->_listType;
		const RsslFieldSetDef *pDef = _levelInfo->_fieldListSetDef;

		/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
		const RsslFieldSetDefEntry *pEncoding = &pDef->pEntries[_levelInfo->_currentCount];

		RSSL_ASSERT(pDef, Invalid parameters or parameters passed in as NULL);

		/* Validate fid */
		if (pField->fieldId != pEncoding->fieldId)
			return RSSL_RET_INVALID_DATA;
  
		/* Encode item according to set type */
		if (pData)
		{
			/* Validate type - only restrict to primitive when void* is passed in */
			if (pField->dataType != _rsslPrimitiveType(pEncoding->dataType))
				return RSSL_RET_INVALID_DATA;

			if ((ret = _rsslEncodeSet(pIter, pEncoding->dataType, pData)) < 0)
			{
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				return ret;
			}
		}
		else if (pField->encData.length && pField->encData.data)/* Encoding pre-encoded data from the field entry */
		{
			if (_rsslIteratorOverrun(pIter, (pField->encData.length + 3))) /* Account for ob16 length byte */
			{
				/* nothing to roll back */
				return(RSSL_RET_BUFFER_TOO_SMALL);
			}

			/* if the dataType is primitive we need to make sure the data is length specified */
			if ((pField->dataType < RSSL_DT_SET_PRIMITIVE_MIN) || (pField->dataType > RSSL_DT_CONTAINER_TYPE_MIN)) 
			{
				/* ensure it does not overrun buffer 16 length */
				if (pField->encData.length > RWF_MAX_16)
				{
					/* nothing to roll back */
					return RSSL_RET_INVALID_DATA;
				}
				pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pField->encData);
			}
			else
			{
				MemCopyByInt(pIter->_curBufPtr, &pField->encData.data, pField->encData.length ),
					pIter->_curBufPtr += pField->encData.length;
			}
		}
		else /* blank */
		{
			if ((ret = _rsslEncodeBlank(pIter, pEncoding->dataType)) != RSSL_RET_SUCCESS) 
			{
				/* possibly nothing to roll back, just to be safe roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				return ret;
			}
		}

		if (++_levelInfo->_currentCount < pDef->count)
			return RSSL_RET_SUCCESS;

		if ((ret = _rsslCompleteFieldSet(pIter, _levelInfo, pFieldList)) < 0)
		{
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(ret);
		}

		return RSSL_RET_SET_COMPLETE;
	}

	if (pData)
	{
		if (_rsslIteratorOverrun(pIter, 2))
			return(RSSL_RET_BUFFER_TOO_SMALL);

		/* Store FieldId as Uint16 */
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pField->fieldId);

		/* Encode the data type */
		_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE;
		ret = _rsslEncodePrimitive( pIter, pField->dataType, pData );
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

		if (ret < 0)
		{
			pIter->_curBufPtr -= 2; /* Rollback field Id */
			return ret;
		}

		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
	else if (pField->encData.length && pField->encData.data)/* Encoding pre-encoded data from the field entry */
	{
		RsslUInt32 encSize = 0;

		if (pField->encData.length > RWF_MAX_16)
			return RSSL_RET_INVALID_DATA;

		encSize = pField->encData.length + 3;
		if (pField->encData.length >= 0xFE)
			encSize += 2;

		if (_rsslIteratorOverrun(pIter, encSize))
			return(RSSL_RET_BUFFER_TOO_SMALL);

		/* Store FieldId as Uint16 */
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pField->fieldId);

		pIter->_curBufPtr = _rsslEncodeBuffer16( pIter->_curBufPtr, &pField->encData );

		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
	else /* Encoding as blank */
	{
		RsslUInt8 zero = 0;
		if (_rsslIteratorOverrun(pIter, 3))
			return(RSSL_RET_BUFFER_TOO_SMALL);

				/* Store FieldId as Uint16 */
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pField->fieldId);

		pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, zero);
		
		_levelInfo->_currentCount++;
		return RSSL_RET_SUCCESS;
	}
}

/* Multi-step field list encoding */

RSSL_API RsslRet rsslEncodeFieldEntryInit( 
				RsslEncodeIterator	*pIter,
				RsslFieldEntry		*pField,
				RsslUInt16			encodingMaxSize )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pField, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_FIELD_LIST, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES || _levelInfo->_encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);
	
	_levelInfo->_initElemStartPos = pIter->_curBufPtr;
	
	/* Set data */
	if (_levelInfo->_encodingState == RSSL_EIS_SET_DATA)
	{
		const RsslFieldSetDef *pDef = _levelInfo->_fieldListSetDef;
		/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
		const RsslFieldSetDefEntry *pEncoding = &pDef->pEntries[_levelInfo->_currentCount];

		RSSL_ASSERT(pDef, Invalid parameters or parameters passed in as NULL);

		RSSL_ASSERT(_rsslValidAggregateDataType(_levelInfo->_containerType), Invalid container type /* When encoding set data, this should never be used on anything but a container. */);
		if (!_rsslValidAggregateDataType(_levelInfo->_containerType))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_UNEXPECTED_ENCODER_CALL;
		}

		/* Validate fid */
		if (pField->fieldId != pEncoding->fieldId)
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_INVALID_DATA;
		}
  
		/* Validate type */
		if (pField->dataType != _rsslPrimitiveType(pEncoding->dataType))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return RSSL_RET_INVALID_DATA;
		}

		if (_rsslIteratorOverrun(pIter, ((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
		{
			_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_WAIT_COMPLETE;
			return(RSSL_RET_BUFFER_TOO_SMALL);
		}

		_levelInfo->_encodingState = RSSL_EIS_SET_ENTRY_INIT;

		/* need to use internal mark 2 here because mark 1 is used to length specify set data */
		pIter->_curBufPtr = _rsslSetupU16Mark( &_levelInfo->_internalMark2,
			encodingMaxSize, pIter->_curBufPtr);

		return RSSL_RET_SUCCESS;
	}

	/* Standard data */
	if (_rsslIteratorOverrun(pIter, (3 + ((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 2 : 0))))
	{
		_levelInfo->_encodingState = RSSL_EIS_ENTRY_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}


	/* Store FieldId as Uint16 */
	pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pField->fieldId);
	_levelInfo->_encodingState = RSSL_EIS_ENTRY_INIT;

	pIter->_curBufPtr = _rsslSetupU16Mark( &_levelInfo->_internalMark,
							encodingMaxSize, pIter->_curBufPtr);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeFieldEntryComplete( 
				RsslEncodeIterator	*pIter,
				RsslBool			success )
{
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_FIELD_LIST, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_ENTRY_WAIT_COMPLETE || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_WAIT_COMPLETE || _levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT((_levelInfo->_currentCount + 1) != 0, Invalid encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	/* Set data (user was encoding a container in the set) */
	if(_levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_INIT || _levelInfo->_encodingState == RSSL_EIS_SET_ENTRY_WAIT_COMPLETE)
	{
		RsslFieldList *pFieldList = (RsslFieldList*)_levelInfo->_listType;
		const RsslFieldSetDef *pDef = _levelInfo->_fieldListSetDef;
		if (success)
		{
			/* need to use internal mark 2 here because mark 1 is used
			to length specify set data */
			if ((ret = _rsslFinishU16Mark(&_levelInfo->_internalMark2, pIter->_curBufPtr)) < 0)
			{
				/* roll back */
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_initElemStartPos = 0;
				return(ret);
			}
			_levelInfo->_initElemStartPos = 0;

			if (++_levelInfo->_currentCount < pDef->count)
			{
				_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
				return RSSL_RET_SUCCESS;
			}

			/* Set is complete. */
			if ((ret = _rsslCompleteFieldSet(pIter, _levelInfo, pFieldList)) < 0)
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
			/* Reset the pointer */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_initElemStartPos = 0;
			_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
			return RSSL_RET_SUCCESS;
		}
	}

	/* Standard data. */
	if (success)
	{
		if ((ret = _rsslFinishU16Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
		{
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_initElemStartPos = 0;
			return(ret);
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

/******************************/
/* Field List set Definitions */

RSSL_API RsslRet rsslEncodeLocalFieldSetDefDb(
				RsslEncodeIterator			*pIter,
				RsslLocalFieldSetDefDb		*pFLSetDb )
{
	RsslUInt8	flags = 0;
	RsslUInt8	defCount;
	int			defs;
	RsslEncodingLevel *_levelInfo;

	/* assertions */
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);
	RSSL_ASSERT(pFLSetDb, Invalid parameters or parameters passed in as NULL);

	if (pIter->_encodingLevel + 1>= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel+1];
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_FIELD_LIST,RSSL_EIS_SET_DEFINITIONS,pFLSetDb, pIter->_curBufPtr);

	/* make sure that required elements can be encoded */
	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 2, pIter->_endBufPtr))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* store flags as uint8 */
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, flags);

	_levelInfo->_countWritePtr = pIter->_curBufPtr; /* store count position */
	pIter->_curBufPtr++;		/* skip count byte */
	/* go through defs and encode them */
	defCount = 0;
	for (defs=0; defs<= RSSL_FIELD_SET_MAX_LOCAL_ID; defs++)
	{
		if (pFLSetDb->definitions[defs].setId != RSSL_FIELD_SET_BLANK_ID)
		{
			int i;
			const RsslFieldSetDef *pSetDef = &pFLSetDb->definitions[defs];

			RSSL_ASSERT(pSetDef->setId == defs, Invalid set definition);

			/* Make sure that required elements can be encoded */
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pSetDef->setId < 0x80) ? 3 : 2), pIter->_endBufPtr ))
				return(RSSL_RET_BUFFER_TOO_SMALL);

			pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pSetDef->setId);
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pSetDef->count);

			/* Make sure that the set field encodings can be encoded. */
			if (_rsslBufferOverrunEndPtr( pIter->_curBufPtr, 3 * pSetDef->count, pIter->_endBufPtr ))
				return(RSSL_RET_BUFFER_TOO_SMALL);

			for (i=0; i<pSetDef->count; i++)
			{
				RsslFieldSetDefEntry	*pFieldEnc = &pSetDef->pEntries[i];
				RSSL_ASSERT(pFieldEnc, Invalid parameters or parameters passed in as NULL);
				pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pFieldEnc->fieldId);
				pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pFieldEnc->dataType);
			}
			++defCount;
		}
	}

	rwfPut8(_levelInfo->_countWritePtr, defCount);
	return RSSL_RET_SUCCESS;
}
