/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2022-2024 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
*	 Bind Cpu core for thread Unit Tests
*
*   Unit testing for the ETA Transport.
*   Includes multithreaded tests.
*
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>
#include <mutex>

#include "gtest/gtest.h"

#include "rtr/rsslBindThread.h"
#include "rtr/rsslThread.h"
#include "rtr/bindthread.h"

#if defined(_WIN32)
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h> 
#include <pthread.h>
#include <signal.h>
#endif

extern void time_sleep(int millisec);

static bool threadBreak = false;

const unsigned MAXLEN = 32;

const char* STR_ERROR_CPU_IS_NOT_SET = "cpuString is not set.";
const char* STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE = "Cpu topology information is unavailable.";
const char* STR_ERROR_CPU_TOPOLOGY_TEST = "CpuId topology error: This is a test error.";

const unsigned MAX_TEST_CORES = 1024;

typedef struct {
	unsigned cpuIdOnline[MAX_TEST_CORES];      // list of online processor units id
	std::string cpuPCTOnline[MAX_TEST_CORES];  // list of online processor units PCT
	unsigned nCpuOnline;
} ProcessorUnitOnlineInfo;

static ProcessorUnitOnlineInfo processorsOnline;

typedef struct {
	const char* strCpuCore;
} CpuCoreArg;

static std::mutex mutexInitProcessorUnitInfo;

void initProcessorUnitOnlineInfo()
{
	if (processorsOnline.nCpuOnline == 0)
	{
		mutexInitProcessorUnitInfo.lock();
		
		if (processorsOnline.nCpuOnline == 0)
		{
			RsslErrorInfo errorInfo;

			memset((void*)&errorInfo, 0, sizeof(RsslErrorInfo));
			if (rsslBindThreadInitialize(&errorInfo.rsslError) == RSSL_RET_SUCCESS)
			{
				RsslUInt32 nProcessors = 0;
				RsslUInt32 nProcessorsOnline = 0;
				char pctText[32];
				RsslBuffer procUnitPCT = { sizeof(pctText), pctText };

				RsslErrorInfo rsslErrorInfo;

#ifdef WIN32
				SYSTEM_INFO systemInfo;
				GetSystemInfo(&systemInfo);
				nProcessors = systemInfo.dwNumberOfProcessors;
#else	// Linux
				nProcessors = sysconf(_SC_NPROCESSORS_CONF);  // all configured processor units: online + offline
#endif
				for (RsslUInt32 cpuId = 0; cpuId < nProcessors && cpuId < MAX_TEST_CORES; ++cpuId)
				{
					if ( isProcessorCoreOnline(cpuId) == RSSL_TRUE )
					{
						*pctText = '\0';
						procUnitPCT.data = pctText;
						procUnitPCT.length = sizeof(pctText);
						if ( getPCTByProcessorCoreNumber(cpuId, &procUnitPCT, &rsslErrorInfo) != RSSL_RET_SUCCESS )
						{
							continue;
						}

						processorsOnline.cpuPCTOnline[nProcessorsOnline].assign(procUnitPCT.data);
						processorsOnline.cpuIdOnline[nProcessorsOnline] = cpuId;

						nProcessorsOnline++;
					}
				}
				processorsOnline.nCpuOnline = nProcessorsOnline;
			}

			rsslClearBindings();
			rsslBindThreadUninitialize();

			time_sleep(50);
		}

		mutexInitProcessorUnitInfo.unlock();
	}
}

RSSL_THREAD_DECLARE(runBindProcessTestThread, pArg)
{
	RsslInt32 cpuId = -1;
	RsslErrorInfo errorInfoThread;

	if (pArg != 0)
		cpuId = *((RsslInt32*)pArg);

	if (cpuId >= 0)
	{
		if (rsslBindProcessorCoreThread(cpuId, &errorInfoThread) != RSSL_RET_SUCCESS)
		{
			return RSSL_THREAD_RETURN();
		}
	}

	while (!threadBreak)
	{
		time_sleep(100);
	}

	return RSSL_THREAD_RETURN();
}

RSSL_THREAD_DECLARE(runBindProcessTestThreadStr, pArg)
{
	CpuCoreArg* cpuCoreArgs = NULL;
	const char* cpuString = NULL;
	RsslErrorInfo errorInfoThread;
	RsslRet ret;

	if (pArg != 0)
		cpuCoreArgs = ((CpuCoreArg*)pArg);

	if (cpuCoreArgs != NULL && cpuCoreArgs->strCpuCore != NULL)
	{
		cpuString = cpuCoreArgs->strCpuCore;
		
		ret = rsslBindThread(cpuString, &errorInfoThread);

		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("runBindProcessTestThreadStr Binding success: (%s)\n", cpuString);
		}
		else
		{
			printf("runBindProcessTestThreadStr error: binding fail. (%s)\n", cpuString);
			return RSSL_THREAD_RETURN();
		}
	}
	else
	{
		printf("runBindProcessTestThreadStr error: bad argument.\n");
		return RSSL_THREAD_RETURN();
	}

	while (!threadBreak)
	{
		time_sleep(100);
	}

	return RSSL_THREAD_RETURN();
}

