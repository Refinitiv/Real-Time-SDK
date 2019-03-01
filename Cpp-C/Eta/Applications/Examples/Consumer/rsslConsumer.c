/*
 * This is the main file for the rsslConsumer application. It is
 * a single-threaded client application. The application uses
 * either the operating parameters entered by the user or a default
 * set of parameters. See readme file for usage.
 *
 * The purpose of this application is to obtain level I or level II 
 * data from a provider. First the application initializes the RSSL
 * transport and connects the client. After that, it sends a login
 * request, a source directory request, a dictionary request, and
 * one or more item requests to a provider and appropriately processes
 * the responses. The dictionary request is only made if a dictionary
 * cannot be found in the directory of execution. If the dictionary
 * is found in the directory of execution, then it is loaded directly
 * from the file. The resulting level I or level II data from the
 * provider is displayed onto the console. The application also sends
 * post messages on the first market price stream. This will be sent
 * every 5 seconds. There are 2 types of post messages, one with data
 * and one with message.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#include "rsslConsumer.h"
#include "rsslLoginConsumer.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslYieldCurveHandler.h"
#include "rsslSendMessage.h"
#include "rsslPostHandler.h"


static fd_set	readfds;
static fd_set	exceptfds;
fd_set	wrtfds; /* used by sendMessage() */
static RsslChannel *rsslConsumerChannel = 0;
static char srvrHostname[128];
static char srvrPortNo[128];
static char serviceName[128];
static char itemName[128];
static char interfaceName[128];
static char traceOutputFile[128];
static char proxyHostname[128];
static char proxyPort[128];
static char proxyUserName[128];
static char proxyPasswd[128];
static char proxyDomain[128];
static char libsslName[255];
static char libcryptoName[255];
static char libcurlName[255];
static char authenticationToken[AUTH_TOKEN_LENGTH];
static char authenticationExtended[AUTH_TOKEN_LENGTH];
static char applicationId[128];
static RsslConnectionTypes connType = RSSL_CONN_TYPE_SOCKET;
static RsslConnectionTypes encryptedConnType = RSSL_CONN_TYPE_INIT;
static RsslEncryptionProtocolTypes tlsProtocol = RSSL_ENC_NONE;
static char sslCAStore[255];
static RsslUInt32 pingTimeoutServer = 0;
static RsslUInt32 pingTimeoutClient = 0;
static time_t nextReceivePingTime = 0;
static time_t nextSendPingTime = 0;
static RsslBool receivedServerMsg = RSSL_FALSE;
static RsslInt32 timeToRun = 600;
static time_t rsslConsumerRuntime = 0;
static RsslBool shouldRecoverConnection = RSSL_TRUE;
static RsslBool connectionRecovery = RSSL_FALSE;
static RsslBool onPostEnabled = RSSL_FALSE, offPostEnabled = RSSL_FALSE;
static RsslBool postInit = RSSL_FALSE;
static RsslBool ycItemReq = RSSL_FALSE;
static RsslBool mpItemReq = RSSL_FALSE;
static RsslBool mboItemReq = RSSL_FALSE;
static RsslBool mbpItemReq = RSSL_FALSE;
static RsslBool slReq = RSSL_FALSE;
static RsslBool isInLoginSuspectState = RSSL_FALSE;

static RsslBool xmlTrace = RSSL_FALSE;
RsslBool showTransportDetails = RSSL_FALSE;
static RsslReadOutArgs readOutArgs;

/* default server host name */
static const char *defaultSrvrHostname = "localhost";
/* default server port number */
static const char *defaultSrvrPortNo = "14002";
/* default service name */
static const char *defaultServiceName = "DIRECT_FEED";
/* default item name */
static const char *defaultItemName = "TRI";
/* default interface name */
static const char *defaultInterface = "";

static const char *defaultProxyHost = "";
static const char *defaultProxyPort = "";
static const char *defaultProxyUserName = "";
static const char *defaultProxyPasswd = "";
static const char *defaultProxyDomain = "";
static const char *defaultCAStore = "";

/* For TREP authentication login reissue */
static RsslUInt loginReissueTime; // represented by epoch time in seconds
static RsslBool canSendLoginReissue;
static RsslBool isLoginReissue;

