/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RSSL_QCONS_CHANNEL_COMMAND_H
#define _RSSL_QCONS_CHANNEL_COMMAND_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslRDMMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif


#define MAX_BUFFER_LENGTH 128
#define MAX_NUM_CAPABILITIES 15
#define MAX_DEST_NAMES 10

/*
 * Contains information associated with the connection 
 */
typedef struct
{
	RsslReactorChannel *reactorChannel;
	RsslReactorConnectOptions cOpts;
	RsslReactorConnectInfo cInfo;
	RsslReactorChannelRole *pRole;

	char hostName[MAX_BUFFER_LENGTH];
	char port[MAX_BUFFER_LENGTH];
	char interfaceName[MAX_BUFFER_LENGTH];
	
	char loginRefreshMemoryArray[4000];
	RsslBuffer loginRefreshMemory;
	RsslRDMLoginRefresh loginRefresh;

	/* service name requested by application */
	char serviceName[MAX_BUFFER_LENGTH];
	/* service id associated with the service name requested by application */
	RsslUInt serviceId;
	/* service name found flag */
	RsslBool reactorChannelReady;
	RsslBool itemsRequested;
	RsslBool isServiceReady;

	/* Service information for queue messaging. */
	RsslBool queueServiceNameFound;
	char queueServiceName[MAX_BUFFER_LENGTH];
	RsslUInt queueServiceId;
	RsslBool isQueueServiceUp;
	RsslBool queueServiceSupportsMessaging;
	RsslBool isQueueStreamOpen;
	RsslBool tunnelStreamOpenRequested;
	RsslBool waitFinalStatusEvent;
	RsslUInt8 tunnelStreamDomain;
	RsslBool useAuthentication;



	RsslUInt			capabilities[MAX_NUM_CAPABILITIES];
	RsslUInt32			capabilitiesCount;
	char* channelCommandArray;
	RsslBuffer channelCommandBuffer;

	/* For queue messaging. */
	time_t nextQueueMsgTime;
	RsslTunnelStream* pHandlerTunnelStream;
	RsslBuffer sourceName;
	RsslBuffer destNames[MAX_DEST_NAMES];
	RsslUInt32 destNameCount;

} ChannelStorage;


/*
 * Initializes the ChannelCommand
 */ 
RTR_C_INLINE void initChannelStorage(ChannelStorage *pCommand)
{
	int i;
	memset(pCommand, 0, sizeof(ChannelStorage));
	rsslClearReactorConnectOptions(&pCommand->cOpts);
	rsslClearReactorConnectInfo(&pCommand->cInfo);
	rsslClearRDMLoginRefresh(&pCommand->loginRefresh);
	pCommand->loginRefreshMemory.data = pCommand->loginRefreshMemoryArray;
	pCommand->loginRefreshMemory.length = sizeof(pCommand->loginRefreshMemoryArray);

	pCommand->channelCommandArray = (char*)malloc(4096*sizeof(char));
	pCommand->channelCommandBuffer.data = pCommand->channelCommandArray;
	pCommand->channelCommandBuffer.length = 4096;

	pCommand->queueServiceNameFound = RSSL_FALSE;
	pCommand->reactorChannelReady = RSSL_FALSE;
	pCommand->itemsRequested = RSSL_FALSE;
	pCommand->isServiceReady = RSSL_FALSE;
	pCommand->tunnelStreamDomain = RSSL_DMT_SYSTEM;
	pCommand->useAuthentication = RSSL_FALSE;

	for (i = 0; i < MAX_NUM_CAPABILITIES; i++)
	{
		pCommand->capabilities[i] = 0;
	}
	pCommand->capabilitiesCount = 0;

	pCommand->pHandlerTunnelStream = NULL;
	pCommand->destNameCount = 0;
	snprintf(pCommand->queueServiceName, sizeof(pCommand->queueServiceName), "");
	pCommand->nextQueueMsgTime = 0;
	pCommand->queueServiceId = 0;
	pCommand->isQueueServiceUp = RSSL_FALSE;
	pCommand->queueServiceSupportsMessaging = RSSL_FALSE;
	pCommand->isQueueStreamOpen = RSSL_FALSE;
	pCommand->tunnelStreamOpenRequested = RSSL_FALSE;
	pCommand->waitFinalStatusEvent = RSSL_FALSE;
}

/*
 * Clears the ChannelCommand
 */ 
RTR_C_INLINE void clearChannelStorage(ChannelStorage *pCommand)
{
	int i;
	pCommand->queueServiceNameFound = RSSL_FALSE;
	pCommand->reactorChannelReady = RSSL_FALSE;
	pCommand->itemsRequested = RSSL_FALSE;
	pCommand->isServiceReady = RSSL_FALSE;

	for (i = 0; i < MAX_NUM_CAPABILITIES; i++)
	{
		pCommand->capabilities[i] = 0;
	}
	pCommand->capabilitiesCount = 0;

	pCommand->pHandlerTunnelStream = NULL;
	pCommand->nextQueueMsgTime = 0;
	pCommand->queueServiceId = 0;
	pCommand->isQueueServiceUp = RSSL_FALSE;
	pCommand->queueServiceSupportsMessaging = RSSL_FALSE;
	pCommand->isQueueStreamOpen = RSSL_FALSE;
	pCommand->tunnelStreamOpenRequested = RSSL_FALSE;
}





#ifdef __cplusplus
};
#endif

#endif
