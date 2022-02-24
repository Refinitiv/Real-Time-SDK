/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|             Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslBindThread.h"
#include "rtr/rsslThread.h"

#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#endif

RSSL_API RsslUInt32 rsslGetNumberOfProcessorCore(void)
{
	RsslUInt32 nProcessors = 0;
#ifdef WIN32
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	nProcessors = systemInfo.dwNumberOfProcessors;
#else	// Linux
	nProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return nProcessors;
}

RSSL_API RsslBool rsslIsProcessorCoreNumberValid(RsslInt32 cpuId)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	return ( 0 < nProcessors && ((RsslUInt32)cpuId < nProcessors) ? RSSL_TRUE : RSSL_FALSE);
}

RSSL_API RsslBool rsslIsStrProcessorCoreNumberValid(const char* cpuString)
{
	char* pEnd = NULL;
	RsslInt32 cpuId = 0;

	if (!cpuString)
	{
		return RSSL_FALSE;
	}

	/* Convert cpuString to an integer: Cpu core id. */
	cpuId = strtol(cpuString, &pEnd, 10);
	if (cpuId > 0 || cpuId == 0 && pEnd && (pEnd - cpuString) > 0)
	{
		return rsslIsProcessorCoreNumberValid(cpuId);
	}

	return RSSL_FALSE;
}

RSSL_API RsslRet rsslBindProcessorCoreThread(RsslInt32 cpuId, RsslErrorInfo* pError)
{
#ifdef WIN32

	DWORD_PTR affinityMask = 1ULL << cpuId;
	RsslInt32 errorCode = 0;

	if (SetThreadAffinityMask(GetCurrentThread(), affinityMask) == 0)
	{
		errorCode = GetLastError();
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpu mask 0x%llX, cpuid %d.  Error code is %d.", affinityMask, cpuId, errorCode);
		return RSSL_RET_FAILURE;
	}

#else	// Linux

	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	CPU_SET(cpuId, &cpuSet);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpuid %d.  Errno is %d.", cpuId, errno);
		return RSSL_RET_FAILURE;
	}

#endif
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThread(const char* cpuString, RsslErrorInfo* pError)
{
	char* pEnd = NULL;
	RsslInt32 cpuId = 0;

	if (!cpuString)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"cpuString is not set.");
		return RSSL_RET_FAILURE;
	}

	/* Convert cpuString to an integer: Cpu core id. */
	cpuId = strtol(cpuString, &pEnd, 10);
	if (cpuId > 0 || cpuId == 0 && pEnd && (pEnd - cpuString) > 0)
	{
		if (rsslBindProcessorCoreThread(cpuId, pError) != RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}
