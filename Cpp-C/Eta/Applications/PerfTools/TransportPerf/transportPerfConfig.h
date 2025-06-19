/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* transportPerfConfig.h
 * Configures the TransportPerf application. */

#ifndef _TRANSPORT_PERF_CONFIG_H
#define _TRANSPORT_PERF_CONFIG_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslTransport.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	APPTYPE_SERVER 		= 1,
	APPTYPE_CLIENT 		= 2
} ApplicationType;

/* Provides configuration for the TransportPerf application. */
typedef struct 
{
	RsslUInt32	runTime;							/* Time application runs befor exiting.  See -runTime */
	RsslInt32	threadCount;						/* Number of threads that handle connections.  See -threads */
	RsslInt32	*threadBindList;					/* CPU ID list for threads that handle connections.  See -threads */
	ApplicationType appType;						/* Time of application(server, client). */
	RsslBool reflectMsgs;							/* Reflect received messages instead of generating them. */

	RsslBool busyRead;								/* If set, the application will continually read
													 * rather than using notification.
													 * Messages cannot be sent in this mode. */

	RsslConnectionTypes	connectionType;				/* Type of connection. See -connType */
	RsslConnectionTypes encryptedConnectionType;		/* Encrypted connection type if the connectionType is ENCRYPTED*/
	char 				portNo[32];					/* Port number. See -p */
	char				interfaceName[128];			/* Name of interface.  See -if */
	RsslBool			tcpNoDelay;					/* Enable/Disable Nagle's algorithm. See -tcpDelay */
	RsslUInt32			guaranteedOutputBuffers;	/* Guaranteed Output Buffers. See -outputBufs */
	RsslUInt32			maxFragmentSize;			/* Maximum Fragment Size. See -maxFragmentSize */
	RsslUInt32			sendBufSize;				/* System Send Buffer Size(-sendBufSize) */
	RsslUInt32			recvBufSize;				/* System Send Buffer Size(-recvBufSize) */
	RsslUInt32			highWaterMark;				/* The "high water mark" (bytes) at which rsslWrite() will automatically flush. 
													   See rsslIoctl() and RSSL_HIGH_WATER_MARK. */
	char				summaryFilename[128];		/* Name of the summary log file(-summaryFile).. */
	RsslUInt32			writeStatsInterval;			/* Controls how often statistics are written. */
	RsslBool			displayStats;				/* Controls whether stats appear on the screen. */

	RsslCompTypes		compressionType;			/* Type of compression to use, if any. */
	int					compressionLevel;			/* Compression level, optional depending on compression algorithm used */
	char				hostName[128];				/* hostName, if using rsslConnect(). See -hostname */
	char				sendAddr[128];				/* Outbound address, if using a multicast connection. See -sa */
	char				recvAddr[128];				/* Inbound address, if using a multicast connection. See -ra */
	char				sendPort[32];				/* Outbound port, if using a multicast connection. See -sp */
	char				recvPort[32];				/* Inbound port, if using a multicast connection. See -rp */
	char				unicastPort[32];			/* Unicast port, if using a mulicast connection.  See -up */
	RsslBool			sAddr;						/* Whether an outbound address was specified. See -sa */
	RsslBool			rAddr;						/* Whether an inbound address was specified. See -ra */
	RsslBool			takeMCastStats;				/* Running a multicast connection and we want stats. */

	char				caStore[255];
	char				serverCert[255];
	char				serverKey[255];
	char				protocolList[128];			/* List of desired or supported sub-protocols for respective client or server websocket connection.*/

} TransportPerfConfig;

/* Contains the global application configuration */
extern TransportPerfConfig transportPerfConfig;

/* Parses command-line arguments to fill in the application's configuration structures. */
void initTransportPerfConfig(int argc, char **argv);

/* Prints out the configuration. */
void printTransportPerfConfig(FILE *file);

/* Prints out the compressionType value. */
const char *compressionTypeToString(RsslCompTypes compType);

/* Exits the application and prints out usage information. */
void exitWithUsage();

#ifdef __cplusplus
};
#endif

#endif

