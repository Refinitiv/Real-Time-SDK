/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* provPerfConfig.h
 * Configures the upacProvPerf application. */

#ifndef _PROV_PERF_CONFIG_H
#define _PROV_PERF_CONFIG_H

#include "providerThreads.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Provides configuration options for the provider. */
typedef struct 
{
	RsslUInt32	runTime;							/* Time application runs befor exiting(-runTime) */
	char 				portNo[32];					/* Port number. See -p */
	char				interfaceName[128];			/* Name of interface. See -if */
	RsslBool			tcpNoDelay;					/* Enable/Disable Nagle's algorithm. See -tcpDelay */
	RsslUInt32			guaranteedOutputBuffers;	/* Guaranteed Output Buffers. See -outputBufs */
	RsslUInt32			maxOutputBuffers;		/* Max Output Buffers. See -maxOutputBufs */
	RsslUInt32			maxFragmentSize;			/* Maximum Fragment Size. See -maxFragmentSize */
	RsslUInt32			highWaterMark;				/* sets the point which will cause UPA to automatically flush */
	RsslUInt32			sendBufSize;				/* System Send Buffer Size. See -sendBufSize */
	RsslUInt32			recvBufSize;				/* System Send Buffer Size. See -recvBufSize */
	char				summaryFilename[128];		/* Name of the summary log file. See -summaryFile */
	RsslUInt32			writeStatsInterval;			/* Controls how often statistics are written. */
	RsslBool			displayStats;				/* Controls whether stats appear on the screen. */
	RsslBool			useReactor;					/* Use the VA Reactor instead of the UPA Channel for sending and receiving. */
	RsslConnectionTypes connType;					/* Connection type for this provider */
	char				serverCert[255];			/* Server certificate file location */
	char				serverKey[255];				/* Server private key file location */
	char				cipherSuite[255];			/* Server cipher suite */
} ProvPerfConfig;

/* Contains the global application configuration */
extern ProvPerfConfig provPerfConfig;

/* Parses command-line arguments to fill in the application's configuration structures. */
void initProvPerfConfig(int argc, char **argv);

/* Prints out the configuration. */
void printProvPerfConfig(FILE *file);

/* Exits the application and prints out usage information. */
void exitWithUsage();

#ifdef __cplusplus
};
#endif

#endif
