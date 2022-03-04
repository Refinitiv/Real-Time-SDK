/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



#ifndef _RTR_RSSL_PROVIDER_H
#define _RTR_RSSL_PROVIDER_H

#include "rtr/rsslTransport.h"
#include "rsslJsonSession.h"
#include "rsslDirectoryHandler.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
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

/* client session information */
typedef struct {
	RsslChannel *clientChannel;
	time_t nextReceivePingTime;
	time_t nextSendPingTime;
	RsslBool receivedClientMsg;
	RsslBool pingsInitialized;
	RsslJsonSession jsonSession;
} RsslClientSessionInfo;

static void readFromChannel(RsslChannel* chnl);
static RsslServer* bindRsslServer(char* portno, RsslError* error);
static void createNewClientSession(RsslServer *srvr);
static void initPingTimes(RsslClientSessionInfo* clientSessionInfo);
static void initRuntime();
static void handlePings();
static void handleRuntime();
static RsslRet processRequest(RsslChannel* chnl, RsslBuffer* msgBuf);
static void removeClientSession(RsslClientSessionInfo* clientSessionInfo);
static void removeChannel(RsslChannel* chnl);
static void setReceivedClientMsgForChannel(RsslChannel *chnl);
static void removeClientSessionForChannel(RsslChannel *chnl);
void cleanUpAndExit();

#ifdef __cplusplus
};
#endif

#endif

