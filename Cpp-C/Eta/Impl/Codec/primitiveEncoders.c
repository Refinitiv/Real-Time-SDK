/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <string.h>

#include "rtr/rsslPrimitiveEncoders.h"
#include "rtr/rwfNet.h"
#include "rtr/encoderTools.h"
#include "rtr/intDataTypes.h"
#include "rtr/rsslIteratorUtilsInt.h"

static RsslUInt8 _rssl1Length = 1;
static RsslUInt8 _rssl1FloatLength = __RSZFLT;
static RsslUInt8 _rssl1DoubleLength = __RSZDBL;
static RsslUInt8 _rssl1DateLen = __RSZDT;
static RsslUInt8 _rssl1TimeLen = __RSZTM;
static RsslUInt8 _rssl1DateTimeLen = __RSZDT + __RSZTM;

RSSL_API RsslRet rsslRealignEncodeIteratorBuffer(
						RsslEncodeIterator	*pIter,
						RsslBuffer			*pNewEncodeBuffer )
{
	rtrIntPtr offset;
	rtrIntPtr encodedLength;
	RsslInt8 i;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_pBuffer, Incorrect iterator use - check buffer);
	RSSL_ASSERT(pIter->_encodingLevel < RSSL_ITER_MAX_LEVELS, Iterator level overrun);
	RSSL_ASSERT(pNewEncodeBuffer && pNewEncodeBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pNewEncodeBuffer->length < pIter->_pBuffer->length)
		return RSSL_RET_BUFFER_TOO_SMALL;

	offset = (rtrIntPtr)(pNewEncodeBuffer->data - pIter->_pBuffer->data);
	encodedLength = (rtrIntPtr)(pIter->_curBufPtr - pIter->_pBuffer->data);

	/* Copy buffer contents */
	memcpy(pNewEncodeBuffer->data, pIter->_pBuffer->data, encodedLength);

	/* Fix main iterator pointers */
	pIter->_curBufPtr += offset;
	pIter->_endBufPtr += offset + (pNewEncodeBuffer->length - pIter->_pBuffer->length);
	pIter->_pBuffer = pNewEncodeBuffer;
	
	/* Fix all pointers in levelInfo */
	i = pIter->_encodingLevel; 
	while (i >= 0)
	{
		RsslEncodingLevel *levelInfo = &pIter->_levelInfo[i--];

		/* Null pointers should remain null, so that any asserts/checks can be triggered normally. */
		if (levelInfo->_internalMark._sizePtr)
			levelInfo->_internalMark._sizePtr += offset;
		if (levelInfo->_internalMark2._sizePtr)
			levelInfo->_internalMark2._sizePtr += offset;
		if (levelInfo->_containerStartPos)
			levelInfo->_containerStartPos += offset;
		if (levelInfo->_countWritePtr)
			levelInfo->_countWritePtr += offset;
		if (levelInfo->_initElemStartPos)
			levelInfo->_initElemStartPos += offset;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodePrimitiveType(	RsslEncodeIterator *pIter, RsslDataType dataType,	const void *pData)
{
	RsslRet ret;

	if ((dataType > RSSL_DT_CONTAINER_TYPE_MIN) || (!(_rsslDataTypeInfo[dataType].bufferEncoders)))
		return RSSL_RET_FAILURE;
	
	return(((ret = (*(_rsslDataTypeInfo[dataType].bufferEncoders))(pIter, pData)) < 0) ?
			RSSL_RET_FAILURE :
			ret);
}



/*********************************/
/* Internal Raw Encoders         */
/*********************************/

/************************/
/* Primitive Data Types */
/************************/

RsslRet RTR_FASTCALL _rsslEncInt(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Primitive type expected);

	if (_rsslIteratorOverrun(pIter,9))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPutLenSpecI64( pIter->_curBufPtr, (*(RsslInt64*)pData));
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncUInt(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,9))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPutLenSpecU64( pIter->_curBufPtr, (*(RsslUInt64*)pData));
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncFloat(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZFLT + 1))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, _rssl1FloatLength );
	pIter->_curBufPtr += rwfPutFloat( pIter->_curBufPtr, (*(RsslFloat*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDouble(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDBL + 1))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, _rssl1DoubleLength );
	pIter->_curBufPtr += rwfPutDouble( pIter->_curBufPtr, (*(RsslDouble*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncReal(RsslEncodeIterator *pIter, const void *pData)
{
	RsslReal	*pReal64 = (RsslReal*)pData;

	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZRL64+1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pReal64->isBlank)
	{
		pIter->_curBufPtr += rwfPutLenSpecBlank(pIter->_curBufPtr);
		return RSSL_RET_SUCCESS;
	}

	/* 31 is currently reserved */
	if ((pReal64->hint > RSSL_RH_NOT_A_NUMBER) || (pReal64->hint == 31) || (pReal64->hint == 32))
		return RSSL_RET_INVALID_DATA;

	switch (pReal64->hint)
	{
		case RSSL_RH_INFINITY:
		case RSSL_RH_NEG_INFINITY:
		case RSSL_RH_NOT_A_NUMBER:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, _rssl1Length );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pReal64->hint );
		break;
		default:	
			pIter->_curBufPtr += rwfPutLenSpecReal64( pIter->_curBufPtr, pReal64->value, pReal64->hint );
	}
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDate(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDate *pDate = (RsslDate*)pData;

	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);
	RSSL_ASSERT(_rssl1DateLen < 0xFE, Invalid length);

	if (_rsslIteratorOverrun(pIter,__RSZDT+1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* We know that the length of date will always fit into a single
	 * UInt16_ob byte, so optimize.
	 */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, _rssl1DateLen );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDate->year) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncTime(RsslEncodeIterator *pIter, const void *pData)
{
	RsslTime *pTime = (RsslTime*)pData;
	RsslUInt8 timeLen;

	RSSL_ASSERT(_rssl1TimeLen < 0xFE, Invalid length);
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (pTime->nanosecond != 0)
		timeLen = 8;
	else if (pTime->microsecond != 0)
		timeLen = 7;
	else if (pTime->millisecond != 0)
		timeLen = 5;
	else if (pTime->second != 0)
		timeLen = 3;
	else
		timeLen = 2;

	if (_rsslIteratorOverrun(pIter,(timeLen+1)))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* We know that the length of time will always fit into a single
	 * UInt16_ob byte, so optimize
	 */
		
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, timeLen );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	switch (timeLen)
	{
		case 2:  // already done
		break;
		case 3:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
		break;
		case 5:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->microsecond) );
		break;
		case 8:
		{
			RsslUInt8 tempNano = (RsslUInt8)pTime->nanosecond;
			RsslUInt16 tempMicro = (((pTime->nanosecond & 0xFF00) << 3) | pTime->microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}	
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDateTime(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDateTime *pDateTime = (RsslDateTime*)pData;
	RsslUInt8 dtLen;

	RSSL_ASSERT(_rssl1DateTimeLen < 0xFE, Invalid length);
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (pDateTime->time.nanosecond != 0)
		dtLen = 12;
	else if (pDateTime->time.microsecond != 0)
		dtLen = 11;
	else if (pDateTime->time.millisecond != 0)
		dtLen = 9;
	else if (pDateTime->time.second != 0)
		dtLen = 7;
	else
		dtLen = 6;

	if (_rsslIteratorOverrun(pIter,dtLen+1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, dtLen );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );

	switch (dtLen)
	{
		case 6: // already done
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
		break;
		case 9:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
		break;
		case 11:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.microsecond) );
		break;
		case 12:
		{
			RsslUInt8 tempNano = (RsslUInt8)(pDateTime->time.nanosecond);
			RsslUInt16 tempMicro = (((pDateTime->time.nanosecond & 0xFF00) << 3) | pDateTime->time.microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncQos(RsslEncodeIterator *pIter, const void *pData)
{
	RsslQos *pQos = (RsslQos*)pData;
	RsslUInt8 dataLength = 1;
	RsslUInt8 Qos;

	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);
	
	if (pQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED ||
		pQos->rate == RSSL_QOS_RATE_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	dataLength += (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ? 2 : 0;
	dataLength += (pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED) ? 2 : 0;

	if (_rsslIteratorOverrun(pIter,(dataLength+1)))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	Qos = (pQos->timeliness << 5);
	Qos |= (pQos->rate << 1);
	Qos |= pQos->dynamic;

    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, dataLength );
    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, Qos );
    if (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->timeInfo);
    }
    if ( pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->rateInfo);
    }
    return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncState(RsslEncodeIterator *pIter, const void *pData)
{
	RsslState *pState = (RsslState*)pData;
	RsslUInt16 dataLength = 3 + pState->text.length;
	RsslUInt8 state;

	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (pState->streamState == RSSL_STREAM_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	if (pState->text.length > 0x80)
		dataLength += 1;

	/* dataLength is RB - if > 0x80, length + 2, else length + 1 */
	if (_rsslIteratorOverrun(pIter,((dataLength > 0x80) ? (dataLength + 2) : (dataLength + 1))))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	state = (pState->streamState << 3);
	state |= pState->dataState;

    pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, dataLength );
    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, state );
    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pState->code );
	pIter->_curBufPtr = _rsslEncodeBuffer15( pIter->_curBufPtr, &pState->text );

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncEnum(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT((pIter->_levelInfo[pIter->_encodingLevel]._encodingState >= RSSL_EIS_SET_DATA) &&
				(pIter->_levelInfo[pIter->_encodingLevel]._encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,3))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPutLenSpecU16( pIter->_curBufPtr, (*(RsslEnum*)pData));
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncBuffer(RsslEncodeIterator *pIter, const void *pData)
{
	RsslBuffer *pBuffer = (RsslBuffer*)pData;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	RSSL_ASSERT((_levelInfo->_encodingState >= RSSL_EIS_SET_DATA) &&
				(_levelInfo->_encodingState <= RSSL_EIS_PRIMITIVE_U15), Unexpected encoding attempted);

	/* Check to see if what type of length encoding. */
	if (_levelInfo->_encodingState == RSSL_EIS_PRIMITIVE_U15)
	{
		if (pBuffer->length > RWF_MAX_U15)
			return(RSSL_RET_ENCODING_UNAVAILABLE);
		if (_rsslIteratorOverrun(pIter,__RSZUI16 + pBuffer->length))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		pIter->_curBufPtr = _rsslEncodeBuffer15( pIter->_curBufPtr, pBuffer );
	}
	else
	{
		if (pBuffer->length > RWF_MAX_16)
			return(RSSL_RET_ENCODING_UNAVAILABLE);
		if (_rsslIteratorOverrun(pIter,__RSZUI16 + 1 + pBuffer->length))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		pIter->_curBufPtr = _rsslEncodeBuffer16( pIter->_curBufPtr, pBuffer );
	}
	return RSSL_RET_SUCCESS;
}

/****************************/
/* End Primitive Data Types */
/****************************/


static const RsslInt64 RSSL_INT8_MIN  = RTR_LL(-128);
static const RsslInt64 RSSL_INT8_MAX  = RTR_LL( 127);
static const RsslInt64 RSSL_INT16_MIN = RTR_LL(-32768);
static const RsslInt64 RSSL_INT16_MAX = RTR_LL( 32767);
static const RsslInt64 RSSL_INT32_MIN = RTR_LL(-2147483648);
static const RsslInt64 RSSL_INT32_MAX = RTR_LL( 2147483647);

static const RsslUInt64 RSSL_UINT8_MAX  = RTR_LL(0xff);
static const RsslUInt64 RSSL_UINT16_MAX = RTR_LL(0xffff);
static const RsslUInt64 RSSL_UINT32_MAX = RTR_LL(0xffffffff);

static const RsslEnum	RSSL_ENUM8_MAX = 0xff;

/****************************/
/* Set Primitive Data Types */
/****************************/

RsslRet RTR_FASTCALL _rsslEncodeInt1(RsslEncodeIterator *pIter, const void *pData)
{
	RsslInt8 myInt8;
	RsslInt64 myInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myInt = *(RsslInt64*)pData;

	if (myInt >= RSSL_INT8_MIN && myInt <= RSSL_INT8_MAX)
	{
		myInt8 = (RsslInt8)myInt;
		pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, myInt8);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt1(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt8 myUInt8;
	RsslUInt64 myUInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myUInt = *(RsslUInt64*)pData;

	if (myUInt <= RSSL_UINT8_MAX)
	{
		myUInt8 = (RsslUInt8)myUInt;
		pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, myUInt8);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeInt2(RsslEncodeIterator *pIter, const void *pData)
{
	RsslInt16 myInt16;
	RsslInt64 myInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myInt = *(RsslInt64*)pData;

	if (myInt >= RSSL_INT16_MIN && myInt <= RSSL_INT16_MAX)
	{
		myInt16 = (RsslInt16)myInt;
		pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, myInt16);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt2(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt16 myUInt16;
	RsslUInt64 myUInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myUInt = *(RsslUInt64*)pData;

	if (myUInt <= RSSL_UINT16_MAX)
	{
		myUInt16 = (RsslUInt16)myUInt;
		pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, myUInt16);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeInt4(RsslEncodeIterator *pIter, const void *pData)
{
	RsslInt32 myInt32;
	RsslInt64 myInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myInt = *(RsslInt64*)pData;

	if (myInt >= RSSL_INT32_MIN && myInt <= RSSL_INT32_MAX)
	{
		myInt32 = (RsslInt32)myInt;
		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, myInt32);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt4(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt32 myUInt32;
	RsslUInt64 myUInt;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myUInt = *(RsslUInt64*)pData;

	if (myUInt <= RSSL_UINT32_MAX)
	{
		myUInt32 = (RsslUInt32)myUInt;
		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, myUInt32);
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeInt8(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPut64( pIter->_curBufPtr, (*(RsslInt64*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt8(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPut64( pIter->_curBufPtr, (*(RsslUInt64*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncFloat_4(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZFLT))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPutFloat( pIter->_curBufPtr, (*(RsslFloat*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDouble_8(RsslEncodeIterator *pIter, const void *pData)
{
	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDBL))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += rwfPutDouble( pIter->_curBufPtr, (*(RsslDouble*)pData) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncReal_4rb(RsslEncodeIterator *pIter, const void *pData)
{
	RsslReal	*real64 = (RsslReal*)pData;
	RsslUInt8	format;
	RsslInt32   value; 

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (real64->value >= RSSL_INT32_MIN && real64->value <= RSSL_INT32_MAX) 
	{
		if (_rsslIteratorOverrun(pIter,__RSZRL32))
			return(RSSL_RET_BUFFER_TOO_SMALL);
		if (real64->isBlank)
			format = 0x20;
		else if ((format = real64->hint) > RSSL_RH_MAX_DIVISOR)
			return RSSL_RET_INVALID_DATA;  // Infinity, Negative Infinity, and NaN are not supported in Set Data for now

		value = (RsslInt32)real64->value;
		pIter->_curBufPtr += RWF_PUT_RESBIT_NUMERIC32( pIter->_curBufPtr, &format, &value );
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncReal_8rb(RsslEncodeIterator *pIter, const void *pData)
{
	RsslReal	*real64 = (RsslReal*)pData;
	RsslUInt8	format;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZRL64))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	if (real64->isBlank)
		format = 0x20;
	else if ((format = real64->hint) > RSSL_RH_MAX_DIVISOR)
		return RSSL_RET_INVALID_DATA; // Infinity, Negative Infinity, and NaN are not supported in Set Data right now

	pIter->_curBufPtr += RWF_PUT_RESBIT_NUMERIC64( pIter->_curBufPtr, &format, &real64->value );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDate_4(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDate *pDate = (RsslDate*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDT))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDate->year) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncTime_3(RsslEncodeIterator *pIter, const void *pData)
{
	RsslTime *pTime = (RsslTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZTM3))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncTime_5(RsslEncodeIterator *pIter, const void *pData)
{
	RsslTime *pTime = (RsslTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZTM5))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncTime_7(RsslEncodeIterator *pIter, const void *pData)
{
	RsslTime *pTime = (RsslTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZTM7))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->microsecond) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncTime_8(RsslEncodeIterator *pIter, const void *pData)
{
	RsslTime *pTime = (RsslTime*)pData;
	RsslUInt8 tempNano = (RsslUInt8)pTime->nanosecond;
	RsslUInt16 tempMicro = (((pTime->nanosecond & 0xFF00) << 3) | pTime->microsecond);

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZTM))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro);
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano);
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDateTime_7(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDateTime *pDateTime = (RsslDateTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDTM7))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDateTime_9(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDateTime *pDateTime = (RsslDateTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDTM9))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDateTime_11(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDateTime *pDateTime = (RsslDateTime*)pData;

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDTM11))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.microsecond) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncDateTime_12(RsslEncodeIterator *pIter, const void *pData)
{
	RsslDateTime *pDateTime = (RsslDateTime*)pData;
	RsslUInt8 tempNano = (RsslUInt8)pDateTime->time.nanosecond;
	RsslUInt16 tempMicro = (((pDateTime->time.nanosecond & 0xFF00) << 3) | pDateTime->time.microsecond);

	RSSL_ASSERT(pIter->_levelInfo[pIter->_encodingLevel]._encodingState == RSSL_EIS_SET_DATA, Unexpected encoding attempted);

	if (_rsslIteratorOverrun(pIter,__RSZDTM))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro);
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano);
	return RSSL_RET_SUCCESS;
}

/********************************/
/* End Set Primitive Data Types */
/********************************/

/*************************************/
/* End Internal Raw Encoders         */
/*************************************/



/*************************************/
/* Begin External Buffer Encoders	 */
/*************************************/


/**
 * Used to copy contents of a buffer into a buffer
 */
RSSL_API RsslRet rsslEncodeBuffer(RsslEncodeIterator *pIter, const RsslBuffer *pBuffer)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pBuffer, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,pBuffer->length))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	MemCopyByInt(pIter->_curBufPtr, pBuffer->data, pBuffer->length);
	pIter->_curBufPtr += pBuffer->length;
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode an Integer into a buffer 
 * @param oBuffer buffer to encode into
 * @param pInt Integer value to encode
 */
RSSL_API RsslRet rsslEncodeInt(RsslEncodeIterator *pIter, const RsslInt64 *pInt)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pInt, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += TRWF_PUT_LENSPEC_I64_NO_LENGTH(pIter->_curBufPtr, pInt);
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode an Unsigned Integer into a buffer 
 * @param oBuffer buffer to encode into
 * @param pUInt Integer value to encode
 */
RSSL_API RsslRet rsslEncodeUInt(RsslEncodeIterator *pIter, const RsslUInt64 *pUInt)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pUInt, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	pIter->_curBufPtr += RWF_PUT_LENSPEC_U64_NO_LENGTH( pIter->_curBufPtr, pUInt);
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a Real into a buffer 
 * @param oBuffer buffer to encode into
 * @param pReal Real value to encode
 */
RSSL_API RsslRet rsslEncodeReal(RsslEncodeIterator *pIter, const RsslReal *pReal)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pReal, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,__RSZRL64))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pReal->isBlank)
		return RSSL_RET_SUCCESS;

	/* 31 is currently reserved */
	if ((pReal->hint > RSSL_RH_NOT_A_NUMBER) || (pReal->hint == 31) || (pReal->hint == 32))
		return RSSL_RET_INVALID_DATA;

	switch(pReal->hint)
	{
		case RSSL_RH_INFINITY:
		case RSSL_RH_NEG_INFINITY:
		case RSSL_RH_NOT_A_NUMBER:
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pReal->hint);
		break;
		default:
			pIter->_curBufPtr += rwfPutReal64( pIter->_curBufPtr, pReal->value, pReal->hint );
	}
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a float into a buffer 
 * @param oBuffer buffer to encode into
 * @param pFloat float value to encode
 */
RSSL_API RsslRet rsslEncodeFloat(RsslEncodeIterator *pIter, const RsslFloat* pFloat)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pFloat, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,__RSZFLT))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPutFloat( pIter->_curBufPtr, *pFloat );
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a double into a buffer 
 * @param oBuffer buffer to encode into
 * @param pDouble double value to encode
 */
RSSL_API RsslRet rsslEncodeDouble(RsslEncodeIterator *pIter, const RsslDouble* pDouble)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pDouble, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,__RSZDBL))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPutDouble( pIter->_curBufPtr, *pDouble );
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode Qos into a buffer 
 * @param oBuffer buffer to encode into
 * @param pQos Qos value to encode
 */
