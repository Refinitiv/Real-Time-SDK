/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/wlSymbolList.h"

static RsslRet wlExtractSymbolListFromMsg(WlBase *pBase, RsslRequestMsg *pRequestMsg,
		RsslUInt32 majorVersion, RsslUInt32 minorVersion, RsslBool *pHasBehaviors,
		RsslUInt *pSymbolListFlags, RsslErrorInfo *pErrorInfo)
{
	RsslDecodeIterator dIter;

	RsslRet ret = 0;
	RsslElementList elementList;
	RsslElementEntry elementEntry;

	/* setup decode iterator */
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, majorVersion, minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRequestMsg->msgBase.encDataBody);

	rsslClearElementList(&elementList);

	*pSymbolListFlags = 0;
	*pHasBehaviors = RSSL_FALSE;

	if (pRequestMsg->msgBase.containerType != RSSL_DT_ELEMENT_LIST)
		return RSSL_RET_SUCCESS; /* Nothing to extract. */

	/* check the container type and decode element list */ 
	if ((ret = rsslDecodeElementList(&dIter, &elementList, 0)) == RSSL_RET_SUCCESS)
	{
		rsslClearElementEntry(&elementEntry);

		/* find and decode the :SymbolListBehaviors */
		while ((ret = rsslDecodeElementEntry(&dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret < RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
						"Error decoding element entry in Request Msg -- %d.", ret);
				return ret;
			}
			else
			{
				if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SYMBOL_LIST_BEHAVIORS))
				{
					RsslElementList behaviorsElementList;

					*pHasBehaviors = RSSL_TRUE;

					if (elementEntry.dataType != RSSL_DT_ELEMENT_LIST)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
								"Error decoding Symbol List behaviors -- Element has wrong data type.");
						return ret;
					}

					if ((ret = rsslDecodeElementList(&dIter, &behaviorsElementList, 0)) == RSSL_RET_SUCCESS)
					{
						RsslElementEntry behaviorsEntry;
						rsslClearElementEntry(&behaviorsEntry);

						/* find and decode the :DataStreams */
						while ((ret = rsslDecodeElementEntry(&dIter, &behaviorsEntry)) != RSSL_RET_END_OF_CONTAINER)
						{
							if (ret < RSSL_RET_SUCCESS)
							{
								rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
										"Error decoding element entry in Request Msg -- %d.", ret);
								return ret;
							}
							else
							{
								if (rsslBufferIsEqual(&behaviorsEntry.name, &RSSL_ENAME_DATA_STREAMS))
								{
									if (behaviorsEntry.dataType != RSSL_DT_UINT)
									{
										rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
												"Error decoding Symbol List Data Streams -- Element has wrong data type.");
										return ret;
									}

									if ((ret = rsslDecodeUInt(&dIter, pSymbolListFlags)) != RSSL_RET_SUCCESS)
									{
										rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
												"Error decoding Symbol List Data Streams -- %d.", ret);
										return ret;
									}

									if (((*pSymbolListFlags) & (RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS))
											== (RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS))
									{
										rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
												"Symbol list request specifies both data streams and data snapshots.");
										return RSSL_RET_INVALID_DATA;
									}

									return RSSL_RET_SUCCESS;
								}
							}
						}
					}
					else
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Error decoding Symbol List Behaviors Element List -- %d.", ret);
						return ret;
					}

					return RSSL_RET_SUCCESS;
				}
			}
		}
	}
	else
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__,
				"Unexpected container type: -- %d, or decoding error -- %d.", pRequestMsg->msgBase.containerType, ret);
		return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlSymbolListRequestInit(WlSymbolListRequest *pSymbolListRequest, WlBase *pBase, 
		WlItems *pItems, WlItemRequestCreateOpts *pOpts, RsslBool hasSymbolListBehaviors,
		RsslUInt slDataStreamFlags, RsslErrorInfo *pErrorInfo)
{
	memset(pSymbolListRequest, 0, sizeof(WlSymbolListRequest));

	pSymbolListRequest->hasBehaviors = hasSymbolListBehaviors;
	pSymbolListRequest->flags = slDataStreamFlags;
	return wlItemRequestInit(&pSymbolListRequest->itemBase, pBase, pItems, pOpts, pErrorInfo);
}

