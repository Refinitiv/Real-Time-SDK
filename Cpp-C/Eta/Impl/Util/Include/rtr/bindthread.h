/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
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

#include "rtr/cputopology.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslTypes.h"

/* CPU topology data structure */
typedef struct {
    RsslUInt32	logicalCpuCount;   // the number of logical processors
    GLKTSN_T* cpu_topology_ptr;  // full CPU topology describers
} RsslCPUTopology;

RsslUInt64 rsslGetAffinityMaskByCpuArray(RsslUInt* cpuArray, RsslUInt cpuCount);

RsslRet rsslBindThreadToCpuArray(const char* cpuString, RsslUInt* cpuArray, RsslUInt cpuCount, RsslInt procSet, RsslErrorInfo* pError);

/* Binds the calling thread to the core with the given CPU as cpuString in P:X C:Y T:Z format. */
/* @param cpuString the Cpu core in string format (P:X C:Y T:Z format). */
RsslRet rsslBindThreadWithString(const char* cpuString, RsslErrorInfo* pError);

/* Clear all the bindings threads to CPU cores. */
RSSL_API void rsslClearBindings();

/**
* @brief Initialize all the resources: mutex, cpuid library.
*
* @return RSSL_RET_SUCCESS when initialization was successful.
* @return RSSL_RET_FAILURE on critical error: memory allocation error.
*/
RSSL_API RsslRet rsslBindThreadInitialize();

/**
* @brief Uninitialize all the resources: mutex, cpuid library.
*
* @return always return RSSL_RET_SUCCESS
*/
RSSL_API RsslRet rsslBindThreadUninitialize();

#ifdef __cplusplus
}
#endif

#endif  // _RTR_BIND_THREAD_H_
