/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright Thomson Reuters 2015. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */


/*
 * This is the UPA NI Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM NI Provider using the UPA Transport layer.
 *
 * Main c source file for the UPA NI Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA NI Provider Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * A Non-Interactive Provider (NIP) writes a provider application that 
 * connects to TREP-RT and sends a specific set (non-interactive) of 
 * information (services, domains, and capabilities). NIPs act like 
 * clients in a client-server relationship. Multiple NIPs can connect 
 * to the same TREP-RT and publish the same items and content. 
 * 
 * In this module, the OMM NIP application initializes the UPA Transport 
 * and establish a connection to an ADH server. Once connected, an OMM NIP 
 * can publish information into the ADH cache without needing to handle 
 * requests for the information. The ADH can cache the information and 
 * along with other Enterprise Platform components, provide the information 
 * to any OMM consumer applications that indicate interest.
 *
 * Detailed Descriptions:
 * The first step of any UPA NIP application is to establish network 
 * communication with an ADH server. To do so, the OMM NIP typically creates 
 * an outbound connection to the well-known hostname and port of an ADH. 
 * The OMM NIP uses the rsslConnect function to initiate the connection 
 * process and then performs connection initialization processes as needed.
 * 
 *
 ************************************************************************
 * UPA NI Provider Training Module 1b: Ping (heartbeat) Management
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
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information 
 * is being exchanged. For example, there may be long periods of time that 
 * elapse between requests made from an OMM NIP application to ADH Infrastructure.
 * In this situation, the NIP would send periodic heartbeat messages to inform 
 * the ADH Infrastructure that it is still alive.
 *
 *
 ************************************************************************
 * UPA NI Provider Training Module 1c: Reading and Writing Data
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
 * UPA NI Provider Training Module 2: Log in
 ************************************************************************
 * Summary:
 * In this module, applications authenticate with one another using the Login 
 * domain model. An OMM NIP must register with the system using a Login request 
 * prior to providing any content. Because this is done in an interactive manner, 
 * the NIP should assign a streamId with a positive value which the ADH will 
 * reference when sending its response.
 *
 * Detailed Descriptions:
 * After receiving a Login request, the ADH determines whether the NIP is 
 * permissioned to access the system. The ADH sends a Login response, indicating 
 * to the NIP whether the ADH grants it access. 
 * 
 * a) If the application is denied, the ADH closes the Login stream and the 
 * NI provider application cannot perform any additional communication. 
 * b) If the application gains access to the ADH, the Login response informs 
 * the application of this. The NI provider must now provide a Source Directory.
 * 
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package. 
 * 
 *
 ************************************************************************
 * UPA NI Provider Training Module 3: Provide Source Directory Information
 ************************************************************************
 * Summary:
 * In this module, OMM NIP application provides Source Directory information.
 * The Source Directory domain model conveys information about all available 
 * services in the system. After completing the Login process, an OMM NIP must 
 * provide a Source Directory refresh.
 * 
 * Detailed Descriptions:
 * The Source Directory domain model conveys information about all 
 * available services in the system. After completing the Login process, 
 * an OMM NIP must provide a Source Directory refresh indicating:
 * 
 * a) Service, service state, QoS, and capability information associated 
 * with the NIP
 * b) Supported domain types and any item group information associated 
 * with the service.
 * 
 * At a minimum, Thomson Reuters recommends that the NIP send the Info, 
 * State, and Group filters for the Source Directory. Because this is provider 
 * instantiated, the NIP should use a streamId with a negative value.
 * 
 * a) The Source Directory Info filter contains service name and serviceId 
 * information for all available services, though NIPs typically provide data 
 * on only one service.
 * b) The Source Directory State filter contains status information for service. 
 * This informs the ADH whether the service is Up and available or Down and 
 * unavailable.
 * c) The Source Directory Group filter conveys item group status information, 
 * including information about group states as well as the merging of groups. 
 * For additional information about item groups, refer to UPAC Developer Guide.
 * 
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 *
 ************************************************************************
 * UPA NI Provider Training Module 4: Load Dictionary Information
 ************************************************************************
 * Summary:
 * Dictionaries may be available locally in a file for an OMM NIP appliation. In 
 * this Training example, the OMM NIP will use dictionaries that are available 
 * locally in a file.
 * 
 * Detailed Descriptions:
 * Some data requires the use of a dictionary for encoding or decoding. This 
 * dictionary typically defines type and formatting information and directs 
 * the application as to how to encode or decode specific pieces of information. 
 * Content that uses the RsslFieldList type requires the use of a field dictionary 
 * (usually the Thomson Reuters RDMFieldDictionary, though it could also be a 
 * user-defined or modified field dictionary).
 * 
 * Dictionaries may be available locally in a file for an OMM NIP appliation. In 
 * this Training example, the OMM NIP will use dictionaries that are available 
 * locally in a file.
 *
 *
 ************************************************************************
 * UPA NI Provider Training Module 5: Provide Content
 ************************************************************************
 * Summary:
 * In this module, after providing a Source Directory, the OMM NIP application can 
 * begin pushing content to the ADH. In this simple example, we just show functions 
 * for sending 1 MP(MarketPrice) domain type Item refresh, update, and
 * close status message(s) to an ADH. 
 * 
 * Detailed Descriptions:
 * After providing a Source Directory, the NIP application can begin pushing content 
 * to the ADH. Each unique information stream should begin with an RsslRefreshMsg, 
 * conveying all necessary identification information for the content. Because the 
 * provider instantiates this information, a negative value streamId should be used 
 * for all streams. The initial identifying refresh can be followed by other status
 * or update messages. Some ADH functionality, such as cache rebuilding, may require 
 * that NIP applications publish the message key on all RsslRefreshMsgs. See the 
 * component specific documentation for more information.
 * 
 * Some components may require that NIP applications publish the msgKey in RsslUpdateMsgs. 
 * To avoid component or transport migration issues, NIP applications may want to always 
 * include this information. 
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

#include "UPANIProvider_Training.h"

/* dictionary file name  */
const char *fieldDictionaryFileName = "RDMFieldDictionary";

/* enum table file name */
const char *enumTypeDictionaryFileName = "enumtype.def";

