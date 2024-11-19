/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2022,2024 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

#pragma once

/**
* bindthread.h
* This header file contains implementation specific declarations
* that uses for accessing to CpuId library.
*/

#ifndef _RTR_BIND_THREAD_H_
#define _RTR_BIND_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "eta/cputopology.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslTypes.h"

/* CPU topology data structure */
typedef struct {
    RsslUInt32	logicalCpuCount;   // the number of logical processors
    GLKTSN_T* cpu_topology_ptr;  // full CPU topology describers
} RsslCPUTopology;


/* Binds the calling thread to the core with the given CPU as cpuString in P:X C:Y T:Z format. */
/* @param cpuString the Cpu core in string format (P:X C:Y T:Z format). */
/* @param pOutputResult on Success, the list of logical core id that were bound for the calling thread; on Fail, an error text description same as in pError. */
RsslRet rsslBindThreadWithString(const char* cpuString, RsslBuffer* pOutputResult, RsslErrorInfo* pError);

/* Clear all the bindings threads to CPU cores. */
RSSL_API void rsslClearBindings();

/**
* @brief Initialize all the resources: mutex, cpuid library.
*
* @return RSSL_RET_SUCCESS when initialization was successful.
* @return RSSL_RET_FAILURE on critical error: memory allocation error.
*/
RSSL_API RsslRet rsslBindThreadInitialize(RsslError* error);

/**
* @brief Uninitialize all the resources: mutex, cpuid library.
*
* @return always return RSSL_RET_SUCCESS
*/
RSSL_API RsslRet rsslBindThreadUninitialize();

/**
* @brief Dump detected Cpu topology: list of the detected Cpu cores.
* When the initialization failed it dumps information about the error and internal details.
*/
RSSL_API void dumpCpuTopology();

/**
* @brief Get the physical CPU core identification in a PCT string that matched for logical core id.
* P(Package):X C(Core):Y T(Thread):Z
* @param (in) cpuId the logical index of Cpu core.
* @param (in/out) pCpuPCTString RsslBuffer with allocated memory space, it will be filled by result PCT string.
*
* @return RSSL_RET_SUCCESS when operation was success.
* @return RSSL_RET_FAILURE when got any error.
*/
RSSL_API RsslRet getPCTByProcessorCoreNumber(RsslInt32 cpuId, RsslBuffer* pCpuPCTString, RsslErrorInfo* pError);

/**
* @brief Is the logical core in online or offline.
* @param (in) cpuId the logical index of Cpu core.
*
* @return RSSL_TRUE when the processor unit is in online.
* @return RSSL_FALSE when the processor unit is in offline, or cputopology is not initialized.
*/
RSSL_API RsslBool isProcessorCoreOnline(RsslInt32 cpuId);

#ifdef __cplusplus
}
#endif

#endif  // _RTR_BIND_THREAD_H_
