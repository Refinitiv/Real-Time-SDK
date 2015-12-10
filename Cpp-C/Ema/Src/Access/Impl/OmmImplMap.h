/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmImplMap_h
#define __thomsonreuters_ema_access_OmmImplMap_h

#include "Mutex.h"

namespace thomsonreuters {

namespace ema {

namespace access {

template< class ImplClient >
class OmmImplMap
{
public :

	static UInt64 add( ImplClient* );
	static void remove( ImplClient* );

#ifdef WIN32
	static BOOL WINAPI TermHandlerRoutine( DWORD dwCtrlType );
#else
	static void sigAction( int sig, siginfo_t * pSiginfo, void* pv );
#endif

private :

	static void init();
	static void atExit();

	static Mutex			_listLock;
	static EmaVector< ImplClient* >	_clientList;
	static UInt64			_id;
	static bool			_clearSigHandler;

#ifndef WIN32
	static struct sigaction _sigAction;
	static struct sigaction _oldSigAction;
#endif 
};

template< class ImplClient > EmaVector< ImplClient* > OmmImplMap< ImplClient >::_clientList;
template< class ImplClient > Mutex OmmImplMap< ImplClient >::_listLock;
template< class ImplClient > UInt64 OmmImplMap< ImplClient >::_id = 0;
template< class ImplClient > bool OmmImplMap< ImplClient >::_clearSigHandler = true;

#ifndef WIN32
template< class ImplClient > struct sigaction OmmImplMap< ImplClient >::_sigAction;
template< class ImplClient > struct sigaction OmmImplMap< ImplClient >::_oldSigAction;
#endif

template< class ImplClient >
void OmmImplMap< ImplClient >::init()
{
#ifdef WIN32
  SetConsoleCtrlHandler( &OmmImplMap::TermHandlerRoutine, TRUE ) ;
#else
  bzero( &_sigAction, sizeof( _sigAction ) );
  bzero( &_oldSigAction, sizeof( _oldSigAction ) );

  _sigAction.sa_sigaction = sigAction;
  _sigAction.sa_flags = SA_SIGINFO;

  sigaction( SIGINT, &_sigAction, &_oldSigAction );
#endif

  _clearSigHandler = false;
}

template< class ImplClient >
UInt64 OmmImplMap< ImplClient >::add( ImplClient* impl )
{
  _listLock.lock();
  if ( _clientList.empty() )
    OmmImplMap< ImplClient> ::init();
  _clientList.push_back( impl );
  ++_id;
  _listLock.unlock();
  return _id;
}

template< class ImplClient >
void OmmImplMap< ImplClient >::remove( ImplClient* impl )
{
	_listLock.lock();

	_clientList.removeValue( impl );

	if ( !_clientList.empty() || _clearSigHandler )
	{
		_listLock.unlock();
		return;
	}
#ifdef WIN32
	SetConsoleCtrlHandler( &OmmImplMap::TermHandlerRoutine, FALSE );
#else
	sigaction( SIGINT, &_oldSigAction, NULL );
#endif

	_clearSigHandler = true;

	_listLock.unlock();
}

template< class ImplClient >
void OmmImplMap< ImplClient >::atExit()
{
  _listLock.lock();
  UInt32 size = _clientList.size();
  while ( size )
  {
    _clientList[size - 1 ]->setAtExit();
    _clientList[size - 1 ]->uninitialize();
    size = _clientList.size();
  }
  _listLock.unlock();
}

#ifdef WIN32
template< class ImplClient >
BOOL WINAPI OmmImplMap< ImplClient >::TermHandlerRoutine( DWORD dwCtrlType )
{
  switch ( dwCtrlType )
    {
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_SHUTDOWN_EVENT:	
    case CTRL_C_EVENT:
      OmmImplMap< ImplClient >::atExit();
      break;
    }
  return FALSE;
}
#else
template< class ImplClient >
void OmmImplMap< ImplClient >::sigAction( int, siginfo_t*, void* ) 
{	
  OmmImplMap< ImplClient >::atExit();
}
#endif 

}

}

}
#endif
