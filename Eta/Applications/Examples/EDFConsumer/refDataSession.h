/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef REF_DATA_SESSION_H
#define REF_DATA_SESSION_H

/* Handles interactions with the ref data server. */

#include <stdlib.h>
#include "rtr/rsslTransport.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "edfExampleConfig.h"

typedef enum
{
	REF_DATA_STATE_START,				/* Starting state. */
	REF_DATA_STATE_LOGIN_REQUESTED,			/* We are logged in to the ref data server. */
	REF_DATA_STATE_READY				/* We requested and received all the information from the ref data
							   server and we are ready to setup all other connections */
} RefDataSessionState;

typedef struct
{
	RefDataSessionState	state;		/* Current state. */
	RsslChannel		*pRsslChannel;	/* RsslChannel associated with this session. */
	RsslBool		setWriteFd;	/* Indicates whether the channel should listen for write 
						 * notification.  May be set when initializing the channel,
						 * or when it needs to flush data. */
	char			name[16];	/* A name to use when printing messages related to this  */
	RsslUInt32		streamId;
	RsslRDMDirectoryMsg	directoryMsgCopy;		/* Copy of the directory message. */
	RsslBuffer		directoryMsgCopyMemory;		/* Memory buffer for directoryMsgCopy. */
	RsslBuffer		directoryMsgCopyMemoryOrig;	/* Copy of memory buffer (used to cleanup) */
	RsslRDMService*		pService;
} RefDataSession;


/* Clears the RefDataSession object. */
RTR_C_INLINE void refDataSessionClear(RefDataSession *pSession)
{
	memset(pSession, 0, sizeof(RefDataSession));
	pSession->state = REF_DATA_STATE_START;
	strncpy(pSession->name, "Ref Data Channel", sizeof(pSession->name));
	pSession->streamId = 2;

	pSession->directoryMsgCopyMemory.data = (char*)malloc(16384);
	pSession->directoryMsgCopyMemory.length = 16384;
	pSession->directoryMsgCopyMemoryOrig = pSession->directoryMsgCopyMemory;
}

/* Cleanup the RefDataSession object. */
RTR_C_INLINE void refDataSessionCleanup(RefDataSession *pSession)
{
	free(pSession->directoryMsgCopyMemoryOrig.data);
}

/* Connects the ref data server channel. */
RsslRet refDataSessionConnect(RefDataSession *pSession, RsslConnectOptions *pOpts);

/* Attempts to initialize the ref data server channel. */
RsslRet refDataSessionInitializeChannel(RefDataSession *pSession);

/* Called when the ref data server channel first completes initialization and can
 * send/receive messages.  Sends the login request. */
RsslRet refDataSessionProcessChannelActive(RefDataSession *pSession);

/* Handles read-ready notification from the channel. Reads messages, or tries to intialize
 * the channel if it's not active yet. Returns a postive value if rsslReadEx()
 * indicated more data is available to read. */
RsslRet refDataSessionProcessReadReady(RefDataSession *pSession);

/* Handles write-ready notification from the channel. Flushes outbound data to the network,
 * or tries to initialize the channel if it's not active yet. */
RsslRet refDataSessionProcessWriteReady(RefDataSession *pSession);

/* Sends messages to request source directory from the ref data server. */
RsslRet refDataSessionRequestSourceDirectory(RefDataSession *pSession);

/* Sends messages to request global set defintions from the ref data server. */
RsslRet refDataSessionRequestGlobalSetDef(RefDataSession *pSession, const char *dictionaryName, RsslInt32 streamId);

/* Returns whether this channel needs to trigger on write notification.  This will be set
 * when outbound data needs to be flushed to the network, or when initializing the channel. */
RTR_C_INLINE RsslBool refDataSessionNeedsWriteNotification(RefDataSession *pSession)
{ return pSession->setWriteFd; }

#endif