RsslRet wlSymbolListRequestCreate(WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo)
{
	WlSymbolListRequest *pSymbolListRequest;
	RsslRet ret;
	RsslUInt slDataStreamFlags = 0;
	RsslBool hasSymbolListBehaviors = RSSL_FALSE;

	if ((ret = wlExtractSymbolListFromMsg(pBase, pOpts->pRequestMsg, pOpts->majorVersion,
					pOpts->minorVersion, &hasSymbolListBehaviors, &slDataStreamFlags, pErrorInfo))
			!= RSSL_RET_SUCCESS)
		return ret;

	if (!(pSymbolListRequest = (WlSymbolListRequest*)rsslMemoryPoolGet(&pBase->requestPool, pErrorInfo)))
		return pErrorInfo->rsslError.rsslErrorId;

	if ((ret = wlSymbolListRequestInit(pSymbolListRequest, pBase, pItems, pOpts, 
					hasSymbolListBehaviors, slDataStreamFlags, pErrorInfo))
			!= RSSL_RET_SUCCESS)
	{
		rsslMemoryPoolPut(&pBase->requestPool, pSymbolListRequest);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet wlSymbolListRequestReissue(WlBase *pBase, WlItems *pItems, WlSymbolListRequest *pExistingRequest,
		WlItemRequestCreateOpts *pCreateOpts, RsslErrorInfo *pErrorInfo)
{
	RsslRet ret;
	RsslUInt slDataStreamFlags = 0;
	RsslBool hasSymbolListBehaviors = RSSL_FALSE;

	if ((ret = wlExtractSymbolListFromMsg(pBase, pCreateOpts->pRequestMsg, pCreateOpts->majorVersion,
					pCreateOpts->minorVersion, &hasSymbolListBehaviors, &pCreateOpts->slDataStreamFlags, pErrorInfo))
			!= RSSL_RET_SUCCESS)
		return ret;

	pExistingRequest->flags = slDataStreamFlags;

	return wlItemRequestReissue(pBase, pItems, &pExistingRequest->itemBase, pCreateOpts, pErrorInfo);
}


void wlSymbolListRequestDestroy(WlBase *pBase, WlItems *pItems, WlSymbolListRequest *pSymbolListRequest)
{
	wlItemRequestCleanup(&pSymbolListRequest->itemBase);
	free(pSymbolListRequest);
}

RsslRet wlProcessSymbolListMsg(WlBase *pBase, WlItems *pItems, RsslMsg *pRsslMsg,
		WlSymbolListRequest *pRequest, RsslErrorInfo *pErrorInfo)
{
	if (!(pRequest->flags & (RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS)))
		return RSSL_RET_SUCCESS;

	switch(pRsslMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
		case RSSL_MC_UPDATE:
		{
			RsslDecodeIterator dIter;
			RsslRet ret;
			RsslMap map;
			RsslMapEntry mapEntry;
			RsslRequestMsg requestMsg;
			WlItemRequest matchRequest;
			RsslQos itemQos;
			WlService *pWlService = pRequest->itemBase.pRequestedService->pMatchingService;
			RDMCachedService *pCachedService = pWlService->pService;

			/* Data streams are requested by service name, so need the service's name. */
			if (!pCachedService->hasServiceName)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
						"Service for Symbol List stream is missing name. Cannot create data streams.");
				return RSSL_RET_FAILURE;
			}

			if (pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
				return RSSL_RET_SUCCESS;

			rsslClearDecodeIterator(&dIter);
			rsslSetDecodeIteratorRWFVersion(&dIter, pBase->pRsslChannel->majorVersion,
					pBase->pRsslChannel->minorVersion);
			rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

			rsslClearRequestMsg(&requestMsg);

			requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
			requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
			requestMsg.flags = RSSL_RQMF_HAS_QOS;

			/* Use best QoS available from service. */
			if (pWlService->pService->rdm.info.qosCount)
				itemQos = pWlService->pService->rdm.info.qosList[0];
			else
			{
				rsslClearQos(&itemQos);
				itemQos.timeliness = RSSL_QOS_TIME_REALTIME;
				itemQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
			}

			if (pRequest->flags & RDM_SYMBOL_LIST_DATA_STREAMS)
				requestMsg.flags |= RSSL_RQMF_STREAMING;

			/* Create a request structure for searching the table of open provider streams
			 * If we find a match, we don't want to request the item again. */
			rsslClearQos(&matchRequest.qos);
			matchRequest.requestMsgFlags = RSSL_RQMF_HAS_QOS;
			matchRequest.qos = itemQos;
			rsslClearQos(&requestMsg.qos);
			requestMsg.qos = itemQos;

			matchRequest.base.domainType = RSSL_DMT_MARKET_PRICE;
			rsslClearMsgKey(&matchRequest.msgKey);
			matchRequest.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
			matchRequest.msgKey.serviceId = (RsslUInt16)pWlService->pService->rdm.serviceId;

			if (pRequest->itemBase.pRequestedService->flags & WL_RSVC_HAS_NAME)
			{
				matchRequest.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;
				matchRequest.pRequestedService = pRequest->itemBase.pRequestedService;
			}

			if ((ret = rsslDecodeMap(&dIter, &map)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, 
						"Error decoding symbol list map -- %d.", ret);
				return ret;
			}

			if (map.keyPrimitiveType != RSSL_DT_BUFFER
					&& map.keyPrimitiveType != RSSL_DT_ASCII_STRING
					&& map.keyPrimitiveType != RSSL_DT_RMTES_STRING)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
						"Incorrect Symbol List map key type %u.", map.keyPrimitiveType);
				return RSSL_RET_INVALID_DATA;
			}

			while ((ret = rsslDecodeMapEntry(&dIter, &mapEntry, &matchRequest.msgKey.name)) !=
					RSSL_RET_END_OF_CONTAINER)
			{
				if (ret != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__, 
							"Error decoding symbol list map entry -- %d.", ret);
					return ret;
				}

				switch(mapEntry.action)
				{
					case RSSL_MPEA_ADD_ENTRY:
					case RSSL_MPEA_UPDATE_ENTRY:
					{
						WlItemRequestCreateOpts opts;
						RsslUInt32 hashSum;
						RsslHashLink *pHashLink;

						hashSum = wlProviderRequestHashSum(&matchRequest);

						/* If streaming items were requested, check that a provider-driven
						 * stream is not already open before requesting. */
						if (pRequest->flags & RDM_SYMBOL_LIST_DATA_STREAMS)
						{
							pHashLink = rsslHashTableFind(&pItems->providerRequestsByAttrib,
									(void*)&matchRequest, &hashSum);

							if (pHashLink)
								continue;

						}

						requestMsg.msgBase.streamId = wlBaseTakeProviderStreamId(pBase);

						requestMsg.msgBase.msgKey = matchRequest.msgKey;
						requestMsg.msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

						wlClearItemRequestCreateOptions(&opts);
						opts.pRequestMsg = &requestMsg;
						opts.pUserSpec = NULL;
						opts.slDataStreamFlags = pRequest->flags;
						opts.majorVersion = pBase->pRsslChannel->majorVersion;
						opts.minorVersion = pBase->pRsslChannel->minorVersion;
						opts.pServiceName = &pCachedService->rdm.info.serviceName;


						if ((ret = wlItemRequestCreate(pBase, pItems, &opts, pErrorInfo))
									!= RSSL_RET_SUCCESS)
							return ret;

						break;
					}

					case RSSL_MPEA_DELETE_ENTRY:
						/* No action (delete action does not close streams). */
						break;
					default:
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_DATA, __FILE__, __LINE__, 
								"Unknown mapEntry action %u.", mapEntry.action);
						return RSSL_RET_INVALID_DATA;
						break;
				}
			}

			return RSSL_RET_SUCCESS;
		}

		default:
			return RSSL_RET_SUCCESS;
	}
}
