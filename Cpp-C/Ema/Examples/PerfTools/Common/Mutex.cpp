/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2016,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Mutex.h"

using namespace perftool::common;

#if defined(WIN32)
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

#else  // WIN32

Mutex::Mutex()
{
	pthread_mutex_init( &m_mutex, NULL );
	pthread_cond_init( &m_cv, NULL );
}

Mutex::~Mutex()
{
	pthread_cond_destroy( &m_cv );
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
