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
	msec = ((unsigned long long)(_time.tv_sec))*1000ULL + ((unsigned long long)(_time.tv_usec)) / 1000ULL;
#endif
	return msec;
}

// application defined client class for receiving and processing of item messages
class AppClient : public refinitiv::ema::access::OmmConsumerClient
{
public :

	AppClient();

	void decode( const refinitiv::ema::access::FieldList& );			// print content of passed in FieldList to screen

	void setOmmConsumer( refinitiv::ema::access::OmmConsumer& );

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );
	
	refinitiv::ema::access::OmmConsumer* _pOmmConsumer;

	bool hasFirstRefresh;
};

#endif // __ema_consumer_h_
