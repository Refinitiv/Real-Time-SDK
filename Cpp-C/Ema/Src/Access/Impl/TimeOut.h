/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_TimeOut_h
#define __refinitiv_ema_access_TimeOut_h

#ifdef WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

#include "EmaList.h"
#include "Mutex.h"

namespace refinitiv {

namespace ema {

namespace access {

#ifdef WIN32
#define TimeOutTimeType LARGE_INTEGER
#else
#define TimeOutTimeType Int64
#endif

class TimeOutClient;

class TimeOut : public ListLinks< TimeOut >
{
public:

	static bool getTimeOutInMicroSeconds(TimeOutClient&, Int64&);

	static void execute(TimeOutClient&);

	TimeOut(TimeOutClient&, Int64, void(*functor)(void *), void* args, bool allocatedOnHeap);

	virtual ~TimeOut();

	bool operator<( const TimeOut& rhs ) const;

	void operator()() { _functor( _args ); }

	void cancel();

	bool isCanceled();

private:

	static TimeOutTimeType	frequency;

	void( *_functor )( void * );
	Int64				_lengthInMicroSeconds;
	void*				_args;
	TimeOutTimeType		_timeoutTime;
	bool				_canceled;
	bool				_allocatedOnHeap;
	TimeOutClient&		_timeOutClient;
};

class TimeOutClient
{
public:
	TimeOutClient();
	virtual ~TimeOutClient();

	virtual EmaList< TimeOut* >& getTimeOutList() = 0;
	virtual void installTimeOut() = 0;
	virtual Mutex& getTimeOutMutex() = 0;


private:
	TimeOutClient(const TimeOutClient&);
	TimeOutClient& operator=(const TimeOutClient&);

};


}

}

}

#endif // __refinitiv_ema_access_TimeOut_h
