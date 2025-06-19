/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_ATOI_H
#define __RTR_ATOI_H

#include "rtr/os.h"
#include "rtr/rtrdefs.h"
#include "limits.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _rtr_acisdigit(c) ((c >= '0')&&(c <= '9'))
#define _rtr_actoint(c) (c-'0')
#define _rtr_ac1toul(c) (rtr_acisdigit(c) ? (unsigned long)_rtr_actoint(c) : 0)
#define UI64_MAX_CUT		(ULLONG_MAX/10)
#define UI64_MAX_LIM		(ULLONG_MAX%10)
#define UI32_MAX_CUT		(UINT_MAX/10)
#define UI32_MAX_LIM		(UINT_MAX%10)
#define UI16_MAX_CUT		(USHRT_MAX/10)
#define UI16_MAX_LIM		(USHRT_MAX%10)
#define UI8_MAX_CUT			(UCHAR_MAX/10)
#define UI8_MAX_LIM			(UCHAR_MAX%10)

#define I64_MAX_CUT			(LLONG_MAX/10)
#define I64_MAX_LIM			(LLONG_MAX%10)
#define I64_MIN_CUT			(((unsigned long long)LLONG_MAX+1)/10)
#define I64_MIN_LIM			(((unsigned long long)LLONG_MAX+1)%10)
#define I32_MIN_CUT			(((unsigned long long)INT_MAX+1)/10)
#define I32_MIN_LIM			(((unsigned long long)INT_MAX+1)%10)
#define I32_MAX_CUT			(INT_MAX/10)
#define I32_MAX_LIM			(INT_MAX%10)
#define I16_MAX_CUT			(SHRT_MAX/10)
#define I16_MAX_LIM			(SHRT_MAX%10)
#define I16_MIN_CUT			(((unsigned long long)SHRT_MAX+1)/10)
#define I16_MIN_LIM			(((unsigned long long)SHRT_MAX+1)%10)
#define I8_MAX_CUT			(SCHAR_MAX/10)
#define I8_MAX_LIM			(SCHAR_MAX%10)
#define I8_MIN_CUT			(((unsigned long long)SCHAR_MAX+1)/10)
#define I8_MIN_LIM			(((unsigned long long)SCHAR_MAX+1)%10)

#define __rtr_removewhitespace_null(ptr) \
	while ((*ptr != '\0') && (*ptr == ' ')) \
		ptr++;

#define __rtr_removewhitespace_end(ptr,endstr) \
	while ((ptr <= endstr) && (*ptr == ' ')) \
		ptr++;

#define __rtr_atonumber_null(ptr,result) \
	while ((*ptr != '\0') && _rtr_acisdigit(*ptr)) \
	{ \
		result = (result * 10) + _rtr_actoint(*ptr); \
		ptr++; \
	}

#define __rtr_atonumber_end(ptr,endstr,result) \
	while ((ptr <= endstr) && _rtr_acisdigit(*ptr)) \
	{ \
		result = (result * 10) + _rtr_actoint(*ptr); \
		ptr++; \
	}

#define __rtr_atonumber_end_size_check(ptr,endstr,result, maxcut, maxlim) \
	while ((ptr <= endstr) && _rtr_acisdigit(*ptr)) \
	{ \
		if (result > maxcut || (result == maxcut && _rtr_actoint(*ptr) > maxlim))\
			break; \
		result = (result * 10) + _rtr_actoint(*ptr); \
		ptr++; \
	}

#define __rtr_atoui64_size_check(ptr, endstr, result) \
	__rtr_atonumber_end_size_check(ptr, endstr, result, UI64_MAX_CUT, UI64_MAX_LIM)

#define __rtr_atoui32_size_check(ptr, endstr, result) \
	__rtr_atonumber_end_size_check(ptr, endstr, result, UI32_MAX_CUT, UI32_MAX_LIM)

#define __rtr_atoui16_size_check(ptr, endstr, result) \
	__rtr_atonumber_end_size_check(ptr, endstr, result, UI16_MAX_CUT, UI16_MAX_LIM)

#define __rtr_atoui8_size_check(ptr, endstr, result) \
	__rtr_atonumber_end_size_check(ptr, endstr, result, UI8_MAX_CUT, UI8_MAX_LIM)

#define __rtr_atoi64_size_check(ptr, endstr, result, isNegative) \
	if(isNegative) \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I64_MIN_CUT, I64_MIN_LIM)}\
	else \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I64_MAX_CUT, I64_MAX_LIM)}