bool isThreadAffinityIncludeCpuId(RsslThreadId rsslThread, RsslInt32 CpuCoreId)
{
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR oldMask = 0;

	// This test has a potential post-effect under Windows.
	// SetThreadAffinityMask is able to bind other Cpu core for the thread
	// when cpuMask is different to oldMask.
	// Therefore, the thread will reschedule to other Cpu core.
	// Be careful with it on real application.
	oldMask = SetThreadAffinityMask(rsslThread.handle, cpuMask);
	if (oldMask)
	{
		if (oldMask != cpuMask)
			SetThreadAffinityMask(rsslThread.handle, oldMask); // restore original
		//printf("isThreadAffinityIncludeCpuId. CpuCoreId=%d. oldMask=%llu, cpuMask=%llu\n", CpuCoreId, oldMask, cpuMask);
		return ((oldMask & cpuMask) != 0);
	}

#else	// Linux
	cpu_set_t cpuSet;
	bool res = false;
	CPU_ZERO(&cpuSet);

	if (pthread_getaffinity_np(rsslThread, sizeof(cpu_set_t), &cpuSet) == 0)
	{
		res = (CPU_ISSET(CpuCoreId, &cpuSet));

		//long i;
		//long nproc = sysconf(_SC_NPROCESSORS_CONF);
		//printf("pthread_getaffinity_np = ");
		//for (i = 0; i < nproc; i++) {
		//	printf("%d ", CPU_ISSET(i, &cpuSet));
		//}
		//printf("\n");

		return res;
	}

#endif
	return false;
}

class ThreadBindProcessorCoreTest : public ::testing::Test {
protected:
	RsslErrorInfo errorInfo;

	virtual void SetUp()
	{
		initProcessorUnitOnlineInfo();
		ASSERT_TRUE(processorsOnline.nCpuOnline > 0);

		memset((void*)&errorInfo, 0, sizeof(RsslErrorInfo));
		ASSERT_EQ(rsslBindThreadInitialize(&errorInfo.rsslError), RSSL_RET_SUCCESS);
	}

	virtual void TearDown()
	{
		rsslClearBindings();
		rsslBindThreadUninitialize();
	}
};

TEST_F(ThreadBindProcessorCoreTest, GetNumberOfProcessorCoreShouldReturnPositive)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	ASSERT_TRUE(nProcessors > 0);
}

TEST_F(ThreadBindProcessorCoreTest, IsProcessorCoreNumberValidTest)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();

	// Negative value - should return False.
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(-1), RSSL_FALSE);
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(-12), RSSL_FALSE);

	// Minimum value - should return True.
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(0), RSSL_TRUE);
	// Maximum value - should return True.
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(nProcessors - 1), RSSL_TRUE);

	// More then maximum - should return False.
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(nProcessors), RSSL_FALSE);
	// More then maximum - should return False.
	ASSERT_EQ(rsslIsProcessorCoreNumberValid(nProcessors+1000), RSSL_FALSE);
}

TEST_F(ThreadBindProcessorCoreTest, rsslIsStrProcessorCoreBindValidTest)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	const char* pStrNull = NULL;

	// Null string - should return False.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(pStrNull), RSSL_FALSE);

	// Empty strings - should return False.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(""), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(" "), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("     "), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("\n"), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("\t"), RSSL_FALSE);

	// Negative value - should return False.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("-1"), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("-12"), RSSL_FALSE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(" -12"), RSSL_FALSE);

	// Minimum value - should return True.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid("0"), RSSL_TRUE);
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(" 0"), RSSL_TRUE);
	
	// Maximum value - should return True.
	char sValue[32];
	snprintf(sValue, sizeof(sValue), "%u", (nProcessors - 1));
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), " %u", (nProcessors - 1));
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);

	// More then maximum - should return False.
	snprintf(sValue, sizeof(sValue), "%u", (nProcessors));
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_FALSE);

	// More then maximum - should return False.
	snprintf(sValue, sizeof(sValue), "%u", (nProcessors + 1000));
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_FALSE);

	// Valid string, PCT format - should return True.
	snprintf(sValue, sizeof(sValue), "P:0 C:0 T:0");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), "\nP:0 C:0 T:0");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), "\tP:0 C:0 T:0");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), "    P:0    C:0    T:0");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), "P:0 C:0 T:0   ");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);
	snprintf(sValue, sizeof(sValue), "\nP:0\tC:0\nT:0   ");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_TRUE);

	// Not valid strings - should return False.
	snprintf(sValue, sizeof(sValue), "K:3");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_FALSE);

	snprintf(sValue, sizeof(sValue), "#");
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sValue), RSSL_FALSE);
}

