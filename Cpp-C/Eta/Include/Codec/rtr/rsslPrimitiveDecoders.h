/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_PRIMITIVE_DECODERS_H
#define __RSSL_PRIMITIVE_DECODERS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslState.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslIterators.h"



/** 
 * @addtogroup RsslGeneralUtils
 * @{
 */

/** 
 * @brief Encoded Primitive Data To String
 * @param pIter RsslDecodeIterator with buffer to decode and convert from. Iterator should also have appropriate version information set
 * @param primitiveType type of input buffer
 * @param oBuffer Output Buffer to put string into
 */
RSSL_API RsslRet rsslEncodedPrimitiveToString(
						RsslDecodeIterator *pIter,
						RsslPrimitiveType primitiveType,
						RsslBuffer *oBuffer);




/** 
 * @brief Primitive To String
 * @param pType the primitive object to convert to a string.
 * @param primitiveType type of the object
 * @param oBuffer Output Buffer to put string into
 */
RSSL_API RsslRet rsslPrimitiveToString(
						void *pType,
						RsslPrimitiveType primitiveType,
						RsslBuffer *oBuffer);

/**
 * @}
 */

/** 
 * @addtogroup RsslGeneralDec
 * @{
 */

/** 
 * @brief Used to decode any primitive type (e.g. \ref RsslInt, RsslReal, RsslDate) from buffer referred to by \ref RsslDecodeIterator.  This function cannot be used for RsslArray decoding.
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information. * @param primitiveType primitive type contained in iterator
 * @param primitiveType RsslPrimitiveType enumeration corresponding to the primitive type to decode (e.g. ::RSSL_DT_REAL)
 * @param oValue void pointer to primitive type representation to decode into (e.g. \ref RsslInt, RsslReal)
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and primitive type is blank value, , ::RSSL_RET_FAILURE if invalid parameter or invalid primitive type is passed
 */
RSSL_API RsslRet rsslDecodePrimitiveType( RsslDecodeIterator *pIter, 
									  RsslPrimitiveType primitiveType,	
									  void *oValue);

/**
 * @}
 */

/**
 * @addtogroup RsslIntDec
 * @{
 */
									  
/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslInt
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslInt primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RSSL_API RsslRet rsslDecodeInt(
					RsslDecodeIterator *pIter,
					RsslInt *value);

/**
 * @}
 */

/**
 * @addtogroup RsslUIntDec
 * @{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslUInt
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslUInt primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RSSL_API RsslRet rsslDecodeUInt(
					RsslDecodeIterator *pIter,
					RsslUInt *value);

/**
 * @}
 */

/**
 * @addtogroup RsslFloatDec
 * @{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslFloat
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslFloat primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RSSL_API RsslRet rsslDecodeFloat(
					RsslDecodeIterator *pIter, 
					RsslFloat *value);

/**
 * @}
 */


/**
 * @addtogroup RsslDoubleDec
 * @{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslDouble
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslDouble primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RSSL_API RsslRet rsslDecodeDouble(
					RsslDecodeIterator *pIter,
					RsslDouble *value);


/** 
 * @}
 */


/**
 * @addtogroup RsslEnumDec
 * @{
 */


/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslEnum 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslEnum to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RSSL_API RsslRet rsslDecodeEnum(
					RsslDecodeIterator *pIter,
					RsslEnum *value);

/**
 * @}
 */



/**
 * @addtogroup RsslBufferDec
 * @{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslBuffer 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslBuffer to decode content into.  \ref RsslBuffer::data will point to contents in iterator's buffer and \ref RsslBuffer::length will indicate length of content.
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RSSL_API RsslRet rsslDecodeBuffer(
					RsslDecodeIterator *pIter,
					RsslBuffer *value);
					
/**
 * @}
 */
 
/**
 *	@addtogroup RsslRealDec
 *	@{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslReal
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslReal to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RSSL_API RsslRet rsslDecodeReal(
					RsslDecodeIterator *pIter,
					RsslReal *value );

/**
 *	@}
 */
 
/**
 *	@addtogroup RsslQosDec
 *	@{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslQos
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslQos to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RSSL_API RsslRet rsslDecodeQos(
					RsslDecodeIterator *pIter, 
					RsslQos *value );

/**
 *	@}
 */
 
/**
 *	@addtogroup RsslStateDec
 *	@{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslState
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslState to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RSSL_API RsslRet rsslDecodeState(
					RsslDecodeIterator *pIter,
					RsslState *value);

/**
 *	@}
 */

/**
 *	@addtogroup RsslDateDec
 *	@{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslDate 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslDate to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RSSL_API RsslRet rsslDecodeDate(
					RsslDecodeIterator *pIter,
					RsslDate *value );


/**
 * @}
 */


/**
 *	@addtogroup RsslTimeDec
 *	@{
 */

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslTime 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslTime to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RSSL_API RsslRet rsslDecodeTime(
					RsslDecodeIterator *pIter,
					RsslTime *value );


/**
 * @}
 */

/**
 *	@addtogroup RsslDateTimeDec
 *	@{
 */

/**
 * @brief Decode DateTime 
 * @param pIter RsslDecodeIterator with buffer to decode from and appropriate version information set
 * @param value dataType to put decoded data into
 * @return RSSL_RET_SUCCESS if success, RSSL_RET_INCOMPLETE_DATA if failure, RSSL_RET_BLANK_DATA if data is blank value
 */
RSSL_API RsslRet rsslDecodeDateTime(
					RsslDecodeIterator *pIter,
					RsslDateTime *value);

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif 



#endif

