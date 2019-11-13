/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "transportPerfConfig.h"
#include "transportThreads.h"
#include "rtr/rsslRDM.h"
#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#define snprintf _snprintf
#define getpid _getpid
#else
#include <unistd.h>
#endif

static RsslInt32 defaultThreadCount = 1;
static RsslInt32 defaultThreadBindList[] = { -1 };

/* Contains the global application configuration */
TransportPerfConfig transportPerfConfig;

static void clearTransportPerfConfig()
{
	transportPerfConfig.runTime = 300;
	snprintf(transportPerfConfig.summaryFilename, sizeof(transportPerfConfig.summaryFilename), "TransportSummary_%d.out", getpid());
	transportPerfConfig.writeStatsInterval = 5;
	transportPerfConfig.displayStats = RSSL_TRUE;
	transportPerfConfig.threadCount = defaultThreadCount;
	transportPerfConfig.threadBindList = defaultThreadBindList;

	transportPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	transportPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
	transportPerfConfig.reflectMsgs = RSSL_FALSE;
	transportPerfConfig.guaranteedOutputBuffers = 5000;
	transportPerfConfig.maxFragmentSize = 6144;
	transportPerfConfig.sendBufSize = 0;
	transportPerfConfig.recvBufSize = 0;
	transportPerfConfig.compressionType = RSSL_COMP_NONE;
	transportPerfConfig.compressionLevel = 5;
	transportPerfConfig.highWaterMark = 0;
	snprintf(transportPerfConfig.interfaceName, sizeof(transportPerfConfig.interfaceName), "");
	snprintf(transportPerfConfig.hostName, sizeof(transportPerfConfig.hostName), "%s", "localhost");
	snprintf(transportPerfConfig.portNo, sizeof(transportPerfConfig.portNo), "%s", "14002");
	transportPerfConfig.tcpNoDelay = RSSL_TRUE;
	snprintf(transportPerfConfig.sendAddr, sizeof(transportPerfConfig.sendAddr), "");
	snprintf(transportPerfConfig.recvAddr, sizeof(transportPerfConfig.recvAddr), "");
	snprintf(transportPerfConfig.sendPort, sizeof(transportPerfConfig.sendPort), "");
	snprintf(transportPerfConfig.recvPort, sizeof(transportPerfConfig.recvPort), "");
	snprintf(transportPerfConfig.unicastPort, sizeof(transportPerfConfig.unicastPort), "");
	transportPerfConfig.sAddr = RSSL_FALSE;
	transportPerfConfig.rAddr = RSSL_FALSE;
	transportPerfConfig.takeMCastStats = RSSL_FALSE;

	snprintf(transportPerfConfig.caStore, sizeof(transportPerfConfig.caStore), "");
	snprintf(transportPerfConfig.serverCert, sizeof(transportPerfConfig.serverCert), "");
	snprintf(transportPerfConfig.serverKey, sizeof(transportPerfConfig.serverKey), "");

	transportPerfConfig.appType = APPTYPE_SERVER;
	transportPerfConfig.busyRead = RSSL_FALSE;
}

void exitConfigError(char **argv)
{
	printf("Run '%s -?' to see usage.\n\n", argv[0]);
	exit(-1);
}

void exitMissingArgument(char **argv, int arg)
{
	printf("Config error: %s missing argument.\n"
			"Run '%s -?' to see usage.\n\n", argv[arg], argv[0]);
	exit(-1);
}

