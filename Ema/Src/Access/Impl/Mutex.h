/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_mutex_h
#define __thomsonreuters_ema_access_mutex_h

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
#endif

namespace thomsonreuters {

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
#endif

private :

	Mutex( const Mutex & );
	Mutex & operator=( const Mutex & );
};

class MutexLocker
{
public:

	MutexLocker( Mutex & m ) : theMutex( m )
	{
		theMutex.lock();
	}

	~MutexLocker()
	{
		theMutex.unlock();
	}

private:

	Mutex & theMutex;
};

}

}

}

#endif // __thomsonreuters_ema_access_mutex_h
