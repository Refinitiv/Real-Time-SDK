/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslBindThread.h"
#include "rtr/rsslErrors.h"
#include "rtr/rtratomic.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#endif

#include "rtr/bindthread.h"

// This is used for Linux CPU information
typedef struct {
	unsigned int cpuId;			// OS defined id of the CPU, from /sys/devices/system/cpu/cpu<CPU Number>
	unsigned int online;			// 1 if CPU is online, 0 if not.
	unsigned int pkgId;			// OS defined package Id of the CPU, from /sys/devices/system/cpu/cpu%d/topology/physical_package_id
	unsigned int coreId;			// OS defined core id of the CPU, from /sys/devices/system/cpu/cpu%d/topology/core_id
	unsigned int threadId;		// This is a calculated value, based on the number of matching pkgId and coreId CPUs are present in /sys/devices/system/cpu/
	// So for example, if cpu3 id has pkgId 1 and coreId 2, and cpu4 has pkgId1 and coreId 2, cpu3 will have a threadId of 0, and cpu4 will have a threadId of 1
} RsslPkgCoreInfo;

/* CPU topology data structure */
typedef struct {
	RsslUInt32	logicalCpuCount;   // the number of logical processors
#ifdef WIN32
	GLKTSN_T* cpu_topology_ptr;  // full CPU topology describers, used only for Windows
#else
	RsslPkgCoreInfo* cpuInfoArray;	// Array of RsslPkgCoreInfo describing all CPUs.  Size is determined by logicalCpuCount.  This is used only for Linux.
#endif

} RsslCPUTopology;

// Allows to print debug information
#undef _DUMP_DEBUG_

// Defined for Linux.
#ifndef MAX_CPUS_ARRAY
#define MAX_CPUS_ARRAY 1024
#endif

static RsslCPUTopology rsslCPUTopology = { 0, NULL };
static rtr_atomic_val initializedCpuTopology = 0L;

// This function reads an integer value from a file at the given path and stores it in the provided value pointer. It returns 1 on success and 0 on failure.
static int _readIntFromFile(const char* path, int* value)
{
	FILE* fp = fopen(path, "r");
	if (!fp)
		return 0;

	if (fscanf(fp, "%d", value) != 1)
	{
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return 1;
}

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
	if (cpuId > 0 || (cpuId == 0 && pEnd && (pEnd - cpuString) > 0))
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

RSSL_API RsslRet checkCpuIdInitializationError(RsslErrorInfo* pError)
{
	RsslErrorInfo* pInitCpuIdLibError = NULL;

#ifdef WIN32
	pInitCpuIdLibError = getErrorInitializationStage();
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
#else
	if (initializedCpuTopology == 0 || rsslCPUTopology.cpuInfoArray == NULL)
	{
		rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__,
			"Cpu topology information is unavailable.");
		return RSSL_RET_FAILURE;
	}
#endif

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
		// THis is the case where the string is just a numeric CPU Id value.  
		// rsslBindThreadInitialize() is optional in this case, so if it has not been called, do not check to see if the CPU is online prior to binding.
		if (currentId < lcl_maxcpu)
		{
			int isOnline = 1;
			if (checkCpuIdInitializationError(pError) == RSSL_RET_SUCCESS)
			{
#ifdef WIN32
				if (rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[currentId].offline)  // the fields offline was removed.
				{
					isOnline = 0;
				}
#else
				isOnline = isProcessorCoreOnline(currentId);
#endif
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
		// This is a C: P: T: formatted string, so iterate through the list of cached logical processor information and attempt to match the requested values.
		// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
		if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
#ifdef WIN32
		/* Iterate through the processor list to find matches */
		for (i = 0; i < lcl_maxcpu; i++)
		{
			// Don't check this logical id if its currently offline or
			// unavailable.
			if (rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].offline)  // the fields offline was removed.
				continue;
			if (rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].packageORD != procId)
				continue;

			if (!(coreId == -1 || rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].coreORD == coreId))
				continue;

			if (!(threadId == -1 || rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].threadORD == threadId))
				continue;

			// This is a match, add it to the list of logical processors to bind to and break out of for loop.
			idArray[*idCount] = i;
			*idCount = *idCount + 1;
		}