TEST_F(ThreadBindProcessorCoreTest, rsslIsStrProcessorCoreBindValidMaxLengthTest)
{
	const char baseABC[] = "abcdefghijklmnoprstuvwxyzABCDEFGHIJKLMNOPRSTUVWXYZ";
	const size_t lenBase = strlen(baseABC) / sizeof(char);

	char sLong[(MAX_CPU_STRING_LEN + 128U)];
	const size_t lenLongStr = sizeof(sLong) / sizeof(char);

	for (unsigned i = 0; i < lenLongStr - 1; ++i)
	{
		sLong[i] = baseABC[i % lenBase];
	}
	sLong[lenLongStr - 1] = '\0';

	// A long string - should return False.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(sLong), RSSL_FALSE);

	// baseABC is a non-PCT format - should return False.
	ASSERT_EQ(rsslIsStrProcessorCoreBindValid(baseABC), RSSL_FALSE);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCoreTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslInt32 CpuCoreId = processorsOnline.cpuIdOnline[1];

	// Setup stage
	ASSERT_EQ(rsslBindProcessorCoreThread(CpuCoreId, &errorInfo), RSSL_RET_SUCCESS);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId));
#endif
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCore_BindMainThenThread_Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[0];
	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[1];

	RsslThreadId rsslThreadId;

	// Initialization stage
	threadBreak = false;

	ASSERT_EQ(rsslBindProcessorCoreThread(cpuIdMainThread, &errorInfo), RSSL_RET_SUCCESS);

	time_sleep(50);

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThread, &cpuIdThread);

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCore_BindThreadThenMain_Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[1];
	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[0];

	RsslThreadId rsslThreadId;

	// Initialization stage
	threadBreak = false;

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThread, &cpuIdThread);

	time_sleep(50);

	ASSERT_EQ(rsslBindProcessorCoreThread(cpuIdMainThread, &errorInfo), RSSL_RET_SUCCESS);

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToUnavailableCpuCoreFailTest)
{
	RsslUInt32 nProcessors = 0;
	RsslInt32 CpuCoreId = -1;

	// Setup stage
#ifdef WIN32
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	nProcessors = systemInfo.dwNumberOfProcessors;
#else	// Linux
	nProcessors = sysconf(_SC_NPROCESSORS_CONF);
#endif

	CpuCoreId = nProcessors + 1;

	//printf("nProcessors=%u, CpuCoreId=%d\n", nProcessors, CpuCoreId);

	// Test stage
	// Try to bind an unavailable processor core
	// Should return Fail: not RSSL_RET_SUCCESS.
	ASSERT_NE(rsslBindProcessorCoreThread(CpuCoreId, &errorInfo), RSSL_RET_SUCCESS);
}

TEST_F(ThreadBindProcessorCoreTest, BindThreadShouldReturnFailOnNullCoreStringTest)
{
	char* pCoreStr = NULL;

	// Setup stage
	ASSERT_EQ(rsslBindThread(pCoreStr, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_IS_NOT_SET);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCore0PCTTest)
{
	// "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[0].c_str();
	RsslInt32 CpuCoreId = processorsOnline.cpuIdOnline[0];

	// Setup stage
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId));
#endif
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCore1PCTTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// P:X C:Y T:Z
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[1].c_str();
	RsslInt32 CpuCoreId = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	// Setup stage
	ret = rsslBindThread(sCpuCorePCT, &errorInfo);
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	
	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId));
#endif
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToUnavailableCpuCorePCTFailTest)
{
	RsslUInt32 nProcessors = 0;
	RsslInt32 CpuCoreId = -1;
	char sCpuCorePCT[MAXLEN];

	// Setup stage
#ifdef WIN32
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	nProcessors = systemInfo.dwNumberOfProcessors;
#else	// Linux
	nProcessors = sysconf(_SC_NPROCESSORS_CONF);
#endif

	CpuCoreId = nProcessors + 1;

	snprintf(sCpuCorePCT, sizeof(sCpuCorePCT), "P:0 C:%u T:0", CpuCoreId);
	//printf("nProcessors=%u, CpuCoreId=%d, PCT={%s}\n", nProcessors, CpuCoreId, sCpuCorePCT);

	// Test stage
	// Try to bind an unavailable processor core
	// Should return Fail: Not RSSL_RET_SUCCESS.
	ASSERT_NE(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_SUCCESS);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCorePCT_BindMainThenThread_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	const char* cpuMainThreadPCT = processorsOnline.cpuPCTOnline[0].c_str(); //"P:0 C:0 T:0";
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[1].c_str();

	CpuCoreArg threadPCTArg = { sCpuCorePCT };

	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[0];
	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[1];

	RsslThreadId rsslThreadId;

	// Initialization stage
	threadBreak = false;

	ASSERT_EQ(rsslBindThread(cpuMainThreadPCT, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	time_sleep(50);

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCorePCT_BindThreadThenMain_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// "P:0 C:0 T:0";
	const char* cpuIdThreadPCT = processorsOnline.cpuPCTOnline[0].c_str();
	const char* cpuIdMainThreadPCT = processorsOnline.cpuPCTOnline[1].c_str();

	CpuCoreArg threadPCTArg = { cpuIdThreadPCT };

	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[0];
	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[1];

	RsslThreadId rsslThreadId;
	RsslRet ret;

	// Initialization stage
	threadBreak = false;

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	ret = rsslBindThread(cpuIdMainThreadPCT, &errorInfo);
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCoreCombinePCT_BindMainThenThread_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char cpuIdMainThreadPCT[MAXLEN] = "0";
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[1].c_str();

	CpuCoreArg threadPCTArg = { sCpuCorePCT };

	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[0];
	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[1];

	RsslThreadId rsslThreadId;

	// Initialization stage
	threadBreak = false;

	snprintf(cpuIdMainThreadPCT, sizeof(cpuIdMainThreadPCT), "%d", cpuIdMainThread);

	ASSERT_EQ(rsslBindThread(cpuIdMainThreadPCT, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	time_sleep(50);

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCoreCombinePCT_BindThreadThenMain_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char cpuIdThreadPCT[MAXLEN] = "0";

	const char* sCpuMainCorePCT = processorsOnline.cpuPCTOnline[1].c_str();

	CpuCoreArg threadPCTArg = { cpuIdThreadPCT };

	RsslInt32 cpuIdMainThread = processorsOnline.cpuIdOnline[1];
	RsslInt32 cpuIdThread = processorsOnline.cpuIdOnline[0];

	RsslThreadId rsslThreadId;
	RsslRet ret;

	// Initialization stage
	threadBreak = false;

	snprintf(cpuIdThreadPCT, sizeof(cpuIdThreadPCT), "%d", cpuIdThread);

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	ret = rsslBindThread(sCpuMainCorePCT, &errorInfo);
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	time_sleep(50);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadIdMain = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadIdMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, cpuIdMainThread));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, cpuIdThread));

#endif

	// Stop
	threadBreak = true;
	RSSL_THREAD_JOIN(rsslThreadId);
}

