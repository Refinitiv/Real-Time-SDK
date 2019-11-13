/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslArray.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"

/* A table of Array entry encoding functions, for each valid combination of primitive type and item length. */
static RsslRet (RTR_FASTCALL *arrayEntryEncodingFunctions[RSSL_DT_RMTES_STRING+1][13])(rsslEncIterPtr,const void*) =
{
	/*              	0					1					2					3				4					5				6	7					8					9					10			11						12*/
	/* (0) */		{	0,					0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* (1) */		{	0,					0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* (2) */		{	0,					0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* INT */		{	_rsslEncInt,		_rsslEncodeInt1,	_rsslEncodeInt2,	0,				_rsslEncodeInt4,	0,				0,	0,					_rsslEncodeInt8,	0,					0,			0,						0},
	/* UINT */		{	_rsslEncUInt,		_rsslEncodeUInt1,	_rsslEncodeUInt2,	0,				_rsslEncodeUInt4,	0,				0,	0,					_rsslEncodeUInt8,	0,					0,			0,						0},
	/* FLOAT */		{	_rsslEncFloat,		0,					0,					0,				_rsslEncFloat_4,	0,				0,	0,					0,					0,					0,			0,						0},
   	/* DOUBLE */	{	_rsslEncDouble,		0,					0,					0,				0,					0,				0,	0,					_rsslEncDouble_8,	0,					0,			0,						0},
	/* (7) */		{	0,					0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* REAL */		{	_rsslEncReal,		0,					0,					0,				0,					0,				0,	0,					0, 					0,					0,			0,						0},
	/* DATE */		{	_rsslEncDate,		0,					0,					0,				_rsslEncDate_4,		0,				0,	0,					0, 					0,					0,			0,						0},
	/* TIME */		{	_rsslEncTime,		0,					0,					_rsslEncTime_3,	0,					_rsslEncTime_5,	0,	_rsslEncTime_7,		_rsslEncTime_8, 	0,					0,			0,						0},
	/* DATETIME */	{	_rsslEncDateTime,	0,					0,					0,				0,					0,				0,	_rsslEncDateTime_7,	0,					_rsslEncDateTime_9,	0,			_rsslEncDateTime_11,	_rsslEncDateTime_12},
	/* QOS */		{	_rsslEncQos,		0,					0,					0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* STATE */		{	_rsslEncState,		0,					0,					0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* ENUM */		{	_rsslEncEnum,		_rsslEncodeEnum1,	_rsslEncodeEnum2,	0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* (ARRAY) */	{	0,					0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* BUFFER */	{	_rsslEncBuffer,		0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* ASCII */		{	_rsslEncBuffer,		0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* UTF8 */		{	_rsslEncBuffer,		0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* RMTES */		{	_rsslEncBuffer,		0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
};

/* A table of Array entry encoding types, for each valid combination of primitive type and item length.
 * It is specifically for use with _rsslEncodeBlank(note that Enum does not have types for itemLength 1 & 2 because types like ENUM_1 and ENUM_2 don't exist.
 * They can't be encoded as blank though, so for purposes of this array this will work. */
static RsslDataTypes arrayEntryBlankTypes[RSSL_DT_RMTES_STRING+1][13] =
{
	/*              	0						1					2					3				4					5				6	7					8					9					10			11						 12*/
	/* (0) */		{	0,						0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* (1) */		{	0,						0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* (2) */		{	0,						0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* INT */		{	RSSL_DT_INT,			RSSL_DT_INT_1,		RSSL_DT_INT_2,		0,				RSSL_DT_INT_4,		0,				0,	0,					RSSL_DT_INT_8,		0,					0,			0,						0},
	/* UINT */		{	RSSL_DT_UINT,			RSSL_DT_UINT_1,		RSSL_DT_UINT_2,		0,				RSSL_DT_UINT_4,		0,				0,	0,					RSSL_DT_UINT_8,		0,					0,			0,						0},
	/* FLOAT */		{	RSSL_DT_FLOAT,			0,					0,					0,				RSSL_DT_FLOAT_4,	0,				0,	0,					0,					0,					0,			0,						0},
   	/* DOUBLE */	{	RSSL_DT_DOUBLE,			0,					0,					0,				0,					0,				0,	0,					RSSL_DT_DOUBLE_8,	0,					0,			0,						0},
	/* (7) */		{	0,						0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* REAL */		{	RSSL_DT_REAL,			0,					0,					0,				0,					0,				0,	0,					0, 					0,					0,			0,						0},
	/* DATE */		{	RSSL_DT_DATE,			0,					0,					0,				RSSL_DT_DATE_4,		0,				0,	0,					0, 					0,					0,			0,						0},
	/* TIME */		{	RSSL_DT_TIME,			0,					0,					RSSL_DT_TIME_3,	0,					RSSL_DT_TIME_5,	0,	RSSL_DT_TIME_7, 	RSSL_DT_TIME_8,		0,					0,			0,						0},
	/* DATETIME */	{	RSSL_DT_DATETIME,		0,					0,					0,				0,					0,				0,	RSSL_DT_DATETIME_7,	0,					RSSL_DT_DATETIME_9,	0,	RSSL_DT_DATETIME_11,	RSSL_DT_DATETIME_12},
	/* QOS */		{	RSSL_DT_QOS,			0,					0,					0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* STATE */		{	RSSL_DT_STATE,			0,					0,					0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* ENUM */		{	RSSL_DT_ENUM,			0,					0,					0,				0,					0,				0,	0, 					0,					0,					0,			0,						0},
	/* (ARRAY) */	{	0,						0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* BUFFER */	{	RSSL_DT_BUFFER,			0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* ASCII */		{	RSSL_DT_ASCII_STRING,	0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* UTF8 */		{	RSSL_DT_UTF8_STRING,	0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
	/* RMTES */		{	RSSL_DT_RMTES_STRING,	0,					0,					0,				0,					0,				0,	0,					0,					0,					0,			0,						0},
};

RSSL_API RsslRet rsslEncodeArrayInit( 
						    RsslEncodeIterator	*pIter,
							RsslArray			*pArray )
{
	RsslEncodingLevel *_levelInfo;

	/* Validations */
	RSSL_ASSERT(pIter && pArray, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_curBufPtr, Invalid iterator use - check buffer);
	if ((pArray->primitiveType >= RSSL_DT_SET_PRIMITIVE_MIN) || (pArray->primitiveType == RSSL_DT_ARRAY) || (pArray->primitiveType == RSSL_DT_UNKNOWN))
		return RSSL_RET_UNSUPPORTED_DATA_TYPE;

	_levelInfo = &pIter->_levelInfo[++pIter->_encodingLevel]; if (pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslInitEncodeIterator(_levelInfo,RSSL_DT_ARRAY,RSSL_EIS_NONE,pArray, pIter->_curBufPtr);
	
	/* make sure required elements can fit */
	/* data type (1), count (2) length (3)*/
	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 6, pIter->_endBufPtr))
	{
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return(RSSL_RET_BUFFER_TOO_SMALL);
	}

	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pArray->primitiveType);

	pIter->_curBufPtr += rwfPutOptByteU16(pIter->_curBufPtr, pArray->itemLength);

	/* save count position and increment iterator */
	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += 2;

	/* change encoding state */
	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
   
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeArrayEntry( RsslEncodeIterator *pIter, const RsslBuffer *pEncData, const void* pData)
{
	RsslArray *array;
	RsslRet ret = 0;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ARRAY, Invalid encoding attempted - wrong type);
	RSSL_ASSERT(_levelInfo->_encodingState == RSSL_EIS_ENTRIES, Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_curBufPtr != 0, Invalid encoding attempted);
	
	array = (RsslArray*)_levelInfo->_listType;

	/* set start position for this entry - needed for rollback */
	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	if (pData)
	{
		RsslRet (RTR_FASTCALL *arrayEntryEncodingFunc)(rsslEncIterPtr,const void*);

		if (!array->itemLength)
		{
			/* Standard data encoding, use the appropriate encode function for this type(if one exists) */
			if (array->primitiveType <= RSSL_DT_RMTES_STRING
					&& (arrayEntryEncodingFunc = arrayEntryEncodingFunctions[array->primitiveType][0]))
			{
				_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE;
				ret = (*arrayEntryEncodingFunc)( pIter, pData );
			}
			else
				return RSSL_RET_INVALID_ARGUMENT;
		}
		else
		{
			/* Set data encoding, use the appropriate set encode function for this type and item length(if one exists) */
			switch(array->primitiveType)
			{
			case RSSL_DT_BUFFER:
			case RSSL_DT_ASCII_STRING:
			case RSSL_DT_UTF8_STRING:
			case RSSL_DT_RMTES_STRING:
				{
					RsslUInt32 lengthDiff;
					/* The buffer/string types are allowed to be any length */
					RsslBuffer *pBuffer = (RsslBuffer*)pData;

					/* Buffer should not be longer than itemLength */
					RSSL_ASSERT(pBuffer->length <= array->itemLength, length of input buffer greater than array->itemLength);

					if (_rsslIteratorOverrun(pIter,array->itemLength))
						return(RSSL_RET_BUFFER_TOO_SMALL);

					/* Buffer CAN be shorter than itemLength.  Make sure we don't copy more bytes than necessary,
					 * or we risk going to out-of-bounds memory. */
					MemCopyByInt(pIter->_curBufPtr, pBuffer->data, ((array->itemLength > pBuffer->length) ? pBuffer->length : array->itemLength));
					/* we should fill in the gap afterwards */
					if (pBuffer->length < array->itemLength)
					{
						lengthDiff = array->itemLength - pBuffer->length;
						pIter->_curBufPtr += pBuffer->length;
						while (lengthDiff)
						{
							pIter->_curBufPtr[0] = '\0';
							--lengthDiff;
						}
					}
					else		
						pIter->_curBufPtr += array->itemLength;

					ret = RSSL_RET_SUCCESS;
				}
				break;

			default:
				/* Use the appropriate primitive encode function, if one exists for this type and itemLength */
				if (array->primitiveType <= RSSL_DT_RMTES_STRING
						&& array->itemLength <= 12 
						&& (arrayEntryEncodingFunc = arrayEntryEncodingFunctions[array->primitiveType][array->itemLength]))
				{
					_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					ret = (*arrayEntryEncodingFunc)( pIter, pData );
					break;
				}
				else
					return RSSL_RET_INVALID_ARGUMENT;
			}
		}

		if (ret >= RSSL_RET_SUCCESS)
			_levelInfo->_currentCount++;
		else
		{
			/* failure - roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_initElemStartPos = 0;
		}

		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

		return ret;
	}
	else if(pEncData)
	{
		if (pEncData->length && pEncData->data)
		{
			if (array->itemLength)
			{
				/* fixed length items */
				/* check that the length given to us is what we expect */
				RSSL_ASSERT(array->itemLength == pEncData->length, Invalid encoded data length);

				if (_rsslIteratorOverrun(pIter,array->itemLength))
					return(RSSL_RET_BUFFER_TOO_SMALL);

				MemCopyByInt(pIter->_curBufPtr, pEncData->data, array->itemLength);
				pIter->_curBufPtr += array->itemLength;
			}
			else
			{
				if (_rsslIteratorOverrun(pIter,(2+pEncData->length)))
					return(RSSL_RET_BUFFER_TOO_SMALL);
				/* ensure length is not overrun */
				if (pEncData->length > RWF_MAX_16)
					return RSSL_RET_INVALID_DATA;
				/* length specified data, just encode it as a buffer */
				pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, pEncData );
			}
		}
		else
		{
			/* Blank */

			if (array->itemLength)
			{
				RsslDataTypes arrayEntryBlankType;
				if (array->primitiveType <= RSSL_DT_RMTES_STRING
						&& array->itemLength <= 9 
						&& (arrayEntryBlankType = arrayEntryBlankTypes[array->primitiveType][array->itemLength]))
				{
					_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
					if ((ret = _rsslEncodeBlank(pIter, arrayEntryBlankType)) != RSSL_RET_SUCCESS) 
						return ret;
				}
				else
					return RSSL_RET_INVALID_ARGUMENT;
			}
			else
			{
				_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE;
				if ((ret = _rsslEncodeBlank(pIter, array->primitiveType)) != RSSL_RET_SUCCESS) 
					return ret;
			}
		}

		_levelInfo->_currentCount++;
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		/* blank via NULL pEncData and pData */
		if (array->itemLength)
		{
			RsslDataTypes arrayEntryBlankType;
			if (array->primitiveType <= RSSL_DT_RMTES_STRING
					&& array->itemLength <= 9 
					&& (arrayEntryBlankType = arrayEntryBlankTypes[array->primitiveType][array->itemLength]))
			{
				_levelInfo->_encodingState = RSSL_EIS_SET_DATA;
				if ((ret = _rsslEncodeBlank(pIter, arrayEntryBlankType)) != RSSL_RET_SUCCESS) 
					return ret;
			}
			else
				return RSSL_RET_INVALID_ARGUMENT;
		}
		else
		{
			_levelInfo->_encodingState = RSSL_EIS_PRIMITIVE;
			if ((ret = _rsslEncodeBlank(pIter, array->primitiveType)) != RSSL_RET_SUCCESS) 
				return ret;
		}
		
		_levelInfo->_currentCount++;
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;
		return RSSL_RET_SUCCESS;
	}
}

RSSL_API RsslRet rsslEncodeArrayComplete( 
								RsslEncodeIterator	*pIter,
								RsslBool			success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RSSL_ASSERT(pIter && _levelInfo->_listType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_containerType == RSSL_DT_ARRAY, Invalid encoding attempted - wrong type);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
			(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);

	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if (success)
	{
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