int main(int argc, char **argv)
{
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */

	/* This example suite uses write descriptor in our client/NI Provider type examples in mainly 2 areas with
	 * the I/O notification mechanism being used:
	 * 1) rsslInitChannel() function which exchanges various messages to perform necessary UPA transport
	 *    negotiations and handshakes to complete channel initialization.
	 * 2) rsslFlush() calls used throughout the application (after module 1a), together with rsslWrite() calls, such
	 *    as in sendMessage() function. The write descriptor can be used to indicate when the socketId has write
	 *    availability and help with determining when the network is able to accept additional bytes for writing.
	 *
	 * For the RsslChannel initialization process, if using I/O, a client/NI Provider application should register the
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
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	RsslChannel *upaChannel;

	char srvrHostname[128], srvrPortNo[128], interfaceName[128], serviceName[128], itemName[128];
	RsslUInt64 serviceId = 1;

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

	/* RSSL Channel Info returned by rsslGetChannelInfo call */
	RsslChannelInfo channelInfo;

	RsslUInt32 maxMsgSize; /* the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool. */
	RsslBuffer* msgBuf = 0;

	/* ping management information */
	UpaPingManagementInfo pingManagementInfo;

	RsslBool isLoginSuccessful = RSSL_FALSE;

	RsslBuffer tempBuffer;

	UpaMarketPriceItem marketPriceItem;
	UpaMarketPriceItemInfo marketPriceItemInfo;

	time_t currentTime = 0;
	time_t upaRuntime = 0;
	RsslUInt32 runTime = 0;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

	/* In this app, we are only interested in using 2 dictionaries:
	 * - Thomson Reuters Field Dictionary (RDMFieldDictionary) and
	 * - Enumerated Types Dictionaries (enumtype.def)
	 *
	 * Dictionaries may be available locally in a file for an OMM NIP appliation. In
	 * this Training example, the OMM NIP will use dictionaries that are available
	 * locally in a file.

	 * Some data requires the use of a dictionary for encoding or decoding. This
	 * dictionary typically defines type and formatting information and directs
	 * the application as to how to encode or decode specific pieces of information.
	 * Content that uses the RsslFieldList type requires the use of a field dictionary
	 * (usually the Thomson Reuters RDMFieldDictionary, though it could also be a
	 * user-defined or modified field dictionary).
	 */

	/* data dictionary */
	RsslDataDictionary dataDictionary;

	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	/* the default option parameters */
	/* connect to server running on same machine */
	snprintf(srvrHostname, 128, "%s", "localhost");
	/* server is running on port number 14003 */
	snprintf(srvrPortNo, 128, "%s", "14003");
	/* use default NIC network interface card to bind to for all inbound and outbound data */
	snprintf(interfaceName, 128, "%s", "");
	/* use default runTime of 300 seconds */
	runTime = 300;
	/* default service name is "DIRECT_FEED" used in source directory handler */
	snprintf(serviceName, 128, "%s", "DIRECT_FEED");
	/* default item name is "TRI" used in market price item response handler */
	snprintf(itemName, 128, "%s", "TRI");

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
			else if (strcmp("-s", argv[i]) == 0)
			{
				i += 2;
				snprintf(serviceName, 128, "%s", argv[i-1]);
			}
			else if(strcmp("-id", argv[i]) == 0)
			{
				i += 2;
				serviceId = atol(argv[i-1]);
			}
			else if (strcmp("-mp", argv[i]) == 0)
			{
				i += 2;
				snprintf(itemName, 128, "%s", argv[i-1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-id <ServiceId>] [-mp <ItemName>] \n", argv[0], argv[0]);
				exit(RSSL_RET_FAILURE);
			}
		}
	}

	/* Initializes market price item fields. */
	marketPriceItem.RDNDISPLAY = 100;
	marketPriceItem.RDN_EXCHID = 155;
	tempBuffer.data = (char *)"05/17/2013";
	tempBuffer.length = (RsslUInt32)strlen("05/17/2013");
	rsslDateStringToDate(&marketPriceItem.DIVPAYDATE, &tempBuffer);
	marketPriceItem.TRDPRC_1 = 1.00;
	marketPriceItem.BID = 0.99;
	marketPriceItem.ASK = 1.03;
	marketPriceItem.ACVOL_1 = 100000;
	marketPriceItem.NETCHNG_1 = 2.15;
	rsslDateTimeLocalTime(&marketPriceItem.ASK_TIME);

	/* Initialize item handler - initialize/reset market price item information */
	snprintf(marketPriceItemInfo.itemName, 128, "%s", itemName);
	marketPriceItemInfo.streamId = MARKETPRICE_ITEM_STREAM_ID_START;
	marketPriceItemInfo.isRefreshComplete = RSSL_FALSE;
	marketPriceItemInfo.itemData = (void*)&marketPriceItem;

	/******************************************************************************************************************
				INITIALIZATION - USING rsslInitialize()
	******************************************************************************************************************/
	/*********************************************************
	 * Client/NIProv Application Liefcycle Major Step 1:
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

	/* Set the runtime of the UPA NI Provider application to be runTime (seconds) */
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

	/*********************************************************
	 * For performance considerations, it is recommended to first load field and enumerated dictionaries from local files,
	 * if they exist, at the earlier stage of the consumer applications.
	 *
	 * When loading from local files, UPA offers several utility functions to load and manage a properly-formatted field dictionary
	 * and enum type dictionary.
	 *
	 * Only make Market Price item request after both dictionaries are successfully loaded from files.
	 * If at least one of the dictionaries fails to get loaded properly, it will continue the code path of downloading the
	 * failed loaded dictionary or both dictionaries (if neither exists in run-time path or loaded properly) from provider
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

	printf("UPA NIP application has successfully loaded both dictionaries from local files.\n\n");

	/******************************************************************************************************************
				CONNECTION SETUP - USING rsslConnect()
	******************************************************************************************************************/
	/*********************************************************
	 * Client/NIProv Application Liefcycle Major Step 2:
	 * Connect using rsslConnect (OS connection establishment handshake)
	 * rsslConnect call Establishes an outbound connection, which can leverage standard sockets, HTTP,
	 * or HTTPS. Returns an RsslChannel that represents the connection to the user. In the event of an error,
	 * NULL is returned and additional information can be found in the RsslError structure.
	 * Connection options are passed in via an RsslConnectOptions structure.
	 *********************************************************/

	if ((upaChannel = rsslConnect(&cOpts, &error)) == 0)
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslConnect. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

		/* End application, uninitialize to clean up first */
		rsslUninitialize();
		exit(RSSL_RET_FAILURE);
	}

	/* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
	/* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
	FD_SET(upaChannel->socketId, &cleanReadFds);
	FD_SET(upaChannel->socketId, &cleanExceptFds);

	/* for non-blocking I/O (default), write descriptor is set initially in case this end starts the message
	 * handshakes that rsslInitChannel() performs. Once rsslInitChannel() is called for the first time the
	 * channel can wait on the read descriptor for more messages. Without using the write descriptor, we would
	 * have to keep looping and calling rsslInitChannel to complete the channel initialization process, which
	 * can be CPU-intensive. We will set the write descriptor again if a FD_CHANGE event occurs.
	 */
	if (!cOpts.blocking)
	{
		if (!FD_ISSET(upaChannel->socketId, &cleanWriteFds))
			FD_SET(upaChannel->socketId, &cleanWriteFds);
	}

	printf("\nChannel IPC descriptor = "SOCKET_PRINT_TYPE"\n", upaChannel->socketId);

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

	while (upaChannel->state != RSSL_CH_STATE_ACTIVE)
	{
		useReadFds = cleanReadFds;
		useWriteFds = cleanWriteFds;
		useExceptFds = cleanExceptFds;

		/* Set a timeout value if the ADH Infra server accepts the connection, but does not initialize it */
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
			closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
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
			switch (upaChannel->state)
			{
				/* Indicates that an RsslChannel requires additional initialization. This initialization is typically additional
				 * connection handshake messages that need to be exchanged.
				 */
				case RSSL_CH_STATE_INITIALIZING:
				{
					/* rsslInitChannel is called if read or write or except is triggered */
					if (FD_ISSET(upaChannel->socketId, &useReadFds) || FD_ISSET(upaChannel->socketId, &useWriteFds) || FD_ISSET(upaChannel->socketId, &useExceptFds))
					{
						/* Write descriptor is set initially in case this end starts the message handshakes that rsslInitChannel() performs.
						 * Once rsslInitChannel() is called for the first time the channel can wait on the read descriptor for more messages.
						 * We will set the write descriptor again if a FD_CHANGE event occurs. */
						FD_CLR(upaChannel->socketId, &cleanWriteFds);

						/*********************************************************
						 * Client/NIProv Application Liefcycle Major Step 3:
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
						if ((retval = rsslInitChannel(upaChannel, &inProgInfo, &error)) < RSSL_RET_SUCCESS)
						{
							printf("Error %s (%d) (errno: %d) encountered with rsslInitChannel fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
								rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannel->socketId, error.text);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
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
										printf("\nChannel In Progress - New FD: "SOCKET_PRINT_TYPE"  Old FD: "SOCKET_PRINT_TYPE"\n",upaChannel->socketId, inProgInfo.oldSocket );

										/* File descriptor has changed, unregister old and register new */
										FD_CLR(inProgInfo.oldSocket, &cleanReadFds);
										FD_CLR(inProgInfo.oldSocket, &cleanWriteFds);
										FD_CLR(inProgInfo.oldSocket, &cleanExceptFds);
										/* newSocket should equal upaChannel->socketId */
										FD_SET(upaChannel->socketId, &cleanReadFds);
										FD_SET(upaChannel->socketId, &cleanWriteFds);
										FD_SET(upaChannel->socketId, &cleanExceptFds);
									}
									else
									{
										printf("\nChannel "SOCKET_PRINT_TYPE" In Progress...\n", upaChannel->socketId);
									}
								}
								break;

								/* channel connection becomes active!
								 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state,
								 * this RsslChannel can be used for other transport operations.
								 */
								case RSSL_RET_SUCCESS:
								{
									printf("\nChannel on fd "SOCKET_PRINT_TYPE" is now active - reading and writing can begin.\n", upaChannel->socketId);

									/*********************************************************
									 * Connection is now active. The RsslChannel can be used for all additional
									 * transport functionality (e.g. reading, writing) now that the state
									 * transitions to RSSL_CH_STATE_ACTIVE
									 *********************************************************/

									/* After channel is active, use UPA Transport utility function rsslGetChannelInfo to query RsslChannel negotiated
									 * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
									 * compression information as well as many other values.
									 */
									if ((retval = rsslGetChannelInfo(upaChannel, &channelInfo, &error)) != RSSL_RET_SUCCESS)
									{
										printf("Error %s (%d) (errno: %d) encountered with rsslGetChannelInfo. Error Text: %s\n",
											rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);

										/* Connection should be closed, return failure */
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
									}

									printf( "Channel "SOCKET_PRINT_TYPE" active. Channel Info:\n"
										"	Max Fragment Size: %u\n"
										"	Output Buffers: %u Max, %u Guaranteed\n"
										"	Input Buffers: %u\n"
										"	Send/Recv Buffer Sizes: %u/%u\n"
										"	Ping Timeout: %u\n"
										"	Connected component version: ",
										upaChannel->socketId,			/*!< @brief Socket ID of this RSSL channel. */
										channelInfo.maxFragmentSize,	/*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */
										channelInfo.maxOutputBuffers,	/*!< @brief This is the maximum number of output buffers available to the channel. */
										channelInfo.guaranteedOutputBuffers, /*!< @brief This is the guaranteed number of output buffers available to the channel. */
										channelInfo.numInputBuffers,	/*!< @brief This is the number of input buffers available to the channel. */
										channelInfo.sysSendBufSize,		/*!< @brief This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
										channelInfo.sysRecvBufSize,		/*!< @brief This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
										channelInfo.pingTimeout 		/*!< @brief This is the value of the negotiated ping timeout */
									);

									if (channelInfo.componentInfoCount == 0)
										printf("(No component info)");
									else
									{
										RsslUInt32 count;
										for(count = 0; count < channelInfo.componentInfoCount; ++count)
										{
											printf("%.*s",
													channelInfo.componentInfo[count]->componentVersion.length,
													channelInfo.componentInfo[count]->componentVersion.data);
											if (count < channelInfo.componentInfoCount - 1)
												printf(", ");
										}
									}
									printf ("\n\n");
								}
								break;
								default: /* Error handling */
								{
									printf("\nBad return value fd="SOCKET_PRINT_TYPE" <%s>\n",
										upaChannel->socketId, error.text);
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
								}
								break;
							}
						}
					}
				}
				break;

				/* Indicates that an RsslChannel is active. This channel can perform any connection related actions, such as reading or writing. */
				case RSSL_CH_STATE_ACTIVE:
				{
					/*********************************************************
					 * Connection is now active. The RsslChannel can be used for all additional
					 * transport functionality (e.g. reading, writing) now that the state
					 * transitions to RSSL_CH_STATE_ACTIVE
					 *********************************************************/
				}
				break;

				/* RSSL_CH_STATE_CLOSED, RSSL_CH_STATE_INACTIVE, and default should be handled same way: just call closeChannelCleanUpAndExit function and break. */

				/* Indicates that an RsslChannel has been closed. This typically occurs as a result of an error inside of a transport function call
				 * and is often related to a socket being closed or becoming unavailable. Appropriate error value return codes and RsslError
				 * information should be available for the user.
				 */
				case RSSL_CH_STATE_CLOSED: /* fall through to default. */

				/* Indicates that an RsslChannel is inactive. This channel cannot be used. This state typically occurs after a channel
				 * is closed by the user.
				 */
				case RSSL_CH_STATE_INACTIVE: /* fall through to default. */

				default: /* Error handling */
				{
					/* Closes channel, cleans up and exits the application. */
					closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
				}
				break;
			}
		}
		else if (selRet < 0)
		{
			/* On error, -1 is returned, and errno is set appropriately; the sets and timeout become undefined */
			printf("\nSelect error.\n");
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
		}
	}

	/* maxMsgSize is the requested size of rsslGetBuffer function. In this application, we set maxMsgSize to
	 * be equal to maxFragmentSize from the channel info. maxFragmentSize from the channel info is the maximum
	 * packable size, that is, the max fragment size before fragmentation and reassembly is necessary.
	 * If the requested size is larger than the maxFragmentSize, the transport will create and return the buffer
	 * to the user. When written, this buffer will be fragmented by the rsslWrite function.
	 * Because of some additional book keeping required when packing, the application must specify whether
	 * a buffer should be 'packable' when calling rsslGetBuffer.
	 * For performance purposes, an application is not permitted to request a buffer larger than maxFragmentSize
	 * and have the buffer be 'packable.'
	 */
	maxMsgSize = channelInfo.maxFragmentSize; /*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */

	/* Initialize ping management handler */
	initPingManagementHandler(upaChannel, &pingManagementInfo);

	/* clear encode iterator for initialization/use - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* Send Login request message */
	if ((retval = sendLoginRequest(upaChannel, maxMsgSize, &encodeIter)) > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush, leave our write notification enabled so we get called again.
		 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
		 */

		/* set write fd if there's still other data queued */
		/* flush is done by application */
		FD_SET(upaChannel->socketId, &cleanWriteFds);
	}
	else if (retval < RSSL_RET_SUCCESS)
	{
		/* Closes channel, cleans up and exits the application. */
		closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
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
		 * for UPA NIProvider, timeout numbers for the select call should be set to be configurable UPDATE_INTERVAL.
		 * We set the Update Rate Interval to be 1 second for NIP application, which is the Update Interval
		 * the NIP application pushes the Update Mssage content to ADH
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
			/* the NIP application can begin pushing content to the ADH. */
			if (isLoginSuccessful)
			{
				/*
				 * Updates the item that's currently in use to simulate the Market Price movement.
				 */
				marketPriceItem.TRDPRC_1 += 0.01;
				marketPriceItem.BID += 0.01;
				marketPriceItem.ASK += 0.01;
				rsslDateTimeLocalTime(&marketPriceItem.ASK_TIME);

				sendMarketPriceItemResponse(upaChannel, maxMsgSize, &encodeIter, &marketPriceItemInfo, (RsslUInt16)serviceId, &dataDictionary);
			}
		}
		else if (selRet > 0)
		{
			/* Received messages and reading from the channel/connection */
			/* On success, select() return the number of file descriptors contained in the three returned descriptor sets
			 * (that is, the total number of bits that are set in readfds, writefds, exceptfds)
			 */

			/* different behaviors are triggered by different file descriptors */
			if (FD_ISSET(upaChannel->socketId, &useReadFds) || FD_ISSET(upaChannel->socketId, &useExceptFds))
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
					 * Client/NIProv Application Liefcycle Major Step 4:
					 * Read using rsslRead
					 * rsslRead provides the user with data received from the connection. This function expects the RsslChannel to be in the active state.
					 * When data is available, an RsslBuffer referring to the information is returned, which is valid until the next call to rsslRead.
					 * A return code parameter passed into the function is used to convey error information as well as communicate whether there is additional
					 * information to read. An I/O notification mechanism may not inform the user of this additional information as it has already been read
					 * from the socket and is contained in the rsslRead input buffer.
					 *********************************************************/

					if ((msgBuf = rsslRead(upaChannel, &retval_rsslRead, &error)) != 0)
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
						rsslSetDecodeIteratorRWFVersion(&decodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

						/* Associates the RsslDecodeIterator with the RsslBuffer from which to decode. */
						if((retval = rsslSetDecodeIteratorBuffer(&decodeIter, msgBuf)) != RSSL_RET_SUCCESS)
						{
							printf("\nrsslSetDecodeIteratorBuffer() failed with return code: %d\n", retval);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
						}

						/* decode contents into the RsslMsg structure */
						retval = rsslDecodeMsg(&decodeIter, &msg);
						if (retval != RSSL_RET_SUCCESS)
						{
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", retval, upaChannel->socketId, msgBuf->length);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
						}

						switch ( msg.msgBase.domainType )
						{
							/*!< (1) Login Message */
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
									closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
								}
								else
								{
									isLoginSuccessful = RSSL_TRUE;
									printf("UPA NI Provider application is granted access and has logged in successfully.\n\n");

									/* The Source Directory domain model conveys information about all available services in the system. After completing the
									 * Login process, an OMM NIP must provide a Source Directory refresh message.
									 */
									printf("UPA NI Provider application is providing a Source Directory refresh message.\n\n");
									sendSourceDirectoryResponse(upaChannel, maxMsgSize, &encodeIter, serviceName, serviceId);
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
						pingManagementInfo.receivedServerMsg = RSSL_TRUE;
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
								pingManagementInfo.receivedServerMsg = RSSL_TRUE;
							}
							break;
							/*!< (-14) Transport Success: rsslRead received an FD change event. The application should unregister the oldSocketId and
							 * register the socketId with its notifier
							 */
							case RSSL_RET_READ_FD_CHANGE:
							{
								/* File descriptor changed, typically due to tunneling keep-alive */
								/* Unregister old socketId and register new socketId */
								printf("\nrsslRead() FD Change - Old FD: "SOCKET_PRINT_TYPE" New FD: "SOCKET_PRINT_TYPE"\n", upaChannel->oldSocketId, upaChannel->socketId);
								FD_CLR(upaChannel->oldSocketId, &cleanReadFds);
								FD_CLR(upaChannel->oldSocketId, &cleanWriteFds);
								FD_CLR(upaChannel->oldSocketId, &cleanExceptFds);
								/* Up to application whether to register with write set - depends on need for write notification. Here we need it for flushing. */
								FD_SET(upaChannel->socketId, &cleanReadFds);
								FD_SET(upaChannel->socketId, &cleanWriteFds);
								FD_SET(upaChannel->socketId, &cleanExceptFds);
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
										upaChannel->socketId, error.text);
									/* Closes channel/connection, cleans up and exits the application. */
									closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
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
			if (FD_ISSET(upaChannel->socketId, &useWriteFds))
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
				if ((retval = rsslFlush(upaChannel, &error)) > RSSL_RET_SUCCESS)
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
							FD_CLR(upaChannel->socketId, &cleanWriteFds);
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
							closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
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
			closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
		}

		/* Processing ping management handler */
		if ((retval = processPingManagementHandler(upaChannel, &pingManagementInfo)) > RSSL_RET_SUCCESS)
		{
			/* There is still data left to flush, leave our write notification enabled so we get called again.
			 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
			 */

			/* set write fd if there's still other data queued */
			/* flush is done by application */
			FD_SET(upaChannel->socketId, &cleanWriteFds);
		}
		else if (retval < RSSL_RET_SUCCESS)
		{
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
		}

		/* get current time */
		time(&currentTime);

		/* Handles the run-time for the UPA NI Provider application. Here we exit the application after a predetermined time to run */
		if (currentTime >= upaRuntime)
		{
			/* Closes all streams for the NI Provider after run-time has elapsed. */

			/* send close status messages to all item streams */
			sendItemCloseStatusMsg(upaChannel, maxMsgSize, &encodeIter, &marketPriceItemInfo);

			/* After publishing content to the system, the NIP application should close all open streams and shut down the network connection.
			 * Issuing an RsslCloseMsg for the Login's streamId will close all other streams opened by the NIP application.
			 */

			/* Close Login stream */
			/* Note that closing Login stream will automatically close all other streams at the provider */
			if ((retval = closeLoginStream(upaChannel, maxMsgSize, &encodeIter)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you close login, we want to make a best effort to get this across the network as it will gracefully
				 * close all open streams. If this cannot be flushed or failed, this application will just close the connection
				 * for simplicity.
				 */

				/* Closes channel, cleans up and exits the application. */
				closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
			}

			/* flush before exiting */
			if (FD_ISSET(upaChannel->socketId, &cleanWriteFds))
			{
				retval = 1;
				while (retval > RSSL_RET_SUCCESS)
				{
					retval = rsslFlush(upaChannel, &error);
				}
				if (retval < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
			}

			printf("\nUPA NI Provider run-time has expired...\n");
			closeChannelCleanUpAndExit(upaChannel, RSSL_RET_SUCCESS);
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
	 * Client/NIProv Application Liefcycle Major Step 5:
	 * Close connection using rsslCloseChannel (OS connection release handshake)
	 * rsslCloseChannel closes the client based RsslChannel. This will release any pool based resources
	 * back to their respective pools, close the connection, and perform any additional necessary cleanup.
	 * When shutting down the RSSL Transport, the application should release all unwritten pool buffers.
	 * Calling rsslCloseChannel terminates the connection to the ADH.
	 *********************************************************/

	if ((retval = rsslCloseChannel(upaChannel, &error)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) (errno: %d) encountered with rsslCloseChannel. Error Text: %s\n",
			rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, error.text);
	}

	/*********************************************************
	 * Client/NIProv Application Liefcycle Major Step 6:
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
		printf("\nUPA NI Provider Training application successfully ended.\n");

	/* End application */
	exit(code);
}

/*
 * Initializes the ping times for upaChannel.
 * upaChannel - The channel for ping management info initialization
 * pingManagementInfo - The ping management information that is used
 */
void initPingManagementHandler(RsslChannel* upaChannel, UpaPingManagementInfo* pingManagementInfo)
{
	/* get current time */
	time(&pingManagementInfo->currentTime);

	/* set ping timeout for client and server */
	/* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
	 * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
	 * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
	 * network or operating system notification that may occur.
	 */
	pingManagementInfo->pingTimeoutClient = upaChannel->pingTimeout/3;
	pingManagementInfo->pingTimeoutServer = upaChannel->pingTimeout;

	/* set time to send next ping from client */
	pingManagementInfo->nextSendPingTime = pingManagementInfo->currentTime + (time_t)pingManagementInfo->pingTimeoutClient;

	/* set time client should receive next message/ping from server */
	pingManagementInfo->nextReceivePingTime = pingManagementInfo->currentTime + (time_t)pingManagementInfo->pingTimeoutServer;

	pingManagementInfo->receivedServerMsg = RSSL_FALSE;
}

/*
 * Processing ping management handler
 * upaChannel - The channel for ping management processing
 * pingManagementInfo - The ping management information that is used
 */
RsslRet processPingManagementHandler(RsslChannel* upaChannel, UpaPingManagementInfo* pingManagementInfo)
{
	/* Handles the ping processing for upaChannel. Sends a ping to the server if the next send ping time has arrived and
	 * checks if a ping has been received from the server within the next receive ping time.
	 */
	RsslRet	retval = RSSL_RET_SUCCESS;
	RsslError error;

	/* get current time */
	time(&pingManagementInfo->currentTime);

	/* handle client pings */
	if (pingManagementInfo->currentTime >= pingManagementInfo->nextSendPingTime)
	{
		/* send ping to server */
		/*********************************************************
		 * Client/NIProv Application Liefcycle Major Step 4:
		 * Ping using rsslPing
		 * Attempts to write a heartbeat message on the connection. This function expects the RsslChannel to be in the active state.
		 * If an application calls the rsslPing function while there are other bytes queued for output, the UPA Transport layer will
		 * suppress the heartbeat message and attempt to flush bytes to the network on the user's behalf.
		 *********************************************************/

		/* rsslPing use - this demonstrates sending of heartbeats */
		if ((retval = rsslPing(upaChannel, &error)) > RSSL_RET_SUCCESS)
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
					/* Ping message has been sent successfully */
					printf("Ping message has been sent successfully ... \n\n");
				}
				break;
				case RSSL_RET_FAILURE: /* fall through to default. */
				default: /* Error handling */
				{
					printf("\nError %s (%d) (errno: %d) encountered with rsslPing() on fd="SOCKET_PRINT_TYPE" with code %d\n. Error Text: %s\n",
						rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError, upaChannel->socketId, retval,
						error.text);
					/* Closes channel/connection, cleans up and exits the application. */
					return RSSL_RET_FAILURE;
				}
			}
		}

		/* set time to send next ping from client */
		pingManagementInfo->nextSendPingTime = pingManagementInfo->currentTime + (time_t)pingManagementInfo->pingTimeoutClient;
	}

	/* handle server pings - an application should determine if data or pings have been received,
	 * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
	 */
	if (pingManagementInfo->currentTime >= pingManagementInfo->nextReceivePingTime)
	{
		/* check if client received message from server since last time */
		if (pingManagementInfo->receivedServerMsg)
		{
			/* reset flag for server message received */
			pingManagementInfo->receivedServerMsg = RSSL_FALSE;

			/* set time client should receive next message/ping from server */
			pingManagementInfo->nextReceivePingTime = pingManagementInfo->currentTime + (time_t)pingManagementInfo->pingTimeoutServer;
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
	 * Client/NIProv Application Liefcycle Major Step 4:
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
 * Send Login request message to a channel. This consists of getting a message buffer, setting the login request
 * information, encoding the login request, and sending the login request to the server. A Login request message is
 * encoded and sent by OMM consumer and OMM non-interactive provider applications. This message registers a user
 * with the system. After receiving a successful Login response, applications can then begin consuming or providing
 * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
 * upaChannel - The channel to send the Login request message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 */
RsslRet sendLoginRequest(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	RsslElementList	elementList;
	RsslElementEntry elementEntry;

	RsslBuffer applicationId, applicationName;
	RsslUInt64 applicationRole;

	char userName[256];
	RsslBuffer userNameBuf;

	/* Prefer use of clear functions for initializations over using static initializers. Clears tend to be more performant than
	 * using static initializers. Although if you do choose to use static initializers instead, you don't need to clear.
	 */
	rsslClearElementList(&elementList);
	rsslClearElementEntry(&elementEntry);

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannel, maxMsgSize, &error)) == NULL) /* first check Error */
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
	rsslClearEncodeIterator(encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(encodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
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
	if ((retval = rsslEncodeMsgInit(encodeIter, (RsslMsg*)&reqMsg, 0)) < RSSL_RET_SUCCESS)
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
	if ((retval = rsslEncodeElementListInit(encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
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
	if ((retval = rsslEncodeElementEntry(encodeIter, &elementEntry, &applicationId)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element ApplicationId with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* ApplicationName */
	applicationName.data = (char*)"UPA NI Provider Training";
	applicationName.length = (RsslUInt32)strlen("UPA NI Provider Training");
	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPNAME;
	if ((retval = rsslEncodeElementEntry(encodeIter, &elementEntry, &applicationName)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element ApplicationName with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* Role */
	elementEntry.dataType = RSSL_DT_UINT;
	elementEntry.name = RSSL_ENAME_ROLE;
	/*!< (1) Application logs in as a provider */
	applicationRole = RDM_LOGIN_ROLE_PROV;
	if ((retval = rsslEncodeElementEntry(encodeIter, &elementEntry, &applicationRole)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeElementEntry() failed for Login Request Element Role with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode element list */
	if ((retval = rsslEncodeElementListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
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
	if ((retval = rsslEncodeMsgKeyAttribComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgKeyAttribComplete() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed for Login Request with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer's encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(encodeIter);

	/* send login request */
	if ((retval = sendMessage(upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
				printf("\nA refresh sent to inform a NIP application of an upstream change in information (i.e., an unsolicited refresh).\n");

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

						/* Currently, ADH/Infra handling of login response was simpler than ADS handling -
						 * The Only Received Login Response from ADH is ApplicationId in the current implementation of ADH.
						 * In some cases, a lot of things don't apply (like SingleOpen, Support*, etc) as these are
						 * consumer based behaviors so ADH does not advertise them.
						 * Also, likely many defaults are being relied on from ADH, while ADS may be sending them even though default.
						 */

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
 * A Login close message is encoded and sent by OMM NIP applications. This message allows a NIP to log out
 * of the system. Closing a Login stream is equivalent to a 'Close All' type of message, where all open streams are
 * closed (thus all other streams associated with the user are closed).
 * upaChannel - The channel to send the Login close message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 */
RsslRet closeLoginStream(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* NI Provider uses RsslCloseMsg to indicate no further interest in an item stream and to close the stream. */
	RsslCloseMsg msg;

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login close.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannel, maxMsgSize, &error)) == NULL) /* first check Error */
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
	rsslClearEncodeIterator(encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(encodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
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
	if ((retval = rsslEncodeMsg(encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		return RSSL_RET_FAILURE;
	}

	/* set the buffer's encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(encodeIter);

	/* send login close */
	if ((retval = sendMessage(upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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

/*
 * Send Source Directory response to a channel. This consists of getting a message buffer, setting the source directory
 * response information, encoding the source directory response, and sending the source directory response to
 * the ADH server. OMM NIP application provides Source Directory information. The Source Directory domain model conveys
 * information about all available services in the system. After completing the Login process, an OMM NIP must
 * provide a Source Directory refresh.
 * upaChannel - The channel to send the Source Directory response message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 * serviceName - The service name specified by the OMM NIP application (Optional to set)
 * serviceId - the serviceId specified by the OMM NIP application (Optional to set)
 */
RsslRet sendSourceDirectoryResponse(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter, char serviceName[128], RsslUInt64 serviceId)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg;

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

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Source Directory response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannel, maxMsgSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Source Directory refresh. */

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
	/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
	/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
	refreshFlags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;

	/* set filter flags */
	/* At a minimum, Thomson Reuters recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
	refreshKey.filter =	RDM_DIRECTORY_SERVICE_INFO_FILTER | \
						RDM_DIRECTORY_SERVICE_STATE_FILTER| \
						/* RDM_DIRECTORY_SERVICE_GROUP_FILTER | \ not applicable for refresh message - here for reference */
						RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
						/* RDM_DIRECTORY_SERVICE_DATA_FILTER | \ not applicable for non-ANSI Page based provider - here for reference */
						RDM_DIRECTORY_SERVICE_LINK_FILTER;

	/* StreamId */
	streamId = SRCDIR_STREAM_ID;

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(encodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
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
	if ((retval = rsslEncodeMsgInit(encodeIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
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
	if ((retval = rsslEncodeMapInit(encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapInit() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* encode map entry */
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	if ((retval = rsslEncodeMapEntryInit(encodeIter, &mapEntry, &serviceId, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapEntry() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
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
	if ((retval = rsslEncodeFilterListInit(encodeIter, &sourceDirectoryFilterList)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFilterListInit() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
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
		if ((retval = rsslEncodeFilterEntryInit(encodeIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* encode the element list */
		/*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		if ((retval = rsslEncodeElementListInit(encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* ServiceName */
		tempBuffer.data = serviceName;
		tempBuffer.length = (RsslUInt32)strlen(serviceName);
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_NAME;
		if ((retval = rsslEncodeElementEntry(encodeIter, &element, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* Capabilities */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_CAPABILITIES;
		if ((retval = rsslEncodeElementEntryInit(encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/*  Set RsslArray.itemLength to 1, as each domainType uses only one byte. */
		rsslArray.itemLength = 1;
		rsslArray.primitiveType = RSSL_DT_UINT;
		if ((retval = rsslEncodeArrayInit(encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
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
			if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &capabilities[i])) < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
				/* Closes channel, cleans up and exits the application. */
				return retval;
			}
		}
		if ((retval = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* DictionariesProvided */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_DICTIONARYS_PROVIDED;
		if ((retval = rsslEncodeElementEntryInit(encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
		if ((retval = rsslEncodeArrayInit(encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFFld";
		tempBuffer.length = (RsslUInt32)strlen("RWFFld");
		if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFEnum";
		tempBuffer.length = (RsslUInt32)strlen("RWFEnum");
		if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* DictionariesUsed */
		element.dataType = RSSL_DT_ARRAY;
		element.name = RSSL_ENAME_DICTIONARYS_USED;
		if ((retval = rsslEncodeElementEntryInit(encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		rsslClearArray(&rsslArray);
		/* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
		if ((retval = rsslEncodeArrayInit(encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFFld";
		tempBuffer.length = (RsslUInt32)strlen("RWFFld");
		if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		tempBuffer.data = (char*)"RWFEnum";
		tempBuffer.length = (RsslUInt32)strlen("RWFEnum");
		if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &tempBuffer)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeElementEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
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
		if ((retval = rsslEncodeElementEntryInit(encodeIter, &element, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
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
		if ((retval = rsslEncodeArrayInit(encodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayEntry(encodeIter, 0, &QoS[0])) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
		if ((retval = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeArrayComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		if ((retval = rsslEncodeElementEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* complete encode element list */
		if ((retval = rsslEncodeElementListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* complete encode filter list item */
		if ((retval = rsslEncodeFilterEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
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
		if ((retval = rsslEncodeFilterEntryInit(encodeIter, &filterListItem, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* encode the element list */
		/*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		if ((retval = rsslEncodeElementListInit(encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListInit() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* ServiceState */
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_SVC_STATE;
		/* Indicates whether the original provider of the data is available to respond to new requests. */
		serviceState = 1; /* Service is Up */
		if ((retval = rsslEncodeElementEntry(encodeIter, &element, &serviceState)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* AcceptingRequests */
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_ACCEPTING_REQS;
		/* Indicates whether the immediate provider can accept new requests and/or handle reissue requests on already open streams. */
		acceptingRequests = 1; /* 1: Yes */
		if ((retval = rsslEncodeElementEntry(encodeIter, &element, &acceptingRequests)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
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
		if ((retval = rsslEncodeElementEntry(encodeIter, &element, &status)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* complete encode element list */
		if ((retval = rsslEncodeElementListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeElementListComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}

		/* complete encode filter list item */
		if ((retval = rsslEncodeFilterEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFilterEntryComplete() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}

	/* complete encode filter list */
	if ((retval = rsslEncodeFilterListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFilterListComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode map entry */
	if ((retval = rsslEncodeMapEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode map */
	if ((retval = rsslEncodeMapComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMapComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer's encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(encodeIter);

	/* send source directory response */
	if ((retval = sendMessage(upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Send just 1 Market Price item response message to a channel. This consists of getting a message buffer, encoding the
 * Market Price item response, and sending the item response to the server. Each unique information stream should begin with
 * an RsslRefreshMsg, conveying all necessary identification information for the content. Because the provider instantiates
 * this information, a negative value streamId should be used for all streams. The initial identifying refresh can be followed
 * by other status or update messages.
 * upaChannel - The channel to send the item response message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 * marketPriceItemInfo - The market price item information
 * serviceId - The service id of the market price response
 * dataDictionary - The dictionary used for encoding
 */
RsslRet sendMarketPriceItemResponse(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter, UpaMarketPriceItemInfo* marketPriceItemInfo, RsslUInt16 serviceId,  RsslDataDictionary* dataDictionary)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a refreshMsg and a updateMsg */
	RsslRefreshMsg refreshMsg;
	RsslUpdateMsg updateMsg;

	RsslMsgBase* msgBase;
	RsslMsg* msg;

	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
	char stateText[128];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	UpaMarketPriceItem* mpItem;

	/* In this simple example, we are sending just 1 Market Price item response message to a channel. */

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Market Price item response.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannel, maxMsgSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Market Price item response msg. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRefreshMsg(&refreshMsg);
	rsslClearUpdateMsg(&updateMsg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(encodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	mpItem = (UpaMarketPriceItem*)marketPriceItemInfo->itemData;

	/* set-up message */
	/* set message depending on whether refresh or update */

	/* After providing a Source Directory, the NIP application can begin pushing content to the ADH. Each unique information stream should begin with an RsslRefreshMsg,
	 * conveying all necessary identification information for the content. Because the provider instantiates this information, a negative value streamId should be used for all streams.
	 * The initial identifying refresh can be followed by other status or update messages. Some ADH functionality, such as cache rebuilding, may require that NIP applications publish
	 * the message key on all RsslRefreshMsgs. See the component specific documentation for more information. */

	if (!marketPriceItemInfo->isRefreshComplete) /* this is a refresh message */
	{
		msgBase = &refreshMsg.msgBase;
		msgBase->msgClass = RSSL_MC_REFRESH;

		/* streaming */
		/*!< (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg) */
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;

		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;

		/* for non-interactive providers, this RsslRefreshMsg is not a solicited response to a consumer's request. */

		/*!< (0x0008) The RsslRefreshMsg has a message key, contained in \ref RsslRefreshMsg::msgBase::msgKey. */
		/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
		/*!< (0x0080) The RsslRefreshMsg has quality of service information, contained in RsslRefreshMsg::qos. */
		/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
		refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;

		sprintf(stateText, "Item Refresh Completed");
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;

		/* ServiceId */
		msgBase->msgKey.serviceId = serviceId;

		/* Item Name */
		msgBase->msgKey.name.data = marketPriceItemInfo->itemName;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(marketPriceItemInfo->itemName);
		/*!< (1) Reuters Instrument Code */
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;

		/* Qos */
		refreshMsg.qos.dynamic = RSSL_FALSE;
		/*!< Rate is Tick By Tick, indicates every change to information is conveyed */
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		/*!< Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		msg = (RsslMsg *)&refreshMsg;

		printf("UPA NI Provider application is providing a MarketPrice Item refresh message.\n\n");
	}
	else /* this is an update message */
	{
		msgBase = &updateMsg.msgBase;
		msgBase->msgClass = RSSL_MC_UPDATE; /*!< (4) Update Message */

		/* include msg key in updates for non-interactive provider streams */
		/* because the provider instantiates this information, a negative value streamId should be used for all streams. */
		updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
		/* ServiceId */
		msgBase->msgKey.serviceId = serviceId;
		/* Itemname */
		msgBase->msgKey.name.data = marketPriceItemInfo->itemName;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(marketPriceItemInfo->itemName);
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;

		msg = (RsslMsg *)&updateMsg;

		printf("UPA NI Provider application is providing a MarketPrice Item update message.\n\n");
	}

	msgBase->domainType = RSSL_DMT_MARKET_PRICE; /*!< (6) Market Price Message */
	msgBase->containerType = RSSL_DT_FIELD_LIST; /*!< (132) Field List container type, used to represent content using fieldID - value pair data.  <BR>*/

	/* StreamId */
	msgBase->streamId = MARKETPRICE_ITEM_STREAM_ID_START; /* negative value streamId should be used for all streams. */

	/* encode message - populate message and encode it */
	if ((retval = rsslEncodeMsgInit(encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* encode field list */
	/*!< (0x08) The RsslFieldList contains standard encoded content (e.g. not set defined).  */
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	/**
	 * @brief 	Begin encoding process for RsslFieldList container type.
	 *
	 * Begins encoding of an RsslFieldList<BR>
	 * Typical use:<BR>
	 *  1. Call rsslEncodeFieldListInit()<BR>
	 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
	 *  3. Call rsslEncodeFieldListComplete() when all entries are completed<BR>
	 */
	if ((retval = rsslEncodeFieldListInit(encodeIter, &fieldList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* encode fields */
	/* if refresh, encode refresh fields */
	if (!marketPriceItemInfo->isRefreshComplete)
	{
		/* RDNDISPLAY */
		rsslClearFieldEntry(&fieldEntry);
		dictionaryEntry = dataDictionary->entriesArray[RDNDISPLAY_FID];
		if (dictionaryEntry)
		{
			fieldEntry.fieldId = RDNDISPLAY_FID;
			fieldEntry.dataType = dictionaryEntry->rwfType;
			if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&mpItem->RDNDISPLAY)) < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
				/* Closes channel, cleans up and exits the application. */
				return retval;
			}
		}
		/* RDN_EXCHID */
		rsslClearFieldEntry(&fieldEntry);
		dictionaryEntry = dataDictionary->entriesArray[RDN_EXCHID_FID];
		if (dictionaryEntry)
		{
			fieldEntry.fieldId = RDN_EXCHID_FID;
			fieldEntry.dataType = dictionaryEntry->rwfType;
			if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&mpItem->RDN_EXCHID)) < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
				/* Closes channel, cleans up and exits the application. */
				return retval;
			}
		}
		/* DIVPAYDATE */
		rsslClearFieldEntry(&fieldEntry);
		dictionaryEntry = dataDictionary->entriesArray[DIVPAYDATE_FID];
		if (dictionaryEntry)
		{
			fieldEntry.fieldId = DIVPAYDATE_FID;
			fieldEntry.dataType = dictionaryEntry->rwfType;
			if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&mpItem->DIVPAYDATE)) < RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
				/* Closes channel, cleans up and exits the application. */
				return retval;
			}
		}
	}
	/* TRDPRC_1 */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[TRDPRC_1_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = TRDPRC_1_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->TRDPRC_1, RSSL_RH_EXPONENT_2);
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}
	/* BID */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[BID_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = BID_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->BID, RSSL_RH_EXPONENT_2);
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}
	/* ASK */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[ASK_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = ASK_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->ASK, RSSL_RH_EXPONENT_2);
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}
	/* ACVOL_1 */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[ACVOL_1_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = ACVOL_1_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->ACVOL_1, RSSL_RH_EXPONENT_2);
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}
	/* NETCHNG_1 */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[NETCHNG_1_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = NETCHNG_1_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &mpItem->NETCHNG_1, RSSL_RH_EXPONENT_2);
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}
	/* ASK_TIME */
	rsslClearFieldEntry(&fieldEntry);
	dictionaryEntry = dataDictionary->entriesArray[ASK_TIME_FID];
	if (dictionaryEntry)
	{
		fieldEntry.fieldId = ASK_TIME_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;
		if ((retval = rsslEncodeFieldEntry(encodeIter, &fieldEntry, (void*)&mpItem->ASK_TIME.time)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", retval);
			/* Closes channel, cleans up and exits the application. */
			return retval;
		}
	}

	/* complete encode field list */
	if ((retval = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* complete encode message */
	if ((retval = rsslEncodeMsgComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer's encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(encodeIter);

	/* set refresh complete flag if this is a refresh */
	if (marketPriceItemInfo->isRefreshComplete == RSSL_FALSE)
	{
		marketPriceItemInfo->isRefreshComplete = RSSL_TRUE;
	}

	/* send Market Price item request */
	if ((retval = sendMessage(upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Sends the item close status message for a channel.
 * upaChannel - The channel to send close status message to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 * marketPriceItemInfo - The market price item information
 */
RsslRet sendItemCloseStatusMsg(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter, UpaMarketPriceItemInfo* marketPriceItemInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Provider uses RsslStatusMsg to send the item close status and to close the stream. */
	RsslStatusMsg msg;

	char stateText[256];

	/* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Item Close Status Msg.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = upaGetBuffer(upaChannel, maxMsgSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the item close status. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearStatusMsg(&msg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(encodeIter, upaChannel->majorVersion, upaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS; /*!< (3) Status Message */
	msg.msgBase.streamId = MARKETPRICE_ITEM_STREAM_ID_START;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE; /*!< (6) Market Price Message */
	/* No payload associated with this close status message */
	msg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	msg.flags = RSSL_STMF_HAS_STATE;
	/*!< (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
	msg.state.streamState = RSSL_STREAM_CLOSED;
	/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	sprintf(stateText, "Item stream closed for item: %s\n", marketPriceItemInfo->itemName);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/* encode message */

	/* Since there is no payload, no need for Init/Complete as everything is in the msg header */
	/* Functions without a suffix of Init or Complete (e.g. rsslEncodeMsg) perform encoding within a single call,
	 * typically used for encoding simple types like Integer or incorporating previously encoded data
	 * (referred to as pre-encoded data).
	 */
	if ((retval = rsslEncodeMsg(encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		return RSSL_RET_FAILURE;
	}

	/* set the buffer's encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(encodeIter);

	/* send item close status */
	if ((retval = sendMessage(upaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* item close status fails */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the item close status doesn't flush, just close channel and exit the app. When you send item close status msg,
		 * we want to make a best effort to get this across the network as it will gracefully close the open item
		 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
		 */

		/* Closes channel, cleans up and exits the application. */
	}

	return retval;
}

