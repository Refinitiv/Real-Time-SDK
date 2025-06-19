/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_NIPROVIDER_H
#define _RTR_RSSL_NIPROVIDER_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslReactor.h"
#include "rsslNIItemHandler.h"
#include "rsslNIChannelCommand.h"

#include "rsslVACacheHandler.h"


#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#include "rtr/rsslPayloadCache.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPDATE_INTERVAL 1

/* client session information */
typedef struct {
	RsslReactorChannel *clientChannel;
	time_t nextReceivePingTime;
	time_t nextSendPingTime;
	RsslBool receivedClientMsg;
	RsslBool pingsInitialized;
} RsslClientSessionInfo;

RsslVACacheInfo* getCacheInfo();
static void readFromChannel(RsslReactorChannel* chnl);
static void initRuntime();
static void handleRuntime();
RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pEvent);
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent);
static RsslReactorCallbackRet processResponse(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent *pEvent);
void cleanUpAndExit();

void recoverConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, NIChannelCommand *pCommand);

#ifdef __cplusplus
};
#endif

#endif
