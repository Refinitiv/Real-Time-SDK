/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include "rtr/shmem.h"
#include "rtr/custmem.h"

#ifdef _WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#define closesocket close
#define ioctlsocket ioctl

/*** Shared Memory Segments ***/

/* Creation should fail if the file already exists(O_CREAT | O_EXCL). */
const int RTR_SHM_SEG_CREATE_FLAGS = O_CREAT | O_EXCL | O_RDWR;

/* Attachers get r/w perms for now. */
const int RTR_SHM_SEG_ATTACH_FLAGS = O_RDWR;
const int RTR_SHM_SEG_ATTACH_PERM = 0666; /* r/w everyone */
const int RTR_SHM_SEG_CREATE_PERM = 0666; /* r/w everyone */

/* On linux, this name becomes the file in /dev/shm(the shared memory folder).
 * The standard asks that the name is prepended with a '/'. */
const char *RTR_SHM_SEG_NAME = "/tr.seg.%s";
const unsigned int RTR_SHM_SEG_KEY_MAX_LEN = __MY_MAX_ADDR - 9; /* __MY_MAX_ADDR - <size of string without key> */

/*** Mmap ***/
/* Everyone gets r/w for now. */
const int RTR_MMAP_CREATE_PERM = PROT_READ | PROT_WRITE;
const int RTR_MMAP_ATTACH_PERM = PROT_READ | PROT_WRITE;

/*** Mutexes ***/

/* Like the shm segments, creation should fail if the file already exists */
const int RTR_MTX_CREATE_FLAGS = O_CREAT | O_RDWR | O_EXCL;

const int RTR_MTX_ATTACH_FLAGS = O_RDWR;
const int RTR_MTX_PERM = 0666; /* r/w everyone */

/* Mutex files also appear in /dev/shm using these names. */
const char *RTR_MTX_NAME = "/tr.mtx.%s.%u";
const char *RTR_EVT_NAME = "/tr.evt.%s.%u";
const unsigned int RTR_MTX_KEY_MAX_LEN = __MY_MAX_ADDR - 15; /* __MY_MAX_ADDR - <size of string without key + longest ushort(5)> */

/*** Named Pipe ***/

/* Like the shm segments, creation should fail if the file already exists */
const int RTR_NAMEDPIPE_CREATE_FLAGS = O_WRONLY;
const int RTR_NAMEDPIPE_ATTACH_FLAGS = O_RDONLY;
const int RTR_NAMEDPIPE_PERM = 0666; /* r/w everyone */

const char *RTR_NAMEDPIPE_NAME = "/dev/shm/tr.namedpipe.%s.%u";
const int RTR_NAMEDPIPE_KEY_MAX_LEN = __MY_MAX_ADDR - 20; /* __MY_MAX_ADDR - <size of string without key + longest ushort(5)> */

#endif

#ifdef _WIN32
#else
/* Internal function for creating/attaching event/mutex. On Linux it's almost the same code in all four cases. */
rtrNamedSem *_rtrShmMutexOpenInt( int isEvent/* is event or mutex? */, int isOwner /* Create or attach? */, const char* key, unsigned short mutexNumber, char *errBuff )
{
	char memMapName[__MY_MAX_ADDR];
	rtrNamedSem *newMutex = malloc(sizeof(rtrNamedSem)); 
	
	if (!newMutex) 
	{ 
		snprintf(errBuff, __ERROR_LEN, "_rtrShmMutexOpenInt() newMutex malloc failed (size = %zu, key = %s)", sizeof(rtrNamedSem), key);
		return 0; 
	}

	if (strlen(key) > RTR_MTX_KEY_MAX_LEN)
	{
		snprintf(errBuff, __ERROR_LEN, "_rtrShmMutexOpenInt() Illegal shared mutex key length %zu (key = %s)",strlen(key), key);
		free(newMutex);
		return 0;
	}

	/* initialize name variable to check for cleaning up later */
	newMutex->name = NULL;

	snprintf(memMapName,__MY_MAX_ADDR, isEvent ? RTR_EVT_NAME : RTR_MTX_NAME, key, mutexNumber );

	newMutex->mtx = sem_open(memMapName, isOwner ? RTR_MTX_CREATE_FLAGS : RTR_MTX_ATTACH_FLAGS, RTR_MTX_PERM, 1);
	if (newMutex->mtx == SEM_FAILED)
	{
		snprintf(errBuff, __ERROR_LEN, "_rtrShmMutexOpenInt() sem_open failed (errno = %d, name = %s)", errno, memMapName);
		free(newMutex);
		return 0;
	}

	if (isOwner) /* If this is the owner, save the mutex name for cleanup */
	{ 
		int tempLen = strlen(memMapName) + 1;
		newMutex->name = malloc(tempLen); 
		if (!newMutex->name) 
		{ 
			snprintf(errBuff, __ERROR_LEN, "_rtrShmMutexOpenInt() newMutex->name malloc failed (size = %zu, key = %s)", 1+strlen(memMapName), key);
			free(newMutex); 
			return 0; 
		}
		strcpy(newMutex->name, memMapName);
	}

	return newMutex;
}
#endif

