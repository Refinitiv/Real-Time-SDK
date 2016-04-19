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
#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

unsigned long long getCurrentTime()
{
	unsigned long long msec = 0;
#ifdef WIN32
	struct	_timeb	_time;
	_ftime_s( &_time );
	msec = _time.time*1000 + _time.millitm;
#else
	struct  timeval _time;
	gettimeofday( &_time, 0 );
	msec = ((unsigned long long)(_time.tv_sec)) * 1000ULL + ((unsigned long long)(_time.tv_usec)) / 1000ULL;
#endif
	return msec;
}

class AppClient : public thomsonreuters::ema::access::OmmConsumerClient		// client receiving messages
{
public :

	void decode( const thomsonreuters::ema::access::FieldList& );			// print content of passed in FieldList to screen

protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );
};

#endif // __ema_consumer_h_
