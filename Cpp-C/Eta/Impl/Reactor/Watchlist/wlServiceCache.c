/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/wlServiceCache.h"
#include "rtr/rsslReactorUtils.h"
#include <assert.h>

/* Delete all existing services from the cache. */
/* Service Cache clear functions. */
void rscClearUpdateEvent(WlServiceCacheUpdateEvent *pEvent);
RDMCachedService *rscCreateService(RsslErrorInfo *pErrorInfo);
void rscRemoveService(WlServiceCache *pServiceCache, RDMCachedService *pCachedService);
void rscDestroyService(RDMCachedService *pCachedService);

/* Update a service in the cache, based on directory message. */
RsslRet rscUpdateService(WlServiceCache *pServiceCache, RDMCachedService *pCachedService, 
		RsslRDMService *pUpdatedService, WlServiceCacheUpdateEvent *pUpdateEvent,
		RsslErrorInfo *pErrorInfo);

RsslRet rscUpdateLink(RDMCachedService *pCachedService, RsslRDMServiceLink *pUpdatedLink,
		RsslErrorInfo *pErrorInfo);

void rscRemoveLink(RDMCachedService *pService, RDMCachedLink *pCachedLink);

WlServiceCache* wlServiceCacheCreate(WlServiceCacheCreateOptions *pOptions, 
		RsslErrorInfo *pErrorInfo)
{
	WlServiceCache *pServiceCache;

	pServiceCache = (WlServiceCache*)malloc(sizeof(WlServiceCache));
	verify_malloc(pServiceCache, pErrorInfo, NULL);

	memset(pServiceCache, 0, sizeof(WlServiceCache));

	if (rsslHeapBufferInit(&pServiceCache->tempMemBuffer, 16384) != RSSL_RET_SUCCESS)
	{
		wlServiceCacheDestroy(pServiceCache);
		return NULL;
	}

	pServiceCache->pUserSpec = pOptions->pUserSpec;
	if (rsslHashTableInit(&pServiceCache->_servicesByName, 100, rsslHashBufferSum, 
					rsslHashBufferCompare, RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		wlServiceCacheDestroy(pServiceCache);
		return NULL;
	}

	if (rsslHashTableInit(&pServiceCache->_servicesById, 100, rsslHashU64Sum, rsslHashU64Compare, 
				RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		wlServiceCacheDestroy(pServiceCache);
		return NULL;
	}
			
	rsslInitQueue(&pServiceCache->_serviceList);
	pServiceCache->_serviceUpdateCallback = pOptions->serviceUpdateCallback;
	pServiceCache->_serviceStateChangeCallback = pOptions->serviceStateChangeCallback;
	pServiceCache->_serviceCacheInitCallback = pOptions->serviceCacheInitCallback;
	pServiceCache->_serviceCacheUpdateCallback = pOptions->serviceCacheUpdateCallback;

	return pServiceCache;
}

void wlServiceCacheDestroy(WlServiceCache *pServiceCache)
{
	wlServiceCacheClear(pServiceCache, RSSL_FALSE, NULL);

	rsslHashTableCleanup(&pServiceCache->_servicesById);
	rsslHashTableCleanup(&pServiceCache->_servicesByName);
	rsslHeapBufferCleanup(&pServiceCache->tempMemBuffer);

	free(pServiceCache);
}

static RsslRet wlscSendUpdatedServiceList(WlServiceCache *pServiceCache, 
		WlServiceCacheUpdateEvent *pUpdateEvent, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	if (pUpdateEvent->updatedServiceList.count)
	{
		RsslQueueLink *pLink;

		ret = (*pServiceCache->_serviceUpdateCallback)(pServiceCache, pUpdateEvent, pErrorInfo);

		/* Cleanup deleted services. Reset any updated flags. */
		RSSL_QUEUE_FOR_EACH_LINK(&pUpdateEvent->updatedServiceList, pLink)
		{
			RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, 
					_updatedServiceLink, pLink);

			if (pService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
			{
				rscRemoveService(pServiceCache, pService);
				rscDestroyService(pService);
			}
			else
			{
				pService->updateFlags = RDM_SVCF_NONE;

				pService->rdm.groupStateList = NULL;
				pService->rdm.groupStateCount = 0;

				rsslClearRDMServiceData(&pService->rdm.data);

				rsslClearRDMServiceLinkInfo(&pService->rdm.linkInfo);
			}

		}

		return ret;
	}

	return RSSL_RET_SUCCESS;
}


RsslRet wlServiceCacheProcessDirectoryMsg(WlServiceCache *pServiceCache, 
		RsslChannel *pChannel, RsslRDMDirectoryMsg *pDirectoryMsg, RsslErrorInfo *pErrorInfo)
{
	RsslRDMService *serviceList = NULL;
	RsslUInt32 serviceCount = 0;
	RsslState *pMsgState = NULL;
	RsslUInt32 i;
	WlServiceCacheUpdateEvent updateEvent;
	RsslQueue deletedServiceList;
	RsslRet ret;
	RsslBool clearCache = RSSL_FALSE;
	RsslBool initDirectory = RSSL_FALSE;

	rscClearUpdateEvent(&updateEvent);

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
			if (pDirectoryMsg->refresh.flags & RDM_DR_RFF_CLEAR_CACHE)
				clearCache = RSSL_TRUE;

			serviceList = pDirectoryMsg->refresh.serviceList;
			serviceCount = pDirectoryMsg->refresh.serviceCount;
			pMsgState = &pDirectoryMsg->refresh.state;

			if (pServiceCache->_serviceList.count == 0)
			{
				initDirectory = RSSL_TRUE;
			}

			break;

		case RDM_DR_MT_UPDATE:
			serviceList = pDirectoryMsg->update.serviceList;
			serviceCount = pDirectoryMsg->update.serviceCount;

			if (pServiceCache->_serviceList.count == 0)
			{
				initDirectory = RSSL_TRUE;
			}

			break;

		case RDM_DR_MT_STATUS:
			if (pDirectoryMsg->status.flags & RDM_DR_STF_CLEAR_CACHE)
				clearCache = RSSL_TRUE;

			if (pDirectoryMsg->status.flags & RDM_SVC_STF_HAS_STATUS)
				pMsgState = &pDirectoryMsg->status.state;
			break;

		case RDM_DR_MT_REQUEST:
		case RDM_DR_MT_CLOSE:
		case RDM_DR_MT_CONSUMER_STATUS:
		default:
			return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pMsgState && pMsgState->streamState != RSSL_STREAM_OPEN)
	{
		if ((ret = wlServiceCacheClear(pServiceCache, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		/* Do not cache information if the stream is not open. Need to recover. */
		return RSSL_RET_SUCCESS;
	}

	if (clearCache)
	{
		if ((ret = wlServiceCacheClear(pServiceCache, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		initDirectory = RSSL_TRUE;
	}

	rsslInitQueue(&deletedServiceList);

	for(i = 0; i < serviceCount; ++i)
	{
		RsslRDMService		*pServiceEntry = &serviceList[i];
		RsslHashLink		*pHashLink;
		RDMCachedService	*pCachedService;

		pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, 
				(void*)&pServiceEntry->serviceId, NULL);

		if (pHashLink)
		{
			pCachedService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);

			if (pServiceEntry->action == RSSL_MPEA_ADD_ENTRY)
				pServiceEntry->action = RSSL_MPEA_UPDATE_ENTRY;

			if (pCachedService->updateFlags)
			{
				/* If a service has been previously updated by this same message, some information
				 * may not be appropriate to conflate. So send what we have now before
				 * processing this message. */
				wlscSendUpdatedServiceList(pServiceCache, &updateEvent, pErrorInfo);
				rscClearUpdateEvent(&updateEvent);

				if (pServiceEntry->action == RSSL_MPEA_DELETE_ENTRY)
					pHashLink = NULL;
			}
		}
		
		if (!pHashLink)
		{
			if (pServiceEntry->action != RSSL_MPEA_ADD_ENTRY)
				continue; /* Ignore unknown directories. */

			if (!(pCachedService = rscCreateService(pErrorInfo)))
				return RSSL_RET_FAILURE;

			pCachedService->rdm.serviceId = pServiceEntry->serviceId;
			rsslHashTableInsertLink(&pServiceCache->_servicesById,
					&pCachedService->_idLink, &pCachedService->rdm.serviceId, NULL);
			rsslQueueAddLinkToBack(&pServiceCache->_serviceList, &pCachedService->_fullListLink);

		}

		if ((ret = rscUpdateService(pServiceCache, pCachedService, pServiceEntry, &updateEvent,
				pErrorInfo)) != RSSL_RET_SUCCESS)
		{
			rscRemoveService(pServiceCache, pCachedService);
			rscDestroyService(pCachedService);
			return ret;
		}

	}

	/* Addtional handling for warm standby feature. */
	if (updateEvent.updatedServiceList.count > 0)
	{
		if (initDirectory)
		{
			if (pServiceCache->_serviceCacheInitCallback)
			{
				pServiceCache->_serviceCacheInitCallback(pServiceCache, &updateEvent, pErrorInfo);
			}
		}
		else
		{
			if (pServiceCache->_serviceCacheUpdateCallback)
			{
				pServiceCache->_serviceCacheUpdateCallback(pServiceCache, &updateEvent, pErrorInfo);
			}
		}
	}

	wlscSendUpdatedServiceList(pServiceCache, &updateEvent, pErrorInfo);

	return RSSL_RET_SUCCESS;
}

RDMCachedService *rscCreateService(RsslErrorInfo *pErrorInfo)
{
	RDMCachedService *pCachedService = (RDMCachedService*)malloc(sizeof(RDMCachedService));
	verify_malloc(pCachedService, pErrorInfo, NULL);

	rsslClearRDMService(&pCachedService->rdm);
	rsslClearBuffer(&pCachedService->oldServiceName);
	pCachedService->updateFlags = RDM_SVCF_NONE;
	pCachedService->infoUpdateFlags = RDM_SVC_IFF_NONE;
	pCachedService->stateUpdateFlags = RDM_SVC_STF_NONE;
	pCachedService->hasServiceName = RSSL_FALSE;
	pCachedService->pUserSpec = NULL;

	rsslInitQueueLink(&pCachedService->_fullListLink);
	rsslHashLinkInit(&pCachedService->_nameLink);
	rsslHashLinkInit(&pCachedService->_idLink);
	rsslInitQueueLink(&pCachedService->_updatedServiceLink);
	rsslInitQueue(&pCachedService->linkList);
	rsslHeapBufferInit(&pCachedService->tempLinkArrayBuffer, sizeof(RsslRDMServiceLink));

	if (rsslHashTableInit(&pCachedService->_itemGroupsById, 101, rsslHashBufferSum, 
				rsslHashBufferCompare, RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rscDestroyService(pCachedService);
		return NULL;
	}

	if (rsslHashTableInit(&pCachedService->linkTable, 11, rsslHashBufferSum, rsslHashBufferCompare, 
			RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		rscDestroyService(pCachedService);
		return NULL;
	}

	return pCachedService;
}

RsslRet rscUpdateLink(RDMCachedService *pCachedService, RsslRDMServiceLink *pUpdatedLink,
		RsslErrorInfo *pErrorInfo)
{
	RsslHashLink *pHashLink;
	RsslUInt32 hashSum;
	RDMCachedLink *pCachedLink;

	hashSum = rsslHashBufferSum(&pUpdatedLink->name);
	if (!(pHashLink = rsslHashTableFind(&pCachedService->linkTable, &pUpdatedLink->name, &hashSum)))
	{
		/* Create link. */

		if (pUpdatedLink->action == RSSL_MPEA_DELETE_ENTRY)
			return RSSL_RET_SUCCESS; /* Ignore unknown links */

		pCachedLink = (RDMCachedLink*)malloc(sizeof(RDMCachedLink));
		verify_malloc(pCachedLink, pErrorInfo, RSSL_RET_FAILURE);
		rsslClearRDMServiceLink(&pCachedLink->link);

		if (rsslHeapBufferCreateCopy(&pCachedLink->link.name, &pUpdatedLink->name) 
				!= RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failed.");
			free(pCachedLink);
			return RSSL_RET_FAILURE;
		}

		rsslQueueAddLinkToBack(&pCachedService->linkList, &pCachedLink->qlLinkList);
		rsslHashTableInsertLink(&pCachedService->linkTable, &pCachedLink->hlLinkTable,
				(void*)&pCachedLink->link.name, &hashSum);
	}
	else
		pCachedLink = RSSL_HASH_LINK_TO_OBJECT(RDMCachedLink, hlLinkTable, pHashLink);

	if (pUpdatedLink->action == RSSL_MPEA_DELETE_ENTRY)
	{
		rscRemoveLink(pCachedService, pCachedLink);
		return RSSL_RET_SUCCESS;
	}

	pCachedLink->linkUpdateFlags = RDM_SVC_LKF_NONE;

	if (pUpdatedLink->flags & RDM_SVC_LKF_HAS_TYPE && pCachedLink->link.type != pUpdatedLink->type)
	{
		pCachedLink->linkUpdateFlags |= RDM_SVC_LKF_HAS_TYPE;
		pCachedLink->link.flags |= RDM_SVC_LKF_HAS_TYPE;
		pCachedLink->link.type = pUpdatedLink->type;
	}

	if (pCachedLink->link.linkState != pUpdatedLink->linkState)
	{
		pCachedLink->link.linkState = pUpdatedLink->linkState;
	}

	if (pUpdatedLink->flags & RDM_SVC_LKF_HAS_CODE && 
			pCachedLink->link.linkCode != pUpdatedLink->linkCode)
	{
		pCachedLink->linkUpdateFlags |= RDM_SVC_LKF_HAS_CODE;
		pCachedLink->link.flags |= RDM_SVC_LKF_HAS_CODE;
		pCachedLink->link.linkCode = pUpdatedLink->linkCode;
	}

	if (pUpdatedLink->flags & RDM_SVC_LKF_HAS_TEXT)
	{
		pCachedLink->linkUpdateFlags |= RDM_SVC_LKF_HAS_TEXT;
		pCachedLink->link.flags |= RDM_SVC_LKF_HAS_TEXT;
		rsslHeapBufferCopy(&pCachedLink->link.text, &pUpdatedLink->text, &pCachedLink->link.text);
	}

	pUpdatedLink->flags = pCachedLink->linkUpdateFlags;

	return RSSL_RET_SUCCESS;
}

void rscRemoveLink(RDMCachedService *pService, RDMCachedLink *pCachedLink)
{
	rsslHashTableRemoveLink(&pService->linkTable, &pCachedLink->hlLinkTable);
	rsslHeapBufferCleanup(&pCachedLink->link.name);
	if (pCachedLink->link.flags & RDM_SVC_LKF_HAS_TEXT)
		rsslHeapBufferCleanup(&pCachedLink->link.text);
	free(pCachedLink);
}

void rscClearUpdateEvent(WlServiceCacheUpdateEvent *pEvent)
{
	rsslInitQueue(&pEvent->updatedServiceList);
}

void rscCleanupInfoFilter(RDMCachedService *pCachedService)
{
	RsslRDMServiceInfo *pCachedInfo = &pCachedService->rdm.info;
	RsslUInt32 ui;

	if (pCachedService->hasServiceName)
		rsslHeapBufferCleanup(&pCachedInfo->serviceName);

	if (pCachedInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
		rsslHeapBufferCleanup(&pCachedInfo->vendor);

	if (pCachedInfo->capabilitiesList)
		free(pCachedInfo->capabilitiesList);

	if (pCachedInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
	{
		for(ui = 0; ui < pCachedInfo->dictionariesProvidedCount; ++ui)
			rsslHeapBufferCleanup(&pCachedInfo->dictionariesProvidedList[ui]);

		free(pCachedInfo->dictionariesProvidedList);
	}

	if (pCachedInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
	{
		for(ui = 0; ui < pCachedInfo->dictionariesUsedCount; ++ui)
			rsslHeapBufferCleanup(&pCachedInfo->dictionariesUsedList[ui]);

		free(pCachedInfo->dictionariesUsedList);
	}


	if (pCachedInfo->flags & RDM_SVC_IFF_HAS_QOS)
		free(pCachedInfo->qosList);

	if (pCachedInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST)
		rsslHeapBufferCleanup(&pCachedInfo->itemList);
}

void rscCleanupLinkFilter(RDMCachedService *pCachedService)
{
	RsslQueueLink *pQueueLink;
	while((pQueueLink = rsslQueueRemoveFirstLink(&pCachedService->linkList)))
	{
		RDMCachedLink *pCachedLink = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedLink, qlLinkList, 
				pQueueLink);
		rscRemoveLink(pCachedService, pCachedLink);
	}
}

void rscRemoveService(WlServiceCache *pServiceCache, RDMCachedService *pCachedService)
{
	rsslQueueRemoveLink(&pServiceCache->_serviceList, &pCachedService->_fullListLink);
	rsslHashTableRemoveLink(&pServiceCache->_servicesById, &pCachedService->_idLink);

	if (pCachedService->hasServiceName)
		rsslHashTableRemoveLink(&pServiceCache->_servicesByName, &pCachedService->_nameLink);
}

void rscDestroyService(RDMCachedService *pCachedService)
{
	RsslRDMService *pService = &pCachedService->rdm;

	rsslHeapBufferCleanup(&pCachedService->oldServiceName);

	if (pService->flags & RDM_SVCF_HAS_INFO)
		rscCleanupInfoFilter(pCachedService);

	if (pService->flags & RDM_SVCF_HAS_LINK)
		rscCleanupLinkFilter(pCachedService);


	rsslHashTableCleanup(&pCachedService->_itemGroupsById);
	rsslHashTableCleanup(&pCachedService->linkTable);

	rsslHeapBufferCleanup(&pCachedService->tempLinkArrayBuffer);

	free(pCachedService);
}

/* Used for sorting the QoS list. */
int rscCompareQos(const void *p1, const void *p2)
{
	const RsslQos *pQos1 = (const RsslQos*)p1;
	const RsslQos *pQos2 = (const RsslQos*)p2;

	if (rsslQosIsBetter(pQos1, pQos2))
		return -1;
	else if (rsslQosIsEqual(pQos1, pQos2))
		return 0;
	else
		return 1;
}

/* Used for sorting the capabilities list. */
int rscCompareCapabilities(const void *p1, const void *p2)
{
	const RsslUInt cap1 = *((const RsslUInt*)p1);
	const RsslUInt cap2 = *((const RsslUInt*)p2);
	
	if (cap1 < cap2) return -1;
	else if (cap1 == cap2) return 0;
	else return 1;
}

RsslRet rscUpdateService(WlServiceCache *pServiceCache, RDMCachedService *pCachedService, 
		RsslRDMService *pUpdatedService, WlServiceCacheUpdateEvent *pUpdateEvent,
		RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 ui;
	RsslRet ret;

	pCachedService->updateFlags = RDM_SVCF_NONE;
	pCachedService->infoUpdateFlags = RDM_SVC_IFF_NONE;
	pCachedService->stateUpdateFlags = RDM_SVC_STF_NONE;
	pCachedService->loadUpdateFlags = RDM_SVC_LDF_NONE;
	pCachedService->rdm.groupStateCount = 0;
	pCachedService->rdm.groupStateList = NULL;
	pCachedService->rdm.action = pUpdatedService->action;
	pCachedService->updatedServiceName = RSSL_FALSE;

	if (pCachedService->rdm.action != RSSL_MPEA_DELETE_ENTRY)
	{
		/* Info Filter. */
		if (pUpdatedService->flags & RDM_SVCF_HAS_INFO)
		{
			RsslRDMServiceInfo *pUpdatedInfo = &pUpdatedService->info;
			RsslRDMServiceInfo *pCachedInfo = &pCachedService->rdm.info;

			pCachedService->updateFlags |= RDM_SVCF_HAS_INFO;

			switch(pUpdatedInfo->action)
			{
				case RSSL_FTEA_SET_ENTRY:
				case RSSL_FTEA_UPDATE_ENTRY:

					/* Set/Update information (Directory decoder sets most defaults when
					 * elements were not present in the message, so we can generally just
					 * check if the value is different than the current one). */

					if (pCachedService->rdm.flags & RDM_SVCF_HAS_INFO)
						pCachedService->rdm.info.action = RSSL_FTEA_UPDATE_ENTRY;

					pCachedService->rdm.flags |= RDM_SVCF_HAS_INFO;

					/* ServiceName */
					if (pCachedService->hasServiceName)
					{
						if (!rsslBufferIsEqual(&pCachedInfo->serviceName, &pUpdatedInfo->serviceName))
						{
							/* Service name changed. */

							RsslUInt32 hashSum = rsslHashBufferSum(&pUpdatedInfo->serviceName);

							pCachedService->updatedServiceName = RSSL_TRUE;

							rsslHeapBufferCopy(&pCachedService->oldServiceName, &pCachedInfo->serviceName,
									&pCachedService->oldServiceName);

							if (rsslHashTableFind(&pServiceCache->_servicesByName, 
										&pUpdatedInfo->serviceName, &hashSum))
							{
								/* Service already exists; return an error. */
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Received directory message with duplicate service name.");
								return RSSL_RET_FAILURE;
							}

							rsslHashTableRemoveLink(&pServiceCache->_servicesByName,
									&pCachedService->_nameLink);

							rsslHeapBufferCopy(&pCachedInfo->serviceName, &pUpdatedInfo->serviceName,
									&pCachedInfo->serviceName);

							rsslHashTableInsertLink(&pServiceCache->_servicesByName,
									&pCachedService->_nameLink, &pCachedInfo->serviceName, &hashSum);
						}
					}
					else
					{
						RsslUInt32 hashSum = rsslHashBufferSum(&pUpdatedInfo->serviceName);

						if (rsslHashTableFind(&pServiceCache->_servicesByName, 
									&pUpdatedInfo->serviceName, &hashSum))
						{
							/* Service already exists; return an error. */
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Received directory message with duplicate service name.");
							return RSSL_RET_FAILURE;
						}

						if (rsslHeapBufferCreateCopy(&pCachedInfo->serviceName, 
									&pUpdatedInfo->serviceName) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Memory allocation failed.");
							return RSSL_RET_FAILURE;
						}

						pCachedService->hasServiceName = RSSL_TRUE;

						rsslHashTableInsertLink(&pServiceCache->_servicesByName,
								&pCachedService->_nameLink, &pCachedInfo->serviceName, &hashSum);
					}

					/* Vendor */
					if (pUpdatedInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_VENDOR;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_VENDOR;

						if (pCachedInfo->flags & RDM_SVC_IFF_HAS_VENDOR) 
							rsslHeapBufferCopy(&pCachedInfo->vendor, &pUpdatedInfo->vendor,
									&pCachedInfo->vendor);
						else if (rsslHeapBufferCreateCopy(&pCachedInfo->vendor, 
									&pUpdatedInfo->vendor) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Memory allocation failed.");
							return RSSL_RET_FAILURE;
						}
					}

					/* IsSource */
					if (pUpdatedInfo->isSource != pCachedInfo->isSource)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_IS_SOURCE;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
						pCachedInfo->isSource = pUpdatedInfo->isSource;
					}

					/* Capabilities */
					if (pCachedInfo->capabilitiesList)
						free(pCachedInfo->capabilitiesList);
					pCachedInfo->capabilitiesCount = pUpdatedInfo->capabilitiesCount;
					if (pCachedInfo->capabilitiesCount)
					{
						RsslUInt32 capabilitiesSize = pUpdatedInfo->capabilitiesCount * sizeof(RsslUInt);
						pCachedInfo->capabilitiesList = (RsslUInt*)malloc(capabilitiesSize);

						verify_malloc(pCachedInfo->capabilitiesList, pErrorInfo, RSSL_RET_FAILURE);

						memcpy(pCachedInfo->capabilitiesList, pUpdatedInfo->capabilitiesList, 
								capabilitiesSize);

						qsort(pCachedInfo->capabilitiesList, pCachedInfo->capabilitiesCount, 
								sizeof(RsslUInt), rscCompareCapabilities);
					}
					else
						pCachedInfo->capabilitiesList = NULL;

					/* DictionariesProvided */
					if (pUpdatedInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
					{
						/* Set filter as updated. */
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

						/* Clear current memory. */
						if (pCachedInfo->dictionariesProvidedList)
						{
							for(ui = 0; ui < pCachedInfo->dictionariesProvidedCount; ++ui)
								rsslHeapBufferCleanup(&pCachedInfo->dictionariesProvidedList[ui]);
							free(pCachedInfo->dictionariesProvidedList);
						}

						pCachedInfo->dictionariesProvidedCount = pUpdatedInfo->dictionariesProvidedCount;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

						/* Copy dictionary list. */
						pCachedInfo->dictionariesProvidedList = (RsslBuffer*)malloc(
								sizeof(RsslBuffer) * pUpdatedInfo->dictionariesProvidedCount);

						verify_malloc(pCachedInfo->dictionariesProvidedList, pErrorInfo, RSSL_RET_FAILURE);

						for(ui = 0; ui < pCachedInfo->dictionariesProvidedCount; ++ui)
							if (rsslHeapBufferCreateCopy(&pCachedInfo->dictionariesProvidedList[ui],
									&pUpdatedInfo->dictionariesProvidedList[ui]) != RSSL_RET_SUCCESS)
							{
								pCachedInfo->dictionariesProvidedCount = ui;
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}

					}

					if (pUpdatedInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
					{
						/* Set filter as updated. */
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_DICTS_USED;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_DICTS_USED;
						
						/* Clear current memory. */
						if (pCachedInfo->dictionariesUsedList)
						{
							for(ui = 0; ui < pCachedInfo->dictionariesUsedCount; ++ui)
								rsslHeapBufferCleanup(&pCachedInfo->dictionariesUsedList[ui]);
							free(pCachedInfo->dictionariesUsedList);
						}

						pCachedInfo->dictionariesUsedCount = pUpdatedInfo->dictionariesUsedCount;

						/* Copy dictionary list. */
						pCachedInfo->dictionariesUsedList = (RsslBuffer*)malloc(
								sizeof(RsslBuffer) * pUpdatedInfo->dictionariesUsedCount);

						verify_malloc(pCachedInfo->dictionariesUsedList, pErrorInfo, RSSL_RET_FAILURE);

						for(ui = 0; ui < pCachedInfo->dictionariesUsedCount; ++ui)
							if (rsslHeapBufferCreateCopy(&pCachedInfo->dictionariesUsedList[ui],
									&pUpdatedInfo->dictionariesUsedList[ui]) != RSSL_RET_SUCCESS)
							{
								pCachedInfo->dictionariesUsedCount = ui;
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
										"Memory allocation failed.");
								return RSSL_RET_FAILURE;
							}
					}

					if (pUpdatedInfo->flags & RDM_SVC_IFF_HAS_QOS)
					{
						int qosListSize = sizeof(RsslQos) * pUpdatedInfo->qosCount;

						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_QOS;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_QOS;

						/* Clear current memory. */
						if (pCachedInfo->qosList)
							free(pCachedInfo->qosList);

						pCachedInfo->qosCount = pUpdatedInfo->qosCount;
						pCachedInfo->qosList = (RsslQos*)malloc(qosListSize);
						verify_malloc(pCachedInfo->qosList, pErrorInfo, RSSL_RET_FAILURE);
						memcpy(pCachedInfo->qosList, pUpdatedInfo->qosList, qosListSize);

						/* Sort the QoS list from best to worst. */
						qsort(pCachedInfo->qosList, pCachedInfo->qosCount, sizeof(RsslQos),
								rscCompareQos);
					}
					else if (!(pCachedInfo->flags & RDM_SVC_IFF_HAS_QOS))
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_QOS;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_QOS;

						/* If no QoS was ever provided, the default is RealTime/Tick-by-tick. */
						pCachedInfo->qosCount = 1;
						pCachedInfo->qosList = (RsslQos*)malloc(sizeof(RsslQos));
						verify_malloc(pCachedInfo->qosList, pErrorInfo, RSSL_RET_FAILURE);
						rsslClearQos(&pCachedInfo->qosList[0]);
						pCachedInfo->qosList[0].timeliness = RSSL_QOS_TIME_REALTIME;
						pCachedInfo->qosList[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;
					}

					/* ItemList */
					if (pUpdatedInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_ITEM_LIST;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_ITEM_LIST;

						if (pCachedInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST) 
							rsslHeapBufferCopy(&pCachedInfo->itemList, &pUpdatedInfo->itemList,
									&pCachedInfo->itemList);
						else if (rsslHeapBufferCreateCopy(&pCachedInfo->itemList, 
									&pUpdatedInfo->itemList) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
									"Memory allocation failed.");
							return RSSL_RET_FAILURE;
						}
					}

					if (pUpdatedInfo->supportsQosRange != pCachedInfo->supportsQosRange)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
						pCachedInfo->supportsQosRange = pUpdatedInfo->supportsQosRange;
					}

					if (pUpdatedInfo->supportsOutOfBandSnapshots != pCachedInfo->supportsOutOfBandSnapshots)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
						pCachedInfo->supportsOutOfBandSnapshots = pUpdatedInfo->supportsOutOfBandSnapshots;
					}

					if (pUpdatedInfo->acceptingConsumerStatus != pCachedInfo->acceptingConsumerStatus)
					{
						pCachedService->infoUpdateFlags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
						pCachedInfo->flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
						pCachedInfo->acceptingConsumerStatus = pUpdatedInfo->acceptingConsumerStatus;
					}

					break;


				case RSSL_FTEA_CLEAR_ENTRY:
					pCachedService->rdm.flags &= ~RDM_SVCF_HAS_INFO;
					if (pCachedService->hasServiceName)
						pCachedService->updatedServiceName = RSSL_TRUE;
					pCachedService->hasServiceName = RSSL_FALSE;
					pCachedService->infoUpdateFlags = pCachedService->rdm.info.flags;
					rsslHashTableRemoveLink(&pServiceCache->_servicesByName,
							&pCachedService->_nameLink);
					pCachedService->rdm.info.action = RSSL_FTEA_CLEAR_ENTRY;
					break;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
							"Unknown Info filter action %u.", pUpdatedInfo->action);
					return RSSL_RET_INVALID_DATA;
			}

		}

		/* State Filter. */
		if (pUpdatedService->flags & RDM_SVCF_HAS_STATE)
		{
			RsslRDMServiceState *pCachedState = &pCachedService->rdm.state;
			RsslRDMServiceState *pUpdatedState = &pUpdatedService->state;

			pCachedService->updateFlags |= RDM_SVCF_HAS_STATE;

			switch(pUpdatedState->action)
			{
				case RSSL_FTEA_SET_ENTRY:
				case RSSL_FTEA_UPDATE_ENTRY:

					if (pCachedService->rdm.flags & RDM_SVCF_HAS_STATE)
						pCachedService->rdm.state.action = RSSL_FTEA_UPDATE_ENTRY;

					pCachedService->rdm.flags |= RDM_SVCF_HAS_STATE;

					if (pCachedState->serviceState != pUpdatedState->serviceState)
					{
						pCachedState->serviceState = pUpdatedState->serviceState;

						if (pServiceCache->_serviceStateChangeCallback != NULL)
						{
							pServiceCache->_serviceStateChangeCallback(pServiceCache, pCachedService, pErrorInfo);
						}
					}

					if (pCachedState->acceptingRequests != pUpdatedState->acceptingRequests)
					{
						pCachedService->stateUpdateFlags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
						pCachedState->flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
						pCachedState->acceptingRequests = pUpdatedState->acceptingRequests;
					}
					else
					{
						pCachedService->stateUpdateFlags &= ~RDM_SVC_STF_HAS_ACCEPTING_REQS;
						pCachedState->flags &= ~RDM_SVC_STF_HAS_ACCEPTING_REQS;
					}

					/* Soft-copy service status (not cached, only needs to be valid during callback). */
					if (pUpdatedState->flags & RDM_SVC_STF_HAS_STATUS)
					{
						pCachedService->stateUpdateFlags |= RDM_SVC_STF_HAS_STATUS;

						pCachedState->flags |= RDM_SVC_STF_HAS_STATUS;
						pCachedService->rdm.state.status = pUpdatedService->state.status;
					}
					else
						pUpdatedState->flags &= ~RDM_SVC_STF_HAS_STATUS;
					break;

				case RSSL_FTEA_CLEAR_ENTRY:
					pCachedService->rdm.flags &= ~RDM_SVCF_HAS_STATE;
					pCachedService->stateUpdateFlags = pCachedService->rdm.state.flags;
					rsslClearRDMServiceState(pCachedState);
					pCachedService->rdm.state.action = RSSL_FTEA_CLEAR_ENTRY;
					break;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
							"Unknown State filter action %u.", pUpdatedState->action);
					return RSSL_RET_INVALID_DATA;

			}
		}


		/* Group Filter. */
		/* Soft-copy group status (not cached, only needs to be valid during callback). */
		if (pUpdatedService->groupStateCount)
		{
			pCachedService->rdm.groupStateCount = pUpdatedService->groupStateCount;
			pCachedService->rdm.groupStateList = pUpdatedService->groupStateList;
		}
		else
		{
			pCachedService->rdm.groupStateCount = 0;
			pCachedService->rdm.groupStateList = NULL;
		}

		/* Data Filter. */
		/* Soft-copy (not cached, only needs to be valid during callback). */
		if (pUpdatedService->flags & RDM_SVCF_HAS_DATA)
		{
			RsslRDMServiceData *pCachedData = &pCachedService->rdm.data;
			RsslRDMServiceData *pUpdatedData = &pUpdatedService->data;

			pCachedService->updateFlags |= RDM_SVCF_HAS_DATA;
			*pCachedData = *pUpdatedData;
		}

		/* Load Filter. */
		if (pUpdatedService->flags & RDM_SVCF_HAS_LOAD)
		{
			RsslRDMServiceLoad *pCachedLoad = &pCachedService->rdm.load;
			RsslRDMServiceLoad *pUpdatedLoad = &pUpdatedService->load;

			pCachedService->updateFlags |= RDM_SVCF_HAS_LOAD;

			switch(pUpdatedLoad->action)
			{
				case RSSL_FTEA_SET_ENTRY:
				case RSSL_FTEA_UPDATE_ENTRY:

					if (pCachedService->rdm.flags & RDM_SVCF_HAS_LOAD)
						pCachedService->rdm.load.action = RSSL_FTEA_UPDATE_ENTRY;

					pCachedService->rdm.flags |= RDM_SVCF_HAS_LOAD;

					if (pUpdatedLoad->flags & RDM_SVC_LDF_HAS_OPEN_LIMIT
							&& ( !(pCachedLoad->flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) || (pCachedLoad->openLimit != pUpdatedLoad->openLimit) ) )
					{
						pCachedService->loadUpdateFlags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
						pCachedLoad->flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
						pCachedLoad->openLimit = pUpdatedLoad->openLimit;
					}

					if (pUpdatedLoad->flags & RDM_SVC_LDF_HAS_OPEN_WINDOW
							&& ( !(pCachedLoad->flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) || (pCachedLoad->openWindow != pUpdatedLoad->openWindow) ) )
					{
						pCachedService->loadUpdateFlags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
						pCachedLoad->flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
						pCachedLoad->openWindow = pUpdatedLoad->openWindow;
					}

					if (pUpdatedLoad->flags & RDM_SVC_LDF_HAS_LOAD_FACTOR
							&& ( !(pCachedLoad->flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) || (pCachedLoad->loadFactor != pUpdatedLoad->loadFactor) ) )
					{
						pCachedService->loadUpdateFlags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
						pCachedLoad->flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
						pCachedLoad->loadFactor = pUpdatedLoad->loadFactor;
					}
					break;

				case RSSL_FTEA_CLEAR_ENTRY:
					pCachedService->rdm.flags &= ~RDM_SVCF_HAS_LOAD;
					pCachedService->loadUpdateFlags = pCachedService->rdm.load.flags;
					rsslClearRDMServiceLoad(&pCachedService->rdm.load);
					pCachedService->rdm.load.action = RSSL_FTEA_CLEAR_ENTRY;
					break;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
							"Unknown Load filter action %u.", pUpdatedLoad->action);
					return RSSL_RET_INVALID_DATA;
			}
		}

		/* Link Filter. */
		if (pUpdatedService->flags & RDM_SVCF_HAS_LINK)
		{
			pCachedService->updateFlags |= RDM_SVCF_HAS_LINK;

			switch(pUpdatedService->linkInfo.action)
			{
				case RSSL_FTEA_SET_ENTRY:
				case RSSL_FTEA_UPDATE_ENTRY:
				{
					if (pCachedService->rdm.flags & RDM_SVCF_HAS_LINK)
						pCachedService->rdm.linkInfo.action = RSSL_FTEA_UPDATE_ENTRY;

					pCachedService->rdm.flags |= RDM_SVCF_HAS_LINK;

					for (ui = 0; ui < pUpdatedService->linkInfo.linkCount; ++ui)
					{

						if ((ret = rscUpdateLink(pCachedService, 
										&pUpdatedService->linkInfo.linkList[ui], pErrorInfo))
								!= RSSL_RET_SUCCESS)
							return ret;
					}


					break;
				}

				case RSSL_FTEA_CLEAR_ENTRY:
					pCachedService->rdm.flags &= ~RDM_SVCF_HAS_LINK;
					rscCleanupLinkFilter(pCachedService);
					rsslClearRDMServiceLinkInfo(&pCachedService->rdm.linkInfo);
					pCachedService->rdm.linkInfo.action = RSSL_FTEA_CLEAR_ENTRY;
					break;

				default:
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
							"Unknown Link filter action %u.", pUpdatedService->linkInfo.action);
					return RSSL_RET_INVALID_DATA;
			}

			/* Soft-copy link updates. */
			pCachedService->rdm.linkInfo = pUpdatedService->linkInfo;
		}
	}

	if (pCachedService->updateFlags
			|| pCachedService->rdm.groupStateCount
			|| pCachedService->rdm.action == RSSL_MPEA_DELETE_ENTRY)
		rsslQueueAddLinkToBack(&pUpdateEvent->updatedServiceList
				, &pCachedService->_updatedServiceLink);

	return RSSL_RET_SUCCESS;
}

RsslRet wlServiceCacheClear(WlServiceCache *pServiceCache, RsslBool callback, 
		RsslErrorInfo *pErrorInfo)
{
	RsslQueueLink *pLink;

	WlServiceCacheUpdateEvent updateEvent;

	rscClearUpdateEvent(&updateEvent);

	if (callback)
	{
		assert(pErrorInfo);

		/* Build deletion list, and call user. */
		pLink = rsslQueuePeekFront(&pServiceCache->_serviceList);
		while (pLink)
		{
			RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink, 
					pLink);

			/* Only send out delete actions for services the watchlist has seen. */
			if (pService->pUserSpec)
			{
				pService->rdm.action = RSSL_MPEA_DELETE_ENTRY;
				rsslQueueAddLinkToBack(&updateEvent.updatedServiceList, &pService->_updatedServiceLink);
			}

			pLink = rsslQueuePeekNext(&pServiceCache->_serviceList, pLink);
		}

		if (updateEvent.updatedServiceList.count)
			(*pServiceCache->_serviceUpdateCallback)(pServiceCache, &updateEvent, pErrorInfo);
	}

	/* Clean up list. */
	while ((pLink = rsslQueuePeekFront(&pServiceCache->_serviceList)))
	{
		RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink, 
				pLink);

		rscRemoveService(pServiceCache, pService);
		rscDestroyService(pService);
	}

	rsslInitQueue(&pServiceCache->_serviceList);

	return RSSL_RET_SUCCESS;
}

