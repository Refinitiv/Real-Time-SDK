/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_INT_DATA_TYPE_H
#define __RSSL_INT_DATA_TYPE_H

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"
#include "rtr/retmacros.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslMap.h"
#include "rtr/rwfNet.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 ** @brief Checks for buffer overrun if length bytes appended, given the
 ** end pointer.
 ** @see RsslBuffer
 **/
#define _rsslBufferOverrunEndPtr( curPtr, length, endPtr ) \
	((curPtr + length) > endPtr)


/* Jump table declarations */
typedef enum
{
	RsslDTFValid		= 0x0001,
	RsslDTFLenSpecEnc 	= 0x0002,	/* Length Specified Encoding */
	RsslDTFFixLenEnc	= 0x0004,	/* Fixed Length Encoding */
	RsslDTFVarLenEnc	= 0x0008	/* Variable Length Encoding */
} RsslDataTypeFlags;

typedef struct
{
	RsslDataType	dataType;
	RsslDataType	primdataType;
	RsslUInt8		maxEncodedSize;
	RsslUInt16		flags;
	RsslUInt16		assertions;

	/* Length Specified Encoder and Decoder */
	RsslRet	(RTR_FASTCALL *rawEncoders)(rsslEncIterPtr,const void*);
	RsslRet	(RTR_FASTCALL *rawDecoders)(rsslDecIterPtr,RsslBuffer*);
	RsslRet	(RTR_FASTCALL *rawToString)(rsslDecIterPtr,RsslBuffer*);
	RsslRet (RTR_FASTCALL *typeToString)(void*, RsslBuffer*);
	RsslRet	(RTR_FASTCALL *bufferEncoders)(rsslEncIterPtr, const void*);
	RsslRet	(RTR_FASTCALL *bufferDecoders)(rsslDecIterPtr, void*);

	const char		*string;
} RsslDataTypeInfo;

extern 			RsslDataTypeInfo	_rsslDataTypeInfo[];

#define _rsslPrimitiveType(DT) \
	( (DT < RSSL_DT_LAST) ? (_rsslDataTypeInfo[DT].primdataType) : RSSL_DT_UNKNOWN)

#define _rsslEncodePrimitive( PITER, DT, PDATA ) \
	( (DT < RSSL_DT_SET_PRIMITIVE_MIN) && (_rsslDataTypeInfo[DT].rawEncoders != 0) ?  \
			(*(_rsslDataTypeInfo[DT].rawEncoders))(PITER,PDATA) : \
			RSSL_RET_UNSUPPORTED_DATA_TYPE )

#define _rsslEncodeSet( PITER, DT, PDATA ) \
	( (DT < RSSL_DT_CONTAINER_TYPE_MIN) && (_rsslDataTypeInfo[DT].rawEncoders != 0) ?  \
			(*(_rsslDataTypeInfo[DT].rawEncoders))(PITER,PDATA) : \
			RSSL_RET_UNSUPPORTED_DATA_TYPE )

#define _rsslDecodeSet( PITER, DT, PBUFFER ) \
	( (DT < RSSL_DT_LAST) && (_rsslDataTypeInfo[DT].rawDecoders != 0) ?  \
			(*(_rsslDataTypeInfo[DT].rawDecoders))(PITER,PBUFFER) : \
			RSSL_RET_UNSUPPORTED_DATA_TYPE )

#define _rsslDecodeItem( PITER, DT, PBUFFER ) \
	( (DT < RSSL_DT_LAST) && (_rsslDataTypeInfo[DT].rawDecoders != 0) ?  \
			(*(_rsslDataTypeInfo[DT].rawDecoders))(PITER,PBUFFER) : \
			RSSL_RET_UNSUPPORTED_DATA_TYPE )

RTR_C_ALWAYS_INLINE RsslBool _rsslValidPrimitiveDataType( RsslDataType type )
{
	return((type < RSSL_DT_SET_PRIMITIVE_MIN) ? (_rsslDataTypeInfo[type].flags != 0) : 0);
}

RTR_C_ALWAYS_INLINE RsslBool _rsslValidAggregateDataType( RsslDataType type )
{
	RsslInt16 minType = RSSL_DT_CONTAINER_TYPE_MIN;
	RsslInt16 lastType = RSSL_DT_LAST;
	RsslInt16 maxRes = RSSL_DT_MAX_RESERVED;
	RsslInt16 maxType = RSSL_DT_CONTAINER_TYPE_MAX;
	/* Valid if known container type, or unreserved type */
	/* type 223 is an infra hack that we need to support */
	return((((type >= minType) && (type <= maxType)) ? (_rsslDataTypeInfo[type].flags != 0) : 0) 
		|| (type > maxRes && type <= lastType) || type == 223);
}

RTR_C_ALWAYS_INLINE RsslInt32 _rsslPrimitiveTypeMaxEncSize( RsslDataType type )
{
	return((type < RSSL_DT_CONTAINER_TYPE_MIN) ?
			(RsslInt32)(_rsslDataTypeInfo[type].maxEncodedSize) : 0);
}


#define __RSZVAR    0xFF    /* Variable length size */

#define __RSZI8     sizeof(RsslInt8)
#define __RSZUI8    sizeof(RsslUInt8)
#define __RSZI16    sizeof(RsslInt16)
#define __RSZUI16   sizeof(RsslUInt16)
#define __RSZI32    sizeof(RsslInt32)
#define __RSZUI32   sizeof(RsslUInt32)
#define __RSZI64    sizeof(RsslInt64)
#define __RSZUI64   sizeof(RsslUInt64)
#define __RSZFLT    sizeof(RsslFloat)
#define __RSZDBL    sizeof(RsslDouble)

