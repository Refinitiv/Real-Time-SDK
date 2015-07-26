/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Thread.h"
#include "Utilities.h"

using namespace thomsonreuters::ema::access;

Thread::Thread() :
 _isStopping( false ),
 _isActive( false ),
 _priority( 0 ),
 _handleException( true ),
#ifdef WIN32
 _handle( 0 ),
#endif
 _threadId( 0 )
{
}

void Thread::catchUnhandledException( bool handleException )
{
	_handleException = handleException;
}

Thread::~Thread()
{
#ifdef WIN32
	assert( _handle == 0 );
#else
	assert( _threadId == 0 );
#endif
}

#ifdef WIN32
unsigned int __stdcall Thread::runThread( void* arg )
{
	__try {
		((Thread*)arg)->run();
	}
	__except( ((Thread*)arg)->_handleException ? ((Thread*)arg)->runLog( GetExceptionInformation(), __FILE__, __LINE__ ) : EXCEPTION_CONTINUE_SEARCH )
	{
		if ( ((Thread*)arg)->_handleException )
		{
			((Thread*)arg)->cleanUp();
		}

		return -1;
	}

	return 0;
}
		
#else
extern "C" 
{
	void *Thread::runThread( void* arg)
	{
		try {
			((Thread*)arg)->run();
		}
		catch ( ... )
		{
			((Thread*)arg)->runLog( NULL, __FILE__, __LINE__ );
			if (((Thread*)arg)->_handleException)
			{
				((Thread*)arg)->cleanUp();
			 	return (void*)-1;
			}
			else
			  throw;
		}

		return 0;
	}
}
#endif

void Thread::cleanUp()
{
}

void Thread::start()
{
	_isStopping = false;

#ifdef WIN32
	_handle = (HANDLE)_beginthreadex( NULL, 0, runThread, this, 0, &_threadId );
	assert( _handle != 0 );

	SetThreadPriority( _handle, _priority );
#else
	pthread_create( &_threadId, NULL, runThread, this );
	assert( _threadId != 0 );
#endif

	_isActive = true;
}

void Thread::stop()
{
	_isStopping = true;
}

void Thread::wait()
{
	if ( _isActive )
	{
		_isActive = false;
#ifdef WIN32
		WaitForSingleObject( _handle, INFINITE );
		CloseHandle( _handle );
		_handle = 0;
		_threadId = 0;
#else
		pthread_join( _threadId, NULL );
		_threadId = 0;
#endif
	}
}

bool Thread::isStopping()
{
	return _isStopping;
}

bool Thread::setPriority( Int64 pri )
{
	_priority = (int)(pri);

#ifndef WIN32
  	pthread_attr_init( &_attr );
	_param.sched_priority = _priority;
	pthread_attr_setschedparam( &_attr, &_param );
#endif

	return false;
}

void Thread::sleep( UInt64 millisecs )
{
#ifdef WIN32
    ::Sleep( (DWORD)(millisecs) );
#else
    struct timespec sleeptime;
    sleeptime.tv_sec = millisecs / 1000;
    sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep(&sleeptime,0);
#endif
}

int Thread::retrieveExceptionContext( void* pExceptionStructure, 
										const char* file,
										unsigned int line,
										char* reportBuffer,
										unsigned int reportBufferLen )
{
#ifdef WIN32
	return emaExceptionFilter( (LPEXCEPTION_POINTERS)pExceptionStructure, file, line, reportBuffer, reportBufferLen );
#else
	return emaProblemReport( (void*)NULL, file, line, "Exception occured", reportBuffer, reportBufferLen );
#endif
}



