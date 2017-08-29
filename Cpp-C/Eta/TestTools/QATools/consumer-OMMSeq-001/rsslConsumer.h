
#ifndef _RTR_RSSL_CONSUMER_H
#define _RTR_RSSL_CONSUMER_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONSUMER_CONNECTION_RETRY_TIME 15 /* rsslConsumer connection retry time in seconds */


/* defines maximum allowed name length for this application */
#define MAX_ITEM_NAME_STRLEN 128
/* defines the maximum range of streamIds within a domain */
#define MAX_STREAM_ID_RANGE_PER_DOMAIN 100

static RsslRet readFromChannel(RsslChannel* chnl);
static RsslChannel* connectToRsslServer(RsslConnectionTypes connType, RsslError* error);
static void initPingHandler(RsslChannel* chnl);
static void initRuntime();
static void handlePings(RsslChannel* chnl);
static void handleRuntime();
static RsslRet processResponse(RsslChannel* chnl, RsslBuffer* msgBuf);
void loginSuccessCallBack(RsslChannel* chnl);
void cleanUpAndExit();
void recoverConnection();

void dumpHexBuffer(const RsslBuffer * buffer);

#ifdef __cplusplus
};
#endif

#endif


