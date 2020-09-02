/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Mutex.h"

using namespace rtsdk::ema::access;

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

#endif // WIN32
