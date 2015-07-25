

#ifndef _RTR_RSSL_LOGIN_HANDLER_H
#define _RTR_RSSL_LOGIN_HANDLER_H

#include "rsslProvider.h"
#include "rsslLoginEncodeDecode.h"

#ifdef __cplusplus
extern "C" {
#endif

void initLoginHandler();
static RsslLoginRequestInfo* getLoginReqInfo(RsslChannel* chnl, RsslMsg* msg);
RsslRet processLoginRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet sendLoginResponse(RsslChannel* chnl, RsslLoginRequestInfo* loginReqInfo);
RsslRet sendLoginCloseStatusMsg(RsslChannel* chnl);
static RsslRet sendLoginRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslLoginRejectReason reason);
void closeLoginChnlStream(RsslChannel* chnl);
static void closeLoginStream(RsslInt32 streamId);
RsslLoginRequestInfo* findLoginReqInfo(RsslChannel* chnl);

#ifdef __cplusplus
};
#endif

#endif
