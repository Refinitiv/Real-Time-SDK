/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_RSSL_QUEUE_MESSAGING_HANDLER_H
#define _RTR_RSSL_QUEUE_MESSAGING_HANDLER_H

#include "rtr/rsslReactor.h"
#include "tunnelStreamHandler.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEST_NAMES 10

/* A handler for demonstration of exchanging queue messages between
 * two consumers through a tunnel stream. */
typedef struct
{
	/* Handler that maintains the tunnel stream. */
	TunnelStreamHandler tunnelStreamHandler;			

	/* Name used when opening the queue stream. */
	RsslBuffer			sourceName; 					

	/* List of destination names for sending queue messages. */
	RsslBuffer			destNames[MAX_DEST_NAMES];		

	/* Number of names in destNames. */
	RsslInt32 			destNameCount;					

	/* Whether the application has opened the queue stream. */
	RsslBool			isQueueStreamOpen;				

	/* Incrementing identifier, which is associated with each queue message sent. */
	RsslInt64 			identifier;						

	/* Next time at which a queue message will be sent, if the tunnel stream is open. */
	time_t				nextMsgTime;				

} QueueMsgHandler;

/* Load FIX field dictionary used with queue messages. */
RsslBool loadFdmDictionary();

/* Cleanup FIX field dictionary. */
void cleanupFdmDictionary();

/* Callback for messages received on the tunnel stream. 
 * This will generally not be called, as all messages for this handler will be
 * queue messages, but can be invoked by returning RSSL_RC_CRET_RAISE from queueMsgHandlerQueueMsgCallback. */
RsslReactorCallbackRet queueMsgHandlerDefaultMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent);

/* Callback for processing incoming queue messages. */
RsslReactorCallbackRet queueMsgHandlerQueueMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamQueueMsgEvent *pEvent);

/* Called when the tunnel stream is first successfully opened. */
void queueMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream);

/* Called when the tunnel stream is closed. */
void queueMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream);

/* Initializes the SimpleTunnelMsgHandler. */
void queueMsgHandlerInit(QueueMsgHandler *pQueueMsgHandler, char *consumerName, RsslUInt8 domainType,
		RsslBool useAuthentication);

/* Sends QueueMsgs at periodic intervals. */
void handleQueueMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, QueueMsgHandler *pQueueMsgHandler);

/* Close the tunnel stream, if it is open. */
void queueMsgHandlerCloseStreams(QueueMsgHandler *pQueueMsgHandler);

/* Send QueueMsgs. */
static void sendQueueMsg(QueueMsgHandler *pQueueMsgHandler);

/* Sends a reply to a received QueueMsg. */
static void ackQueueMsg(QueueMsgHandler *pQueueMsgHandler, RsslBuffer *pDestName, RsslInt32 streamId);

/* Decodes the payload of a QueueData or QueueDataExpired message. */
static RsslBool queueMsgHandlerDecodeQueueData(RsslDecodeIterator *pIter);

#ifdef __cplusplus
};
#endif

#endif

