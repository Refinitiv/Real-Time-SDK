/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "edfExampleConfig.h"
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif

EDFExampleConfig exampleConfig;

static void printUsageAndExit(char *appName);

void edfExampleConfigInit(int argc, char **argv)
{
	int i,j;

	RsslBool
		hasRealTimeInterface 	= RSSL_FALSE,
		hasGapFillInterface	= RSSL_FALSE,
		hasRefDataAddr		= RSSL_FALSE,
		hasRefDataPort		= RSSL_FALSE,
		hasStatsArraySize	= RSSL_FALSE;

	memset(&exampleConfig, 0, sizeof(EDFExampleConfig));

	for (i = 0; i < MAX_ITEMS; i++)
	{
		exampleConfig.itemList[i].symbolName[0] = '\0';
		// set the domain type to invalid value
		exampleConfig.itemList[i].domainType = 0;
	}

	exampleConfig.serviceId = 0;

	exampleConfig.runTime = 300;

	exampleConfig.realTimeDataVerboseOutput = RSSL_FALSE;
	exampleConfig.gapFillServerVerboseOutput = RSSL_FALSE;

	exampleConfig.xmlRealTimeTrace = RSSL_FALSE;
	exampleConfig.xmlRefDataTrace = RSSL_FALSE;

        snprintf(exampleConfig.setDefDictName, sizeof(exampleConfig.setDefDictName), "%s",
                        "EDF_BATS");

	i = 1;
	j = 0;

	while(i < argc)
	{
		if(!(j < MAX_ITEMS))
		{
			printf("Number of items exceeded\n");
			break;
		}

		if(strcmp("-rtda", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.realTimeDataAddr, sizeof(exampleConfig.realTimeDataAddr), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-rtdp", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.realTimeDataPort, sizeof(exampleConfig.realTimeDataPort), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-ssa", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.snapshotServerAddr, sizeof(exampleConfig.snapshotServerAddr), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-ssp", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.snapshotServerPort, sizeof(exampleConfig.snapshotServerPort), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-grsa", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.gapRequestServerAddr, sizeof(exampleConfig.gapRequestServerAddr), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-grsp", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.gapRequestServerPort, sizeof(exampleConfig.gapRequestServerPort), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-gfsa", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.gapFillServerAddr, sizeof(exampleConfig.gapFillServerAddr), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-gfsp", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.gapFillServerPort, sizeof(exampleConfig.gapFillServerPort), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-gfif", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.gapFillIntf, sizeof(exampleConfig.gapFillIntf), 
				"%s", argv[i-1]);

			exampleConfig.hasGapFillInterface = RSSL_TRUE;
			hasGapFillInterface = RSSL_TRUE;
		}
		else if(strcmp("-rtif", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.realTimeDataIntf, sizeof(exampleConfig.realTimeDataIntf), 
				"%s", argv[i-1]);

			exampleConfig.hasRealTimeInterface = RSSL_TRUE;
			hasRealTimeInterface = RSSL_TRUE;
		}
		else if(strcmp("-rdsa", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.refDataServerAddr, sizeof(exampleConfig.refDataServerAddr), 
					"%s", argv[i-1]);
			hasRefDataAddr = RSSL_TRUE;
		}
		else if(strcmp("-rdsp", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.refDataServerPort, sizeof(exampleConfig.refDataServerPort), 
					"%s", argv[i-1]);
			hasRefDataPort = RSSL_TRUE;
		}
		else if(strcmp("-setDefFileName", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.setDefFileName, sizeof(exampleConfig.setDefFileName), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-setDefDictName", argv[i]) == 0)
		{
			i += 2;
			snprintf(exampleConfig.setDefDictName, sizeof(exampleConfig.setDefDictName), 
					"%s", argv[i-1]);
		}
		else if(strcmp("-runTime", argv[i]) == 0)
		{
			i += 2;
			exampleConfig.runTime = atoi(argv[i-1]);
		}
		else if(strcmp("-serviceId", argv[i]) == 0)
		{
			i += 2;
			exampleConfig.serviceId = atoi(argv[i-1]);
		}
		else if(strcmp("-mbo", argv[i]) == 0)
		{
			i += 2;
                        exampleConfig.itemList[j].domainType = RSSL_DMT_MARKET_BY_ORDER;
			snprintf(exampleConfig.itemList[j].symbolName, sizeof(exampleConfig.itemList[j].symbolName), "%s", argv[i-1]);
			j++;
		}
		else if(strcmp("-mp", argv[i]) == 0)
		{
			i += 2;
                        exampleConfig.itemList[j].domainType = RSSL_DMT_MARKET_PRICE;
			snprintf(exampleConfig.itemList[j].symbolName, sizeof(exampleConfig.itemList[j].symbolName), "%s", argv[i-1]);
			j++;
		}
		else if(strcmp("-mbp", argv[i]) == 0)
		{
			i += 2;
                        exampleConfig.itemList[j].domainType = RSSL_DMT_MARKET_BY_PRICE;
			snprintf(exampleConfig.itemList[j].symbolName, sizeof(exampleConfig.itemList[j].symbolName), "%s", argv[i-1]);
			j++;
		}
		else if(strcmp("-gfsVerbose", argv[i]) == 0)
		{
			i += 1;
			exampleConfig.gapFillServerVerboseOutput = RSSL_TRUE;
		}
		else if(strcmp("-rtdVerbose", argv[i]) == 0)
		{
			i += 1;
			exampleConfig.realTimeDataVerboseOutput = RSSL_TRUE;
		}
		else if (strcmp("-rtdx", argv[i]) == 0)
		{
			i++;
			exampleConfig.xmlRealTimeTrace = RSSL_TRUE;
			snprintf(exampleConfig.realTimeTraceOutputFile, 128, "RealtimeChannelTrace_\0");
		}
		else if (strcmp("-rdx", argv[i]) == 0)
		{
			i++;
			exampleConfig.xmlRefDataTrace = RSSL_TRUE;
			snprintf(exampleConfig.refDataTraceOutputFile, 128, "ReferenceDataServerTrace_\0");
		}
		else if(strcmp("-?", argv[i]) == 0)
		{
			printUsageAndExit(argv[0]);
		}
		else
		{
			printf("Config Error: Unrecognized option: %s\n\n", argv[i]);
			printUsageAndExit(argv[0]);
		}
	}

	if ( hasRefDataAddr == RSSL_FALSE ||
			hasRefDataPort   == RSSL_FALSE ||
			hasRealTimeInterface   == RSSL_FALSE ||
			hasGapFillInterface   == RSSL_FALSE )
	{
		printf("Config Error: The arguments -rtif, -gfif, -rdsa, -rdsp and -mbo|mp must be specified.\n\n");
		printUsageAndExit(argv[0]);
	}

	printf( "Command Line Config:\n"
			"  realTimeDataAddr: %s\n"
			"  realTimeDataPort: %s\n"
			"  realTimeDataIntf: %s \n"
			"  snapshotServerAddr: %s\n"
			"  snapshotServerPort: %s\n"
			"  gapRequestServerAddr: %s\n"
			"  gapRequestServerPort: %s\n"
			"  gapFillServerAddr: %s\n"
			"  gapFillServerPort: %s\n"
			"  gapFillIntf: %s \n"
			"  refDataServerAddr: %s\n"
			"  refDataServerPort: %s\n"
			"  setDefFileName: %s\n"
			"  setDefDictName: %s\n"
			"  serviceId: %d\n"
			"  runTime: %u seconds\n"
			"  realTimeDataVerbose flag: %d \n"
			"  gapFillServerVerbose flag: %d \n"
			"  rtdx: %d \n"
			"  rdx: %d \n"
			"\n",
			exampleConfig.realTimeDataAddr,
			exampleConfig.realTimeDataPort,
			(exampleConfig.hasRealTimeInterface ? exampleConfig.realTimeDataIntf : "(none)"),
			exampleConfig.snapshotServerAddr,
			exampleConfig.snapshotServerPort,
			exampleConfig.gapRequestServerAddr,
			exampleConfig.gapRequestServerPort,
			exampleConfig.gapFillServerAddr,
			exampleConfig.gapFillServerPort,
			(exampleConfig.hasGapFillInterface ? exampleConfig.gapFillIntf : "(none)"),
			exampleConfig.refDataServerAddr,
			exampleConfig.refDataServerPort,
			exampleConfig.setDefFileName,
			exampleConfig.setDefDictName,
			exampleConfig.serviceId,  
			exampleConfig.runTime,
			exampleConfig.realTimeDataVerboseOutput,
			exampleConfig.gapFillServerVerboseOutput,
			exampleConfig.xmlRealTimeTrace,
			exampleConfig.xmlRefDataTrace
			);
}

static void printUsageAndExit(char *appName)
{
	printf("Usage: %s -rtda <address> -rtdp <port> [ -rtif <address | name> ] -ssa <address> -ssp <port> -grsa <address> -grsp <port> -gfsa <address> -gfsp <port> -gfif <address | name> -rdsa <address> -rfsp <port> -serviceId <id> [-mp <MarketPrice ItemName>] [-mbo <MarketByOrder ItemName>] [ -setdef <filename> ] [ -runTime <seconds> ] [ -rdx ] [ -rtdx ]\n"
			"  -rtda specifies the address of the Realtime data feed.\n"
			"  -rtdp specifies the port of the Realtime data feed.\n"
			"  -rtif specifies the network interface address of the Realtime data feed.\n"
			"  -ssa specifies the address of the Snapshot server.\n"
			"  -ssp specifies the port of the Snapshot server.\n"
			"  -grsa specifies the address of the Gap Request server.\n"
			"  -grsp specifies the port of the Gap Request server.\n"
			"  -gfsa specifies the address of the Gap Fill server.\n"
			"  -gfsp specifies the port of the Gap Fill server.\n"
			"  -gfif specifies the network interface address of the Gap Fill data feed.\n"
			"  -rdsa specifies the address of the Ref Data server.\n"
			"  -rdsp specifies the port of the Ref Data server.\n"
			"  -serviceId specifies the id of the service to be used.\n"
			"  -mbo For each occurrence, requests item using Market By Order domain.\n"
			"  -mp For each occurrence, requests item using Market Price domain.\n"
			"  -mbp For each occurrence, requests item using Market By Price domain.\n"
			"  -setDefFileName specifies the global set definition file name.\n"
			"  -setDefDictName specifies the global set definition dictionary name to be requested from Ref Data Server.\n"
			"  -gfsVerbose specifies the output type for Gap Fill server.\n"
			"  -rtdVerbose specifies the output type for Realtime data feed.\n"
			"  -runTime specifies the running time of the application.\n"
			"  -rdx enables xml tracing of messages on reference data connection.\n"
			"  -rtdx enables xml tracing of messages on real time data connection.\n"
			"The arguments -rtif, -gfif, -rdsa, -rdsp and -mbo|mp must be specified.\n\n"
			, appName);
	exit(-1);
}

