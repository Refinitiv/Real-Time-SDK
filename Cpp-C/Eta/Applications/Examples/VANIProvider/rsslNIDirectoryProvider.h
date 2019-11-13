/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "rsslVASendMessage.h"
#include "rsslNIChannelCommand.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

RsslRet setupDirectoryResponseMsg(NIChannelCommand* chnlCommand, RsslInt32 streamId);

#ifdef __cplusplus
}
#endif
