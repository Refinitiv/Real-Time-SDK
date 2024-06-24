/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef SIMPLE_TUNNEL_MSG_HANDLER_H
#define SIMPLE_TUNNEL_MSG_HANDLER_H

#include "tunnelStreamHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A handler for simple demonstration of exchanging messages between
 * a consumer and provider through a tunnel stream. */
typedef struct
{
	/* Handler that maintains the tunnel stream. */
	TunnelStreamHandler tunnelStreamHandler;	

	/* Used as a timer for sending messsages. */
	time_t				nextMsgTime;

	/* Counts the number of messages sent. This value is added as part of each message. */
	RsslUInt32			msgCount;

	/* Providers should be not to send messages to consumers that we have not authenticated. */
	RsslBool			waitingForAuthenticationRequest;

	/* Indicates if this is a provider handling the stream. */
	RsslBool			isProvider;

} SimpleTunnelMsgHandler;

/* Initializes the SimpleTunnelMsgHandler. */
void simpleTunnelMsgHandlerInit(SimpleTunnelMsgHandler *pMsgHandler,
		char *consumerName, RsslUInt8 domainType, RsslBool useAuthentication, RsslBool isProvider);

/* Handles time-based events, such as sending messages. */
void handleSimpleTunnelMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler);

/* Used by providers. Accepts or rejects a new tunnel stream. */
void simpleTunnelMsgHandlerProcessNewStream(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent);

/* Close the tunnel stream, if it is open. */
void simpleTunnelMsgHandlerCloseStreams(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler);

/* Callback for messages received by consumers on this tunnel stream. */
static RsslReactorCallbackRet simpleTunnelMsgHandlerConsumerMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);

/* Callback for messages received by providers on this tunnel stream. */
static RsslReactorCallbackRet simpleTunnelMsgHandlerProviderMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);

/* Send a simple generic message on this tunnel stream. */
static void simpleTunnelMsgHandlerSendMessage(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler);

/* Called when the tunnel stream is opened. */
static void simpleTunnelMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream);

/* Called when the tunnel stream is closed. */
static void simpleTunnelMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream);

/* Used by a provider to decode & check the requested class of service of a requested tunnel stream. */
char* simpleTunnelMsgHandlerCheckRequestedClassOfService(SimpleTunnelMsgHandler *pMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent, RsslClassOfService *pCos);

#ifdef __cplusplus
};
#endif

#endif

