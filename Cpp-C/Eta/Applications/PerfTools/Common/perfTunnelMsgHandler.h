/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#pragma once
#ifndef PERF_TUNNEL_MSG_HANDLER_H
#define PERF_TUNNEL_MSG_HANDLER_H

#include "tunnelStreamHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Signature of an Item Message Process Callback function.
* @see RsslTunnelStream, RsslTunnelStreamMsgEvent
*/
typedef RsslRet RsslProcessItemMsg(RsslReactorChannel *pReactorChannel, RsslMsg *pMsg);

/** A handler for simple demonstration of exchanging messages between
 * a consumer and provider through a tunnel stream.
 * @see SimpleTunnelMsgHandler
 */
typedef struct
{
	/* Handler that maintains the tunnel stream. */
	TunnelStreamHandler tunnelStreamHandler;	

	/* Providers should be not to send messages to consumers that we have not authenticated. */
	RsslBool			waitingForAuthenticationRequest;

	/* Indicates if this is a provider handling the stream. */
	RsslBool			isProvider;

	/* Callback for processing an item message */
	RsslProcessItemMsg* processItemMsg;

	/* Guaranteed Output Tunnel Buffers. See RsslReactorAcceptTunnelStreamOptions in call rsslReactorAcceptTunnelStream. */
	RsslUInt32			guaranteedOutputTunnelBuffers;
} PerfTunnelMsgHandler;

/* Initializes the PerfTunnelMsgHandler. */
void perfTunnelMsgHandlerInit(PerfTunnelMsgHandler *pMsgHandler,
		char *consumerName, RsslUInt8 domainType, RsslBool useAuthentication, RsslBool isProvider, RsslUInt32 guaranteedOutputTunnelBuffers);

/* Start the tunnel stream, opening it if the desired service is online. */
void startPerfTunnelMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, PerfTunnelMsgHandler *pPerfTunnelMsgHandler);

/* Used by providers. Accepts or rejects a new tunnel stream. */
void perfTunnelMsgHandlerProcessNewStream(PerfTunnelMsgHandler *pPerfTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent);

/* Close the tunnel stream, if it is open. */
void perfTunnelMsgHandlerCloseStreams(PerfTunnelMsgHandler *pPerfTunnelMsgHandler);

/* Add callback for processing of an item message */
void perfTunnelMsgHandlerAddCallbackProcessItemMsg(PerfTunnelMsgHandler *pMsgHandler, RsslProcessItemMsg* callbackProcessItemMsg);

/* Callback for messages received by consumers on this tunnel stream. */
static RsslReactorCallbackRet perfTunnelMsgHandlerConsumerMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);

/* Callback for messages received by providers on this tunnel stream. */
static RsslReactorCallbackRet perfTunnelMsgHandlerProviderMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);

/* Called when the tunnel stream is opened. */
static void perfTunnelMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream);

/* Called when the tunnel stream is closed. */
static void perfTunnelMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream);

/* Used by a provider to decode & check the requested class of service of a requested tunnel stream. */
char* perfTunnelMsgHandlerCheckRequestedClassOfService(PerfTunnelMsgHandler *pMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent, RsslClassOfService *pCos);

/* Get the tunnel stream Id */
RsslInt perfTunnelMsgHandlerGetStreamId(void);

#ifdef __cplusplus
};
#endif

#endif  // PERF_TUNNEL_MSG_HANDLER_H

