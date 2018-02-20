/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __ripc_int
#define __ripc_int

#include "rtr/ripcplat.h"
#include "rtr/ripcdatadef.h"
#include "rtr/ripch.h"

#include <sys/types.h>

#if defined(DEV_SVR4) || defined(DEV_USE_O_NONBLOCK)
#define _IPC_WOULD_BLOCK EAGAIN
#else
#define _IPC_WOULD_BLOCK EWOULDBLOCK
#endif


#if defined(_WIN32)

#undef ETIMEDOUT
#undef EISCONN
#undef EADDRNOTAVAIL
#undef EADDRINUSE
#undef ECONNREFUSED
#undef EWOULDBLOCK
#undef EINPROGRESS
#define ETIMEDOUT               WSAETIMEDOUT
#define EISCONN                 WSAEISCONN
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define EADDRINUSE              WSAEADDRINUSE
#define ECONNREFUSED            WSAECONNREFUSED
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS

#ifdef  errno
#undef  errno
#endif
#define errno	h_errno		// WSAGetLastError()

#include        <sys/types.h>           

#define ioctl           ioctlsocket
#define sock_close      closesocket

#else

#define sock_close	close       /* for compatibility */

#endif


#define RIPC_DEBUG

#ifdef RIPC_DEBUG
#include <assert.h>
#define RIPC_ASSERT(EX) assert(EX)
#else
#define RIPC_ASSERT(EX)
#endif

/* these are for the ipcOpcodes/flags that are in every ripc header */
/* if set to IPC_EXTENDED_FLAGS there is another byte with more flags - 
	this allows us to have some flags available for future expansion */
/* these are flags that are common, things that would likely be set on most messages */
/* 0x10, 0x20, 0x40 are available in the standard flags */
#define IPC_EXTENDED_FLAGS	0x1  /* extra byte of flags is present */
#define IPC_DATA			0x2  /* normal, uncompressed data - this is mostly here as an integrity check */
#define IPC_COMP_DATA		0x4  /* compressed data - this may require the extended header if there is fragmentation involved */
#define IPC_COMP_FRAG		0x8  /* fragmented compressed buffer */
#define IPC_PACKING			0x10 /* packing is on in this buffer */

#define RSSL_COMP_FORCE	0x80	/* force compression for connection level */

/* these are for the extended flags/opcodes */
/* these are things that should happen less frequently (eg. ack/nak of connection, fragmentation, etc)
	so the extra byte for the extended flags should not be on most messages */
/* 0x10, 0x20, 0x40 are available in the extended headers */
#define IPC_CONNACK     0x1 /* this is a connection ack */ 
#define IPC_CONNNAK     0x2 /* this is a connection nak */ 
#define IPC_FRAG		0x4 /* fragmentation id is present (1 byte) */
#define IPC_FRAG_HEADER 0x8 /* the frag header is present - this has the length of the total message 
								(up to 4 bytes), and a 1 byte fragId*/

/* connection request flags/http request flags - some of these are specific to tunneling solutions */
#define RIPC_TUNNEL_STREAMING  0x0  // streaming channel message
#define RIPC_TUNNEL_CONTROL    0x1  // control channel message
#define RIPC_TUNNEL_RECONNECT  0x2	// this is a reconnect message, so we dont need to send full acknowledgment on connection 
#define RIPC_JAVA_TUNNEL_RECONNECT 0x04	// this is a reconnect message on single connection (for Java), so we dont need to send full acknowledgment on connection
#define RIPC_KEY_EXCHANGE	   0x8  // The presence of this flag indicates we want to do the key exchange.  Right now, always sending this on CONN_VER_14 or higher but wanted it as flag in case we want to stop at some point.

#define RIPC_TUNNEL_SUPPORT_PIPELINING 0x10 // For clients that support pipelining so the server knows it can pipeline
#define RIPC_OPENSSL_TUNNELING   0x20   // using openSSL we only have one channel and dont need to worry about HTTP headers */1
#define RIPC_WININET_TUNNELING   0x40   // we want this to work in all above flag schemes so if this is ever set we can handle the entire message differently
#define RIPC_JAVA_WITH_HTTP_TUNNELING   0x80  //for clients that have a single HTTP connection (e.g. RFA Java consumer 'http' rssl connection)
#define RIPC_JAVA_WITH_TLS_TUNNELING    0x100  //for clients that have a single TLS encrypted connection (e.g. RFA Java consumer 'Encrypted' rssl connection

