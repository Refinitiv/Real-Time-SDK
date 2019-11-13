/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the UPA Consumer Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Consumer using the UPA Transport layer.
 *
 * Main h header file for the UPA Consumer Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA Consumer Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * In this module, the application initializes the UPA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * the Enterprise Platform, Data Feed Direct, and Elektron.
 *
 * Detailed Descriptions:
 * The first step of any UPA consumer application is to establish a 
 * network connection with its peer component (i.e., another application 
 * with which to interact). An OMM consumer typically creates an outbound 
 * connection to the well-known hostname and port of a server (Interactive 
 * Provider or ADS). The consumer uses the rsslConnect() function to initiate 
 * the connection and then uses the rsslInitChannel() function to complete 
 * channel initialization.
 *
 */

#ifndef _TR_UPA_CONSUMER_TRAINING_H
#define _TR_UPA_CONSUMER_TRAINING_H

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

/* channel management information */
typedef struct {
	RsslChannel* upaChannel;
	RsslChannelInfo upaChannelInfo; /* UPA Channel Info returned by rsslGetChannelInfo call */
} UpaChannelManagementInfo;

/*
 * Closes channel, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* upaChannel, int code);

#ifdef __cplusplus
};
#endif

#endif


