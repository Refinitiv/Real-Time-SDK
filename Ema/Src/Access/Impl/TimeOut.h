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

class OmmConsumerImpl;

class TimeOut : public ListLinks< TimeOut >
{
public :

	TimeOut( OmmConsumerImpl & , Int64 , void (*functor)( void * ), void* args = 0 );
	bool operator <( const TimeOut & rhs ) const;
	void operator()() { _functor( _args ); }
	static bool getTimeOutInMicroSeconds( OmmConsumerImpl &, Int64 & );
	static void execute( OmmConsumerImpl &, EmaList<TimeOut> & );
	void cancel() { canceled = true; }

private :

	Int64 _lengthInMicroSeconds;
	void (*_functor)( void * );
	void * _args;
	TimeOutTimeType setAt;
	TimeOutTimeType timeoutTime;
	static TimeOutTimeType frequency;
	bool canceled;	
	TimeOutTimeType setTime() { return setAt; }
	OmmConsumerImpl & theConsumer;
};

}

}

}

#endif // __thomsonreuters_ema_access_TimeOut_h
