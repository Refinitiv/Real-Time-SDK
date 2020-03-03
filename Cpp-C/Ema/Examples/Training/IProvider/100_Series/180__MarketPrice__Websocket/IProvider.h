///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
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

class AppClient : public thomsonreuters::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const thomsonreuters::ema::access::ReqMsg&, const thomsonreuters::ema::access::OmmProviderEvent&);

	void processMarketPriceRequest(const thomsonreuters::ema::access::ReqMsg&, const thomsonreuters::ema::access::OmmProviderEvent&);

	void processInvalidItemRequest(const thomsonreuters::ema::access::ReqMsg&, const thomsonreuters::ema::access::OmmProviderEvent&);

protected:

	void onReqMsg( const thomsonreuters::ema::access::ReqMsg&, const thomsonreuters::ema::access::OmmProviderEvent& );
};

// application defined error client class for receiving and processing of error notifications
class AppErrorClient : public thomsonreuters::ema::access::OmmProviderErrorClient
{
public:

	void onInaccessibleLogFile( const thomsonreuters::ema::access::EmaString&, const thomsonreuters::ema::access::EmaString& );

	void onSystemError( thomsonreuters::ema::access::Int64, void*, const thomsonreuters::ema::access::EmaString& );

	void onMemoryExhaustion( const thomsonreuters::ema::access::EmaString& );

	void onInvalidUsage( const thomsonreuters::ema::access::EmaString&, thomsonreuters::ema::access::Int32 );

	void onJsonConverter( const thomsonreuters::ema::access::EmaString& text, thomsonreuters::ema::access::Int32 errorCode,
		const thomsonreuters::ema::access::ProviderSessionInfo& sessionInfo );
};

#endif // __ema_iprovider_h_
