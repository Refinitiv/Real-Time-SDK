

#ifndef _RTR_RSSL_NIPROVIDER_H
#define _RTR_RSSL_NIPROVIDER_H

#include "rtr/rsslTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPDATE_INTERVAL 1
#define NI_PROVIDER_CONNECTION_RETRY_TIME 15 /* rsslNIProvider connection retry time in seconds */
#define MAX_ITEM_LIST_SIZE 100 /* maximum length list of items to publish */

static RsslRet readFromChannel(RsslChannel* chnl);
static RsslChannel* connectToInfrastructure(RsslConnectionTypes connType, RsslError* error);
static void initPingHandler(RsslChannel* chnl);
static void initRuntime();
static void handlePings();
static void handleRuntime();
static RsslRet processResponse(RsslChannel* chnl, RsslBuffer* msgBuf);
static void removeChannel(RsslChannel* chnl);
void loginSuccessCallBack(RsslChannel* chnl);
void cleanUpAndExit();
void recoverConnection();

#ifdef __cplusplus
};
#endif

#endif