///

TEST_F(ThreadBindProcessorCoreTest, BindThreadExShouldReturnFailOnNullCoreStringTest)
{
	char* pCoreStr = NULL;
	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(pCoreStr, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_IS_NOT_SET);

	ASSERT_STREQ(outputResult.data, STR_ERROR_CPU_IS_NOT_SET);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToCpuCore0PCTTest)
{
	// "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[0].c_str();
	RsslInt32 CpuCoreId = processorsOnline.cpuIdOnline[0];
	char outStr[MAXLEN] = "0";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	snprintf(outStr, sizeof(outStr), "%d", CpuCoreId);

	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToCpuCore1PCTTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// P:X C:Y T:Z
	const char* sCpuCorePCT = processorsOnline.cpuPCTOnline[1].c_str();
	RsslInt32 CpuCoreId = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(outStr, sizeof(outStr), "%d", CpuCoreId);

	// Setup stage
	ret = rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo);
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = 1ULL << CpuCoreId;
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char listCpuIdThread[MAXLEN] = "0,1";
	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "%d,%d", CpuCoreId0, CpuCoreId1);
	snprintf(outStr, sizeof(outStr), "%d,%d", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCTTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str(); // "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore1PCT = processorsOnline.cpuPCTOnline[1].c_str();

	char bufCpuIdThreadsPCT[128];

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCore0PCT, sCpuCore1PCT);
	snprintf(outStr, sizeof(outStr), "%d,%d", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCTElementsRepeat0Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str(); // "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore1PCT = processorsOnline.cpuPCTOnline[1].c_str();

	char bufCpuIdThreadsPCT[256];

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s, %s, %s, %s, %s, %s", sCpuCore0PCT, sCpuCore1PCT, sCpuCore0PCT, sCpuCore1PCT, sCpuCore0PCT, sCpuCore1PCT, sCpuCore1PCT);
	snprintf(outStr, sizeof(outStr), "%d,%d", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCTElementsRepeat1Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str(); // "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore1PCT = processorsOnline.cpuPCTOnline[1].c_str();

	char bufCpuIdThreadsPCT[256];

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s, %s, %s, %s, %s, %s", sCpuCore1PCT, sCpuCore0PCT, sCpuCore0PCT, sCpuCore1PCT, sCpuCore0PCT, sCpuCore1PCT, sCpuCore0PCT);
	snprintf(outStr, sizeof(outStr), "%d,%d", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresNumericElements0Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one or two processor cores are available in this system then return.
	// This test is required more than two cores.
	if (nProcessors <= 2)
	{
		ASSERT_TRUE(1) << "Test skiped. Processors count should be more than 2.";
		return;
	}

	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str(); // "P:0 C:0 T:0";  // P:X C:Y T:Z

	char bufCpuIdThreadsPCT[2560];

	unsigned indCpu;
	RsslUInt32 CpuCores[MAX_TEST_CORES];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[2560];
	int n;
	unsigned i;
	unsigned nTestProcessors;

	RsslBuffer outputResult;
	char textResult[2560];

	// Construct binding string, i.e. list of processor cores will bind for the calling thread
	n = snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s", sCpuCore0PCT);
	CpuCores[0] = processorsOnline.cpuIdOnline[0];
	nTestProcessors = 1;
	for (i = 1, indCpu = 2; i < MAX_TEST_CORES && indCpu < processorsOnline.nCpuOnline && n < sizeof(bufCpuIdThreadsPCT); ++i, indCpu += 2)
	{
		n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, ",%u", processorsOnline.cpuIdOnline[indCpu]);
		CpuCores[i] = processorsOnline.cpuIdOnline[indCpu];
		++nTestProcessors;
	}

	// Construct result string, i.e. list of processor cores will bind for the calling thread in ascending order
	n = 0;
	for (i = 0; i < nTestProcessors && n < sizeof(outStr); ++i)
	{
		if (i > 0)
			n += snprintf(outStr + n, sizeof(outStr) - n, ",");

		n += snprintf(outStr + n, sizeof(outStr) - n, "%u", CpuCores[i]);
	}

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	RsslThreadId threadId = { 0, GetCurrentThread() };

	for (i = 0; i < nTestProcessors; ++i)
	{
		ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCores[i])) << "Error on test " << i << ", CpuCore=" << CpuCores[i];
	}

