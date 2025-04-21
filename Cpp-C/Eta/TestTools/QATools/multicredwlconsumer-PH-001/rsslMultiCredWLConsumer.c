/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2021-2025 LSEG. All rights reserved.
*/

/*
 * This is the main file for the rsslMultiCredWLConsumer application.  It is a single-threaded
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
#include <time.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#endif
#include <stdlib.h>
#include <sys/timeb.h>

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
static RsslBool loginChannelClosed = RSSL_FALSE;

RsslBool runTimeExpired = RSSL_FALSE;

fd_set readFds, exceptFds;

RsslUInt foundServiceId = 0;
RsslBool isServiceFound = RSSL_FALSE;

RsslBool prefHostPrintDetails = RSSL_TRUE;

/* For UserAuthn authentication login reissue */
static RsslUInt loginReissueTime; // represented by epoch time in seconds
static RsslBool canSendLoginReissue;

extern RsslDataDictionary dictionary;

RTR_C_INLINE RsslUInt32 dumpDateTime(char* buf, RsslUInt32 size)
{
	long hour = 0,
		min = 0,
		sec = 0,
		msec = 0;

	RsslUInt32 res = 0;

	struct tm stamptime;
	time_t currTime;
	currTime = time(NULL);

#if defined(WIN32)
	struct _timeb	_time;
	_ftime(&_time);
	sec = (long)(_time.time - 60 * (_time.timezone - _time.dstflag * 60));
	min = sec / 60 % 60;
	hour = sec / 3600 % 24;
	sec = sec % 60;
	msec = _time.millitm;

	/* get date from currtime */
	localtime_s(&stamptime, &currTime);
#elif defined(LINUX)
	/* localtime must be used to get the correct system time. */
	stamptime = *localtime_r(&currTime, &stamptime);
	sec = stamptime.tm_sec;
	min = stamptime.tm_min;
	hour = stamptime.tm_hour;

	/* localtime, however, does not give us msec. */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
#endif

	// yyyy-MM-dd HH:mm:ss.SSS
	res = snprintf(buf, size, "<!-- %4d-%02d-%02d %02ld:%02ld:%02ld.%03ld -->",
		(stamptime.tm_year + 1900),
		(stamptime.tm_mon + 1),
		stamptime.tm_mday,
		hour,
		min,
		sec,
		msec);
	return res;
}

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

	if (watchlistConsumerConfig.restProxyHost[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyHostName = watchlistConsumerConfig.restProxyHost;
	}

	if (watchlistConsumerConfig.restProxyPort[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPort = watchlistConsumerConfig.restProxyPort;
	}

	if (watchlistConsumerConfig.restProxyUserName[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyUserName = watchlistConsumerConfig.restProxyUserName;
	}

	if (watchlistConsumerConfig.restProxyPasswd[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPasswd = watchlistConsumerConfig.restProxyPasswd;
	}

	if (watchlistConsumerConfig.restProxyDomain[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyDomain = watchlistConsumerConfig.restProxyDomain;
	}

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pReactor->eventFd, &readFds);
	FD_SET(pReactor->eventFd, &exceptFds);

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);

	watchlistConsumerConfig.connectionOpts.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	watchlistConsumerConfig.connectionOpts.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	watchlistConsumerConfig.connectionOpts.reconnectAttemptLimit = watchlistConsumerConfig.reconnectAttemptLimit;
	watchlistConsumerConfig.connectionOpts.reconnectMinDelay = watchlistConsumerConfig.reconnectMinDelay;
	watchlistConsumerConfig.connectionOpts.reconnectMaxDelay = watchlistConsumerConfig.reconnectMaxDelay;

	// Set preferred host options.
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.enablePreferredHostOptions = preferredHostConfig.preferredHostOptions.enablePreferredHostOptions;
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.detectionTimeInterval = preferredHostConfig.preferredHostOptions.detectionTimeInterval;
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.detectionTimeSchedule = preferredHostConfig.preferredHostOptions.detectionTimeSchedule;
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.connectionListIndex = preferredHostConfig.preferredHostOptions.connectionListIndex;
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.warmStandbyGroupListIndex = preferredHostConfig.preferredHostOptions.warmStandbyGroupListIndex;
	watchlistConsumerConfig.connectionOpts.preferredHostOptions.fallBackWithInWSBGroup = preferredHostConfig.preferredHostOptions.fallBackWithInWSBGroup;

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
		RsslReactorDispatchOptions	dispatchOpts;
		fd_set useReadFds = readFds, useExceptFds = exceptFds;

		selectTime.tv_sec = 1; selectTime.tv_usec = 0;
		ret = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);

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

		/* Preferred Host. Check timeout and Initiate fallback direct call. */
		ret = handlePreferredHostRuntime(&rsslErrorInfo);
		if (ret < RSSL_RET_SUCCESS)
		{
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

		if (currentTime >= stopTime || loginChannelClosed)
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
		/*API QA*/
		//submitMsgOpts.pServiceName = &watchlistConsumerConfig.serviceName;
		char* itemstr = watchlistConsumerConfig.itemList[itemListIndex].name.data;
		char* serviceName = strtok(itemstr, ":");
		printf("QA prints Service Name: %s\n", serviceName);
		char* itemName = strtok(NULL, ":");
		printf("QA prints Item Name: %s\n", itemName);
		RsslBuffer tempServiceName;
		submitMsgOpts.pServiceName = &tempServiceName;
		submitMsgOpts.pServiceName->data = serviceName;
		submitMsgOpts.pServiceName->length = strlen(serviceName);
		requestMsg.msgBase.msgKey.name.data = itemName;
		requestMsg.msgBase.msgKey.name.length = strlen(itemName);
		//requestMsg.msgBase.msgKey.name = watchlistConsumerConfig.itemList[itemListIndex].name;
		/*End API QA*/
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

	printf("(Channel "SOCKET_PRINT_TYPE"): \n", pChannel->socketId);

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

			if (pLoginRefresh->state.streamState != RSSL_STREAM_OPEN)
			{
				printf("  Login attempt failed.\n");
				loginChannelClosed = RSSL_TRUE;
			}

			break;
		}

		case RDM_LG_MT_STATUS:
		{
			RsslRDMLoginStatus *pLoginStatus = &pLoginMsg->status;

			printStreamInfo(NULL, RSSL_DMT_LOGIN, pLoginMsg->rdmMsgBase.streamId, "RDM_LG_MT_STATUS");

			if (pLoginStatus->flags & RDM_LG_STF_HAS_STATE)
			{
				printRsslState(&pLoginStatus->state);
				if (pLoginStatus->state.streamState != RSSL_STREAM_OPEN)
				{
					printf("  Login attempt failed or Login stream was closed.\n");
					loginChannelClosed = RSSL_TRUE;
				}
			}

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

	printf("(Channel "SOCKET_PRINT_TYPE"): \n", pChannel->socketId);

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

	printf("(Channel "SOCKET_PRINT_TYPE"): \n", pChannel->socketId);

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

	printf("(Channel "SOCKET_PRINT_TYPE"): \n", pChannel->socketId);

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
	printf("Channel Open callback\n");

	if (fieldDictionaryLoaded && enumDictionaryLoaded)
		requestItems(pReactor, pReactorChannel);
	else
		requestDictionaries(pReactor, pReactorChannel);

	return RSSL_RC_CRET_SUCCESS;
}

/* Callback for channel events. */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	char timeBuf[248] = { 0 };

	// Print out timestamp in yyyy-MM-dd HH:mm:ss.SSS format
	dumpDateTime(timeBuf, sizeof(timeBuf));

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* Save the channel on our info structure. */
			pConsumerChannel = pReactorChannel;
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				/* Set file descriptor. */
				FD_SET(pReactorChannel->socketId, &readFds);
				FD_SET(pReactorChannel->socketId, &exceptFds);
			}
			else if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				/* Set WSB file descriptors. */
				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_SET(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &readFds);
					FD_SET(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &exceptFds);
				}
			}

			printf("%s Channel "SOCKET_PRINT_TYPE" is up!\n\n", timeBuf, pReactorChannel->socketId);

			RsslErrorInfo rsslErrorInfo;
			RsslReactorChannelInfo reactorChannelInfo;

			if (rsslReactorGetChannelInfo(pReactorChannel, &reactorChannelInfo, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorGetChannelInfo(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
			else
			{
				RsslChannelInfo* pRsslChannelInfo = &reactorChannelInfo.rsslChannelInfo;
				switch (pRsslChannelInfo->encryptionProtocol)
				{
				case RSSL_ENC_TLSV1_2:
					printf("Encryption protocol: TLSv1.2\n\n");
					break;
				case RSSL_ENC_TLSV1_3:
					printf("Encryption protocol: TLSv1.3\n\n");
					break;
				default:
					printf("Encryption protocol: unknown\n\n");
				}

				RsslChannel* pRsslChannel = pReactorChannel->pRsslChannel;
				if (pRsslChannel)
				{
					printf("pRsslChannel.state=%d, socketId="SOCKET_PRINT_TYPE"\n\n", pRsslChannel->state, pRsslChannel->socketId);
				}
				else
				{
					printf("pRsslChannel is Null\n\n");
				}

				RsslReactorPreferredHostInfo* pPreferredHostInfo = &reactorChannelInfo.rsslPreferredHostInfo;
				printf("Preferred host feature: %s\n", (pPreferredHostInfo->isPreferredHostEnabled ? "Enabled" : "Disabled"));
				if (pPreferredHostInfo->isPreferredHostEnabled && prefHostPrintDetails)
				{
					printf("   The channel is preferred: %s\n", (pPreferredHostInfo->isChannelPreferred ? "Yes" : "No"));
					printf("   Connection list index: %u\n", pPreferredHostInfo->connectionListIndex);
					printf("   WarmStandBy group list index: %u\n", pPreferredHostInfo->warmStandbyGroupListIndex);

					if (pPreferredHostInfo->detectionTimeSchedule.data && pPreferredHostInfo->detectionTimeSchedule.length > 0)
						printf("   Cron schedule: %*s\n", pPreferredHostInfo->detectionTimeSchedule.length, pPreferredHostInfo->detectionTimeSchedule.data);

					printf("   Detection time interval: %u\n", pPreferredHostInfo->detectionTimeInterval);
					printf("   Remaining time: %u\n", pPreferredHostInfo->remainingDetectionTime);

					printf("\n");
				}

				/* Adjust the Ioctl preferred host options. */
				/* Defaults to whatever application has already set it to so it doesn't change. */
				if (preferredHostConfig.ioctlCallTimeInterval > 0)
				{
					RsslPreferredHostOptions* pIoctlPreferredHostOpts = &preferredHostConfig.rsslIoctlPreferredHostOpts;

					/* If a new Ioctl Preferred host parameter is not specified on the command line, */
					/* we will use the value that the application has previously set. */
					if (!preferredHostConfig.setIoctlEnablePH)
					{
						pIoctlPreferredHostOpts->enablePreferredHostOptions = pPreferredHostInfo->isPreferredHostEnabled;
					}
					if (!preferredHostConfig.setIoctlConnectListIndex)
					{
						pIoctlPreferredHostOpts->connectionListIndex = pPreferredHostInfo->connectionListIndex;
					}
					if (!preferredHostConfig.setIoctlDetectionTimeInterval)
					{
						pIoctlPreferredHostOpts->detectionTimeInterval = pPreferredHostInfo->detectionTimeInterval;
					}
					if (!preferredHostConfig.setIoctlDetectionTimeSchedule)
					{
						RsslUInt32 length = sizeof(preferredHostConfig.ioctlDetectionTimeCron);
						if (length > pPreferredHostInfo->detectionTimeSchedule.length)
							length = pPreferredHostInfo->detectionTimeSchedule.length;

						memcpy(preferredHostConfig.ioctlDetectionTimeCron, pPreferredHostInfo->detectionTimeSchedule.data, length);
						pIoctlPreferredHostOpts->detectionTimeSchedule.data = preferredHostConfig.ioctlDetectionTimeCron;
						pIoctlPreferredHostOpts->detectionTimeSchedule.length = length;
					}
					if (!preferredHostConfig.setIoctlWarmstandbyGroupListIndex)
					{
						pIoctlPreferredHostOpts->warmStandbyGroupListIndex = pPreferredHostInfo->warmStandbyGroupListIndex;
					}
					if (!preferredHostConfig.setIoctlFallBackWithinWSBGroup)
					{
						pIoctlPreferredHostOpts->fallBackWithInWSBGroup = pPreferredHostInfo->fallBackWithInWSBGroup;
					}
				}

				preferredHostConfig.directFallbackTime = 0;
				preferredHostConfig.ioctlCallTime = 0;
				time_t currentTime = 0;

				time(&currentTime);

				/* Set timeout when MultiCredWLConsumer should initiate fallback directly */
				if (preferredHostConfig.directFallbackTimeInterval > 0)
				{
					preferredHostConfig.directFallbackTime = currentTime + (time_t)preferredHostConfig.directFallbackTimeInterval;

					printf("   Direct Fallback.\n");
					printf("   Time interval: %u\n", preferredHostConfig.directFallbackTimeInterval);
					printf("   Remaining time: %lld\n", (long long int)(preferredHostConfig.directFallbackTime - time(NULL)));
					printf("\n");
				}

				/* Set timeout when MultiCredWLConsumer should initiate Ioctl call */
				if (preferredHostConfig.ioctlCallTimeInterval > 0)
				{
					preferredHostConfig.ioctlCallTime = currentTime + (time_t)preferredHostConfig.ioctlCallTimeInterval;

					printf("   Ioctl call to update PreferredHostOptions.\n");
					printf("   Time interval: %u\n", preferredHostConfig.ioctlCallTimeInterval);
					printf("   Remaining time: %lld\n", (long long int)(preferredHostConfig.ioctlCallTime - time(NULL)));
					printf("\n");
				}
			}

			if (isXmlTracingEnabled() == RSSL_TRUE) 
			{
				RsslTraceOptions traceOptions;
				char traceOutputFile[128];
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				snprintf(traceOutputFile, 128, "rsslWatchlistConsumer");
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
			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
			{
				FD_CLR(pReactorChannel->oldSocketId, &readFds);
				FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
				FD_SET(pReactorChannel->socketId, &readFds);
				FD_SET(pReactorChannel->socketId, &exceptFds);
			}
			else if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->oldSocketIdCount; index++)
				{
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->oldSocketIdList[index], &readFds);
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->oldSocketIdList[index], &exceptFds);
				}

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_SET(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &readFds);
					FD_SET(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &exceptFds);
				}
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			RsslErrorInfo rsslErrorInfo;

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
				printf("%s Channel "SOCKET_PRINT_TYPE" down.\n", timeBuf, pReactorChannel->socketId);
			else
				printf("%s Channel down.\n", timeBuf);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &readFds);
				FD_CLR(pReactorChannel->socketId, &exceptFds);
			}

			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &readFds);
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &exceptFds);
				}
			}

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
			int port = 0;

			memset(hostName, 0, 512);

			if (pRsslChannel != NULL )
			{
				port = (int)pRsslChannel->port;

				if (pRsslChannel->hostname != NULL)
				{
					memcpy(hostName, pRsslChannel->hostname, strlen(pRsslChannel->hostname));
				}
			}

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
				printf("%s Channel "SOCKET_PRINT_TYPE" down. Reconnecting hostname %s:%i\n", timeBuf, pReactorChannel->socketId, hostName, port);
			else
				printf("%s Channel down. Reconnecting hostname %s:%i\n", timeBuf, hostName, port);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &readFds);
				FD_CLR(pReactorChannel->socketId, &exceptFds);
			}

			if (pReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
			{
				RsslUInt32 index;

				for (index = 0; index < pConsumerChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &readFds);
					FD_CLR(pConsumerChannel->pWarmStandbyChInfo->socketIdList[index], &exceptFds);
				}
			}

			isConsumerChannelUp = RSSL_FALSE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("%s Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK:
		{
			/* The preferred host operation has started. */
			/* The event means - that a timer or function triggered preferred the host operation has started. */
			printf("%s Received PREFERRED_HOST_STARTING_FALLBACK for Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
		{
			/* The preferred host operation is complete and the connections are up. */
			/* The event means - that a timer or function triggered preferred host operation has completed. */
			printf("%s Received PREFERRED_HOST_COMPLETE for Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);

			// Re-set the direct fallback time interval.
			if (preferredHostConfig.directFallbackTimeInterval != 0)
			{
				time_t currentTime = 0;
				time(&currentTime);
				preferredHostConfig.directFallbackTime = currentTime + (time_t)preferredHostConfig.directFallbackTimeInterval;

				printf("\n");
				printf("   Setting next Direct Fallback time.\n");
				printf("   Time interval: %u\n", preferredHostConfig.directFallbackTimeInterval);
				printf("   Remaining time: %lld\n", (long long int)preferredHostConfig.directFallbackTime);
				printf("\n");
			}
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("%s Unknown connection event!\n", timeBuf);
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

	printf("Submitting OAuth credentials for OAuth credential name: %*s\n",
		pRequestCredential->credentialName.length, pRequestCredential->credentialName.data);


	/* For a V1 credential, only the password requires updating.  For a V2 credential, the clientSecret needs to be updated. */
	if (pRequestCredential->oAuthCredential.password.length != 0)
		reactorOAuthCredentialRenewal.password = pRequestCredential->oAuthCredential.password; /* Specified password as needed */
	else if(pRequestCredential->oAuthCredential.clientSecret.length != 0)
		reactorOAuthCredentialRenewal.clientSecret = pRequestCredential->oAuthCredential.clientSecret;
	else
		reactorOAuthCredentialRenewal.clientJWK = pRequestCredential->oAuthCredential.clientJWK;


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

/*
 * Preferred Host.
 * Check and Initiate Ioctl and direct fallback call.
 */
static RsslRet handlePreferredHostRuntime(RsslErrorInfo* pErrorInfo)
{
	RsslRet ret;
	time_t currentTime = 0;

	time(&currentTime);

	if (preferredHostConfig.directFallbackTime > 0 && currentTime >= preferredHostConfig.directFallbackTime)
	{
		preferredHostConfig.directFallbackTime = 0;

		if (pConsumerChannel != NULL)
		{
			if ((ret = rsslReactorFallbackToPreferredHost(pConsumerChannel, pErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorFallbackToPreferredHost failed:  %d(%s)\n", ret, pErrorInfo->rsslError.text);
			}
			else
			{
				printf("Direct Fallback initiated.\n");
			}
		}
	}

	if (preferredHostConfig.ioctlCallTime > 0 && currentTime >= preferredHostConfig.ioctlCallTime)
	{
		preferredHostConfig.ioctlCallTime = 0;
		if (pConsumerChannel != NULL)
		{
			if ((ret = rsslReactorChannelIoctl(pConsumerChannel,
				RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS,
				(void*)&preferredHostConfig.rsslIoctlPreferredHostOpts,
				pErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl failed:  %d(%s)\n", ret, pErrorInfo->rsslError.text);
			}
			else
			{
				printf("rsslReactorChannelIoctl initiated.\n");
			}
		}
	}
	return RSSL_RET_SUCCESS;
}
