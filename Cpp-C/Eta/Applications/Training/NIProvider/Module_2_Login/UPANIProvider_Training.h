/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the UPA NI Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM NI Provider using the UPA Transport layer.
 *
 * Main h header file for the UPA NI Provider Training application. It is a 
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
 * the expected time frame, the connection can be terminated. Refinitiv 
 * recommends sending ping messages at intervals one-third the 
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
 */

#ifndef _TR_UPA_NI_Provider_TRAINING_H
#define _TR_UPA_NI_Provider_TRAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

/* We set the Update Rate Interval to be 1 second for NIP application, which is the Update Interval
 * the NIP application pushes the Update Mssage content to ADH
 */
#define UPDATE_INTERVAL 1

#define LOGIN_STREAM_ID 1

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

/* ping management information */
typedef struct {
	RsslUInt32	pingTimeoutServer; /* server ping timeout */
	RsslUInt32	pingTimeoutClient; /* client ping timeout */
	time_t		nextReceivePingTime; /* time client should receive next message/ping from server */
	time_t		nextSendPingTime; /* time to send next ping from client */
	time_t		currentTime;	/* current time from system clock */
	RsslBool	receivedServerMsg; /* flag for server message received */
} UpaPingManagementInfo;

/*
 * Closes channel, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* upaChannel, int code);

/* 
 * Initializes the ping times for upaChannel. 
 * upaChannel - The channel for ping management info initialization
 * pingManagementInfo - The ping management information that is used
 */
void initPingManagementHandler(RsslChannel* upaChannel, UpaPingManagementInfo* pingManagementInfo);

/* 
 * Processing ping management handler 
 * upaChannel - The channel for ping management processing
 * pingManagementInfo - The ping management information that is used
 */
RsslRet processPingManagementHandler(RsslChannel* upaChannel, UpaPingManagementInfo* pingManagementInfo);

/*
 * Sends a message buffer to a channel.  
 * upaChannel - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* upaChannel, RsslBuffer* msgBuf);

/* 
 * Send Login request message to a channel. This consists of getting a message buffer, setting the login request 
 * information, encoding the login request, and sending the login request to the server. A Login request message is 
 * encoded and sent by OMM NI Provider and OMM non-interactive provider applications. This message registers a user 
 * with the system. After receiving a successful Login response, applications can then begin consuming or providing 
 * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
 * upaChannel - The channel to send the Login request message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 */
RsslRet sendLoginRequest(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter);

/*
 * Processes a login response. This consists of decoding the response.
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginResponse(RsslMsg* msg, RsslDecodeIterator* decodeIter);

/*
 * Close the Login stream. Note that closing Login stream will automatically close all other streams at the provider. 
 * A Login close message is encoded and sent by OMM NI Provider applications. This message allows a NI Provider to log out 
 * of the system. Closing a Login stream is equivalent to a 'Close All' type of message, where all open streams are 
 * closed (thus all other streams associated with the user are closed).
 * upaChannel - The channel to send the Login close message buffer to
 * maxMsgSize - the requested size of the buffer for rsslGetBuffer function to obtain from the guaranteed/shared buffer pool.
 * encodeIter - The encode iterator
 */
RsslRet closeLoginStream(RsslChannel* upaChannel, RsslUInt32 maxMsgSize, RsslEncodeIterator* encodeIter);

/* 
 * upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplies the example codes and make the codes more readable.
 */
RsslBuffer* upaGetBuffer(RsslChannel *upaChannel, RsslUInt32 size, RsslError *rsslError);

#ifdef __cplusplus
};
#endif

#endif


