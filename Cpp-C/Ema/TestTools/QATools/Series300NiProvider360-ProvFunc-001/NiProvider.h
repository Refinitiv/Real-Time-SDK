///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
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

//APIQA
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

void sendDirRefresh(refinitiv::ema::access::OmmProvider& provider);

//END APIQA

class AppClient : public refinitiv::ema::access::OmmProviderClient
{
public :

	AppClient();
	virtual ~AppClient();

	bool isConnectionUp() const;
	
    //APIQA	
	bool sendRefreshMsg() const;
	void sendRefreshMsg(bool sending);
    // END APIQA
	
protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmProviderEvent& );
	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmProviderEvent& );
    void onClose( const refinitiv::ema::access::ReqMsg&, const refinitiv::ema::access::OmmProviderEvent& );
	bool  _bConnectionUp;
	
    //APIQA	
	bool _sendRefreshMsg;
    // END APIQA

};

#endif // __ema_niprovider_h_
