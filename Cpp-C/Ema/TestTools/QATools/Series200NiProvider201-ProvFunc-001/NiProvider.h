///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
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

class AppClient : public rtsdk::ema::access::OmmProviderClient
{
public :

        AppClient();
        virtual ~AppClient();

        bool isConnectionUp() const;
		void decode(const rtsdk::ema::access::Msg&, bool complete = false);    // print content of passed in Msg to screen

protected :

        void onRefreshMsg( const rtsdk::ema::access::RefreshMsg&, const rtsdk::ema::access::OmmProviderEvent& );
        void onStatusMsg( const rtsdk::ema::access::StatusMsg&, const rtsdk::ema::access::OmmProviderEvent& );

        bool  _bConnectionUp;
};

#endif // __ema_niprovider_h_
