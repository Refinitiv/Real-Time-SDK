/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2022-2023 Refinitiv. All rights reserved.          --
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

typedef struct {
	unsigned n;
	char* strCpuCore;
} CpuCoreArg;

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

	if (cpuCoreArgs != NULL && cpuCoreArgs->n > 0)
	{
		// A lot of different topologies for the the processor core.
		// At least, one of them must be correct.
		// P:X C:Y T:Z
		for (unsigned long long k = 0; k < cpuCoreArgs->n; ++k)
		{
			cpuString = (cpuCoreArgs->strCpuCore + k*MAXLEN);
			//printf("runBindProcessTestThreadStr: %s\n", cpuString);

			ret = rsslBindThread(cpuString, &errorInfoThread);
			if (ret == RSSL_RET_SUCCESS)
			{
				//printf("runBindProcessTestThreadStr Binding success: k=%u (%s)\n", k, cpuString);
				break;
			}
		}

		if (ret != RSSL_RET_SUCCESS)
		{
			printf("runBindProcessTestThreadStr error: binding fail.\n");
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
		//long nproc = sysconf(_SC_NPROCESSORS_ONLN);
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
	RsslInt32 CpuCoreId = 1;

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
	RsslInt32 cpuIdMainThread = 0;
	RsslInt32 cpuIdThread = 1;

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
	RsslInt32 cpuIdMainThread = 1;
	RsslInt32 cpuIdThread = 0;

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
	nProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	if (nProcessors < 64)
	{
		CpuCoreId = nProcessors + 1;
	}

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
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslInt32 CpuCoreId = 0;

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
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };
	RsslInt32 CpuCoreId = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Setup stage
	// Try to choose one of the Cpu core templates
	for (unsigned k = 0; k < 3; ++k)
	{
		ret = rsslBindThread(sCpuCorePCT[k], &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
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
	nProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	if (nProcessors < 64)
	{
		CpuCoreId = nProcessors + 1;
	}

	snprintf(sCpuCorePCT, sizeof(sCpuCorePCT), "P:0 C:%u T:0", CpuCoreId);

	//printf("nProcessors=%u, CpuCoreId=%d, PCT={%s}\n", nProcessors, CpuCoreId, sCpuCorePCT);

	// Test stage
	// Try to bind an unavailable processor core
	// Should return Fail: Not RSSL_RET_SUCCESS.
	ASSERT_NE(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_SUCCESS);
}

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCorePCT_BindMainThenThread_Test)
{
	char cpuIdMainThreadPCT[MAXLEN] = "P:0 C:0 T:0";

	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	CpuCoreArg threadPCTArg = { 3, &(sCpuCorePCT[0][0]) };

	RsslInt32 cpuIdMainThread = 0;
	RsslInt32 cpuIdThread = 1;

	RsslThreadId rsslThreadId;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
	threadBreak = false;

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

TEST_F(ThreadBindProcessorCoreTest, BindTwoThreadsToCpuCorePCT_BindThreadThenMain_Test)
{
	char cpuIdThreadPCT[MAXLEN] = "P:0 C:0 T:0";

	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	CpuCoreArg threadPCTArg = { 1, &(cpuIdThreadPCT[0]) };

	RsslInt32 cpuIdMainThread = 1;
	RsslInt32 cpuIdThread = 0;

	RsslThreadId rsslThreadId;
	RsslRet ret;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
	threadBreak = false;

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	// Try to choose one of the Cpu core templates
	for (unsigned k = 0; k < 3; ++k)
	{
		ret = rsslBindThread(sCpuCorePCT[k], &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
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
	char cpuIdMainThreadPCT[MAXLEN] = "0";

	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	CpuCoreArg threadPCTArg = { 3, &(sCpuCorePCT[0][0]) };

	RsslInt32 cpuIdMainThread = 0;
	RsslInt32 cpuIdThread = 1;

	RsslThreadId rsslThreadId;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
	threadBreak = false;

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
	char cpuIdThreadPCT[MAXLEN] = "0";

	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	CpuCoreArg threadPCTArg = { 1, &(cpuIdThreadPCT[0]) };

	RsslInt32 cpuIdMainThread = 1;
	RsslInt32 cpuIdThread = 0;

	RsslThreadId rsslThreadId;
	RsslRet ret;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
	threadBreak = false;

	RSSL_THREAD_START(&rsslThreadId, runBindProcessTestThreadStr, &threadPCTArg);

	time_sleep(50);

	// Try to choose one of the Cpu core templates
	for (unsigned k = 0; k < 3; ++k)
	{
		ret = rsslBindThread(sCpuCorePCT[k], &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
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
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslInt32 CpuCoreId = 0;
	const char* outStr = "0";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
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
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCorePCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };
	RsslInt32 CpuCoreId = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStr = "1";

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Setup stage
	// Try to choose one of the Cpu core templates
	for (unsigned k = 0; k < 3; ++k)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCorePCT[k], &outputResult, &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
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
	const char* listCpuIdThread = "0,1";
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

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
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[128];

	unsigned indCpu;
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStrPreSetup = "1";
	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}
	
	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	ASSERT_STREQ(outputResult.data, outStrPreSetup);  // "1"

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCore0PCT, sCpuCore1PCT[indCpu]);

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
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[256];

	unsigned indCpu;
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStrPreSetup = "1";
	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	ASSERT_STREQ(outputResult.data, outStrPreSetup);  // "1"

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s, %s, %s, %s, %s, %s", sCpuCore0PCT, sCpuCore1PCT[indCpu], sCpuCore0PCT, sCpuCore1PCT[indCpu], sCpuCore0PCT, sCpuCore1PCT[indCpu], sCpuCore1PCT[indCpu]);

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
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[256];

	unsigned indCpu;
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStrPreSetup = "1";
	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	ASSERT_STREQ(outputResult.data, outStrPreSetup);  // "1"

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s, %s, %s, %s, %s, %s", sCpuCore1PCT[indCpu], sCpuCore0PCT, sCpuCore0PCT, sCpuCore1PCT[indCpu], sCpuCore0PCT, sCpuCore1PCT[indCpu], sCpuCore0PCT);

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
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z

	char bufCpuIdThreadsPCT[256];

	unsigned indCpu;
	RsslInt32 CpuCores[64];
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];
	int n;
	unsigned i;
	unsigned nTestProcessors;

	RsslBuffer outputResult;
	char textResult[256];

	// If one or two processor cores are available in this system then return.
	// This test is required more than two cores.
	if (nProcessors <= 2)
	{
		ASSERT_TRUE(1) << "Test skiped. Processors count should be more than 2.";
		return;
	}

	// Construct binding string, i.e. list of processor cores that bind for the calling thread
	n = snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s", sCpuCore0PCT);
	CpuCores[0] = 0;
	nTestProcessors = 1;
	for (i = 1, indCpu = 2; i < 64 && indCpu < nProcessors && n < sizeof(bufCpuIdThreadsPCT); ++i, indCpu += 2)
	{
		n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, ",%u", indCpu);
		CpuCores[i] = indCpu;
		++nTestProcessors;
	}

	// Construct result string, i.e. list of processor cores that bind for the calling thread in ascending order
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
	char bufCpuIdThreadsPCT[256];

	int indCpu;

	RsslInt32 CpuCores[64];
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];
	int n;
	int i;
	int nTestProcessors;

	RsslBuffer outputResult;
	char textResult[256];

	// If one or two processor cores are available in this system then return.
	// This test is required more than two cores.
	if (nProcessors <= 2)
	{
		ASSERT_TRUE(1) << "Test skiped. Processors count should be more than 2.";
		return;
	}

	// Construct binding string, i.e. list of processor cores that bind for the calling thread
	n = 0;
	nTestProcessors = 0;
	for (i = 0, indCpu = nProcessors-1; i < 64 && 0 <= indCpu && n < sizeof(bufCpuIdThreadsPCT); ++i, indCpu -= 2)
	{
		if (i > 0)
			n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, ",");

		n += snprintf(bufCpuIdThreadsPCT + n, sizeof(bufCpuIdThreadsPCT) - n, "%d", indCpu);

		CpuCores[i] = indCpu;
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
	char sCpuCore0[MAXLEN] = "0";  // CPU core id
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[128];

	unsigned indCpu;
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStrPreSetup = "1";
	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_SUCCESS)
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_SUCCESS) << "errorInfo: " << errorInfo.rsslError.text;
	ASSERT_STREQ(outputResult.data, outStrPreSetup);  // "1"

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCore0, sCpuCore1PCT[indCpu]);

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
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	char sCpuCore1[MAXLEN] = "1";  // CPU core id

	char bufCpuIdThreadsPCT[128];

	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	const char* outStrPreSetup = "1";
	const char* outStr = "0,1";

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCore1, sCpuCore0PCT);

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
	char cpuIdList[MAXLEN] = "0, 1";
	
	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
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

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadToCpuCoreList_10_Test)
{
	char cpuIdList[MAXLEN] = "1, 0";

	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;

	// If one processor core is available in this system then return.
	// Case binding to Core:0 has already tested.
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Initialization stage
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

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCores0_FailedTest)
{
	char listCpuIdThread[MAXLEN];
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "0,%u", nProcessors);
	snprintf(outStr, sizeof(outStr), "Configuration setting %u did not match any physical processors on the system.", nProcessors);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCores1_FailedTest)
{
	char listCpuIdThread[MAXLEN];
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	snprintf(listCpuIdThread, sizeof(listCpuIdThread), "%u, 0", nProcessors);
	snprintf(outStr, sizeof(outStr), "Configuration setting %u did not match any physical processors on the system.", nProcessors);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(listCpuIdThread, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCT0_FailedTest)
{
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[128];

	unsigned indCpu;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates.
	// For Fail-test we choose an invalid CPU core.
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_FAILURE)  // invalid CPU topology
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_FAILURE);

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s,%s", sCpuCore0PCT, sCpuCore1PCT[indCpu]);
	snprintf(outStr, sizeof(outStr), "Configuration setting %s did not match any physical processors on the system.", sCpuCore1PCT[indCpu]);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(bufCpuIdThreadsPCT, &outputResult, &errorInfo), RSSL_RET_FAILURE) << "outputResult: " << outputResult.data;

	// Check stage
	ASSERT_STREQ(outputResult.data, outStr);
}

