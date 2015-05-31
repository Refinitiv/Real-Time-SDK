//****************************************************//
//** This file is preliminary and subject to change **//
//****************************************************//

///*
// *|---------------------------------------------------------------
// *| Confidential and Proprietary Information of Thomson Reuters.
// *| Copyright Thomson Reuters 2015
// *|---------------------------------------------------------------
// */

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

#include <iostream>
#include <set>

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

class AppClient : public thomsonreuters::ema::access::OmmConsumerClient		// client receiving messages
{
public:

    AppClient( thomsonreuters::ema::access::OmmConsumer& );

private:

	virtual void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	thomsonreuters::ema::access::OmmConsumer& _ommConsumer;
};

#endif // __ema_consumer_h_

