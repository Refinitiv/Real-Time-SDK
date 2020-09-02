/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBaseImpl.h"
#include "TimeOut.h"
#include "Utilities.h"

using namespace rtsdk::ema::access;

#ifdef WIN32
TimeOutTimeType TimeOut::frequency = { 0, 0 };
#endif

TimeOut::TimeOut( TimeOutClient& timeOutClient, Int64 lengthInMicroSeconds, void( *functor )( void* ), void* args, bool allocatedOnHeap ) :
	_functor( functor ),
	_lengthInMicroSeconds( lengthInMicroSeconds ),
	_args( args ),
	_timeoutTime(),
	_canceled( false ),
	_allocatedOnHeap( allocatedOnHeap ),
	_timeOutClient(timeOutClient)
{
	if ( lengthInMicroSeconds == 0 )
	{
		timeOutClient.getTimeOutList().insert(this);
		_canceled = true;
		return;
	}

	TimeOutTimeType setAt;

#ifdef WIN32
	if ( !frequency.QuadPart )
		QueryPerformanceFrequency( &frequency );

	QueryPerformanceCounter( &setAt );
	_timeoutTime.QuadPart = setAt.QuadPart + ( frequency.QuadPart * lengthInMicroSeconds ) / 1000000;
#else
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	setAt = ts.tv_sec * static_cast<int>( 1E9 ) + ts.tv_nsec;
	_timeoutTime = setAt + lengthInMicroSeconds * 1000;
#endif				

	_timeOutClient.getTimeOutList().insert(this);

	_timeOutClient.installTimeOut();
}

TimeOut::~TimeOut()
{
}

bool TimeOut::operator<( const TimeOut& rhs ) const
{
#ifdef WIN32
	return _timeoutTime.QuadPart < rhs._timeoutTime.QuadPart;
#else
	return _timeoutTime < rhs._timeoutTime;
#endif
}

bool TimeOut::getTimeOutInMicroSeconds(TimeOutClient& timeOutClient, Int64& value)
{
	MutexLocker ml(timeOutClient.getTimeOutMutex());

	EmaList< TimeOut* >& _theTimeOuts(timeOutClient.getTimeOutList());

	if ( _theTimeOuts.empty() )
		return false;

	TimeOut* p( _theTimeOuts.front() );
	while ( true )
	{
		if ( !p )
			return false;

		if ( p->_canceled )
		{
			TimeOut * toBeDeleted( p );
			p = p->next();
			_theTimeOuts.remove( toBeDeleted );
			if ( toBeDeleted->_allocatedOnHeap )
				delete toBeDeleted;
		}
		else if ( !p->_lengthInMicroSeconds )
			p = p->next();
		else
			break;
	}

	TimeOutTimeType current;

#ifdef WIN32
	QueryPerformanceCounter( &current );
	if ( p->_timeoutTime.QuadPart < current.QuadPart )
		value = 0;
	else
		value = static_cast<Int64> ( ( p->_timeoutTime.QuadPart - current.QuadPart ) * 1000000 / frequency.QuadPart );
#else
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	current = ts.tv_sec * static_cast<int>( 1E9 ) + ts.tv_nsec;
	if ( p->_timeoutTime < current )
		value = 0;
	else
		value = static_cast<Int64> ( ( p->_timeoutTime - current ) / 1000 );
#endif
	return true;
}

void TimeOut::cancel()
{
	MutexLocker ml(_timeOutClient.getTimeOutMutex());

	_canceled = true;

	EmaList< TimeOut* > & _theTimeOuts(_timeOutClient.getTimeOutList());

	_theTimeOuts.remove( this );

	if ( _allocatedOnHeap ) delete this;
}

bool TimeOut::isCanceled()
{
	return _canceled;
}

void TimeOut::execute( TimeOutClient& timeOutClient )
{
	MutexLocker ml(timeOutClient.getTimeOutMutex());

	TimeOutTimeType current;
#ifdef WIN32
	QueryPerformanceCounter( &current );
#else
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	current = ts.tv_sec * static_cast<int>( 1E9 ) + ts.tv_nsec;
#endif

	TimeOut * p(timeOutClient.getTimeOutList().front());
	while ( p )
	{
#ifdef WIN32
		if ( current.QuadPart >= p->_timeoutTime.QuadPart )
#else
		if ( current >= p->_timeoutTime )
#endif
		{
			if ( !p->_canceled )
				( *p )( );
			if ( p->_allocatedOnHeap )
				delete timeOutClient.getTimeOutList().pop_front();
			else
				timeOutClient.getTimeOutList().pop_front();
			p = timeOutClient.getTimeOutList().front();
		}
		else
			return;
	}
}

TimeOutClient::TimeOutClient()
{

}
TimeOutClient::~TimeOutClient()
{
}
