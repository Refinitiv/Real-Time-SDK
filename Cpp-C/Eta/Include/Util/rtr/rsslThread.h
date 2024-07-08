/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* Contains per-platform definitions for threads and mutexes. */

#ifndef RSSL_THREAD_H
#define RSSL_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

#include <windows.h>
#include <process.h>
#include "rtr/rtratomic.h"

typedef struct
{
	unsigned int threadId;
	HANDLE handle;
} RsslThreadId;

#define RSSL_THREAD_DECLARE(__threadDeclaration,__pArg) unsigned _stdcall __threadDeclaration(void *(__pArg))
#define RSSL_THREAD_START(__pThreadId, __threadFunction,__pArg) \
	( \
		(((__pThreadId)->handle = (HANDLE)_beginthreadex(NULL, 0, __threadFunction, (__pArg), 0, &(__pThreadId)->threadId)) != (HANDLE)-1) ? 0 : -1 \
	) \

#define RSSL_THREAD_DETACH(__pThreadId) CloseHandle((__pThreadId)->handle)

#define RSSL_THREAD_JOIN(__threadId) WaitForSingleObject((__threadId).handle, INFINITE)
#define RSSL_THREAD_RETURN() 0

#define RSSL_THREAD_KILL(__pThreadId) TerminateThread((__pThreadId)->handle, 0)

typedef CRITICAL_SECTION RsslMutex;
#define RSSL_MUTEX_INIT(__pMutex) (InitializeCriticalSection(__pMutex), 0)
#define RSSL_MUTEX_INIT_RTSDK(__pMutex) (InitializeCriticalSectionAndSpinCount((__pMutex), 512), 0)
#define RSSL_MUTEX_DESTROY(__pMutex) (DeleteCriticalSection(__pMutex), 0)
#define RSSL_MUTEX_LOCK(__pMutex) (EnterCriticalSection(__pMutex), 0)
#define RSSL_MUTEX_UNLOCK(__pMutex) (LeaveCriticalSection(__pMutex), 0)

// TryEnterCriticalSection returns a non-zero value on success
#define RSSL_MUTEX_TRYLOCK(__pMutex) (TryEnterCriticalSection(__pMutex) ? RSSL_TRUE : RSSL_FALSE)

/* For windows need to implement as a spin lock */
#define RSSL_STATIC_MUTEX_DECL(G_VAR)	rtr_atomic_val G_VAR=0;

/* RTR_ATOMIC_SET_RETOLD returns the previous value of rtr_my_spin_lock.
 * If it was already set to 1 then somebody else has already set it.
 */
#define RSSL_STATIC_MUTEX_LOCK(G_VAR) \
		do { \
			while (RTR_ATOMIC_SET_RETOLD(G_VAR,1) == 1) \
				Sleep(0); \
		} while( 0 )

#define RSSL_STATIC_MUTEX_UNLOCK(G_VAR) RTR_ATOMIC_SET_RETOLD(G_VAR,0)

#else	// Linux

#include <pthread.h>

typedef pthread_t RsslThreadId;
#define RSSL_THREAD_DECLARE(__threadDeclaration,__pArg) void *__threadDeclaration(void *(__pArg))
#define RSSL_THREAD_START(__pThreadId, __threadFunction,__pArg) pthread_create((__pThreadId), NULL, __threadFunction, (__pArg))
#define RSSL_THREAD_DETACH(__pThreadId) pthread_detach(*(__pThreadId))

#define RSSL_THREAD_RETURN() (void*)NULL
#define RSSL_THREAD_JOIN(__threadId) pthread_join((__threadId), NULL)
#define RSSL_THREAD_KILL(__pThreadId) pthread_cancel(*(__pThreadId))

typedef pthread_mutex_t RsslMutex;
RTR_C_INLINE int RSSL_MUTEX_INIT(RsslMutex *pMutex)
{
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init(pMutex, &mutexAttr);
}
#define RSSL_MUTEX_INIT_RTSDK(__pMutex) (pthread_mutex_init((__pMutex), NULL) ? RSSL_FALSE : RSSL_TRUE)
#define RSSL_MUTEX_DESTROY(__pMutex) (pthread_mutex_destroy(__pMutex))
#define RSSL_MUTEX_LOCK(__pMutex) (pthread_mutex_lock(__pMutex)) 
#define RSSL_MUTEX_UNLOCK(__pMutex) (pthread_mutex_unlock(__pMutex))

// pthread_mutex_trylock returns 0 on success
#define RSSL_MUTEX_TRYLOCK(__pMutex) (pthread_mutex_trylock(__pMutex) ? RSSL_FALSE : RSSL_TRUE)

#define RSSL_STATIC_MUTEX_DECL(G_VAR)     RsslMutex G_VAR = PTHREAD_MUTEX_INITIALIZER
#define RSSL_STATIC_MUTEX_LOCK(G_VAR)     do { (void) RSSL_MUTEX_LOCK(&G_VAR); } while( 0 )
#define RSSL_STATIC_MUTEX_UNLOCK(G_VAR)   do { (void) RSSL_MUTEX_UNLOCK(&G_VAR); } while( 0 )

#endif

#ifdef __cplusplus
}
#endif

#endif