#else

		for (i = 0; i < (int)lcl_maxcpu; ++i)
		{

			// Don't check this logical id if its currently offline or
			// unavailable.
			if (rsslCPUTopology.cpuInfoArray[i].online == 0)  // The CPU is offline.
				continue;

			if (rsslCPUTopology.cpuInfoArray[i].pkgId != (unsigned int)procId)
				continue;

			if (!(coreId == -1 || rsslCPUTopology.cpuInfoArray[i].coreId == (unsigned int)coreId))
				continue;

			if (!(threadId == -1 || rsslCPUTopology.cpuInfoArray[i].threadId == (unsigned int)threadId))
				continue;

			// This is a match, add it to the list of logical processors to bind to and break out of for loop.
			idArray[*idCount] = i;
			*idCount = *idCount + 1;
		}
#endif
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
	unsigned i;

	for (i = 0; i < cpuCount; i++)
	{
		RsslUInt idProcessorUnit = cpuIdArray[i];
		if (idProcessorUnit < rsslCPUTopology.logicalCpuCount && idProcessorUnit < MAX_CPUS_ARRAY)
		{
			cpuIdAssign[idProcessorUnit] = 1;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslBindThreadToCpuAssignmentArray(const char* cpuString, RsslUInt8* cpuIdAssign, RsslErrorInfo* pError)
{
	RsslUInt32 lcl_maxcpu = rsslGetNumberOfProcessorCore();
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
	RsslUInt32 lcl_maxcpu = rsslCPUTopology.logicalCpuCount; //rsslGetNumberOfProcessorCore();

	printf("MaxCpu = %u\n\n", lcl_maxcpu);

	RsslErrorInfo* pInitCpuIdLibError = NULL;

#ifdef WIN32
	pInitCpuIdLibError = getErrorInitializationStage();
	if (pInitCpuIdLibError != NULL)
	{
		printf("Error of the initialization stage.\n");
		printf("rsslErrorInfoCode = %d\n", pInitCpuIdLibError->rsslErrorInfoCode);
		printf("errorLocation = {%s}\n", pInitCpuIdLibError->errorLocation);
		printf("rsslError: rsslErrorId = %d, sysError = %u, text = {%s}\n\n",
			pInitCpuIdLibError->rsslError.rsslErrorId, pInitCpuIdLibError->rsslError.sysError, pInitCpuIdLibError->rsslError.text);
		return;
	}

	if (rsslCPUTopology.cpu_topology_ptr == NULL)
	{
		printf("Cpu topology information is unavailable.\n");
	}
	else
	{
		// internal details
		if (pInitCpuIdLibError != NULL)
		{
			printf("EnumeratedPkgCount = %u\n", rsslCPUTopology.cpu_topology_ptr->EnumeratedPkgCount);
			printf("EnumeratedCoreCount = %u\n", rsslCPUTopology.cpu_topology_ptr->EnumeratedCoreCount);
			printf("EnumeratedThreadCount = %u\n\n", rsslCPUTopology.cpu_topology_ptr->EnumeratedThreadCount);
			printf("SMTSelectMask = %u (0x%X)\n", rsslCPUTopology.cpu_topology_ptr->SMTSelectMask, rsslCPUTopology.cpu_topology_ptr->SMTSelectMask);
			printf("PkgSelectMask = %u (0x%X)\n", rsslCPUTopology.cpu_topology_ptr->PkgSelectMask, rsslCPUTopology.cpu_topology_ptr->PkgSelectMask);
			printf("CoreSelectMask = %u (0x%X)\n", rsslCPUTopology.cpu_topology_ptr->CoreSelectMask, rsslCPUTopology.cpu_topology_ptr->CoreSelectMask);
			printf("PkgSelectMaskShift = %u\n", rsslCPUTopology.cpu_topology_ptr->PkgSelectMaskShift);
			printf("SMTMaskWidth = %u\n\n", rsslCPUTopology.cpu_topology_ptr->SMTMaskWidth);
		}

		for (i = 0; i < lcl_maxcpu; i++)
		{
			printf("[%u]  P:%u C:%u T:%u | APICID=%u OrdIndexOAMsk=%u pkg_IDAPIC=%u Core_IDAPIC=%u SMT_IDAPIC=%u online=%c\n",
				i,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].packageORD,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].coreORD,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].threadORD,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].APICID,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].OrdIndexOAMsk,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].pkg_IDAPIC,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].Core_IDAPIC,
				rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].SMT_IDAPIC,
				(rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].offline != 0 ? 'n' : 'y')
			);
		}
	}
