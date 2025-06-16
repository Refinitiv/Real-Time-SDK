/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license
 *| AS IS with no warranty or guarantee of fit for purpose.
 *| See the project's LICENSE.md for details.
 *| Copyright (C) 2019, 2025 LSEG. All rights reserved.
 *|-------------------------------------------------------------------------------
 */


/*
 * This is the ETA Interactive Provider Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Interactive Provider using the ETA Transport layer.
 *
 * Main c source file for the ETA Interactive Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * ETA Interactive Provider Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * An OMM Interactive Provider application opens a listening socket on a well-known 
 * port allowing OMM consumer applications to connect. Once connected, consumers 
 * can request data from the Interactive Provider.
 * 
 * In this module, the OMM Interactive Provider application opens a listening socket 
 * on a well-known port allowing OMM consumer applications to connect.
 *
 * Detailed Descriptions:
 * The first step of any ETA Interactive Provider application is to establish 
 * a listening socket, usually on a well-known port so that consumer applications 
 * can easily connect. In this example, the port is consistent with that of 
 * other training consumer examples.
 * The provider uses the rsslBind function to open the port
 * and listen for incoming connection attempts.
 * Whenever an OMM consumer application attempts to connect, the provider uses 
 * the rsslAccept function to begin the connection initialization process.
 * 
 * For this simple training app, the interactive provider only supports a single client. 
 *
 *
 ************************************************************************
 * ETA Interactive Provider Training Module 1b: Ping (heartbeat) Management
 ************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might 
 * need to be exchanged. The negotiated ping timeout is available via 
 * the RsslChannel. If ping heartbeats are not sent or received within 
 * the expected time frame, the connection can be terminated. LSEG 
 * recommends sending ping messages at intervals one-third the 
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Once the connection is active, the consumer and provider applications 
 * might need to exchange ping messages. A negotiated ping timeout is available 
 * via RsslChannel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats 
 * are not sent or received within the expected time frame. LSEG 
 * recommends sending ping messages at intervals one-third the size of the ping timeout.
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information is 
 * being exchanged. Because the provider application is likely sending more frequent 
 * information, providing updates on any streams the consumer has requested, 
 * it may not need to send heartbeats as the other data is sufficient to announce 
 * its continued presence. It is the responsibility of each connection to manage 
 * the sending and receiving of heartbeat messages.
 *
 * Add one more main loop for message processing. Similar to consumer module_ping, this
 * module adds functions to initial ping and process ping.
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

#include "Provider_Training.h"

int main(int argc, char **argv)
{
	/* This example suite uses write descriptor in our server/Interactive Provider type examples in mainly 1 area with
	 * the I/O notification mechanism being used:
	 * 1) rsslFlush() calls used throughout the application (after module 1a), together with rsslWrite() calls, such
	 *    as in sendMessage() function. The write descriptor can be used to indicate when the socketId has write
	 *    availability and help with determining when the network is able to accept additional bytes for writing.
	 */

	/* This example suite also uses a clean FD sets and a dirty FD sets for I/O notification.

	 *		select() - a system call for examining the status of file_descriptors.
	 *					Tells us that there is data to read on the fds.

	 * Since select() modifies its file descriptor sets, if the call is being used in a loop, then the fd sets must
	 * be reinitialized before each call. Since they act as input/output parameters for the select() system call;
	 * they are read by and modified by the system call. When select() returns, the values have all been modified
	 * to reflect the set of file descriptors ready. So, every time before you call select(), you have to
	 * (re)initialize the fd_set values. Here we maintain 2 sets FD sets:
	 * a) clean FD sets so that we can repeatedly call select call
	 * b) dirty FD sets used in the actual select call (I/O notification mechanism)
	 * Typically, you reset the dirty FD sets to be equal to the clean FD sets before you call select().
	 */

	/**************************************************************************************************
			DECLARING VARIABLES
	**************************************************************************************************/
	/* ETA Server structure returned via the rsslBind call */
	RsslServer* etaSrvr = 0;

	RsslBool clientAccepted = RSSL_FALSE;

	char srvrPortNo[128];

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

	/* ETA Bind Options used in the rsslBind call. */
	RsslBindOptions bindOpts = RSSL_INIT_BIND_OPTS;

	/* ETA Accept Options used in the rsslAccept call */
	RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

	/* RsslInProgInfo Information for the In Progress Connection State */
	RsslInProgInfo inProgInfo = RSSL_INIT_IN_PROG_INFO;

	/* ETA channel management information */
	EtaChannelManagementInfo etaChannelManagementInfo;

	RsslBuffer* msgBuf = 0;

	time_t currentTime = 0;
	time_t etaRuntime = 0;
	RsslUInt32 runTime = 0;

	/* For this simple training app, the interactive provider only supports a single client. If the consumer disconnects,
	 * the interactive provider would simply exit.
	 *
	 * If you want the provider to support multiple client sessions at the same time, you need to implement support
	 * for multiple client sessions feature similar to what rsslProvider example is doing.
	 */
	etaChannelManagementInfo.etaChannel = 0;

	/* the default option parameters */
	/* server is running on port number 14002 */
	snprintf(srvrPortNo, 128, "%s", "14002");
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
			if (strcmp("-p", argv[i]) == 0)
			{
				i += 2;
				snprintf(srvrPortNo, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-r", argv[i]) == 0)
			{
				i += 2;
				sscanf(argv[i-1], "%u", &runTime);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-p <SrvrPortNo>] [-r <runTime>] \n", argv[0], argv[0]);
				exit(RSSL_RET_FAILURE);
			}
		}
	}

	/******************************************************************************************************************
				INITIALIZATION - USING rsslInitialize()
	******************************************************************************************************************/
	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 1:
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

	/* get current time */
	time(&currentTime);

	/* Set the runtime of the ETA Interactive Provider application to be runTime (seconds) */
	etaRuntime = currentTime + (time_t)runTime;

	/* populate bind options, then pass to rsslBind function -
	 * ETA Transport should already be initialized
	 */
	bindOpts.serviceName = srvrPortNo;	/* server is running on default port number 14002 */
	bindOpts.pingTimeout = 60;			/* servers desired ping timeout is 60 seconds, pings should be sent every 20 */
	bindOpts.minPingTimeout = 30;		/* min acceptable ping timeout is 30 seconds, pings should be sent every 10 */

	/* set up buffering, configure for shared and guaranteed pools */
	bindOpts.guaranteedOutputBuffers = 1000;
	bindOpts.maxOutputBuffers = 2000;
	bindOpts.sharedPoolSize = 50000;
	bindOpts.sharedPoolLock = RSSL_TRUE;

	bindOpts.serverBlocking = RSSL_FALSE;		/* perform non-blocking I/O */
	bindOpts.channelsBlocking = RSSL_FALSE;		/* perform non-blocking I/O */
	bindOpts.compressionType = RSSL_COMP_NONE;	/* server does not desire compression for this connection */

	/* populate version and protocol with RWF information (found in rsslIterators.h) or protocol specific info */
	bindOpts.protocolType = RSSL_RWF_PROTOCOL_TYPE;	/* Protocol type definition for RWF */
	bindOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	bindOpts.minorVersion = RSSL_RWF_MINOR_VERSION;

	/******************************************************************************************************************
				BINDING SETUP - USING rsslBind()
	******************************************************************************************************************/
	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 2:
	 * Create listening socket using rsslBind
	 * Establishes a listening socket connection, which supports connections from standard socket and HTTP
	 * rsslConnect users. Returns an RsslServer that represents the listening socket connection to the user.
	 * In the event of an error, NULL is returned and additional information can be found in the RsslError structure.
	 * Options are passed in via an RsslBindOptions structure. Once a listening socket is established, this
	 * RsslServer can begin accepting connections.
	 *********************************************************/

	/* Bind ETA server */
	if ((etaSrvr = rsslBind(&bindOpts, &error)) != NULL)
		printf("\nServer IPC descriptor = "SOCKET_PRINT_TYPE" bound on port %d\n", etaSrvr->socketId, etaSrvr->portNumber);
	else
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslBind. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

		/* End application, uninitialize to clean up first */
		rsslUninitialize();
		exit(RSSL_RET_FAILURE);
	}

	/* rsslBind listening socket connection was successful, add socketId to I/O notification mechanism and
	 * initialize connection
	 */
	/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
	FD_SET(etaSrvr->socketId, &cleanReadFds);
	FD_SET(etaSrvr->socketId, &cleanExceptFds);

	/******************************************************************************************************************
				MAIN LOOP TO SEE IF CONNECTION RECEIVED FROM CONSUMER
	******************************************************************************************************************/
	/* Main Loop #1 for detecting incoming client connections. The loop calls select() to wait for notification
	 * when the socketId of the server detects something to be read, this will check for incoming client connections.
	 * When a client successfully connects, a RsslChannel is returned which corresponds to this connection.
	 */

	/*
	 *If we want a non-blocking read call to the selector, we use select before read as read is a blocking call but select is not
	 *If we want a blocking read call to the selector, such that we want to wait till we get a message, we should use read without select.
	 *In the program below we will use select(), as it is non-blocking
	 */

	while (!clientAccepted)
	{
		useReadFds = cleanReadFds;
		useWriteFds = cleanWriteFds;
		useExceptFds = cleanExceptFds;

		/* Set a timeout value for waiting and checking messages received from incoming client connections. */
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
			/* no messages received from incoming connections, continue to wait and check for incoming client connections. */
		}
		else if (selRet > 0)
		{
			/* Received a response from the consumer. */

			/* On success, select() return the number of file descriptors contained in the three returned descriptor sets
			 * (that is, the total number of bits that are set in readfds, writefds, exceptfds)
			 */
			/* rsslInitChannel is called if read is triggered */
			if (FD_ISSET(etaSrvr->socketId, &useReadFds))
			{
				/* Use rsslAccept for incoming connections, read and write data to established connections, etc */

				/* Accept is typically called when servers socketId indicates activity */
				/* After a server is created using the rsslBind call, the rsslAccept call can be made. When the socketId of
				 * the server detects something to be read, this will check for incoming client connections. When a client
				 * successfully connects, a RsslChannel is returned which corresponds to this connection. This channel can
				 * be used to read or write with the connected client. If a clients connect message is not accepted, a
				 * negative acknowledgment is sent to the client and no RsslChannel is returned.
				 *
				 * For this simple training app, the interactive provider only supports a single client.
				 */

				/* populate accept options, then pass to rsslAccept function - ETA Transport should already be initialized */

				/*!< @brief If RSSL_TRUE, rsslAccept will send a NAK - even if the connection request is valid. */
				acceptOpts.nakMount = RSSL_FALSE; /* allow the connection */

				/*********************************************************
				 * Server/Provider Application Liefcycle Major Step 3:
				 * Accept connection using rsslAccept
				 * This step is performed per connected client/connection/channel
				 * Uses the RsslServer that represents the listening socket connection and begins the accepting process
				 * for an incoming connection request. Returns an RsslChannel that represents the client connection.
				 * In the event of an error, NULL is returned and additional information can be found in the RsslError
				 * structure.
				 * The rsslAccept function can also begin the rejection process for a connection through the use of the
				 * RsslAcceptOptions structure.
				 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state, this RsslChannel
				 * can be used for other transport operations.
				 *********************************************************/

				/* An OMM Provider application can begin the connection accepting or rejecting process by using the rsslAccept function */
				if ((etaChannelManagementInfo.etaChannel = rsslAccept(etaSrvr, &acceptOpts, &error)) == 0)
				{
					printf("Error %s (%d) (errno: %d) encountered with rsslAccept. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

					/* End application, uninitialize to clean up first */
					rsslUninitialize();
					exit(RSSL_RET_FAILURE);
				}
				else
				{
					/* For this simple training app, the interactive provider only supports one client session from the consumer. */

					printf("\nServer fd="SOCKET_PRINT_TYPE": New client on Channel fd="SOCKET_PRINT_TYPE"\n",
						etaSrvr->socketId,etaChannelManagementInfo.etaChannel->socketId);

					/* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
					/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
					FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanReadFds);
					FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanExceptFds);

					// set clientAccepted to be TRUE and exit the while Main Loop #1
					clientAccepted = RSSL_TRUE;
				}
			}
		}
		else if (selRet < 0)
		{
			/* On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined */
			printf("\nSelect error.\n");

			/* End application, uninitialize to clean up first */
			rsslUninitialize();
			exit(RSSL_RET_FAILURE);
		}
	}

	/******************************************************************************************************************
				SECOND MAIN LOOP TO CONNECTION ACTIVE - LISTENING/SENDING DATA AMD PING MANAGEMENT
	******************************************************************************************************************/
	/* Main Loop #2 for getting connection active and successful completion of the initialization process
	 * The loop calls select() to wait for notification
	 * Currently, the main loop would exit if an error condition is triggered or
	 * RsslChannel.state transitions to RSSL_CH_STATE_ACTIVE.
	 */
	while (etaChannelManagementInfo.etaChannel->state != RSSL_CH_STATE_ACTIVE)
	{
		useReadFds = cleanReadFds;
		useWriteFds = cleanWriteFds;
		useExceptFds = cleanExceptFds;

		/* Set a timeout value for getting connection active and successful completion of the initialization process */
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
			/* Closes channel, closes server, cleans up and exits the application. */
			closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
		}
		else if (selRet > 0)
		{
			/* Received a response from the consumer. */

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
			if (FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useReadFds) || FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useExceptFds))
			{

				/*********************************************************
				 * Server/Provider Application Liefcycle Major Step 4:
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
					/* Closes channel, closes server, cleans up and exits the application. */
					closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
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
								FD_CLR(inProgInfo.oldSocket, &cleanExceptFds);
								/* newSocket should equal etaChannelManagementInfo.etaChannel->socketId */
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanReadFds);
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
								/* Closes channel, closes server, cleans up and exits the application. */
								closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
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

							/* do not allow new client to connect  */

							/* For this simple training app, the interactive provider only supports a single client. Once a client
							 * successfully connects, we call rsslCloseServer function to close the listening socket associated with the
							 * RsslServer. The connected RsslChannels will remain open. This allows the established connection to continue
							 * to send and receive data, while preventing new clients from connecting.
							 */

							/* clean up server */
							FD_CLR(etaSrvr->socketId, &cleanReadFds);
							FD_CLR(etaSrvr->socketId, &cleanExceptFds);

							/*********************************************************
							 * Server/Provider Application Liefcycle Major Step 7:
							 * Closes a listening socket associated with an RsslServer. This will release any pool based resources
							 * back to their respective pools, close the listening socket, and perform any additional necessary cleanup.
							 * Any established connections will remain open, allowing for continued information exchange.
							 *********************************************************/

							/* clean up server using rsslCloseServer call.
							 * If a server is being shut down, the rsslCloseServer function should be used to close the listening socket and perform
							 * any necessary cleanup. All currently connected RsslChannels will remain open. This allows applications to continue
							 * to send and receive data, while preventing new applications from connecting. The server has the option of calling
							 * rsslCloseChannel to shut down any currently connected applications.
							 * When shutting down the ETA Transport, the application should release any unwritten pool buffers.
							 * The listening socket can be closed by calling rsslCloseServer. This prevents any new connection attempts.
							 * If shutting down connections for all connected clients, the provider should call rsslCloseChannel for each connection client.
							*/
							if (rsslCloseServer(etaSrvr, &error) < RSSL_RET_SUCCESS)
							{
								printf("Error %s (%d) (errno: %d) encountered with rsslCloseServer.  Error Text: %s\n",
									rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

								/* End application, uninitialize to clean up first */
								rsslUninitialize();
								exit(RSSL_RET_FAILURE);
							}

							// set etaSrvr to be NULL
							etaSrvr = 0;
						}
						break;
						default: /* Error handling */
						{
							printf("\nBad return value fd="SOCKET_PRINT_TYPE" <%s>\n",
								etaChannelManagementInfo.etaChannel->socketId, error.text);
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
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
			/* Closes channel, closes server, cleans up and exits the application. */
			closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
		}
	}

	/* Initialize ping management handler */
	initPingManagementHandler(&etaChannelManagementInfo);

	/******************************************************************************************************************
				THIRD MAIN LOOP FOR MESSAGE PROCESSSING - READING/WRITING DATA AMD PING MANAGEMENT
	******************************************************************************************************************/
	/* Here were are using a new Main loop #3. An alternative design would be to combine this Main loop #3 (message processing)
	 * with the other 2 earlier Main loops, namely, Main Loop #1 (detecting incoming client connections), and
	 * Main Loop #2 (getting connection active and successful completion of the initialization process) as a single provider Main Loop.
	 * Some bookkeeping would be required for that approach.
	 */

	/* Main Loop #3 for message processing (reading data, writing data, and ping management, etc.)
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
		 * for ETA Interactive Provider, timeout numbers for the select call should be set to be configurable UPDATE_INTERVAL.
		 * We set the Update Rate Interval to be 1 second for Interactive Provider application, which
		 * is the Update Interval the provider application sends the Update Message content to client
		 */
		/* On Linux platform, select() modifies timeout to reflect the amount of time not slept;
		 * most other implementations do not do this. (POSIX.1-2001 permits either behaviour.)
		 * This causes problems both when Linux code which reads timeout is ported to other operating systems,
		 * and when code is ported to Linux that reuses a struct timeval for multiple select()s
		 * in a loop without reinitializing it. Consider timeout to be undefined after select() returns.
		 *
		 * Note: You should reset the values of your timeout before you call select() every time.
		 */
		time_interval.tv_sec = UPDATE_INTERVAL;
		time_interval.tv_usec = 0;

		/* To check if any messages have arrived by calling select() */
		selRet = select(FD_SETSIZE, &useReadFds, &useWriteFds, &useExceptFds, &time_interval);

		if (selRet == 0)
		{
			/* no messages received, send item updates and continue */

			/* for this simple example, the interactive provider can send market price updates for
			 * the single connected client/channel
			 */
		}
		else if (selRet > 0)
		{
			/* Received messages and reading from the channel/connection */
			/* On success, select() return the number of file descriptors contained in the three returned descriptor sets
			 * (that is, the total number of bits that are set in readfds, writefds, exceptfds)
			 */

			/* different behaviors are triggered by different file descriptors */
			if (FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useReadFds) || FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useExceptFds))
			{
				/* reading data from channel via Read/Exception FD */

				/* When a client RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is possible for an application to receive data from the connection.
				 * The arrival of this information is often announced by the I/O notification mechanism that the RsslChannel.socketId is registered with.
				 * The ETA Transport reads information from the network as a byte stream, after which it determines RsslBuffer boundaries and returns
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
					 * Server/Provider Application Liefcycle Major Step 5:
					 * Read using rsslRead
					 * rsslRead provides the user with data received from the connection. This function expects the RsslChannel to be in the active state.
					 * When data is available, an RsslBuffer referring to the information is returned, which is valid until the next call to rsslRead.
					 * A return code parameter passed into the function is used to convey error information as well as communicate whether there is additional
					 * information to read. An I/O notification mechanism may not inform the user of this additional information as it has already been read
					 * from the socket and is contained in the rsslRead input buffer.
					 *********************************************************/

					if ((msgBuf = rsslRead(etaChannelManagementInfo.etaChannel, &retval_rsslRead, &error)) == 0)
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
								etaChannelManagementInfo.pingManagementInfo.receivedClientMsg = RSSL_TRUE;
								printf("Ping message has been received successfully from the client due to ping message ... \n\n");
							}
							break;
							/*!< (-14) Transport Success: rsslRead received an FD change event. The application should unregister the oldSocketId and
							 * register the socketId with its notifier
							 */
							case RSSL_RET_READ_FD_CHANGE:
							{
								/* File descriptor changed, typically due to tunneling keep-alive */
								/* Unregister old socketId and register new socketId */
								printf("\nrsslRead() FD Change - Old FD: "SOCKET_PRINT_TYPE" New FD: "SOCKET_PRINT_TYPE"\n", etaChannelManagementInfo.etaChannel->oldSocketId, etaChannelManagementInfo.etaChannel->socketId);
								FD_CLR(etaChannelManagementInfo.etaChannel->oldSocketId, &cleanReadFds);
								FD_CLR(etaChannelManagementInfo.etaChannel->oldSocketId, &cleanWriteFds);
								FD_CLR(etaChannelManagementInfo.etaChannel->oldSocketId, &cleanExceptFds);
								/* Up to application whether to register with write set - depends on need for write notification. Here we need it for flushing. */
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanReadFds);
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);
								FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanExceptFds);
							}
							break;
							/*!< (-11) Transport Success: Reading was blocked by the OS. Typically indicates that there are no bytes available to read,
							 * returned from rsslRead.
							 */
							case RSSL_RET_READ_WOULD_BLOCK: /* Nothing to read */
							break;
							case RSSL_RET_FAILURE: /* fall through to default. */
							{
								printf("\nchannelInactive fd="SOCKET_PRINT_TYPE" <%s>\n", etaChannelManagementInfo.etaChannel->socketId, error.text);
							}
							default: /* Error handling */
							{
								if (retval_rsslRead < 0)
								{
									printf("Error %s (%d) (errno: %d) encountered with rsslRead fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
										rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError,
										etaChannelManagementInfo.etaChannel->socketId, error.text);
									/* Closes channel/connection, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
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
			if (FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &useWriteFds))
			{
				/* flushing via write FD and active state */

				/* Because it may not be possible for the rsslWrite function to pass all data to the underlying socket, some data
				 * may be queued by the ETA Transport. The rsslFlush function is provided for the application to continue attempting
				 * to pass queued data to the connection. If data is queued, this may be a result of all available output space being
				 * used for a connection. An I/O notification mechanism can be used to alert the application when output space becomes
				 * available on a connection.
				 *
				 * rsslFlush function performs any writing of queued data to the connection. This function expects the RsslChannel
				 * to be in the active state. If no information is queued, the rsslFlush function is not required to be called and
				 * should return immediately.
				 *
				 * This function also performs any buffer reordering that may occur due to priorities passed in on the rsslWrite
				 * function. For more information about priority writing, refer to ETA C developers guide.
				 */

				/* rsslFlush use, be sure to keep track of the return values from rsslFlush so data is not stranded in the output buffer
				 * - rsslFlush may need to be called again to continue attempting to pass data to the connection
				 */
			}
		}
		else if (selRet < 0)
		{
			/* On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined */
			printf("\nSelect error.\n");
			/* Closes channel, closes server, cleans up and exits the application. */
			closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
		}

		/* Processing ping management handler */
		if ((retval = processPingManagementHandler(&etaChannelManagementInfo)) > RSSL_RET_SUCCESS)
		{
			/* There is still data left to flush, leave our write notification enabled so we get called again.
			 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
			 */

			/* set write fd if there's still other data queued */
			/* flush is done by application */
			FD_SET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);
		}
		else if (retval < RSSL_RET_SUCCESS)
		{
			/* Closes channel, closes server, cleans up and exits the application. */
			closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
		}

		/* get current time */
		time(&currentTime);

		/* Handles the run-time for the ETA Interactive Provider application. Here we exit the application after a predetermined time to run */
		if (currentTime >= etaRuntime)
		{
			/* Closes all streams for the Interactive Provider after run-time has elapsed in our simple Interactive Provider example.
			 * If the provider application must shut down, it can either leave consumer connections intact or shut them down. If the provider
			 * decides to close consumer connections, the provider should send an RsslStatusMsg on each connection’s Login stream closing the stream.
			 * At this point, the consumer should assume that its other open streams are also closed.
			 */

			printf("\nETA Interactive Provider run-time has expired...\n");
			closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_SUCCESS);
		}
	}
}

