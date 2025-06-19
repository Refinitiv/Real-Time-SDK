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
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
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

	void decode( const refinitiv::ema::access::Msg&, bool complete = false );				// print content of passed in Msg to screen
	
	void decode( const refinitiv::ema::access::FieldList& );		// print content of passed in FieldList to screen

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );
};

#endif // __ema_consumer_h_
