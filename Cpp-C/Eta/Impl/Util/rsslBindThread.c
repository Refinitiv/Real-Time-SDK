/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslBindThread.h"
#include "rtr/rsslErrors.h"
#include "rtr/rsslThread.h"
#include "rtr/bindthread.h"
#include "rtr/rtratomic.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#endif

// Allows to print debug information
#undef _DUMP_DEBUG_

static RsslCPUTopology rsslCPUTopology;
static rtr_atomic_val initializedCpuTopology = 0L;

RSSL_API RsslUInt32 rsslGetNumberOfProcessorCore(void)
{
	RsslUInt32 nProcessors = 0;
#ifdef WIN32
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	nProcessors = systemInfo.dwNumberOfProcessors;
#else	// Linux
	nProcessors = sysconf(_SC_NPROCESSORS_CONF);
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
	RsslErrorInfo* pInitCpuIdLibError = getErrorInitializationStage();
	if (rsslCPUTopology.cpu_topology_ptr == NULL || pInitCpuIdLibError != NULL)
	{
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
*					The idArray and idCount store the list of logical ids
*					found in this string
*
* Returns:		The number of logical cpus found in this string in idCount, id of requested logical processor unit in idArray.
*			RSSL_RET_SUCCESS when parsed a logical processor unit id successfuly; otherwise return RSSL_RET_FAILURE.
*/
RsslRet parseSingleCpuString(char* cpuString, RsslUInt* idArray, RsslUInt* idCount, RsslErrorInfo* pError)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslUInt32 i;
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount;

	RsslUInt16 procExpected = 0;
	RsslUInt16 coreExpected = 0;
	RsslUInt16 threadExpected = 0;

	RsslInt procId = -1;
	RsslInt coreId = -1;
	RsslInt threadId = -1;
	RsslInt currentId = 0;

	RsslUInt16 foundInteger = 0;
	RsslUInt16 foundChar = 0;
	RsslUInt idCount0 = *idCount;


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
					"Syntax for cpu binding for string (eos) is invalid. (%s)", cpuString);
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
		if (currentId < lcl_maxcpu)
		{
			int isOnline = 1;
			if (checkCpuIdInitializationError(pError) == RSSL_RET_SUCCESS)
			{
				GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
				if (pCpuTopology->pApicAffOrdMapping[currentId].offline)  // the fields offline was removed.
				{
					isOnline = 0;
				}
			}

			if (isOnline)
			{
				idArray[*idCount] = currentId;
				*idCount = *idCount + 1;
			}
		}
	}
	else
	{
		// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
		if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

		GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;

		/* Iterate through the processor list to find matches */
		for (i = 0; i < lcl_maxcpu; i++)
		{
			// Don't check this logical id if its currently offline or
			// unavailable.
			if (pCpuTopology->pApicAffOrdMapping[i].offline)  // the fields offline was removed.
				continue;
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

	ret = (*idCount > idCount0 ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE);

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
RsslRet parseFullCpuString(const char* cpuString, RsslUInt* idArray, RsslUInt* idCount, RsslErrorInfo* pError)
{
	char tempString[MAX_CPU_STRING_LEN];
	char* stringIter = tempString;
	char* currentString = stringIter;

	RsslInt endtoken = 0;

	*idCount = 0;

	if (cpuString[0] == '\0' ||  strcmp(cpuString, "-1") == 0)
	{
		return RSSL_RET_SUCCESS;
	}

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

			if (parseSingleCpuString(currentString, idArray, idCount, pError) != RSSL_RET_SUCCESS)
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

/* Calculate assignments for each logical processor units */
RsslRet convertCpuIdArrayToAssignment(const RsslUInt* cpuIdArray, RsslUInt cpuCount, RsslUInt8* cpuIdAssign)
{
#ifdef _DUMP_DEBUG_
	printf("rsslBindThreadToCpuArray. dump CpuIdArray: ");
	dumpCpuArray(cpuIdArray, cpuCount);
#endif // _DUMP_DEBUG_
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount;
	unsigned i;

	for (i = 0; i < cpuCount; i++)
	{
		RsslUInt idProcessorUnit = cpuIdArray[i];
		if (idProcessorUnit < lcl_maxcpu && idProcessorUnit < MAX_CPUS_ARRAY)
		{
			cpuIdAssign[idProcessorUnit] = 1;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslBindThreadToCpuAssignmentArray(const char* cpuString, RsslUInt8* cpuIdAssign, RsslErrorInfo* pError)
{
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount;
#if defined(Linux) && !defined(x86_Linux_2X)
	RsslUInt i;
	cpu_set_t currentCPUSet;
	CPU_ZERO(&currentCPUSet);
	for (i = 0; i < lcl_maxcpu && i < MAX_CPUS_ARRAY; i++)
	{
		if (cpuIdAssign[i] != 0)
		{
			CPU_SET(i, &currentCPUSet);
		}
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
	RsslUInt64 affinityMask = 0ULL;
	RsslUInt i;

	for (i = 0; i < lcl_maxcpu && i < MAX_CPUS_ARRAY; i++)
	{
		if (cpuIdAssign[i] != 0)
		{
			affinityMask = (affinityMask | (1ULL << i));
		}
	}

	RsslInt32 errorCode = 0;

	if (SetThreadAffinityMask(GetCurrentThread(), affinityMask) == 0)
	{
		errorCode = GetLastError();
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
			"Unable to set processor affinity for cpu mask 0x%llX, cpu configuration %s.  Error code is %d.",
			affinityMask, cpuString, errorCode);
		return RSSL_RET_FAILURE;
	}
#endif
	return RSSL_RET_SUCCESS;
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

RSSL_API void dumpCpuTopology()
{
	RsslUInt32 i;
	RsslUInt32 lcl_maxcpu;

	GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
	lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //getLogicalCpuCount();

	printf("MaxCpu = %u\n\n", lcl_maxcpu);

	RsslErrorInfo* pInitCpuIdLibError = getErrorInitializationStage();
	if (pInitCpuIdLibError != NULL)
	{
		printf("Error of the initialization stage.\n");
		printf("rsslErrorInfoCode = %d\n", pInitCpuIdLibError->rsslErrorInfoCode);
		printf("errorLocation = {%s}\n", pInitCpuIdLibError->errorLocation);
		printf("rsslError: rsslErrorId = %d, sysError = %u, text = {%s}\n\n",
			pInitCpuIdLibError->rsslError.rsslErrorId, pInitCpuIdLibError->rsslError.sysError, pInitCpuIdLibError->rsslError.text);
	}

	if (pCpuTopology == NULL)
	{
		printf("Cpu topology information is unavailable.\n");
	}
	else
	{
		// internal details
		if (pInitCpuIdLibError != NULL)
		{
			printf("EnumeratedPkgCount = %u\n", pCpuTopology->EnumeratedPkgCount);
			printf("EnumeratedCoreCount = %u\n", pCpuTopology->EnumeratedCoreCount);
			printf("EnumeratedThreadCount = %u\n\n", pCpuTopology->EnumeratedThreadCount);
			printf("SMTSelectMask = %u (0x%X)\n", pCpuTopology->SMTSelectMask, pCpuTopology->SMTSelectMask);
			printf("PkgSelectMask = %u (0x%X)\n", pCpuTopology->PkgSelectMask, pCpuTopology->PkgSelectMask);
			printf("CoreSelectMask = %u (0x%X)\n", pCpuTopology->CoreSelectMask, pCpuTopology->CoreSelectMask);
			printf("PkgSelectMaskShift = %u\n", pCpuTopology->PkgSelectMaskShift);
			printf("SMTMaskWidth = %u\n\n", pCpuTopology->SMTMaskWidth);
		}

		for (i = 0; i < lcl_maxcpu; i++)
		{
			printf("[%u]  P:%u C:%u T:%u | APICID=%u OrdIndexOAMsk=%u pkg_IDAPIC=%u Core_IDAPIC=%u SMT_IDAPIC=%u online=%c\n",
				i,
				pCpuTopology->pApicAffOrdMapping[i].packageORD,
				pCpuTopology->pApicAffOrdMapping[i].coreORD,
				pCpuTopology->pApicAffOrdMapping[i].threadORD,
				pCpuTopology->pApicAffOrdMapping[i].APICID,
				pCpuTopology->pApicAffOrdMapping[i].OrdIndexOAMsk,
				pCpuTopology->pApicAffOrdMapping[i].pkg_IDAPIC,
				pCpuTopology->pApicAffOrdMapping[i].Core_IDAPIC,
				pCpuTopology->pApicAffOrdMapping[i].SMT_IDAPIC,
				(pCpuTopology->pApicAffOrdMapping[i].offline != 0 ? 'n' : 'y')
			);
		}
	}
}

RsslRet printLogicalIds(RsslUInt cpuCount, RsslUInt8* cpuIdAssign, RsslBuffer* pOutputResult)
{
	if (pOutputResult != NULL && pOutputResult->length > 0 && pOutputResult->data != NULL)
	{
		if (cpuCount == 0)
		{
			pOutputResult->length = snprintf(pOutputResult->data, pOutputResult->length, "");
			return RSSL_RET_SUCCESS;
		}

		RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount;
		int bytes = 0;
		int n;
		RsslUInt i;
		RsslUInt iCpu = 0;
		RsslRet ret = RSSL_RET_SUCCESS;

		for (i = 0; i < lcl_maxcpu && bytes < (int)pOutputResult->length && i < MAX_CPUS_ARRAY; ++i)
		{
			if (cpuIdAssign[i] != 0)  // Is the CPU core assigned?
			{
				if (iCpu > 0)
				{
					if (pOutputResult->length - bytes < 2)
					{
						ret = RSSL_RET_FAILURE;
						break;
					}
					bytes += snprintf(pOutputResult->data + bytes, pOutputResult->length - bytes, ",");
				}

				if (pOutputResult->length - bytes < 6)
				{
					n = snprintf(NULL, 0, "%llu", i);
					if ((int)pOutputResult->length - bytes < (n + 1))
					{
						ret = RSSL_RET_FAILURE;
						break;
					}
				}

				bytes += snprintf(pOutputResult->data + bytes, pOutputResult->length - bytes, "%llu", i);
				++iCpu;
			}
		}

		pOutputResult->length = bytes;
		return ret;
	}

	return RSSL_RET_FAILURE;
}

RsslRet rsslBindThreadWithString(const char* cpuString, RsslBuffer* pOutputResult, RsslErrorInfo* pError)
{
	RsslUInt cpuCount = 0;
	RsslUInt cpuIdArray[MAX_CPUS_ARRAY]; // array of logical processor unit ids: result of parsing cpuString

	RsslUInt8 cpuIdAssign[MAX_CPUS_ARRAY];  // For each logical processor unit: does it have a thread assignment True(1) / False(0)

	memset((void*)cpuIdAssign, 0, sizeof(cpuIdAssign));

	if (parseFullCpuString(cpuString, cpuIdArray, &cpuCount, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (cpuCount > 0)
	{
		if (convertCpuIdArrayToAssignment(cpuIdArray, cpuCount, cpuIdAssign) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

		if (rsslBindThreadToCpuAssignmentArray(cpuString, cpuIdAssign, pError) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	// on Success, print the list of logical core id that were bound for the calling thread
	printLogicalIds(cpuCount, cpuIdAssign, pOutputResult);

	return RSSL_RET_SUCCESS;
}

RsslRet rsslBindThreadImpl(const char* cpuString, RsslBuffer* pOutputResult, RsslErrorInfo* pError)
{
	char* pEnd = NULL;
	RsslBool fillOutputResult = RSSL_FALSE;

	if (pOutputResult != NULL && pOutputResult->length > 0 && pOutputResult->data != NULL)
	{
		fillOutputResult = RSSL_TRUE;
		*pOutputResult->data = '\0';
	}

	if (!cpuString)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"cpuString is not set.");
		if (fillOutputResult == RSSL_TRUE)
			pOutputResult->length = snprintf(pOutputResult->data, pOutputResult->length, "%s", pError->rsslError.text);
		return RSSL_RET_FAILURE;
	}

	/* Parse cpuString - specifies Cpu core in string format P:X C:Y T:Z or logical core id. */
	if (rsslBindThreadWithString(cpuString, pOutputResult, pError) != RSSL_RET_SUCCESS)
	{
		if (fillOutputResult == RSSL_TRUE)
			pOutputResult->length = snprintf(pOutputResult->data, pOutputResult->length, "%s", pError->rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThread(const char* cpuString, RsslErrorInfo* pError)
{
	return rsslBindThreadImpl(cpuString, (RsslBuffer*)NULL, pError);
}

RSSL_API RsslRet rsslBindThreadEx(const char* cpuString, RsslBuffer* outputResult, RsslErrorInfo* pError)
{
	return rsslBindThreadImpl(cpuString, outputResult, pError);
}

RSSL_API RsslRet rsslBindThreadInitialize(RsslError* error)
{
	if (!initializedCpuTopology)
	{
		RsslErrorInfo rsslErrorInfo;

		RTR_ATOMIC_SET(initializedCpuTopology, 1);

		memset((void*)&rsslErrorInfo, 0, sizeof(RsslErrorInfo));

		initCpuTopologyMutex();

		/* Get CPU affinity of the thread */
#ifdef WIN32
		DWORD_PTR cpuMask = (DWORD_PTR)(-1LL);
		DWORD_PTR oldMask = 0ULL;

		oldMask = SetThreadAffinityMask(GetCurrentThread(), cpuMask);
		if (!oldMask)
		{
			// SetThreadAffinityMask retruns an error
			// If the thread affinity mask requests a processor that is not selected for the process affinity mask,
			// the last error code is ERROR_INVALID_PARAMETER.
		}
#else	// Linux
		RsslBool restoreAffinity = RSSL_TRUE;
		cpu_set_t cpuSet;
		CPU_ZERO(&cpuSet);

		if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) != 0)
		{
			// pthread_getaffinity_np returns an error so don't restore the process affinity mask.
			restoreAffinity = RSSL_FALSE;
		}
#endif

		if (initializeCpuTopology(&rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
#if defined(__GNUC__) && (__GNUC__ >= 9)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s> %s\n", rsslErrorInfo.errorLocation, rsslErrorInfo.rsslError.text);
#if defined(__GNUC__) && (__GNUC__ >= 9)
	#pragma GCC diagnostic pop
#endif

			destroyCpuTopologyMutex();
			RTR_ATOMIC_SET(initializedCpuTopology, 0);
			return RSSL_RET_FAILURE;
		}

		rsslCPUTopology.cpu_topology_ptr = getCpuTopology();
		rsslCPUTopology.logicalCpuCount = getLogicalCpuCount();

#ifdef _DUMP_DEBUG_
		if (rsslCPUTopology.cpu_topology_ptr != NULL)
			dumpCpuTopology();
#endif

		/* Restore CPU affinity for the thread */
#ifdef WIN32
		if (oldMask != 0)
		{
			cpuMask = SetThreadAffinityMask(GetCurrentThread(), oldMask);
			if (!cpuMask)
			{
				RsslInt32 errorCode = GetLastError();
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,
					"<%s:%d> rsslBindThreadInitialize() Unable to restore processor affinity for cpu mask 0x%llX.\n", __FILE__, __LINE__, oldMask);

				unInitializeCpuTopology();
				destroyCpuTopologyMutex();
				rsslCPUTopology.cpu_topology_ptr = NULL;
				RTR_ATOMIC_SET(initializedCpuTopology, 0);
				return RSSL_RET_FAILURE;
			}
		}
#else	// Linux
		if ( (restoreAffinity == RSSL_TRUE) && (sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet) < 0) )
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,
				"<%s:%d> rsslBindThreadInitialize() Unable to restore processor affinity.\n", __FILE__, __LINE__);

			unInitializeCpuTopology();
			destroyCpuTopologyMutex();
			rsslCPUTopology.cpu_topology_ptr = NULL;
			RTR_ATOMIC_SET(initializedCpuTopology, 0);
			return RSSL_RET_FAILURE;
		}
#endif
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThreadUninitialize()
{
	if (initializedCpuTopology)
	{
		unInitializeCpuTopology();
		destroyCpuTopologyMutex();
		rsslCPUTopology.cpu_topology_ptr = NULL;
		RTR_ATOMIC_SET(initializedCpuTopology, 0);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API void rsslClearBindings()
{
	int lcl_maxcpu = rsslGetNumberOfProcessorCore();
	unsigned long mask = 0;
	int i;
	GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
#if defined(Linux) && !defined(x86_Linux_2X)
	cpu_set_t currentCPU;
	CPU_ZERO(&currentCPU);
	for (i = 0; i < lcl_maxcpu; i++)
	{
		// Don't check this logical id if its currently offline or 
		// unavailable. 
		if (pCpuTopology != NULL && pCpuTopology->pApicAffOrdMapping[i].offline)
			continue;
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
		// Don't check this logical id if its currently offline or 
		// unavailable. 
		if (pCpuTopology != NULL && pCpuTopology->pApicAffOrdMapping[i].offline)
			continue;
		affinity = affinity | (DWORD_PTR)(1ULL << i);
	}
	if (SetThreadAffinityMask(GetCurrentThread(), affinity) != 0)
		return;
#endif

	return;
}

RsslRet getPCTByProcessorCoreNumber(RsslInt32 cpuId, RsslBuffer* pCpuPCTString, RsslErrorInfo* pError)
{
	GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //getLogicalCpuCount();
	int len;

	// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
	if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (cpuId < 0 || lcl_maxcpu <= (unsigned)cpuId)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"Configuration setting %d did not match any physical processors on the system.", cpuId);
		return RSSL_RET_FAILURE;
	}

	if (pCpuPCTString == NULL || pCpuPCTString->data == NULL || pCpuPCTString->length == 0)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"pCpuPCTString is not set.");
		return RSSL_RET_FAILURE;
	}

	len = snprintf(pCpuPCTString->data, pCpuPCTString->length, "P:%u C:%u T:%u",
			pCpuTopology->pApicAffOrdMapping[cpuId].packageORD,
			pCpuTopology->pApicAffOrdMapping[cpuId].coreORD,
			pCpuTopology->pApicAffOrdMapping[cpuId].threadORD
			);

	if (len > 0)
		pCpuPCTString->length = (unsigned)len;
	else
		pCpuPCTString->length = 0;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslBool isProcessorCoreOnline(RsslInt32 cpuId)
{
	GLKTSN_T* pCpuTopology = rsslCPUTopology.cpu_topology_ptr;
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //getLogicalCpuCount();

	RsslErrorInfo rsslError;

	// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
	if (checkCpuIdInitializationError(&rsslError) != RSSL_RET_SUCCESS)
		return RSSL_FALSE;

	if (0 <= cpuId && (unsigned)cpuId < lcl_maxcpu)
	{
		if (pCpuTopology->pApicAffOrdMapping[cpuId].offline)
			return RSSL_FALSE;
		return RSSL_TRUE;
	}

	return RSSL_FALSE;
}

RsslRet rsslGetLogicalCpuIdsbyPCTImpl(const char* cpuString, RsslBuffer* pLogicalIds, RsslErrorInfo* pError)
{
	RsslUInt cpuCount = 0;
	RsslUInt cpuIdArray[MAX_CPUS_ARRAY]; // array of logical processor unit ids: result of parsing cpuString
	RsslUInt8 cpuIdAssign[MAX_CPUS_ARRAY];  // For each logical processor unit: does it have a thread assignment True(1) / False(0)

	memset((void*)cpuIdAssign, 0, sizeof(cpuIdAssign));

	if (parseFullCpuString(cpuString, cpuIdArray, &cpuCount, pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (convertCpuIdArrayToAssignment(cpuIdArray, cpuCount, cpuIdAssign) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	// on Success, print the list of logical core id
	if (printLogicalIds(cpuCount, cpuIdAssign, pLogicalIds) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"pLogicalIds buffer length is not sufficient for the CPU logical Ids.");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslGetLogicalCpuIdsbyPCT(const char* cpuString, RsslBuffer* pLogicalIds, RsslErrorInfo* pError)
{
	if (pLogicalIds == NULL || pLogicalIds->length == 0 || pLogicalIds->data == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"pLogicalIds buffer is not provided.");
		return RSSL_RET_FAILURE;
	}

	if (!cpuString)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"cpuString is not set.");
		pLogicalIds->length = snprintf(pLogicalIds->data, pLogicalIds->length, "");
		return RSSL_RET_FAILURE;
	}

	if (rsslGetLogicalCpuIdsbyPCTImpl(cpuString, pLogicalIds, pError) != RSSL_RET_SUCCESS)
	{
		pLogicalIds->length = snprintf(pLogicalIds->data, pLogicalIds->length, "");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}