void initTransportPerfConfig(int argc, char **argv)
{
	int iargs;
	int scanLen;

	clearTransportPerfConfig();
	clearTransportThreadConfig();

	/* Go through the argument list, and fill in configuration structures as appropriate. */
	for(iargs = 1; iargs < argc; ++iargs)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			exitWithUsage();
		}

		else if (0 == strcmp("-runTime", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.runTime);
		}
		else if (strcmp("-summaryFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.summaryFilename, sizeof(transportPerfConfig.summaryFilename), "%s_%d.out", argv[iargs], getpid());
		}
		else if (strcmp("-statsFile", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportThreadConfig.statsFilename, sizeof(transportThreadConfig.statsFilename), "%s_%d", argv[iargs],getpid());
		}
		else if (strcmp("-writeStatsInterval", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.writeStatsInterval);
		}
		else if (strcmp("-noDisplayStats", argv[iargs]) == 0)
		{
			transportPerfConfig.displayStats = RSSL_FALSE;
		}
		else if (0 == strcmp("-threads", argv[iargs]))
		{
			char *pToken;

			transportPerfConfig.threadCount = 0;

			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
		   
			transportPerfConfig.threadBindList = (RsslInt32*)malloc(128 * sizeof(RsslInt32));
			pToken = strtok(argv[iargs], ",");
			while(pToken)
			{
				if (++transportPerfConfig.threadCount > 128)
				{
					printf("Config Error: Too many threads specified.\n");
					exitConfigError(argv);
				}

				sscanf(pToken, "%d", &transportPerfConfig.threadBindList[transportPerfConfig.threadCount-1]);

				pToken = strtok(NULL, ",");
			}

		}
		else if (0 == strcmp("-connType", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (0 == strcmp(argv[iargs], "socket"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_SOCKET;
			else if (0 == strcmp(argv[iargs], "http"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_HTTP;
			else if (0 == strcmp(argv[iargs], "encrypted"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
			else if (0 == strcmp(argv[iargs], "reliableMCast"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
			else if (0 == strcmp(argv[iargs], "shmem"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;
			else if(0 == strcmp(argv[iargs], "seqMCast"))
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_SEQ_MCAST;
			else
				transportPerfConfig.connectionType = RSSL_CONN_TYPE_INIT; /* error */
		}
		else if (strcmp("-encryptedConnType", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (strcmp("socket", argv[iargs]) == 0)
				transportPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_SOCKET;
			else if (strcmp("http", argv[iargs]) == 0)
			{
#ifdef Linux  
				printf("Config Error: Encrypted HTTP connection type not supported on Linux.\n", argv[iargs]);
				exitConfigError(argv);
#else // HTTP connnections spported only through Windows WinInet 
				transportPerfConfig.encryptedConnectionType = RSSL_CONN_TYPE_HTTP;
#endif
			}
		}
		else if (0 == strcmp("-appType", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (0 == strcmp(argv[iargs], "server"))
				transportPerfConfig.appType = APPTYPE_SERVER;
			else if (0 == strcmp(argv[iargs], "client"))
				transportPerfConfig.appType = APPTYPE_CLIENT;
			else
			{
				printf("Unknown appType: %s\n", argv[iargs]);
				exitConfigError(argv);
			}
		}
		else if (0 == strcmp("-reflectMsgs", argv[iargs]))
		{
			transportPerfConfig.reflectMsgs = RSSL_TRUE;
		}
		else if (0 == strcmp("-msgSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportThreadConfig.msgSize);
		}
		else if (0 == strcmp("-busyRead", argv[iargs]))
		{
			transportPerfConfig.busyRead = RSSL_TRUE;
		}
		else if (0 == strcmp("-outputBufs", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.guaranteedOutputBuffers);
		}
		else if (0 == strcmp("-maxFragmentSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.maxFragmentSize);
		}
		else if (0 == strcmp("-sendBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &transportPerfConfig.sendBufSize);
		}
		else if (0 == strcmp("-recvBufSize", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &transportPerfConfig.recvBufSize);
		}
		else if (0 == strcmp("-highWaterMark", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.highWaterMark);
		}
		else if (0 == strcmp("-latencyFile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			transportThreadConfig.logLatencyToFile = RSSL_TRUE;
			snprintf(transportThreadConfig.latencyLogFilename, sizeof(transportThreadConfig.latencyLogFilename), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-compressionType", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);

			if (0 == strcmp(argv[iargs], "none"))
				transportPerfConfig.compressionType = RSSL_COMP_NONE;
			else if (0 == strcmp(argv[iargs], "zlib"))
				transportPerfConfig.compressionType = RSSL_COMP_ZLIB;
			else if (0 == strcmp(argv[iargs], "lz4"))
				transportPerfConfig.compressionType = RSSL_COMP_LZ4;
			else
			{
				/* Read it as a number. */
				sscanf(argv[iargs], "%u%n", &transportPerfConfig.compressionType, &scanLen);
				if (scanLen != strlen(argv[iargs]))
				{
					printf("Config Error: Unknown compressionType: %s\n", argv[iargs]);
					exitConfigError(argv);
				}
			}
		}
		else if (0 == strcmp("-compressionLevel", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%u", &transportPerfConfig.compressionLevel);
		}
		else if (0 == strcmp("-if", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.interfaceName, sizeof(transportPerfConfig.interfaceName), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-p", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.portNo, sizeof(transportPerfConfig.portNo), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-rp", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.recvPort, sizeof(transportPerfConfig.recvPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-sp", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.sendPort, sizeof(transportPerfConfig.sendPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-u", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.unicastPort, sizeof(transportPerfConfig.unicastPort), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-h", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.hostName, sizeof(transportPerfConfig.hostName), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-ra", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.recvAddr, sizeof(transportPerfConfig.recvAddr), "%s", argv[iargs]);
			transportPerfConfig.rAddr = RSSL_TRUE;
		}
		else if (0 == strcmp("-sa", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.sendAddr, sizeof(transportPerfConfig.sendAddr), "%s", argv[iargs]);
			transportPerfConfig.sAddr = RSSL_TRUE;
		}
		else if (0 == strcmp("-tcpDelay", argv[iargs]))
		{
			transportPerfConfig.tcpNoDelay = RSSL_FALSE;
		}
		else if (0 == strcmp("-msgRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &transportThreadConfig.msgsPerSec);
		}
		else if (0 == strcmp("-latencyMsgRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			if (0 == strcmp("all", argv[iargs]))
				transportThreadConfig.latencyMsgsPerSec = ALWAYS_SEND_LATENCY_MSG;
			else
				sscanf(argv[iargs], "%d", &transportThreadConfig.latencyMsgsPerSec);
		}
		else if (0 == strcmp("-tickRate", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &transportThreadConfig.ticksPerSec);
		}
		else if (0 == strcmp("-pack", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			sscanf(argv[iargs], "%d", &transportThreadConfig.totalBuffersPerPack);
		}
		else if (0 == strcmp("-directWrite", argv[iargs]))
		{
			transportThreadConfig.writeFlags |= RSSL_WRITE_DIRECT_SOCKET_WRITE;
		}
		else if (0 == strcmp("-mcastStats", argv[iargs]))
		{
			transportPerfConfig.takeMCastStats = RSSL_TRUE;
		}
		else if (0 == strcmp("-castore", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.caStore, sizeof(transportPerfConfig.caStore), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-keyfile", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.serverKey, sizeof(transportPerfConfig.serverKey), "%s", argv[iargs]);
		}
		else if (0 == strcmp("-cert", argv[iargs]))
		{
			++iargs; if (iargs == argc) exitMissingArgument(argv, iargs - 1);
			snprintf(transportPerfConfig.serverCert, sizeof(transportPerfConfig.serverCert), "%s", argv[iargs]);
		}
		else
		{
			printf("Config Error: Unrecognized option: %s\n", argv[iargs]);
			exitConfigError(argv);
		}
	}

	if (transportThreadConfig.ticksPerSec < 1)
	{
		printf("Config Error: Tick rate cannot be less than 1.\n");
		exitConfigError(argv);
	} 

	/* Conditions */

	if (transportPerfConfig.appType == APPTYPE_CLIENT && 
		transportPerfConfig.connectionType == RSSL_CONN_TYPE_UNIDIR_SHMEM && 
		transportThreadConfig.msgsPerSec != 0)
	{
		printf("Config Error: shared memory client can only read from shared memory. -msgRate must be zero for the client\n");
		exitConfigError(argv);
	}

	if (transportPerfConfig.connectionType == RSSL_CONN_TYPE_UNIDIR_SHMEM
			&& transportPerfConfig.reflectMsgs)
	{
		printf("Config Error: Cannot use reflection with unidirectional shared memory connection.\n");
		exitConfigError(argv);
	} 

	if (transportPerfConfig.writeStatsInterval < 1)
	{
		printf("Config error: Write Stats Interval cannot be less than 1.\n");
		exitConfigError(argv);
	}

	if (transportThreadConfig.totalBuffersPerPack > 1
			&& transportPerfConfig.reflectMsgs)
	{
		printf("Config Error: Reflection does not pack messages.\n");
		exitConfigError(argv);
	} 
		

	if (transportPerfConfig.appType == APPTYPE_SERVER &&
		(transportPerfConfig.connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST || transportPerfConfig.connectionType == RSSL_CONN_TYPE_SEQ_MCAST))
	{
		printf("Config Error: appType for Multicast connections must be client\n");
		exitConfigError(argv);
	}

	if (transportPerfConfig.connectionType == RSSL_CONN_TYPE_INIT)
	{
		printf("Config Error: Unknown connectionType. Valid types are \"socket\", \"http\", \"encrypted\", \"reliableMCast\", \"shmem\", \"seqMCast\" \n");
		exitConfigError(argv);
	} 

	if (transportPerfConfig.appType == APPTYPE_SERVER &&
		transportPerfConfig.connectionType == RSSL_CONN_TYPE_UNIDIR_SHMEM)
	{
		/* Shared memory servers cannot receive pings. */
		transportThreadConfig.checkPings = RSSL_FALSE;
	}

	initTransportThreadConfig();
}

static const char *connectionTypeToString(RsslConnectionTypes connType)
{
	switch(connType)
	{
		case RSSL_CONN_TYPE_SOCKET:
			return "socket";
		case RSSL_CONN_TYPE_HTTP:
			return "http";
		case RSSL_CONN_TYPE_ENCRYPTED:
			return "encrypted";
		case RSSL_CONN_TYPE_RELIABLE_MCAST:
			return "reliableMCast";
		case RSSL_CONN_TYPE_UNIDIR_SHMEM:
			return "shmem";
		case RSSL_CONN_TYPE_SEQ_MCAST:
			return "seqMCast";
		default:
			return "unknown";
	}

}

const char *compressionTypeToString(RsslCompTypes compType)
{
	switch(compType)
	{
		case RSSL_COMP_NONE:
			return "none";
		case RSSL_COMP_ZLIB:
			return "zlib";
		case RSSL_COMP_LZ4:
			return "lz4";
		default:
			return "unknown";
	}
}

void printTransportPerfConfig(FILE *file)
{
	int i;
	int threadStringPos = 0;
	char threadString[128];

	/* Build thread list */
	threadStringPos += snprintf(threadString, 128, "%d", transportPerfConfig.threadBindList[0]);
	for(i = 1; i < transportPerfConfig.threadCount; ++i)
		threadStringPos += snprintf(threadString + threadStringPos, 128 - threadStringPos, ",%d", transportPerfConfig.threadBindList[i]);

	fprintf(file, 	"--- TEST INPUTS ---\n\n");
	
	fprintf(file, 	
			"               Runtime: %u sec\n"
			"       Connection Type: %s\n",
			transportPerfConfig.runTime,
			connectionTypeToString(transportPerfConfig.connectionType));
	if (transportPerfConfig.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		if (transportPerfConfig.appType == APPTYPE_CLIENT)
		{
			fprintf(file,
				"Encrypted Connection Type: %s\n"
				"			  CA Store: %s\n",
				connectionTypeToString(transportPerfConfig.encryptedConnectionType),
				transportPerfConfig.caStore);
		}
		else
		{
			fprintf(file,
				"	Server Private Key: %s\n"
				"	Server Certificate: %s\n",
				transportPerfConfig.serverKey,
				transportPerfConfig.serverCert);
		}
	}
	
	if(transportPerfConfig.connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST && transportPerfConfig.sAddr)
	{
		fprintf(file,  "        Multicast Conn: (send %s:%s, recv %s:%s, unicast %s)\n",
				transportPerfConfig.sendAddr, transportPerfConfig.sendPort,
				transportPerfConfig.recvAddr, transportPerfConfig.recvPort,
				transportPerfConfig.unicastPort);
	}
	else if(transportPerfConfig.connectionType == RSSL_CONN_TYPE_SEQ_MCAST && transportPerfConfig.sAddr)
	{
		fprintf(file,  "        Multicast Conn: (send %s:%s, recv %s:%s)\n",
				transportPerfConfig.sendAddr, transportPerfConfig.sendPort,
				transportPerfConfig.recvAddr, transportPerfConfig.recvPort);
	}
	else
	{
		fprintf(file,
				"              Hostname: %s\n"
				"                  Port: %s\n",
				transportPerfConfig.hostName,
				transportPerfConfig.portNo);
	}

	fprintf(file,
			"              App Type: %s\n"
			"           Thread List: %s\n"
			"             Busy Read: %s\n"
			"              Msg Size: %u\n"
			"              Msg Rate: %u%s\n"
			"      Latency Msg Rate: %u%s\n"
			"        Output Buffers: %u\n"
			"     Max Fragment Size: %u\n"
			"      Send Buffer Size: %u%s\n"
			"      Recv Buffer Size: %u%s\n"
			"       High Water Mark: %u%s\n"
			"      Compression Type: %s(%u)\n"
			"     Compression Level: %u\n"
			"        Interface Name: %s\n"
			"           Tcp_NoDelay: %s\n"
			"             Tick Rate: %u\n"
			"     Use Direct Writes: %s\n"
			"      Latency Log File: %s\n"
			"          Summary File: %s\n"
			"            Stats File: %s\n"
			"  Write Stats Interval: %u\n"
			"         Display Stats: %s\n",
			transportPerfConfig.appType == APPTYPE_SERVER ? "server" : "client", 
			threadString,
			transportPerfConfig.busyRead ? "Yes" : "No",
			transportThreadConfig.msgSize,
			transportPerfConfig.reflectMsgs ? 0 : transportThreadConfig.msgsPerSec,
			transportPerfConfig.reflectMsgs ? "(reflecting)" : "",
			transportPerfConfig.reflectMsgs ? 0 : transportThreadConfig.latencyMsgsPerSec,
			transportPerfConfig.reflectMsgs ? "(reflecting)" : "",
			transportPerfConfig.guaranteedOutputBuffers,
			transportPerfConfig.maxFragmentSize,
			transportPerfConfig.sendBufSize, (transportPerfConfig.sendBufSize ? " bytes" : "(use default)"),
			transportPerfConfig.recvBufSize, (transportPerfConfig.recvBufSize ? " bytes" : "(use default)"),
			transportPerfConfig.highWaterMark, (transportPerfConfig.highWaterMark ? " bytes" : "(use default)"),
			compressionTypeToString(transportPerfConfig.compressionType),
			transportPerfConfig.compressionType,
			transportPerfConfig.compressionLevel,
			strlen(transportPerfConfig.interfaceName) ? transportPerfConfig.interfaceName : "(use default)",
			(transportPerfConfig.tcpNoDelay ? "Yes" : "No"),
			transportThreadConfig.ticksPerSec,
			(transportThreadConfig.writeFlags & RSSL_WRITE_DIRECT_SOCKET_WRITE) ? "Yes" : "No",
			transportThreadConfig.logLatencyToFile ? transportThreadConfig.latencyLogFilename : "(none)",
			transportPerfConfig.summaryFilename,
			transportThreadConfig.statsFilename,
			transportPerfConfig.writeStatsInterval,
			(transportPerfConfig.displayStats ? "Yes" : "No")
		  );

	if (transportThreadConfig.totalBuffersPerPack > 1)
		fprintf(file,  "               Packing: Yes(%d per pack)\n",
			transportThreadConfig.totalBuffersPerPack);
	else
		fprintf(file,  "               Packing: No\n");


	fprintf(file, "\n");

}

void exitWithUsage()
{
	printf(	"Options:\n"
			"  -?                         Shows this usage\n"
			"\n"
			"Connection options(for socket-based connections):\n"
			"  -h <hostname>              Name of host to connect to\n"
			"  -p <port number>           Port number\n"
			"\n"
			"Connection options(for segmented multicast connections):\n"
			"  -rp <port number>          Receive port\n"
			"  -sp <port number>          Send port\n"
			"  -u  <port number>          Unicast port\n"
			"  -ra <receive Address>      Receive Address\n"
			"  -sa <send Address>         Send Address\n"
			"\n"
			"  -appType <type>            Type of application(server, client)\n"
			"\n"
			"  -connType <type>           Type of connection(\"socket\", \"http\", \"encrypted\", \"reliableMCast\", \"shmem\", \"seqMCast\")\n"
			"  -encryptedConnType <type>  Encrypted connection protocol for a client connection only. Only used if the \"encrypted\" connection type is selected. \"http\" type is only supported on Windows. (\"socket\", \"http\")\n"
			"  -outputBufs <count>        Number of output buffers(configures guaranteedOutputBuffers in the RSSL bind/connection options)\n"
			"  -maxFragmentSize <count>   Max size of buffers(configures maxFragmentSize in the RSSL bind/connection options)\n"
			"  -sendBufSize <size>        System Send Buffer Size(configures sysSendBufSize in the RSSL bind/connection options)\n"
			"  -recvBufSize <size>        System Receive Buffer Size(configures sysRecvBufSize in the RSSL bind/connection options)\n"
			"  -highWaterMark <bytes>     Number of queued bytes at which rsslWrite() internally flushes.\n"
			"  -compressionType <type>    Type of compression to use(\"none\", \"zlib\", \"lz4\")\n"
			"  -compressionLevel <num>    Level of compression.\n"
			"  -if <interface name>       Name of network interface to use\n"
			"  -tcpDelay                  Turns off tcp_nodelay in RsslBindOpts, enabling Nagle's\n"
			"\n"
			"  -tickRate <ticks/sec>      Ticks per second\n"
			"  -msgRate <msgs/second>     Message rate per second\n"
			"  -latencyMsgRate <msgs/sec> Latency Message rate (can specify \"all\" to send it as every msg)\n"
			"  -reflectMsgs               Reflect received messages back, rather than generating our own.\n"
			"  -pack <count>              Number of messages packed in a buffer(when count > 1, rsslPackBuffer() is used)\n"
			"  -directWrite               Sets direct socket write flag when using rsslWrite()\n"
			"  -mcastStats                Take Multicast Statistics(Warning: This enables the per-channel lock).\n"
			"\n"
			"  -busyRead                  Continually read instead of using notification.\n"
			"  -msgSize                   Size of messages to send.\n"
			"  -runTime <sec>             Runtime of the application, in seconds\n"
			"  -summaryFile <filename>    Name of file for logging summary info.\n"
			"  -statsFile <filename>      Base name of file for logging periodic statistics.\n"
			"  -writeStatsInterval <sec>  Controls how often stats are written to the file.\n"
			"  -noDisplayStats            Stop printout of stats to screen.\n"
			"  -latencyFile <filename>    Base name of file for logging latency.\n"
			"\n"
			"  -threads <thread list>     list of threads, by their bound CPU. Comma-separated list. -1 means do not bind.\n"
			"                               (e.g. \"-threads 0,1 \" creates two threads bound to CPU's 0 and 1)\n"
			"\n"
			"  -castore					  File location of the certificate authority store for client connections.\n"
			"  -keyfile				  	  Server private key for OpenSSL encryption.\n"
			"  -cert					  Server certificate for openSSL encryption.\n"
			"\n"
			
			);
#ifdef _WIN32
		printf("\nPress Enter or Return key to exit application:");
		getchar();
#endif
	exit(-1);
}

