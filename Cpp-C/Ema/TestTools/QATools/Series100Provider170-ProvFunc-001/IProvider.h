/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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

class AppClient : public refinitiv::ema::access::OmmProviderClient
{
public:
  void processLoginRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);
  void processMarketPriceRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);
  void processInvalidItemRequest(const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent&);

protected:
  void onReqMsg( const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent& );
  void onClose(const refinitiv::ema::access::ReqMsg& reqMsg,
			   const refinitiv::ema::access::OmmProviderEvent& event);
};
//API QA
// application defined error client class for receiving and processing of error notifications
class AppErrorClient : public refinitiv::ema::access::OmmProviderErrorClient
{
public:

	void onInvalidHandle(refinitiv::ema::access::UInt64, const refinitiv::ema::access::EmaString&);

	void onInaccessibleLogFile(const refinitiv::ema::access::EmaString&, const refinitiv::ema::access::EmaString&);

	void onSystemError(refinitiv::ema::access::Int64, void*, const refinitiv::ema::access::EmaString&);

	void onMemoryExhaustion(const refinitiv::ema::access::EmaString&);

	void onInvalidUsage(const refinitiv::ema::access::EmaString&, refinitiv::ema::access::Int32);
};
//END API QA

#endif // __ema_iprovider_h_
