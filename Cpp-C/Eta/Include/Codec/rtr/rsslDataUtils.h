/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2019,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_DATA_UTILS_H
#define __RSSL_DATA_UTILS_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_WIN32) || defined(WIN32)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#define RTR_STRNICMP(X,Y,Z) _strnicmp(X,Y,Z)
#else
#include <strings.h>
#define RTR_STRNICMP(X,Y,Z) strncasecmp(X,Y,Z)
#endif

/**
 *	@addtogroup RSSLWFDataCommonHelpers
 *	@{
 */

/**
 * @brief Compare the content of two RsslBuffers 
 * @return RSSL_TRUE if the buffers contain the same data, RSSL_FALSE if not.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslBufferIsEqual(const RsslBuffer *pBuf1, const RsslBuffer *pBuf2)
{
	return (pBuf1->length == pBuf2->length && !memcmp(pBuf1->data, pBuf2->data, pBuf1->length)) ? RSSL_TRUE : RSSL_FALSE;
}

/**
 * @brief DEPRECATED: Compare two RsslBuffers as strings. Users should migrate to use rsslBufferIsEqual.
 * May be used on strings that are not necessarily null-terminated, making it useful for
 * comparing things like element names.
 * Note: Do not call with null arguments.
 * @return Positive value if pBuf1 is lexically greater, 0 if they are equal, or negative value if pBuf2 is lexically greater.
 */
RTR_C_ALWAYS_INLINE RsslInt32 rsslCompareString(const RsslBuffer *pBuf1, const RsslBuffer *pBuf2)
{
	RsslUInt32 len;

	/* Get the shorter length */
	len = (pBuf1->length < pBuf2->length) ? pBuf1->length : pBuf2->length;

	/* Match the ends of the strings -- either they should be the same length,
	 * or the longer one should have a null-terminator where the other one ends. */
	if (pBuf1->length > len && pBuf1->data[len] != '\0') return 1;
	if (pBuf2->length > len && pBuf2->data[len] != '\0') return -1;
	
	/* If the above succeeded, strnicmp is sufficient because it will compare 
	 * all relevant characters (or up to null-terminators if they are found). */
	return strncmp(pBuf1->data, pBuf2->data, len);
}

/**
 * @brief DEPRECATED: Compare two RsslBuffers as strings(ignoring case). Users should migrate to use rsslBufferIsEqual.
 * May be used on strings that are not necessarily null-terminated, making it useful for
 * comparing things like element names.
 * Note: Do not call with null arguments.
 * @return Positive value if pBuf1 is lexically greater, 0 if they are equal, or negative value if pBuf2 is lexically greater.
 */
RTR_C_ALWAYS_INLINE RsslInt32 rsslCompareStringIgnoreCase(const RsslBuffer *pBuf1, const RsslBuffer *pBuf2)
{
	RsslUInt32 len;

	/* Get the shorter length */
	len = (pBuf1->length < pBuf2->length) ? pBuf1->length : pBuf2->length;

	/* Match the ends of the strings -- either they should be the same length,
	 * or the longer one should have a null-terminator where the other one ends. */
	if (pBuf1->length > len && pBuf1->data[len] != '\0') return 1;
	if (pBuf2->length > len && pBuf2->data[len] != '\0') return -1;
	
	/* If the above succeeded, strnicmp is sufficient because it will compare 
	 * all relevant characters (or up to null-terminators if they are found). */
	return RTR_STRNICMP(pBuf1->data, pBuf2->data, len);
}


/**
 * @}
 */


/** 
 *  @addtogroup RSSLWFDataConv
 * @{
 */

/* @brief Translates certain RSSL type enumerations to their respective basic primitive type.
 * Useful mainly for set types, e.g. UINT_1 returns UINT.
 * See RsslDataType
 */
 RSSL_API RsslDataType rsslPrimitiveBaseType(RsslDataType dataType);





/**
 * @}
 */



/** 
 * @addtogroup RSSLPrimitiveEncoders
 * @{
 */

 /**
 * @brief Begin encoding of a Non-RWF type (e.g. AnsiPage, Opaque, XML, etc)
 * This allows a user to obtain a starting position to encode from as well as 
 * length of data left in the buffer to encode into.  User can then leverage
 * non-RWF encoding functions/APIs or memcpy routines to nest non-RWF data 
 * into the buffer.  
 * Typical use:<BR>
 *	1. Use your rssl Init function (e.g. rsslEncodeMsgInit, rsslEncodeMapEntryInit) for the container you intend to nest non-RWF data into<BR>
 *  2. Call rsslEncodeNonRWFDataTypeInit with the iterator being used for encoding and a RsslBuffer structure.<BR>
 *  3. rsslEncodeNonRWFDataTypeInit will populate the pBuffer->data with the address where encoding should begin and the pBuffer->length with the amount of space left to encode into<BR>
 *	4. When NonRWF encoding is complete, set pBuffer->length to actual number of bytes encoded and pass pIter and pBuffer to rsslEncodeNonRWFDataTypeComplete<BR>
 *  5. Finish RWF container encoding (e.g. message, map entry, etc).  <BR>
 *  Note: User should ensure not to encode or copy more than pBuffer->length indicates is available <BR>
 * @param pIter RsslEncodeIterator used for encoding container
 * @param pBuffer RsslBuffer to populate with pointer to encode to and available length to encode into
 */
 RSSL_API RsslRet rsslEncodeNonRWFDataTypeInit(RsslEncodeIterator *pIter, RsslBuffer *pBuffer);

 /**
 * @brief Complete encoding of a Non-RWF type (e.g. AnsiPage, Opaque, XML, etc)
 * This allows a user to complete encoding of a Non-RWF type.  User must set
 * buffer.length to the amount of data encoded or copied into the buffer.data.  
 * Typical use:<BR>
 *	1. Use your rssl Init function (e.g. rsslEncodeMsgInit, rsslEncodeMapEntryInit) for the container you intend to nest non-RWF data into<BR>
 *  2. Call rsslEncodeNonRWFDataTypeInit with the iterator being used for encoding and a RsslBuffer structure.<BR>
 *  3. rsslEncodeNonRWFDataTypeInit will populate the pBuffer->data with the address where encoding should begin and the pBuffer->length with the amount of space left to encode into<BR>
 *	4. When NonRWF encoding is complete, set pBuffer->length to actual number of bytes encoded and pass pIter and pBuffer to rsslEncodeNonRWFDataTypeComplete<BR>
 *  5. Finish RWF container encoding (e.g. message, map entry, etc).  <BR>
 *  Note: User should ensure not to encode or copy more than pBuffer->length indicates is available <BR>
 * @param pIter RsslEncodeIterator used for encoding container
 * @param pBuffer RsslBuffer obtained from rsslEncodeNonRWFDataTypeInit.  pBuffer->length should be set to number of bytes put into pBuffer->data.  pBuffer->data must remain unchanged.  
 * @param success if RSSL_TRUE, encoding was successful and RSSL will complete transfer of data.  If RSSL_FALSE, RSSL will roll back to last successful encoding prior to Non-RWF Init call.
 */
 RSSL_API RsslRet rsslEncodeNonRWFDataTypeComplete(RsslEncodeIterator *pIter, RsslBuffer *pBuffer, RsslBool success);


 
/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
