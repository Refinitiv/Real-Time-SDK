/*      Copyright 2008-2016 Intel Corporation
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    Neither the name of the Intel Corp. nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 *  util_os.c
 *  This is the auxilary source file that contain wrapper routines of
 *  OS-specific services called by functions in the file cpu_topo.c
 *  The source files can be compiled under 32-bit and 64-bit Windows and Linux.
 *
 *  Written by Patrick Fay and Shihjong Kuo
 */
#include "eta/cputopology.h"

#ifdef __linux__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <alloca.h>
#include <stdarg.h>

#ifdef __CPU_ISSET
#define MY_CPU_SET   __CPU_SET
#define MY_CPU_ZERO  __CPU_ZERO
#define MY_CPU_ISSET __CPU_ISSET
#else
#define MY_CPU_SET   CPU_SET
#define MY_CPU_ZERO  CPU_ZERO
#define MY_CPU_ISSET CPU_ISSET
#endif



#define __cdecl
#ifndef _alloca
#define _alloca alloca
#endif

#ifdef __x86_64__
#define LNX_PTR2INT unsigned long long
#define LNX_MY1CON 1LL
#else
#define LNX_PTR2INT unsigned int
#define LNX_MY1CON 1
#endif

#else

#include <windows.h>

#ifdef _M_IA64
#define LNX_PTR2INT unsigned long long
#define LNX_MY1CON 1LL
#else
#ifdef  _x86_64
#define LNX_PTR2INT __int64
#define LNX_MY1CON 1LL
#else
#define LNX_PTR2INT unsigned int
#define LNX_MY1CON 1
#endif
#endif

#endif


extern  GLKTSN_T * glbl_ptr;
extern int countBits(DWORD_PTR x);

static char scratch[BLOCKSIZE_4K];  // scratch space large enough for OS to write SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX

/*
 * BindContext
 * A wrapper function that can compile under two OS environments
 * The size of the bitmap that underlies cpu_set_t is configurable
 * at Linux Kernel Compile time. Each distro may set limit on its own.
 * Some newer Linux distro may support 256 logical processors,
 * For simplicity we don't show the check for range on the ordinal index
 * of the target cpu in Linux, interested reader can check Linux kernel documentation.
 * Prior to Windows OS with version signature 0601H, it has size limit of 64 cpu
 * in 64-bit mode, 32 in 32-bit mode, the size limit is checked.
 * Starting with Windows OS version 0601H (e.g. Windows 7), it supports up to 4 sets of
 * affinity masks, referred to as "GROUP_AFFINITY".
 * Constext switch within the same group is done by the same API as was done in previous
 * generations of windows (such as Vista). In order to bind the current executing
 * process context to a logical processor in a different group, it must be be binded
 * using a new API to the target processor group, followed by a similar
 * SetThreadAffinityMask API
 * New API related to GROUP_AFFINITY are present only in kernel32.dll of the OS with the
 * relevant version signatures. So we dynamically examine the presence of thse API
 * and fall back of legacy AffinityMask API if the new APIs are not available.
 * Limitation, New Windows APIs that support GROUP_AFFINITY requires
 *  Windows platform SDK 7.0a. The executable
 *  using this SDK and recent MS compiler should be able to perform topology enumeration on
 *  Windows 7 and prior versions
 *  If the executable is compiled with prior versions of platform SDK,
 *  the topology enumeration will not use API and data structures defined in SDK 7.0a,
 *  So enumeration will be limited to the active processor group.
 * Arguments:
 *      cpu :   the ordinal index to reference a logical processor in the system
 * Return:        0 is no error
 */
 int BindContext(unsigned int cpu)
{ int ret = -1;
#ifdef __linux__
    cpu_set_t currentCPU;
        // add check for size of cpumask_t.
        MY_CPU_ZERO(&currentCPU);
        // turn on the equivalent bit inside the bitmap corresponding to affinitymask
        MY_CPU_SET(cpu, &currentCPU);
        if ( !sched_setaffinity (0, sizeof(currentCPU), &currentCPU)  )
        {   ret = 0;
        }
#else
    DWORD_PTR affinity;
    // compile with SDK 7.0a will allow EXE to run on Windows versions
    // with and without GROUP_AFFINITY support
#if (_WIN32_WINNT >= 0x0601)
    //we resolve API dynamically at runtime, data structures required by new API is determined at compile time

    unsigned int cpu_beg = 0, cnt , cpu_cnt, j ;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX    *pSystem_rel_info = NULL;
    GROUP_AFFINITY  grp_affinity, prev_grp_affinity;
    HANDLE  hLib = LoadLibrary("kernel32.dll");
    FARPROC lpFnThrGrpAff,  lpFnLpInfoEx;
    if( cpu >= MAX_WIN7_LOG_CPU || !hLib) return ret;

    lpFnThrGrpAff = GetProcAddress(hLib, "SetThreadGroupAffinity");
    lpFnLpInfoEx = GetProcAddress(hLib, "GetLogicalProcessorInformationEx");
    // checking the availability of new API should be equivalent to checking version signature

    if(lpFnLpInfoEx && lpFnThrGrpAff)
    {   cnt = BLOCKSIZE_4K;
        memset(&scratch[0], 0, cnt);
        pSystem_rel_info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *) &scratch[0];

        //if (!GetLogicalProcessorInformationEx (RelationGroup, pSystem_rel_info, &cnt) ) return ret;
        if (!lpFnLpInfoEx (RelationGroup, pSystem_rel_info, &cnt) ) return ret;
        if( pSystem_rel_info->Relationship != RelationGroup) return ret;
        // to determine the input ordinal 'cpu' number belong to which processor group,
        // we consider each processor group to have its logical processors assigned with
        // numerical index consecutively in increasing order
        for (j= 0; j < pSystem_rel_info->Group.ActiveGroupCount; j ++ )
        {
            cpu_cnt = pSystem_rel_info->Group.GroupInfo[j].ActiveProcessorCount;
            // if the 'cpu' value is within the lower and upper bounds of a
            // processor group, we can use the new API to bind current thread to
            // the target thread affinity bit in the target processor group
            if(cpu >= cpu_beg && cpu < (cpu_beg + cpu_cnt))
            {
                memset(&grp_affinity, 0, sizeof(GROUP_AFFINITY));
                grp_affinity.Group = j;
                grp_affinity.Mask = (KAFFINITY)((DWORD_PTR)(LNX_MY1CON << (cpu - cpu_beg)) );
                //if( !SetThreadGroupAffinity(GetCurrentThread(), &grp_affinity, &prev_grp_affinity) ) return ret;
                if( !lpFnThrGrpAff(GetCurrentThread(), &grp_affinity, &prev_grp_affinity) )
                {   GetLastError();
                    return ret;
                }
                return 0;
            }
            // if the value of 'cpu' is not this processor group, we move to the next group
            cpu_beg += cpu_cnt;
        }
    }
    else  // if kernel32.dll does not support new GROUP_AFFINITY API, OS can only manage one processor
        // group and legacy API will suffice
    {   // cpu is ordinal index not count
        if( cpu >= MAX_PREWIN7_LOG_CPU ) return ret;
        // flip on the bit in the affinity mask corresponding to the input ordinal index
        affinity = (DWORD_PTR)(LNX_MY1CON << cpu);
        if ( SetThreadAffinityMask(GetCurrentThread(),  affinity) )
        { ret = 0;  }
    }
#else // If SDK version does not support GROUP_AFFINITY,
      // only the active processor group and be succesfully queried and analyzed for topology information
    if( cpu >= MAX_PREWIN7_LOG_CPU ) return ret;
    // flip on the bit in the affinity mask corresponding to the input ordinal index

    affinity = (DWORD_PTR)(1ULL << cpu);
    if ( SetThreadAffinityMask(GetCurrentThread(),  affinity) )
    { ret = 0;    }

#endif
#endif
    return ret;
}



/*
 * GetMaxCPUSupportedByOS
 * A wrapper function that calls OS specific system API to find out
 * how many logical processor the OS supports
 * Return:        a non-zero value
 */
