/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_SHMEM
#define __RTR_SHMEM

#include "rtr/os.h"
#include "rtr/socket.h"

#if defined (_WIN32) || defined (_WIN64)

#include <windows.h>

#ifndef snprintf
#if _MSC_VER < 1900
#define snprintf	_snprintf
#endif
#endif

#else

#include <sys/types.h>

/* Semaphores */
#include <semaphore.h>
#include <fcntl.h>
#include "rtr/rtrdefs.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define __MY_MAX_ADDR 256
#define __ERROR_LEN 512

#define RTR_4BYTE_ALIGN_SHIFT	2
#define RTR_4BYTE_ALIGN_SIZE	(1UL << RTR_4BYTE_ALIGN_SHIFT)	/* 4 */
#define RTR_4BYTE_ALIGN_MASK	(~(RTR_4BYTE_ALIGN_SIZE - 1))
#define RTR_4BYTE_ALIGN(___addr) (((___addr) + RTR_4BYTE_ALIGN_SIZE - 1) & RTR_4BYTE_ALIGN_MASK)

#define RTR_8BYTE_ALIGN_SHIFT	3
#define RTR_8BYTE_ALIGN_SIZE	(1UL << RTR_8BYTE_ALIGN_SHIFT)	/* 8 */
#define RTR_8BYTE_ALIGN_MASK	(~(RTR_8BYTE_ALIGN_SIZE - 1))
#define RTR_8BYTE_ALIGN(___addr) (((___addr) + RTR_8BYTE_ALIGN_SIZE - 1) & RTR_8BYTE_ALIGN_MASK)

// do 8 byte alignment on both 32 and 64 bit machines
// always do 8 byte alignment to ensure atomic reads/writes
#define RTR_MACH_ALIGN(___addr)	RTR_8BYTE_ALIGN(___addr)


/*
 * The shared memory region can start at a different address
 * in every process.  Shared memory "pointers" are actually
 * offsets relative to the start of the shared memory region(s).
 */
typedef rtrUInt64 RTR_SHM_OFFSET;

RTR_C_INLINE size_t RTR_SHM_ALIGNBYTES( size_t bytes )
{
	return RTR_MACH_ALIGN(bytes);
}

#define RTR_INVALID_OFFSET	(-1)

/* coerce an offset into a pointer in this process's address space */
#define RTR_SHM_MAKE_PTR(___baseptr,___offset) \
  ((RTR_SHM_OFFSET)___baseptr + ___offset)

/* coerce a pointer into a shmem offset */
#define RTR_SHM_MAKE_OFFSET(___baseptr,___ptr) \
  ((RTR_SHM_OFFSET) ((RTR_SHM_OFFSET)___ptr - (RTR_SHM_OFFSET)___baseptr))

#define RTR_SHM_PTR_VALID(___baseptr,___ptr) \
  (___ptr > ___baseptr)

/* cannot have an offset to ShmemFreeStart (offset 0) */
#define RTR_SHM_OFFSET_VALID(___offset) \
  (((___offset) != 0) && ((___offset) != INVALID_OFFSET))

typedef SOCKET rtrShmSocket;

#ifdef _WIN32

typedef HANDLE rtrShmFile;
typedef HANDLE rtrShmMutexPtr;
typedef HANDLE rtrShmEventPtr;
typedef HANDLE rtrShmNamedPipePtr;
typedef BOOL rtrShmBool;

#define RTR_SHM_TRUE TRUE
#define RTR_SHM_FALSE FALSE 

#else

typedef int rtrShmFile; /* File descriptor */

/* If the person getting the mutex is the 'owner,' the name has to be saved to clean it up. */
typedef struct
{
	sem_t		*mtx;
	char		*name;
} rtrNamedSem;

typedef struct
{
	SOCKET		fd;
	char		*name;
} rtrNamedPipe;


typedef rtrNamedSem *rtrShmMutexPtr;
typedef rtrNamedSem *rtrShmEventPtr;
typedef rtrNamedPipe *rtrShmNamedPipePtr;

typedef RwfBool rtrShmBool;

