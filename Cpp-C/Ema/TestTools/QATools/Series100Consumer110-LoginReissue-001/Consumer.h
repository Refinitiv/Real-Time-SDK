///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

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
	::Sleep( (DWORD)(millisecs) );
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep( &sleeptime, 0 );
#endif
}

// application defined client class for receiving and processing of item messages
class AppClient : public thomsonreuters::ema::access::OmmConsumerClient
{
public :

	AppClient();

	void decode( const thomsonreuters::ema::access::FieldList& );				// print content of passed in FieldList to screen

	void setOmmConsumer( thomsonreuters::ema::access::OmmConsumer&, thomsonreuters::ema::access::UInt64 );

protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	thomsonreuters::ema::access::OmmConsumer* _pOmmConsumer;

	thomsonreuters::ema::access::UInt64 _updateNumber;

	thomsonreuters::ema::access::UInt64 _loginHandle;
};

#endif // __ema_consumer_h_
