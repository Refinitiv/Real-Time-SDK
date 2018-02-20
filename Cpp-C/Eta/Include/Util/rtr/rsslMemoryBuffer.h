/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef RSSL_MEMORY_BUFFER_H
#define RSSL_MEMORY_BUFFER_H 
#include "rtr/rsslTypes.h"
#include "rtr/rsslVAExports.h"

#ifndef WIN32
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup RSSLVAUtils
 *	@{
 */

/**
 * @brief Reserves a portion of an RsslBuffer for use as an array of one or more elements.
 * @param pBuffer The buffer to apportion from.  The buffer will be changed to point to the remaining unused memory.
 * @param elementCount The number of elements of to reserve.
 * @param sizeOfElement The size of the element(s) to reserve.
 * @return Pointer to the start of the reserved memory.
 * @return NULL if the buffer did not have enough space for the requested memory.
 * @see rsslReserveAlignedBufferMemory, rsslCopyBufferMemory
 */
RSSL_API void * rsslReserveBufferMemory(RsslBuffer *pBuffer, RsslUInt32 elementCount, RsslUInt32 sizeofElement);

/**
 * @brief Reserves a portion of an RsslBuffer for use as an array of one or more elements.
 * Works like rsslReserveBufferMemory() but first aligns the memory on a boundary appropriate for the size of the element.
 * @param pBuffer The buffer to apportion from.  The buffer will be changed to point to the remaining unused memory.
 * @param elementCount The number of elements of to reserve.
 * @param sizeOfElement The size of the element(s) to reserve.
 * @return Pointer to the start of the reserved memory.
 * @return NULL if the buffer did not have enough space for the requested memory.
 * @see rsslReserveBufferMemory, rsslCopyBufferMemory
 */
/* Reserves memory from an allocated block.
 * Similar to rsslReserveBufferMemory, but ensures the memory is aligned on an appropriate boundary. */
RSSL_API void * rsslReserveAlignedBufferMemory(RsslBuffer *pBuffer, RsslUInt32 elementCount, RsslUInt32 sizeOfElement);

/**
 * @brief Reserves a portion of an RsslBuffer for use as a string and copies the given string.
 * @param pOutBuf the RsslBuffer that will be populated to point to the copy of the string.
 * @param pInBuf the RsslBuffer containing the string to copy.
 * @param pMemoryBuffer RsslBuffer pointing to memory that will be used to hold the string. The buffer will be changed to point to the remaining unused memory.
 * @return Pointer to the start of the reserved memory.
 * @return NULL if the buffer did not have enough space for the requested memory.
 * @see rsslReserveBufferMemory, rsslReserveAlignedBufferMemory
 */
RSSL_API char *rsslCopyBufferMemory(RsslBuffer *pOutBuf, RsslBuffer *pInBuf, RsslBuffer *pMemoryBuffer);

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
