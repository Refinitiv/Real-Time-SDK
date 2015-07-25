/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RSSL_VA_NI_LOGIN_PROVIDER_H
#define _RSSL_VA_NI_LOGIN_PROVIDER_H

#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"
#include "rsslVASendMessage.h"
#include "rsslNIChannelCommand.h"

#ifdef __cplusplus
extern "C" {
#endif

RsslRet setupLoginRequest(NIChannelCommand* chnlCommand, RsslInt32 streamId);

RsslReactorCallbackRet processLoginResponse(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pEvent);

#ifdef __cplusplus
}
#endif

#endif
