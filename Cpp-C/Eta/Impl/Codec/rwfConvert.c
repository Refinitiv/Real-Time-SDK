/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2024 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#if defined (_WIN32) || defined(WIN32)
#include <string.h>
#define snprintf _snprintf
#define strnicmp _strnicmp
#else
#include <strings.h>
#define strnicmp strncasecmp
#endif

#include "rtr/rwfConvert.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rtrdiv10.h"
#include "rtr/retmacros.h"
#include "rtr/rwfNetwork.h"

static rwfTosOptions rwfDfltOpts = RWF_INIT_TOS_OPTS;

static const char* _rwf_denominator[] = {"1/", "2/", "4/", "8/", "61/", "23/", "46/", "821/", "652/"};
static const int _rwf_numeratorMask[] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

rtrInt8		MAX_INT8 = SCHAR_MAX;
rtrInt16	MAX_INT16 = SHRT_MAX;
rtrInt32	MAX_INT32 = INT_MAX;
rtrInt64	MAX_INT64 = RTR_LL(0x7FFFFFFFFFFFFFFF);
rtrInt64	MAX_UINT64 = RTR_ULL(0xFFFFFFFFFFFFFFFF);

rtrInt8		MAX_INT8DIV10 = SCHAR_MAX / 10;
rtrInt16	MAX_INT16DIV10 = SHRT_MAX / 10;
rtrInt32	MAX_INT32DIV10 = INT_MAX/10;
rtrInt64	MAX_INT64DIV10 = (RTR_LL(0x7FFFFFFFFFFFFFFF))/10;
rtrUInt64	MAX_UINT64DIV10 = (RTR_ULL(0xFFFFFFFFFFFFFFFF))/10;

#define powerof( value, x, y ) \
{\
	int i; \
	value = x; \
	for (i = 1; i < y; i++ ) \
		value *= x;\
}

#ifdef _WIN64

#define RTR_DO_LONG_TO_STRING(value,dptr,tmpchar) \
	if (value == 0) \
		*(--dptr) = '0'; \
	while (value != 0) \
	{ \
		/* Store value in temporary 'tmpchar'. \
		 * Helps performance on Win32 \
		 * significantly. Keep Mod (%) and div (/) \
		 * operations together. \
		 */ \
		tmpchar = (char) (value % 10 + '0'); \
		value /= 10; \
		*(--dptr) = tmpchar; \
	}


#define RTR_DO_LONG_TO_DECIMAL_STRING(value,dptr,tmpchar,exponent) \
	while (value != 0) \
	{ \
		/* Store value in temporary 'tmpchar'. \
		 * Helps performance on Win32 \
		 * significantly. Keep Mod (%) and div (/) \
		 * operations together. \
		 */ \
		tmpchar = (char) (value % 10 + '0'); \
		value /= 10; \
		switch( (exponent++) ) \
		{ \
		case 0 : *(--dptr) = '.'; \
		default : *(--dptr) = tmpchar; \
				 break; \
		} \
	} \
	while (exponent <= 0) \
	{ \
		while ((exponent++) < 0) \
			*(--dptr) = '0'; \
		*(--dptr) = '.'; \
		*(--dptr) = '0'; \
		(++exponent) ; \
	}

#else

#define RTR_DO_LONG_TO_STRING(value,dptr,tmpchar) \
{\
	unsigned long quo,rem; \
	quo=0; \
	rem=0; \
	if (value == 0) \
		*(--dptr) = '0'; \
	while (value != 0) \
	{ \
		__rtr_u32div10(value,quo,rem); \
		tmpchar = (char) (rem + '0'); \
		*(--dptr) = tmpchar; \
		value = quo; \
	} \
}

#define RTR_DO_LONG_TO_DECIMAL_STRING(value,dptr,tmpchar,exponent) \
	unsigned long quo,rem; \
	quo=0; \
	rem=0; \
	while (value != 0) \
	{ \
		__rtr_u32div10(value,quo,rem); \
		tmpchar = (char) (rem + '0'); \
		switch( (exponent++) ) \
		{ \
		case 0 : *(--dptr) = '.'; \
		default : *(--dptr) = tmpchar; \
				 break; \
		} \
		value = quo; \
	} \
	while (exponent <= 0) \
	{ \
		while ((exponent++) < 0) \
			*(--dptr) = '0'; \
		*(--dptr) = '.'; \
		*(--dptr) = '0'; \
		(++exponent) ; \
	}

#endif

#define RTR_DO_LONGLONG_TO_STRING(value,dptr,tmpchar) \
	if (value == 0) \
		*(--dptr) = '0'; \
	while (value != 0) \
	{ \
		/* Store value in temporary 'tmpchar'. \
		 * Helps performance on Win32 \
		 * significantly. Keep Mod (%) and div (/) \
		 * operations together. \
		 */ \
		tmpchar = (char) (value % 10 + '0'); \
		value /= 10; \
		*(--dptr) = tmpchar; \
	}


#define RTR_DO_LONGLONG_TO_DECIMAL_STRING(value,dptr,tmpchar,exponent) \
	while (value != 0) \
	{ \
		/* Store value in temporary 'tmpchar'. \
		 * Helps performance on Win32 \
		 * significantly. Keep Mod (%) and div (/) \
		 * operations together. \
		 */ \
		tmpchar = (char) (value % 10 + '0'); \
		value /= 10; \
		switch( (exponent++) ) \
		{ \
		case 0 : *(--dptr) = '.'; \
		default : *(--dptr) = tmpchar; \
				 break; \
		} \
	} \
	while (exponent <= 0) \
	{ \
		while ((exponent++) < 0) \
			*(--dptr) = '0'; \
		*(--dptr) = '.'; \
		*(--dptr) = '0'; \
		(++exponent) ; \
	}


