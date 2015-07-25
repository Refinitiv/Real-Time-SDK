/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the main file for the rsslVAQueueConsumer application.  It is a single-threaded
 * client application that connects to and exchanges messages with a Queue Provider.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the UPA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>


#ifdef _WIN32
#include <winsock2.h>
#define strtok_r strtok_s
#define snprintf _snprintf
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif


#include "rsslVAQueueCons.h"
#include "QConsSimpleQueueMsgHandler.h"


#include "rsslLoginConsumer.h"
#include "QConsSimpleDirectoryHandler.h"

#include "rtr/rsslReactor.h"


RsslInt32 timeToRun = 300;
fd_set readFds, exceptFds;
static RsslReactor *pReactor = NULL;
static time_t rsslConsumerRuntime = 0;
RsslBool runTimeExpired = RSSL_FALSE;
static ChannelStorage chanCommand;
char userNameBlock[128];
RsslBuffer userName = RSSL_INIT_BUFFER;

static RsslBool xmlTrace = RSSL_FALSE;
static char traceOutputFile[128];

void printUsageAndExit(char *appName)
{
	
	printf("Usage: %s or\n%s  [-c [<hostname>:<port>]] [-qServiceName <QProv service>] [-qSourceName <localQueue>] [-qDestName <remoteQueue>] [-uname <LoginUsername>] [-tsAuth ] [-tsDomain <domain>] [-x] [-runtime <seconds>]"
			"\n -c specifies a connection to open :\n"
			"\n     hostname:        Hostname of provider to connect to"
			"\n     port:            Port of provider to connect to"			
			"\n -tsDomain specifies the domainType for the tunnel stream\n"
			"\n -tsAuth causes the consumer to enable authentication when opening the tunnel stream\n"
			"\n -qSourceName specifies the source name for queue messages\n"
			"\n -qDestName specifies the destination name for queue messages\n"
			"\n -qServiceName specifies the name of the service to use for queue messages\n"		
			"\n -uname changes the username used when logging into the provider.\n"
			"\n -x enables tracing of messages sent to and received from the channel\n"
			"\n -runtime adjusts the running time of the application.\n"
			, appName, appName);

	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(-1);
}



/*
 * Initializes the run-time.
 */
static void initRuntime()
{
	time_t currentTime = 0;
	
	/* get current time */
	time(&currentTime);

	rsslConsumerRuntime = currentTime + (time_t)timeToRun;

}



void exitApp(int code)
{
	printf("Exiting.\n");
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(code);
}

/*
 * Cleans up and exits the application.
 */
