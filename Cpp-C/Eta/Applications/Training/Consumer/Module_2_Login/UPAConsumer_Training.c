/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright (C) 2019 Refinitiv. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */


/*
 * This is the UPA Consumer Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Consumer using the UPA Transport layer.
 *
 * Main c source file for the UPA Consumer Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA Consumer Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * In this module, the application initializes the UPA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * the Enterprise Platform, Data Feed Direct, and Elektron.
 *
 * Detailed Descriptions:
 * The first step of any UPA consumer application is to establish a 
 * network connection with its peer component (i.e., another application 
 * with which to interact). User must start a interactive provider (server) or a 
 * non-interactive provider which connects to an ADH. 
 * There are training examples available for Providers and NIProviders.
 * An OMM consumer typically creates an outbound 
 * connection to the well-known hostname and port of a server (Interactive 
 * Provider or ADS). The consumer uses the rsslConnect() function to initiate 
 * the connection and then uses the rsslInitChannel() function to complete 
 * channel initialization.(rssl stands for Reuters Source Sink Library.
 * UPA stands for “Ultra Performance API.” It was the previous name of the product, 
 * before it was re-branded as ETA “Elektron Transport API”.)
 * 
 *
 ************************************************************************
 * UPA Consumer Training Module 1b: Ping (heartbeat) Management
 ************************************************************************
 * Summary:
 * Ping or heartbeat messages indicate the continued presence of an application. 
 * After the consumer’s connection is active, ping messages must be exchanged. 
 * The negotiated ping timeout is retrieved using the rsslGetChannelInfo() function. 
 * The connection will be terminated if ping heartbeats are not sent or received 
 * within the expected time frame.
 *
 * Detailed Descriptions:
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information 
 * is being exchanged. For example, there may be long periods of time that 
 * elapse between requests made from an OMM consumer application. In this 
 * situation, the consumer would send periodic heartbeat messages to inform 
 * the providing application that it is still alive. Because the provider 
 * application is likely sending more frequent information, providing updates 
 * on any streams the consumer has requested, it may not need to send 
 * heartbeats as the other data is sufficient to announce its continued 
 * presence. It is the responsibility of each connection to manage the sending
 * and receiving of heartbeat messages.
 *
 *
 ************************************************************************
 * UPA Consumer Training Module 1c: Reading and Writing Data
 ************************************************************************
 * Summary:
 * When channel initialization is complete, the state of the channel 
 * (RsslChannel.state) is RSSL_CH_STATE_ACTIVE, and applications can send 
 * and receive data.(Read and Write)
 *
 * Detailed Descriptions:
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to receive data from the connection. The 
 * arrival of this information is often announced by the I/O notification 
 * mechanism that the RsslChannel.socketId is registered with. The UPA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines RsslBuffer boundaries and returns each buffer one by 
 * one.
 * 
 * (Buffer is used since Ping module and all the latter examples.)
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to write data to the connection. Writing 
 * involves a several step process. Because the UPA Transport provides 
 * efficient buffer management, the user is required to obtain a buffer 
 * from the UPA Transport buffer pool. This can be the guaranteed output 
 * buffer pool associated with an RsslChannel. After a buffer is acquired, 
 * the user can populate the RsslBuffer.data and set the RsslBuffer.length 
 * to the number of bytes referred to by data. If queued information cannot 
 * be passed to the network, a function is provided to allow the application 
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able 
 * to accept additional bytes for writing. The UPA Transport can continue to
 * queue data, even if the network is unable to write. 
 * 
 * 
 *
 ************************************************************************
 * UPA Consumer Training Module 2: Log in
 ************************************************************************
 * Summary:
 * Applications authenticate using the Login domain model. An OMM consumer must 
 * authenticate with a provider using a Login request prior to issuing any other 
 * requests or opening any other streams. After receiving a Login request, an 
 * Interactive Provider determines whether a user is permissioned to access the 
 * system. The Interactive Provider sends back a Login response, indicating to 
 * the consumer whether access is granted.
 *
 * Detailed Descriptions:
 * After receiving a Login request, an Interactive Provider determines whether 
 * a user is permissioned to access the system. The Interactive Provider sends 
 * back a Login response, indicating to the consumer whether access is granted.
 * 
 * a) If the application is denied, the Login stream is closed, and the 
 * consumer application cannot send additional requests.
 * b) If the application is granted access, the Login response contains 
 * information about available features, such as Posting, Pause and Resume, 
 * and the use of Dynamic Views. The consumer application can use this 
 * information to tailor its interaction with the provider.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package. 
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

#include "UPAConsumer_Training.h"

int main(int argc, char **argv)
{
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */

	/* This example suite uses write descriptor in our client/consumer type examples in mainly 2 areas with
	 * the I/O notification mechanism being used:
	 * Details of those functions could be found in API development guide.
	 * 1) rsslInitChannel() function which exchanges various messages to perform necessary UPA transport
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
	RsslRet retval_rsslRead = RSSL_RET_FAILURE; /* used exclusively for rsslRead call */

	RsslError error;

	RsslConnectOptions cOpts  = RSSL_INIT_CONNECT_OPTS;

	/* RsslInProgInfo Information for the In Progress Connection State */
	RsslInProgInfo inProgInfo = RSSL_INIT_IN_PROG_INFO;

	/* UPA channel management information */
	UpaChannelManagementInfo upaChannelManagementInfo;

	RsslBuffer* msgBuf = 0;

	time_t currentTime = 0;
	time_t upaRuntime = 0;
	RsslUInt32 runTime = 0;

	/* UPA provides clear functions for its structures (e.g., rsslClearDecodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_DECODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Refinitiv recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	upaChannelManagementInfo.upaChannel = 0;

	/* the default option parameters */
	/* connect to server running on same machine */
	snprintf(srvrHostname, 128, "%s", "localhost");
	/* server is running on port number 14002 */
	snprintf(srvrPortNo, 128, "%s", "14002");
	/* use default NIC network interface card to bind to for all inbound and outbound data */
	snprintf(interfaceName, 128, "%s", "");
	/* use default runTime of 300 seconds */
	runTime = 300;

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
			else if (strcmp("-r", argv[i]) == 0)
			{
				i += 2;
				sscanf(argv[i-1], "%u", &runTime);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] \n", argv[0], argv[0]);
				exit(RSSL_RET_FAILURE);
			}
		}
	}

	/******************************************************************************************************************
				INITIALIZATION - USING rsslInitialize()
	******************************************************************************************************************/
	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 1:
	 * Initialize UPA Transport using rsslInitialize
	 * The first UPA Transport function that an application should call. This creates and initializes
	 * internal memory and structures, as well as performing any boot strapping for underlying dependencies.
	 * The rsslInitialize function also allows the user to specify the locking model they want applied
	 * to the UPA Transport.
	 *********************************************************/

	/* RSSL_LOCK_NONE is used since this is a single threaded application.
	 * For applications with other thread models (RSSL_LOCK_GLOBAL_AND_CHANNEL, RSSL_LOCK_GLOBAL),
	 * see the UPA C developers guide for definitions of other locking models supported by UPA
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

	/* get current time */
	time(&currentTime);

	/* Set the runtime of the UPA Consumer application to be runTime (seconds) */
	upaRuntime = currentTime + (time_t)runTime;

	/* populate connect options, then pass to rsslConnect function -
	 * UPA Transport should already be initialized
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

	if ((upaChannelManagementInfo.upaChannel = rsslConnect(&cOpts, &error)) == 0)
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslConnect. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

		/* End application, uninitialize to clean up first */
		rsslUninitialize();
		exit(RSSL_RET_FAILURE);
	}

	/* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
	/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
	FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanReadFds);
	FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanExceptFds);

	/* for non-blocking I/O (default), write descriptor is set initially in case this end starts the message
	 * handshakes that rsslInitChannel() performs. Once rsslInitChannel() is called for the first time the
	 * channel can wait on the read descriptor for more messages. Without using the write descriptor, we would
	 * have to keep looping and calling rsslInitChannel to complete the channel initialization process, which
	 * can be CPU-intensive. We will set the write descriptor again if a FD_CHANGE event occurs.
	 */
	if (!cOpts.blocking)
	{
		if (!FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds))
			FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
	}

	printf("\nChannel IPC descriptor = "SOCKET_PRINT_TYPE"\n", upaChannelManagementInfo.upaChannel->socketId);

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

	while (upaChannelManagementInfo.upaChannel->state != RSSL_CH_STATE_ACTIVE)
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
			closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
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
			if (FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useReadFds) || FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useWriteFds) || FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useExceptFds))
			{
				/* Write descriptor is set initially in case this end starts the message handshakes that rsslInitChannel() performs.
				 * Once rsslInitChannel() is called for the first time the channel can wait on the read descriptor for more messages.
				 * We will set the write descriptor again if a FD_CHANGE event occurs. */
				FD_CLR(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);

				/*********************************************************
				 * Client/Consumer Application Lifecycle Major Step 3:
				 * Initialize until active using rsslInitChannel (UPA Transport connection establishment handshake)
				 * Continues initialization of an RsslChannel. This channel could originate from rsslConnect or rsslAccept.
				 * This function exchanges various messages to perform necessary UPA negotiations and handshakes to
				 * complete channel initialization.
				 * Requires the use of an RsslInProgInfo structure.
				 * The RsslChannel can be used for all additional transport functionality (e.g. reading, writing) once the
				 * state transitions to RSSL_CH_STATE_ACTIVE. If a connection is rejected or initialization fails,
				 * the state will transition to RSSL_CH_STATE_CLOSED.
				 *********************************************************/

				/* Internally, the UPA initialization process includes several actions. The initialization includes
				 * any necessary UPA connection handshake exchanges, including any HTTP or HTTPS negotiation.
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
				if ((retval = rsslInitChannel(upaChannelManagementInfo.upaChannel, &inProgInfo, &error)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) (errno: %d) encountered with rsslInitChannel fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannelManagementInfo.upaChannel->socketId, error.text);
					/* Closes channel, cleans up and exits the application. */
					closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
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
								printf("\nChannel In Progress - New FD: "SOCKET_PRINT_TYPE"  Old FD: "SOCKET_PRINT_TYPE"\n",upaChannelManagementInfo.upaChannel->socketId, inProgInfo.oldSocket );

								/* File descriptor has changed, unregister old and register new */
								FD_CLR(inProgInfo.oldSocket, &cleanReadFds);
								FD_CLR(inProgInfo.oldSocket, &cleanWriteFds);
								FD_CLR(inProgInfo.oldSocket, &cleanExceptFds);
								/* newSocket should equal upaChannelManagementInfo.upaChannel->socketId */
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanReadFds);
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanExceptFds);
							}
							else
							{
								printf("\nChannel "SOCKET_PRINT_TYPE" In Progress...\n", upaChannelManagementInfo.upaChannel->socketId);
							}
						}
						break;

						/* channel connection becomes active!
						 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state,
						 * this RsslChannel can be used for other transport operations.
						 */
						case RSSL_RET_SUCCESS:
						{
							printf("\nChannel on fd "SOCKET_PRINT_TYPE" is now active - reading and writing can begin.\n", upaChannelManagementInfo.upaChannel->socketId);

							/*********************************************************
							 * Connection is now active. The RsslChannel can be used for all additional
							 * transport functionality (e.g. reading, writing) now that the state
							 * transitions to RSSL_CH_STATE_ACTIVE
							 *********************************************************/

							/* After channel is active, use UPA Transport utility function rsslGetChannelInfo to query RsslChannel negotiated
							 * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
							 * compression information as well as many other values.
							 */
							if ((retval = rsslGetChannelInfo(upaChannelManagementInfo.upaChannel, &upaChannelManagementInfo.upaChannelInfo, &error)) != RSSL_RET_SUCCESS)
							{
								printf("Error %s (%d) (errno: %d) encountered with rsslGetChannelInfo. Error Text: %s\n",
									rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

								/* Connection should be closed, return failure */
								/* Closes channel, cleans up and exits the application. */
								closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
							}

							printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
								"	Max Fragment Size: %u\n"
								"	Output Buffers: %u Max, %u Guaranteed\n"
								"	Input Buffers: %u\n"
								"	Send/Recv Buffer Sizes: %u/%u\n"
								"	Ping Timeout: %u\n"
								"	Connected component version: ",
								upaChannelManagementInfo.upaChannel->socketId,				/*!< @brief Socket ID of this UPA channel. */
								upaChannelManagementInfo.upaChannelInfo.maxFragmentSize,	/*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */
								upaChannelManagementInfo.upaChannelInfo.maxOutputBuffers,	/*!< @brief This is the maximum number of output buffers available to the channel. */
								upaChannelManagementInfo.upaChannelInfo.guaranteedOutputBuffers, /*!< @brief This is the guaranteed number of output buffers available to the channel. */
								upaChannelManagementInfo.upaChannelInfo.numInputBuffers,	/*!< @brief This is the number of input buffers available to the channel. */
								upaChannelManagementInfo.upaChannelInfo.sysSendBufSize,		/*!< @brief This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
								upaChannelManagementInfo.upaChannelInfo.sysRecvBufSize,		/*!< @brief This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
								upaChannelManagementInfo.upaChannelInfo.pingTimeout 		/*!< @brief This is the value of the negotiated ping timeout */
							);

							if (upaChannelManagementInfo.upaChannelInfo.componentInfoCount == 0)
								printf("(No component info)");
							else
							{
								RsslUInt32 count;
								for(count = 0; count < upaChannelManagementInfo.upaChannelInfo.componentInfoCount; ++count)
								{
									printf("%.*s",
										upaChannelManagementInfo.upaChannelInfo.componentInfo[count]->componentVersion.length,
										upaChannelManagementInfo.upaChannelInfo.componentInfo[count]->componentVersion.data);
									if (count < upaChannelManagementInfo.upaChannelInfo.componentInfoCount - 1)
										printf(", ");
								}
							}
							printf ("\n\n");
						}
						break;

						default: /* Error handling */
						{
							printf("\nBad return value fd="SOCKET_PRINT_TYPE" <%s>\n",
								upaChannelManagementInfo.upaChannel->socketId, error.text);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
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
			closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
		}
	}

	/* Initialize ping management handler */
	initPingManagementHandler(&upaChannelManagementInfo);

	/* Send Login request message */
	if ((retval = sendLoginRequest(&upaChannelManagementInfo)) > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush, leave our write notification enabled so we get called again.
		 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
		 */

		/* set write fd if there's still other data queued */
		/* flush is done by application */
		FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
	}
	else if (retval < RSSL_RET_SUCCESS)
	{
		/* Closes channel, cleans up and exits the application. */
		closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
	}

	/*****************************************************************************************************************
				SECOND MAIN LOOP TO CONNECTION ACTIVE - KEEP LISTEINING FOR INCOMING DATA
	******************************************************************************************************************/
	/* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
	 * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
	 */

	/* Main loop for message processing (reading data, writing data, and ping management, etc.)
	 * The loop calls select() to wait for notification
	 * Currently, the only way to exit this Main loop is when an error condition is triggered or after
	 * a predetermined run-time has elapsed.
	 */
	while (1)
	{
		useReadFds = cleanReadFds;
		useWriteFds = cleanWriteFds;
		useExceptFds = cleanExceptFds;

		/* now that the channel is active, need to reset the time_interval for the select call -
		 * timeout numbers for the select call should be set to be a factor of the server ping timeout.
		 */
		/* On Linux platform, select() modifies timeout to reflect the amount of time not slept;
		 * most other implementations do not do this. (POSIX.1-2001 permits either behaviour.)
		 * This causes problems both when Linux code which reads timeout is ported to other operating systems,
		 * and when code is ported to Linux that reuses a struct timeval for multiple select()s
		 * in a loop without reinitializing it. Consider timeout to be undefined after select() returns.
		 *
		 * Note: You should reset the values of your timeout before you call select() every time.
		 */
		time_interval.tv_sec =  upaChannelManagementInfo.upaChannel->pingTimeout/60;
		time_interval.tv_usec = 0;

		/* To check if any messages have arrived by calling select() */
		selRet = select(FD_SETSIZE, &useReadFds, &useWriteFds, &useExceptFds, &time_interval);

		if (selRet == 0)
		{
			/* select has timed out, no messages received, continue */
			/* On success, select() return zero if the timeout expires before anything interesting happens. */
		}
		else if (selRet > 0)
		{
			/* Received messages and reading from the channel/connection */
			/* On success, select() return the number of file descriptors contained in the three returned descriptor sets
			 * (that is, the total number of bits that are set in readfds, writefds, exceptfds)
			 */

			/* different behaviors are triggered by different file descriptors */
			if (FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useReadFds) || FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useExceptFds))
			{
				/* reading data from channel via Read/Exception FD */

				/* When a client RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is possible for an application to receive data from the connection.
				 * The arrival of this information is often announced by the I/O notification mechanism that the RsslChannel.socketId is registered with.
				 * The UPA Transport reads information from the network as a byte stream, after which it determines RsslBuffer boundaries and returns
				 * each buffer one by one.
				 */

				retval_rsslRead = 1; /* initialize to a positive value for rsslRead call in case we have more data that is available to read */

				/* Check the return code to determine whether more data is available to read */
				while (retval_rsslRead > RSSL_RET_SUCCESS) /* read until no more to read */
				{
					/* There is more data to read and process and I/O notification may not trigger for it
					 * Either schedule another call to read or loop on read until retCode == RSSL_RET_SUCCESS
					 * and there is no data left in internal input buffer
					 */

					/*********************************************************
					 * Client/Consumer Application Lifecycle Major Step 4:
					 * Read using rsslRead
					 * rsslRead provides the user with data received from the connection. This function expects the RsslChannel to be in the active state.
					 * When data is available, an RsslBuffer referring to the information is returned, which is valid until the next call to rsslRead.
					 * A return code parameter passed into the function is used to convey error information as well as communicate whether there is additional
					 * information to read. An I/O notification mechanism may not inform the user of this additional information as it has already been read
					 * from the socket and is contained in the rsslRead input buffer.
					 *********************************************************/

					if ((msgBuf = rsslRead(upaChannelManagementInfo.upaChannel, &retval_rsslRead, &error)) != 0)
					{
						/* if a buffer is returned, we have data to process and code is success */

						/* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
						 * calling the applicable specific function for further processing.
						 */

						/* No need to clear the message before we decode into it. UPA Decoding populates all message members (and that is true for any
						 * decoding with UPA, you never need to clear anything but the iterator)
						 */
						RsslMsg msg;

						/* This rsslClearDecodeIterator clear iterator function should be used to achieve the best performance while clearing the iterator. */
						/* Clears members necessary for decoding and readies the iterator for reuse. You must clear RsslDecodeIterator
						 * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
						 */
						rsslClearDecodeIterator(&decodeIter);

						/* Set the RWF version to decode with this iterator */
						rsslSetDecodeIteratorRWFVersion(&decodeIter, upaChannelManagementInfo.upaChannel->majorVersion, upaChannelManagementInfo.upaChannel->minorVersion);

						/* Associates the RsslDecodeIterator with the RsslBuffer from which to decode. */
						if((retval = rsslSetDecodeIteratorBuffer(&decodeIter, msgBuf)) != RSSL_RET_SUCCESS)
						{
							printf("\nrsslSetDecodeIteratorBuffer() failed with return code: %d\n", retval);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
						}

						/* decode contents into the RsslMsg structure */
						retval = rsslDecodeMsg(&decodeIter, &msg);
						if (retval != RSSL_RET_SUCCESS)
						{
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", retval, upaChannelManagementInfo.upaChannel->socketId, msgBuf->length);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
						}

						switch ( msg.msgBase.domainType )
						{
							/*!< (1) Login Message */
							/* main difference with last module_1c_read_and_write
							* add sendMessage(), sendLoginRequest(),processLoginResponse(), closeLoginStream()
							* and upaGetBuffer() functions at the end.*/

							case RSSL_DMT_LOGIN:
							{
								if (processLoginResponse(&msg, &decodeIter) != RSSL_RET_SUCCESS)
								{
									/* Login Failed and the application is denied - Could be one of the following 3 possibilities:
									 *
									 * - RSSL_STREAM_CLOSED_RECOVER (Stream State): (3) Closed, the applications may attempt to re-open the stream later
									 *   (can occur via either an RsslRefreshMsg or an RsslStatusMsg), OR
									 *
									 * - RSSL_STREAM_CLOSED (Stream State): (4) Closed (indicates that the data is not available on this service/connection
									 *   and is not likely to become available), OR
									 *
									 * - RSSL_DATA_SUSPECT (Data State): (2) Data is Suspect (similar to a stale data state, indicates that the health of
									 *	 some or all data associated with the stream is out of date or cannot be confirmed that it is current)
									 */

									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
								}
								else
								{
									printf("UPA Consumer application is granted access and has logged in successfully.\n\n");
								}
							}
							break;
							default: /* Error handling */
							{
								printf("Unhandled Domain Type: %d\n", msg.msgBase.domainType);
							}
							break;
						}

						/* Process data and update ping monitor since data was received */
						/* set flag for server message received */
						upaChannelManagementInfo.pingManagementInfo.receivedServerMsg = RSSL_TRUE;
						printf("Ping message has been received successfully from the server due to data message ... \n\n");
					}
					else
					{
						/* keep track of the return values from read so data is not stranded in the input buffer.
						 * Handle return codes appropriately, not all return values are failure conditions
						 */
						switch (retval_rsslRead)
						{
							/*!< (-13) Transport Success: rsslRead has received a ping message. There is no buffer in this case. */
							case RSSL_RET_READ_PING:
							{
								/* Update ping monitor */
								/* set flag for server message received */
								upaChannelManagementInfo.pingManagementInfo.receivedServerMsg = RSSL_TRUE;
								printf("Ping message has been received successfully from the server due to ping message ... \n\n");
							}
							break;
							/*!< (-14) Transport Success: rsslRead received an FD change event. The application should unregister the oldSocketId and
							 * register the socketId with its notifier
							 */
							case RSSL_RET_READ_FD_CHANGE:
							{
								/* File descriptor changed, typically due to tunneling keep-alive */
								/* Unregister old socketId and register new socketId */
								printf("\nrsslRead() FD Change - Old FD: "SOCKET_PRINT_TYPE" New FD: "SOCKET_PRINT_TYPE"\n", upaChannelManagementInfo.upaChannel->oldSocketId, upaChannelManagementInfo.upaChannel->socketId);
								FD_CLR(upaChannelManagementInfo.upaChannel->oldSocketId, &cleanReadFds);
								FD_CLR(upaChannelManagementInfo.upaChannel->oldSocketId, &cleanWriteFds);
								FD_CLR(upaChannelManagementInfo.upaChannel->oldSocketId, &cleanExceptFds);
								/* Up to application whether to register with write set - depends on need for write notification. Here we need it for flushing. */
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanReadFds);
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanExceptFds);
							}
							break;
							/*!< (-11) Transport Success: Reading was blocked by the OS. Typically indicates that there are no bytes available to read,
							 * returned from rsslRead.
							 */
							case RSSL_RET_READ_WOULD_BLOCK: /* Nothing to read */
							break;
							case RSSL_RET_FAILURE: /* fall through to default. */
							default: /* Error handling */
							{
								if (retval_rsslRead < 0)
								{
									printf("Error %s (%d) (errno: %d) encountered with rsslRead fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
										rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError,
										upaChannelManagementInfo.upaChannel->socketId, error.text);
									/* Closes channel/connection, cleans up and exits the application. */
									closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
								}
							}
							break;
						}
					}
				}
			}

			/* An I/O notification mechanism can be used to indicate when the operating system can accept more data for output.
			 * rsslFlush function is called because of a write file descriptor alert
			 */
			if (FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useWriteFds))
			{
				/* flushing via write FD and active state */

				/* Because it may not be possible for the rsslWrite function to pass all data to the underlying socket, some data
				 * may be queued by the UPA Transport. The rsslFlush function is provided for the application to continue attempting
				 * to pass queued data to the connection. If data is queued, this may be a result of all available output space being
				 * used for a connection. An I/O notification mechanism can be used to alert the application when output space becomes
				 * available on a connection.
				 *
				 * rsslFlush function performs any writing of queued data to the connection. This function expects the RsslChannel
				 * to be in the active state. If no information is queued, the rsslFlush function is not required to be called and
				 * should return immediately.
				 *
				 * This function also performs any buffer reordering that may occur due to priorities passed in on the rsslWrite
				 * function. For more information about priority writing, refer to UPA C developers guide.
				 */

				/* rsslFlush use, be sure to keep track of the return values from rsslFlush so data is not stranded in the output buffer
				 * - rsslFlush may need to be called again to continue attempting to pass data to the connection
				 */
				retval = RSSL_RET_FAILURE;

				/* this section of code was called because of a write file descriptor alert */
				if ((retval = rsslFlush(upaChannelManagementInfo.upaChannel, &error)) > RSSL_RET_SUCCESS)
				{
					/* There is still data left to flush, leave our write notification enabled so we get called again.
					 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
					 */
				}
				else
				{
					switch (retval)
					{
						case RSSL_RET_SUCCESS:
						{
							/* Everything has been flushed, no data is left to send - unset/clear write fd notification */
							FD_CLR(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
						}
						break;
						case RSSL_RET_FAILURE: /* fall through to default. */
						default: /* Error handling */
						{
							printf("Error %s (%d) (errno: %d) encountered with rsslFlush() with return code %d. Error Text: %s\n",
								rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, retval,
								error.text);
							/* Connection should be closed, return failure */
							/* Closes channel/connection, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
						}
					}
				}
			}
		}
		else if (selRet < 0)
		{
			/* On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined */
			printf("\nSelect error.\n");
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
		}

		/* Processing ping management handler */
		if ((retval = processPingManagementHandler(&upaChannelManagementInfo)) > RSSL_RET_SUCCESS)
		{
			/* There is still data left to flush, leave our write notification enabled so we get called again.
			 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
			 */

			/* set write fd if there's still other data queued */
			/* flush is done by application */
			FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds);
		}
		else if (retval < RSSL_RET_SUCCESS)
		{
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
		}

		/* get current time */
		time(&currentTime);

		/* Handles the run-time for the UPA Consumer application. Here we exit the application after a predetermined time to run */
		if (currentTime >= upaRuntime)
		{
			/* Closes all streams for the consumer after run-time has elapsed. */

			/* Close Login stream */
			/* Note that closing Login stream will automatically close all other streams at the provider */
			if ((retval = closeLoginStream(&upaChannelManagementInfo)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you close login, we want to make a best effort to get this across the network as it will gracefully
				 * close all open streams. If this cannot be flushed or failed, this application will just close the connection
				 * for simplicity.
				 */

				/* Closes channel, cleans up and exits the application. */
				closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_FAILURE);
			}

			/* flush before exiting */
			if (FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &cleanWriteFds))
			{
				retval = 1;
				while (retval > RSSL_RET_SUCCESS)
				{
					retval = rsslFlush(upaChannelManagementInfo.upaChannel, &error);
				}
				if (retval < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
			}

			printf("\nUPA Consumer run-time has expired...\n");
			closeChannelCleanUpAndExit(upaChannelManagementInfo.upaChannel, RSSL_RET_SUCCESS);
		}
	}
}

/*
 * Closes channel, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* upaChannel, int code)
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

	if ((upaChannel) && (retval = rsslCloseChannel(upaChannel, &error) < RSSL_RET_SUCCESS))
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseChannel. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 6:
	 * Uninitialize UPA Transport using rsslUninitialize
	 * The last UPA Transport function that an application should call. This uninitializes internal data
	 * structures and deletes any allocated memory.
	 *********************************************************/

	/* All UPA Transport use is complete, must uninitialize.
	 * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
	 */
	rsslUninitialize();

	/* For applications that do not exit due to errors/exceptions such as:
	 * Exits the application if the run-time has expired.
	 */
	if (code == RSSL_RET_SUCCESS)
		printf("\nUPA Consumer Training application successfully ended.\n");

	/* End application */
	exit(code);
}

