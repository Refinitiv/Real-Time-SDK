

#ifndef _RTR_RSSL_PROVIDER_H
#define _RTR_RSSL_PROVIDER_H

#include "rtr/rsslTransport.h"
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

/*
 * Clears the client session information.
 * clientSessionInfo - The client session information to be cleared
 */
RTR_C_INLINE void clearClientSessionInfo(RsslClientSessionInfo* clientSessionInfo)
{
	clientSessionInfo->clientChannel = 0;
	clientSessionInfo->nextReceivePingTime = 0;
	clientSessionInfo->nextSendPingTime = 0;
	clientSessionInfo->receivedClientMsg = RSSL_FALSE;
	clientSessionInfo->pingsInitialized = RSSL_FALSE;
}

#ifdef __cplusplus
};
#endif

#endif