#else
	if(initializedCpuTopology == 0)
	{
		printf("rsslBindThreadInitialize has not been called.\n");
		return;
	}

	if (rsslCPUTopology.cpuInfoArray == NULL)
	{
		printf("Cpu topology information is unavailable.\n");
	}
	else
	{
		printf("Processor Count: %u\n", lcl_maxcpu);

		for (i = 0; i < lcl_maxcpu; i++)
		{
			if (rsslCPUTopology.cpuInfoArray[i].online == 0)
			{
				printf("[%u] online=n", i);
			}
			else
			{
				printf("[%u]  P:%u C:%u T:%u | online=y\n",
					i,
					rsslCPUTopology.cpuInfoArray[i].pkgId,
					rsslCPUTopology.cpuInfoArray[i].coreId,
					rsslCPUTopology.cpuInfoArray[i].threadId);
			}
		}
	}

#endif
}

RsslRet printLogicalIds(RsslUInt cpuCount, RsslUInt8* cpuIdAssign, RsslBuffer* pOutputResult)
{
	if (pOutputResult != NULL && pOutputResult->length > 0 && pOutputResult->data != NULL)
	{
		if (cpuCount == 0)
		{
			pOutputResult->data[0] = '\0';
			pOutputResult->length = 0;
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

		/* Get CPU affinity of the thread */
#ifdef WIN32
		initCpuTopologyMutex();

		DWORD_PTR cpuMask = (DWORD_PTR)(-1LL);
		DWORD_PTR oldMask = 0ULL;

		oldMask = SetThreadAffinityMask(GetCurrentThread(), cpuMask);
		if (!oldMask)
		{
			// SetThreadAffinityMask retruns an error
			// If the thread affinity mask requests a processor that is not selected for the process affinity mask,
			// the last error code is ERROR_INVALID_PARAMETER.
		}
#endif

// Set the logical CPU count here.
rsslCPUTopology.logicalCpuCount = rsslGetNumberOfProcessorCore();

#ifdef WIN32
	if (initializeCpuTopology(&rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s> %s\n", rsslErrorInfo.errorLocation, rsslErrorInfo.rsslError.text);

		destroyCpuTopologyMutex();
		RTR_ATOMIC_SET(initializedCpuTopology, 0);
		return RSSL_RET_FAILURE;
	}

	rsslCPUTopology.cpu_topology_ptr = getCpuTopology();
#else
	{
		int i, j;
		rsslCPUTopology.cpuInfoArray = (RsslPkgCoreInfo*)malloc(rsslCPUTopology.logicalCpuCount * sizeof(RsslPkgCoreInfo));
		if (rsslCPUTopology.cpuInfoArray == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "Could not allocate cpuInfoArray\n");
			RTR_ATOMIC_SET(initializedCpuTopology, 0);
			return RSSL_RET_FAILURE;
		}

		memset((void*)rsslCPUTopology.cpuInfoArray, 0, rsslCPUTopology.logicalCpuCount * sizeof(RsslPkgCoreInfo));
		// Iterate through the list of CPUs contained in /sys/devices/system/cpu/.
		// For each one, determine if it is online, and store the associated package, core Id, and calculate the threadId for the that specific CPU.
		// These are all represented by 
		for (i = 0; i < (int)rsslCPUTopology.logicalCpuCount; ++i)
		{
			int pkg = -1;
			int core = -1;
			int threadId = 0;
			int online = 0;
			char path[256];

			rsslCPUTopology.cpuInfoArray[i].cpuId = i;

			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/online", online);
			if (!_readIntFromFile(path, &online))
				rsslCPUTopology.cpuInfoArray[i].online = 1; /* cpu0 or kernels without online file */
			else
			{
				rsslCPUTopology.cpuInfoArray[i].online = online;
				if (online == 0)
					continue;
			}

			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", i);
			if (!_readIntFromFile(path, &pkg))
				continue;

			rsslCPUTopology.cpuInfoArray[i].pkgId = pkg;

			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/core_id", i);
			if (!_readIntFromFile(path, &core))
				continue;

			rsslCPUTopology.cpuInfoArray[i].coreId = core;

			// Determine the threadId for the core.
			threadId = 0;
			for (j = 0; j < i; ++j)
			{
				if (rsslCPUTopology.cpuInfoArray[j].coreId == core && rsslCPUTopology.cpuInfoArray[j].pkgId == pkg)
					++threadId;
			}
			rsslCPUTopology.cpuInfoArray[i].threadId = threadId;
		}
	}
#endif
		

#ifdef _DUMP_DEBUG_
#ifdef WIN32
	if (rsslCPUTopology.cpu_topology_ptr != NULL)
		dumpCpuTopology();
#else
	if(rsslCPUTopology.cpuInfoArray != NULL)
		dumpCpuTopology();
#endif // WIN32
#endif // _DUMP_DEBUG_

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
#endif
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslBindThreadUninitialize()
{
	if (initializedCpuTopology == 1)
	{
#ifdef WIN32
		unInitializeCpuTopology();
		rsslCPUTopology.cpu_topology_ptr = NULL;
#else
		if (rsslCPUTopology.cpuInfoArray != NULL)
			free((void*)rsslCPUTopology.cpuInfoArray);
		rsslCPUTopology.cpuInfoArray = NULL;
#endif
		RTR_ATOMIC_SET(initializedCpuTopology, 0);
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
		// Don't check this logical id if its currently offline or 
		// unavailable.  If rsslCPUTopology.cpuInfoArray is NULL, this was not initialized, but we can still clear the bindings.
		if (rsslCPUTopology.cpuInfoArray != NULL && rsslCPUTopology.cpuInfoArray[i].online == 0)
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
		if (rsslCPUTopology.cpu_topology_ptr != NULL && rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[i].offline)
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
	int len;

	// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
	if (checkCpuIdInitializationError(pError) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (cpuId < 0 || rsslCPUTopology.logicalCpuCount <= (unsigned)cpuId)
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

	#ifdef WIN32
	len = snprintf(pCpuPCTString->data, pCpuPCTString->length, "P:%u C:%u T:%u",
			rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[cpuId].packageORD,
			rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[cpuId].coreORD,
			rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[cpuId].threadORD
			);
	#else
		len = snprintf(pCpuPCTString->data, pCpuPCTString->length, "P:%u C:%u T:%u",
			rsslCPUTopology.cpuInfoArray[cpuId].pkgId,
			rsslCPUTopology.cpuInfoArray[cpuId].coreId,
			rsslCPUTopology.cpuInfoArray[cpuId].threadId
		);
	#endif

	if (len > 0)
		pCpuPCTString->length = (unsigned)len;
	else
		pCpuPCTString->length = 0;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslBool isProcessorCoreOnline(RsslInt32 cpuId)
{
	int online = 0;

	RsslErrorInfo rsslError;

	// When during initialization CpuTopology got an error we can not perform mapping PCT to logical processor unit id
	if (checkCpuIdInitializationError(&rsslError) != RSSL_RET_SUCCESS)
		return RSSL_FALSE;
#ifdef WIN32
	if (0 <= cpuId && (unsigned)cpuId < rsslCPUTopology.logicalCpuCount)
	{
		if (rsslCPUTopology.cpu_topology_ptr->pApicAffOrdMapping[cpuId].offline)
			return RSSL_FALSE;
		return RSSL_TRUE;
	}
#else
	if (0 <= cpuId && (unsigned)cpuId < rsslCPUTopology.logicalCpuCount)
		return rsslCPUTopology.cpuInfoArray[cpuId].online == 1 ? RSSL_TRUE : RSSL_FALSE;
#endif

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
		pLogicalIds->data[0] = '\0';
		pLogicalIds->length = 0;
		return RSSL_RET_FAILURE;
	}

	if (rsslGetLogicalCpuIdsbyPCTImpl(cpuString, pLogicalIds, pError) != RSSL_RET_SUCCESS)
	{
		pLogicalIds->data[0] = '\0';
		pLogicalIds->length = 0;
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API void setTestErrorInitializationFailure()
{
#ifdef WIN32
	setTestErrorInitializationStage();
#else
	// Just clean out everything
	if (rsslCPUTopology.cpuInfoArray != NULL)
		free((void*)rsslCPUTopology.cpuInfoArray);

	rsslCPUTopology.cpuInfoArray = NULL;
	RTR_ATOMIC_SET(initializedCpuTopology, 0);
#endif
}