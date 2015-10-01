/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright Thomson Reuters 2015. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */


/*
 * This is the UPA Interactive Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Interactive Provider using the UPA Transport layer.
 *
 * Main c source file for the UPA Interactive Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 1a: Establish network communication
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
 * The first step of any UPA Interactive Provider application is to establish 
 * a listening socket, usually on a well-known port so that consumer applications 
 * can easily connect. The provider uses the rsslBind function to open the port 
 * and listen for incoming connection attempts.
 * Whenever an OMM consumer application attempts to connect, the provider uses 
 * the rsslAccept function to begin the connection initialization process.
 * 
 * For this simple training app, the interactive provider only supports a single client. 
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

#include "UPAProvider_Training.h"

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
	/* UPA Server structure returned via the rsslBind call */
	RsslServer* upaSrvr = 0;

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

	RsslError error;

	/* UPA Bind Options used in the rsslBind call. */
	RsslBindOptions bindOpts = RSSL_INIT_BIND_OPTS;

	/* UPA Accept Options used in the rsslAccept call */
	RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

	/* RsslInProgInfo Information for the In Progress Connection State */
	RsslInProgInfo inProgInfo = RSSL_INIT_IN_PROG_INFO;

	/* UPA channel management information */
	UpaChannelManagementInfo upaChannelManagementInfo;

	/* For this simple training app, the interactive provider only supports a single client. If the consumer disconnects,
	 * the interactive provider would simply exit.
	 *
	 * If you want the provider to support multiple client sessions at the same time, you need to implement support
	 * for multiple client sessions feature similar to what rsslProvider example is doing.
	 */
	upaChannelManagementInfo.upaChannel = 0;

	/* the default option parameters */
	/* server is running on port number 14002 */
	snprintf(srvrPortNo, 128, "%s", "14002");

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
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-p <SrvrPortNo>] \n", argv[0], argv[0]);
				exit(RSSL_RET_FAILURE);
			}
		}
	}

	/******************************************************************************************************************
				INITIALIZATION - USING rsslInitialize()
	******************************************************************************************************************/
	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 1:
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

	/* populate bind options, then pass to rsslBind function -
	 * UPA Transport should already be initialized
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

	/* Bind UPA server */
	if ((upaSrvr = rsslBind(&bindOpts, &error)) != NULL)
		printf("\nServer IPC descriptor = %d bound on port %d\n", upaSrvr->socketId, upaSrvr->portNumber);
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
	FD_SET(upaSrvr->socketId, &cleanReadFds);
	FD_SET(upaSrvr->socketId, &cleanExceptFds);

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
			if (FD_ISSET(upaSrvr->socketId, &useReadFds))
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

				/* populate accept options, then pass to rsslAccept function - UPA Transport should already be initialized */

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
				if ((upaChannelManagementInfo.upaChannel = rsslAccept(upaSrvr, &acceptOpts, &error)) == 0)
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

					printf("\nServer fd=%d: New client on Channel fd=%d\n",
						upaSrvr->socketId,upaChannelManagementInfo.upaChannel->socketId);

					/* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
					/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
					FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanReadFds);
					FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanExceptFds);

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
	while (upaChannelManagementInfo.upaChannel->state != RSSL_CH_STATE_ACTIVE)
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
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE);
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
			if (FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useReadFds) || FD_ISSET(upaChannelManagementInfo.upaChannel->socketId, &useExceptFds))
			{

				/*********************************************************
				 * Server/Provider Application Liefcycle Major Step 4:
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
					printf("Error %s (%d) (errno: %d) encountered with rsslInitChannel fd=%d. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannelManagementInfo.upaChannel->socketId, error.text);
					/* Closes channel, closes server, cleans up and exits the application. */
					closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE);
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
								printf("\nChannel In Progress - New FD: %d  Old FD: %d\n",upaChannelManagementInfo.upaChannel->socketId, inProgInfo.oldSocket );

								/* File descriptor has changed, unregister old and register new */
								FD_CLR(inProgInfo.oldSocket, &cleanReadFds);
								FD_CLR(inProgInfo.oldSocket, &cleanExceptFds);
								/* newSocket should equal upaChannelManagementInfo.upaChannel->socketId */
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanReadFds);
								FD_SET(upaChannelManagementInfo.upaChannel->socketId, &cleanExceptFds);
							}
							else
							{
								printf("\nChannel %d In Progress...\n", upaChannelManagementInfo.upaChannel->socketId);
							}
						}
						break;

						/* channel connection becomes active!
						 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state,
						 * this RsslChannel can be used for other transport operations.
						 */
						case RSSL_RET_SUCCESS:
						{
							printf("\nChannel on fd %d is now active - reading and writing can begin.\n", upaChannelManagementInfo.upaChannel->socketId);

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
								/* Closes channel, closes server, cleans up and exits the application. */
								closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE);
							}

							printf( "Channel %d active. Channel Info:\n"
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

							/* do not allow new client to connect  */

							/* For this simple training app, the interactive provider only supports a single client. Once a client
							 * successfully connects, we call rsslCloseServer function to close the listening socket associated with the
							 * RsslServer. The connected RsslChannels will remain open. This allows the established connection to continue
							 * to send and receive data, while preventing new clients from connecting.
							 */

							/* clean up server */
							FD_CLR(upaSrvr->socketId, &cleanReadFds);
							FD_CLR(upaSrvr->socketId, &cleanExceptFds);

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
							 * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
							 * The listening socket can be closed by calling rsslCloseServer. This prevents any new connection attempts.
							 * If shutting down connections for all connected clients, the provider should call rsslCloseChannel for each connection client.
							*/
							if ((upaSrvr) && (rsslCloseServer(upaSrvr, &error) < RSSL_RET_SUCCESS))
							{
								printf("Error %s (%d) (errno: %d) encountered with rsslCloseServer.  Error Text: %s\n",
									rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

								/* End application, uninitialize to clean up first */
								rsslUninitialize();
								exit(RSSL_RET_FAILURE);
							}

							// set upaSrvr to be NULL
							upaSrvr = 0;
						}
						break;
						default: /* Error handling */
						{
							printf("\nBad return value fd=%d <%s>\n",
								upaChannelManagementInfo.upaChannel->socketId, error.text);
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE);
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
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE);
		}
	}
}

/*
 * Closes channel, closes server, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * upaSrvr - The RsslServer that represents the listening socket connection to the user to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelServerCleanUpAndExit(RsslChannel* upaChannel, RsslServer* upaSrvr, int code)
{
	RsslRet	retval = 0;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 6:
	 * Close connection using rsslCloseChannel (OS connection release handshake)
	 * rsslCloseChannel closes the server based RsslChannel. This will release any pool based resources
	 * back to their respective pools, close the connection, and perform any additional necessary cleanup.
	 * When shutting down the UPA Transport, the application should release all unwritten pool buffers.
	 * Calling rsslCloseChannel terminates the connection for each connection client.
	 *********************************************************/
	if ((upaChannel) && (retval = rsslCloseChannel(upaChannel, &error) < RSSL_RET_SUCCESS))
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
	 * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
	 * The listening socket can be closed by calling rsslCloseServer. This prevents any new connection attempts.
	 * If shutting down connections for all connected clients, the provider should call rsslCloseChannel for each connection client.
	*/
	if ((upaSrvr) && (retval = rsslCloseServer(upaSrvr, &error) < RSSL_RET_SUCCESS))
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseServer.  Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Server/Provider Application Liefcycle Major Step 8:
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
		printf("\nUPA Interactive Provider Training application successfully ended.\n");

	/* End application */
	exit(code);
}