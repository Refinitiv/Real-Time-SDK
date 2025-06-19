/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __ema_iprovider_h_
#define __ema_iprovider_h_

#include <iostream>

#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

unsigned long long getCurrentTime()
{
	unsigned long long msec = 0;
#ifdef WIN32
	struct	_timeb	_time;
	_ftime_s(&_time);
	msec = _time.time * 1000 + _time.millitm;
#else
	struct  timeval _time;
	gettimeofday(&_time, 0);
	msec = ((unsigned long long)(_time.tv_sec)) * 1000ULL + ((unsigned long long)(_time.tv_usec)) / 1000ULL;
#endif
	return msec;
}

void sleep( int millisecs )
{
#if defined WIN32
	::Sleep( ( DWORD )( millisecs ) );
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = ( millisecs % 1000 ) * 1000000;
	nanosleep( &sleeptime, 0 );
#endif
}

class AppClient : public refinitiv::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	void processMarketPriceRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	void processInvalidItemRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	void decode(const refinitiv::ema::access::Msg&, bool complete = false);

protected:

	void onReqMsg( const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent& );

	void onPostMsg(const refinitiv::ema::access::PostMsg&, const refinitiv::ema::access::OmmProviderEvent& );

};

#endif // __ema_iprovider_h_
