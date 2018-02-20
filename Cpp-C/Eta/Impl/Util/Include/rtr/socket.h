/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

//  description:
//  Socket layer initialization routines.
//

#ifndef __rtr_socket_h
#define __rtr_socket_h

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _WIN32
#define FD_SETSIZE 1024
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#undef EWOULDBLOCK
#define EWOULDBLOCK		WSAEWOULDBLOCK
#define _IPC_WOULD_BLOCK EWOULDBLOCK

#ifndef snprintf
#define snprintf	_snprintf
#endif

#undef EINTR
#define EINTR                   WSAEINTR

#define RSSL_INVALID_SOCKET		INVALID_SOCKET
#else
#include <sys/socket.h>
#define _IPC_WOULD_BLOCK EAGAIN
typedef int SOCKET;
#define RSSL_INVALID_SOCKET		-1
#endif




extern int rtr_socket_startup(char *errorText);
extern int rtr_socket_shutdown();

#if defined(__cplusplus)
}
#endif


#endif
