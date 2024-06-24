/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/rsslDataTypeEnums.h"
#include "rtr/intDataTypes.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static RsslUInt32 real32LenHints[] = {2, 3, 4, 5};

static RsslUInt32 real64LenHints[] = {3, 5, 7, 9};

/* New */

/**********************/
/*  Internal decoders */
/**********************/

RsslRet RTR_FASTCALL _rsslDecBuf8(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	RsslUInt8 length;

	RSSL_ASSERT(pIter->_curBufPtr == levelInfo->_nextEntryPtr, Invalid decoding attempted);

	/* Move _curBufPtr and _endBufPtr around data. _nextEntryPtr should point after it. */
	pIter->_curBufPtr += rwfGet8(length, pIter->_curBufPtr);
	pData->length = length;
	pData->data = (pData->length > 0) ? (char *) pIter->_curBufPtr : 0;
	levelInfo->_nextEntryPtr = pIter->_curBufPtr + length;
	return(levelInfo->_nextEntryPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDecBuf16(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	RsslUInt16 tlen;

	RSSL_ASSERT(pIter->_curBufPtr == levelInfo->_nextEntryPtr, Invalid decoding attempted);

	pIter->_curBufPtr += rwfGetOptByteU16(&tlen, pIter->_curBufPtr);
	pData->length = tlen;
	pData->data = (pData->length > 0) ? (char *) pIter->_curBufPtr : 0;
	levelInfo->_nextEntryPtr = pIter->_curBufPtr + tlen;
	return(levelInfo->_nextEntryPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}


/*******************************************/
/*  Internal decoders to decode to a void* */
/*******************************************/

RsslRet RTR_FASTCALL _rsslDecBuffer16(RsslDecodeIterator *pIter, void *value)
{
	((RsslBuffer*)value)->length = (rtrUInt32)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr));
	((RsslBuffer*)value)->data = pIter->_curBufPtr;
	return RSSL_RET_SUCCESS;
}

RsslRet RTR_FASTCALL _rsslDecInt(RsslDecodeIterator *pIter, void *value)
{
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		 (rwfGetLenSpecI64Size((RsslInt64*)value, pIter->_curBufPtr, (rtrUInt8)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
		  RSSL_RET_SUCCESS :
			RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecUInt(RsslDecodeIterator *pIter, void *value)
{
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		(rwfGetLenSpecU64Size((RsslUInt64*)value, pIter->_curBufPtr, (rtrUInt8)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
		  RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecFloat(RsslDecodeIterator *pIter, void *value)
{
		/* always 4 bytes */
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		 ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 4) ?
		  rwfGetFloat((*(RsslFloat*)value), pIter->_curBufPtr),
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecDouble(RsslDecodeIterator *pIter, void *value)
{
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 8) ?
		  rwfGetDouble((*(RsslDouble*)value), pIter->_curBufPtr),
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecReal(RsslDecodeIterator *pIter, void *value)
{
	RsslUInt8 format;
	RsslReal *pReal = (RsslReal*)value;
	
	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0: 
			pReal->isBlank = RSSL_TRUE;
			pReal->hint = 0;
			pReal->value = 0;
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
			rwfGet8(format, pIter->_curBufPtr);
			switch(format & 0x3F)
			{
				case RSSL_RH_INFINITY:
				case RSSL_RH_NEG_INFINITY:
				case RSSL_RH_NOT_A_NUMBER:
					pReal->hint = format & 0x3F;
					pReal->isBlank = RSSL_FALSE;
				break;
				default:
					pReal->isBlank = RSSL_TRUE;
					pReal->hint = 0;
				break;
			}
			pReal->value = 0;
			return RSSL_RET_SUCCESS;			
		break;
		default:
			if (rwfGetReal64(&pReal->value, &format, (rtrUInt16)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr)), pIter->_curBufPtr ) > 0)
			{
				switch(format & 0x3F)
				{
					case RSSL_RH_INFINITY:
					case RSSL_RH_NEG_INFINITY:
					case RSSL_RH_NOT_A_NUMBER:
						pReal->hint = format & 0x3F;
						pReal->isBlank = RSSL_FALSE;
					break;
					default:
						pReal->isBlank = (format & 0x20) ? RSSL_TRUE : RSSL_FALSE;
						pReal->hint = format & 0x1F;
					break;
				}									
				return RSSL_RET_SUCCESS;
			}
			return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_INVALID_DATA;
}

RsslRet RTR_FASTCALL _rsslDecDate(RsslDecodeIterator *pIter, void *value)
{
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			((RsslDate*)value)->day = 0,
			((RsslDate*)value)->month = 0,
			((RsslDate*)value)->year = 0,
			RSSL_RET_BLANK_DATA :
		((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 4) ?
		  rwfGet8(((RsslDate*)value)->day, pIter->_curBufPtr),
		  rwfGet8(((RsslDate*)value)->month, (pIter->_curBufPtr + 1)),
		  rwfGet16(((RsslDate*)value)->year, (pIter->_curBufPtr + 2)),
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecTime(RsslDecodeIterator *pIter, void *value)
{
	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			rsslBlankTime((RsslTime*)value);
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 2:
			rwfGet8(((RsslTime*)value)->hour, pIter->_curBufPtr);
			rwfGet8(((RsslTime*)value)->minute, (pIter->_curBufPtr + 1));

			(((RsslTime*)value)->hour == 255) ?
			(((RsslTime*)value)->second = 255,
			((RsslTime*)value)->millisecond = 65535,
			((RsslTime*)value)->microsecond = 2047,
			((RsslTime*)value)->nanosecond = 2047) :
			(((RsslTime*)value)->second = 0,
			((RsslTime*)value)->millisecond = 0,
			((RsslTime*)value)->microsecond = 0,
			((RsslTime*)value)->nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 3:
			rwfGet8(((RsslTime*)value)->hour, pIter->_curBufPtr);
			rwfGet8(((RsslTime*)value)->minute, (pIter->_curBufPtr + 1));
			rwfGet8(((RsslTime*)value)->second, (pIter->_curBufPtr + 2));

			(((RsslTime*)value)->hour == 255) ?
			(((RsslTime*)value)->millisecond = 65535,
			((RsslTime*)value)->microsecond = 2047,
			((RsslTime*)value)->nanosecond = 2047) : 
			(((RsslTime*)value)->millisecond = 0,
			((RsslTime*)value)->microsecond = 0,
			((RsslTime*)value)->nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 4: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 5:
			rwfGet8(((RsslTime*)value)->hour, pIter->_curBufPtr);
			rwfGet8(((RsslTime*)value)->minute, (pIter->_curBufPtr + 1));
			rwfGet8(((RsslTime*)value)->second, (pIter->_curBufPtr + 2));
			rwfGet16(((RsslTime*)value)->millisecond, (pIter->_curBufPtr + 3));
			(((RsslTime*)value)->hour == 255) ?
			(((RsslTime*)value)->microsecond = 2047,
			((RsslTime*)value)->nanosecond = 2047) : 
			(((RsslTime*)value)->microsecond = 0,
			((RsslTime*)value)->nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 6:
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 7:
			rwfGet8(((RsslTime*)value)->hour, pIter->_curBufPtr);
			rwfGet8(((RsslTime*)value)->minute, (pIter->_curBufPtr + 1));
			rwfGet8(((RsslTime*)value)->second, (pIter->_curBufPtr + 2));
			rwfGet16(((RsslTime*)value)->millisecond, (pIter->_curBufPtr + 3));
			rwfGet16(((RsslTime*)value)->microsecond, (pIter->_curBufPtr + 5));
			
			(((RsslTime*)value)->hour == 255) ?
			(((RsslTime*)value)->nanosecond = 2047) : 
			(((RsslTime*)value)->nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 8:
		{
			RsslUInt16 tempMicro;
			RsslUInt8 tempNano;
			rwfGet8(((RsslTime*)value)->hour, pIter->_curBufPtr);
			rwfGet8(((RsslTime*)value)->minute, (pIter->_curBufPtr + 1));
			rwfGet8(((RsslTime*)value)->second, (pIter->_curBufPtr + 2));
			rwfGet16(((RsslTime*)value)->millisecond, (pIter->_curBufPtr + 3));
			rwfGet16(tempMicro, (pIter->_curBufPtr + 5));
			rwfGet8(tempNano, (pIter->_curBufPtr + 7));
			((RsslTime*)value)->microsecond = tempMicro & 0x07FF;
			((RsslTime*)value)->nanosecond = (((tempMicro & 0x3800) >> 3) + tempNano);

			return RSSL_RET_SUCCESS;
		}
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}

RsslRet RTR_FASTCALL _rsslDecDateTime(RsslDecodeIterator *pIter, void *value)
{
	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			rsslClearDateTime(((RsslDateTime*)value));			
			return RSSL_RET_BLANK_DATA;
		break;
		case 6:
			rwfGet8(((RsslDateTime*)value)->date.day, pIter->_curBufPtr);
			rwfGet8(((RsslDateTime*)value)->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(((RsslDateTime*)value)->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(((RsslDateTime*)value)->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(((RsslDateTime*)value)->time.minute, (pIter->_curBufPtr + 5));

			(((RsslDateTime*)value)->time.hour == 255) ?
			(((RsslDateTime*)value)->time.second = 255,
			((RsslDateTime*)value)->time.millisecond = 65535,
			((RsslDateTime*)value)->time.microsecond = 2047,
			((RsslDateTime*)value)->time.nanosecond = 2047) :
			(((RsslDateTime*)value)->time.second = 0,
			((RsslDateTime*)value)->time.millisecond = 0,
			((RsslDateTime*)value)->time.microsecond = 0,
			((RsslDateTime*)value)->time.nanosecond = 0);
			
			return RSSL_RET_SUCCESS;
		break;
		case 7:
			rwfGet8(((RsslDateTime*)value)->date.day, pIter->_curBufPtr);
			rwfGet8(((RsslDateTime*)value)->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(((RsslDateTime*)value)->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(((RsslDateTime*)value)->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(((RsslDateTime*)value)->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(((RsslDateTime*)value)->time.second, (pIter->_curBufPtr + 6));

			(((RsslDateTime*)value)->time.hour == 255) ?
			(((RsslDateTime*)value)->time.millisecond = 65535,
			((RsslDateTime*)value)->time.microsecond = 2047,
			((RsslDateTime*)value)->time.nanosecond = 2047) :
			(((RsslDateTime*)value)->time.millisecond = 0,
			((RsslDateTime*)value)->time.microsecond = 0,
			((RsslDateTime*)value)->time.nanosecond = 0);
		  
			return RSSL_RET_SUCCESS;
		case 9:
			rwfGet8(((RsslDateTime*)value)->date.day, pIter->_curBufPtr);
			rwfGet8(((RsslDateTime*)value)->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(((RsslDateTime*)value)->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(((RsslDateTime*)value)->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(((RsslDateTime*)value)->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(((RsslDateTime*)value)->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(((RsslDateTime*)value)->time.millisecond, (pIter->_curBufPtr + 7));

			(((RsslDateTime*)value)->time.hour == 255) ?
			(((RsslDateTime*)value)->time.microsecond = 2047,
			((RsslDateTime*)value)->time.nanosecond = 2047):
			(((RsslDateTime*)value)->time.microsecond = 0,
			((RsslDateTime*)value)->time.nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 11:
			rwfGet8(((RsslDateTime*)value)->date.day, pIter->_curBufPtr);
			rwfGet8(((RsslDateTime*)value)->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(((RsslDateTime*)value)->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(((RsslDateTime*)value)->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(((RsslDateTime*)value)->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(((RsslDateTime*)value)->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(((RsslDateTime*)value)->time.millisecond, (pIter->_curBufPtr + 7));
			rwfGet16(((RsslDateTime*)value)->time.microsecond, (pIter->_curBufPtr + 9));

			(((RsslDateTime*)value)->time.hour == 255) ?
			(((RsslDateTime*)value)->time.nanosecond = 2047):
			(((RsslDateTime*)value)->time.nanosecond = 0);

			return RSSL_RET_SUCCESS;
		break;
		case 12:
		{
			RsslUInt16 tempMicro;
			RsslUInt8 tempNano;

			rwfGet8(((RsslDateTime*)value)->date.day, pIter->_curBufPtr);
			rwfGet8(((RsslDateTime*)value)->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(((RsslDateTime*)value)->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(((RsslDateTime*)value)->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(((RsslDateTime*)value)->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(((RsslDateTime*)value)->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(((RsslDateTime*)value)->time.millisecond, (pIter->_curBufPtr + 7));			
			rwfGet16(tempMicro, (pIter->_curBufPtr + 9));
			rwfGet8(tempNano, (pIter->_curBufPtr + 11));
			((RsslDateTime*)value)->time.microsecond = tempMicro & 0x07FF;
			((RsslDateTime*)value)->time.nanosecond = (((tempMicro & 0x3800) >> 3) + tempNano);

			return RSSL_RET_SUCCESS;
		}
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}

RsslRet RTR_FASTCALL _rsslDecQos(RsslDecodeIterator *pIter, void *value)
{
	RsslUInt8 qosValue;

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
	  ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) >= 1) ?
		rwfGet8(qosValue, pIter->_curBufPtr),
		((RsslQos*)value)->timeliness = qosValue >> 5,
		((RsslQos*)value)->rate = (qosValue >> 1) & 0xF,
		((RsslQos*)value)->dynamic = qosValue & 0x1,
		((((RsslQos*)value)->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ?
		 rwfGet16(((RsslQos*)value)->timeInfo, (pIter->_curBufPtr + 1)) :
	     (((RsslQos*)value)->timeInfo = 0)),
		((((RsslQos*)value)->rate > RSSL_QOS_RATE_JIT_CONFLATED) ?
		 ((((RsslQos*)value)->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ?
		 rwfGet16(((RsslQos*)value)->rateInfo, (pIter->_curBufPtr + 3)) : 
		 rwfGet16(((RsslQos*)value)->rateInfo, (pIter->_curBufPtr + 1))) :
		 (((RsslQos*)value)->rateInfo = 0)),
	   RSSL_RET_SUCCESS :
	   RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecState(RsslDecodeIterator *pIter, void *value)
{
	RsslUInt8 state;

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
		RSSL_RET_BLANK_DATA :
	  ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) >= 3) ?
		rwfGet8(state, pIter->_curBufPtr),
		rwfGet8(((RsslState*)value)->code, (pIter->_curBufPtr + 1)),
		_rsslDecodeBuffer15(&((RsslState*)value)->text, pIter->_curBufPtr + 2),
		((RsslState*)value)->streamState = state >> 3,
		((RsslState*)value)->dataState = state & 0x7,
	   RSSL_RET_SUCCESS :	
	   RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDecEnum(RsslDecodeIterator *pIter, void *value)
{
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
		RSSL_RET_BLANK_DATA :
		(rwfGetLenSpecU16_Size((rtrUInt16*)(value), pIter->_curBufPtr, (rtrUInt16)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
	   RSSL_RET_SUCCESS :
	   RSSL_RET_INCOMPLETE_DATA );
}

RsslRet RTR_FASTCALL _rsslDec8(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 1;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr++;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec16(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 2;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 2;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec24(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 3;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 3;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec32(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 4;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 4;
	return(pIter->_curBufPtr <= pIter->_levelInfo[pIter->_decodingLevel]._endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec40(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 5;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 5;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec56(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 7;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 7;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec64(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 8;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 8;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec72(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 9;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 9;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec88(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 11;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 11;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDec96(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	pData->length = 12;
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += 12;
	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDecReal_4rb(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	RsslUInt8 format = 0;

	rwfGet8(format, pIter->_curBufPtr);

	if (format & 0x20)  // If blank bit is set, this is 1 byte, even for our additional hints (Inf, -Inf, NaN).  
		pData->length = 1;
	else
	{
		/* mask out all but first two bits */
		format = format >> 6;

		pData->length = real32LenHints[format];
	}
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += pData->length;

	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}

RsslRet RTR_FASTCALL _rsslDecReal_8rb(RsslDecodeIterator *pIter, RsslBuffer *pData)
{
	RsslDecodingLevel *levelInfo = &pIter->_levelInfo[pIter->_decodingLevel];
	RsslUInt8 format = 0;

	rwfGet8(format, pIter->_curBufPtr);

	if (format & 0x20)  // If blank bit is set, this is 1 byte, even for our additional hints (Inf, -Inf, NaN).
		pData->length = 1;
	else
	{
		/* mask out all but first two bits */
		format = format >> 6;

		pData->length = real64LenHints[format];
	}
	pData->data = pIter->_curBufPtr;
	levelInfo->_nextEntryPtr += pData->length;

	return(pIter->_curBufPtr <= levelInfo->_endBufPtr? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA);
}


/********************************************/
/* Internal string conversion functions,    */
/* Accessed via rsslEncodedPrimitiveToString*/
/********************************************/

/* For blanking out strings. Use when BLANK_DATA was returned from a primitive decode. */
RTR_C_INLINE RsslRet _blankAsString(RsslBuffer *oBuffer)
{
	if (oBuffer->length > 0) 
	{
		/* Blank out string (length 0 is consistent with other string funcs since they don't include the null term in it)*/
		oBuffer->length = 0;
		oBuffer->data[0] = '\0';
		return RSSL_RET_BLANK_DATA;
	}

	/* Can't blank out the string because it is 0-length. */
	return RSSL_RET_FAILURE;
}

RsslRet RTR_FASTCALL _rsslIntAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslInt64 i64 = 0;
	
	if ((ret = rsslDecodeInt(pIter, &i64)) < 0)
		return ret; /* Decode value */
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = _rsslIntToString(&i64, oBuffer)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslIntToString(void *pInt, RsslBuffer *oBuffer)
{
	int length = 0;

	length = snprintf(oBuffer->data, oBuffer->length, RTR_LONGLONG_SPEC, *(RsslInt64*)pInt);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_FASTCALL _rsslUIntAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslUInt64 ui64 = 0;
	
	if ((ret = rsslDecodeUInt(pIter, &ui64)) < 0)
		return ret; /* Decode value */
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = _rsslUIntToString(&ui64, oBuffer)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslUIntToString(void *pUInt, RsslBuffer *oBuffer)
{
	int length = 0;

	length = snprintf(oBuffer->data, oBuffer->length, RTR_ULONGLONG_SPEC, *(RsslUInt64*)pUInt);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_FASTCALL _rsslFloatAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslFloat fl; /* Initialization skipped to work around bug between this code and GCC 4.1.2 */

	if ((ret = rsslDecodeFloat(pIter, &fl)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = _rsslFloatToString(&fl, oBuffer)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslFloatToString(void *pFloat, RsslBuffer *oBuffer)
{
	int length = 0;
	rtrUInt32 numDigits = 7;
	if (3 < oBuffer->length && oBuffer->length < 11)
	{
		numDigits = oBuffer->length - 3;
	}
	length = snprintf(oBuffer->data, oBuffer->length, "%.*g", numDigits, *(RsslFloat*)pFloat);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_FASTCALL _rsslDoubleAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslDouble dub; /* Initialization skipped to work around bug between this code and GCC 4.1.2 */

	if ((ret = rsslDecodeDouble(pIter, &dub)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = _rsslDoubleToString(&dub, oBuffer)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslDoubleToString(void  *pDouble, RsslBuffer *oBuffer)
{
	int length = 0;
	rtrUInt32 numDigits = 16;
	if (3 < oBuffer->length && oBuffer->length < 19)
	{
		numDigits = oBuffer->length - 3;
	}
	length = snprintf(oBuffer->data, oBuffer->length, "%.*g", numDigits, *(RsslDouble*)pDouble);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_FASTCALL _rsslRealAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslReal real64 = RSSL_INIT_REAL;

	if ((ret = rsslDecodeReal(pIter, &real64)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslRealToString(oBuffer, &real64)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslRealToString(void *pReal, RsslBuffer *oBuffer)
{
	return rsslRealToString(oBuffer, (RsslReal*)pReal);
}

RsslRet RTR_FASTCALL _rsslEnumAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslEnum eVal = 0;

	if ((ret = rsslDecodeEnum(pIter, &eVal)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = _rsslEnumToString(&eVal, oBuffer)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslEnumToString(void *pEnum, RsslBuffer *oBuffer)
{
	int length = 0;

	length = snprintf(oBuffer->data, oBuffer->length, "%d", *(RsslEnum*)pEnum);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RsslRet RTR_FASTCALL _rsslDateAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{	
	RsslRet ret, ret2;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

	if ((ret = rsslDecodeDate(pIter, &dateTime.date)) < 0) return ret; /* Decode value */
	if (ret == RSSL_RET_BLANK_DATA) return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslDateTimeToStringFormat(oBuffer, RSSL_DT_DATE, &dateTime, RSSL_STR_DATETIME_RSSL)) < 0) return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslDateToString(void *pDate, RsslBuffer *oBuffer)
{
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	dateTime.date = *(RsslDate*)pDate;
	return rsslDateTimeToStringFormat(oBuffer, RSSL_DT_DATE, &dateTime, RSSL_STR_DATETIME_RSSL);
}

RsslRet RTR_FASTCALL _rsslTimeAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

	if ((ret = rsslDecodeTime(pIter, &dateTime.time)) < 0) return ret; /* Decode value */
	if (ret == RSSL_RET_BLANK_DATA) return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslDateTimeToStringFormat(oBuffer, RSSL_DT_TIME, &dateTime, RSSL_STR_DATETIME_RSSL)) < 0) return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslTimeToString(void *pTime, RsslBuffer *oBuffer)
{
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	dateTime.time = *(RsslTime*)pTime;
	return rsslDateTimeToStringFormat(oBuffer, RSSL_DT_TIME, &dateTime, RSSL_STR_DATETIME_RSSL);
}

RsslRet RTR_FASTCALL _rsslDateTimeAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;

	if ((ret = rsslDecodeDateTime(pIter, &dateTime)) < 0) return ret; /* Decode value */
	if (ret == RSSL_RET_BLANK_DATA) return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslDateTimeToStringFormat(oBuffer, RSSL_DT_DATETIME, &dateTime, RSSL_STR_DATETIME_RSSL)) < 0) return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslDateTimeToString(void *pDateTime, RsslBuffer *oBuffer)
{
	return rsslDateTimeToStringFormat(oBuffer, RSSL_DT_DATETIME, (RsslDateTime*)pDateTime, RSSL_STR_DATETIME_RSSL);
}

RsslRet RTR_FASTCALL _rsslQosAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslQos qos = RSSL_INIT_QOS;

	if ((ret = rsslDecodeQos(pIter, &qos)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslQosToString(oBuffer, &qos)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslQosToString(void *pQos, RsslBuffer *oBuffer)
{
	return rsslQosToString(oBuffer, (RsslQos*)pQos);
}

RsslRet RTR_FASTCALL _rsslStateAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslRet ret, ret2;
	RsslState state = RSSL_INIT_STATE;

	if ((ret = rsslDecodeState(pIter, &state)) < 0) /* Decode value */
		return ret;
	if (ret == RSSL_RET_BLANK_DATA)
		return _blankAsString(oBuffer); /* If blank, clear string */
	if ((ret2 = rsslStateToString(oBuffer, &state)) < 0)
		return ret2; /* Else write string */
	return ret; /* Return code from decode function(will usually just be success) */
}

RsslRet RTR_FASTCALL _rsslStateToString(void *pState, RsslBuffer *oBuffer)
{
	return rsslStateToString(oBuffer, (RsslState*)pState);
}

RsslRet RTR_FASTCALL _rsslBufferAsString(RsslDecodeIterator *pIter, RsslBuffer *oBuffer)
{
	RsslBuffer iBuffer = { (rtrUInt32)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr)), pIter->_curBufPtr };
	return _rsslBufferToString(&iBuffer, oBuffer);
}

RsslRet RTR_FASTCALL _rsslBufferToString(void *iBuffer, RsslBuffer *oBuffer)
{
	/* Must be larger so that we can null-terminate */
	if (oBuffer->length > ((RsslBuffer*)iBuffer)->length) 
	{
		MemCopyByInt(oBuffer->data, ((RsslBuffer*)iBuffer)->data, ((RsslBuffer*)iBuffer)->length);
		oBuffer->data[((RsslBuffer*)iBuffer)->length] = '\0';
		oBuffer->length = ((RsslBuffer*)iBuffer)->length; /* (consistently with other functions, null-term doesn't count toward length) */
		return RSSL_RET_SUCCESS;
	}
	return RSSL_RET_FAILURE;
}



/*************/
/*  External */
/*************/


RSSL_API RsslRet rsslEncodedPrimitiveToString( RsslDecodeIterator *pIter, RsslPrimitiveType primitiveType,	RsslBuffer *oBuffer)
{
	RsslRet ret;

	RSSL_ASSERT(oBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oBuffer->data, Invalid parameter);

	if ((primitiveType >= RSSL_DT_SET_PRIMITIVE_MIN) || (!(_rsslDataTypeInfo[primitiveType].rawToString)))
		return RSSL_RET_FAILURE;
	
	return(((ret = (*(_rsslDataTypeInfo[primitiveType].rawToString))(pIter, oBuffer)) < 0) ?
			RSSL_RET_FAILURE :
			ret);
}

RSSL_API RsslRet rsslPrimitiveToString( void *pType, RsslPrimitiveType primitiveType,	RsslBuffer *oBuffer)
{
	RsslRet ret;

	RSSL_ASSERT(oBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pType, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oBuffer->data, Invalid parameter);
	
	if ((primitiveType >= RSSL_DT_SET_PRIMITIVE_MIN) || (!(_rsslDataTypeInfo[primitiveType].typeToString)))
		return RSSL_RET_FAILURE;
	
	return(((ret = (*(_rsslDataTypeInfo[primitiveType].typeToString))(pType, oBuffer)) < 0) ?
			RSSL_RET_FAILURE :
			ret);
}

/********************************************************/
/* External interface that takes a void* to encode from */
/********************************************************/

RSSL_API RsslRet rsslDecodePrimitiveType( RsslDecodeIterator *pIter, RsslPrimitiveType primitiveType,	void *oValue)
{
	RsslRet ret;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oValue, Invalid parameters or parameters passed in as NULL);

	if ((primitiveType >= RSSL_DT_SET_PRIMITIVE_MIN) || (!(_rsslDataTypeInfo[primitiveType].bufferDecoders)))
		return RSSL_RET_FAILURE;
	
	return(((ret = (*(_rsslDataTypeInfo[primitiveType].bufferDecoders))(pIter, oValue)) < 0) ?
			RSSL_RET_FAILURE :
			ret);
}


/***************************************/
/* External Interface Decode Functions */
/***************************************/

RSSL_API RsslRet rsslDecodeInt(RsslDecodeIterator *pIter, RsslInt64 *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		 (rwfGetLenSpecI64Size((RsslInt64*)value, pIter->_curBufPtr, (rtrUInt8)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeUInt(RsslDecodeIterator *pIter, RsslUInt64 *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		(rwfGetLenSpecU64Size((RsslUInt64*)value, pIter->_curBufPtr, (rtrUInt8)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeFloat(RsslDecodeIterator *pIter, RsslFloat *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	/* always 4 bytes */
	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		 ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 4) ?
		  rwfGetFloat((*value), pIter->_curBufPtr),
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeDouble(RsslDecodeIterator *pIter, RsslDouble *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
		((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 8) ?
		  rwfGetDouble((*value), pIter->_curBufPtr),
		RSSL_RET_SUCCESS :
		RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeReal(RsslDecodeIterator *pIter, RsslReal *value)
{
	RsslUInt8 format;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0: 
			value->isBlank = RSSL_TRUE;
			value->hint = 0;
			value->value = 0;
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
			rwfGet8(format, pIter->_curBufPtr);
			switch(format & 0x3F)
			{
				case RSSL_RH_INFINITY:
				case RSSL_RH_NEG_INFINITY:
				case RSSL_RH_NOT_A_NUMBER:
					value->hint = format & 0x3F;
					value->isBlank = RSSL_FALSE;
				break;
				default:
					value->isBlank = RSSL_TRUE;
					value->hint = 0;
				break;
			}
			value->value = 0;
			return RSSL_RET_SUCCESS;
		break;
		default:
			if (rwfGetReal64(&value->value, &format, (rtrUInt16)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr)), pIter->_curBufPtr ) > 0)
			{
				switch(format & 0x3F)
				{
					case RSSL_RH_INFINITY:
					case RSSL_RH_NEG_INFINITY:
					case RSSL_RH_NOT_A_NUMBER:
						value->hint = format & 0x3F;
						value->isBlank = RSSL_FALSE;
					break;
					default:
						value->isBlank = (format & 0x20) ? RSSL_TRUE : RSSL_FALSE;
						value->hint = format & 0x1F;
					break;
				}

				if (!(value->hint >= RSSL_RH_EXPONENT_14 && value->hint <= RSSL_RH_NOT_A_NUMBER && value->hint != 31 && value->hint != 32))
				{
					return RSSL_RET_INVALID_DATA;
				}

				return RSSL_RET_SUCCESS;
			}
			return RSSL_RET_INVALID_DATA;
	}
	return RSSL_RET_INVALID_DATA;
}

RSSL_API RsslRet rsslDecodeDate(RsslDecodeIterator *pIter, RsslDate *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			value->day = 0,
			value->month = 0,
			value->year = 0,
			RSSL_RET_BLANK_DATA :
		((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 4) ?
		  rwfGet8(value->day, pIter->_curBufPtr),
		  rwfGet8(value->month, (pIter->_curBufPtr + 1)),
		  rwfGet16(value->year, (pIter->_curBufPtr + 2)),
		(((value->day == 0) && (value->year == 0) && (value->month == 0)) ?
			RSSL_RET_BLANK_DATA : 
			RSSL_RET_SUCCESS) :
		RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeTime(RsslDecodeIterator *pIter, RsslTime *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			rsslClearTime(value);			
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 2:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));

			return (((value->hour == 255) && (value->minute == 255)) ?
					(value->second = 255, value->millisecond = 65535, value->microsecond = 2047, value->nanosecond = 2047, RSSL_RET_BLANK_DATA) :
					(value->second = 0, value->millisecond = 0, value->microsecond = 0, value->nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 3:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));

			return (((value->hour == 255) && (value->minute == 255) && (value->second == 255)) ?
					(value->millisecond = 65535, value->microsecond = 2047, value->nanosecond = 2047, RSSL_RET_BLANK_DATA) : 
					(value->millisecond = 0, value->microsecond = 0, value->nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 4: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 5:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));
			rwfGet16(value->millisecond, (pIter->_curBufPtr + 3));

			return (((value->hour == 255) && (value->minute == 255) && (value->second == 255) && (value->millisecond == 65535)) ?
					(value->microsecond = 2047, value->nanosecond = 2047, RSSL_RET_BLANK_DATA) : 
					(value->microsecond = 0, value->nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 7:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));
			rwfGet16(value->millisecond, (pIter->_curBufPtr + 3));
			rwfGet16(value->microsecond, (pIter->_curBufPtr + 5));

			return (((value->hour == 255) && (value->minute == 255) && (value->second == 255) && (value->millisecond == 65535) && (value->microsecond == 2047)) ?
					(value->nanosecond = 2047, RSSL_RET_BLANK_DATA) : 
					(value->nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 8:
		{
			RsslUInt16 tempMicro;
			RsslUInt8 tempNano;

			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));
			rwfGet16(value->millisecond, (pIter->_curBufPtr + 3));
			rwfGet16(tempMicro, (pIter->_curBufPtr + 5));
			rwfGet8(tempNano, (pIter->_curBufPtr + 7));

			value->microsecond = tempMicro & 0x07FF;
			value->nanosecond = (((tempMicro & 0x3800) >> 3) + tempNano);

			return (((value->hour == 255) && (value->minute == 255) && (value->second == 255) && (value->millisecond == 65535) && (value->microsecond == 2047) && (value->nanosecond == 2047)) ?
					(RSSL_RET_BLANK_DATA) : 
					(RSSL_RET_SUCCESS));
		}
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}


RSSL_API RsslRet rsslDecodeDateTime(RsslDecodeIterator *pIter, RsslDateTime *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			rsslClearDateTime(value);
			return RSSL_RET_BLANK_DATA;
		break;
		case 6:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));

			/* need this to populate rest of time properly */
			/* if the time we took from wire matches blank, fill in rest of time as blank, 
			 * then ensure that date portion is also blank - if so, return blank data.  
			 * If time portion was not blank, just return success */
			return (((value->time.hour == 255) && (value->time.minute == 255)) ?
					(value->time.second = 255, value->time.millisecond = 65535, value->time.microsecond = 2047, value->time.nanosecond = 2047,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.second = 0,value->time.millisecond = 0,value->time.microsecond = 0,value->time.nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 7:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));

			/* need this to populate rest of time properly */
			/* if the time we took from wire matches blank, fill in rest of time as blank, 
			 * then ensure that date portion is also blank - if so, return blank data.  
			 * If time portion was not blank, just return success */
			return (((value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255)) ?
					(value->time.millisecond = 65535, value->time.microsecond = 2047, value->time.nanosecond = 2047,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.millisecond = 0, value->time.microsecond = 0, value->time.nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 9:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(value->time.millisecond, (pIter->_curBufPtr + 7));


			return (((value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255) && (value->time.millisecond == 65535)) ?
					(value->time.microsecond = 2047, value->time.nanosecond = 2047,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.microsecond = 0, value->time.nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 11:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(value->time.millisecond, (pIter->_curBufPtr + 7));
			rwfGet16(value->time.microsecond, (pIter->_curBufPtr + 9));

			return (((value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255) && (value->time.millisecond == 65535) && (value->time.microsecond == 2047)) ?
					(value->time.nanosecond = 2047,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.nanosecond = 0, RSSL_RET_SUCCESS));
		break;
		case 12:
		{
			RsslUInt16 tempMicro;
			RsslUInt8 tempNano;

			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(value->time.millisecond, (pIter->_curBufPtr + 7));
			rwfGet16(tempMicro, (pIter->_curBufPtr + 9));
			rwfGet8(tempNano, (pIter->_curBufPtr + 11));

			value->time.microsecond = tempMicro & 0x07FF;
			value->time.nanosecond = (((tempMicro & 0x3800) >> 3) + tempNano);

			if ((value->date.day == 0) && (value->date.year == 0) && (value->date.month == 0) &&
				(value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255) &&
				(value->time.millisecond == 65535) && (value->time.microsecond == 2047) && (value->time.nanosecond == 2047))
				return RSSL_RET_BLANK_DATA;
			else
				return RSSL_RET_SUCCESS;
		}
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}


RSSL_API RsslRet rsslDecodeQos(RsslDecodeIterator *pIter, RsslQos *value)
{
	RsslUInt8 qosValue;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
			RSSL_RET_BLANK_DATA :
	  ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) >= 1) ?
		rwfGet8(qosValue, pIter->_curBufPtr),
		value->timeliness = qosValue >> 5,
		value->rate = (qosValue >> 1) & 0xF,
		value->dynamic = qosValue & 0x1,
		((value->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ?
		 rwfGet16(value->timeInfo, (pIter->_curBufPtr + 1)) :
	     (value->timeInfo = 0)),
		((value->rate > RSSL_QOS_RATE_JIT_CONFLATED) ?
		 ((value->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ?
		 rwfGet16(value->rateInfo, (pIter->_curBufPtr + 3)) : 
		 rwfGet16(value->rateInfo, (pIter->_curBufPtr + 1))) :
		 (value->rateInfo = 0)),
	   RSSL_RET_SUCCESS :
	   RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeState(RsslDecodeIterator *pIter, RsslState *value)
{
	RsslUInt8 state;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
		RSSL_RET_BLANK_DATA :
	  ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) >= 3) ?
		rwfGet8(state, pIter->_curBufPtr),
		rwfGet8(value->code, (pIter->_curBufPtr + 1)),
		_rsslDecodeBuffer15(&value->text, pIter->_curBufPtr + 2),
		value->streamState = state >> 3,
		value->dataState = state & 0x7,
	   RSSL_RET_SUCCESS :	
	   RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeEnum(RsslDecodeIterator *pIter, RsslEnum *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	return( ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr) == 0) ?
		RSSL_RET_BLANK_DATA :
		(rwfGetLenSpecU16_Size(value, pIter->_curBufPtr, (rtrUInt16)((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))) > -1) ?
	   RSSL_RET_SUCCESS :
	   RSSL_RET_INCOMPLETE_DATA );
}

RSSL_API RsslRet rsslDecodeBuffer(RsslDecodeIterator *pIter, RsslBuffer *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	value->length = (rtrUInt32)(pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr);

	if (value->length == 0)
	{
		value->data = 0;
		return RSSL_RET_BLANK_DATA;
	}
	else
	{
		value->data = pIter->_curBufPtr;
		return RSSL_RET_SUCCESS;
	}
}


