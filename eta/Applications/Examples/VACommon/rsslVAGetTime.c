/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifdef LINUX

#include "rsslGetTime.h"
#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtr/rtrmutx.h"
#include "rtr/rtrenv.h"

static RTR_STATIC_MUTEX_DECL(rssl_TimeInitMutex);
static int rssl_gettime_first=1;
static double rssl_FreqNanoS=0;
static double rssl_FreqMicroS=0;
static double rssl_FreqMilliS=0;

	/* This is more a question of hardware, but it is easier to check for the OS
	 * since we're not likely to be building / running AS 2.1 on modern hardware.
	 */
#ifdef x86_Linux_2X
inline long long int rssl_rdtsc()
{
	long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
#else
inline long long int rssl_rdtsc()
{
	unsigned int lo, hi;
	unsigned long long int result;
#ifdef _LP64
#ifdef __pic__
		/* On UNIX -fPIC uses rbx. Need to save and then restore */
	__asm__ __volatile__ ("xorl %%eax,%%eax;push %%rbx;cpuid":::"%rax","rcx","rdx");
	__asm__ __volatile__ ("rdtsc;pop %%rbx" : "=a" (lo), "=d" (hi));
#else
	__asm__ __volatile__ ("xorl %%eax,%%eax;cpuid":::"%rax","rbx","rcx","rdx");
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
#endif
#else
#ifdef __pic__
		/* On UNIX -fPIC uses ebx. Need to save and then restore */
	__asm__ __volatile__ ("xorl %%eax,%%eax;pushl %%ebx;cpuid":::"%eax","ecx","edx");
	__asm__ __volatile__ ("rdtsc;popl %%ebx" : "=a" (lo), "=d" (hi));
#else
	__asm__ __volatile__ ("xorl %%eax,%%eax;cpuid":::"%eax","ebx","ecx","edx");
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
#endif
#endif
	result = (unsigned long long int) hi << 32 | lo;
	return (long long int)result;
}
#endif

static gettime_do_first()
{
	RTR_STATIC_MUTEX_LOCK(rssl_TimeInitMutex);
	if (rssl_gettime_first)
	{
		FILE		*cpuinfo;
		double		frequency=0.0;
		double		mhertz=0.0;
		char		buf[256];


			/* The only way on Linux to get the speed
			 * of the cpu is by reading it out of the
			 * cpuinfo file. This value is accurate since
			 * it is calculated by the kernel. Also
			 * have an environment variable override
			 * just in case.
			 */
		if (RTRGetEnv("RTR_CPU_CLOCK_MHZ",buf,255) != RTR_ENV_FAILURE)
		{
			mhertz = atof(buf);
			frequency = mhertz * 1000000.0;
		}
		else if ((cpuinfo = fopen("/proc/cpuinfo","r")) != 0)
		{
			while (fgets(buf,255,cpuinfo))
			{
				if (sscanf(buf,"cpu MHz	: %lf\n", &mhertz))
				{
					frequency = mhertz * 1000000.0;
					break;
				}
			}
		}

			/* If you hit this assert the cpu speed could
			 * not be accessed. Might need to find another
			 * way of getting the cpu speed (bogomips??).
			 */
		RTRASSERT(frequency != 0.0);

			/* Calculate the number of clock ticks per microsecond. */
			/* Get the number of clock ticks per second */
			/* 2259531000 */
		rssl_FreqNanoS = frequency/1000000000.0;
		rssl_FreqMicroS = frequency/1000000.0;
		rssl_FreqMilliS = frequency/1000.0;

		rssl_gettime_first = 0;

		/*
		printf("MHz %lf  Frequency %lf\n",mhertz,frequency);
		*/
	}
	RTR_STATIC_MUTEX_UNLOCK(rssl_TimeInitMutex);
}

rssl_time_value rssl_gettime_nano()
{
	long long int	clocktick;
	double			ret;

	if (rssl_gettime_first)
		gettime_do_first();

	clocktick = rssl_rdtsc();
	ret = (double)clocktick / rssl_FreqNanoS;
	return( (rssl_time_value)ret );
}

rssl_time_value rssl_gettime_micro()
{
	long long int	clocktick;
	double			ret;

	if (rssl_gettime_first)
		gettime_do_first();

	clocktick = rssl_rdtsc();
	ret = (double)clocktick / rssl_FreqMicroS;
	return( (rssl_time_value)ret );
}

rssl_time_value rssl_gettime_milli()
{
	long long int	clocktick;
	double			ret;

	if (rssl_gettime_first)
		gettime_do_first();

	clocktick = rssl_rdtsc();
	ret = (double)clocktick / rssl_FreqMilliS;
	return( (rssl_time_value)ret );
}

double rssl_getticks_per_nano()
{
	if (rssl_gettime_first)
		gettime_do_first();

	return rssl_FreqNanoS;
}

double rssl_getticks_per_micro()
{
	if (rssl_gettime_first)
		gettime_do_first();

	return rssl_FreqMicroS;
}

double rssl_getticks_per_milli()
{
	if (rssl_gettime_first)
		gettime_do_first();

	return rssl_FreqMilliS;
}

rssl_time_value rssl_getticks()
{
	return (rssl_rdtsc());
}

#endif

