/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
// APIQA:
#include <string.h>
// END APIQA:
#endif

#include "Ema.h"

void sleep( int millisecs )
{
#if defined WIN32
	::Sleep( (DWORD)(millisecs) );
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep( &sleeptime, 0 );
#endif
}

// application defined client class for receiving and processing of item messages
class AppClient : public refinitiv::ema::access::OmmConsumerClient
{
public :

	AppClient();

	virtual ~AppClient();

	void setOmmConsumer( refinitiv::ema::access::OmmConsumer& );

	void setTunnelStreamHandle( refinitiv::ema::access::UInt64 );
    // APIQA: subitem handle
    refinitiv::ema::access::UInt64			_subItemHandle;
    // END APIQA:

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );
	// APIQA: GenericMsg handler
	void onGenericMsg( const refinitiv::ema::access::GenericMsg&, const refinitiv::ema::access::OmmConsumerEvent& );
	// END APIQA:
	refinitiv::ema::access::OmmConsumer*	_pOmmConsumer;

	refinitiv::ema::access::UInt64			_tunnelStreamHandle;

	bool										_bSubItemOpen;
};

#endif // __ema_consumer_h_
