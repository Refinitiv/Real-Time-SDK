///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

#include <iostream>

#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

unsigned long long getCurrentTime()
{
	unsigned long long msec = 0;
#ifdef WIN32
	struct	_timeb	_time;
	_ftime_s( &_time );
	msec = _time.time*1000 + _time.millitm;
#else
	struct  timeval _time;
	gettimeofday( &_time, 0 );
	msec = ((unsigned long long)(_time.tv_sec)) * 1000ULL + ((unsigned long long)(_time.tv_usec)) / 1000ULL;
#endif
	return msec;
}

// application defined client class for receiving and processing of item messages
class AppClient : public thomsonreuters::ema::access::OmmConsumerClient
{
public :

	void decode( const thomsonreuters::ema::access::FieldList& );			// print content of passed in FieldList to screen

protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );
};

// application defined error client class for receiving and processing of error notifications
class AppErrorClient : public thomsonreuters::ema::access::OmmConsumerErrorClient
{
public :

	void onInvalidHandle( thomsonreuters::ema::access::UInt64, const thomsonreuters::ema::access::EmaString& );

	void onInaccessibleLogFile( const thomsonreuters::ema::access::EmaString&, const thomsonreuters::ema::access::EmaString& );

	void onSystemError( thomsonreuters::ema::access::Int64, void* , const thomsonreuters::ema::access::EmaString& );
	
	void onMemoryExhaustion( const thomsonreuters::ema::access::EmaString& );
	
	void onInvalidUsage( const thomsonreuters::ema::access::EmaString&, thomsonreuters::ema::access::Int32 );
};

#endif // __ema_consumer_h_
