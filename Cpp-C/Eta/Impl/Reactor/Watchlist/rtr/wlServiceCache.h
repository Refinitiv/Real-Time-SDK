/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef WL_SERVICE_CACHE_H
#define WL_SERVICE_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslHashTable.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslHeapBuffer.h"

typedef struct WlServiceCache WlServiceCache;

typedef struct WlServiceCacheUpdateEvent WlServiceCacheUpdateEvent;

typedef struct RDMCachedService RDMCachedService;

typedef RsslRet WlServiceCacheUpdateCallback(WlServiceCache*, WlServiceCacheUpdateEvent*,
		RsslErrorInfo *pErrorInfo);

typedef RsslRet RDMCachedServiceStateChangeCallback(WlServiceCache*, RDMCachedService*, RsslErrorInfo *pErrorInfo);

typedef struct RDMCachedLink RDMCachedLink;

/* A service stored in the cache. */
struct RDMCachedService
{
	RsslUInt16			updateFlags;			
	RsslRDMService		rdm;
	RsslBool			hasServiceName;
	RsslBool			updatedServiceName;
	RsslUInt32			infoUpdateFlags;
	RsslUInt32			stateUpdateFlags;
	RsslUInt32			loadUpdateFlags;
	void				*pUserSpec;
	RsslQueueLink		_fullListLink;			/* Link for WlServiceCache full list. */
	RsslHashLink		_nameLink;				/* Link for WlServiceCache _servicesByName. */
	RsslHashLink		_idLink;				/* Link for WlServiceCache _servicesById. */
	RsslQueueLink		_updatedServiceLink;	/* Link for WlServiceCacheUpdateEvent 
												 * updatedServiceList. */
	RsslHashTable		_itemGroupsById;
	RsslBuffer			oldServiceName;

	RsslQueue			linkList;
	RsslHashTable		linkTable;				/* Table of sources for the Link filter. */
	RsslBuffer			tempLinkArrayBuffer;

};

struct RDMCachedLink
{
	RsslQueueLink		qlLinkList;
	RsslHashLink		hlLinkTable;
	RsslRDMServiceLink	link;
	RsslUInt32			linkUpdateFlags;
};


/* An item group that may be added to the cache. */
typedef struct
{
	RsslHashLink		_idLink;			/* Link by Group ID. */
	RsslBuffer			groupId;			/* Group ID. */
	RDMCachedService	*pParentService;	/* Service associated with the group. */
	void				*pUserSpec;			/* User-specified pointer. */
} WlServiceCacheItemGroup;

/* Event containing changes to service cache. */
struct WlServiceCacheUpdateEvent
{
	RsslQueue	updatedServiceList; /* Services whose information has changed. */
};

struct WlServiceCache
{
	void							*pUserSpec;					/* User-specified pointer. */
	RsslHashTable					_servicesByName;			/* Service table by Name. */
	RsslHashTable					_servicesById;				/* Service table by ID. */
	RsslQueue						_serviceList;				/* Full service list. */
	RsslInt32						_directoryStreamId;			/* Associated Stream ID. */
	RsslInt32						*_pRsslChannel;				/* Associated RsslChannel. */
	RsslBuffer						tempMemBuffer;
};

typedef struct
{
	void *pUserSpec;
	RDMCachedServiceStateChangeCallback *serviceStateChangeCallback;
} WlServiceCacheCreateOptions;

RTR_C_INLINE void wlServiceCacheClearCreateOptions(WlServiceCacheCreateOptions *pOptions)
{
	memset(pOptions, 0, sizeof(WlServiceCacheCreateOptions));
}

WlServiceCache* wlServiceCacheCreate(WlServiceCacheCreateOptions *pOptions, 
		RsslErrorInfo *pErrorInfo);

RsslRet wlServiceCacheClear(WlServiceCache *pServiceCache, RsslBool callback, 
		RsslErrorInfo *pErrorInfo);

void wlServiceCacheDestroy(WlServiceCache *pServiceCache);

RsslRet wlServiceCacheProcessDirectoryMsg(WlServiceCache *pServiceCache, 
		RsslChannel *pChannel, RsslRDMDirectoryMsg *pDirectoryMsg, RsslErrorInfo *pErrorInfo);

RsslRet wlServiceCacheProcessInactiveChannel(WlServiceCache *pServiceCache, 
		RsslChannel *pChannel, RsslErrorInfo *pErrorInfo);

RDMCachedService* wlServiceCacheFindService(WlServiceCache *pServiceCache, 
		const RsslBuffer *pServiceName, const RsslUInt *pServiceId);

RsslRet wlServiceCacheAddItemGroup(WlServiceCache *pServiceCache,
		WlServiceCacheItemGroup *pItemGroup);

RsslRet wlServiceCacheRemoveItemGroup(WlServiceCache *pServiceCache,
		WlServiceCacheItemGroup *pItemGroup);

typedef struct
{
	RsslRDMService *serviceList;
	RsslUInt32 serviceCount;
} WlServiceList;

/* Populates pServiceList with the list of services matching the given criteria.
 * Warning: service list is only valid until wlServiceCacheGetServiceList is called again. */
RsslRet wlServiceCacheGetServiceList(WlServiceCache *pServiceCache, WlServiceList *pServiceList,
		RsslUInt *pServiceId, RsslBuffer *pServiceName, RsslBool updatesOnly, 
		RsslErrorInfo *pErrorInfo);

/* Some directory information is sorted. These functions may be used to perform binary
 * searches on those lists. */
int rscCompareCapabilities(const void *p1, const void *p2);
int rscCompareQos(const void *p1, const void *p2);

#ifdef __cplusplus
};
#endif

#endif