RDMCachedService* wlServiceCacheFindService(WlServiceCache *pServiceCache, 
		const RsslBuffer *pServiceName, const RsslUInt *pServiceId)
{
	assert(pServiceName || pServiceId);
	assert(!(pServiceName && pServiceId));

	if (pServiceName)
	{	
		RsslHashLink *pLink = rsslHashTableFind(&pServiceCache->_servicesByName, (void*)pServiceName, NULL);
		return (pLink ? RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _nameLink, pLink) : NULL);
	}
	else /* if (pServiceId) */
	{
		RsslHashLink *pLink = rsslHashTableFind(&pServiceCache->_servicesById, (void*)pServiceId, NULL);
		return (pLink ? RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pLink) : NULL);
	}
		
}

/* Build the full list of cached links. Used when providing the link filter with a refresh. */
static RsslRet wlServiceCacheSetRefreshLinkInfo(RDMCachedService *pService, 
		RsslRDMServiceLinkInfo *pLinkInfo, RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 bufferLength = pService->linkList.count * sizeof(RsslRDMServiceLink);
	RsslUInt32 ui;
	RsslQueueLink *pLink;
	RsslRDMServiceLink *linkArray;

	if (!pService->linkList.count)
	{
		pLinkInfo->linkList = NULL;
		pLinkInfo->linkCount = 0;
		return RSSL_RET_SUCCESS;
	}

	/* Resize link list buffer if needed. */
	if (rsslHeapBufferResize(&pService->tempLinkArrayBuffer, bufferLength, RSSL_FALSE) 
			!= RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failure.");
		return RSSL_RET_FAILURE;
	}

	ui = 0;
	linkArray = (RsslRDMServiceLink*)pService->tempLinkArrayBuffer.data;
	RSSL_QUEUE_FOR_EACH_LINK(&pService->linkList, pLink)
	{
		RDMCachedLink *pCachedLink = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedLink, qlLinkList, pLink);
		assert(ui < pService->linkList.count);
		linkArray[ui] = pCachedLink->link;
		linkArray[ui].action = RSSL_MPEA_ADD_ENTRY;
		++ui;
	}

	pLinkInfo->linkList = linkArray;
	pLinkInfo->linkCount = pService->linkList.count;
	return RSSL_RET_SUCCESS;
}

