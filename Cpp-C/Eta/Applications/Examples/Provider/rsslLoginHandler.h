/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



#ifndef _RTR_RSSL_LOGIN_HANDLER_H
#define _RTR_RSSL_LOGIN_HANDLER_H

#include "rsslProvider.h"
#include "rsslLoginEncodeDecode.h"

#ifdef __cplusplus
extern "C" {
#endif

void initLoginHandler(RsslBool rttSupport);
static RsslLoginRequestInfo* getLoginReqInfo(RsslChannel* chnl, RsslMsg* msg);
RsslRet processLoginRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet sendLoginResponse(RsslChannel* chnl, RsslLoginRequestInfo* loginReqInfo);
static RsslRet sendLoginRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslLoginRejectReason reason);
void closeLoginChnlStream(RsslChannel* chnl);
static void closeLoginStream(RsslInt32 streamId);
RsslLoginRequestInfo* findLoginReqInfo(RsslChannel* chnl);
RsslRet sendRTTLoginMsg(RsslChannel* chnl);
RsslBool checkLoginCockies(const char* coockies);
RsslRet sendCoockiesLoginResponse(RsslChannel* chnl);

#ifdef __cplusplus
};
#endif

#endif
