/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_TimeOut_h
#define __thomsonreuters_ema_access_TimeOut_h

#ifdef WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

#include "EmaList.h"

namespace thomsonreuters {

namespace ema {

namespace access {

#ifdef WIN32
#define TimeOutTimeType LARGE_INTEGER
#else
#define TimeOutTimeType Int64
#endif

class OmmBaseImpl;

class TimeOut : public ListLinks< TimeOut >
{
public:

	static bool getTimeOutInMicroSeconds( OmmBaseImpl&, Int64& );

	static void execute( OmmBaseImpl& );

	TimeOut( OmmBaseImpl&, Int64, void( *functor )( void * ), void* args, bool allocatedOnHeap );

	virtual ~TimeOut();

	bool operator<( const TimeOut& rhs ) const;

	void operator()() { _functor( _args ); }

	void cancel();

private:

	static TimeOutTimeType	frequency;

	void( *_functor )( void * );
	Int64				_lengthInMicroSeconds;
	void*				_args;
	TimeOutTimeType		_timeoutTime;
	bool				_canceled;
	bool				_allocatedOnHeap;
	OmmBaseImpl&		_ommBaseImpl;
};

}

}

}

#endif // __thomsonreuters_ema_access_TimeOut_h
