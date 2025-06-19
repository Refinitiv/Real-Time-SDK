/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*	These functions provide fast rwf to string convertions.
 *	The function returns a pointer, within 'str' to be beginning
 *	of the string. The ascii string value is constructed backwards
 *	from the end of (str + strlen) for efficiency reasons.
*/

#ifndef __RSSL_RWFCONVERT_H
#define __RSSL_RWFCONVERT_H

#include "rtr/rsslReal.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rtratoi.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct {
	unsigned int	printSign;		/* Force the printing of the sign */
	unsigned int	padZero;		/* Pad with zeros. The value of padZero
									 * tells the max size to pad. */
} rwfTosOptions;
#define RWF_INIT_TOS_OPTS {0,0}


/* These functions convert a signed integer (32 bit) to an ascii string. */
extern char * rwfItos(			char *str,
								RsslUInt32 strlen,
								RsslInt32 iVal);
extern char * rwfItosOpts(		char *str,
								RsslUInt32 strlen,
								RsslInt32 iVal,
								rwfTosOptions *opts);

/* These functions convert an unsigned integer (32 bit) to an ascii string. */
extern char * rwfUItos(			char *str,
								RsslUInt32 strlen,
								RsslUInt32 iVal);
extern char * rwfUItosOpts(		char *str,
								RsslUInt32 strlen,
								RsslUInt32 iVal,
								rwfTosOptions *opts);


/* These functions convert a signed integer (64 bit) to an ascii string. */
extern char * rwfI64tos(		char *str,
								RsslUInt32 strlen,
								RsslInt64 iVal);
extern char * rwfI64tosOpts(	char *str,
								RsslUInt32 strlen,
								RsslInt64 iVal,
								rwfTosOptions *opts);

/* These functions convert a unsigned integer (64 bit) to an ascii string. */
extern char * rwfUI64tos(		char *str,
								RsslUInt32 strlen,
								RsslUInt64 iVal);
extern char * rwfUI64tosOpts(	char *str,
								RsslUInt32 strlen,
								RsslUInt64 iVal,
								rwfTosOptions *opts);

/* These functions convert an real64 to an ascii string. */
extern char * rwfReal64tosOpts(	char *str,
								RsslUInt32 strlen,
								RsslReal *iVal,
								rwfTosOptions *opts);

/* These functions convert a date time to an ascii string. */
extern char * rwfDateTimetos(	char *str,
								RsslUInt32 strlen,
								RsslDateTime *iDTime );


/* These functions convert an ascii string to an RsslInt32 */
RTR_C_INLINE RsslInt32 rwf_stoi(const char *strptr)
{
	return (rtr_atol((char*)strptr));
}
RTR_C_INLINE RsslInt32 rwf_stoi_size(const char *strptr, const char *endptr)
{
	return (rtr_atol_size((char*)strptr,(char*)endptr));
}

/* These functions convert an ascii string to an RsslUInt32 */
RTR_C_INLINE RsslUInt32 rwf_stoui(const char *strptr)
{
	return (rtr_atoul((char*)strptr));
}
RTR_C_INLINE RsslUInt32 rwf_stoui_size(const char *strptr, const char *endptr)
{
	return (rtr_atoul_size((char*)strptr,(char*)endptr));
}

/* These functions convert an ascii string to an RsslInt64 */
RTR_C_INLINE RsslInt64 rwf_stoi64(const char *strptr)
{
	return (rtr_atoll((char*)strptr));
}
RTR_C_INLINE RsslInt64 rwf_stoi64_size(const char *strptr, const char *endptr)
{
	return (rtr_atoll_size((char*)strptr,(char*)endptr));
}

/* These functions convert an ascii string to an RsslUInt64 */
RTR_C_INLINE RsslUInt64 rwf_stoui64(const char *strptr)
{
	return (rtr_atoull((char*)strptr));
}
RTR_C_INLINE RsslUInt64 rwf_stoui64_size(const char *strptr, const char *endptr)
{
	return (rtr_atoull_size((char*)strptr,(char*)endptr));
}

/* These functions convert an ascii string to an RsslReal */
extern RsslRet rwf_storeal64(
					RsslReal *oReal64,
					const char *strptr );
extern RsslRet rwf_storeal64_size(
					RsslReal *oReal64,
					const char *strptr,
					const char *endptr );

/* These functions convert an ascii string to an RsslDateTime */
extern RsslRet rwf_stodatetime(
					RsslDateTime *oDTime,
					const char *strptr );

extern RsslRet rwf_stodatetime_size(
					RsslDateTime *oDTime,
					const char *strptr,
					const char *endptr );

#if defined(__cplusplus)
}
#endif


#endif

