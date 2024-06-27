/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license
 *| AS IS with no warranty or guarantee of fit for purpose.
 *| See the project's LICENSE.md for details.
 *| Copyright (C) 2019 LSEG. All rights reserved.
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
 * other training consumer examples. The provider uses the rsslBind function to open the port 
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
 *
 ************************************************************************
 * ETA Interactive Provider Training Module 1c: Reading and Writing Data
 ************************************************************************
 * Summary:
 * In this module, when a client or server RsslChannel.state is 
 * RSSL_CH_STATE_ACTIVE, it is possible for an application to receive 
 * data from the connection. Similarly, when a client or server 
 * RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is possible for an 
 * application to write data to the connection. Writing involves a several 
 * step process. 
 *
 * Detailed Descriptions:
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to receive data from the connection. The 
 * arrival of this information is often announced by the I/O notification 
 * mechanism that the RsslChannel.socketId is registered with. The ETA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines RsslBuffer boundaries and returns each buffer one by 
 * one.
 *
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to write data to the connection. Writing 
 * involves a several step process. Because the ETA Transport provides 
 * efficient buffer management, the user is required to obtain a buffer 
 * from the ETA Transport buffer pool. This can be the guaranteed output 
 * buffer pool associated with an RsslChannel. After a buffer is acquired, 
 * the user can populate the RsslBuffer.data and set the RsslBuffer.length 
 * to the number of bytes referred to by data. If queued information cannot 
 * be passed to the network, a function is provided to allow the application 
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able 
 * to accept additional bytes for writing. The ETA Transport can continue to
 * queue data, even if the network is unable to write. 
 *
 *
 ************************************************************************
 * ETA Interactive Provider Training Module 2: Perform/Handle Login Process
 ************************************************************************
 * Summary:
 * Applications authenticate with one another using the Login domain model. 
 * An OMM Interactive Provider must handle the consumer�s Login request messages 
 * and supply appropriate responses.
 * 
 * In this module, after receiving a Login request, the Interactive Provider 
 * can perform any necessary authentication and permissioning.
 *
 * Detailed Descriptions:
 * After receiving a Login request, the Interactive Provider can perform any 
 * necessary authentication and permissioning.
 *
 * a) If the Interactive Provider grants access, it should send an RsslRefreshMsg 
 * to convey that the user successfully connected. This message should indicate 
 * the feature set supported by the provider application.
 * b) If the Interactive Provider denies access, it should send an RsslStatusMsg, 
 * closing the connection and informing the user of the reason for denial.
 *
 * The login handler for this simple Interactive Provider application only allows
 * one login stream per channel. It provides functions for processing login requests
 * from consumers and sending back the responses. Functions for sending login request
 * reject/close status messages, initializing the login handler, and closing login streams 
 * are also provided.
 *
 * Also please note for simple training app, the interactive provider only supports 
 * one client session from the consumer, that is, only supports one channel/client connection.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA 
 * Data Package. 
 *
 *
 ************************************************************************
 * ETA Interactive Provider Training Module 3: Provide Source Directory Information
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Source Directory 
 * information. The Source Directory domain model conveys information about all 
 * available services in the system. An OMM consumer typically requests a Source Directory 
 * to retrieve information about available services and their capabilities. 
 * 
 * Detailed Descriptions:
 * The Source Directory domain model conveys information about all available services 
 * in the system. An OMM consumer typically requests a Source Directory to retrieve 
 * information about available services and their capabilities. This includes information 
 * about supported domain types, the service�s state, the QoS, and any item group 
 * information associated with the service. LSEG recommends that at a minimum, 
 * an Interactive Provider supply the Info, State, and Group filters for the Source Directory.
 * 
 * a) The Source Directory Info filter contains the name and serviceId for each 
 * available service. The Interactive Provider should populate the filter with information 
 * specific to the services it provides.
 *
 * b) The Source Directory State filter contains status information for the service informing 
 * the consumer whether the service is Up (available), or Down (unavailable).
 *
 * c) The Source Directory Group filter conveys item group status information, including 
 * information about group states, as well as the merging of groups. If a provider determines 
 * that a group of items is no longer available, it can convey this information by sending 
 * either individual item status messages (for each affected stream) or a Directory message 
 * containing the item group status information. 
 * 
 * Content is encoded and decoded using the ETA Message Package and the ETA 
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

	char srvrPortNo[128], serviceName[128];

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

	/* ETA provides clear functions for its structures (e.g., rsslClearDecodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_DECODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

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
	/* default service name is "DIRECT_FEED" used in source directory handler */
	snprintf(serviceName, 128, "%s", "DIRECT_FEED");

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
			else if (strcmp("-s", argv[i]) == 0)
			{
				i += 2;
				snprintf(serviceName, 128, "%s", argv[i-1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-p <SrvrPortNo>] [-r <runTime>] [-s <ServiceName>] \n", argv[0], argv[0]);
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
							if ((etaSrvr) && (rsslCloseServer(etaSrvr, &error) < RSSL_RET_SUCCESS))
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

	/* Clears the login request information */
	clearLoginReqInfo(&etaChannelManagementInfo.loginRequestInfo);

	/* Clears the source directory request information */
	clearSourceDirectoryReqInfo(&etaChannelManagementInfo.sourceDirectoryRequestInfo);

	/* set the ServiceName and Service Id */
	snprintf(etaChannelManagementInfo.sourceDirectoryRequestInfo.ServiceName, 128, "%s", serviceName);
	/* service id associated with the service name of provider */
	etaChannelManagementInfo.sourceDirectoryRequestInfo.ServiceId = 1234;

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

					if ((msgBuf = rsslRead(etaChannelManagementInfo.etaChannel, &retval_rsslRead, &error)) != 0)
					{
						/* if a buffer is returned, we have data to process and code is success */

						/* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
						 * calling the applicable specific function for further processing.
						 */

						/* No need to clear the message before we decode into it. ETA Decoding populates all message members (and that is true for any
						 * decoding with ETA, you never need to clear anything but the iterator)
						 */
						RsslMsg msg;

						/* This rsslClearDecodeIterator clear iterator function should be used to achieve the best performance while clearing the iterator. */
						/* Clears members necessary for decoding and readies the iterator for reuse. You must clear RsslDecodeIterator
						 * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
						 */
						rsslClearDecodeIterator(&decodeIter);

						/* Set the RWF version to decode with this iterator */
						rsslSetDecodeIteratorRWFVersion(&decodeIter, etaChannelManagementInfo.etaChannel->majorVersion, etaChannelManagementInfo.etaChannel->minorVersion);

						/* Associates the RsslDecodeIterator with the RsslBuffer from which to decode. */
						if((retval = rsslSetDecodeIteratorBuffer(&decodeIter, msgBuf)) != RSSL_RET_SUCCESS)
						{
							printf("\nrsslSetDecodeIteratorBuffer() failed with return code: %d\n", retval);
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
						}

						/* decode contents into the RsslMsg structure */
						retval = rsslDecodeMsg(&decodeIter, &msg);
						if (retval != RSSL_RET_SUCCESS)
						{
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", retval, etaChannelManagementInfo.etaChannel->socketId, msgBuf->length);
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
						}

						switch ( msg.msgBase.domainType )
						{
							/*!< (1) Login Message */
							case RSSL_DMT_LOGIN:
							{
								if ((retval = processLoginRequest(&etaChannelManagementInfo, &msg, &decodeIter)) > RSSL_RET_SUCCESS)
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
									/* Clears the login request information */
									clearLoginReqInfo(&etaChannelManagementInfo.loginRequestInfo);

									/* Closes channel, closes server, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
								}
							}
							break;
							/*!< (4) Source Message */
							case RSSL_DMT_SOURCE:
							{
								if ((retval = processSourceDirectoryRequest(&etaChannelManagementInfo, &msg, &decodeIter)) > RSSL_RET_SUCCESS)
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
									/* Clears the source directory request information */
									clearSourceDirectoryReqInfo(&etaChannelManagementInfo.sourceDirectoryRequestInfo);

									/* Closes channel, closes server, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
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
						etaChannelManagementInfo.pingManagementInfo.receivedClientMsg = RSSL_TRUE;
						printf("Ping message has been received successfully from the client due to data message ... \n\n");
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
				retval = RSSL_RET_FAILURE;

				/* this section of code was called because of a write file descriptor alert */
				if ((retval = rsslFlush(etaChannelManagementInfo.etaChannel, &error)) > RSSL_RET_SUCCESS)
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
							FD_CLR(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds);
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
							closeChannelServerCleanUpAndExit(etaChannelManagementInfo.etaChannel, etaSrvr, RSSL_RET_FAILURE);
						}
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
			 * If the provider application must shut down, it can either leave consumer connections intact or shut them down.
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

/*
 * Sends a message buffer to a channel.
 * etaChannelManagementInfo.etaChannel - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* etaChannel, RsslBuffer* msgBuf)
{
	RsslError error;
	RsslRet	retval = 0;
	RsslUInt32 bytesWritten = 0;
	RsslUInt32 uncompressedBytesWritten = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS; /*!< (0x00) No Write Flags */

	/* send the request */

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 5:
	 * Write using rsslWriter
	 * rsslWriter performs any writing or queuing of data. This function expects the RsslChannel to be in the active state and the buffer to be properly populated,
	 * where length reflects the actual number of bytes used. This function allows for several modifications to be specified for this call. Here we use
	 * RSSL_WRITE_NO_FLAGS. For more information on other flag enumeration such as RSSL_WRITE_DO_NOT_COMPRESS or RSSL_WRITE_DIRECT_SOCKET_WRITE, see the ETA C
	 * developers guide for rsslWrite Flag Enumeration Values supported by ETA Transport.
	 *
	 * The ETA Transport also supports writing data at different priority levels.
	 * The application can pass in two integer values used for reporting information about the number of bytes that will be written. The uncompressedBytesWritten
	 * parameter will return the number of bytes to be written, including any transport header overhead but not taking into account any compression. The bytesWritten
	 * parameter will return the number of bytes to be written, including any transport header overhead and taking into account any compression. If compression is
	 * disabled, uncompressedBytesWritten and bytesWritten should match.
	 * The number of bytes saved through the compression process can be calculated by (bytesWritten - uncompressedBytesWritten).
	 * Note:
	 * Before passing a buffer to rsslWrite, it is required that the application set length to the number of bytes actually used. This ensures that only the required
	 * bytes are written to the network.
	 *********************************************************/

	/* Now write the data - keep track of ETA Transport return code -
	 * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
	 */

	/* this example writes buffer as high priority and no write modification flags */
	if ((retval = rsslWrite(etaChannel, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &bytesWritten, &uncompressedBytesWritten, &error)) == RSSL_RET_WRITE_CALL_AGAIN)
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
			if ((retval = rsslFlush(etaChannel, &error)) < RSSL_RET_SUCCESS)
			{
				printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			}
			/* call the rsslWrite function again with this same exact buffer to continue the fragmentation process. */
			retval = rsslWrite(etaChannel, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &bytesWritten, &uncompressedBytesWritten, &error);
		}
	}

	/* set write fd if there's still data queued */
	if (retval > RSSL_RET_SUCCESS)
	{
		/* The write was successful and there is more data queued in ETA Transport. The rsslFlush function should be used to continue attempting to flush data
		 * to the connection. ETA will release buffer.
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
				/* Continue with next operations. ETA will release buffer.*/
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
				/* The write was successful, but an attempt to flush failed. ETA will release buffer.*/
				/* Must check channel state to determine if this is unrecoverable or not */
				if (etaChannel->state == RSSL_CH_STATE_CLOSED)
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
 * Processes a login request. This consists of decoding the login request and calling
 * sendLoginResponse() to send the login response.
 * etaChannelInfo - The channel management information including the login request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginRequest(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter)
{
	RsslMsgKey* requestKey = 0;

	RsslRet retval = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			/* get request message key - retrieve the RsslMsgKey structure from the provided decoded message structure */
			requestKey = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* check if key has user name */
			/* user name is only login user type accepted by this application (user name is the default type) */
			if (!(requestKey->flags & RSSL_MKF_HAS_NAME) || ((requestKey->flags & RSSL_MKF_HAS_NAME_TYPE) && (requestKey->nameType != RDM_LOGIN_USER_NAME))) /*!< (1) Name */
			{
				if (sendLoginRequestRejectStatusMsg(etaChannelManagementInfo, msg->msgBase.streamId, NO_USER_NAME_IN_REQUEST) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* if stream id is different from last request, this is an invalid request */
			if (etaChannelManagementInfo->loginRequestInfo.IsInUse && (etaChannelManagementInfo->loginRequestInfo.StreamId != msg->msgBase.streamId))
			{
				if (sendLoginRequestRejectStatusMsg(etaChannelManagementInfo, msg->msgBase.streamId, MAX_LOGIN_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* decode login request */

			/* get StreamId */
			etaChannelManagementInfo->loginRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

			etaChannelManagementInfo->loginRequestInfo.IsInUse = RSSL_TRUE;

			/* get Username */
			if (requestKey->name.length < 128)
			{
				strncpy(etaChannelManagementInfo->loginRequestInfo.Username, requestKey->name.data, requestKey->name.length);
				etaChannelManagementInfo->loginRequestInfo.Username[requestKey->name.length] = '\0';
			}
			else
			{
				strncpy(etaChannelManagementInfo->loginRequestInfo.Username, requestKey->name.data, 127);
				etaChannelManagementInfo->loginRequestInfo.Username[128 - 1] = '\0';
			}

			/* decode key opaque data */

			/**
			 * @brief Allows the user to continue decoding of any message key attributes with the same \ref RsslDecodeIterator used when calling rsslDecodeMsg
			 *
			 * Typical use:<BR>
			 *  1. Call rsslDecodeMsg()<BR>
			 *  2. If there are any message key attributes and the application wishes to decode them using the same \ref RsslDecodeIterator, call rsslDecodeMsgKeyAttrib() and continue decoding using the appropriate container type decode functions, as indicated by RsslMsgKey::attribContainerType<BR>
			 *  3. If payload is present and the application wishes to decode it, use the appropriate decode functions, as specified in \ref RsslMsgBase::containerType<BR>
			 */
			if ((retval = rsslDecodeMsgKeyAttrib(decodeIter, requestKey)) < RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeMsgKeyAttrib() failed with return code: %d\n", retval);
				return retval;
			}

			/* decode element list */

			/**
			 * @brief Decodes an RsslElementList container
			 *
			 * Typical use:<BR>
			 *  1. Call rsslDecodeElementList()<BR>
			 *  2. Call rsslDecodeElementEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
			 */
			if ((retval = rsslDecodeElementList(decodeIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
			{
				/* decode each element entry in list */
				while ((retval = rsslDecodeElementEntry(decodeIter, &element)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (retval == RSSL_RET_SUCCESS)
					{
						/* get login request information */

						/* ApplicationId */
						if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPID))
						{
							if (element.encData.length < 128)
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.ApplicationId, element.encData.data, element.encData.length);
								etaChannelManagementInfo->loginRequestInfo.ApplicationId[element.encData.length] = '\0';
							}
							else
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.ApplicationId, element.encData.data, 127);
								etaChannelManagementInfo->loginRequestInfo.ApplicationId[128 - 1] = '\0';
							}
						}

						/* ApplicationName */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
						{
							if (element.encData.length < 128)
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.ApplicationName, element.encData.data, element.encData.length);
								etaChannelManagementInfo->loginRequestInfo.ApplicationName[element.encData.length] = '\0';
							}
							else
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.ApplicationName, element.encData.data, 127);
								etaChannelManagementInfo->loginRequestInfo.ApplicationName[128 - 1] = '\0';
							}
						}

						/* Position */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
						{
							if (element.encData.length < 128)
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.Position, element.encData.data, element.encData.length);
								etaChannelManagementInfo->loginRequestInfo.Position[element.encData.length] = '\0';
							}
							else
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.Position, element.encData.data, 127);
								etaChannelManagementInfo->loginRequestInfo.Position[128 - 1] = '\0';
							}
						}

						/* Password */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PASSWORD))
						{
							if (element.encData.length < 128)
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.Password, element.encData.data, element.encData.length);
								etaChannelManagementInfo->loginRequestInfo.Password[element.encData.length] = '\0';
							}
							else
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.Password, element.encData.data, 127);
								etaChannelManagementInfo->loginRequestInfo.Password[128 - 1] = '\0';
							}
						}

						/* InstanceId */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_INST_ID))
						{
							if (element.encData.length < 128)
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.InstanceId, element.encData.data, element.encData.length);
								etaChannelManagementInfo->loginRequestInfo.InstanceId[element.encData.length] = '\0';
							}
							else
							{
								strncpy(etaChannelManagementInfo->loginRequestInfo.InstanceId, element.encData.data, 127);
								etaChannelManagementInfo->loginRequestInfo.InstanceId[128 - 1] = '\0';
							}
						}

						/* Role */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ROLE))
						{
							retval = rsslDecodeUInt(decodeIter, &etaChannelManagementInfo->loginRequestInfo.Role);
							if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
							{
								printf("rsslDecodeUInt() failed with return code: %d\n", retval);
								return retval;
							}
						}
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

			printf("\nReceived Login Request for Username: %.*s\n", (int)strlen(etaChannelManagementInfo->loginRequestInfo.Username), etaChannelManagementInfo->loginRequestInfo.Username);

			/* send login response */
			if (retval = sendLoginResponse(etaChannelManagementInfo) != RSSL_RET_SUCCESS)
				return retval;
		}
		break;

		case RSSL_MC_CLOSE:
		{
			printf("\nReceived Login Close for StreamId %d\n", msg->msgBase.streamId);

			/* close login stream */
			closeLoginStream(msg->msgBase.streamId, etaChannelManagementInfo);
		}
		break;

		default:
		{
			printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
    		return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a Login refresh response to a channel. This consists of getting a message buffer, setting the login response information,
 * encoding the login response, and sending the login response to the client. If the Interactive Provider grants access, it should
 * send an RsslRefreshMsg to convey that the user successfully connected. This message should indicate the feature set supported by
 * the provider application.
 * etaChannelInfo - The channel management information including the login request information and
 * including the channel to send a login refresh response to
 */
RsslRet sendLoginResponse(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer applicationId, applicationName, position;
	char hostName[256], stateText[128];

	RsslUInt64	supportBatchRequests;

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Login response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Login refresh response. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRefreshMsg(&refreshMsg);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* provide login refresh response information */

	/* set refresh flags */

	/* set-up message */
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH; /*!< (2) Refresh Message */
	refreshMsg.msgBase.domainType = RSSL_DMT_LOGIN; /*!< (1) Login Message */
	refreshMsg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x0008) The RsslRefreshMsg has a message key, contained in \ref RsslRefreshMsg::msgBase::msgKey. */
	/*!< (0x0020) Indicates that this RsslRefreshMsg is a solicited response to a consumer's request. */
	/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
	/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;

	/*!< (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg) */
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;

	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;
	sprintf(stateText, "Login accepted by host ");
	if (gethostname(hostName, sizeof(hostName)) != 0)
	{
		sprintf(hostName, "localhost");
	}
	strcat(stateText, hostName);
	refreshMsg.state.text.data = stateText;
	refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);

	/* provide login response information */

	/* StreamId - just set the Login response stream id info to be the same as the Login request stream id info */
	refreshMsg.msgBase.streamId = etaChannelManagementInfo->loginRequestInfo.StreamId;

	/* set msgKey members */
	/*!< (0x0020) This RsslMsgKey has additional attribute information, contained in \ref RsslMsgKey::encAttrib. The container type of the attribute information is contained in \ref RsslMsgKey::attribContainerType. */
	/*!< (0x0004) This RsslMsgKey has a nameType enumeration, contained in \ref RsslMsgKey::nameType. */
	/*!< (0x0002) This RsslMsgKey has a name buffer, contained in \ref RsslMsgKey::name.  */
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME;

	/* Username */
	refreshMsg.msgBase.msgKey.name.data = etaChannelManagementInfo->loginRequestInfo.Username;
	refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(etaChannelManagementInfo->loginRequestInfo.Username);
	refreshMsg.msgBase.msgKey.nameType = RDM_LOGIN_USER_NAME; /*!< (1) Name */

	refreshMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* encode message */

	/* since our msgKey has opaque that we want to encode, we need to use rsslEncodeMsgInit */

	/* rsslEncodeMsgInit should return and inform us to encode our key opaque */

	/**
	 * @brief 	Begin encoding process for an RsslMsg.
	 *
	 * Begins encoding of an RsslMsg<BR>
	 * Typical use:<BR>
	 *  1. Populate desired members on the RsslMsg<BR>
	 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
	 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
	 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
	 *  5. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
	 *  6. Call rsslEncodeMsgComplete() when all content is completed<BR>
	 */
	if ((retval = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* encode our msgKey opaque */

	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA; /*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined). */

	/**
	 * @brief 	Begin encoding process for RsslElementList container type.
	 *
	 * Begins encoding of an RsslElementList<BR>
	 * Typical use:<BR>
	 *  1. Call rsslEncodeElementListInit()<BR>
	 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
	 *  3. Call rsslEncodeElementListComplete() when all entries are completed<BR>
	 */
	if ((retval = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementListInit() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* ApplicationId */
	applicationId.data = (char*)"256";
	applicationId.length = (RsslUInt32)strlen("256");
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPID;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &applicationId)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* ApplicationName */
	applicationName.data = (char*)"ETA Provider Training";
	applicationName.length = (RsslUInt32)strlen("ETA Provider Training");
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPNAME;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &applicationName)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* Position - just set the Login response position info to be the same as the Login request position info */
	position.data = (char*)etaChannelManagementInfo->loginRequestInfo.Position;
	position.length = (RsslUInt32)strlen(etaChannelManagementInfo->loginRequestInfo.Position);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_POSITION;
	if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &position)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* SupportBatchRequests */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_BATCH;
	supportBatchRequests = 0; /* this simple provider does not support batch requests */
	if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &supportBatchRequests)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode element list - Completes encoding of an RsslElementList */
	if ((retval = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode key */

	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	   for us to encode our container/msg payload */
	if ((retval = rsslEncodeMsgKeyAttribComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgKeyAttribComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer�s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login response */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* Closes channel, closes server, cleans up and exits the application. */
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
 * Closes a login stream.
 * streamId - The stream id to close the login for
 * etaChannelInfo - The channel management information including the login request information
 */
void closeLoginStream(RsslInt32 streamId, EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* find original request information associated with streamId */
	if(etaChannelManagementInfo->loginRequestInfo.StreamId == streamId)
	{
		printf("Closing login stream id %d with user name: %.*s\n", etaChannelManagementInfo->loginRequestInfo.StreamId, (int)strlen(etaChannelManagementInfo->loginRequestInfo.Username), etaChannelManagementInfo->loginRequestInfo.Username);

		/* Clears the original login request information */
		clearLoginReqInfo(&etaChannelManagementInfo->loginRequestInfo);
	}
}

/*
 * etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplifies the example codes and make the codes more readable.
 */
RsslBuffer* etaGetBuffer(RsslChannel *etaChannel, RsslUInt32 size, RsslError *rsslError)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for any request Msg.
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
	if ((msgBuf = rsslGetBuffer(etaChannel, size, RSSL_FALSE, &error)) == NULL) /* first check Error */
	{
		/* Check to see if this is just out of buffers or if it�s unrecoverable */
		if (error.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
		{
			/* it�s unrecoverable Error */
			printf("Error %s (%d) (errno: %d) encountered with rsslGetBuffer. Error Text: %s\n",
				rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
			/* Connection should be closed, return failure */
			/* Closes channel, closes server, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/*!< (-4) Transport Failure: There are no buffers available from the buffer pool, returned from rsslGetBuffer.
		 * This can happen if the reader isn't keeping up and/or we have a lot of write threads in multithreaded apps.
		 * Use rsslIoctl to increase pool size or use rsslFlush to flush data and return buffers to pool.
		 */

		/* The rsslFlush function could be used to attempt to free buffers back to the pool */
		retval = rsslFlush(etaChannel, &error);
		if (retval < RSSL_RET_SUCCESS)
		{
			printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			/* Closes channel, closes server, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/* call rsslGetBuffer again to see if it works now after rsslFlush */
		if ((msgBuf = rsslGetBuffer(etaChannel, size, RSSL_FALSE, &error)) == NULL)
		{
			printf("Error %s (%d) (errno: %d) encountered with rsslGetBuffer. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
			/* Closes channel, closes server, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}
	}

	/* return RSSL buffer to be filled in with valid memory */
	return msgBuf;
}

/*
 * Sends the login request reject status message for a channel.
 * etaChannelInfo - The channel management information including the login request information and
 * including the channel to send the login request reject status message to
 * streamId - The stream id of the login request reject status
 * reason - The reason for the reject
 */
RsslRet sendLoginRequestRejectStatusMsg(EtaChannelManagementInfo *etaChannelManagementInfo, RsslInt32 streamId, EtaLoginRejectReason reason)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the login request reject status message. */
	RsslStatusMsg msg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Login request Reject Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the login request reject status. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearStatusMsg(&msg);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS; /*!< (3) Status Message */
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN; /*!< (1) Login Message */
	/* No payload associated with this close status message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	msg.flags = RSSL_STMF_HAS_STATE;
	/*!< (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;

	switch(reason)
	{
		case MAX_LOGIN_REQUESTS_REACHED:
			/*!< (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
			msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
			sprintf(stateText, "Login request rejected for stream id %d - max request count reached", streamId);
			msg.state.text.data = stateText;
			msg.state.text.length = (RsslUInt32)strlen(stateText);
			break;
		case NO_USER_NAME_IN_REQUEST:
			/*!< (5) Usage Error (indicates an invalid usage within the system) */
			msg.state.code = RSSL_SC_USAGE_ERROR;
			sprintf(stateText, "Login request rejected for stream id %d - request does not contain user name", streamId);
			msg.state.text.data = stateText;
			msg.state.text.length = (RsslUInt32)strlen(stateText);
			break;
		default:
			break;
	}

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
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set the buffer�s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login request reject status */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* send login request reject status fails */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the login request reject status doesn't flush, just close channel and exit the app. When you send login request reject status msg,
		 * we want to make a best effort to get this across the network as it will gracefully close the open login
		 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
		 */

		/* Closes channel, closes server, cleans up and exits the application. */
	}

	return retval;
}

/*
 * Processes a source directory request. This consists of decoding the source directory request and calling
 * sendSourceDirectoryResponse() to send the source directory response.
 * etaChannelInfo - The channel management information including the source directory request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processSourceDirectoryRequest(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter)
{
	RsslMsgKey* requestKey = 0;

	RsslRet retval = 0;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			/* get request message key - retrieve the RsslMsgKey structure from the provided decoded message structure */
			requestKey = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* check if key has minimal filter flags -
			 * Does key have minimal filter flags.  Request key must minimally have
			 * RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER,
			 * and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags.
			 */
			if (!(requestKey->flags & RSSL_MKF_HAS_FILTER) &&
				(requestKey->filter & RDM_DIRECTORY_SERVICE_INFO_FILTER) &&
				(requestKey->filter & RDM_DIRECTORY_SERVICE_STATE_FILTER) &&
				(requestKey->filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER))
			{
				if (sendSrcDirectoryRequestRejectStatusMsg(etaChannelManagementInfo, msg->msgBase.streamId, INCORRECT_FILTER_FLAGS) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* if stream id is different from last request, this is an invalid request */
			if (etaChannelManagementInfo->sourceDirectoryRequestInfo.IsInUse && (etaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId != msg->msgBase.streamId))
			{
				if (sendSrcDirectoryRequestRejectStatusMsg(etaChannelManagementInfo, msg->msgBase.streamId, MAX_SRCDIR_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* decode source directory request */

			/* get StreamId */
			etaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

			etaChannelManagementInfo->sourceDirectoryRequestInfo.IsInUse = RSSL_TRUE;

			printf("\nReceived Source Directory Request\n");

			/* send source directory response */
			if (retval = sendSourceDirectoryResponse(etaChannelManagementInfo, etaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName, etaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceId) != RSSL_RET_SUCCESS)
				return retval;
		}
		break;

		case RSSL_MC_CLOSE:
		{
			printf("\nReceived Source Directory Close for StreamId %d\n", msg->msgBase.streamId);

			/* close source directory stream */
			closeSourceDirectoryStream(msg->msgBase.streamId, etaChannelManagementInfo);
		}
		break;

		default:
		{
			printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg->msgBase.msgClass);
    		return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Send Source Directory response to a channel. This consists of getting a message buffer, setting the source directory
 * response information, encoding the source directory response, and sending the source directory response to
 * the consumer. The Source Directory domain model conveys information about all available services in the system.
 * An OMM consumer typically requests a Source Directory to retrieve information about available services and their capabilities.
 * etaChannelInfo - The channel management information including the source directory request information
 * serviceName - The service name specified by the OMM interactive provider application (Optional to set)
 * serviceId - the serviceId specified by the OMM interactive provider  application (Optional to set)
 */
RsslRet sendSourceDirectoryResponse(EtaChannelManagementInfo *etaChannelManagementInfo, char serviceName[128], RsslUInt64 serviceId)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* The refresh flags of the source directory response */
	RsslUInt16 refreshFlags = 0;

	/* The refresh key with filter flags that indicate the filter entries to include */
	RsslMsgKey refreshKey = RSSL_INIT_MSG_KEY;

	RsslInt32 streamId;

	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFilterList sourceDirectoryFilterList = RSSL_INIT_FILTER_LIST;
	char stateText[256];

	RsslFilterEntry filterListItem = RSSL_INIT_FILTER_ENTRY;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslArray rsslArray = RSSL_INIT_ARRAY;

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Source Directory response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Source Directory refresh response. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRefreshMsg(&refreshMsg);

	/* provide source directory response information */

	/* set refresh flags */
	/* The content of a Source Directory Refresh message is expected to be atomic and contained in a single part,
	 * therefore RSSL_RFMF_REFRESH_COMPLETE should be set.
	 */

	/*!< (0x0008) The RsslRefreshMsg has a message key, contained in \ref RsslRefreshMsg::msgBase::msgKey. */
	/*!< (0x0020) Indicates that this RsslRefreshMsg is a solicited response to a consumer's request. */
	/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
	/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
	refreshFlags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;

	/* set filter flags */
	/* At a minimum, LSEG recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
	refreshKey.filter =	RDM_DIRECTORY_SERVICE_INFO_FILTER | \
						RDM_DIRECTORY_SERVICE_STATE_FILTER| \
						/* RDM_DIRECTORY_SERVICE_GROUP_FILTER | \ not applicable for refresh message - here for reference */
						RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
						/* RDM_DIRECTORY_SERVICE_DATA_FILTER | \ not applicable for non-ANSI Page based provider - here for reference */
						RDM_DIRECTORY_SERVICE_LINK_FILTER;

	/* StreamId */
	streamId = etaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId;

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH; /*!< (2) Refresh Message */
	refreshMsg.msgBase.domainType = RSSL_DMT_SOURCE; /*!< (4) Source Message */
	/*!< (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.flags = refreshFlags;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;
	sprintf(stateText, "Source Directory Refresh Completed");
	refreshMsg.state.text.data = stateText;
	refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);

	/* set members in msgKey */
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
	refreshMsg.msgBase.msgKey.filter = refreshKey.filter;

	/* StreamId */
	refreshMsg.msgBase.streamId = streamId;

	/* encode message - populate message and encode it */
	if ((retval = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* encode map */
	map.keyPrimitiveType = RSSL_DT_UINT;
	map.containerType = RSSL_DT_FILTER_LIST;

	/**
	 * @brief 	Begins encoding of an RsslMapEntry, where any payload is encoded after this call using the appropriate container type encode functions, as specified by RsslMap::containerType.
	 *
	 * Begins encoding of an RsslMapEntry<BR>
	 * Typical use:<BR>
	 *  1. Call rsslEncodeMapInit()<BR>
	 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
	 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
	 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
	 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
	 */
	if ((retval = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapInit() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* encode map entry */
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	if ((retval = rsslEncodeMapEntryInit(&encodeIter, &mapEntry, &serviceId, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapEntry() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* encode filter list */
	sourceDirectoryFilterList.containerType = RSSL_DT_ELEMENT_LIST;

	/**
	 * @brief 	Begin encoding process for RsslFilterList container type.
	 *
	 * Begins encoding of an RsslFilterList<BR>
	 * Typical use:<BR>
	 *  1. Call rsslEncodeFilterListInit()<BR>
	 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
	 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
	 */
	if ((retval = rsslEncodeFilterListInit(&encodeIter, &sourceDirectoryFilterList)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFilterListInit() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* encode filter list items -
	 * for each filter list, element name use default values if not set
	 */

	/*!< (0x00000001) Source Info Filter Mask */
	if (refreshKey.filter & RDM_DIRECTORY_SERVICE_INFO_FILTER)
	{
		/* Encodes the service's general information. */

		RsslUInt64 capabilities[10];
		RsslQos QoS[5];

		RsslBuffer tempBuffer;
		int i;

		rsslClearFilterEntry(&filterListItem);
		rsslClearElementEntry(&element);
		rsslClearElementList(&elementList);
		rsslClearArray(&rsslArray);

		/* encode filter list item */
		filterListItem.id = RDM_DIRECTORY_SERVICE_INFO_ID; /*!< (1) Service Info Filter ID */
		filterListItem.action = RSSL_FTEA_SET_ENTRY;
		if ((retval = rsslEncodeFilterEntryInit(&encodeIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* encode the element list */
		/*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		if ((retval = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* ServiceName */
		tempBuffer.data = serviceName;
		tempBuffer.length = (RsslUInt32)strlen(serviceName);
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_NAME;
		if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* Capabilities */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_CAPABILITIES;
		if ((retval = rsslEncodeElementEntryInit(&encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/*  Set RsslArray.itemLength to 1, as each domainType uses only one byte. */
		rsslArray.itemLength = 1;
		rsslArray.primitiveType = RSSL_DT_UINT;
		if ((retval = rsslEncodeArrayInit(&encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* Lists the domains which this service can provide.*/
		capabilities[0] = RSSL_DMT_DICTIONARY; /*!< (5) Dictionary Message */
		capabilities[1] = RSSL_DMT_MARKET_PRICE; /*!< (6) Market Price Message */
		capabilities[2] = RSSL_DMT_MARKET_BY_ORDER; /*!< (7) Market by Order/Order Book Message */
		capabilities[3] = RSSL_DMT_SYMBOL_LIST; /*!< (10) Symbol List Messages */
		capabilities[4] = RSSL_DMT_YIELD_CURVE; /*!< (22) Yield Curve */
		for (i = 5; i < 10; i++)
		{
			capabilities[i] = 0;
		}

		/* break out of decoding array items when predetermined max capabilities (10) reached */
		for(i = 0; i < 10 && capabilities[i] != 0; i++)
		{
			if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &capabilities[i])) < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
				/* Closes channel, closes server, cleans up and exits the application. */
				return retval;
			}
		}
		if ((retval = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* DictionariesProvided */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_DICTIONARYS_PROVIDED;
		if ((retval = rsslEncodeElementEntryInit(&encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
		if ((retval = rsslEncodeArrayInit(&encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFFld";
		tempBuffer.length = (RsslUInt32)strlen("RWFFld");
		if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFEnum";
		tempBuffer.length = (RsslUInt32)strlen("RWFEnum");
		if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* DictionariesUsed */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_DICTIONARYS_USED;
		if ((retval = rsslEncodeElementEntryInit(&encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
		if ((retval = rsslEncodeArrayInit(&encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFFld";
		tempBuffer.length = (RsslUInt32)strlen("RWFFld");
		if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFEnum";
		tempBuffer.length = (RsslUInt32)strlen("RWFEnum");
		if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* QoS */
		QoS[0].dynamic = RSSL_FALSE;
		/*!< Rate is Tick By Tick, indicates every change to information is conveyed */
		QoS[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;
		/*!< Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
		QoS[0].timeliness = RSSL_QOS_TIME_REALTIME;

		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_QOS;
		if ((retval = rsslEncodeElementEntryInit(&encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		rsslClearArray(&rsslArray);
		/* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_QOS;

		/**
		 * @brief Perform array item encoding (item can only be simple primitive type such as \ref RsslInt, RsslReal, or RsslDate and not another RsslArray or container type)
		 *
		 * Encodes entries in an RsslArray.<BR>
		 * Typical use:<BR>
		 *	1. Call rsslEncodeArrayInit()<BR>
		 *	2. Call rsslEncodeArrayEntry() for each item in the array<BR>
		 *	3. Call rsslEncodeArrayComplete()<BR>
		 *
		 * @note Only one of pEncBuffer or pData should be supplied.
		 */
		if ((retval = rsslEncodeArrayInit(&encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayEntry(&encodeIter, 0, &QoS[0])) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		if ((retval = rsslEncodeElementEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* complete encode element list */
		if ((retval = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* complete encode filter list item */
		if ((retval = rsslEncodeFilterEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
	}

	/*!< (0x00000002) Source State Filter Mask */
	if (refreshKey.filter & RDM_DIRECTORY_SERVICE_STATE_FILTER)
	{
		/* Encodes the service's state information. */

		RsslUInt64 serviceState;
		RsslUInt64 acceptingRequests;

		/* Specifies a status change to apply to all items provided by this service.
			* It is equivalent to sending an RsslStatusMsg to each item.
			*/
		RsslState status;

		rsslClearFilterEntry(&filterListItem);
		rsslClearElementEntry(&element);
		rsslClearElementList(&elementList);

		/* encode filter list item */
		filterListItem.id = RDM_DIRECTORY_SERVICE_STATE_ID; /*!< (2) Source State Filter ID */
		filterListItem.action = RSSL_FTEA_SET_ENTRY;
		if ((retval = rsslEncodeFilterEntryInit(&encodeIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* encode the element list */
		/*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		if ((retval = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListInit() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* ServiceState */
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_SVC_STATE;
		/* Indicates whether the original provider of the data is available to respond to new requests. */
		serviceState = 1; /* Service is Up */
		if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &serviceState)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* AcceptingRequests */
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_ACCEPTING_REQS;
		/* Indicates whether the immediate provider can accept new requests and/or handle reissue requests on already open streams. */
		acceptingRequests = 1; /* 1: Yes */
		if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &acceptingRequests)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* Status */
		element.dataType = RSSL_DT_STATE;
		element.name = RSSL_ENAME_STATUS;
		/* The Status element can change the state of items provided by this service.
		 * Prior to changing a service status, LSEG recommends that you issue item or group
		 * status messages to update item states.
		 */
		status.streamState = RSSL_STREAM_OPEN;
		status.dataState = RSSL_DATA_OK;
		status.code = RSSL_SC_NONE;
		status.text.data = (char *)"OK";
		status.text.length = (RsslUInt32)strlen("OK");
		if ((retval = rsslEncodeElementEntry(&encodeIter, &element, &status)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* complete encode element list */
		if ((retval = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}

		/* complete encode filter list item */
		if ((retval = rsslEncodeFilterEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, closes server, cleans up and exits the application. */
			return retval;
		}
	}

	/* complete encode filter list */
	if ((retval = rsslEncodeFilterListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFilterListComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode map entry */
	if ((retval = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode map */
	if ((retval = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer�s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send source directory response */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* Closes channel, closes server, cleans up and exits the application. */
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
 * Sends the source directory request reject status message for a channel.
 * etaChannelInfo - The channel management information including the source directory request information and
 * including the channel to send the source directory request reject status message to
 * streamId - The stream id of the source directory request reject status
 * reason - The reason for the reject
 */
RsslRet sendSrcDirectoryRequestRejectStatusMsg(EtaChannelManagementInfo *etaChannelManagementInfo, RsslInt32 streamId, EtaSourceDirectoryRejectReason reason)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the source directory request reject status message. */
	RsslStatusMsg msg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Source Directory request Reject Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the source directory request reject status. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearStatusMsg(&msg);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS; /*!< (3) Status Message */
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SOURCE; /*!< (4) Source Message */
	/* No payload associated with this close status message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	msg.flags = RSSL_STMF_HAS_STATE;
	/*!< (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;

	switch(reason)
	{
		case MAX_SRCDIR_REQUESTS_REACHED:
			/*!< (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
			msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
			sprintf(stateText, "Source directory request rejected for stream id %d - max request count reached", streamId);
			msg.state.text.data = stateText;
			msg.state.text.length = (RsslUInt32)strlen(stateText);
			break;
		case INCORRECT_FILTER_FLAGS:
			/*!< (5) Usage Error (indicates an invalid usage within the system) */
			msg.state.code = RSSL_SC_USAGE_ERROR;
			sprintf(stateText, "Source directory request rejected for stream id %d - request must minimally have RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER, and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags", streamId);
			msg.state.text.data = stateText;
			msg.state.text.length = (RsslUInt32)strlen(stateText);
			break;
		default:
			break;
	}

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
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set the buffer�s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send source directory request reject status */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* send source directory request reject status fails */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the source directory request reject status doesn't flush, just close channel and exit the app. When you send source directory request reject status msg,
		 * we want to make a best effort to get this across the network as it will gracefully close the open source directory
		 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
		 */

		/* Closes channel, closes server, cleans up and exits the application. */
	}

	return retval;
}

/*
 * Closes a source directory stream.
 * streamId - The stream id to close the source directory for
 * etaChannelInfo - The channel management information including the source directory request information
 */
void closeSourceDirectoryStream(RsslInt32 streamId, EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* find original request information associated with streamId */
	if(etaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId == streamId)
	{
		printf("Closing source directory stream id %d with service name: %.*s\n", etaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId, (int)strlen(etaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName), etaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName);

		/* Clears the original source directory request information */
		clearSourceDirectoryReqInfo(&etaChannelManagementInfo->sourceDirectoryRequestInfo);
	}
}