int rtrShmSegCreate( rtrShmSeg *seg, const char * key, size_t size, char *errBuff )
#ifdef _WIN32
{
	DWORD ProcessID = GetCurrentProcessId();
	char memMapName[__MY_MAX_ADDR];
	wchar_t wMemMapName[__MY_MAX_ADDR];
	SECURITY_DESCRIPTOR SecurityDescriptor;
	SECURITY_ATTRIBUTES SecurityAttributes;

	seg->shMemSeg = 0;
	seg->base = 0;
	seg->hdr = 0;

	size += RTR_SHM_ALIGNBYTES(sizeof(rtrShmHdr));

	/*  Create the security secriptor */
	InitializeSecurityDescriptor(&SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	SecurityDescriptor.Dacl = NULL;
	/* Create the security attributes */
	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
	SecurityAttributes.bInheritHandle = 1;

	if (strlen(key) > (__MY_MAX_ADDR - 6))
	{
		snprintf(errBuff, __ERROR_LEN, "rtrShmSegCreate() Illegal shared memory key length %zu (key = %s)",strlen(key), key);
		return -1;
	}

	snprintf(memMapName, __MY_MAX_ADDR, "%s_mem", key);
	MultiByteToWideChar(CP_ACP, 0, memMapName, -1, wMemMapName, __MY_MAX_ADDR);

	// Create the file mapping
	seg->shMemSeg = CreateFileMapping(	INVALID_HANDLE_VALUE,
										&SecurityAttributes,
										PAGE_READWRITE,
										0,
										(DWORD)size,
										(LPCSTR)memMapName);

	if ((seg->shMemSeg == NULL) || (seg->shMemSeg == INVALID_HANDLE_VALUE))
   	{
	   	snprintf(errBuff, __ERROR_LEN, "rtrShmSegCreate() CreateFileMapping() Failed (errno = %d)", errno);
	   	return -1;
   	}
 
	// Map to the file
	seg->base = (char*)MapViewOfFile(seg->shMemSeg,				// handle to map object
										FILE_MAP_ALL_ACCESS,	// read/write permission
										0,
										0,
										size ); 
	if (seg->base == NULL)
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() MapViewOfFile() Failed (errno = %d)", errno);
		rtrShmSegDestroy(seg);
		return -1; 
	}

	// Clear the buffer
	ZeroMemory(seg->base, size);

	seg->hdr = (rtrShmHdr*)seg->base;
	seg->hdr->magic = RTR_SHM_MAGIC;
	seg->hdr->creatorPid = (unsigned short)ProcessID;
	seg->hdr->totalLen = size;
	seg->hdr->headerLen = (unsigned short)RTR_SHM_ALIGNBYTES(sizeof(rtrShmHdr));
	seg->hdr->freeOffset = seg->hdr->headerLen;

	return 1;
};
#else
{
	char memMapName[__MY_MAX_ADDR];
	int tempLen;
	seg->shMemSeg = 0;
	seg->base = 0;
	seg->hdr = 0;
	size += RTR_SHM_ALIGNBYTES(sizeof(rtrShmHdr));

	if (strlen(key) > RTR_SHM_SEG_KEY_MAX_LEN)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() Illegal shared memory key length %zu (key = %s)",strlen(key), key);
		return -1;
	}

	snprintf(memMapName, __MY_MAX_ADDR, RTR_SHM_SEG_NAME, key );

	/* Create shm file descriptor */
	seg->shMemSeg = shm_open(memMapName, RTR_SHM_SEG_CREATE_FLAGS, RTR_SHM_SEG_CREATE_PERM);
	if (seg->shMemSeg == -1) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() shm_open() Failed (errno = %d, mapName = %s)", errno, memMapName);
		return -1; 
	}

	/* Set the size of the file to the desired segment size.
	 * When the shm file is opened, it has a size of zero. Attachers need to know the correct size to call mmap() with. */
	if (ftruncate(seg->shMemSeg, (off_t)size) == -1 ) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() ftruncate() Failed (errno = %d, mapName = %s)", errno, memMapName);
		rtrShmSegDestroy(seg);
		shm_unlink(memMapName);
		return -1;
	}

	/* Map to memory space. */
	seg->base = mmap((caddr_t)0, size, RTR_MMAP_CREATE_PERM, MAP_SHARED, seg->shMemSeg, 0);
	if (seg->base == MAP_FAILED) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() mmap() Failed (errno = %d, mapName = %s)", errno, memMapName);
		rtrShmSegDestroy(seg);
		shm_unlink(memMapName);
		return -1; 
	}


	/* Lock the segment. It is critical that the shared memory segments don't get paged */
	/* You don't typically get shared memory locking privileges by default on our Solaris boxes, so this will fail there. */
	if (mlock(seg->base, size) != 0) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() mlock() Failed (errno = %d, mapName = %s, size = %zu)", errno, memMapName, size);
		rtrShmSegDestroy(seg);
		shm_unlink(memMapName);
		return -1;
	}

	MemSetByInt(seg->base, 0, size);
	seg->hdr = (rtrShmHdr*)seg->base;
	seg->hdr->magic = RTR_SHM_MAGIC;
	seg->hdr->creatorPid = (unsigned short)getpid();
	seg->hdr->totalLen = size;
	seg->hdr->headerLen = (unsigned short)RTR_SHM_ALIGNBYTES(sizeof(rtrShmHdr));
	seg->hdr->freeOffset = seg->hdr->headerLen;

	/* Save name for cleanup */
	tempLen = strlen(memMapName)+1*sizeof(char);
	seg->hdr->name = malloc(tempLen); 
	if (!seg->hdr->name) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreate() malloc() Failed (mapName = %s, size = %zu)", memMapName, size);
		rtrShmSegDestroy(seg); 
		shm_unlink(memMapName);
		return -1; 
	}
	strncpy(seg->hdr->name, memMapName, tempLen);

	return 1;

}
#endif