RSSL_API RsslRet rsslEncodeQos(RsslEncodeIterator *pIter, const RsslQos *pQos )
{
	RsslUInt8 Qos;
	RsslUInt8 dataLength = 1;

	RSSL_ASSERT(pQos, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);

	dataLength += (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ? 2 : 0;
	dataLength += (pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED) ? 2 : 0;

	if (_rsslIteratorOverrun(pIter,dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED ||
		pQos->rate == RSSL_QOS_RATE_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	Qos = (pQos->timeliness << 5);
	Qos |= (pQos->rate << 1);
	Qos |= pQos->dynamic;

    pIter->_curBufPtr+= rwfPut8( pIter->_curBufPtr, Qos );
    if (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->timeInfo);
    }
    if ( pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->rateInfo);
    }
    return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode State into a buffer 
 * @param oBuffer buffer to encode into
 * @param pState State value to encode
 */
RSSL_API RsslRet rsslEncodeState(RsslEncodeIterator *pIter, const RsslState *pState)
{
	RsslUInt8 state;
	RsslUInt16 dataLength = 3 + pState->text.length;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pState, Invalid parameters or parameters passed in as NULL);

	if (pState->streamState == RSSL_STREAM_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	if (pState->text.length > 0x80)
		dataLength += 1;

	if (_rsslIteratorOverrun(pIter,dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	state = (pState->streamState << 3);
	state |= pState->dataState;

    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, state );
    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pState->code );
	pIter->_curBufPtr = _rsslEncodeBuffer15( pIter->_curBufPtr, &pState->text );

	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a Enum into a buffer 
 * @param oBuffer buffer to encode into
 * @param pEnum enum value to encode
 */
RSSL_API RsslRet rsslEncodeEnum(RsslEncodeIterator *pIter, const RsslEnum* pEnum)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEnum, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, *pEnum );
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a date into a buffer 
 * @param oBuffer buffer to encode into
 * @param pDate date value to encode
 */
RSSL_API RsslRet rsslEncodeDate(RsslEncodeIterator *pIter, const RsslDate *pDate)
{	
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pDate, Invalid parameters or parameters passed in as NULL);

	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr+= rwfPut8( pIter->_curBufPtr, (pDate->day) );
	pIter->_curBufPtr+= rwfPut8( pIter->_curBufPtr, (pDate->month) );
	pIter->_curBufPtr+= rwfPut16( pIter->_curBufPtr, (pDate->year) );
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a time into a buffer 
 * @param oBuffer buffer to encode into
 * @param pTime time value to encode
 */
RSSL_API RsslRet rsslEncodeTime(RsslEncodeIterator *pIter, const RsslTime *pTime)
{
	RsslUInt8 timeLen;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pTime, Invalid parameters or parameters passed in as NULL);

	if (pTime->nanosecond != 0)
		timeLen = 8;
	else if (pTime->microsecond != 0)
		timeLen = 7;
	else if (pTime->millisecond != 0)
		timeLen = 5;
	else if (pTime->second != 0)
		timeLen = 3;
	else
		timeLen = 2;
	
	if (_rsslIteratorOverrun(pIter,timeLen))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	switch (timeLen)
	{
		case 2:  // already done
		break;
		case 3:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
		break;
		case 5:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->microsecond) );
		break;
		case 8:
		{
			RsslUInt8 tempNano = (RsslUInt8)pTime->nanosecond;
			RsslUInt16 tempMicro = (((pTime->nanosecond & 0xFF00) << 3) | pTime->microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}	
	return RSSL_RET_SUCCESS;
}

/** 
 * @brief Used to encode a datetime into a buffer 
 * @param oBuffer buffer to encode into
 * @param pDateTime datetime value to encode
 */
RSSL_API RsslRet rsslEncodeDateTime(RsslEncodeIterator *pIter, const RsslDateTime *pDateTime)
{
	RsslUInt8 dtLen;
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pDateTime, Invalid parameters or parameters passed in as NULL);
	
	if (pDateTime->time.nanosecond != 0)
		dtLen = 12;
	else if (pDateTime->time.microsecond != 0)
		dtLen = 11;
	else if (pDateTime->time.millisecond != 0)
		dtLen = 9;
	else if (pDateTime->time.second != 0)
		dtLen = 7;
	else
		dtLen = 6;
	
	if (_rsslIteratorOverrun(pIter,dtLen))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );

	switch (dtLen)
	{
		case 6: // already done
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
		break;
		case 9:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
		break;
		case 11:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.microsecond) );
		break;
		case 12:
		{
			RsslUInt8 tempNano = (RsslUInt8)(pDateTime->time.nanosecond);
			RsslUInt16 tempMicro = (((pDateTime->time.nanosecond & 0xFF00) << 3) | pDateTime->time.microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_SUCCESS;
}



/*************************************/
/* Begin Internal Buffer Encoders	 */
/*************************************/


RsslRet RTR_FASTCALL _rsslEncodeInt_1( RsslEncodeIterator *pIter, const void *value )
{
	if (_rsslIteratorOverrun(pIter,1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (*(RsslInt8*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt_1(RsslEncodeIterator *pIter, const void *value )
{
	if (_rsslIteratorOverrun(pIter,1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (*(RsslUInt8*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeInt_2(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr +=  rwfPut16( pIter->_curBufPtr, (*(RsslInt16*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt_2(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (*(RsslUInt16*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeInt_4(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, (*(RsslInt32*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt_4(RsslEncodeIterator *pIter, const void *value)
{

	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, (*(RsslUInt32*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeInt_8(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut64( pIter->_curBufPtr, (*(RsslInt64*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeUInt_8(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,8))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut64( pIter->_curBufPtr, (*(RsslUInt64*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeReal(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt8	format;
	char *startPos = pIter->_curBufPtr;
	RsslReal *pReal64 = (RsslReal*)pData;

	if (_rsslIteratorOverrun(pIter, __RSZRL64))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	
	if (pReal64->isBlank)
		format = 0x20;		
	else if (((format = pReal64->hint) > RSSL_RH_NOT_A_NUMBER) || (pReal64->hint == 31) || (pReal64->hint == 32))  // 31 is currently reserved
		return RSSL_RET_INVALID_DATA;
	
	switch (pReal64->hint)
	{
		case RSSL_RH_INFINITY:
		case RSSL_RH_NEG_INFINITY:
		case RSSL_RH_NOT_A_NUMBER:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, _rssl1Length );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pReal64->hint );
		break;
		default:
			pIter->_curBufPtr += RWF_PUT_RESBIT_NUMERIC64( pIter->_curBufPtr, &format, &pReal64->value );
	}
	if (pIter->_curBufPtr - startPos > 0)
		*startPos &= 0x3F;

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeFloat(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter, __RSZFLT))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPutFloat( pIter->_curBufPtr, (*(RsslFloat*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeDouble(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter, __RSZDBL))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPutDouble( pIter->_curBufPtr, (*(RsslDouble*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeQos(RsslEncodeIterator *pIter, const void *pData )
{
	RsslUInt8 Qos;
	RsslUInt8 dataLength = 1;
	RsslQos *pQos = (RsslQos*)pData;

	dataLength += (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ? 2 : 0;
	dataLength += (pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED) ? 2 : 0;

	if (_rsslIteratorOverrun(pIter, dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED ||
		pQos->rate == RSSL_QOS_RATE_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	Qos = (pQos->timeliness << 5);
	Qos |= (pQos->rate << 1);
	Qos |= pQos->dynamic;

    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, Qos );
    if (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->timeInfo);
    }
    if ( pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
    {
        pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pQos->rateInfo);
    }
    return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeState(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt8 state;
	RsslState *pState = (RsslState*)pData;
	RsslUInt16 dataLength = 3 + pState->text.length;

	if (pState->streamState == RSSL_STREAM_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	if (pState->text.length > 0x80)
		dataLength += 1;

	if (_rsslIteratorOverrun(pIter,dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	state = (pState->streamState << 3);
	state |= pState->dataState;

    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, state );
    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pState->code );
	pIter->_curBufPtr = _rsslEncodeBuffer15( pIter->_curBufPtr, &pState->text );

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeEnum2(RsslEncodeIterator *pIter, const void *value)
{
	if (_rsslIteratorOverrun(pIter,2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (*(RsslEnum*)value) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeEnum1(RsslEncodeIterator *pIter, const void *value)
{
	RsslEnum myEnum;
	RsslUInt8 myEnum8;
	if (_rsslIteratorOverrun(pIter,1))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	myEnum = *(RsslEnum*)value;

	if (myEnum <= RSSL_ENUM8_MAX)
	{
    	myEnum8 = (RsslUInt8)myEnum;
    	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, myEnum8 );
    	return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_VALUE_OUT_OF_RANGE;
}

RsslRet RTR_FASTCALL _rsslEncodeDate(RsslEncodeIterator *pIter, const void *pData)
{	
	RsslDate *pDate = (RsslDate*)pData;
	if (_rsslIteratorOverrun(pIter,4))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDate->month) );
	pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDate->year) );
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeTime(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt8 timeLen;

	RsslTime *pTime = (RsslTime*)pData;

	if (pTime->nanosecond != 0)
		timeLen = 8;
	else if (pTime->microsecond != 0)
		timeLen = 7;
	else if (pTime->millisecond != 0)
		timeLen = 5;
	else if (pTime->second != 0)
		timeLen = 3;
	else
		timeLen = 2;

	if (_rsslIteratorOverrun(pIter,(timeLen)))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* We know that the length of time will always fit into a single
	 * UInt16_ob byte, so optimize
	 */
	
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->minute) );
	switch (timeLen)
	{
		case 2:  // already done
		break;
		case 3:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
		break;
		case 5:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->microsecond) );
		break;
		case 8:
		{
			RsslUInt8 tempNano = (RsslUInt8)pTime->nanosecond;
			RsslUInt16 tempMicro = (((pTime->nanosecond & 0xFF00) << 3) | pTime->microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pTime->second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pTime->millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}	
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeDateTime(RsslEncodeIterator *pIter, const void *pData)
{
	RsslUInt8 dtLen;
	RsslDateTime *pDateTime = (RsslDateTime*)pData;

	if (pDateTime->time.nanosecond != 0)
		dtLen = 12;
	else if (pDateTime->time.microsecond != 0)
		dtLen = 11;
	else if (pDateTime->time.millisecond != 0)
		dtLen = 9;
	else if (pDateTime->time.second != 0)
		dtLen = 7;
	else
		dtLen = 6;
	
	if (_rsslIteratorOverrun(pIter,dtLen))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.day) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->date.month) );
	pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, (pDateTime->date.year) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.hour) );
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.minute) );

	switch (dtLen)
	{
		case 6: // already done
		break;
		case 7:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
		break;
		case 9:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
		break;
		case 11:
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.microsecond) );
		break;
		case 12:
		{
			RsslUInt8 tempNano = (RsslUInt8)(pDateTime->time.nanosecond);
			RsslUInt16 tempMicro = (((pDateTime->time.nanosecond & 0xFF00) << 3) | pDateTime->time.microsecond);

			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pDateTime->time.second) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, (pDateTime->time.millisecond) );
			pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, tempMicro );
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, tempNano );
		}
		break;
		default:
			return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslEncodeBuffer(RsslEncodeIterator *pIter, const void *pData)
{
	RsslBuffer *pBuffer = (RsslBuffer*)pData;
	if (_rsslIteratorOverrun(pIter,pBuffer->length))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	MemCopyByInt(pIter->_curBufPtr, pBuffer->data, pBuffer->length);
	pIter->_curBufPtr += pBuffer->length;
	return RSSL_RET_SUCCESS;
}

/*********************************/
/*   End of Buffer Encoders      */
/*********************************/

