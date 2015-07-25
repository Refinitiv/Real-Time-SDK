/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef EDF_EXAMPLE_CONFIG_H
#define EDF_EXAMPLE_CONFIG_H

#include "rtr/rsslTypes.h"
#include "itemList.h"

/* Handles configuration of the EDF consumer example application. */

typedef struct
{
	char realTimeDataAddr[128];		/* Address for the realtime feed connection. */
	char realTimeDataPort[128];		/* Port for the realtime feed connection. */
	char realTimeDataIntf[128];		/* Network interface address for the realtime feed connection. */
	RsslBool hasRealTimeInterface;		/* Indicates if network interface for realtime feed was
										 * specified. */
	char snapshotServerAddr[128];		/* Address for the snapshot server connection. */
	char snapshotServerPort[128];		/* Port for the snapshot server connection. */
	char gapRequestServerAddr[128];		/* Address for the gap request server connection. */
	char gapRequestServerPort[128];		/* Port for the gap request server connection. */
	char gapFillServerAddr[128];		/* Address for the gap fill server connection. */
	char gapFillServerPort[128];		/* Port for the gap fill server connection. */
	char gapFillIntf[128];			/* Network interface address for the realtime feed connection. */
	RsslBool hasGapFillInterface;		/* Indicates if network interface for gap fill feed was 
										 * specified. */
	char refDataServerAddr[128];		/* Address for the ref data server connection. */
	char refDataServerPort[128];		/* Port for the ref data server connection. */
	char setDefFileName[128];		/* File containing global set definitions. */
	char setDefDictName[128];		/* Name of the global set def definitions reguested from ref data server. */
	char refDataTraceOutputFile[128];	/* reference data server xml trace file name */
	char realTimeTraceOutputFile[128];	/* real time data server xml trace file name */
	RsslBool xmlRefDataTrace;		/* enables xml tracing of messages from reference data server */
	RsslBool xmlRealTimeTrace;		/* enables xml tracing of messages from real time serrver */
	RsslInt32 serviceId;			/* Service Id of the service. */
	RsslUInt32 runTime;			/* Application's running time. */
	RsslBool gapFillServerVerboseOutput;	/* specifies to print all data from gap fill server to the screen */
	RsslBool realTimeDataVerboseOutput;	/* specifies to print all data from real time data feed to the screen */
	Item itemList[MAX_ITEMS];
} EDFExampleConfig;


/* The global configuration. */
extern EDFExampleConfig exampleConfig;

/* Initializes the example configuration. */
void edfExampleConfigInit(int argc, char **argv);

static RsslFieldSetDefDb globalFieldSetDefDb;

#endif

