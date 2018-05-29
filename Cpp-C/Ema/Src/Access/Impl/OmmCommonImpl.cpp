/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2015. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "OmmBaseImplMap.h"

using namespace thomsonreuters::ema::access;

#ifdef USING_POLL
int OmmCommonImpl::addFd( int fd, short events )
{
	if ( _eventFdsCount == _eventFdsCapacity )
	{
		_eventFdsCapacity *= 2;
		pollfd* tmp( new pollfd[ _eventFdsCapacity ] );
		for ( int i = 0; i < _eventFdsCount; ++i )
			tmp[ i ] = _eventFds[ i ];
		delete [] _eventFds;
		_eventFds = tmp;
	}
	_eventFds[ _eventFdsCount ].fd = fd;
	_eventFds[ _eventFdsCount ].events = events;

	return _eventFdsCount++;
}

void OmmCommonImpl::removeFd( int fd )
{
  _pipeReadEventFdsIdx = -1;

  int i = 0;
  while (i < _eventFdsCount) {
	if (_eventFds[i].fd == fd)
	  break;
	++i;
  }

  if (i == _eventFdsCount)		// did not find the fd in the list
	return;
  else if (i == _eventFdsCount - 1) { // removed last item
	--_eventFdsCount;
	return;
  }

  --_eventFdsCount;
  while (i < _eventFdsCount)
	_eventFds[i] = _eventFds[++i];
}
#endif