#define __rtr_atoi32_size_check(ptr, endstr, result, isNegative) \
	if(isNegative) \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I32_MIN_CUT, I32_MIN_LIM)} \
	else \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I32_MAX_CUT, I32_MAX_LIM)}

#define __rtr_atoi16_size_check(ptr, endstr, result, isNegative) \
	if(isNegative) \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I16_MIN_CUT, I16_MIN_LIM)} \
	else \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I16_MAX_CUT, I16_MAX_LIM)}

#define __rtr_atoi8_size_check(ptr, endstr, result, isNegative) \
	if(isNegative) \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I8_MIN_CUT, I8_MIN_LIM)} \
	else \
		{__rtr_atonumber_end_size_check(ptr, endstr, result, I8_MAX_CUT, I8_MAX_LIM)}

#define __rtr_checknegative_skipsign(ptr) \
	(*ptr == '-' ? ptr++, 1 : (*ptr == '+' ? ptr++, 0 : 0 ) )

	/* Test to see if an ascii character is a valid digit.
	 * Returns : 1 if true, 0 otherwise.
 	 */
RTR_C_INLINE int rtr_acisdigit(char c)
{
	return(_rtr_acisdigit(c));
}

	/* Convert an ascii character to a long/unsigned long.
	 * Returns : value.
	 */
RTR_C_INLINE long rtr_actol(char c)
{
	RTPRECONDITION(_rtr_acisdigit(c));
	return((long)_rtr_actoint(c));
}

RTR_C_INLINE unsigned long rtr_actoul(char c)
{
	RTPRECONDITION(_rtr_acisdigit(c));
	return((unsigned long)_rtr_actoint(c));
}

#define rtr_actoi(c) (int)rtr_actol(c)
#define rtr_actoui(c) (unsigned int)rtr_actoul(c)


	/* Convert 1 or 2 byte ascii string to an unsigned long.
	 * Returns : value.
	 */
RTR_C_INLINE unsigned long rtr_ac1toul(char *input)
{
	return(_rtr_ac1toul(input[0]));
}

RTR_C_INLINE unsigned long rtr_ac2toul(char *input)
{
	return( (_rtr_ac1toul(input[0]) * 10) + _rtr_ac1toul(input[1]) );
}

#define rtr_ac1toui(ptr) (int)rtr_ac1toul(ptr)
#define rtr_ac2toui(ptr) (unsigned int)rtr_ac2toul(ptr)


	/* Convert ascii string to a long/unsinged long. String ends at
	 * null pointer. Leading whitespace is removed. Number
	 * conversion stops when a non-digit character is encountered.
	 * Returns : value.
	 */
RTR_C_INLINE long rtr_atol(char *str)
{
	long result=0;
	int isNeg;

	__rtr_removewhitespace_null(str);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atonumber_null(str,result);

	if (isNeg)
		result = -result;

	return result;
}
#define rtr_atoi(strptr) (int)rtr_atol(strptr)

RTR_C_INLINE unsigned long rtr_atoul(char *str)
{
	unsigned long result=0;

	__rtr_removewhitespace_null(str);

	if (*str == '+')
		str++;

	__rtr_atonumber_null(str,result);

	return result;
}


#define rtr_atoui(strptr) (unsigned int)rtr_atoul(strptr)

	/* Convert ascii string to a long/unsinged long. String ends at,
	 * and includes, endstr. Leading whitespace is removed. Number
	 * conversion stops when a non-digit character is encountered.
	 * Returns : value.
	 */