void cleanUpAndExit(int code)
{
	RsslErrorInfo rsslErrorInfo;

	if (pReactor && rsslDestroyReactor(pReactor, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error cleaning up reactor: %s\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}

	rsslUninitialize();

	exitApp(code);
}


/*
 * Handles time-related events.
 * Calls rsslReactorConnect() for any channels that we want to recover.
 * Exits the application if the run-time has expired.
 */
static void handleRuntime()
{
	time_t currentTime = 0;
	RsslRet	retval = 0;
	RsslErrorInfo rsslErrorInfo;

	/* get current time */
	time(&currentTime); 


	if (currentTime >= rsslConsumerRuntime)
	{
		if (!runTimeExpired)
		{
			runTimeExpired = RSSL_TRUE;
			printf("Run time expired.\n\n");
			
			if (chanCommand.pHandlerTunnelStream != NULL)
				printf("Waiting for tunnel stream to close...\n");


			/* Close tunnel streams if any are open. */
			closeTunnelStreams(chanCommand.reactorChannel, &rsslErrorInfo);		
		}
		else
		{
			if (chanCommand.pHandlerTunnelStream == NULL
					|| currentTime >= rsslConsumerRuntime + 10)
			{
				if (chanCommand.pHandlerTunnelStream != NULL)
					printf("Tunnel stream still open after ten seconds, giving up.\n");
				cleanUpAndExit(-1);
			}
		}
	}
}




void dumpHexBuffer(const RsslBuffer * buffer)
{
      unsigned int i;
      char * position = buffer->data;
      for (i = 0; i < buffer->length; i++, position++)
      {
            if (i % 32 == 0)
            {
                  if (i != 0)
                  {
                        printf("\n");
                  }
            }
            else if ((i != 0) && (i % 2 == 0))
            {
                  printf(" ");
            }
            printf("%2.2X", (unsigned char) *position);
      }
}



/* 
 * configures options based on command line arguments.
 */
void parseCommandLine(int argc, char **argv)
{

	RsslInt32 i;
	

	/* Check usage and retrieve operating parameters */
	{
		ChannelStorage *pCommand = NULL;
		RsslBool hasQueueServiceName = RSSL_FALSE;		
		RsslBool hasConnection = RSSL_FALSE;

		i = 1;

		if (argc == 1)
			printUsageAndExit(argv[0]);

		while(i < argc)
		{
		
			if ((strcmp("-c", argv[i]) == 0))
			{
				char *pToken;				
				
				/* Syntax:
				 *  -tcp hostname:port
				 */

				i += 1;
				if (i >= argc) printUsageAndExit(argv[0]);

				pCommand = &chanCommand;

				/* Hostname */
				pToken = strtok(argv[i], ":");
				if (!pToken) { printf("Error: Missing hostname.\n"); printUsageAndExit(argv[0]); }
				snprintf(pCommand->hostName, MAX_BUFFER_LENGTH, pToken);
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName;

				/* Port */
				pToken = strtok(NULL, ":");
				if (!pToken) { printf("Error: Missing port.\n"); printUsageAndExit(argv[0]); }
				snprintf(pCommand->port, MAX_BUFFER_LENGTH, pToken);
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port;

				pToken = strtok(NULL, ":");
				if (pToken) { printf("Error: extra input after <hostname>:<port>.\n"); printUsageAndExit(argv[0]); }

				i += 1;

				hasConnection = RSSL_TRUE;
				
			}
			else if (strcmp("-runtime", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				timeToRun = atoi(argv[i-1]);
				if (timeToRun == 0)
					timeToRun = 5;
			}
			else if (strcmp("-tsDomain", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsDomain specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				pCommand->tunnelStreamDomain = atoi(argv[i-1]);
			}
			else if (strcmp("-tsAuth", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsAuth specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 1;
				pCommand->useAuthentication = RSSL_TRUE;
			}
			else if (strcmp("-qServiceName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-qServiceName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				hasQueueServiceName = RSSL_TRUE;
				snprintf(pCommand->queueServiceName, sizeof(pCommand->queueServiceName), argv[i-1]);
				snprintf(pCommand->serviceName, sizeof(pCommand->serviceName), pCommand->queueServiceName);
			}
			else if (strcmp("-qSourceName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-qSourceName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				pCommand->sourceName.data = argv[i-1];
				pCommand->sourceName.length = (RsslUInt32)strlen(pCommand->sourceName.data);				
			}
			else if (strcmp("-qDestName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-qDestName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);

				if (pCommand->destNameCount == MAX_DEST_NAMES)
				{
					printf("Error: Example only supports %d queue destination names.\n", MAX_DEST_NAMES);
					printUsageAndExit(argv[0]);
				}

				pCommand->destNames[pCommand->destNameCount].data = argv[i-1];
				pCommand->destNames[pCommand->destNameCount].length = (RsslUInt32)strlen(pCommand->destNames[pCommand->destNameCount].data);				
				++pCommand->destNameCount;
			}
			else if(strcmp("-uname", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				userName.length = snprintf(userNameBlock, sizeof(userNameBlock), "%s", argv[i-1]);
				userName.data = userNameBlock;
			}
			else if (strcmp("-?", argv[i]) == 0)
			{
				printUsageAndExit(argv[0]);
			}
			else if (strcmp("-x", argv[i]) == 0)
			{
				i += 1;
				xmlTrace = RSSL_TRUE;
				snprintf(traceOutputFile, 128, "RsslVAQueueConsumer\0");
			}
			else
			{
				printf("Unknown option: %s\n", argv[i]);
				printUsageAndExit(argv[0]);
			}

			/* Check channel-specific options. */
			if (pCommand != NULL && (i >= argc || strcmp("-c", argv[i]) == 0))
			{
				/* Ensure queue source is specified if queue destinations are specified. */
				if (pCommand->sourceName.length == 0)
				{
					printf("Error: No source queue specified.\n");
					printUsageAndExit(argv[0]);
				}

				/* If service not specified for queue messaging, use the service given for other items instead. */
				if (hasQueueServiceName == RSSL_FALSE)
				{					
					printf("Error: Missing Queue Service name.\n");
					printUsageAndExit(argv[0]);
				}
			}
		}

		if (!hasConnection || !hasQueueServiceName || pCommand->sourceName.length == 0)
		{
			printf("Error: Missing Queue Service, Queue Source Name, or conneciton information.\n");
			printUsageAndExit(argv[0]);
		}

	}

}

/* 
 * Closes a channel in the RsslReactor
 */
void closeConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, ChannelStorage *pCommand)
{
	RsslErrorInfo rsslErrorInfo;

	if (pChannel->socketId != REACTOR_INVALID_SOCKET)
	{
		FD_CLR(pChannel->socketId, &readFds);
		FD_CLR(pChannel->socketId, &exceptFds);
	}

	if (rsslReactorCloseChannel(pReactor, pChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %s\n", rsslErrorInfo.rsslError.text);
	}

	pCommand->reactorChannelReady = RSSL_FALSE;
	clearChannelStorage(pCommand);
}

/* 
 * Processes events about the state of an RsslReactorChannel.
 */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	ChannelStorage *pCommand = (ChannelStorage*)pReactorChannel->userSpecPtr;

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorConnect() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. This will drive the process of setting up the connection
			 * by exchanging the Login, Directory, and (if not already loaded)Dictionary messages. 
			 * The application will receive the response messages in the appropriate callback
			 * function we specified. */

#ifdef _WIN32
			int rcvBfrSize = 65535;
			int sendBfrSize = 65535;
			RsslErrorInfo rsslErrorInfo;
#endif

			printf("Connection up! Channel fd=%d\n\n", pReactorChannel->socketId);

			/* Set file descriptor. */
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);

			/* Save the channel on our info structure. */
			pCommand->reactorChannel = pReactorChannel;


#ifdef _WIN32
			/* WINDOWS: change size of send/receive buffer since it's small by default */
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
#endif			

			if (xmlTrace) 
			{
				RsslTraceOptions traceOptions;
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			pCommand->reactorChannelReady = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			/* The file descriptor representing the RsslReactorChannel has been changed.
			 * Update our file descriptor sets. */
			printf("Fd change: %d to %d\n", pReactorChannel->oldSocketId, pReactorChannel->socketId);
			FD_CLR(pReactorChannel->oldSocketId, &readFds);
			FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			/* The channel has failed and has gone down.  Print the error, close the channel, and reconnect later. */

			printf("Connection down: Channel fd=%d.\n", pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			/* It is important to make sure that no more interface calls are made using the channel after
			 * calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
			 * to call it inside callback functions. */
			closeConnection(pReactor, pReactorChannel, pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			printf("Connection down, reconnecting.  Channel fd=%d\n", pReactorChannel->socketId);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &readFds);
				FD_CLR(pReactorChannel->socketId, &exceptFds);
			}

			clearChannelStorage(pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("Received warning for Channel fd=%d.\n", pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("Unknown connection event!\n");
			cleanUpAndExit(-1);
		}
	}


	return RSSL_RC_CRET_SUCCESS;
}


/*
 * Processes all RSSL messages that aren't processed by 
 * any domain-specific callback functions.  Responses for
 * items requested by the function are handled here. 
 */
static RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent)
{

	/* We are not expecting any non-queue data in this example */
	printf("Unexpected Content received on channel %d", pChannel->socketId);
	
	return RSSL_RC_CRET_SUCCESS;
}


/*** MAIN ***/
int main(int argc, char **argv)
{
	RsslRet ret;
	RsslCreateReactorOptions reactorOpts;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorOMMConsumerRole consumerRole;
	RsslRDMLoginRequest loginRequest;
	RsslRDMDirectoryRequest dirRequest;
	RsslReactorDispatchOptions dispatchOpts;
	ChannelStorage *pCommand = &chanCommand;
	RsslReactorConnectOptions *pOpts = &pCommand->cOpts;
	RsslReactorConnectInfo *pInfo = &pCommand->cInfo;
	RsslUInt32 i;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}
	
	
	initChannelStorage(&chanCommand);

	/* Initialize parameters from config. */
	parseCommandLine(argc, argv);

	/* Initialize run-time */
	initRuntime();

	/* Initialize the default login request(Use 1 as the Login Stream ID). */
	if (rsslInitDefaultRDMLoginRequest(&loginRequest, 1) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMLoginRequest() failed\n");
		cleanUpAndExit(-1);
	}		

	if (userName.length)
		loginRequest.userName = userName;

	/* Initialize the default directory request(Use 2 as the Directory Stream Id) */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, 2) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		cleanUpAndExit(-1);
	}

	/* Setup callback functions and use them on all connections*/
	rsslClearOMMConsumerRole(&consumerRole);
	consumerRole.loginMsgCallback = loginMsgCallback;
	consumerRole.directoryMsgCallback = directoryMsgCallback;
//	consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	consumerRole.base.channelEventCallback = channelEventCallback;
	consumerRole.base.defaultMsgCallback = defaultMsgCallback;

	/* Set the messages to send when the channel is up */
	consumerRole.pLoginRequest = &loginRequest;
	consumerRole.pDirectoryRequest = &dirRequest;

	printf("Connections:\n");

	/* Print out a summary of the connections and desired items. */



	printf("	%s:%s\n", pCommand->hostName, pCommand->port);
	printf("    QueueService: %s\n", pCommand->queueServiceName);
	printf("    Source Queue: %s\n", pCommand->sourceName.data);
	printf("	Dest Queues:");
	for (i = 0; i < pCommand->destNameCount; ++i)
		printf(" %s", pCommand->destNames[i].data);
	printf("\n\n");


	/* Initialize connection options and try to load dictionaries. */

	

	pCommand->pRole = (RsslReactorChannelRole*)&consumerRole;
	pInfo->rsslConnectOptions.guaranteedOutputBuffers = 500;
	pInfo->rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	pInfo->rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	pInfo->rsslConnectOptions.userSpecPtr = &chanCommand;
	pInfo->initializationTimeout = 5;
	pOpts->reactorConnectionList = pInfo;
	pOpts->connectionCount = 1;
	pOpts->reconnectAttemptLimit = -1;
	pOpts->reconnectMaxDelay = 5000;
	pOpts->reconnectMinDelay = 1000;


	printf("\n");

	/* Create an RsslReactor which will manage our channels. */

	rsslClearCreateReactorOptions(&reactorOpts);

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors 
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pReactor->eventFd, &readFds);

	printf("Adding connection to %s:%s...\n", pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address, pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName );

	if (rsslReactorConnect(pReactor, &pCommand->cOpts, pCommand->pRole, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error rsslReactorConnect(): %s\n", rsslErrorInfo.rsslError.text);
	}

	printf("\n");



	rsslClearReactorDispatchOptions(&dispatchOpts);

	/* Main loop. The loop
	 * calls select() to wait for notification, then calls rsslReactorDispatch(). */
	do
	{
		struct timeval selectTime;
		int dispatchCount = 0;
		fd_set useReadFds = readFds, useExceptFds = exceptFds;
		selectTime.tv_sec = 1; selectTime.tv_usec = 0;

		handleRuntime();

		if (!runTimeExpired)
		{
			if (handleQueueMessaging(pReactor, &chanCommand) != RSSL_RET_SUCCESS)
				cleanUpAndExit(-1);
		}
	

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
			cleanUpAndExit(-1);
		}

		
		/* Call rsslReactorDispatch().  This will handle any events that have occurred on its channels.
		 * If there are events or messages for the application to process, they will be delivered
		 * through the callback functions given by the consumerRole object. 
		 * A return value greater than RSSL_RET_SUCCESS indicates there may be more to process. */
		while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
			cleanUpAndExit(-1);
		}

	} while(ret >= RSSL_RET_SUCCESS);

	cleanUpAndExit(-1);
}







