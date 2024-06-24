/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

//	Description: a C pipe implementation
//
//	Object description:
//	rssl_pipe:Pipe struct
//		This needs to be initialized before create is called
//		_fds[0] contains the read file descriptor
//		_fds[1] contains the write file descriptor
//		_initialized is a boolean that determines if the pipe is valid
//
//	Function description:
//	rssl_pipe_create:Pipe initialization
//		Input: _pipe: pointer to a pipe struct
//		Initializes the pipe.  This is done only when _initialized is 0.
//		Returns 1 if the pipe is sucessfully created, and 0 if not.
//
//	rssl_pipe_read:
//		Input:  _pipe: pointer to a pipe struct
//				buffer: buffer for place the read information
//				length: number of bytes to be read
//		Reads length bytes from the pipe, and places it into the buffer.
//		Returns either the length read from the pipe, or a POSIX/Winsock error value.
//
//	rssl_pipe_write:
//		Input:	_pipe: pointer to a pipe struct
//				buffer: buffer to read the information that's going to be written
//				length: number of bytes to be written
//		 Attempts to write lengh bytes from the buffer to the pipe.
//		 Returns either the number of bytes added to the pipe, or a POSIX/Winsock error.
//
//	rssl_pipe_close:
//		Input:	_pipe: pointer to a pipe struct
//		Closes the pipe.  _initialized is set to 0 after this is done.


#ifndef _rssl_pipe_h 
#define _rssl_pipe_h

#include "rtr/os.h"
#include "rtr/socket.h"
#include "rtr/ripcutils.h"

#ifdef _WIN32
#else
#include  <unistd.h>
#include  <fcntl.h>
#include  <sys/types.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

// Pipe struct
// This needs to be initialized before create is called
// _fds[0] contains the read file descriptor
// _fds[1] contains the write file descriptor
// _initialized is a boolean that determines if the pipe is valid
typedef struct
{
	SOCKET	_fds[2];
	int	_initialized;
} rssl_pipe;

#define RTR_INIT_PIPE {-1,-1, 0}

RTR_C_INLINE void rssl_pipe_init(rssl_pipe* _pipe) {
	_pipe->_fds[0] = -1;
	_pipe->_fds[1] = -1; 
	_pipe->_initialized = 0;
}

RTR_C_INLINE SOCKET rssl_pipe_get_read_fd(rssl_pipe* _pipe) {
	if ( _pipe->_initialized)
		return _pipe->_fds[0];
	else
		return 0;
}

RTR_C_INLINE SOCKET rssl_pipe_get_write_fd(rssl_pipe* _pipe) {
	if ( _pipe->_initialized)
		return _pipe->_fds[1];
	else
		return 0;
}

// Closes the pipe.
RTR_C_INLINE void rssl_pipe_close(rssl_pipe* _pipe) {
	if( _pipe->_initialized )
	{
		if(_pipe->_fds[0] != -1 ) {
#ifdef _WIN32
		closesocket( _pipe->_fds[0] );
#else
		close( _pipe->_fds[0] );
#endif
		_pipe->_fds[0] = -1;
		}

		if(_pipe->_fds[1] != -1) {
#ifdef _WIN32
		closesocket( _pipe->_fds[1] );
#else
		close( _pipe->_fds[1] );
#endif
		_pipe->_fds[1] = -1;
		}

		rssl_socket_shutdown();

		_pipe->_initialized = 0;
	}
}