unsigned  int GetMaxCPUSupportedByOS()
{unsigned  int lcl_OSProcessorCount = 0;
#ifdef __linux__

    lcl_OSProcessorCount = sysconf(_SC_NPROCESSORS_CONF); //This will tell us how many CPUs are currently enabled.

#else
    SYSTEM_INFO si;
#if  ( _WIN32_WINNT >= 0x0601 )
    unsigned short grpCnt;
    HANDLE  hLib = LoadLibrary("kernel32.dll");
    FARPROC lpFnMaxProcCnt;
    FARPROC lpFnProcessGrpAff, lpFnActProcGrpCnt, lpGFnThrGrpAff, lpSFnThrGrpAff, lpFnLpInfoEx;
    unsigned int cnt , cpu_cnt, i ;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX    *pSystem_rel_info = NULL;

    if(!hLib) return lcl_OSProcessorCount;
    lpFnMaxProcCnt = GetProcAddress(hLib, "GetMaximumProcessorCount");
    lpFnLpInfoEx = GetProcAddress(hLib, "GetLogicalProcessorInformationEx");
    lpFnActProcGrpCnt = GetProcAddress(hLib, "GetActiveProcessorGroupCount");
    // runtime check if os version is greater than 0601h

    if( lpFnLpInfoEx && lpFnActProcGrpCnt )
    {   lcl_OSProcessorCount = 0;
        // if Windows version support processor groups
        // tally actually populated logical processors in each group
        grpCnt = (WORD) lpFnActProcGrpCnt();
        cnt = BLOCKSIZE_4K;
        memset(&scratch[0], 0, cnt);
        pSystem_rel_info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *) &scratch[0];

        if (!lpFnLpInfoEx (RelationGroup, pSystem_rel_info, &cnt) )
        {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
        }
        if( pSystem_rel_info->Relationship != RelationGroup)
        {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
        }
        for (i = 0; i < grpCnt; i ++)       lcl_OSProcessorCount += pSystem_rel_info->Group.GroupInfo[i].ActiveProcessorCount;
    }
    else
    {
        GetSystemInfo(&si);
        lcl_OSProcessorCount = si.dwNumberOfProcessors;
    }
#else
    {
        GetSystemInfo(&si);
        lcl_OSProcessorCount = si.dwNumberOfProcessors;
    }
#endif
#endif
    return lcl_OSProcessorCount;
}

/*
 * SetChkProcessAffinityConsistency
 * A wrapper function that calls OS specific system API to find out
 * the number of logical processor support by OS matches
 * the same set of logical processors this process is allowed to run on
 * if the two set matches, set the corresponding bits in our
 * generic affinity mask construct.
 * if inconsistency is found, app-specific error code is set
 * Return:        none,
 */
