/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|             Copyright (C) 2020 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslDataPackage.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rwfConvert.h"
#include "rtr/encoderTools.h"

#include <stdlib.h>
#include <string.h>


#include <math.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>

/* Suppress warning C4756: overflow in constant arithmetic that occurs only on VS2013 */
#if defined(WIN32) &&  _MSC_VER == 1800
#pragma warning( disable : 4056 4756)
#endif

#ifdef WIN32
	#ifndef INFINITY
		#define INFINITY HUGE_VAL
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -HUGE_VAL
		#endif
	#else
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -INFINITY		
		#endif
	#endif
	#ifndef NAN
        static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
        #define NAN (*(const double *) __nan)
    #endif
#else
	#define NEG_INFINITY -INFINITY
#endif


static RsslDouble powHintsEx[] = { 1e14, 1e13, 1e12, 1e11, 1e10, 1e9, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1.0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1., 2., 4., 8., 16., 32., 64., 128., 256. };

static RsslDouble powHints[] = { 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1., 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1., 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125, 0.00390625 };


RSSL_API RsslRet rsslDoubleToReal(RsslReal * oReal, RsslDouble * iValue, RsslUInt8 iHint)
{
	double res;

	RSSL_ASSERT(oReal, Invalid parameters or parameters passed in as NULL);

	if (iHint > RSSL_RH_NOT_A_NUMBER || iHint == 31)
		return RSSL_RET_FAILURE;

	if (!iValue)
	{
		/* blank value */
		oReal->isBlank = RSSL_TRUE;
		oReal->value = 0;
		return RSSL_RET_SUCCESS;
	}

	if (*iValue == INFINITY)
	{
		oReal->hint = RSSL_RH_INFINITY;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else if (*iValue == NEG_INFINITY)
	{
		oReal->hint = RSSL_RH_NEG_INFINITY;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else if (*iValue == NAN)
	{
		oReal->hint = RSSL_RH_NOT_A_NUMBER;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else
	{
		oReal->hint = iHint;

		res = floor((*iValue) * powHintsEx[iHint] + 0.5);

		if (res < LLONG_MIN || LLONG_MAX < res) {
			return RSSL_RET_FAILURE;
		}

		// Checks the corner cases.
		// Uses direct assignment value LLONG_MAX (64 bits) and prohibits the conversation from double value (53 bits).
		if ((double)LLONG_MAX == res)
			oReal->value = LLONG_MAX;
		else if ((double)LLONG_MIN == res)
			oReal->value = LLONG_MIN;
		else
			oReal->value = (RsslInt)(res);

		oReal->isBlank = RSSL_FALSE;
	}
	return RSSL_RET_SUCCESS;
}
	
RSSL_API RsslRet rsslFloatToReal(RsslReal * oReal, RsslFloat * iValue, RsslUInt8 iHint)
{
	float res;

	RSSL_ASSERT(oReal, Invalid parameters or parameters passed in as NULL);
	
	if (iHint > RSSL_RH_NOT_A_NUMBER || iHint == 31)
		return RSSL_RET_FAILURE;

	if (!iValue)
	{
		/* blank value */
		oReal->isBlank = RSSL_TRUE;
		oReal->value = 0;
		return RSSL_RET_SUCCESS;
	}

	if ((double)(*iValue) == INFINITY)
	{
		oReal->hint = RSSL_RH_INFINITY;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else if ((double)(*iValue) == NEG_INFINITY)
	{
		oReal->hint = RSSL_RH_NEG_INFINITY;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else if ((double)(*iValue) == NAN)
	{
		oReal->hint = RSSL_RH_NOT_A_NUMBER;
		oReal->isBlank = RSSL_FALSE;
		oReal->value = 0;		
	}
	else
	{
		oReal->hint = iHint;

		res = floorf((*iValue) * (float)powHintsEx[iHint] + 0.5f);

		if (res < LLONG_MIN || LLONG_MAX < res) {
			return RSSL_RET_FAILURE;
		}

		// Checks the corner cases.
		// Uses direct assignment value LLONG_MAX (64 bits) and prohibits the conversation from float value (24 bits).
		if ((float)LLONG_MAX == res)
			oReal->value = LLONG_MAX;
		else if ((float)LLONG_MIN == res)
			oReal->value = LLONG_MIN;
		else
			oReal->value = (RsslInt)(res);

		oReal->isBlank = RSSL_FALSE;
	}
	return RSSL_RET_SUCCESS;
}


RSSL_API RsslRet rsslRealToString(RsslBuffer * buffer, RsslReal * iReal)
{
	char tbuf[128];
	char *ret;
	RsslUInt32 length;

	RSSL_ASSERT(buffer, Invalid parameters or parameters passed in as NULL);
	if (!iReal)
	{
		buffer->length = 0;
		if (buffer->data)
			buffer->data[0] = '\0';
	}
		
	ret = rwfReal64tosOpts( tbuf, 64, iReal, 0 );
	if (ret == 0)
		return RSSL_RET_INVALID_DATA;

	length = (RsslUInt32)strlen(ret);
	if (length < buffer->length)
	{
		memcpy(buffer->data, ret, length);
		buffer->data[length] = '\0';
		buffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	return RSSL_RET_FAILURE;
}

RSSL_API const char* rsslRealHintToOmmString(RsslUInt8 hint)
{
	switch(hint)
	{
		case RSSL_RH_EXPONENT_14: return RSSL_OMMSTR_RH_EXPONENT_14.data;
		case RSSL_RH_EXPONENT_13: return RSSL_OMMSTR_RH_EXPONENT_13.data;
		case RSSL_RH_EXPONENT_12: return RSSL_OMMSTR_RH_EXPONENT_12.data;
		case RSSL_RH_EXPONENT_11: return RSSL_OMMSTR_RH_EXPONENT_11.data;
		case RSSL_RH_EXPONENT_10: return RSSL_OMMSTR_RH_EXPONENT_10.data;
		case RSSL_RH_EXPONENT_9: return RSSL_OMMSTR_RH_EXPONENT_9.data;
		case RSSL_RH_EXPONENT_8: return RSSL_OMMSTR_RH_EXPONENT_8.data;
		case RSSL_RH_EXPONENT_7: return RSSL_OMMSTR_RH_EXPONENT_7.data;
		case RSSL_RH_EXPONENT_6: return RSSL_OMMSTR_RH_EXPONENT_6.data;
		case RSSL_RH_EXPONENT_5: return RSSL_OMMSTR_RH_EXPONENT_5.data;
		case RSSL_RH_EXPONENT_4: return RSSL_OMMSTR_RH_EXPONENT_4.data;
		case RSSL_RH_EXPONENT_3: return RSSL_OMMSTR_RH_EXPONENT_3.data;
		case RSSL_RH_EXPONENT_2: return RSSL_OMMSTR_RH_EXPONENT_2.data;
		case RSSL_RH_EXPONENT_1: return RSSL_OMMSTR_RH_EXPONENT_1.data;
		case RSSL_RH_EXPONENT0: return RSSL_OMMSTR_RH_EXPONENT0.data;
		case RSSL_RH_EXPONENT1: return RSSL_OMMSTR_RH_EXPONENT1.data;
		case RSSL_RH_EXPONENT2: return RSSL_OMMSTR_RH_EXPONENT2.data;
		case RSSL_RH_EXPONENT3: return RSSL_OMMSTR_RH_EXPONENT3.data;
		case RSSL_RH_EXPONENT4: return RSSL_OMMSTR_RH_EXPONENT4.data;
		case RSSL_RH_EXPONENT5: return RSSL_OMMSTR_RH_EXPONENT5.data;
		case RSSL_RH_EXPONENT6: return RSSL_OMMSTR_RH_EXPONENT6.data;
		case RSSL_RH_EXPONENT7: return RSSL_OMMSTR_RH_EXPONENT7.data;
		case RSSL_RH_FRACTION_1: return RSSL_OMMSTR_RH_FRACTION_1.data;
		case RSSL_RH_FRACTION_2: return RSSL_OMMSTR_RH_FRACTION_2.data;
		case RSSL_RH_FRACTION_4: return RSSL_OMMSTR_RH_FRACTION_4.data;
		case RSSL_RH_FRACTION_8: return RSSL_OMMSTR_RH_FRACTION_8.data;
		case RSSL_RH_FRACTION_16: return RSSL_OMMSTR_RH_FRACTION_16.data;
		case RSSL_RH_FRACTION_32: return RSSL_OMMSTR_RH_FRACTION_32.data;
		case RSSL_RH_FRACTION_64: return RSSL_OMMSTR_RH_FRACTION_64.data;
		case RSSL_RH_FRACTION_128: return RSSL_OMMSTR_RH_FRACTION_128.data;
		case RSSL_RH_FRACTION_256: return RSSL_OMMSTR_RH_FRACTION_256.data;
		case RSSL_RH_INFINITY: return RSSL_OMMSTR_RH_INFINITY.data;
		case RSSL_RH_NEG_INFINITY: return RSSL_OMMSTR_RH_NEG_INFINITY.data;
		case RSSL_RH_NOT_A_NUMBER: return RSSL_OMMSTR_RH_NOT_A_NUMBER.data;
		default: return NULL;
	}
}



RSSL_API RsslRet rsslRealToDouble(RsslDouble * oValue, RsslReal * iReal)
{
	RSSL_ASSERT(iReal && oValue, Invalid parameters or parameters passed in as NULL);

	if (iReal->isBlank == RSSL_TRUE)
		return RSSL_RET_FAILURE;
	
	/* TODO: Better checking here if we change hints */
	switch(iReal->hint)
	{
		case RSSL_RH_NOT_A_NUMBER:
			*oValue = NAN;
			return RSSL_RET_SUCCESS;
		break;
		case RSSL_RH_INFINITY:
			*oValue = (RsslDouble)INFINITY;
			return RSSL_RET_SUCCESS;
		break;
		case RSSL_RH_NEG_INFINITY:
			*oValue = (RsslDouble)NEG_INFINITY;
			return RSSL_RET_SUCCESS;
		break;
	}

	if (iReal->hint > RSSL_RH_MAX_DIVISOR)
		return RSSL_RET_FAILURE;

 	*oValue = (iReal->value*powHints[iReal->hint]);
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslNumericStringToDouble(RsslDouble * oValue, RsslBuffer * iNumericString)
{
	RsslRet		ret;
	RsslReal	real64;

	RSSL_ASSERT(iNumericString && oValue, Invalid parameters or parameters passed in as NULL);

	if (!iNumericString || !iNumericString->data)
	{
		*oValue = NAN;
		return RSSL_RET_BLANK_DATA;
	}

	ret = rwf_storeal64_size( 
						&real64,
						iNumericString->data,
						iNumericString->data + iNumericString->length - 1/*Point to the last character*/);

	if (ret == RSSL_RET_BLANK_DATA)
	{
		*oValue = NAN;
		return (ret);
	}
	else if (ret != RSSL_RET_SUCCESS)
		return (ret);

	switch(real64.hint)
	{
		case RSSL_RH_NOT_A_NUMBER:
			*oValue = NAN;
			return RSSL_RET_SUCCESS;
		break;
		case RSSL_RH_INFINITY:
			*oValue = (RsslDouble)INFINITY;
			return RSSL_RET_SUCCESS;
		break;
		case RSSL_RH_NEG_INFINITY:
			*oValue = (RsslDouble)NEG_INFINITY;
			return RSSL_RET_SUCCESS;
		break;
	}

	*oValue = (real64.value*powHints[real64.hint]);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslNumericStringToReal(RsslReal * oReal, RsslBuffer * iNumericString)
{
	RSSL_ASSERT(oReal, Invalid parameters or parameters passed in as NULL);

	if (!iNumericString || !iNumericString->data)
	{
		oReal->value = 0;
		oReal->hint = 0;
		oReal->isBlank = RSSL_TRUE;

		return RSSL_RET_BLANK_DATA;
	}

	return rwf_storeal64_size(
						oReal,
						iNumericString->data,
						iNumericString->data + iNumericString->length - 1/*Point to the last character*/);
}

