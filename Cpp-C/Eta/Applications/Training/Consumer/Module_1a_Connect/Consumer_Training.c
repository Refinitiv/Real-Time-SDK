/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.							--
 *| Copyright (C) 2019 Refinitiv. All rights reserved.				--
 *|-------------------------------------------------------------------------------
 */


/*
 * This is the ETA Consumer Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Consumer using the ETA Transport layer.
 *
 * Main c source file for the ETA Consumer Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * ETA Consumer Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * In this module, the application initializes the ETA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * Refinitiv Real-Time Distribution Systems, Refinitiv Data Feed Direct,
 * and Refinitiv Real-Time. 
 *
 * Detailed Descriptions:
 * The first step of any ETA consumer application is to establish a 
 * network connection with its peer component (i.e., another application 
 * with which to interact). User must start a interactive provider (server) 
 * or a non-interactive provider which connects to an ADH. 
 * There are training examples available for Providers and NIProviders.
 * An OMM consumer typically creates an outbound
 * connection to the well-known hostname and port of a server (Interactive 
 * Provider or ADS). The consumer uses the rsslConnect() function to initiate 
 * the connection and then uses the rsslInitChannel() function to complete 
 * channel initialization.(rssl stands for Refinitiv Source Sink Library.
 * ETA stands for “Ultra Performance API.” It was the previous name of the product, 
 * before it was re-branded as ETA “Enterprise Transport API”.)
 *
 *
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

#include "Consumer_Training.h"

int main(int argc, char **argv)
{
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	/* This example uses Unix I/O contents and basic TCP/IP like file descriptor and socket.*/

	/* This example suite uses write descriptor in our client/consumer type examples in mainly 2 areas with
	 * the I/O notification mechanism being used:
	 * Details of those functions could be found in API development guide.

	 * 1) rsslInitChannel() function which exchanges various messages to perform necessary ETA transport
	 *    negotiations and handshakes to complete channel initialization.
	 * 2) rsslFlush() calls used throughout the application (after module 1a), together with rsslWrite() calls, such
	 *    as in sendMessage() function. The write descriptor can be used to indicate when the socketId has write
	 *    availability and help with determining when the network is able to accept additional bytes for writing.
	 *
	 * For the RsslChannel initialization process, if using I/O, a client/consumer application should register the
	 * RsslChannel.socketId with the read, write, and exception file descriptor sets. When the write descriptor
	 * alerts the user that the socketId is ready for writing, rsslInitChannel is called (this sends the
	 * initial connection handshake message). When the read file descriptor alerts the user that the socketId
	 * has data to read, rsslInitChannel is called - this typically reads the next portion of the handshake.
	 * This process would continue until the connection is active.
	 *
	 * Typically, calls to rsslInitChannel are driven by I/O on the connection, however this can also be
	 * accomplished by using a timer to periodically call the function or looping on a call until the channel
	 * transitions to active or a failure occurs. Other than any overhead associated with the function call,
	 * there is no harm in calling rsslInitChannel more frequently than required. If no work is required at
	 * the current time, the function will return and indicate that connection is still in progress.
	 */

	/* This example suite also uses a clean FD sets and a dirty FD sets for I/O notification.

	 *		select() - a system call for examining the status of file_descriptors.
	 *					Tells us that there is data to read on the FDs.

	 * Since select() modifies its file descriptor sets, if the call is being used in a loop, then the fd sets must
	 * be reinitialized before each call. Since they act as input/output parameters for the select() system call;
	 * they are read by and modified by the system call. When select() returns, the values have all been modified
	 * to reflect the set of file descriptors ready. So, every time before you call select(), you have to
	 * (re)initialize the fd_set values. Here we maintain 2 sets FD sets:
	 * a) clean FD sets so that we can repeatedly call select call
	 * b) dirty FD sets used in the actual select call (I/O notification mechanism)
	 * Typically, you reset the dirty FD sets to be equal to the clean FD sets before you call select().
	 */

	/******************************************************************************************************************
				DECLARING VARIABLES
	******************************************************************************************************************/
	char srvrHostname[128], srvrPortNo[128], interfaceName[128];

	/* clean FD sets so that we can repeatedly call select call */
	fd_set cleanReadFds;
	fd_set cleanExceptFds;
	fd_set cleanWriteFds;

	/* dirty FD sets used in the actual select call (I/O notification mechanism) */
	fd_set useReadFds;
	fd_set useExceptFds;
	fd_set useWriteFds;

	struct timeval time_interval;
	int selRet;
	RsslRet retval = RSSL_RET_FAILURE;

	RsslError error;

	RsslConnectOptions cOpts  = RSSL_INIT_CONNECT_OPTS;

	/* RsslInProgInfo Information for the In Progress Connection State */
	RsslInProgInfo inProgInfo = RSSL_INIT_IN_PROG_INFO;

	/* ETA channel management information */
	EtaChannelManagementInfo etaChannelManagementInfo;

	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	etaChannelManagementInfo.etaChannel = 0;

	/* the default option parameters */
	/* connect to server running on same machine */
	snprintf(srvrHostname, 128, "%s", "localhost");
	/* server is running on port number 14002 */
	snprintf(srvrPortNo, 128, "%s", "14002");
	/* use default NIC network interface card to bind to for all inbound and outbound data */
	snprintf(interfaceName, 128, "%s", "");

	/* User specifies options such as address, port, and interface from the command line.
	 * User can have the flexibilty of specifying any or all of the parameters in any order.
	 */
	if (argc > 1)
	{
		int i = 1;

		while (i < argc)
		{
			if (strcmp("-h", argv[i]) == 0)
			{
				i += 2;
				snprintf(srvrHostname, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-p", argv[i]) == 0)
			{
				i += 2;
				snprintf(srvrPortNo, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-i", argv[i]) == 0)
			{
				i += 2;
				snprintf(interfaceName, 128, "%s", argv[i-1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] \n", argv[0], argv[0]);
				exit(RSSL_RET_FAILURE);
			}
		}
	}

	/******************************************************************************************************************
				INITIALIZATION - USING rsslInitialize()
	******************************************************************************************************************/
	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 1:
	 * Initialize ETA Transport using rsslInitialize
	 * The first ETA Transport function that an application should call. This creates and initializes
	 * internal memory and structures, as well as performing any boot strapping for underlying dependencies.
	 * The rsslInitialize function also allows the user to specify the locking model they want applied
	 * to the ETA Transport.
	 *********************************************************/

	/* RSSL_LOCK_NONE is used since this is a single threaded application.
	 * For applications with other thread models (RSSL_LOCK_GLOBAL_AND_CHANNEL, RSSL_LOCK_GLOBAL),
	 * see the ETA C developers guide for definitions of other locking models supported by ETA
	 */
	if (rsslInitialize(RSSL_LOCK_NONE, &error) != RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslInitialize. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
		/* End application */
		exit(RSSL_RET_FAILURE);
	}

	FD_ZERO(&cleanReadFds);
	FD_ZERO(&cleanExceptFds);
	FD_ZERO(&cleanWriteFds);

	/* populate connect options, then pass to rsslConnect function -
	 * ETA Transport should already be initialized
	 */
	/* use standard socket connection */
	cOpts.connectionType = RSSL_CONN_TYPE_SOCKET; /*!< (0) Channel is a standard TCP socket connection type */
	cOpts.connectionInfo.unified.address = srvrHostname;
	cOpts.connectionInfo.unified.serviceName = srvrPortNo;
	cOpts.connectionInfo.unified.interfaceName = interfaceName;

	/* populate version and protocol with RWF information (found in rsslIterators.h) or protocol specific info */
	cOpts.protocolType = RSSL_RWF_PROTOCOL_TYPE; /* Protocol type definition for RWF */
	cOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	cOpts.minorVersion = RSSL_RWF_MINOR_VERSION;

	/******************************************************************************************************************
				CONNECTION SETUP - USING rsslConnect()
	******************************************************************************************************************/
	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 2:
	 * Connect using rsslConnect (OS connection establishment handshake)
	 * rsslConnect call Establishes an outbound connection, which can leverage standard sockets, HTTP,
	 * or HTTPS. Returns an RsslChannel that represents the connection to the user. In the event of an error,
	 * NULL is returned and additional information can be found in the RsslError structure.
	 * Connection options are passed in via an RsslConnectOptions structure.
	 *********************************************************/

	if ((etaChannelManagementInfo.etaChannel = rsslConnect(&cOpts, &error)) == 0)
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslConnect. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

		/* End application, uninitialize to clean up first */
		rsslUninitialize();
		exit(RSSL_RET_FAILURE);
	}

	/* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
	/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
	FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanReadFds);
	FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanExceptFds);

	/* for non-blocking I/O (default), write descriptor is set initially in case this end starts the message
	 * handshakes that rsslInitChannel() performs. Once rsslInitChannel() is called for the first time the
	 * channel can wait on the read descriptor for more messages. Without using the write descriptor, we would
	 * have to keep looping and calling rsslInitChannel to complete the channel initialization process, which
	 * can be CPU-intensive. We will set the write descriptor again if a FD_CHANGE event occurs.
	 */
	if (!cOpts.blocking)
	{
		if (!FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds))
			FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);
	}

	printf("\nChannel IPC descriptor = "SOCKET_PRINT_TYPE"\n", etaChannelManagementInfo.etaChannel->socketId);

	/******************************************************************************************************************
				MAIN LOOP TO SEE IF RESPONSE RECEIVED FROM PROVIDER
	******************************************************************************************************************/
	/* Main loop for getting connection active and successful completion of the initialization process
	 * The loop calls select() to wait for notification
	 * Currently, the main loop would exit if an error condition is triggered or
	 * RsslChannel.state transitions to RSSL_CH_STATE_ACTIVE.
	 */

	/*
	 *If we want a non-blocking read call to the selector, we use select before read as read is a blocking call but select is not
	 *If we want a blocking read call to the selector, such that we want to wait till we get a message, we should use read without select.
	 *In the program below we will use select(), as it is non-blocking
	 */

	while (etaChannelManagementInfo.etaChannel->state != RSSL_CH_STATE_ACTIVE)
	{
		useReadFds = cleanReadFds;
		useWriteFds = cleanWriteFds;
		useExceptFds = cleanExceptFds;

		/* Set a timeout value if the provider accepts the connection, but does not initialize it */
		/* On Linux platform, select() modifies timeout to reflect the amount of time not slept;
		 * most other implementations do not do this. (POSIX.1-2001 permits either behaviour.)
		 * This causes problems both when Linux code which reads timeout is ported to other operating systems,
		 * and when code is ported to Linux that reuses a struct timeval for multiple select()s
		 * in a loop without reinitializing it. Consider timeout to be undefined after select() returns.
		 *
		 * Note: You should reset the values of your timeout before you call select() every time.
		 */
		time_interval.tv_sec = 60;
		time_interval.tv_usec = 0;

		/* By employing an I/O notification mechanism (e.g. select, poll), an application can leverage a
		 * non-blocking I/O model, using the I/O notification to alert the application when data is available
		 * to read or when output space is available for writing to. The training examples are written from a
		 * non-blocking I/O perspective. Here, we use the select I/O notification mechanism in our examples.
		 */
		selRet = select(FD_SETSIZE, &useReadFds, &useWriteFds, &useExceptFds, &time_interval);

		if (selRet == 0)
		{
			/* select has timed out, close the channel and exit */
			/* On success, select() return zero if the timeout expires before anything interesting happens. */
			printf("\nChannel initialization has timed out, exiting...\n");
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE);
		}
		else if (selRet > 0)
		{
			/* Received a response from the provider. */

			/* On success, select() return the number of file descriptors contained in the three returned descriptor sets
			 * (that is, the total number of bits that are set in readfds, writefds, exceptfds)
			 */

			/* Wait for channel to become active. After an RsslChannel is returned from the client's rsslConnect or server's rsslAccept call,
			 * the channel may need to continue the initialization process. This additional initialization is required
			 * as long as the RsslChannel.state is RSSL_CH_STATE_INITIALIZING. When using non-blocking I/O, this is the
			 * typical state that an RsslChannel will start from and it may require multiple initialization calls to
			 * transition to active. rsslInitChannel is typically called based on activity on the socketId, though a timer or
			 * looping can be used - the rsslInitChannel function should continue to be called until the
			 * connection becomes active, at which point reading and writing can begin.
			 */

			/* Indicates that an RsslChannel requires additional initialization. This initialization is typically additional
			 * connection handshake messages that need to be exchanged.
			 */

			/* rsslInitChannel is called if read or write or except is triggered */
			if (FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useReadFds) || FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useWriteFds) || FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useExceptFds))
			{
				/* Write descriptor is set initially in case this end starts the message handshakes that rsslInitChannel() performs.
				 * Once rsslInitChannel() is called for the first time the channel can wait on the read descriptor for more messages.
				 * We will set the write descriptor again if a FD_CHANGE event occurs. */
				FD_CLR(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);

				/*********************************************************
				 * Client/Consumer Application Lifecycle Major Step 3:
				 * Initialize until active using rsslInitChannel (ETA Transport connection establishment handshake)
				 * Continues initialization of an RsslChannel. This channel could originate from rsslConnect or rsslAccept.
				 * This function exchanges various messages to perform necessary ETA negotiations and handshakes to
				 * complete channel initialization.
				 * Requires the use of an RsslInProgInfo structure.
				 * The RsslChannel can be used for all additional transport functionality (e.g. reading, writing) once the
				 * state transitions to RSSL_CH_STATE_ACTIVE. If a connection is rejected or initialization fails,
				 * the state will transition to RSSL_CH_STATE_CLOSED.
				 *********************************************************/

				/* Internally, the ETA initialization process includes several actions. The initialization includes
				 * any necessary ETA connection handshake exchanges, including any HTTP or HTTPS negotiation.
				 * Compression, ping timeout, and versioning related negotiations also take place during the
				 * initialization process. This process involves exchanging several messages across the connection,
				 * and once all message exchanges have completed the RsslChannel.state will transition. If the connection
				 * is accepted and all types of negotiations completed properly, the RsslChannel.state will become
				 * RSSL_CH_STATE_ACTIVE. If the connection is rejected, either due to some kind of negotiation failure
				 * or because an RsslServer rejected the connection by setting nakMount to RSSL_TRUE, the RsslChannel.state
				 * will become RSSL_CH_STATE_CLOSED.
				 *
				 * Note:
				 * For both client and server channels, more than one call to rsslInitChannel can be required to complete
				 * the channel initialization process.
				 */
				if ((retval = rsslInitChannel(etaChannelManagementInfo.etaChannel, &inProgInfo, &error)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) (errno: %d) encountered with rsslInitChannel fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, etaChannelManagementInfo.etaChannel->socketId, error.text);
					/* Closes channel, cleans up and exits the application. */
					closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE);
					break;
				}
				else
				{
					/* Handle return code appropriately */
			  		switch (retval)
					{
						/*!< (2)  Transport Success: Channel initialization is In progress, returned from rsslInitChannel. */
						case RSSL_RET_CHAN_INIT_IN_PROGRESS:
						{
							/* Initialization is still in progress, check the RsslInProgInfo for additional information */
							if (inProgInfo.flags & RSSL_IP_FD_CHANGE)
							{
								/* The rsslInitChannel function requires the use of an additional parameter, a RsslInProgInfo structure.
								 * Under certain circumstances, the initialization process may be required to create new or additional underlying connections.
								 * If this occurs, the application is required to unregister the previous socketId and register the new socketId with
								 * the I/O notification mechanism being used. When this occurs, the information is conveyed by the RsslInProgInfo and the RsslInProgFlags.
								 *
								 * RSSL_IP_FD_CHANGE indicates that a socketId change has occurred as a result of this call. The previous socketId has been
								 * stored in RsslInProgInfo.oldSocket so it can be unregistered with the I/O notification mechanism.
								 * The new socketId has been stored in RsslInProgInfo.newSocket so it can be registered with the
								 * I/O notification mechanism. The channel initialization is still in progress and subsequent calls
								 * to rsslInitChannel are required to complete it.
								 */
								printf("\nChannel In Progress - New FD: "SOCKET_PRINT_TYPE"  Old FD: "SOCKET_PRINT_TYPE"\n",etaChannelManagementInfo.etaChannel->socketId, inProgInfo.oldSocket );

								/* File descriptor has changed, unregister old and register new */
								FD_CLR(inProgInfo.oldSocket, &cleanReadFds);
								FD_CLR(inProgInfo.oldSocket, &cleanWriteFds);
								FD_CLR(inProgInfo.oldSocket, &cleanExceptFds);
								/* newSocket should equal etaChannelManagementInfo.etaChannel->socketId */
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanReadFds);
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanExceptFds);
							}
							else
							{
								printf("\nChannel "SOCKET_PRINT_TYPE" In Progress...\n", etaChannelManagementInfo.etaChannel->socketId);
							}
						}
						break;

						/* channel connection becomes active!
						 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state,
						 * this RsslChannel can be used for other transport operations.
						 */
						case RSSL_RET_SUCCESS:
						{
							printf("\nChannel on fd "SOCKET_PRINT_TYPE" is now active - reading and writing can begin.\n", etaChannelManagementInfo.etaChannel->socketId);

							/*********************************************************
							 * Connection is now active. The RsslChannel can be used for all additional
							 * transport functionality (e.g. reading, writing) now that the state
							 * transitions to RSSL_CH_STATE_ACTIVE
							 *********************************************************/

							/* After channel is active, use ETA Transport utility function rsslGetChannelInfo to query RsslChannel negotiated
							 * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
							 * compression information as well as many other values.
							 */
							if ((retval = rsslGetChannelInfo(etaChannelManagementInfo.etaChannel, &etaChannelManagementInfo.etaChannelInfo, &error)) != RSSL_RET_SUCCESS)
							{
								printf("Error %s (%d) (errno: %d) encountered with rsslGetChannelInfo. Error Text: %s\n",
									rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

								/* Connection should be closed, return failure */
								/* Closes channel, cleans up and exits the application. */
								closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE);
							}

							printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
								"	Max Fragment Size: %u\n"
								"	Output Buffers: %u Max, %u Guaranteed\n"
								"	Input Buffers: %u\n"
								"	Send/Recv Buffer Sizes: %u/%u\n"
								"	Ping Timeout: %u\n"
								"	Connected component version: ",
								etaChannelManagementInfo.etaChannel->socketId,				/*!< @brief Socket ID of this ETA channel. */
								etaChannelManagementInfo.etaChannelInfo.maxFragmentSize,	/*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */
								etaChannelManagementInfo.etaChannelInfo.maxOutputBuffers,	/*!< @brief This is the maximum number of output buffers available to the channel. */
								etaChannelManagementInfo.etaChannelInfo.guaranteedOutputBuffers, /*!< @brief This is the guaranteed number of output buffers available to the channel. */
								etaChannelManagementInfo.etaChannelInfo.numInputBuffers,	/*!< @brief This is the number of input buffers available to the channel. */
								etaChannelManagementInfo.etaChannelInfo.sysSendBufSize,		/*!< @brief This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
								etaChannelManagementInfo.etaChannelInfo.sysRecvBufSize,		/*!< @brief This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
								etaChannelManagementInfo.etaChannelInfo.pingTimeout 		/*!< @brief This is the value of the negotiated ping timeout */
							);

							if (etaChannelManagementInfo.etaChannelInfo.componentInfoCount == 0)
								printf("(No component info)");
							else
							{
								RsslUInt32 count;
								for(count = 0; count < etaChannelManagementInfo.etaChannelInfo.componentInfoCount; ++count)
								{
									printf("%.*s",
										etaChannelManagementInfo.etaChannelInfo.componentInfo[count]->componentVersion.length,
										etaChannelManagementInfo.etaChannelInfo.componentInfo[count]->componentVersion.data);
									if (count < etaChannelManagementInfo.etaChannelInfo.componentInfoCount - 1)
										printf(", ");
								}
							}
							printf ("\n\n");
						}
						break;

						default: /* Error handling */
						{
							printf("\nBad return value fd="SOCKET_PRINT_TYPE" <%s>\n",
								etaChannelManagementInfo.etaChannel->socketId, error.text);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE);
						}
						break;
					}
				}
			}
		}
		else if (selRet < 0)
		{
			/* On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined */
			printf("\nSelect error.\n");
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE);
		}
	}
}

/*
 * Closes channel, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* etaChannel, int code)
{
	RsslRet	retval = 0;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 5:
	 * Close connection using rsslCloseChannel (OS connection release handshake)
	 * rsslCloseChannel closes the client based RsslChannel. This will release any pool based resources
	 * back to their respective pools, close the connection, and perform any additional necessary cleanup.
	 *********************************************************/

	if ((etaChannel) && (retval = rsslCloseChannel(etaChannel, &error) < RSSL_RET_SUCCESS))
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseChannel. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 6:
	 * Uninitialize ETA Transport using rsslUninitialize
	 * The last ETA Transport function that an application should call. This uninitializes internal data
	 * structures and deletes any allocated memory.
	 *********************************************************/

	/* All ETA Transport use is complete, must uninitialize.
	 * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
	 */
	rsslUninitialize();

	/* For applications that do not exit due to errors/exceptions such as:
	 * Exits the application if the run-time has expired.
	 */
	if (code == RSSL_RET_SUCCESS)
		printf("\nETA Consumer Training application successfully ended.\n");

	/* End application */
	exit(code);
}

