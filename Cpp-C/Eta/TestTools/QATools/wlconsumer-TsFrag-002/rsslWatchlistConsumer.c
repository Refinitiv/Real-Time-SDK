/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the main file for the rsslWatchlistConsumer application.  It is a single-threaded
 * client application that utilizes the UPA Reactor's watchlist to provide recovery of data.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the UPA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application makes use of the RDM package for easier decoding of Login & Source Directory
 * messages.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include "watchlistConsumerConfig.h"
#include "itemDecoder.h"
#include "postHandler.h"
#include "simpleTunnelMsgHandler.h"
#include "queueMsgHandler.h"
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
#define SOCKET_PRINT_TYPE "%llu"    /* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"  /* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

static RsslReactorChannel *pConsumerChannel = NULL;
static RsslBool itemsRequested = RSSL_FALSE;

static QueueMsgHandler queueMsgHandler;
static SimpleTunnelMsgHandler simpleTunnelMsgHandler;
static void initTunnelStreamMessaging();
RsslBool runTimeExpired = RSSL_FALSE;

int main(int argc, char **argv)
{
	RsslReactor					*pReactor;

	RsslRet						ret;
	RsslCreateReactorOptions	reactorOpts;
	RsslReactorConnectOptions	reactorConnectOpts;
	RsslReactorConnectInfo		reactorConnectInfo;
	RsslErrorInfo				rsslErrorInfo;

	RsslReactorOMMConsumerRole	consumerRole;
	RsslRDMLoginRequest			loginRequest;
	RsslRDMDirectoryRequest		dirRequest;

	time_t						stopTime;
	time_t						currentTime;
	time_t						nextPostTime;

	RsslBool					postWithMsg = RSSL_TRUE;

	watchlistConsumerConfigInit(argc, argv);

	itemDecoderInit();
	postHandlerInit();
	initTunnelStreamMessaging();

	stopTime = time(NULL);

	if (stopTime < 0)
	{
		printf("time() failed.\n");
		exit(-1);
	}

	nextPostTime = stopTime + POST_MESSAGE_FREQUENCY;
	stopTime += watchlistConsumerConfig.runTime;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		exit(-1);
	}
	
	/* Prepare a default login request(Use 1 as the Login Stream ID). 
	 * This function sets login request parameters according to what a consumer
	 * application would normally set. */
	if (rsslInitDefaultRDMLoginRequest(&loginRequest, LOGIN_STREAM_ID) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		exit(-1);
	}

	/* Set login userName if specified. Otherwise, rsslInitDefaultRDMLoginRequest()
	 * will set it to the user's system login name. */
	if (watchlistConsumerConfig.userName.length)
		loginRequest.userName = watchlistConsumerConfig.userName;

	/* Setup consumer role for connection. */
	rsslClearOMMConsumerRole(&consumerRole);
	consumerRole.pLoginRequest = &loginRequest;
	consumerRole.base.channelEventCallback = channelEventCallback;
	consumerRole.base.defaultMsgCallback = msgCallback;
	consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	consumerRole.watchlistOptions.channelOpenCallback = channelOpenCallback;

	consumerRole.loginMsgCallback = loginMsgCallback;

	/* Prepare a default directory request. */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, DIRECTORY_STREAM_ID) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		exit(-1);
	}

	consumerRole.pDirectoryRequest = &dirRequest;
	consumerRole.directoryMsgCallback = directoryMsgCallback;
	consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;

	/* Create Reactor. */
	rsslClearCreateReactorOptions(&reactorOpts);
	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	/* Setup connection options. */
	rsslClearReactorConnectOptions(&reactorConnectOpts);
	rsslClearReactorConnectInfo(&reactorConnectInfo);

	if (watchlistConsumerConfig.connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		reactorConnectInfo.rsslConnectOptions.connectionType = watchlistConsumerConfig.connectionType;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.hostName;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.port;
		reactorConnectInfo.rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
	}
	else
	{
		/* Segmented multicast connection. */
		reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.sendAddress = watchlistConsumerConfig.sendAddress;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.recvAddress = watchlistConsumerConfig.recvAddress;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.sendServiceName = watchlistConsumerConfig.sendPort;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.recvServiceName = watchlistConsumerConfig.recvPort;
		reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.unicastServiceName = watchlistConsumerConfig.unicastPort;
		reactorConnectInfo.rsslConnectOptions.multicastOpts.flags = RSSL_MCAST_FILTERING_ON;

		if (strlen(watchlistConsumerConfig.interface))
			reactorConnectInfo.rsslConnectOptions.connectionInfo.segmented.interfaceName = watchlistConsumerConfig.interface;

		if (watchlistConsumerConfig.enableHostStatMessages)
		{
			/* Configure transport to publish Host Statistics Messages. */
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmMultAddress = watchlistConsumerConfig.hsmAddress;
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmPort = watchlistConsumerConfig.hsmPort;
			reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmInterval = watchlistConsumerConfig.hsmInterval;
			if (strlen(watchlistConsumerConfig.hsmInterface))
				reactorConnectInfo.rsslConnectOptions.multicastOpts.hsmInterface = watchlistConsumerConfig.hsmInterface;
		}
	}

	reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	/* Use one connection, with an unlimited number of reconnect attempts. */
	reactorConnectOpts.reactorConnectionList = &reactorConnectInfo;
	reactorConnectOpts.connectionCount = 1;
	reactorConnectOpts.reconnectAttemptLimit = -1;
	reactorConnectOpts.reconnectMinDelay = 500;
	reactorConnectOpts.reconnectMaxDelay = 3000;


	/* Connect. */
	if ((ret = rsslReactorConnect(pReactor, &reactorConnectOpts, 
					(RsslReactorChannelRole*)&consumerRole, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorConnect() failed: %d(%s)\n", 
				ret, rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	/* Dispatch until application stops. */
	do
	{
		struct timeval 				selectTime;
		fd_set						readFds;
		fd_set						exceptFds;
		RsslReactorDispatchOptions	dispatchOpts;

		FD_ZERO(&readFds);
		FD_ZERO(&exceptFds);
		FD_SET(pReactor->eventFd, &readFds);
		FD_SET(pReactor->eventFd, &exceptFds);

		if (pConsumerChannel)
		{
			FD_SET(pConsumerChannel->socketId, &readFds);
			FD_SET(pConsumerChannel->socketId, &exceptFds);
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
			exit(-1);
		}

		if ((currentTime = time(NULL)) < 0)
		{
			printf("time() failed.\n");
		}

		if (pConsumerChannel && !runTimeExpired)
		{
			if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
				handleSimpleTunnelMsgHandler(pReactor, pConsumerChannel, &simpleTunnelMsgHandler);
			if (watchlistConsumerConfig.isQueueMessagingEnabled)
				handleQueueMsgHandler(pReactor, pConsumerChannel, &queueMsgHandler);

			/* Handle posting, if configured. */
			if (currentTime >= nextPostTime)
			{
				nextPostTime = currentTime + POST_MESSAGE_FREQUENCY;

				if (watchlistConsumerConfig.post)
					sendOnStreamPostMsg(pReactor, pConsumerChannel, postWithMsg);

				if (watchlistConsumerConfig.offPost)
					sendOffStreamPostMsg(pReactor, pConsumerChannel, postWithMsg);

				if (postWithMsg)
					postWithMsg = RSSL_FALSE;
				else
					postWithMsg = RSSL_TRUE;
			}
		}

		if (currentTime >= stopTime)
		{
			if (!runTimeExpired)
			{
				runTimeExpired = RSSL_TRUE;
				printf("Run time expired.\n");
				if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL
						|| queueMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
					printf("Waiting for tunnel stream to close...\n\n");

				/* Close tunnel streams if any are open. */
				simpleTunnelMsgHandlerCloseStreams(&simpleTunnelMsgHandler);
				queueMsgHandlerCloseStreams(&queueMsgHandler);
			}

			/* Wait for tunnel streams to close before closing channel. */
			if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream == NULL
					&& queueMsgHandler.tunnelStreamHandler.pTunnelStream == NULL)
				break;
			else if (currentTime >= stopTime + 10)
			{
				printf("Tunnel stream still open after ten seconds, giving up.\n");
				break;
			}
		}

	} while(ret >= RSSL_RET_SUCCESS);

	/* Clean up and exit. */

	if (pConsumerChannel)
	{
		if ((ret = rsslReactorCloseChannel(pReactor, pConsumerChannel, &rsslErrorInfo))
				!= RSSL_RET_SUCCESS)
		{
			printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}

	if ((ret = rsslDestroyReactor(pReactor, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	cleanupFdmDictionary();

	rsslUninitialize();

	itemDecoderCleanup();

	watchlistConsumerConfigCleanup();

	exit(0);
}

/* When requesting symbol list items, this application will also request data streams from
 * those items.  This function demonstrates how to encode the item request payload to request 
 * data streams. */
RsslRet encodeSymbolListDataStreamsPayload(RsslBuffer *pDataBody, RsslUInt dataStreamFlags)
{
        RsslEncodeIterator eIter;
        RsslElementList eList, eBehaviorsList;
        RsslElementEntry eEntry, eDataStreamsEntry;
        RsslRet ret;

        rsslClearEncodeIterator(&eIter);
        rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
        rsslSetEncodeIteratorBuffer(&eIter, pDataBody);

        rsslClearElementList(&eList);
        eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode symbol list behaviors element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_SYMBOL_LIST_BEHAVIORS;
        eEntry.dataType = RSSL_DT_ELEMENT_LIST;
        if ((ret = rsslEncodeElementEntryInit(&eIter, &eEntry, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        rsslClearElementList(&eBehaviorsList);
        eBehaviorsList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eBehaviorsList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode data streams element. */
        rsslClearElementEntry(&eDataStreamsEntry);
        eDataStreamsEntry.name = RSSL_ENAME_DATA_STREAMS;
        eDataStreamsEntry.dataType = RSSL_DT_UINT;
        if ((ret = rsslEncodeElementEntry(&eIter, &eDataStreamsEntry, &dataStreamFlags)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementEntryComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        pDataBody->length = rsslGetEncodedBufferLength(&eIter);

        return RSSL_RET_SUCCESS;
}

/* Demonstrates encoding of a field list view in a request payload. */
RsslRet encodeViewPayload(RsslBuffer *pDataBody, RsslInt *fieldIdList, RsslUInt32 fieldIdCount)
{
        RsslEncodeIterator eIter;
        RsslElementList eList;
        RsslElementEntry eEntry;
        RsslRet ret;
        RsslUInt viewType;
        RsslArray rsslArray;

        RsslUInt32 i;

        rsslClearEncodeIterator(&eIter);
        rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
        rsslSetEncodeIteratorBuffer(&eIter, pDataBody);

        rsslClearElementList(&eList);
        eList.flags = RSSL_ELF_HAS_STANDARD_DATA;
        if ((ret = rsslEncodeElementListInit(&eIter, &eList, 0, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementList() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode view type (field ID list) into the ViewType element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_VIEW_TYPE;
        eEntry.dataType = RSSL_DT_UINT;
        viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
        if ((ret = rsslEncodeElementEntry(&eIter, &eEntry, &viewType)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        /* Encode the fields desired into the ViewData element. */
        rsslClearElementEntry(&eEntry);
        eEntry.name = RSSL_ENAME_VIEW_DATA;
        eEntry.dataType = RSSL_DT_ARRAY;
        viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
        if ((ret = rsslEncodeElementEntryInit(&eIter, &eEntry, 0)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        rsslClearArray(&rsslArray);
        rsslArray.primitiveType = RSSL_DT_INT;
        rsslArray.itemLength = 2;
        if ((ret = rsslEncodeArrayInit(&eIter, &rsslArray)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementArrayInit() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

		/* Encode each desired field into the array. */
        for (i = 0; i < fieldIdCount; ++i)
        {
                if ((ret = rsslEncodeArrayEntry(&eIter, NULL, &fieldIdList[i])) != RSSL_RET_SUCCESS)
                {
                        printf("rsslEncodeArrayEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                        return ret;
                }
        }

        if ((ret = rsslEncodeArrayComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeArrayComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementEntryComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementEntryComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        if ((ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
        {
                printf("rsslEncodeElementListComplete() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
                return ret;
        }

        pDataBody->length = rsslGetEncodedBufferLength(&eIter);

        return RSSL_RET_SUCCESS;
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


		if (requestMsg.msgBase.domainType == RSSL_DMT_SYMBOL_LIST && 
				watchlistConsumerConfig.itemList[itemListIndex].symbolListData)
		{
			/* Encode the message payload with a request for data streams of the items
			 * received in the symbol list response. This will cause items present in
			 * response messages to be automatically opened on behalf of the application. */
			RsslUInt dataStreamFlags = RDM_SYMBOL_LIST_DATA_STREAMS;

			requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
			if (encodeSymbolListDataStreamsPayload(&dataBody, dataStreamFlags) != RSSL_RET_SUCCESS)
			{
				printf("encodeSymbolListDataStreamsPayload() failed.");
				exit(-1);
			}
			requestMsg.msgBase.encDataBody = dataBody;
		}

		/* If desired, request a view for Market Price items. */
		if (requestMsg.msgBase.domainType == RSSL_DMT_MARKET_PRICE && watchlistConsumerConfig.setView)
		{
			RsslInt fieldIdList[] = { 22, 25, 30, 31, 1025 };
			RsslUInt32 fieldIdCount = 5;

			requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
			requestMsg.flags |= RSSL_RQMF_HAS_VIEW;
			if (encodeViewPayload(&dataBody, fieldIdList, fieldIdCount) != RSSL_RET_SUCCESS)
			{
				printf("encodeViewPayload() failed.");
				exit(-1);
			}
			requestMsg.msgBase.encDataBody = dataBody;

		}


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

			break;
		}

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

			if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
				tunnelStreamHandlerProcessServiceUpdate(&simpleTunnelMsgHandler.tunnelStreamHandler,
				&watchlistConsumerConfig.tunnelStreamServiceName, pService);

			if (watchlistConsumerConfig.isQueueMessagingEnabled)
				tunnelStreamHandlerProcessServiceUpdate(&queueMsgHandler.tunnelStreamHandler,
				&watchlistConsumerConfig.tunnelStreamServiceName, pService);
		}
	}

	printf("\n");

	if (watchlistConsumerConfig.isTunnelStreamMessagingEnabled)
	{
		if (!simpleTunnelMsgHandler.tunnelStreamHandler.isTunnelServiceFound)
			printf("  Directory response does not contain service name for tunnel streams: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
		else if (!simpleTunnelMsgHandler.tunnelStreamHandler.tunnelServiceSupported)
			printf("  Service in use for tunnel streams does not support them: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
	}

	if (watchlistConsumerConfig.isQueueMessagingEnabled)
	{
		if (!queueMsgHandler.tunnelStreamHandler.isTunnelServiceFound)
			printf("  Directory response does not contain service name for queue messaging: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
		else if (!queueMsgHandler.tunnelStreamHandler.tunnelServiceSupported)
			printf("  Service in use for queue messaging does not support it: %s\n\n",
				watchlistConsumerConfig.tunnelStreamServiceName.data);
	}


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
 * This callback handles provider-driven streams, which this application will receive
 * when requesting data with any symbol list request. As new item streams are received,
 * it will store the stream's information for display.
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

				/* Store new item information, if not currently stored and stream is 
				 * provider-driven. */
				if (!pItem && pRsslMsg->msgBase.streamId < 0)
					pItem = addProvidedItemInfo(pRsslMsg->msgBase.streamId,
							&pRsslMsg->msgBase.msgKey, pRsslMsg->msgBase.domainType);

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

				/* Store new item information, if not currently stored and stream is 
				 * provider-driven. */
				if (!pItem && pRsslMsg->msgBase.streamId < 0)
					pItem = addProvidedItemInfo(pRsslMsg->msgBase.streamId,
							&pRsslMsg->msgBase.msgKey, pRsslMsg->msgBase.domainType);
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

	/* Check state of any provider-driven streams.
	 * If the state indicates the item was closed, remove it from our list. */
	if (pState && pItem && pRsslMsg->msgBase.streamId < 0 && pState->streamState != RSSL_STREAM_OPEN)
		removeProvidedItemInfo(pItem);

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
			printf("Channel "SOCKET_PRINT_TYPE" is up!\n\n", pReactorChannel->socketId);
			if (isXmlTracingEnabled() == RSSL_TRUE) 
			{
				RsslTraceOptions traceOptions;
				char traceOutputFile[128];
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				snprintf(traceOutputFile, 128, "rsslWatchlistConsumer\0");
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		case RSSL_RC_CET_FD_CHANGE:
			return RSSL_RC_CRET_SUCCESS;
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
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
				printf("Channel "SOCKET_PRINT_TYPE" down. Reconnecting\n", pReactorChannel->socketId);
			else
				printf("Channel down. Reconnecting\n");

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			pConsumerChannel = NULL;
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
			exit(-1);
		}
	}


	return RSSL_RC_CRET_SUCCESS;
}

/* Initialize TunnelStreamHandler object, in which we store information about that tunnel stream
 * we will open. */
static void initTunnelStreamMessaging()
{
	simpleTunnelMsgHandlerInit(&simpleTunnelMsgHandler, (char*)"WatchlistConsumer", 
			watchlistConsumerConfig.tunnelStreamDomainType,
			watchlistConsumerConfig.useAuthentication, RSSL_FALSE);
	// APIQA:
	setMsgSize(watchlistConsumerConfig.msgSize);
	// END APIQA:

	if (watchlistConsumerConfig.isQueueMessagingEnabled)
	{
		RsslUInt32 i;

		if (loadFdmDictionary() == RSSL_FALSE)
			exit(-1);

		queueMsgHandlerInit(&queueMsgHandler, (char*)"WatchlistConsumer", watchlistConsumerConfig.tunnelStreamDomainType,
				watchlistConsumerConfig.useAuthentication);
		queueMsgHandler.sourceName = watchlistConsumerConfig.queueSourceName;
		queueMsgHandler.destNameCount = watchlistConsumerConfig.queueDestNameCount;
		for (i = 0; i < watchlistConsumerConfig.queueDestNameCount; ++i)
			queueMsgHandler.destNames[i] = watchlistConsumerConfig.queueDestNameList[i];
	}
}
