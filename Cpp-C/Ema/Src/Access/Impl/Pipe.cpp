/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include  "Pipe.h"

#ifdef WIN32
#include  <WinSock2.h>
#else
#include  <unistd.h>
#include  <fcntl.h>
#include  <sys/types.h>
#endif

using namespace thomsonreuters::ema::access;

Pipe::Pipe() :
 _initialized( false )
{
	_fds[0] = -1;
	_fds[1] = -1;
}

Pipe::~Pipe()
{
	close();
}

bool Pipe::create( int winPipePort )
{
#ifdef WIN32
	WSADATA wsData;
	int wsStartupRetVal = WSAStartup( MAKEWORD( 2, 2 ), &wsData );

	static const int PORT_SPAN = 1000;
	static const int PORT_MIN = 1024;
	static const int PORT_MAX = 65535;

	int PORT_ADJ = winPipePort;
	if ( winPipePort < PORT_MIN || winPipePort > PORT_MAX ) 
	{
		winPipePort = WinPipePort_Default;
		PORT_ADJ = WinPipePort_Default;
	}
	else if ( winPipePort - PORT_SPAN < PORT_MIN )
		PORT_ADJ = PORT_MIN + PORT_SPAN;
	else if ( winPipePort + PORT_SPAN > PORT_MAX )
		PORT_ADJ = PORT_MAX - PORT_SPAN;

	if ( !socketpair( _fds[0], _fds[1], winPipePort, PORT_ADJ - PORT_SPAN, PORT_ADJ + PORT_SPAN ) )
	{
		close();
		return false;
	}

	u_long arg = 1;
	if ( 0 != ioctlsocket( _fds[0], FIONBIO, &arg ) )
		return false;

	if ( 0 != ioctlsocket( _fds[1], FIONBIO, &arg ) )
		return false;

	char flag = 1;
	if ( 0 != setsockopt( _fds[0], IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(char) ) )
		return false;

	if ( 0 != setsockopt( _fds[1], IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(char) ) )
		return false;

#else
	if ( -1 == pipe( _fds ) )
	{
		close();
		return false;
	}

	if ( 0 > fcntl( _fds[0], F_SETFL, ( fcntl( _fds[0], F_GETFL) | O_NONBLOCK ) ) )
		return false;
#endif
		
	return _initialized = true;
}

void Pipe::close()
{
	if ( _initialized )
	{
		closeFD( 0 );
		closeFD( 1 );

#ifdef WIN32
		WSACleanup();
#endif

		_initialized = false;
	}
}

int Pipe::read( void* buffer, int length )
{
#ifdef WIN32

	int retryCount = 500;
	int retVal = SOCKET_ERROR;
	int dataLength = length;
	char* dataBuf = (char *)buffer;
	do
	{
		if ( retVal != SOCKET_ERROR )
		{
			dataLength -= retVal;
			dataBuf += retVal;
		}
		retVal = ::recv( _fds[0], dataBuf, dataLength, 0 );
		if ( retVal == SOCKET_ERROR )
		{
			if ( WSAGetLastError() != WSAEWOULDBLOCK )
				return retVal;
		}
	}
	while ( dataLength > 0 && retVal != dataLength && ( Sleep(0), retryCount-- ) );

	return retVal <= 0 ? retVal : length;
#else
	return ::read( _fds[0], buffer, length );
#endif	
}

int	Pipe::write( const void* buffer, int length )
{
#ifdef WIN32
	return ::send( _fds[1], (const char *)buffer, length, 0 );
#else
	return ::write( _fds[1], buffer, length );
#endif
}

void Pipe::closeFD( int idx )
{
	if ( _fds[idx] != -1 )
	{
#ifdef WIN32
		::closesocket( _fds[idx] );
#else
		::close( _fds[idx] );
#endif
		_fds[idx] = -1;
	}
}

#ifdef WIN32
bool Pipe::socketpair( int& appFD, int& threadFD, int& port, 
				 	    int startport, int maxport )
{
	const int PORT_REQ = port;
	if (( port < startport ) || ( port > maxport ) )
		port = startport; 

	SOCKET serverFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

	struct sockaddr_in mySAddr, cliSAddr;
	mySAddr.sin_family = AF_INET;
	mySAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

	int status = -1;
	int nAttempts = 0;
	int direction = 1;
	while ( status == -1 )
	{
		nAttempts++;
		mySAddr.sin_port = htons( port );

#ifdef SO_EXCLUSIVEADDRUSE
		int reuseExclusiveFlag = 1;
		setsockopt( serverFD, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&reuseExclusiveFlag, (int)sizeof(reuseExclusiveFlag) );
#endif
		status = bind( serverFD, (struct sockaddr*) &mySAddr, sizeof mySAddr );	
		if ( nAttempts > maxport - startport )
		{
			return false;
		}
		if ( status == -1 )
		{
			port += direction;
			if ( port > maxport )
			{
				port = PORT_REQ - 1;
				direction = -1;
			}
		}
	}

	listen( serverFD, 1 );
	int SAddrSize = sizeof( struct sockaddr_in );

	struct sockaddr_in srvrSAddr;
#pragma warning( disable : 4244 )
	threadFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );
	srvrSAddr.sin_family = AF_INET;
	srvrSAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	srvrSAddr.sin_port = htons( port );

	status = connect( threadFD, (struct sockaddr* ) &srvrSAddr, 
						  sizeof srvrSAddr );
	if ( status == -1 )
	{
		closesocket( serverFD ); 
		return false;
	}
		
	appFD = accept( serverFD, (struct sockaddr *)&cliSAddr, &SAddrSize );
#pragma warning( default : 4244 )
	if ( appFD == -1 )
	{
		closesocket( serverFD ); 
		return false;
	}
	
	closesocket( serverFD ); 
	return true;
}

#endif
