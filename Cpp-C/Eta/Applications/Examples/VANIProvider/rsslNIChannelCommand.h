/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_NICHANNEL_COMMAND_H
#define _RTR_RSSL_NICHANNEL_COMMAND_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslRDMMsg.h"
#include "rsslVANIMarketByOrderItems.h"
#include "rsslVANIMarketPriceItems.h"
#include "rsslNIItemInfo.h"
#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif



#define CHAN_CMD_MAX_ITEMS 5
#define MAX_BUFFER_LENGTH 128
#define LOGIN_ARRAY_LENGTH 2048
#define MAX_AUTHN_LENGTH 1024

/* Used to tell each channel what to request through the VA example */
typedef struct
{
	RsslReactorChannel *reactorChannel;
	RsslReactorConnectOptions cOpts;
	RsslReactorChannelRole *pRole;
	RsslUInt32 marketPriceItemCount;
	RsslUInt32 marketByOrderItemCount;
	RsslBool startWrite;
	RsslNIItemInfo marketPriceItemInfo[CHAN_CMD_MAX_ITEMS];
	RsslNIItemInfo marketByOrderItemInfo[CHAN_CMD_MAX_ITEMS];
	RsslMarketPriceItem marketPriceItems[CHAN_CMD_MAX_ITEMS];
	RsslMarketByOrderItem marketByOrderItems[CHAN_CMD_MAX_ITEMS];
	char* loginRefreshArray;
	char* channelCommandArray;
	RsslDataDictionary *dictionary;


	RsslBool reconnect;
	int timeToReconnect;
	
	RsslBuffer hostName;
	RsslBuffer port;
	RsslBuffer interfaceName;
	RsslBuffer recvHostName;
	RsslBuffer recvPort;
	RsslBuffer unicastServiceName;
	
	RsslBuffer username;
	RsslBuffer authenticationToken;
	RsslBuffer authenticationExtended;
	RsslBuffer applicationId;
	RsslBuffer password;
	RsslBuffer instanceId;
	RsslBuffer position;
	
	
	RsslBuffer serviceName;
	RsslUInt16 serviceId;
	RsslBuffer vendorName;
	RsslUInt capabilitiesList[3];
	RsslQos qualityOfService;
	
	
	/* Memory buffer for login */
	RsslBuffer channelCommandBuffer;
	RsslBuffer loginRefreshBuffer;
	
	RsslRDMLoginRequest loginRequest;
	RsslRDMLoginRefresh loginRefresh;
	RsslInt32 loginStreamId;

	/* source directory response information */
	RsslRDMService 		service;
	RsslRDMDirectoryRefresh directoryRefresh;
	/* service id associated with the service name requested by application */

	/* For TREP authentication login reissue */
	RsslUInt loginReissueTime; // represented by epoch time in seconds
	RsslBool canSendLoginReissue;
} NIChannelCommand;

