/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license
 *| AS IS with no warranty or guarantee of fit for purpose.
 *| See the project's LICENSE.md for details.
 *| Copyright (C) 2019 LSEG. All rights reserved.
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
 * LSEG Real-Time Distribution Systems, Data Feed Direct,
 * and LSEG Real-Time.
 *
 * Detailed Descriptions:
 * The first step of any ETA consumer application is to establish a 
 * network connection with its peer component (i.e., another application 
 * with which to interact). User must start a interactive provider (server) or a 
 * non-interactive provider which connects to an ADH. 
 * There are training examples available for Providers and NIProviders.
 * An OMM consumer typically creates an outbound 
 * connection to the well-known hostname and port of a server (Interactive 
 * Provider or ADS). The consumer uses the rsslConnect() function to initiate 
 * the connection and then uses the rsslInitChannel() function to complete 
 * channel initialization.
 * 
 *
 ************************************************************************
 * ETA Consumer Training Module 1b: Ping (heartbeat) Management
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
 * ETA Consumer Training Module 1c: Reading and Writing Data
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
 * mechanism that the RsslChannel.socketId is registered with. The ETA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines RsslBuffer boundaries and returns each buffer one by 
 * one.
 * 
 * (Buffer is used since Ping module and all the latter examples.)
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
 *
 ************************************************************************
 * ETA Consumer Training Module 2: Log in
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
 * Content is encoded and decoded using the ETA Message Package and the ETA  
 * Data Package. 
 * 
 *
 ************************************************************************
 * ETA Consumer Training Module 3: Obtain Source Directory
 ************************************************************************
 * Summary:
 * The Source Directory domain model conveys information about all available 
 * services in the system. An OMM consumer typically requests a Source 
 * Directory to retrieve information about available services and their 
 * capabilities. This includes information about supported domain types, the 
 * service’s state, the quality of service (QoS), and any item group 
 * information associated with the service.
 *
 * Detailed Descriptions:
 * The Source Directory Info filter contains service name and serviceId 
 * information for all available services. When an appropriate service is 
 * discovered by the OMM consumer, it uses the serviceId associated with the 
 * service on all subsequent requests to that service. 
 *
 * The Source Directory State filter contains status information for service, 
 * which informs the consumer whether the service is Up and available, or Down
 * and unavailable. 
 *
 * The Source Directory Group filter conveys item group status information, 
 * including information about group states, as well as the merging of groups. 
 *

 * Content is encoded and decoded using the ETA Message Package and the ETA 
 * Data Package.
 *
 *
 ************************************************************************
 * ETA Consumer Training Module 4: Obtain Dictionary Information
 ************************************************************************
 * Summary:
 * Consumer applications often require a dictionary for encoding or decoding 
 * specific pieces of information. This dictionary typically defines type and 
 * formatting information. Content that uses the RsslFieldList type requires 
 * the use of a field dictionary (usually the LSEG RDMFieldDictionary, 
 * although it could also be a user-defined or user-modified field dictionary).
 * A consumer application can choose whether to load necessary dictionary 
 * information from a local file or download the information from an available 
 * provider.
 *
 * Detailed Descriptions:
 * The Source Directory message should inform (from previous Module 3):
 * - DictionariesProvided: Which dictionaries are available for download.
 * - DictionariesUsed: The consumer of any dictionaries required to decode 
 *   the content provided on a service. (Not used in previous Module 3)
 *
 * A consumer application can determine whether to load necessary dictionary 
 * information from a local file or download the information from the
 * provider if available.
 * 
 * - If loading from a file, ETA offers several utility functions to load and 
 *   manage a properly-formatted field dictionary.
 * - If downloading information, the application issues a request using the 
 *   Dictionary domain model. The provider application should respond with a 
 *   dictionary response, typically broken into a multi-part message. ETA 
 *   offers several utility functions for encoding and decoding of the
 *   Dictionary domain content.
 * 
 *
 * Change the function closeChannelCleanUpAndExit() by adding parameter of datadictionary.
 *
 * After check for source directory, it will check dictionary(add more code after source directory).
 * Also added are two more functions:processDictionaryResponse() and sendDictionaryRequest().
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA  
 * Data Package.
 *
 *
 ************************************************************************
 * ETA Consumer Training Module 5: Issue Item Requests 
 ************************************************************************
 * Summary:
 * After the consumer application successfully logs in and obtains Source 
 * Directory and Dictionary information, it can request additional content. 
 * When issuing the request, the consuming application specifies the serviceId 
 * of the desired service along with a streamId. Requests can be sent for any 
 * domain using the formats defined in that domain model specification. In this 
 * simple example, we show how to make a Market Price level I data Item request 
 * to obtain the data from a provider.
 * 
 * Detailed Descriptions:
 * The Market Price domain provides access to Level I market information such as 
 * trades, indicative quotes, and top-of-book quotes. All information is sent as 
 * an RsslFieldList. Field-value pairs contained in the field list include information 
 * related to that item (i.e., net change, bid, ask, volume, high, low, or last price).
 *
 * A Market Price request message is encoded and sent by OMM consumer applications. The 
 * request specifies the name and attributes of an item in which the consumer is 
 * interested. If a consumer wishes to receive updates, it can make a "streaming"
 * request by setting the RSSL_RQMF_STREAMING flag. If the flag is not set, the consumer 
 * is requesting a "snapshot," and the refresh should end the request.
 *
 * Market Price data is conveyed as an RsslFieldList, where each RsslFieldEntry 
 * corresponds to a piece of information and its current value. The field list should be 
 * decoded using its associated Field Dictionary, indicated by the dictionaryId present 
 * in the field list.
 * Similar to module_Dictionary, the main change is to add one more option in the second main loop.
 * Also added 4 more functions: sendMarketPriceItemRequest(), processMarketPriceItemResponse(), decodeMarketPricePayload()
 * and closeMarketPriceItemStream(). More details are in those functions.
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

#include "Consumer_Training.h"

/* dictionary file name  */
const char *fieldDictionaryFileName = "RDMFieldDictionary";
/* dictionary download name */
const char *dictionaryDownloadName = "RWFFld";

/* enum table file name */
const char *enumTypeDictionaryFileName = "enumtype.def";
/* enum table download name */
const char *enumTableDownloadName = "RWFEnum";

