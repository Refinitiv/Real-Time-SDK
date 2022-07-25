/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2021-2022 Refinitiv. All rights reserved.
*/

/*
 * This is the main file for the rsslWatchlistConsumer application.  It is a single-threaded
 * client application that utilizes the ETA Reactor's watchlist to provide recovery of data.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the ETA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application makes use of the RDM package for easier decoding of Login & Source Directory
 * messages.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include "rsslMultiCredWLConsumerConfig.h"
#include "itemDecoder.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslMessagePackage.h"
#include <time.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <stdlib.h>

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

static RsslReactorChannel *pConsumerChannel = NULL;
static RsslBool itemsRequested = RSSL_FALSE;
static RsslBool isConsumerChannelUp = RSSL_FALSE;

RsslBool runTimeExpired = RSSL_FALSE;
RsslSocket socketIdList[2] = { 0, 0 };
RsslUInt32 socketIdListCount = 0;

RsslUInt foundServiceId = 0;
RsslBool isServiceFound = RSSL_FALSE;

/* For UserAuthn authentication login reissue */
static RsslUInt loginReissueTime; // represented by epoch time in seconds
static RsslBool canSendLoginReissue;

extern RsslDataDictionary dictionary;

int main(int argc, char **argv)
{
	RsslReactor							*pReactor;

	RsslRet								ret;
	RsslCreateReactorOptions			reactorOpts;
	RsslErrorInfo						rsslErrorInfo;
	RsslReactorJsonConverterOptions		jsonConverterOptions;

	RsslRDMDirectoryRequest				dirRequest;
	RsslRDMLoginRequest					loginRequest;

	time_t								stopTime;
	time_t								currentTime;

	RsslBool							postWithMsg = RSSL_TRUE;

	watchlistConsumerConfigInit(argc, argv);

	itemDecoderInit();

	stopTime = time(NULL);

	if (stopTime < 0)
	{
		printf("time() failed.\n");
		clearAllocatedMemory();
		exit(-1);
	}

	stopTime += watchlistConsumerConfig.runTime;


	/* Setup consumer role for connection. */
	watchlistConsumerConfig.consumerRole.base.channelEventCallback = channelEventCallback;
	watchlistConsumerConfig.consumerRole.base.defaultMsgCallback = msgCallback;
	watchlistConsumerConfig.consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	watchlistConsumerConfig.consumerRole.watchlistOptions.channelOpenCallback = channelOpenCallback;

	/* Prepare a default login request(Use 1 as the Login Stream ID).
	* This function sets login request parameters according to what a consumer
	* application would normally set. This will only be set if there isn't any other request set. */
	
	if (watchlistConsumerConfig.consumerRole.pLoginRequestList == NULL)
	{
		if (rsslInitDefaultRDMLoginRequest(&loginRequest, LOGIN_STREAM_ID) != RSSL_RET_SUCCESS)
		{
			printf("rsslInitDefaultRDMLoginRequest() failed\n");
			exit(-1);
		}
		watchlistConsumerConfig.consumerRole.pLoginRequest = &loginRequest;
	}

	watchlistConsumerConfig.consumerRole.loginMsgCallback = loginMsgCallback;

	/* Prepare a default directory request. */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, DIRECTORY_STREAM_ID) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		clearAllocatedMemory();
		exit(-1);
	}

	watchlistConsumerConfig.consumerRole.pDirectoryRequest = &dirRequest;
	watchlistConsumerConfig.consumerRole.directoryMsgCallback = directoryMsgCallback;
	watchlistConsumerConfig.consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;


	/* Create Reactor. */
	rsslClearCreateReactorOptions(&reactorOpts);
	
	if (watchlistConsumerConfig.tokenURLV1.data != NULL)
	{
		reactorOpts.tokenServiceURL_V1 = watchlistConsumerConfig.tokenURLV1;
	}

	if (watchlistConsumerConfig.tokenURLV2.data != NULL)
	{
		reactorOpts.tokenServiceURL_V2 = watchlistConsumerConfig.tokenURLV2;
	}

	if (watchlistConsumerConfig.serviceDiscoveryURL.data != NULL)
	{
		reactorOpts.serviceDiscoveryURL = watchlistConsumerConfig.serviceDiscoveryURL;
	}

	if (watchlistConsumerConfig.restEnableLog || watchlistConsumerConfig.restEnableLogCallback)
	{
		reactorOpts.restEnableLog = watchlistConsumerConfig.restEnableLog;
		reactorOpts.restLogOutputStream = watchlistConsumerConfig.restOutputStreamName;
		if (watchlistConsumerConfig.restEnableLogCallback)
		{
			reactorOpts.pRestLoggingCallback = restLoggingCallback;
		}
	}

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);

	watchlistConsumerConfig.connectionOpts.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	watchlistConsumerConfig.connectionOpts.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	watchlistConsumerConfig.connectionOpts.reconnectAttemptLimit = watchlistConsumerConfig.reconnectLimit;
	watchlistConsumerConfig.connectionOpts.reconnectMinDelay = 500;
	watchlistConsumerConfig.connectionOpts.reconnectMaxDelay = 3000;

	/* Connect. */
	if ((ret = rsslReactorConnect(pReactor, &watchlistConsumerConfig.connectionOpts,
					(RsslReactorChannelRole*)&watchlistConsumerConfig.consumerRole, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorConnect() failed: %d(%s)\n", 
				ret, rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	jsonConverterOptions.pDictionary = &dictionary;
	jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdCallback;
	jsonConverterOptions.pJsonConversionEventCallback = jsonConversionEventCallback;

	if (rsslReactorInitJsonConverter(pReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error initializing RWF/JSON Converter: %s\n", rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	/* Dispatch until application stops. */
	do
	{
		struct timeval 				selectTime;
		fd_set						readFds;
		fd_set						exceptFds;
		RsslReactorDispatchOptions	dispatchOpts;
		RsslUInt32					index;

		FD_ZERO(&readFds);
		FD_ZERO(&exceptFds);
		FD_SET(pReactor->eventFd, &readFds);
		FD_SET(pReactor->eventFd, &exceptFds);

		if (pConsumerChannel)
		{
			if (pConsumerChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				if (pConsumerChannel->pRsslChannel && pConsumerChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
				{
					FD_SET(pConsumerChannel->socketId, &readFds);
					FD_SET(pConsumerChannel->socketId, &exceptFds);
				}
			}
			else if (pConsumerChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				for (index = 0; index < socketIdListCount; index++)
				{
					FD_SET(socketIdList[index], &readFds);
					FD_SET(socketIdList[index], &exceptFds);
				}
			}
		}

		selectTime.tv_sec = 1; selectTime.tv_usec = 0;
		ret = select(FD_SETSIZE, &readFds, NULL, &exceptFds, &selectTime);

		if (ret < 0)
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;

			printf("Error: select: %d\n", WSAGetLastError());
#else
			if (errno == EINTR)
				continue;

			perror("select");
#endif
			clearAllocatedMemory();
			exit(-1);
		}

		rsslClearReactorDispatchOptions(&dispatchOpts);
		
		/* Call rsslReactorDispatch().  This will handle any events that have occurred on its channels.
		 * If there are events or messages for the application to process, they will be delivered
		 * through the callback functions given by the consumerRole object. 
		 * A return value greater than RSSL_RET_SUCCESS indicates there may be more to process. */
		while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
			clearAllocatedMemory();
			exit(-1);
		}

		if ((currentTime = time(NULL)) < 0)
		{
			printf("time() failed.\n");
		}

		if (currentTime >= stopTime)
		{
			if (!runTimeExpired)
			{
				runTimeExpired = RSSL_TRUE;
				break;
			}
		}
	} while(ret >= RSSL_RET_SUCCESS);

	if (pConsumerChannel)
	{
		if ((ret = rsslReactorCloseChannel(pReactor, pConsumerChannel, &rsslErrorInfo))
				!= RSSL_RET_SUCCESS)
		{
			printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			clearAllocatedMemory();
			exit(-1);
		}
	}

	if ((ret = rsslDestroyReactor(pReactor, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	if (reactorOpts.restLogOutputStream)
		fclose(reactorOpts.restLogOutputStream);

	rsslUninitialize();

	itemDecoderCleanup();
	clearAllocatedMemory();
	
	exit(0);
}

/* Requests the desired items.  */
RsslRet requestItems(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslUInt32 itemListIndex;

	if (itemsRequested)
		return RSSL_RET_SUCCESS;

	itemsRequested = RSSL_TRUE;


	for(itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount; ++itemListIndex)
	{
		char dataBodyBuf[512];
		RsslBuffer dataBody = { 512, dataBodyBuf };

		rsslClearRequestMsg(&requestMsg);
		requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
		requestMsg.flags = RSSL_RQMF_STREAMING;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		requestMsg.msgBase.domainType =
			watchlistConsumerConfig.itemList[itemListIndex].domainType;

		rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

		submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
		submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;


		requestMsg.msgBase.msgKey.name = watchlistConsumerConfig.itemList[itemListIndex].name;
		requestMsg.msgBase.streamId = watchlistConsumerConfig.itemList[itemListIndex].streamId;
		if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
						&rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
					ret, rsslErrorInfo.rsslError.text);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/* Requests the field dictionary and enumerated types dictionary. */
RsslRet requestDictionaries(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;

	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = RDM_DICTIONARY_NORMAL;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
	submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;

	requestMsg.msgBase.msgKey.name = fieldDictionaryName;
	requestMsg.msgBase.streamId = FIELD_DICTIONARY_STREAM_ID;
	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	requestMsg.msgBase.msgKey.name = enumDictionaryName;
	requestMsg.msgBase.streamId = ENUM_DICTIONARY_STREAM_ID;
	if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorSubmitMsg() failed: %d(%s)\n",
				ret, rsslErrorInfo.rsslError.text);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}


/* Prints out information from an RsslState. */
static void printRsslState(const RsslState *pState)
{
	char stateString[256];
	RsslBuffer stateBuffer = { 256, stateString };

	if (rsslStateToString(&stateBuffer, (RsslState*)pState) != RSSL_RET_SUCCESS)
	{ printf("(rsslStateToString() failed)\n"); }

	printf("%.*s\n", stateBuffer.length, stateBuffer.data);
}

/* Prints information about a stream. */
static void printStreamInfo(RsslBuffer *pItemName, RsslUInt8 domainType, RsslInt32 streamId,
		const char *msgClass)
{
	if (pItemName && pItemName->data)
		printf(	"Name: %-20.*s   ", pItemName->length, pItemName->data);

	printf(	"Domain: %s\n", rsslDomainTypeToString(domainType));

	printf( "Stream: %-20d Msg Class: %s\n", streamId, msgClass);


}

/* Callback function for login message responses. */
static RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pMsgEvent)
{
	RsslRDMLoginMsg *pLoginMsg = pMsgEvent->pRDMLoginMsg;

	if (!pLoginMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Login message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REFRESH:
		{
			RsslRDMLoginRefresh *pLoginRefresh = &pLoginMsg->refresh;

			printStreamInfo(NULL, RSSL_DMT_LOGIN, pLoginMsg->rdmMsgBase.streamId, "RDM_LG_MT_REFRESH");
			printRsslState(&pLoginRefresh->state);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME)
				printf("  UserName: %.*s\n", pLoginRefresh->userName.length, pLoginRefresh->userName.data);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
				printf("  AuthenticationTTReissue: %llu\n", pLoginRefresh->authenticationTTReissue);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
				printf("  AuthenticationExtendedResp: %.*s\n", pLoginRefresh->authenticationExtendedResp.length, pLoginRefresh->authenticationExtendedResp.data);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginRefresh->authenticationErrorCode);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginRefresh->authenticationErrorText.length, pLoginRefresh->authenticationErrorText.data);

			break;
		}

		case RDM_LG_MT_STATUS:
		{
			RsslRDMLoginStatus *pLoginStatus = &pLoginMsg->status;

			printStreamInfo(NULL, RSSL_DMT_LOGIN, pLoginMsg->rdmMsgBase.streamId, "RDM_LG_MT_STATUS");

			if (pLoginStatus->flags & RDM_LG_STF_HAS_STATE)
				printRsslState(&pLoginStatus->state);

			if (pLoginStatus->flags & RDM_LG_STF_HAS_USERNAME)
				printf("  UserName: %.*s\n", pLoginStatus->userName.length, pLoginStatus->userName.data);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginStatus->authenticationErrorCode);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginStatus->authenticationErrorText.length, pLoginStatus->authenticationErrorText.data);

			break;
		}
		case RDM_LG_MT_RTT:
			printf("\nReceived Login RTT Msg\n");
			printf("	Ticks: %llu\n", pLoginMsg->RTT.ticks);
			if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_LATENCY)
				printf("	Last Latency: %llu\n", pLoginMsg->RTT.lastLatency);
			if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_TCP_RETRANS)
				printf("	TCP Retransmissions: %llu\n", pLoginMsg->RTT.tcpRetrans);
			if (pMsgEvent->flags & RSSL_RDM_LG_LME_RTT_RESPONSE_SENT)
				printf("RTT Response sent to provider.\n");
			break;

		default:
			printf("\n  Received Unhandled Login Msg Type: %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	printf("\n");

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for directory message responses. 
 * As many of the behaviors normally performed by the application in response to directory
 * messages(e.g. item recovery or status fanout) are automatically handled by the watchlist, 
 * these messages are primarily informational. */
static RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent* pMsgEvent)
{
	RsslUInt32 i;
	RsslRDMDirectoryMsg *pDirectoryMsg = pMsgEvent->pRDMDirectoryMsg;
	RsslRDMService *serviceList = NULL; 
	RsslUInt32 serviceCount = 0;

	if (!pDirectoryMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Directory message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REFRESH:
		{
			RsslRDMDirectoryRefresh *pDirectoryRefresh = &pDirectoryMsg->refresh;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_REFRESH");
			printRsslState(&pDirectoryRefresh->state);

			serviceList = pDirectoryRefresh->serviceList;
			serviceCount = pDirectoryRefresh->serviceCount;

			break;
		}
		case RDM_DR_MT_UPDATE:
		{
			RsslRDMDirectoryUpdate *pDirectoryUpdate = &pDirectoryMsg->update;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_UPDATE");

			serviceList = pDirectoryUpdate->serviceList;
			serviceCount = pDirectoryUpdate->serviceCount;

			break;
		}
		case RDM_DR_MT_STATUS:
		{
			RsslRDMDirectoryStatus *pDirectoryStatus = &pDirectoryMsg->status;

			printStreamInfo(NULL, RSSL_DMT_SOURCE, pDirectoryMsg->rdmMsgBase.streamId, "RDM_DR_MT_STATUS");

			if (pDirectoryStatus->flags & RDM_DR_STF_HAS_STATE)
				printRsslState(&pDirectoryMsg->status.state);

			break;
		}

		default:
			printf("\n  Received Unhandled Source Directory Msg Type: %d\n", pDirectoryMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	/* Refresh and update messages contain updates to service information. */
	if (serviceList)
	{
		for (i = 0; i <  serviceCount; ++i)
		{
			RsslRDMService *pService = &serviceList[i];
			char tmpBuf[128];

			/* Print service ID (and service name, if present). */
			if (pService->flags & RDM_SVCF_HAS_INFO)
			{
				snprintf(tmpBuf, 128, "%llu(%.*s)", pService->serviceId, 
						pService->info.serviceName.length, pService->info.serviceName.data);

			}
			else
				snprintf(tmpBuf, 128, "%llu", pService->serviceId);

			printf("  Service: %-20s Action: %s\n", tmpBuf, mapActionToString(pService->action));

			if (rsslBufferIsEqual(&watchlistConsumerConfig.serviceName, &pService->info.serviceName))
			{
				isServiceFound = RSSL_TRUE;
				foundServiceId = pService->serviceId;
			}
		}
	}

	printf("\n");


	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for dictionary message responses. */
static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDictionaryMsgEvent* pMsgEvent)
{
	RsslRDMDictionaryMsg *pDictionaryMsg = pMsgEvent->pRDMDictionaryMsg;

	if (!pDictionaryMsg)
	{
		RsslErrorInfo *pError = pMsgEvent->baseMsgEvent.pErrorInfo;
		printf("Dictionary message error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		return RSSL_RC_CRET_SUCCESS;
	}

	if (pMsgEvent->baseMsgEvent.pSeqNum || pMsgEvent->baseMsgEvent.pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->baseMsgEvent.pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->baseMsgEvent.pSeqNum);

		if (pMsgEvent->baseMsgEvent.pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->baseMsgEvent.pFTGroupId);
		printf("\n");
	}

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REFRESH:
		{
			RsslRDMDictionaryRefresh *pDictionaryRefresh = &pDictionaryMsg->refresh;

			printStreamInfo(&pDictionaryRefresh->dictionaryName,RSSL_DMT_DICTIONARY, 
					pDictionaryRefresh->rdmMsgBase.streamId, "RDM_DC_MT_REFRESH");

			printRsslState(&pDictionaryRefresh->state);

			decodeDictionaryDataBody(pChannel, pDictionaryRefresh);
			if (fieldDictionaryLoaded && enumDictionaryLoaded)
				requestItems(pReactor, pChannel);

			break;
		}

		case RDM_DC_MT_STATUS:
		{
			RsslRDMDictionaryStatus *pDictionaryStatus = &pDictionaryMsg->status;
			RsslBuffer *pDictionaryName = NULL;

			switch(pDictionaryMsg->rdmMsgBase.streamId)
			{
				case FIELD_DICTIONARY_STREAM_ID:
					pDictionaryName = &fieldDictionaryName; break;
				case ENUM_DICTIONARY_STREAM_ID:
					pDictionaryName = &enumDictionaryName; break;
				default:
					printf("Error: Received dictionary status message on unknown stream %d\n",
							pDictionaryMsg->rdmMsgBase.streamId);
					return RSSL_RC_CRET_SUCCESS;

			}

			printStreamInfo(pDictionaryName, RSSL_DMT_DICTIONARY, 
					pDictionaryStatus->rdmMsgBase.streamId, "RDM_DC_MT_STATUS");

			if (pDictionaryStatus->flags & RDM_DC_STF_HAS_STATE)
				printRsslState(&pDictionaryStatus->state);

			break;
		}

		default:
			printf("\n  Received Unhandled Dictionary Msg Type: %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
			break;
	}

	printf("\n");

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback function for message responses. 
 * Messages regarding items opened by the application are handled here. 
 *
 **/
static RsslReactorCallbackRet msgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent)
{
	const RsslState *pState = NULL;
	RsslMsg *pRsslMsg = pMsgEvent->pRsslMsg;
	RsslBuffer *pItemName = NULL;
	ItemInfo *pItem = NULL;

	if (!pRsslMsg)
	{
		printf("Message Error.\n");
		if (pMsgEvent->pErrorInfo)
			printf("	Error text: %s\n\n", pMsgEvent->pErrorInfo->rsslError.text);
		return RSSL_RC_CRET_SUCCESS;
	}

	/* Get stored item information associated with this stream, if any. 
	 * This may be used to print out item information such as the name. */
	pItem = getItemInfo(pRsslMsg->msgBase.streamId);

	if (pMsgEvent->pSeqNum || pMsgEvent->pFTGroupId)
	{
		/* Sequence number and/or FTGroup ID may be present when receiving multicast messages. */
		if (pMsgEvent->pSeqNum)
			printf("SeqNum: %-20u ", *pMsgEvent->pSeqNum);

		if (pMsgEvent->pFTGroupId)
			printf("FTGroupId: %u", *pMsgEvent->pFTGroupId);
		printf("\n");
	}

	switch(pRsslMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printRsslState(&pRsslMsg->refreshMsg.state);
			pState = &pRsslMsg->refreshMsg.state;


			/* Decode data body according to its domain. */
			decodeDataBody(pChannel, pRsslMsg);
			break;


		case RSSL_MC_UPDATE:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			/* Decode data body according to its domain. */
			decodeDataBody(pChannel, pRsslMsg);
			break;

		case RSSL_MC_STATUS:

			/* Get key, if present. Otherwise use our stored info when printing. */
			if (pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;

			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			if (pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				printRsslState(&pRsslMsg->statusMsg.state);
				pState = &pRsslMsg->statusMsg.state;
			}

			printf("\n");
			break;

		case RSSL_MC_ACK:

			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
			{
				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
					pItemName = &pRsslMsg->msgBase.msgKey.name;
			}
			else if (pItem)
				pItemName = &pItem->name;

			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printf("  ackId:   %u\n", pRsslMsg->ackMsg.ackId);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				printf("  seqNum:  %u\n", pRsslMsg->ackMsg.seqNum);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
				printf("  nakCode: %u\n", pRsslMsg->ackMsg.nakCode);
			if (pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				printf("  text:    %.*s\n", pRsslMsg->ackMsg.text.length, pRsslMsg->ackMsg.text.data);
			printf("\n");
			break;

		default:
			printStreamInfo(pItemName, pRsslMsg->msgBase.domainType, pRsslMsg->msgBase.streamId,
					rsslMsgClassToString(pRsslMsg->msgBase.msgClass));

			printf("  Error: Unhandled message class.\n\n");
			break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for when the channel is first opened by the application.
 * If dictionaries are loaded, items will immediately be requested.
 * Otherwise, dictionaries must be retrieved before requesting items. 
 * Note that at the channel is not yet up and should only be used
 * to open requests. */
RsslReactorCallbackRet channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	if (fieldDictionaryLoaded && enumDictionaryLoaded)
		requestItems(pReactor, pReactorChannel);
	else
		requestDictionaries(pReactor, pReactorChannel);

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for channel events. */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* Save the channel on our info structure. */
			pConsumerChannel = pReactorChannel;
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					socketIdList[index] = pConsumerChannel->pWarmStandbyChInfo->socketIdList[index];
				}

				socketIdListCount = pConsumerChannel->pWarmStandbyChInfo->socketIdCount;
			}

			printf("Channel "SOCKET_PRINT_TYPE" is up!\n\n", pReactorChannel->socketId);
			if (isXmlTracingEnabled() == RSSL_TRUE) 
			{
				RsslTraceOptions traceOptions;
				char traceOutputFile[128];
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				snprintf(traceOutputFile, 128, "rsslWatchlistConsumer\0");
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ | RSSL_TRACE_DUMP;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			isConsumerChannelUp = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
			return RSSL_RC_CRET_SUCCESS;
		case RSSL_RC_CET_FD_CHANGE:
		{
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;
				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					socketIdList[index] = pConsumerChannel->pWarmStandbyChInfo->socketIdList[index];
				}

				socketIdListCount = pConsumerChannel->pWarmStandbyChInfo->socketIdCount;
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			RsslErrorInfo rsslErrorInfo;

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
				printf("Channel "SOCKET_PRINT_TYPE" down.\n", pReactorChannel->socketId);
			else
				printf("Channel down.\n");

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			pConsumerChannel = NULL;
			rsslReactorCloseChannel(pReactor, pReactorChannel, &rsslErrorInfo);
			exit(-1);

			isConsumerChannelUp = RSSL_FALSE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			RsslChannel *pRsslChannel = pReactorChannel->pRsslChannel;
			char hostName[512];

			memset(hostName, 0, 512);

			if (pRsslChannel != NULL && pRsslChannel->hostname != NULL)
			{
				memcpy(hostName, pRsslChannel->hostname, strlen(pRsslChannel->hostname));
			}

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
				printf("Channel "SOCKET_PRINT_TYPE" down. Reconnecting hostname %s\n", pReactorChannel->socketId, hostName);
			else
				printf("Channel down. Reconnecting hostname %s\n",  hostName);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			socketIdListCount = 0;
			isConsumerChannelUp = RSSL_FALSE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("Unknown connection event!\n");
			clearAllocatedMemory();
			exit(-1);
		}
	}


	return RSSL_RC_CRET_SUCCESS;
}


RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorJsonConversionEvent* pEvent)
{
	if (pEvent->pError)
	{
		printf("Error Id: %d, Text: %s\n", pEvent->pError->rsslError.rsslErrorId, pEvent->pError->rsslError.text);
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslRet serviceNameToIdCallback(RsslReactor* pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	if (isServiceFound && rsslBufferIsEqual(&watchlistConsumerConfig.serviceName, pServiceName))
	{
		*pServiceId = (RsslUInt16)foundServiceId;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}


RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent)
{
	if (pLogEvent && pLogEvent->pRestLoggingMessage && pLogEvent->pRestLoggingMessage->data)
	{
		FILE* pOutputStream = watchlistConsumerConfig.restOutputStreamName;
		if (!pOutputStream)
			pOutputStream = stdout;

		fprintf(pOutputStream, "{restLoggingCallback}: %s", pLogEvent->pRestLoggingMessage->data);
		fflush(pOutputStream);
	}
	return RSSL_RC_CRET_SUCCESS;
}


/* This callback is to show how to present updated credentials to the reactor when called back. Please note that this is not intended for production use, as it 
   does not use a secure credential storage. Please use best practices for managing sensitive credential information. */
RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor* pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent)
{
	RsslReactorOAuthCredentialRenewalOptions renewalOptions;
	RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
	RsslErrorInfo rsslError;
	OAuthRequestCredential* pRequestCredential = (OAuthRequestCredential*)pOAuthCredentialEvent->userSpecPtr;
	rsslClearReactorOAuthCredentialRenewalOptions(&renewalOptions);

	/* We are not changing the password here. If the password needs to be updated for a V1 login, use RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE and set the newPassword buffer with the new password. 
	If the clientSecret has changed for a V2 credential, use RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD and provide the updated clientSecret. */
	renewalOptions.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;

	rsslClearReactorOAuthCredentialRenewal(&reactorOAuthCredentialRenewal);

	printf("Submitting OAuth credentials for OAuth credential name: %*s\n", pRequestCredential->credentialName.length, pRequestCredential->credentialName.data);


	/* For a V1 credential, only the password requires updating.  For a V2 credential, the clientSecret needs to be updated. */
	if (pRequestCredential->oAuthCredential.password.length != 0)
		reactorOAuthCredentialRenewal.password = pRequestCredential->oAuthCredential.password; /* Specified password as needed */
	else
		reactorOAuthCredentialRenewal.clientSecret = pRequestCredential->oAuthCredential.clientSecret;

	rsslReactorSubmitOAuthCredentialRenewal(pReactor, &renewalOptions, &reactorOAuthCredentialRenewal, &rsslError);

	return RSSL_RC_CRET_SUCCESS;
}


/* This callback is to show how to present updated login message to the reactor when called back. Please note that this is not intended for production use, as it
   does not provide an updated login message or manage any sensitive information. Please use best practices for managing sensitive credential information. */
RsslReactorCallbackRet loginMsgEventCallback(RsslReactor* pReactor, RsslReactorChannel* pChannel, RsslReactorLoginRenewalEvent* pLoginCredentialEvent)
{
	RsslErrorInfo rsslError;
	LoginRequestCredential* pRequestCredential = (LoginRequestCredential*)pLoginCredentialEvent->userSpecPtr;
	RsslReactorLoginCredentialRenewalOptions options;
	
	printf("Submitting Login credentials for Login credential name: %*s\n", pRequestCredential->loginName.length, pRequestCredential->loginName.data);

	/* Clear our options, and then set them on the channel */
	rsslClearReactorLoginCredentialRenewalOptions(&options);
	options.userName = pRequestCredential->requestMsgCredential.loginRequestMsg->userName;

	rsslReactorSubmitLoginCredentialRenewal(pReactor, pChannel, &options, &rsslError);

	return RSSL_RC_CRET_SUCCESS;
}
