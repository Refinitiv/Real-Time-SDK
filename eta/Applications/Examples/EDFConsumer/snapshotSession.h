/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef SNAPSHOT_SESSION_H
#define SNAPSHOT_SESSION_H

/* Handles interactions with the snapshot server. */

#include "rtr/rsslTransport.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "edfExampleConfig.h"

typedef enum
{
	SNAPSHOT_STATE_START,			/* Starting state. */
	SNAPSHOT_STATE_LOGIN_REQUESTED,	/* We sent a login request to the snapshot server. */
	SNAPSHOT_STATE_SYMBOL_LIST_RECEIVED, /* We have received the full symbol list from the snapshot server, and will be connecting to the requested realtime and gap fill multicast channel(s) */ 
	SNAPSHOT_STATE_ITEMS_REQUESTED	/* We requested our items from the snapshot server. */
} SnapshotSessionState;

typedef struct
{
	RsslUInt	channelId;
	RsslUInt	domain;
} ChannelInfo;

typedef struct
{
	RsslUInt32	id;
	char		name[128];
	ChannelInfo	streamingChannels[3];
	ChannelInfo	gapChannels[3];
} SymbolListEntry;

typedef struct
{
	SnapshotSessionState	state;			/* Current state. */
	RsslChannel				*pRsslChannel;	/* RsslChannel associated with this session. */
	RsslBool				setWriteFd;		/* Indicates whether the channel should listen for write 
											 * notification.  May be set when initializing the channel,
											 * or when it needs to flush data. */
	char					name[16];		/* A name to use when printing messages related to this 
											 * channel. */
	RsslUInt32				serviceId;
	RsslRDMService*			pService;

	SymbolListEntry* 	symbolListEntry[9024];
} SnapshotSession;

/* Clears the SnapshotSession object. */
RTR_C_INLINE void snapshotSessionClear(SnapshotSession *pSession)
{
	pSession->state = SNAPSHOT_STATE_START;
	pSession->pRsslChannel = NULL;
	strncpy(pSession->name, "Snapshot Channel", sizeof(pSession->name));
	pSession->serviceId = 0;
	pSession->pService = NULL;
}

/* Connects the snapshot server channel. */
RsslRet snapshotSessionConnect(SnapshotSession *pSession, RsslConnectOptions *pOpts, RsslRDMService* service);

/* Attempts to initialize the snapshot server channel. */
RsslRet snapshotSessionInitializeChannel(SnapshotSession *pSession);

/* Sends messages to request refreshes from the snapshot server. */
RsslRet snapshotSessionRequestItems(SnapshotSession *pSession);

/* Called when the snapshot server channel first completes initialization and can
 * send/receive messages.  Sends the login request. */
RsslRet snapshotSessionProcessChannelActive(SnapshotSession *pSession);

/* Handles read-ready notification from the channel. Reads messages, or tries to intialize
 * the channel if it's not active yet. Returns a postive value if rsslReadEx()
 * indicated more data is available to read. */
RsslRet snapshotSessionProcessReadReady(SnapshotSession *pSession);

/* Handles write-ready notification from the channel. Flushes outbound data to the network,
 * or tries to initialize the channel if it's not active yet. */
RsslRet snapshotSessionProcessWriteReady(SnapshotSession *pSession);

/* Returns whether this channel needs to trigger on write notification.  This will be set
 * when outbound data needs to be flushed to the network, or when initializing the channel. */
RTR_C_INLINE RsslBool snapshotSessionNeedsWriteNotification(SnapshotSession *pSession)
{ return pSession->setWriteFd; }

#endif