/*
 * Closes channel, closes server, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * etaSrvr - The RsslServer that represents the listening socket connection to the user to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelServerCleanUpAndExit(RsslChannel* etaChannel, RsslServer* etaSrvr, int code)
{
	RsslRet	retval = 0;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 6:
	 * Close connection using rsslCloseChannel (OS connection release handshake)
	 * rsslCloseChannel closes the server based RsslChannel. This will release any pool based resources
	 * back to their respective pools, close the connection, and perform any additional necessary cleanup.
	 * When shutting down the ETA Transport, the application should release all unwritten pool buffers.
	 * Calling rsslCloseChannel terminates the connection for each connection client.
	 *********************************************************/
	if ((etaChannel) && (retval = rsslCloseChannel(etaChannel, &error) < RSSL_RET_SUCCESS))
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseChannel. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 7:
	 * Closes a listening socket associated with an RsslServer. This will release any pool based resources
	 * back to their respective pools, close the listening socket, and perform any additional necessary cleanup.
	 * Any established connections will remain open, allowing for continued information exchange. If desired,
	 * the server can use rsslCloseChannel to shutdown any remaining connections.
	 *********************************************************/

	/* clean up server using rsslCloseServer call.
	 * If a server is being shut down, the rsslCloseServer function should be used to close the listening socket and perform
	 * any necessary cleanup. All currently connected RsslChannels will remain open. This allows applications to continue
	 * to send and receive data, while preventing new applications from connecting. The server has the option of calling
	 * rsslCloseChannel to shut down any currently connected applications.
	 * When shutting down the ETA Transport, the application should release any unwritten pool buffers.
	 * The listening socket can be closed by calling rsslCloseServer. This prevents any new connection attempts.
	 * If shutting down connections for all connected clients, the provider should call rsslCloseChannel for each connection client.
	*/
	if ((etaSrvr) && (retval = rsslCloseServer(etaSrvr, &error) < RSSL_RET_SUCCESS))
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseServer.  Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 8:
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
		printf("\nETA Interactive Provider Training application successfully ended.\n");

	/* End application */
	exit(code);
}

