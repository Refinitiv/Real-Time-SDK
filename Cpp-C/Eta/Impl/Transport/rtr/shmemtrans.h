/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2025 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef __RTR_SHMEM_TRANS
#define __RTR_SHMEM_TRANS

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/os.h"
#include "rtr/socket.h"
#ifdef WIN32
#ifndef snprintf
#if _MSC_VER < 1900
#define snprintf	_snprintf
#endif
#endif
#endif
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "rtr/spinlock.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslpipe.h"
#include "rtr/rtratomic.h"
#include "rtr/shmemcirbuf.h"

//#define SHM_SOCKET_TCP 1			// use a TCP socket as notifier
//#define SHM_SOCKET_UDP 1			// use a UDP socket as notifier
//#define SHM_NAMEDPIPE 1			// use a named pipe as notifier
#define SHM_PIPE 1				// use an unnamed pipe with no notifier

#define RTR_SHTRANS_LOCK(___lockPtr)	RTR_SLOCK_ACQUIRE(___lockPtr)
#define RTR_SHTRANS_UNLOCK(___lockPtr)	RTR_SLOCK_RELEASE(___lockPtr)

#define RSSL_SHM_MIN_SEQ_NUM			0
#define RSSL_SHM_COMPONENT_VERSION_SIZE	255	// plus 1 bytes for the length

typedef struct
{
	rtrShmSeg			shMemSeg;
	rtrShmMutexPtr		controlMutex;	// Not in shared memory segment - used for connecting/disconnecting/initialization
	rtrShmNamedPipePtr	namedPipe;		// not in shared memory segment - used to notify reader when data is available
	rssl_pipe			_bindPipe;		// not in shared memory segment
	rssl_pipe			_acceptPipe;	// not in shared memory segment. There is only once accept for shmem connection
	RsslSocket			_bindFD;		// not in shared memory segment. file descriptor we use for bind
	RsslSocket			_acceptFD;		// not in shared memory segment. file descriptor we use for accept
	struct sockaddr_in	cliAddr;		// not stored in shared memory - used for UDP notifier
	rtrUInt8			_hasBuffer;		// not in shared memory segment. only one rsslBuffer can be used by the shmem connection
	RsslBool			serverBlocking;	// Not in shared memory segment - If RSSL_TRUE, the server will be allowed to block.
	RsslBool			channelsBlocking;// Not in shared memory segment - If RSSL_TRUE, the channels will be allowed to block.

	// this starts the shmem seg
	RsslUInt16*			shmemVersion;	// the version of the shmem transport used to create this shmem seg
	RsslUInt16*			flags;
	RsslUInt32*			byteWritten;	// used to signify when there is a byte in the notifier
	RsslUInt32*			pingTimeout;
	RsslUInt8*			serverComponentVersionLen;	// component version length followed by the component version
	char*				serverComponentVersion;
	rtrSpinLock*  		userLock;		// lock for adding new users
	RsslUInt32*			protocolType;
	RsslUInt32*			majorVersion;
	RsslUInt32*			minorVersion;

#if defined (COMPILE_64BITS)			// We dont need an atomic increment because there is only one writer, but we do need an atomic 64 bit load/store. 
	RsslUInt64*			seqNumServer;	// server sequence number
#else
	rtr_atomic_val64*	seqNumServer;	// server sequence number, 32 bit platform does not do native atomic 64 load/store */
#endif
	rtrShmCirBuf*		circularBufferServer;
} rtrShmTransServer;


#define RSSL_SHMBUF_PACKED_BUFFER	0x01	// the buffer contains packed messages
#define RSSL_SHMBUF_PING			0x02	// this is a ping

typedef struct
{
	RsslUInt16		length;
	RsslUInt16		maxLength;
	RsslUInt32		flags;
	char			buffer[1];	/* gives us our pointer to the data portion of the buffer */
} rtrShmBuffer; // sizeof (rtrShmBuffer) will return diff values on 32 vrs 64 bit machines

#define RSSL_SHM_SERVER_PING_ENABLED	0x01
#define RSSL_SHM_SERVER_SHUTDOWN		0x02	// the server has shutdown and destroyed the shmem segment
#define RSSL_SHM_SERVER_INITIALIZED		0x04	// the shmem segment is ready to accept connections 

typedef struct
{
	rtrShmSeg			shMemSeg;
	rtrShmMutexPtr		controlMutex;	/* Not in shared memory segment  - used for connecting/disconnecting/init */
	rtrShmNamedPipePtr	namedPipe;		// not in shared memory segment - used to notify reader when data is available
	RsslUInt8			blockingIO;		// not in shared memory segment
	rssl_pipe			_pipe;			// not stored in shared memory - used for signaling if needed
	struct sockaddr_in srvrSAddr;		// not stored in shared memory - The servers UDP address
	RsslSocket			clientFD;		// The file descriptor given back to the client
#if defined (COMPILE_64BITS)			// We dont really need an atomic increment, but we do need an atomic 64 bit load/store. 
	RsslUInt64*			seqNumServer;	/* server sequence number */
#else
	rtr_atomic_val64*	seqNumServer;	/* server sequence number, 32 bit platform does not do native atomic 64 load/store */
#endif
	rtrSpinLock*  		userLock;				// lock for adding/removing users
	rtrShmCirBuf*		circularBufferServer;
	rtrShmBuffer*		readBuffer;				// not stored in shared memory - pointer to last read buffer
	rtrShmCirBuf		circularBufferClient;	// not stored in shared memory
	RsslUInt64			seqNumClient;			/* not stored in shared memory - client sequence number */
	RsslUInt64			readRetries;			/* the number of consecutive times the reader recevied nothing from a read attempt */
	RsslUInt64			maxReaderRetryThreshhold;/* maximum number of read retries before the client waits for a notification */
	RsslUInt64			maxReaderSeqNumLag;		/* not stored in shared memory */
	RsslUInt16*			shmemVersion;			// the version of the shmem transport used to create the shmem seg
	RsslUInt16*			flags;
	RsslUInt32*			byteWritten;			// used to signify when there is a byte in the named pipe
	RsslUInt32*			pingTimeout;
	RsslUInt8*			serverComponentVersionLen;
	char*				serverComponentVersion;
	RsslUInt32*			protocolType;
	RsslUInt32*			majorVersion;
	RsslUInt32*			minorVersion;
	char *				currentBuffer;
} rtrShmTransClient;


