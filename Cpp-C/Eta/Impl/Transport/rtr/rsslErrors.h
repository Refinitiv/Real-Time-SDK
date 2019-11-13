/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rsslerrors_h
#define __rsslerrors_h

#include <stdio.h>
#include "rtr/rsslTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef snprintf
#define snprintf        _snprintf
#endif
#endif

/* checks for null pointer and creates error if it exists */
RTR_C_ALWAYS_INLINE int _rsslNullPtr(char *func, char *ptrname, char *file, int line, RsslError *error)
{
	error->channel = NULL;
	error->sysError = 0;
	error->rsslErrorId = RSSL_RET_FAILURE;
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> %s() Error: 0002 Null pointer error. Argument %s cannot be NULL.\n", file, line, func, ptrname);

	return 1;
}

/* sets error values */
RTR_C_ALWAYS_INLINE void _rsslSetError(RsslError *error, RsslChannel *chnl, RsslRet retVal, int syserr)
{
	error->channel = chnl;
	error->sysError = syserr;
	error->rsslErrorId = retVal;
	
	return;
}

#define RSSL_NULL_PTR(ptr, func, ptrname, err) \
	(( ptr == 0) ? _rsslNullPtr(func, ptrname, __FILE__, __LINE__, err) : 0 )


/* Macro used by message encoders and decoders.  Upon failure, the error is set and the calling function returns an error */
#define RSSL_ERR_CHECK(__cond, __ret, __pError) {																\
		if (!(__cond)) {																						\
			_rsslSetError((__pError), 0, (__ret), 0);															\
			snprintf(__pError->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 (%s) failed with ret=%d", __FILE__, __LINE__, (#__cond), (__ret));		\
			return RSSL_RET_FAILURE;																			\
		}																										\
	}

/* Macro used by message encoders and decoders.  Upon failure, the error is set and the calling function returns an error */
#define RSSL_COND_CHECK(__cond, __ret, __pError) {															\
		if (!(__cond)) {																					\
			_rsslSetError((__pError), 0, (__ret), 0);														\
			snprintf(__pError->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> (%s) failed", __FILE__, __LINE__, (#__cond));					\
			return RSSL_RET_FAILURE;																		\
		}																									\
        }   

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
