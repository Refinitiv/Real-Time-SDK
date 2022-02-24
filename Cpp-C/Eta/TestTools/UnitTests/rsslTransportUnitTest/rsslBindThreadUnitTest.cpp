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

class ThreadBindProcessorCoreTest : public ::testing::Test {
protected:
	RsslErrorInfo errorInfo;

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
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
			printf("isThreadAffinityIncludeCpuId. CpuCoreId=%d. oldMask=%llu, cpuMask=%llu\n", CpuCoreId, oldMask, cpuMask);
			return ((oldMask & cpuMask) != 0);
		}

#else	// Linux
		cpu_set_t cpuSet;
		bool res = false;
		CPU_ZERO(&cpuSet);

		if (pthread_getaffinity_np(rsslThread, sizeof(cpu_set_t), &cpuSet) == 0)
		{
			res = (CPU_ISSET(CpuCoreId, &cpuSet));

			long i;
			long nproc = sysconf(_SC_NPROCESSORS_ONLN);
			printf("pthread_getaffinity_np = ");
			for (i = 0; i < nproc; i++) {
				printf("%d ", CPU_ISSET(i, &cpuSet));
			}
			printf("\n");

			return res;
		}

#endif
		return false;
	}

};

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

	if (nProcessors < 32)
	{
		CpuCoreId = nProcessors + 1;
	}

	printf("nProcessors = %u, CpuCoreId=%d\n", nProcessors, CpuCoreId);

	// Test stage
	// Try to bind an unavailable processor core
	// Should return Fail: not RSSL_RET_SUCCESS.
	ASSERT_NE(rsslBindProcessorCoreThread(CpuCoreId, &errorInfo), RSSL_RET_SUCCESS);
}
