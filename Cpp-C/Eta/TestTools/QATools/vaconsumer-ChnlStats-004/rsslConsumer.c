/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2024 LSEG. All rights reserved.
*/

/*
 * This is the main file for the rsslVAConsumer application.  It is a single-threaded
 * client application.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the ETA Reactor, makes the desired connections, and
 * dispatches for events.
 *
 * This application is intended as a basic usage example.  Some of the design choices
 * were made to favor simplicity and readability over performance.  This application 
 * is not intended to be used for measuring performance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#define strtok_r strtok_s
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

#include "rsslConsumer.h"
#include "rsslLoginConsumer.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslYieldCurveHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslVASendMessage.h"
#include "rsslPostHandler.h"

#include "rtr/rsslPayloadEntry.h"

#include "rtr/rsslReactor.h"

static RsslInt32 timeToRun = 300;
static RsslInt32 maxEventsInPool = 500;
static time_t rsslConsumerRuntime = 0;
static RsslBool runTimeExpired = RSSL_FALSE;
static time_t cacheTime = 0;
static time_t cacheInterval = 0;
static time_t statisticInterval = 0;
static RsslBool onPostEnabled = RSSL_FALSE, offPostEnabled = RSSL_FALSE;
static RsslBool xmlTrace = RSSL_FALSE;
static RsslBool enableSessionMgnt = RSSL_FALSE;
static RsslBool RTTSupport = RSSL_FALSE;
static RsslBool takeExclusiveSignOnControl = RSSL_TRUE;
static RsslBool restEnableLog = RSSL_FALSE;
static RsslBool restVerboseMode = RSSL_FALSE;
static RsslUInt restEnableLogViaCallback = 0U;  // 0: disabled, 1: enabled from the start, 2: enabled after initialization stage
static RsslUInt32 reactorDebugLevel = RSSL_RC_DEBUG_LEVEL_NONE;
static time_t debugInfoIntervalMS = 50;
static time_t nextDebugTimeMS = 0;
static RsslBool sendJsonConvError = RSSL_FALSE;

static RsslUInt32 jsonOutputBufferSize = 0;
static RsslUInt32 jsonTokenIncrementSize = 0;

#define MAX_CHAN_COMMANDS 4
static ChannelCommand chanCommands[MAX_CHAN_COMMANDS];
static int channelCommandCount = 0;

fd_set readFds, exceptFds;
static RsslReactor *pReactor = NULL;
static FILE *restLogFileName = NULL;

char userNameBlock[128];
char passwordBlock[128];
//API QA
char userName2Block[128];
char password2Block[128];
//End API QA
char clientIdBlock[128];
char clientSecretBlock[128];
char scopeBlock[128];
char clientJWKBlock[2048];
char audienceBlock[255];
static char traceOutputFile[128];
static char protocolList[128];
char authnTokenBlock[1024];
char authnExtendedBlock[1024];
char appIdBlock[128];
char proxyHost[256];
char proxyPort[256];
char proxyUserName[128];
char proxyPasswd[128];
char proxyDomain[128];
RsslBuffer userName = RSSL_INIT_BUFFER;
RsslBuffer password = RSSL_INIT_BUFFER;
// API QA
RsslBuffer userName2 = RSSL_INIT_BUFFER;
RsslBuffer password2 = RSSL_INIT_BUFFER;
// End API QA
RsslBuffer clientId = RSSL_INIT_BUFFER;
RsslBuffer clientSecret = RSSL_INIT_BUFFER;
RsslBuffer clientJWK = RSSL_INIT_BUFFER;
RsslBuffer audience = RSSL_INIT_BUFFER;
RsslBuffer authnToken = RSSL_INIT_BUFFER;
RsslBuffer authnExtended = RSSL_INIT_BUFFER;
RsslBuffer appId = RSSL_INIT_BUFFER;
RsslBuffer tokenURLV1 = RSSL_INIT_BUFFER;
RsslBuffer tokenURLV2 = RSSL_INIT_BUFFER;
RsslBuffer serviceDiscoveryURL = RSSL_INIT_BUFFER;
RsslBuffer serviceDiscoveryLocation = RSSL_INIT_BUFFER;
RsslBuffer tokenScope = RSSL_INIT_BUFFER;
RsslReactorChannelStatistic channelStatistics;
//APIQA
char statisticFilter[128];
char tokenServiceUrlBlock[512];
RsslBuffer tokenServiceUrl = RSSL_INIT_BUFFER;
int reissueTokenAttemptLimit = 0;
int restRequestTimeOut = 0;
int reissueTokenAttemptInterval = 0;
double tokenReissueRatio = 0.0;
//END APIQA

static char libsslName[255];
static char libcryptoName[255];
static char libcurlName[255];

static char tokenURLNameV1[255];
static char tokenURLNameV2[255];
static char serviceDiscoveryURLName[255];
static char serviceDiscoveryLocationName[255];
static char tokenScopeName[255];

static char restProxyHost[256];
static char restProxyPort[256];
static char restProxyUserName[128];
static char restProxyPasswd[128];
static char restProxyDomain[128];


static char sslCAStore[255];
static RsslEncryptionProtocolTypes tlsProtocol = RSSL_ENC_NONE;
/* default sub-protocol list */
static const char *defaultProtocols = "tr_json2";

static RsslBool prefHostEnabled = RSSL_FALSE;			/* Whether to use Preferred host feature. */
static RsslUInt32 prefHostConnectionListIndex = 0U;		/* Specifies an index to set as preferred host. */
static char prefHostDetectionTimeCron[RSSL_REACTOR_MAX_BUFFER_LEN_INFO_CRON];
static RsslUInt32 prefHostDetectionTimeInterval = 0U;

static RsslBool prefHostPrintDetails = RSSL_TRUE;

static RsslReactorConnectOptions prefHostReactorConnectOpts;
static RsslReactorConnectInfo prefHostReactorConnectionList[MAX_CHAN_COMMANDS];

static RsslInt32 reconnectAttemptLimit = -1;
static RsslInt32 reconnectMinDelay = 500;
static RsslInt32 reconnectMaxDelay = 6000;

static void displayCache(ChannelCommand *pCommand);
static void displayCacheDomain(ChannelCommand *pCommand, RsslUInt8 domainType, RsslBool privateStreams, RsslInt32 itemCount, ItemRequest items[]);
static RsslRet decodeEntryFromCache(ChannelCommand *pCommand, RsslPayloadEntryHandle cacheEntryHandle, RsslUInt8 domainType);
static void sendItemRequests(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel);
static RsslRet displayStatistic(ChannelCommand* pCommand, time_t currentTime, RsslErrorInfo* pErrorInfo);

static char _bufferArray[6144];

static void parseConnectionListArg(ChannelCommand* pCommand, char* argStr, char* appName);

static PreferredHostConfig preferredHostConfig;

/* 
 * Prints example usage and exits. 
 */
void printUsageAndExit(char *appName)
{
	
	printf("Usage: %s or\n%s  [-tcp|-encrypted|-encryptedSocket|-encryptedWebSocket|-encryptedHttp [<hostname>:<port> <service name>]] [<domain>:<item name>,...] ] [-uname <LoginUsername>] [-passwd <LoginPassword>] [ -clientId <Client ID> ] [-sessionMgnt] [-view] [-post] [-offpost] [-snapshot] [-runtime <seconds>] [-cache] [-cacheInterval <seconds>] [-statisticInterval <seconds>] [-tunnel] [-tsDomain <number> ] [-tsAuth] [-tsServiceName] [-x] [-runtime] [-rtt]\n"
			"\n -tcp specifies a socket connection while -encrypted specifies a encrypted connection to open and a list of items to request:\n"
			"\n     hostname:        Hostname of provider to connect to"
			"\n     port:            Port of provider to connect to"
			"\n     service:         Name of service to request items from on this connection"
			"\n     domain:itemName: Domain and name of an item to request."
			"\n         A comma-separated list of these may be specified."
			"\n         The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)\n"
			"\n         The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)\n"
			"\n         Example Usage: -tcp localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl"
			"\n           (for SymbolList requests, a name can be optionally specified)\n"
			"\n -webSocket specifies an encrypted websocket connection to open.  Host, port, service, and items are the same as -tcp above. Also note argument -protocolList\n"
			"\n -encryptedSocket specifies an encrypted connection to open.  Host, port, service, and items are the same as -tcp above.\n"
			"\n -encryptedWebSocket specifies an encrypted websocket connection to open.  Host, port, service, and items are the same as -tcp above. Also note argument -protocolList\n"
			"\n -encryptedHttp specifies an encrypted WinInet-based Http connection to open.  Host, port, service, and items are the same as -tcp above.  This option is only available on Windows.\n"
			"\n -uname specifies the username used when logging into the provider. The machine ID for Real-Time - Optimized (optional).\n"
			"\n -passwd specifies the password used when logging into the provider. The password for Real-Time - Optimized (optional).\n"
			/*API QA*/
			"\n -uname2 specifies the username used when logging into the provider. The machine ID for Real-Time - Optimized (mandatory).\n"
			"\n -passwd2 specifies the password used when logging into the provider. The password for Real-Time - Optimized (mandatory).\n"
			/*END API QA*/
			"\n -clientId specifies the Client ID for Real-Time - Optimized, or the client ID for login v2 (mandatory). To generate clientID for a V1 login, login to Eikon,\n"
			"\n  and search for App Key Generator. The App Key is the Client ID.\n"
			"\n -clientSecret associated client secret for the client ID with the v2 login.\n"
			"\n -jwkFile File containing the private JWK information for V2 login with JWT.\n"
			"\n -audience audience claim for v2 JWT logins.\n"
			"\n -sessionMgnt Enables session management in the Reactor for Real-Time - Optimized.\n"
			"\n -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials.\n"
			"\n -at Specifies the Authentication Token. If this is present, the login user name type will be RDM_LOGIN_USER_AUTHN_TOKEN.\n"
			"\n -ax Specifies the Authentication Extended information. \n"
			"\n -aid Specifies the Application ID.\n"
			"\n -view specifies each request using a basic dynamic view.\n"
			"\n -post specifies that the application should attempt to send post messages on the first requested Market Price item.\n"
			"\n -offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)\n"
			"\n -snapshot specifies each request using non-streaming.\n" 
			"\n -cache will store all open items in cache and periodically dump contents.\n"
			"\n -cacheInterval number of seconds between displaying cache contents; 0 = on exit only (default)\n"
			"\n -statisticInterval number of seconds between displaying channel statistics.\n"
			/*APIQA*/
			"\n -statisticFilter valid values are READ / WRITE / PING.\n"
			"\n -tokenServiceUrl specifies the (deprecated) tokenServiceURL in RsslCreateReactorOptions, see also tokenURLV1, tokenURLV2.\n"
			"\n -restRequestTimeout \n"
			"\n -tokenReissueRatio \n"
			"\n -reissueTokenAttemptLimit \n"
			"\n -reissueTokenAttemptInterval \n"
			/*END APIQA*/
			"\n -tunnel causes the consumer to open a tunnel stream that exchanges basic messages.\n"
			"\n -tsAuth causes the consumer to enable authentication when opening tunnel streams.\n"
			"\n -tsServiceName specifies the name of the service to use for tunnel streams (if not specified, the service name specified in -c/-tcp is used)\n"
			"\n -x provides an XML trace of messages\n"
			"\n -rtt all connections support the RTT feature in login\n"
			"\n"
			" Options for establishing connection(s) and sending requests through a proxy server:\n"
			"   [ -ph <proxy host> ] [ -pp <proxy port> ] [ -plogin <proxy username> ] [ -ppasswd <proxy password> ] [ -pdomain <proxy domain> ] \n"
			"\n -castore specifies the filename or directory of the OpenSSL CA store\n"
			"\n -libcurlName specifies the name of the libcurl shared object"
			"\n -libsslName specifies the name of libssl shared object"
			"\n -libcryptName specifies the name of libcrypto shared object\n"
			"\n -spTLSv1.2 enable use of cryptographic protocol TLSv1.2 used with linux encrypted connections\n"
			"\n -spTLSv1.3 enable use of cryptographic protocol TLSv1.3 used with linux encrypted connections\n"
			"\n -runtime adjusts the running time of the application.\n"
			"\n -maxEventsInPool size of event pool\n"
			"\n -restEnableLog enable REST logging message\n"
			"\n -restVerbose enable verbose REST logging message\n"
			"\n -restLogFileName set REST logging output stream\n"
			"\n -restEnableLogViaCallback <type> enable an alternative way to receive REST logging messages via callback. 0 - disabled, 1 - enabled from the start, 2 - enabled after initialization stage.\n"
			"\n -tokenURLV1 token generator URL V1\n"
			"\n -tokenURLV2 token generator URL V2\n"
			"\n -serviceDiscoveryURL Service Discovery URL\n"
			"\n -location specifies location/region when dogin service discovery\n"
			"\n -tokenScope Scope for the token. Used with both V1 and V2 tokens\n"
			"\n -restProxyHost <proxy host> Proxy host name. Used for Rest requests only: service discovery, auth\n"
			"\n -restProxyPort <proxy port> Proxy port. Used for Rest requests only: service discovery, auth\n"
			"\n -restProxyUserName <proxy username> Proxy user name. Used for Rest requests only: service discovery, auth\n"
			"\n -restProxyPasswd <proxy password> Proxy password. Used for Rest requests only: service discovery, auth\n"
			"\n -restProxyDomain <proxy domain> Proxy domain of the user. Used for Rest requests only: service discovery, auth\n"
			"\n -debugConn set 'connection' rector debug info level"
			"\n -debugEventQ set 'eventqueue' rector debug info level"
			"\n -debugTunnelStream set 'tunnelstream' debug info level"
			"\n -debugAll enable all levels of debug info"
			"\n -debugInfoInterval set time interval for debug log"
			"\n -jsonOutputBufferSize size of the buffer that the converter will allocate for its output buffer. The conversion fails if the size is not large enough"
			"\n -jsonTokenIncrementSize number of json token increment size for parsing JSON messages"
			"\n -sendJsonConvError enable send json conversion error to provider"
			"\n"
			"\n Options for Preferred host:"
			"\n -enablePH enable Preferred host feature"
			"\n -preferredHostIndex <integer value> specifies the preferred host as an index in the connection list, starting at 0"
			"\n -detectionTimeInterval <value in seconds> specifies time interval to switch over to a preferred host or WSB group. 0 indicates that the detection time interval is disabled"
			"\n -detectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host or WSB group. detectionTimeInterval is used instead if this member is set to empty"
			"\n"
			"\n -fallBackInterval <value in seconds> specifies time interval in the application before call Ad Hoc Fallback Function call is invoked"
			"\n -ioctlInterval <value in seconds> specifies time interval in the application before call IOCTL is invoked"
			"\n"
			"\n Additional set of options for Preferred host. These options will be set by the IOCTL call"
			"\n -ioctlEnablePH <true/false> enable Preferred host feature"
			"\n -ioctlConnectListIndex <integer value> specifies the preferred host as an index in the connection list, starting at 0"
			"\n -ioctlDetectionTimeInterval <value in seconds> specifies time interval to switch over to a preferred host or WSB group. 0 indicates that the detection time interval is disabled"
			"\n -ioctlDetectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host or WSB group. detectionTimeInterval is used instead if this member is set to empty"
			"\n"
			"\n -reconnectAttemptLimit <integer value> specifies the maximum number of times the RsllReactor will attempt to reconnect a channel. If set to -1, there is no limit"
			"\n -reconnectMinDelay <milliseconds> specifies the minimum time the RsslReactor will wait before attempting to reconnect"
			"\n -reconnectMaxDelay <milliseconds> specifies the maximum time the RsslReactor will wait before attempting to reconnect"
			"\n"
		, appName, appName);

	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(-1);
}

