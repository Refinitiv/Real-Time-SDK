/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
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

int main(int argc, char **argv)
{
	/* For this simple training app, only a single channel/connection is used for the entire life of this app. */
	RsslChannel *upaChannel;

	char srvrHostname[128], srvrPortNo[128], interfaceName[128];

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
	 * Since select() modifies its file descriptor sets, if the call is being used in a loop, then the fd sets must 
	 * be reinitialized before each call. Since they act as input/output parameters for the select() system call; 
	 * they are read by and modified by the system call. When select() returns, the values have all been modified 
	 * to reflect the set of file descriptors ready. So, every time before you call select(), you have to 
	 * (re)initialize the fd_set values. Here we maintain 2 sets FD sets: 
	 * a) clean FD sets so that we can repeatedly call select call 
	 * b) dirty FD sets used in the actual select call (I/O notification mechanism)
	 * Typically, you reset the dirty FD sets to be equal to the clean FD sets before you call select().
	 */

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

	time_t currentTime = 0;
	time_t upaRuntime = 0;

	/* UPA provides clear functions for its structures (e.g., rsslClearEncodeIterator) as well as static initializers 
	 * (e.g., RSSL_INIT_ENCODE_ITERATOR). These functions are tuned to be efficient and avoid initializing unnecessary
	 * structure members, and allow for optimal structure use and reuse. In general, Thomson Reuters recommends that 
	 * you use the clear functions over static initializers, because the clear functions are more efficient.
	 */
	/* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslEncodeIterator encodeIter; /* the encode iterator is created (typically stack allocated)  */

	/* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
	RsslDecodeIterator decodeIter; /* the decode iterator is created (typically stack allocated)  */

	/* the default option parameters */
	/* connect to server running on same machine */
	snprintf(srvrHostname, 128, "%s", "localhost");
	/* server is running on port number 14003 */
	snprintf(srvrPortNo, 128, "%s", "14003");	
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

	/* Set the runtime of the UPA NI Provider application to be 300 (seconds) */
	upaRuntime = currentTime + (time_t)300;

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
	
	printf("\nChannel IPC descriptor = %d\n", upaChannel->socketId);

	/* Main loop for getting connection active and successful completion of the initialization process 
	 * The loop calls select() to wait for notification 
	 * Currently, the main loop would exit if an error condition is triggered or
	 * RsslChannel.state transitions to RSSL_CH_STATE_ACTIVE. 
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
							printf("Error %s (%d) (errno: %d) encountered with rsslInitChannel fd=%d. Error Text: %s\n", 
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
										printf("\nChannel In Progress - New FD: %d  Old FD: %d\n",upaChannel->socketId, inProgInfo.oldSocket );

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
										printf("\nChannel %d In Progress...\n", upaChannel->socketId);
									}
								}
								break;
								/* channel connection becomes active! 
								 * Once a connection is established and transitions to the RSSL_CH_STATE_ACTIVE state, 
								 * this RsslChannel can be used for other transport operations.
								 */
								case RSSL_RET_SUCCESS:			
								{
									printf("\nChannel on fd %d is now active - reading and writing can begin.\n", upaChannel->socketId);					

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

									printf( "Channel %d active. Channel Info:\n"
										"	Max Fragment Size: %u\n"
										"	Output Buffers: %u Max, %u Guaranteed\n"
										"	Input Buffers: %u\n"
										"	Send/Recv Buffer Sizes: %u/%u\n"
										"	Ping Timeout: %u\n"
										"	Connected component version: ", 
										upaChannel->socketId, /*!< @brief Socket ID of this RSSL channel. */
										channelInfo.maxFragmentSize, /*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */ 
										channelInfo.maxOutputBuffers, /*!< @brief This is the maximum number of output buffers available to the channel. */
										channelInfo.guaranteedOutputBuffers, /*!< @brief This is the guaranteed number of output buffers available to the channel. */
										channelInfo.numInputBuffers, /*!< @brief This is the number of input buffers available to the channel. */
										channelInfo.sysSendBufSize, /*!< @brief This is the systems Send Buffer size. This reports the systems send buffer size 
																	respective to the transport type being used (TCP, UDP, etc) */
										channelInfo.sysRecvBufSize, /*!< @brief This is the systems Receive Buffer size. This reports the systems receive buffer 
																	size respective to the transport type being used (TCP, UDP, etc) */
										channelInfo.pingTimeout /*!< @brief This is the value of the negotiated ping timeout */
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
									printf("\nBad return value fd=%d <%s>\n",
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
							printf("\nrsslDecodeMsg(): Error %d on SessionData fd=%d  Size %d \n", retval, upaChannel->socketId, msgBuf->length);
							/* Closes channel, cleans up and exits the application. */
							closeChannelCleanUpAndExit(upaChannel, RSSL_RET_FAILURE);
						}

						switch ( msg.msgBase.domainType )
						{
							/*!< (1) Login Message */
							case RSSL_DMT_LOGIN:
							{
								
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
								printf("\nrsslRead() FD Change - Old FD: %d New FD: %d\n", upaChannel->oldSocketId, upaChannel->socketId);
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
									printf("Error %s (%d) (errno: %d) encountered with rsslRead fd=%d. Error Text: %s\n", 
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
					printf("\nError %s (%d) (errno: %d) encountered with rsslPing() on fd=%d with code %d\n. Error Text: %s\n", 
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

