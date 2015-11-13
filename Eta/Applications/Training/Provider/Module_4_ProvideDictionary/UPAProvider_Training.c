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
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 1b: Ping (heartbeat) Management
 ************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might 
 * need to be exchanged. The negotiated ping timeout is available via 
 * the RsslChannel. If ping heartbeats are not sent or received within 
 * the expected time frame, the connection can be terminated. Thomson 
 * Reuters recommends sending ping messages at intervals one-third the 
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Once the connection is active, the consumer and provider applications 
 * might need to exchange ping messages. A negotiated ping timeout is available 
 * via RsslChannel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats 
 * are not sent or received within the expected time frame. Thomson Reuters 
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
 * UPA Interactive Provider Training Module 1c: Reading and Writing Data
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
 * mechanism that the RsslChannel.socketId is registered with. The UPA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines RsslBuffer boundaries and returns each buffer one by 
 * one.
 *
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
 ************************************************************************
 * UPA Interactive Provider Training Module 2: Perform/Handle Login Process
 ************************************************************************
 * Summary:
 * Applications authenticate with one another using the Login domain model. 
 * An OMM Interactive Provider must handle the consumer’s Login request messages 
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
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package. 
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 3: Provide Source Directory Information
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
 * about supported domain types, the service’s state, the QoS, and any item group 
 * information associated with the service. Thomson Reuters recommends that at a minimum, 
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
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 4: Provide Necessary Dictionaries
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Necessary Dictionaries.
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information.
 *
 * Detailed Descriptions:
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information. Content that uses the RsslFieldList type requires the 
 * use of a field dictionary (usually the Thomson Reuters RDMFieldDictionary, though it 
 * can instead be a user-defined or modified field dictionary).
 * 
 * The Source Directory message should notify the consumer about dictionaries needed to 
 * decode content sent by the provider. If the consumer needs a dictionary to decode 
 * content, it is ideal that the Interactive Provider application also make this dictionary
 * available to consumers for download. The provider can inform the consumer whether the
 * dictionary is available via the Source Directory.
 * 
 * If loading from a file, UPA offers several utility functions for loading and managing 
 * a properly-formatted field dictionary. There are also utility functions provided to 
 * help the provider encode into an appropriate format for downloading. 
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

#include "UPAProvider_Training.h"

/* dictionary file name  */
const char *fieldDictionaryFileName = "RDMFieldDictionary";
/* dictionary download name */
const char *fieldDictionaryDownloadName = "RWFFld";