#define LOCAL_HOST_NAME  "localhost"

#define CONN_VERSION_10		0x0013 /* 19 */
#define CONN_VERSION_11		0x0014 /* 20 */
#define CONN_VERSION_12		0x0015 /* 21 */
#define CONN_VERSION_13		0x0016 /* 22 */
#define CONN_VERSION_14		0x0017 /* 23 */

RTR_C_ALWAYS_INLINE u32 dumpConnVersion(u32 version_number)
{
	switch (version_number)
	{
		case CONN_VERSION_10:
			return 10;
		break;
		case CONN_VERSION_11:
			return 11;
		break;
		case CONN_VERSION_12:
			return 12;
		break;
		case CONN_VERSION_13:
			return 13;
		break;
		case CONN_VERSION_14:
			return 14;
		break;
		default:
			return 0;
	}
}

static const u32 _conn_version_10	=	CONN_VERSION_10;
static const u32 _conn_version_11	=	CONN_VERSION_11;
static const u32 _conn_version_12	=	CONN_VERSION_12;
static const u32 _conn_version_13	=	CONN_VERSION_13;
static const u32 _conn_version_14	=	CONN_VERSION_14;

	/* These version numbers for non-negotiated clients was
	 * picked because the old way sent an sockaddr_in structure
	 * across as a header. The non-negotiated way will only pass
	 * an int for a version number. Mapping this int to a port
	 * number of sin_family is impossible.
	 */

	/* The following defines the protocol for the initial connection
	 * request sent from the client to the server. This different
	 * versions will be defined.
	 * < _conn_version_451 - oursockaddr_in structure. Has to be exactly
	 *                       16 bytes in length.
	 * = _conn_version_451 - 4 Byte message equal to _conn_version_451.
	 * = _conn_version_46 - 8 Byte message. First 4 bytes equal to _conn_version_46
	 *                      and second 4 bytes equal to the ripc version.
	 *                      4 bytes connection version. 4 byte ripc version.
	 * = _conn_version_10 - Min 12 bytes.
	 *                        _conn_version_10				4 bytes
	 *                        RIPC_VERSION_10 or greater	4 bytes
	 *                        12							2 bytes
	 *                        0								1 byte
	 *                        0								1 byte
	 *                      4 byte conn version. 4 byte ripc version. 2 byte
	 *                      message length (including previous version info).
	 *                      1 byte flags bitmap.
	 *                      1 byte number of bytes in compression types bitmap,
	 *                      followed by n bytes of the compression bitmap.
	 */

/* Compression type definitions */

/* Array definitions, index of byte in bitmap, bit within the byte, decimal value when sent */
#define RSSL_COMP_BYTEINDEX 0
#define RSSL_COMP_BYTEBIT 1
#define RSSL_COMP_TYPE 2

/* Current number of bytes in the compression bitmap */
#define RSSL_COMP_BITMAP_SIZE 1


#define IPC_100_OTHER_HEADER_SIZE	8	/* Non Data opcode header size */
#define IPC_100_CONN_ACK		    10	
#define IPC_100_DATA_HEADER_SIZE	3  /* Data header size for ripc1.0 */


#define OURSOCKADDR_SIZE		16

#define V10_MIN_CONN_HDR		14
#define V10_MIN_HDR				3


#define MAKE_BITMAP(X)    (1<<(X))
#define RING_HIGH_WATER   2000


#define MAX_SOCK_BUFSZ  16384
#define MAX_SERV_NAME	1024

 
#ifdef SOCK_IO_W_4_ARGS
#define SOCK_RECV(_fd, _buf, _len, _flag)   recv(_fd, _buf, _len, _flag)
#define SOCK_SEND(_fd, _buf, _len, _flag)   send(_fd, _buf, _len, _flag)
#else
#define SOCK_RECV(_fd, _buf, _len, _flag)   read(_fd, _buf, _len)
#define SOCK_SEND(_fd, _buf, _len, _flag)   write(_fd, _buf, _len)
#endif

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif


#endif
