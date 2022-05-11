/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
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
		for (unsigned k = 0; k < cpuCoreArgs->n; ++k)
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

class ThreadBindProcessorCoreTest : public ::testing::Test {
protected:
	RsslErrorInfo errorInfo;

	virtual void SetUp()
	{
		ASSERT_EQ(rsslBindThreadInitialize(), RSSL_RET_SUCCESS);
	}

	virtual void TearDown()
	{
		rsslClearBindings();
		rsslBindThreadUninitialize();
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

TEST(ThreadBindProcessorCoreTestInit, BindThreadBeforeInitializationShouldReturnError)
{
	char sCpuCorePCT[MAXLEN] = "P:0 C:0 T:0";  // P:X C:Y T:Z
	RsslErrorInfo errorInfo;

	// Setup stage
	ASSERT_EQ(rsslBindThread(sCpuCorePCT, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, "Cpu topology information is unavailable.");
}

TEST(ThreadBindProcessorCoreTestInit, BindProcessorCoreThreadBeforeInitializationShouldReturnError)
{
	RsslInt32 cpuIdThread = 0;
	RsslErrorInfo errorInfo;

	// Setup stage
	ASSERT_EQ(rsslBindProcessorCoreThread(cpuIdThread, &errorInfo), RSSL_RET_FAILURE);

	ASSERT_EQ(errorInfo.rsslErrorInfoCode, RSSL_EIC_FAILURE);
	ASSERT_EQ(errorInfo.rsslError.rsslErrorId, RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(errorInfo.rsslError.text, "Cpu topology information is unavailable.");
}