void rtrShmSegDestroy( rtrShmSeg *seg )
#ifdef _WIN32
{
	if (seg->base)
	{
		UnmapViewOfFile(seg->base);
		seg->base = 0;
	}
	if (seg->shMemSeg)
	{
		CloseHandle(seg->shMemSeg);
		seg->shMemSeg = 0;
	};
	seg->hdr = 0;
};
#else
{
	if (seg->base && seg->hdr)
	{
		if (seg->hdr->name)
		{ 
			shm_unlink(seg->hdr->name); 
			free(seg->hdr->name);
			seg->hdr->name = 0;
		}

		munmap(seg->base, seg->hdr->totalLen);
		seg->base = 0;
	}
	if (seg->shMemSeg)
	{
		close(seg->shMemSeg);
		seg->shMemSeg = 0;
	};
	seg->hdr = 0;
}
#endif

char* rtrShmBytesReserve( rtrShmSeg *seg, size_t size )
{
	char *ret = seg->base + seg->hdr->freeOffset;
	if (ret >= (seg->base + seg->hdr->totalLen))
		return 0;
	seg->hdr->freeOffset += RTR_SHM_ALIGNBYTES(size);
	return ret;
}


int rtrShmSegAttach( rtrShmSeg *seg, const char *key, char *errBuff )
#ifdef _WIN32
{
	char memMapName[__MY_MAX_ADDR];
	wchar_t wMemMapName[__MY_MAX_ADDR];
	SECURITY_DESCRIPTOR SecurityDescriptor;
	SECURITY_ATTRIBUTES SecurityAttributes;

	seg->shMemSeg = 0;
	seg->base = 0;
	seg->hdr = 0;

	// Create the security secriptor
	InitializeSecurityDescriptor(&SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	SecurityDescriptor.Dacl = NULL;
	// Create the security attributes
	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
	SecurityAttributes.bInheritHandle = 1;

	if (strlen(key) > (__MY_MAX_ADDR - 6))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Illegal shared memory key length %zu (key = %s)", strlen(key), key);
		return -1;
	}

	snprintf(memMapName, __MY_MAX_ADDR, "%s_mem", key);
	MultiByteToWideChar(CP_ACP, 0, memMapName, -1, wMemMapName, __MY_MAX_ADDR);

	// Open the shared file
	seg->shMemSeg = OpenFileMapping(FILE_MAP_ALL_ACCESS,	// read/write access
									FALSE,					// do not inherit the name
									(LPCSTR)memMapName);

	if ((seg->shMemSeg == NULL) || (seg->shMemSeg == INVALID_HANDLE_VALUE))
   	{
	   	snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() OpenFileMapping() Failed(errno = %d)", errno);
	   	return -1;
   	}
 
	// Map to the file
	seg->base = (char*)MapViewOfFile(seg->shMemSeg,				// handle to map object
										FILE_MAP_ALL_ACCESS,	// read/write permission
										0,
										0,
										0 );
	if (seg->base == NULL)
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() MapViewOfFile() Failed(errno = %d)", errno);
		rtrShmSegDetach(seg);
		return -1; 
	}

	seg->hdr = (rtrShmHdr*)seg->base;

	if (seg->hdr->magic != RTR_SHM_MAGIC)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Illegal Shared Memory Magic 0x%x",seg->hdr->magic);
		return -1;
	}

	return 1;
};
#else
{
	char memMapName[__MY_MAX_ADDR];
	struct stat mStat;

	seg->shMemSeg = 0;
	seg->base = 0;
	seg->hdr = 0;
	
	if (strlen(key) > RTR_SHM_SEG_KEY_MAX_LEN)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Illegal shared memory key length %zu %s",strlen(key), key);
		return -1;
	}

	snprintf(memMapName, __MY_MAX_ADDR, RTR_SHM_SEG_NAME, key );

	/* Get shm file descriptor */
	seg->shMemSeg = shm_open(memMapName, RTR_SHM_SEG_ATTACH_FLAGS, RTR_SHM_SEG_ATTACH_PERM);
	if (seg->shMemSeg == -1) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Errno(%d) from shm_open with \'%s\'", errno, memMapName);
		return -1; 
	}

	/* Get size of file. Hopefully this works -- need the correct size for passing to mmap */
	if ( fstat(seg->shMemSeg, &mStat) == -1) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Errno(%d) from fstat with \'%s\'", errno, memMapName);
		return -1; 
	}

	/* Map to memory */
	seg->base = mmap(0, mStat.st_size, RTR_MMAP_ATTACH_PERM, MAP_SHARED, seg->shMemSeg, 0);
	if (seg->base == MAP_FAILED) 
	{ 
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Errno(%d) from mmap with \'%s\'", errno, memMapName);
		return -1; 
	}

	seg->hdr = (rtrShmHdr*)seg->base;

	if (seg->hdr->magic != RTR_SHM_MAGIC)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttach() Illegal Shared Memory Magic 0x%lx\n",seg->hdr->magic);
		return -1;
	}
	
	// read all of shared memory to get the pages into cache and mapped
	{
		volatile char *segment = seg->base;
		volatile int i;
		volatile char byte;

		for (i=0; i<mStat.st_size; i++)
			byte = segment[i];
	}

	return 1;
}
#endif

