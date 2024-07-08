///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
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
public:

	void decode( const refinitiv::ema::access::Data& );					// print content of passed in Data to screen

	void decodeFieldList( const refinitiv::ema::access::FieldList& );	// print content of passed in FieldList to screen

	void decodeMap( const refinitiv::ema::access::Map& );				// print content of passed in Map to screen

	void decodeRefreshMsg( const refinitiv::ema::access::RefreshMsg& );	// print content of passed in RefreshMsg to screen

	void decodeGenericMsg( const refinitiv::ema::access::GenericMsg& );	// print content of passed in GenericMsg to screen

	void decodeUpdateMsg( const refinitiv::ema::access::UpdateMsg& );	// print content of passed in UpdateMsg to screen

	void decodeStatusMsg( const refinitiv::ema::access::StatusMsg& );	// print content of passed in StatusMsg to screen

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );
};

#endif // __ema_consumer_h_