RTR_C_INLINE void rsslInitNIChannelCommand(NIChannelCommand *pChannelCommand)
{
	int i;
	rsslClearReactorConnectOptions(&pChannelCommand->cOpts);
	pChannelCommand->cOpts.initializationTimeout = 30;
	
	pChannelCommand->marketPriceItemCount = 0;
	pChannelCommand->marketByOrderItemCount = 0;
	pChannelCommand->startWrite = RSSL_FALSE;
	
	for(i = 0; i < CHAN_CMD_MAX_ITEMS; i++)
	{
		clearNIItemInfo(&pChannelCommand->marketPriceItemInfo[i]);
		clearNIItemInfo(&pChannelCommand->marketByOrderItemInfo[i]);
		initMarketPriceItemFields(&pChannelCommand->marketPriceItems[i]);
		pChannelCommand->marketPriceItems[i].isInUse = RSSL_FALSE;
		initMarketByOrderItemFields(&pChannelCommand->marketByOrderItems[i]);
		pChannelCommand->marketByOrderItems[i].isInUse = RSSL_FALSE;
		
		pChannelCommand->marketPriceItemInfo[i].itemData = (void*)&pChannelCommand->marketPriceItems[i];
		pChannelCommand->marketByOrderItemInfo[i].itemData = (void*)&pChannelCommand->marketByOrderItems[i];
	}
	pChannelCommand->loginRefreshArray = (char*)malloc(LOGIN_ARRAY_LENGTH*sizeof(char));
	pChannelCommand->loginRefreshBuffer.data = pChannelCommand->loginRefreshArray;
	pChannelCommand->loginRefreshBuffer.length = LOGIN_ARRAY_LENGTH;
	
	pChannelCommand->channelCommandArray = (char*)malloc(6144*sizeof(char));
	pChannelCommand->channelCommandBuffer.data = pChannelCommand->channelCommandArray;
	pChannelCommand->channelCommandBuffer.length = 6144;
	
	pChannelCommand->reconnect = RSSL_FALSE;
	pChannelCommand->timeToReconnect = 0;
	
	pChannelCommand->hostName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->hostName.length = 0;
	
	pChannelCommand->port.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->port.length = 0;
	
	pChannelCommand->interfaceName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->interfaceName.length = 0;
	
	pChannelCommand->recvHostName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->recvHostName.length = 0;
	
	pChannelCommand->recvPort.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->recvPort.length = 0;
	
	pChannelCommand->username.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->username.length = 0;
	
	pChannelCommand->unicastServiceName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->unicastServiceName.length = 0;
	
	pChannelCommand->authenticationToken.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_AUTHN_LENGTH);
	pChannelCommand->authenticationToken.length = 0;
	
	pChannelCommand->authenticationExtended.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_AUTHN_LENGTH);
	pChannelCommand->authenticationExtended.length = 0;
	
	pChannelCommand->applicationId.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->applicationId.length = 0;
	
	pChannelCommand->password.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->password.length = 0;
	
	pChannelCommand->position.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->position.length = 0;
	
	pChannelCommand->serviceName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->serviceName.length = 0;
	
	pChannelCommand->vendorName.data = (char*)rsslReserveBufferMemory(&pChannelCommand->channelCommandBuffer, 1, MAX_BUFFER_LENGTH);
	pChannelCommand->vendorName.length = 0;
	
	rsslClearRDMLoginRequest(&pChannelCommand->loginRequest);
	
	rsslClearRDMLoginRefresh(&pChannelCommand->loginRefresh);
	pChannelCommand->loginStreamId = 1;
	
	rsslClearRDMService(&pChannelCommand->service);
	rsslClearRDMDirectoryRefresh(&pChannelCommand->directoryRefresh);
	pChannelCommand->serviceId = 0;
}

RTR_C_INLINE void rsslCleanupNIChannelCommand(NIChannelCommand *pChannelCommand)
{
	rsslDeleteDataDictionary(pChannelCommand->dictionary);
	free(pChannelCommand->channelCommandArray);
	free(pChannelCommand->loginRefreshArray);
}

RTR_C_INLINE void rsslClearNIChannelCommand(NIChannelCommand *pChannelCommand)
{
	rsslClearConnectOpts(&pChannelCommand->cOpts.rsslConnectOptions);
		
	pChannelCommand->loginRefreshBuffer.data = pChannelCommand->loginRefreshArray;
	pChannelCommand->loginRefreshBuffer.length = 2048;
	
	pChannelCommand->reconnect = RSSL_FALSE;
	pChannelCommand->timeToReconnect = 0;
	
	pChannelCommand->hostName.length = 0;
	
	pChannelCommand->recvHostName.length = 0;
	pChannelCommand->recvPort.length= 0;
	
	pChannelCommand->port.length = 0;
	
	pChannelCommand->username.length = 0;
	
	pChannelCommand->applicationId.length = 0;
	
	pChannelCommand->instanceId.length = 0;
	
	rsslClearRDMLoginRequest(&pChannelCommand->loginRequest);
	
	rsslClearRDMLoginMsg((RsslRDMLoginMsg*)&pChannelCommand->loginRefresh);
	pChannelCommand->loginStreamId = 1;
	
	rsslClearRDMService(&pChannelCommand->service);
	rsslClearRDMDirectoryMsg((RsslRDMDirectoryMsg*)&pChannelCommand->directoryRefresh);
	pChannelCommand->serviceId = 0;
}





#ifdef __cplusplus
};
#endif

#endif
