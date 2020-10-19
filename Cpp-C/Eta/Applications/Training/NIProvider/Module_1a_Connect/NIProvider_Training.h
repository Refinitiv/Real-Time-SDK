/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the ETA NI Provider Training series of the ETA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a ETA OMM NI Provider using the ETA Transport layer.
 *
 * Main h header file for the ETA NI Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * ETA NI Provider Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * A Non-Interactive Provider (NIP) writes a provider application that 
 * connects to Refinitiv Real-Time Distribution System and sends a specific
 * set (non-interactive) of information (services, domains, and capabilities).
 * NIPs act like clients in a client-server relationship. Multiple NIPs can
 * connect to the same Refinitiv Real-Time Distribution System and publish
 * the same items and content.
 * 
 * In this module, the OMM NIP application initializes the ETA Transport 
 * and establish a connection to an ADH server. Once connected, an OMM NIP 
 * can publish information into the ADH cache without needing to handle 
 * requests for the information. The ADH can cache the information and 
 * along with other Refinitiv Real-Time Distribution System components, 
 * provide the information to any NIProvider applications that indicate interest.
 *
 * Detailed Descriptions:
 * The first step of any ETA NIP application is to establish network 
 * communication with an ADH server. To do so, the OMM NIP typically creates 
 * an outbound connection to the well-known hostname and port of an ADH. 
 * The consumer uses the rsslConnect function to initiate the connection 
 * process and then performs connection initialization processes as needed.
 * 
 */

#ifndef _ETA_NI_Provider_TRAINING_H
#define _ETA_NI_Provider_TRAINING_H

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

/*
 * Closes channel, cleans up and exits the application.
 * etaChannel - The channel to be closed
 * code - if exit due to errors/exceptions
 */
void closeChannelCleanUpAndExit(RsslChannel* etaChannel, int code);

#ifdef __cplusplus
};
#endif

#endif


