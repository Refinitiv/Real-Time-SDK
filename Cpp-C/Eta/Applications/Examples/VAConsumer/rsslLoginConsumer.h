/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_LOGIN_CONSUMER_H
#define _RTR_RSSL_LOGIN_CONSUMER_H

#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"
#include "rsslChannelCommand.h"

#ifdef __cplusplus
extern "C" {
#endif

/* processes login response messages */
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent);
/* returns attributes from the login response - useful to determine features supported by provider */
RsslRDMLoginRefresh* getLoginRefreshInfo(ChannelCommand *pCommand);

#ifdef __cplusplus
};
#endif

#endif
