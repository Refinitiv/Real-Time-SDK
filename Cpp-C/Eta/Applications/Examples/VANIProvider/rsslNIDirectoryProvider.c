/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rsslNIDirectoryProvider.h"
#include "rsslVASendMessage.h"
#include "rsslNIChannelCommand.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"

RsslRet setupDirectoryResponseMsg(NIChannelCommand* chnlCommand, RsslInt32 streamId)
{
	RsslRDMDirectoryRefresh *pRefresh = &chnlCommand->directoryRefresh;
	
	rsslClearRDMDirectoryRefresh(pRefresh);
	
	chnlCommand->capabilitiesList[0] = RSSL_DMT_DICTIONARY;
	chnlCommand->capabilitiesList[1] = RSSL_DMT_MARKET_PRICE;
	chnlCommand->capabilitiesList[2] = RSSL_DMT_MARKET_BY_ORDER;
	rsslClearQos(&chnlCommand->qualityOfService);
	chnlCommand->qualityOfService.timeliness = RSSL_QOS_TIME_REALTIME;
	chnlCommand->qualityOfService.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	
	pRefresh->rdmMsgBase.streamId = streamId;
	
	pRefresh->state.streamState = RSSL_STREAM_OPEN;
	pRefresh->state.dataState = RSSL_DATA_OK;
	
	pRefresh->serviceList = &chnlCommand->service;
	pRefresh->filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER;
	pRefresh->serviceCount = 1;
	
	chnlCommand->service.flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
	chnlCommand->service.action = RSSL_MPEA_ADD_ENTRY;
	chnlCommand->service.serviceId = chnlCommand->serviceId;
	
	chnlCommand->service.info.serviceName = chnlCommand->serviceName;
	chnlCommand->service.info.vendor = chnlCommand->vendorName;
	chnlCommand->service.info.flags |= RDM_SVC_IFF_HAS_VENDOR;
	
	chnlCommand->service.info.capabilitiesCount = 3;
	chnlCommand->service.info.capabilitiesList = chnlCommand->capabilitiesList;
	
	chnlCommand->service.info.qosCount = 1;
	chnlCommand->service.info.qosList = &chnlCommand->qualityOfService;
	chnlCommand->service.info.flags |= RDM_SVC_IFF_HAS_QOS;
	
	chnlCommand->service.state.serviceState = 1;
	chnlCommand->service.state.acceptingRequests = 0;
	
	return RSSL_RET_SUCCESS;
}
