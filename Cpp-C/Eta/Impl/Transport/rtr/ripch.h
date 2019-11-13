/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__ripch_h
#define	__ripch_h

#include "rtr/ripcplat.h"
#include "rtr/ripcdatadef.h"
#include "rtr/cutilcbuffer.h"
#include "rtr/socket.h"
#include "rtr/ripc_internal.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"

//#include "rtr/ripcversion.h"

/* Defined macros to be used:
 *  ripcValidSocket(s) - If RsslSocket s a valid socket?
 *  RIPC_INVALID_SOCKET - Value to test RsslSocket for invalid.
 */

#include "rtr/socket.h"

#if defined(_WIN32) || defined(_WIN16)
#include <time.h>
#else
#include <sys/time.h>
#endif


#if defined(_WIN32) || defined(_WIN16)
#define RIPC_INVALID_SOCKET INVALID_SOCKET
#define ripcValidSocket(s) ((s) != RIPC_INVALID_SOCKET)
#else
#define RIPC_INVALID_SOCKET -1
#define ripcValidSocket(s) ((s) >= 0)
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define IPC_MAX_HOST_NAME	256
#define RIPC_MAX_FLUSH_STRATEGY 32
#define RIPC_MAX_PRIORITY_QUEUE 3

/*  ripc IOCTL codes */
#define RIPC_MAX_POOL_BUFS			1	/* Set value to maximum number of pool buffers */
#define RIPC_GUAR_OUTPUT_BUFS		2	/* Set value to number of guaranteed output buffers */
#define RIPC_HIGH_WATER_MARK		4	/* Set value to high water mark */
#define RIPC_SYSTEM_READ_BUFFERS	5	/* Set value to # of bytes for buffers */
#define RIPC_SYSTEM_WRITE_BUFFERS	6	/* Set value to # of bytes for buffers */
#define RIPC_DBG_FLAGS				8	/* Set value to the enum ripcSessDbgFlags */
#define RIPC_PRIORITY_FLUSH_STRATEGY 9  /* sets the priority flush strategy */
#define RIPC_COMPRESSION_THRESHOLD  10  /* sets the compression threshold */
#define RIPC_PEAK_POOL_BUF_RESET	11  /* resets the peak number of pool buffers */
#define RIPC_ELEKTRON_SAFE_LZ4		12  /* does not allow large packets to compressed with LZ4 */
#define RIPC_IGNORE_CERT_REVOCATION 13
#define RIPC_MAX_IOCTL_CODE			13

/* Existing versions of RIPC */
typedef enum
{
	RIPC_VERSION_10 = 5,
	RIPC_VERSION_11 = 6,
	RIPC_VERSION_12 = 7,
	RIPC_VERSION_13 = 8,
	RIPC_VERSION_14 = 9,
	RIPC_VERSION_LATEST = RIPC_VERSION_14 /* Denotes the latest version of RIPC handshake.  Should be 
										   * updated as newer versions of RIPC handshake are introduced. */
} RsslRipcVersion;

typedef struct {
	RsslMutex	*mutex;
	RsslSocket	stream;
	RsslSocket	oldStream;
	int			state; // Temporary change the type to int;
	char	    *hostname;
	char		*IPaddress;
	RsslConnectionTypes	connectionType;
	RsslUInt32	rsslFlags;
	RsslUInt32	pingTimeout;
	RsslUInt8	majorVersion;
	RsslUInt8	minorVersion;
	RsslUInt8	protocolType;
	char		*componentVer; /* component version set by user - we dont own this memory */
	RsslUInt32	componentVerLen;
	char		*outComponentVer;	/* component version from wire - we own this memory */	
	RsslUInt32	outComponentVerLen;
	RsslUInt64	shared_key;  /* used for encryption/decryption - 0 when not available */
} RIPC_SOCKET;

#define RSSL_COMP_ALL_TYPE ((unsigned)RSSL_COMP_ZLIB | (unsigned)RSSL_COMP_LZ4)
#define RSSL_COMP_MAX_TYPE 0x02		/* set to the highest ripcCompressType enum value */
#define ZLIB_COMP_MAX_LEVEL 9
#define ZLIB_COMP_MIN_LEVEL 0


#define MAX_RIPC_ERROR_TEXT 1200

/* ripc Error Definitions */
typedef enum {
  RIPC_SYS_ERR	= 2,			/* General ipc system call error */
} ripcError;

/* Server Session Init Operations */
typedef enum {
	RIPC_CONN_ERROR			= -1,
	RIPC_CONN_IN_PROGRESS	= 1,
	RIPC_CONN_REFUSED		= 2,
	RIPC_CONN_ACTIVE		= 3,
	RIPC_CONN_HTTP_ID_RQD		= 4,
	RIPC_CONN_HTTP_READ		= 5
} ripcSessInit;

typedef enum {
	RIPC_INPROG_NEW_FD			= 0x01,
	RIPC_INPROG_WRT_NOT			= 0x02
} ripcInProgOper;

typedef struct {
	ripcInProgOper		types;
	RsslSocket			oldSocket;
	RIPC_SOCKET			newSocket;
	u32					intConnState;
} ripcSessInProg;

typedef enum {
	RIPC_WRITE_DO_NOT_COMPRESS		= 0x01
} ripcWriteFlags;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