/* enum table file name */
const char *enumTypeDictionaryFileName = "enumtype.def";
/* enum table download name */
const char *enumTypeDictionaryDownloadName = "RWFEnum";

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

	/* UPA Bind Options used in the rsslBind call. */
	RsslBindOptions bindOpts = RSSL_INIT_BIND_OPTS;

	/* UPA Accept Options used in the rsslAccept call */
	RsslAcceptOptions acceptOpts = RSSL_INIT_ACCEPT_OPTS;

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
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

	/* In this app, we are only interested in using 2 dictionaries:
	 * - Thomson Reuters Field Dictionary (RDMFieldDictionary) and
	 * - Enumerated Types Dictionaries (enumtype.def)
	 *
	 * We will just use dictionaries that are available locally in a file.
	 */

	/* data dictionary */
	RsslDataDictionary dataDictionary;

	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

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

	/* Set the runtime of the UPA Interactive Provider application to be runTime (seconds) */
	upaRuntime = currentTime + (time_t)runTime;

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

	/*********************************************************
	 * We will just use dictionaries that are available locally in a file. We will exit the interactive provider application
	 * if any dictionary cannot be loaded properly or does not exist in the current runtime path.
	 *
	 * For performance considerations, it is recommended to first load field and enumerated dictionaries from local files,
	 * if they exist, at the earlier stage of the interactive provider applications.
	 *
	 * When loading from local files, UPA offers several utility functions to load and manage a properly-formatted field dictionary
	 * and enum type dictionary.
	 *********************************************************/

	/* clear the RsslDataDictionary dictionary before first use/load
	 * This should be done prior to the first call of a dictionary loading function, if the initializer is not used.
	 */
	rsslClearDataDictionary(&dataDictionary);

	/* load field dictionary from file - adds data from a Field Dictionary file to the RsslDataDictionary */
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &dataDictionary, &errorText) < 0)
	{
		printf("\nUnable to load field dictionary: %s.\n\tError Text: %s\n", fieldDictionaryFileName, errorText.data);
		exit(RSSL_RET_FAILURE);
	}
	else
		printf("Successfully loaded field dictionary from local file.\n\n");

	/* load enumerated dictionary from file - adds data from an Enumerated Types Dictionary file to the RsslDataDictionary */
	if (rsslLoadEnumTypeDictionary(enumTypeDictionaryFileName, &dataDictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s.\n\tError Text: %s\n", enumTypeDictionaryFileName, errorText.data);
		exit(RSSL_RET_FAILURE);
	}
	else
		printf("Successfully loaded enum type dictionary from local file.\n\n");

	printf("UPA provider app has successfully loaded both dictionaries from local files.\n\n");

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
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
					closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
								closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
							closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
		}
	}

	/* Initialize ping management handler */
	initPingManagementHandler(&upaChannelManagementInfo);

	/* Clears the login request information */
	clearLoginReqInfo(&upaChannelManagementInfo.loginRequestInfo);

	/* Clears the source directory request information */
	clearSourceDirectoryReqInfo(&upaChannelManagementInfo.sourceDirectoryRequestInfo);

	/* Clears the dictionary request information */
	clearDictionaryReqInfo(&upaChannelManagementInfo.fieldDictionaryRequestInfo);
	clearDictionaryReqInfo(&upaChannelManagementInfo.enumTypeDictionaryRequestInfo);

	/* set the ServiceName and Service Id */
	snprintf(upaChannelManagementInfo.sourceDirectoryRequestInfo.ServiceName, 128, "%s", serviceName);
	/* service id associated with the service name of provider */
	upaChannelManagementInfo.sourceDirectoryRequestInfo.ServiceId = 1234;

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
		 * for UPA Interactive Provider, timeout numbers for the select call should be set to be configurable UPDATE_INTERVAL.
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
					 * Server/Provider Application Liefcycle Major Step 5:
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
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
						}

						/* decode contents into the RsslMsg structure */
						retval = rsslDecodeMsg(&decodeIter, &msg);
						if (retval != RSSL_RET_SUCCESS)
						{
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%d  Size %d \n", retval, upaChannelManagementInfo.upaChannel->socketId, msgBuf->length);
							/* Closes channel, closes server, cleans up and exits the application. */
							closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
						}

						switch ( msg.msgBase.domainType )
						{
							/*!< (1) Login Message */
							case RSSL_DMT_LOGIN:
							{
								if ((retval = processLoginRequest(&upaChannelManagementInfo, &msg, &decodeIter)) > RSSL_RET_SUCCESS)
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
									/* Clears the login request information */
									clearLoginReqInfo(&upaChannelManagementInfo.loginRequestInfo);

									/* Closes channel, closes server, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
								}
							}
							break;
							/*!< (4) Source Message */
							case RSSL_DMT_SOURCE:
							{
								if ((retval = processSourceDirectoryRequest(&upaChannelManagementInfo, &msg, &decodeIter)) > RSSL_RET_SUCCESS)
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
									/* Clears the source directory request information */
									clearSourceDirectoryReqInfo(&upaChannelManagementInfo.sourceDirectoryRequestInfo);

									/* Closes channel, closes server, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
								}
							}
							break;
							/*!< (5) Dictionary Message */
							case RSSL_DMT_DICTIONARY:
							{
								if ((retval = processDictionaryRequest(&upaChannelManagementInfo, &msg, &decodeIter, &dataDictionary)) > RSSL_RET_SUCCESS)
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
									/* Clears the dictionary request information */
									clearDictionaryReqInfo(&upaChannelManagementInfo.fieldDictionaryRequestInfo);
									clearDictionaryReqInfo(&upaChannelManagementInfo.enumTypeDictionaryRequestInfo);

									/* Closes channel, closes server, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
						upaChannelManagementInfo.pingManagementInfo.receivedClientMsg = RSSL_TRUE;
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
								upaChannelManagementInfo.pingManagementInfo.receivedClientMsg = RSSL_TRUE;
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
								printf("\nrsslRead() FD Change - Old FD: %d New FD: %d\n", upaChannelManagementInfo.upaChannel->oldSocketId, upaChannelManagementInfo.upaChannel->socketId);
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
							{
								printf("\nchannelInactive fd=%d <%s>\n", upaChannelManagementInfo.upaChannel->socketId, error.text);
							}
							default: /* Error handling */
							{
								if (retval_rsslRead < 0)
								{
									printf("Error %s (%d) (errno: %d) encountered with rsslRead fd=%d. Error Text: %s\n",
										rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError,
										upaChannelManagementInfo.upaChannel->socketId, error.text);
									/* Closes channel/connection, cleans up and exits the application. */
									closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
							closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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
			/* Closes channel, closes server, cleans up and exits the application. */
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
		}

		/* get current time */
		time(&currentTime);

		/* Handles the run-time for the UPA Interactive Provider application. Here we exit the application after a predetermined time to run */
		if (currentTime >= upaRuntime)
		{
			/* Closes all streams for the Interactive Provider after run-time has elapsed in our simple Interactive Provider example.
			 * If the provider application must shut down, it can either leave consumer connections intact or shut them down. If the provider
			 * decides to close consumer connections, the provider should send an RsslStatusMsg on each connection’s Login stream closing the stream.
			 * At this point, the consumer should assume that its other open streams are also closed.
			 */

			/* send close status messages to all streams on the connected client channel */

			/* send close status message to dictionary stream */
			if ((retval = sendDictionaryCloseStatusMsg(&upaChannelManagementInfo)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you send close status message to dictionary stream, we want to make a best effort to get this across the network as it will gracefully
				 * close all open dictionary consumer streams. If this cannot be flushed or failed, this application will just close the connection
				 * for simplicity.
				 */

				/* Closes channel, closes server, cleans up and exits the application. */
				closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
			}

			/* send close status message to login stream */
			if ((retval = sendLoginCloseStatusMsg(&upaChannelManagementInfo)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you send close status message to login stream, we want to make a best effort to get this across the network as it will gracefully
				 * close all open consumer streams. If this cannot be flushed or failed, this application will just close the connection
				 * for simplicity.
				 */

				/* Closes channel, closes server, cleans up and exits the application. */
				closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_FAILURE, &dataDictionary);
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

			printf("\nUPA Interactive Provider run-time has expired...\n");
			closeChannelServerCleanUpAndExit(upaChannelManagementInfo.upaChannel, upaSrvr, RSSL_RET_SUCCESS, &dataDictionary);
		}
	}
}

/*
 * Closes channel, closes server, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * upaSrvr - The RsslServer that represents the listening socket connection to the user to be closed
 * code - if exit due to errors/exceptions
 * dataDictionary -  the dictionaries that need to be unloaded to clean up memory
 */
void closeChannelServerCleanUpAndExit(RsslChannel* upaChannel, RsslServer* upaSrvr, int code, RsslDataDictionary* dataDictionary)
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

	/* when users are done, they should unload dictionaries to clean up memory */
	if (dataDictionary == 0)
		printf("\nNULL Dictionary pointer.\n");
	else if (!dataDictionary->isInitialized)
		printf("\nNo need to delete dictionary - dictionary was not loaded yet.\n");
	else if (rsslDeleteDataDictionary(dataDictionary) < 0)
	{
		printf("\nUnable to delete dictionary.\n");
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

/*
 * Initializes the ping times for upaChannelManagementInfo.upaChannel.
 * upaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* get current time */
	time(&upaChannelManagementInfo->pingManagementInfo.currentTime);

	/* set ping timeout for server and client */
	/* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
	 * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
	 * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
	 * network or operating system notification that may occur.
	 */
	upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer = upaChannelManagementInfo->upaChannel->pingTimeout/3;
	upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient = upaChannelManagementInfo->upaChannel->pingTimeout;

	/* set time to send next ping from server */
	upaChannelManagementInfo->pingManagementInfo.nextSendPingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;

	/* set time server should receive next message/ping from client */
	upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;

	upaChannelManagementInfo->pingManagementInfo.receivedClientMsg = RSSL_FALSE;
}

/*
 * Processing ping management handler for upaChannelManagementInfo.upaChannel.
 * upaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* Handles the ping processing for upaChannelManagementInfo.upaChannel. Sends a ping to the client if the next send ping time has arrived and
	 * checks if a ping has been received from the client within the next receive ping time.
	 */
	RsslRet	retval = RSSL_RET_SUCCESS;
	RsslError error;

	/* get current time */
	time(&upaChannelManagementInfo->pingManagementInfo.currentTime);

	/* handle server pings */
	if (upaChannelManagementInfo->pingManagementInfo.currentTime >= upaChannelManagementInfo->pingManagementInfo.nextSendPingTime)
	{
		/* send ping to client */
		/*********************************************************
		 * Server/Provider Application Liefcycle Major Step 5:
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
					/* Ping message has been sent successfully to the client */
					printf("Ping message has been sent successfully to the client ... \n\n");
				}
				break;
				case RSSL_RET_FAILURE: /* fall through to default. */
				default: /* Error handling */
				{
					printf("\nError %s (%d) (errno: %d) encountered with rsslPing() on fd=%d with code %d\n. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannelManagementInfo->upaChannel->socketId, retval,
						error.text);
					/* Closes channel/connection, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
			}
		}

		/* set time to send next ping from server */
		upaChannelManagementInfo->pingManagementInfo.nextSendPingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;
	}

	/* handle client pings - an application should determine if data or pings have been received,
	 * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
	 */
	if (upaChannelManagementInfo->pingManagementInfo.currentTime >= upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime)
	{
		/* check if server received message from client since last time */
		if (upaChannelManagementInfo->pingManagementInfo.receivedClientMsg)
		{
			/* reset flag for client message received */
			upaChannelManagementInfo->pingManagementInfo.receivedClientMsg = RSSL_FALSE;

			/* set time server should receive next message/ping from client */
			upaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = upaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)upaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;
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
 * upaChannelManagementInfo.upaChannel - The channel to send the message buffer to
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
	 * Server/Provider Application Liefcycle Major Step 5:
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
 * upaChannelInfo - The channel management information including the login request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter)
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
				if (sendLoginRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, NO_USER_NAME_IN_REQUEST) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* if stream id is different from last request, this is an invalid request */
			if (upaChannelManagementInfo->loginRequestInfo.IsInUse && (upaChannelManagementInfo->loginRequestInfo.StreamId != msg->msgBase.streamId))
			{
				if (sendLoginRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, MAX_LOGIN_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* decode login request */

			/* get StreamId */
			upaChannelManagementInfo->loginRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

			upaChannelManagementInfo->loginRequestInfo.IsInUse = RSSL_TRUE;

			/* get Username */
			if (requestKey->name.length < 128)
			{
				strncpy(upaChannelManagementInfo->loginRequestInfo.Username, requestKey->name.data, requestKey->name.length);
				upaChannelManagementInfo->loginRequestInfo.Username[requestKey->name.length] = '\0';
			}
			else
			{
				strncpy(upaChannelManagementInfo->loginRequestInfo.Username, requestKey->name.data, 127);
				upaChannelManagementInfo->loginRequestInfo.Username[128 - 1] = '\0';
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
								strncpy(upaChannelManagementInfo->loginRequestInfo.ApplicationId, element.encData.data, element.encData.length);
								upaChannelManagementInfo->loginRequestInfo.ApplicationId[element.encData.length] = '\0';
							}
							else
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.ApplicationId, element.encData.data, 127);
								upaChannelManagementInfo->loginRequestInfo.ApplicationId[128 - 1] = '\0';
							}
						}

						/* ApplicationName */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
						{
							if (element.encData.length < 128)
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.ApplicationName, element.encData.data, element.encData.length);
								upaChannelManagementInfo->loginRequestInfo.ApplicationName[element.encData.length] = '\0';
							}
							else
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.ApplicationName, element.encData.data, 127);
								upaChannelManagementInfo->loginRequestInfo.ApplicationName[128 - 1] = '\0';
							}
						}

						/* Position */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
						{
							if (element.encData.length < 128)
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.Position, element.encData.data, element.encData.length);
								upaChannelManagementInfo->loginRequestInfo.Position[element.encData.length] = '\0';
							}
							else
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.Position, element.encData.data, 127);
								upaChannelManagementInfo->loginRequestInfo.Position[128 - 1] = '\0';
							}
						}

						/* Password */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PASSWORD))
						{
							if (element.encData.length < 128)
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.Password, element.encData.data, element.encData.length);
								upaChannelManagementInfo->loginRequestInfo.Password[element.encData.length] = '\0';
							}
							else
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.Password, element.encData.data, 127);
								upaChannelManagementInfo->loginRequestInfo.Password[128 - 1] = '\0';
							}
						}

						/* InstanceId */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_INST_ID))
						{
							if (element.encData.length < 128)
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.InstanceId, element.encData.data, element.encData.length);
								upaChannelManagementInfo->loginRequestInfo.InstanceId[element.encData.length] = '\0';
							}
							else
							{
								strncpy(upaChannelManagementInfo->loginRequestInfo.InstanceId, element.encData.data, 127);
								upaChannelManagementInfo->loginRequestInfo.InstanceId[128 - 1] = '\0';
							}
						}

						/* Role */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ROLE))
						{
							retval = rsslDecodeUInt(decodeIter, &upaChannelManagementInfo->loginRequestInfo.Role);
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

			printf("\nReceived Login Request for Username: %.*s\n", strlen(upaChannelManagementInfo->loginRequestInfo.Username), upaChannelManagementInfo->loginRequestInfo.Username);

			/* send login response */
			if (retval = sendLoginResponse(upaChannelManagementInfo) != RSSL_RET_SUCCESS)
				return retval;
		}
		break;

		case RSSL_MC_CLOSE:
		{
			printf("\nReceived Login Close for StreamId %d\n", msg->msgBase.streamId);

			/* close login stream */
			closeLoginStream(msg->msgBase.streamId, upaChannelManagementInfo);
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
 * upaChannelInfo - The channel management information including the login request information and
 * including the channel to send a login refresh response to
 */
RsslRet sendLoginResponse(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
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

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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
	refreshMsg.msgBase.streamId = upaChannelManagementInfo->loginRequestInfo.StreamId;

	/* set msgKey members */
	/*!< (0x0020) This RsslMsgKey has additional attribute information, contained in \ref RsslMsgKey::encAttrib. The container type of the attribute information is contained in \ref RsslMsgKey::attribContainerType. */
	/*!< (0x0004) This RsslMsgKey has a nameType enumeration, contained in \ref RsslMsgKey::nameType. */
	/*!< (0x0002) This RsslMsgKey has a name buffer, contained in \ref RsslMsgKey::name.  */
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME;

	/* Username */
	refreshMsg.msgBase.msgKey.name.data = upaChannelManagementInfo->loginRequestInfo.Username;
	refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(upaChannelManagementInfo->loginRequestInfo.Username);
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
		printf("rsslEncodeMsgInit() failed with return code: %d\n", refreshMsg);
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
	applicationName.data = (char*)"UPA Provider Training";
	applicationName.length = (RsslUInt32)strlen("UPA Provider Training");
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
	position.data = (char*)upaChannelManagementInfo->loginRequestInfo.Position;
	position.length = (RsslUInt32)strlen(upaChannelManagementInfo->loginRequestInfo.Position);
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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login response */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * upaChannelInfo - The channel management information including the login request information
 */
void closeLoginStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* find original request information associated with streamId */
	if(upaChannelManagementInfo->loginRequestInfo.StreamId == streamId)
	{
		printf("Closing login stream id %d with user name: %.*s\n", upaChannelManagementInfo->loginRequestInfo.StreamId, strlen(upaChannelManagementInfo->loginRequestInfo.Username), upaChannelManagementInfo->loginRequestInfo.Username);

		/* Clears the original login request information */
		clearLoginReqInfo(&upaChannelManagementInfo->loginRequestInfo);
	}
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
			/* Closes channel, closes server, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/*!< (-4) Transport Failure: There are no buffers available from the buffer pool, returned from rsslGetBuffer.
		 * This can happen if the reader isn't keeping up and/or we have a lot of write threads in multithreaded apps.
		 * Use rsslIoctl to increase pool size or use rsslFlush to flush data and return buffers to pool.
		 */

		/* The rsslFlush function could be used to attempt to free buffers back to the pool */
		retval = rsslFlush(upaChannel, &error);
		if (retval < RSSL_RET_SUCCESS)
		{
			printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			/* Closes channel, closes server, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/* call rsslGetBuffer again to see if it works now after rsslFlush */
		if ((msgBuf = rsslGetBuffer(upaChannel, size, RSSL_FALSE, &error)) == NULL)
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
 * upaChannelInfo - The channel management information including the login request information and
 * including the channel to send the login request reject status message to
 * streamId - The stream id of the login request reject status
 * reason - The reason for the reject
 */
RsslRet sendLoginRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaLoginRejectReason reason)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the login request reject status message. */
	RsslStatusMsg msg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request Reject Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login request reject status */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Sends the login close status message for a channel.
 * upaChannelInfo - The channel management information including the login request information and
 * including the channel to send the login close status message to
 */
RsslRet sendLoginCloseStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the login close status and to close the stream. */
	RsslStatusMsg msg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login Close Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the login close status. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearStatusMsg(&msg);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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
	msg.msgBase.streamId = upaChannelManagementInfo->loginRequestInfo.StreamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN; /*!< (1) Login Message */
	/* No payload associated with this close status message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	msg.flags = RSSL_STMF_HAS_STATE;
	/*!< (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
	msg.state.streamState = RSSL_STREAM_CLOSED;
	/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	sprintf(stateText, "Login stream closed");
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send login close status */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* login close status fails */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the login close status doesn't flush, just close channel and exit the app. When you send login close status msg,
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
 * upaChannelInfo - The channel management information including the source directory request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processSourceDirectoryRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter)
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
				if (sendSrcDirectoryRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, INCORRECT_FILTER_FLAGS) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* if stream id is different from last request, this is an invalid request */
			if (upaChannelManagementInfo->sourceDirectoryRequestInfo.IsInUse && (upaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId != msg->msgBase.streamId))
			{
				if (sendSrcDirectoryRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, MAX_SRCDIR_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}

			/* decode source directory request */

			/* get StreamId */
			upaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

			upaChannelManagementInfo->sourceDirectoryRequestInfo.IsInUse = RSSL_TRUE;

			printf("\nReceived Source Directory Request\n");

			/* send source directory response */
			if (retval = sendSourceDirectoryResponse(upaChannelManagementInfo, upaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName, upaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceId) != RSSL_RET_SUCCESS)
				return retval;
		}
		break;

		case RSSL_MC_CLOSE:
		{
			printf("\nReceived Source Directory Close for StreamId %d\n", msg->msgBase.streamId);

			/* close source directory stream */
			closeSourceDirectoryStream(msg->msgBase.streamId, upaChannelManagementInfo);
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
 * upaChannelInfo - The channel management information including the source directory request information
 * serviceName - The service name specified by the OMM interactive provider application (Optional to set)
 * serviceId - the serviceId specified by the OMM interactive provider  application (Optional to set)
 */
RsslRet sendSourceDirectoryResponse(UpaChannelManagementInfo *upaChannelManagementInfo, char serviceName[128], RsslUInt64 serviceId)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
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

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Source Directory response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	/* At a minimum, Thomson Reuters recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
	refreshKey.filter =	RDM_DIRECTORY_SERVICE_INFO_FILTER | \
						RDM_DIRECTORY_SERVICE_STATE_FILTER| \
						/* RDM_DIRECTORY_SERVICE_GROUP_FILTER | \ not applicable for refresh message - here for reference */
						RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
						/* RDM_DIRECTORY_SERVICE_DATA_FILTER | \ not applicable for non-ANSI Page based provider - here for reference */
						RDM_DIRECTORY_SERVICE_LINK_FILTER;

	/* StreamId */
	streamId = upaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId;

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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
		 * Prior to changing a service status, Thomson Reuters recommends that you issue item or group
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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send source directory response */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * upaChannelInfo - The channel management information including the source directory request information and
 * including the channel to send the source directory request reject status message to
 * streamId - The stream id of the source directory request reject status
 * reason - The reason for the reject
 */
RsslRet sendSrcDirectoryRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaSourceDirectoryRejectReason reason)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the source directory request reject status message. */
	RsslStatusMsg msg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Source Directory request Reject Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send source directory request reject status */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * upaChannelInfo - The channel management information including the source directory request information
 */
void closeSourceDirectoryStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* find original request information associated with streamId */
	if(upaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId == streamId)
	{
		printf("Closing source directory stream id %d with service name: %.*s\n", upaChannelManagementInfo->sourceDirectoryRequestInfo.StreamId, strlen(upaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName), upaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceName);

		/* Clears the original source directory request information */
		clearSourceDirectoryReqInfo(&upaChannelManagementInfo->sourceDirectoryRequestInfo);
	}
}

