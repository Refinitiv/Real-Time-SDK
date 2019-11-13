/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_ENCODER_TOOLS_H
#define __RSSL_ENCODER_TOOLS_H

#include "rtr/retmacros.h"
#include "rtr/rwfNetwork.h"
#include "rtr/rsslIterators.h"
#include "rtr/intDataTypes.h"
#include "rtr/custmem.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslReal.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define _rsslIteratorOverrun( pIter, length ) \
	(((pIter)->_curBufPtr + length) > (pIter)->_endBufPtr)

RTR_C_INLINE char* _rsslSetupU15Mark(RsslEncodeSizeMark	*pMark,
									RsslUInt16			maxSize,
									char				*position )
									{
	RSSL_ASSERT(position != 0, Invalid encoding attempted);
	RSSL_ASSERT(pMark && pMark->_sizePtr == 0, Invalid encoding attempted);

	pMark->_sizePtr = position;
	if ((maxSize == 0) || (maxSize >= 0x80))
	{
		pMark->_sizeBytes = 2;
		position += 2;
	}
	else
	{
		pMark->_sizeBytes = 1;
		position += 1;
	}
	return (position);
}

RTR_C_INLINE RsslRet _rsslFinishU15Mark(	RsslEncodeSizeMark	*pMark,
									char				*position )
{
	RsslUInt16 dataLength;

	RSSL_ASSERT(pMark, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pMark->_sizePtr != 0, Invalid encoding attempted);
	RSSL_ASSERT(pMark->_sizeBytes != 0, Invalid encoding attempted);
	RSSL_ASSERT(position != 0, Invalid encoding attempted);

	dataLength = (RsslUInt16)(position - pMark->_sizePtr - pMark->_sizeBytes);

	if (dataLength > RWF_MAX_U15)
		return RSSL_RET_INVALID_DATA;

	if (pMark->_sizeBytes == 1)
	{
		RsslUInt8 tmp8 = (RsslUInt8)dataLength;
		if (dataLength >= 0x80)
			return RSSL_RET_INVALID_DATA;
		rwfPut8( pMark->_sizePtr, tmp8 );
	}
	else
	{
		RSSL_ASSERT(pMark->_sizeBytes == 2, Invalid encoding attempted);
		dataLength |= 0x8000;
		rwfPut16( pMark->_sizePtr, dataLength);
	}
	pMark->_sizePtr = 0;
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE char* _rsslSetupU16Mark(RsslEncodeSizeMark	*pMark,
									RsslUInt16			maxSize,
									char				*position )
{
	RSSL_ASSERT(position != 0, Invalid encoding attempted);
	RSSL_ASSERT(pMark && pMark->_sizePtr == 0, Invalid encoding attempted);

	pMark->_sizePtr = position;
	if ((maxSize == 0) || (maxSize >= 0xFE))
	{
		pMark->_sizeBytes = 3;
		position += 3;
	}
	else
	{
		pMark->_sizeBytes = 1;
		position += 1;
	}
	return (position);
}

RTR_C_INLINE RsslRet _rsslFinishU16Mark(	RsslEncodeSizeMark	*pMark,
									char				*position )
{
	RsslUInt32 dataLength;

	RSSL_ASSERT(pMark, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pMark->_sizePtr != 0, Invalid encoding attempted);
	RSSL_ASSERT(pMark->_sizeBytes != 0, Invalid encoding attempted);
	RSSL_ASSERT(position != 0, Invalid encoding attempted);

	dataLength = (RsslUInt32)(position - pMark->_sizePtr - pMark->_sizeBytes);

	if (pMark->_sizeBytes == 1)
	{
		RsslUInt8 tmp8 = (RsslUInt8)dataLength;
		if (dataLength >= 0xFE)
			return RSSL_RET_INVALID_DATA;
		rwfPut8( pMark->_sizePtr, tmp8 );
	}
	else
	{
		RsslUInt16 dl = (RsslUInt16)dataLength;
		RSSL_ASSERT(pMark->_sizeBytes == 3, Invalid encoding attempted);

		if (dataLength > RWF_MAX_16)
			return RSSL_RET_INVALID_DATA;

		*(pMark->_sizePtr) = (char)0xFE;
		rwfPut16( (pMark->_sizePtr + 1), dl);
	}
	pMark->_sizePtr = 0;
	return RSSL_RET_SUCCESS;
}


RTR_C_INLINE char * _rsslEncodeBuffer8(char *position, const RsslBuffer * buffer)
{
	RsslUInt8 tlen = (RsslUInt8)buffer->length;
	position += rwfPut8(position, tlen);
	MemCopyByInt(position, buffer->data, tlen);
	position += tlen;
	return position;
}

RTR_C_INLINE char * _rsslEncodeBuffer15(char *position, const RsslBuffer * buffer)
{
	RsslUInt16 tlen = (RsslUInt16)buffer->length;
	position += rwfPutResBitU15( position, tlen);
	MemCopyByInt( position, buffer->data, tlen );
	position += tlen;
	return position;
}

RTR_C_INLINE char * _rsslEncodeBuffer16(char *position, const RsslBuffer * buffer)
{
	RsslUInt16 tlen = (RsslUInt16)buffer->length;
	position += rwfPutOptByteU16( position, tlen );
	MemCopyByInt( position, buffer->data, tlen );
	position += tlen;
	return position;
}

RTR_C_INLINE RsslRet _rsslEncodeBlank(RsslEncodeIterator *pIter, const RsslDataType type)
{
	switch(type)
	{
		case RSSL_DT_REAL_4RB:
		{
			RsslReal blankReal = RSSL_BLANK_REAL;
			return _rsslEncReal_4rb(pIter, &blankReal);
		}
		case RSSL_DT_REAL_8RB:
		{
			RsslReal blankReal = RSSL_BLANK_REAL;
			return _rsslEncReal_8rb(pIter, &blankReal);
		}
		case RSSL_DT_DATE_4:
		{
			RsslDate blankDate = RSSL_BLANK_DATE;
			return _rsslEncDate_4(pIter, &blankDate);
		}
		case RSSL_DT_TIME_3:
		{
			RsslTime blankTime = RSSL_BLANK_TIME;
			return _rsslEncTime_3(pIter, &blankTime);
		}
		case RSSL_DT_TIME_5:
		{
			RsslTime blankTime = RSSL_BLANK_TIME;
			return _rsslEncTime_5(pIter, &blankTime);
		}
		case RSSL_DT_TIME_7:
		{
			RsslTime blankTime = RSSL_BLANK_TIME;
			return _rsslEncTime_7(pIter, &blankTime);
		}
		case RSSL_DT_TIME_8:
		{
			RsslTime blankTime = RSSL_BLANK_TIME;
			return _rsslEncTime_8(pIter, &blankTime);
		}
		case RSSL_DT_DATETIME_7:
		{
			RsslDateTime blankDateTime = RSSL_BLANK_DATETIME;
			return _rsslEncDateTime_7(pIter, &blankDateTime);
		}
		case RSSL_DT_DATETIME_9:
		{
			RsslDateTime blankDateTime = RSSL_BLANK_DATETIME;
			return _rsslEncDateTime_9(pIter, &blankDateTime);
		}
		case RSSL_DT_DATETIME_11:
		{
			RsslDateTime blankDateTime = RSSL_BLANK_DATETIME;
			return _rsslEncDateTime_11(pIter, &blankDateTime);
		}
		case RSSL_DT_DATETIME_12:
		{
			RsslDateTime blankDateTime = RSSL_BLANK_DATETIME;
			return _rsslEncDateTime_12(pIter, &blankDateTime);
		}

		default:
		{
			RsslUInt8 zero = 0;

			/* Except for the above, only length-spec primitives can be blank. */
			if (type > RSSL_DT_BASE_PRIMITIVE_MAX)
				return RSSL_RET_INVALID_ARGUMENT;

			if (_rsslIteratorOverrun(pIter, 1))
				return(RSSL_RET_BUFFER_TOO_SMALL);

			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, zero);
			return RSSL_RET_SUCCESS;
		}
	}
}

/* this function assumes the buffer size check has already been done */
RTR_C_INLINE char * _rsslEncodeCopyData(
						char				*position,
						const RsslBuffer	*pBuf )
{
	MemCopyByInt( position, pBuf->data, pBuf->length );
	position += pBuf->length;

	return position;
}


#ifdef __cplusplus
}
#endif 



#endif /* __RSSL_ENCODER_TOOLS_H */