/* contains options and shared components of shm segment */
#define SHMKEY_SIZE		256
typedef struct
{
	char	       shMemKey[SHMKEY_SIZE];		// this is a strcat of the service and interface name 
	RsslUInt16	   numBuffers;
	RsslUInt16	   maxBufSize;
	RsslUInt16	   serverToClientPing;		// 0 means no pings
	RsslUInt16	   pingTimeout;
	RsslUInt32	   protocolType;
	RsslUInt32      majorVersion;
	RsslUInt32      minorVersion;
	void		   *userSpecPtr;  
	RsslBool		serverBlocking;			/*!< If RSSL_TRUE, the server will be allowed to block. */
	RsslBool		channelsBlocking;		/*!< If RSSL_TRUE, the channels will be allowed to block. */
} rtrShmCreateOpts;

typedef struct
{
	char	       shMemKey[SHMKEY_SIZE];		// this is a strcat of the service and interface name 
	RsslUInt8	   blockingIO;
	RsslUInt32	   protocolType;
	RsslUInt32	   majorVersion;
	RsslUInt32	   minorVersion;
	RsslUInt64	   maxReaderSeqNumLag;
	RsslUInt64	   maxReaderRetryThreshhold;	// used by the notifier
	void		   *userSpecPtr;  
} rtrShmAttachOpts;

rtrShmTransServer* rtrShmTransCreate(rtrShmCreateOpts *createOpts, RsslError *error);
	/* Create a shared memory transport server. */
RsslInt32 rtrShmTransDestroy(void *trans, RsslError *error );
	/* Destory a shared memory transport server. */
RsslInt32 rtrShmTransAccept(rtrShmTransServer *trans, RsslError *error);
	/* Server uses to 'accept' connection */


rtrShmTransClient* rtrShmTransAttach( rtrShmAttachOpts *attachOpts, RsslError *error );
	/* Attached to a shared memory segment transport server */
RsslInt32 rtrShmTransDetach(rtrShmTransClient *trans, RsslError *error);
	/* Detach from a shared memory segment transport server */


rtrShmBuffer* rtrShmTransClientRead(rtrShmTransClient *trans, RsslChannel *chnl, RsslRet *readRet, RsslError *error);
	/* Retrieve an empty output buffer. */

/* server uses to get a buffer to write into */
RTR_C_ALWAYS_INLINE rtrShmBuffer* rtrShmTransGetFreeBuffer(rtrShmTransServer *trans)
{
	return (rtrShmBuffer*)RTRShmCirBufGetWriteBuf(trans->circularBufferServer, &trans->shMemSeg);
}

/* clients use to release buffer after read and process is done */
RTR_C_ALWAYS_INLINE RsslInt32 rtrShmTransRelBuffer(rtrShmTransClient *trans)
{
	trans->seqNumClient++;
	RTRShmCirBufReadComplete(&trans->circularBufferClient);
	return 0;
}

#if defined(LINUX) // this code is only here temporarily until we move to a newer cutil
#ifndef COMPILE_64BITS	// only for 32 bit linux builds
#undef RTR_ATOMIC_INCREMENT64
#define RTR_ATOMIC_INCREMENT64(___var)		(void)_rsslInterExch64(&___var,___var+1)

RTR_C_ALWAYS_INLINE rtr_atomic_val64 _rsslInterExch64(volatile rtr_atomic_val64 *volatile var, volatile rtr_atomic_val64 newVal)
{
	volatile rtrInt32* volatile p1 = (rtrInt32*)var;
	volatile rtrInt32* volatile p2 = ((rtrInt32*)var) + 1;

	volatile rtrInt32* volatile pn1 = (rtrInt32*)(&newVal);
	volatile rtrInt32* volatile pn2 = ((rtrInt32*)(&newVal)) + 1;

	__asm__ __volatile__ ( "1: lock cmpxchg8b %0 ; jnz 1"
	: "=m"(*var) : "a" (*p1), "d"(*p2),  "b" (*pn1), "c" (*pn2) : "cc");
	return *var;
}
#endif
#endif

/* server uses to write to each client */
RTR_C_ALWAYS_INLINE void rtrShmTransServerWrite(rtrShmTransServer *trans)
{
	RTR_SHTRANS_LOCK(trans->userLock);
#if defined (COMPILE_64BITS)
	++(*trans->seqNumServer);
#else
	RTR_ATOMIC_INCREMENT64(*trans->seqNumServer);
#endif
	RTRShmCirBufWritten(trans->circularBufferServer);
	RTR_SHTRANS_UNLOCK(trans->userLock);
//	printf("seqNumServer = %llu writeoffset = %llu\n", *trans->seqNumServer, trans->circularBufferServer->write);
	return;
}

RsslInt32 ripcGetCountShmTransCreate();

RsslInt32 ripcGetCountShmTransDestroy();

#ifdef __cplusplus
};
#endif

#endif