#else	// Linux

	pthread_t threadMain = pthread_self();

	for (i = 0; i < nTestProcessors; ++i)
	{
		ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCores[i])) << "Error on test " << i << ", CpuCore=" << CpuCores[i];
	}
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresNumericElements1Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one or two processor cores are available in this system then return.
	// This test is required more than two cores.
	if (nProcessors <= 2)
	{
		ASSERT_TRUE(1) << "Test skiped. Processors count should be more than 2.";
		return;
	}

	char bufCpuIdThreadsPCT[2560];

	int indCpu;

	RsslUInt32 CpuCores[MAX_TEST_CORES];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[2560];
	int n;
	int i;
	int nTestProcessors;

	RsslBuffer outputResult;
	char textResult[2560];

	// Construct binding string, i.e. list of processor cores will bind for the calling thread
	n = 0;
	nTestProcessors = 0;
	for (i = 0, indCpu = processorsOnline.nCpuOnline-1; i < MAX_TEST_CORES && 0 <= indCpu && n < sizeof(bufCpuIdThreadsPCT); ++i, indCpu -= 2)
	{
		if (i > 0)
			n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, ",");

		n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, "%u", processorsOnline.cpuIdOnline[indCpu]);

		CpuCores[i] = processorsOnline.cpuIdOnline[indCpu];
		++nTestProcessors;
	}

	// Construct result string, i.e. list of processor cores that bind for the calling thread in ascending order
	n = 0;
	for (i = nTestProcessors-1; 0 <= i && n < sizeof(outStr); --i)
	{
		n += snprintf(outStr + n, sizeof(outStr) - n, "%u", CpuCores[i]);
		if (i > 0)
			n += snprintf(outStr + n, sizeof(outStr) - n, ",");
	}

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	RsslThreadId threadId = { 0, GetCurrentThread() };

	for (i = 0; i < nTestProcessors; ++i)
	{
		ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCores[i])) << "Error on test " << i << ", CpuCore=" << CpuCores[i];
	}

#else	// Linux

	pthread_t threadMain = pthread_self();

	for (i = 0; i < nTestProcessors; ++i)
	{
		ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCores[i])) << "Error on test " << i << ", CpuCore=" << CpuCores[i];
	}
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresCombine0Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// P:X C:Y T:Z
	const char* sCpuCore1PCT = processorsOnline.cpuPCTOnline[1].c_str();

	char bufCpuIdThreadsPCT[128];

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%u, %s", CpuCoreId0, sCpuCore1PCT);
	snprintf(outStr, sizeof(outStr), "%u,%u", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresCombine1Test)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str();

	char bufCpuIdThreadsPCT[128];

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%u, %s", CpuCoreId1, sCpuCore0PCT);
	snprintf(outStr, sizeof(outStr), "%u,%u", CpuCoreId0, CpuCoreId1);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;
	//printf("Binding success: %s\n", bufCpuIdThreadsPCT);

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCoreList_01_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char cpuIdList[MAXLEN] = "0, 1";
	
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];

	snprintf(cpuIdList, sizeof(cpuIdList), "%u, %u", CpuCoreId0, CpuCoreId1);

	// Initialization stage
	ASSERT_EQ(rsslBindThread(cpuIdList, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	//printf("Binding success: %s\n", cpuIdList);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId1));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));

#endif
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCoreList_10_Test)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char cpuIdList[MAXLEN] = "1, 0";

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];

	snprintf(cpuIdList, sizeof(cpuIdList), "%u, %u", CpuCoreId1, CpuCoreId0);

	// Initialization stage
	ASSERT_EQ(rsslBindThread(cpuIdList, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	//printf("Binding success: %s\n", cpuIdList);

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId1));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));