RsslRet wlServiceCacheGetServiceList(WlServiceCache *pServiceCache, WlServiceList *pServiceList,
		RsslUInt *pServiceId, RsslBuffer *pServiceName, RsslBool updatesOnly, 
		RsslErrorInfo *pErrorInfo)
{
	RsslUInt32 msgLength = 0;
	RDMCachedService *pService;
	RsslRet ret;
	RsslRDMService *wlServiceList;

	pServiceList->serviceCount = 0;
	pServiceList->serviceList = NULL;

	if (pServiceId || pServiceName)
		msgLength = sizeof(RsslRDMService);
	else
		msgLength = sizeof(RsslRDMService) 
			* rsslQueueGetElementCount(&pServiceCache->_serviceList);


	/* Resize/reset reusable buffer */
	if (rsslHeapBufferResize(&pServiceCache->tempMemBuffer, msgLength, RSSL_FALSE) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failure.");
		return RSSL_RET_FAILURE;
	}

	wlServiceList = (RsslRDMService*)pServiceCache->tempMemBuffer.data;

	if (pServiceId)
	{
		RsslHashLink *pHashLink;
		RDMCachedService *pService;

		pHashLink = rsslHashTableFind(&pServiceCache->_servicesById, pServiceId, NULL);

		if (!pHashLink)
			return RSSL_RET_SUCCESS;

		pService = RSSL_HASH_LINK_TO_OBJECT(RDMCachedService, _idLink, pHashLink);
		pServiceList->serviceCount = 1;
		pServiceList->serviceList = wlServiceList;

		if ((ret = wlServiceCacheSetRefreshLinkInfo(pService, &pService->rdm.linkInfo, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		wlServiceList[0] = pService->rdm;
		wlServiceList[0].action = RSSL_MPEA_ADD_ENTRY;
		wlServiceList[0].info.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].state.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].load.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].linkInfo.action = RSSL_FTEA_SET_ENTRY;

		return RSSL_RET_SUCCESS;
	}
	else if (pServiceName)
	{
		RsslHashLink *pHashLink;

		pHashLink = rsslHashTableFind(&pServiceCache->_servicesByName, pServiceName, NULL);

		if (!pHashLink)
			return RSSL_RET_SUCCESS;

		pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _nameLink, pHashLink);
		pServiceList->serviceCount = 1;
		pServiceList->serviceList = wlServiceList;

		if ((ret = wlServiceCacheSetRefreshLinkInfo(pService, &pService->rdm.linkInfo, pErrorInfo))
				!= RSSL_RET_SUCCESS)
			return ret;

		wlServiceList[0] = pService->rdm;
		wlServiceList[0].action = RSSL_MPEA_ADD_ENTRY;
		wlServiceList[0].info.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].state.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].load.action = RSSL_FTEA_SET_ENTRY;
		wlServiceList[0].linkInfo.action = RSSL_FTEA_SET_ENTRY;

		return RSSL_RET_SUCCESS;
	}
	else
	{
		/* Full list. */
		RsslUInt32 ui = 0;
		RsslQueueLink *pLink;

		if (pServiceCache->_serviceList.count == 0)
			return RSSL_RET_SUCCESS;

		RSSL_QUEUE_FOR_EACH_LINK(&pServiceCache->_serviceList, pLink)
		{
			RDMCachedService *pService = RSSL_QUEUE_LINK_TO_OBJECT(RDMCachedService, _fullListLink,
					pLink);

			if ((ret = wlServiceCacheSetRefreshLinkInfo(pService, &pService->rdm.linkInfo, pErrorInfo))
					!= RSSL_RET_SUCCESS)
				return ret;

			wlServiceList[ui] = pService->rdm;

			wlServiceList[ui].action = RSSL_MPEA_ADD_ENTRY;
			wlServiceList[ui].info.action = RSSL_FTEA_SET_ENTRY;
			wlServiceList[ui].state.action = RSSL_FTEA_SET_ENTRY;
			wlServiceList[ui].load.action = RSSL_FTEA_SET_ENTRY;
			wlServiceList[ui].linkInfo.action = RSSL_FTEA_SET_ENTRY;

			++ui;
		}

		assert(ui == rsslQueueGetElementCount(&pServiceCache->_serviceList));

		pServiceList->serviceCount = ui;
		pServiceList->serviceList = wlServiceList;

		return RSSL_RET_SUCCESS;
	}


}