int main(int argc, char **argv)
{
	struct timeval time_interval;
	RsslError error;
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	int selRet;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslRet	retval = 0;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	int i;
	RsslInitializeExOpts initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
	time_t currentTime;
#ifdef _WIN32
	int rcvBfrSize = 65535;
	int sendBfrSize = 65535;
#endif
	snprintf(srvrHostname, 128, "%s", defaultSrvrHostname);
	snprintf(srvrPortNo, 128, "%s", defaultSrvrPortNo);
	snprintf(serviceName, 128, "%s", defaultServiceName);
	snprintf(interfaceName, 128, "%s", defaultInterface);
	setUsername((char *)"");
	setAuthenticationToken((char *)"");
	setAuthenticationExtended((char *)"");
	setApplicationId((char *)"");
	snprintf(proxyHostname, 128, "%s", defaultProxyHost);
	snprintf(proxyPort, 128, "%s", defaultProxyPort);
	snprintf(proxyUserName, 128, "%s", defaultProxyUserName);
	snprintf(proxyPasswd, 128, "%s", defaultProxyPasswd);
	snprintf(proxyDomain, 128, "%s", defaultProxyDomain);
	snprintf(sslCAStore, 255, "%s", defaultCAStore);
	tlsProtocol = RSSL_ENC_NONE;

    /* Check usage and retrieve operating parameters */
	if (argc == 1) /* use default operating parameters */
	{
		snprintf(itemName, 128, "%s", defaultItemName);
		/* add item name in market price handler */
		addMarketPriceItemName(itemName, RSSL_FALSE);
		mpItemReq = RSSL_TRUE;

		printf("\nUsing default operating parameters...\n\n");
		printf("srvrHostname: %s\n", srvrHostname);
		printf("srvrPortNo: %s\n", srvrPortNo);
		printf("serviceName: %s\n", serviceName);
		printf("itemName: %s\n", itemName);
		printf("connType: %d\n", connType);
	}
	else if (argc > 1) /* all operating parameters entered by user */
	{
		i = 1;

		while(i < argc)
		{
			if(strcmp("-libsslName", argv[i]) == 0)
			{
				i += 2;
				snprintf(libsslName, 255, "%s", argv[i-1]);
				initOpts.jitOpts.libsslName = libsslName;
			}
			else if(strcmp("-libcryptoName", argv[i]) == 0)
			{
				i += 2;
				snprintf(libcryptoName, 255, "%s", argv[i-1]);
				initOpts.jitOpts.libcryptoName = libcryptoName;
			}
			else if (strcmp("-libcurlName", argv[i]) == 0)
			{
				i += 2;
				snprintf(libcurlName, 255, "%s", argv[i - 1]);
				initOpts.jitOpts.libcurlName = libcurlName;
			}
			else if(strcmp("-uname", argv[i]) == 0)
			{
				i += 2;
				setUsername(argv[i-1]);
			}
			else if(strcmp("-at", argv[i]) == 0)
			{
				i += 2;
				setAuthenticationToken(argv[i-1]);
			}
			else if(strcmp("-ax", argv[i]) == 0)
			{
				i += 2;
				setAuthenticationExtended(argv[i-1]);
			}
			else if(strcmp("-aid", argv[i]) == 0)
			{
				i += 2;
				setApplicationId(argv[i-1]);
			}
			else if(strcmp("-h", argv[i]) == 0)
			{
				i += 2;
				snprintf(srvrHostname, 128, "%s", argv[i-1]);
			}
			else if(strcmp("-p", argv[i]) == 0)
			{
				i += 2;
				snprintf(srvrPortNo, 128, "%s", argv[i-1]);
			}
			else if(strcmp("-ph", argv[i]) == 0)
			{
				i += 2;
				snprintf(proxyHostname, 128, "%s", argv[i-1]);
			}
			else if(strcmp("-pp", argv[i]) == 0)
			{
				i += 2;
				snprintf(proxyPort, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-plogin", argv[i]) == 0)
			{
				i += 2;
				snprintf(proxyUserName, 128, "%s", argv[i - 1]);
			}
			else if (strcmp("-ppasswd", argv[i]) == 0)
			{
				i += 2;
				snprintf(proxyPasswd, 128, "%s", argv[i - 1]);
			}
			else if (strcmp("-pdomain", argv[i]) == 0)
			{
				i += 2;
				snprintf(proxyDomain, 128, "%s", argv[i - 1]);
			}
			else if (strcmp("-spTLSv1", argv[i]) == 0)
			{
				i++;
				tlsProtocol |= RSSL_ENC_TLSV1;
			}
			else if (strcmp("-spTLSv1.1", argv[i]) == 0)
			{
				i++;
				tlsProtocol |= RSSL_ENC_TLSV1_1;
			}
			else if (strcmp("-spTLSv1.2", argv[i]) == 0)
			{
				i++;
				tlsProtocol |= RSSL_ENC_TLSV1_2;
			}
			else if (strcmp("-castore", argv[i]) == 0)
			{
				i += 2;
				snprintf(sslCAStore, 255, "%s", argv[i - 1]);
			}
			else if(strcmp("-s", argv[i]) == 0)
			{
				i += 2;
				snprintf(serviceName, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-c", argv[i]) == 0)
			{
				i += 1;				
				if (0 == strcmp(argv[i], "socket") || 0 == strcmp(argv[i], "0"))
					connType = RSSL_CONN_TYPE_SOCKET;
				else if (0 == strcmp(argv[i], "http") || 0 == strcmp(argv[i], "2"))
					connType = RSSL_CONN_TYPE_HTTP;
				else if (0 == strcmp(argv[i], "encrypted") || 0 == strcmp(argv[i], "1"))
					connType = RSSL_CONN_TYPE_ENCRYPTED;
				else if (0 == strcmp(argv[i], "reliableMCast") || 0 == strcmp(argv[i], "4"))
					connType = RSSL_CONN_TYPE_RELIABLE_MCAST;
				else 
				{
					connType = (RsslConnectionTypes)atoi(argv[i]);	
				}
				i += 1;
			}
			else if (strcmp("-ec", argv[i]) == 0)
			{
				i += 1;
				if (0 == strcmp(argv[i], "socket") || 0 == strcmp(argv[i], "0"))
					encryptedConnType = RSSL_CONN_TYPE_SOCKET;
				else if (0 == strcmp(argv[i], "http") || 0 == strcmp(argv[i], "2"))
					encryptedConnType = RSSL_CONN_TYPE_HTTP;
				else
				{
					encryptedConnType = (RsslConnectionTypes)atoi(argv[i]);
				}
				i += 1;
			}
			else if (strcmp("-yc", argv[i]) == 0)
			{
				i += 2;
				/* add item name in yield curve handler */
				addYieldCurveItemName(argv[i-1], RSSL_FALSE);
				ycItemReq = RSSL_TRUE;
			}
			else if(strcmp("-mp", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market price handler */
				addMarketPriceItemName(argv[i-1], RSSL_FALSE);	
				mpItemReq = RSSL_TRUE;
				
			}
			else if(strcmp("-mbo", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market by order handler */
				addMarketByOrderItemName(argv[i-1], RSSL_FALSE);
				mboItemReq = RSSL_TRUE;
			}
			else if(strcmp("-mbp", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market by price handler */
				addMarketByPriceItemName(argv[i-1], RSSL_FALSE);	
				mbpItemReq = RSSL_TRUE;
			}
			else if(strcmp("-sl", argv[i]) == 0)
			{
				i++;
				slReq = RSSL_TRUE;
				/* check the rest of the command line arguments */
				if(i != argc)
				{
					/* check to see if next strings is the symbol list name */
					if(!(strncmp("-", argv[i], 1)))
					{
						/* user didn't specifiy name */
						setSymbolListInfo(RSSL_FALSE, slReq);
					}
					else
					{
						i++;
						setSymbolListName(argv[i-1]);
						setSymbolListInfo(RSSL_TRUE, slReq);
					}
				}
				else
				{
					/* no more command line arguments, so user didn't specifiy SL name */
					setSymbolListInfo(RSSL_FALSE, slReq);
				}
			}
			else if(strcmp("-view", argv[i]) == 0)
			{
				i++;
				setViewRequest();
			}
			else if (strcmp("-post", argv[i]) == 0)
			{
				i++;
				onPostEnabled = RSSL_TRUE;
				enableOnstreamPost();
			}
			else if (strcmp("-x", argv[i]) == 0)
			{
				i++;
				xmlTrace = RSSL_TRUE;
				snprintf(traceOutputFile, 128, "RsslConsumer\0");
			}
			else if (strcmp("-td", argv[i]) == 0)
			{
				i++;
				showTransportDetails = RSSL_TRUE;
			}
			else if (strcmp("-offpost", argv[i]) == 0)
			{
				i++;
				offPostEnabled = RSSL_TRUE;
			}
			else if(strcmp("-snapshot", argv[i]) == 0)
			{
				i++;
				setMPSnapshotRequest();
				setMBOSnapshotRequest();
				setMBPSnapshotRequest();
				setSLSnapshotRequest();
				setYCSnapshotRequest();
			}
			else if(strcmp("-mpps", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market price handler */
				addMarketPriceItemName(argv[i-1], RSSL_TRUE);	
				mpItemReq = RSSL_TRUE;
				
			}
			else if(strcmp("-mbops", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market by order handler */
				addMarketByOrderItemName(argv[i-1], RSSL_TRUE);
				mboItemReq = RSSL_TRUE;
			}
			else if(strcmp("-mbpps", argv[i]) == 0)
			{
				i += 2;
				/* add item name in market by price handler */
				addMarketByPriceItemName(argv[i-1], RSSL_TRUE);	
				mbpItemReq = RSSL_TRUE;
			}
			else if (strcmp("-ycps", argv[i]) == 0)
			{
				i += 2;
				/* add item name in yield curve handler */
				addYieldCurveItemName(argv[i-1], RSSL_TRUE);
				ycItemReq = RSSL_TRUE;
			}
			else if(strcmp("-runtime", argv[i]) == 0)
			{
				i += 2;
				timeToRun = atoi(argv[i-1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-uname <LoginUsername>] [-h <SrvrHostname>] [-p <SrvrPortNo>] [-c <ConnType>] [-s <ServiceName>] [-view] [-post] [-offpost] [-snapshot] [-sl [<SymbolList Name>]] [-mp|-mpps <MarketPrice ItemName>] [-mbo|-mbops <MarketByOrder ItemName>] [-mbp|-mbpps <MarketByPrice ItemName>] [-runtime <seconds>] [-td]\n", argv[0], argv[0]);
				printf("\n -mp or -mpps For each occurance, requests item using Market Price domain. (-mpps for private stream)\n");
				printf("\n -mbo or -mbops For each occurance, requests item using Market By Order domain. (-mbops for private stream)\n");
				printf("\n -mbp or -mbpps For each occurance, requests item using Market By Price domain. (-mbpps for private stream)\n");
				printf("\n -yc or -ycps For each occurance, requests item using Yield Curve domain. (-ycps for private stream)\n");
				printf("\n -view specifies each request using a basic dynamic view.\n");
				printf("\n -post specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream)\n");
				printf("\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n");
				printf("\n -snapshot specifies each request using non-streaming.\n");
				printf("\n -sl requests symbol list using Symbol List domain. (symbol list name optional)\n");
				printf("\n -td prints out additional transport details from rsslReadEx() and rsslWriteEx() function calls \n");
				printf("\n -x provides an XML trace of messages.\n");
				printf("\n -at Specifies the Authentication Token. If this is present, the login user name type will be RDM_LOGIN_USER_AUTHN_TOKEN.\n");
				printf("\n -ax Specifies the Authentication Extended information.\n");
				printf("\n -aid Specifies the Application ID.\n");
				printf("\n -runtime adjusts the time the application runs.\n");
#ifdef _WIN32
				/* WINDOWS: wait for user to enter something before exiting  */
				printf("\nPress Enter or Return key to exit application:");
				getchar();
#endif
				exit(RSSL_RET_FAILURE);

			}


		}
		
		printf("Proxy host: %s\n", proxyHostname);
		printf("Proxy port: %s\n", proxyPort);
		
		printf("\nInput arguments...\n\n");
		printf("Using Connection Type = %d\n", connType);
		printf("srvrHostname: %s\n", srvrHostname);
		printf("srvrPortNo: %s\n", srvrPortNo);
			

		printf("serviceName: %s\n", serviceName);

		/* if no items were requested but a command line was specified, use the default item */
		if (!mpItemReq && !mboItemReq && !mbpItemReq && !slReq && !ycItemReq)
		{
			snprintf(itemName, 128, "%s", defaultItemName);
			/* add item name in market price handler */
			addMarketPriceItemName(itemName, RSSL_FALSE);
			mpItemReq = RSSL_TRUE;
		}

		/* this application requires at least one market price item to be requested
		   for on-stream posting to be performed */
		if (onPostEnabled && !mpItemReq)
		{
			printf("\nPosting will not be performed as no Market Price items were requested\n");
			onPostEnabled = RSSL_FALSE;
		}
	}

	/* set service name in directory handler */
	setServiceName(serviceName);

	/* load dictionary */
	loadDictionary();


	/* Initialize RSSL */
	/* RSSL_LOCK_NONE is used since this is a single threaded application. */
	if (rsslInitializeEx(&initOpts, &error) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", error.text);
		/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
		exit(RSSL_RET_FAILURE);
	}

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_ZERO(&wrtfds);

	/* Initialize run-time */
	initRuntime();

	/* this is the main loop */
	while(1)
	{
		/* this is the connection recovery loop */
		while(shouldRecoverConnection)
		{
			int first = 0;
			/* Connect to RSSL server */
			printf("\nAttempting to connect to server %s:%s...\n", srvrHostname, srvrPortNo);
			if ((rsslConsumerChannel = connectToRsslServer(connType, &error)) == NULL)
			{
				printf("Unable to connect to RSSL server: <%s>\n",error.text);
			}
			else
			{
				FD_SET(rsslConsumerChannel->socketId, &readfds);
				FD_SET(rsslConsumerChannel->socketId, &wrtfds);
				FD_SET(rsslConsumerChannel->socketId, &exceptfds);
			}

			if (rsslConsumerChannel != NULL && rsslConsumerChannel->state == RSSL_CH_STATE_ACTIVE)
				shouldRecoverConnection = RSSL_FALSE;

			first = 1;
			/* Wait for channel to become active.  This finalizes the three-way handshake. */
			while (rsslConsumerChannel != NULL && rsslConsumerChannel->state != RSSL_CH_STATE_ACTIVE)
			{
				useRead = readfds;
				useWrt = wrtfds;
				useExcept = exceptfds;

				/* Set a timeout value if the provider accepts the connection, but does not initialize it */
				time_interval.tv_sec = 60;
				time_interval.tv_usec = 0;

				if (first == 1) {
					selRet = select(FD_SETSIZE, &useRead, NULL, &useExcept, &time_interval);
					first = 0;
				}
				else {
					selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
				}


				/* select has timed out, close the channel and attempt to reconnect */
				if(selRet == 0)
				{
					printf("\nChannel initialization has timed out, attempting to reconnect...\n");

					FD_CLR(rsslConsumerChannel->socketId, &readfds);
					FD_CLR(rsslConsumerChannel->socketId, &exceptfds);
					if (FD_ISSET(rsslConsumerChannel->socketId, &wrtfds))
						FD_CLR(rsslConsumerChannel->socketId, &wrtfds);
					
					recoverConnection();
				}
				else
				/* Received a response from the provider. */
				if(selRet > 0 && (FD_ISSET(rsslConsumerChannel->socketId, &useRead) || FD_ISSET(rsslConsumerChannel->socketId, &useWrt) || FD_ISSET(rsslConsumerChannel->socketId, &useExcept)))
				{
					if (rsslConsumerChannel->state == RSSL_CH_STATE_INITIALIZING)
					{
						FD_CLR(rsslConsumerChannel->socketId,&wrtfds);
						if ((retval = rsslInitChannel(rsslConsumerChannel, &inProg, &error)) < RSSL_RET_SUCCESS)
						{
							printf("\nchannelInactive fd="SOCKET_PRINT_TYPE" <%s>\n",
								rsslConsumerChannel->socketId,error.text);
							recoverConnection();
							break; 
						}
						else 
						{
			  				switch (retval)
							{
							case RSSL_RET_CHAN_INIT_IN_PROGRESS:
								if (inProg.flags & RSSL_IP_FD_CHANGE)
								{
									printf("\nChannel In Progress - New FD: "SOCKET_PRINT_TYPE"  Old FD: "SOCKET_PRINT_TYPE"\n",rsslConsumerChannel->socketId, inProg.oldSocket );

									FD_CLR(inProg.oldSocket,&readfds);
									FD_CLR(inProg.oldSocket,&exceptfds);
									FD_SET(rsslConsumerChannel->socketId,&readfds);
									FD_SET(rsslConsumerChannel->socketId,&exceptfds);
									FD_SET(rsslConsumerChannel->socketId,&wrtfds);
								}
								else
								{
									printf("\nChannel "SOCKET_PRINT_TYPE" In Progress...\n", rsslConsumerChannel->socketId);
									//Sleep(1000);
								}
								break;
							case RSSL_RET_SUCCESS:
								{
									RsslChannelInfo chanInfo;
									
									printf("\nChannel "SOCKET_PRINT_TYPE" Is Active\n" ,rsslConsumerChannel->socketId);
									/* reset should recover connection flag */
									shouldRecoverConnection = RSSL_FALSE;
									/* if device we connect to supports connected component versioning, 
									 * also display the product version of what this connection is to */
									if ((retval = rsslGetChannelInfo(rsslConsumerChannel, &chanInfo, &error)) >= RSSL_RET_SUCCESS)
									{
										RsslUInt32 i;
										for (i = 0; i < chanInfo.componentInfoCount; i++)
										{
											printf("Connected to %s device.\n", chanInfo.componentInfo[i]->componentVersion.data);
										}
									}
										
								}
								break;
							default:
								printf("\nBad return value fd="SOCKET_PRINT_TYPE" <%s>\n",
									   rsslConsumerChannel->socketId,error.text);
								cleanUpAndExit();
								break;
							}
						}
					}
				}
				else
				if(selRet < 0)
				{
					printf("\nSelect error.\n");
					cleanUpAndExit();
				}

				handleRuntime();
			}

			/* sleep before trying again */
			if (shouldRecoverConnection)
			{
				for(i = 0; i < CONSUMER_CONNECTION_RETRY_TIME; ++i)
				{
#ifdef _WIN32
					Sleep(1000);
#else
					sleep(1);
#endif
					handleRuntime();

				}
			}

		}

		/* WINDOWS: change size of send/receive buffer since it's small by default */
#ifdef _WIN32
		if (rsslIoctl(rsslConsumerChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &error) != RSSL_RET_SUCCESS)
		{
			printf("rsslIoctl(): failed <%s>\n", error.text);
		}
		if (rsslIoctl(rsslConsumerChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &error) != RSSL_RET_SUCCESS)
		{
			printf("rsslIoctl(): failed <%s>\n", error.text);
		}
#endif

		/* Initialize ping handler */
		initPingHandler(rsslConsumerChannel);

		/* Send login request message */
		isLoginReissue = RSSL_FALSE;
		if (sendLoginRequest(rsslConsumerChannel, "rsslConsumer", RSSL_CONSUMER, &loginSuccessCallBack) != RSSL_RET_SUCCESS)
		{
			cleanUpAndExit();
		}
		
	
		/* this is the message processing loop */
		while(1)
		{
			useRead = readfds;
			useExcept = exceptfds;
			useWrt = wrtfds;
			time_interval.tv_sec = 1;
			time_interval.tv_usec = 0;

			/* Call select() to check for any messages */
			selRet = select(FD_SETSIZE,&useRead,
				&useWrt,&useExcept,&time_interval);

			if (selRet < 0) /* no messages received, continue */
			{
#ifdef _WIN32
				if (WSAGetLastError() == WSAEINTR)
					continue;
#else
				if (errno == EINTR)
				{
					continue;
				}
#endif
			}
			else if (selRet > 0) /* messages received, read from channel */
			{
				if ((rsslConsumerChannel != NULL) && (rsslConsumerChannel->socketId != -1))
				{
					if ((FD_ISSET(rsslConsumerChannel->socketId, &useRead)) ||
						(FD_ISSET(rsslConsumerChannel->socketId, &useExcept)))
					{
						if (readFromChannel(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							recoverConnection();
					}

					/* flush for write file descriptor and active state */
					if (rsslConsumerChannel != NULL &&
						FD_ISSET(rsslConsumerChannel->socketId, &useWrt) &&
						rsslConsumerChannel->state == RSSL_CH_STATE_ACTIVE)
					{
						if ((retval = rsslFlush(rsslConsumerChannel, &error)) < RSSL_RET_SUCCESS)
						{
							printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
						}
						else if (retval == RSSL_RET_SUCCESS)
						{
							/* clear write fd */
							FD_CLR(rsslConsumerChannel->socketId, &wrtfds);
						}
					}
				}
			}

			/* break out of message processing loop if should recover connection */
			if (shouldRecoverConnection == RSSL_TRUE)
			{
				printf("Recovering connection in 5 seconds\n");
				connectionRecovery = RSSL_TRUE;
				#ifdef _WIN32
					Sleep(5000);
#else
					sleep(5);
#endif
				break;
			}

			/* Handle pings */
			handlePings(rsslConsumerChannel);

			/* Handle run-time */
			handleRuntime();

			if (onPostEnabled || offPostEnabled)
			{
				/* if configured to post, send post messages if its time */
				/* Handle Post */
				if (handlePosts(rsslConsumerChannel, connectionRecovery) != RSSL_RET_SUCCESS)
					recoverConnection();
				connectionRecovery = RSSL_FALSE;
			}

			if ((currentTime = time(NULL)) < 0)
			{
				printf("time() failed.\n");
			}

			// send login reissue if login reissue time has passed
			if (canSendLoginReissue == RSSL_TRUE &&
				currentTime >= (RsslInt)loginReissueTime)
			{
				isLoginReissue = RSSL_TRUE;
				if (sendLoginRequest(rsslConsumerChannel, "rsslConsumer", RSSL_CONSUMER, &loginSuccessCallBack) != RSSL_RET_SUCCESS)
				{
					printf("Login reissue failed\n");
				}
				else
				{
					printf("Login reissue sent\n");
				}
				canSendLoginReissue = RSSL_FALSE;
			}
		}
	}
}



/*
 * Reads from a channel.
 * chnl - The channel to be read from
 */
static RsslRet readFromChannel(RsslChannel* chnl)
{
	RsslError		error;
	RsslBuffer *msgBuf=0;
	RsslRet	readret;
	RsslReadInArgs	readInArgs;

	if(chnl->socketId != -1 && chnl->state == RSSL_CH_STATE_ACTIVE)
	{
		readret = 1;
		while (readret > 0) /* read until no more to read */
		{
			rsslClearReadInArgs(&readInArgs);
			if ((msgBuf = rsslReadEx(chnl, &readInArgs, &readOutArgs, &readret,&error)) != 0)
			{
				if (showTransportDetails)
				{
					printf("\nMessage Details:\n");
					printf("Cumulative bytesRead=%d\n", readOutArgs.bytesRead);
					printf("Cumulative uncompressedBytesRead=%d\n", readOutArgs.uncompressedBytesRead);
				}
				if (processResponse(chnl, msgBuf) == RSSL_RET_SUCCESS)	
				{
					/* set flag for server message received */
					receivedServerMsg = RSSL_TRUE;
				}
				else
					return RSSL_RET_FAILURE;
			}
			else
			{
				switch (readret)
				{
					case RSSL_RET_CONGESTION_DETECTED:
					case RSSL_RET_SLOW_READER:
					case RSSL_RET_PACKET_GAP_DETECTED:
					{
						if (chnl->state != RSSL_CH_STATE_CLOSED)
						{
							/* disconnectOnGaps must be false.  Connection is not closed */
							printf("\nRead Error: %s <%d>\n", error.text, readret);
							/* break out of switch */
							break;
						}
						/* if channel is closed, we want to fall through */
					}
					case RSSL_RET_FAILURE:
					{
						printf("\nchannelInactive fd="SOCKET_PRINT_TYPE" <%s>\n",
					    chnl->socketId,error.text);
						recoverConnection();
					}
					break;
					case RSSL_RET_READ_FD_CHANGE:
					{
						printf("\nrsslRead() FD Change - Old FD: "SOCKET_PRINT_TYPE" New FD: "SOCKET_PRINT_TYPE"\n", chnl->oldSocketId, chnl->socketId);
						FD_CLR(chnl->oldSocketId, &readfds);
						FD_CLR(chnl->oldSocketId, &exceptfds);
						FD_SET(chnl->socketId, &readfds);
						FD_SET(chnl->socketId, &exceptfds);
					}
					break;
					case RSSL_RET_READ_PING: 
					{
						/* set flag for server message received */
						receivedServerMsg = RSSL_TRUE;
					}
					break;
					default:
						if (readret < 0 && readret != RSSL_RET_READ_WOULD_BLOCK)
						{
							printf("\nRead Error: %s <%d>\n", error.text, readret);
						
						}
					break;
				}
			}
		}
	}
	else if (chnl->state == RSSL_CH_STATE_CLOSED)
	{
		printf("Channel fd="SOCKET_PRINT_TYPE" Closed.\n", chnl->socketId);
		recoverConnection();
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Connects to the RSSL server and returns the channel to the caller.
 * hostname - The hostname of the server to connect to
 * portno - The port number of the server to connect to
 * error - The error information in case of failure
 */
static RsslChannel* connectToRsslServer(RsslConnectionTypes connType, RsslError* error)
{
	RsslChannel* chnl;
	RsslConnectOptions copts = RSSL_INIT_CONNECT_OPTS;		

	printf("\nAttempting to connect to server %s:%s...\n", srvrHostname, srvrPortNo);
	copts.connectionInfo.unified.address = srvrHostname;
	copts.connectionInfo.unified.serviceName = srvrPortNo;
	copts.connectionInfo.unified.interfaceName = interfaceName;
	copts.proxyOpts.proxyHostName = proxyHostname;
	copts.proxyOpts.proxyPort = proxyPort;
	copts.proxyOpts.proxyUserName = proxyUserName;
	copts.proxyOpts.proxyPasswd = proxyPasswd;
	copts.proxyOpts.proxyDomain = proxyDomain;
	copts.encryptionOpts.encryptionProtocolFlags = tlsProtocol;
	copts.encryptionOpts.openSSLCAStore = sslCAStore;

	copts.guaranteedOutputBuffers = 500;
	copts.connectionType = connType;
	if (connType == RSSL_CONN_TYPE_ENCRYPTED && encryptedConnType != RSSL_CONN_TYPE_INIT)
		copts.encryptionOpts.encryptedProtocol = encryptedConnType;
	
	copts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	copts.minorVersion = RSSL_RWF_MINOR_VERSION;
	copts.protocolType = RSSL_RWF_PROTOCOL_TYPE;

	rsslClearReadOutArgs(&readOutArgs);

	if ( (chnl = rsslConnect(&copts,error)) != 0)
	{
		FD_SET(chnl->socketId,&readfds);
		FD_SET(chnl->socketId,&exceptfds);
		if (xmlTrace) 
		{
			int debugFlags = 0x2C0;
			RsslTraceOptions traceOptions;

			rsslClearTraceOptions(&traceOptions);
			traceOptions.traceMsgFileName = traceOutputFile;
			traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_READ | RSSL_TRACE_WRITE;
			traceOptions.traceMsgMaxFileSize = 100000000;
			rsslIoctl(chnl, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, error);
		}

		printf("\nChannel IPC descriptor = "SOCKET_PRINT_TYPE"\n", chnl->socketId);
		if (!copts.blocking)
		{	
			if (!FD_ISSET(chnl->socketId,&wrtfds))
				FD_SET(chnl->socketId,&wrtfds);
		}
	}

	return chnl;
}

/*
 * Initializes the ping times for a channel.
 * chnl - The channel to initialize ping times for
 */
static void initPingHandler(RsslChannel* chnl)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	/* set ping timeout for client and server */
	if ((chnl != NULL) && (chnl->socketId != -1))
	{
		pingTimeoutClient = chnl->pingTimeout/3;
		pingTimeoutServer = chnl->pingTimeout;
		printf("Ping Timeout = %d\n", chnl->pingTimeout);
	}
	else
	{
		pingTimeoutClient = 20;
		pingTimeoutServer = 60;
	}

	/* set time to send next ping from client */
	nextSendPingTime = currentTime + (time_t)pingTimeoutClient;

	/* set time client should receive next message/ping from server */
	nextReceivePingTime = currentTime + (time_t)pingTimeoutServer;
}

/*
 * Initializes the run-time for the rsslConsumer.
 */
static void initRuntime()
{
	time_t currentTime = 0;
	
	/* get current time */
	time(&currentTime);

	rsslConsumerRuntime = currentTime + (time_t)timeToRun;

}

/*
 * Handles the ping processing for a channel.  Sends a ping to the
 * server if the next send ping time has arrived and checks if a
 * ping has been received from the server within the next receive
 * ping time.
 * chnl - The channel to handle pings for
 */
static void handlePings(RsslChannel* chnl)
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	/* handle client pings */
	if (currentTime >= nextSendPingTime)
	{
		/* send ping to server */
		if (sendPing(chnl) != RSSL_RET_SUCCESS)
		{
			cleanUpAndExit();
		}

		/* set time to send next ping from client */
		nextSendPingTime = currentTime + (time_t)pingTimeoutClient;
	}

	/* handle server pings */
	if (currentTime >= nextReceivePingTime)
	{
		/* check if client received message from server since last time */
		if (receivedServerMsg)
		{
			/* reset flag for server message received */
			receivedServerMsg = RSSL_FALSE;

			/* set time client should receive next message/ping from server */
			nextReceivePingTime = currentTime + (time_t)pingTimeoutServer;
		}
		else /* lost contact with server */
		{
			printf("\nLost contact with server...\n");
			cleanUpAndExit();
		}
	}
}

/*
 * Handles the run-time for the rsslConsumer.  Closes all streams
 * for the consumer after run-time has elapsed.
 */
static void handleRuntime()
{
	time_t currentTime = 0;
	RsslRet	retval = 0;
	RsslError error;

	/* get current time */
	time(&currentTime);

	if (currentTime >= rsslConsumerRuntime)
	{
		if (rsslConsumerChannel != NULL && rsslConsumerChannel->socketId != -1 && rsslConsumerChannel->state == RSSL_CH_STATE_ACTIVE)
		{
			/* close all streams */
			closeOffstreamPost(rsslConsumerChannel);
			
			/* close all item streams */
			closeSymbolListStream(rsslConsumerChannel);
			closeMarketPriceItemStreams(rsslConsumerChannel);
			closeMarketByOrderItemStreams(rsslConsumerChannel);
			closeMarketByPriceItemStreams(rsslConsumerChannel);
			closeYieldCurveItemStreams(rsslConsumerChannel);

			/* close dictionary stream */
			if (!isFieldDictionaryLoadedFromFile())
    			closeDictionaryStream(rsslConsumerChannel, FIELD_DICTIONARY_STREAM_ID);
			if (!isEnumTypeDictionaryLoadedFromFile())
    			closeDictionaryStream(rsslConsumerChannel, ENUM_TYPE_DICTIONARY_STREAM_ID);

			/* close source directory stream */
			closeSourceDirectoryStream(rsslConsumerChannel);

			/* close login stream */
			/* note that closing login stream will automatically
			   close all other streams at the provider */
			if (closeLoginStream(rsslConsumerChannel) != RSSL_RET_SUCCESS)
			{
				cleanUpAndExit();
			}

			/* flush before exiting */
			if (FD_ISSET(rsslConsumerChannel->socketId, &wrtfds))
			{
				retval = 1;
				while (retval > RSSL_RET_SUCCESS)
				{
					retval = rsslFlush(rsslConsumerChannel, &error);
				}
				if (retval < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
			}
		}

		printf("\nrsslConsumer run-time expired...\n");
		cleanUpAndExit();
	}
}

/*
 * Processes a response from a channel.  This consists of
 * performing a high level decode of the message and then
 * calling the applicable specific function for further
 * processing.
 * chnl - The channel of the response
 * buffer - The message buffer containing the response
 */
static RsslRet processResponse(RsslChannel* chnl, RsslBuffer* buffer)
{
	RsslRet ret = 0;
	RsslMsg msg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;
	RsslLoginResponseInfo *loginRespInfo = NULL;
	
	/* clear decode iterator */
	rsslClearDecodeIterator(&dIter);
	
	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, chnl->majorVersion, chnl->minorVersion);

	if((ret = rsslSetDecodeIteratorBuffer(&dIter, buffer)) != RSSL_RET_SUCCESS)
	{
		printf("\nrsslSetDecodeIteratorBuffer() failed with return code: %d\n", ret);
		return RSSL_RET_FAILURE;
	}
	ret = rsslDecodeMsg(&dIter, &msg);				
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE" Size %d \n", ret, chnl->socketId, buffer->length);
		return RSSL_RET_FAILURE;
	}

	switch ( msg.msgBase.domainType )
	{
		case RSSL_DMT_LOGIN:
			if (processLoginResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
			{
				if (isLoginStreamClosed())
				{
					return RSSL_RET_FAILURE;
				}
				else if (isLoginStreamClosedRecoverable())
				{
					recoverConnection();
				}
				else if (isLoginStreamSuspect())
				{
					loginRespInfo = getLoginResponseInfo();
					/* if not single open provider, close source directory stream and item streams */
					if (!loginRespInfo->SingleOpen)
					{
						if (closeSourceDirectoryStream(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (closeSymbolListStream(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (closeYieldCurveItemStreams(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (closeMarketPriceItemStreams(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (closeMarketByOrderItemStreams(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;

						if (closeMarketByPriceItemStreams(rsslConsumerChannel) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
					}
					isInLoginSuspectState = RSSL_TRUE;
				}
			}
			else
			{
				if (isInLoginSuspectState)
				{
					isInLoginSuspectState = RSSL_FALSE;
				}
			}
			break;
		case RSSL_DMT_SOURCE:
			if (processSourceDirectoryResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			/* if we have loaded the dictionaries, now that we have the directory, set up posting if its enabled */
			if ((isFieldDictionaryLoaded() || isEnumTypeDictionaryLoaded()) && ((onPostEnabled || offPostEnabled) && !postInit))
			{
				/* Initialize Post Processing after sending the login request message */
				/* ensure that provider supports posting - if not, disable posting */
				RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
					
				if (loginInfo->SupportOMMPost == RSSL_TRUE)
				{
					/* This sets up our basic timing so post messages will be sent periodically */
					initPostHandler();
					/* posting has been initialized */
					postInit = RSSL_TRUE;
				}
				else
				{
					/* provider does not support posting, disable it */
					onPostEnabled = RSSL_FALSE;
					offPostEnabled = RSSL_FALSE;
					disableOnstreamPost();
					disableOffstreamPost();
					printf("\nConnected Provider does not support OMM Posting.  Disabling Post functionality.\n");
				}
			}

			break;
		case RSSL_DMT_DICTIONARY:
			if (processDictionaryResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			/* Now that we have downloaded dictionaries and directory set up posting if its enabled */
			if ((onPostEnabled || offPostEnabled) && !postInit)
			{
				/* Initialize Post Processing after sending the login request message */
				/* ensure that provider supports posting - if not, disable posting */
				RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();
					
				if (loginInfo->SupportOMMPost == RSSL_TRUE)
				{
					/* This sets up our basic timing so post messages will be sent periodically */
					initPostHandler();
					/* posting has been initialized */
					postInit = RSSL_TRUE;
				}
				else
				{
					/* provider does not support posting, disable it */
					onPostEnabled = RSSL_FALSE;
					offPostEnabled = RSSL_FALSE;
					disableOnstreamPost();
					disableOffstreamPost();
					printf("\nConnected Provider does not support OMM Posting.  Disabling Post functionality.\n");
				}
			}

			break;
		case RSSL_DMT_MARKET_PRICE:
			if (!isInLoginSuspectState)
			{
				if (processMarketPriceResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			if (!isInLoginSuspectState)
			{
				if (processMarketByOrderResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_MARKET_BY_PRICE:
			if (!isInLoginSuspectState)
			{
				if (processMarketByPriceResponse(chnl, &msg, &dIter)  != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_YIELD_CURVE:
			if(!isInLoginSuspectState)
			{
				if (processYieldCurveResponse(chnl, &msg, &dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_SYMBOL_LIST:
			if(!isInLoginSuspectState)
			{
				if (processSymbolListResponse(&msg, &dIter) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			break;
		default:
			printf("Unhandled Domain Type: %d\n", msg.msgBase.domainType);
			break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Called upon successful login.  Sends source directory request
 * to the channel.
 * chnl - The channel of the successful login
 */
void loginSuccessCallBack(RsslChannel* chnl)
{
	RsslLoginResponseInfo* loginInfo = getLoginResponseInfo();

	if (isLoginReissue == RSSL_FALSE)
	{
		if (!isInLoginSuspectState || (isInLoginSuspectState && !loginInfo->SingleOpen))
		{
			sendSourceDirectoryRequest(chnl);
			if (offPostEnabled)
			{
				enableOffstreamPost();
			}
		}
	}

	// get login reissue time from authenticationTTReissue
	if (loginInfo->AuthenticationTTReissue != 0)
	{
		loginReissueTime = loginInfo->AuthenticationTTReissue;
		canSendLoginReissue = RSSL_TRUE;
	}
}


/*
 * Removes a channel.
 * chnl - The channel to be removed
 */
void removeChannel(RsslChannel* chnl)
{
	RsslError error;
	RsslRet ret;

	FD_CLR(chnl->socketId, &readfds);
	FD_CLR(chnl->socketId, &exceptfds);
	if (FD_ISSET(chnl->socketId, &wrtfds))
		FD_CLR(chnl->socketId, &wrtfds);

	if ((ret = rsslCloseChannel(chnl, &error)) < RSSL_RET_SUCCESS)
	{
		printf("rsslCloseChannel() failed with return code: %d\n", ret);
	}
}


/*
 * Cleans up and exits the application.
 */
void cleanUpAndExit()
{
	/* clean up channel */
	if ((rsslConsumerChannel != NULL) && (rsslConsumerChannel->socketId != -1))
	{
		removeChannel(rsslConsumerChannel);
	}

	/* free memory for dictionary */
	freeDictionary();
	resetDictionaryStreamId();

	rsslUninitialize();

	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(0);
}

/*
 * Recovers connection in case of connection failure.
 */
void recoverConnection()
{
	/* clean up channel */
	if ((rsslConsumerChannel != NULL) && (rsslConsumerChannel->socketId != -1))
	{
		removeChannel(rsslConsumerChannel);
		rsslConsumerChannel = NULL;
	}
	/* set connection recovery flag */
	shouldRecoverConnection = RSSL_TRUE;

	/* if the dictionary stream IDs are non-zero, it indicates we downloaded the dictionaries.  Because of this, we want to free the memory before recovery 
	since we will download them again upon recovery.  If the stream IDs are zero, it implies no dictionary or dictionary was loaded from file, so we only 
	want to release when we are cleaning up the entire app. */
	if (needToDeleteDictionary())
	{
		/* free memory for dictionary */
		freeDictionary();
		resetDictionaryStreamId();
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