// Creates and initializes the pipe.
// Returns 1 if the pipe is sucessfully created, and 0 if not.
RTR_C_INLINE int rssl_pipe_create(rssl_pipe* _pipe) {
#ifdef _WIN32
	int port;
	int flag;
	int ioctlRetVal;
	int setTCPNoDelayRetVal;
	u_long	arg;
	int status;
	int nAttempts;
	SOCKET serverFD;
	struct sockaddr_in mySAddr, cliSAddr;
	int SAddrSize;
	struct sockaddr_in srvrSAddr;
	char			eText[128];


	if (rssl_socket_startup(eText) < 0)
		return 0;
	
	// create and bind the two file descriptors.
	port = 55000;

	//loop until we find an unused port to bind to 
	status = -1;
	nAttempts = 0;
	while ( status == -1 )
	{
		nAttempts++;

		if ( port > 55050 )
			port = 54950;
		
		if ( nAttempts > 100 )
		{
			return 0;
		}

		//set up a socket and act as a server

		serverFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

		mySAddr.sin_family = AF_INET;
		mySAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );


		// If any of bind/listen/connect/accept fail, try again with a different port.
		// Failure of connect has been observed while using firewalls despite a successful
		// bind and listen(the connection was refused).

		mySAddr.sin_port = htons( port );
		status = bind( serverFD, (struct sockaddr*) &mySAddr, sizeof mySAddr );

		if ( status == -1 )
		{
			closesocket(serverFD);
			++port;
			continue;
		}

		status = listen( serverFD, 1 );
		if (status == -1)
		{
			closesocket(serverFD);
			++port;
			continue;
		}

		SAddrSize = sizeof( struct sockaddr_in );

		_pipe->_fds[1] = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

		srvrSAddr.sin_family = AF_INET;
		srvrSAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
		srvrSAddr.sin_port = htons( port );

		status = connect( _pipe->_fds[1], (struct sockaddr* ) &srvrSAddr, 
				sizeof srvrSAddr );

		if (status == -1)
		{
			closesocket(_pipe->_fds[1]);
			closesocket(serverFD);
			++port;
			continue;
		}

		_pipe->_fds[0] = accept( serverFD, (struct sockaddr *)&cliSAddr, &SAddrSize );
		
		if (_pipe->_fds[0] == -1)
		{
			closesocket(_pipe->_fds[1]);
			closesocket(serverFD);
			++port;
			continue;
		}


	}

	closesocket( serverFD );

	arg = 1;
	ioctlRetVal = ioctlsocket( _pipe->_fds[0], FIONBIO, &arg );
	if(ioctlRetVal != 0) {
		rssl_pipe_close(_pipe);
		return 0;
	}

	arg = 1;
	ioctlRetVal = ioctlsocket( _pipe->_fds[1], FIONBIO, &arg );
	if(ioctlRetVal != 0) {
		rssl_pipe_close(_pipe);
		return 0;
	}

	flag = 1;
	setTCPNoDelayRetVal = setsockopt( _pipe->_fds[0], IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int));
	if(setTCPNoDelayRetVal != 0) {
		rssl_pipe_close(_pipe);
		return 0;
	}

	flag = 1;
	setTCPNoDelayRetVal = setsockopt( _pipe->_fds[1], IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int));
	if(setTCPNoDelayRetVal != 0) {
		rssl_pipe_close(_pipe);
		return 0;
	}
#else
	int pipeRetVal;
	int fcntlRetVal;
	pipeRetVal = pipe( _pipe->_fds );
	if(pipeRetVal == -1) {
		// Cannot create the communication pipe
		return 0;
	}

	fcntlRetVal = fcntl(_pipe->_fds[0],F_SETFL, (fcntl( _pipe->_fds[0], F_GETFL) | O_NONBLOCK));
	if(fcntlRetVal != 0) {
		//Cannot set non blocking option on th ecomminication pipe
		rssl_pipe_close(_pipe);
		return 0;
	}

#endif

	_pipe->_initialized = 1;
	return 1;
}

// Attempts to write lengh bytes from the buffer to the pipe.
// Returns either the number of bytes added to the pipe, or a POSIX/Winsock error.
RTR_C_INLINE int rssl_pipe_write(rssl_pipe* _pipe, const void * buffer, size_t length ) {
#ifdef _WIN32
	int retVal;
	retVal = send( _pipe->_fds[1], ( const char *)buffer, (int)length, 0 );

	if( retVal != length )
	{
		int lastError = WSAGetLastError();
	}

	return retVal;
#else
	return write( _pipe->_fds[1], buffer, length );
#endif
}

// Reads length bytes from the pipe, and places it into the buffer.
// Returns either the length read from the pipe, or a POSIX/Winsock error value.
RTR_C_INLINE int rssl_pipe_read(rssl_pipe* _pipe, void * buffer, size_t length ) {
#ifdef _WIN32

	int retryCount = 50000;
	int retVal = -1;
	int dataLength = (int)length;
	char *dataBuf = ( char *)buffer;
	do
	{
		if( retVal > 0 )	// Handle partial reads
		{
			dataLength -= retVal;
			dataBuf += retVal;
		}
		retVal = recv( _pipe->_fds[0], ( char *)dataBuf, dataLength, 0 );
		if ( retVal < 0 ) {
			// sometimes, the byte we are expecting is not here yet even though we have been told by our select() it is here.
			// this code does a "controlled delay" before the next try of recv(). this should give enough time for the byte to go thru pipe
			// the expected error code here is WSAEWOULDBLOCK (10035) which means that the pipe did not return the byte that we think is in it
			if (WSAGetLastError() != WSAEWOULDBLOCK) 
				return retVal;
		}
	}
	while( dataLength > 0 && retVal != dataLength && ( Sleep(0), retryCount-- )  );

	return retVal <= 0 ? retVal : (int)length;
#else
	return read( _pipe->_fds[0], buffer, length );
#endif	
}
#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