/*
 * Initializes the ping times for upaChannelManagementInfo.upaChannel.
 * upaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* get current time */
	time(&upaChannelManagementInfo->pingManagementInfo.currentTime);

	/* set ping timeout for client and server */
	/* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
	 * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
	 * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
	 * network or operating system notification that may occur.
	 */
	upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient = upaChannelManagementInfo->upaChannel->pingTimeout/3;
	upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer = upaChannelManagementInfo->upaChannel->pingTimeout;

	/* set time to send next ping from client */
	upaChannelManagementInfo->pingManagementInfo.nextSendPingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;

	/* set time client should receive next message/ping from server */
	upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;

	upaChannelManagementInfo->pingManagementInfo.receivedServerMsg = RSSL_FALSE;
}

/*
 * Processing ping management handler for upaChannelManagementInfo.upaChannel.
 * upaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* Handles the ping processing for upaChannelManagementInfo.upaChannel. Sends a ping to the server if the next send ping time has arrived and
	 * checks if a ping has been received from the server within the next receive ping time.
	 */
	RsslRet	retval = RSSL_RET_SUCCESS;
	RsslError error;

	/* get current time */
	time(&upaChannelManagementInfo->pingManagementInfo.currentTime);

	/* handle client pings */
	if (upaChannelManagementInfo->pingManagementInfo.currentTime >= upaChannelManagementInfo->pingManagementInfo.nextSendPingTime)
	{
		/* send ping to server */
		/*********************************************************
		 * Client/Consumer Application Lifecycle Major Step 4:
		 * Ping using rsslPing
		 * Attempts to write a heartbeat message on the connection. This function expects the RsslChannel to be in the active state.
		 * If an application calls the rsslPing function while there are other bytes queued for output, the UPA Transport layer will
		 * suppress the heartbeat message and attempt to flush bytes to the network on the user's behalf.
		 *********************************************************/

		/* rsslPing use - this demonstrates sending of heartbeats */
		if ((retval = rsslPing(upaChannelManagementInfo->upaChannel, &error)) > RSSL_RET_SUCCESS)
		{
			/* Indicates that queued data was sent as a heartbeat and there is still information internally queued by the transport.
			 * The rsslFlush function must be called to continue attempting to pass the queued bytes to the connection. This information may
			 * still be queued because there is not sufficient space in the connections output buffer.
			 * An I/O notification mechanism can be used to indicate when the socketId has write availability.
			 *
			 * There is still data left to flush, leave our write notification enabled so we get called again.
			 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
			 */

			/* flush needs to be done by application */
		}
		else
		{
			switch (retval)
			{
				case RSSL_RET_SUCCESS:
				{
					/* Ping message has been sent successfully to the server ... */
					printf("Ping message has been sent successfully to the server ... \n\n");
				}
				break;
				case RSSL_RET_FAILURE: /* fall through to default. */
				default: /* Error handling */
				{
					printf("\nError %s (%d) (errno: %d) encountered with rsslPing() on fd="SOCKET_PRINT_TYPE" with code %d\n. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannelManagementInfo->upaChannel->socketId, retval,
						error.text);
					/* Closes channel/connection, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
			}
		}

		/* set time to send next ping from client */
		upaChannelManagementInfo->pingManagementInfo.nextSendPingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;
	}

	/* handle server pings - an application should determine if data or pings have been received,
	 * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
	 */
	if (upaChannelManagementInfo->pingManagementInfo.currentTime >= upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime)
	{
		/* check if client received message from server since last time */
		if (upaChannelManagementInfo->pingManagementInfo.receivedServerMsg)
		{
			/* reset flag for server message received */
			upaChannelManagementInfo->pingManagementInfo.receivedServerMsg = RSSL_FALSE;

			/* set time client should receive next message/ping from server */
			upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;
		}
		else /* lost contact with server */
		{
			printf("\nLost contact with server...\n");
			/* Closes channel/connection, cleans up and exits the application. */
			return RSSL_RET_FAILURE;
		}
	}

	return retval;
}

/*
 * Sends a message buffer to a channel.
 * upaChannel - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* upaChannel, RsslBuffer* msgBuf)
{
	RsslError error;
	RsslRet	retval = 0;
	RsslUInt32 bytesWritten = 0;
	RsslUInt32 uncompressedBytesWritten = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS; /*!< (0x00) No Write Flags */

	/* send the request */

	/*********************************************************
	 * Client/Consumer Application Lifecycle Major Step 4:
	 * Write using rsslWriter
	 * rsslWriter performs any writing or queuing of data. This function expects the RsslChannel to be in the active state and the buffer to be properly populated,
	 * where length reflects the actual number of bytes used. This function allows for several modifications to be specified for this call. Here we use
	 * RSSL_WRITE_NO_FLAGS. For more information on other flag enumeration such as RSSL_WRITE_DO_NOT_COMPRESS or RSSL_WRITE_DIRECT_SOCKET_WRITE, see the UPA C
	 * developers guide for rsslWrite Flag Enumeration Values supported by UPA Transport.
	 *
	 * The UPA Transport also supports writing data at different priority levels.
	 * The application can pass in two integer values used for reporting information about the number of bytes that will be written. The uncompressedBytesWritten
	 * parameter will return the number of bytes to be written, including any transport header overhead but not taking into account any compression. The bytesWritten
	 * parameter will return the number of bytes to be written, including any transport header overhead and taking into account any compression. If compression is
	 * disabled, uncompressedBytesWritten and bytesWritten should match.
	 * The number of bytes saved through the compression process can be calculated by (bytesWritten - uncompressedBytesWritten).
	 * Note:
	 * Before passing a buffer to rsslWrite, it is required that the application set length to the number of bytes actually used. This ensures that only the required
	 * bytes are written to the network.
	 *********************************************************/

	/* Now write the data - keep track of UPA Transport return code -
	 * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
	 */

	/* this example writes buffer as high priority and no write modification flags */
	if ((retval = rsslWrite(upaChannel, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &bytesWritten, &uncompressedBytesWritten, &error)) == RSSL_RET_WRITE_CALL_AGAIN)
	{
		/*!< (-10) Transport Success: rsslWrite is fragmenting the buffer and needs to be called again with the same buffer. This indicates that rsslWrite was
		 * unable to send all fragments with the current call and must continue fragmenting
		 */

		/* Large buffer is being split by transport, but out of output buffers. Schedule a call to rsslFlush and then call the rsslWrite function again with
		 * this same exact buffer to continue the fragmentation process. Only release the buffer if not passing it to rsslWrite again. */

		/* call flush and write again - breaking out if the return code is something other than RSSL_RET_WRITE_CALL_AGAIN (write call again) */
		while (retval == RSSL_RET_WRITE_CALL_AGAIN)
		{
			/* Schedule a call to rsslFlush */
			if ((retval = rsslFlush(upaChannel, &error)) < RSSL_RET_SUCCESS)
			{
				printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			}
			/* call the rsslWrite function again with this same exact buffer to continue the fragmentation process. */
			retval = rsslWrite(upaChannel, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &bytesWritten, &uncompressedBytesWritten, &error);
		}
	}

	/* set write fd if there's still data queued */
	if (retval > RSSL_RET_SUCCESS)
	{
		/* The write was successful and there is more data queued in UPA Transport. The rsslFlush function should be used to continue attempting to flush data
		 * to the connection. UPA will release buffer.
		 */

		/* flush needs to be done by application */
	}
	else
	{
		/* Handle return codes appropriately, not all return values are failure conditions */
		switch(retval)
		{
			case RSSL_RET_SUCCESS:
			{
				/* Successful write and all data has been passed to the connection */
				/* Continue with next operations. UPA will release buffer.*/
			}
			break;
			/*!< (-21) Codec Failure: The buffer provided does not have sufficient space to perform the operation. */
			case RSSL_RET_BUFFER_TOO_SMALL:  /* Nothing to read */
			{
				/* Indicates that either the buffer has been corrupted, possibly by exceeding the allowable length, or it is not a valid pool buffer. */
				/* Buffer somehow got corrupted, if it was from rsslGetBuffer, release it */

				/**
				 * @brief Releases a RsslBuffer after use
				 *
				 * Typical use: <BR>
				 * This is called when a buffer is done being used. The rsslWrite function will release the buffer if it
				 * successfully writes. The user should only need to use this function when they get a buffer that they do not need
				 * or when rsslWrite fails.
				 *
				 * @param buffer RSSL buffer to be released
				 * @param error RSSL Error, to be populated in event of an error
				 * @return RsslRet RSSL return value
				 */
				rsslReleaseBuffer(msgBuf, &error);
			}
			break;
			/*!< (-9)  Transport Success: rsslWrite internally attempted to flush data to the connection but was blocked. This is not a failure and the user should not release their buffer */
			case RSSL_RET_WRITE_FLUSH_FAILED:
			{
				/* The write was successful, but an attempt to flush failed. UPA will release buffer.*/
				/* Must check channel state to determine if this is unrecoverable or not */
				if (upaChannel->state == RSSL_CH_STATE_CLOSED)
				{
					/* Channel is Closed - This is terminal. Treat as error, and buffer must be released - fall through to default. */
				}
				else
				{
					/* rsslWrite internally attempted to flush data to the connection but was blocked. This is not a failure and the user should not release their buffer.";
					/* Successful write call, data is queued. The rsslFlush function should be used to continue attemting to flush data to the connection. */

					/* set write fd if flush failed */
					/* flush needs to be done by application */

					/* Channel is still open, but rsslWrite() tried to flush internally and failed.
					 * Return positive value so the caller knows there's bytes to flush.
					 */
					return RSSL_RET_SUCCESS + 1;
				}
			}
			case RSSL_RET_FAILURE: /* fall through to default. */
			default: /* Error handling */
			{
				printf("Error %s (%d) (errno: %d) encountered with rsslWrite. Error Text: %s\n",
					rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError,
					error.text);
				/*  Buffer must be released - return code from rsslReleaseBuffer can be checked */
				rsslReleaseBuffer(msgBuf, &error);
				/* Connection should be closed, return failure */
				return RSSL_RET_FAILURE;
			}
			break;
		}
	}

	return retval;
}

/*
 * Send Login request message to a channel. This consists of getting a message buffer, setting the login request
 * information, encoding the login request, and sending the login request to the server. A Login request message is
 * encoded and sent by OMM consumer and OMM non-interactive provider applications. This message registers a user
 * with the system. After receiving a successful Login response, applications can then begin consuming or providing
 * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
 * upaChannelInfo - The channel management information including the channel to send the Login request message buffer to
 */
RsslRet sendLoginRequest(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Refinitiv recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	RsslElementList	elementList;
	RsslElementEntry elementEntry;

	RsslBuffer applicationId, applicationName;
	RsslUInt64 applicationRole;

	char userName[256];
	RsslBuffer userNameBuf;

	/* Prefer use of clear functions for initializations over using static initializers. Clears tend to be more performant than
	 * using static initializers. Although if you do choose to use static initializers instead, you don’t need to clear.
	 */
	rsslClearElementList(&elementList);
	rsslClearElementEntry(&elementEntry);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Login request. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRequestMsg(&reqMsg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	reqMsg.msgBase.msgClass = RSSL_MC_REQUEST; /*!< (1) Request Message */
	/* StreamId */
	reqMsg.msgBase.streamId = LOGIN_STREAM_ID;
	reqMsg.msgBase.domainType = RSSL_DMT_LOGIN; /*!< (1) Login Message */
	reqMsg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/
	/* The initial Login request must be streaming (i.e., a RSSL_RQMF_STREAMING flag is required). */
	reqMsg.flags = RSSL_RQMF_STREAMING;

	/* set msgKey members */
	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME;

	/* Username */
	userNameBuf.data = userName;
	userNameBuf.length = sizeof(userName);

	/* The UPA Transport layer provides several utility functions. rsslGetUserName utility function takes an RsslBuffer with associated memory
	 * pointed to by data, where length is set to the amount of space available. Queries the username associated with the owner of the
	 * current process, and returns it in the provided buffer.
	 */
	if (rsslGetUserName(&userNameBuf) == RSSL_RET_SUCCESS)
	{
		reqMsg.msgBase.msgKey.name.data = userName;
		reqMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(userName);
	}
	else
	{
		reqMsg.msgBase.msgKey.name.data = (char *)"Unknown";
		reqMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen("Unknown");
	}

	reqMsg.msgBase.msgKey.nameType = RDM_LOGIN_USER_NAME; /*!< (1) Name */
	/*!< (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
	reqMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* encode message */

	/* since our msgKey has opaque that we want to encode, we need to use rsslEncodeMsgInit */
	/* rsslEncodeMsgInit should return and inform us to encode our key opaque */
	if ((retval = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&reqMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* encode our msgKey opaque */

	/* encode the element list */
	rsslClearElementList(&elementList);

	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA; /*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */

	/* Begins encoding of an RsslElementList. */
	if ((retval = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementListInit() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* ApplicationId */
	applicationId.data = (char*)"256";
	applicationId.length = (RsslUInt32)strlen("256");
	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPID;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &elementEntry, &applicationId)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element ApplicationId with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* ApplicationName */
	applicationName.data = (char*)"UPA Consumer Training";
	applicationName.length = (RsslUInt32)strlen("UPA Consumer Training");
	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPNAME;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &elementEntry, &applicationName)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element ApplicationName with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* Role */
	elementEntry.dataType = RSSL_DT_UINT;
	elementEntry.name = RSSL_ENAME_ROLE;
	/*!< (0) Application logs in as a consumer */
	applicationRole = RDM_LOGIN_ROLE_CONS;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &elementEntry, &applicationRole)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element Role with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode element list */
	if ((retval = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementListComplete() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	 * for us to encode our container/msg payload
	 */
	if ((retval = rsslEncodeMsgKeyAttribComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgKeyAttribComplete() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login request */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush, leave our write notification enabled so we get called again.
		 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
		 */

		/* set write fd if there's still other data queued */
		/* flush needs to be done by application */
	}

	return retval;
}

/*
 * Processes a login response. This consists of decoding the response.
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginResponse(RsslMsg* msg, RsslDecodeIterator* decodeIter)
{
	RsslRet retval;
	RsslState *pState = 0;

	char tempData[1024];
	RsslBuffer tempBuffer;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH: /*!< (2) Refresh Message */
		{
			RsslMsgKey* key = 0;
			RsslElementList elementList;
			RsslElementEntry elementEntry;

			/* check stream id */
			printf("\nReceived Login Refresh Msg with Stream Id %d\n", msg->msgBase.streamId);

			/* check if it's a solicited refresh */
			if (msg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
				printf("\nThe refresh msg is a solicited refresh (sent as a response to a request).\n");
			else
				printf("\nA refresh sent to inform a consumer of an upstream change in information (i.e., an unsolicited refresh).\n");

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* decode key opaque data */
			if ((retval = rsslDecodeMsgKeyAttrib(decodeIter, key)) != RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeMsgKeyAttrib() failed with return code: %d\n", retval);
				return retval;
			}

			/* decode element list */
			if ((retval = rsslDecodeElementList(decodeIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
			{
				/* decode each element entry in list */
				while ((retval = rsslDecodeElementEntry(decodeIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (retval == RSSL_RET_SUCCESS)
					{
						/* get login response information */
						/* ApplicationId */
						if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_APPID))
							printf("\tReceived Login Response for ApplicationId: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);

						/* ApplicationName */
						else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_APPNAME))
							printf("\tReceived Login Response for ApplicationName: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);

						/* Position */
						else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_POSITION))
							printf("\tReceived Login Response for Position: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);
					}
					else
					{
						printf("rsslDecodeElementEntry() failed with return code: %d\n", retval);
						return retval;
					}
				}
			}
			else
			{
				printf("rsslDecodeElementList() failed with return code: %d\n", retval);
				return retval;
			}

			/* get Username */
			if (key)
				printf("\nReceived Login Response for Username: %.*s\n", key->name.length, key->name.data);
			else
				printf("\nReceived Login Response for Username: Unknown\n");

			/* get state information */
			pState = &msg->refreshMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

			/* check if login okay and is solicited */
			if ((msg->refreshMsg.flags & RSSL_RFMF_SOLICITED) && (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_OK))
			{
				printf("Login stream is OK and solicited\n");
			}
			else /* handle error cases */
			{
				if (pState->streamState == RSSL_STREAM_CLOSED_RECOVER)
				{
					/* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
					printf("\nLogin stream is closed recover\n");
					return RSSL_RET_FAILURE;
				}
				else if (pState->streamState == RSSL_STREAM_CLOSED)
				{
					/* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
					printf("\nLogin attempt failed (stream closed)\n");
					return RSSL_RET_FAILURE;
				}
				else if (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_SUSPECT)
				{
					/* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
					 * information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg)
					 *
					 * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
					 * is out of date or cannot be confirmed that it is current )
					 */
					printf("\nLogin stream is suspect\n");
					return RSSL_RET_FAILURE;
				}
			}
		}
		break;
		case RSSL_MC_STATUS: /*!< (3) Status Message */
		{
			printf("\nReceived Login StatusMsg\n");
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE) /*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state. */
    		{
				/* get state information */
				pState = &msg->statusMsg.state;
				rsslStateToString(&tempBuffer, pState);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
				/* handle error cases */
				if (pState->streamState == RSSL_STREAM_CLOSED_RECOVER)
				{
					/* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
					printf("\nLogin stream is closed recover\n");
					return RSSL_RET_FAILURE;
				}
				else if (pState->streamState == RSSL_STREAM_CLOSED)
				{
					/* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
					printf("\nLogin stream is closed\n");
					return RSSL_RET_FAILURE;
				}
				else if (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_SUSPECT)
				{
					/* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
					 * information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg)
					 *
					 * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
					 * is out of date or cannot be confirmed that it is current )
					 */
					printf("\nLogin stream is suspect\n");
					return RSSL_RET_FAILURE;
				}
    		}
		}
		break;
		case RSSL_MC_UPDATE: /*!< (4) Update Message */
		{
			printf("\nReceived Login Update\n");
		}
		break;
		case RSSL_MC_CLOSE: /*!< (5) Close Message */
		{
			printf("\nReceived Login Close\n");
			return RSSL_RET_FAILURE;
		}
		break;
		default: /* Error handling */
		{
			printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
		}
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Close the Login stream. Note that closing Login stream will automatically close all other streams at the provider.
 * A Login close message is encoded and sent by OMM consumer applications. This message allows a consumer to log out
 * of the system. Closing a Login stream is equivalent to a 'Close All' type of message, where all open streams are
 * closed (thus all other streams associated with the user are closed).
 * upaChannelInfo - The channel management information including the channel to send the Login close message buffer to
 */
RsslRet closeLoginStream(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Consumer uses RsslCloseMsg to indicate no further interest in an item stream and to close the stream. */
	RsslCloseMsg msg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Refinitiv recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login close.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Login close. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearCloseMsg(&msg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE; /*!< (5) Close Message */
	msg.msgBase.streamId = LOGIN_STREAM_ID;
	msg.msgBase.domainType = RSSL_DMT_LOGIN; /*!< (1) Login Message */
	/* No payload associated with this close message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/* encode message */

	/* Since there is no payload, no need for Init/Complete as everything is in the msg header */
	/* Functions without a suffix of Init or Complete (e.g. rsslEncodeMsg) perform encoding within a single call,
	 * typically used for encoding simple types like Integer or incorporating previously encoded data
	 * (referred to as pre-encoded data).
	 */
	if ((retval = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		return RSSL_RET_FAILURE;
	}

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login close */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* login close fails */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the login close doesn't flush, just close channel and exit the app. When you close login, we want to
		 * make a best effort to get this across the network as it will gracefully close all open streams. If this
		 * cannot be flushed, this application will just close the connection for simplicity.
		 */

		/* Closes channel, cleans up and exits the application. */
	}

	return retval;
}

/*
 * upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplifies the example codes and make the codes more readable.
 */
RsslBuffer* upaGetBuffer(RsslChannel *upaChannel, RsslUInt32 size, RsslError *rsslError)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for any request Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */

	/**
	 * @brief Retrieves a RsslBuffer for use
	 *
	 * Typical use: <BR>
	 * This is called when a buffer is needed to write data to. Generally, the user will populate the RsslBuffer structure and then pass it to
	 * the rsslWrite function.
	 *
	 * @param chnl RSSL Channel who requests the buffer
	 * @param size Size of the requested buffer
	 * @param packedBuffer Set to RSSL_TRUE if you plan on packing multiple messages into the same buffer
	 * @param error RSSL Error, to be populated in event of an error
	 * @return RsslBuffer RSSL buffer to be filled in with valid memory
	 * @see RsslReturnCodes
	 */
	if ((msgBuf = rsslGetBuffer(upaChannel, size, RSSL_FALSE, &error)) == NULL) /* first check Error */
	{
		/* Check to see if this is just out of buffers or if it’s unrecoverable */
		if (error.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
		{
			/* it’s unrecoverable Error */
			printf("Error %s (%d) (errno: %d) encountered with rsslGetBuffer. Error Text: %s\n",
				rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
			/* Connection should be closed, return failure */
			/* Closes channel, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/*!< (-4) Transport Failure: There are no buffers available from the buffer pool, returned from rsslGetBuffer.
		 * Use rsslIoctl to increase pool size or use rsslFlush to flush data and return buffers to pool.
		 */

		/* The rsslFlush function could be used to attempt to free buffers back to the pool */
		retval = rsslFlush(upaChannel, &error);
		if (retval < RSSL_RET_SUCCESS)
		{
			printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			/* Closes channel, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/* call rsslGetBuffer again to see if it works now after rsslFlush */
		if ((msgBuf = rsslGetBuffer(upaChannel, size, RSSL_FALSE, &error)) == NULL)
		{
			printf("Error %s (%d) (errno: %d) encountered with rsslGetBuffer. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
			/* Closes channel, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}
	}

	/* return RSSL buffer to be filled in with valid memory */
	return msgBuf;
}

