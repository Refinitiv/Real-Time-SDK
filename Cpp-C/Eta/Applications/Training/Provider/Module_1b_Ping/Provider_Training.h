/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
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

/* channel management information */
typedef struct {
	RsslChannel* etaChannel;
	RsslChannelInfo etaChannelInfo; /* ETA Channel Info returned by rsslGetChannelInfo call */
	EtaPingManagementInfo pingManagementInfo;
} EtaChannelManagementInfo;

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

#ifdef __cplusplus
};
#endif

#endif

