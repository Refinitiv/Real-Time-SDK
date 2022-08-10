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

// maximal length of cpu string for binding
#define MAX_CPU_STRING_LEN 512U

/**
* @brief The number of logical processors.
* 
* @return The number of logical processors.
*/
RSSL_API RsslUInt32 rsslGetNumberOfProcessorCore(void);

/**
* Tests that the logical processor number is valid in the system.
* @param cpuId the logical index of Cpu core.
* @return RSSL_TRUE when the requested processor number is valid in the system; otherwise RSSL_FALSE.
*/
RSSL_API RsslBool rsslIsProcessorCoreNumberValid(RsslInt32 cpuId);

/**
* @brief Is the logical processor number valid.
* This is a quick test only.
* @param cpuString Specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format).
* @retun RSSL_TRUE when the Cpu core specified in correct format.
* @return RSSL_FALSE when any error detected.
*/
RSSL_API RsslBool rsslIsStrProcessorCoreBindValid(const char* cpuString);

/**
 * @brief Binds the calling thread to the core with the given CPU ID.
 * @param cpuId the logical index of Cpu core.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS if the thread binding was successful.
 * @return RSSL_RET_FAILURE when an error received, specific description of binding error see in pError.
 */
RSSL_API RsslRet rsslBindProcessorCoreThread(RsslInt32 cpuId, RsslErrorInfo* pError);

/** 
 * @brief Binds the calling thread to the Cpu core.
 * @param cpuString Specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format).
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS if the thread binding was successful.
 * @return RSSL_RET_FAILURE when an error received, specific description of binding error see in pError.
 */
RSSL_API RsslRet rsslBindThread(const char* cpuString, RsslErrorInfo* pError);

/**
 * @brief Binds the calling thread to the Cpu core.
 * @param cpuString Specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format).
 * @param pOutputResult on Success, the list of logical core id that were bound for the calling thread; on Fail, an error text description same as in pError.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS if the thread binding was successful.
 * @return RSSL_RET_FAILURE when an error received, specific description of binding error see in pError.
 */
RSSL_API RsslRet rsslBindThreadEx(const char* cpuString, RsslBuffer* pOutputResult, RsslErrorInfo* pError);

#ifdef __cplusplus
}
#endif

#endif  // __RSSL_BIND_THREAD_H_
