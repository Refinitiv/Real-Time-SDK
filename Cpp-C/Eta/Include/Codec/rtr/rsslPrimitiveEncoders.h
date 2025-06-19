/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_PRIMITIVE_ENCODERS_H
#define __RSSL_PRIMITIVE_ENCODERS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslState.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslIterators.h"


/** 
 * @addtogroup RsslGeneralEnc
 * @{
 */

/**
 * @brief Used to encode any primitive type (e.g. \ref RsslInt, RsslReal, RsslDate) into buffer referred to by \ref RsslEncodeIterator.  This function cannot be used for RsslArray encoding.
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param primitiveType RsslPrimitiveType enumeration corresponding to the primitive type to encode (e.g. ::RSSL_DT_REAL)
 * @param pData void pointer to primitive type representation to encode from (e.g. \ref RsslInt, RsslReal)
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if contents exceed length of encode buffer referred to by \ref RsslEncodeIterator, ::RSSL_RET_FAILURE if invalid parameter or invalid primitive type is passed
 */
RSSL_API RsslRet rsslEncodePrimitiveType( 
					RsslEncodeIterator  *pIter,
					RsslPrimitiveType	primitiveType,
					const void			*pData );

/**
 * @}
 */

/**
 * @addtogroup RsslIntEnc
 * @{
 */

/** 
 * @brief Used to encode an  \ref RsslInt into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pInt The \ref RsslInt value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslInt contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeInt(RsslEncodeIterator *pIter, const RsslInt *pInt);

/**
 * @}
 */

/**
 * @addtogroup RsslUIntEnc
 * @{
 */

/** 
 * @brief Used to encode an  \ref RsslUInt into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pUInt The \ref RsslUInt value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslUInt contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeUInt(RsslEncodeIterator *pIter, const RsslUInt *pUInt);

/**
 * @}
 */


/**
 * @addtogroup RsslFloatEnc
 * @{
 */

/** 
 * @brief Used to encode an \ref RsslFloat into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pFloat The \ref RsslFloat value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslFloat contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeFloat(RsslEncodeIterator *pIter, const RsslFloat *pFloat);

/**
 * @}
 */


/**
 * @addtogroup RsslDoubleEnc
 * @{
 */

/** 
 * @brief Used to encode an \ref RsslDouble into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pDouble The \ref RsslDouble value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslDouble contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeDouble(RsslEncodeIterator *pIter, const RsslDouble *pDouble);

/**
 * @}
 */


/**
 * @addtogroup RsslEnumEnc
 * @{
 */

/** 
 * @brief Used to encode an \ref RsslEnum into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pEnum The \ref RsslEnum to encode.
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslEnum contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeEnum(RsslEncodeIterator *pIter, const RsslEnum *pEnum);

/**
 *	@}
 */

/**
 * @addtogroup RsslBufferEnc
 * @{
 */

/** 
 * @brief Used to encode an \ref RsslBuffer into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pBuffer The \ref RsslBuffer to encode, where \ref RsslBuffer::data points to the content and \ref RsslBuffer::length indicates the length of content.
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslBuffer contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeBuffer(RsslEncodeIterator *pIter, const RsslBuffer *pBuffer);

/**
 *	@}
 */
 
/**
 *	@addtogroup RsslRealEnc
 *	@{
 */
 
/** 
 * @brief Used to encode an RsslReal into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pReal The RsslReal value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslReal contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeReal(RsslEncodeIterator *pIter, const RsslReal *pReal);

/**
 *	@}
 */

/**
 *	@addtogroup RsslQosEnc
 *	@{
 */
 
/** 
 * @brief Used to encode an RsslQos into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pQos The RsslQos value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslQos contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeQos(RsslEncodeIterator *pIter, const RsslQos *pQos );

/**
 *	@}
 */
 
/**
 *	@addtogroup RsslStateEnc
 *	@{
 */
 
/** 
 * @brief Used to encode an RsslState into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pState The RsslState value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslState contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeState(RsslEncodeIterator *pIter, const RsslState *pState);

/**
 *	@}
 */

/**
 *	@addtogroup RsslDateEnc
 *	@{
 */

/** 
 * @brief Used to encode an RsslDate into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pDate The RsslDate value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if RsslDate contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeDate(RsslEncodeIterator *pIter, const RsslDate *pDate);

/**
 * @}
 */



/**
 *	@addtogroup RsslTimeEnc
 *	@{
 */


/** 
 * @brief Used to encode an RsslTime into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pTime The RsslTime value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if RsslTime contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RSSL_API RsslRet rsslEncodeTime(RsslEncodeIterator *pIter, const RsslTime *pTime);


/**
 * @}
 */

/**
 *	@addtogroup RsslDateTimeEnc
 *	@{
 */
/** 
 * @brief Used to encode a datetime into a buffer 
 * @param pIter RsslEncodeIterator with buffer to encode into. Iterator should also have appropriate version information set
 * @param pDateTime datetime value to encode
 */
RSSL_API RsslRet rsslEncodeDateTime(RsslEncodeIterator *pIter, const RsslDateTime *pDateTime);


/**
 *	@}
 */

#ifdef __cplusplus
}
#endif


#endif 

