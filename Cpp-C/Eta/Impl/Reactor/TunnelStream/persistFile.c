/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/persistFile.h"
#include "rtr/rsslReactorUtils.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#define rssl_errno (GetLastError())
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#define rssl_errno errno
#endif

/* Persistence store file format version. */
typedef enum
{
	PERS_VER_0_1 = 0,
	PERS_VER_0_2 = 1, 	/* Java persistence file */
	PERS_VER_0_3 = 2		/* C persistence file */
} PersistenceVersion;

typedef enum
{
	PERS_MP_NEXT_MSG		= 0,
	PERS_MP_FLAGS			= PERS_MP_NEXT_MSG + 4,
	PERS_MP_LENGTH			= PERS_MP_FLAGS + 4,
	PERS_MP_TIME_QUEUED		= PERS_MP_LENGTH + 4,
	PERS_MP_TIME_TO_LIVE	= PERS_MP_TIME_QUEUED + 8,
	PERS_MP_MSG_BUFFER		= PERS_MP_TIME_TO_LIVE + 8,
	PERS_MP_END				= PERS_MP_MSG_BUFFER + 4
} PersistentMsgPosition;

/* Flags for a persistent message. */
typedef enum
{
	PERS_MF_NONE		= 0,	/* None. */
	PERS_MF_TRANSMITTED	= 0x1	/* Message has been previously transmitted and has a sequence number. Do not attempt to expire it. */
} PersistentMsgFlags;

typedef enum
{
	PERS_HF_NONE		= 0x00	/* None */
} PersistFileFlags;

/* Persistence file header positions */
typedef enum
{
	PERS_HP_FILE_VERSION			= 0,
	PERS_HP_MAX_MSGS				= PERS_HP_FILE_VERSION + 4,
	PERS_HP_MAX_MSG_LEN				= PERS_HP_MAX_MSGS + 4,
	PERS_HP_CUR_MSG_COUNT			= PERS_HP_MAX_MSG_LEN + 4,
	PERS_HP_LAST_OUT_SEQ_NUM		= PERS_HP_CUR_MSG_COUNT + 4,
	PERS_HP_LAST_IN_SEQ_NUM			= PERS_HP_LAST_OUT_SEQ_NUM +4,
	PERS_HP_FREE_HEAD				= PERS_HP_LAST_IN_SEQ_NUM + 4,
	PERS_HP_SAVED_HEAD				= PERS_HP_FREE_HEAD + 4,

	PERS_HP_FLAGS					= PERS_HP_SAVED_HEAD + 4,
	PERS_HP_END						= PERS_HP_FLAGS + 4
} PersistenceHeaderPosition;


/* Update file links to a message from one list to the other. */
static RsslRet persistFileMoveMsg(PersistFile *pFile,
		RsslQueue *pOldQueue, RsslUInt32 oldListHeadFilePos, 
		RsslQueue *pNewQueue, RsslUInt32 newListHeadFilePos,
		PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo);

RsslBool persistFileLock(PersistFile *pFile)
{
#ifdef WIN32
	return (LockFile(pFile->_file, 0, 0, 0, 0) == 0) ? RSSL_FALSE : RSSL_TRUE;
#else
	struct flock flockOpts;
	memset(&flockOpts, 0, sizeof(flockOpts));
	flockOpts.l_type = F_RDLCK | F_WRLCK;
	flockOpts.l_whence = SEEK_SET;
	return (fcntl(pFile->_file, F_SETLK, &flockOpts) < 0) ? RSSL_FALSE : RSSL_TRUE;

#endif
}

