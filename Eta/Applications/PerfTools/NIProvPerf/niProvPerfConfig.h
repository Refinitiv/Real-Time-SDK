/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* niProvPerfConfig.h
 * Configures the upacNIProvPerf application. */

#ifndef _NI_PROV_PERF_CONFIG_H
#define _NI_PROV_PERF_CONFIG_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Provides configuration options for the provider. */
typedef struct 
{
	RsslUInt32	runTime;							/* Time application runs befor exiting(-runTime).*/

	RsslConnectionTypes	connectionType;				/* Type of connection(-connType).*/
	char 				portNo[32];					/* Port number(-p).*/
	char				interfaceName[128];			/* Name of interface(-if).*/
	char				username[128];				/* Username used when logging in. */
	RsslBool			tcpNoDelay;					/* Enable/Disable Nagle's algorithm(-tcpDelay).*/
	RsslUInt32			guaranteedOutputBuffers;	/* Guaranteed Output Buffers(-outputBufs).*/
	RsslUInt32			sendBufSize;				/* System Send Buffer Size(-sendBufSize) */
	RsslUInt32			recvBufSize;				/* System Send Buffer Size(-recvBufSize) */
	RsslUInt32			highWaterMark;				/* sets the point which will cause UPA to automatically flush */
	char				summaryFilename[128];		/* Name of the summary log file(-summaryFile). */
	char				statsFilename[128];			/* Name of the statistics log file(-statsFile). */
	RsslUInt32			writeStatsInterval;			/* Controls how often statistics are written. */
	RsslBool			displayStats;				/* Controls whether stats appear on the screen. */

	char				hostName[128];				/* Name of host to connect to(-hostname).*/
	char				sendAddr[128];				/* Outbound address, if using a multicast connection(-sa).*/
	char				recvAddr[128];				/* Inbound address, if using a multicast connection(-ra).*/
	char				sendPort[32];				/* Outbound port, if using a multicast connection(-sp).*/
	char				recvPort[32];				/* Inbound port, if using a multicast connection(-rp).*/
	char				unicastPort[32];			/* Unicast port, if using a mulicast connection(-up).*/
	RsslBool			sAddr;						/* Whether an outbound address was specified(-sa).*/
	RsslBool			rAddr;						/* Whether an inbound address was specified(-ra).*/

	RsslInt32			itemPublishCount;			/* Number of items to publish noninteractively(-itemCount).*/
	RsslInt32			commonItemCount;			/* Number of items common to all providers, if using multiple connections. */
} NIProvPerfConfig;

/* Contains the global application configuration */
extern NIProvPerfConfig niProvPerfConfig;

/* Parses command-line arguments to fill in the application's configuration structures. */
void initNIProvPerfConfig(int argc, char **argv);

/* Prints out the configuration. */
void printNIProvPerfConfig(FILE *file);

/* Exits the application and prints out usage information. */
void exitWithUsage();

#ifdef __cplusplus
};
#endif

#endif
