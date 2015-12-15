/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* Handles processing of messages from the realtime feed. */

#ifndef GAP_FILL_SESSION
#define GAP_FILL_SESSION

#include "rtr/rsslTransport.h"
#include "itemList.h"
#include "edfExampleConfig.h"

static short gap_channel_count = 0;

typedef struct
{
	RsslChannel		*pRsslChannel;	/* RsslChannel associated with this session. */
	char			name[20];	/* A name to use when printing messages related to this 
						 * channel. */
        RsslBool                connected;
} GapFillSession;


/* Clears the RealTimeSession object. */
RTR_C_INLINE void gapFillSessionClear(GapFillSession *pSession)
{
	pSession->pRsslChannel = NULL;
	snprintf(pSession->name, 20, "Gap Fill Channel %d", gap_channel_count);
	gap_channel_count++;
	pSession->connected = RSSL_FALSE;
}

/* Connects to the realtime feed. */
RsslRet gapFillSessionConnect(GapFillSession *pSession, RsslConnectOptions *pOpts);

/* Handles read-ready notification from the channel. Reads messages. Returns a postive 
 * value if rsslReadEx() indicated more data is available to read. */
RsslRet gapFillSessionProcessReadReady(GapFillSession *pSession);

#endif
