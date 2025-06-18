/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/wlService.h"

WlService *wlServiceCreate(RDMCachedService *pService, RsslErrorInfo *pErrorInfo)
{
	WlService *pWlService = 
		(WlService*)malloc(sizeof(WlService));

	verify_malloc(pWlService, pErrorInfo, NULL);

	pWlService->pService = pService;
	rsslInitQueue(&pWlService->openStreamList);
	rsslInitQueue(&pWlService->requestedServices);
	rsslInitQueue(&pWlService->streamsPendingRefresh);
	rsslInitQueue(&pWlService->streamsPendingWindow);
	rsslInitQueue(&pWlService->itemGroups);
	
	if (rsslHashTableInit(&pWlService->itemGroupTable, 10, rsslHashBufferSum, rsslHashBufferCompare,
				RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
	{
		wlServiceDestroy(pWlService);
		return NULL;
	}


	if (pService)
		pService->pUserSpec = (void*)pWlService;

	return pWlService;
}

void wlServiceDestroy(WlService *pWlService)
{ 
	rsslHashTableCleanup(&pWlService->itemGroupTable);
	free(pWlService); 
}

WlRequestedService* wlRequestedServiceOpen(WlBase *pBase, RsslBuffer *pServiceName, 
		RsslUInt *pServiceId, RsslErrorInfo *pErrorInfo)
{
	RsslHashLink *pLink;
	RsslUInt32 hashSum;
	WlRequestedService *pRequestedService;
	RDMCachedService *pService;

	assert(pServiceName || pServiceId);

	if (pServiceName)
	{
		hashSum = rsslHashBufferSum(pServiceName);

		pLink = rsslHashTableFind(
				&pBase->requestedSvcByName, (void*)pServiceName, 
				&hashSum);

		if (pLink)
			return RSSL_HASH_LINK_TO_OBJECT(WlRequestedService,
					hlServiceRequests, pLink);

	}
	else
	{
		hashSum = rsslHashU64Sum(pServiceId);

		pLink = rsslHashTableFind(
				&pBase->requestedSvcById, (void*)pServiceId,
				&hashSum);

		if (pLink)
			return RSSL_HASH_LINK_TO_OBJECT(WlRequestedService,
					hlServiceRequests, pLink);
	}

	/* Requested service not present, create new one. */

	if (!(pRequestedService = (WlRequestedService*) malloc(sizeof(WlRequestedService))))
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				"Memory allocation failed.");
		return NULL;
	}

	rsslInitQueue(&pRequestedService->recoveringList);
	rsslInitQueue(&pRequestedService->itemRequests);
	rsslInitQueue(&pRequestedService->directoryRequests);
	rsslInitQueue(&pRequestedService->openDirectoryRequests);
	pRequestedService->flags = 0;

	if (pServiceName)
	{
		pRequestedService->flags |= WL_RSVC_HAS_NAME;
		pRequestedService->serviceName.length = pServiceName->length;
		if (!(pRequestedService->serviceName.data = (char*)malloc(pServiceName->length)))
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
					"Memory allocation failed.");
			free(pRequestedService);
			return NULL;
		}


		memcpy(pRequestedService->serviceName.data, pServiceName->data, pServiceName->length);
		rsslHashTableInsertLink(&pBase->requestedSvcByName, 
				&pRequestedService->hlServiceRequests, 
				(void*)&pRequestedService->serviceName, &hashSum);

	}
	else /* Service ID. */
	{
		pRequestedService->flags |= WL_RSVC_HAS_ID;
		pRequestedService->serviceId = *pServiceId;

		rsslHashTableInsertLink(&pBase->requestedSvcById, 
				&pRequestedService->hlServiceRequests, 
				(void*)&pRequestedService->serviceId, &hashSum);
	}

	rsslQueueAddLinkToBack(&pBase->requestedServices,
			&pRequestedService->qlServiceRequests);

	/* Check if there's a service matching this criteria. */
	pService = wlServiceCacheFindService(pBase->pServiceCache,
			pServiceName, pServiceId);

	// If the service's action is currently DELETE, this has been requested for a directory fanout, so do not set the matching service here.
	if (pService && pService->rdm.action != RSSL_MPEA_DELETE_ENTRY)
	{
		WlService *pWlService = (WlService*)pService->pUserSpec;
		assert(pWlService);
		pRequestedService->pMatchingService = pWlService;
		rsslQueueAddLinkToBack(&pWlService->requestedServices, 
				&pRequestedService->qlDirectoryRequests);
	}
	else
		pRequestedService->pMatchingService = NULL;

	return pRequestedService;
}

void wlRequestedServiceClose(WlBase *pBase, WlRequestedService *pRequestedService)
{ 
	assert(pRequestedService->itemRequests.count == 0);
	assert(pRequestedService->recoveringList.count == 0);

	if (pRequestedService->flags & WL_RSVC_HAS_NAME)
	{
		rsslHashTableRemoveLink(&pBase->requestedSvcByName, &pRequestedService->hlServiceRequests);
	}
	else
	{
		assert(pRequestedService->flags & WL_RSVC_HAS_ID);
		rsslHashTableRemoveLink(&pBase->requestedSvcById, &pRequestedService->hlServiceRequests);
	}

	if (pRequestedService->pMatchingService)
	{
		rsslQueueRemoveLink(&pRequestedService->pMatchingService->requestedServices, 
				&pRequestedService->qlDirectoryRequests);
	}

	rsslQueueRemoveLink(&pBase->requestedServices, &pRequestedService->qlServiceRequests);
}

void wlRequestedServiceDestroy(WlRequestedService *pRequestedService)
{
	if (pRequestedService->flags & WL_RSVC_HAS_NAME)
		free(pRequestedService->serviceName.data);
	free(pRequestedService);
}

void wlRequestedServiceCheckRefCount(WlBase *pBase, WlRequestedService *pRequestedService)
{
	if (pRequestedService->itemRequests.count == 0
			&& pRequestedService->directoryRequests.count == 0)
	{
		wlRequestedServiceClose(pBase, pRequestedService);
		wlRequestedServiceDestroy(pRequestedService);
	}
}
