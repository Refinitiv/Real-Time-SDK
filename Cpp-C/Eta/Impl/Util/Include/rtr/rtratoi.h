/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_ATOI_H
#define __RTR_ATOI_H

#include "rtr/os.h"
#include "rtr/rtrdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _rtr_acisdigit(c) ((c >= '0')&&(c <= '9'))
#define _rtr_actoint(c) (c-'0')
#define _rtr_ac1toul(c) (rtr_acisdigit(c) ? (unsigned long)_rtr_actoint(c) : 0)

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

RTR_C_INLINE char * rtr_atoull_size_check(char *str, char *endstr, rtrUInt64* result)
{
	*result = 0;

	__rtr_removewhitespace_end(str, endstr);

	if (*str == '+')
		str++;

	__rtr_atonumber_end(str, endstr, (*result));

	return str;
}

#ifdef __cplusplus
};
#endif

#endif
