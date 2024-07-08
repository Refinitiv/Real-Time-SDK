/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslMemoryBuffer.h"

#ifndef WIN32
#include <stdint.h>
#endif

RSSL_API char *rsslCopyBufferMemory(RsslBuffer *pOutBuf, RsslBuffer *pInBuf, RsslBuffer *pMemoryBuffer)
{
	char *pBufferMem = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pInBuf->length, sizeof(char));

	if (pBufferMem)
	{
		memcpy(pBufferMem, pInBuf->data, pInBuf->length);
		pOutBuf->data = pBufferMem;
		pOutBuf->length = pInBuf->length;
	}
	return pBufferMem;
}

RSSL_API void * rsslReserveAlignedBufferMemory(RsslBuffer *pBuffer, RsslUInt32 elementCount, RsslUInt32 sizeOfElement)
{
	intptr_t alignMask;
	RsslInt32 i;
	char *pCurPos = pBuffer->data;

	/* Create alignment mask based on size */
	if (sizeOfElement >= sizeof(long))
		alignMask = sizeof(long) - 1;
	else
	{
		alignMask = sizeOfElement - 1;
		for (i = 1; i <= sizeof(long)/2; i *= 2)
			alignMask |= (alignMask >> i);
	}

	/* Use mask to see if address is aligned.
	 * If it isn't, move pointer to next aligned address.  */
	if ((intptr_t)pCurPos & alignMask) 
	{
		pCurPos = (char*)((intptr_t)pCurPos & ~alignMask) + (alignMask + 1);
		if ((uintptr_t)(pCurPos - pBuffer->data) >= pBuffer->length || pCurPos < pBuffer->data /* overflow */) 
			return NULL;

		pBuffer->length -= (RsslUInt32)(pCurPos - pBuffer->data); /* Known to be less than a UInt32 */
		pBuffer->data = pCurPos;
	}

	return rsslReserveBufferMemory(pBuffer, elementCount, sizeOfElement);
}

RSSL_API void * rsslReserveBufferMemory(RsslBuffer *pBuffer, RsslUInt32 elementCount, RsslUInt32 sizeofElement)
{
	RsslUInt64 totalBytes = elementCount * sizeofElement;

	if (totalBytes < pBuffer->length)
	{
		char *pCurPos = pBuffer->data;
		pBuffer->length -= (RsslUInt32)totalBytes; /* Known to be less than a UInt32 */
		pBuffer->data += totalBytes;

		return pCurPos;
	}
	else 
		return NULL;
}