void rtrShmSegDetach( rtrShmSeg *seg )
#ifdef _WIN32
{
	if (seg->base)
	{
		UnmapViewOfFile(seg->base);
		seg->base = 0;
	}
	if (seg->shMemSeg)
	{
		CloseHandle(seg->shMemSeg);
		seg->shMemSeg = 0;
	};
	seg->hdr = 0;
};
#else
{
	if (seg->base && seg->hdr)
	{
		munmap(seg->base, seg->hdr->totalLen);
		seg->base = 0;
	}
	if (seg->shMemSeg)
	{
		close(seg->shMemSeg);
		seg->shMemSeg = 0;
	};
	seg->hdr = 0;
}
#endif

char* rtrShmBytesAttach( char **loc, size_t size )
{
	char *ret = *loc;
	*loc += RTR_SHM_ALIGNBYTES(size);
	return ret;
}


rtrShmEventPtr rtrShmSegCreateEvent( rtrShmSeg *seg, const char *key, rtrShmBool manualReset, unsigned short mutexNumber, char *errBuff )
#ifdef _WIN32
{
	HANDLE ret;
	char eventName[__MY_MAX_ADDR];

	if (strlen(key) > (__MY_MAX_ADDR - 10))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateEvent() Illegal shared event key length %zu (key = %s)",strlen(key), key);
		return 0;
	}

	snprintf(eventName,__MY_MAX_ADDR, "%s_evt%u", key, mutexNumber );

	ret = CreateEventA(NULL, manualReset, FALSE, (LPCSTR)eventName);
	if ((ret == NULL) || (ret == INVALID_HANDLE_VALUE))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateEvent() Could not create shared memory event %s",eventName);
		return 0;
	}

	return ret;
};
#else
{
	return _rtrShmMutexOpenInt(1, 1, key, mutexNumber, errBuff); 
}
#endif

