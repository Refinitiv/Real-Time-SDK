/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

/*
 * This is the ETA Interactive Provider Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM Interactive Provider using the ETA Transport layer.
 *
 * Main h header file for the ETA Interactive Provider Training application. It is a 
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
 * can easily connect. The provider uses the rsslBind function to open the port 
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
 */

#ifndef _ETA_Provider_TRAINING_H
#define _ETA_Provider_TRAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

/* We set the Update Rate Interval to be 1 second for Interactive Provider application, which 
 * is the Update Interval the provider application sends the Update Message content to client
 */
#define UPDATE_INTERVAL 1

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
	RsslUInt32	pingTimeoutClient; /* client ping timeout */
	RsslUInt32	pingTimeoutServer; /* server ping timeout */
	time_t		nextReceivePingTime; /* time server should receive next message/ping from client */
	time_t		nextSendPingTime; /* time to send next ping from server */
	time_t		currentTime;	/* current time from system clock */
	RsslBool	receivedClientMsg; /* flag for client message received */
} EtaPingManagementInfo;

/* login request information */
typedef struct {
	RsslInt32	StreamId;
	char		Username[128];
	char		ApplicationId[128];
	char		ApplicationName[128];
	char		Position[128];
	char		Password[128];
	char		InstanceId[128];
	RsslUInt64	Role;
	RsslBool	IsInUse;
} EtaLoginRequestInfo;

/* channel management information */
typedef struct {
	RsslChannel* etaChannel;
	RsslChannelInfo etaChannelInfo; /* ETA Channel Info returned by rsslGetChannelInfo call */
	EtaPingManagementInfo pingManagementInfo;
	EtaLoginRequestInfo loginRequestInfo;
} EtaChannelManagementInfo;

/* reasons a login request is rejected */
typedef enum {
	MAX_LOGIN_REQUESTS_REACHED	= 0,
	NO_USER_NAME_IN_REQUEST		= 1
} EtaLoginRejectReason;

/*
 * Closes channel, closes server, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * etaSrvr - The RsslServer that represents the listening socket connection to the user to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelServerCleanUpAndExit(RsslChannel* etaChannel, RsslServer* etaSrvr, int code);

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
 * Processes a login request. This consists of decoding the login request and calling
 * sendLoginResponse() to send the login response.
 * etaChannelInfo - The channel management information including the login request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginRequest(EtaChannelManagementInfo *etaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter);

/*
 * Sends a Login refresh response to a channel. This consists of getting a message buffer, setting the login response information, 
 * encoding the login response, and sending the login response to the client. If the Interactive Provider grants access, it should 
 * send an RsslRefreshMsg to convey that the user successfully connected. This message should indicate the feature set supported by 
 * the provider application.
 * etaChannelInfo - The channel management information including the login request information and 
 * including the channel to send a login refresh response to
 */
RsslRet sendLoginResponse(EtaChannelManagementInfo *etaChannelManagementInfo);

/*
 * Sends the login request reject status message for a channel.
 * etaChannelInfo - The channel management information including the login request information and 
 * including the channel to send the login request reject status message to
 * streamId - The stream id of the login request reject status
 * reason - The reason for the reject
 */
RsslRet sendLoginRequestRejectStatusMsg(EtaChannelManagementInfo *etaChannelManagementInfo, RsslInt32 streamId, EtaLoginRejectReason reason);

/* 
 * Closes a login stream. 
 * streamId - The stream id to close the login for
 * etaChannelInfo - The channel management information including the login request information
 */
void closeLoginStream(RsslInt32 streamId, EtaChannelManagementInfo *etaChannelManagementInfo);

/*
 * Clears the login request information.
 * loginRequestInfo - The login request information to be cleared
 */
RTR_C_INLINE void clearLoginReqInfo(EtaLoginRequestInfo* loginRequestInfo)
{
	loginRequestInfo->StreamId = 0;
	loginRequestInfo->Username[0] = '\0';
	loginRequestInfo->ApplicationId[0] = '\0';
	loginRequestInfo->ApplicationName[0] = '\0';
	loginRequestInfo->Position[0] = '\0';
	loginRequestInfo->InstanceId[0] = '\0';
	loginRequestInfo->Password[0] = '\0';
	loginRequestInfo->Role = 0;
	loginRequestInfo->IsInUse = RSSL_FALSE;
}

/* 
 * etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplies the example codes and make the codes more readable.
 */
RsslBuffer* etaGetBuffer(RsslChannel *etaChannel, RsslUInt32 size, RsslError *rsslError);

#ifdef __cplusplus
};
#endif

#endif