/*
 * Processes a dictionary request. This consists of decoding the dictionary request and calling the corresponding flavors
 * of the sendDictionaryResponse() functions to send the dictionary response.
 * upaChannelInfo - The channel management information including the dictionary request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 * dataDictionary - The dictionary to encode field information or enumerated type information from
 */
RsslRet processDictionaryRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary)
{
	RsslMsgKey* requestKey = 0;

	RsslRet retval = 0;

	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			/* get request message key - retrieve the RsslMsgKey structure from the provided decoded message structure */
			requestKey = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* decode dictionary request */

			/* first check if this is fieldDictionary or enumTypeDictionary request */
			if (!strncmp(fieldDictionaryDownloadName, requestKey->name.data, strlen(fieldDictionaryDownloadName)))
			{
				upaChannelManagementInfo->fieldDictionaryRequestInfo.IsInUse = RSSL_TRUE;

				/* get StreamId */
				upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

				if (rsslCopyMsgKey(&upaChannelManagementInfo->fieldDictionaryRequestInfo.MsgKey, requestKey) == RSSL_RET_FAILURE)
				{
					if (sendDictionaryRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, MAX_DICTIONARY_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					break;
				}
				upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName[upaChannelManagementInfo->fieldDictionaryRequestInfo.MsgKey.name.length] = '\0';

				printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", strlen(upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName), upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName);

				/* send field dictionary response */
				if ((retval = sendDictionaryResponse(upaChannelManagementInfo, dataDictionary, DICTIONARY_FIELD_DICTIONARY)) != RSSL_RET_SUCCESS)
				{
					return retval;
				}
			}
			else if (!strncmp(enumTypeDictionaryDownloadName, requestKey->name.data, strlen(enumTypeDictionaryDownloadName)))
			{
				upaChannelManagementInfo->enumTypeDictionaryRequestInfo.IsInUse = RSSL_TRUE;

				/* get StreamId */
				upaChannelManagementInfo->enumTypeDictionaryRequestInfo.StreamId = msg->requestMsg.msgBase.streamId;

				if (rsslCopyMsgKey(&upaChannelManagementInfo->enumTypeDictionaryRequestInfo.MsgKey, requestKey) == RSSL_RET_FAILURE)
				{
					if (sendDictionaryRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, MAX_DICTIONARY_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					break;
				}
				upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName[upaChannelManagementInfo->enumTypeDictionaryRequestInfo.MsgKey.name.length] = '\0';

				printf("\nReceived Dictionary Request for DictionaryName: %.*s\n", strlen(upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName), upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName);

				/* send enum type dictionary response */
				if ((retval = sendDictionaryResponse(upaChannelManagementInfo, dataDictionary, DICTIONARY_ENUM_TYPE)) != RSSL_RET_SUCCESS)
				{
					return retval;
				}
			}
			else
			{
				if (sendDictionaryRequestRejectStatusMsg(upaChannelManagementInfo, msg->msgBase.streamId, UNKNOWN_DICTIONARY_NAME) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}
		}
		break;

		case RSSL_MC_CLOSE:
		{
			printf("\nReceived Dictionary Close for StreamId %d\n", msg->msgBase.streamId);

			/* close dictionary stream */
			closeDictionaryStream(msg->msgBase.streamId, upaChannelManagementInfo);
		}
		break;

		default:
		{
			printf("\nReceived Unhandled Dictionary Msg Class: %d\n", msg->msgBase.msgClass);
    		return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the field dictionary or enumType dictionary response to a channel. This consists of getting a message buffer,
 * encoding the field dictionary or enumType dictionary response, and sending the field dictionary or enumType dictionary response to the server.
 * upaChannelInfo - The channel management information including the dictionary request information and
 * including the channel to send the field dictionary or enumType dictionary response to
 * dataDictionary - The dictionary to encode field information or enumerated type information from
 */
RsslRet sendDictionaryResponse(UpaChannelManagementInfo *upaChannelManagementInfo, RsslDataDictionary* dataDictionary, UpaDictionaryType dictionaryType)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	RsslInt32 dictionaryFid = RSSL_MIN_FID;

	/* flag indicating first part of potential multi part refresh message */
	RsslBool firstPartMultiPartRefresh = RSSL_TRUE;

	char stateText[128];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslBool dictionaryComplete = RSSL_FALSE;

	/* Provider uses RsslRefreshMsg to send the field dictionary or enum type dictionary refresh response to a channel. */
	RsslRefreshMsg refreshMsg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	switch(dictionaryType)
	{
		case DICTIONARY_FIELD_DICTIONARY:
			/* set starting fid to loaded field dictionary's minimum fid */
			dictionaryFid = dataDictionary->minFid;
			break;
		case DICTIONARY_ENUM_TYPE:
			/* set starting fid to loaded enumType dictionary's minimum fid, which is 0 in this case */
			/* for rsslEncodeEnumTypeDictionaryAsMultiPart() API all, must be initialized to 0 on the first call and is updated with each successfully encoded part. */
			dictionaryFid = 0;
			break;
		default:
			break;
	}

	/* field Dictionary takes multiple parts to respond */
	while (RSSL_TRUE)
	{
		/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Dictionary response.
		 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
		 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
		 * This ensures that only the required bytes are written to the network.
		 */
		/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
				{
					/* Connection should be closed, return failure */
					/* Closes channel, closes server, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
				break;
			case DICTIONARY_ENUM_TYPE:
				/* EnumType Dictionary now supports fragmenting at a message level - However, some EnumType Dictionary message can be still very large, up to 10K */
				if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, &error)) == NULL) /* first check Error */
				{
					/* Connection should be closed, return failure */
					/* Closes channel, closes server, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
				break;
			default:
				break;
		}

		/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

		/* Encodes the field dictionary or enumType dictionary refresh response. */

		/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
		 * the clear function when initializing any messages.
		 */
		rsslClearRefreshMsg(&refreshMsg);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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
		refreshMsg.msgBase.domainType = RSSL_DMT_DICTIONARY; /*!< (5) Dictionary Message */

		/* StreamId */
		refreshMsg.msgBase.streamId = upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId;

		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				refreshMsg.msgBase.streamId = upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId;
				break;
			case DICTIONARY_ENUM_TYPE:
				refreshMsg.msgBase.streamId = upaChannelManagementInfo->enumTypeDictionaryRequestInfo.StreamId;
				break;
			default:
				break;
		}

		/*!< (138) Series container type, represents row based tabular information where no specific indexing is required.   <BR>*/
		refreshMsg.msgBase.containerType = RSSL_DT_SERIES;

		/*!< (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg) */
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;

		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				refreshMsg.msgBase.msgKey.filter = upaChannelManagementInfo->fieldDictionaryRequestInfo.MsgKey.filter;
				break;
			case DICTIONARY_ENUM_TYPE:
				refreshMsg.msgBase.msgKey.filter = upaChannelManagementInfo->enumTypeDictionaryRequestInfo.MsgKey.filter;
				break;
			default:
				break;
		}

		/*!< (0x0002) This RsslMsgKey has a name buffer, contained in \ref RsslMsgKey::name.  */
		/*!< (0x0008) This RsslMsgKey has a filter, contained in \ref RsslMsgKey::filter.  */
		/*!< (0x0001) This RsslMsgKey has a service id, contained in \ref RsslMsgKey::serviceId.  */
		refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_SERVICE_ID;
		refreshMsg.msgBase.msgKey.serviceId = (RsslUInt16)upaChannelManagementInfo->sourceDirectoryRequestInfo.ServiceId;

		/*!< (0x0008) The RsslRefreshMsg has a message key, contained in \ref RsslRefreshMsg::msgBase::msgKey. */
		/*!< (0x0020) Indicates that this RsslRefreshMsg is a solicited response to a consumer's request. */
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

		/* when doing a multi part refresh only the first part has the RSSL_RFMF_CLEAR_CACHE flag set */
		if (firstPartMultiPartRefresh)
		{
			/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
			refreshMsg.flags |= RSSL_RFMF_CLEAR_CACHE;
		}

		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				sprintf(stateText, "Field Dictionary Refresh (starting fid %d)", dictionaryFid);
				break;
			case DICTIONARY_ENUM_TYPE:
				sprintf(stateText, "Enum Type Dictionary Refresh (starting fid %d)", dictionaryFid);
				break;
			default:
				break;
		}
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);

		/* DictionaryName */
		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				refreshMsg.msgBase.msgKey.name.data = upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName;
				refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName);
				break;
			case DICTIONARY_ENUM_TYPE:
				refreshMsg.msgBase.msgKey.name.data = upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName;
				refreshMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName);
				break;
			default:
				break;
		}

		/* encode message */

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
			return RSSL_RET_FAILURE;
		}

		/* encode dictionary into message */

		switch(dictionaryType)
		{
			case DICTIONARY_FIELD_DICTIONARY:
				/**
				 * @brief Encode the field definitions dictionary information into a data payload according the domain model, using the field information from the entries present in this dictionary.
				 * This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary,
				 *  subsequent calls can be made to this function, each producing the next segment of fields.
				 * @param eIter Iterator to be used for encoding. Prior to each call, the iterator must be cleared and initialized to the buffer to be used for encoding.
				 * @param dictionary The dictionary to encode field information from.
				 * @param currentFid Tracks which fields have been encoded in case of multi-part encoding. Must be initialized to dictionary->minFid on the first call and is updated with each successfully encoded part.
				 * @param verbosity The desired verbosity to encode. See RsslDictionaryVerbosity.
				 * @param errorText Buffer to hold error text if the encoding fails.
				 * @return RSSL_RET_DICT_PART_ENCODED when encoding parts, RSSL_RET_SUCCESS for final part or single complete payload.
				 * @see RsslDataDictionary, RsslDictionaryVerbosity, rsslDecodeFieldDictionary
				 */
				if ((retval = rsslEncodeFieldDictionary(&encodeIter, dataDictionary, &dictionaryFid, (RDMDictionaryVerbosityValues)upaChannelManagementInfo->fieldDictionaryRequestInfo.MsgKey.filter, &errorText)) != RSSL_RET_SUCCESS)
				{
					/* dictionary encode failed */
					if (retval != RSSL_RET_DICT_PART_ENCODED) /*!< (10) Dictionary Success: Successfully encoded part of a dictionary message, returned from the rssl dictionary processing functions. */
					{
						rsslReleaseBuffer(msgBuf, &error);
						printf("rsslEncodeFieldDictionary() failed '%s'\n",errorText.data );
						return retval;
					}
				}
				else /* dictionary encode complete */
				{
					dictionaryComplete = RSSL_TRUE;

					/* set refresh complete flag */
					/* Set the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer */
					rsslSetRefreshCompleteFlag(&encodeIter);
				}
				break;
			case DICTIONARY_ENUM_TYPE:
				/**
				 * @brief Encode the enumerated types dictionary according the domain model, using the information from the tables and referencing fields present in this dictionary.
				 * This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary,
				 *  subsequent calls can be made to this function, each producing the next segment of fields.
				 * Note: This function will use the type RSSL_DT_ASCII_STRING for the DISPLAY array.
				 * @param eIter Iterator to be used for encoding.
				 * @param dictionary The dictionary to encode enumerated type information from.
				 * @param currentFid Tracks which fields have been encoded in case of multi-part encoding. Must be initialized to 0 on the first call and is updated with each successfully encoded part.
				 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
				 * @param errorText Buffer to hold error text if the encoding fails.
				 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionary, rsslDecodeEnumTypeDictionary
				 */
				if ((retval = rsslEncodeEnumTypeDictionaryAsMultiPart(&encodeIter, dataDictionary, &dictionaryFid, (RDMDictionaryVerbosityValues)upaChannelManagementInfo->enumTypeDictionaryRequestInfo.MsgKey.filter, &errorText)) != RSSL_RET_SUCCESS)
				{
					/* dictionary encode failed */
					if (retval != RSSL_RET_DICT_PART_ENCODED) /*!< (10) Dictionary Success: Successfully encoded part of a dictionary message, returned from the rssl dictionary processing functions. */
					{
						rsslReleaseBuffer(msgBuf, &error);
						printf("rsslEncodeEnumTypeDictionaryAsMultiPart() failed '%s'\n",errorText.data );
						return retval;
					}
				}
				else /* dictionary encode complete */
				{
					dictionaryComplete = RSSL_TRUE;

					/* set refresh complete flag */
					/* Set the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer */
					rsslSetRefreshCompleteFlag(&encodeIter);
				}
				break;
			default:
				break;
		}

		/* complete encode message */
		if ((retval = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeMsgComplete() failed with return code: %d\n", retval);
			return retval;
		}

		/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
		/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
		 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
		 * in the buffer. This ensures that only the required bytes are written to the network.
		 */
		msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

		firstPartMultiPartRefresh = RSSL_FALSE;

		/* send dictionary refresh response */
		if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
		{
			/* dictionary refresh response fails */
			/* Closes channel, closes server, cleans up and exits the application. */
			return RSSL_RET_FAILURE;
		}
		else if (retval > RSSL_RET_SUCCESS)
		{
			/* There is still data left to flush */

			/* If the dictionary close status doesn't flush, just close channel and exit the app. When you send dictionary close status msg,
			 * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
			 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
			 */

			/* Closes channel, closes server, cleans up and exits the application. */
		}

		/* break out of loop when all dictionary responses sent */
		if (dictionaryComplete)
		{
			break;
		}

		/* sleep between dataDictionary responses */
#if defined(_WIN32)
		Sleep(1);
#else
		struct timespec sleeptime;
		sleeptime.tv_sec = 0;
		sleeptime.tv_nsec = 1000000;
		nanosleep(&sleeptime, 0);
#endif
	}

	return retval;
}

