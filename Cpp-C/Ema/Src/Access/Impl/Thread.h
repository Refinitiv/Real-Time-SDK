/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_thread_h
#define __refinitiv_ema_access_thread_h

#include "Common.h"

#include <assert.h>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <windows.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace rtsdk {

namespace ema {
	
namespace access {

class Thread
{
public :

	virtual bool setPriority( Int64 );

	virtual void sleep( UInt64 sleepTime );

protected:

	virtual void start();

	virtual void stop();

	virtual void wait();

	virtual void run() = 0;

	virtual void cleanUp();

	virtual int runLog( void* pExceptionStructure, const char* file, unsigned int line ) = 0;

	Thread();

	virtual ~Thread();

	bool isStopping();

	void catchUnhandledException( bool );

	int retrieveExceptionContext( void* pExceptionStructure, const char* file, unsigned int line, char* context, unsigned int contextLen );

private:

	bool	_isStopping;
	bool	_isActive;
	int		_priority;
	bool	_handleException;

#ifdef WIN32
	static unsigned int __stdcall runThread( void * arg );
	static int handleException( void * arg, _EXCEPTION_POINTERS*  );

	HANDLE			_handle;
	unsigned int	_threadId;
#else
	static  void *runThread( void * arg );
	
	struct sched_param		_param;
	pthread_attr_t			_attr;
	pthread_t				_threadId;
#endif

	Thread( const Thread& );
	Thread& operator=( const Thread& );
};

}

}

}

#endif // __refinitiv_ema_access_thread_h
