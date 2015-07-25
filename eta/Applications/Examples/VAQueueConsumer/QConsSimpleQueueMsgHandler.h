/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_QCONS_SIMPLE_QUEUE_MESSAGING_HANDLER_H
#define _RTR_QCONS_SIMPLE_QUEUE_MESSAGING_HANDLER_H

#include "rtr/rsslRDMDirectoryMsg.h"
#include "QConsSimpleDirectoryHandler.h"
#include "rtr/rsslTunnelStream.h"
#include "QConsChannelStorage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Used by the Consumer to open a tunnel stream once the Consumer's channel
 * is opened and the desired service identified. */
int openTunnelStream(RsslReactor *pReactor, RsslReactorChannel *pChnl, ChannelStorage *pCommand, RsslErrorInfo *errorInfo);

/*
 * Used by the Consumer to close any tunnel streams it opened
 * for the reactor channel.
 */
RsslRet closeTunnelStreams(RsslReactorChannel *pChnl, RsslErrorInfo *pErrorInfo);

/* Sends QueueMsgs at periodic intervals. */
RsslRet handleQueueMessaging(RsslReactor *pReactor, ChannelStorage *pCommand);

/* Open a tunnel stream for queue messaging. */
static void sendOpenTunnelStreamRequest(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel);

/* Send QueueMsgs. */
static void sendQueueMsg(RsslReactorChannel *pChnl);

#ifdef __cplusplus
};
#endif

#endif