#endif
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCores0_FailedTest)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char listCpuIdThread[MAXLEN];

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "%u,%u", CpuCoreId0, nProcessors);
	snprintf(outStr, sizeof(outStr), "Configuration setting %u did not match any physical processors on the system.", nProcessors);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCores1_FailedTest)
{
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char listCpuIdThread[MAXLEN];

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "%u, %u", nProcessors, CpuCoreId0);
	snprintf(outStr, sizeof(outStr), "Configuration setting %u did not match any physical processors on the system.", nProcessors);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCT0_FailedTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str();

	// The next processor core
	char sCpuCoreNextPCT[MAXLEN];  // P:X C:Y T:Z

	char bufCpuIdThreadsPCT[128];

	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// For Fail-test we choose an invalid CPU core.
	// Construct an invalid topology.
	unsigned indCpu = 0;
	unsigned iPackage = indCpu / 3;
	unsigned iCore = indCpu / 3;
	unsigned iThread = indCpu / 3;

	do {
		switch (indCpu % 3)
		{
		case 0:  // Package
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:%u C:0 T:0", (++iPackage));
			break;
		case 1:  // Core
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:0 C:%u T:0", (++iCore));
			break;
		case 2:  // Thread
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:0 C:0 T:%u", (++iThread));
			break;
		}

		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCoreNextPCT, &outputResult, &errorInfo);
		if (ret == RSSL_RET_FAILURE)  // an invalid CPU topology
		{
			//printf("Binding failure: indCpu=%u (%s)\n", indCpu, sCpuCoreNextPCT);
			break;
		}

		indCpu++;
	} while (ret == RSSL_RET_SUCCESS && indCpu < nProcessors);
	ASSERT_EQ(ret, RSSL_RET_FAILURE);

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s,%s", sCpuCore0PCT, sCpuCoreNextPCT);
	snprintf(outStr, sizeof(outStr), "Configuration setting %s did not match any physical processors on the system.", sCpuCoreNextPCT);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCT1_FailedTest)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// "P:0 C:0 T:0";  // P:X C:Y T:Z
	const char* sCpuCore0PCT = processorsOnline.cpuPCTOnline[0].c_str();

	// The next processor core
	char sCpuCoreNextPCT[MAXLEN];  // P:X C:Y T:Z

	char bufCpuIdThreadsPCT[128];

	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// For Fail-test we choose an invalid CPU core.
	// Construct an invalid topology.
	unsigned indCpu = 0;
	unsigned iPackage = indCpu / 3;
	unsigned iCore = indCpu / 3;
	unsigned iThread = indCpu / 3;

	do {
		switch (indCpu % 3)
		{
		case 0:  // Package
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:%u C:0 T:0", (++iPackage));
			break;
		case 1:  // Core
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:0 C:%u T:0", (++iCore));
			break;
		case 2:  // Thread
			snprintf(sCpuCoreNextPCT, sizeof(sCpuCoreNextPCT), "P:0 C:0 T:%u", (++iThread));
			break;
		}

		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCoreNextPCT, &outputResult, &errorInfo);
		if (ret == RSSL_RET_FAILURE)  // an invalid CPU topology
		{
			//printf("Binding failure: indCpu=%u (%s)\n", indCpu, sCpuCoreNextPCT);
			break;
		}

		indCpu++;
	} while (ret == RSSL_RET_SUCCESS && indCpu < nProcessors);
	ASSERT_EQ(ret, RSSL_RET_FAILURE);

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCoreNextPCT, sCpuCore0PCT);
	snprintf(outStr, sizeof(outStr), "Configuration setting %s did not match any physical processors on the system.", sCpuCoreNextPCT);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindThreadExShouldReturnFailOnBadFormatCoreStringTest)
{
	const char* pCoreStrBad[3] = { "P", "C", "T" };
	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	for (unsigned i = 0; i < 3; ++i)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		snprintf(outStr, sizeof(outStr), "Syntax for cpu binding for string (eos) is invalid. (%s)", pCoreStrBad[i]);

		// Setup stage
		ASSERT_EQ(rsslBindThreadEx(pCoreStrBad[i], &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

		ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
		ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
		ASSERT_STREQ(errorInfo.rsslError.text, outStr);
		ASSERT_STREQ(outputResult.data, outStr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

class ThreadBindProcessorCoreTestInit : public ::testing::Test {
protected:
	virtual void SetUp()
	{
		initProcessorUnitOnlineInfo();
		ASSERT_TRUE(processorsOnline.nCpuOnline > 0);
	}

	virtual void TearDown()
	{
	}
};

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadBeforeInitializationShouldReturnOk)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char cpuIdList[MAXLEN] = "0, 1";

	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslErrorInfo errorInfo;

	snprintf(cpuIdList, sizeof(cpuIdList), "%u, %u", CpuCoreId0, CpuCoreId1);

	// Initialization stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThread(cpuIdList, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId1));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));

#endif
}

TEST_F(ThreadBindProcessorCoreTestInit, BindProcessorCoreThreadBeforeInitializationShouldReturnOk)
{
	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslErrorInfo errorInfo;

	// Initialization stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindProcessorCoreThread(CpuCoreId1, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

	// Check stage
#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId1));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));

#endif
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadPCTBeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Setup stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadPCTCombine1BeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "0, P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Setup stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadPCTCombine2BeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0, 1";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Setup stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadExBeforeInitializationShouldReturnOk)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	char listCpuIdThread[MAXLEN] = "0,1";
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslUInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslErrorInfo errorInfo;

	char outStr[MAXLEN] = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "%u, %u", CpuCoreId0, CpuCoreId1);
	snprintf(outStr, sizeof(outStr), "%u,%u", CpuCoreId0, CpuCoreId1);

	// Setup stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "outputResult: " << outputResult.data;

	// Check stage
#ifdef WIN32
	DWORD_PTR cpuMask = (1ULL << CpuCoreId0) | (1ULL << CpuCoreId1);
	DWORD_PTR dwprocessAffinity, dwSystemAffinity;

	ASSERT_TRUE(GetProcessAffinityMask(GetCurrentProcess(), &dwprocessAffinity, &dwSystemAffinity));
	ASSERT_EQ((cpuMask & dwprocessAffinity), cpuMask);

	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
	//printf("cpuMask=%llu, dwprocessAffinity=%llu, dwSystemAffinity=%llu\n", cpuMask, dwprocessAffinity, dwSystemAffinity);

#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadExPCTBeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
	ASSERT_STREQ(outputResult.data, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadExBeforeInitializationEmptyOutputResultShouldReturnOk)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];

	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;

	rsslClearBuffer(&outputResult);

	snprintf(cpuIdThreadPCT, sizeof(cpuIdThreadPCT), "%u", CpuCoreId0);

	// Setup stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThreadEx(cpuIdThreadPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));