RTR_C_INLINE long rtr_atol_size(char *str, char *endstr)
{
	long result=0;
	int isNeg;

	__rtr_removewhitespace_end(str,endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atonumber_end(str,endstr,result);

	if (isNeg)
		result = -result;

	return result;
}

#define rtr_atoi_size(strptr,endstrptr) (int)rtr_atol_size(strptr,endstrptr)

RTR_C_INLINE unsigned long rtr_atoul_size(char *str, char *endstr)
{
	unsigned long result=0;

	__rtr_removewhitespace_end(str,endstr);

	if (*str == '+')
		str++;

	__rtr_atonumber_end(str,endstr,result);

	return result;
}
#define rtr_atoui_size(strptr,endstrptr) (unsigned int)rtr_atoul_size(strptr,endstrptr)


	/* Convert ascii string to a long long/unsinged long long. String
	 * end at null pointer. Leading whitespace is removed. Number
	 * conversion stops when a non-digit character is encountered.
	 * Returns : value.
	 */
RTR_C_INLINE rtrInt64 rtr_atoll(char *str)
{
	rtrInt64 result=0;
	int isNeg;

	__rtr_removewhitespace_null(str);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atonumber_null(str,result);

	if (isNeg)
		result = -result;

	return result;
}

RTR_C_INLINE rtrUInt64 rtr_atoull(char *str)
{
	rtrUInt64 result=0;

	__rtr_removewhitespace_null(str);

	if (*str == '+')
		str++;

	__rtr_atonumber_null(str,result);

	return result;
}

	/* Convert ascii string to a long long/unsinged long long. String ends
	 * at, and includes, endstr. Leading whitespace is removed. Number
	 * conversion stops when a non-digit character is encountered.
	 * Returns : value.
	 */
RTR_C_INLINE rtrInt64 rtr_atoll_size(char *str, char *endstr)
{
	rtrInt64 result=0;
	int isNeg;

	__rtr_removewhitespace_end(str,endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atonumber_end(str,endstr,result);

	if (isNeg)
		result = -result;

	return result;
}

RTR_C_INLINE rtrUInt64 rtr_atoull_size(char *str, char *endstr)
{
	rtrUInt64 result=0;

	__rtr_removewhitespace_end(str,endstr);

	if (*str == '+')
		str++;

	__rtr_atonumber_end(str,endstr,result);

	return result;
}

RTR_C_INLINE char *rtr_atoui8_size_check(char *str, char *endstr, rtrUInt8* result)
{
	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	if (__rtr_checknegative_skipsign(str))
		return str;

	__rtr_atoui8_size_check(str, endstr, (*result));

	return str;
}

RTR_C_INLINE char *rtr_atoui16_size_check(char *str, char *endstr, rtrUInt16* result)
{
	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	if (__rtr_checknegative_skipsign(str))
		return str;

	__rtr_atoui16_size_check(str, endstr, (*result));

	return str;
}

RTR_C_INLINE char *rtr_atoui32_size_check(char *str, char *endstr, rtrUInt32* result)
{
	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	if (__rtr_checknegative_skipsign(str))
		return str;

	__rtr_atoui32_size_check(str, endstr, (*result));

	return str;
}


RTR_C_INLINE char *rtr_atoui64_size_check(char *str, char *endstr, rtrUInt64 *result)
{
	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	if (__rtr_checknegative_skipsign(str))
		return str;

	__rtr_atoui64_size_check(str, endstr, (*result));

	return str;
}

RTR_C_INLINE char *rtr_atoi8_size_check(char *str, char *endstr, rtrInt8* result)
{
	char isNeg = 0;
	unsigned long long res = 0;

	__rtr_removewhitespace_end(str, endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atoi8_size_check(str, endstr, res, isNeg);

	if (isNeg)
		/* this need code excludes int8 overflow risk*/
		*result = (-((rtrInt8)(res - 1))) - 1;
	else
		*result = (rtrInt8)res;

	return str;
}


RTR_C_INLINE char *rtr_atoi16_size_check(char *str, char *endstr, rtrInt16* result)
{
	char isNeg = 0;
	unsigned long long res = 0;

	__rtr_removewhitespace_end(str, endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atoi16_size_check(str, endstr, res, isNeg);

	if (isNeg)
		/* this need code excludes int16 overflow risk*/
		*result = (-((rtrInt16)(res - 1))) - 1;
	else
		*result = (rtrInt16)res;

	return str;
}

RTR_C_INLINE char *rtr_atoi32_size_check(char *str, char *endstr, rtrInt32* result)
{
	char isNeg = 0;
	unsigned long long res = 0;

	__rtr_removewhitespace_end(str, endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atoi32_size_check(str, endstr, res, isNeg);

	if (isNeg)
		/* this need code excludes int32 overflow risk*/
		*result = (-((rtrInt32)(res - 1))) - 1;
	else
		*result = (rtrInt32)res;

	return str;
}

RTR_C_INLINE char *rtr_atoi64_size_check(char *str, char *endstr, rtrInt64 *result)
{
	unsigned long long res = 0;
	char isNeg = 0;

	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	isNeg = __rtr_checknegative_skipsign(str);

	__rtr_atoi64_size_check(str, endstr, res, isNeg);


	if (isNeg)
		/* this need code excludes int64 overflow risk*/
		*result = (-((rtrInt64)(res - 1))) - 1;
	else
		*result = (rtrInt64)res;

	return str;
}

#ifdef __cplusplus
};
#endif

#endif