TEST_F(ThreadBindProcessorCoreTest, BindCurrentThreadExToListCpuCoresPCT1_FailedTest)
{
	char sCpuCore0PCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	// Three different topologies for the next processor core.
	// At least, one of them must be correct.
	// P:X C:Y T:Z
	char sCpuCore1PCT[3][MAXLEN] = { "P:0 C:0 T:1", "P:0 C:1 T:0", "P:1 C:0 T:0" };

	char bufCpuIdThreadsPCT[128];

	unsigned indCpu;
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	char outStr[256];

	RsslBuffer outputResult;
	char textResult[256];

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

	// Pre-setup stage. Calculate topology for the 2-nd Cpu core.
	// Try to choose one of the Cpu core templates.
	// For Fail-test we choose an invalid CPU core.
	for (indCpu = 0; indCpu < 3; ++indCpu)
	{
		outputResult.length = sizeof(textResult);
		outputResult.data = textResult;

		ret = rsslBindThreadEx(sCpuCore1PCT[indCpu], &outputResult, &errorInfo);
		if (ret == RSSL_RET_FAILURE)  // invalid CPU topology
		{
			//printf("Binding success: k=%u (%s)\n", k, sCpuCorePCT[k]);
			break;
		}
	}
	ASSERT_EQ(ret, RSSL_RET_FAILURE);

	rsslClearBindings();
	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	snprintf(bufCpuIdThreadsPCT, sizeof(bufCpuIdThreadsPCT), "%s, %s", sCpuCore1PCT[indCpu], sCpuCore0PCT);
	snprintf(outStr, sizeof(outStr), "Configuration setting %s did not match any physical processors on the system.", sCpuCore1PCT[indCpu]);

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

TEST(ThreadBindProcessorCoreTestInit, BindThreadBeforeInitializationShouldReturnError)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslErrorInfo errorInfo;

	// Setup stage
	ASSERT_EQ(rsslBindThread(cpuIdThreadPCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadPCTBeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Setup stage
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindProcessorCoreThreadBeforeInitializationShouldReturnError)
{
	RsslInt32 cpuIdThread = 0;
	RsslErrorInfo errorInfo;

	// Setup stage
	ASSERT_EQ(rsslBindProcessorCoreThread(cpuIdThread, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadExBeforeInitializationShouldReturnError)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(cpuIdThreadPCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
	ASSERT_STREQ(outputResult.data, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadExPCTBeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;
	char textResult[256];

	outputResult.length = sizeof(textResult);
	outputResult.data = textResult;

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
	ASSERT_STREQ(outputResult.data, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadExBeforeInitializationEmptyOutputResultShouldReturnError)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;

	rsslClearBuffer(&outputResult);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(cpuIdThreadPCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
	
	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadExPCTBeforeInitializationEmptyOutputResultShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;
	RsslBuffer outputResult;

	rsslClearBuffer(&outputResult);

	// Setup stage
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);

	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadWhenSkipCpuIDTopoInitShouldReturnError)
{
	char cpuIdThreadPCT[MAXLEN] = "0";
	RsslErrorInfo errorInfo;
	RsslError error;
	RsslInitializeExOpts rsslInitExOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	// Setup stage
	// Set the flag to skip CpuID Topology initialization
	rsslInitExOpts.shouldInitializeCPUIDlib = RSSL_FALSE;

	ASSERT_EQ(rsslInitializeEx(&rsslInitExOpts, &error), RSSL_RET_SUCCESS);

	// Test stage
	ASSERT_EQ(rsslBindThread(cpuIdThreadPCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);
}

TEST(ThreadBindProcessorCoreTestInit, BindThreadExWhenSkipCpuIDTopoInitShouldReturnError)
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
	ASSERT_EQ(rsslBindThreadEx(sCpuCorePCT, &outputResult, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, STR_ERROR_CPU_TOPOLOGY_UNAVAILABLE);

	ASSERT_EQ(NULL, outputResult.data);
	ASSERT_EQ(0, outputResult.length);
}


class ThreadBindProcessorCoreTestChangeAffinity : public ::testing::Test {
protected:
	RsslErrorInfo errorInfo;

	virtual void SetUp()
	{
		//ASSERT_EQ(rsslBindThreadInitialize(&errorInfo.rsslError), RSSL_RET_SUCCESS);
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
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

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
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

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
	RsslUInt32 nProcessors = rsslGetNumberOfProcessorCore();
	RsslRet ret = RSSL_RET_SUCCESS;

	RsslInt32 CpuCoreId0 = 0;
	RsslInt32 CpuCoreId1 = 1;

	// If one processor core is available in this system then return.
	// This test is required two cores.
	if (nProcessors <= 1)
	{
		ASSERT_TRUE(1);
		return;
	}

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