#endif

	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}

TEST_F(ThreadBindProcessorCoreTestInit, BindThreadExPCTBeforeInitializationEmptyOutputResultShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;

	rsslClearBuffer(&outputResult);

	// Setup stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);

	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}

/////////////////////////////////////////////////////////////////////////////////////

class ThreadBindProcessorCoreTestETAInit : public ::testing::Test {
protected:
	virtual void SetUp()
	{
		initProcessorUnitOnlineInfo();
		ASSERT_TRUE(processorsOnline.nCpuOnline > 0);
	}

	virtual void TearDown()
	{
		rsslClearBindings();
		rsslUninitialize();
	}
};

TEST_F(ThreadBindProcessorCoreTestETAInit, BindThreadWhenSkipCpuIDTopoInitShouldReturnOk)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslErrorInfo errorInfo;
	RsslError error;
	RsslInitializeExOpts rsslInitExOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	snprintf(cpuIdThreadPCT, sizeof(cpuIdThreadPCT), "%u", CpuCoreId0);

	// Setup stage
	// Set the flag to skip CpuID Topology initialization
	rsslInitExOpts.shouldInitializeCPUIDlib = RSSL_FALSE;

	ASSERT_EQ(rsslInitializeEx(&rsslInitExOpts, &error), RSSL_RET_SUCCESS);

	// Test stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThread(cpuIdThreadPCT, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));

#endif
}

TEST_F(ThreadBindProcessorCoreTestETAInit, BindThreadExWhenSkipCpuIDTopoInitShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslError error;
	RsslInitializeExOpts rsslInitExOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
	RsslBuffer outputResult;

	rsslClearBuffer(&outputResult);

	// Setup stage
	// Set the flag to skip CpuID Topology initialization
	rsslInitExOpts.shouldInitializeCPUIDlib = RSSL_FALSE;

	ASSERT_EQ(rsslInitializeEx(&rsslInitExOpts, &error), RSSL_RET_SUCCESS);

	// Test stage
	// Mapping from PCT to number - should return an error.
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);

	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}

TEST_F(ThreadBindProcessorCoreTestETAInit, CpuTopologyInitializationShouldReturnOk)
{
	RsslError error;
	RsslInitializeExOpts rsslInitExOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	ASSERT_EQ(RSSL_TRUE, rsslInitExOpts.shouldInitializeCPUIDlib);

	// Setup stage
	ASSERT_EQ(rsslInitializeEx(&rsslInitExOpts, &error), RSSL_RET_SUCCESS);

	// Test stage
	// Dump detected Cpu topology: list of the detected Cpu cores.
	dumpCpuTopology();

	// Verify that there are no Cpu topology initialization errors.
	RsslErrorInfo* pInitCpuIdLibError = getErrorInitializationStage();
	ASSERT_EQ((RsslErrorInfo *)NULL, pInitCpuIdLibError) << "Please report about the CPU topology detection error to RTSDK developers.";
}

/////////////////////////////////////////////////////////////////////////////////////

// ETA initialization passed ok but Cpu topology initialization failed
class ThreadBindProcessorCoreInitializationFail : public ::testing::Test {
protected:
	virtual void SetUp()
	{
		initProcessorUnitOnlineInfo();
		ASSERT_TRUE(processorsOnline.nCpuOnline > 0);

		RsslError error;
		RsslInitializeExOpts rsslInitExOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
		ASSERT_EQ(rsslInitializeEx(&rsslInitExOpts, &error), RSSL_RET_SUCCESS);

		// Set up the test error in Cpu topology
		setTestErrorInitializationStage();
	}

	virtual void TearDown()
	{
		rsslClearBindings();
		rsslUninitialize();
	}
};

TEST_F(ThreadBindProcessorCoreInitializationFail, BindThreadShouldReturnOk)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslErrorInfo errorInfo;

	snprintf(cpuIdThreadPCT, sizeof(cpuIdThreadPCT), "%u", CpuCoreId0);

	// Test stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThread(cpuIdThreadPCT, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));

#endif
}

TEST_F(ThreadBindProcessorCoreInitializationFail, BindThreadPCTShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Test stage
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_FAILURE);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_TEST);
}

TEST_F(ThreadBindProcessorCoreInitializationFail, BindProcessorCoreThreadShouldReturnOk)
{
	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslErrorInfo errorInfo;

	// Test stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindProcessorCoreThread(CpuCoreId0, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));

#endif
}