/*
 * Sends the dictionary close status message for a channel.
 * upaChannelInfo - The channel management information including the dictionary request information and
 * including the channel to send the dictionary close status message to
 */
RsslRet sendDictionaryCloseStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the dictionary close status and to close the stream. */
	RsslStatusMsg msg;

	int i;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	for (i = 0; i < 2; i++)
	{
		/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Dictionary Close Status Msg.
		 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
		 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
		 * This ensures that only the required bytes are written to the network.
		 */
		/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
		if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
		{
			/* Connection should be closed, return failure */
			/* Closes channel, closes server, cleans up and exits the application. */
			return RSSL_RET_FAILURE;
		}

		/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

		/* Encodes the dictionary close status. */

		/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
		 * the clear function when initializing any messages.
		 */
		rsslClearStatusMsg(&msg);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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

		if (i == 0)
			msg.msgBase.streamId = upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId;
		else
			msg.msgBase.streamId = upaChannelManagementInfo->enumTypeDictionaryRequestInfo.StreamId;

		msg.msgBase.domainType = RSSL_DMT_DICTIONARY; /*!< (5) Dictionary Message */
		/* No payload associated with this close status message */
		msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

		/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
		msg.flags = RSSL_STMF_HAS_STATE;
		/*!< (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
		msg.state.streamState = RSSL_STREAM_CLOSED;
		/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
		msg.state.dataState = RSSL_DATA_SUSPECT;
		msg.state.code = RSSL_SC_NONE;
		sprintf(stateText, "Dictionary stream closed");
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText);

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

		/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
		/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
		 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
		 * in the buffer. This ensures that only the required bytes are written to the network.
		 */
		msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

		/* send dictionary close status */
		if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
		{
			/* dictionary close status fails */
			/* Closes channel, closes server, cleans up and exits the application. */
			return RSSL_RET_FAILURE;
		}
		else if (retval > RSSL_RET_SUCCESS)
		{
			/* There is still data left to flush */

			/* If the dictionary close status doesn't flush, just close channel and exit the app. When you send dictionary close status msg,
			 * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
			 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
			 */

			/* Closes channel, closes server, cleans up and exits the application. */
		}
	}

	return retval;
}