char * rwfItos(char *str, RsslUInt32 strlen, RsslInt32 iVal)
{
	register char *psz = str+strlen;	/* Work backwards */
	register char digit;

	/* Work with unsigned values since it is faster for division/mod */
	register RsslUInt32 workVal = (iVal < 0) ? (RsslUInt32)(-iVal) : (RsslUInt32)iVal;

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (workVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

		/* Convert integer to string */
	RTR_DO_LONG_TO_STRING(workVal, psz, digit);

		/* If the number is negative then prepend the sign */
	if (iVal < 0)
		*(--psz) = '-';

	return(psz);
}

char * rwfItosOpts(char *str, RsslUInt32 strlen, RsslInt32 iVal, rwfTosOptions *popts)
{
	register char *psz = str+strlen;
	register char digit;

	/* Work with unsigned values since it is faster for calculations */
	register RsslUInt32 workVal = (iVal < 0) ? (RsslUInt32)(-iVal) : (RsslUInt32)iVal;
	rwfTosOptions *opts = (popts ? popts : &rwfDfltOpts);

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (workVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	/* Convert integer to string */
	RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	/* Prepend zeroes if necessary */
	if (opts->padZero)
	{
		/* Calculate beginning position based on pad size */
		register char *epsz = str+strlen-(opts->padZero)-1;
		if ((iVal < 0) || opts->printSign)
			epsz++;
		while (psz > epsz)
			*(--psz) = '0';
	}

	if (iVal < 0) // if the number is negative appen the sign irrespective.
		*(--psz) = '-';
	else if ( opts->printSign ) // we want to include the signs to the number.
		*(--psz) = '+';

	return(psz);
}

char * rwfUItos(char *str, RsslUInt32 strlen, RsslUInt32 iVal)
{
	register char *psz = str+strlen;
	register char digit;

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (iVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	/* Convert integer to string */
	RTR_DO_LONG_TO_STRING(iVal, psz, digit);

	return(psz);
}

char * rwfUItosOpts(char *str, RsslUInt32 strlen, RsslUInt32 iVal, rwfTosOptions *popts)
{
	register char *psz = str+strlen;
	register char digit;
	rwfTosOptions *opts = (popts ? popts : &rwfDfltOpts);

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (iVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	/* Convert integer to string */
	RTR_DO_LONG_TO_STRING(iVal, psz, digit);

	/* Prepend zeroes if necessary */
	if (opts->padZero)
	{
		/* Calculate beginning position based on pad size */
		register char *epsz = str+strlen-(opts->padZero)-1;
		if (opts->printSign)
			epsz++;
		while (psz > epsz)
			*(--psz) = '0';
	}

	if ( opts->printSign ) // we want to include the signs to the number.
		*(--psz) = '+';

	return(psz);
}

char * rwfDateTimetos(char *str, RsslUInt32 strlen, RsslDateTime *iDTime)
{
	register char *psz = str+strlen;	/* Work backwards */
	register char digit;

	/* Work with unsigned values since it is faster for division/mod */
	register RsslUInt32 workVal = 0;

	*(--psz) = 0;	/* Null terminate the end of the string */

	workVal = iDTime->time.millisecond;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	if (iDTime->time.millisecond < 10)
	{
		*(--psz) = '0';
		*(--psz) = '0';
	}
	else if (iDTime->time.millisecond < 100)
		*(--psz) = '0';

	*(--psz) = ':';

	workVal = iDTime->time.second;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	if (iDTime->time.second < 10)
		*(--psz) = '0';

	*(--psz) = ':';

	workVal = iDTime->time.minute;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	if (iDTime->time.minute < 10)
		*(--psz) = '0';

	*(--psz) = ':';

	workVal = iDTime->time.hour;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	if (iDTime->time.hour < 10)
		*(--psz) = '0';

	*(--psz) = ' ';

	workVal = iDTime->date.year;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	*(--psz) = '/';

	workVal = iDTime->date.day;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	*(--psz) = '/';

	workVal = iDTime->date.month;
	if (workVal == 0)
		*(--psz) = '0';
	else
		RTR_DO_LONG_TO_STRING(workVal, psz, digit);

	return(psz);
}


/***********************************************************************************/
/* 64 bit																			*/
/***********************************************************************************/

char * rwfI64tos(char *str, RsslUInt32 strlen, RsslInt64 iVal)
{
	register char *psz = str+strlen;	/* Work backwards */
	register char digit;

	/* Work with unsigned values since it is faster for division/mod */
	register RsslUInt64 workVal = (iVal < 0) ? (RsslUInt64)(-iVal) : (RsslUInt64)iVal;

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (workVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	if (workVal <= 0xFFFFFFFF)
	{
		register RsslUInt32 tval=(RsslUInt32)workVal;
		/* Use 32 bit value for convertion since compiler
		 * can turn 32 bit divides into 64 bit multiply.
		 */
		RTR_DO_LONG_TO_STRING(tval, psz, digit);
	}
	else
	{
		/* Convert integer to string */
		RTR_DO_LONGLONG_TO_STRING(workVal, psz, digit);
	}

	/* If the number is negative then prepend the sign */
	if (iVal < 0)
		*(--psz) = '-';

	return(psz);
}

char * rwfI64tosOpts(char *str, RsslUInt32 strlen, RsslInt64 iVal, rwfTosOptions *popts)
{
	register char *psz = str+strlen;
	register char digit;

	/* Work with unsigned values since it is faster for division/mod */
	register RsslUInt64 workVal = (iVal < 0) ? (RsslUInt64)(-iVal) : (RsslUInt64)iVal;
	rwfTosOptions *opts = (popts ? popts : &rwfDfltOpts);

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (workVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	if (workVal <= 0xFFFFFFFF)
	{
		register RsslUInt32 tval=(RsslUInt32)workVal;
		/* Use 32 bit value for convertion since compiler
		 * can turn 32 bit divides into 64 bit multiply.
		 */
		RTR_DO_LONG_TO_STRING(tval, psz, digit);
	}
	else
	{
		/* Convert integer to string */
		RTR_DO_LONGLONG_TO_STRING(workVal, psz, digit);
	}

	/* Prepend zeroes if necessary */
	if (opts->padZero)
	{
		/* Calculate beginning position based on pad size */
		register char *epsz = str+strlen-(opts->padZero)-1;
		if ((iVal < 0) || opts->printSign)
			epsz++;
		while (psz > epsz)
			*(--psz) = '0';
	}

	if (iVal < 0) // if the number is negative appen the sign irrespective.
		*(--psz) = '-';
	else if ( opts->printSign ) // we want to include the signs to the number.
		*(--psz) = '+';

	return(psz);
}

char * rwfUI64tos(char *str, RsslUInt32 strlen, RsslUInt64 iVal)
{
	register char *psz = str+strlen;
	register char digit;

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (iVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	if (iVal <= 0xFFFFFFFF)
	{
		register RsslUInt32 tval=(RsslUInt32)iVal;
		/* Use 32 bit value for convertion since compiler
		 * can turn 32 bit divides into 64 bit multiply.
		 */
		RTR_DO_LONG_TO_STRING(tval, psz, digit);
	}
	else
	{
		/* Convert integer to string */
		RTR_DO_LONGLONG_TO_STRING(iVal, psz, digit);
	}

	return(psz);
}

char * rwfUI64tosOpts(char *str, RsslUInt32 strlen, RsslUInt64 iVal, rwfTosOptions *popts)
{
	register char *psz = str+strlen;
	register char digit;
	rwfTosOptions *opts = (popts ? popts : &rwfDfltOpts);

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (iVal == 0)
	{
		*(--psz) = '0';
		return(psz);
	}

	if (iVal <= 0xFFFFFFFF)
	{
		register RsslUInt32 tval=(RsslUInt32)iVal;
		/* Use 32 bit value for convertion since compiler
		 * can turn 32 bit divides into 64 bit multiply.
		 */
		RTR_DO_LONG_TO_STRING(tval, psz, digit);
	}
	else
	{
		/* Convert integer to string */
		RTR_DO_LONGLONG_TO_STRING(iVal, psz, digit);
	}

	/* Prepend zeroes if necessary */
	if (opts->padZero)
	{
		/* Calculate beginning position based on pad size */
		register char *epsz = str+strlen-(opts->padZero)-1;
		if (opts->printSign)
			epsz++;
		while (psz > epsz)
			*(--psz) = '0';
	}

	if ( opts->printSign ) // we want to include the signs to the number.
		*(--psz) = '+';

	return(psz);
}


/***********************************************************************************/
/*  Real																	*/
/***********************************************************************************/

/* careful while using this function as it assumes that the character
	string is pointing to the last character position.
	We are filling the string backwards.
*/
int rwfUI32tofractions(char *psz, RsslUInt8 frachint, RsslUInt32 value)
{
	register char* tstr= psz;
	register char digit = 0;
	register RsslUInt32 wholeNumber = 0;
	register RsslUInt32 remainder = 0;
	const char* p = 0;

	/* fractional processing
	   read frachint... this indicates the fractional hint. 2^n ( n=0 indicates no fraction)*/
	if ( frachint )
	{
		wholeNumber = (value >> frachint);
		remainder = (value & _rwf_numeratorMask[frachint]);
		if (remainder > 0)
		{ /* "wholeNumber" + " " + "remainder" + "/" + "frachint" */
			p = _rwf_denominator[frachint];
			while ( *p != 0)
				*(--tstr) = *p++;
			RTR_DO_LONG_TO_STRING(remainder, tstr, digit);
			if (wholeNumber > 0)
			{
				*(--tstr) = ' ';
				RTR_DO_LONG_TO_STRING(wholeNumber, tstr, digit);
			}
		}
		else /* "wholeNumber" */
		{	
			RTR_DO_LONG_TO_STRING(wholeNumber, tstr, digit);
		}
	}
	else /* "wholeNumber" */
	{
		RTR_DO_LONG_TO_STRING(value, tstr, digit);
	}
	return (int)(psz - tstr);
}

/* careful while using this function as it assumes that the character
	string is pointing to the last character position.
	We are filling the string backwards.
*/
int UI64tos(RsslUInt64 value, char *psz, char digit)
{
	register char* tstr = psz;
	if (value <= 0xFFFFFFFF)
	{
		register RsslUInt32 tval=(RsslUInt32)value;
		/* Use 32 bit value for convertion since compiler
		 * can turn 32 bit divides into 64 bit multiply.
		 */
		RTR_DO_LONG_TO_STRING(tval, tstr, digit);
	}
	else
	{
		/* Convert integer to string */
		RTR_DO_LONGLONG_TO_STRING(value, tstr, digit);
	}
	return (int)(psz - tstr);
}

/* careful while using this function as it assumes that the character
	string is pointing to the last character position.
	We are filling the string backwards.
*/
int rwfUI64tofractions(char *psz, RsslUInt8 frachint, RsslUInt64 value)
{
	register char* tstr= psz;
	register char digit = 0;
	register RsslUInt64 wholeNumber = 0;
	register RsslUInt8 remainder = 0;
	const char* p = 0;

	/* fractional processing read frachint... this indicates the fractional hint. 2^n ( n=0 indicates no fraction)*/
	if ( frachint )
	{
		wholeNumber = (RsslUInt64) (value >> frachint);
		remainder = (RsslUInt8) (value & _rwf_numeratorMask[frachint]);
		if (remainder > 0)
		{ /* "wholeNumber" + " " + "remainder" + "/" + "frachint" */
			register RsslUInt32 tval=(RsslUInt32)remainder;

			p = _rwf_denominator[frachint];
			while ( *p != 0)
				*(--tstr) = *p++;

			/* Use 32 bit value for convertion since compiler
			 * can turn 32 bit divides into 64 bit multiply.
			 */
			RTR_DO_LONG_TO_STRING(tval, tstr, digit);

			if (wholeNumber > 0)
			{
				*(--tstr) = ' ';
				if (wholeNumber <= 0xFFFFFFFF)
				{
					register RsslUInt32 tval=(RsslUInt32)wholeNumber;
					/* Use 32 bit value for convertion since compiler
					 * can turn 32 bit divides into 64 bit multiply.
					 */
					RTR_DO_LONG_TO_STRING(tval, tstr, digit);
				}
				else
				{
					RTR_DO_LONGLONG_TO_STRING(wholeNumber, tstr, digit);
				}
			}
		}
		else /* "wholeNumber" */
		{
			if (wholeNumber <= 0xFFFFFFFF)
			{
				register RsslUInt32 tval=(RsslUInt32)wholeNumber;
				/* Use 32 bit value for convertion since compiler
				 * can turn 32 bit divides into 64 bit multiply.
				 */
				RTR_DO_LONG_TO_STRING(tval, tstr, digit);
			}
			else
			{
				RTR_DO_LONGLONG_TO_STRING(wholeNumber, tstr, digit);
			}
		}
	}
	else /* "wholeNumber" */
	{
		if (value <= 0xFFFFFFFF)
		{
			register RsslUInt32 tval=(RsslUInt32)value;
			/* Use 32 bit value for convertion since compiler
			 * can turn 32 bit divides into 64 bit multiply.
			 */
			RTR_DO_LONG_TO_STRING(tval, tstr, digit);
		}
		else
		{
			RTR_DO_LONGLONG_TO_STRING(value, tstr, digit);
		}
	}

	return (int)(psz - tstr);
}

char * rwfReal64tosOpts(char *str, RsslUInt32 strlen, RsslReal *iVal, rwfTosOptions *popts)
{
	register char 		*psz = str+strlen;
	register char 		digit = 0;
	register RsslUInt64	value = iVal->value;
	register int		isNegative;
	rwfTosOptions *opts = (popts ? popts : &rwfDfltOpts);

	if (iVal->value < 0)
	{
		value = -iVal->value;
		isNegative = 1;
	}
	else
	{
		value = iVal->value;
		isNegative = 0;
	}

	*(--psz) = 0;	/* Null terminate the end of the string */

	if (iVal->isBlank)
		return(psz);

	switch (iVal->hint)
	{
		/* Do noting the value is fine */
		case RSSL_RH_MIN_EXP: break; /* and RSSL_RH_EXPONENT_14 */
		case RSSL_RH_EXPONENT_13: break;
		case RSSL_RH_EXPONENT_12: break;
		case RSSL_RH_EXPONENT_11: break;
		case RSSL_RH_EXPONENT_10: break;
		case RSSL_RH_EXPONENT_9: break;
		case RSSL_RH_EXPONENT_8: break;
		case RSSL_RH_EXPONENT_7: break;
		case RSSL_RH_EXPONENT_6: break;
		case RSSL_RH_EXPONENT_5: break;
		case RSSL_RH_EXPONENT_4: break;
		case RSSL_RH_EXPONENT_3: break;
		case RSSL_RH_EXPONENT_2: break;
		case RSSL_RH_EXPONENT_1: break;
		case RSSL_RH_EXPONENT0: break;
		case RSSL_RH_EXPONENT1: break;
		case RSSL_RH_EXPONENT2: break;
		case RSSL_RH_EXPONENT3: break;
		case RSSL_RH_EXPONENT4: break;
		case RSSL_RH_EXPONENT5: break;
		case RSSL_RH_EXPONENT6: break;
		case RSSL_RH_EXPONENT7: break; /* and RSSL_RH_MAX_EXP */
		case RSSL_RH_FRACTION_1: break; /* and RSSL_RH_MIN_DIVISOR*/
		case RSSL_RH_FRACTION_2: break;
		case RSSL_RH_FRACTION_4: break;
		case RSSL_RH_FRACTION_8: break;
		case RSSL_RH_FRACTION_16: break;
		case RSSL_RH_FRACTION_32: break;
		case RSSL_RH_FRACTION_64: break;
		case RSSL_RH_FRACTION_128: break;
		case RSSL_RH_FRACTION_256: break; /* and RSSL_RH_MAX_DIVISOR */
		case RSSL_RH_INFINITY:
			psz = "Inf";
			return(psz);
		case RSSL_RH_NEG_INFINITY:
			psz = "-Inf";
			return(psz);
		case RSSL_RH_NOT_A_NUMBER:
			psz = "NaN";
			return(psz);
		default:
			//Invalid data. Error case here.
			return NULL;
	}

	/* check if the number is decimal / fraction */
	if (iVal->hint >= RSSL_RH_FRACTION_1)
	{
		psz -= rwfUI64tofractions(psz, iVal->hint - RSSL_RH_FRACTION_1, value );
	}
	else
	{
		register int exponent = iVal->hint - RSSL_RH_EXPONENT0;
		if (exponent < 0)
		{
			if (value <= 0xFFFFFFFF)
			{
				register RsslUInt32 tval=(RsslUInt32)value;
				/* Use 32 bit value for convertion since compiler
				 * can turn 32 bit divides into 64 bit multiply.
				 */
				RTR_DO_LONG_TO_DECIMAL_STRING(tval, psz, digit, exponent);
			}
			else
			{
				/* Convert integer to string */
				RTR_DO_LONGLONG_TO_DECIMAL_STRING(value, psz, digit, exponent);
			}
		}
		else
		{
			if (value == 0)
			{
				*(--psz) = '0';
			}
			else
			{
				while (exponent-- > 0)
					*(--psz) = '0'; // pad zeroes at the end.

				if (value <= 0xFFFFFFFF)
				{
					register RsslUInt32 tval=(RsslUInt32)value;
					/* Use 32 bit value for convertion since compiler
					 * can turn 32 bit divides into 64 bit multiply.
					 */
					RTR_DO_LONG_TO_STRING(tval, psz, digit);
				}
				else
				{
					/* Convert integer to string */
					RTR_DO_LONGLONG_TO_STRING(value, psz, digit);
				}
			}
		}
	}

	if (isNegative)
		*(--psz) = '-';
	else if ( opts->printSign )
		*(--psz) = '+';

	return(psz);
}

#define __rtr_removewhitespace_null(ptr) \
	while ((*ptr != '\0') && (*ptr == ' ')) \
	ptr++;

#define __rtr_removewhitespace_end(ptr,endstr) \
	while ((ptr <= endstr) && (*ptr == ' ')) \
	ptr++;


#define __rwf_atonumber_null(ptr,result,foundDigit,maxVal,nextDigit,tempValue,isNeg) \
	while ((*ptr != '\0') && _rtr_acisdigit(*ptr)) \
	{ \
		if ( result > maxVal/10 ) \
			break; \
		foundDigit = 1; \
		tempValue = result * 10; \
		nextDigit = _rtr_actoint(*ptr); \
		if (isNeg && (tempValue + nextDigit == -maxVal - 1)) \
		{ \
			result = tempValue + nextDigit; \
			ptr++; \
			break; \
		} \
		if ( tempValue + nextDigit < tempValue) \
			break; \
		result = tempValue + nextDigit; \
		ptr++; \
	}

#define __rwf_atonumber_null_trailzero(ptr,result,foundDigit,trailzerovalue,trailzerocount,maxVal,nextDigit,tempValue,isNeg) \
	while ((*ptr != '\0') && _rtr_acisdigit(*ptr)) \
	{ \
		if ( result > maxVal/10 ) \
			break; \
		tempValue = result * 10; \
		nextDigit = _rtr_actoint(*ptr); \
		if (isNeg && (tempValue + nextDigit == -maxVal - 1)) \
		{ \
			result = tempValue + nextDigit; \
			trailzerocount = 0; \
			ptr++; \
			break; \
		} \
		if ( tempValue + nextDigit < tempValue) \
			break; \
		if (*ptr == '0') \
		{ \
			if (trailzerocount == 0) \
				trailzerovalue = result; \
			trailzerocount++; \
		} \
		else \
			trailzerocount = 0; \
		foundDigit = 1; \
		result = tempValue + nextDigit; \
		ptr++; \
	}

#define __rwf_atonumber_end(ptr,endstr,result,foundDigit,maxVal,nextDigit,tempValue,isNeg) \
	while ((ptr <= endstr) && _rtr_acisdigit(*ptr)) \
	{ \
		if ( result > maxVal/10 ) \
			break; \
		foundDigit = 1; \
		tempValue = result * 10; \
		nextDigit = _rtr_actoint(*ptr); \
		if (isNeg && (tempValue + nextDigit == -maxVal - 1)) \
		{ \
			result = tempValue + nextDigit; \
			ptr++; \
			break; \
		} \
		if ( tempValue + nextDigit < tempValue ) \
			break; \
		result = tempValue + nextDigit; \
		ptr++; \
	}

#define __rwf_atonumber_end_trailzero(ptr,endstr,result,foundDigit,trailzerovalue,trailzerocount,maxVal,nextDigit,tempValue,isNeg) \
	while ((ptr <= endstr) && _rtr_acisdigit(*ptr)) \
	{ \
		if ( result > maxVal/10 ) \
			break; \
		tempValue = result * 10; \
		nextDigit = _rtr_actoint(*ptr); \
		if (isNeg && (tempValue + nextDigit == -maxVal - 1))\
		{ \
			result = tempValue + nextDigit; \
			trailzerocount = 0; \
			ptr++; \
			break;\
		}\
		if ( tempValue + nextDigit < tempValue) \
			break; \
		if (*ptr == '0') \
		{ \
			if (trailzerocount == 0) \
				trailzerovalue = result; \
			if (foundDigit == 2) \
				trailzerocount++; \
		} \
		else \
		{ \
			trailzerocount = 0; \
			foundDigit = 2; \
		} \
		if (foundDigit != 2) \
			foundDigit = 1; \
		result = tempValue + nextDigit; \
		ptr++; \
	}

#define __rtr_checknegative_skipsign(ptr) \
	(*ptr == '-' ? ptr++, 1 : (*ptr == '+' ? ptr++, 0 : 0 ) )


RsslUInt8 _rwf_SetFractionHint( RsslInt64 denom )
{
	RsslUInt8 retval = 0;
	switch (denom)
	{
		case 1:
			retval = RSSL_RH_FRACTION_1;
			break;
		case 2:
			retval = RSSL_RH_FRACTION_2;
			break;
		case 4:
			retval = RSSL_RH_FRACTION_4;
			break;
		case 8:
			retval = RSSL_RH_FRACTION_8;
			break;
		case 16:
			retval = RSSL_RH_FRACTION_16;
			break;
		case 32:
			retval = RSSL_RH_FRACTION_32;
			break;
		case 64:
			retval = RSSL_RH_FRACTION_64;
			break;
		case 128:
			retval = RSSL_RH_FRACTION_128;
			break;
		case 256:
			retval = RSSL_RH_FRACTION_256;
			break;
	}
	return retval;
}

RsslRet rwf_storeal64( RsslReal *oReal64, const char *strptr )
{
	RsslInt64	value = 0,tempValue=0;
	RsslInt64	trailzerovalue = 0;
	RsslInt32	trailzerocount = 0;
	int			isNeg;
	int			foundDigit = 0;
	int			nextDigit;

	if (!strptr)
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_BLANK_DATA;
	}

	__rtr_removewhitespace_null(strptr);

	if (*strptr == '\0')
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_BLANK_DATA;
	}

	if ((*strptr == '+') && (*(strptr+1) == '0') && (*(strptr+2) == '\0'))
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_SUCCESS;
	}

	if (strncmp(strptr, "Inf", 3 ) == 0)
	{
		strptr += 3;
		if (*strptr == '\0')
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_INFINITY;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (strncmp(strptr, "-Inf", 4) == 0)
	{
		strptr += 4;
		if (*strptr == '\0')
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_NEG_INFINITY;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (strncmp(strptr, "NaN", 3) == 0)
	{
		strptr += 3;
		if (*strptr == '\0')
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_NOT_A_NUMBER;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}

	isNeg = __rtr_checknegative_skipsign(strptr);

	__rwf_atonumber_null_trailzero(strptr,value,foundDigit,trailzerovalue,trailzerocount,MAX_INT64,nextDigit,tempValue,isNeg);

	/* Check for decimal value */
	if (*strptr == '.')
	{
		/* It is a decimal value */
		const char *startdec = ++strptr;
		RsslUInt8 exponent;
		const char* test = NULL;

		__rwf_atonumber_null(strptr,value,foundDigit,MAX_INT64,nextDigit,tempValue,isNeg);

		if (foundDigit && strptr != '\0')
		{
			// We have more digits - if the rest are zeros, it might still fit
			test = strptr;
			while (*test == '0' && test != '\0')
				test++;
			if (test != '\0')
				return RSSL_RET_INVALID_DATA;
		}
		exponent = (RsslUInt8)(strptr - startdec);

		if (exponent > 14)
		{
			/* Returns invalid data as the number is larger than the maximum hint value. */
			return RSSL_RET_INVALID_DATA;
		}

		if (value == 0)
		{
			/* this was +0.00000..... so its blank */
			oReal64->isBlank = RSSL_TRUE;
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = 0;
		}
		else
		{
			/*Put a pointer on the last zero*/
			if (test)
				strptr = test;

			if (strptr == '\0')
			{
				oReal64->isBlank = RSSL_FALSE;
				oReal64->hint = RSSL_RH_EXPONENT0 - exponent;
				oReal64->value = (isNeg ? -1 * value : value);
			}
			else
				return RSSL_RET_INVALID_DATA;
		}
	}
	else if (*strptr == ' ')
	{
		strptr++;
		__rtr_removewhitespace_null(strptr);

		/* Check for another digit. Then might be a fraction. */
		if (_rtr_acisdigit(*strptr))
		{
			RsslInt64	numerator=0,tempValue=0;
			RsslInt64	denominator=0;

			__rwf_atonumber_null(strptr,numerator,foundDigit,MAX_INT64,nextDigit,tempValue,isNeg);

			/* Verify fraction */
			if (*strptr != '/')
				return RSSL_RET_INVALID_DATA;

			strptr++;
			__rwf_atonumber_null(strptr,denominator,foundDigit,MAX_INT32,nextDigit,tempValue,isNeg);

			if ((oReal64->hint = _rwf_SetFractionHint(denominator)) == 0)
				return RSSL_RET_INVALID_DATA;

			if (*strptr == '\0')
			{
				RsslInt64	multiplyValue;

				multiplyValue = value * denominator;
				/*Check overflow by multiplying*/
				if (value != 0 && multiplyValue / value != denominator)
					return RSSL_RET_INVALID_DATA;

				/*Check overflow by sum*/
				if (numerator > 0 && multiplyValue > MAX_INT64 - numerator)
					return RSSL_RET_INVALID_DATA;

				value = multiplyValue + numerator;
				oReal64->isBlank = RSSL_FALSE;
				oReal64->value = (isNeg ? -1 * value : value);
			}
			else
				return RSSL_RET_INVALID_DATA;
		}
		else if (*strptr == '\0')
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = (isNeg ? -1 * value : value);
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (*strptr == '/')
	{
		RsslInt32	denominator=0,tempValue=0;

		strptr++;
		__rwf_atonumber_null(strptr,denominator,foundDigit,MAX_INT32,nextDigit,tempValue,isNeg);

		if ((oReal64->hint = _rwf_SetFractionHint(denominator)) == 0)
			return RSSL_RET_INVALID_DATA;

		if (strptr == '\0')
		{
			/* value stays as value */
			oReal64->isBlank = RSSL_FALSE;
			oReal64->value = (isNeg ? -1 * value : value);
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (foundDigit)
	{
		if ( *strptr != '\0' )
		{
			// We have more digits - if the rest are zeros, it might still fit
			if ( *strptr == '0' )
			{
				if ( trailzerocount == 0 )
					trailzerovalue = value;
				while ( *strptr == '0' )
				{
					trailzerocount++;
					strptr++;
				}
				if ( *strptr != '\0' )
					return RSSL_RET_INVALID_DATA;
			}
			else
				return RSSL_RET_INVALID_DATA;
		}

		/* Exponential */
		oReal64->isBlank = RSSL_FALSE;
		if (trailzerocount > 0)
		{
			if (trailzerocount > 7)
			{
				while ( trailzerocount > 7 )
				{
					if ( trailzerovalue > MAX_INT64DIV10 )
						break;
					trailzerovalue = trailzerovalue * 10;
					trailzerocount--;
				}
				if ( trailzerocount > 7 )
					return RSSL_RET_INVALID_DATA;
			}
			oReal64->hint = RSSL_RH_EXPONENT0 + (RsslUInt8)trailzerocount;
			oReal64->value = (isNeg ? -1 * value : value);
		}
		else
		{
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = (isNeg ? -1 * value : value);
		}
	}
	else if (*strptr == '\0')
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_BLANK_DATA;
	}	
	else
		return RSSL_RET_INVALID_DATA;

	return RSSL_RET_SUCCESS;
}

RsslRet rwf_storeal64_size( RsslReal *oReal64, const char *strptr, const char *endptr )
{
	RsslInt64	value = 0;
	RsslInt64	tempValue = 0;
	RsslInt64	trailzerovalue = 0;
	RsslInt32	trailzerocount = 0;
	int			isNeg;
	int			foundDigit = 0;
	int			nextDigit;
	int			plusZero = 0;

	__rtr_removewhitespace_end(strptr,endptr);

	if (strptr > endptr)
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_BLANK_DATA;
	}

	if ((*strptr == '+') && (*(strptr+1) == '0'))
	{
		if (strptr+1 == endptr)
		{
			oReal64->isBlank = RSSL_TRUE;
			oReal64->hint = 0;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			plusZero = 1;
	}

	if (strnicmp(strptr, "Inf", 3 ) == 0)
	{
		strptr += 3;
		if (strptr > endptr)
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_INFINITY;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (strnicmp(strptr, "-Inf", 4) == 0)
	{
		strptr += 4;
		if (strptr > endptr)
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_NEG_INFINITY;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (strnicmp(strptr, "NaN", 3) == 0)
	{
		strptr += 3;
		if (strptr > endptr)
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_NOT_A_NUMBER;
			oReal64->value = 0;
			return RSSL_RET_SUCCESS;
		}
		else
			return RSSL_RET_INVALID_DATA;
	}

	isNeg = __rtr_checknegative_skipsign(strptr);

	__rwf_atonumber_end_trailzero(strptr,endptr,value,foundDigit,trailzerovalue,trailzerocount,MAX_INT64,nextDigit,tempValue,isNeg);

	/* Check for decimal value */
	if (*strptr == '.')
	{
		/* It is a decimal value */
		const char *startdec = ++strptr;
		RsslUInt8 exponent;
		const char* test = NULL;

		__rwf_atonumber_end(strptr,endptr,value,foundDigit,MAX_INT64,nextDigit,tempValue,isNeg);

		if (foundDigit && strptr <= endptr)
		{
			// We have more digits - if the rest are zeros, it might still fit
			test = strptr;
			while (*test == '0' && test <= endptr)
				test++;
			if (test <= endptr)
				return RSSL_RET_INVALID_DATA;
		}

		exponent = (RsslUInt8)(strptr - startdec);

		if (exponent > 14)
		{
			/* Returns invalid data as the number is larger than the maximum hint value. */
			return RSSL_RET_INVALID_DATA;
		}

		if ((value == 0) && (plusZero == 1))
		{
			/* this was +0.00000..... so its blank */
			oReal64->isBlank = RSSL_TRUE;
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = 0;
		}
		else
		{
			/*Put a pointer on the last zero*/
			if (test)
				strptr = test;

			if (strptr > endptr)
			{
				oReal64->isBlank = RSSL_FALSE;
				oReal64->hint = RSSL_RH_EXPONENT0 - exponent;
				oReal64->value = (isNeg ? -1 * value : value);
			}
			else
				return RSSL_RET_INVALID_DATA;
		}
	}
	else if (*strptr == ' ')
	{
		strptr++;
		__rtr_removewhitespace_end(strptr,endptr);

		/* Check for another digit. Then might be a fraction. */
		if (_rtr_acisdigit(*strptr))
		{
			RsslInt64	numerator=0,tempValue=0;
			RsslInt64	denominator=0;

			__rwf_atonumber_end(strptr,endptr,numerator,foundDigit,MAX_INT64,nextDigit,tempValue,isNeg);

			/* Verify fraction */
			if (*strptr != '/')
				return RSSL_RET_INVALID_DATA;

			strptr++;
			__rwf_atonumber_end(strptr,endptr,denominator,foundDigit,MAX_INT32,nextDigit,tempValue,isNeg);

			if ((oReal64->hint = _rwf_SetFractionHint(denominator)) == 0)
				return RSSL_RET_INVALID_DATA;

			if (strptr > endptr)
			{
				RsslInt64	multiplyValue;

				multiplyValue = value * denominator;
				/*Check overflow by multiplying*/
				if (value != 0 && multiplyValue / value != denominator)
					return RSSL_RET_INVALID_DATA;

				/*Check overflow by sum*/
				if (numerator > 0 && multiplyValue > MAX_INT64 - numerator)
					return RSSL_RET_INVALID_DATA;

				value = multiplyValue + numerator;
				oReal64->isBlank = RSSL_FALSE;
				oReal64->value = (isNeg ? -1 * value : value);
			}
			else
				return RSSL_RET_INVALID_DATA;
		}

		if (strptr > endptr)
		{
			oReal64->isBlank = RSSL_FALSE;
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = (isNeg ? -1 * value : value);
		}
		else
			return RSSL_RET_INVALID_DATA;
	}
	else if (*strptr == '/')
	{
		RsslInt32	denominator=0,tempValue=0;

		strptr++;
		__rwf_atonumber_end(strptr,endptr,denominator,foundDigit,MAX_INT32,nextDigit,tempValue,isNeg);

		if ((oReal64->hint = _rwf_SetFractionHint(denominator)) == 0)
			return RSSL_RET_INVALID_DATA;

		if (strptr > endptr)
		{
			/* value stays as value */
			oReal64->isBlank = RSSL_FALSE;
			oReal64->value = (isNeg ? -1 * value : value);
		}
		else
			return RSSL_RET_INVALID_DATA;

	}
	else if (foundDigit)
	{
		if ( strptr <= endptr )
		{
			// We have more digits - if the rest are zeros, it might still fit
			if ( *strptr == '0' )
			{
				if ( trailzerocount == 0 )
					trailzerovalue = value;
				while ( *strptr == '0' && strptr <= endptr)
				{
					trailzerocount++;
					strptr++;
				}
				if (strptr <= endptr)
					return RSSL_RET_INVALID_DATA;
			}
			else
				return RSSL_RET_INVALID_DATA;
		}

		/* Exponential */
		oReal64->isBlank = RSSL_FALSE;
		if (trailzerocount > 0)
		{
			if (trailzerocount > 7)
			{
				while ( trailzerocount > 7 )
				{
					if ( trailzerovalue > MAX_INT64DIV10 )
						break;
					trailzerovalue = trailzerovalue * 10;
					trailzerocount--;
				}
				if ( trailzerocount > 7 )
					return RSSL_RET_INVALID_DATA;
			}
			oReal64->hint = RSSL_RH_EXPONENT0 + (RsslUInt8)trailzerocount;
			oReal64->value = (isNeg == 0) ? trailzerovalue : -(RsslInt)trailzerovalue;
		}
		else
		{
			oReal64->hint = RSSL_RH_EXPONENT0;
			oReal64->value = (isNeg ? -1 * value : value);
			if ((value == 0) && (plusZero == 1))
			{
				/* this is actually blank */
				oReal64->isBlank = RSSL_TRUE;
			}
		}
	}
	else if (strptr > endptr)
	{
		oReal64->isBlank = RSSL_TRUE;
		oReal64->hint = 0;
		oReal64->value = 0;
		return RSSL_RET_BLANK_DATA;
	}
	else
		return RSSL_RET_INVALID_DATA;

	return RSSL_RET_SUCCESS;
}

RsslRet rwf_stodatetime( RsslDateTime *oDTime, const char *strptr )
{
	int			foundDigit = 0;
	rtrInt8		nextDigit,value8=0,tValue8=0;
	rtrInt16	value16=0,tValue16=0;

	rsslClearDateTime(oDTime);

	__rtr_removewhitespace_null(strptr);
	__rwf_atonumber_null(strptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != '/')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.month = value8;
	value8 = 0;

	__rwf_atonumber_null(strptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != '/')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.day = value8;
	value8 = 0;

	__rwf_atonumber_null(strptr,value16,foundDigit,MAX_INT16,nextDigit,tValue16,RSSL_FALSE);
	if (*strptr++ != ' ')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.year = value16;
	value16 = 0;

	__rwf_atonumber_null(strptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != ':')
		return RSSL_RET_INVALID_DATA;
	oDTime->time.hour = value8;
	value8 = 0;

	__rwf_atonumber_null(strptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != ':')
		return RSSL_RET_INVALID_DATA;
	oDTime->time.minute = value8;
	value8 = 0;

	__rwf_atonumber_null(strptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	oDTime->time.second	= value8;
	
	/* milliseconds */
	if (*strptr++ == ':') 
	{
		__rwf_atonumber_null(strptr,value16,foundDigit,MAX_INT16,nextDigit,tValue16,RSSL_FALSE);
		oDTime->time.millisecond = value16;
	}
	else
		oDTime->time.millisecond = 0;
		
	return RSSL_RET_SUCCESS;
}

RsslRet rwf_stodatetime_size( RsslDateTime *oDTime, const char *strptr, const char *endptr )
{
	int			foundDigit = 0;
	rtrInt8		nextDigit,value8=0,tValue8=0;
	rtrInt16	value16=0,tValue16=0;

	rsslClearDateTime(oDTime);

	__rtr_removewhitespace_end(strptr,endptr);
	__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != '/')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.month = value8;
	value8 = 0;

	__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != '/')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.day = value8;
	value8 = 0;

	__rwf_atonumber_end(strptr,endptr,value16,foundDigit,MAX_INT16,nextDigit,tValue16,RSSL_FALSE);
	if (*strptr++ != ' ')
		return RSSL_RET_INVALID_DATA;
	oDTime->date.year = value16;

	__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != ':')
		return RSSL_RET_INVALID_DATA;
	oDTime->time.hour = value8;
	value8 = 0;

	__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	if (*strptr++ != ':')
		return RSSL_RET_INVALID_DATA;
	oDTime->time.minute = value8;
	value8 = 0;

	__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT8,nextDigit,tValue8,RSSL_FALSE);
	oDTime->time.second = value8;
	value8 = 0;
	
	if (*strptr++ == ':')
	{
		__rwf_atonumber_end(strptr,endptr,value8,foundDigit,MAX_INT16,nextDigit,tValue8,RSSL_FALSE);
		oDTime->time.millisecond = value8;
	}
	else
		oDTime->time.millisecond = 0;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslIPAddrBufferToUInt(RsslUInt32 *pAddrUInt, const RsslBuffer *pAddrString)
{
	const char *strptr = pAddrString->data;  
	const char *endptr = pAddrString->data + pAddrString->length - 1; 
	RsslUInt8 val[4]; 
	int byteCount = 0;

	RSSL_ASSERT(pAddrString && pAddrUInt, Invalid parameters or parameters passed in as NULL);

	if (pAddrString->length == 0)
	{
		*pAddrUInt = 0;
		return RSSL_RET_SUCCESS;
	}

	__rtr_removewhitespace_end(strptr, endptr);

	if (strptr > endptr)
	{
		*pAddrUInt = 0;
		return RSSL_RET_SUCCESS;
	}

	while (byteCount < 4)
	{
		RsslInt64 value = 0, tempValue = 0;
		int foundDigit = 0, nextDigit;

		__rwf_atonumber_end(strptr, endptr, value, foundDigit, MAX_INT64, nextDigit, tempValue, RSSL_FALSE);

		if (!foundDigit) 
			break;
		
		if (value > 255)
			return RSSL_RET_FAILURE;

		val[byteCount] = (RsslUInt8)value; /* Store in network byte order, will swap to host */
		byteCount++;

		if (strptr > endptr)
			break;

		if (*strptr == '.')	
			strptr++;
	}

	if (strptr <= endptr)
		__rtr_removewhitespace_end(strptr, endptr);

	if (strptr > endptr && byteCount == 4)
	{
		rwfGet32(*pAddrUInt, val); /* Swap to host byte order */
		return RSSL_RET_SUCCESS;
	}
	else
		return RSSL_RET_FAILURE;
}

/* Null terminated string is required */
RSSL_API RsslRet rsslIPAddrStringToUInt(const char *pAddrString, RsslUInt32 *pAddrUInt)
{
	RsslBuffer buffer;

	RSSL_ASSERT(pAddrString && pAddrUInt, Invalid parameters or parameters passed in as NULL);

	buffer.data = (char*)pAddrString;
	buffer.length = (RsslUInt32)strlen(pAddrString);

	return rsslIPAddrBufferToUInt(pAddrUInt, &buffer);
}

RSSL_API void rsslIPAddrUIntToString(RsslUInt32 addrUInt, char *pAddrString)
{
	RsslUInt32 swappedUInt;
	RSSL_ASSERT(pAddrString, Invalid parameters or parameters passed in as NULL);

	/* Swap back to network byte order and print */
	rwfGet32(swappedUInt, &addrUInt);

	snprintf(pAddrString, 16, "%u.%u.%u.%u",
		((RsslUInt8*)&swappedUInt)[0],
		((RsslUInt8*)&swappedUInt)[1],
		((RsslUInt8*)&swappedUInt)[2],
		((RsslUInt8*)&swappedUInt)[3]);
}