TEST_F(ThreadBindProcessorCoreInitializationFail, BindThreadExShouldReturnOk)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslUInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];

	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;
	char textResult[256];

	char outStr[MAXLEN] = "0";

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(cpuIdThreadPCT, sizeof(cpuIdThreadPCT), "%u", CpuCoreId0);
	snprintf(outStr, sizeof(outStr), "%u", CpuCoreId0);

	// Test stage
	// No mapping from PCT to number - should passed ok.
	ASSERT_EQ(rsslBindThreadEx(cpuIdThreadPCT, &outputResult, &errorInfo), RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;

#ifdef WIN32

	RsslThreadId rsslThreadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(rsslThreadId, CpuCoreId0));

#else	// Linux

	pthread_t threadId = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));

#endif

	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreInitializationFail, BindThreadExPCTShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Test stage
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_FAILURE);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_TEST);
	ASSERT_STREQ(outputResult.data, STR_ERROR_CPU_TOPOLOGY_TEST);
}

/////////////////////////////////////////////////////////////////////////////////////

class ThreadBindProcessorCoreTestChangeAffinity : public ::testing::Test {
protected:
	RsslErrorInfo errorInfo;

	virtual void SetUp()
	{
		initProcessorUnitOnlineInfo();
		ASSERT_TRUE(processorsOnline.nCpuOnline > 0);
	}

	virtual void TearDown()
	{
		rsslClearBindings();
		rsslBindThreadUninitialize();
	}

	RsslRet testBindThread(RsslInt32 cpuId, RsslInt32 cpuId1 = -1)
	{
#if defined(WIN32)
		DWORD cpuMask = 1 << cpuId;
		if (cpuId1 >= 0)
		{
			DWORD cpuMask1 = 1 << cpuId1;
			cpuMask |= cpuMask1;
		}
		return (SetThreadAffinityMask(GetCurrentThread(), cpuMask) != 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#elif defined(Linux)
		cpu_set_t cpuSet;
		CPU_ZERO(&cpuSet);
		CPU_SET(cpuId, &cpuSet);
		if (cpuId1 >= 0)
		{
			CPU_SET(cpuId1, &cpuSet);
		}

		return (sched_setaffinity(0, sizeof(cpu_set_t), &cpuSet) == 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#else /* Solaris */
		return (processor_bind(P_LWPID, P_MYID, cpuId, NULL) == 0) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE;
#endif
	}
};

TEST_F(ThreadBindProcessorCoreTestChangeAffinity, SystemBindThreadCore0_CheckAfterInitialization)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];

	// Bind the current thread by not using rsslBindThread method - system API
	ASSERT_EQ(RSSL_RET_SUCCESS, testBindThread(CpuCoreId0));

	// Check that CPU Core #0 bound to the current thread
	// But the CPU Core #1 did not bind to the current thread
#ifdef WIN32
	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	// Initialize ETA Bind API
	ASSERT_EQ(rsslBindThreadInitialize(&errorInfo.rsslError), RSSL_RET_SUCCESS);

	// Check that CPU Core #0 bound to the current thread
	// But the CPU Core #1 did not bind to the current thread
#ifdef WIN32
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif
}

TEST_F(ThreadBindProcessorCoreTestChangeAffinity, SystemBindThreadCore1_CheckAfterInitialization)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];

	// Bind the current thread by not using rsslBindThread method - system API
	ASSERT_EQ(RSSL_RET_SUCCESS, testBindThread(CpuCoreId1));

	// Check that CPU Core #1 bound to the current thread
	// But the CPU Core #0 did not bind to the current thread
#ifdef WIN32
	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	// Initialize ETA Bind API
	ASSERT_EQ(rsslBindThreadInitialize(&errorInfo.rsslError), RSSL_RET_SUCCESS);

	// Check that CPU Core #1 bound to the current thread
	// But the CPU Core #0 did not bind to the current thread
#ifdef WIN32
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux
	ASSERT_FALSE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif
}

TEST_F(ThreadBindProcessorCoreTestChangeAffinity, SystemBindThreadCore01_CheckAfterInitialization)
{
	RsslUInt32 nProcessors = processorsOnline.nCpuOnline;
	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = processorsOnline.cpuIdOnline[0];
	RsslInt32 CpuCoreId1 = processorsOnline.cpuIdOnline[1];

	// Bind the current thread by not using rsslBindThread method - system API only
	ASSERT_EQ(RSSL_RET_SUCCESS, testBindThread(CpuCoreId0, CpuCoreId1));

	// Check that both CPU Core #0 and #1 bound to the current thread
#ifdef WIN32
	RsslThreadId threadId = { 0, GetCurrentThread() };

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux

	pthread_t threadMain = pthread_self();

	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif

	// Initialize ETA Bind API
	ASSERT_EQ(rsslBindThreadInitialize(&errorInfo.rsslError), RSSL_RET_SUCCESS);

	// Check that both CPU Core #0 and #1 bound to the current thread
#ifdef WIN32
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadId, CpuCoreId1));
#else	// Linux
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId0));
	ASSERT_TRUE(isThreadAffinityIncludeCpuId(threadMain, CpuCoreId1));
#endif
}