/*
 * Initializes the ping times for etaChannelManagementInfo.etaChannel.
 * etaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* get current time */
	time(&etaChannelManagementInfo->pingManagementInfo.currentTime);

	/* set ping timeout for server and client */
	/* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
	 * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
	 * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
	 * network or operating system notification that may occur.
	 */
	etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer = etaChannelManagementInfo->etaChannel->pingTimeout/3;
	etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient = etaChannelManagementInfo->etaChannel->pingTimeout;

	/* set time to send next ping from server */
	etaChannelManagementInfo->pingManagementInfo.nextSendPingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;

	/* set time server should receive next message/ping from client */
	etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;

	etaChannelManagementInfo->pingManagementInfo.receivedClientMsg = RSSL_FALSE;
}

/*
 * Processing ping management handler for etaChannelManagementInfo.etaChannel.
 * etaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* Handles the ping processing for etaChannelManagementInfo.etaChannel. Sends a ping to the client if the next send ping time has arrived and
	 * checks if a ping has been received from the client within the next receive ping time.
	 */
	RsslRet	retval = RSSL_RET_SUCCESS;
	RsslError error;

	/* get current time */
	time(&etaChannelManagementInfo->pingManagementInfo.currentTime);

	/* handle server pings */
	if (etaChannelManagementInfo->pingManagementInfo.currentTime >= etaChannelManagementInfo->pingManagementInfo.nextSendPingTime)
	{
		/* send ping to client */
		/*********************************************************
		 * Server/Provider Application Liefcycle Major Step 5:
		 * Ping using rsslPing
		 * Attempts to write a heartbeat message on the connection. This function expects the RsslChannel to be in the active state.
		 * If an application calls the rsslPing function while there are other bytes queued for output, the ETA Transport layer will
		 * suppress the heartbeat message and attempt to flush bytes to the network on the user's behalf.
		 *********************************************************/

		/* rsslPing use - this demonstrates sending of heartbeats */
		if ((retval = rsslPing(etaChannelManagementInfo->etaChannel, &error)) > RSSL_RET_SUCCESS)
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
					/* Ping message has been sent successfully to the client */
					printf("Ping message has been sent successfully to the client ... \n\n");
				}
				break;
				case RSSL_RET_FAILURE: /* fall through to default. */
				default: /* Error handling */
				{
					printf("\nError %s (%d) (errno: %d) encountered with rsslPing() on fd="SOCKET_PRINT_TYPE" with code %d\n. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, etaChannelManagementInfo->etaChannel->socketId, retval,
						error.text);
					/* Closes channel/connection, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
			}
		}

		/* set time to send next ping from server */
		etaChannelManagementInfo->pingManagementInfo.nextSendPingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;
	}

	/* handle client pings - an application should determine if data or pings have been received,
	 * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
	 */
	if (etaChannelManagementInfo->pingManagementInfo.currentTime >= etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime)
	{
		/* check if server received message from client since last time */
		if (etaChannelManagementInfo->pingManagementInfo.receivedClientMsg)
		{
			/* reset flag for client message received */
			etaChannelManagementInfo->pingManagementInfo.receivedClientMsg = RSSL_FALSE;

			/* set time server should receive next message/ping from client */
			etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;
		}
		else /* lost contact with client */
		{
			printf("\nLost contact with client...\n");
			/* Closes channel/connection, cleans up and exits the application. */
			return RSSL_RET_FAILURE;
		}
	}

	return retval;
}

