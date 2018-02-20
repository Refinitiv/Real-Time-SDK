/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_SPIN_LOCK_H
#define __RTR_SPIN_LOCK_H

#include "rtr/os.h"

#ifdef _WIN32
#include <windows.h>

typedef LONG rtrSpinLock;
#else
#include <pthread.h>
typedef pthread_spinlock_t rtrSpinLock;
#endif

#ifdef _WIN32

RTR_C_ALWAYS_INLINE void ___rtr_spin_delay()
{
/*	__asm rep nop; */
	Sleep(0);
}

extern LONG  __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
#pragma intrinsic (_InterlockedCompareExchange)
#define ___RTR_TAS(lock) (_InterlockedCompareExchange(lock, 1, 0))

#define ___RTR_SPIN_DELAY() ___rtr_spin_delay()


#define RTR_SLOCK_ACQUIRE(lock) while (___RTR_TAS(lock)) { ___RTR_SPIN_DELAY(); }
#define RTR_SLOCK_RELEASE(lock) (*((volatile rtrSpinLock *)(lock)) = 0)
#define RTR_SLOCK_INIT(lock) RTR_SLOCK_RELEASE(lock)
#define RTR_SLOCK_DESTROY(lock) (*(lock) == 0)

#else

#define RTR_SLOCK_ACQUIRE(lock) pthread_spin_lock(lock)
#define RTR_SLOCK_RELEASE(lock) pthread_spin_unlock(lock)
#define RTR_SLOCK_INIT(lock) pthread_spin_init(lock, PTHREAD_PROCESS_SHARED)
#define RTR_SLOCK_DESTROY(lock) pthread_spin_destroy(lock)

#endif

#endif
