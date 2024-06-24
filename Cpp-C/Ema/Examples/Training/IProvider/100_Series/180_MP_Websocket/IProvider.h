///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2020 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#ifndef __ema_iprovider_h_
#define __ema_iprovider_h_

#include <iostream>

#include "Ema.h"

#ifdef WIN32
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

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

class AppClient : public refinitiv::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	void processMarketPriceRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

	void processInvalidItemRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

protected:

	void onReqMsg( const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent& );
};

// application defined error client class for receiving and processing of error notifications
class AppErrorClient : public refinitiv::ema::access::OmmProviderErrorClient
{
public:

	void onInaccessibleLogFile( const refinitiv::ema::access::EmaString&, const refinitiv::ema::access::EmaString& );

	void onSystemError( refinitiv::ema::access::Int64, void*, const refinitiv::ema::access::EmaString& );

	void onMemoryExhaustion( const refinitiv::ema::access::EmaString& );

	void onInvalidUsage( const refinitiv::ema::access::EmaString&, refinitiv::ema::access::Int32 );

	void onJsonConverter( const refinitiv::ema::access::EmaString& text, refinitiv::ema::access::Int32 errorCode,
		const refinitiv::ema::access::ProviderSessionInfo& sessionInfo );
};

#endif // __ema_iprovider_h_
