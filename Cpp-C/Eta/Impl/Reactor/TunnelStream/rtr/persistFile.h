/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "rtr/rsslQueue.h"
#include "rtr/rsslTunnelStream.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	typedef HANDLE RsslFilePtr;
#else
	typedef int RsslFilePtr;
#endif

/* Represents a persistent message, either saved or free. */
typedef struct
{
	RsslQueueLink 	_qLink;			/* Link for freeList/savedList */
	RsslUInt32		_filePosition;	/* Position of this msg in the file. */
	RsslUInt32		_flags;			/* PersistentMsgFlags. */
	RsslUInt32		_seqNum;		/* Sequence number. Note that this is assigned only when the message is transmitted.
									 * Check the PERS_MF_TRANSMITTED flag. */
	RsslUInt32		_msgLength;		/* Length of the persisted message, if saved. */
	RsslInt64		_timeQueued;	/* Time this message was queued, if timeout was not a code. */
	RsslInt64		_timeout;		/* Timeout of this message, if saved. May be a code. */
} PersistentMsg;

/* Clears a persitent message. */
RTR_C_INLINE void persistentMsgClear(PersistentMsg *pMsg);

/* Returns the encoded length of the persisted message. */
RTR_C_INLINE RsslUInt32 persistentMsgGetLength(PersistentMsg *pMsg);

RTR_C_INLINE void persistentMsgSetTimeout(PersistentMsg *pMsg, RsslInt64 timeout);

/* Represents a storage of persisted messages. */
typedef struct
{
	RsslQueue			_freeList;		/* List of available buffers for saving messages. */
	RsslQueue			_savedList;		/* List of buffers saved to the file. */
	RsslUInt32			_flags;			/* PersistFileFlags */

	RsslFilePtr			_file;			/* Persistence file, if this persistence is file-backed. */
	RsslInt32			_streamId;		/* Stream ID of the messages in this persistent store. */
	RsslUInt32			_version;		/* Version of persistence format in use. */

	RsslUInt32			_maxMsgLength;	/* Maximum size of messages that can be stored. */
	RsslUInt32			_maxMsgCount;	/* Maximum number of messages present in the file. */
} PersistFile;

/* Save an encoded message. */
PersistentMsg *persistFileSaveMsg(PersistFile *pFile, RsslBuffer *pBuffer, RsslInt64 msgTimeoutMs,
		RsslInt64 currentTimeMs, RsslErrorInfo *pErrorInfo);

/* Copy a persisted message from the storage into a buffer. This is meant for sending buffers
 * that were found in the persistence file when it was opened and
 * should be sent before application messages are allowed to be transmitted
 * for this queue stream. */
RsslRet persistFileReadSavedMsg(PersistFile *pFile, RsslBuffer *pBuffer, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Mark a message as transmitted and set a sequence number.
 * Use when the message is about to be sent. */
RsslRet persistentMsgUpdateForTransmit(PersistFile *pFile, PersistentMsg *pMsg, RsslBuffer *pBuffer, RsslUInt32 *pLastOutSeqNum, RsslErrorInfo *pErrorInfo);

/* Frees messages based on a received sequence number. */
RsslRet persistFileFreeMsgs(PersistFile *pFile, RsslUInt32 seqNum, RsslErrorInfo *pErrorInfo);

/* Frees a specific message. */
RsslRet persistenceFreeMsg(PersistFile *pFile, PersistentMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Returns the saved message list. Used to resend messages. */
RTR_C_INLINE RsslQueue *persistFileGetSavedList(PersistFile *pFile);

/* Set the last received sequence number. */
RsslRet persistFileSaveLastInSeqNum(PersistFile *pFile, RsslUInt32 seqNum, RsslErrorInfo *pErrorInfo);

/* Options for persistFileOpen */
typedef struct
{
	char		*filename;
	RsslInt32	streamId;
	RsslUInt32	maxMsgSize;
	RsslUInt32	maxMsgCount;
	RsslInt64	currentTimeMs;
} PersistFileOpenOptions;

/* Clears a PersistFileOpenOptions structure. */
RTR_C_INLINE void persistFileOpenOptionsClear(PersistFileOpenOptions *pOptions);

/* Creates a persistent storage, and loads a file if a file name is given (creating the file if it does not exist). */
PersistFile *persistFileOpen(PersistFileOpenOptions *pOpts, RsslUInt32 *pLastInSeqNum, RsslUInt32 *pLastOutSeqNum, RsslErrorInfo *pErrorInfo);

/* Cleans up a persistent storage. */
void persistFileClose(PersistFile *pFile);

/* PersistentMsg inline functions */

RTR_C_INLINE void persistentMsgClear(PersistentMsg *pMsg)
{
	pMsg->_flags = 0;
	pMsg->_seqNum = 0;
	pMsg->_msgLength = 0;
	pMsg->_timeQueued = 0;
	pMsg->_timeout = 0;
}

RTR_C_INLINE RsslUInt32 persistentMsgGetLength(PersistentMsg *pMsg)
{
	return pMsg->_msgLength;
}

RTR_C_INLINE void persistentMsgSetTimeout(PersistentMsg *pMsg, RsslInt64 timeout)
{
	pMsg->_timeout = timeout;
}

/* Persistence inline functions */

RTR_C_INLINE void persistFileOpenOptionsClear(PersistFileOpenOptions *pOptions)
{
	memset(pOptions, 0, sizeof(PersistFileOpenOptions));
	pOptions->maxMsgSize = 1024;
	pOptions->maxMsgCount = 1024;
}

RTR_C_INLINE RsslQueue *persistFileGetSavedList(PersistFile *pFile)
{
	return &pFile->_savedList;
}

#ifdef __cplusplus
}
#endif

#endif
