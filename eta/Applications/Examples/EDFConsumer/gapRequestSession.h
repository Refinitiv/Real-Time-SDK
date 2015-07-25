/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef GAP_REQUEST_SESSION_H
#define GAP_REQUEST_SESSION_H

/* Handles interactions with the gap request server. */

#include "rtr/rsslTransport.h"
#include "edfExampleConfig.h"

typedef enum
{
	GAP_REQUEST_STATE_START,		/* Starting state. */
	GAP_REQUEST_STATE_LOGGED_IN		/* We are logged in to the gap request server. We can request fill data */
} GapRequestSessionState;

typedef struct 
{
	RsslUInt64    port;
	char          address[128];
        RsslUInt64    start;
        RsslUInt64    end;
} gap_info_t;

typedef struct
{
	gap_info_t	gapInfo;
	RsslUInt32	streamId;
} GapRequestData;

typedef struct
{
	GapRequestSessionState	state;		/* Current state. */
	RsslChannel		*pRsslChannel;	/* RsslChannel associated with this session. */
	RsslBool		setWriteFd;	/* Indicates whether the channel should listen for write 
						 * notification.  May be set when initializing the channel,
						 * or when it needs to flush data. */
	char			name[20];	/* A name to use when printing messages related to this 
						 * channel. */
	RsslUInt32 		streamId;	/* keeps track of the last streamId used for the request message */

} GapRequestSession;


/* Clears the GapSession object. */
RTR_C_INLINE void gapRequestSessionClear(GapRequestSession *pSession)
{
	memset(pSession, 0, sizeof(GapRequestSession));
	pSession->state = GAP_REQUEST_STATE_START;
	strncpy(pSession->name, "Gap Request Channel", sizeof(pSession->name));
	pSession->streamId = 2;
}

/* Connects the gap request server channel. */
RsslRet gapRequestSessionConnect(GapRequestSession *pSession, RsslConnectOptions *pOpts);

/* Attempts to initialize the gap request server channel. */
RsslRet gapRequestSessionInitializeChannel(GapRequestSession *pSession);

/* Called when the gap request server channel first completes initialization and can
 * send/receive messages.  Sends the login request. */
RsslRet gapRequestSessionProcessChannelActive(GapRequestSession *pSession);

/* Handles read-ready notification from the channel. Reads messages, or tries to intialize
 * the channel if it's not active yet. Returns a postive value if rsslReadEx()
 * indicated more data is available to read. */
RsslRet gapRequestSessionProcessReadReady(GapRequestSession *pSession);

/* Handles write-ready notification from the channel. Flushes outbound data to the network,
 * or tries to initialize the channel if it's not active yet. */
RsslRet gapRequestSessionProcessWriteReady(GapRequestSession *pSession);

/* Sends messages to request missing data from the gap server. */
RsslRet gapRequestSessionRequestFill(GapRequestSession *pSession, gap_info_t *info);

/* Returns whether this channel needs to trigger on write notification.  This will be set
 * when outbound data needs to be flushed to the network, or when initializing the channel. */
RTR_C_INLINE RsslBool gapRequestSessionNeedsWriteNotification(GapRequestSession *pSession)
{ return pSession->setWriteFd; }

#endif
