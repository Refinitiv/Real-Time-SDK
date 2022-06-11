/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|             Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslBindThread.h"
#include "rtr/rsslThread.h"
#include "rtr/bindthread.h"
#include "rtr/rtratomic.h"

#include <stdlib.h>
#include <ctype.h>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#endif

// Allows to print debug information
#undef _DUMP_DEBUG_

static RsslCPUTopology rsslCPUTopology;
static rtr_atomic_val initializedCpuTopology = 0;

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

RSSL_API RsslBool rsslIsStrProcessorCoreBindValid(const char* cpuString)
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
	// else when Cpu core is specified in format P:X C:Y T:Z
	// checks the 1-st character only.
	else
	{
		RsslUInt32 len = 0;
		const char* pStr = cpuString;
		while (len < MAX_CPU_STRING_LEN && isspace(*pStr))
		{
			++len;
			++pStr;
		}

		// checks the valid characters
		if (len < MAX_CPU_STRING_LEN)
		{
			switch (*pStr)
			{
			case 'P':
			case 'C':
			case 'T':
				return RSSL_TRUE;
			}
		}
	}

	return RSSL_FALSE;
}

RsslRet checkCpuIdInitializationError(RsslErrorInfo* pError)
{
	if (rsslCPUTopology.cpu_topology_ptr == NULL)
	{
		RsslErrorInfo* pInitCpuIdLibError = getErrorInitializationStage();
		if (pInitCpuIdLibError != NULL)
		{
			rsslCopyErrorInfo(pError, pInitCpuIdLibError);
		}
		else
		{
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
				"Cpu topology information is unavailable.");
		}
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/* Name:  parseSingleCpuString
*
* Description:	Takes as input a string of the following forms
*					P:# C:# T:# - denotes physical processor, core, and thread
*								  requested.
*					#			- Just a single number.  If only a number is
*								  found,  this will be the logical id assigned
*								  by the bios
*
*					The cpuArray and cpuCount store the list of logical ids
*					found in this string
*
* Returns:		The number of logical cpus found in this string. The
*					procSet variable will be set to 1 if processor sets are
*					requested.
*/
RsslRet parseSingleCpuString(char* cpuString, RsslUInt* idArray, RsslUInt* idCount, RsslInt* procSet, RsslErrorInfo* pError)
{
	RsslUInt32 i;
	RsslUInt32 lcl_maxcpu;

	RsslUInt16 procExpected = 0;
	RsslUInt16 coreExpected = 0;
	RsslUInt16 threadExpected = 0;

	RsslInt procId = -1;
	RsslInt coreId = -1;
	RsslInt threadId = -1;
	RsslInt currentId = 0;

	RsslUInt16 foundInteger = 0;
	RsslUInt16 foundChar = 0;

	char* stringIter = cpuString;
	for (;; stringIter++)
	{
		if (isspace(*stringIter))
		{
			continue;
		}
		else if ((isalpha(*stringIter)) || (*stringIter == '\0'))
		{
			if (isalpha(*stringIter))
				foundChar = 1;
			if ((foundInteger) && (foundChar))
			{
				if (procExpected)
				{
					procId = currentId;
				}
				else if (coreExpected)
				{
					coreId = currentId;
				}
				else if (threadExpected)
				{
					threadId = currentId;
				}
				else
				{
					rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
						"Syntax for cpu binding is invalid. unknow token. %s (%s)", stringIter, cpuString);
					return RSSL_RET_FAILURE;
				}

				currentId = 0;
				procExpected = 0;
				coreExpected = 0;
				threadExpected = 0;
				foundInteger = 0;
			}
			if (*stringIter == '\0')
				break;
			switch (*stringIter)
			{
			case  'P':
				procExpected = 1;
				coreExpected = 0;
				threadExpected = 0;
				break;
			case 'C':
				coreExpected = 1;
				procExpected = 0;
				threadExpected = 0;
				break;
			case 'T':
				threadExpected = 1;
				procExpected = 0;
				coreExpected = 0;
				break;
			}

			stringIter++;
			while ((*stringIter != '\0') && (*stringIter != ':') && (!isdigit(*stringIter)))
			{
				stringIter++;
			}

			if (*stringIter == '\0')
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
					"Syntax for cpu binding for string (eos) is invalid. (%s)", stringIter, cpuString);
				return RSSL_RET_FAILURE;
			}

			if (isdigit(*stringIter))
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
					"Syntax for cpu binding for string %s is invalid. (%s)", stringIter, cpuString);
				return RSSL_RET_FAILURE;
			}

			if (*stringIter == ':')
			{
				continue;
			}
		}
		else if (isdigit(*stringIter))
		{
			foundInteger = 1;
			currentId = (currentId * 10) + (int)*stringIter - '0';
		}
	}
	if (!foundChar)
	{
		idArray[*idCount] = currentId;
		*idCount = *idCount + 1;
	}
	else
	{
		GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;

		/* Iterate through the processor list to find matches */
		lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //getLogicalCpuCount();
		for (i = 0; i < lcl_maxcpu; i++)
		{
			// Don't check this logical id if its currently offline or
			// unavailable.
			//if (pCpuTopology->pApicAffOrdMapping[i].offline)  // the fields offline was removed.
			//	continue;
			if (pCpuTopology->pApicAffOrdMapping[i].packageORD != procId)
				continue;

			if (!(coreId == -1 || pCpuTopology->pApicAffOrdMapping[i].coreORD == coreId))
				continue;

			if (!(threadId == -1 || pCpuTopology->pApicAffOrdMapping[i].threadORD == threadId))
				continue;

			idArray[*idCount] = i;
			*idCount = *idCount + 1;
		}
	}

	RsslRet ret = (*idCount > 0 ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE);
	if (ret != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"Configuration setting %s did not match any physical processors on the system.", cpuString);
	}

	return ret;
}


