/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef TUNNEL_STREAM_HANDLER_H
#define TUNNEL_STREAM_HANDLER_H

#include "rtr/rsslReactor.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maintains information about a tunnel stream.
 * The SimpleTunnelMsgHandler has this structure
 * as a member, and use it to maintain the tunnel stream 
 * on which they operate. Code associated with opening a tunnel
 * stream and maintaining our knowledge of its state is provided
 * here and in tunnelStreamHandler.c. */
typedef struct
{
	/* Name used by consumers when opening the tunnel stream. */
	char								*consumerName;					

	/* Whether to use authentication when opening the tunnel stream. */
	RsslBool							useAuthentication;				

	/* Open tunnel stream associated with this handler. */
	RsslTunnelStream					*pTunnelStream;					

	/* Whether the consumer or provider has opened a tunnel stream. */
	RsslBool							tunnelStreamOpenRequested;		

	/* Service ID to use when opening the tunnel stream. */
	RsslUInt16							serviceId;						

	/* DomainType to use when opening the tunnel stream. */
	RsslUInt8							domainType;

	/* Whether we have identified the service we will use for our tunnel stream. */
	RsslBool							isTunnelServiceFound;			

	/* Whether the service we will use for the tunnel stream is up. */
	RsslBool							isTunnelServiceUp;

	/* Whether the service we will use for our tunnel stream supports it. */
	RsslBool							tunnelServiceSupported;

	/* Next time at which the handler will attempt to open the tunnel stream. */
	time_t								nextRecoveryTime;

	/* Function to call when the tunnel stream is first successfully opened. */
	void								(*processTunnelStreamOpened)(RsslTunnelStream*);

	/* Function to call when the tunnel stream is closed. */
	void								(*processTunnelStreamClosed)(RsslTunnelStream*);

	/* A default message callback to provide to rsslReactorOpenTunnelStream. */
	RsslTunnelStreamDefaultMsgCallback	*defaultMsgCallback;

	/* Remember whether we are waiting on the final status event. */
	RsslBool							waitFinalStatusEvent;

} TunnelStreamHandler;

/* Resets information about the state of the tunnel stream. */
void tunnelStreamHandlerClear(TunnelStreamHandler *pTunnelHandler);

/* Clears any information about the service associated with the tunnel stream. */
void tunnelStreamHandlerClearServiceInfo(TunnelStreamHandler *pTunnelHandler);

/* Closes the tunnel stream, if it is open. */
void tunnelStreamHandlerCloseStreams(TunnelStreamHandler *pTunnelHandler);

/* Handles the tunnel stream, opening it if the desired service is online. */
void handleTunnelStreamHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		TunnelStreamHandler *pTunnelHandler);

/* Checks a service provided by a directory message, to see if it can be
 * used to open a tunnel stream. */
void tunnelStreamHandlerProcessServiceUpdate(TunnelStreamHandler *pTunnelHandler, 
											 RsslBuffer *pMatchServiceName, RsslRDMService* pService);

/* Callback for tunnel stream status events. */
RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamStatusEvent *pEvent);


/* Initializes the tunnel stream handler, clearing all members. */
RTR_C_ALWAYS_INLINE void tunnelStreamHandlerInit(TunnelStreamHandler *pTunnelHandler,
       						 char *consumerName, RsslUInt8 domainType, 
							 RsslBool useAuthentication,
		        		 	 void processTunnelStreamOpened(RsslTunnelStream*),
				        	 void processTunnelStreamClosed(RsslTunnelStream*),
						     RsslTunnelStreamDefaultMsgCallback *defaultMsgCallback)
{
	pTunnelHandler->consumerName = consumerName;
    pTunnelHandler->useAuthentication = useAuthentication;
    pTunnelHandler->processTunnelStreamOpened = processTunnelStreamOpened;
    pTunnelHandler->processTunnelStreamClosed = processTunnelStreamClosed;
    pTunnelHandler->defaultMsgCallback = defaultMsgCallback;

    pTunnelHandler->pTunnelStream = NULL;
    pTunnelHandler->serviceId = 0;
    pTunnelHandler->domainType = domainType;
    pTunnelHandler->tunnelStreamOpenRequested = RSSL_FALSE;
    pTunnelHandler->isTunnelServiceFound = RSSL_FALSE;
    pTunnelHandler->isTunnelServiceUp = RSSL_FALSE;
    pTunnelHandler->tunnelServiceSupported = RSSL_FALSE;
    pTunnelHandler->waitFinalStatusEvent = RSSL_FALSE;
}



#ifdef __cplusplus
};
#endif

#endif