void  SetChkProcessAffinityConsistency(unsigned  int lcl_OSProcessorCount, RsslErrorInfo* pError)
{unsigned int i, sum = 0;
#ifdef __linux__
    cpu_set_t allowedCPUs;

    sched_getaffinity(0, sizeof(allowedCPUs), &allowedCPUs);
    for (i = 0; i < lcl_OSProcessorCount; i++ )
    {
        if ( MY_CPU_ISSET(i, &allowedCPUs) == 0 )
        {
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR;
        }
        else
        {
            SetGenericAffinityBit(&glbl_ptr->cpu_generic_processAffinity, i, pError);
            SetGenericAffinityBit(&glbl_ptr->cpu_generic_systemAffinity, i, pError);
        }
    }

#else
    DWORD_PTR processAffinity;
    DWORD_PTR systemAffinity;
    //unsigned short grpCnt, grpAffinity[MAX_THREAD_GROUPS_WIN7];


#if  ( _WIN32_WINNT >= 0x0601 )
    HANDLE  hLib = LoadLibrary("kernel32.dll");
    FARPROC lpFnProcessGrpAff, lpFnActProcGrpCnt, lpGFnThrGrpAff, lpSFnThrGrpAff, lpFnLpInfoEx;
    GROUP_AFFINITY  grp_affinity, prev_grp_affinity;
    unsigned int cnt , cpu_cnt ;
    //WORD  grp_cnt;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX    *pSystem_rel_info = NULL;
    if(!hLib)
    {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
    }
    lpFnProcessGrpAff = GetProcAddress(hLib, "GetProcessGroupAffinity");
    lpFnActProcGrpCnt = GetProcAddress(hLib, "GetActiveProcessorGroupCount");
    lpGFnThrGrpAff = GetProcAddress(hLib, "GetThreadGroupAffinity");
    lpSFnThrGrpAff = GetProcAddress(hLib, "SetThreadGroupAffinity");
    lpFnLpInfoEx = GetProcAddress(hLib, "GetLogicalProcessorInformationEx");


    if( lpFnProcessGrpAff && lpFnActProcGrpCnt && lpFnLpInfoEx && lpGFnThrGrpAff && lpSFnThrGrpAff)
    {   cnt = BLOCKSIZE_4K;
        memset(&scratch[0], 0, cnt);
        pSystem_rel_info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *) &scratch[0];

        if (!lpFnLpInfoEx (RelationGroup, pSystem_rel_info, &cnt) )
        {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
        }
        if( pSystem_rel_info->Relationship != RelationGroup)
        {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
        }
        if (lcl_OSProcessorCount > MAX_WIN7_LOG_CPU)
        {
            glbl_ptr->error |= _MSGTYP_OSAFFCAP_ERROR ;  // If the os supports more processors than allowed, make change as required.
        }
        //grpCnt = GetActiveProcessorGroupCount();
        grpCnt = (WORD) lpFnActProcGrpCnt();
        for (i = 0; i < grpCnt; i ++)
        {   cnt = grpCnt;
            //GetProcessGroupAffinity(GetCurrentProcess(), &grpCnt, &grpAffinity[0]);
            //ret = (int) lpFnProcessGrpAff(GetCurrentProcess(), &cnt, &grpAffinity[0]);
            if( ! lpFnProcessGrpAff(GetCurrentProcess(), &cnt, &grpAffinity[0]) )
            {   //throw some exception here, no full affinity for the process
                glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS;
                break;
            }
            else
            {   cpu_cnt = pSystem_rel_info->Group.GroupInfo[i].ActiveProcessorCount;
                memset(&grp_affinity, 0, sizeof(GROUP_AFFINITY));
                grp_affinity.Group = (WORD) i;
                if(cpu_cnt == (sizeof(DWORD_PTR) *8) )
                    grp_affinity.Mask = (DWORD_PTR)( - LNX_MY1CON );
                else
                    grp_affinity.Mask = (DWORD_PTR)(((DWORD_PTR)LNX_MY1CON << cpu_cnt) -1 );
                //if( !SetThreadGroupAffinity(GetCurrentThread(), &grp_affinity, &prev_grp_affinity) ) return ret;
                if( !lpSFnThrGrpAff(GetCurrentThread(), &grp_affinity, &prev_grp_affinity) )
                {   glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS; return ;
                }

                lpGFnThrGrpAff(GetCurrentProcess(), &grp_affinity);
                sum += countBits(grp_affinity.Mask);  // count bits on each target affinity group

                if( sum > lcl_OSProcessorCount)
                {   //throw some exception here, no full affinity for the process
                    glbl_ptr->error |= _MSGTYP_USERAFFINITYERR ;
                    break;
                }
            }
        }
        if( sum != lcl_OSProcessorCount) // check cumulative bit counts matches processor count
        {   // if this process is restricted and not able to run on all logical processors managed by OS
            // the LS bytes can be extracted to indicate the affinity restrictions
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR + sum;
            return;
        }

        for (i = 0; i < lcl_OSProcessorCount; i++ )
        {   SetGenericAffinityBit(&glbl_ptr->cpu_generic_processAffinity, i, pError);
        }
        return;
    }
    else
    {
        if (lcl_OSProcessorCount > MAX_PREWIN7_LOG_CPU)
        {
            glbl_ptr->error |= _MSGTYP_OSAFFCAP_ERROR ;  // If the os supports more processors than existing win32 or win64 API,
                                                // we need to know the new API interface in that OS
        }
        GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity);
        sum = countBits(processAffinity);
        if ( lcl_OSProcessorCount != (unsigned long) sum )
        {
            //throw some exception here, no full affinity for the process
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR + sum;
        }

        if (lcl_OSProcessorCount != (unsigned long) countBits(systemAffinity) )
        {
            //throw some exception here, no full system affinity
            glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS;
        }
    }
#else
    {
        if (lcl_OSProcessorCount > MAX_PREWIN7_LOG_CPU)
        {
            glbl_ptr->error |= _MSGTYP_OSAFFCAP_ERROR ;  // If the os supports more processors than existing win32 or win64 API,
                                                // we need to know the new API interface in that OS
        }
        GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity);
        sum = countBits(processAffinity);
        if ( lcl_OSProcessorCount != (unsigned long) sum  )
        {
            //throw some exception here, no full affinity for the process
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR + sum;
        }

        if (lcl_OSProcessorCount != (unsigned long) countBits(systemAffinity) )
        {
            //throw some exception here, no full system affinity
            glbl_ptr->error |= _MSGTYP_UNKNOWNERR_OS;
        }
    }
#endif
    for (i = 0; i < lcl_OSProcessorCount; i++ )
    {
        // This logic assumes that looping over OSProcCount will let us inspect all the affinity bits
        // That is, we can't need more than OSProcessorCount bits in the affinityMask
        if ((  (unsigned long)systemAffinity & (LNX_MY1CON << i)) == 0) {
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR;
        }
        else {
            SetGenericAffinityBit(&glbl_ptr->cpu_generic_systemAffinity, i, pError);
        }
        if (((unsigned long)processAffinity & (LNX_MY1CON << i)) == 0) {
            glbl_ptr->error |= _MSGTYP_USERAFFINITYERR;
        }
        else {
            SetGenericAffinityBit(&glbl_ptr->cpu_generic_processAffinity, i, pError);
        }
    }
#endif

}

