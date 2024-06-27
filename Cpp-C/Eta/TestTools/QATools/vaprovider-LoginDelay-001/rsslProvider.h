/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_PROVIDER_H
#define _RTR_RSSL_PROVIDER_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslReactor.h"

#include "rsslVACacheHandler.h"
#include "simpleTunnelMsgHandler.h"

#include <time.h>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CLIENT_SESSIONS_LIMIT 7
#define MAX_CLIENT_SESSIONS 10
#define UPDATE_INTERVAL 1
#define MAX_TUNNEL_STREAMS 5

/* client session information */
typedef struct {
	RsslBool isInUse;
	RsslReactorChannel *clientChannel;
	time_t nextReceivePingTime;
	time_t nextSendPingTime;
	RsslBool receivedClientMsg;
	RsslBool pingsInitialized;
	SimpleTunnelMsgHandler simpleTunnelMsgHandler[MAX_TUNNEL_STREAMS];
	// API QA
	RsslUInt nSendingLoginRefreshDelayedCycles; /* When > 0 then Provider delays the sending Login Refresh on the number of main-loop cycles (see main method) */
									/* otherwise Provider sends Login Refresh normal way, i.e. answer to Consumer Login request (see loginMsgCallback method). */
	// END API QA
} RsslClientSessionInfo;

static void initRuntime();
static void handleRuntime();
RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent *pMsgEvent);
RsslReactorCallbackRet tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent *pEvent, RsslErrorInfo *pErrorInfo);
RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamStatusEvent *pEvent);
RsslReactorCallbackRet tunnelStreamMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pChannelEvent);
static void removeClientSession(RsslClientSessionInfo* clientSessionInfo);
void removeClientSessionForChannel(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel);
void cleanUpAndExit();
RsslVACacheInfo* getCacheInfo();
// API QA
RsslBool delaySendLoginRefresh();
// END API QA

/*
 * Clears the client session information.
 * clientSessionInfo - The client session information to be cleared
 */
RTR_C_INLINE void clearClientSessionInfo(RsslClientSessionInfo* clientSessionInfo)
{
	int i;
	clientSessionInfo->isInUse = RSSL_FALSE;
	clientSessionInfo->clientChannel = 0;
	clientSessionInfo->nextReceivePingTime = 0;
	clientSessionInfo->nextSendPingTime = 0;
	clientSessionInfo->receivedClientMsg = RSSL_FALSE;
	clientSessionInfo->pingsInitialized = RSSL_FALSE;
	for (i = 0; i < MAX_TUNNEL_STREAMS; i++)
	{
		simpleTunnelMsgHandlerInit(&clientSessionInfo->simpleTunnelMsgHandler[i], NULL, 0, RSSL_FALSE, RSSL_TRUE);
	}
	// API QA
	clientSessionInfo->nSendingLoginRefreshDelayedCycles = 0U;
	// END API QA
}

#ifdef __cplusplus
};
#endif

#endif
