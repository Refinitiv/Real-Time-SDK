/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the ETA Consumer Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Consumer using the ETA Transport layer.
 *
 * Main h header file for the ETA Consumer Training application. It is a 
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
 * with which to interact). An OMM consumer typically creates an outbound 
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
 * and receive data.
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
 */

#ifndef _ETA_CONSUMER_TRAINING_H
#define _ETA_CONSUMER_TRAINING_H

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#define LOGIN_STREAM_ID 1

/* ping management information */
typedef struct {
	RsslUInt32	pingTimeoutServer; /* server ping timeout */
	RsslUInt32	pingTimeoutClient; /* client ping timeout */
	time_t		nextReceivePingTime; /* time client should receive next message/ping from server */
	time_t		nextSendPingTime; /* time to send next ping from client */
	time_t		currentTime;	/* current time from system clock */
	RsslBool	receivedServerMsg; /* flag for server message received */
} EtaPingManagementInfo;

/* channel management information */
typedef struct {
	RsslChannel* etaChannel;
	RsslChannelInfo etaChannelInfo; /* ETA Channel Info returned by rsslGetChannelInfo call */
	EtaPingManagementInfo pingManagementInfo;
} EtaChannelManagementInfo;

/*
 * Closes channel, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* etaChannel, int code);

/* 
 * Initializes the ping times for etaChannelManagementInfo.etaChannel. 
 * etaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo);

/* 
 * Processing ping management handler for etaChannelManagementInfo.etaChannel. 
 * etaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(EtaChannelManagementInfo *etaChannelManagementInfo);

/*
 * Sends a message buffer to a channel.  
 * etaChannel - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* etaChannel, RsslBuffer* msgBuf);

/* 
 * Send Login request message to a channel. This consists of getting a message buffer, setting the login request 
 * information, encoding the login request, and sending the login request to the server. A Login request message is 
 * encoded and sent by OMM consumer and OMM non-interactive provider applications. This message registers a user 
 * with the system. After receiving a successful Login response, applications can then begin consuming or providing 
 * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
 * etaChannelInfo - The channel management information including the channel to send the Login request message buffer to
 */
RsslRet sendLoginRequest(EtaChannelManagementInfo *etaChannelManagementInfo);

/*
 * Processes a login response. This consists of decoding the response.
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginResponse(RsslMsg* msg, RsslDecodeIterator* decodeIter);

/*
 * Close the Login stream. Note that closing Login stream will automatically close all other streams at the provider. 
 * A Login close message is encoded and sent by OMM consumer applications. This message allows a consumer to log out 
 * of the system. Closing a Login stream is equivalent to a 'Close All' type of message, where all open streams are 
 * closed (thus all other streams associated with the user are closed).
 * etaChannelInfo - The channel management information including the channel to send the Login close message buffer to
 */
RsslRet closeLoginStream(EtaChannelManagementInfo *etaChannelManagementInfo);

/* 
 * etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplifies the example codes and make the codes more readable.
 */
RsslBuffer* etaGetBuffer(RsslChannel *etaChannel, RsslUInt32 size, RsslError *rsslError);

#ifdef __cplusplus
};
#endif

#endif