/*
 * Sends the dictionary request reject status message for a channel.
 * upaChannelInfo - The channel management information including the dictionary request information and
 * including the channel to send the dictionary request reject status message to
 * streamId - The stream id of the dictionary request reject status
 * reason - The reason for the reject
 */
RsslRet sendDictionaryRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaDictionaryRejectReason reason)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the dictionary request reject status message. */
	RsslStatusMsg msg;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	char stateText[256];

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Dictionary request Reject Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannelManagementInfo->upaChannel, upaChannelManagementInfo->upaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the dictionary request reject status. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearStatusMsg(&msg);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, upaChannelManagementInfo->upaChannel->majorVersion, upaChannelManagementInfo->upaChannel->minorVersion);

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
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY; /*!< (5) Dictionary Message */
	/* No payload associated with this close status message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	msg.flags = RSSL_STMF_HAS_STATE;
	/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;

	switch(reason)
	{
		case UNKNOWN_DICTIONARY_NAME:
			/*!< (1) Not found (indicates that requested information was not found, it may become available at a later time or by changing some of the requested parameters) */
			msg.state.code = RSSL_SC_NOT_FOUND;
			/*!< (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
			msg.state.streamState = RSSL_STREAM_CLOSED;
			sprintf(stateText, "Dictionary request rejected for stream id %d - dictionary name unknown", streamId);
			msg.state.text.data = stateText;
			msg.state.text.length = (RsslUInt32)strlen(stateText);
			break;
		case MAX_DICTIONARY_REQUESTS_REACHED:
			/*!< (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
			msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
			/*!< (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
			msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
			sprintf(stateText, "Dictionary request rejected for stream id %d - max request count reached", streamId);
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

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send dictionary request reject status */
	if ((retval = sendMessage(upaChannelManagementInfo->upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* send dictionary request reject status fails */
		/* Closes channel, closes server, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the dictionary request reject status doesn't flush, just close channel and exit the app. When you send dictionary request reject status msg,
		 * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
		 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
		 */

		/* Closes channel, closes server, cleans up and exits the application. */
	}

	return retval;
}

/*
 * Closes a dictionary stream.
 * streamId - The stream id to close the dictionary for
 * upaChannelInfo - The channel management information including the dictionary request information
 */
void closeDictionaryStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo)
{
	/* find the original request information associated with streamId */
	if (upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId == streamId)
	{
		printf("Closing dictionary stream id %d with dictionary name: %.*s\n", upaChannelManagementInfo->fieldDictionaryRequestInfo.StreamId, strlen(upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName), upaChannelManagementInfo->fieldDictionaryRequestInfo.DictionaryName);

		/* Clears the original field dictionary request information */
		clearDictionaryReqInfo(&upaChannelManagementInfo->fieldDictionaryRequestInfo);
	}
	else if (upaChannelManagementInfo->enumTypeDictionaryRequestInfo.StreamId == streamId)
	{
		printf("Closing dictionary stream id %d with dictionary name: %.*s\n", upaChannelManagementInfo->enumTypeDictionaryRequestInfo.StreamId, strlen(upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName), upaChannelManagementInfo->enumTypeDictionaryRequestInfo.DictionaryName);

		/* Clears the original enumType dictionary request information */
		clearDictionaryReqInfo(&upaChannelManagementInfo->enumTypeDictionaryRequestInfo);
	}
}

