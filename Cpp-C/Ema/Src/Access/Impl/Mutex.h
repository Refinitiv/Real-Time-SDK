/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_mutex_h
#define __refinitiv_ema_access_mutex_h

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#else
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#endif

namespace refinitiv {

namespace ema {
	
namespace access {

class Mutex
{
public :

	Mutex();

	virtual ~Mutex();

	void lock();

	void unlock();

private :

#ifdef WIN32
	CRITICAL_SECTION		m_cs;
#else
	pthread_mutex_t			m_mutex;

	friend class ConditionVariable;
#endif

private :

	Mutex( const Mutex & );
	Mutex & operator=( const Mutex & );
};

class MutexLocker
{
public:

	MutexLocker( Mutex& m ) : theMutex( m )
	{
		theMutex.lock();
	}

	virtual ~MutexLocker()
	{
		theMutex.unlock();
	}

private:

	MutexLocker( const MutexLocker& );
	MutexLocker& operator=( const MutexLocker& );

	Mutex & theMutex;
};

#ifndef WIN32

class ConditionVariable {
public:
	ConditionVariable();

	void wait(Mutex& mutex);
	void timedwait(Mutex& mutex, timespec* abstime);
	void notify();

private:
	pthread_cond_t  condition_var;

};  // class ConditionVariable

#endif

}

}

}

#endif // __refinitiv_ema_access_mutex_h
