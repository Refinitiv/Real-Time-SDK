///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_niprovider_h_
#define __ema_niprovider_h_

#include <iostream>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

using namespace rtsdk::ema::access;

void sleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}

UInt64 getCurrentTime()
{
	UInt64 msec = 0;
#ifdef WIN32
	struct	_timeb	_time;
	_ftime_s(&_time);
	msec = _time.time;
#else
	struct  timeval _time;
	gettimeofday(&_time, 0);
	msec = ((unsigned long long)(_time.tv_sec));
#endif
	return msec;
}

class AppLoginClient : public rtsdk::ema::access::OmmProviderClient
{
public:
	AppLoginClient();

	UInt64 _handle;
	UInt64 _TTReissue;
protected:
	void onRefreshMsg(const rtsdk::ema::access::RefreshMsg&, const rtsdk::ema::access::OmmProviderEvent&);

	void onUpdateMsg(const rtsdk::ema::access::UpdateMsg&, const rtsdk::ema::access::OmmProviderEvent&);

	void onStatusMsg(const rtsdk::ema::access::StatusMsg&, const rtsdk::ema::access::OmmProviderEvent&);
};


#endif // __ema_niprovider_h_