#define RTR_SHM_TRUE 1 
#define RTR_SHM_FALSE 0 

#endif

typedef struct
{
	unsigned long	magic;
#define RTR_SHM_MAGIC 0x12FE68BA
	unsigned short	creatorPid;
	unsigned short	headerLen;
	size_t			totalLen;
	size_t			freeOffset;
#ifdef _WIN32
#else
	/* Owner needs to save name of segment for cleanup */
	char*		name;
#endif
} rtrShmHdr;

typedef struct
{
	rtrShmFile	shMemSeg;
	char			*base;
	rtrShmHdr		*hdr;
} rtrShmSeg;



int rtrShmSegCreate( rtrShmSeg *seg, const char *key, size_t size, char *errBuff );
void rtrShmSegDestroy( rtrShmSeg *seg );
char* rtrShmBytesReserve( rtrShmSeg *seg, size_t size );

int rtrShmSegAttach( rtrShmSeg *seg, const char *key, char *errBuff );
void rtrShmSegDetach( rtrShmSeg *seg );
char* rtrShmBytesAttach( char **loc, size_t size );

rtrShmEventPtr rtrShmSegCreateEvent( rtrShmSeg *seg, const char *key, rtrShmBool manualReset, unsigned short mutexNumber, char *errBuff );
void rtrShmSegDestroyEvent( rtrShmEventPtr shmEvent );

rtrShmEventPtr rtrShmSegAttachEvent( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff );
void rtrShmSegDetachEvent( rtrShmEventPtr shmEvent );

rtrShmMutexPtr rtrShmSegCreateMutex( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff );
void rtrShmSegDestroyMutex( rtrShmMutexPtr shmMutex );

rtrShmMutexPtr rtrShmSegAttachMutex( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff );
void rtrShmSegDetachMutex( rtrShmMutexPtr shmMutex );

rtrShmNamedPipePtr rtrShmSegCreateNamedPipe( const char *key, unsigned short namedPipeNumber, char *errBuff );
int rtrShmSegAcceptNamedPipe( rtrShmNamedPipePtr namedPipe, char *errBuff );
void rtrShmSegDestroyNamedPipe( rtrShmNamedPipePtr shmNamedPipe );

rtrShmNamedPipePtr rtrShmSegAttachNamedPipe( const char *key, unsigned short namedPipeNumber, char *errBuff );
void rtrShmSegDetachNamedPipe( rtrShmNamedPipePtr shmNamedPipe );

rtrShmSocket rtrShmSegBindSocketTCP( const char *portNumber, char *errBuff, RwfBool blocking );
rtrShmSocket rtrShmSegAcceptSocketTCP( rtrShmSocket serverFD, char *errBuff, RwfBool blocking );
rtrShmSocket rtrShmSegAttachSocketTCP( char *portNumber, char *errBuff, RwfBool blocking );

rtrShmSocket rtrShmSegBindSocketUDP( const char *portNumber, char *errBuff );
int rtrShmSegAcceptSocketUDP( rtrShmSocket serverFD, struct sockaddr_in *cliAddr, int *cliLen, char *errBuff );
rtrShmSocket rtrShmSegAttachSocketUDP( char *portNumber, char *errBuff );

void rtrShmSegDestroySocket( rtrShmSocket shmBindSocket );
void rtrShmSegDetachSocket( rtrShmSocket shmSocket );

/* No timeout support. For now that doesn't appear to be needed */
#ifdef _WIN32
#define rtrWaitForMutex(mtx) WaitForSingleObject((mtx), INFINITE)
#define rtrReleaseMutex(mtx) ReleaseMutex((mtx))
#define rtrSignalEvent(mtx) SetEvent((mtx))
#else
#define rtrWaitForMutex(rtr_mtx) sem_wait((rtr_mtx)->mtx)
#define rtrReleaseMutex(rtr_mtx) sem_post((rtr_mtx)->mtx)
#define rtrSignalEvent(rtr_mtx) sem_post((rtr_mtx)->mtx)
#endif

#ifdef __cplusplus
};
#endif

#endif