RTR_C_INLINE RsslRet fileRead(PersistFile *pFile, RsslUInt32 position, RsslUInt32 length, void *pValue)
{
#ifdef WIN32
	DWORD outBytes;
#endif
	/* Move to writing position */
#ifdef WIN32
	if (SetFilePointer(pFile->_file, position, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
#else
	if (lseek(pFile->_file, position, SEEK_SET) < 0)
#endif
		return RSSL_RET_FAILURE;

	/*  Read value */
#ifdef WIN32
	if (ReadFile(pFile->_file, pValue, length, &outBytes, NULL) != TRUE)
#else
	if (read(pFile->_file, pValue, length) != length)
#endif
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;

}

RTR_C_INLINE RsslRet fileWrite(PersistFile *pFile, RsslUInt32 position, RsslUInt32 length, void *pValue)
{
#ifdef WIN32
	DWORD outBytes;
#endif

	/* Move to writing position */
#ifdef WIN32
	if (SetFilePointer(pFile->_file, position, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
#else
	if (lseek(pFile->_file, position, SEEK_SET) < 0)
#endif
		return RSSL_RET_FAILURE;

	/*  Write value */
#ifdef WIN32
	if (WriteFile(pFile->_file, pValue, length, &outBytes, NULL) != TRUE)
#else
	if (write(pFile->_file, pValue, length) != length)
#endif
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/* Reads an Int32 from a file. */
RTR_C_INLINE RsslRet fileReadUInt32(PersistFile *pFile, RsslUInt32 position, RsslUInt32 *pValue)
{
	return fileRead(pFile, position, 4, (void*)pValue);
}

/* Reads an Int64 from a file. */
RTR_C_INLINE RsslRet fileReadInt64(PersistFile *pFile, RsslUInt32 position, RsslInt64 *pValue)
{
	return fileRead(pFile, position, 8, (void*)pValue);
}

/* Reads a buffer from a file. */
RTR_C_INLINE RsslRet fileReadBuffer(PersistFile *pFile, RsslUInt32 position, RsslBuffer *pBuffer)
{
	return fileRead(pFile, position, pBuffer->length, (void*)pBuffer->data);
}

/* Writes an Int32 to a file. */
RTR_C_INLINE RsslRet fileWriteUInt32(PersistFile *pFile, RsslUInt32 position, RsslUInt32 value)
{
	return fileWrite(pFile, position, 4, (void*)&value);
}

/* Writes an Int64 to a file. */
RTR_C_INLINE RsslRet fileWriteInt64(PersistFile *pFile, RsslUInt32 position, RsslInt64 value)
{
	return fileWrite(pFile, position, 8, (void*)&value);
}

/* Writes a buffer to a file. */
RTR_C_INLINE RsslRet fileWriteBuffer(PersistFile *pFile, RsslUInt32 position, RsslBuffer *pBuffer)
{
	return fileWrite(pFile, position, pBuffer->length, (void*)pBuffer->data);
}

RTR_C_INLINE RsslRet persistFileCommit(PersistFile *pFile, RsslErrorInfo *pErrorInfo)
{
#ifdef WIN32
	if (FlushFileBuffers(pFile->_file) == FALSE)
#else
	if (fsync(pFile->_file) < 0)
#endif
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Persistence file sync failed: SysError %d", rssl_errno);
		pErrorInfo->rsslError.sysError = rssl_errno;
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/* Populates a list of persisted message buffers from the file. */
static RsslRet persistenceLoadMsgList(PersistFile *pFile, RsslQueue *pQueue, RsslUInt32 filePosition, RsslUInt32 *pTransmittedCount, RsslErrorInfo *pErrorInfo)
{
	*pTransmittedCount = 0;

	while (filePosition != 0)
	{
		PersistentMsg *pMsg;

		if (rsslQueueGetElementCount(pQueue) > pFile->_maxMsgCount)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Message list exceeds max message count. Persistence file may be corrupt.");
			return RSSL_RET_FAILURE;
		}

		/* Load message */
		if ((pMsg = (PersistentMsg*)malloc(sizeof(PersistentMsg))) == NULL)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to allocate persistent msg reference");
			return RSSL_RET_FAILURE;
		}

		pMsg->_filePosition = filePosition;

		if (fileReadUInt32(pFile, filePosition + PERS_MP_FLAGS, &pMsg->_flags) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file message flags.");
			return RSSL_RET_FAILURE;
		}

		if (pMsg->_flags & PERS_MF_TRANSMITTED)
			++*pTransmittedCount;

		if (fileReadUInt32(pFile, filePosition + PERS_MP_LENGTH, &pMsg->_msgLength) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file message length.");
			return RSSL_RET_FAILURE;
		}

		if (fileReadInt64(pFile, filePosition + PERS_MP_TIME_QUEUED, &pMsg->_timeQueued) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file message time queued.");
			return RSSL_RET_FAILURE;
		}

		if (fileReadInt64(pFile, filePosition + PERS_MP_TIME_TO_LIVE, &pMsg->_timeout) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file message timeout.");
			return RSSL_RET_FAILURE;
		}

		/* Get next message position. */
		if (fileReadUInt32(pFile, filePosition + PERS_MP_NEXT_MSG, &filePosition) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file message link position.");
			return RSSL_RET_FAILURE;
		}

		rsslQueueAddLinkToBack(pQueue, &pMsg->_qLink);

	}

	return RSSL_RET_SUCCESS;
}

RsslRet persistenceFreeMsg(PersistFile *pFile, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	/* Update file state and our queue links. */
	if (persistFileMoveMsg(pFile, &pFile->_savedList, PERS_HP_SAVED_HEAD, 
				&pFile->_freeList, PERS_HP_FREE_HEAD, pMsg,
				pErrorInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	persistentMsgClear(pMsg);

	return RSSL_RET_SUCCESS;
}


PersistentMsg *persistFileSaveMsg(PersistFile *pFile, RsslBuffer *pBuffer, RsslInt64 msgTimeoutMs,
		RsslInt64 currentTimeMs, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;
	PersistentMsg *pMsg;

	if (pBuffer->length > pFile->_maxMsgLength)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Message length is greater than persistence file maximum message length.");
		return NULL;
	}

	/* Get a buffer. */
	if ((pLink = rsslQueuePeekFront(&pFile->_freeList)) == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_PERSISTENCE_FULL,
				__FILE__, __LINE__, "Local persistence file is full.  Space may become available later as delivered messages are acknowledged.");

		return NULL;
	}

	pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);

	persistentMsgClear(pMsg);

	pMsg->_msgLength = pBuffer->length;
	pMsg->_timeout = msgTimeoutMs;
	pMsg->_timeQueued = currentTimeMs;

	/* Write flags */
	if (fileWriteUInt32(pFile, pMsg->_filePosition + PERS_MP_FLAGS, pMsg->_flags))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to write persistent message flags.");
		return NULL;
	}

	/* Write length */
	if (fileWriteUInt32(pFile, pMsg->_filePosition + PERS_MP_LENGTH, pMsg->_msgLength))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to write persistent message length.");
		return NULL;
	}

	if (fileWriteInt64(pFile, pMsg->_filePosition + PERS_MP_TIME_TO_LIVE, pMsg->_timeout))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to write persistent message timeout.");
		return NULL;
	}

	if (pMsg->_timeout > RDM_QMSG_TC_IMMEDIATE)
		if (fileWriteInt64(pFile, pMsg->_filePosition + PERS_MP_TIME_QUEUED, pMsg->_timeQueued))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to write persistent message time queued.");
			return NULL;
		}



	/* Write buffer. */
	if (fileWriteBuffer(pFile, pMsg->_filePosition + PERS_MP_END, pBuffer)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to write persistent message to file.");
		return NULL;
	}

	/* Update file state and our queue links. */
	if (persistFileMoveMsg(pFile, &pFile->_freeList, PERS_HP_FREE_HEAD,
				&pFile->_savedList, PERS_HP_SAVED_HEAD, pMsg,
				pErrorInfo) != RSSL_RET_SUCCESS)
		return NULL;


	return pMsg;
}

