/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef RSSL_HEAP_BUFFER_H
#define RSSL_HEAP_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include <malloc.h>

/* RsslBuffer
 * An RsslBuffer whose data is heap-allocated and growable. */

/* Reset the buffer of an RsslBuffer to its original state. */
RTR_C_INLINE void rsslHeapBufferReset(RsslBuffer *pBuffer, RsslBuffer *pMemBuffer)
{ 
	*pBuffer = *pMemBuffer;
}

/* Initialize an RsslBuffer. */
RTR_C_INLINE RsslRet rsslHeapBufferInit(RsslBuffer *pBuffer, RsslUInt32 length)
{
	if (length)
	{
		pBuffer->data = (char*)malloc(length);

		if (!pBuffer->data)
			return RSSL_RET_FAILURE;

		pBuffer->length = length;
	}
	else
	{
		pBuffer->data = NULL;
		pBuffer->length = 0;
	}

	return RSSL_RET_SUCCESS;
}

/* Cleanup an RsslBuffer. */
RTR_C_INLINE void rsslHeapBufferCleanup(RsslBuffer *pBuffer)
{
	if (pBuffer->data)
		free(pBuffer->data);
}


/* Change the size of an RsslBuffer and reset it. */
RTR_C_INLINE RsslRet rsslHeapBufferResize(RsslBuffer *pBuffer, RsslUInt32 length, RsslBool keepData)
{
	if (pBuffer->length < length)
	{
		if (keepData)
			pBuffer->data = (char*)realloc((void*)pBuffer->data, length);
		else
		{
			free(pBuffer->data);
			pBuffer->data = (char*)malloc(length);
		}

		if (!pBuffer->data)
			return RSSL_RET_FAILURE;
		pBuffer->length = length;
	}

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet rsslHeapBufferCreateCopy(RsslBuffer *pOutBuffer, RsslBuffer *pInBuffer)
{
	RsslRet ret;

	if ((ret = rsslHeapBufferInit(pOutBuffer, pInBuffer->length)) != RSSL_RET_SUCCESS)
		return ret;

	memcpy(pOutBuffer->data, pInBuffer->data, pInBuffer->length);
	return RSSL_RET_SUCCESS;
}

/* Copy an RsslBuffer to the RsslBuffer. */
RTR_C_INLINE RsslRet rsslHeapBufferCopy(RsslBuffer *pOutBuffer, RsslBuffer *pInBuffer, 
		RsslBuffer *pMemBuffer)
{

	if (pMemBuffer->length < pInBuffer->length)
	{
		RsslRet ret;

		if (pMemBuffer->data)
			free(pMemBuffer->data);
		if ((ret = rsslHeapBufferInit(pMemBuffer, pInBuffer->length)) != RSSL_RET_SUCCESS)
			return ret;
	}

	rsslHeapBufferReset(pOutBuffer, pMemBuffer);


	memcpy(pOutBuffer->data, pInBuffer->data, pInBuffer->length);
	pOutBuffer->length = pInBuffer->length;
	return RSSL_RET_SUCCESS;
}

#ifdef __cplusplus
};
#endif

#endif
