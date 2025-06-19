/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_DECODER_TOOLS_H
#define __RSSL_DECODER_TOOLS_H

#include "rtr/rwfNetwork.h"
#include "rtr/intDataTypes.h"
#include "rtr/custmem.h"
#include "rtr/rsslDataUtils.h"


#ifdef __cplusplus
extern "C" {
#endif

	
/* This function does not perform any buffer size checking.
 * Can use the _rsslValidBufferPtr(), _rsslInvalidBufferPointer(),
 * _rsslValidBufferPointerEndPtr() or _rsslInvalidBufferPointerEndPtr()
 * after to check since no data copy is happening.
 */
RTR_C_ALWAYS_INLINE char * _rsslDecodeBuffer16(RsslBuffer * buffer, char * position)
{
	RsslUInt16 tlen;

	position += rwfGetOptByteU16(&tlen, position);
	buffer->length = tlen;
	buffer->data = (buffer->length > 0) ? (char *) position : 0;
	position += buffer->length;
	return position;
}

/* This function does not perform any buffer size checking.
 * Can use the _rsslValidBufferPtr(), _rsslInvalidBufferPointer(),
 * _rsslValidBufferPointerEndPtr() or _rsslInvalidBufferPointerEndPtr()
 * after to check since no data copy is happening.
 */
RTR_C_ALWAYS_INLINE char * _rsslDecodeBuffer15(RsslBuffer * buffer, char * position)
{
	RsslUInt16 tlen;

	position += rwfGetResBitU15(&tlen, position);
	buffer->length = tlen;
	buffer->data = (buffer->length > 0) ? (char *) position : 0;
	position += buffer->length;
	return position;
}

/* Writes a string representing flags into the RsslBuffer. If successful, adjusts buffer length to 
 * that of the written string.
 * Returns RSSL_RET_SUCCESS if successful, RSSL_RET_BUFFER_TOO_SMALL if out of space. */
RTR_C_ALWAYS_INLINE RsslRet _rsslFlagsToOmmString(RsslBuffer *oBuffer, const char **values, int valuesCount)
{
	const char *delimiter = "";
	int i;
	int length = 0;
	for (i = 0; i < valuesCount; ++i)
	{
		int ret = snprintf(oBuffer->data + length, oBuffer->length - length, "%s%s", delimiter, values[i]);
		if (ret < 0) return RSSL_RET_FAILURE;

		/* Stop if we're out of space. */
		length += ret;
		if ((RsslUInt32)length >= oBuffer->length) return RSSL_RET_BUFFER_TOO_SMALL;

		delimiter = "|"; /* Include delimiter for any subsequent flags. */
	}

	oBuffer->length = length; /* Set buffer length that of written string. */
	return RSSL_RET_SUCCESS;
}




#ifdef __cplusplus
}
#endif 

#endif
