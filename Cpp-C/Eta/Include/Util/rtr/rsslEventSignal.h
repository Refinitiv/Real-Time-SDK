/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* Contains functionality for creating a file descriptor to use for notification signals.
 * These functions are not thread-safe -- the owner of the object is expected to
 * have locked around these calls to correctly check whatever condition 
 * necessitates setting or resetting the descriptor. */

#ifndef RSSL_EVENT_FD_H
#define RSSL_EVENT_FD_H

#include "rtr/os.h"

#ifdef _WIN32
#else
#include  <unistd.h>
#include  <fcntl.h>
#include  <sys/types.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	RsslSocket	_fds[2];
	int _byteWritten;
} RsslEventSignal;

RTR_C_INLINE void rsslClearEventSignal(RsslEventSignal *pSignal)
{
	pSignal->_fds[0] = -1;
	pSignal->_fds[1] = -1; 
}

RTR_C_INLINE int rsslGetEventSignalFD(RsslEventSignal* pSignal) {
	return (int)(pSignal->_fds[0]);
}

RTR_C_INLINE void rsslCleanupEventSignal(RsslEventSignal* pSignal) 
{
	if(pSignal->_fds[0] != -1 ) {
#ifdef _WIN32
		closesocket( pSignal->_fds[0] );
#else
		close( pSignal->_fds[0] );
#endif
		pSignal->_fds[0] = -1;
	}

	if(pSignal->_fds[1] != -1) {
#ifdef _WIN32
		closesocket( pSignal->_fds[1] );
#else
		close( pSignal->_fds[1] );
#endif
		pSignal->_fds[1] = -1;
	}
}

RTR_C_INLINE int rsslInitEventSignal(RsslEventSignal* pSignal) 
{
#ifdef _WIN32
	int flag;
	int ioctlRetVal;
	int setTCPNoDelayRetVal;
	u_long	arg;
	int status;
	SOCKET serverFD, tmpSocketFD;
	struct sockaddr_in mySAddr, cliSAddr;
	int SAddrSize;
	struct sockaddr_in srvrSAddr;
	struct sockaddr_in srvrSockName;
	int srvrSockLen = sizeof(struct sockaddr_in);

	// create and bind the two file descriptors.

	//set up a socket and act as a server

	serverFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

	mySAddr.sin_family = AF_INET;
	mySAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

	mySAddr.sin_port = 0; /* Use ephemeral port. */
	status = bind( serverFD, (struct sockaddr*) &mySAddr, sizeof mySAddr );

	if (status < 0)
		return 0;
		
	listen( serverFD, 1 );
	SAddrSize = sizeof( struct sockaddr_in );

	tmpSocketFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

	if ( tmpSocketFD == INVALID_SOCKET )
	{
		closesocket( serverFD ); 
		return 0;
	}

	pSignal->_fds[1] = (RsslSocket)tmpSocketFD;
	
	/* Get the port assigned to the server socket. */
	status = getsockname(serverFD, (struct sockaddr*)&srvrSockName, &srvrSockLen);

	if (status < 0 || srvrSockLen < sizeof(struct sockaddr_in))
	{
		closesocket( serverFD ); 
		return 0;
	}
	srvrSAddr.sin_family = AF_INET;
	srvrSAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	srvrSAddr.sin_port = srvrSockName.sin_port;

	status = connect( pSignal->_fds[1], (struct sockaddr* ) &srvrSAddr, 
						  sizeof srvrSAddr );
		
	if (status < 0)
	{
		closesocket( serverFD ); 
		return 0;
	}
		
	tmpSocketFD = accept( serverFD, (struct sockaddr *)&cliSAddr, &SAddrSize );
	if ( tmpSocketFD == INVALID_SOCKET )
	{
		closesocket(pSignal->_fds[1]);
		closesocket( serverFD ); 
		return 0;
	}

	pSignal->_fds[0] = (RsslSocket)tmpSocketFD;

	closesocket( serverFD );

	arg = 1;
	ioctlRetVal = ioctlsocket( pSignal->_fds[0], FIONBIO, &arg );
	if(ioctlRetVal != 0) {
		rsslCleanupEventSignal(pSignal);
		return 0;
	}

	arg = 1;
	ioctlRetVal = ioctlsocket( pSignal->_fds[1], FIONBIO, &arg );
	if(ioctlRetVal != 0) {
		rsslCleanupEventSignal(pSignal);
		return 0;
	}

	flag = 1;
	setTCPNoDelayRetVal = setsockopt( pSignal->_fds[0], IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int));
	if(setTCPNoDelayRetVal != 0) {
		rsslCleanupEventSignal(pSignal);
		return 0;
	}

	flag = 1;
	setTCPNoDelayRetVal = setsockopt( pSignal->_fds[1], IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int));
	if(setTCPNoDelayRetVal != 0) {
		rsslCleanupEventSignal(pSignal);
		return 0;
	}
#else
	int pipeRetVal;
	pipeRetVal = pipe( pSignal->_fds );
	if(pipeRetVal == -1) {
		// Cannot create the communication pipe
		return 0;
	}
#endif

	pSignal->_byteWritten = 0;
	return 1;
}

RTR_C_INLINE int rsslSetEventSignal(RsslEventSignal* pSignal) 
{
	int retVal;

	if (pSignal->_byteWritten)
		return 0;

	do
	{
#ifdef WIN32
		retVal = send( pSignal->_fds[1], "b", 1, 0 );
		if (retVal < 0 && WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAEINTR)
			return retVal;
#else
		retVal = write( pSignal->_fds[1], "b", 1 );
		if (retVal < 0 && errno != EAGAIN && errno != EINTR)
			return retVal;
#endif
	}
	while (retVal <= 0);

	pSignal->_byteWritten = 1;

	return 0;
}


RTR_C_INLINE int rsslResetEventSignal(RsslEventSignal* pSignal)
{
	int retVal;
	char dummyBuffer[1];

	if (!pSignal->_byteWritten)
		return 0;

	do
	{
#ifdef WIN32
		retVal = recv( pSignal->_fds[0], dummyBuffer, 1, 0 );
		if (retVal < 0 && WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != WSAEINTR)
			return retVal;
#else
		retVal = read( pSignal->_fds[0], dummyBuffer, 1 );
		if (retVal < 0 && errno != EAGAIN && errno != EINTR)
			return retVal;
#endif
	}
	while (retVal <= 0);

	pSignal->_byteWritten = 0;

	return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

