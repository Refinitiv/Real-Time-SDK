/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|             Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __RSSL_BIND_THREAD_H_
#define __RSSL_BIND_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslTypes.h"

/* The number of logical processors. */
RSSL_API RsslUInt32 rsslGetNumberOfProcessorCore(void);

/* Is the logical processor number valid. */
RSSL_API RsslBool rsslIsProcessorCoreNumberValid(RsslInt32 cpuId);

/* Is the logical processor number valid. */
/* cpuString - specifies Cpu core: Cpu core id */
RSSL_API RsslBool rsslIsStrProcessorCoreNumberValid(const char* cpuString);

/* Binds the calling thread to the core with the given CPU ID. */
RSSL_API RsslRet rsslBindProcessorCoreThread(RsslInt32 cpuId, RsslErrorInfo* pError);

/* Binds the calling thread to the Cpu core. */
/* cpuString - specifies Cpu core: Cpu core id */
RSSL_API RsslRet rsslBindThread(const char* cpuString, RsslErrorInfo* pError);

#ifdef __cplusplus
}
#endif

#endif  // __RSSL_BIND_THREAD_H_
