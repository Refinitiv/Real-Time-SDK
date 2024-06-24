/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef WL_SERVICE_H
#define WL_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslHashTable.h"
#include "rtr/wlServiceCache.h"
#include "rtr/wlBase.h"

typedef struct WlService WlService;

typedef struct WlRequestedService WlRequestedService;

/* Lists of streams associated with a service. */
struct WlService
{
	RsslQueueLink		qlServices;
	RDMCachedService	*pService;			/* Associated cached service information. */
	RsslQueue			openStreamList;		/* Streams opened on this service. */
	RsslQueue			itemGroups;			/* List of item groups associated with this service. */
	RsslHashTable		itemGroupTable;		/* Table of item groups, by ID. */
	RsslQueue			requestedServices;	/* Associated directory service requests 
											 * (WlRequestedService) */
	RsslQueue			streamsPendingRefresh;	/* Streams waiting for a refresh. */
	RsslQueue			streamsPendingWindow;	/* Streams that are waiting for room in the
												 * service's OpenWindow. */
};

/* Creates a WlService. */
WlService *wlServiceCreate(RDMCachedService *pService, RsslErrorInfo *pErrorInfo);

/* Destroys a WlService. */
void wlServiceDestroy(WlService *pWlService);

typedef enum
{
	WL_RSVC_HAS_NAME	= 0x1,	/* Requested service is by name. */
	WL_RSVC_HAS_ID		= 0x2	/* Requested service is by ID. */
} WlRequestedServiceFlags;

/* Identies a service that has been requested, by name or ID.
 * Keeps track of items requested to the service, and matches them to the service when it
 * appears in the directory. */
struct WlRequestedService
{
	RsslHashLink		hlServiceRequests;
	RsslQueueLink		qlServiceRequests;
	RsslQueueLink		qlDirectoryRequests;
	RsslUInt8			flags;				/* WlRequestedServiceFlags. */
	RsslBuffer			serviceName;		/* Name of the requested service. */
	RsslUInt			serviceId;			/* ID of the requested service. */
	WlService			*pMatchingService;	/* Service matching this request, if available. */
	RsslQueue			recoveringList;		/* Item requests recovering to this service. */
	RsslQueue			itemRequests;		/* Item requests associated with this service. */
	RsslQueue			directoryRequests;	/* Directory requests for this service. */
	RsslQueue			openDirectoryRequests;	/* Directory requests that have been served
												 * by the cache and can receive updates from
												 * the directory stream. */

};

/* Creates a WlRequestedService. */
WlRequestedService* wlRequestedServiceOpen(WlBase *pBase, RsslBuffer *pServiceName, 
		RsslUInt *pServiceId, RsslErrorInfo *pErrorInfo);

/* Checks if the service can be cleaned up. */
void wlRequestedServiceCheckRefCount(WlBase *pBase, WlRequestedService *pRequestedService);

/* Closes a requested service and destroys it. */
void wlRequestedServiceClose(WlBase *pBase, WlRequestedService *pRequestedService);

/* Destroys a WlRequestedService. */
void wlRequestedServiceDestroy(WlRequestedService *pRequestedService);


#ifdef __cplusplus
}
#endif


#endif

