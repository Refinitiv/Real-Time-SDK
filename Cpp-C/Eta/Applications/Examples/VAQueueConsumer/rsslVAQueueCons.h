/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_VA_QUEUE_CONS_H
#define _RTR_VA_QUEUE_CONS_H

#include "rtr/rsslTunnelStream.h"
#include "QConsChannelStorage.h"

#ifdef __cplusplus
extern "C" {
#endif

void closeConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, ChannelStorage *pCommand);
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent);
static RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent *pMsgEvent);

#ifdef __cplusplus
};
#endif

#endif