/* Name:		parseFullCpuString
*
* Description:	Takes a full comma delimited cpu string containing either the
*				physical syntax for cpu's or logical. This method will call
*				parseSingleCpuString to actually extract the individual
*				elements of this array.
*/
RsslRet parseFullCpuString(const char* cpuString, RsslUInt* idArray, RsslUInt* idCount, RsslInt* procSet, RsslErrorInfo* pError)
{
	char tempString[MAX_CPU_STRING_LEN];
	char* stringIter = tempString;
	char* currentString = stringIter;

	RsslInt endtoken = 0;

	*idCount = 0;

	strncpy(tempString, cpuString, (MAX_CPU_STRING_LEN - 1));
	tempString[(MAX_CPU_STRING_LEN - 1)] = '\0';
	while (*idCount < MAX_CPUS_ARRAY)
	{
		if ((*stringIter == ',') || (*stringIter == '\0'))
		{
			endtoken = 0;
			if (*stringIter == '\0')
				endtoken = 1;
			*stringIter = '\0';

			if (parseSingleCpuString(currentString, idArray, idCount, procSet, pError) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			if (endtoken)
				break;
			else
			{
				currentString = stringIter + 1;
			}
		}
		stringIter++;
	}

	return RSSL_RET_SUCCESS;
}

#ifdef _DUMP_DEBUG_
void dumpCpuArray(RsslUInt* cpuArray, RsslUInt cpuCount)
{
	RsslUInt i;

	for (i = 0; i < cpuCount; i++)
	{
		printf("%llu ", cpuArray[i]);
	}
	printf("\n");
}
#endif // _DUMP_DEBUG_

RsslUInt64 rsslGetAffinityMaskByCpuArray(RsslUInt* cpuArray, RsslUInt cpuCount)
{
	RsslUInt64 affinity = 0ULL;
	RsslUInt i;

	for (i = 0; i < cpuCount; i++)
	{
		affinity = (affinity | (1ULL << cpuArray[i]));
	}
	return affinity;
}

RsslRet rsslBindThreadToCpuArray(const char* cpuString, RsslUInt* cpuArray, RsslUInt cpuCount, RsslInt procSet, RsslErrorInfo* pError)
{
#if defined(Linux) && !defined(x86_Linux_2X)
	RsslUInt i;
	cpu_set_t currentCPUSet;
	CPU_ZERO(&currentCPUSet);
	for (i = 0; i < cpuCount; i++)
	{
		CPU_SET(cpuArray[i], &currentCPUSet);
	}
#if defined(x86_Linux_3X) && !defined(x86_Linux_S9X)
	if (sched_setaffinity(0, &currentCPUSet) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpu configuration %s.  Errno is %d.", cpuString, errno);
		return RSSL_RET_FAILURE;
	}
#else
	if (sched_setaffinity(0, sizeof(currentCPUSet), &currentCPUSet) < 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpu configuration %s.  Errno is %d.", cpuString, errno);
		return RSSL_RET_FAILURE;
	}
#endif
#elif WIN32
	RsslUInt64 affinityMask = rsslGetAffinityMaskByCpuArray(cpuArray, cpuCount);
	RsslInt32 errorCode = 0;

	if (SetThreadAffinityMask(GetCurrentThread(), affinityMask) == 0)
	{
		errorCode = GetLastError();
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpu mask 0x%llX, cpu configuration %s.  Error code is %d.", affinityMask, cpuString, errorCode);
		return RSSL_RET_FAILURE;
	}
#endif
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindProcessorCoreThread(RsslInt32 cpuId, RsslErrorInfo* pError)
{
	// CpuId library initialization should be invoked before to work together with rsslBindThreadWithString.
	// Binding has side effect on an analyzing cpu topology procedure.
	if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

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

#ifdef _DUMP_DEBUG_
void dumpCpuTopology()
{
	RsslUInt32 i;
	RsslUInt32 lcl_maxcpu;

	GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
	lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //getLogicalCpuCount();

	printf("MaxCpu = %u\n", lcl_maxcpu);
	for (i = 0; i < lcl_maxcpu; i++)
	{
		printf("[%d]  P:%d C:%d T:%d | APICID=%d OrdIndexOAMsk=%d pkg_IDAPIC=%d Core_IDAPIC=%d SMT_IDAPIC=%d\n",
			i,
			pCpuTopology->pApicAffOrdMapping[i].packageORD,
			pCpuTopology->pApicAffOrdMapping[i].coreORD,
			pCpuTopology->pApicAffOrdMapping[i].threadORD,
			pCpuTopology->pApicAffOrdMapping[i].APICID,
			pCpuTopology->pApicAffOrdMapping[i].OrdIndexOAMsk,
			pCpuTopology->pApicAffOrdMapping[i].pkg_IDAPIC,
			pCpuTopology->pApicAffOrdMapping[i].Core_IDAPIC,
			pCpuTopology->pApicAffOrdMapping[i].SMT_IDAPIC
			);
	}
}
#endif

RsslRet rsslBindThreadWithString(const char* cpuString, RsslErrorInfo* pError)
{
	RsslUInt cpuCount = 0;
	RsslUInt cpuArray[MAX_CPUS_ARRAY];
	RsslInt procSet = 0;

	if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (parseFullCpuString(cpuString, cpuArray, &cpuCount, &procSet, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (rsslBindThreadToCpuArray(cpuString, cpuArray, cpuCount, procSet, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

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
	else  /* Parse cpuString - specifies Cpu core in string format P:X C:Y T:Z. */
	{
		if (rsslBindThreadWithString(cpuString, pError) != RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThreadInitialize()
{
	if (!initializedCpuTopology)
	{
		RsslErrorInfo rsslErrorInfo;

		RTR_ATOMIC_SET(initializedCpuTopology, 1);

		initCpuTopologyMutex();

		if (initializeCpuTopology(&rsslErrorInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

		rsslCPUTopology.cpu_topology_ptr = getCpuTopology();
		rsslCPUTopology.logicalCpuCount = getLogicalCpuCount();

#ifdef _DUMP_DEBUG_
		if (rsslCPUTopology.cpu_topology_ptr != NULL)
			dumpCpuTopology();
#endif
		rsslClearBindings();
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThreadUninitialize()
{
	if (initializedCpuTopology)
	{
		RTR_ATOMIC_SET(initializedCpuTopology, 0);
		unInitializeCpuTopology();
		destroyCpuTopologyMutex();
		rsslCPUTopology.cpu_topology_ptr = NULL;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API void rsslClearBindings()
{
	int lcl_maxcpu = rsslGetNumberOfProcessorCore();
	unsigned long mask = 0;
	int i;
#if defined(Linux) && !defined(x86_Linux_2X)
	cpu_set_t currentCPU;
	CPU_ZERO(&currentCPU);
	for (i = 0; i < lcl_maxcpu; i++)
	{
#if defined(x86_Linux_3X) && !defined(x86_Linux_S9X)
		mask = (unsigned long)(DWORD_PTR)(1ULL << i);
#else
		mask = i;
#endif
		CPU_SET(mask, &currentCPU);
	}
#if defined(x86_Linux_3X) && !defined(x86_Linux_S9X)
	if (!sched_setaffinity(0, &currentCPU))
		return;
#else
	if (!sched_setaffinity(0, sizeof(currentCPU), &currentCPU))
		return;
#endif
#elif WIN32
	DWORD_PTR affinity = 0;
	for (i = 0; i < lcl_maxcpu; i++)
	{
		affinity = affinity | (DWORD_PTR)(1ULL << i);
	}
	if (SetThreadAffinityMask(GetCurrentThread(), affinity) != 0)
		return;
#endif

	return;
}
