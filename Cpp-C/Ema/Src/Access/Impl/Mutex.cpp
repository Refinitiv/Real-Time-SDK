/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Mutex.h"

using namespace refinitiv::ema::access;

#ifdef WIN32

Mutex::Mutex()
{
	InitializeCriticalSection( &m_cs );
}

Mutex::~Mutex()
{
	DeleteCriticalSection( &m_cs );
}

void Mutex::lock()
{
	EnterCriticalSection( &m_cs );
}

void Mutex::unlock()
{
	LeaveCriticalSection( &m_cs );
}

#else

Mutex::Mutex()
{
	pthread_mutexattr_t mutexAttr;

	pthread_mutexattr_init( &mutexAttr );
	pthread_mutexattr_settype( &mutexAttr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_mutex, &mutexAttr );
}

Mutex::~Mutex()
{
	pthread_mutex_destroy( &m_mutex );
}

void Mutex::lock()
{
	pthread_mutex_lock( &m_mutex );
}

void Mutex::unlock()
{
	pthread_mutex_unlock( &m_mutex );
}


ConditionVariable::ConditionVariable()
{
	pthread_cond_init(&condition_var, NULL);
}

void ConditionVariable::wait(Mutex& mutex)
{
	pthread_cond_wait(&condition_var, &mutex.m_mutex);
}

void ConditionVariable::timedwait(Mutex& mutex, timespec* abstime)
{
	pthread_cond_timedwait(&condition_var, &mutex.m_mutex, abstime);
}

void ConditionVariable::notify()
{
	pthread_cond_signal(&condition_var);
}

#endif // WIN32