int main(int argc, char **argv)
{
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */

	/* This example suite uses write descriptor in our client/consumer type examples in mainly 2 areas with
	 * the I/O notification mechanism being used:
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
	char srvrHostname[128], srvrPortNo[128], interfaceName[128], serviceName[128], itemName[128];

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

	/* ETA channel management information */
	EtaChannelManagementInfo etaChannelManagementInfo;

	RsslBuffer* msgBuf = 0;

	time_t currentTime = 0;
	time_t etaRuntime = 0;
	RsslUInt32 runTime = 0;

	/* In this app, we are only interested in using 2 dictionaries:
	 * - Field Dictionary (RDMFieldDictionary) and
	 * - Enumerated Types Dictionaries (enumtype.def)
	 *
	 * Dictionaries may be available locally in a file, or available for request over the network from an upstream provider.
	 */

	/* data dictionary */
	RsslDataDictionary dataDictionary;

	/* dictionary loaded from file flag */
	RsslBool fieldDictionaryLoadedFromFile = RSSL_FALSE;

	/* enum table loaded from file flag */
	RsslBool enumTypeDictionaryLoadedFromFile = RSSL_FALSE;

	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};


	/* ETA provides clear functions for its structures (e.g., rsslClearDecodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_DECODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	etaChannelManagementInfo.etaChannel = 0;

	/* the default option parameters */
	/* connect to server running on same machine */
	snprintf(srvrHostname, 128, "%s", "localhost");
	/* server is running on port number 14002 */
	snprintf(srvrPortNo, 128, "%s", "14002");
	/* use default NIC network interface card to bind to for all inbound and outbound data */
	snprintf(interfaceName, 128, "%s", "");
	/* use default runTime of 300 seconds */
	runTime = 300;
	/* default service name is "DIRECT_FEED" used in source directory handler */
	snprintf(serviceName, 128, "%s", "DIRECT_FEED");
	/* default item name is "TRI" used in market price item request handler */
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
			else if (strcmp("-mp", argv[i]) == 0)
			{
				i += 2;
				snprintf(itemName, 128, "%s", argv[i-1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[i]);
				printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-mp <ItemName>] \n", argv[0], argv[0]);
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

	/* get current time */
	time(&currentTime);

	/* Set the runtime of the ETA Consumer application to be runTime (seconds) */
	etaRuntime = currentTime + (time_t)runTime;

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

	/* Initialize/clear source directory service discovery information */
	/* Initializes/Clears to set if RDMFieldDictionary and enumtype.def are available for downloading to be RSSL_FALSE
	 * and Clears/initializes the service's state information to be 0 (Down) and 0 (Not Accepting Requests)
	 */
	etaChannelManagementInfo.serviceDiscoveryInfo.serviceId = 0;
	etaChannelManagementInfo.serviceDiscoveryInfo.serviceNameFound = RSSL_FALSE;
	etaChannelManagementInfo.serviceDiscoveryInfo.etalDMTDictionarySupported = RSSL_FALSE;
	etaChannelManagementInfo.serviceDiscoveryInfo.etaDMTMarketPriceSupported = RSSL_FALSE;
	etaChannelManagementInfo.serviceDiscoveryInfo.RDMFieldDictionaryProvided = RSSL_FALSE;
	etaChannelManagementInfo.serviceDiscoveryInfo.enumtypeProvided = RSSL_FALSE;
	etaChannelManagementInfo.serviceDiscoveryInfo.ServiceState = 0;
	etaChannelManagementInfo.serviceDiscoveryInfo.AcceptingRequests = 0;

	/* Initialize/clear dictionaries loaded information */
	etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded = RSSL_FALSE;
	etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded = RSSL_FALSE;
	etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryFirstPart = RSSL_TRUE;
	etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryFirstPart = RSSL_TRUE;

	/* Initialize/reset market price item information */
	snprintf(etaChannelManagementInfo.marketPriceItemInfo.itemName, 128, "%s", itemName);
	etaChannelManagementInfo.marketPriceItemInfo.streamId = MARKETPRICE_ITEM_STREAM_ID;
	/* state management will be updated as refresh and status messages are received */
	rsslClearState(&etaChannelManagementInfo.marketPriceItemInfo.itemState);

	/*********************************************************
	 * For performance considerations, it is recommended to first load field and enumerated dictionaries from local files,
	 * if they exist, at the earlier stage of the consumer applications.
	 *
	 * When loading from local files, ETA offers several utility functions to load and manage a properly-formatted field dictionary
	 * and enum type dictionary.
	 *
	 * Only make Market Price item request after both dictionaries are successfully loaded from files.
	 * If at least one of the dictionaries fails to get loaded properly, it will continue the code path of downloading the
	 * failed loaded dictionary or both dictionaries (if neither exists in run-time path or loaded properly) from provider
	 *********************************************************/

	/* clear the RsslDataDictionary dictionary before first use/load
	 * This should be done prior to the first call of a dictionary loading function, if the static initializer is not used.
	 */
	rsslClearDataDictionary(&dataDictionary);

	/* load field dictionary from file - adds data from a Field Dictionary file to the RsslDataDictionary */
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &dataDictionary, &errorText) < 0)
		printf("\nUnable to load field dictionary. Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
	{
		printf("Successfully loaded field dictionary from (local) file.\n\n");
		etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded = RSSL_TRUE;
		fieldDictionaryLoadedFromFile = RSSL_TRUE;
	}

	/* load enumerated dictionary from file - adds data from an Enumerated Types Dictionary file to the RsslDataDictionary */
	if (rsslLoadEnumTypeDictionary(enumTypeDictionaryFileName, &dataDictionary, &errorText) < 0)
		printf("\nUnable to load enum type dictionary. Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
	{
		printf("Successfully loaded enum type dictionary from (local) file.\n\n");
		etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded = RSSL_TRUE;
		enumTypeDictionaryLoadedFromFile = RSSL_TRUE;
	}

	if ((etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded) && (etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded))
	{
		printf("ETA Consumer application has successfully loaded both dictionaries from (local) files.\n\n");
	}

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
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
					closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
								closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
							closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
		}
	}

	/* Initialize ping management handler */
	initPingManagementHandler(&etaChannelManagementInfo);

	/* Send Login request message */
	if ((retval = sendLoginRequest(&etaChannelManagementInfo)) > RSSL_RET_SUCCESS)
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
		/* Closes channel, cleans up and exits the application. */
		closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
		time_interval.tv_sec =  etaChannelManagementInfo.etaChannel->pingTimeout/60;
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
					 * Client/Consumer Application Lifecycle Major Step 4:
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
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
						}

						/* decode contents into the RsslMsg structure */
						retval = rsslDecodeMsg(&decodeIter, &msg);
						if (retval != RSSL_RET_SUCCESS)
						{
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd="SOCKET_PRINT_TYPE"  Size %d \n", retval, etaChannelManagementInfo.etaChannel->socketId, msgBuf->length);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}
								else
								{
									printf("ETA Consumer application is granted access and has logged in successfully.\n\n");

									snprintf(etaChannelManagementInfo.serviceDiscoveryInfo.serviceName, 128, "%s", serviceName);

									/* Send Source Directory request message */
									if ((retval = sendSourceDirectoryRequest(&etaChannelManagementInfo)) > RSSL_RET_SUCCESS)
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
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}
								}
							}
							break;
							/*!< (4) Source Message */
							case RSSL_DMT_SOURCE:
							{
								if (processSourceDirectoryResponse(&etaChannelManagementInfo, &msg, &decodeIter) != RSSL_RET_SUCCESS)
								{
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}
								else
								{
									printf("ETA Consumer application has successfully received source directory information.\n\n");
								}

								/* exit app if service name entered by user cannot be found */
								if (!etaChannelManagementInfo.serviceDiscoveryInfo.serviceNameFound)
								{
									printf("\nSource directory response does not contain service name: %s. Exit app.\n", serviceName);
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}

								/* exit app if service we care about is NOT up and accepting requests */
								if ((etaChannelManagementInfo.serviceDiscoveryInfo.ServiceState != RDM_DIRECTORY_SERVICE_STATE_UP) || (!etaChannelManagementInfo.serviceDiscoveryInfo.AcceptingRequests))
								{
									printf("\nService name: %s is NOT up and accepting requests. Exit app.\n", serviceName);
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}

								/* A consumer application can determine whether to load necessary dictionary information from a local file or
								 * download the information from the provider if available.
								 *
								 * - If loading from a file, ETA offers several utility functions to load and manage a properly-formatted field dictionary.
								 * - If downloading information, the application issues a request using the Dictionary domain model. The provider application
								 * should respond with a dictionary response, typically broken into a multi-part message. ETA offers several utility functions
								 * for encoding and decoding of the Dictionary domain content.
								 *
								 * In this simple app, we are trying to first loading necessary dictionary information from a local file, if it exists.
								 * For performance considerations, we first load field and enumerated dictionaries from local files, if they exist,
								 * at the earlier stage of the consumer applications.
								 *
								 * Otherwise, if loading dictionary information from a local file fails, we download the necessary dictionary information
								 * from provider if available.
								 */

								/*********************************************************
								 * The Consumer app should already first tried to load field and enumerated dictionaries from local files at the beginning of
								 * the app.
								 *
								 * Only make Market Price item request after both dictionaries are successfully loaded from files.
								 * If at least one of the dictionaries fails to get loaded properly, it will continue the code path of downloading the
								 * failed loaded dictionary or both dictionaries (if neither exists in run-time path or loaded properly) from provider
								 *********************************************************/

								/* Only make Market Price item request after both dictionaries are successfully loaded from files. */
								if ((etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded) && (etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded))
								{
									/* check to see if the provider supports the Market Price Domain Type (RSSL_DMT_MARKET_PRICE) */
									if (!etaChannelManagementInfo.serviceDiscoveryInfo.etaDMTMarketPriceSupported)
									{
										printf("\nRSSL_DMT_MARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									/* For this simple example, send just 1 Market Price item request message */

									/* Send just 1 Market Price item request message */
									if ((retval = sendMarketPriceItemRequest(&etaChannelManagementInfo)) > RSSL_RET_SUCCESS)
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
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									break;
								}

								/*********************************************************
								 * Need to continue the code path to download at least 1 dictionary from provider
								 * because at least 1 loading of either field and enumerated dictionaries from local files failed
								 *
								 * If downloading information, the application issues a request using the Dictionary domain model. The provider application
								 * should respond with a dictionary response, typically broken into a multi-part message. ETA offers several utility functions
								 * for encoding and decoding of the Dictionary domain content.
								 *
								 * Only make Market Price item request after both dictionaries are successfully downloaded from provider or loaded from file
								 * already (in that case, would not download for that dictionary).
								 *********************************************************/

								/* clear the RsslDataDictionary dictionary before first use/load
								 * This should be done prior to the first call of a dictionary loading function, if the static initializer is not used.
								 */
								rsslClearDataDictionary(&dataDictionary);

								/* Will attempt to download the LSEG Field Dictionary (RDMFieldDictionary) from provider. */
								if (!etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded)
								{
									/* check if Dictionary Domain Type is supported */
									if (!etaChannelManagementInfo.serviceDiscoveryInfo.etalDMTDictionarySupported)
									{
										printf("\nDictionary Domain Type is NOT supported. Exit app.\n");
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									/* check if RDMFieldDictionary is available for downloading */
									if (!etaChannelManagementInfo.serviceDiscoveryInfo.RDMFieldDictionaryProvided)
									{
										printf("\nRDMFieldDictionary is NOT available for downloading. Exit app.\n");
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									/* Send RDMFieldDictionary Dictionary request message */
									if ((retval = sendDictionaryRequest(&etaChannelManagementInfo, dictionaryDownloadName)) > RSSL_RET_SUCCESS)
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
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}
								}

								/* Will attempt to download Enumerated Types Dictionaries (enumtype.def) from provider. */
								if (!etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded)
								{
									/* check if Dictionary Domain Type is supported */
									if (!etaChannelManagementInfo.serviceDiscoveryInfo.etalDMTDictionarySupported)
									{
										printf("\nDictionary Domain Type is NOT supported. Exit app.\n");
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									/* check if enumtype.def is available for downloading */
									if (!etaChannelManagementInfo.serviceDiscoveryInfo.enumtypeProvided)
									{
										printf("\nenumtype.def is NOT available for downloading. Exit app.\n");
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}

									/* Send enumtype.def Dictionary request message */
									if ((retval = sendDictionaryRequest(&etaChannelManagementInfo, enumTableDownloadName)) > RSSL_RET_SUCCESS)
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
										/* Closes channel, cleans up and exits the application. */
										closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
									}
								}
							}
							break;
							/*!< (5) Dictionary Message */
							case RSSL_DMT_DICTIONARY:
							{
								if (processDictionaryResponse(&etaChannelManagementInfo, &msg, &decodeIter, &dataDictionary) != RSSL_RET_SUCCESS)
								{
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}
								else
								{
									/* Only make Market Price item request after both dictionaries are successfully downloaded from the provider. */
									if ((etaChannelManagementInfo.dictionariesLoadedInfo.fieldDictionaryLoaded) && (etaChannelManagementInfo.dictionariesLoadedInfo.enumTypeDictionaryLoaded))
									{
										printf("ETA Consumer application has successfully downloaded both dictionaries or successfully downloaded 1 dictionary if the other dictionary has already been loaded from (local) file successfully.\n\n");

										/* check to see if the provider supports the Market Price Domain Type (RSSL_DMT_MARKET_PRICE) */
										if (!etaChannelManagementInfo.serviceDiscoveryInfo.etaDMTMarketPriceSupported)
										{
											printf("\nRSSL_DMT_MARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
											/* Closes channel, cleans up and exits the application. */
											closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
										}

										/* For this simple example, send just 1 Market Price item request message */

										/* Send just 1 Market Price item request message */
										if ((retval = sendMarketPriceItemRequest(&etaChannelManagementInfo)) > RSSL_RET_SUCCESS)
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
											/* Closes channel, cleans up and exits the application. */
											closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
										}
									}
								}
							}
							break;
							/*!< (6) Market Price Message */
							case RSSL_DMT_MARKET_PRICE:
							{
								if (processMarketPriceItemResponse(&etaChannelManagementInfo, &msg, &decodeIter, &dataDictionary) != RSSL_RET_SUCCESS)
								{
									/* Closes channel, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
								}
								else
								{
									printf("ETA Consumer application has successfully received Market Price item response.\n\n");
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
						etaChannelManagementInfo.pingManagementInfo.receivedServerMsg = RSSL_TRUE;
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
								etaChannelManagementInfo.pingManagementInfo.receivedServerMsg = RSSL_TRUE;
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
							default: /* Error handling */
							{
								if (retval_rsslRead < 0)
								{
									printf("Error %s (%d) (errno: %d) encountered with rsslRead fd="SOCKET_PRINT_TYPE". Error Text: %s\n",
										rsslRetCodeToString(error.rsslErrorId), error.rsslErrorId, error.sysError,
										etaChannelManagementInfo.etaChannel->socketId, error.text);
									/* Closes channel/connection, cleans up and exits the application. */
									closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
							closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
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
			/* Closes channel, cleans up and exits the application. */
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
		}

		/* get current time */
		time(&currentTime);

		/* Handles the run-time for the ETA Consumer application. Here we exit the application after a predetermined time to run */
		if (currentTime >= etaRuntime)
		{
			/* Closes all streams for the consumer after run-time has elapsed. */
			/* Close Market Price item stream */
			if ((retval = closeMarketPriceItemStream(&etaChannelManagementInfo)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you close Market Price item, we want to make a best effort to get this across the network as it will gracefully
				 * close the open Market Price item stream. If this cannot be flushed or failed, this application will just close the
				 * connection for simplicity.
				 */

				/* Closes channel, cleans up and exits the application. */
				closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
			}

			/* Close Login stream */
			/* Note that closing Login stream will automatically close all other streams at the provider */
			if ((retval = closeLoginStream(&etaChannelManagementInfo)) != RSSL_RET_SUCCESS) /* (retval > RSSL_RET_SUCCESS) or (retval < RSSL_RET_SUCCESS) */
			{
				/* When you close login, we want to make a best effort to get this across the network as it will gracefully
				 * close all open streams. If this cannot be flushed or failed, this application will just close the connection
				 * for simplicity.
				 */

				/* Closes channel, cleans up and exits the application. */
				closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_FAILURE, &dataDictionary);
			}

			/* flush before exiting */
			if (FD_ISSET(etaChannelManagementInfo.etaChannel->socketId, &cleanWriteFds))
			{
				retval = 1;
				while (retval > RSSL_RET_SUCCESS)
				{
					retval = rsslFlush(etaChannelManagementInfo.etaChannel, &error);
				}
				if (retval < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
			}

			printf("\nETA Consumer run-time has expired...\n");
			closeChannelCleanUpAndExit(etaChannelManagementInfo.etaChannel, RSSL_RET_SUCCESS, &dataDictionary);
		}
	}
}

/*
 * Closes channel, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 * dataDictionary -  the dictionaries that need to be unloaded to clean up memory
 */
void closeChannelCleanUpAndExit(RsslChannel* etaChannel, int code, RsslDataDictionary* dataDictionary)
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

/*
 * Initializes the ping times for etaChannelManagementInfo.etaChannel.
 * etaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* get current time */
	time(&etaChannelManagementInfo->pingManagementInfo.currentTime);

	/* set ping timeout for client and server */
	/* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
	 * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
	 * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
	 * network or operating system notification that may occur.
	 */
	etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient = etaChannelManagementInfo->etaChannel->pingTimeout/3;
	etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer = etaChannelManagementInfo->etaChannel->pingTimeout;

	/* set time to send next ping from client */
	etaChannelManagementInfo->pingManagementInfo.nextSendPingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;

	/* set time client should receive next message/ping from server */
	etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;

	etaChannelManagementInfo->pingManagementInfo.receivedServerMsg = RSSL_FALSE;
}

/*
 * Processing ping management handler for etaChannelManagementInfo.etaChannel.
 * etaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	/* Handles the ping processing for etaChannelManagementInfo.etaChannel. Sends a ping to the server if the next send ping time has arrived and
	 * checks if a ping has been received from the server within the next receive ping time.
	 */
	RsslRet	retval = RSSL_RET_SUCCESS;
	RsslError error;

	/* get current time */
	time(&etaChannelManagementInfo->pingManagementInfo.currentTime);

	/* handle client pings */
	if (etaChannelManagementInfo->pingManagementInfo.currentTime >= etaChannelManagementInfo->pingManagementInfo.nextSendPingTime)
	{
		/* send ping to server */
		/*********************************************************
		 * Client/Consumer Application Lifecycle Major Step 4:
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
					/* Ping message has been sent successfully to the server ... */
					printf("Ping message has been sent successfully to the server ... \n\n");
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

		/* set time to send next ping from client */
		etaChannelManagementInfo->pingManagementInfo.nextSendPingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutClient;
	}

	/* handle server pings - an application should determine if data or pings have been received,
	 * if not application should determine if pingTimeout has elapsed, and if so connection should be closed
	 */
	if (etaChannelManagementInfo->pingManagementInfo.currentTime >= etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime)
	{
		/* check if client received message from server since last time */
		if (etaChannelManagementInfo->pingManagementInfo.receivedServerMsg)
		{
			/* reset flag for server message received */
			etaChannelManagementInfo->pingManagementInfo.receivedServerMsg = RSSL_FALSE;

			/* set time client should receive next message/ping from server */
			etaChannelManagementInfo->pingManagementInfo.nextReceivePingTime = etaChannelManagementInfo->pingManagementInfo.currentTime + (time_t)etaChannelManagementInfo->pingManagementInfo.pingTimeoutServer;
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
 * etaChannel - The channel to send the message buffer to
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
	 * Client/Consumer Application Lifecycle Major Step 4:
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
 * Send Login request message to a channel. This consists of getting a message buffer, setting the login request
 * information, encoding the login request, and sending the login request to the server. A Login request message is
 * encoded and sent by OMM consumer and OMM non-interactive provider applications. This message registers a user
 * with the system. After receiving a successful Login response, applications can then begin consuming or providing
 * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
 * etaChannelInfo - The channel management information including the channel to send the Login request message buffer to
 */
RsslRet sendLoginRequest(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
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

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Login request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

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

	/* The ETA Transport layer provides several utility functions. rsslGetUserName utility function takes an RsslBuffer with associated memory
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
	applicationName.data = (char*)"ETA Consumer Training";
	applicationName.length = (RsslUInt32)strlen("ETA Consumer Training");
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
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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

			/* decode key attrib data, if present */
			if (key->flags & RSSL_MKF_HAS_ATTRIB)
			{
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
								printf("\tReceived Login Response for ApplicationId: %.*s\n", elementEntry.encData.length, elementEntry.encData.data);

							/* ApplicationName */
							else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_APPNAME))
								printf("\tReceived Login Response for ApplicationName: %.*s\n", elementEntry.encData.length, elementEntry.encData.data);

							/* Position */
							else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_POSITION))
								printf("\tReceived Login Response for Position: %.*s\n", elementEntry.encData.length, elementEntry.encData.data);
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
 * etaChannelInfo - The channel management information including the channel to send the Login close message buffer to
 */
RsslRet closeLoginStream(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Consumer uses RsslCloseMsg to indicate no further interest in an item stream and to close the stream. */
	RsslCloseMsg msg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Login close.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
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
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

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
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
		retval = rsslFlush(etaChannel, &error);
		if (retval < RSSL_RET_SUCCESS)
		{
			printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
			/* Closes channel, cleans up and exits the application. */
			*rsslError = error;
			return NULL;
		}

		/* call rsslGetBuffer again to see if it works now after rsslFlush */
		if ((msgBuf = rsslGetBuffer(etaChannel, size, RSSL_FALSE, &error)) == NULL)
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
 * Send Source Directory request message to a channel. This consists of getting a message buffer, setting the source
 * directory request information, encoding the source directory request, and sending the source directory request to
 * the server. A Source Directory request message is encoded and sent by OMM consumer applications. The Source Directory
 * domain model conveys information about all available services in the system. An OMM consumer typically requests a
 * Source Directory to retrieve information about available services and their capabilities.
 * etaChannelInfo - The channel management information including the channel to send the Source Directory request message buffer to
 */
RsslRet sendSourceDirectoryRequest(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Source Directory request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Source Directory request. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRequestMsg(&reqMsg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	reqMsg.msgBase.msgClass = RSSL_MC_REQUEST; /*!< (1) Request Message */
	reqMsg.msgBase.streamId = SRCDIR_STREAM_ID;
	reqMsg.msgBase.domainType = RSSL_DMT_SOURCE; /*!< (4) Source Message */
	reqMsg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/* RSSL_RQMF_STREAMING flag set. When the flag is set, the request is known as a "streaming" request, meaning
		* that the refresh will be followed by updates.
		*/
	reqMsg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;

	/*!< (0x002) This RsslRequestMsg has priority information, contained in \ref RsslRequestMsg::priorityClass and
		* \ref RsslRequestMsg::priorityCount. This is used to indicate the importance of this stream.
		*/
	reqMsg.priorityClass = 1;
	reqMsg.priorityCount = 1;

	/* A consumer can request information about all services by omitting serviceId information
	 * If the consumer wishes to receive information about all services, the consumer should not specify a serviceId
	 * (i.e., the consumer should not set RSSL_MKF_HAS_SERVICE_ID).
	 */

	/* set members in msgKey */
	/*!< (0x0008) This RsslMsgKey has a filter, contained in \ref RsslMsgKey::filter.  */
	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;

	/* Because the Source Directory domain uses an RsslFilterList, a consumer can indicate the specific source related
	 * information in which it is interested via a msgKey.filter. Each bit-value represented in the filter corresponds
	 * to an information set that can be provided in response messages.
	 * LSEG recommends that a consumer application minimally request Info, State, and Group filters for the
	 * Source Directory:
	 * - The Info filter contains the service name and serviceId data for all available services. When an appropriate
	 *   service is discovered by the OMM Consumer, the serviceId associated with the service is used on subsequent
	 *   requests to that service.
	 * - The State filter contains status data for the service. Such data informs the Consumer whether the service is
	 *   up (and available) or down (and unavailable).
	 * - The Group filter conveys any item group status information, including group states and as regards the merging
	 *   of groups if applicable.
	 */
	reqMsg.msgBase.msgKey.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | \
								RDM_DIRECTORY_SERVICE_STATE_FILTER | \
								RDM_DIRECTORY_SERVICE_GROUP_FILTER;

	/* encode message - populate message and encode it */
	if ((retval = rsslEncodeMsg(&encodeIter, (RsslMsg*)&reqMsg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send source directory request */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Processes a source directory response. This consists of decoding the response.
 * etaChannelInfo - The channel management information including the source directory service discovery information that is populated
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processSourceDirectoryResponse(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter)
{
	RsslRet retval = 0;
	RsslState *pState = 0;

	char tempData[1024];
	RsslBuffer tempBuffer;

	RsslMap rsslMap;
	/* create a single map entry to reuse */
	RsslMapEntry mapEntry;

	RsslFilterList filterList;
	/* create single filter entry and reuse while decoding each entry */
	RsslFilterEntry filterEntry;

	RsslElementList	elementList;
	RsslElementEntry elementEntry;

	RsslArray rsslArray;
	RsslBuffer arrayBuffer;
	int arrayCount = 0;

	int serviceCount = 0;

	/* this keeps track of which service we are actually interested in, that is,
	 * when we run into the serviceName requested by the application
	 */
	int foundServiceIndex = -1;

	char serviceName[128];
	/* create primitive value to have key decoded into */
	RsslUInt64 serviceId;

	RsslUInt64 capabilities;

	char dictionariesProvided[128];

	RsslQos	QoS;
	RsslBuffer QosBuf;
	RsslBool foundQoS = RSSL_FALSE;

	/* The ServiceState and AcceptingRequests elements in the State filter entry work together
	 * to indicate the ability of a particular service to provide data:
	 */
	RsslUInt64 serviceState;
	RsslUInt64 acceptingRequests;

	RsslState serviceStatus;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		/* Process Refresh and Update in similar fashion */
		case RSSL_MC_REFRESH: /*!< (2) Refresh Message */
		case RSSL_MC_UPDATE: /*!< (4) Update Message */
		{
			/* decode source directory refresh or update */

			if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
				printf("\nReceived Source Directory Refresh\n");
			else /*!< (4) Update Message */
			{
				printf("\nReceived Source Directory Update\n");

				/* When displaying update information, we should also display the updateType information. */
				printf("UPDATE TYPE: %u\n", msg->updateMsg.updateType);
			}

			/* decode contents into the map structure */
			if ((retval = rsslDecodeMap(decodeIter, &rsslMap)) < RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeMap. Error Text: %s\n",
					rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
				return retval;
			}

			/*!< @brief The primitive type associated with each RsslMapEntry key. */
			/* source directory refresh or update key data type must be RSSL_DT_UINT */
			if (rsslMap.keyPrimitiveType != RSSL_DT_UINT)
			{
				printf("rsslMap has incorrect keyPrimitiveType: %s", rsslDataTypeToString(rsslMap.keyPrimitiveType));
				return RSSL_RET_FAILURE;
			}

			/* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
			 * indicates to ETA that user wants to decode summary data
			 */
			if (rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA)	{
				/* rsslMap summary data is present. Its type should be that of rsslMap.containerType */
				printf("summary data is present. Its type should be that of rsslMap.containerType\n");
				/* Continue decoding ... */
			}
			else
			{
				//printf("\nrsslMap summary data is NOT present.\n");
			}

			/* decode each map entry, passing in pointer to keyPrimitiveType decodes mapEntry key as well
			 * service id is contained in map entry encKey
			 * store service id in source directory refresh or update information
			 */
			while ((retval = rsslDecodeMapEntry(decodeIter, &mapEntry, &serviceId)) != RSSL_RET_END_OF_CONTAINER)
			{
				/* break out of decoding when predetermined max services (15) reached */
				if (serviceCount == 15)
				{
					/* The decoding process typically runs until the end of each container, indicated by RSSL_RET_END_OF_CONTAINER.
					 * This rsslFinishDecodeEntries function sets the application to skip remaining entries in a container and continue
					 * the decoding process. This function will skip past remaining entries in the container and perform necessary
					 * synchronization between the content and iterator so that decoding can continue.
					 */
					rsslFinishDecodeEntries(decodeIter);
					printf("processSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
					break;
				}

				if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
				{
					/* decoding failure tends to be unrecoverable */
					printf("Error %s (%d) encountered with rsslDecodeMapEntry. Error Text: %s\n",
						rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
					return retval;
				}
				else
				{
					if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
						printf("\nReceived Source Directory Refresh for Decoded Service Id: " RTR_LLU "", serviceId);
					else /*!< (4) Update Message */
						printf("\nReceived Source Directory Update for Decoded Service Id: " RTR_LLU "", serviceId);

					/* if this is the current serviceId we are interested in */
					if ((serviceId == etaChannelManagementInfo->serviceDiscoveryInfo.serviceId) && (etaChannelManagementInfo->serviceDiscoveryInfo.serviceNameFound == RSSL_TRUE))
					{
						/* this is the current serviceId we are interested in and requested by the ETA Consumer application */
						printf(" (%s)\n", etaChannelManagementInfo->serviceDiscoveryInfo.serviceName);
					}

					/* decode contents into the filter list structure */
					if ((retval = rsslDecodeFilterList(decodeIter, &filterList)) < RSSL_RET_SUCCESS)
					{
						/* decoding failure tends to be unrecoverable */
						printf("Error %s (%d) encountered with rsslDecodeFilterList. Error Text: %s\n",
							rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
						return retval;
					}

					/* decode each filter entry until there are no more left.
					 * Decodes an RsslFilterEntry. This function expects the same RsslDecodeIterator that was used with rsslDecodeFilterList.
					 * This populates encData with an encoded entry.
					 */
					while ((retval = rsslDecodeFilterEntry(decodeIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
					{
						if (retval < RSSL_RET_SUCCESS)
						{
							/* decoding failure tends to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeFilterEntry. Error Text: %s\n",
								rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
							return retval;
						}
						else
						{
							/* Show how to use RsslContainerType.
							 * After rsslDecodeFilterEntry function returns, the RsslFilterList.containerType (or RsslFilterEntry.containerType if present)
							 * can invoke the correct contained type’s decode functions.
							 * if filterEntry.containerType is present, switch on that; otherwise switch on filterList.containerType
							 */
							RsslContainerType cType;
							if (filterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE)
								cType = filterEntry.containerType;
							else
								cType = filterList.containerType;

							switch (cType)
							{
								/*!< (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
								case RSSL_DT_MAP:
								{
									/* Continue decoding map entries. call rsslDecodeMap function
									 * printf("rsslDecodeFilterEntry: Continue decoding map entries.\n");
									 */
								}
								break;
								/*!< (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
								case RSSL_DT_ELEMENT_LIST:
								{
									/* For Source Directory response, we actually know it is RSSL_DT_ELEMENT_LIST up-front
									 * See code below in the next switch block: we are calling rsslDecodeElementList to
									 * continue decoding element entries
									 */

									/* Continue decoding element entries. call rsslDecodeElementList function
									 * printf("rsslDecodeFilterEntry: Continue decoding element entries.\n");
									 */
								}
								break;
								default: /* Error handling */
								{
									printf("\nUnkonwn RsslContainerType: %d\n", cType);
									return RSSL_RET_FAILURE;
								}
							}

							/* decode source directory response information */
							switch (filterEntry.id)
							{
								case RDM_DIRECTORY_SERVICE_INFO_ID: /*!< (1) Service Info Filter ID */
								{
									printf("\nDecoding Service Info Filter ID FilterListEntry\n");

									/* decode element list - third parameter is 0 because we do not have set definitions in this example */
									if ((retval = rsslDecodeElementList(decodeIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
									{
										/* decoding failure tends to be unrecoverable */
										printf("Error %s (%d) encountered with rsslDecodeElementList. Error Text: %s\n",
											rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
										return retval;
									}

									/* decode element list elements */
									while ((retval = rsslDecodeElementEntry(decodeIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										if (retval < RSSL_RET_SUCCESS)
										{
											/* decoding failure tends to be unrecoverable */
											printf("Error %s (%d) encountered with rsslDecodeElementEntry. Error Text: %s\n",
												rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
											return retval;
										}
										else
										{
											/* get service general information */

											/* Name */
											if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_NAME))
											{
												if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
													printf("\tReceived Source Directory Refresh for ServiceName: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);
												else /*!< (4) Update Message */
													printf("\tReceived Source Directory Update for ServiceName: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);

												/* When an appropriate service is discovered by the OMM consumer, it uses the serviceId associated with the service
												 * on all subsequent requests to that service. Check if service name received in response matches that entered by user
												 * if it does, store the service id
												 */
												strncpy(serviceName, elementEntry.encData.data, elementEntry.encData.length);
												serviceName[elementEntry.encData.length] = '\0';

												if (!strcmp(serviceName, etaChannelManagementInfo->serviceDiscoveryInfo.serviceName))
												{
													/* serviceName requested by the application is FOUND */
													foundServiceIndex = serviceCount;

													printf("\tService name: %s (" RTR_LLU ") is discovered by the OMM consumer. \n", serviceName, serviceId);
													etaChannelManagementInfo->serviceDiscoveryInfo.serviceId = serviceId;
													etaChannelManagementInfo->serviceDiscoveryInfo.serviceNameFound = RSSL_TRUE;
												}
											}

											/* Capabilities */
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_CAPABILITIES))
											{
												/* decode into the array structure header */
												if ((retval = rsslDecodeArray(decodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
												{
													/* decoding failure tends to be unrecoverable */
													printf("Error %s (%d) encountered with rsslDecodeArray. Error Text: %s\n",
														rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
													return retval;
												}
												while ((retval = rsslDecodeArrayEntry(decodeIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
												{
													/* break out of decoding array items when predetermined max capabilities (10) reached */
													if (arrayCount == 10)
													{
														/* The decoding process typically runs until the end of each container, indicated by RSSL_RET_END_OF_CONTAINER.
														 * This rsslFinishDecodeEntries function sets the application to skip remaining entries in a container and continue
														 * the decoding process. This function will skip past remaining entries in the container and perform necessary
														 * synchronization between the content and iterator so that decoding can continue.
														 */
														rsslFinishDecodeEntries(decodeIter);
														break;
													}

													if (retval == RSSL_RET_SUCCESS)
													{
														retval = rsslDecodeUInt(decodeIter, &capabilities);
														if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
														{
															printf("Error %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
																	rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
															return retval;
														}
														if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
															printf("\tReceived Source Directory Refresh for Decoded Capabilities[%d]: " RTR_LLU "\n", arrayCount, capabilities);
														else /*!< (4) Update Message */
															printf("\tReceived Source Directory Update for Decoded Capabilities[%d]: " RTR_LLU "\n", arrayCount, capabilities);

														/* if advertising Dictionary domain type is supported */
														if (capabilities == RSSL_DMT_DICTIONARY)
														{
															printf("\tRSSL_DMT_DICTIONARY domain type is supported.\n");
															etaChannelManagementInfo->serviceDiscoveryInfo.etalDMTDictionarySupported = RSSL_TRUE;
														}

														/* if advertising MarketPrice domain type is supported */
														if (capabilities == RSSL_DMT_MARKET_PRICE)
														{
															printf("\tRSSL_DMT_MARKET_PRICE domain type is supported.\n");
															etaChannelManagementInfo->serviceDiscoveryInfo.etaDMTMarketPriceSupported = RSSL_TRUE;
														}

													}
													else if (retval != RSSL_RET_BLANK_DATA)
													{
														/* decoding failure tends to be unrecoverable */
														printf("Error %s (%d) encountered with rsslDecodeArrayEntry. Error Text: %s\n",
															rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
														return retval;
													}
													arrayCount++;
												}
												arrayCount = 0;
											}

											/* DictionariesProvided */
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_DICTIONARYS_PROVIDED))
											{
												/* decode into the array structure header */
												if ((retval = rsslDecodeArray(decodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
												{
													/* decoding failure tends to be unrecoverable */
													printf("Error %s (%d) encountered with rsslDecodeArray. Error Text: %s\n",
														rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
													return retval;
												}
												while ((retval = rsslDecodeArrayEntry(decodeIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
												{
													/* break out of decoding array items when predetermined max dictionaries (5) reached */
													if (arrayCount == 5)
													{
														/* The decoding process typically runs until the end of each container, indicated by RSSL_RET_END_OF_CONTAINER.
														 * This rsslFinishDecodeEntries function sets the application to skip remaining entries in a container and continue
														 * the decoding process. This function will skip past remaining entries in the container and perform necessary
														 * synchronization between the content and iterator so that decoding can continue.
														 */
														rsslFinishDecodeEntries(decodeIter);
														break;
													}

													if (retval == RSSL_RET_SUCCESS)
													{
														if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
															printf("\tReceived Source Directory Refresh for DictionariesProvided[%d]: %.*s\n",arrayCount, arrayBuffer.length, arrayBuffer.data);
														else /*!< (4) Update Message */
															printf("\tReceived Source Directory Update for DictionariesProvided[%d]: %.*s\n",arrayCount, arrayBuffer.length, arrayBuffer.data);

														/* DictionariesProvided provide the dictionaries that are available for downloading */
														/* Our training ETA Consumer app only cares about RDMFieldDictionary and enumtype.def */
														strncpy(dictionariesProvided, arrayBuffer.data, arrayBuffer.length);
														dictionariesProvided[arrayBuffer.length] = '\0';

														if (!strcmp(dictionariesProvided, dictionaryDownloadName))
														{
															/* dictionary RDMFieldDictionary is available for downloading */
															printf("\tDictionary Provided: %s with filename: %s \n", dictionariesProvided, fieldDictionaryFileName);
															etaChannelManagementInfo->serviceDiscoveryInfo.RDMFieldDictionaryProvided = RSSL_TRUE;
														}
														else if (!strcmp(dictionariesProvided, enumTableDownloadName))
														{
															/* dictionary enumtype.def is available for downloading */
															printf("\tDictionary Provided: %s with filename: %s \n", enumTableDownloadName, enumTypeDictionaryFileName);
															etaChannelManagementInfo->serviceDiscoveryInfo.enumtypeProvided = RSSL_TRUE;
														}
													}
													else if (retval != RSSL_RET_BLANK_DATA)
													{
														/* decoding failure tends to be unrecoverable */
														printf("Error %s (%d) encountered with rsslDecodeArrayEntry. Error Text: %s\n",
															rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
														return retval;
													}
													arrayCount++;
												}
												arrayCount = 0;
											}

											/* QoS */
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_QOS))
											{
												foundQoS = RSSL_TRUE;
												/* decode into the array structure header */
												if ((retval = rsslDecodeArray(decodeIter, &rsslArray)) < RSSL_RET_SUCCESS)
												{
													/* decoding failure tends to be unrecoverable */
													printf("Error %s (%d) encountered with rsslDecodeArray. Error Text: %s\n",
														rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
													return retval;
												}
												while ((retval = rsslDecodeArrayEntry(decodeIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
												{
													/* break out of decoding array items when predetermined max QOS (5) reached */
													if (arrayCount == 5)
													{
														/* The decoding process typically runs until the end of each container, indicated by RSSL_RET_END_OF_CONTAINER.
														 * This rsslFinishDecodeEntries function sets the application to skip remaining entries in a container and continue
														 * the decoding process. This function will skip past remaining entries in the container and perform necessary
														 * synchronization between the content and iterator so that decoding can continue.
														 */
														rsslFinishDecodeEntries(decodeIter);
														break;
													}

													if (retval == RSSL_RET_SUCCESS)
													{
														/* Obtain QoS information such as data timeliness (e.g. real time) and rate (e.g. tick-by-tick). */
														retval = rsslDecodeQos(decodeIter, &QoS);
														if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
														{
															printf("rsslDecodeQos() failed with return code: %d\n", retval);
															return retval;
														}
														else
														{
															QosBuf.data = (char*)alloca(100);
															QosBuf.length = 100;
															rsslQosToString(&QosBuf, &QoS);
															printf("\tReceived %s\n", QosBuf.data);
														}
													}
													else if (retval != RSSL_RET_BLANK_DATA)
													{
														/* decoding failure tends to be unrecoverable */
														printf("Error %s (%d) encountered with rsslDecodeArrayEntry. Error Text: %s\n",
															rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
														return retval;
													}
													arrayCount++;
												}
												arrayCount = 0;

												/* if this is the serviceName that is requested by the application */
												if (serviceCount == foundServiceIndex)
												{
													/* Need to store the Source Directory QoS information */
													etaChannelManagementInfo->serviceDiscoveryInfo.QoS[0] = QoS;
												}
											}
										}
									}

									/* if QoS was not send in the directory refresh message set it to the default values */
									if (!foundQoS)
									{
										printf("\tNot Received Source Directory Refresh for QoS\n");
										printf("\tSet default QoS: Realtime/TickByTick/Static\n");

										for (arrayCount = 0; arrayCount < 5; arrayCount++)
										{
 											etaChannelManagementInfo->serviceDiscoveryInfo.QoS[arrayCount].timeliness = RSSL_QOS_TIME_REALTIME;
 											etaChannelManagementInfo->serviceDiscoveryInfo.QoS[arrayCount].rate = RSSL_QOS_RATE_TICK_BY_TICK;
 											etaChannelManagementInfo->serviceDiscoveryInfo.QoS[arrayCount].dynamic = RSSL_FALSE;
 										}
									}
								}
								break;
								case RDM_DIRECTORY_SERVICE_STATE_ID: /*!< (2) Source State Filter ID */
								{
									printf("\nDecoding Source State Filter ID FilterListEntry\n");

									/* decode element list - third parameter is 0 because we do not have set definitions in this example */
									if ((retval = rsslDecodeElementList(decodeIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
									{
										/* decoding failure tends to be unrecoverable */
										printf("Error %s (%d) encountered with rsslDecodeElementList. Error Text: %s\n",
											rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
										return retval;
									}

									/* decode element list elements */
									while ((retval = rsslDecodeElementEntry(decodeIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										if (retval < RSSL_RET_SUCCESS)
										{
											/* decoding failure tends to be unrecoverable */
											printf("Error %s (%d) encountered with rsslDecodeElementEntry. Error Text: %s\n",
												rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
											return retval;
										}
										else
										{
											/* get service state information */

											/* The ServiceState and AcceptingRequests elements in the State filter entry work together to indicate the ability of
											 * a particular service to provide data:
											 * - ServiceState indicates whether the source of the data is accepting requests.
											 * - AcceptingRequests indicates whether the immediate upstream provider (the provider to which the consumer is directly connected)
											 *   can accept new requests and/or process reissue requests on already open streams.
											 */

											/* for our training app, for the service we are intersted in to be considered really up, both ServiceState has to be Up (1), and
											 * AcceptingRequests has to be Yes (1). That way, New requests and reissue requests can be successfully processed.
											 */

											/* ServiceState - Up(1), Down (0) */
											/* RDM_DIRECTORY_SERVICE_STATE_DOWN - !< (0) Service state down */
											/* RDM_DIRECTORY_SERVICE_STATE_UP - !< (1) Service state up */
											if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_SVC_STATE))
											{
												retval = rsslDecodeUInt(decodeIter, &serviceState);
												if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
												{
													printf("Error %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
														rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
													return retval;
												}
												if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
													printf("\tReceived Source Directory Refresh for Decoded ServiceState: " RTR_LLU "\n", serviceState);
												else /*!< (4) Update Message */
													printf("\tReceived Source Directory Update for Decoded ServiceState: " RTR_LLU "\n", serviceState);

												/* if this is the serviceName that is requested by the application */
												if (serviceCount == foundServiceIndex)
												{
													/* Need to track that service we care about is up */
													etaChannelManagementInfo->serviceDiscoveryInfo.ServiceState = serviceState;
												}
											}

											/* AcceptingRequests - Yes (1), No (0) */
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_ACCEPTING_REQS))
											{
												retval = rsslDecodeUInt(decodeIter, &acceptingRequests);
												if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
												{
													printf("Error %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
														rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
													return retval;
												}
												if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
													printf("\tReceived Source Directory Refresh for Decoded AcceptingRequests: " RTR_LLU "\n", acceptingRequests);
												else /*!< (4) Update Message */
													printf("\tReceived Source Directory Update for Decoded AcceptingRequests: " RTR_LLU "\n", acceptingRequests);

												/* if this is the serviceName that is requested by the application */
												if (serviceCount == foundServiceIndex)
												{
													/* Need to track that service we care about is accepting requests */
													etaChannelManagementInfo->serviceDiscoveryInfo.AcceptingRequests = acceptingRequests;
												}
											}

											/* Status */
											else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_STATUS))
											{
												retval = rsslDecodeState(decodeIter, &serviceStatus);
												if (retval != RSSL_RET_SUCCESS && retval != RSSL_RET_BLANK_DATA)
												{
													printf("<%s:%d> Error decoding rsslState.\n", __FILE__, __LINE__);
													return retval;
												}
												if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
													printf("\tReceived Source Directory Refresh for Decoded State: %d %d %d %.*s\n", serviceStatus.streamState,
														serviceStatus.dataState, serviceStatus.code, serviceStatus.text.length, serviceStatus.text.data);
												else /*!< (4) Update Message */
													printf("\tReceived Source Directory Update for Decoded State: %d %d %d %.*s\n", serviceStatus.streamState,
														serviceStatus.dataState, serviceStatus.code, serviceStatus.text.length, serviceStatus.text.data);

												rsslStateToString(&tempBuffer, &serviceStatus);
												printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
											}
										}
									}

								}
								break;
								case RDM_DIRECTORY_SERVICE_GROUP_ID: /*!< (3) Source Group Filter ID */
								{
									printf("\nDecoding Source Group Filter ID FilterListEntry\n");

									/* decode element list - third parameter is 0 because we do not have set definitions in this example */
									if ((retval = rsslDecodeElementList(decodeIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
									{
										/* decoding failure tends to be unrecoverable */
										printf("Error %s (%d) encountered with rsslDecodeElementList. Error Text: %s\n",
											rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
										return retval;
									}

									/* decode element list elements */
									while ((retval = rsslDecodeElementEntry(decodeIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
									{
										if (retval < RSSL_RET_SUCCESS)
										{
											/* decoding failure tends to be unrecoverable */
											printf("Error %s (%d) encountered with rsslDecodeElementEntry. Error Text: %s\n",
												rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
											return retval;
										}
										else
										{
											/* get service group information */

											/* Group */
											if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_GROUP))
											{
												if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
													printf("\tReceived Source Directory Refresh for Group: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);
												else /*!< (4) Update Message */
													printf("\tReceived Source Directory Update for Group: %.*s\n",elementEntry.encData.length, elementEntry.encData.data);
											}
										}
									}
								}
								break;
								case RDM_DIRECTORY_SERVICE_LOAD_ID: /*!< (4) Source Load Filter ID */
								{
									printf("\nDecoding Source Load Filter ID FilterListEntry not supported in this app\n");
								}
								break;
								case RDM_DIRECTORY_SERVICE_DATA_ID: /*!< (5) Source Data Filter ID */
								{
									printf("\nDecoding Source Data Filter ID FilterListEntry not supported in this app\n");
								}
								break;
								case RDM_DIRECTORY_SERVICE_LINK_ID: /*!< (6) Communication Link Filter ID */
								{
									printf("\nDecoding Communication Link Filter ID FilterListEntry not supported in this app\n");
								}
								break;
								default: /* Error handling */
								{
									printf("\nUnkonwn FilterListEntry filterID: %d\n", filterEntry.id);
									return RSSL_RET_FAILURE;
								}
							}
						}
					}
				}
				serviceCount++;
			}

			if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				pState = &msg->refreshMsg.state;
				/* need to reset tempBuffer length to large enough number if we are re-using tempBuffer */
				tempBuffer.length = 1024;
				rsslStateToString(&tempBuffer, pState);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
			}
		}
		break;
		case RSSL_MC_STATUS: /*!< (3) Status Message */
		{
			printf("\nReceived Source Directory StatusMsg\n");
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			pState = &msg->statusMsg.state;
				rsslStateToString(&tempBuffer, pState);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    		}
		}
		break;
		case RSSL_MC_CLOSE: /*!< (5) Close Message */
		{
			printf("\nReceived Source Directory Close\n");
		}
		break;
		default: /* Error handling */
		{
			printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
		}
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Processes a dictionary response. This consists of decoding the response.
 * etaChannelInfo - The channel management information including the dictionaries loaded information that is populated/updated
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 * dataDictionary - the dictionary used for decoding the field entry data
 */
RsslRet processDictionaryResponse(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary)
{
	RsslMsgKey* key = 0;
	RsslState *pState = 0;

	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	char tempData[1024];
	RsslBuffer tempBuffer;

	RDMDictionaryTypes dictionaryType = (RDMDictionaryTypes)0;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH: /*!< (2) Refresh Message */
		{
			/* decode dictionary response */

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			if (key)
			{
				printf("Received Dictionary Response: %.*s\n", key->name.length, key->name.data);
			}
			else
			{
				printf("Received Dictionary Response\n");
			}

			pState = &msg->refreshMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

			/* The dictionary response is typically broken into a multi-part message due to large size of the dictionary */
			if (((etaChannelManagementInfo->dictionariesLoadedInfo.fieldDictionaryFirstPart) && (msg->msgBase.streamId == FIELD_DICTIONARY_STREAM_ID))
				|| ((etaChannelManagementInfo->dictionariesLoadedInfo.enumTypeDictionaryFirstPart) && (msg->msgBase.streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)))
			{
				/* The first part of a dictionary refresh should contain information about its type.
				 * Save this information and use it as subsequent parts arrive. */

				/* Extracts the RDM Dictionary Type (RDMDictionaryTypes) information from an encoded dictionary. This can determine the
				 * specific dictionary decode function that should be used (e.g. for dictionary type of RDM_DICTIONARY_FIELD_DEFINITIONS,
				 * the user would next invoke the rsslDecodeFieldDictionary function).
				 * This is expected to be called after rsslDecodeMsg (where the RsslMsg.domainType is RSSL_DMT_DICTIONARY), but before
				 * decoding the RsslMsg.encDataBody payload.
				 */
				if (rsslExtractDictionaryType(decodeIter, &dictionaryType, &errorText) != RSSL_RET_SUCCESS)
    			{
    				printf("rsslGetDictionaryType() failed: %.*s\n", errorText.length, errorText.data);
    				return RSSL_RET_FAILURE;
    			}

				switch (dictionaryType)
				{
					case RDM_DICTIONARY_FIELD_DEFINITIONS:
					{
						/*!< (1) Field Dictionary type, typically referring to an RDMFieldDictionary */
						etaChannelManagementInfo->dictionariesLoadedInfo.fieldDictionaryFirstPart = RSSL_FALSE;
					}
					break;
					case RDM_DICTIONARY_ENUM_TABLES:
					{
						/*!< (2) Enumeration Dictionary type, typically referring to an enumtype.def */
						etaChannelManagementInfo->dictionariesLoadedInfo.enumTypeDictionaryFirstPart = RSSL_FALSE;
					}
					break;
					default: /* Error handling */
					{
						printf("Unknown dictionary type %llu from message on stream %d\n", (RsslUInt)dictionaryType, msg->msgBase.streamId);
						return RSSL_RET_FAILURE;
					}
				}
			}

			if (msg->msgBase.streamId == FIELD_DICTIONARY_STREAM_ID)
			{
				/* Adds data from a Field Dictionary message payload to the RsslDataDictionary */
    			if (rsslDecodeFieldDictionary(decodeIter, dataDictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
    			{
    				printf("Decoding Field Dictionary failed: %.*s\n", errorText.length, errorText.data);
    				return RSSL_RET_FAILURE;
    			}

				/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages,
				 * as well as the final message in a multi-part response message sequence.
				 */
				if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
				{
					etaChannelManagementInfo->dictionariesLoadedInfo.fieldDictionaryLoaded = RSSL_TRUE;
					if (!etaChannelManagementInfo->dictionariesLoadedInfo.enumTypeDictionaryLoaded)
						printf("Field Dictionary complete, waiting for Enum Table...\n");
					else
						printf("Field Dictionary complete.\n");
				}
			}
			else if (msg->msgBase.streamId == ENUM_TYPE_DICTIONARY_STREAM_ID)
			{
				/* Adds data from an Enumerated Types Dictionary message payload to the RsslDataDictionary */
    			if (rsslDecodeEnumTypeDictionary(decodeIter, dataDictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
    			{
    				printf("Decoding Enumerated Types Dictionary failed: %.*s\n", errorText.length, errorText.data);
    				return RSSL_RET_FAILURE;
    			}

				/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages,
				* as well as the final message in a multi-part response message sequence.
				*/
				if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
				{
					etaChannelManagementInfo->dictionariesLoadedInfo.enumTypeDictionaryLoaded = RSSL_TRUE;
					if (!etaChannelManagementInfo->dictionariesLoadedInfo.fieldDictionaryLoaded)
						printf("Enumerated Types Dictionary complete, waiting for Field Dictionary...\n");
					else
						printf("Enumerated Types Dictionary complete.\n");
				}
			}
			else
			{
				printf("Received unexpected dictionary message on stream %d\n", msg->msgBase.streamId);
				return RSSL_RET_FAILURE;
			}

			if (etaChannelManagementInfo->dictionariesLoadedInfo.fieldDictionaryLoaded && etaChannelManagementInfo->dictionariesLoadedInfo.enumTypeDictionaryLoaded)
			{
				printf("Dictionary ready, requesting Market Price item...\n\n");

				/* Ok to send Market Price item request now */

			}
		}
    	break;
		case RSSL_MC_STATUS: /*!< (3) Status Message */
		{
			printf("\nReceived StatusMsg for dictionary\n");
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			RsslState *pState = &msg->statusMsg.state;
				rsslStateToString(&tempBuffer, pState);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    		}
		}
		break;
		default: /* Error handling */
		{
			printf("\nReceived Unhandled Dictionary Msg Class: %d\n", msg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
		}
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Send a Dictionary request message to a channel. This consists of getting a message buffer, setting the dictionary
 * request information, encoding the dictionary request, and sending the dictionary request to the server. A Dictionary
 * request message is encoded and sent by OMM consumer applications. Some data requires the use of a dictionary for
 * encoding or decoding. This dictionary typically defines type and formatting information and directs the application
 * as to how to encode or decode specific pieces of information. Content that uses the RsslFieldList type requires the
 * use of a field dictionary (usually the LSEG RDMFieldDictionary, though it could also be a user-defined or
 * modified field dictionary).
 * etaChannelInfo - The channel management information including the channel to send the Dictionary request message buffer to and
 *					the obtained source directory service discovery information that is used for sending Dictionary Request
 * dictionaryName - The name of the dictionary to request
 */
RsslRet sendDictionaryRequest(EtaChannelManagementInfo *etaChannelManagementInfo, const char* dictionaryName)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Dictionary request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Dictionary request. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRequestMsg(&reqMsg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	reqMsg.msgBase.msgClass = RSSL_MC_REQUEST; /*!< (1) Request Message */
	reqMsg.msgBase.domainType = RSSL_DMT_DICTIONARY; /*!< (5) Dictionary Message */
	reqMsg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/
	reqMsg.flags = RSSL_RQMF_NONE; /*!< (0x000) No RsslRequestMsg flags are present */

	if (!strcmp(dictionaryName, dictionaryDownloadName))
	{
		reqMsg.msgBase.streamId = FIELD_DICTIONARY_STREAM_ID;
	}
	else if (!strcmp(dictionaryName, enumTableDownloadName))
	{
		reqMsg.msgBase.streamId = ENUM_TYPE_DICTIONARY_STREAM_ID;
	}
	else
	{
		printf("\nSend Dictionary Request with Unknown Dictionary Name: %s\n", dictionaryName);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set msgKey members */
	/*!< (0x0008) This RsslMsgKey has a filter, contained in \ref RsslMsgKey::filter.  */
	/*!< (0x0004) This RsslMsgKey has a nameType enumeration, contained in \ref RsslMsgKey::nameType.  */
	/*!< (0x0002) This RsslMsgKey has a name buffer, contained in \ref RsslMsgKey::name.  */
	/*!< (0x0001) This RsslMsgKey has a service id, contained in \ref RsslMsgKey::serviceId.  */
	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;

	/* msgKey.nameType is Not used for Dictionary Request Message Use, per ETA C RDM Usage Guide.
	 * So we probably don't need to do this.
	 */
	reqMsg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC; /*!< (1) Instrument Code */

	/* msgKey.name is Required. Specify a msgKey.flags value of RSSL_MKF_HAS_NAME. Populate msgKey.name with the name
	 * of the desired dictionary as seen in the Source Directory response.
	 */
	reqMsg.msgBase.msgKey.name.data = (char *)dictionaryName;
	reqMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(dictionaryName);

	/* msgKey.serviceId is Required. Set this to the serviceId of the service from which the consumer requests the dictionary. */
	reqMsg.msgBase.msgKey.serviceId = (RsslUInt16)etaChannelManagementInfo->serviceDiscoveryInfo.serviceId;

	/* msgKey.filter is Required. The filter represents the desired verbosity of the dictionary.
	 * The consumer should set the filter according to how much information is needed:
	 * - RDM_DICTIONARY_INFO == 0x00: Version information only
	 * - RDM_DICTIONARY_MINIMAL == 0x03: Provides information needed for caching
	 * - RDM_DICTIONARY_NORMAL == 0x07: Provides all information needed for decoding
	 * - RDM_DICTIONARY_VERBOSE == 0x0F: Provides all information(including comments)
	 * Providers are not required to support the MINIMAL and VERBOSE filters.
	 */
	reqMsg.msgBase.msgKey.filter = RDM_DICTIONARY_VERBOSE; /*!< (0x0F) "Verbose" Verbosity, e.g. all with description */

	/* encode message - populate message and encode it */
	if ((retval = rsslEncodeMsg(&encodeIter, (RsslMsg*)&reqMsg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send dictionary request */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Send just 1 Market Price item request message to a channel. This consists of getting a message buffer, encoding the
 * Market Price item request, and sending the item request to the server. A Market Price request message is encoded and
 * sent by OMM consumer applications. The request specifies the name and attributes of an item in which the consumer is interested.
 * etaChannelInfo - The channel management information including the channel to send the item request message buffer to and
 *					the obtained source directory service discovery information that is used for sending Market Price Item Request and
 *					the market price item information
 */
RsslRet sendMarketPriceItemRequest(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Populate and encode a requestMsg */
	RsslRequestMsg reqMsg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* In this simple example, we are sending just 1 Market Price item request as individual request message. */

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Market Price item request.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Market Price item request. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearRequestMsg(&reqMsg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

	/* set the buffer on an RsslEncodeIterator */
	if((retval = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* set-up message */
	reqMsg.msgBase.msgClass = RSSL_MC_REQUEST; /*!< (1) Request Message */
	reqMsg.msgBase.streamId = etaChannelManagementInfo->marketPriceItemInfo.streamId;
	reqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE; /*!< (6) Market Price Message */

	/* No Batch request - since we are only showing one item in the itemList, it is a waste of bandwidth to send a batch request */

	/* No View */
	reqMsg.msgBase.containerType = RSSL_DT_NO_DATA; /*!< (128) No Data <BR>*/

	/* RSSL_RQMF_STREAMING flag set. When the flag is set, the request is known as a "streaming" request, meaning
	 * that the refresh will be followed by updates.
	 */
	reqMsg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;

	/*!< (0x002) This RsslRequestMsg has priority information, contained in \ref RsslRequestMsg::priorityClass and
	 * \ref RsslRequestMsg::priorityCount. This is used to indicate the importance of this stream.
	 */
	reqMsg.priorityClass = 1;
	reqMsg.priorityCount = 1;

	/*!< (0x040) This RsslRequestMsg contains quality of service information, contained in RsslRequestMsg::qos.
	 * If only \ref RsslRequestMsg::qos is present, this is the QoS that will satisfy the request.
	 * If RsslRequestMsg::qos and RsslRequestMsg::worstQos are both present, this indicates that any QoS
	 * in that range will satisfy the request. '
	 */
	/* copy the QoS information */
	rsslCopyQos(&(reqMsg.qos), &(etaChannelManagementInfo->serviceDiscoveryInfo.QoS[0]));

	/* specify msgKey members */
	reqMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;

	/* msgKey.nameType Optional. When consuming from LSEG sources, typically set to
	 * RDM_INSTRUMENT_NAME_TYPE_RIC (the "Instrument Code"). If this is not specified,
	 * msgKey.nameType defaults to RDM_INSTRUMENT_NAME_TYPE_RIC.
	 */
	reqMsg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;

	/* msgKey.name Required in initial request, otherwise optional. The name of the requested item. */
	reqMsg.msgBase.msgKey.name.data = etaChannelManagementInfo->marketPriceItemInfo.itemName;
	reqMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(etaChannelManagementInfo->marketPriceItemInfo.itemName);

	/* msgKey.serviceId is Required. This should be the Id associated with the service from which the consumer
	 * wishes to request the item.
	 */
	reqMsg.msgBase.msgKey.serviceId = (RsslUInt16)etaChannelManagementInfo->serviceDiscoveryInfo.serviceId;

	/* encode message - populate message and encode it */
	if ((retval = rsslEncodeMsg(&encodeIter, (RsslMsg*)&reqMsg)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsg() failed with return code: %d\n", retval);
		/* Closes channel, cleans up and exits the application. */
		return retval;
	}

	/* set the buffer’s encoded content length prior to writing, this can be obtained from the iterator. */
	/* rsslGetEncodedBufferLength returns the size (in bytes) of content encoded with the RsslEncodeIterator.
	 * After encoding is complete, use this function to set RsslBuffer.length to the size of data contained
	 * in the buffer. This ensures that only the required bytes are written to the network.
	 */
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	/* send Market Price item request */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
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
 * Processes a market price response. This consists of extracting the key, printing out the item name contained in the key,
 * decoding the field list and field entry data.
 * etaChannelInfo - The channel management information including the dictionaries loaded information that is populated/updated
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 * marketPriceItemInfo - The market price item information for updates
 * dataDictionary - the dictionary used for decoding the field entry data
 */
RsslRet processMarketPriceItemResponse(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary)
{
	/* The RsslMsgKey contains a variety of attributes used to identify the contents flowing within a particular stream.
	 * This information, in conjunction with domainType and quality of service information, can be used to uniquely identify a
	 * data stream.
	 */
	RsslMsgKey* key = 0;

	RsslFieldList fieldList;

	RsslRet retval;

	char tempData[1024];
	RsslBuffer tempBuffer;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH: /*!< (2) Refresh Message */
		{
			printf("\nReceived Item Refresh Msg for stream %i \n", msg->refreshMsg.msgBase.streamId);

			/* update our item state list if its a refresh, then process just like update */
			etaChannelManagementInfo->marketPriceItemInfo.itemState.dataState = msg->refreshMsg.state.dataState;
			etaChannelManagementInfo->marketPriceItemInfo.itemState.streamState = msg->refreshMsg.state.streamState;

			/* refresh continued - process just like update */
		}

		case RSSL_MC_UPDATE: /*!< (4) Update Message */
		{
			/* decode market price response for both Refresh Msg and Update Msg */

			if (msg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				printf("\nReceived Item Update Msg for stream %i \n", msg->updateMsg.msgBase.streamId);

				/* When displaying update information, we should also display the updateType information. */
				/*!< @brief Indicates domain-specific information about the type of content contained in this update.
				 * See rsslRDM.h RDMUpdateEventTypes enum for domain-specific enumerations for usage with the 
				 * Domain Models.
				 */
				printf("UPDATE TYPE: %u\n", msg->updateMsg.updateType);
			}

			if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);
			}

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* print out item name from key if it has it */
			if (key)
			{
				printf("\n%.*s\nDOMAIN: %s\n", key->name.length, key->name.data, rsslDomainTypeToString(msg->msgBase.domainType));
			}
			else /* cached item name */
			{
				printf("\n%s\nDOMAIN: %s\n", etaChannelManagementInfo->marketPriceItemInfo.itemName, rsslDomainTypeToString(msg->msgBase.domainType));
			}

			/* decode into the MarketPrice Payload field list structure */
			if ((retval = decodeMarketPricePayload(&fieldList, decodeIter, dataDictionary)) != RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with decodeMarketPricePayload. Error Text: %s\n",
					rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
				return retval;
			}
		}
		break;

		case RSSL_MC_STATUS: /*!< (3) Status Message */
		{
			printf("\nReceived Item Status Msg for stream %i \n", msg->statusMsg.msgBase.streamId);

			/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in
			 * RsslStatusMsg::state.
			 */
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
    			rsslStateToString(&tempBuffer, &msg->statusMsg.state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

				/* Update our state table with the new state */
				etaChannelManagementInfo->marketPriceItemInfo.itemState.dataState = msg->statusMsg.state.dataState;
				etaChannelManagementInfo->marketPriceItemInfo.itemState.streamState = msg->statusMsg.state.streamState;
			}
		}
		break;

		case RSSL_MC_ACK:
		{
			printf("\nReceived Item Ack Msg for stream %i \n", msg->msgBase.streamId);

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* print out item name from key if it has it */
			if (key)
			{
				printf("\n%.*s\nDOMAIN: %s\n", key->name.length, key->name.data, rsslDomainTypeToString(msg->msgBase.domainType));
			}
			else /* cached item name */
			{
				printf("\n%s\nDOMAIN: %s\n", etaChannelManagementInfo->marketPriceItemInfo.itemName, rsslDomainTypeToString(msg->msgBase.domainType));
			}

			printf("\tackId=%u\n", msg->ackMsg.ackId);

			if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				printf("\tseqNum=%u\n", msg->ackMsg.seqNum);

			if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
				printf("\tnakCode=%u\n", msg->ackMsg.nakCode);

			if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				printf("\ttext=%.*s\n", msg->ackMsg.text.length, msg->ackMsg.text.data);
		}
		break;

		default: /* Error handling */
		{
			printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
		}
		break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes into the MarketPrice Payload field list structure. Inside the MarketPrice Payload field list structure,
 * decodes the field entry data in list and prints out the field entry data with help of the dictionary.
 * Returns success if decoding succeeds or failure if decoding fails.
 * fieldList - The field list data
 * decodeIter - The decode iterator
 * dataDictionary - the dictionary used for decoding the field entry data
 */
RsslRet decodeMarketPricePayload(RsslFieldList* fieldList, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary)
{
	RsslRet retval;

	RsslFieldEntry fieldEntry;

	/*!< (0) Unknown Data Type. This is only valid when decoding an RsslFieldEntry type that requires a dictionary look-up.
	 * If content is set defined, actual type enum will be present. <BR>
	 */
	RsslDataType dataType = RSSL_DT_UNKNOWN;

	RsslUInt64 fidUIntValue = 0;
	RsslInt64 fidIntValue = 0;
	RsslFloat tempFloat = 0;
	RsslDouble tempDouble = 0;
	RsslReal fidRealValue;
	RsslEnum fidEnumValue;
	RsslFloat fidFloatValue = 0;
	RsslDouble fidDoubleValue = 0;
	RsslQos fidQosValue;
	RsslDateTime fidDateTimeValue;
	RsslState fidStateValue;
	RsslBuffer fidBufferValue;
	RsslBuffer fidDateTimeBuf;
	RsslBuffer fidRealBuf;
	RsslBuffer fidStateBuf;
	RsslBuffer fidQosBuf;

	RsslDataDictionary* dictionary = dataDictionary;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* Prefer use of clear functions for initializations over using static initializers. Clears tend to be more performant than
	 * using static initializers. Although if you do choose to use static initializers instead, you don’t need to clear.
	 */
	rsslClearReal(&fidRealValue);
	rsslClearQos(&fidQosValue);


	/* decode into the field list structure */
	if ((retval = rsslDecodeFieldList(decodeIter, fieldList, 0)) == RSSL_RET_SUCCESS)
	{
		/* decode each field entry in list */
		while ((retval = rsslDecodeFieldEntry(decodeIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retval == RSSL_RET_SUCCESS)
			{
				/* decode each field entry info */
				/* look up type in field dictionary and call correct primitive decode function */

				/* get dictionary entry */
				if (!dictionary->isInitialized)
				{
					return RSSL_RET_FAILURE;
				}
				else
					dictionaryEntry = dictionary->entriesArray[fieldEntry.fieldId];

				/* return if no entry found */
				if (!dictionaryEntry)
				{
					printf("\tFid %d not found in dictionary\n", fieldEntry.fieldId);
					return RSSL_RET_FAILURE;
				}

				/* print out fid name */
				printf("\t%-20s", dictionaryEntry->acronym.data);
				/* decode and print out fid value */
				dataType = dictionaryEntry->rwfType;
				switch (dataType)
				{
					case RSSL_DT_UINT:
					{
						if ((retval = rsslDecodeUInt(decodeIter, &fidUIntValue)) == RSSL_RET_SUCCESS)
						{
							printf("" RTR_LLU "\n", fidUIntValue);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_INT:
					{
						if ((retval = rsslDecodeInt(decodeIter, &fidIntValue)) == RSSL_RET_SUCCESS)
						{
							printf(RTR_LLD "\n", fidIntValue);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeInt() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_FLOAT:
					{
						if ((retval = rsslDecodeFloat(decodeIter, &fidFloatValue)) == RSSL_RET_SUCCESS)
						{
							printf("%f\n", fidFloatValue);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeFloat() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_DOUBLE:
					{
						if ((retval = rsslDecodeDouble(decodeIter, &fidDoubleValue)) == RSSL_RET_SUCCESS)
						{
							printf("%f\n", fidDoubleValue);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeDouble() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_REAL:
					{
						if ((retval = rsslDecodeReal(decodeIter, &fidRealValue)) == RSSL_RET_SUCCESS)
						{
							fidRealBuf.data = (char*)alloca(35);
							fidRealBuf.length = 35;
							rsslRealToString(&fidRealBuf, &fidRealValue);
							printf("%s\n", fidRealBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeReal() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_ENUM:
					{
						if ((retval = rsslDecodeEnum(decodeIter, &fidEnumValue)) == RSSL_RET_SUCCESS)
						{
							RsslEnumType *pEnumType = getFieldEntryEnumType(dictionaryEntry, fidEnumValue);
							if (pEnumType)
    							printf("%.*s(%d)\n", pEnumType->display.length, pEnumType->display.data, fidEnumValue);
							else
    							printf("%d\n", fidEnumValue);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeEnum() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_DATE:
					{
						if ((retval = rsslDecodeDate(decodeIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
						{
							fidDateTimeBuf.data = (char*)alloca(30);
							fidDateTimeBuf.length = 30;
							rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
							printf("%s\n", fidDateTimeBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeDate() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_TIME:
					{
						if ((retval = rsslDecodeTime(decodeIter, &fidDateTimeValue.time)) == RSSL_RET_SUCCESS)
						{
							fidDateTimeBuf.data = (char*)alloca(30);
							fidDateTimeBuf.length = 30;
							rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_TIME, &fidDateTimeValue);
							printf("%s\n", fidDateTimeBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeTime() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_DATETIME:
					{
						if ((retval = rsslDecodeDateTime(decodeIter, &fidDateTimeValue)) == RSSL_RET_SUCCESS)
						{
							fidDateTimeBuf.data = (char*)alloca(50);
							fidDateTimeBuf.length = 50;
							rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATETIME, &fidDateTimeValue);
							printf("%s\n", fidDateTimeBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeDateTime() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_QOS:
					{
						if((retval = rsslDecodeQos(decodeIter, &fidQosValue)) == RSSL_RET_SUCCESS) {
							fidQosBuf.data = (char*)alloca(100);
							fidQosBuf.length = 100;
							rsslQosToString(&fidQosBuf, &fidQosValue);
							printf("%s\n", fidQosBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeQos() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					case RSSL_DT_STATE:
					{
						if((retval = rsslDecodeState(decodeIter, &fidStateValue)) == RSSL_RET_SUCCESS) {
							int stateBufLen = 80;
							if (fidStateValue.text.data)
								stateBufLen += fidStateValue.text.length;
							fidStateBuf.data = (char*)alloca(stateBufLen);
							fidStateBuf.length = stateBufLen;
							rsslStateToString(&fidStateBuf, &fidStateValue);
							printf("%.*s\n", fidStateBuf.length, fidStateBuf.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeState() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;

					/* For an example of array decoding, see ETA C Developers Guide and ETAC example codes */
					case RSSL_DT_ARRAY:
					break;
					case RSSL_DT_BUFFER:
					case RSSL_DT_ASCII_STRING:
					case RSSL_DT_UTF8_STRING:
					case RSSL_DT_RMTES_STRING:
					{
						if((retval = rsslDecodeBuffer(decodeIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
						{
							printf("%.*s\n", fidBufferValue.length, fidBufferValue.data);
						}
						else if (retval != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeBuffer() failed with return code: %d\n", retval);
							return retval;
						}
					}
					break;
					default: /* Error handling */
					{
						printf("Unsupported data type (%d) for fid value\n", dataType);
					}
					break;
				}
				if (retval == RSSL_RET_BLANK_DATA)
				{
					printf("<blank data>\n");
				}

			}
			else
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeFieldEntry. Error Text: %s\n",
					rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
				return retval;
			}
		}
	}
	else
	{
		/* decoding failure tends to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeFieldList. Error Text: %s\n",
			rsslRetCodeToString(retval), retval, rsslRetCodeInfo(retval));
		return retval;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Close the Market Price item stream. A Market Price item close message is encoded and sent by OMM consumer applications.
 * etaChannelInfo - The channel management information including the channel to send the Market Price item close message buffer to
 */
RsslRet closeMarketPriceItemStream(EtaChannelManagementInfo *etaChannelManagementInfo)
{
	RsslRet retval;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* Consumer uses RsslCloseMsg to indicate no further interest in an item stream and to close the stream. */
	RsslCloseMsg msg;

	/* ETA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, LSEG recommends that
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for the Market Price item close.
	 * When the RsslBuffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
	 * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
	 * This ensures that only the required bytes are written to the network.
	 */
	/* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
	if ((msgBuf = etaGetBuffer(etaChannelManagementInfo->etaChannel, etaChannelManagementInfo->etaChannelInfo.maxFragmentSize, &error)) == NULL) /* first check Error */
	{
		/* Connection should be closed, return failure */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}

	/* if a buffer is returned, we can populate and write, encode an RsslMsg into the buffer */

	/* Encodes the Market Price item close. */

	/* On larger structures, like messages, the clear functions tend to outperform the static initializer. It is recommended to use
	 * the clear function when initializing any messages.
	 */
	rsslClearCloseMsg(&msg);

	/* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
	rsslClearEncodeIterator(&encodeIter);

	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encodeIter, etaChannelManagementInfo->etaChannel->majorVersion, etaChannelManagementInfo->etaChannel->minorVersion);

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
	msg.msgBase.streamId = etaChannelManagementInfo->marketPriceItemInfo.streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE; /*!< (6) Market Price Message */
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

	/* send Market Price item close */
	if ((retval = sendMessage(etaChannelManagementInfo->etaChannel, msgBuf)) < RSSL_RET_SUCCESS)
	{
		/* Market Price item close fails */
		/* Closes channel, cleans up and exits the application. */
		return RSSL_RET_FAILURE;
	}
	else if (retval > RSSL_RET_SUCCESS)
	{
		/* There is still data left to flush */

		/* If the Market Price item close doesn't flush, just close channel and exit the app. When you close Market Price
		 * item request stream, we want to make a best effort to get this across the network as it will gracefully close
		 * the open Market Price item stream. If this cannot be flushed, this application will just close the connection
		 * for simplicity.
		 */

		/* Closes channel, cleans up and exits the application. */
	}

	return retval;
}

