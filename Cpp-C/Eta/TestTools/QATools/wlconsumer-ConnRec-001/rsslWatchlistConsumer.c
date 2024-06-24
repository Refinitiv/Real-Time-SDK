/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 LSEG. All rights reserved.     
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

#include "watchlistConsumerConfig.h"
#include "itemDecoder.h"
#include "postHandler.h"
#include "simpleTunnelMsgHandler.h"
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

static PostServiceInfo serviceInfo;

static SimpleTunnelMsgHandler simpleTunnelMsgHandler;
static void initTunnelStreamMessaging();
RsslBool runTimeExpired = RSSL_FALSE;

/* For UserAuthn authentication login reissue */
static RsslUInt loginReissueTime; // represented by epoch time in seconds
static RsslBool canSendLoginReissue;

int main(int argc, char **argv)
{
	RsslReactor							*pReactor;

	RsslRet								ret;
	RsslCreateReactorOptions			reactorOpts;
	RsslReactorConnectOptions			reactorConnectOpts;
	//APQA
	RsslReactorConnectInfo		reactorConnectInfo[3];
	RsslErrorInfo						rsslErrorInfo;
	RsslReactorServiceDiscoveryOptions	serviceDiscoveryOpts;

	RsslReactorOMMConsumerRole			consumerRole;
	RsslRDMLoginRequest					loginRequest;
	RsslRDMDirectoryRequest				dirRequest;

	time_t								stopTime;
	time_t								currentTime;
	time_t								nextPostTime;

	RsslBool							postWithMsg = RSSL_TRUE;

	RsslInitializeExOpts		initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	watchlistConsumerConfigInit(argc, argv);

	itemDecoderInit();
	postHandlerInit();
	initTunnelStreamMessaging();

	/* handle service updates for channel posting purposes */
	clearPostServiceInfo(&serviceInfo);

	stopTime = time(NULL);

	if (stopTime < 0)
	{
		printf("time() failed.\n");
		exit(-1);
	}

	nextPostTime = stopTime + POST_MESSAGE_FREQUENCY;
	stopTime += watchlistConsumerConfig.runTime;

	initOpts.jitOpts.libsslName = watchlistConsumerConfig.libsslName;
	initOpts.jitOpts.libcryptoName = watchlistConsumerConfig.libcryptoName;
	initOpts.jitOpts.libcurlName = watchlistConsumerConfig.libcurlName;

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


	rsslClearReactorServiceDiscoveryOptions(&serviceDiscoveryOpts);
	/* Set login userName if specified. Otherwise, rsslInitDefaultRDMLoginRequest()
	 * will set it to the user's system login name. */
	if (watchlistConsumerConfig.userName.length)
	{
		loginRequest.userName = watchlistConsumerConfig.userName;
		
		if(watchlistConsumerConfig.queryEndpoint) 
			serviceDiscoveryOpts.userName = loginRequest.userName;
	}

	/* Set login password if specified. */
	if (watchlistConsumerConfig.password.length)
	{
		loginRequest.password = watchlistConsumerConfig.password;

		if(watchlistConsumerConfig.queryEndpoint) 
			serviceDiscoveryOpts.password = loginRequest.password;
	}
	
	/* If the authentication Token is specified, set it and authenticationExtended(if present) to the loginRequest */
	if (watchlistConsumerConfig.authenticationToken.length)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
		loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		loginRequest.userName = watchlistConsumerConfig.authenticationToken;
		
		if(watchlistConsumerConfig.authenticationExtended.length)
		{
			loginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
			loginRequest.authenticationExtended = watchlistConsumerConfig.authenticationExtended;
		}
	}
	
	if (watchlistConsumerConfig.appId.length)
	{
		loginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
		loginRequest.applicationId = watchlistConsumerConfig.appId;
	}

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

	if (watchlistConsumerConfig.clientId.data)
	{
		consumerRole.clientId = watchlistConsumerConfig.clientId;
	}

	/* Create Reactor. */
	rsslClearCreateReactorOptions(&reactorOpts);
	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	/* Setup connection options. */
	rsslClearReactorConnectOptions(&reactorConnectOpts);
	//APIQA
    RsslUInt32 i;
	for (i = 0; i < watchlistConsumerConfig.numConnections; ++i)
	{
		rsslClearReactorConnectInfo(&reactorConnectInfo[i]);
	
		if(watchlistConsumerConfig.location.length == 0) // Use the default location from the Reactor if not specified
		{
			watchlistConsumerConfig.location.length =
	                (RsslUInt32)snprintf(watchlistConsumerConfig._locationMem, 255, "%s", reactorConnectInfo[i].location.data);
	            watchlistConsumerConfig.location.data = watchlistConsumerConfig._locationMem;					
		}
	
		if (watchlistConsumerConfig.queryEndpoint)
		{
			if (watchlistConsumerConfig.clientId.length)
				serviceDiscoveryOpts.clientId = watchlistConsumerConfig.clientId;
	
			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
			{
				serviceDiscoveryOpts.transport = RSSL_RD_TP_TCP;
			}
			else
			{
				printf("Error: Invalid connection type %d for querying RDP service discovery", watchlistConsumerConfig.connectionType);
				exit(-1);
			}
	
			if (watchlistConsumerConfig.proxyHost[0] != '\0')
			{
				serviceDiscoveryOpts.proxyHostName.data = watchlistConsumerConfig.proxyHost;
				serviceDiscoveryOpts.proxyHostName.length = (RsslUInt32)strlen(serviceDiscoveryOpts.proxyHostName.data);
			}
	
			if (watchlistConsumerConfig.proxyPort[0] != '\0')
			{
				serviceDiscoveryOpts.proxyPort.data = watchlistConsumerConfig.proxyPort;
				serviceDiscoveryOpts.proxyPort.length = (RsslUInt32)strlen(serviceDiscoveryOpts.proxyPort.data);
			}
	
			if (watchlistConsumerConfig.proxyUserName[0] != '\0')
			{
				serviceDiscoveryOpts.proxyUserName.data = watchlistConsumerConfig.proxyUserName;
				serviceDiscoveryOpts.proxyUserName.length = (RsslUInt32)strlen(serviceDiscoveryOpts.proxyUserName.data);
			}
	
			if (watchlistConsumerConfig.proxyPasswd[0] != '\0')
			{
				serviceDiscoveryOpts.proxyPasswd.data = watchlistConsumerConfig.proxyPasswd;
				serviceDiscoveryOpts.proxyPasswd.length = (RsslUInt32)strlen(serviceDiscoveryOpts.proxyPasswd.data);
			}
			if (watchlistConsumerConfig.proxyDomain[0] != '\0')
	
			{
				serviceDiscoveryOpts.proxyDomain.data = watchlistConsumerConfig.proxyDomain;
				serviceDiscoveryOpts.proxyDomain.length = (RsslUInt32)strlen(serviceDiscoveryOpts.proxyDomain.data);
			}
	
			serviceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;
	
			if(rsslReactorQueryServiceDiscovery(pReactor, &serviceDiscoveryOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("Error: %s\n", rsslErrorInfo.rsslError.text);
				exit(-1);
			}
		}
	
		/* Setup connection options. */
		if (watchlistConsumerConfig.connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
		{
			reactorConnectInfo[i].rsslConnectOptions.connectionType = watchlistConsumerConfig.connectionType;

			if (watchlistConsumerConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED && watchlistConsumerConfig.encryptedConnectionType != RSSL_CONN_TYPE_INIT)
							reactorConnectInfo[i].rsslConnectOptions.encryptionOpts.encryptedProtocol = watchlistConsumerConfig.encryptedConnectionType;

			if (i == 0)
                        {
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.hostName1;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.port1;
                        }
			if (i == 1)
                        {
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.hostName2;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.port2;
                        }
			if (i == 2)
                        {
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = watchlistConsumerConfig.hostName3;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = watchlistConsumerConfig.port3;
                        }
			
			reactorConnectInfo[i].rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
			reactorConnectInfo[i].rsslConnectOptions.proxyOpts.proxyHostName = watchlistConsumerConfig.proxyHost;
			reactorConnectInfo[i].rsslConnectOptions.proxyOpts.proxyPort = watchlistConsumerConfig.proxyPort;
			reactorConnectInfo[i].rsslConnectOptions.proxyOpts.proxyUserName = watchlistConsumerConfig.proxyUserName;
			reactorConnectInfo[i].rsslConnectOptions.proxyOpts.proxyPasswd = watchlistConsumerConfig.proxyPasswd;
			reactorConnectInfo[i].rsslConnectOptions.proxyOpts.proxyDomain = watchlistConsumerConfig.proxyDomain;
			reactorConnectInfo[i].enableSessionManagement = watchlistConsumerConfig.enableSessionMgnt;
			reactorConnectInfo[i].location = watchlistConsumerConfig.location;
			reactorConnectInfo[i].rsslConnectOptions.encryptionOpts.openSSLCAStore = watchlistConsumerConfig.sslCAStore;
		}
		else
		{
			/* Segmented multicast connection. */
			reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
			reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.sendAddress = watchlistConsumerConfig.sendAddress;
			reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.recvAddress = watchlistConsumerConfig.recvAddress;
			reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.sendServiceName = watchlistConsumerConfig.sendPort;
			reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.recvServiceName = watchlistConsumerConfig.recvPort;
			reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.unicastServiceName = watchlistConsumerConfig.unicastPort;
			reactorConnectInfo[0].rsslConnectOptions.multicastOpts.flags = RSSL_MCAST_FILTERING_ON;
	
			if (strlen(watchlistConsumerConfig.interface))
				reactorConnectInfo[0].rsslConnectOptions.connectionInfo.segmented.interfaceName = watchlistConsumerConfig.interface;
	
			if (watchlistConsumerConfig.enableHostStatMessages)
			{
				/* Configure transport to publish Host Statistics Messages. */
				reactorConnectInfo[0].rsslConnectOptions.multicastOpts.hsmMultAddress = watchlistConsumerConfig.hsmAddress;
				reactorConnectInfo[0].rsslConnectOptions.multicastOpts.hsmPort = watchlistConsumerConfig.hsmPort;
				reactorConnectInfo[0].rsslConnectOptions.multicastOpts.hsmInterval = watchlistConsumerConfig.hsmInterval;
				if (strlen(watchlistConsumerConfig.hsmInterface))
					reactorConnectInfo[0].rsslConnectOptions.multicastOpts.hsmInterface = watchlistConsumerConfig.hsmInterface;
			}
	
		}
	
		reactorConnectInfo[i].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		reactorConnectInfo[i].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	}
	/* Use one connection, with an unlimited number of reconnect attempts. */
	reactorConnectOpts.reactorConnectionList = &reactorConnectInfo;
	reactorConnectOpts.connectionCount = watchlistConsumerConfig.numConnections;
	reactorConnectOpts.reconnectAttemptLimit = watchlistConsumerConfig.attempLimit;
	reactorConnectOpts.reconnectMinDelay = watchlistConsumerConfig.minDelay;
	reactorConnectOpts.reconnectMaxDelay = watchlistConsumerConfig.maxDelay;


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

		if (pConsumerChannel && pConsumerChannel->pRsslChannel && pConsumerChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
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

			/* Handle posting, if configured. */
			if (currentTime >= nextPostTime)
			{
				nextPostTime = currentTime + POST_MESSAGE_FREQUENCY;

				if (watchlistConsumerConfig.post && isConsumerChannelUp && serviceInfo.isServiceFound && serviceInfo.isServiceUp)
					sendOnStreamPostMsg(pReactor, pConsumerChannel, postWithMsg);

				if (watchlistConsumerConfig.offPost && isConsumerChannelUp && serviceInfo.isServiceFound && serviceInfo.isServiceUp)
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
				if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
					printf("Waiting for tunnel stream to close...\n\n");

				/* Close tunnel streams if any are open. */
				simpleTunnelMsgHandlerCloseStreams(&simpleTunnelMsgHandler);
			}

			/* Wait for tunnel streams to close before closing channel. */
			if (simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream == NULL)
				break;
			else if (currentTime >= stopTime + 10)
			{
				printf("Tunnel stream still open after ten seconds, giving up.\n");
				break;
			}
		}

		// send login reissue if login reissue time has passed
		if (canSendLoginReissue == RSSL_TRUE &&
			currentTime >= (time_t)loginReissueTime)
		{
			RsslReactorSubmitMsgOptions submitMsgOpts;
			RsslErrorInfo rsslErrorInfo;

			rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
			submitMsgOpts.pRDMMsg = (RsslRDMMsg*)&loginRequest;
			if ((ret = rsslReactorSubmitMsg(pReactor,pConsumerChannel,&submitMsgOpts,&rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf("Login reissue failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
			}
			else
			{
				printf("Login reissue sent\n");
			}
			canSendLoginReissue = RSSL_FALSE;
		}
	} while(ret >= RSSL_RET_SUCCESS);

	/* Clean up and exit. */
	clearPostServiceInfo(&serviceInfo);

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

	rsslUninitialize();

	itemDecoderCleanup();
	
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
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
				printf("  AuthenticationTTReissue: %llu\n", pLoginRefresh->authenticationTTReissue);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
				printf("  AuthenticationExtendedResp: %.*s\n", pLoginRefresh->authenticationExtendedResp.length, pLoginRefresh->authenticationExtendedResp.data);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginRefresh->authenticationErrorCode);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginRefresh->authenticationErrorText.length, pLoginRefresh->authenticationErrorText.data);

			// get login reissue time from authenticationTTReissue
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
			{
				loginReissueTime = pLoginRefresh->authenticationTTReissue;
				canSendLoginReissue = watchlistConsumerConfig.enableSessionMgnt ? RSSL_FALSE : RSSL_TRUE;
			}

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

			/* handle service updates for channel posting purposes */
			processPostServiceUpdate(&serviceInfo, &watchlistConsumerConfig.serviceName, pService);
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

	/* handle service updates for channel posting purposes */
	if (watchlistConsumerConfig.post || watchlistConsumerConfig.offPost)
	{
		if (!serviceInfo.isServiceFound)
			printf("  Directory response does not contain service name to send post messages: %s\n\n",
				watchlistConsumerConfig.serviceName.data);
		else if (!serviceInfo.isServiceUp)
			printf("  Service in use to send post messages is down: %s\n\n",
				watchlistConsumerConfig.serviceName.data);
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

			isConsumerChannelUp = RSSL_TRUE;
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

			isConsumerChannelUp = RSSL_FALSE;
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
}

RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pEndPointEvent)
{
	RsslUInt32 index;
	RsslReactorServiceEndpointInfo *pServiceEndpointInfo;
	for(index = 0; index < pEndPointEvent->serviceEndpointInfoCount; index++)
	{
		pServiceEndpointInfo = &pEndPointEvent->serviceEndpointInfoList[index];
		if(pServiceEndpointInfo->locationCount == 2) // Get an endpoint that provides auto failover for the specified location
		{
			if (strncmp(watchlistConsumerConfig.location.data, pServiceEndpointInfo->locationList[0].data, watchlistConsumerConfig.location.length) == 0 )
			{
				snprintf(watchlistConsumerConfig.hostName1, 255, "%s", pServiceEndpointInfo->endPoint.data);	
				snprintf(watchlistConsumerConfig.port1, 255, "%s", pServiceEndpointInfo->port.data);	
				break;
			}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void clearPostServiceInfo(PostServiceInfo *serviceInfo)
{
	serviceInfo->isServiceFound = RSSL_FALSE;
	serviceInfo->isServiceUp = RSSL_FALSE;
	serviceInfo->serviceId = 0;
}

void processPostServiceUpdate(PostServiceInfo *serviceInfo, RsslBuffer *pMatchServiceName, RsslRDMService* pService)
{
	/* Save service information for tunnel stream. */
	if (!serviceInfo->isServiceFound)
	{
		/* Check if the name matches the service we're looking for. */
		if (pService->flags & RDM_SVCF_HAS_INFO
			&& rsslBufferIsEqual(&pService->info.serviceName, pMatchServiceName))
		{
			serviceInfo->isServiceFound = RSSL_TRUE;
			serviceInfo->serviceId = (RsslUInt16)pService->serviceId;
		}
	}

	if (pService->serviceId == serviceInfo->serviceId)
	{
		/* Process the state of the tunnel stream service. */
		if (pService->action != RSSL_MPEA_DELETE_ENTRY)
		{
			/* Check state. */
			if (pService->flags & RDM_SVCF_HAS_STATE)
			{
				serviceInfo->isServiceUp =
					pService->state.serviceState == 1 &&
					(!(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS) ||
						pService->state.acceptingRequests == 1);
			}
		}
		else
		{
			clearPostServiceInfo(serviceInfo);
		}
	}
}
