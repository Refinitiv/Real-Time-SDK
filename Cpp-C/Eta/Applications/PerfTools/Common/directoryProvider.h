/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/* directoryProvider.h
 * The directory handler for the ProvPerf and NIProvPerf.  Configures a single
 * service and provides encoding of a directory message to advertise that service. */

#ifndef _DIRECTORY_PROVIDER_H
#define _DIRECTORY_PROVIDER_H

#include "rtr/rsslRDMDirectoryMsg.h"
#include "channelHandler.h"
#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	RsslUInt64	serviceId;			/* ID of the provided service */
	char		serviceName[128];	/* Name of the provided service */
	RsslUInt64	openLimit;			/* Advertised OpenLimit (set to 0 to not provide this) */
	RsslQos		qos;				/* Advertised Quality of Service */
} DirectoryConfig;

RTR_C_INLINE void clearDirectoryConfig(DirectoryConfig *pConfig)
{
	pConfig->serviceId = 1;
	snprintf(pConfig->serviceName, sizeof(pConfig->serviceName), "%s", "DIRECT_FEED");
	pConfig->openLimit = 1000000;
	rsslClearQos(&pConfig->qos);
}

/* Contains the global DirectoryHandler configuration. */
extern DirectoryConfig directoryConfig;

/* The preconfigured directory service. */
extern RsslRDMService service;

/* Initialize the Global Directory Handler Configuration, based on the configuration options set by the application. */
void initDirectoryConfig();

/* Initialize the service to provide. */
void directoryServiceInit();

/* Decodes a directory request and sends an appropriate response. */
RsslRet processDirectoryRequest(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, RsslMsg* msg, RsslDecodeIterator* dIter);

/* Decodes a directory request and sends an appropriate response using the ETA VA Reactor. */
RsslRet processDirectoryRequestReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsg *pDirectoryMsg);

/* Used by Non-Interactive Provider to send out a directory refresh. */
RsslRet publishDirectoryRefresh(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, RsslInt32 streamId);

#ifdef __cplusplus
};
#endif

#endif
