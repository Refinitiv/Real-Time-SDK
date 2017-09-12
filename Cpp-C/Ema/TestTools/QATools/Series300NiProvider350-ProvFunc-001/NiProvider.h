///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2017. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_niprovider_h_
#define __ema_niprovider_h_

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

class AppClient : public thomsonreuters::ema::access::OmmProviderClient
{
public:

	void decode(const thomsonreuters::ema::access::Msg&, bool complete = false);	// print content of passed in Msg to screen

protected:

	void onRefreshMsg(const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmProviderEvent&);

	void onStatusMsg(const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmProviderEvent&);
};


#endif // __ema_niprovider_h_