#define __RSZDT     2 * sizeof(RsslUInt8) + sizeof(RsslUInt16)
#define __RSZTM     (4 * sizeof(RsslUInt8)) + (2 * sizeof(RsslUInt16))  /* full length time and time_8 */
#define __RSZDTM    __RSZDT + __RSZTM /* full length date time and datetime_12 */

#define __RSZTM3	3 * sizeof(RsslUInt8)
#define __RSZTM5	3 * sizeof(RsslUInt8) + sizeof(RsslUInt16)
#define __RSZTM7	(3 * sizeof(RsslUInt8)) + (2 * sizeof(RsslUInt16))
#define __RSZDTM7	__RSZDT + __RSZTM3
#define __RSZDTM9   __RSZDT + __RSZTM5
#define __RSZDTM11   __RSZDT + __RSZTM7

/* type * best/worst value */
#define __RSZQOS    sizeof(RsslUInt8) + sizeof(RsslUInt16) + sizeof(RsslUInt16)
	                /* flags */         /* timeliness */        /* rate */

#define __RSZRL32   sizeof(RsslUInt8) + sizeof(RsslInt32)
#define __RSZRL64   sizeof(RsslUInt8) + sizeof(RsslInt64)
                    /* storage + display */     /* price */

#define __RDTFLSPE  (RsslDTFValid | RsslDTFLenSpecEnc)
#define __RDTFFLEN  (RsslDTFValid | RsslDTFFixLenEnc)
#define __RDTFVLEN  (RsslDTFValid | RsslDTFVarLenEnc)


/* Length Specified encoding */
extern RsslRet RTR_FASTCALL _rsslEncInt(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncUInt(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncFloat(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDouble(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncReal(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDate(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncTime(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDateTime(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncQos(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncState(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncEnum(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncBuffer(rsslEncIterPtr,const void*);

/* Optimized base types */
extern RsslRet RTR_FASTCALL _rsslEncodeInt1(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt1(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeInt2(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt2(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeInt4(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt4(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeInt8(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt8(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncFloat_4(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDouble_8(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncReal_4rb(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncReal_8rb(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDate_4(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncTime_3(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncTime_5(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncTime_7(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncTime_8(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDateTime_7(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDateTime_9(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDateTime_11(rsslEncIterPtr,const void*);
extern RsslRet RTR_FASTCALL _rsslEncDateTime_12(rsslEncIterPtr,const void*);

/* Length Specified decoding */
extern RsslRet RTR_FASTCALL _rsslDecBuf8(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDecBuf16(rsslDecIterPtr, RsslBuffer*);

/* Set types decoding */
extern RsslRet RTR_FASTCALL _rsslDec8(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec16(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec24(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec32(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec40(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec56(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec64(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec72(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec88(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDec96(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDecReal_4rb(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDecReal_8rb(rsslDecIterPtr, RsslBuffer*);

/*	primitive types decoding */
extern RsslRet RTR_FASTCALL _rsslDecInt(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecUInt(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecFloat(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecDouble(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecReal(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecDate(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecTime(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecDateTime(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecQos(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecState(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecEnum(rsslDecIterPtr, void *value);
extern RsslRet RTR_FASTCALL _rsslDecBuffer16(rsslDecIterPtr, void *value);

/* decode types to string */
extern RsslRet RTR_FASTCALL _rsslIntAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslIntToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslUIntAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslUIntToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslFloatAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslFloatToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDoubleAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDoubleToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslRealAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslRealToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslEnumAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslEnumToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDateAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDateToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslTimeAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslTimeToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDateTimeAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslDateTimeToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslQosAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslQosToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslStateAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslStateToString(void*, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslBufferAsString(rsslDecIterPtr, RsslBuffer*);
extern RsslRet RTR_FASTCALL _rsslBufferToString(void*, RsslBuffer*);

/* encode types into buffer */
extern RsslRet RTR_FASTCALL _rsslEncodeInt_1( rsslEncIterPtr, const void *value );
extern RsslRet RTR_FASTCALL _rsslEncodeUInt_1(rsslEncIterPtr, const void *value );
extern RsslRet RTR_FASTCALL _rsslEncodeInt_2(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt_2(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeInt_4(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt_4(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeInt_8(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeUInt_8(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeReal(rsslEncIterPtr, const void *pData);
extern RsslRet RTR_FASTCALL _rsslEncodeFloat(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeDouble(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeQos(rsslEncIterPtr, const void *pData );
extern RsslRet RTR_FASTCALL _rsslEncodeState(rsslEncIterPtr, const void *pData);
extern RsslRet RTR_FASTCALL _rsslEncodeEnum2(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeEnum1(rsslEncIterPtr, const void *value);
extern RsslRet RTR_FASTCALL _rsslEncodeDate(rsslEncIterPtr, const void *pData);
extern RsslRet RTR_FASTCALL _rsslEncodeTime(rsslEncIterPtr, const void *pData);
extern RsslRet RTR_FASTCALL _rsslEncodeDateTime(rsslEncIterPtr, const void *pData);
extern RsslRet RTR_FASTCALL _rsslEncodeBuffer(rsslEncIterPtr, const void *pData);

 
#ifdef __cplusplus
}
#endif

#endif /* __RSSL_INT_DATA_TYPE_H */

