///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_iprovider_h_
#define __ema_iprovider_h_

#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

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

class AppClient : public rtsdk::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent&);

	void processMarketPriceRequest(const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent&);

	void processInvalidItemRequest(const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent&);

protected:

	void onReqMsg( const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent& );

};
//API QA
// application defined error client class for receiving and processing of error notifications
class AppErrorClient : public rtsdk::ema::access::OmmProviderErrorClient
{
public:

	void onInvalidHandle(rtsdk::ema::access::UInt64, const rtsdk::ema::access::EmaString&);

	void onInaccessibleLogFile(const rtsdk::ema::access::EmaString&, const rtsdk::ema::access::EmaString&);

	void onSystemError(rtsdk::ema::access::Int64, void*, const rtsdk::ema::access::EmaString&);

	void onMemoryExhaustion(const rtsdk::ema::access::EmaString&);

	void onInvalidUsage(const rtsdk::ema::access::EmaString&, rtsdk::ema::access::Int32);
};
//END API QA
#endif // __ema_iprovider_h_