void rtrShmSegDestroyEvent( rtrShmEventPtr shmEvent )
#ifdef _WIN32
{
	if (shmEvent)
		CloseHandle(shmEvent);
};
#else
{
	if (!shmEvent) return;
	if (shmEvent->mtx) sem_close(shmEvent->mtx);
	if (shmEvent->name) sem_unlink(shmEvent->name);
	free(shmEvent->name);
	free(shmEvent);
}
#endif

rtrShmEventPtr rtrShmSegAttachEvent( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff )
#ifdef _WIN32
{
	HANDLE ret;
	char eventName[__MY_MAX_ADDR];

	if (strlen(key) > (__MY_MAX_ADDR - 10))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachEvent() Illegal shared event key length %zu (key = %s)",strlen(key), key);
		return 0;
	}

	snprintf(eventName,__MY_MAX_ADDR, "%s_evt%u", key, mutexNumber );

	ret = OpenEventA(EVENT_ALL_ACCESS,FALSE,(LPCSTR)eventName);
	if ((ret == NULL) || (ret == INVALID_HANDLE_VALUE))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachEvent() Could not open shared memory event %s",eventName);
		return 0;
	}

	return ret;
};
#else
{
	return _rtrShmMutexOpenInt(1, 0, key, mutexNumber, errBuff); 
}
#endif

void rtrShmSegDetachEvent( rtrShmEventPtr shmEvent )
#ifdef _WIN32
{
	rtrShmSegDestroyEvent(shmEvent);
};
#else
{
	if (!shmEvent) return;
	if (shmEvent->mtx) sem_close(shmEvent->mtx);
	free(shmEvent);
}
#endif

rtrShmMutexPtr rtrShmSegCreateMutex( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff )
#ifdef _WIN32
{
	HANDLE ret;
	char mutexName[__MY_MAX_ADDR];

	if (strlen(key) > (__MY_MAX_ADDR - 10))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateMutex() Illegal shared mutex key length %zu (key = %s)",strlen(key), key);
		return 0;
	}

	snprintf(mutexName,__MY_MAX_ADDR, "%s_mtx%u", key, mutexNumber );

		/* Create the mutex */
	ret = CreateMutexA(NULL, FALSE, (LPCSTR)mutexName);
	if ((ret == NULL) || (ret == INVALID_HANDLE_VALUE))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateMutex() Could not create shared memory mutex %s",mutexName);
		return 0;
	}

	return ret;
};
#else
{
	return _rtrShmMutexOpenInt(0, 1, key, mutexNumber, errBuff); 
}
#endif

void rtrShmSegDestroyMutex( rtrShmMutexPtr shmMutex )
#ifdef _WIN32
{
	if (shmMutex)
		CloseHandle(shmMutex);
};
#else
{
	if (!shmMutex) return;
	if (shmMutex->mtx) sem_close(shmMutex->mtx);
	if (shmMutex->name) sem_unlink(shmMutex->name);
	free(shmMutex->name);
	free(shmMutex);
}
#endif

rtrShmMutexPtr rtrShmSegAttachMutex( rtrShmSeg *seg, const char *key, unsigned short mutexNumber, char *errBuff )
#ifdef _WIN32
{
	HANDLE ret;
	char mutexName[__MY_MAX_ADDR];

	if (strlen(key) > (__MY_MAX_ADDR - 10))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachMutex() Illegal shared mutex key length %zu (key = %s)",strlen(key), key);
		return 0;
	}

	snprintf(mutexName,__MY_MAX_ADDR, "%s_mtx%u", key, mutexNumber );

		// Create the events
	ret = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName);
	if ((ret == NULL) || (ret == INVALID_HANDLE_VALUE))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachMutex() Could not open shared memory mutex %s",mutexName);
		return 0;
	}

	return ret;
}
#else
{
	return _rtrShmMutexOpenInt(0, 0, key, mutexNumber, errBuff);
}
#endif