RsslRet persistFileReadSavedMsg(PersistFile *pFile, RsslBuffer *pBuffer, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	assert(pMsg->_msgLength <= pBuffer->length);

	if (fileReadBuffer(pFile, pMsg->_filePosition + PERS_MP_END, pBuffer) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to read persisted message.");
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet persistentMsgUpdateForTransmit(PersistFile *pFile, PersistentMsg *pMsg, RsslBuffer *pBuffer, RsslUInt32 *pLastOutSeqNum, RsslErrorInfo *pErrorInfo)
{
	if (!(pMsg->_flags & PERS_MF_TRANSMITTED))
	{
		/* Mark message as transmitted so we will stop processing its timeout. */
		pMsg->_flags |= PERS_MF_TRANSMITTED;
		if (fileWriteUInt32(pFile, pMsg->_filePosition + PERS_MP_FLAGS, 
					pMsg->_flags) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to mark persisted message as transmitted.");
			return RSSL_RET_FAILURE;
		}

		/* Update the last sent sequence number. */
		if (fileWriteUInt32(pFile, PERS_HP_LAST_OUT_SEQ_NUM, *pLastOutSeqNum + 1)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to update last sent sequence number while transmitting message.");
			return RSSL_RET_FAILURE;
		}

		pMsg->_seqNum = *pLastOutSeqNum + 1;

		if (persistFileCommit(pFile, pErrorInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

		++*pLastOutSeqNum;

	}

	/* No list move -- message is still in the saved list. */
		
	return RSSL_RET_SUCCESS;
}

RsslRet persistFileFreeMsgs(PersistFile *pFile, RsslUInt32 seqNum, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;

	while ((pLink = rsslQueuePeekFront(&pFile->_savedList)) != NULL)
	{
		PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
		RsslRet ret;

		/* Stop if we reach a message past this sequence number, or one that has never
		 * been transmitted (and thus has no sequence number assigned) */
		if (!(pMsg->_flags & PERS_MF_TRANSMITTED) || rsslSeqNumCompare(pMsg->_seqNum, seqNum) > 0)
			break;

		if ((ret = persistenceFreeMsg(pFile, pMsg, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet persistFileSaveLastInSeqNum(PersistFile *pFile, RsslUInt32 seqNum, RsslErrorInfo *pErrorInfo)
{
	if (fileWriteUInt32(pFile, PERS_HP_LAST_IN_SEQ_NUM, seqNum)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to write persistence file last received sequence number.");
		return RSSL_RET_FAILURE;
	}

	if (persistFileCommit(pFile, pErrorInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;
	
	return RSSL_RET_SUCCESS;
}

PersistFile *persistFileOpen(PersistFileOpenOptions *pOpts, RsslUInt32 *pLastInSeqNum, RsslUInt32 *pLastOutSeqNum, RsslErrorInfo *pErrorInfo)
{
	PersistFile *pFile;
	RsslBool fileExists;

	assert(pOpts->filename);

	pFile = (PersistFile*)malloc(sizeof(PersistFile));
	if (pFile == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to allocate persistent file object.");
		return NULL;
	}

	memset(pFile, 0, sizeof(PersistFile));
#ifdef WIN32
	pFile->_file = INVALID_HANDLE_VALUE;
#endif
	rsslInitQueue(&pFile->_freeList);
	rsslInitQueue(&pFile->_savedList);
		
	/* Ideally checking for file existence and opening it would be atomic,
	 * but have not found a portable way to do this. */
#ifdef WIN32
	if ((pFile->_file = CreateFile(pOpts->filename, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
#else
	if ((pFile->_file = open(pOpts->filename, O_RDWR)) < 0)
#endif
	{
		if (rssl_errno == ENOENT)
			fileExists = RSSL_FALSE;
		else
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to open persistence file: SysError %d", rssl_errno);
			pErrorInfo->rsslError.sysError = rssl_errno;
			persistFileClose(pFile);
			return NULL;
		}

	}
	else
		fileExists = RSSL_TRUE;

#ifdef WIN32
		if (!fileExists && (pFile->_file = CreateFile(pOpts->filename, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
#else
	if (!fileExists && (pFile->_file = open(pOpts->filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
#endif
	{
		if (rssl_errno == ENOENT)
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to open new persistence file: SysError %d. Ensure that the persistence file path exists.", rssl_errno);
		else
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to open new persistence file: SysError %d", rssl_errno);
		pErrorInfo->rsslError.sysError = rssl_errno;
		persistFileClose(pFile);
		return NULL;
	}

	/* Try to acquire the file lock, to ensure that it is not in use. */
	if (persistFileLock(pFile) == RSSL_FALSE)
	{
		if (rssl_errno == EWOULDBLOCK)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Persistence file is already locked; it may be in use.");
		}
		else
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to lock persistence file: SysError %d.", rssl_errno);
			pErrorInfo->rsslError.sysError = rssl_errno;
		}

		persistFileClose(pFile);
		return NULL;
	}

	pFile->_streamId = pOpts->streamId;

	if (!fileExists)
	{
		int fileSize = PERS_HP_END
		   + pOpts->maxMsgCount * (PERS_MP_END + pOpts->maxMsgSize);

		RsslUInt32 i;

		pFile->_version = PERS_VER_0_3;
		pFile->_maxMsgCount = pOpts->maxMsgCount;
		pFile->_maxMsgLength = pOpts->maxMsgSize;

		/* Set file size. */
#ifdef WIN32
		if (SetEndOfFile(pFile->_file) == 0)
#else
		if (ftruncate(pFile->_file, fileSize) < 0)
#endif
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to set persistence file size: SysError %d", rssl_errno);
			pErrorInfo->rsslError.sysError = rssl_errno;
			persistFileClose(pFile);
			return NULL;
		}

		/* Initialize file header. */

		if (fileWriteUInt32(pFile, PERS_HP_MAX_MSGS, pFile->_maxMsgCount)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file max messages.");
			persistFileClose(pFile);
			return NULL;
		}

		if (fileWriteUInt32(pFile, PERS_HP_MAX_MSG_LEN, pFile->_maxMsgLength)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file max message length.");
			persistFileClose(pFile);
			return NULL;
		}

		if (fileWriteUInt32(pFile, PERS_HP_CUR_MSG_COUNT, 0)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file msg count.");
			persistFileClose(pFile);
			return NULL;
		}

		if (fileWriteUInt32(pFile, PERS_HP_LAST_OUT_SEQ_NUM, 0)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file last sent sequence number.");
			persistFileClose(pFile);
			return NULL;
		}

		if (fileWriteUInt32(pFile, PERS_HP_LAST_IN_SEQ_NUM, 0)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file last received sequence number.");
			persistFileClose(pFile);
			return NULL;
		}

		/* First free buffer is right after the file header. */
		if (fileWriteUInt32(pFile, PERS_HP_FREE_HEAD, PERS_HP_END)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file free list head.");
			persistFileClose(pFile);
			return NULL;
		}

		if (fileWriteUInt32(pFile, PERS_HP_SAVED_HEAD, 0)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file saved list head.");
			persistFileClose(pFile);
			return NULL;
		}

		for(i = 0; i < pFile->_maxMsgCount; ++i)
		{
			PersistentMsg *pMsg;
			if ((pMsg = (PersistentMsg*)malloc(sizeof(PersistentMsg))) == NULL)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
						__FILE__, __LINE__, "Failed to allocate persistence file message object.");
				persistFileClose(pFile);
				return NULL;
			}
			memset(pMsg, 0, sizeof(PersistentMsg));

			rsslQueueAddLinkToBack(&pFile->_freeList, &pMsg->_qLink);
			pMsg->_filePosition = PERS_HP_END + i * (PERS_MP_END + pFile->_maxMsgLength);

			if (fileWriteUInt32(pFile, pMsg->_filePosition + PERS_MP_NEXT_MSG, 
				(i < pFile->_maxMsgCount - 1) ? 
				pMsg->_filePosition + (PERS_MP_END + pFile->_maxMsgLength) : 0)
				!= RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file message.");
				persistFileClose(pFile);
				return NULL;
			}
		}

		pFile->_flags = PERS_HF_NONE;
		if (fileWriteUInt32(pFile, PERS_HP_FLAGS, pFile->_flags)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file flags.");
			persistFileClose(pFile);
			return NULL;
		}

		/* File successfully initialized. Write the file version. */
		if (fileWriteUInt32(pFile, PERS_HP_FILE_VERSION, pFile->_version)
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to initialize persistence file version.");
			persistFileClose(pFile);
			return NULL;
		}

		if (persistFileCommit(pFile, pErrorInfo) != RSSL_RET_SUCCESS)
		{
			persistFileClose(pFile);
			return NULL;
		}
	}
	else
	{
		RsslUInt32 currentMsgCount;
		RsslUInt32 freeListHead;
		RsslUInt32 savedListHead;
		RsslQueueLink *pLink;
		RsslUInt32 tmpSeqNum;
		RsslUInt32 transmittedCount;

		if ((fileReadUInt32(pFile, PERS_HP_FILE_VERSION, &pFile->_version)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file version.");
			persistFileClose(pFile);
			return NULL;
		}

		/* Check for known version. */
		if (pFile->_version != PERS_VER_0_3)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Unrecognized persistence file version.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_FLAGS, &pFile->_flags)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to read persistence file header flags.");
			persistFileClose(pFile);
			return NULL;
		}
		

		if ((fileReadUInt32(pFile, PERS_HP_MAX_MSGS, &pFile->_maxMsgCount)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file max msg count.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_MAX_MSG_LEN, &pFile->_maxMsgLength)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file max msg length.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_CUR_MSG_COUNT, &currentMsgCount)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file current msg count.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_LAST_OUT_SEQ_NUM, pLastOutSeqNum)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file last sent sequence number.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_LAST_IN_SEQ_NUM, pLastInSeqNum)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Failed to read persistence file last received sequence number.");
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_FREE_HEAD, &freeListHead)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to read persistence file free list head."); 
			persistFileClose(pFile);
			return NULL;
		}

		if ((fileReadUInt32(pFile, PERS_HP_SAVED_HEAD, &savedListHead)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to read persistence file saved list head.");
			persistFileClose(pFile);
			return NULL;
		}
		
		/* Populate lists. */

		if (persistenceLoadMsgList(pFile, &pFile->_freeList, freeListHead, &transmittedCount, pErrorInfo) != RSSL_RET_SUCCESS)
		{
			persistFileClose(pFile);
			return NULL;
		}

		if (persistenceLoadMsgList(pFile, &pFile->_savedList, savedListHead, &transmittedCount, pErrorInfo) != RSSL_RET_SUCCESS)
		{
			persistFileClose(pFile);
			return NULL;
		}

		/* Derive sequence numbers of transmitted messages (as they do not expire,
		 * they will always be sequential and so don't need to be saved). */
		tmpSeqNum = *pLastOutSeqNum - transmittedCount;
		for(pLink = rsslQueueStart(&pFile->_savedList); pLink != NULL;
				pLink = rsslQueueForth(&pFile->_savedList))
		{
			PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);

			if (!(pMsg->_flags & PERS_MF_TRANSMITTED))
				break;

			pMsg->_seqNum = tmpSeqNum + 1;
			++tmpSeqNum;
		}

		if (rsslQueueGetElementCount(&pFile->_freeList) + rsslQueueGetElementCount(&pFile->_savedList) 
			!= pFile->_maxMsgCount)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
					__FILE__, __LINE__, "Message lists count do not match max message count. Persistence file may be corrupt.");
			persistFileClose(pFile);
			return NULL;
		}

	}
	
	return pFile;
}

void persistFileClose(PersistFile *pFile)
{
	RsslQueueLink *pLink;

#ifdef WIN32
	if (pFile->_file != INVALID_HANDLE_VALUE)
		CloseHandle(pFile->_file);
#else
	if (pFile->_file != 0)
		close(pFile->_file);
#endif

	for (pLink = rsslQueueStart(&pFile->_freeList); pLink != NULL;
			pLink = rsslQueueForth(&pFile->_freeList))
	{
		PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
		free(pMsg);
	}

	for (pLink = rsslQueueStart(&pFile->_savedList); pLink != NULL;
			pLink = rsslQueueForth(&pFile->_savedList))
	{
		PersistentMsg *pMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
		free(pMsg);
	}

	free(pFile);
}

static RsslRet persistFileMoveMsg(PersistFile *pFile,
		RsslQueue *pOldQueue, RsslUInt32 oldListHeadFilePos, 
		RsslQueue *pNewQueue, RsslUInt32 newListHeadFilePos,
		PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink, *pPrevLink;
	PersistentMsg *pNextMsg;
	RsslUInt32 oldLinkPos, oldLinkOldValue, oldLinkNewValue;
	RsslUInt32 newLinkPos;

	/* The old list head will be updated to the next message (if any) from
	 * the one we're moving. */
	if ((pLink = rsslQueuePeekNext(pOldQueue, &pMsg->_qLink)) != NULL)
	{
		pNextMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
		oldLinkNewValue = pNextMsg->_filePosition;
	}
	else
	{
		pNextMsg = NULL;
		oldLinkNewValue = 0;
	}

	/* If this message is at the head of the old queue, save & update
	 * the list head. Otherwise, save and update the link of the
	 * buffer that precedes this one. */
	if ((pPrevLink = rsslQueuePeekPrev(pOldQueue, &pMsg->_qLink)) == NULL)
	{
		oldLinkPos = oldListHeadFilePos;
		oldLinkOldValue = pMsg->_filePosition;
	}
	else
	{
		PersistentMsg *pPrevMsg = RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pPrevLink);
		oldLinkPos = pPrevMsg->_filePosition + PERS_MP_NEXT_MSG;
		oldLinkOldValue = pMsg->_filePosition;
	}
	/* Store new list tail. */
	if ((pLink = rsslQueuePeekBack(pNewQueue)) != NULL)
	{
		PersistentMsg *pNewListTailMsg = 
			RSSL_QUEUE_LINK_TO_OBJECT(PersistentMsg, _qLink, pLink);
		newLinkPos = pNewListTailMsg->_filePosition + PERS_MP_NEXT_MSG;
		
	}
	else
	{
		newLinkPos = newListHeadFilePos;
	}

	/* Update old list */
	if (fileWriteUInt32(pFile, oldLinkPos, oldLinkNewValue)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to save persistence file restore info.");
		persistFileClose(pFile);
		return RSSL_RET_FAILURE;
	}

	/* Update newList */
	if (fileWriteUInt32(pFile, newLinkPos, pMsg->_filePosition)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to save persistence file restore info.");
		persistFileClose(pFile);
		return RSSL_RET_FAILURE;
	}

	/* Update buffer link */
	if (fileWriteUInt32(pFile, pMsg->_filePosition + PERS_MP_NEXT_MSG, 0)
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, 
				__FILE__, __LINE__, "Failed to save persistence file restore info.");
		persistFileClose(pFile);
		return RSSL_RET_FAILURE;
	}

	
	if (persistFileCommit(pFile, pErrorInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	rsslQueueRemoveLink(pOldQueue, &pMsg->_qLink);
	rsslQueueAddLinkToBack(pNewQueue, &pMsg->_qLink);


	return RSSL_RET_SUCCESS;
}

