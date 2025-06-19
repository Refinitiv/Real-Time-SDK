/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __ema_niprovider_h_
#define __ema_niprovider_h_

#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

class AppClient : public refinitiv::ema::access::OmmProviderClient
{
public :

	AppClient();
	virtual ~AppClient();

	bool isConnectionUp() const;

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmProviderEvent& );
	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmProviderEvent& );

	bool  _bConnectionUp;
};

#endif // __ema_niprovider_h_
