/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* Handles processing of messages from the realtime feed. */

#ifndef REAL_TIME_SESSION
#define REAL_TIME_SESSION

#include "rtr/rsslTransport.h"
#include "gapRequestSession.h"
#include "edfExampleConfig.h"

static short real_time_channel_count = 0;

typedef struct
{
	RsslChannel		*pRsslChannel;	/* RsslChannel associated with this session. */
	char			name[25];		/* A name to use when printing messages related to this 
									 * channel. */
	gap_info_t		gapInfo;
	RsslBool		gapDetected;
	RsslBool		connected;
} RealTimeSession;


/* Clears the RealTimeSession object. */
RTR_C_INLINE void realTimeSessionClear(RealTimeSession *pSession)
{
	pSession->pRsslChannel = NULL;
	snprintf(pSession->name, 25, "Realtime Channel %d", real_time_channel_count);
	real_time_channel_count++;
	memset(&pSession->gapInfo, 0, sizeof(gap_info_t));
	pSession->gapDetected = RSSL_FALSE;
	pSession->connected = RSSL_FALSE;
}

/* Connects to the realtime feed. */
RsslRet realTimeSessionConnect(RealTimeSession *pSession, RsslConnectOptions *pOpts);

/* Handles read-ready notification from the channel. Reads messages. Returns a postive 
 * value if rsslReadEx() indicated more data is available to read. */
RsslRet realTimeSessionProcessReadReady(RealTimeSession *pSession);

#endif