void rtrShmSegDetachMutex( rtrShmMutexPtr shmMutex )
#ifdef _WIN32
{
	rtrShmSegDestroyMutex(shmMutex);
}
#else
{
	if (!shmMutex) return;
	if (shmMutex->mtx) sem_close(shmMutex->mtx);
	free(shmMutex);
}
#endif

#ifdef LINUX
//
// Named pipes were only implemented on UNIX
// windows named pipes use a handle and so they cant be used with a select()
//
//
// routines for handing named pipes used with shared memory
// windows named pipes are much different than UNIX named pipes.
// named pipes on UNIX are persistent. However, they are not persistent on windows.
// windows named pipes only exist while both ends of the pipe are connected.
// return 0=failure 
//
rtrShmNamedPipePtr rtrShmSegCreateNamedPipe( const char *key, unsigned short namedPipeNumber, char *errBuff )
{
	int fd;
	int tempLen;
	rtrShmNamedPipePtr pipe;
	char namedPipeName[__MY_MAX_ADDR];

	if (strlen(key) > (__MY_MAX_ADDR - 20))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() Illegal named pipe key length %zu (key = %s)",strlen(key), key);
		return 0;
	}

	snprintf(namedPipeName,__MY_MAX_ADDR, RTR_NAMEDPIPE_NAME, key, namedPipeNumber );	//TODO: fix the name
	if (mkfifo(namedPipeName, RTR_NAMEDPIPE_PERM) != 0)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() mkfifo failed (errno = %d, name = %s)", errno, namedPipeName);
		return 0;
	}
	if ((pipe = malloc(sizeof(rtrNamedPipe))) == NULL)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() malloc pipe failed (errno = %d, name = %s)", errno, namedPipeName);
		return 0;
	}
	tempLen = strlen(namedPipeName) + 1;
	if ((pipe->name = malloc(tempLen)) == NULL)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() malloc pipe name failed (errno = %d, name = %s)", errno, namedPipeName);
		free(pipe);
		return 0;
	}
	strcpy(pipe->name, namedPipeName);
	pipe->fd = fd;

	return pipe;
}

SOCKET rtrShmSegAcceptNamedPipe( rtrShmNamedPipePtr namedPipe, char *errBuff )
{
	SOCKET acceptFD;

	if ((acceptFD = open(namedPipe->name, RTR_NAMEDPIPE_CREATE_FLAGS)) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptNamedPipe() accept failed (errno = %d)", errno);
		return -1;
	}

	if (fcntl( acceptFD, F_SETFL, fcntl(acceptFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptNamedPipe() ioctl failed (errno = %d)", errno);
		return -1;
	}
	namedPipe->fd = acceptFD;
	return acceptFD;
}

// 
// named pipes are persistent on UNIX. So, they must be removed.
//
void rtrShmSegDestroyNamedPipe( rtrShmNamedPipePtr shmNamedPipe )
{
	if (!shmNamedPipe) return;

	rtrShmSegDetachNamedPipe(shmNamedPipe);
	unlink (shmNamedPipe->name);
}

rtrShmNamedPipePtr rtrShmSegAttachNamedPipe( const char *key, unsigned short namedPipeNumber, char *errBuff )
{
	SOCKET fd;
	int tempLen;
	rtrShmNamedPipePtr pipe;
	char namedPipeName[__MY_MAX_ADDR];
#if WIN32
	unsigned long arg = 1;
#endif

	if (strlen(key) > (__MY_MAX_ADDR - 20))
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachNamedPipe() Illegal shared mutex key length %zu (key = %s)",strlen(key), key);
		return 0;
	}
	snprintf(namedPipeName, __MY_MAX_ADDR, RTR_NAMEDPIPE_NAME, key, namedPipeNumber );

	if ((fd = open(namedPipeName, RTR_NAMEDPIPE_ATTACH_FLAGS)) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() open failed (errno = %d, name = %s)", errno, namedPipeName);
		return 0;
	}
	// TODO: do a blocking open (for the prototype)
	if (fcntl( fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptNamedPipe() ioctl failed (errno = %d)", errno);
		return 0;
	}

	// TODO: why do we malloc this? We also do this for the mutex
	if ((pipe = malloc(sizeof(rtrNamedPipe))) == NULL)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() malloc pipe failed (errno = %d, name = %s)", errno, namedPipeName);
		return 0;
	}
	// TODO: this name has a max length. SHould we really malloc this? This is done for the mutex too
	tempLen = strlen(namedPipeName) + 1;
	if ((pipe->name = malloc(tempLen)) == NULL)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegCreateNamedPipe() malloc pipe name failed (errno = %d, name = %s)", errno, namedPipeName);
		free(pipe);
		return 0;
	}
	strcpy(pipe->name, namedPipeName);
	pipe->fd = fd;

	return pipe;
}

