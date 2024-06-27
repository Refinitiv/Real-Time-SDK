/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_RDM_CODES_H
#define _RTR_RSSL_RDM_CODES_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslDataUtils.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup RSSLVAUtils
 *	@{
 */

 /**
 * @brief Error Codes for RsslErrorInfo.  
 * @see RsslErrorInfo
 */
typedef enum
{
	RSSL_EIC_SUCCESS		= 0,	/*!< Success */
	RSSL_EIC_FAILURE		= -1,	/*!< General failure */
	RSSL_EIC_SHUTDOWN		= -2	/*!< Failure. RsslReactor was shut down */
} RsslErrorInfoCode;

#define RSSL_ERROR_INFO_LOCATION_LENGTH (1024)

 /**
 * @brief Error Information.  Used by RsslRDMMsg encode and decode functions, as well as the RsslReactor.
 * @see RsslErrorInfoCode
 */
typedef struct
{
	RsslErrorInfoCode	rsslErrorInfoCode;									/*!< Error Info Code. */
	RsslError			rsslError;											/*!< Rssl Error object.  When an error occurs, information about the error can be retrieved from this object. */
	char 				errorLocation[RSSL_ERROR_INFO_LOCATION_LENGTH];		/*!< Specifies the location within the RDM encoder/decoder or reactor source code where the error occurred. */
} RsslErrorInfo;

 /**
 * @brief Sets a given file:line location on an RsslErrorInfo structure.  Typically used by the RsslReactor when the rsslError member has been set by a function call.
 * @see RsslErrorInfo
 */
RTR_C_INLINE void rsslSetErrorInfoLocation(RsslErrorInfo *pError, const char *fileName, int lineNum)
{
	snprintf(pError->errorLocation, RSSL_ERROR_INFO_LOCATION_LENGTH, "%s:%d", fileName, lineNum);
}

 /**
 * @brief Sets the given error information and file:line location on an RsslErrorInfo structure.  
 * @see RsslErrorInfo
 */
RTR_C_INLINE void rsslSetErrorInfo(RsslErrorInfo *pError, RsslErrorInfoCode errorInfoCode, RsslRet rsslCode, const char *fileName, int lineNum, const char *errorString, ...)
{
	if (pError)
	{
		if (errorString) /* If present, fill in. Otherwise it was already filled by an Rssl function call. */
		{
			va_list fmtArgs; 
			va_start(fmtArgs, errorString);
			vsnprintf(pError->rsslError.text, MAX_RSSL_ERROR_TEXT, errorString, fmtArgs);
			va_end(fmtArgs);
			pError->rsslError.rsslErrorId = rsslCode;
		}
		pError->rsslError.sysError = 0;
		pError->rsslError.channel = NULL;
		rsslSetErrorInfoLocation(pError, fileName, lineNum);
		pError->rsslErrorInfoCode = errorInfoCode;
	}
}

 /**
 * @brief Copies an RsslErrorInfo structure.
 * @see RsslErrorInfo
 */
RTR_C_INLINE void rsslCopyErrorInfo(RsslErrorInfo *pNewError, RsslErrorInfo *pOldError)
{
	pNewError->rsslErrorInfoCode = pOldError->rsslErrorInfoCode;
	pNewError->rsslError.rsslErrorId = pOldError->rsslError.rsslErrorId;
	pNewError->rsslError.channel = pOldError->rsslError.channel;
	pNewError->rsslError.sysError = pOldError->rsslError.sysError;
	snprintf(pNewError->rsslError.text, MAX_RSSL_ERROR_TEXT, "%s", pOldError->rsslError.text);
	snprintf(pNewError->errorLocation, RSSL_ERROR_INFO_LOCATION_LENGTH, "%s", pOldError->errorLocation);
}

 /**
 * @brief Checks the given condition. If the condition is false, calls rsslSetErrorInfo() 
 * with a string representing the condition and the current location in the source code.
 * @see RsslErrorInfo
 */
#define RSSL_ERROR_INFO_CHECK(__cond, __ret, __pError) ((__cond) || (rsslSetErrorInfo((__pError), RSSL_EIC_FAILURE, (__ret), __FILE__, __LINE__, (#__cond)), 0))

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