/* 
 * configures options based on command line arguments.
 */
void parseCommandLine(int argc, char **argv)
{
	RsslInt32 i;
	RsslBool cacheOption = RSSL_FALSE;
	FILE* pFile = NULL;
	int readSize = 0;
	RsslBool setIoctlPreferredHostOptValue = RSSL_FALSE;

	/* Check usage and retrieve operating parameters */
	{
		ChannelCommand *pCommand = NULL;
		RsslBool hasTunnelStreamServiceName = RSSL_FALSE;
		RsslBool useTunnelStreamAuthentication = RSSL_FALSE;
		RsslUInt8 tunnelStreamDomainType = RSSL_DMT_SYSTEM;

		snprintf(protocolList, 128, "%s", defaultProtocols);
		snprintf(proxyHost, sizeof(proxyHost), "%s", "");
		snprintf(proxyPort, sizeof(proxyPort), "%s", "");
		snprintf(proxyUserName, sizeof(proxyUserName), "%s", "");
		snprintf(proxyPasswd, sizeof(proxyPasswd), "%s", "");
		snprintf(proxyDomain, sizeof(proxyDomain), "%s", "");
		for (i = 1; i < argc; i++)
		{
			if (strcmp("-sessionMgnt", argv[i]) == 0)
			{
				enableSessionMgnt = RSSL_TRUE;
				break;
			}
		}

		snprintf(libcryptoName, sizeof(libcryptoName), "%s", "");
		snprintf(libsslName, sizeof(libsslName), "%s", "");
		snprintf(libcurlName, sizeof(libcurlName), "%s", "");
		snprintf(sslCAStore, sizeof(sslCAStore), "%s", "");

		snprintf(prefHostDetectionTimeCron, sizeof(prefHostDetectionTimeCron), "%s", "");

		clearPreferredHostConfig(&preferredHostConfig);

		i = 1;

		while(i < argc)
		{
			if (strcmp("-libsslName", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(libsslName, 255, "%s", argv[i - 1]);
			}
			else if (strcmp("-libcryptoName", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(libcryptoName, 255, "%s", argv[i - 1]);
			}
			else if (strcmp("-libcurlName", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(libcurlName, 255, "%s", argv[i - 1]);
			}
			else if (strcmp("-castore", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(sslCAStore, 255, "%s", argv[i - 1]);
			}
			else if(strcmp("-uname", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				userName.length = snprintf(userNameBlock, sizeof(userNameBlock), "%s", argv[i-1]);
				userName.data = userNameBlock;
			}
			else if (strcmp("-clientId", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				clientId.length = snprintf(clientIdBlock, sizeof(clientIdBlock), "%s", argv[i - 1]);
				clientId.data = clientIdBlock;
			}
			else if (strcmp("-clientSecret", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				clientSecret.length = snprintf(clientSecretBlock, sizeof(clientSecretBlock), "%s", argv[i - 1]);
				clientSecret.data = clientSecretBlock;
			}
			else if (0 == strcmp(argv[i], "-jwkFile"))
			{
				/* As this is an example program showing API, this handling of the JWK is not secure. */
				if (++i == argc) printUsageAndExit(argv[0]);
				
				pFile = fopen(argv[i], "rb");
				if (pFile == NULL)
				{
					printf("Cannot load jwk file.\n");
					printUsageAndExit(argv[0]);
				}
				/* Read the JWK contents into a pre-allocated buffer*/
				readSize = (int)fread(clientJWKBlock, sizeof(char), 2048, pFile);
				if (readSize == 0)
				{
					printf("Cannot read jwk file.\n");
					printUsageAndExit(argv[0]);
				}
				clientJWK.data = clientJWKBlock;
				clientJWK.length = readSize;

				fclose(pFile);
				i++;
			}
			else if (strcmp("-audience", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				audience.length = snprintf(audienceBlock, sizeof(audienceBlock), "%s", argv[i - 1]);
				audience.data = audienceBlock;
			}
			else if (strcmp("-tokenURLV1", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tokenURLV1.length = snprintf(tokenURLNameV1, sizeof(tokenURLNameV1), "%s", argv[i - 1]);
				tokenURLV1.data = tokenURLNameV1;
			}
			else if (strcmp("-tokenURLV2", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tokenURLV2.length = snprintf(tokenURLNameV2, sizeof(tokenURLNameV2), "%s", argv[i - 1]);
				tokenURLV2.data = tokenURLNameV2;
			}
			else if (strcmp("-serviceDiscoveryURL", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				serviceDiscoveryURL.length = snprintf(serviceDiscoveryURLName, sizeof(serviceDiscoveryURLName), "%s", argv[i - 1]);
				serviceDiscoveryURL.data = serviceDiscoveryURLName;
			}
			else if (strcmp("-location", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				serviceDiscoveryLocation.length = snprintf(serviceDiscoveryLocationName, sizeof(serviceDiscoveryLocationName), "%s", argv[i - 1]);
				serviceDiscoveryLocation.data = serviceDiscoveryLocationName;
			}
			else if (strcmp("-tokenScope", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tokenScope.length = snprintf(tokenScopeName, sizeof(tokenScopeName), "%s", argv[i - 1]);
				tokenScope.data = tokenScopeName;
			}
			else if (strcmp("-passwd", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				password.length = snprintf(passwordBlock, sizeof(passwordBlock), "%s", argv[i - 1]);
				password.data = passwordBlock;
			}
			else if(strcmp("-at", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				authnToken.length = snprintf(authnTokenBlock, sizeof(authnTokenBlock), "%s", argv[i-1]);
				authnToken.data = authnTokenBlock;
			}
			else if(strcmp("-ax", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				authnExtended.length = snprintf(authnExtendedBlock, sizeof(authnExtendedBlock), "%s", argv[i-1]);
				authnExtended.data = authnExtendedBlock;
			}
			else if(strcmp("-aid", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				appId.length = snprintf(appIdBlock, sizeof(appIdBlock), "%s", argv[i-1]);
				appId.data = appIdBlock;
			}
			else if(strcmp("-view", argv[i]) == 0)
			{
				i++;
				setViewRequest();
			}
			else if (strcmp("-post", argv[i]) == 0)
			{
				i++;
				onPostEnabled = RSSL_TRUE;
			}
			else if (strcmp("-offpost", argv[i]) == 0)
			{
				i++;
				offPostEnabled = RSSL_TRUE;
			}
			else if(strcmp("-snapshot", argv[i]) == 0)
			{
				i++;
				setMPSnapshotRequest();
				setMBOSnapshotRequest();
				setMBPSnapshotRequest();
				setSLSnapshotRequest();
				setYCSnapshotRequest();
			}
			else if (strcmp("-ph", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(proxyHost, sizeof(proxyHost), "%s", argv[i - 1]);
			}
			else if (strcmp("-pp", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(proxyPort, sizeof(proxyPort), "%s", argv[i - 1]);
			}
			else if (strcmp("-plogin", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(proxyUserName, sizeof(proxyUserName), "%s", argv[i - 1]);
			}
			else if (strcmp("-ppasswd", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(proxyPasswd, sizeof(proxyPasswd), "%s", argv[i - 1]);
			}
			else if (strcmp("-pdomain", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(proxyDomain, sizeof(proxyDomain), "%s", argv[i - 1]);
			}
			else if ((strcmp("-protocolList", argv[i]) == 0) || (strcmp("-pl", argv[i]) == 0))
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(protocolList, 128, "%s", argv[i-1]);
			}
			else if (strcmp("-restLogFileName", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				restLogFileName = fopen(argv[i - 1], "w");
				if (!restLogFileName)
				{
					printf("Error: Unable to open the specified file name : %s \n", argv[i - 1]);
					printUsageAndExit(argv[0]);
				}
			}
			else if (strcmp("-cache", argv[i]) == 0)
			{
				i++;
				cacheOption = RSSL_TRUE;
			}
			else if (strcmp("-cacheInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				cacheInterval = atoi(argv[i-1]);
			}
			else if (strcmp("-statisticInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				statisticInterval = atoi(argv[i - 1]);
			}
			//APIQA
			else if (strcmp("-uname2", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				userName2.length = snprintf(userName2Block, sizeof(userName2Block), "%s", argv[i - 1]);
				userName2.data = userName2Block;
			}
			else if (strcmp("-passwd2", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				password2.length = snprintf(password2Block, sizeof(password2Block), "%s", argv[i - 1]);
				password2.data = password2Block;
			}
			else if (strcmp("-statisticFilter", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(statisticFilter, sizeof(statisticFilter), "%s", argv[i - 1]);
			}
			else if (strcmp("-tokenServiceUrl", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tokenServiceUrl.length = snprintf(tokenServiceUrlBlock, sizeof(tokenServiceUrlBlock), "%s", argv[i - 1]);
				tokenServiceUrl.data = tokenServiceUrlBlock;
			}
			else if (strcmp("-restRequestTimeout", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				restRequestTimeOut = atoi(argv[i - 1]);
			}
			else if (strcmp("-tokenReissueRatio", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tokenReissueRatio = atof(argv[i - 1]);
			}
			else if (strcmp("-reissueTokenAttemptInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				reissueTokenAttemptInterval = atoi(argv[i - 1]);
			}
			else if (strcmp("-reissueTokenAttemptLimit", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				reissueTokenAttemptLimit = atoi(argv[i - 1]);
			}
			//END APIQA
			else if (strcmp("-rtt", argv[i]) == 0)
			{
				i++;
				RTTSupport = RSSL_TRUE;
			}
			else if (strcmp("-restEnableLog", argv[i]) == 0)
			{
				i++;
				restEnableLog = RSSL_TRUE;
			}
			else if (strcmp("-restVerbose", argv[i]) == 0)
			{
				i++;
				restVerboseMode = RSSL_TRUE;
			}
			else if (strcmp("-restEnableLogViaCallback", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				restEnableLogViaCallback = atoi(argv[i - 1]);
			}
			else if (strcmp("-sendJsonConvError", argv[i]) == 0)
			{
				i++;
				sendJsonConvError = RSSL_TRUE;
			}
			else if ((strcmp("-c", argv[i]) == 0) || (strcmp("-tcp", argv[i]) == 0) ||
				(strcmp("-webSocket", argv[i]) == 0) ||
				(strcmp("-encrypted", argv[i]) == 0) ||
				(strcmp("-encryptedHttp", argv[i]) == 0) ||
				(strcmp("-encryptedWebSocket", argv[i]) == 0) ||
				(strcmp("-encryptedSocket", argv[i]) == 0))
			{
				char* pToken, * pToken2, * pSaveToken, * pSaveToken2;

				RsslUInt8 itemDomain;

				if (channelCommandCount == MAX_CHAN_COMMANDS)
				{
					printf("Too many connections requested.\n");
					printUsageAndExit(argv[0]);
				}

				pCommand = &chanCommands[channelCommandCount];
				hasTunnelStreamServiceName = RSSL_FALSE;
				useTunnelStreamAuthentication = RSSL_FALSE;
				tunnelStreamDomainType = RSSL_DMT_SYSTEM;

				if (strstr(argv[i], "-webSocket") != 0)
				{
					pCommand->cInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
					pCommand->cInfo.rsslConnectOptions.wsOpts.protocols = protocolList;
				}

				if (strstr(argv[i], "-encrypted") != 0)
				{
					pCommand->cInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
				}

				if (strcmp("-encryptedHttp", argv[i]) == 0)
				{
					/* HTTP is only supported with Windows WinInet connections*/
#ifdef LINUX
					printf("Error: Encrypted HTTP protocol is not supported on this platform.\n");
					printUsageAndExit(argv[0]);
#endif
					pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_HTTP;
				}
				else if (strcmp("-encryptedWebSocket", argv[i]) == 0)
				{
					pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_WEBSOCKET;
					pCommand->cInfo.rsslConnectOptions.wsOpts.protocols = protocolList;
				}
				else if (strcmp("-encryptedSocket", argv[i]) == 0)
				{
					pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
				}

				simpleTunnelMsgHandlerInit(&pCommand->simpleTunnelMsgHandler, (char*)"VAConsumer", RSSL_DMT_SYSTEM, RSSL_FALSE, RSSL_FALSE);

				/* Check whether the session management is enable */
				if (enableSessionMgnt)
				{
					pCommand->cInfo.enableSessionManagement = enableSessionMgnt;
					pCommand->cInfo.pAuthTokenEventCallback = authTokenEventCallback;
				}

				/* Syntax:
				 *  -tcp hostname:port[,hostname:port][,hostname:port][,hostname:port] SERVICE_NAME mp:TRI,mp:.DJI
				 */

				i += 1;
				if (i >= argc) printUsageAndExit(argv[0]);

				/* Checks whether the host:port was specified */
				if (strstr(argv[i], ":"))
				{
					/* Connection List parsing */
					parseConnectionListArg(pCommand, argv[i], argv[0]);
					i += 1;
				}

				/* Item Service Name */
				pToken = argv[i];
				snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", pToken);

				i += 1;
				if (i < argc)
				{
					if (argv[i][0] != '-')
					{
						/* Item List */
						pToken = strtok_r(argv[i], ",", &pSaveToken);

						while(pToken)
						{
							ItemRequest *pItemRequest;
							/* domain */
							pToken2 = strtok_r(pToken, ":", &pSaveToken2);
							if (!pToken2) { printf("Error: Missing domain.\n"); printUsageAndExit(argv[0]); }

							if (0 == strcmp(pToken2, "mp"))
							{
								if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPriceItems[pCommand->marketPriceItemCount];
									++pCommand->marketPriceItemCount;
								}
								else
								{
									printf("Number of Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbo"))
							{
								if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderItems[pCommand->marketByOrderItemCount];
									++pCommand->marketByOrderItemCount;
								}
								else
								{
									printf("Number of Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbp"))
							{
								if (pCommand->marketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPriceItems[pCommand->marketByPriceItemCount];
									++pCommand->marketByPriceItemCount;
								}
								else
								{
									printf("Number of Market By Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "yc"))
							{
								if (pCommand->yieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurveItems[pCommand->yieldCurveItemCount];
									++pCommand->yieldCurveItemCount;
								}
								else
								{
									printf("Number of Yield Curve items exceeded\n");
								}
							}
							else if(0 == strcmp(pToken2, "sl"))
							{
								itemDomain = RSSL_DMT_SYMBOL_LIST;
								pItemRequest = &pCommand->symbolListRequest;
								pCommand->sendSymbolList = RSSL_TRUE;
							}
							else if (0 == strcmp(pToken2, "mpps"))
							{
								if (pCommand->privateStreamMarketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPricePSItems[pCommand->privateStreamMarketPriceItemCount];
									++pCommand->privateStreamMarketPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbops"))
							{
								if (pCommand->privateStreamMarketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderPSItems[pCommand->privateStreamMarketByOrderItemCount];
									++pCommand->privateStreamMarketByOrderItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "ycps"))
							{
								if (pCommand->privateStreamYieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurvePSItems[pCommand->privateStreamYieldCurveItemCount];
									++pCommand->privateStreamYieldCurveItemCount;
								}
								else
								{
									printf("Number of Private Stream Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbpps"))
							{
								if (pCommand->privateStreamMarketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPricePSItems[pCommand->privateStreamMarketByPriceItemCount];
									++pCommand->privateStreamMarketByPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Price items exceeded\n");
								}
							}
							else
							{
								printf("Error: Unknown item domain: %s\n", pToken2);
								printUsageAndExit(argv[0]);
							}

							if (pItemRequest)
							{
								pItemRequest->cacheEntryHandle = 0;
								pItemRequest->streamId = 0;
								pItemRequest->groupIdIndex = -1;
							}

							/* name */
							pToken2 = strtok_r(NULL, ":", &pSaveToken2);
							if (!pToken2) 
							{ 
								if(itemDomain != RSSL_DMT_SYMBOL_LIST)
								{
									printf("Error: Missing item name.\n"); 
									printUsageAndExit(argv[0]);
								}
								else
								{
									/* No specific name given for the symbol list request.
									 * A name will be retrieved from the directory
									 * response if available. */
									pItemRequest->itemName.data = NULL;
									pItemRequest->itemName.length = 0;
								}
							}
							else
							{
								snprintf(pItemRequest->itemNameString, 128, "%s", pToken2);
								pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
								pItemRequest->itemName.data = pItemRequest->itemNameString;

								/* A specific name was specified for the symbol list request. */
								if(itemDomain == RSSL_DMT_SYMBOL_LIST)
									pCommand->userSpecSymbolList = RSSL_TRUE;
							}
							pToken = strtok_r(NULL, ",", &pSaveToken);
						}

						i += 1;
					}
				}

				++channelCommandCount;
			}
			else if (strcmp("-encryptedSocket", argv[i]) == 0)
			{
				char *pToken, *pToken2, *pSaveToken, *pSaveToken2;

				RsslUInt8 itemDomain;

				if (channelCommandCount == MAX_CHAN_COMMANDS)
				{
					printf("Too many connections requested.\n");
					printUsageAndExit(argv[0]);
				}

				pCommand = &chanCommands[channelCommandCount];
				hasTunnelStreamServiceName = RSSL_FALSE;
				useTunnelStreamAuthentication = RSSL_FALSE;
				tunnelStreamDomainType = RSSL_DMT_SYSTEM;
				pCommand->cInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
				pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;

				simpleTunnelMsgHandlerInit(&pCommand->simpleTunnelMsgHandler, (char*)"VAConsumer", RSSL_DMT_SYSTEM, RSSL_FALSE, RSSL_FALSE);


				/* Syntax:
				*  -encryptedSocket hostname:port[,hostname:port][,hostname:port][,hostname:port] SERVICE_NAME mp:TRI,mp:.DJI
				*/

				i += 1;
				if (i >= argc) printUsageAndExit(argv[0]);

				/* Connection List parsing */
				parseConnectionListArg(pCommand, argv[i], argv[0]);

				i += 1;

				/* Item Service Name */
				pToken = argv[i];
				snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", pToken);

				i += 1;
				if (i < argc)
				{
					if (argv[i][0] != '-')
					{
						/* Item List */
						pToken = strtok_r(argv[i], ",", &pSaveToken);

						while (pToken)
						{
							ItemRequest *pItemRequest;
							/* domain */
							pToken2 = strtok_r(pToken, ":", &pSaveToken2);
							if (!pToken2) { printf("Error: Missing domain.\n"); printUsageAndExit(argv[0]); }

							if (0 == strcmp(pToken2, "mp"))
							{
								if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPriceItems[pCommand->marketPriceItemCount];
									++pCommand->marketPriceItemCount;
								}
								else
								{
									printf("Number of Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbo"))
							{
								if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderItems[pCommand->marketByOrderItemCount];
									++pCommand->marketByOrderItemCount;
								}
								else
								{
									printf("Number of Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbp"))
							{
								if (pCommand->marketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPriceItems[pCommand->marketByPriceItemCount];
									++pCommand->marketByPriceItemCount;
								}
								else
								{
									printf("Number of Market By Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "yc"))
							{
								if (pCommand->yieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurveItems[pCommand->yieldCurveItemCount];
									++pCommand->yieldCurveItemCount;
								}
								else
								{
									printf("Number of Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "sl"))
							{
								itemDomain = RSSL_DMT_SYMBOL_LIST;
								pItemRequest = &pCommand->symbolListRequest;
								pCommand->sendSymbolList = RSSL_TRUE;
							}
							else if (0 == strcmp(pToken2, "mpps"))
							{
								if (pCommand->privateStreamMarketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPricePSItems[pCommand->privateStreamMarketPriceItemCount];
									++pCommand->privateStreamMarketPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbops"))
							{
								if (pCommand->privateStreamMarketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderPSItems[pCommand->privateStreamMarketByOrderItemCount];
									++pCommand->privateStreamMarketByOrderItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "ycps"))
							{
								if (pCommand->privateStreamYieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurvePSItems[pCommand->privateStreamYieldCurveItemCount];
									++pCommand->privateStreamYieldCurveItemCount;
								}
								else
								{
									printf("Number of Private Stream Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbpps"))
							{
								if (pCommand->privateStreamMarketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPricePSItems[pCommand->privateStreamMarketByPriceItemCount];
									++pCommand->privateStreamMarketByPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Price items exceeded\n");
								}
							}
							else
							{
								printf("Error: Unknown item domain: %s\n", pToken2);
								printUsageAndExit(argv[0]);
							}

							if (pItemRequest)
							{
								pItemRequest->cacheEntryHandle = 0;
								pItemRequest->streamId = 0;
								pItemRequest->groupIdIndex = -1;
							}

							/* name */
							pToken2 = strtok_r(NULL, ":", &pSaveToken2);
							if (!pToken2)
							{
								if (itemDomain != RSSL_DMT_SYMBOL_LIST)
								{
									printf("Error: Missing item name.\n");
									printUsageAndExit(argv[0]);
								}
								else
								{
									/* No specific name given for the symbol list request.
									* A name will be retrieved from the directory
									* response if available. */
									pItemRequest->itemName.data = NULL;
									pItemRequest->itemName.length = 0;
								}
							}
							else
							{
								snprintf(pItemRequest->itemNameString, 128, "%s", pToken2);
								pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
								pItemRequest->itemName.data = pItemRequest->itemNameString;

								/* A specific name was specified for the symbol list request. */
								if (itemDomain == RSSL_DMT_SYMBOL_LIST)
									pCommand->userSpecSymbolList = RSSL_TRUE;
							}
							pToken = strtok_r(NULL, ",", &pSaveToken);
						}

						i += 1;
					}
				}

				++channelCommandCount;
			}
			else if (strcmp("-encryptedHttp", argv[i]) == 0)
			{
				char *pToken, *pToken2, *pSaveToken, *pSaveToken2;

				RsslUInt8 itemDomain;

				/* HTTP is only supported with Windows WinInet connections*/
#ifdef LINUX
				printf("Error: Encrypted HTTP protocol is not supported on this platform.\n");
				printUsageAndExit(argv[0]);
#endif

				if (channelCommandCount == MAX_CHAN_COMMANDS)
				{
					printf("Too many connections requested.\n");
					printUsageAndExit(argv[0]);
				}

				pCommand = &chanCommands[channelCommandCount];
				hasTunnelStreamServiceName = RSSL_FALSE;
				useTunnelStreamAuthentication = RSSL_FALSE;
				tunnelStreamDomainType = RSSL_DMT_SYSTEM;
				pCommand->cInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
				pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_HTTP;

				simpleTunnelMsgHandlerInit(&pCommand->simpleTunnelMsgHandler, (char*)"VAConsumer", RSSL_DMT_SYSTEM, RSSL_FALSE, RSSL_FALSE);


				/* Syntax:
				*  -encryptedHttp hostname:port[,hostname:port][,hostname:port][,hostname:port] SERVICE_NAME mp:TRI,mp:.DJI
				*/

				i += 1;
				if (i >= argc) printUsageAndExit(argv[0]);

				/* Connection List parsing */
				parseConnectionListArg(pCommand, argv[i], argv[0]);

				i += 1;

				/* Item Service Name */
				pToken = argv[i];
				snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", pToken);

				i += 1;
				if (i < argc)
				{
					if (argv[i][0] != '-')
					{
						/* Item List */
						pToken = strtok_r(argv[i], ",", &pSaveToken);

						while (pToken)
						{
							ItemRequest *pItemRequest;
							/* domain */
							pToken2 = strtok_r(pToken, ":", &pSaveToken2);
							if (!pToken2) { printf("Error: Missing domain.\n"); printUsageAndExit(argv[0]); }

							if (0 == strcmp(pToken2, "mp"))
							{
								if (pCommand->marketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPriceItems[pCommand->marketPriceItemCount];
									++pCommand->marketPriceItemCount;
								}
								else
								{
									printf("Number of Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbo"))
							{
								if (pCommand->marketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderItems[pCommand->marketByOrderItemCount];
									++pCommand->marketByOrderItemCount;
								}
								else
								{
									printf("Number of Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbp"))
							{
								if (pCommand->marketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPriceItems[pCommand->marketByPriceItemCount];
									++pCommand->marketByPriceItemCount;
								}
								else
								{
									printf("Number of Market By Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "yc"))
							{
								if (pCommand->yieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurveItems[pCommand->yieldCurveItemCount];
									++pCommand->yieldCurveItemCount;
								}
								else
								{
									printf("Number of Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "sl"))
							{
								itemDomain = RSSL_DMT_SYMBOL_LIST;
								pItemRequest = &pCommand->symbolListRequest;
								pCommand->sendSymbolList = RSSL_TRUE;
							}
							else if (0 == strcmp(pToken2, "mpps"))
							{
								if (pCommand->privateStreamMarketPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_PRICE;
									pItemRequest = &pCommand->marketPricePSItems[pCommand->privateStreamMarketPriceItemCount];
									++pCommand->privateStreamMarketPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market Price items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbops"))
							{
								if (pCommand->privateStreamMarketByOrderItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_ORDER;
									pItemRequest = &pCommand->marketByOrderPSItems[pCommand->privateStreamMarketByOrderItemCount];
									++pCommand->privateStreamMarketByOrderItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Order items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "ycps"))
							{
								if (pCommand->privateStreamYieldCurveItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_YIELD_CURVE;
									pItemRequest = &pCommand->yieldCurvePSItems[pCommand->privateStreamYieldCurveItemCount];
									++pCommand->privateStreamYieldCurveItemCount;
								}
								else
								{
									printf("Number of Private Stream Yield Curve items exceeded\n");
								}
							}
							else if (0 == strcmp(pToken2, "mbpps"))
							{
								if (pCommand->privateStreamMarketByPriceItemCount < CHAN_CMD_MAX_ITEMS)
								{
									itemDomain = RSSL_DMT_MARKET_BY_PRICE;
									pItemRequest = &pCommand->marketByPricePSItems[pCommand->privateStreamMarketByPriceItemCount];
									++pCommand->privateStreamMarketByPriceItemCount;
								}
								else
								{
									printf("Number of Private Stream Market By Price items exceeded\n");
								}
							}
							else
							{
								printf("Error: Unknown item domain: %s\n", pToken2);
								printUsageAndExit(argv[0]);
							}

							if (pItemRequest)
							{
								pItemRequest->cacheEntryHandle = 0;
								pItemRequest->streamId = 0;
								pItemRequest->groupIdIndex = -1;
							}

							/* name */
							pToken2 = strtok_r(NULL, ":", &pSaveToken2);
							if (!pToken2)
							{
								if (itemDomain != RSSL_DMT_SYMBOL_LIST)
								{
									printf("Error: Missing item name.\n");
									printUsageAndExit(argv[0]);
								}
								else
								{
									/* No specific name given for the symbol list request.
									* A name will be retrieved from the directory
									* response if available. */
									pItemRequest->itemName.data = NULL;
									pItemRequest->itemName.length = 0;
								}
							}
							else
							{
								snprintf(pItemRequest->itemNameString, 128, "%s", pToken2);
								pItemRequest->itemName.length = (RsslUInt32)strlen(pItemRequest->itemNameString);
								pItemRequest->itemName.data = pItemRequest->itemNameString;

								/* A specific name was specified for the symbol list request. */
								if (itemDomain == RSSL_DMT_SYMBOL_LIST)
									pCommand->userSpecSymbolList = RSSL_TRUE;
							}
							pToken = strtok_r(NULL, ",", &pSaveToken);
						}

						i += 1;
					}
				}

				++channelCommandCount;
			}
			else if (strcmp("-x", argv[i]) == 0)
			{
				i++;
				xmlTrace = RSSL_TRUE;
				snprintf(traceOutputFile, 128, "RsslVAConsumer");
			}
			else if (strcmp("-runtime", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				timeToRun = atoi(argv[i-1]);
				if (timeToRun == 0)
					timeToRun = 5;
			}
			else if (strcmp("-maxEventsInPool", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				maxEventsInPool = atoi(argv[i - 1]);
			}
			else if (0 == strcmp("-debugConn", argv[i]))
			{
				i++;
				reactorDebugLevel |= RSSL_RC_DEBUG_LEVEL_CONNECTION;
			}
			else if (0 == strcmp("-debugEventQ", argv[i]))
			{
				i++;
				reactorDebugLevel |= RSSL_RC_DEBUG_LEVEL_EVENTQUEUE;
			}
			else if (0 == strcmp("-debugTunnelStream", argv[i]))
			{
				i++;
				reactorDebugLevel |= RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM;
			}
			else if (0 == strcmp("-debugAll", argv[i]))
			{
				i++;
				reactorDebugLevel = RSSL_RC_DEBUG_LEVEL_CONNECTION | RSSL_RC_DEBUG_LEVEL_EVENTQUEUE | RSSL_RC_DEBUG_LEVEL_TUNNELSTREAM;
			}
			else if (0 == strcmp("-debuginfoInterval", argv[i]))
			{
				i+=2; if (i > argc) printUsageAndExit(argv[0]);
				reactorDebugLevel = (time_t)atoi(argv[i]);;
			}
			else if (strcmp("-tsServiceName", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsServiceName specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				hasTunnelStreamServiceName = RSSL_TRUE;
				snprintf(pCommand->tunnelStreamServiceName, sizeof(pCommand->tunnelStreamServiceName), "%s", argv[i-1]);
			}
			else if (strcmp("-tunnel", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tunnel specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}
				pCommand->tunnelMessagingEnabled = RSSL_TRUE;

				i += 1;
			}
			else if (strcmp("-tsDomain", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsDomain specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tunnelStreamDomainType = atoi(argv[i-1]);
			}
			else if (strcmp("-tsAuth", argv[i]) == 0)
			{
				if (pCommand == NULL)
				{
					printf("-tsAuth specified before a connection.\n");
					printUsageAndExit(argv[0]);
				}

				useTunnelStreamAuthentication = RSSL_TRUE;
				i += 1;
			}
			else if (strcmp("-?", argv[i]) == 0)
			{
				printUsageAndExit(argv[0]);
			}
			else if (strcmp("-sessionMgnt", argv[i]) == 0)
			{
				i++; // Do nothing as the parameter is already handled
			}
			else if (strcmp("-takeExclusiveSignOnControl", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				if (RTR_STRNICMP(argv[i - 1], "true", 4) == 0)
				{
					takeExclusiveSignOnControl = RSSL_TRUE;
				}
				else if (RTR_STRNICMP(argv[i - 1], "false", 5) == 0)
				{
					takeExclusiveSignOnControl = RSSL_FALSE;
				}
			}
			else if (strcmp("-jsonOutputBufferSize", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				jsonOutputBufferSize = atoi(argv[i - 1]);
			}
			else if (strcmp("-jsonTokenIncrementSize", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				jsonTokenIncrementSize = atoi(argv[i - 1]);
			}
			else if (strcmp("-restProxyHost", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(restProxyHost, sizeof(restProxyHost), "%s", argv[i - 1]);
			}
			else if (strcmp("-restProxyPort", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(restProxyPort, sizeof(restProxyPort), "%s", argv[i - 1]);
			}
			else if (strcmp("-restProxyUserName", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(restProxyUserName, sizeof(restProxyUserName), "%s", argv[i - 1]);
			}
			else if (strcmp("-restProxyPasswd", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(restProxyPasswd, sizeof(restProxyPasswd), "%s", argv[i - 1]);
			}
			else if (strcmp("-restProxyDomain", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(restProxyDomain, sizeof(restProxyDomain), "%s", argv[i - 1]);
			}
			else if (strcmp("-spTLSv1.2", argv[i]) == 0)
			{
				i++;
				tlsProtocol |= RSSL_ENC_TLSV1_2;
			}
			else if (strcmp("-spTLSv1.3", argv[i]) == 0)
			{
				i++;
				tlsProtocol |= RSSL_ENC_TLSV1_3;
			}
			else if (strcmp("-enablePH", argv[i]) == 0)
			{
				i++;
				prefHostEnabled = RSSL_TRUE;
			}
			else if (strcmp("-preferredHostIndex", argv[i]) == 0)
			{
				int tempIndex = -1;
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tempIndex = atoi(argv[i - 1]);
				if (tempIndex >= 0)
				{
					prefHostConnectionListIndex = tempIndex;
				}
			}
			else if (strcmp("-detectionTimeInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				prefHostDetectionTimeInterval = atoi(argv[i - 1]);
			}
			else if (strcmp("-detectionTimeSchedule", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(prefHostDetectionTimeCron, sizeof(prefHostDetectionTimeCron), "%s", argv[i - 1]);
			}
			else if (strcmp("-fallBackInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				preferredHostConfig.directFallbackTimeInterval = atoi(argv[i - 1]);
			}
			else if (strcmp("-ioctlInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				preferredHostConfig.ioctlCallTimeInterval = atoi(argv[i - 1]);
			}
			else if (strcmp("-ioctlEnablePH", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				if (RTR_STRNICMP(argv[i - 1], "true", 4) == 0)
				{
					preferredHostConfig.rsslIoctlPreferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
				}
				else if (RTR_STRNICMP(argv[i - 1], "false", 5) == 0)
				{
					preferredHostConfig.rsslIoctlPreferredHostOpts.enablePreferredHostOptions = RSSL_FALSE;
				}
				setIoctlPreferredHostOptValue = RSSL_TRUE;
				preferredHostConfig.setIoctlEnablePH = RSSL_TRUE;
			}
			else if (strcmp("-ioctlConnectListIndex", argv[i]) == 0)
			{
				int tempIndex = -1;
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				tempIndex = atoi(argv[i - 1]);
				if (tempIndex >= 0)
				{
					preferredHostConfig.rsslIoctlPreferredHostOpts.connectionListIndex = tempIndex;
				}
				setIoctlPreferredHostOptValue = RSSL_TRUE;
				preferredHostConfig.setIoctlConnectListIndex = RSSL_TRUE;
			}
			else if (strcmp("-ioctlDetectionTimeInterval", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeInterval = atoi(argv[i - 1]);
				setIoctlPreferredHostOptValue = RSSL_TRUE;
				preferredHostConfig.setIoctlDetectionTimeInterval = RSSL_TRUE;
			}
			else if (strcmp("-ioctlDetectionTimeSchedule", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				snprintf(preferredHostConfig.ioctlDetectionTimeCron, sizeof(preferredHostConfig.ioctlDetectionTimeCron), "%s", argv[i - 1]);
				preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeSchedule.data = preferredHostConfig.ioctlDetectionTimeCron;
				preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeSchedule.length = (RsslUInt32)strlen(preferredHostConfig.ioctlDetectionTimeCron);
				setIoctlPreferredHostOptValue = RSSL_TRUE;
				preferredHostConfig.setIoctlDetectionTimeSchedule = RSSL_TRUE;
			}
			else if (strcmp("-reconnectAttemptLimit", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				reconnectAttemptLimit = atoi(argv[i - 1]);
			}
			else if (strcmp("-reconnectMinDelay", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				reconnectMinDelay = atoi(argv[i - 1]);
			}
			else if (strcmp("-reconnectMaxDelay", argv[i]) == 0)
			{
				i += 2; if (i > argc) printUsageAndExit(argv[0]);
				reconnectMaxDelay = atoi(argv[i - 1]);
			}
			else
			{
				printf("Error: Unrecognized option: %s\n", argv[i]);
				printUsageAndExit(argv[0]);
			}

			/* Check channel-specific options. */
			if (pCommand != NULL && (i >= argc || strcmp("-c", argv[i]) == 0 || strcmp("-tcp", argv[i]) == 0 || strcmp("-webSocket", argv[i]) == 0))
			{
				/* If service not specified for tunnel stream, use the service given for other items instead. */
				if (pCommand->tunnelMessagingEnabled && hasTunnelStreamServiceName == RSSL_FALSE)
				{
					snprintf(pCommand->tunnelStreamServiceName, sizeof(pCommand->tunnelStreamServiceName), "%s", pCommand->serviceName);
				}

				/* Check whether the session management is enable */
				if (enableSessionMgnt)
				{
					pCommand->cInfo.enableSessionManagement = enableSessionMgnt;
					pCommand->cInfo.pAuthTokenEventCallback = authTokenEventCallback;
				}

				pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.useAuthentication = useTunnelStreamAuthentication;
				pCommand->simpleTunnelMsgHandler.tunnelStreamHandler.domainType = tunnelStreamDomainType;
			}
		}

	}

	/* If no connections were specified on the command line,
	 * the default will be to create two connections to localhost:14002. */ 
	if (!channelCommandCount)
	{
		ChannelCommand *pCommand;
		RsslReactorConnectInfo *pInfo;

		channelCommandCount = 2;

		/* 1st Connection*/
		pCommand = &chanCommands[0];
		pInfo = &pCommand->cInfo;

		/* 1st Connection: Connect to localhost:14002 */
		snprintf(pCommand->hostName[0], MAX_BUFFER_LENGTH, "%s", "localhost");
		pInfo->rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName[0];
		snprintf(pCommand->port[0], MAX_BUFFER_LENGTH, "%s", "14002");
		pInfo->rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port[0];

		/* 1st Connection: Service name is DIRECT_FEED */
		snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", "DIRECT_FEED");

		/* 1st Connection: Request Item TRI.N */
		pCommand->marketPriceItemCount = 1;
		pCommand->marketPriceItems[0].itemName.data = (char *)"TRI.N"; 
		pCommand->marketPriceItems[0].itemName.length = 5;

		/* 2nd Connection*/ 
		pCommand = &chanCommands[1];
		pInfo = &pCommand->cInfo;

		/* 2nd Connection: Connect to localhost:14002 */
		snprintf(pCommand->hostName[0], MAX_BUFFER_LENGTH, "%s", "localhost");
		pInfo->rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName[0];
		snprintf(pCommand->port[0], MAX_BUFFER_LENGTH, "%s", "14002"); 
		pInfo->rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port[0];


		/* 2nd Connection: Service name is DIRECT_FEED */
		snprintf(pCommand->serviceName, MAX_BUFFER_LENGTH, "%s", "DIRECT_FEED");
		pInfo->rsslConnectOptions.userSpecPtr = &chanCommands[0];

		/* 2nd Connection: Request Item TRI.N & .DJI*/
		pCommand->marketPriceItemCount = 2;
		pCommand->marketPriceItems[0].itemName.data = (char *)"TRI.N"; 
		pCommand->marketPriceItems[0].itemName.length = 5;
		pCommand->marketPriceItems[1].itemName.data = (char *)".DJI"; 
		pCommand->marketPriceItems[1].itemName.length = 4;
	}

	if (channelCommandCount > 1 && prefHostEnabled)
	{
		printf("Error: when VAConsumer configures to use Preferred host then channelCommandCount (%d) should be only one connection.\n\n",
			channelCommandCount);
		printUsageAndExit(argv[0]);
	}

	if (prefHostConnectionListIndex > 0 && prefHostConnectionListIndex >= chanCommands[0].hostsCount)
	{
		printf("Error: prefHostConnectionListIndex (%u) should be less than hosts count (%u).\n\n",
			prefHostConnectionListIndex, chanCommands[0].hostsCount);
		printUsageAndExit(argv[0]);
	}

	if (preferredHostConfig.ioctlCallTimeInterval == 0 && setIoctlPreferredHostOptValue == RSSL_TRUE)
	{
		printf("Error: -ioctlInterval should be specified and have a non-zero positive value if any ioctl Preferred host parameters are specified.\n\n");
		printUsageAndExit(argv[0]);
	}

	if (cacheOption)
	{
		ChannelCommand *pCommand;
		for (i = 0; i < channelCommandCount; i++)
		{
			pCommand = &chanCommands[i];
			pCommand->cacheInfo.useCache = RSSL_TRUE;
			pCommand->cacheInfo.cursorHandle = 0;
			pCommand->cacheInfo.cacheOptions.maxItems = 100000;
			pCommand->cacheInfo.cacheHandle = rsslPayloadCacheCreate(&pCommand->cacheInfo.cacheOptions, &pCommand->cacheInfo.cacheErrorInfo);
			if (pCommand->cacheInfo.cacheHandle == 0)
			{
				printf("Error: Failed to create cache on channel %s:%s.\n\tError (%d): %s\n",
						pCommand->hostName[0], pCommand->port[0],
						pCommand->cacheInfo.cacheErrorInfo.rsslErrorId, pCommand->cacheInfo.cacheErrorInfo.text);
				pCommand->cacheInfo.useCache = RSSL_FALSE;
			}
			snprintf(pCommand->cacheInfo.cacheDictionaryKey, sizeof(pCommand->cacheInfo.cacheDictionaryKey), "cacheDictionary%d", i);
		}
	}

	/* Set proxy and RTT info for every channel. */
	for (i = 0; i < channelCommandCount; i++)
	{
		ChannelCommand *pCommand = &chanCommands[i];
		pCommand->cInfo.rsslConnectOptions.proxyOpts.proxyHostName = proxyHost;
		pCommand->cInfo.rsslConnectOptions.proxyOpts.proxyPort = proxyPort;
		pCommand->cInfo.rsslConnectOptions.proxyOpts.proxyUserName = proxyUserName;
		pCommand->cInfo.rsslConnectOptions.proxyOpts.proxyPasswd = proxyPasswd;
		pCommand->cInfo.rsslConnectOptions.proxyOpts.proxyDomain = proxyDomain;
		pCommand->cInfo.rsslConnectOptions.encryptionOpts.openSSLCAStore = sslCAStore;
		if (tlsProtocol != RSSL_ENC_NONE)
			pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = tlsProtocol;
	}
}

/*
 * Parse Connection list command line option
 */
static void parseConnectionListArg(ChannelCommand* pCommand, char* argStr, char* appName)
{
	char *pToken, *pToken2, *pSaveToken = NULL, *pSaveToken2 = NULL;
	RsslUInt32 indHost;

	/* Connection List parsing: hostname:port[,hostname:port][,hostname:port][,hostname:port] */
	pCommand->hostsCount = 0;

	pToken = strtok_r(argStr, ",", &pSaveToken);
	while (pToken)
	{
		indHost = pCommand->hostsCount;
		if (indHost >= CHAN_CMD_MAX_HOSTS)
		{
			printf("Number of hosts in connection list exceeded\n");
			break;
		}

		/* Checks whether the host:port was specified */
		if (strstr(pToken, ":"))
		{
			/* Hostname */
			pToken2 = strtok_r(pToken, ":", &pSaveToken2);
			if (!pToken2 && !enableSessionMgnt) { printf("Error: Missing hostname. ind: %u\n", indHost); printUsageAndExit(appName); }
			snprintf(pCommand->hostName[indHost], MAX_BUFFER_LENGTH, "%s", pToken2);
			if (indHost == 0)
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName[indHost];

			/* Port */
			pToken2 = strtok_r(NULL, ":", &pSaveToken2);
			if (!pToken2 && !enableSessionMgnt) { printf("Error: Missing port. ind: %u\n", indHost); printUsageAndExit(appName); }
			snprintf(pCommand->port[indHost], MAX_BUFFER_LENGTH, "%s", pToken2);
			if (indHost == 0)
				pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port[indHost];

			pToken2 = strtok_r(NULL, ":", &pSaveToken2);
			if (pToken2) { printf("Error: extra input after <hostname>:<port>. ind: %u\n", indHost); printUsageAndExit(appName); }
		}
		else
		{
			printf("Error: Missing hostname and port. ind: %u\n", indHost); printUsageAndExit(appName);
		}

		++pCommand->hostsCount;

		pToken = strtok_r(NULL, ",", &pSaveToken);
	}
	return;
}

/* 
 * Closes a channel in the RsslReactor
 */
void closeConnection(RsslReactor *pReactor, RsslReactorChannel *pChannel, ChannelCommand *pCommand)
{
	RsslErrorInfo rsslErrorInfo;

	if (pChannel->socketId != REACTOR_INVALID_SOCKET)
	{
		FD_CLR(pChannel->socketId, &readFds);
		FD_CLR(pChannel->socketId, &exceptFds);
	}

	if (rsslReactorCloseChannel(pReactor, pChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslReactorCloseChannel() failed: %s\n", rsslErrorInfo.rsslError.text);
	}

	pCommand->reactorChannelReady = RSSL_FALSE;
	pCommand->reactorChannelClosed = RSSL_TRUE;
}

RsslReactorCallbackRet authTokenEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorAuthTokenEvent *pAuthTokenEvent)
{
	RsslRet ret;
	ChannelCommand *pCommand = pReactorChannel ? (ChannelCommand*)pReactorChannel->userSpecPtr: NULL;
	char timeBuf[64];

	// yyyy-MM-dd HH:mm:ss.SSS
	dumpDateTime(timeBuf, sizeof(timeBuf));

	if (pAuthTokenEvent->pError)
	{
		printf("Retrieve an access token failed. Text: %s\n", pAuthTokenEvent->pError->rsslError.text);
	}
	else if (pCommand && pCommand->canSendLoginReissue && pAuthTokenEvent->pReactorAuthTokenInfo)
	{
		RsslReactorSubmitMsgOptions submitMsgOpts;
		RsslErrorInfo rsslErrorInfo;

		printf("%s New auth token received, submitting Login reissue\n", timeBuf);

		rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

		/* Update the access token */
		pCommand->pRole->ommConsumerRole.pLoginRequest->userName = pAuthTokenEvent->pReactorAuthTokenInfo->accessToken;
		pCommand->pRole->ommConsumerRole.pLoginRequest->flags |= (RDM_LG_RQF_HAS_USERNAME_TYPE | RDM_LG_RQF_NO_REFRESH);
		pCommand->pRole->ommConsumerRole.pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

		submitMsgOpts.pRDMMsg = (RsslRDMMsg*)pCommand->pRole->ommConsumerRole.pLoginRequest;
		if ((ret = rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("Login reissue failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
		}
		else
		{
			printf("Login reissue sent\n");
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor *pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent)
{
	RsslReactorOAuthCredentialRenewalOptions renewalOptions;
	RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
	RsslErrorInfo rsslError;
	ChannelCommand* pCommand = (ChannelCommand*)pOAuthCredentialEvent->userSpecPtr;
	char timeBuf[64];

	// yyyy-MM-dd HH:mm:ss.SSS
	dumpDateTime(timeBuf, sizeof(timeBuf));

	printf("%s Submitting OAuth credentials.\n", timeBuf);

	rsslClearReactorOAuthCredentialRenewalOptions(&renewalOptions);
	renewalOptions.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;

	rsslClearReactorOAuthCredentialRenewal(&reactorOAuthCredentialRenewal);

//API QA
	//if (password.length != 0)
	//	reactorOAuthCredentialRenewal.password = password; /* Specified password as needed */

	if (pCommand->password.length != 0)
		reactorOAuthCredentialRenewal.password = pCommand->password; /* Specified password as needed */
//END APIQA
	else if (clientSecret.length != 0)
		reactorOAuthCredentialRenewal.clientSecret = clientSecret;
	else
		reactorOAuthCredentialRenewal.clientJWK = clientJWK;

	rsslReactorSubmitOAuthCredentialRenewal(pReactor, &renewalOptions, &reactorOAuthCredentialRenewal, &rsslError);

	return RSSL_RC_CRET_SUCCESS;
}

/* 
 * Processes events about the state of an RsslReactorChannel.
 */
RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pConnEvent)
{
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	char timeBuf[64];
	time_t currTime;

	currTime = time(NULL);

	// yyyy-MM-dd HH:mm:ss.SSS
	dumpDateTime(timeBuf, sizeof(timeBuf));

	switch(pConnEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			/* A channel that we have requested via rsslReactorConnect() has come up.  Set our
			 * file descriptor sets so we can be notified to start calling rsslReactorDispatch() for
			 * this channel. This will drive the process of setting up the connection
			 * by exchanging the Login, Directory, and (if not already loaded)Dictionary messages. 
			 * The application will receive the response messages in the appropriate callback
			 * function we specified. */

#ifdef _WIN32
			int rcvBfrSize = 65535;
			int sendBfrSize = 65535;
#endif
			RsslErrorInfo rsslErrorInfo;

			printf("%s Connection up! Channel fd="SOCKET_PRINT_TYPE"\n\n", timeBuf, pReactorChannel->socketId);

			RsslReactorChannelInfo reactorChannelInfo;

			if (rsslReactorGetChannelInfo(pReactorChannel, &reactorChannelInfo, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorGetChannelInfo(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
			else
			{
				RsslChannelInfo* pRsslChannelInfo = &reactorChannelInfo.rsslChannelInfo;
				switch (pRsslChannelInfo->encryptionProtocol)
				{
				case RSSL_ENC_TLSV1_2:
					printf("Encryption protocol: TLSv1.2\n\n");
					break;
				case RSSL_ENC_TLSV1_3:
					printf("Encryption protocol: TLSv1.3\n\n");
					break;
				default:
					printf("Encryption protocol: unknown\n\n");
				}

				RsslChannel* pRsslChannel = pReactorChannel->pRsslChannel;

				if (pRsslChannel)
				{
					char hostName[512] = "";
					unsigned port = (unsigned)pRsslChannel->port;

					if (pRsslChannel->hostname != NULL)
					{
						size_t len = strlen(pRsslChannel->hostname);
						if (len > sizeof(hostName))
							len = sizeof(hostName);
						memcpy(hostName, pRsslChannel->hostname, len);
					}

					printf("pRsslChannel.state=%d, socketId=%llu. Host=%s:%u\n\n",
						pRsslChannel->state, pRsslChannel->socketId, hostName, port);
				}
				else
				{
					printf("pRsslChannel is Null\n\n");
				}

				RsslReactorPreferredHostInfo* pPreferredHostInfo = &reactorChannelInfo.rsslPreferredHostInfo;
				printf("Preferred host feature: %s\n", (pPreferredHostInfo->isPreferredHostEnabled ? "Enabled" : "Disabled"));
				if (pPreferredHostInfo->isPreferredHostEnabled && prefHostPrintDetails)
				{
					printf("   The channel is preferred: %s\n", (pPreferredHostInfo->isChannelPreferred ? "Yes" : "No"));
					printf("   Connection list index: %u\n", pPreferredHostInfo->connectionListIndex);
					printf("   WarmStandBy group list index: %u\n", pPreferredHostInfo->warmStandbyGroupListIndex);

					if (pPreferredHostInfo->detectionTimeSchedule.data && pPreferredHostInfo->detectionTimeSchedule.length>0)
						printf("   Cron schedule: %*s\n", pPreferredHostInfo->detectionTimeSchedule.length, pPreferredHostInfo->detectionTimeSchedule.data);

					printf("   Detection time interval: %u\n", pPreferredHostInfo->detectionTimeInterval);
					printf("   Remaining time: %u\n", pPreferredHostInfo->remainingDetectionTime);

					printf("\n");
				}

				/* Adjust the Ioctl preferred host options. */
				/* Defaults to whatever application has already set it to so it doesn't change. */
				if (preferredHostConfig.ioctlCallTimeInterval > 0)
				{
					RsslPreferredHostOptions* pIoctlPreferredHostOpts = &preferredHostConfig.rsslIoctlPreferredHostOpts;

					/* If a new Ioctl Preferred host parameter is not specified on the command line, */
					/* we will use the value that the application has previously set. */
					if (!preferredHostConfig.setIoctlEnablePH)
					{
						pIoctlPreferredHostOpts->enablePreferredHostOptions = pPreferredHostInfo->isPreferredHostEnabled;
					}
					if (!preferredHostConfig.setIoctlConnectListIndex)
					{
						pIoctlPreferredHostOpts->connectionListIndex = pPreferredHostInfo->connectionListIndex;
					}
					if (!preferredHostConfig.setIoctlDetectionTimeInterval)
					{
						pIoctlPreferredHostOpts->detectionTimeInterval = pPreferredHostInfo->detectionTimeInterval;
					}
					if (!preferredHostConfig.setIoctlDetectionTimeSchedule)
					{
						RsslUInt32 length = sizeof(preferredHostConfig.ioctlDetectionTimeCron);
						if (length > pPreferredHostInfo->detectionTimeSchedule.length)
							length = pPreferredHostInfo->detectionTimeSchedule.length;

						memcpy(preferredHostConfig.ioctlDetectionTimeCron, pPreferredHostInfo->detectionTimeSchedule.data, length);
						pIoctlPreferredHostOpts->detectionTimeSchedule.data = preferredHostConfig.ioctlDetectionTimeCron;
						pIoctlPreferredHostOpts->detectionTimeSchedule.length = length;
					}
				}
			}

			/* Set file descriptor. */
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);

			/* Save the channel on our info structure. */
			pCommand->reactorChannel = pReactorChannel;

			if (!pCommand->dictionariesLoaded)
			{
				rsslDeleteDataDictionary(&pCommand->dictionary);
				rsslClearDataDictionary(&pCommand->dictionary);
			}

#ifdef _WIN32
			/* WINDOWS: change size of send/receive buffer since it's small by default */
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
			if (rsslReactorChannelIoctl(pReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				printf("rsslReactorChannelIoctl(): failed <%s>\n", rsslErrorInfo.rsslError.text);
			}
#endif
			if (xmlTrace) 
			{
				RsslTraceOptions traceOptions;
				RsslErrorInfo rsslErrorInfo;

				rsslClearTraceOptions(&traceOptions);
				traceOptions.traceMsgFileName = traceOutputFile;
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE | RSSL_TRACE_TO_STDOUT | RSSL_TRACE_TO_MULTIPLE_FILES | RSSL_TRACE_WRITE | RSSL_TRACE_READ | RSSL_TRACE_DUMP;
				traceOptions.traceMsgMaxFileSize = 100000000;

				rsslReactorChannelIoctl(pReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo);
			}

			/* Set timeout when VAConsumer should initiate fallback directly */
			if (preferredHostConfig.directFallbackTimeInterval > 0)
			{
				preferredHostConfig.directFallbackTime = currTime + (time_t)preferredHostConfig.directFallbackTimeInterval;

				printf("   Direct Fallback.\n");
				printf("   Time interval: %u\n", preferredHostConfig.directFallbackTimeInterval);
				printf("   Remaining time: %lld\n", (preferredHostConfig.directFallbackTime - time(NULL)));
				printf("\n");
			}

			/* Set timeout when VAConsumer should initiate Ioctl call */
			if (preferredHostConfig.ioctlCallTimeInterval > 0)
			{
				preferredHostConfig.ioctlCallTime = currTime + (time_t)preferredHostConfig.ioctlCallTimeInterval;

				printf("   Ioctl call to update PreferredHostOptions.\n");
				printf("   Time interval: %u\n", preferredHostConfig.ioctlCallTimeInterval);
				printf("   Remaining time: %lld\n", (preferredHostConfig.ioctlCallTime - time(NULL)));
				printf("\n");
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			pCommand->reactorChannelReady = RSSL_TRUE;
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			/* The file descriptor representing the RsslReactorChannel has been changed.
			 * Update our file descriptor sets. */
			printf("%s Fd change: "SOCKET_PRINT_TYPE" to "SOCKET_PRINT_TYPE"\n",
				timeBuf, pReactorChannel->oldSocketId, pReactorChannel->socketId);
			FD_CLR(pReactorChannel->oldSocketId, &readFds);
			FD_CLR(pReactorChannel->oldSocketId, &exceptFds);
			FD_SET(pReactorChannel->socketId, &readFds);
			FD_SET(pReactorChannel->socketId, &exceptFds);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			/* The channel has failed and has gone down.  Print the error, close the channel, and reconnect later. */

			printf("%s Connection down: Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			/* It is important to make sure that no more interface calls are made using the channel after
			 * calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
			 * to call it inside callback functions. */
			closeConnection(pReactor, pReactorChannel, pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			char hostName[512] = "";
			unsigned port = 0;

			if (pReactorChannel->pRsslChannel)
			{
				port = (unsigned)pReactorChannel->pRsslChannel->port;

				if (pReactorChannel->pRsslChannel->hostname != NULL)
				{
					size_t len = strlen(pReactorChannel->pRsslChannel->hostname);
					if (len > sizeof(hostName))
						len = sizeof(hostName);
					memcpy(hostName, pReactorChannel->pRsslChannel->hostname, len);
				}
			}

			printf("%s Connection down, reconnecting.  Channel fd="SOCKET_PRINT_TYPE". Host=%s:%u\n",
				timeBuf, pReactorChannel->socketId, hostName, port);

			if (pConnEvent->pError)
				printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				FD_CLR(pReactorChannel->socketId, &readFds);
				FD_CLR(pReactorChannel->socketId, &exceptFds);
			}

			clearChannelCommand(pCommand);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			/* We have received a warning event for this channel. Print the information and continue. */
			printf("%s Received warning for Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);
			printf("	Error text: %s\n\n", pConnEvent->pError->rsslError.text);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
		{
			/* The preferred host operation is complete and the connections are up. */
			/* The event means - that a timer or function triggered preferred host operation has completed. */
			printf("%s Received PREFERRED_HOST_COMPLETE for Channel fd="SOCKET_PRINT_TYPE".\n", timeBuf, pReactorChannel->socketId);
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			printf("%s Unknown connection event! (%d)\n", timeBuf, pConnEvent->channelEventType);
			cleanUpAndExit(-1);
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

static void sendItemRequests(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel)
{
	ChannelCommand *pCommand;

	if (!pReactorChannel)
		return;

	pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;

	if (pCommand->itemsRequested == RSSL_TRUE)
		return;

	if (pCommand->serviceNameFound != RSSL_TRUE || pCommand->reactorChannelReady != RSSL_TRUE)
		return;

	if (sendMarketPriceItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendMarketByOrderItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendMarketByPriceItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendYieldCurveItemRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	if (sendSymbolListRequests(pReactor, pReactorChannel) != RSSL_RET_SUCCESS)
	{
		/* It is important to make sure that no more interface calls are made using the channel after
		* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe
		* to call it inside callback functions. */
		closeConnection(pReactor, pReactorChannel, pCommand);
		return;
	}

	/* Start posting, if the provider supports it */
	if ((onPostEnabled||offPostEnabled) && !pCommand->postEnabled)
	{
		RsslRDMLoginRefresh* loginInfo = getLoginRefreshInfo(pCommand);

		if (loginInfo->supportOMMPost == RSSL_TRUE)
		{
			pCommand->postEnabled = RSSL_TRUE;
			pCommand->shouldOnStreamPost = onPostEnabled;
			pCommand->shouldOffStreamPost = offPostEnabled;
			/* This sets up our basic timing so post messages will be sent periodically */
			initPostHandler(pCommand);
		}
		else
		{
			/* provider does not support posting, disable it */
			pCommand->postEnabled = RSSL_FALSE;
			printf("\nConnected Provider does not support OMM Posting.  Disabling Post functionality.\n");
		}
	}

	pCommand->itemsRequested = RSSL_TRUE;
}

RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorJsonConversionEvent *pEvent)
{
	if (pEvent->pError)
	{
		printf("Error Id: %d, Text: %s\n", pEvent->pError->rsslError.rsslErrorId, pEvent->pError->rsslError.text);
	}

	return RSSL_RC_CRET_SUCCESS;
}


RsslRet serviceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	ChannelCommand *pCommand;
	int i = 0;

	for (i = 0; i < channelCommandCount; i++)
	{
		pCommand = &chanCommands[i];

		if (pCommand->serviceNameFound)
		{
			if (strncmp(&pCommand->serviceName[0], pServiceName->data, pServiceName->length) == 0)
			{
				*pServiceId = (RsslUInt16)pCommand->serviceId;
				return RSSL_RET_SUCCESS;
			}
		}
	}

	return RSSL_RET_FAILURE;
}

RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent)
{
	if (pLogEvent &&  pLogEvent->pRestLoggingMessage  &&  pLogEvent->pRestLoggingMessage->data)
	{
		FILE* pOutputStream = restLogFileName;
		if (!pOutputStream)
			pOutputStream = stdout;

		fprintf(pOutputStream, "{restLoggingCallback}: %s", pLogEvent->pRestLoggingMessage->data);
		fflush(pOutputStream);
	}
	return RSSL_RC_CRET_SUCCESS;
}

/*** MAIN ***/
int main(int argc, char **argv)
{
	RsslRet ret;
	RsslCreateReactorOptions reactorOpts;
	RsslErrorInfo rsslErrorInfo;
	int i;

	// API QA
	RsslReactorOMMConsumerRole consumerRole[MAX_CHAN_COMMANDS];
	RsslRDMLoginRequest loginRequest[MAX_CHAN_COMMANDS];
	RsslReactorOAuthCredential oAuthCredential[MAX_CHAN_COMMANDS]; /* This is used to specify additional OAuth credential's parameters */
	// END API QA
	//RsslReactorOMMConsumerRole consumerRole;
	//RsslRDMLoginRequest loginRequest;
	//RsslReactorOAuthCredential oAuthCredential; /* This is used to specify additional OAuth credential's parameters */
	RsslRDMDirectoryRequest dirRequest;
	RsslReactorDispatchOptions dispatchOpts;
	RsslInitializeExOpts initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
	RsslReactorJsonConverterOptions jsonConverterOptions;

	//rsslClearReactorOAuthCredential(&oAuthCredential);
	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);

	if ((ret = rsslPayloadCacheInitialize()) != RSSL_RET_SUCCESS)
	{
		printf("rsslPayloadCacheInitialize() failed: %d (%s)", ret,rsslRetCodeToString(ret));
		exitApp(-1);
	}

	for(i = 0; i < MAX_CHAN_COMMANDS; ++i)
		initChannelCommand(&chanCommands[i]);

	/* Initialize parameters from config. */
	parseCommandLine(argc, argv);

	initOpts.jitOpts.libcryptoName = libcryptoName;
	initOpts.jitOpts.libsslName = libsslName;
	initOpts.jitOpts.libcurlName = libcurlName;
	initOpts.rsslLocking = RSSL_LOCK_GLOBAL_AND_CHANNEL;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitializeEx(&initOpts, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}

	/* Initialize run-time */
	initRuntime();

	/*Initialize reactor debug interval*/
	if (reactorDebugLevel != RSSL_RC_DEBUG_LEVEL_NONE)
	{
		initReactorNextDebugTime();
	}

//APIQA commented out
	///* Initialize the default login request(Use 1 as the Login Stream ID). */
	//if (rsslInitDefaultRDMLoginRequest(&loginRequest, 1) != RSSL_RET_SUCCESS)
	//{
	//	printf("rsslInitDefaultRDMLoginRequest() failed\n");
	//	cleanUpAndExit(-1);
	//}
	//
	///* If a username was specified, change username on login request. */
	//if (userName.length)
	//	loginRequest.userName = userName;

	///* If a password was specified */
	//if (password.length)
	//{
	//	oAuthCredential.password = password;
	//}

	///* If a client ID was specified */
	//if (clientId.length)
	//{
	//	oAuthCredential.clientId = clientId;
	//	/* This is only used with LSEG token service V1 */
	//	oAuthCredential.takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	//	
	//	/* Specified the RsslReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
	//	oAuthCredential.pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
	//}

	///* If a client secret was specified */
	//if (clientSecret.length)
	//{
	//	oAuthCredential.clientSecret = clientSecret;
	//}

	///* If a JWK was specified */
	//if (clientJWK.length)
	//{
	//	oAuthCredential.clientJWK = clientJWK;
	//}

	///* If an audience was specified */
	//if (audience.length)
	//{
	//	oAuthCredential.audience = audience;
	//}

	///* If an authentication Token was specified, set it on the login request and set the user name type to RDM_LOGIN_USER_AUTHN_TOKEN */
	//if (authnToken.length)
	//{
	//	loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	//	loginRequest.userName = authnToken;
	//	loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	//	
	//	if(authnExtended.length)
	//	{
	//		loginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
	//		loginRequest.authenticationExtended = authnExtended;
	//	}
	//}

	///* If the token scope was specified */
	//if (tokenScope.length)
	//{
	//	oAuthCredential.tokenScope = tokenScope;
	//}
	//	
	//if (appId.length)
	//{
	//	loginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
	//	loginRequest.applicationId = appId;
	//}

	//if (RTTSupport == RSSL_TRUE)
	//{
	//	loginRequest.flags |= RDM_LG_RQF_RTT_SUPPORT;
	//}
// END APIQA Commented out
// API QA
	for (i = 0; i < channelCommandCount; ++i)
	{
		RsslReactorOMMConsumerRole* pConsumerRole = &consumerRole[i];
		RsslRDMLoginRequest* pLoginRequest = &loginRequest[i];
		RsslReactorOAuthCredential* pOAuthCredential = &oAuthCredential[i];
		ChannelCommand* pCommand = &chanCommands[i];

		rsslClearOMMConsumerRole(pConsumerRole);
		rsslClearReactorOAuthCredential(pOAuthCredential);

		pOAuthCredential->userSpecPtr = (void*)pCommand;

		/* Initialize the default login request(Use 1 as the Login Stream ID). */
		if (rsslInitDefaultRDMLoginRequest(pLoginRequest, 1) != RSSL_RET_SUCCESS)
		{
			printf("rsslInitDefaultRDMLoginRequest() failed\n");
			cleanUpAndExit(-1);
		}
	
		if (i == 0)
		{
			/* If a username was specified, change username on login request. */
			if (userName.length)
			{
				pLoginRequest->userName = userName;
				pCommand->userName = userName;
			}

			/* If a password was specified */
			if (password.length)
			{
				pOAuthCredential->password = password;
				pCommand->password = password; // it can be use in oAuthCredentialEventCallback
				/* Specified the RsslReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
				pOAuthCredential->pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
			}
		}
		else
		{
			/* If a username was specified, change username on login request. */
			if (userName2.length)
			{
				pLoginRequest->userName = userName2;
				pCommand->userName = userName2;
			}

			/* If a password was specified */
			if (password2.length)
			{
				pOAuthCredential->password = password2;
				pCommand->password = password2; // it can be use in oAuthCredentialEventCallback
				/* Specified the RsslReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
				pOAuthCredential->pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
			}
		}

		/* If a client ID was specified */
		if (clientId.length)
		{
			pOAuthCredential->clientId = clientId;
			/* This is only used with LSEG token service V1 */
			pOAuthCredential->takeExclusiveSignOnControl = takeExclusiveSignOnControl;

			/* Specified the RsslReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
			pOAuthCredential->pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
		}

		/* If a client secret was specified */
		if (clientSecret.length)
		{
			pOAuthCredential->clientSecret = clientSecret;
		}

		/* If a JWK was specified */
		if (clientJWK.length)
		{
			pOAuthCredential->clientJWK = clientJWK;
		}

		/* If an audience was specified */
		if (audience.length)
		{
			pOAuthCredential->audience = audience;
		}

		/* If an authentication Token was specified, set it on the login request and set the user name type to RDM_LOGIN_USER_AUTHN_TOKEN */
		if (authnToken.length)
		{
			pLoginRequest->flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
			pLoginRequest->userName = authnToken;
			pLoginRequest->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

			if (authnExtended.length)
			{
				pLoginRequest->flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
				pLoginRequest->authenticationExtended = authnExtended;
			}
		}

		/* If the token scope was specified */
		if (tokenScope.length)
		{
			pOAuthCredential->tokenScope = tokenScope;
		}

		if (appId.length)
		{
			pLoginRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
			pLoginRequest->applicationId = appId;
		}

		if (RTTSupport == RSSL_TRUE)
		{
			pLoginRequest->flags |= RDM_LG_RQF_RTT_SUPPORT;
		}

		/* Setup callback functions and use them on all connections*/
		pConsumerRole->loginMsgCallback = loginMsgCallback;
		pConsumerRole->directoryMsgCallback = directoryMsgCallback;
		pConsumerRole->dictionaryMsgCallback = dictionaryMsgCallback;
		pConsumerRole->base.channelEventCallback = channelEventCallback;
		pConsumerRole->base.defaultMsgCallback = defaultMsgCallback;

		/* Set the messages to send when the channel is up */
		pConsumerRole->pLoginRequest = pLoginRequest;
		pConsumerRole->pDirectoryRequest = &dirRequest;
		pConsumerRole->pOAuthCredential = pOAuthCredential; /* This is used only when the session management is enabled */
	}
// END API QA

// APIQA Commented out
	///* Setup callback functions and use them on all connections*/
	//rsslClearOMMConsumerRole(&consumerRole);
	//consumerRole.loginMsgCallback = loginMsgCallback;
	//consumerRole.directoryMsgCallback = directoryMsgCallback;
	//consumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	//consumerRole.base.channelEventCallback = channelEventCallback;
	//consumerRole.base.defaultMsgCallback = defaultMsgCallback;

	///* Set the messages to send when the channel is up */
	//consumerRole.pLoginRequest = &loginRequest;
	//consumerRole.pDirectoryRequest = &dirRequest;
	//consumerRole.pOAuthCredential = &oAuthCredential; /* This is used only when the session management is enabled */
// END APIQA Commented out

	/* Initialize the default directory request(Use 2 as the Directory Stream Id) */
	if (rsslInitDefaultRDMDirectoryRequest(&dirRequest, 2) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitDefaultRDMDirectoryRequest() failed\n");
		cleanUpAndExit(-1);
	}

	printf("Connections:\n");

	/* Print out a summary of the connections and desired items. */
	for(i = 0; i < channelCommandCount; ++i)
	{
		RsslInt32 j;

		ChannelCommand *pCommand = &chanCommands[i];

		for (j = 0; j < (int)pCommand->hostsCount; ++j)
		{
			printf("	%s:%s%s\n", pCommand->hostName[j], pCommand->port[j],
				(prefHostEnabled && prefHostConnectionListIndex == j ? "  (Preferred host)" : ""));
		}
		printf("	%s\n", pCommand->serviceName);

		printf("		MarketPriceItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketPriceItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		MarketByOrderItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByOrderItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		MarketByPriceItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByPriceItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");

		printf("		YieldCurveItems:");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->yieldCurveItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}

		printf("\n");

		printf("		MarketPriceItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketPricePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		MarketByOrderItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByOrderPSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		MarketByPriceItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->marketByPricePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}
		printf("\n");
		printf("		YieldCurveItems (Private Stream):");
		for(j = 0; j < CHAN_CMD_MAX_ITEMS; ++j)
		{
			ItemRequest *pRequest = &pCommand->yieldCurvePSItems[j];
			if (pRequest->itemName.data)
			printf(" %.*s", pRequest->itemName.length, pRequest->itemName.data);
		}

		printf("\n");
	}

	/* Initialize connection options and try to load dictionaries. */
	for(i = 0; i < channelCommandCount; ++i)
	{
		ChannelCommand *pCommand = &chanCommands[i];
		RsslReactorConnectOptions *pOpts = &pCommand->cOpts;
		RsslReactorConnectInfo *pInfo = &pCommand->cInfo;
		unsigned j;

		loadDictionary(pCommand);
// API QA
		pCommand->pRole = (RsslReactorChannelRole*)&consumerRole[i];
		//pCommand->pRole = (RsslReactorChannelRole*)&consumerRole;
// END API QA
		pInfo->rsslConnectOptions.guaranteedOutputBuffers = 500;
		pInfo->rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		pInfo->rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		pInfo->rsslConnectOptions.userSpecPtr = &chanCommands[i];
		pInfo->initializationTimeout = 30;
		pOpts->reactorConnectionList = pInfo;
		pOpts->connectionCount = 1;
		pOpts->reconnectAttemptLimit = reconnectAttemptLimit;
		pOpts->reconnectMaxDelay = reconnectMaxDelay;
		pOpts->reconnectMinDelay = reconnectMinDelay;

		if (serviceDiscoveryLocation.length != 0)
		{
			pOpts->reactorConnectionList->location = serviceDiscoveryLocation;
		}

		/* Specify interests to get channel statistics */
		if(statisticInterval > 0)
		{
			//APIQA
			if (0 == strcmp(statisticFilter, "READ"))
			{
				pCommand->cOpts.statisticFlags = RSSL_RC_ST_READ;
			}
			else if (0 == strcmp(statisticFilter, "WRITE"))
			{
				pCommand->cOpts.statisticFlags = RSSL_RC_ST_WRITE;
			}
			else if (0 == strcmp(statisticFilter, "PING"))
			{
				pCommand->cOpts.statisticFlags = RSSL_RC_ST_PING;
			}
			else
			// END API QA
			{
				pCommand->cOpts.statisticFlags = RSSL_RC_ST_READ | RSSL_RC_ST_WRITE | RSSL_RC_ST_PING;
			}
		}

		/* Initialize connection list */
		/* When hostsCount is 0 service discovery should be called */
		if (pCommand->hostsCount > 0)
		{
			for (j = 0; j < pCommand->hostsCount; ++j)
			{
				pInfo = &pCommand->InfoItems[j];
				rsslClearReactorConnectInfo(pInfo);

				pInfo->rsslConnectOptions.connectionInfo.unified.address = pCommand->hostName[j];
				pInfo->rsslConnectOptions.connectionInfo.unified.serviceName = pCommand->port[j];

				pInfo->rsslConnectOptions.guaranteedOutputBuffers = 500;
				pInfo->rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
				pInfo->rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
				pInfo->rsslConnectOptions.userSpecPtr = &chanCommands[i];
				pInfo->initializationTimeout = 30;

				pInfo->rsslConnectOptions.pingTimeout = 600;

				pInfo->rsslConnectOptions.connectionType = pCommand->cInfo.rsslConnectOptions.connectionType;
				pInfo->rsslConnectOptions.wsOpts.protocols = pCommand->cInfo.rsslConnectOptions.wsOpts.protocols;
				pInfo->rsslConnectOptions.encryptionOpts.encryptedProtocol = pCommand->cInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol;

				if (tlsProtocol != RSSL_ENC_NONE)
					pInfo->rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = tlsProtocol;

				if (enableSessionMgnt)
				{
					pInfo->enableSessionManagement = enableSessionMgnt;
					pInfo->pAuthTokenEventCallback = authTokenEventCallback;
				}
			}

			pOpts->reactorConnectionList = pCommand->InfoItems;
			pOpts->connectionCount = pCommand->hostsCount;
		}

		/* Preferred host */
		if (prefHostEnabled)
		{
			pOpts->preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
			pOpts->preferredHostOptions.connectionListIndex = prefHostConnectionListIndex;
			pOpts->preferredHostOptions.detectionTimeInterval = prefHostDetectionTimeInterval;
			pOpts->preferredHostOptions.detectionTimeSchedule.data = prefHostDetectionTimeCron;
			pOpts->preferredHostOptions.detectionTimeSchedule.length = (RsslUInt32)strlen(prefHostDetectionTimeCron);
		}
	}

	printf("\n");

	printf("Preferred Host enabled: %s\n\n", (prefHostEnabled ? "Yes" : "No"));

	/* Create an RsslReactor which will manage our channels. */

	rsslClearCreateReactorOptions(&reactorOpts);

	reactorOpts.maxEventsInPool = maxEventsInPool;

	reactorOpts.debugLevel = reactorDebugLevel;

	reactorOpts.restEnableLog = restEnableLog;
	reactorOpts.restVerboseMode = restVerboseMode;

	if (restLogFileName)
		reactorOpts.restLogOutputStream = restLogFileName;

	if (restEnableLogViaCallback > 0)
		reactorOpts.pRestLoggingCallback = restLoggingCallback;

	if (restEnableLogViaCallback == 1)  // enabled from the start
		reactorOpts.restEnableLogViaCallback = RSSL_TRUE;

	// API QA
	if (tokenServiceUrl.length > 0)
	{
		printf("Connecting to tokenServiceUrl %s:\n", tokenServiceUrl.data);
		reactorOpts.tokenServiceURL = tokenServiceUrl;
	}
	if (tokenReissueRatio > 0.0)
	{
		printf("QA Prints tokenReissueRatio: %0.1f\n", tokenReissueRatio);
		reactorOpts.tokenReissueRatio = tokenReissueRatio;
	}
	if (restRequestTimeOut > 0)
	{
		printf("QA Prints restRequestTimeout: %d\n", restRequestTimeOut);
		reactorOpts.restRequestTimeOut = restRequestTimeOut;
	}
	if (reissueTokenAttemptInterval > 0)
	{
		printf("QA Prints reissueTokenAttemptInterval: %d\n", reissueTokenAttemptInterval);
		reactorOpts.reissueTokenAttemptInterval = reissueTokenAttemptInterval;
	}
	if (reissueTokenAttemptLimit > 0)
	{
		printf("QA Prints reissueTokenAttemptLimit: %d\n", reissueTokenAttemptLimit);
		reactorOpts.reissueTokenAttemptLimit = reissueTokenAttemptLimit;
	}
	// END API QA

	if (tokenURLV1.length != 0)
	{
		reactorOpts.tokenServiceURL_V1 = tokenURLV1;
	}

	if (tokenURLV2.length != 0)
	{
		reactorOpts.tokenServiceURL_V2 = tokenURLV2;
	}

	if (serviceDiscoveryURL.length != 0)
	{
		// API QA
		printf("Connecting to serviceDiscoveryUrl %s:\n", serviceDiscoveryURL.data);
		// END API QA
		reactorOpts.serviceDiscoveryURL = serviceDiscoveryURL;
	}

	if (restProxyHost[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyHostName = restProxyHost;
	}

	if (restProxyPort[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPort = restProxyPort;
	}

	if (restProxyUserName[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyUserName = restProxyUserName;
	}

	if (restProxyPasswd[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyPasswd = restProxyPasswd;
	}

	if (restProxyDomain[0] != '\0')
	{
		reactorOpts.restProxyOptions.proxyDomain = restProxyDomain;
	}

	if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
	{
		printf("Error: %s", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	/* Set the reactor's event file descriptor on our descriptor set. This, along with the file descriptors 
	 * of RsslReactorChannels, will notify us when we should call rsslReactorDispatch(). */
	FD_SET(pReactor->eventFd, &readFds);
	FD_SET(pReactor->eventFd, &exceptFds);

	/* Add the desired connections to the reactor. */
	for (i = 0; i < channelCommandCount; ++i)
	{
		ChannelCommand* pCommand = &chanCommands[i];
		if (!isDictionaryLoaded(pCommand))
			consumerRole[i].dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
		//	consumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;

		if (prefHostEnabled)
		{
			RsslUInt32 connectionListIndex = pCommand->cOpts.preferredHostOptions.connectionListIndex;
			RsslReactorConnectInfo* pInfo = &pCommand->InfoItems[connectionListIndex];
			printf("Adding connection to %s:%s...\n", pInfo->rsslConnectOptions.connectionInfo.unified.address, pInfo->rsslConnectOptions.connectionInfo.unified.serviceName);
		}
		else
		{
			printf("Adding connection to %s:%s...\n", pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.address, pCommand->cInfo.rsslConnectOptions.connectionInfo.unified.serviceName);
		}

		if (rsslReactorConnect(pReactor, &pCommand->cOpts, pCommand->pRole, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error rsslReactorConnect(): %s\n", rsslErrorInfo.rsslError.text);
		}

		if (statisticInterval > 0)
			chanCommands[i].nextStatisticRetrivalTime = time(NULL) + statisticInterval;

		printf("\n");
	}

	jsonConverterOptions.pDictionary = &(chanCommands[0].dictionary);
	jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdCallback;
	jsonConverterOptions.pJsonConversionEventCallback = jsonConversionEventCallback;
	jsonConverterOptions.sendJsonConvError = sendJsonConvError;

	if (jsonOutputBufferSize > 0)
	{
		jsonConverterOptions.outputBufferSize = jsonOutputBufferSize;
	}
	if (jsonTokenIncrementSize > 0)
	{
		jsonConverterOptions.jsonTokenIncrementSize = jsonTokenIncrementSize;
	}

	if (rsslReactorInitJsonConverter(pReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error initializing RWF/JSON Converter: %s\n", rsslErrorInfo.rsslError.text);
		exit(-1);
	}

	if (restEnableLogViaCallback == 2)  // enabled after initialization stage
	{
		RsslInt value = 1;

		if (rsslReactorIoctl(pReactor, RSSL_RIC_ENABLE_REST_CALLBACK_LOGGING, &value, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			printf("Error initialization Rest callback logging: %s\n", rsslErrorInfo.rsslError.text);
			exit(-1);
		}
	}

	rsslClearReactorDispatchOptions(&dispatchOpts);

	/* Main loop. The loop
	 * calls select() to wait for notification, then calls rsslReactorDispatch(). */
	do
	{
		RsslErrorInfo rsslErrorInfo;
		struct timeval selectTime;
		int dispatchCount = 0;
		fd_set useReadFds = readFds, useExceptFds = exceptFds;
		selectTime.tv_sec = 1; selectTime.tv_usec = 0;

		if ((rsslReactorGetDebugLevel(pReactor, &reactorDebugLevel, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
			printf("rsslGetReactorDebugLevel failed: %s\n", rsslErrorInfo.rsslError.text);

		if (reactorDebugLevel != RSSL_RC_DEBUG_LEVEL_NONE)
			reactorDebugPrint();

		handleRuntime();

		if (!runTimeExpired)
		{
			RsslBool reactorChannelsClosed = RSSL_TRUE;

			for (i = 0; i < channelCommandCount; ++i)
			{
				if (!chanCommands[i].reactorChannelClosed)
				{
					reactorChannelsClosed = RSSL_FALSE;
					break;
				}
			}

			if (reactorChannelsClosed)
			{
				printf("All reactor channels closed.\n\n");
				cleanUpAndExit(-1);
			}


			/* Preferred Host. Check timeout and Initiate fallback direct call. */
			if (ret = handlePreferredHostRuntime(&rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				cleanUpAndExit(-1);
			}

			for (i = 0; i < channelCommandCount; ++i)
			{
				sendItemRequests(pReactor, chanCommands[i].reactorChannel);
				if (chanCommands[i].tunnelMessagingEnabled)
					handleSimpleTunnelMsgHandler(pReactor, chanCommands[i].reactorChannel, &chanCommands[i].simpleTunnelMsgHandler);
			}

			if (onPostEnabled ||offPostEnabled)
			{
				for(i = 0; i < channelCommandCount; ++i)
				{
					if (handlePosts(pReactor, &chanCommands[i]) != RSSL_RET_SUCCESS)
						cleanUpAndExit(-1);
				}
			}

			// send login reissue if login reissue time has passed and print channel statistics
			for (i = 0; i < channelCommandCount; ++i)
			{
				time_t currentTime = 0;

				if (chanCommands[i].reactorChannelReady != RSSL_TRUE)
				{
					continue;
				}
	
				/* get current time */
				if ((currentTime = time(NULL)) < 0)
				{
					printf("time() failed.\n");
				}

				if (displayStatistic(&chanCommands[i], currentTime, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					printf("Retrieve channel statistic failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
				}

				if ((!chanCommands[i].cInfo.enableSessionManagement) && chanCommands[i].canSendLoginReissue == RSSL_TRUE &&
					currentTime >= (RsslInt)(chanCommands[i].loginReissueTime))
				{
					RsslReactorSubmitMsgOptions submitMsgOpts;

					rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
					submitMsgOpts.pRDMMsg = (RsslRDMMsg*)chanCommands[i].pRole->ommConsumerRole.pLoginRequest;
					if ((ret = rsslReactorSubmitMsg(pReactor,chanCommands[i].reactorChannel,&submitMsgOpts,&rsslErrorInfo)) != RSSL_RET_SUCCESS)
					{
						printf("Login reissue failed:  %d(%s)\n", ret, rsslErrorInfo.rsslError.text);
					}
					else
					{
						printf("Login reissue sent\n");
					}
					chanCommands[i].canSendLoginReissue = RSSL_FALSE;
				}
			}
		}

		ret = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);

		if (ret < 0)
		{
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;

			printf("Error: select: %d\n", WSAGetLastError());
#else
			if (errno == EINTR)
				continue;

			perror("select");
#endif
			cleanUpAndExit(-1);
		}
		
		/* Call rsslReactorDispatch().  This will handle any events that have occurred on its channels.
		 * If there are events or messages for the application to process, they will be delivered
		 * through the callback functions given by the consumerRole object. 
		 * A return value greater than RSSL_RET_SUCCESS indicates there may be more to process. */
		while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			printf("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
			cleanUpAndExit(-1);
		}

	} while(ret >= RSSL_RET_SUCCESS);

	cleanUpAndExit(-1);
}

/*
 * Initializes the run-time.
 */
static void initRuntime()
{
	time_t currentTime = 0;
	
	/* get current time */
	time(&currentTime);

	rsslConsumerRuntime = currentTime + (time_t)timeToRun;

	cacheTime = currentTime + (time_t)cacheInterval;
}

/*
 * Printout reactor debug information if debug was enabled.
 */
static void initReactorNextDebugTime()
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	nextDebugTimeMS = (currentTime * 1000) + debugInfoIntervalMS;
}

/*
 * Print debug data from reactor
 */

static void reactorDebugPrint()
{
	time_t currentTime = 0, currentTimeMS = 0;
	RsslReactorDebugInfo pReactorDebugInfo = { 0 };
	RsslErrorInfo pError = { 0 };
	RsslUInt32 i;

	/* get current time */
	time(&currentTime);

	if (currentTime * 1000 > nextDebugTimeMS)
	{
		if (RSSL_RET_SUCCESS != rsslReactorGetDebugInfo(pReactor, &pReactorDebugInfo, &pError))
		{
			printf("rsslReactorGetDebugInfo FAILED with error code: %d, error text: %s\n", pError.rsslError.rsslErrorId, pError.rsslError.text);
			return;
		}
		else
		{
			for (i = 0; i < pReactorDebugInfo.debugInfoBuffer.length; i++)
				printf("%c", pReactorDebugInfo.debugInfoBuffer.data[i]);
		}

		/*set new time for debug*/
		nextDebugTimeMS += debugInfoIntervalMS;
	}
}

/*
 * Handles time-related events.
 * Calls rsslReactorConnect() for any channels that we want to recover.
 * Exits the application if the run-time has expired.
 */
static void handleRuntime()
{
	time_t currentTime = 0;
	RsslRet	retval = 0;
	int i;
	RsslState state;

	/* get current time */
	time(&currentTime); 

	if (currentTime >= cacheTime && cacheInterval > 0)
	{
		for (i = 0; i < channelCommandCount; ++i)
		{
			if (chanCommands[i].cacheInfo.useCache)
				displayCache(&chanCommands[i]);
		}

		cacheTime = currentTime + (time_t)cacheInterval;
	}

	if (currentTime >= rsslConsumerRuntime)
	{
		RsslBool tunnelStreamsOpen = RSSL_FALSE;

		if (!runTimeExpired)
		{
			RsslBool waitingTunnelStreamFree = RSSL_FALSE;
			printf("rsslVAConsumer run-time expired.\n\n");
			runTimeExpired = RSSL_TRUE;

			state.streamState = RSSL_STREAM_CLOSED;
			state.dataState = RSSL_DATA_SUSPECT;
			state.code = 0;
			for(i = 0; i < channelCommandCount; ++i)
			{
				closeOffstreamPost(pReactor, &chanCommands[i]);

				/* for each connection, close all item streams */
				closeMarketPriceItemStreams(pReactor, &chanCommands[i]);
				closeMarketByOrderItemStreams(pReactor, &chanCommands[i]);
				closeMarketByPriceItemStreams(pReactor, &chanCommands[i]);
				closeYieldCurveItemStreams(pReactor, &chanCommands[i]);

				if (chanCommands[i].simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
				{
					if (waitingTunnelStreamFree == RSSL_FALSE)
					{
						waitingTunnelStreamFree = RSSL_TRUE;
						printf("Waiting for tunnel streams to close...\n");
					}

					simpleTunnelMsgHandlerCloseStreams(&chanCommands[i].simpleTunnelMsgHandler);
				}

				setItemStates(&chanCommands[i], -1, &state);
			}
		}

		/* If any tunnel streams are open, wait for them to close before quitting. */
		for(i = 0; i < channelCommandCount; ++i)
		{
			if (chanCommands[i].simpleTunnelMsgHandler.tunnelStreamHandler.pTunnelStream != NULL)
			{
				tunnelStreamsOpen = RSSL_TRUE;
				break;
			}
		}

		if (!tunnelStreamsOpen || currentTime >= rsslConsumerRuntime + 10)
		{
			if (tunnelStreamsOpen)
				printf("Tunnel streams still open after ten seconds, giving up.\n");
			cleanUpAndExit(0);
		}
	}
}

/*
 * Preferred Host.
 * Check and Initiate Ioctl and direct fallback call.
 */
static RsslRet handlePreferredHostRuntime(RsslErrorInfo* pErrorInfo)
{
	RsslRet ret;
	time_t currentTime = 0;
	int i;

	time(&currentTime);

	RsslBool callDirectFallback =
		(preferredHostConfig.directFallbackTime > 0 && currentTime >= preferredHostConfig.directFallbackTime ? RSSL_TRUE : RSSL_FALSE);
	RsslBool callIoctl =
		(preferredHostConfig.ioctlCallTime > 0 && currentTime >= preferredHostConfig.ioctlCallTime ? RSSL_TRUE : RSSL_FALSE);

	if (callDirectFallback || callIoctl)
	{
		for (i = 0; i < channelCommandCount; ++i)
		{
			if (callDirectFallback)
			{
				if (chanCommands[i].reactorChannelReady == RSSL_TRUE)
				{
					preferredHostConfig.directFallbackTime = 0;
					callDirectFallback = RSSL_FALSE;

					if ((ret = rsslReactorFallbackToPreferredHost(chanCommands[i].reactorChannel, pErrorInfo)) != RSSL_RET_SUCCESS)
					{
						printf("rsslReactorFallbackToPreferredHost failed: %d(%s)\n", ret, pErrorInfo->rsslError.text);
					}
					else
					{
						printf("Direct Fallback initiated.\n");
					}
				}
			}

			if (callIoctl)
			{
				if (chanCommands[i].reactorChannelReady == RSSL_TRUE)
				{
					preferredHostConfig.ioctlCallTime = 0;
					callIoctl = RSSL_FALSE;

					if ((ret = rsslReactorChannelIoctl(chanCommands[i].reactorChannel,
						RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS,
						(void*)&preferredHostConfig.rsslIoctlPreferredHostOpts,
						pErrorInfo)) != RSSL_RET_SUCCESS)
					{
						printf("rsslReactorChannelIoctl failed:  %d(%s)\n", ret, pErrorInfo->rsslError.text);
					}
					else
					{
						printf("rsslReactorChannelIoctl initiated.\n");
					}
				}
			}
		}
	}
	return RSSL_RET_SUCCESS;
}

/*
 * Processes all RSSL messages that aren't processed by 
 * any domain-specific callback functions.  Responses for
 * items requested by the function are handled here. 
 */
static RsslReactorCallbackRet defaultMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent)
{
	RsslRet ret = 0;
	RsslDecodeIterator dIter;
	RsslReactorCallbackRet cret = RSSL_RC_CRET_SUCCESS;
	RsslMsg *pMsg = pMsgEvent->pRsslMsg;
	ChannelCommand *pCommand = (ChannelCommand*)pChannel->userSpecPtr;

	if (!pMsg)
	{
		/* The message is not present because an error occurred while decoding it.  Print 
		 * the error and close the channel. If desired, the un-decoded message buffer 
		 * is available in pMsgEvent->pRsslMsgBuffer. */

		RsslErrorInfo *pError = pMsgEvent->pErrorInfo;
		printf("defaultMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		closeConnection(pReactor, pChannel, pCommand);
		return RSSL_RC_CRET_SUCCESS;
	}

	/* Clear the iterator and set it to the message payload. */
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pChannel->majorVersion, pChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pMsg->msgBase.encDataBody);

	switch ( pMsg->msgBase.domainType )
	{
		case RSSL_DMT_MARKET_PRICE:
			if (processMarketPriceResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			if (processMarketByOrderResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_MARKET_BY_PRICE:
			if (processMarketByPriceResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_YIELD_CURVE:
			if (processYieldCurveResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		case RSSL_DMT_SYMBOL_LIST:
			if (processSymbolListResponse(pReactor, pChannel, pMsgEvent, pMsg, &dIter) != RSSL_RET_SUCCESS)
			{
				/* It is important to make sure that no more interface calls are made using the channel after
				* calling rsslReactorCloseChannel(). Because this application is single-threaded, it is safe 
				* to call it inside callback functions. */
				closeConnection(pReactor, pChannel, pCommand);
			}
			break;
		default:
			printf("Unhandled Domain Type %d received on channel "SOCKET_PRINT_TYPE"\n", pMsg->msgBase.domainType, pChannel->socketId);
			break;
	}

	return cret;
}


/*
 * Cleans up and exits the application.
 */
void cleanUpAndExit(int code)
{
	RsslErrorInfo rsslErrorInfo;
	int i;
	unsigned int n;

	if (pReactor && rsslDestroyReactor(pReactor, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		printf("Error cleaning up reactor: %s\n", rsslErrorInfo.rsslError.text);
		exitApp(-1);
	}

	if (restLogFileName)
		fclose(restLogFileName);

	for (i = 0; i < channelCommandCount; ++i)
	{
		if (chanCommands[i].cacheInfo.useCache)
			displayCache(&chanCommands[i]);
	}

	for (i = 0; i < channelCommandCount; ++i)
	{
		if (chanCommands[i].cacheInfo.useCache && chanCommands[i].cacheInfo.cacheHandle)
			rsslPayloadCacheDestroy(chanCommands[i].cacheInfo.cacheHandle);

		for (n=0; n < chanCommands[i].groupIdCount; n++)
		{
			if (chanCommands[i].groupIdBuffers[n].data)
				free (chanCommands[i].groupIdBuffers[n].data);
		}
	}

	for(i = 0; i < MAX_CHAN_COMMANDS; ++i)
		cleanupChannelCommand(&chanCommands[i]);

	rsslPayloadCacheUninitialize();
	rsslUninitialize();

	exitApp(code);
}

void exitApp(int code)
{
	printf("Exiting.\n");
	/* WINDOWS: wait for user to enter something before exiting  */
#ifdef _WIN32
	printf("\nPress Enter or Return key to exit application:");
	getchar();
#endif
	exit(code);
}

void dumpHexBuffer(const RsslBuffer * buffer)
{
      unsigned int i;
      char * position = buffer->data;
      for (i = 0; i < buffer->length; i++, position++)
      {
            if (i % 32 == 0)
            {
                  if (i != 0)
                  {
                        printf("\n");
                  }
            }
            else if ((i != 0) && (i % 2 == 0))
            {
                  printf(" ");
            }
            printf("%2.2X", (unsigned char) *position);
      }
}

static void displayCache(ChannelCommand *pCommand)
{
	printf("\nStarting Cache Display ");

	if (pCommand->reactorChannel)
		printf("(Channel "SOCKET_PRINT_TYPE"):\n", pCommand->reactorChannel->socketId);
	else
		printf(":\n");

	if (pCommand->cacheInfo.cacheHandle)
		printf("Total Items in Cache: " RTR_LLU "\n", rsslPayloadCacheGetEntryCount(pCommand->cacheInfo.cacheHandle));

	if (!pCommand->dictionary.isInitialized)
	{
		printf("\tDictionary for decoding cache entries is not available\n");
		return;
	}

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_PRICE, RSSL_FALSE, pCommand->marketPriceItemCount, pCommand->marketPriceItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_PRICE, RSSL_TRUE, pCommand->privateStreamMarketPriceItemCount, pCommand->marketPricePSItems);

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_ORDER, RSSL_FALSE, pCommand->marketByOrderItemCount, pCommand->marketByOrderItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_ORDER, RSSL_TRUE, pCommand->privateStreamMarketByOrderItemCount, pCommand->marketByOrderPSItems);

	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_PRICE, RSSL_FALSE, pCommand->marketByPriceItemCount, pCommand->marketByPriceItems);
	displayCacheDomain(pCommand, RSSL_DMT_MARKET_BY_PRICE, RSSL_TRUE, pCommand->privateStreamMarketByPriceItemCount, pCommand->marketByPricePSItems);

	displayCacheDomain(pCommand, RSSL_DMT_YIELD_CURVE, RSSL_FALSE, pCommand->yieldCurveItemCount, pCommand->yieldCurveItems);
	displayCacheDomain(pCommand, RSSL_DMT_YIELD_CURVE, RSSL_TRUE, pCommand->privateStreamYieldCurveItemCount, pCommand->yieldCurvePSItems);

	printf("\nCache Display Complete\n");
}

/*
 * Display contents of cache for a given list of items from a given domain
 */
static void displayCacheDomain(ChannelCommand *pCommand, RsslUInt8 domainType, RsslBool privateStreams, RsslInt32 itemCount, ItemRequest items[])
{
	int i;
	RsslRet ret;
	char tempData[128];
	RsslBuffer tempBuffer;

	printf("Retrieving cache data for %d %s %s items:\n", itemCount,
			rsslDomainTypeToString(domainType),
			privateStreams ? "Private Stream" : "");
	for (i = 0; i < itemCount; i++)
	{
		if (items[i].cacheEntryHandle)
		{
			printf("\n%.*s\n", items[i].itemName.length, items[i].itemName.data);

			tempBuffer.data = tempData;
			tempBuffer.length = sizeof(tempData);
			rsslStateToString(&tempBuffer, &items[i].itemState);
			printf("%.*s\n", tempBuffer.length, tempBuffer.data);

			if ((ret = decodeEntryFromCache(pCommand, items[i].cacheEntryHandle, domainType)) != RSSL_RET_SUCCESS)
				printf("Error decoding cache content: %d\n", ret);
		}
		else
		{
			printf("\n%.*s\tno data in cache\n", items[i].itemName.length, items[i].itemName.data);
		}
	}
}

static RsslRet decodeEntryFromCache(ChannelCommand *pCommand, RsslPayloadEntryHandle cacheEntryHandle, RsslUInt8 domainType)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	RsslCacheError cacheError;
	RsslBuffer buffer;
	RsslLocalFieldSetDefDb localFieldSetDefDb;
	RsslUInt8 majorVersion;
	RsslUInt8 minorVersion;

	buffer.data = _bufferArray;
	buffer.length = sizeof(_bufferArray);

	if (pCommand->reactorChannel)
	{
		majorVersion = pCommand->reactorChannel->majorVersion;
		minorVersion = pCommand->reactorChannel->minorVersion;
	}
	else
	{
		majorVersion = pCommand->cInfo.rsslConnectOptions.majorVersion;
		minorVersion = pCommand->cInfo.rsslConnectOptions.minorVersion;
	}

	rsslClearLocalFieldSetDefDb(&localFieldSetDefDb);

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, majorVersion, minorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, &buffer);

	if (rsslPayloadEntryRetrieve(cacheEntryHandle, &eIter, 0, &cacheError) != RSSL_RET_SUCCESS)
	{
		printf("Failed retrieving cache entry.\n\tError (%d): %s\n", cacheError.rsslErrorId, cacheError.text);
		return RSSL_RET_FAILURE;
	}
	else
	{
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorBuffer(&dIter, &buffer);
		rsslSetDecodeIteratorRWFVersion(&dIter, majorVersion, minorVersion);

		switch (domainType)
		{
		case RSSL_DMT_MARKET_PRICE:
			ret = decodeMarketPriceFieldList(&pCommand->dictionary, &dIter);
			break;

		case RSSL_DMT_MARKET_BY_ORDER:
			ret = decodeMarketMap(&pCommand->dictionary, RSSL_DMT_MARKET_BY_ORDER, &dIter);
			break;

		case RSSL_DMT_MARKET_BY_PRICE:
			ret = decodeMarketMap(&pCommand->dictionary, RSSL_DMT_MARKET_BY_PRICE, &dIter);
			break;

		case RSSL_DMT_YIELD_CURVE:
			ret = decodeYieldCurvePayload(&pCommand->dictionary, &dIter, &localFieldSetDefDb);
			break;

		default:
			break;
		}
		if (ret > RSSL_RET_SUCCESS)
			ret = RSSL_RET_SUCCESS;
	}

	return ret;
}

static void cumulativeValue(RsslUInt* destination, RsslUInt value)
{
	if ( (*destination + value) > UINT64_MAX )
	{
		*destination = value;
	}
	else
	{
		*destination += value;
	}
}

static RsslRet displayStatistic(ChannelCommand* pCommand, time_t currentTime, RsslErrorInfo* pErrorInfo)
{
	RsslRet ret = RSSL_RET_SUCCESS;

	if (statisticInterval && currentTime >= pCommand->nextStatisticRetrivalTime)
	{
		RsslReactorChannelStatistic statistics;
		rsslClearReactorChannelStatistic(&statistics);
		if ((ret = rsslReactorRetrieveChannelStatistic(pReactor, pCommand->reactorChannel, &statistics, pErrorInfo)) != RSSL_RET_SUCCESS)
		{
			return ret;
		}

		cumulativeValue(&pCommand->channelStatistic.bytesRead, statistics.bytesRead);
		cumulativeValue(&pCommand->channelStatistic.uncompressedBytesRead, statistics.uncompressedBytesRead);
		cumulativeValue(&pCommand->channelStatistic.bytesWritten, statistics.bytesWritten);
		cumulativeValue(&pCommand->channelStatistic.uncompressedBytesWritten, statistics.uncompressedBytesWritten);
		cumulativeValue(&pCommand->channelStatistic.pingReceived, statistics.pingReceived);
		cumulativeValue(&pCommand->channelStatistic.pingSent, statistics.pingSent);

		//APIQA
		time_t currentTime;
		struct tm* time_info;
		char timeString[32];  // space for "HH:MM:SS\0"
		time(&currentTime);
		time_info = localtime(&currentTime);

		strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
		printf("\nStatistic Time: %s\n", timeString);
		//END APIQA

		printf("\nReactor channel statistic: Channel fd="SOCKET_PRINT_TYPE".\n", pCommand->reactorChannel->socketId);
		printf("\tBytes read : %llu\n", pCommand->channelStatistic.bytesRead);
		printf("\tUncompressed bytes read : %llu\n", pCommand->channelStatistic.uncompressedBytesRead);
		printf("\tBytes written : %llu\n", pCommand->channelStatistic.bytesWritten);
		printf("\tUncompressed bytes written : %llu\n", pCommand->channelStatistic.uncompressedBytesWritten);
		printf("\tPing received : %llu\n", pCommand->channelStatistic.pingReceived);
		printf("\tPing sent : %llu\n", pCommand->channelStatistic.pingSent);

		pCommand->nextStatisticRetrivalTime = currentTime + statisticInterval;
	}

	return ret;
}