void rtrShmSegDetachNamedPipe( rtrShmNamedPipePtr shmNamedPipe )
{
	if (shmNamedPipe == 0)
		return;

	close (shmNamedPipe->fd);
}
#endif

//
// routines for handing TCP sockets used with shared memory
// return 0=failure 
//
rtrShmSocket rtrShmSegBindSocketTCP( const char *portNumber, char *errBuff, RwfBool blocking )
{
	rtrShmSocket serverFD;
	struct sockaddr_in mySAddr;
	int port;
#if WIN32
	unsigned long arg = 1;
	int flag = 1;
#endif

	port = atoi(portNumber);
	mySAddr.sin_family = AF_INET;
	mySAddr.sin_port = htons( port );
	mySAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

	if ((serverFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC )) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketTCP() socket failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}
#if defined(_WIN32) && defined(SO_EXCLUSIVEADDRUSE)
	if (setsockopt(serverFD, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&flag, sizeof(flag)) < 0)
	{
		closesocket(serverFD);
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketTCP() SO_EXCLUSIVEADDRUSE failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}
#endif
	if (bind( serverFD, (struct sockaddr*) &mySAddr, sizeof mySAddr ) == -1)
	{
		closesocket(serverFD);
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketTCP() bind failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}

	if (!blocking)
	{
#if WIN32
		if (ioctlsocket( serverFD, FIONBIO, &arg ) == -1)
#else
		if (fcntl( serverFD, F_SETFL, fcntl(serverFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
#endif
		{
			snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketTCP() ioctlsocket failed (errno = %d)", errno);
			return -1;
		}
	}
	if (listen( serverFD, 1 ) == -1)
	{
		closesocket(serverFD);
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketTCP() listen failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}

	return serverFD;
}


rtrShmSocket rtrShmSegAcceptSocketTCP( rtrShmSocket serverFD, char *errBuff, RwfBool blocking )
{
	struct sockaddr_in cliSAddr;
	rtrShmSocket acceptFD;
	int flag = 1;
	int SAddrSize = sizeof( struct sockaddr_in );
#if WIN32
	unsigned long arg = 1;
#define ArgType int*
#else
#define ArgType socklen_t*
#endif

	if ((acceptFD = accept( serverFD, (struct sockaddr *)&cliSAddr, (ArgType)&SAddrSize )) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptSocketTCP() accept failed (errno = %d)", errno);
		return -1;
	}

	if (!blocking)
	{
#if WIN32
		if (ioctlsocket( acceptFD, FIONBIO, &arg ) == -1)
#else
		if (fcntl( acceptFD, F_SETFL, fcntl(acceptFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
#endif
		{
			snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptSocketTCP() ioctlsocket failed (errno = %d)", errno);
			return -1;
		}
	}

	if ((setsockopt( acceptFD, IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int))) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptSocketTCP() setsockopt failed (errno = %d)", errno);
		return -1;
	}
	return acceptFD;
}

rtrShmSocket rtrShmSegAttachSocketTCP( char *portNumber, char *errBuff, RwfBool blocking )
{
	struct sockaddr_in srvrSAddr;
	rtrShmSocket clientFD;
	int flag = 1;
	int port;
#if WIN32
	unsigned long arg = 1;
#endif

	if ((clientFD = socket( AF_INET, SOCK_STREAM, PF_UNSPEC )) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketTCP() socket failed (errno = %d)", errno);
		return -1;
	}

	if (setsockopt( clientFD, IPPROTO_TCP, TCP_NODELAY,(char *) &flag, sizeof(int)) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketTCP() setsockopt failed (errno = %d)", errno);
		return -1;
	}

	if (!blocking)
	{
#if WIN32
		if (ioctlsocket( clientFD, FIONBIO, &arg ) == -1)
#else
		if (fcntl( clientFD, F_SETFL, fcntl(clientFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
#endif
		{
			snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketTCP() ioctlsocket failed (errno = %d)", errno);
			return -1;
		}
	}

	port = atoi(portNumber);
	srvrSAddr.sin_family = AF_INET;
	srvrSAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	srvrSAddr.sin_port = htons( port );

	if (connect( clientFD, (struct sockaddr* ) &srvrSAddr, sizeof srvrSAddr ) == -1)
	{
#if _WIN32
		if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK)
#else
		if ((errno != EINPROGRESS)&&(errno != EALREADY))
#endif
		{
			snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketTCP() connect failed (errno = %d)", errno);
			return -1;
		}
	}

	return clientFD;
}

//
// routines for handing UDP sockets used with shared memory
// return 0=failure 
//
rtrShmSocket rtrShmSegBindSocketUDP( const char *portNumber, char *errBuff )
{
	rtrShmSocket serverFD;
	struct sockaddr_in mySAddr;
	int port;
#if WIN32
	unsigned long arg = 1;
#endif

	port = atoi(portNumber);
	mySAddr.sin_family = AF_INET;
	mySAddr.sin_port = htons( port );
	mySAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

	if ((serverFD = socket( AF_INET, SOCK_DGRAM, PF_UNSPEC )) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketUDP() socket failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}

	if (bind( serverFD, (struct sockaddr*) &mySAddr, sizeof mySAddr ) == -1)
	{
		closesocket(serverFD);
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegBindSocketUDP() bind failed (errno = %d, port = %s)", errno, portNumber);
		return -1;
	}

#if WIN32
	if (ioctlsocket( serverFD, FIONBIO, &arg ) == -1)
#else
	if (fcntl( serverFD, F_SETFL, fcntl(serverFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
#endif
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptSocketUDP() ioctlsocket failed (errno = %d)", errno);
		return -1;
	}


	return serverFD;
}
//
// this isnt the same kind of accept that TCP does. 
// This does a read and gets the network address of the sender so we know the client we are talking to
//
 int rtrShmSegAcceptSocketUDP( rtrShmSocket serverFD, struct sockaddr_in *cliAddr, int *cliLen, char *errBuff )
{
	int count;
	char buffer[BUFSIZ];

#if WIN32
#define ArgType int*
#else
#define ArgType socklen_t*
#endif

	if ((count = recvfrom(serverFD, buffer, sizeof(buffer), 0, (struct sockaddr *) cliAddr, (ArgType) cliLen)) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAcceptSocketTCP() recvfrom failed (errno = %d)", errno);
		return -1;
	}

	return 0;
}

rtrShmSocket rtrShmSegAttachSocketUDP( char *portNumber, char *errBuff )
{
	struct sockaddr_in srvrSAddr;
	rtrShmSocket clientFD;
	int port;
#if WIN32
	unsigned long arg = 1;
#endif

	if ((clientFD = socket( AF_INET, SOCK_DGRAM, PF_UNSPEC )) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketUDP() socket failed (errno = %d)", errno);
		return -1;
	}

	port = atoi(portNumber);
	srvrSAddr.sin_family = AF_INET;
	srvrSAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	srvrSAddr.sin_port = htons( port );

	if (connect( clientFD, (struct sockaddr* ) &srvrSAddr, sizeof srvrSAddr ) == -1)
	{
#if _WIN32
		if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK)
#else
		if ((errno != EINPROGRESS)&&(errno != EALREADY))
#endif
		{
			snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketUDP() connect failed (errno = %d)", errno);
			return -1;
		}
	}

	if (send(clientFD, "1", 1, 0) == -1)
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketUDP() socket sendto failed (errno = %d)", errno);
		return -1;
	}

#if WIN32
	if (ioctlsocket( clientFD, FIONBIO, &arg ) == -1)
#else
	if (fcntl( clientFD, F_SETFL, fcntl(clientFD, F_GETFL, 0) | O_NONBLOCK | O_NDELAY ) == -1)
#endif
	{
		snprintf(errBuff,__ERROR_LEN, "rtrShmSegAttachSocketUDP() ioctlsocket failed (errno = %d)", errno);
		return -1;
	}

	return clientFD;
}

// These routines can be used for UDP or TCP
void rtrShmSegDetachSocket( rtrShmSocket shmSocket )
{
	if (shmSocket != -1)
		closesocket (shmSocket);
}

void rtrShmSegDestroySocket( rtrShmSocket shmBindSocket )
{
	if (shmBindSocket != -1)
		closesocket(shmBindSocket);
}


