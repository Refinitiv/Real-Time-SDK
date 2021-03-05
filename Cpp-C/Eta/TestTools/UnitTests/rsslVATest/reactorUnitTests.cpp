/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslReactor.h"
#include "testFramework.h"
#include "gtest/gtest.h"
#include "rtr/rsslVAUtils.h"
#include "rtr/rsslNotifier.h"

#include "rtr/rsslQueue.h"
#include "rtr/rsslEventSignal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "rtr/rsslThread.h"
#include "rtr/rsslGetTime.h"

#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#define strtok_r strtok_s
#undef FD_SETSIZE
#define FD_SETSIZE 6400
#else
//#define FD_SETSIZE 6400
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

void time_sleep(int millisec)
{
#ifdef WIN32
	Sleep(millisec);
#else
	if (millisec)
	{
		struct timespec ts;
		ts.tv_sec = millisec / 1000;
		ts.tv_nsec = (millisec % 1000) * 1000000;
		nanosleep(&ts, NULL);
	}
#endif
}

#define MAX_REACTOR_CONS (FD_SETSIZE/3)

/* When we dispatch, we will copy whatever message we got from the callback
 * so we can verify it's what we expected. */

typedef enum
{
	MUT_MSG_NONE,
	MUT_MSG_CONN,
	MUT_MSG_RDM,
	MUT_MSG_RSSL
} MutMsgType;

/* Note: Keeping things simple for now -- using a struct and not a union for this */
typedef struct
{
	MutMsgType mutMsgType;
	RsslReactorChannelEvent channelEvent;
	RsslRDMMsg  rdmMsg;

	char memoryBlock[4000];
	RsslBuffer memoryBuffer;

	char rsslMsgBlock[4000];
	RsslBuffer rsslMsgBuffer;
	RsslMsg *pRsslMsg;

	RsslReactorChannel *pReactorChannel;

	/* Count for number of times the callback has been invoked */
	int msgCount;

} MutMsg;

void clearMutMsg(MutMsg *pMutMsg)
{
	pMutMsg->mutMsgType = MUT_MSG_NONE;
}


typedef struct
{
	RsslReactor *pReactor;
	MutMsg mutMsg;
	fd_set readFds, writeFds, exceptFds;
	RsslBool closeConnections; /* Automatically close connections when they go down(return RSSL_RC_CRET_CLOSE instead of RSSL_RC_CRET_SUCCESS) */
	RsslInt32 reconnectAttempts; /* For some callbacks, means an additional connection will be added in the callback */
	RsslRet previousDispatchRet;
	RsslNotifier *pNotifier; /* Notifier the test will use for this reactor's descriptors. If not set, it will use select() for notification. */
	RsslNotifierEvent *pReactorNotifierEvent; /* Notification for the Reactor's event queue. */

	RsslInt32 channelDownEventCount; /* Number of channel-down events received. */
	RsslInt32 channelDownReconnectingEventCount; /* Number of channel-down-reconnecting events received. */

	RsslInt32 numOfAuthTokenInfoCount; /* Number of authentication token received. */
	RsslInt32 numOfAuthTokenInfoErrorCount; /* Number of authentication token error received. */
	RsslInt32 authTokenInfoErrorTextLength; /* The length of the error message if any. */
	RsslUInt32 authEventStatusCode; /* The HTTP status code */

} MyReactor;

typedef struct
{
	MyReactor *pMyReactor;
	RsslReactorChannel *pReactorChannel;
	RsslBool isServer;

	/* Used by Multithreaded dispatch test */
	RsslInt32 msgsToSend;
	RsslInt32 msgsToRecv;

	RsslNotifierEvent *pNotifierEvent; /* Notification for the reactorChannel. */
} MyReactorChannel;

/*These copy of structures from Reactor structures need to be used for casting in reactorUnitTests_EventPoolSize
@ATTENTION! 
@These structures would be modified if the original (from Reactor) structures were modified!
*/
typedef struct 
{
	RsslQueue eventPool;
	RsslQueue eventQueue;
	RsslMutex eventPoolLock;
	RsslMutex eventQueueLock;
	void *pLastEvent;
	void *pParentGroup;
	RsslQueueLink readyEventQueueLink;
	RsslBool isInActiveEventQueueGroup;
}MyReactorEventQueue;

typedef struct 
{
	RsslQueue readyEventQueueGroup;
	RsslMutex lock;
	RsslEventSignal eventSignal;
}MyReactorEventQueueGroup;

typedef struct 
{
	RsslQueue initializingChannels;	
	RsslQueue activeChannels;			
	RsslQueue inactiveChannels;			
	RsslQueue reconnectingChannels;
	RsslQueue disposableRestHandles; 
	RsslNotifier *pNotifier; 
	RsslNotifierEvent *pQueueNotifierEvent;	
	RsslInt64 lastRecordedTimeMs;
	RsslThreadId thread;
	MyReactorEventQueue workerQueue;
}MyRsslReactorWorker;

typedef struct 
{
	RsslReactor reactor;							
	void *channelPoolArray;
	RsslQueue channelPool;				
	RsslQueue initializingChannels;	
	RsslQueue activeChannels;			
	RsslQueue inactiveChannels;			
	RsslQueue closingChannels;			
	RsslQueue reconnectingChannels;		
	RsslThreadId thread;
	MyReactorEventQueue reactorEventQueue; 
	RsslNotifier *pNotifier; 
	RsslNotifierEvent *pQueueNotifierEvent; 
	RsslBuffer memoryBuffer;
	RsslInt64 lastRecordedTimeMs;
	RsslInt32 channelCount;			
	RsslInt32 maxEventsInPool; 
	RsslMutex interfaceLock; 
	RsslBool inReactorFunction;
	MyReactorEventQueueGroup activeEventQueueGroup;
	MyRsslReactorWorker reactorWorker;			
}MyReactorImpl;


static RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent);
static RsslReactorCallbackRet defaultMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pInfo);
static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent *pInfo);
static RsslReactorCallbackRet authTokenEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorAuthTokenEvent *pAuthTokenEvent);
static RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pServiceEndpointEvent);
static RsslReactorCallbackRet serviceEndpointEventCallbackForError(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pServiceEndpointEvent);
static RsslRet dispatchEvent(MyReactor *pMyReactor, RsslUInt32 timeoutMsec);

void clearMyReactor(MyReactor *pMyReactor)
{
	memset(pMyReactor, 0, sizeof(MyReactor));
}

static MyReactor myReactors[2];
static RsslServer* pServer[2];
static MyReactor *pConsMon = &myReactors[0], *pProvMon = &myReactors[1];
static RsslReactorChannel *pConsCh[1], *pProvCh[1];

static void cleanupReactors(RsslBool sameReactor);

static void initReactors(RsslCreateReactorOptions *pOpts, RsslBool sameReactor);

static RsslErrorInfo rsslErrorInfo;
static RsslCreateReactorOptions mOpts;
static RsslBindOptions bindOpts[2];
static RsslReactorConnectOptions connectOpts[2];
static RsslReactorAcceptOptions acceptOpts;

static RsslReactorOMMConsumerRole ommConsumerRole;
static RsslReactorOMMProviderRole ommProviderRole;
static RsslReactorOMMNIProviderRole ommNIProviderRole;

static RsslEncodeIterator eIter;

static RsslRDMLoginRequest loginRequest;
static RsslRDMLoginRefresh loginRefresh;
static RsslRDMLoginStatus loginSuspectStatus;

static RsslRDMDirectoryRequest directoryRequest;
static RsslRDMDirectoryRefresh directoryRefresh;
static RsslBuffer fieldDictionaryName = { 6, const_cast<char*>("RWFFld") };
static RsslBuffer enumDictionaryName = { 7, const_cast<char*>("RWFEnum") };
static RsslBuffer dictionariesProvidedList[] = {{ 6, const_cast<char*>("RWFFld") }, { 7, const_cast<char*>("RWFEnum") }}; 
RsslUInt32 dictionariesProvidedCount = 2;
static RsslBuffer itemName = { 5, const_cast<char*>("IBM.N") };
static RsslRDMService directoryService;

static char enumDictionaryText[] =
	"!tag Filename    ENUMTYPE.001\n"
	"!tag Desc        Marketstream enumerated tables\n"
	"!tag RT_Version  4.00\n"
	"!tag DT_Version  12.11\n"
	"!tag Date        13-Aug-2010\n"
	"PRCTCK_1      14\n"
	"      0          \" \"   no tick\n"
	"      1         #DE#   up tick or zero uptick\n"
	"      2         #FE#   down tick or zero downtick\n"
	"      3          \" \"   unchanged tick\n";

static char fieldDictionaryText[] =
	"!tag Filename  RWF.DAT\n"
	"!tag Desc      RDFD RWF field set\n"
	"!tag Type      1\n"
	"!tag Version   4.00.11\n"
	"!tag Build     002\n"
	"!tag Date      17-Sep-2010\n"
	"PROD_PERM  \"PERMISSION\"             1  NULL        INTEGER             5  UINT64           2\n"
	"RDNDISPLAY \"DISPLAYTEMPLATE\"        2  NULL        INTEGER             3  UINT64           1\n";

static RsslDataDictionary dataDictionary;
static RsslRDMDictionaryRefresh fieldDictionaryRefresh, enumDictionaryRefresh;

static void reactorUnitTests_AutoMsgs(RsslConnectionTypes connectionType);
static void reactorUnitTests_Raise(RsslConnectionTypes connectionType);
static void reactorUnitTests_InitializationAndPingTimeout(RsslConnectionTypes connectionType);
static void reactorUnitTests_ShortPingInterval(RsslConnectionTypes connectionType);
static void reactorUnitTests_InvalidArguments(RsslConnectionTypes connectionType);
static void reactorUnitTests_BigDirectoryMsg(RsslConnectionTypes connectionType);

static void reactorUnitTests_DisconnectFromCallbacks(RsslConnectionTypes connectionType);
static void reactorUnitTests_AddConnectionFromCallbacks(RsslConnectionTypes connectionType);
static void reactorUnitTests_MultiThreadDispatch(RsslConnectionTypes connectionType);
#ifdef COMPILE_64BITS
static void reactorUnitTests_ManyConnections(RsslConnectionTypes connectionType);
static void reactorUnitTests_EventPoolSize(RsslConnectionTypes connectionType);
#endif
static void reactorUnitTests_WaitWhileChannelDown(RsslConnectionTypes connectionType);
static void reactorUnitTests_ReconnectAttemptLimit(RsslConnectionTypes connectionType);
static void reactorUnitTests_InvalidNetworkInterface(RsslConnectionTypes connectionType);

static void reactorUtilTest_ConnectDeepCopy(RsslConnectionTypes connectionType);

class ReactorUtilTest : public ::testing::TestWithParam<RsslConnectionTypes>  {
public:

	static void SetUpTestCase()
	{
		RsslError rsslError;
		RsslBuffer errorText = { 255, (char*)alloca(255) };
		rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);

		rsslClearBindOpts(&bindOpts[0]);
		bindOpts[0].serviceName = const_cast<char*>("14009");
		ASSERT_TRUE((pServer[0] = rsslBind(&bindOpts[0], &rsslErrorInfo.rsslError)));

		rsslClearBindOpts(&bindOpts[1]);
		bindOpts[1].serviceName = const_cast<char*>("15000");
		bindOpts[1].wsOpts.protocols = const_cast<char*>("rssl.json.v2");
		ASSERT_TRUE((pServer[1] = rsslBind(&bindOpts[1], &rsslErrorInfo.rsslError)));

		rsslClearDataDictionary(&dataDictionary);
		createFileFromString("tmp_dictionary.txt", enumDictionaryText, sizeof(enumDictionaryText));
		ASSERT_TRUE(rsslLoadEnumTypeDictionary("tmp_dictionary.txt", &dataDictionary, &errorText) == RSSL_RET_SUCCESS);

		createFileFromString("tmp_dictionary.txt", fieldDictionaryText, sizeof(fieldDictionaryText));
		ASSERT_TRUE(rsslLoadFieldDictionary("tmp_dictionary.txt", &dataDictionary, &errorText) == RSSL_RET_SUCCESS);

		deleteFile("tmp_dictionary.txt");

		rsslClearCreateReactorOptions(&mOpts);
		initReactors(&mOpts, RSSL_FALSE);
	}

	static void TearDownTestCase()
	{
		cleanupReactors(RSSL_FALSE);

		ASSERT_TRUE(rsslCloseServer(pServer[0], &rsslErrorInfo.rsslError) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslCloseServer(pServer[1], &rsslErrorInfo.rsslError) == RSSL_RET_SUCCESS);
		rsslDeleteDataDictionary(&dataDictionary);
		rsslUninitialize();
	}
};

TEST_P(ReactorUtilTest, ConnectDeepCopy)
{
	reactorUtilTest_ConnectDeepCopy(GetParam());
}

TEST_P(ReactorUtilTest, MultiThreadDispatch)
{
	if(GetParam() != RSSL_CONN_TYPE_WEBSOCKET)
		reactorUnitTests_MultiThreadDispatch(GetParam());
}

TEST_P(ReactorUtilTest, AddConnectionFromCallbacks)
{
	reactorUnitTests_AddConnectionFromCallbacks(GetParam());
}

TEST_P(ReactorUtilTest, BigDirectoryMsg)
{
	reactorUnitTests_BigDirectoryMsg(GetParam());
}

TEST_P(ReactorUtilTest, DisconnectFromCallbacks)
{
	reactorUnitTests_DisconnectFromCallbacks(GetParam());
}

TEST_P(ReactorUtilTest, InvalidArguments)
{
	reactorUnitTests_InvalidArguments(GetParam());
}

TEST_P(ReactorUtilTest, InitializationAndPingTimeout)
{
	reactorUnitTests_InitializationAndPingTimeout(GetParam());
}

TEST_P(ReactorUtilTest, ShortPingInterval)
{
	if (GetParam() != RSSL_CONN_TYPE_WEBSOCKET)
		reactorUnitTests_ShortPingInterval(GetParam());
}

TEST_P(ReactorUtilTest, AutoMsgs)
{
	reactorUnitTests_AutoMsgs(GetParam());
}

TEST_P(ReactorUtilTest, Raise)
{
	reactorUnitTests_Raise(GetParam());
}

TEST_P(ReactorUtilTest, WaitWhileChannelDown)
{
	reactorUnitTests_WaitWhileChannelDown(GetParam());
}

TEST_P(ReactorUtilTest, ReconnectAttemptLimit)
{
	reactorUnitTests_ReconnectAttemptLimit(GetParam());
}

#ifdef COMPILE_64BITS
TEST_P(ReactorUtilTest, ManyConnections)
{
	reactorUnitTests_ManyConnections(GetParam());
}
TEST_P(ReactorUtilTest, EventQueuePerformance)
{
	reactorUnitTests_EventPoolSize(GetParam());
}
#endif

TEST_P(ReactorUtilTest, UnreachableAddress)
{
	reactorUnitTests_InvalidNetworkInterface(GetParam());
}

INSTANTIATE_TEST_CASE_P(
	TestingReactorUtilTests,
	ReactorUtilTest,
	::testing::Values(
		RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET
	));


class ReactorSessionMgntTest : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		RsslError rsslError;
		RsslBuffer errorText = { 255, (char*)alloca(255) };
		rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);

		connectInfoCount = 3;
	}

	static void TearDownTestCase()
	{
		rsslUninitialize();
	}
protected:

	void SetUp()
	{
		int i = 0;
		rsslClearReactorAcceptOptions(&_reactorAcceptOpts);
		rsslClearReactorConnectOptions(&_reactorConnectionOpts);
		rsslClearRDMLoginRequest(&_rdmLoginRequest);
		rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
		rsslClearOMMConsumerRole(&_reactorOmmConsumerRole);
		rsslClearOMMProviderRole(&_reactorOmmProviderRole);
		pConsMon->numOfAuthTokenInfoCount = 0;
		pConsMon->numOfAuthTokenInfoErrorCount = 0;
		pConsMon->authTokenInfoErrorTextLength = 0;
		pConsMon->channelDownReconnectingEventCount = 0;
		
		for(;i < connectInfoCount; i++)
		{
			rsslClearReactorConnectInfo(&_reactorConnectInfo[i]);
			_reactorConnectInfo[i].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
			_reactorConnectInfo[i].rsslConnectOptions.majorVersion = RSSL_RWF_MINOR_VERSION;
			_reactorConnectInfo[i].rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
			_reactorConnectInfo[i].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
		}

		_reactorConnectionOpts.reconnectAttemptLimit = -1;
		_reactorConnectionOpts.reconnectMinDelay = 500;
		_reactorConnectionOpts.reconnectMaxDelay = 3000;

		_reactorOmmConsumerRole.base.channelEventCallback = channelEventCallback;
		_reactorOmmConsumerRole.base.defaultMsgCallback = defaultMsgCallback;
		_reactorOmmConsumerRole.loginMsgCallback = loginMsgCallback;

		rsslInitDefaultRDMLoginRequest(&_rdmLoginRequest, 1);
		_reactorOmmConsumerRole.base.roleType = RSSL_RC_RT_OMM_CONSUMER;

		_reactorOmmProviderRole.base.channelEventCallback = channelEventCallback;
		_reactorOmmProviderRole.base.defaultMsgCallback = defaultMsgCallback;
		cleanupReactor = RSSL_TRUE; // Cleaning up the reactor by default
	}

	void TearDown()
	{
		if (cleanupReactor)
		{
			cleanupReactors(RSSL_TRUE);
		}
	}

	RsslReactorAcceptOptions _reactorAcceptOpts;
	RsslReactorConnectOptions _reactorConnectionOpts;
	static int connectInfoCount;
	RsslReactorConnectInfo _reactorConnectInfo[3];
	RsslReactorOMMConsumerRole _reactorOmmConsumerRole;
	RsslReactorOMMProviderRole _reactorOmmProviderRole;
	RsslRDMLoginRequest _rdmLoginRequest;
	RsslReactorOAuthCredential _reactorOAuthCredential;
	RsslConsumerWatchlistOptions _watchlistOptions;
	RsslBool cleanupReactor;

};

int ReactorSessionMgntTest::connectInfoCount = 3;

TEST_F(ReactorSessionMgntTest, UnauthorizedUser)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName.data = const_cast<char*>("invalid_user");
	_rdmLoginRequest.userName.length = (RsslUInt32)strlen(_rdmLoginRequest.userName.data);

	_rdmLoginRequest.password.data = const_cast<char*>("invalid_password");
	_rdmLoginRequest.password.length = (RsslUInt32)strlen(_rdmLoginRequest.password.data);

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Failed to request authentication token information with HTTP error 400. Text: {\"error\":\"access_denied\"  ,\"error_description\":\"Invalid username or password.\" } ");
 
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Failed to request authentication token information with HTTP error 400. Text: {\"error\":\"access_denied\"  ,\"error_description\":\"Invalid username or password.\" } ");
}

TEST_F(ReactorSessionMgntTest, NoOAuthCredentialForEnablingSessionMgnt)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "There is no user credential available for enabling session management.");
}

TEST_F(ReactorSessionMgntTest, NoUserNameForEnablingSessionMgnt)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Failed to copy OAuth credential for enabling the session management; OAuth user name does not exist.");
}

TEST_F(ReactorSessionMgntTest, NoClientIdForEnablingSessionMgnt)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Failed to copy OAuth credential for enabling the session management; OAuth Client ID does not exist.");
}

RsslReactorCallbackRet oAuthCredentialEventCallback(RsslReactor *pReactor, RsslReactorOAuthCredentialEvent* pOAuthCredentialEvent)
{
	return RSSL_RC_CRET_SUCCESS;
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_Callback)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.pOAuthCredentialEventCallback = NULL;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The RsslReactorOAuthCredentialEventCallback of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_NULL_Callback)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.pOAuthCredentialEventCallback = NULL;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The RsslReactorOAuthCredentialEventCallback of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_ClientSecret)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.clientSecret = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.clientSecret = g_password;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The Client secret of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_ClientID)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_password;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The Client ID of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_Password)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_userName;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The password of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_TokenScope)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.tokenScope = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The token scope of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, MultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pOAuthCredential = NULL;
	rsslClearRDMLoginRequest(&_rdmLoginRequest);
	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	_reactorOmmConsumerRole.pLoginRequest = NULL;
	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);
	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.takeExclusiveSignOnControl = RSSL_FALSE;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The takeExclusiveSignOnControl of RsslReactorOAuthCredential is not equal for the same token session.");
}

TEST_F(ReactorSessionMgntTest, NoPasswordForEnablingSessionMgnt)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	rsslClearReactorOAuthCredential(&_reactorOAuthCredential);

	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Failed to copy OAuth credential for enabling the session management; OAuth Password does not exist.");
}

TEST_F(ReactorSessionMgntTest, UsingOmmNiProviderRoleForSessionMgnt)
{
	RsslReactorOMMNIProviderRole reactorOmmNIProviderRole;
	rsslClearOMMNIProviderRole(&reactorOmmNIProviderRole);
	reactorOmmNIProviderRole.base.channelEventCallback = channelEventCallback;
	reactorOmmNIProviderRole.base.defaultMsgCallback = defaultMsgCallback;
	reactorOmmNIProviderRole.loginMsgCallback = loginMsgCallback;
	reactorOmmNIProviderRole.pLoginRequest = &_rdmLoginRequest;

	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&reactorOmmNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The session management supports only on the RSSL_RC_RT_OMM_CONSUMER role type.");
}

TEST_F(ReactorSessionMgntTest, EmptyAuthTokenServiceURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.tokenServiceURL.data = const_cast<char*>("");
	mOpts.tokenServiceURL.length = 0;
	cleanupReactor = RSSL_FALSE;
	ASSERT_TRUE(rsslCreateReactor(&mOpts, &rsslErrorInfo) == NULL);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The token service URL is not available.");
}

TEST_F(ReactorSessionMgntTest, InvalidAuthTokenServiceURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.tokenServiceURL.data = const_cast<char*>("https://#######@_@/token");
	mOpts.tokenServiceURL.length = (RsslUInt32)strlen(mOpts.tokenServiceURL.data);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_TRUE(strstr(rsslErrorInfo.rsslError.text, "Error: Failed to perform the request") != NULL);

	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_TRUE(strstr(rsslErrorInfo.rsslError.text, "Error: Failed to perform the request") != NULL);
}

TEST_F(ReactorSessionMgntTest, EmptyServiceDiscoveryURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.serviceDiscoveryURL.data = const_cast<char*>("");
	mOpts.serviceDiscoveryURL.length = 0;
	cleanupReactor = RSSL_FALSE;
	ASSERT_TRUE(rsslCreateReactor(&mOpts, &rsslErrorInfo) == NULL);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "The service discovery URL is not available.");
}

TEST_F(ReactorSessionMgntTest, InvalidServiceDiscoveryURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.serviceDiscoveryURL.data = const_cast<char*>("https://#######@_@/streaming/pricing/v1");
	mOpts.serviceDiscoveryURL.length = (RsslUInt32)strlen(mOpts.serviceDiscoveryURL.data);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_TRUE(strstr(rsslErrorInfo.rsslError.text, "Failed to perform the request") != NULL);

	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
	ASSERT_TRUE(strstr(rsslErrorInfo.rsslError.text, "Failed to perform the request") != NULL);
}

TEST_F(ReactorSessionMgntTest, InvalidConnectionType)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts,(RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Invalid connection type(0) for requesting RDP service discovery.");
        
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts,(RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Invalid connection type(0) for requesting RDP service discovery.");
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_usingDefaultLocation)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon,500) >= RSSL_RET_SUCCESS); 
	ASSERT_TRUE(dispatchEvent(pConsMon,800) >= RSSL_RET_SUCCESS); 
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && (pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP));

	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_usingDefaultLocation_with_RsslReactorOAuthCredential)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;

	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 800) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 800) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && (pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP));

	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_usingDefaultLocation_with_RsslReactorOAuthCredential_WebSocket)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_WEBSOCKET;
	_reactorConnectInfo[0].rsslConnectOptions.wsOpts.protocols = const_cast<char*>("tr_json2");

	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;

	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 800) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 800) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && (pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP));

	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_usingDefaultLocation_with_InvalidRsslRDMLoginRequest_and_RsslReactorOAuthCredential)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName.data = const_cast<char*>("invalid_user");
	_rdmLoginRequest.userName.length = (RsslUInt32)strlen(_rdmLoginRequest.userName.data);
	_rdmLoginRequest.password.data = const_cast<char*>("invalid_password");
	_rdmLoginRequest.password.length = (RsslUInt32)strlen(_rdmLoginRequest.password.data);
	_reactorOmmConsumerRole.clientId = _rdmLoginRequest.userName;

	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 500) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 500) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && (pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP));

	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_SpecifiedLocation)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[0].location.data = (char*)"eu-west";
	_reactorConnectInfo[0].location.length = 7;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 400) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 1000) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectSuccessWithOneConnection_AuthTokenEventCallback)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[0].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOmmConsumerRole.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 200);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 1);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 0);

	/* Checks for the Channel up event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 500) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 700) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 700) >= RSSL_RET_SUCCESS);

	ASSERT_TRUE((pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP || pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY));
	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted_UnauthorizedUser)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = const_cast<char*>("invalid_user");
	_rdmLoginRequest.userName.length = (RsslUInt32)strlen(_rdmLoginRequest.userName.data);

	_rdmLoginRequest.password.data = const_cast<char*>("invalid_password");
	_rdmLoginRequest.password.length = (RsslUInt32)strlen(_rdmLoginRequest.password.data);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidlocalhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");

	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) >= RSSL_RET_SUCCESS);

	time_sleep(300);
	ASSERT_TRUE(dispatchEvent(pConsMon, 300) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 10000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 400);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 0);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 1);
	ASSERT_TRUE(pConsMon->authTokenInfoErrorTextLength > 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
    ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted_InvalidAuthTokenServiceURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.tokenServiceURL.data = const_cast<char*>("https://#######@_@/token");
	mOpts.tokenServiceURL.length = (RsslUInt32)strlen(mOpts.tokenServiceURL.data);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = g_userName.data;
	_rdmLoginRequest.userName.length = g_userName.length;

	_rdmLoginRequest.password.data = g_password.data;
	_rdmLoginRequest.password.length = g_password.length;

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");

	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	time_sleep(100);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount >= 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 8000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 0); /* No HTTP response status code as the URL is invalid*/
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 0);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 1);
	ASSERT_TRUE(pConsMon->authTokenInfoErrorTextLength > 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted_InvalidServiceDiscoveryURL)
{
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.serviceDiscoveryURL.data = const_cast<char*>("https://#######@_@/streaming/pricing/v1");
	mOpts.serviceDiscoveryURL.length = (RsslUInt32)strlen(mOpts.serviceDiscoveryURL.data);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = g_userName.data;
	_rdmLoginRequest.userName.length = g_userName.length;

	_rdmLoginRequest.password.data = g_password.data;
	_rdmLoginRequest.password.length = g_password.length;

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");

	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	time_sleep(100);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount >= 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 8000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 200);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 1);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = g_userName.data;
	_rdmLoginRequest.userName.length = g_userName.length;

	_rdmLoginRequest.password.data = g_password.data;
	_rdmLoginRequest.password.length = g_password.length;
 
	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");
        
	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	time_sleep(100);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 10000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 5000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 200);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 1);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP ||
			pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted_WebSocket)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = g_userName.data;
	_rdmLoginRequest.userName.length = g_userName.length;

	_rdmLoginRequest.password.data = g_password.data;
	_rdmLoginRequest.password.length = g_password.length;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");

	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_WEBSOCKET;
	_reactorConnectInfo[1].rsslConnectOptions.wsOpts.protocols = const_cast<char*>("tr_json2");


	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	time_sleep(100);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 10000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 5000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 200);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 1);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
}

TEST_F(ReactorSessionMgntTest, ConnectionRecoveryFromSocketToEncrypted_for_RsslReactorOAthCredential_and_ClientSecret)
{
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_rdmLoginRequest.userName.data = const_cast<char*>("invalid_user");
	_rdmLoginRequest.userName.length = (RsslUInt32)strlen(_rdmLoginRequest.userName.data);
	_rdmLoginRequest.password.data = const_cast<char*>("invalid_password");
	_rdmLoginRequest.password.length = (RsslUInt32)strlen(_rdmLoginRequest.password.data);
	_reactorOmmConsumerRole.clientId = _rdmLoginRequest.userName;

	_reactorOAuthCredential.userName = g_userName;
	_reactorOAuthCredential.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;
	_reactorOAuthCredential.clientSecret.data = const_cast<char*>("ABCDEFG");
	_reactorOAuthCredential.clientSecret.length = (RsslUInt32)strlen(_reactorOAuthCredential.clientSecret.data);

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;
	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	_reactorConnectInfo[0].enableSessionManagement = RSSL_FALSE;
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalidhost");
	_reactorConnectInfo[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15009");

	_reactorConnectInfo[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[1].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[1].enableSessionManagement = RSSL_TRUE;
	_reactorConnectInfo[1].pAuthTokenEventCallback = authTokenEventCallback;

	_reactorConnectionOpts.connectionCount = 2;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;

	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 0);
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	time_sleep(100);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Ensure that the first connection is down */
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 1);

	/* Wait until the connection is switched successfully to the encrypted connection */
	ASSERT_TRUE(dispatchEvent(pConsMon, 10000) >= RSSL_RET_SUCCESS);
	time_sleep(2000);
	ASSERT_TRUE(dispatchEvent(pConsMon, 3000) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(dispatchEvent(pConsMon, 5000) >= RSSL_RET_SUCCESS);

	/* Check for token information */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 200);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 1);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 0);

	ASSERT_TRUE(pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP ||
		pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
}

TEST_F(ReactorSessionMgntTest, MultipleOpenAndCloseConnections)
{
	RsslUInt32 numberOfConnections = 25; /* To ensure that this test reuse the ReactorChannel from the channel pool(size = 10)*/
	RsslUInt32 index = 0;
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;
	_reactorOAuthCredential.clientId = g_userName;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.pOAuthCredential = &_reactorOAuthCredential;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	for (; index < numberOfConnections; index++)
	{
		ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Checks for the Channel up event */
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
	}
}

TEST_F(ReactorSessionMgntTest, MultipleOpenAndCloseConnections_UsingOnly_RsslRMDLoginRequest)
{
	RsslUInt32 numberOfConnections = 15; /* To ensure that this test reuse the ReactorChannel from the channel pool(size = 10)*/
	RsslUInt32 index = 0;
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
#ifdef _WIN32 // Use OPENSSL for creating the encrypted connection instead of the legacy WinInet-based protocol
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	for (; index < numberOfConnections; index++)
	{
		ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Checks for the Channel up event */
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		time_sleep(100);
	}
}

TEST_F(ReactorSessionMgntTest, MultipleOpenAndCloseConnections_UsingOnly_RsslRMDLoginRequest_WebSocket)
{
	RsslUInt32 numberOfConnections = 15; /* To ensure that this test reuse the ReactorChannel from the channel pool(size = 10)*/
	RsslUInt32 index = 0;
	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	_reactorConnectInfo[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	_reactorConnectInfo[0].rsslConnectOptions.encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_WEBSOCKET;
	_reactorConnectInfo[0].rsslConnectOptions.wsOpts.protocols = const_cast<char*>("tr_json2");

	_reactorConnectInfo[0].enableSessionManagement = RSSL_TRUE;

	_reactorConnectionOpts.connectionCount = 1;
	_reactorConnectionOpts.reactorConnectionList = &_reactorConnectInfo[0];

	_rdmLoginRequest.userName = g_userName;
	_rdmLoginRequest.password = g_password;

	_reactorOmmConsumerRole.pLoginRequest = &_rdmLoginRequest;
	_reactorOmmConsumerRole.clientId = g_userName;
	_reactorOmmConsumerRole.watchlistOptions.enableWatchlist = RSSL_FALSE;

	for (; index < numberOfConnections; index++)
	{
		ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &_reactorConnectionOpts, (RsslReactorChannelRole*)&_reactorOmmConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Checks for the Channel up event */
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		ASSERT_TRUE(rsslReactorCloseChannel(pConsMon->pReactor, pConsMon->mutMsg.pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
		dispatchEvent(pConsMon, 800);
		dispatchEvent(pConsMon, 800);
		time_sleep(100);
	}
}

TEST_F(ReactorSessionMgntTest, SubmitInvalidUserCredentials_without_tokensession)
{
	RsslReactorOAuthCredentialRenewalOptions renewalOptions;
	RsslReactorOAuthCredentialRenewal credentialRenewal;

	rsslClearCreateReactorOptions(&mOpts);
	initReactors(&mOpts, RSSL_TRUE);

	rsslClearReactorOAuthCredentialRenewalOptions(&renewalOptions);

	renewalOptions.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;
	renewalOptions.pAuthTokenEventCallback = authTokenEventCallback;

	rsslClearReactorOAuthCredentialRenewal(&credentialRenewal);
	credentialRenewal.userName.data = const_cast<char*>("invalid");
	credentialRenewal.userName.length = (RsslUInt32)strlen(credentialRenewal.userName.data);
	credentialRenewal.password = credentialRenewal.userName;
	credentialRenewal.clientId = credentialRenewal.userName;

	ASSERT_TRUE(rsslReactorSubmitOAuthCredentialRenewal(pConsMon->pReactor, &renewalOptions, &credentialRenewal, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	dispatchEvent(pConsMon, 10000);

	/* Check for token information from the callback */
	ASSERT_TRUE(pConsMon->authEventStatusCode == 401);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoCount == 0);
	ASSERT_TRUE(pConsMon->numOfAuthTokenInfoErrorCount == 1);
	ASSERT_TRUE(pConsMon->authTokenInfoErrorTextLength > 0);
}

struct ReactorServiceDiscoveryEndpointResult
{
	RsslUInt32 NumServiceEndpointInfo;
	RsslUInt32 Num_rwf_DataFormat;
	RsslUInt32 Num_tr_json2_DataFormat;
	RsslUInt32 Num_apac_endpoints;
	RsslUInt32 Num_emea_endpoints;
	RsslUInt32 Num_amer_endpoints;
	RsslUInt32 Num_us_east_locations;
	RsslUInt32 Num_eu_west_locations;
	RsslUInt32 Num_ap_se_locations;
	RsslUInt32 Num_14002_port;
	RsslUInt32 Num_443_port;
	RsslUInt32 Num_aws_provider;
	RsslUInt32 Num_tcp_transport;
	RsslUInt32 Num_websocket_transport;
	RsslUInt32 HttpStatusCode;
};

struct ReactorServiceDiscoveryEndpointError
{
	RsslUInt32 HttpStatusCode;
	char* expectedText;
};

void rsslClearReactorServiceDiscoveryEndpointResult(ReactorServiceDiscoveryEndpointResult *result)
{
	memset(result, 0, sizeof(ReactorServiceDiscoveryEndpointResult));
}

RsslBool validateServiceDiscoveryEndpoints(RsslReactorServiceEndpointEvent *event, ReactorServiceDiscoveryEndpointResult *expectedResult)
{
	ReactorServiceDiscoveryEndpointResult actualResult;
	RsslReactorServiceEndpointInfo *pReactorServiceEndpointInfo;
	RsslUInt32 serviceInfoIndex;
	RsslUInt32 dataFormatIndex;
	RsslUInt32 locationIndex;

	rsslClearReactorServiceDiscoveryEndpointResult(&actualResult);
	actualResult.NumServiceEndpointInfo = event->serviceEndpointInfoCount;
	actualResult.HttpStatusCode = event->statusCode;

	for (serviceInfoIndex = 0; serviceInfoIndex < event->serviceEndpointInfoCount; serviceInfoIndex++)
	{
		pReactorServiceEndpointInfo = &event->serviceEndpointInfoList[serviceInfoIndex];

		for (dataFormatIndex = 0; dataFormatIndex < pReactorServiceEndpointInfo->dataFormatCount; dataFormatIndex++)
		{
			if (strncmp("rwf", pReactorServiceEndpointInfo->dataFormatList[dataFormatIndex].data, 3) == 0) {
				actualResult.Num_rwf_DataFormat++;
			}
			else if (strncmp("tr_json2", pReactorServiceEndpointInfo->dataFormatList[dataFormatIndex].data, 8) == 0) {
				actualResult.Num_tr_json2_DataFormat++;
			}
		}

		if (strncmp("amer-", pReactorServiceEndpointInfo->endPoint.data, 5) == 0) {
			actualResult.Num_amer_endpoints++;
		}
		else if (strncmp("emea-", pReactorServiceEndpointInfo->endPoint.data, 5) == 0) {
			actualResult.Num_emea_endpoints++;
		}
		else if (strncmp("apac-", pReactorServiceEndpointInfo->endPoint.data, 5) == 0) {
			actualResult.Num_apac_endpoints++;
		}

		for (locationIndex = 0; locationIndex < pReactorServiceEndpointInfo->locationCount; locationIndex++)
		{
			if (strncmp("us-east", pReactorServiceEndpointInfo->locationList[locationIndex].data, 7) == 0) {
				actualResult.Num_us_east_locations++;
			}
			else if (strncmp("eu-west", pReactorServiceEndpointInfo->locationList[locationIndex].data, 7) == 0) {
				actualResult.Num_eu_west_locations++;
			}
			else if (strncmp("ap-southeast", pReactorServiceEndpointInfo->locationList[locationIndex].data, 12) == 0) {
				actualResult.Num_ap_se_locations++;
			}

		}

		if (strncmp("14002", pReactorServiceEndpointInfo->port.data, 5) == 0) {
			actualResult.Num_14002_port++;
		}
		else if (strncmp("443", pReactorServiceEndpointInfo->port.data, 3) == 0) {
			actualResult.Num_443_port++;
		}

		if (strncmp("aws", pReactorServiceEndpointInfo->provider.data, 3) == 0) {
			actualResult.Num_aws_provider++;
		}

		if (strncmp("tcp", pReactorServiceEndpointInfo->transport.data, 3) == 0) {
			actualResult.Num_tcp_transport++;
		}
		else if (strncmp("websocket", pReactorServiceEndpointInfo->transport.data, 9) == 0) {
			actualResult.Num_websocket_transport++;
		}
	}

	if (expectedResult->NumServiceEndpointInfo > actualResult.NumServiceEndpointInfo)
		return RSSL_FALSE;

	if (expectedResult->Num_14002_port > actualResult.Num_14002_port)
		return RSSL_FALSE;

        if (expectedResult->Num_443_port > actualResult.Num_443_port)
		return RSSL_FALSE;

	if(expectedResult->Num_amer_endpoints > actualResult.Num_amer_endpoints)
		return RSSL_FALSE;

	if(expectedResult->Num_emea_endpoints > actualResult.Num_emea_endpoints)
		return RSSL_FALSE;
 
	if(expectedResult->Num_aws_provider > actualResult.Num_aws_provider)
		return RSSL_FALSE;

        if (expectedResult->Num_eu_west_locations > actualResult.Num_eu_west_locations)
		return RSSL_FALSE;

        if (expectedResult->Num_us_east_locations > actualResult.Num_us_east_locations)
		return RSSL_FALSE;

        if (expectedResult->Num_rwf_DataFormat > actualResult.Num_rwf_DataFormat)
		return RSSL_FALSE;

        if (expectedResult->Num_tr_json2_DataFormat > actualResult.Num_tr_json2_DataFormat)
		return RSSL_FALSE;

        if (expectedResult->Num_tcp_transport > actualResult.Num_tcp_transport)
		return RSSL_FALSE;
 
        if (expectedResult->Num_websocket_transport > actualResult.Num_websocket_transport)
		return RSSL_FALSE; 

	return RSSL_TRUE;
}

class ReactorQueryServiceDiscoveryTest : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		RsslError rsslError;
		RsslBuffer errorText = { 255, (char*)alloca(255) };
		rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);
	}

	static void TearDownTestCase()
	{
		rsslUninitialize();
	}

protected:

	void TearDown()
	{
		ASSERT_TRUE(rsslDestroyReactor(_pReactor, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	}

	RsslReactor *_pReactor;
	RsslReactorServiceDiscoveryOptions _reactorServiceDiscoveryOpts;
};

TEST_F(ReactorQueryServiceDiscoveryTest, RsslReactorServiceDiscoveryOptions_UserName_NotSpecified)
{
	rsslClearCreateReactorOptions(&mOpts);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);
	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ("RsslReactorServiceDiscoveryOptions.userName not provided.", rsslErrorInfo.rsslError.text);
}

TEST_F(ReactorQueryServiceDiscoveryTest, RsslReactorServiceDiscoveryOptions_Password_NotSpecified)
{
	rsslClearCreateReactorOptions(&mOpts);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ("RsslReactorServiceDiscoveryOptions.password not provided.", rsslErrorInfo.rsslError.text);
}

TEST_F(ReactorQueryServiceDiscoveryTest, RsslReactorServiceDiscoveryOptions_ClientId_NotSpecified)
{
	rsslClearCreateReactorOptions(&mOpts);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ("RsslReactorServiceDiscoveryOptions.clientId not provided.", rsslErrorInfo.rsslError.text);
}

TEST_F(ReactorQueryServiceDiscoveryTest, RsslReactorServiceDiscoveryOptions_Callback_NotSpecified)
{
	rsslClearCreateReactorOptions(&mOpts);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ("RsslReactorServiceDiscoveryOptions.pServiceEndpointEventCallback not provided.", rsslErrorInfo.rsslError.text);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetAllServiceEndpoints)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 14;
	expectedResult.Num_14002_port = 7;
	expectedResult.Num_443_port = 7;
	expectedResult.Num_apac_endpoints = 6;
	expectedResult.Num_amer_endpoints = 6;
	expectedResult.Num_emea_endpoints = 2;
	expectedResult.Num_aws_provider = 14;
	expectedResult.Num_eu_west_locations = 4;
	expectedResult.Num_us_east_locations = 8;
	expectedResult.Num_ap_se_locations = 8;
	expectedResult.Num_rwf_DataFormat = 7;
	expectedResult.Num_tr_json2_DataFormat = 7;
	expectedResult.Num_tcp_transport = 7;
	expectedResult.Num_websocket_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;

	// Checking the service discovery response in the serviceEndpointEventCallback
	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

 	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsFor_TcpTransport)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_14002_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_rwf_DataFormat = 7;
	expectedResult.Num_tcp_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_TCP;

	// Checking the service discovery response in the serviceEndpointEventCallback
	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsFor_RwfDataformat)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_14002_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_rwf_DataFormat = 7;
	expectedResult.Num_tcp_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.dataFormat = RSSL_RD_DP_RWF;

	// Checking the service discovery response in the serviceEndpointEventCallback
	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsFor_WebsocketTransport)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_443_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_tr_json2_DataFormat = 7;
	expectedResult.Num_websocket_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;

	// Checking the service discovery response in the serviceEndpointEventCallback
	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsFor_Tr_json2Dataformat)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_443_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_tr_json2_DataFormat = 7;
	expectedResult.Num_websocket_transport = 7; 
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.dataFormat = RSSL_RD_DP_JSON2;

	// Checking the service discovery response in the serviceEndpointEventCallback
	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_tokenScope_ClientSecret)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_443_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_tr_json2_DataFormat = 7;
	expectedResult.Num_websocket_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.clientSecret.length = 5;
	_reactorServiceDiscoveryOpts.clientSecret.data = const_cast<char*>("ABCDE");
	_reactorServiceDiscoveryOpts.tokenScope.length = 5;
	_reactorServiceDiscoveryOpts.tokenScope.data = const_cast<char*>("trapi");
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_empty_tokenscope)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	expectedResult.NumServiceEndpointInfo = 7;
	expectedResult.Num_443_port = 7;
	expectedResult.Num_apac_endpoints = 3;
	expectedResult.Num_amer_endpoints = 3;
	expectedResult.Num_emea_endpoints = 1;
	expectedResult.Num_aws_provider = 7;
	expectedResult.Num_eu_west_locations = 2;
	expectedResult.Num_us_east_locations = 4;
	expectedResult.Num_ap_se_locations = 4;
	expectedResult.Num_tr_json2_DataFormat = 7;
	expectedResult.Num_websocket_transport = 7;
	expectedResult.HttpStatusCode = 200;

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.tokenScope.length = 0; 
	_reactorServiceDiscoveryOpts.tokenScope.data = const_cast<char*>(""); /* the Reactor uses the default scope instead */
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidCombination_TCP_JSON2)
{
	ReactorServiceDiscoveryEndpointError expectedError;
	expectedError.expectedText = (char*)"Operation found no data";
	expectedError.HttpStatusCode = 404;

	rsslClearCreateReactorOptions(&mOpts);

	mOpts.userSpecPtr = &expectedError;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_TCP;
	_reactorServiceDiscoveryOpts.dataFormat = RSSL_RD_DP_JSON2;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallbackForError;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidCombination_WEBSOCKET_RWF)
{
	ReactorServiceDiscoveryEndpointError expectedError;
	expectedError.expectedText = (char*)"Operation found no data";
	expectedError.HttpStatusCode = 404;

	rsslClearCreateReactorOptions(&mOpts);

	mOpts.userSpecPtr = &expectedError;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;
	_reactorServiceDiscoveryOpts.dataFormat = RSSL_RD_DP_RWF;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallbackForError;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidTransportProtocol)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.transport = (RsslReactorDiscoveryTransportProtocol)99;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Invalid transport protocol type 99.");
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidDataformatProtocol)
{
	ReactorServiceDiscoveryEndpointResult expectedResult;

	rsslClearReactorServiceDiscoveryEndpointResult(&expectedResult);
	rsslClearCreateReactorOptions(&mOpts);

	mOpts.userSpecPtr = &expectedResult;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;
	_reactorServiceDiscoveryOpts.dataFormat = (RsslReactorDiscoveryDataFormatProtocol)88;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallback;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_STREQ(rsslErrorInfo.rsslError.text, "Invalid dataformat protocol type 88.");
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidClientID)
{
	ReactorServiceDiscoveryEndpointError expectedError;
	expectedError.expectedText = (char*)"clientId field is invalid";
	expectedError.HttpStatusCode = 401;
	RsslBuffer clientId = { 17, (char*)"invalid_clientID" };

	rsslClearCreateReactorOptions(&mOpts);

	mOpts.userSpecPtr = &expectedError;
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = clientId;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallbackForError;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidTokenServiceUrl)
{
	rsslClearCreateReactorOptions(&mOpts);

	mOpts.tokenServiceURL.data = (char *)"https://abc.edp.refinitiv.com/auth/oauth2/beta1/token/xx1";
	mOpts.tokenServiceURL.length = (RsslUInt32)strlen(mOpts.tokenServiceURL.data);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallbackForError;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_FAILURE);
}

TEST_F(ReactorQueryServiceDiscoveryTest, GetServiceEndpointsWith_InvalidServiceDiscoveryUrl)
{
	rsslClearCreateReactorOptions(&mOpts);

	mOpts.serviceDiscoveryURL.data = (char *)"https://abc.edp.refinitiv.com/streaming/pricing/xx1";
	mOpts.serviceDiscoveryURL.length = (RsslUInt32)strlen(mOpts.serviceDiscoveryURL.data);
	_pReactor = rsslCreateReactor(&mOpts, &rsslErrorInfo);

	rsslClearReactorServiceDiscoveryOptions(&_reactorServiceDiscoveryOpts);
	_reactorServiceDiscoveryOpts.userName = g_userName;
	_reactorServiceDiscoveryOpts.password = g_password;
	_reactorServiceDiscoveryOpts.clientId = g_userName;

	_reactorServiceDiscoveryOpts.pServiceEndpointEventCallback = serviceEndpointEventCallbackForError;

	ASSERT_TRUE(rsslReactorQueryServiceDiscovery(_pReactor, &_reactorServiceDiscoveryOpts, &rsslErrorInfo) == RSSL_RET_FAILURE);
}

static void copyMutRDMMsg(MutMsg *pMutMsg, RsslRDMMsg *pRDMMsg, RsslReactorChannel *pReactorChannel)
{
	pMutMsg->memoryBuffer.data = pMutMsg->memoryBlock;
	pMutMsg->memoryBuffer.length = sizeof(pMutMsg->memoryBlock);
	pMutMsg->mutMsgType = MUT_MSG_RDM;
	ASSERT_TRUE(rsslCopyRDMMsg(&pMutMsg->rdmMsg, (RsslRDMMsg*)pRDMMsg, &pMutMsg->memoryBuffer) == RSSL_RET_SUCCESS);
}

static void copyMutRsslMsg(MutMsg *pMutMsg, RsslMsg *pRsslMsg, RsslReactorChannel *pReactorChannel)
{
	pMutMsg->mutMsgType = MUT_MSG_RSSL;
	pMutMsg->rsslMsgBuffer.data = pMutMsg->rsslMsgBlock;
	pMutMsg->rsslMsgBuffer.length = sizeof(pMutMsg->rsslMsgBlock);
	ASSERT_TRUE(pMutMsg->pRsslMsg = rsslCopyMsg(pRsslMsg, RSSL_CMF_KEY_NAME, 0, &pMutMsg->rsslMsgBuffer)); /* Copy the key name -- need it when we get dictionary names. */
}

static void copyMutConnectionEvent(MutMsg *pMutMsg, RsslReactorChannelEvent *pConnEvent, RsslReactorChannel *pReactorChannel)
{
	pMutMsg->mutMsgType = MUT_MSG_CONN;
	pMutMsg->channelEvent = *pConnEvent; /* NOTE: NOT a  deep copy; won't do them unless tests require it */
	pMutMsg->pReactorChannel = pReactorChannel;
}

static RsslBool waitForConnection(RsslServer *pRsslServer, RsslUInt32 msec)
{
	fd_set readFds, exceptFds;
	struct timeval selectTime;
	int selectRet;

	FD_ZERO(&readFds);
	FD_ZERO(&exceptFds);

	FD_SET(pRsslServer->socketId, &readFds);
	FD_SET(pRsslServer->socketId, &exceptFds);

	selectTime.tv_sec = 0; selectTime.tv_usec = msec*1000;
	selectRet = select(FD_SETSIZE, &readFds, NULL, &exceptFds, &selectTime);

	return (selectRet > 0) ? RSSL_TRUE : RSSL_FALSE;
}

/* Wait for notification, then call rsslReactorDispatch to get events.
 * Call rsslReactorDispatch with the specified maxMessages
 * Returns RSSL_RET_READ_WOULD_BLOCK if rsslReactorDispatch was never called. 
 * NOTE: This test framework only stores the last received event into MyReactor::mutMsg.
 * If multiple events are received from the call to rsslReactorDispatch, all events
 * before for the last one will be overwritten. */
static RsslRet dispatchEvents(MyReactor *pMyReactor, RsslUInt32 timeoutMsec, RsslUInt32 maxMessages)
{
	RsslErrorInfo rsslErrorInfo;
	RsslReactorDispatchOptions dispatchOpts;
	int ret;

	if (pMyReactor->pNotifier != NULL)
	{
		/* Use RsslNotifer for notification */
		ret = rsslNotifierWait(pMyReactor->pNotifier, timeoutMsec*1000);
	}
	else
	{
		/* Use select() for notification */
		struct timeval selectTime;
		fd_set useReadFds = pMyReactor->readFds, useExceptFds = pMyReactor->exceptFds;

		selectTime.tv_sec = timeoutMsec/1000;
		selectTime.tv_usec = (timeoutMsec - selectTime.tv_sec * 1000) * 1000;
		ret = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);
	}

	clearMutMsg(&pMyReactor->mutMsg);
	if (ret > 0 || pMyReactor->previousDispatchRet > 0)
	{
		rsslClearReactorDispatchOptions(&dispatchOpts);
		dispatchOpts.maxMessages = maxMessages;
		pMyReactor->previousDispatchRet = rsslReactorDispatch(pMyReactor->pReactor, &dispatchOpts, &rsslErrorInfo);
		return pMyReactor->previousDispatchRet;
	}
	
	/* rsslReactorDispatch won't return this, so we can use it to signify that we didn't dispatch */
	pMyReactor->previousDispatchRet = RSSL_RET_READ_WOULD_BLOCK; /* Store it so we can easily see that this happened while debugging. */
	return RSSL_RET_READ_WOULD_BLOCK; 
}

/* Wait for notification, then call rsslReactorDispatch on the specific channel to get events.
* Call rsslReactorDispatch with the specified maxMessages
* Returns RSSL_RET_READ_WOULD_BLOCK if rsslReactorDispatch was never called.
* NOTE: This test framework only stores the last received event into MyReactor::mutMsg.
* The channel must be from the given MyReactor's RsslReactor.
* If multiple events are received from the call to rsslReactorDispatch, all events
* before for the last one will be overwritten. */
static RsslRet dispatchChannelEvents(MyReactor *pMyReactor, RsslReactorChannel *pChannel, RsslUInt32 timeoutMsec, RsslUInt32 maxMessages)
{
	RsslErrorInfo rsslErrorInfo;
	RsslReactorDispatchOptions dispatchOpts;
	int ret;

	if (pMyReactor->pNotifier != NULL)
	{
		/* Use RsslNotifer for notification */
		ret = rsslNotifierWait(pMyReactor->pNotifier, timeoutMsec * 1000);
	}
	else
	{
		/* Use select() for notification */
		struct timeval selectTime;
		fd_set useReadFds = pMyReactor->readFds, useExceptFds = pMyReactor->exceptFds;

		selectTime.tv_sec = timeoutMsec / 1000;
		selectTime.tv_usec = (timeoutMsec - selectTime.tv_sec * 1000) * 1000;
		ret = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);
	}

	clearMutMsg(&pMyReactor->mutMsg);
	if (ret > 0 || pMyReactor->previousDispatchRet > 0)
	{
		rsslClearReactorDispatchOptions(&dispatchOpts);
		dispatchOpts.maxMessages = maxMessages;
		dispatchOpts.pReactorChannel = pChannel;
		pMyReactor->previousDispatchRet = rsslReactorDispatch(pMyReactor->pReactor, &dispatchOpts, &rsslErrorInfo);
		return pMyReactor->previousDispatchRet;
	}

	/* rsslReactorDispatch won't return this, so we can use it to signify that we didn't dispatch */
	pMyReactor->previousDispatchRet = RSSL_RET_READ_WOULD_BLOCK; /* Store it so we can easily see that this happened while debugging. */
	return RSSL_RET_READ_WOULD_BLOCK;
}

/* Wait for notification, then call rsslReactorDispatch to get an event.
 * Returns RSSL_RET_READ_WOULD_BLOCK if rsslReactorDispatch was never called. */
static RsslRet dispatchEvent(MyReactor *pMyReactor, RsslUInt32 timeoutMsec)
{
	return dispatchEvents(pMyReactor, timeoutMsec, 1);
}

/* Wait for notification, then call rsslReactorDispatch to get an event.
* Returns RSSL_RET_READ_WOULD_BLOCK if rsslReactorDispatch was never called. */
static RsslRet dispatchChannelEvent(MyReactor *pMyReactor, RsslReactorChannel *pChannel, RsslUInt32 timeoutMsec)
{
	return dispatchChannelEvents(pMyReactor, pChannel, timeoutMsec, 100);

}

static void removeConnection(MyReactor *pMyReactor, RsslReactorChannel *pReactorChannel)
{
	if (pMyReactor->pNotifier != NULL)
	{
		/* We're using rsslNotifier; remove notification for this channel. */
		MyReactorChannel *pMyChannel = (MyReactorChannel*)pReactorChannel->userSpecPtr;
		ASSERT_TRUE(pMyChannel != NULL);

		ASSERT_TRUE(rsslNotifierRemoveEvent(pMyReactor->pNotifier, pMyChannel->pNotifierEvent) >= 0);
		rsslDestroyNotifierEvent(pMyChannel->pNotifierEvent);
		pMyChannel->pNotifierEvent = NULL;
	}
	else
	{
		if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
		{
			if (pMyReactor->pNotifier != NULL)
			{
				/* We're using rsslNotifier; remove notification for this channel. */
				MyReactorChannel *pMyChannel = (MyReactorChannel*)pReactorChannel->userSpecPtr;
				ASSERT_TRUE(pMyChannel != NULL);

				ASSERT_TRUE(rsslNotifierRemoveEvent(pMyReactor->pNotifier, pMyChannel->pNotifierEvent) >= 0);
				rsslDestroyNotifierEvent(pMyChannel->pNotifierEvent);
				pMyChannel->pNotifierEvent = NULL;
			}
			else
			{
				/* We're using select for notification. */
				FD_CLR(pReactorChannel->socketId, &pMyReactor->readFds);
				FD_CLR(pReactorChannel->socketId, &pMyReactor->exceptFds);
			}
		}
	}
	ASSERT_TRUE(rsslReactorCloseChannel(pMyReactor->pReactor, pReactorChannel, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}

static RsslReactorCallbackRet defaultMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pInfo->pRsslMsg);
	EXPECT_TRUE(pInfo->pRsslMsgBuffer);
	EXPECT_TRUE(!pInfo->pErrorInfo);

	copyMutRsslMsg(pMutMsg, pInfo->pRsslMsg, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet channelEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;
	RsslErrorInfo localErrorInfo;
	MyReactorChannel *pMyChannel = (MyReactorChannel*)pReactorChannel->userSpecPtr;

#ifdef _WIN32
			int rcvBfrSize = 65535;
			int sendBfrSize = 65535;
			RsslErrorInfo rsslErrorInfo;
#endif

	copyMutConnectionEvent(pMutMsg, pEvent, pReactorChannel);

	if (pMyChannel)
	{
		if (pMyChannel->pReactorChannel == NULL)
			pMyChannel->pReactorChannel = pReactorChannel; /* Channel is new */
		else
			EXPECT_TRUE(pMyChannel->pReactorChannel == pReactorChannel); /* Make sure the ReactorChannel and our object's channel match. */
	}

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pEvent);

	switch(pEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
			if (pMyReactor->pNotifier != NULL)
			{
				/* We're using RsslNotifier; add notification for this channel. */
				pMyChannel->pNotifierEvent = rsslCreateNotifierEvent();
				EXPECT_TRUE(pMyChannel->pNotifierEvent != NULL);

				rsslNotifierAddEvent(pMyReactor->pNotifier, pMyChannel->pNotifierEvent, pReactorChannel->socketId, pMyChannel);
				rsslNotifierRegisterRead(pMyReactor->pNotifier, pMyChannel->pNotifierEvent);
			}
			else
			{
				/* We're using select for notification; add FD */
				FD_SET(pReactorChannel->socketId, &pMyReactor->readFds);
				FD_SET(pReactorChannel->socketId, &pMyReactor->exceptFds);
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
			return RSSL_RC_CRET_SUCCESS;
		case RSSL_RC_CET_CHANNEL_READY:
			return RSSL_RC_CRET_SUCCESS;
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		case RSSL_RC_CET_CHANNEL_DOWN:

			if (pEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING)
				++pMyReactor->channelDownReconnectingEventCount;
			else
				++pMyReactor->channelDownEventCount;

			if (pReactorChannel->socketId != REACTOR_INVALID_SOCKET)
			{
				if (pMyReactor->pNotifier != NULL)
				{
					if (pMyChannel->pNotifierEvent != NULL)
					{
						/* We're using RsslNotifier; remove and destroy this channel's event. */
						rsslNotifierRemoveEvent(pMyReactor->pNotifier, pMyChannel->pNotifierEvent);
						rsslDestroyNotifierEvent(pMyChannel->pNotifierEvent);
					}
				}
				else
				{
					/* We're using select for notification; remove FD */
					FD_CLR(pReactorChannel->socketId, &pMyReactor->readFds);
					FD_CLR(pReactorChannel->socketId, &pMyReactor->exceptFds);
				}
			}

			if (pMyReactor->closeConnections)
				rsslReactorCloseChannel(pReactor, pReactorChannel, &localErrorInfo);

			return RSSL_RC_CRET_SUCCESS;
		default:
			return RSSL_RC_CRET_SUCCESS;
	}
}

RsslReactorCallbackRet authTokenEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorAuthTokenEvent *pAuthTokenEvent)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;

	pMyReactor->authEventStatusCode = pAuthTokenEvent->statusCode;

	if (pAuthTokenEvent->pError)
	{
		pMyReactor->numOfAuthTokenInfoErrorCount++;
		pConsMon->authTokenInfoErrorTextLength = (RsslUInt32)strlen(pAuthTokenEvent->pError->rsslError.text);
	}
	else if (pAuthTokenEvent->pReactorAuthTokenInfo)
	{
		pMyReactor->numOfAuthTokenInfoCount++;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pServiceEndpointEvent)
{
	ReactorServiceDiscoveryEndpointResult *expectedResult = (ReactorServiceDiscoveryEndpointResult *)pReactor->userSpecPtr;

	EXPECT_TRUE(pServiceEndpointEvent->pErrorInfo == 0);

	EXPECT_TRUE(validateServiceDiscoveryEndpoints(pServiceEndpointEvent, expectedResult));

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet serviceEndpointEventCallbackForError(RsslReactor *pReactor, RsslReactorServiceEndpointEvent *pServiceEndpointEvent)
{
	ReactorServiceDiscoveryEndpointError* expectedError = (ReactorServiceDiscoveryEndpointError*)pReactor->userSpecPtr;

	EXPECT_TRUE(pServiceEndpointEvent->pErrorInfo->rsslError.rsslErrorId == RSSL_RET_FAILURE);
	EXPECT_TRUE(strtok(pServiceEndpointEvent->pErrorInfo->rsslError.text, expectedError->expectedText) != NULL);
	EXPECT_TRUE(pServiceEndpointEvent->statusCode == expectedError->HttpStatusCode);

	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet loginMsgCallbackRaise(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent *pInfo)
{
	return RSSL_RC_CRET_RAISE;
}
static RsslReactorCallbackRet directoryMsgCallbackRaise(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent *pInfo)
{
	return RSSL_RC_CRET_RAISE;
}
static RsslReactorCallbackRet dictionaryMsgCallbackRaise(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent *pInfo)
{
	return RSSL_RC_CRET_RAISE;
}

static RsslReactorCallbackRet loginMsgCallbackDisconnect(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;
	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMLoginMsg, pReactorChannel);
	removeConnection(pMyReactor, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}
static RsslReactorCallbackRet directoryMsgCallbackDisconnect(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;
	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMDirectoryMsg, pReactorChannel);
	removeConnection(pMyReactor, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}
static RsslReactorCallbackRet dictionaryMsgCallbackDisconnect(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;
	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMDictionaryMsg, pReactorChannel);
	removeConnection(pMyReactor, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet defaultMsgCallbackDisconnect(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pInfo->pRsslMsg);
	EXPECT_TRUE(pInfo->pRsslMsgBuffer);
	EXPECT_TRUE(!pInfo->pErrorInfo);

	EXPECT_TRUE(pMutMsg->msgCount < 1);

	pMutMsg->msgCount++;

	copyMutRsslMsg(pMutMsg, pInfo->pRsslMsg, pReactorChannel);
	removeConnection(pMyReactor, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet channelEventCallbackDisconnect(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pEvent);

	copyMutConnectionEvent(pMutMsg, pEvent, pReactorChannel);
	removeConnection(pMyReactor, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet channelEventCallbackAddConnection(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;
	int index = (pReactorChannel->pRsslChannel->connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	EXPECT_TRUE(pReactor != NULL);
	EXPECT_TRUE(pReactorChannel != NULL);
	EXPECT_TRUE(pEvent != NULL);

	copyMutConnectionEvent(pMutMsg, pEvent, pReactorChannel);

	/* Call normal callback */
	channelEventCallback(pReactor, pReactorChannel, pEvent);

	/* Try reconnecting */
	if(pEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN && pMyReactor->reconnectAttempts > 0)
	{
		EXPECT_TRUE( rsslReactorConnect(pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS );
		--pMyReactor->reconnectAttempts;
	}
	return RSSL_RC_CRET_SUCCESS;
}

static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pInfo);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsgBuffer);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsg);
	EXPECT_TRUE(!pInfo->baseMsgEvent.pErrorInfo);
	EXPECT_TRUE(pInfo->pRDMLoginMsg);

	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMLoginMsg, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
} 

static RsslReactorCallbackRet directoryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pInfo);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsgBuffer);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsg);
	EXPECT_TRUE(!pInfo->baseMsgEvent.pErrorInfo);
	EXPECT_TRUE(pInfo->pRDMDirectoryMsg);

	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMDirectoryMsg, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
} 

static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent *pInfo)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pInfo);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsgBuffer);
	EXPECT_TRUE(pInfo->baseMsgEvent.pRsslMsg);
	EXPECT_TRUE(!pInfo->baseMsgEvent.pErrorInfo);
	EXPECT_TRUE(pInfo->pRDMDictionaryMsg);

	copyMutRDMMsg(pMutMsg, (RsslRDMMsg*)pInfo->pRDMDictionaryMsg, pReactorChannel);
	return RSSL_RC_CRET_SUCCESS;
} 


static void clearObjects()
{
	pConsMon->pNotifier = NULL;
	pProvMon->pNotifier = NULL;
	pConsMon->pReactorNotifierEvent = NULL;
	pProvMon->pReactorNotifierEvent = NULL;
	pConsMon->closeConnections = RSSL_TRUE;
	pConsMon->channelDownEventCount = 0;
	pConsMon->numOfAuthTokenInfoCount = 0;
	pConsMon->numOfAuthTokenInfoErrorCount = 0;
	pConsMon->channelDownReconnectingEventCount = 0;
	pProvMon->closeConnections = RSSL_TRUE;
	pProvMon->channelDownEventCount = 0;
	pProvMon->channelDownReconnectingEventCount = 0;

	rsslClearReactorConnectOptions(&connectOpts[0]);
	connectOpts[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14009");

	rsslClearReactorConnectOptions(&connectOpts[1]);
	connectOpts[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	connectOpts[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
	connectOpts[1].rsslConnectOptions.wsOpts.protocols = const_cast<char*>("rssl.json.v2");

	rsslClearReactorAcceptOptions(&acceptOpts);

	rsslInitDefaultRDMLoginRequest(&loginRequest, 1);
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);

	rsslClearRDMLoginRefresh(&loginRefresh);
	loginRefresh.rdmMsgBase.streamId = 1;

	rsslClearRDMLoginStatus(&loginSuspectStatus);
	loginSuspectStatus.state.streamState = RSSL_STREAM_OPEN;
	loginSuspectStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = 2;
	rsslClearRDMService(&directoryService);
	directoryService.flags |= RDM_SVCF_HAS_INFO;
	directoryService.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
	directoryService.info.dictionariesProvidedList = dictionariesProvidedList;
	directoryService.info.dictionariesProvidedCount = dictionariesProvidedCount;
	directoryService.serviceId = 1;
	directoryRefresh.serviceList = &directoryService;
	directoryRefresh.serviceCount = 1;

	rsslClearOMMConsumerRole(&ommConsumerRole);
	ommConsumerRole.base.defaultMsgCallback = defaultMsgCallback;
	ommConsumerRole.base.channelEventCallback = channelEventCallback;

	rsslClearOMMProviderRole(&ommProviderRole);
	ommProviderRole.base.defaultMsgCallback = defaultMsgCallback;
	ommProviderRole.base.channelEventCallback = channelEventCallback;

	rsslClearOMMNIProviderRole(&ommNIProviderRole);
	ommNIProviderRole.base.defaultMsgCallback = defaultMsgCallback;
	ommNIProviderRole.base.channelEventCallback = channelEventCallback;

	rsslClearRDMDictionaryRefresh(&fieldDictionaryRefresh);
	fieldDictionaryRefresh.flags = RDM_DC_RFF_SOLICITED;
	fieldDictionaryRefresh.pDictionary = &dataDictionary;
	fieldDictionaryRefresh.dictionaryName = fieldDictionaryName;
	fieldDictionaryRefresh.verbosity = RDM_DICTIONARY_NORMAL;
	fieldDictionaryRefresh.type = RDM_DICTIONARY_FIELD_DEFINITIONS;
	fieldDictionaryRefresh.serviceId = 1;

	rsslClearRDMDictionaryRefresh(&enumDictionaryRefresh);
	enumDictionaryRefresh.pDictionary = &dataDictionary;
	enumDictionaryRefresh.dictionaryName = enumDictionaryName;
	enumDictionaryRefresh.verbosity = RDM_DICTIONARY_NORMAL;
	enumDictionaryRefresh.type = RDM_DICTIONARY_ENUM_TABLES;
	enumDictionaryRefresh.serviceId = 1;

}

static void initReactors(RsslCreateReactorOptions *pOpts, RsslBool sameReactor)
{
	RsslErrorInfo rsslErrorInfo;
	RsslReactorJsonConverterOptions jsonConverterOptions;

	rsslClearReactorJsonConverterOptions(&jsonConverterOptions);
	clearMyReactor(pConsMon);
	clearMyReactor(pProvMon);
	mOpts.userSpecPtr = pConsMon;
	ASSERT_TRUE(pConsMon->pReactor = rsslCreateReactor(pOpts, &rsslErrorInfo));

	jsonConverterOptions.pDictionary = &dataDictionary;
	jsonConverterOptions.defaultServiceId = 1;

	ASSERT_TRUE(rsslReactorInitJsonConverter(pConsMon->pReactor, &jsonConverterOptions, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	if (sameReactor)
		pProvMon->pReactor = pConsMon->pReactor;
	else
	{
		mOpts.userSpecPtr = &myReactors[1];
		ASSERT_TRUE(pProvMon->pReactor = rsslCreateReactor(pOpts, &rsslErrorInfo));

		jsonConverterOptions.pDictionary = &dataDictionary;
		jsonConverterOptions.defaultServiceId = 1;

		ASSERT_TRUE(rsslReactorInitJsonConverter(pProvMon->pReactor, &jsonConverterOptions, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	}

	FD_ZERO(&pConsMon->readFds);
	FD_ZERO(&pConsMon->writeFds);
	FD_ZERO(&pConsMon->exceptFds);
	FD_ZERO(&pProvMon->readFds);
	FD_ZERO(&pProvMon->writeFds);
	FD_ZERO(&pProvMon->exceptFds);

	FD_SET(pConsMon->pReactor->eventFd, &pConsMon->readFds);
	FD_SET(pConsMon->pReactor->eventFd, &pConsMon->exceptFds);
	FD_SET(pProvMon->pReactor->eventFd, &pProvMon->readFds);
	FD_SET(pProvMon->pReactor->eventFd, &pProvMon->exceptFds);
}



static void cleanupReactors(RsslBool sameReactor)
{
	ASSERT_TRUE(rsslDestroyReactor(pConsMon->pReactor, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	if (!sameReactor)
		ASSERT_TRUE(rsslDestroyReactor(pProvMon->pReactor, &rsslErrorInfo) == RSSL_RET_SUCCESS);
}



static void sendRDMMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMMsg *pRDMMsg, RsslUInt32 bufferSize)
{
	RsslBuffer *pBuffer;
	RsslReactorSubmitOptions submitOpts;

	rsslClearReactorSubmitOptions(&submitOpts);

	ASSERT_TRUE((pBuffer = rsslReactorGetBuffer(pReactorChannel, bufferSize, RSSL_FALSE, &rsslErrorInfo)));

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, pBuffer) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeRDMMsg(&eIter, pRDMMsg, &pBuffer->length, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslReactorSubmit(pReactor, pReactorChannel, pBuffer, &submitOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);

}

static void reactorUnitTests_AutoMsgsInt(RsslConnectionTypes connectionType)
{
	/* Test automatically sent login, directory, and dictionary messages, with and without callbacks */

	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	/* Login only */
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Prov: Conn up */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh[0] = pProvMon->mutMsg.pReactorChannel;

	/* Prov: Conn ready */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Prov: (none) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* Cons: Conn up */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh[0] = pConsMon->mutMsg.pReactorChannel;

	if (ommConsumerRole.pLoginRequest)
	{
		/* Cons: (flush complete) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: (none) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

		/* Prov: Receive Login Request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.loginMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send login suspect status (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginSuspectStatus, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive Login Suspect Status */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		if (ommConsumerRole.loginMsgCallback)
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS );
		else
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS );

		/* Prov: Send login refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive Login Refresh */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		if (ommConsumerRole.loginMsgCallback)
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH );
		else
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );
	}

	if (ommConsumerRole.pDirectoryRequest)
	{
		/* Cons: (flush complete) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive Directory Request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.directoryMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send directory refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&directoryRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive Directory Refresh */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		if (ommConsumerRole.directoryMsgCallback)
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH );
		else
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );
	}

	if (ommConsumerRole.dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE)
	{
		/* Cons: (flush complete) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive dictionary requests and send refreshes */
		while (dispatchEvent(pProvMon, 100) != RSSL_RET_READ_WOULD_BLOCK)
		{
			if (ommProviderRole.dictionaryMsgCallback && pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM)
			{
				RsslRDMDictionaryRequest *pRDMRequest;
				ASSERT_TRUE(pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_REQUEST );

				pRDMRequest = (RsslRDMDictionaryRequest*)&pProvMon->mutMsg.rdmMsg;

				if (rsslBufferIsEqual(&pRDMRequest->dictionaryName, &fieldDictionaryName))
				{
					fieldDictionaryRefresh.rdmMsgBase.streamId = pRDMRequest->rdmMsgBase.streamId;
					sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&fieldDictionaryRefresh, 400); /* (no fragments) */
				}
				else if (rsslBufferIsEqual(&pRDMRequest->dictionaryName, &enumDictionaryName))
				{
					enumDictionaryRefresh.rdmMsgBase.streamId = pRDMRequest->rdmMsgBase.streamId;
					sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&enumDictionaryRefresh, 400);
				}
				else
				{
					ASSERT_TRUE(0);
				}
			}
			else if (!ommProviderRole.dictionaryMsgCallback && pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL)
			{
				RsslRequestMsg *pReqMsg;

				ASSERT_TRUE(pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );
				pReqMsg = &pProvMon->mutMsg.pRsslMsg->requestMsg;
				ASSERT_TRUE(pReqMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);

				if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &fieldDictionaryName))
				{
					fieldDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
					sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&fieldDictionaryRefresh, 400);
				}
				else if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &enumDictionaryName))
				{
					enumDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
					sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&enumDictionaryRefresh, 400);
				}
				else
				{
					ASSERT_TRUE(0);
				}

			}
			else
			{
				/* Anything that didn't fall into the above cases should have been a completed flush event */
				ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
			}
		}

		/* Cons: receive dictionary refreshes */
		{
			RsslBool gotFieldDictionary = RSSL_FALSE, gotEnumDictionary = RSSL_FALSE, gotConnReadyEvent = RSSL_FALSE;
			RsslInt32 dictionaryCount = 0;
			while(dispatchEvent(pConsMon, 100) != RSSL_RET_READ_WOULD_BLOCK)
			{

				if (ommConsumerRole.dictionaryMsgCallback)
				{
					if (pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pConsMon->mutMsg.rdmMsg.dictionaryMsg.refresh.type == RDM_DICTIONARY_FIELD_DEFINITIONS)
					{
						ASSERT_TRUE(gotFieldDictionary == RSSL_FALSE); gotFieldDictionary = RSSL_TRUE; ++dictionaryCount;
					}
					else if (pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pConsMon->mutMsg.rdmMsg.dictionaryMsg.refresh.type == RDM_DICTIONARY_ENUM_TABLES)
					{
						ASSERT_TRUE(gotEnumDictionary == RSSL_FALSE); gotEnumDictionary = RSSL_TRUE; ++dictionaryCount;
					}
					else if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) 
					{
						ASSERT_TRUE(!gotConnReadyEvent); gotConnReadyEvent = RSSL_TRUE;
					}
					else
						ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
				}
				else
				{
					if(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH )
					{
						++dictionaryCount;
					}
					else if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) 
					{
						ASSERT_TRUE(!gotConnReadyEvent); gotConnReadyEvent = RSSL_TRUE;
					}
					else
						ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
				}
			}

			ASSERT_TRUE(dictionaryCount == 2 && gotConnReadyEvent);
		}

		/* Prov: Receive dictionary closes */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.dictionaryMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_CLOSE );

		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.dictionaryMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_CLOSE );
	}
	else
	{
		/* Cons: Connection ready */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	}

	/* Cons: Close (+ ack) */
	removeConnection(pConsMon, pConsCh[0]);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Conn down */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh[0]);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

}

static void reactorUnitTests_AutoMsgsInt_NiProv()
{
	/* Test automatically sent login and directory messages, with and without callbacks */

	/* Login only */
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[0], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[0], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[0], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Prov: Conn up */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh[0] = pProvMon->mutMsg.pReactorChannel;

	/* Prov: Conn ready */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Prov: (none) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* NiProv: Conn up */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh[0] = pConsMon->mutMsg.pReactorChannel;

	if (ommNIProviderRole.pLoginRequest)
	{
		/* NiProv: (flush complete) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* NiProv: (none) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

		/* Prov: Receive Login Request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.loginMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send login suspect status (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginSuspectStatus, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive Login Suspect Status */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		if (ommConsumerRole.loginMsgCallback)
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS );
		else
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS );

		/* Prov: Send login refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* NiProv: Receive Login Refresh */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		if (ommNIProviderRole.loginMsgCallback)
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH );
		else
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );
	}

	if (ommNIProviderRole.pDirectoryRefresh)
	{
		/* Prov: Receive Directory Refresh */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		if (ommProviderRole.directoryMsgCallback)
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH );
		else
			ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );

		/* NiProv: Flush complete & Connection ready */
		/* (Since the events come from different threads, there's no certainty about which we will get first, though in general it should be the conn ready event) */
		{
			RsslBool gotNoEvent = RSSL_FALSE, gotConnReadyEvent = RSSL_FALSE;

			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
			if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) gotConnReadyEvent = RSSL_TRUE;
			if (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE) gotNoEvent = RSSL_TRUE;

			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
			if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) gotConnReadyEvent = RSSL_TRUE;
			if (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE) gotNoEvent = RSSL_TRUE;

			ASSERT_TRUE(gotConnReadyEvent && gotNoEvent);
		}
	}
	else
	{
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	}

	/* Cons: Close (+ ack) */
	removeConnection(pConsMon, pConsCh[0]);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Conn down */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh[0]);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

static void reactorUnitTests_AutoMsgs(RsslConnectionTypes connectionType)
{
	/* Basic connection (no messages exchanged) */
	clearObjects();
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add login request to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add login callback to provider */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add login callback to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add directory request to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add directory callback to provider */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add directory callback to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add dictionary request to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add dictionary callback to provider */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommProviderRole.dictionaryMsgCallback = dictionaryMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);

	/* Add dictionary callback to consumer */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommProviderRole.dictionaryMsgCallback = dictionaryMsgCallback;
	ommConsumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	reactorUnitTests_AutoMsgsInt(connectionType);


	/* Test NonInteractive Provider */
	clearObjects();
	reactorUnitTests_AutoMsgsInt_NiProv();

	/* Add login request to NiProv */
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	reactorUnitTests_AutoMsgsInt_NiProv();

	/* Add login callback to provider */
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	reactorUnitTests_AutoMsgsInt_NiProv();

	/* Add directory refresh to NiProv */
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommNIProviderRole.pDirectoryRefresh = &directoryRefresh;
	reactorUnitTests_AutoMsgsInt_NiProv();
}

static void reactorUnitTests_Raise(RsslConnectionTypes connectionType)
{
	/* Set all callbacks to raise to the default callback.  This tests that RSSL_RC_CRET_RAISE works,
	 * and that the reactor can continue getting the connection ready. */

	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallbackRaise;
	ommConsumerRole.loginMsgCallback = loginMsgCallbackRaise;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommProviderRole.directoryMsgCallback = directoryMsgCallbackRaise;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallbackRaise;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommProviderRole.dictionaryMsgCallback = dictionaryMsgCallbackRaise;
	ommConsumerRole.dictionaryMsgCallback = dictionaryMsgCallbackRaise;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Prov: Conn up */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh[0] = pProvMon->mutMsg.pReactorChannel;

	/* Prov: Conn ready */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Prov: (none) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* Cons: Conn up */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh[0] = pConsMon->mutMsg.pReactorChannel;

	/* Cons: (flush complete) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Cons: (none) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* Prov: Receive Login Request */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);

	/* Since all RDM callbacks return RSSL_RC_CRET_RAISE, they should all fall into the defaultMsgCallback and the copied mutMsg will always be type MUT_MSG_RSSL */
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

	/* Prov: Send login refresh (+ flush) */
	sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginRefresh, 400);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Cons: Receive Login Refresh */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );

	/* Cons: (flush complete) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Receive Directory Request */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

	/* Prov: Send directory refresh (+ flush) */
	sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&directoryRefresh, 400);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Cons: Receive Directory Refresh */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );


	/* Cons: (flush complete) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Receive dictionary requests and send refreshes */
	while (dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS)
	{
		RsslRequestMsg *pReqMsg;

		if (pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL)
		{
			ASSERT_TRUE(pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );
			pReqMsg = &pProvMon->mutMsg.pRsslMsg->requestMsg;
			ASSERT_TRUE(pReqMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);

			if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &fieldDictionaryName))
			{
				fieldDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
				sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&fieldDictionaryRefresh, 400);
			}
			else if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &enumDictionaryName))
			{
				enumDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
				sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&enumDictionaryRefresh, 400);
			}
			else
			{
				/* May get flush complete events */
				ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
			}
		}

	}

	{
		RsslBool gotConnReadyEvent = RSSL_FALSE;
		RsslInt32 dictionaryCount = 0;
		while(dispatchEvent(pConsMon, 100) != RSSL_RET_READ_WOULD_BLOCK)
		{

			if(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH )
			{
				++dictionaryCount;
			}
			else if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) 
			{
				ASSERT_TRUE(!gotConnReadyEvent); gotConnReadyEvent = RSSL_TRUE;
			}
			else
				ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		}

		ASSERT_TRUE(dictionaryCount == 2 && gotConnReadyEvent);
	}

	/* Cons: Close (+ ack) */
	removeConnection(pConsMon, pConsCh[0]);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Receive dictionary closes */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_CLOSE );
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_CLOSE );


	/* Prov: Conn down */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh[0]);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

void reactorUnitTests_InitializationAndPingTimeout(RsslConnectionTypes connectionType)
{
	/* Test that initialization and ping timeouts work */
	RsslServer *pRsslServer;
	RsslChannel *pRsslCh;
	RsslInProgInfo inProg;
	RsslBindOptions rsslBindOpts;
	RsslReactorDispatchOptions dispatchOpts;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslReactorChannel *pConsChannel, *pProvChannel;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	rsslClearReactorDispatchOptions(&dispatchOpts);
	dispatchOpts.maxMessages = 1;

	connectOpts[index].initializationTimeout = 1;
	connectOpts[index].rsslConnectOptions.pingTimeout = 1;
	acceptOpts.initializationTimeout = 1;

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = const_cast<char*>("14010");
	rsslBindOpts.pingTimeout = 1;
	rsslBindOpts.minPingTimeout = 1;
	rsslBindOpts.wsOpts.protocols = const_cast<char*>("rssl.json.v2");

	ASSERT_TRUE((pRsslServer = rsslBind(&rsslBindOpts, &rsslErrorInfo.rsslError)));

	connectOpts[index].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14010");

	/*** Test initialization on connection ***/

	/* Use rsslAccept() on server for this test so that initializing/pinging isn't done by server */
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	time_sleep(100);
	ASSERT_TRUE(pRsslCh = rsslAccept(pRsslServer, &acceptOpts.rsslAcceptOptions, &rsslErrorInfo.rsslError));

	/* Cons: Should get conn down since provider didn't initialize in time */
	ASSERT_TRUE(dispatchEvent(pConsMon, 1200) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Cons: No message(close ack) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(rsslCloseChannel(pRsslCh, &rsslErrorInfo.rsslError) >= RSSL_RET_SUCCESS);

	/*** Test initialization on accepting connection ***/

	/* Same test, but from server side (use rsslConnect())*/
	ASSERT_TRUE(pRsslCh = rsslConnect(&connectOpts[index].rsslConnectOptions, &rsslErrorInfo.rsslError));
	ASSERT_TRUE(waitForConnection(pRsslServer, 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pRsslServer, &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Prov: Should get conn down since consumer didn't initialize in time */
	ASSERT_TRUE(dispatchEvent(pProvMon, 1200) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(rsslCloseChannel(pRsslCh, &rsslErrorInfo.rsslError) >= RSSL_RET_SUCCESS);

	/* Prov: No message(close ack) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/*** Test ping timeout on connection ***/

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	time_sleep(10);
	ASSERT_TRUE(pRsslCh = rsslAccept(pRsslServer, &acceptOpts.rsslAcceptOptions, &rsslErrorInfo.rsslError));

	while(pRsslCh->state == RSSL_CH_STATE_INITIALIZING)
	{
		RsslRet ret = rsslInitChannel(pRsslCh, &inProg, &rsslErrorInfo.rsslError);
		ASSERT_TRUE(ret == RSSL_RET_SUCCESS || ret == RSSL_RET_CHAN_INIT_IN_PROGRESS);
	}

	ASSERT_TRUE(pRsslCh->state == RSSL_CH_STATE_ACTIVE);

	/* Cons: Should get conn up/ready event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	pConsChannel = pConsMon->mutMsg.pReactorChannel;

	/* Cons: Should get conn down since provider doesn't send pings */
	ASSERT_TRUE(dispatchEvent(pConsMon, 1200) == RSSL_RET_READ_WOULD_BLOCK);
	ASSERT_TRUE(rsslReactorDispatch(pConsMon->pReactor, &dispatchOpts, &rsslErrorInfo) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Cons: Try to send a message. This should fail since the channel is down (want to test ping timeout specifically -- even though
	 * the Reactor considers the channel down, the underlying RsslChannel is still active) . */
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	ASSERT_TRUE(rsslReactorSubmitMsg(pConsMon->pReactor, pConsChannel, &submitMsgOpts, &rsslErrorInfo) == RSSL_RET_FAILURE);

	/* Cons: No message(close ack) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/*** Test ping timeout on accepting connection ***/

	ASSERT_TRUE(pRsslCh = rsslConnect(&connectOpts[index].rsslConnectOptions, &rsslErrorInfo.rsslError));
	ASSERT_TRUE(waitForConnection(pRsslServer, 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pRsslServer, &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	while(pRsslCh->state == RSSL_CH_STATE_INITIALIZING)
	{
		RsslRet ret = rsslInitChannel(pRsslCh, &inProg, &rsslErrorInfo.rsslError);
		ASSERT_TRUE(ret == RSSL_RET_SUCCESS || ret == RSSL_RET_CHAN_INIT_IN_PROGRESS);
	}

	/* Prov: Should get conn up/ready event */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	pProvChannel = pProvMon->mutMsg.pReactorChannel;

	/* Prov: Should get conn down since provider doesn't send pings */
	ASSERT_TRUE(dispatchEvent(pProvMon, 1200) == RSSL_RET_READ_WOULD_BLOCK);
	ASSERT_TRUE(rsslReactorDispatch(pProvMon->pReactor, &dispatchOpts, &rsslErrorInfo) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Try to send a message. This should fail since the channel is down (want to test ping timeout specifically -- even though
	 * the Reactor considers the channel down, the underlying RsslChannel is still active) . */
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	ASSERT_TRUE(rsslReactorSubmitMsg(pProvMon->pReactor, pProvChannel, &submitMsgOpts, &rsslErrorInfo) == RSSL_RET_FAILURE);

	/* Prov: No message(close ack) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(rsslCloseChannel(pRsslCh, &rsslErrorInfo.rsslError) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslCloseServer(pRsslServer, &rsslErrorInfo.rsslError) == RSSL_RET_SUCCESS); 
}

static void reactorUnitTests_InvalidArguments(RsslConnectionTypes connectionType)
{
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	/* Test bad uses of the interface */

	/*** rsslReactorConnect()/rsslReactorAccept() ***/

	/* No defaultMsgCallback */
	clearObjects();
	ommConsumerRole.base.defaultMsgCallback = NULL;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* No channelEventCallback */
	clearObjects();
	ommConsumerRole.base.channelEventCallback = NULL;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* Consumer provides directory request without login request */
	clearObjects();
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* Consumer requests dictionary download without login & directory requests */
	clearObjects();
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	clearObjects();
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommConsumerRole.pLoginRequest = &loginRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	clearObjects();
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	
	/* NIProv sends directory refresh without login request */
	clearObjects();
	ommNIProviderRole.pDirectoryRefresh = &directoryRefresh;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* AddConnection without server */
	clearObjects();
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* Add/AcceptConnection with bad role */
	clearObjects();
	ommConsumerRole.base.roleType = (RsslReactorChannelRoleType)5;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/*** Wrong msg type ***/

	/* Cons: Add/AcceptConnection with wrong type for login request setup msg */
	clearObjects();
	ommConsumerRole.pLoginRequest = (RsslRDMLoginRequest*)&directoryRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* Cons: Add/AcceptConnection with wrong type for directory request setup msg */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.pDirectoryRequest = (RsslRDMDirectoryRequest*)&loginRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	
	/* NiProv: Add/AcceptConnection with wrong type for login request setup msg */
	clearObjects();
	ommNIProviderRole.pLoginRequest = (RsslRDMLoginRequest*)&directoryRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);

	/* NiProv: Add/AcceptConnection with wrong type for directory request setup msg */
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommNIProviderRole.pDirectoryRefresh = (RsslRDMDirectoryRefresh*)&loginRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommNIProviderRole.pDirectoryRefresh = (RsslRDMDirectoryRefresh*)&directoryRequest;
	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
	ASSERT_TRUE(rsslReactorAccept(pConsMon->pReactor, NULL, &acceptOpts, (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_INVALID_ARGUMENT);
}

void reactorUnitTests_ShortPingInterval(RsslConnectionTypes connectionType)
{
	/* Test that connection can stay up with a very small ping interval */
	RsslServer *pRsslServer;
	RsslReactorChannel *pProvCh, *pConsCh;
	RsslBindOptions rsslBindOpts;
	RsslReactorDispatchOptions dispatchOpts;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	rsslClearReactorDispatchOptions(&dispatchOpts);
	dispatchOpts.maxMessages = 1;

	connectOpts[index].initializationTimeout = 1;
	connectOpts[index].rsslConnectOptions.pingTimeout = 1;
	acceptOpts.initializationTimeout = 1;

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = const_cast<char*>("14010");
	rsslBindOpts.pingTimeout = 1;
	rsslBindOpts.minPingTimeout = 1;
	if (index == 1)
	{
		rsslBindOpts.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	ASSERT_TRUE((pRsslServer = rsslBind(&rsslBindOpts, &rsslErrorInfo.rsslError)));

	connectOpts[index].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14010");

	/*** Test initialization on connection ***/

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pRsslServer, 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pRsslServer, &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Cons: Should get conn up/ready event */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh = pConsMon->mutMsg.pReactorChannel;
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Prov: Should get conn up/ready event */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh = pProvMon->mutMsg.pReactorChannel;
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Check that both sides got a ping (dispatch is called and no message is received). */
	ASSERT_TRUE(dispatchEvent(pConsMon, 1100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Same again (but dispatch provider for the full second). */
	ASSERT_TRUE(dispatchEvent(pProvMon, 1100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Same again (but dispatch consumer for the full second). */
	ASSERT_TRUE(dispatchEvent(pConsMon, 1100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	removeConnection(pProvMon, pProvCh);
	removeConnection(pConsMon, pConsCh);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(rsslCloseServer(pRsslServer, &rsslErrorInfo.rsslError) == RSSL_RET_SUCCESS); 
}

static void reactorUnitTests_DisconnectFromCallbacksInt_Cons(bool channelDispatch, RsslConnectionTypes connectionType)
{
	RsslReactorChannel *pProvCh;
	RsslReactorChannel *pConsCh;
	int i;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 1000));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	
	pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
	/* For some reason, after several iterations, the connection runs very slow */
	while(pProvMon->mutMsg.mutMsgType != MUT_MSG_CONN)
		ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);

	ASSERT_EQ(pProvMon->mutMsg.mutMsgType, MUT_MSG_CONN);
	ASSERT_EQ(pProvMon->mutMsg.channelEvent.channelEventType, RSSL_RC_CET_CHANNEL_UP);
	pProvCh = pProvMon->mutMsg.pReactorChannel;

	pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
	while(pProvMon->mutMsg.mutMsgType != MUT_MSG_CONN)
		ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);
	ASSERT_EQ(pProvMon->mutMsg.mutMsgType, MUT_MSG_CONN);
	ASSERT_EQ(pProvMon->mutMsg.channelEvent.channelEventType, RSSL_RC_CET_CHANNEL_READY);

	/* For some reason, after several iterations, the connection runs very slow */
	pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
	while(pConsMon->mutMsg.mutMsgType != MUT_MSG_CONN)
		ASSERT_TRUE(dispatchEvent(pConsMon, 1000) >= RSSL_RET_SUCCESS);
	ASSERT_EQ(pConsMon->mutMsg.mutMsgType, MUT_MSG_CONN);
	ASSERT_EQ(pConsMon->mutMsg.channelEvent.channelEventType, RSSL_RC_CET_CHANNEL_UP);
	pConsCh = pConsMon->mutMsg.pReactorChannel;

	/* If loginMsgCallback provided, exchange login messages */
	if (ommConsumerRole.loginMsgCallback)
	{
		ASSERT_TRUE(ommConsumerRole.pLoginRequest); /* Consumer should have provided a loginRequest to test this */
		ASSERT_TRUE(ommConsumerRole.base.channelEventCallback == channelEventCallback); /* Should be using standard callbacks elsewhere */

		/* Cons: (flush login request) */
		if(channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive login request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send login refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&loginRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive login refresh */
		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH );
	}

	if (ommConsumerRole.directoryMsgCallback)
	{
		
		ASSERT_TRUE(ommConsumerRole.pDirectoryRequest); /* Consumer should have provided a directoryRequest to test this */

		/* Should be using standard callbacks elsewhere */
		ASSERT_TRUE(ommConsumerRole.loginMsgCallback == loginMsgCallback); 
		ASSERT_TRUE(ommConsumerRole.base.channelEventCallback == channelEventCallback); 

		/* Cons: (flush directoryRequest) */
		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive directory request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send directory refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&directoryRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive directory refresh */
		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	}

	if (ommConsumerRole.dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE)
	{
		RsslRequestMsg *pReqMsg;

		/* Should be using standard callbacks elsewhere */
		ASSERT_TRUE(ommConsumerRole.base.channelEventCallback == channelEventCallback);
		ASSERT_TRUE(ommConsumerRole.loginMsgCallback == loginMsgCallback);
		ASSERT_TRUE(ommConsumerRole.directoryMsgCallback == directoryMsgCallback);

		/* Cons: (flush dictionaryRequests -- we will get 2 since we sent out more than one message) */
		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive dictionary requests */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		pReqMsg = &pProvMon->mutMsg.pRsslMsg->requestMsg;
		/* Prov: Send dictionary refresh (+ flush). Just send one, consumer should immediately quit */
		if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &fieldDictionaryName))
		{
			fieldDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
			sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&fieldDictionaryRefresh, 400);
		}
		else if (rsslBufferIsEqual(&pReqMsg->msgBase.msgKey.name, &enumDictionaryName))
		{
			enumDictionaryRefresh.rdmMsgBase.streamId = pReqMsg->msgBase.streamId;
			sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&enumDictionaryRefresh, 400);
		}
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		if (channelDispatch)
			ASSERT_TRUE(dispatchChannelEvent(pConsMon, pConsCh, 100) >= RSSL_RET_SUCCESS);
		else
		{
			ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		}

		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_REFRESH);
	}

	
	/* Send 50 messages from the provider to the consumer, only if the default message callback has been set. */
	if (ommConsumerRole.base.defaultMsgCallback == defaultMsgCallbackDisconnect)
	{
		pConsMon->mutMsg.msgCount = 0;
		for (i = 0; i < 50; ++i)
		{
			RsslBuffer *pMsgBuf = rsslReactorGetBuffer(pProvCh, 500, RSSL_FALSE, &rsslErrorInfo);
			RsslEncodeIterator eIter;
			RsslRefreshMsg refreshMsg;
			RsslMsgBase *msgBase;
			char stateText[50];
			RsslMsg *msg;
			RsslReactorSubmitOptions submitOpts;

			rsslClearReactorSubmitOptions(&submitOpts);


			EXPECT_TRUE(pMsgBuf);

			/* set-up message */
			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pProvCh->majorVersion, pProvCh->minorVersion);
			ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, pMsgBuf) == RSSL_RET_SUCCESS);

			rsslClearRefreshMsg(&refreshMsg);
			msgBase = &refreshMsg.msgBase;
			refreshMsg.state.streamState = RSSL_STREAM_OPEN;
			refreshMsg.state.dataState = RSSL_DATA_OK;
			refreshMsg.state.code = RSSL_SC_NONE;
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS;

			sprintf(stateText, "Item Refresh Completed");
			refreshMsg.state.text.data = stateText;
			refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
			msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
			/* ServiceId */
			msgBase->msgKey.serviceId = 5;
			/* Itemname */
			msgBase->msgKey.name.data = itemName.data;
			msgBase->msgKey.name.length = itemName.length;
			msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
			/* Qos */
			refreshMsg.qos.dynamic = RSSL_FALSE;
			refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
			refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
			msg = (RsslMsg *)&refreshMsg;

			msgBase->domainType = RSSL_DMT_MARKET_PRICE;
			msgBase->containerType = RSSL_DT_NO_DATA;

			/* StreamId */
			msgBase->streamId = 10;

			/* encode message */
			ASSERT_TRUE(rsslEncodeMsg(&eIter, msg) == RSSL_RET_SUCCESS);
			pMsgBuf->length = rsslGetEncodedBufferLength(&eIter);

			ASSERT_TRUE(rsslReactorSubmit(pProvMon->pReactor, pProvCh, pMsgBuf, &submitOpts, &rsslErrorInfo) == RSSL_RET_SUCCESS);
		}

		pConsMon->mutMsg.mutMsgType = MUT_MSG_NONE;
		if (channelDispatch)
			while(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE)
				ASSERT_TRUE(dispatchChannelEvents(pConsMon, pConsCh, 1000, 50) >= RSSL_RET_SUCCESS);
		else
		{
			while(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE)
				ASSERT_TRUE(dispatchEvents(pConsMon, 1000, 50) >= RSSL_RET_SUCCESS);
		}
		ASSERT_EQ(pConsMon->mutMsg.mutMsgType, MUT_MSG_RSSL);
	}

	/* Cons: Ack close */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov channel down */
	pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
	while (pProvMon->mutMsg.mutMsgType != MUT_MSG_CONN)
	{
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	}
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
	
	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

static void reactorUnitTests_DisconnectFromCallbacksInt_Prov(RsslConnectionTypes connectionType)
{
	RsslReactorChannel *pProvCh;
	RsslReactorChannel *pConsCh;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh = pConsMon->mutMsg.pReactorChannel;
	pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
	/* Make sure connection is up */
	while(pProvMon->mutMsg.mutMsgType != MUT_MSG_CONN)
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh = pProvMon->mutMsg.pReactorChannel;
	/* If loginMsgCallback provided, exchange login messages */
	if (ommProviderRole.loginMsgCallback)
	{
		ASSERT_TRUE(ommConsumerRole.pLoginRequest); /* Consumer should have provided a loginRequest to test this */
		ASSERT_TRUE(ommProviderRole.base.channelEventCallback == channelEventCallback); /* Should be using standard callbacks elsewhere */

		/* cons flush login request) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		
		/* Make sure the provider receives the login msg */
		pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
		while (pProvMon->mutMsg.mutMsgType != MUT_MSG_RDM)
			ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);
	}
	else
	{
		/* Consumer will get connection ready event before provider goes down */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	}

	if (ommProviderRole.directoryMsgCallback)
	{
		ASSERT_TRUE(ommConsumerRole.pDirectoryRequest); /* Consumer should have provided a directoryRequest to test this */

		/* Should be using standard callbacks elsewhere */
		ASSERT_TRUE(ommProviderRole.loginMsgCallback == loginMsgCallback); 
		ASSERT_TRUE(ommProviderRole.base.channelEventCallback == channelEventCallback); 

		/* Prov: Received the login request above */
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

		/* Prov: Send login refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&loginRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Receive login refresh(+ flush directoryRequest) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Make sure the provider receives the directoryRequest msg */
		pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
		while (pProvMon->mutMsg.mutMsgType != MUT_MSG_RDM)
			ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);

	}

	if (ommConsumerRole.dictionaryDownloadMode == RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE)
	{
		/* Prov: Received directory request above */
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

		/* Prov: Send directory refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&directoryRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Should be using standard callbacks elsewhere */
		ASSERT_TRUE(ommProviderRole.base.channelEventCallback == channelEventCallback);
		ASSERT_TRUE(ommProviderRole.loginMsgCallback == loginMsgCallback);
		ASSERT_TRUE(ommProviderRole.directoryMsgCallback == directoryMsgCallback);

		/* Cons: Receive directory refresh(+ flush dictionaryRequests) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pConsMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH );
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* (Since more than one message is written we should get a second flush) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Make sure the  msg */
		pProvMon->mutMsg.mutMsgType = MUT_MSG_NONE;
		while (pProvMon->mutMsg.mutMsgType != MUT_MSG_RDM)
			ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);
	}



	/* Prov: (ack close) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Cons: Close(+ ack) */
	removeConnection(pConsMon, pConsCh);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

static void reactorUnitTests_DisconnectFromCallbacksInt_NiProv()
{
	RsslReactorChannel *pProvCh;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[0], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[0], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[0], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh = pProvMon->mutMsg.pReactorChannel;

	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* If loginMsgCallback provided, exchange login messages */
	if (ommNIProviderRole.loginMsgCallback)
	{
		ASSERT_TRUE(ommNIProviderRole.pLoginRequest); /* Consumer should have provided a loginRequest to test this */
		ASSERT_TRUE(ommNIProviderRole.base.channelEventCallback == channelEventCallback); /* Should be using standard callbacks elsewhere */

		/* Cons: Connection up (+ flush login request) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Prov: Receive login request */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST );

		/* Prov: Send login refresh (+ flush) */
		sendRDMMsg(pProvMon->pReactor, pProvCh, (RsslRDMMsg*)&loginRefresh, 400);
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	}

	/* At this point, whatever callback is being tested will disconnect */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);

	/* Cons: (ack close) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

static void reactorUnitTests_DisconnectFromCallbacks(RsslConnectionTypes connectionType)
{
	/*** Cons tests ***/

	/* Disconnect from connection callback */
	clearObjects();
	ommConsumerRole.base.channelEventCallback = channelEventCallbackDisconnect;
	printf("Running Cons Connection callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(false, connectionType);
	printf("Running Cons Connection callback disconnect with dispatch channel\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(true, connectionType);


	/* Disconnect from login callback */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.loginMsgCallback = loginMsgCallbackDisconnect;
	printf("Running Cons login callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(false, connectionType);
	printf("Running Cons login callback disconnect with dispatch channel\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(true, connectionType);

	/* Disconnect from directory callback */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallbackDisconnect;
	printf("Running Cons directory callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(false, connectionType);
	printf("Running Cons directory callback disconnect with dispatch channel\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(true, connectionType);

	/* Disconnect from dictionary callback */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommConsumerRole.dictionaryMsgCallback = dictionaryMsgCallbackDisconnect;
	printf("Running Cons dictionary callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(false, connectionType);
	printf("Running Cons dictionary callback disconnect with dispatch channel\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(true, connectionType);

	/* Disconnect from standard msg callback */
	clearObjects();
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.loginMsgCallback = loginMsgCallback;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommConsumerRole.directoryMsgCallback = directoryMsgCallback;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	ommConsumerRole.dictionaryMsgCallback = dictionaryMsgCallback;
	ommConsumerRole.base.defaultMsgCallback = defaultMsgCallbackDisconnect;
	printf("Running Cons default msg callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(false, connectionType);
	printf("Running Cons default msg callback disconnect with dispatch channel\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Cons(true, connectionType);


	/*** Prov tests ***/

	/* Disconnect from connection callback */
	clearObjects();
	ommProviderRole.base.channelEventCallback = channelEventCallbackDisconnect;
	printf("Running Prov channel event callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Prov(connectionType);

	/* Disconnect from login callback */
	clearObjects();
	ommProviderRole.base.channelEventCallback = channelEventCallback;
	ommProviderRole.loginMsgCallback = loginMsgCallbackDisconnect;
	ommConsumerRole.pLoginRequest = &loginRequest;
	printf("Running Prov login callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Prov(connectionType);

	/* Disconnect from directory callback */
	clearObjects();
	ommProviderRole.base.channelEventCallback = channelEventCallback;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommProviderRole.directoryMsgCallback = directoryMsgCallbackDisconnect;
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	printf("Running Prov directory callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Prov(connectionType);

	/* Disconnect from dictionary callback */
	clearObjects();
	ommProviderRole.base.channelEventCallback = channelEventCallback;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommProviderRole.directoryMsgCallback = directoryMsgCallback;
	ommProviderRole.dictionaryMsgCallback = dictionaryMsgCallbackDisconnect;
	ommConsumerRole.pLoginRequest = &loginRequest;
	ommConsumerRole.pDirectoryRequest = &directoryRequest;
	ommConsumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_FIRST_AVAILABLE;
	printf("Running Prov dictionary callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_Prov(connectionType);

	/*** NIProv tests ***/

	clearObjects();
	ommNIProviderRole.base.channelEventCallback = channelEventCallbackDisconnect;
	printf("Running NIProv channel event callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_NiProv();

	/* Disconnect from login callback */
	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommNIProviderRole.loginMsgCallback = loginMsgCallbackDisconnect;
	printf("Running NIProv login callback disconnect with dispatch all\n");
	reactorUnitTests_DisconnectFromCallbacksInt_NiProv();
}

static void reactorUnitTests_BigDirectoryMsg(RsslConnectionTypes connectionType)
{

	RsslRDMService bigDirectoryServiceList[300];
	char serviceNames[300][16];
	RsslInt32 i;
	RsslChannelInfo rsslChannelInfo;

	RsslEncodeIterator testEncodeIter;
	RsslBuffer testEncodeBuffer;
	RsslErrorInfo encodeErrorInfo;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;


	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = 2;

	for(i = 0; i < 300; ++i)
	{
		rsslClearRDMService(&bigDirectoryServiceList[i]);
		bigDirectoryServiceList[i].flags |= RDM_SVCF_HAS_INFO;
		bigDirectoryServiceList[i].info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

		bigDirectoryServiceList[i].info.serviceName.data = serviceNames[i];
		bigDirectoryServiceList[i].info.serviceName.length = snprintf(serviceNames[i], 16, "Service_%d", i);

		bigDirectoryServiceList[i].info.dictionariesProvidedList = dictionariesProvidedList;
		bigDirectoryServiceList[i].info.dictionariesProvidedCount = dictionariesProvidedCount;
		bigDirectoryServiceList[i].serviceId = 1;
	}


	clearObjects();
	ommNIProviderRole.pLoginRequest = &loginRequest;
	ommProviderRole.loginMsgCallback = loginMsgCallback;
	ommNIProviderRole.loginMsgCallback = loginMsgCallback;
	ommNIProviderRole.pDirectoryRefresh = &directoryRefresh;

	directoryRefresh.serviceList = bigDirectoryServiceList;
	directoryRefresh.serviceCount = 300;

	if (index == 1)
	{
		/* Override the default maximum message size for the WebSocket connection to make encoding error for big directory message */
		connectOpts[index].rsslConnectOptions.wsOpts.maxMsgSize = 6144;
	}

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommNIProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 100));
	ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Prov: Conn up */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pProvCh[0] = pProvMon->mutMsg.pReactorChannel;

	/* Prov: Conn ready */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Prov: (none) */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* NiProv: Conn up */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
	pConsCh[0] = pConsMon->mutMsg.pReactorChannel;

	/* The reactor always attempts to send the message by first using a buffer of size maxFragmentSize. 
	 * Test encoding with that size to ensure it will fail. */
	ASSERT_TRUE(rsslGetChannelInfo(pConsCh[0]->pRsslChannel, &rsslChannelInfo, &encodeErrorInfo.rsslError) == RSSL_RET_SUCCESS);
	testEncodeBuffer.data = (char*)alloca(rsslChannelInfo.maxFragmentSize);
	testEncodeBuffer.length = rsslChannelInfo.maxFragmentSize;
	rsslClearEncodeIterator(&testEncodeIter);
	rsslSetEncodeIteratorRWFVersion(&testEncodeIter, pConsCh[0]->majorVersion, pConsCh[0]->minorVersion);
	rsslSetEncodeIteratorBuffer(&testEncodeIter, &testEncodeBuffer);
	ASSERT_TRUE(rsslEncodeRDMMsg(&testEncodeIter, (RsslRDMMsg*)&directoryRefresh, &testEncodeBuffer.length, &encodeErrorInfo) == RSSL_RET_BUFFER_TOO_SMALL);
	ASSERT_TRUE(encodeErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL);

	/* NiProv: (flush complete) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* NiProv: (none) */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) == RSSL_RET_READ_WOULD_BLOCK);

	/* Prov: Receive Login Request */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RDM && pProvMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pProvMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST );

	/* Prov: Send login refresh (+ flush) */
	sendRDMMsg(pProvMon->pReactor, pProvMon->mutMsg.pReactorChannel, (RsslRDMMsg*)&loginRefresh, 400);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* NiProv: Receive Login Refresh */
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RDM && pConsMon->mutMsg.rdmMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN && pConsMon->mutMsg.rdmMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH );

	/* NiProv: Flush complete (if we get one -- the message is already large and could result in multiple or even zero flush calls if rsslWrite() does it) & Connection ready */
	/* (Since the events come from different threads, there's no certainty about which we will get first, though in general it should be the conn ready event) */
	{
		RsslBool gotConnReadyEvent;
		while(dispatchEvent(pConsMon, 100) != RSSL_RET_READ_WOULD_BLOCK)
		{
			if (pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) gotConnReadyEvent = RSSL_TRUE;
			else ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		}

		ASSERT_TRUE(gotConnReadyEvent);
	}

	/* Prov: Receive Directory Refresh */
	{
		RsslBool gotDirectoryRefresh = RSSL_FALSE;
		while(dispatchEvent(pProvMon, 100) != RSSL_RET_READ_WOULD_BLOCK)
		{
			if (pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL && pProvMon->mutMsg.pRsslMsg->msgBase.domainType == RSSL_DMT_SOURCE && pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_REFRESH )
				gotDirectoryRefresh = RSSL_TRUE;
			else ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		}

		ASSERT_TRUE(gotDirectoryRefresh);
	}

	/* Cons: Close (+ ack) */
	removeConnection(pConsMon, pConsCh[0]);
	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	/* Prov: Conn down */
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/* Prov: Close(+ ack) */
	removeConnection(pProvMon, pProvCh[0]);
	ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	if (index == 1)
	{
		/* Reset to original */
		connectOpts[index].rsslConnectOptions.wsOpts.maxMsgSize = 61440;
	}
}

static void reactorUnitTests_AddConnectionFromCallbacksInt_Cons(RsslInt32 reconnectAttempts, RsslConnectionTypes connectionType)
{
	RsslReactorChannel *pProvCh;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	pConsMon->reconnectAttempts = reconnectAttempts;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	do
	{
		ASSERT_TRUE(waitForConnection(pServer[index], 100));
		ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
		pProvCh = pProvMon->mutMsg.pReactorChannel;

		ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		ASSERT_TRUE(dispatchEvent(pConsMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

		/* Cons: ready */
		ASSERT_TRUE(dispatchEvent(pConsMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Prov: Disconnect connection.  */
		removeConnection(pProvMon, pProvMon->mutMsg.pReactorChannel);
		ASSERT_TRUE(dispatchEvent(pProvMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: conn down */
		ASSERT_TRUE(dispatchEvent(pConsMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

		/* Cons: (ack close) */
		ASSERT_TRUE(dispatchEvent(pConsMon, 200) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	}
	while (reconnectAttempts-- > 0);
}

static void reactorUnitTests_AddConnectionFromCallbacks(RsslConnectionTypes connectionType)
{
	/*** Cons tests ***/

	/* AddConnection from connection callback */
	clearObjects();
	ommConsumerRole.base.channelEventCallback = channelEventCallbackAddConnection;
	reactorUnitTests_AddConnectionFromCallbacksInt_Cons(0, connectionType);

	clearObjects();
	ommConsumerRole.base.channelEventCallback = channelEventCallbackAddConnection;
	reactorUnitTests_AddConnectionFromCallbacksInt_Cons(1, connectionType);

	clearObjects();
	ommConsumerRole.base.channelEventCallback = channelEventCallbackAddConnection;
	reactorUnitTests_AddConnectionFromCallbacksInt_Cons(2, connectionType);
}

RTR_C_INLINE void clearMyReactorChannel(MyReactorChannel *pInfo)
{
	memset(pInfo, 0, sizeof(MyReactorChannel));
}



static RsslReactorCallbackRet defaultMsgCallback_multiThreadDispatch(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pInfo) {
	MyReactorChannel *pMyReactorChannel = (MyReactorChannel*)pReactorChannel->userSpecPtr;
	RsslInt32 recvValue;

	pInfo->pRsslMsgBuffer->length = 4;

	memcpy(&recvValue, pInfo->pRsslMsgBuffer->data, pInfo->pRsslMsgBuffer->length);

	EXPECT_TRUE(recvValue == pMyReactorChannel->msgsToRecv);
	--pMyReactorChannel->msgsToRecv;

	return RSSL_RC_CRET_SUCCESS;
}

/* Thread for reactorUnitTests_MultiThreadDispatch */
RSSL_THREAD_DECLARE(reactorUnitTests_pingPongThread, pArg)
{
	MyReactorChannel *pMyReactorChannel = (MyReactorChannel*)pArg;
	RsslReactorChannel *pReactorChannel = pMyReactorChannel->pReactorChannel;
	MyReactor *pMyReactor = pMyReactorChannel->pMyReactor;
	RsslReactor *pReactor = pMyReactor->pReactor;

	RsslReactorSubmitOptions submitOpts;
	RsslReactorDispatchOptions dispatchOpts;

	rsslClearReactorSubmitOptions(&submitOpts);
	rsslClearReactorDispatchOptions(&dispatchOpts);

	dispatchOpts.maxMessages = 1;
	dispatchOpts.pReactorChannel = pReactorChannel;

	while(pMyReactorChannel->msgsToRecv > 0 || pMyReactorChannel->msgsToSend > 0)
	{
		fd_set readFds;
		RsslRet ret;
		struct timeval selectTime;

		if (pMyReactorChannel->msgsToSend)
		{
			RsslBuffer *pMsgBuf = rsslReactorGetBuffer(pReactorChannel, 4, RSSL_FALSE, &rsslErrorInfo);

			EXPECT_TRUE(pMsgBuf);

			memcpy(pMsgBuf->data, &pMyReactorChannel->msgsToSend, 4);
			pMsgBuf->length = 4;
			rsslReactorSubmit(pReactor, pReactorChannel, pMsgBuf, &submitOpts, &rsslErrorInfo);
			--pMyReactorChannel->msgsToSend;
		}
		else
		{
			/* No longer sending messages -- wait for remaining messages so we don't hog the lock. */
			/* TODO This is necessary because of the big lock used in the reactor.  Once a better
			 * locking scheme is implemented we should be able to remove this. */
			FD_ZERO(&readFds);
			FD_SET(pReactorChannel->socketId, &readFds);
			selectTime.tv_sec = 0;
			selectTime.tv_usec = 1000;
			select(FD_SETSIZE, &readFds, NULL, NULL, NULL);
		}

		while((ret = rsslReactorDispatch(pMyReactor->pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS);
		EXPECT_TRUE(ret == RSSL_RET_SUCCESS || ret == RSSL_RET_READ_WOULD_BLOCK);
	}

	return 0;
}

static void reactorUnitTests_MultiThreadDispatch(RsslConnectionTypes connectionType)
{
	RsslThreadId threadId1, threadId2;
	MyReactorChannel myReactorChannel1, myReactorChannel2;
	RsslReactorOMMConsumerRole role;
	MyReactor *pMyReactor = &myReactors[0];
	RsslReactor *pReactor = pMyReactor->pReactor;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	/* Create two threads that work on different connections on the same reactor.
	 * Each will send a receive a given number of messages. */

	clearObjects();

	clearMyReactorChannel(&myReactorChannel1);
	clearMyReactorChannel(&myReactorChannel2);

	rsslClearOMMConsumerRole(&role);
	role.base.channelEventCallback = channelEventCallback;
	role.base.defaultMsgCallback = defaultMsgCallback_multiThreadDispatch;

	myReactorChannel1.pMyReactor = pMyReactor;
	myReactorChannel1.msgsToSend = myReactorChannel1.msgsToRecv =  10000;
	myReactorChannel1.isServer = RSSL_TRUE;
	connectOpts[index].rsslConnectOptions.userSpecPtr = &myReactorChannel1;

	myReactorChannel2.pMyReactor = pMyReactor;
	myReactorChannel2.msgsToSend = myReactorChannel1.msgsToRecv;
	myReactorChannel2.msgsToRecv = myReactorChannel1.msgsToSend;
	myReactorChannel2.isServer = RSSL_FALSE;
	acceptOpts.rsslAcceptOptions.userSpecPtr = &myReactorChannel2;

	/* Start connections */
	ASSERT_TRUE(rsslReactorConnect(pMyReactor->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&role, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(waitForConnection(pServer[index], 100));
	ASSERT_TRUE(rsslReactorAccept(pMyReactor->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&role, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Wait for connections */
	while(!myReactorChannel1.pReactorChannel || !myReactorChannel2.pReactorChannel)
	{
		ASSERT_TRUE(dispatchEvent(pMyReactor, 400) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pMyReactor->mutMsg.mutMsgType == MUT_MSG_CONN); 
		ASSERT_TRUE(pMyReactor->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP
				|| pMyReactor->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	}

	/* Start threads */
	RSSL_THREAD_START(&threadId1, reactorUnitTests_pingPongThread, &myReactorChannel1);
	RSSL_THREAD_START(&threadId2, reactorUnitTests_pingPongThread, &myReactorChannel2);

	/* Wait for threads. They should exit once they have sent and received all messages. */
	RSSL_THREAD_JOIN(threadId1);
	RSSL_THREAD_JOIN(threadId2);


	/* Cleanup */
	{
		RsslRet ret;

		removeConnection(pMyReactor, myReactorChannel1.pReactorChannel);

		while(ret = dispatchEvent(pMyReactor, 100) != RSSL_RET_READ_WOULD_BLOCK)
		{
			ASSERT_TRUE(ret >= RSSL_RET_SUCCESS);

			if (pMyReactor->mutMsg.mutMsgType == MUT_MSG_CONN && pMyReactor->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN)
			{
				ASSERT_TRUE(pMyReactor->mutMsg.pReactorChannel = myReactorChannel2.pReactorChannel);
				removeConnection(pMyReactor, myReactorChannel2.pReactorChannel);
			}
			else
				ASSERT_TRUE(pMyReactor->mutMsg.mutMsgType == MUT_MSG_NONE);
		}
	}
}

/* Sleeps for one second when channel goes down. */
static RsslReactorCallbackRet channelEventCallbackWait(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent)
{
	MyReactor *pMyReactor = (MyReactor*)pReactor->userSpecPtr;
	MutMsg *pMutMsg = &pMyReactor->mutMsg;

	EXPECT_TRUE(pReactor);
	EXPECT_TRUE(pReactorChannel);
	EXPECT_TRUE(pEvent);

	copyMutConnectionEvent(pMutMsg, pEvent, pReactorChannel);

	/* Call normal callback */
	channelEventCallback(pReactor, pReactorChannel, pEvent);

	if(pEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING
			|| pEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN)
	{
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}
	return RSSL_RC_CRET_SUCCESS;
}

void reactorUnitTests_WaitWhileChannelDown(RsslConnectionTypes connectionType)
{
	/* When reconnecting, test that only one channel event can be received per disconnection. See ETA-1956. */
	RsslServer *pRsslServer;
	RsslReactorChannel *pProvCh, *pConsCh;
	RsslBindOptions rsslBindOpts;
	RsslReactorDispatchOptions dispatchOpts;
	int i;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	rsslClearReactorDispatchOptions(&dispatchOpts);
	dispatchOpts.maxMessages = 1;

	connectOpts[index].rsslConnectOptions.pingTimeout = 1;
	connectOpts[index].reconnectAttemptLimit = -1;
	connectOpts[index].reconnectMinDelay = 500;
	connectOpts[index].reconnectMaxDelay = 500;
	pConsMon->closeConnections = RSSL_FALSE;

	/* Callback sleeps for a second when the channel goes down. This gives the worker a chance detect failure on ping and send an extra 
	 * channel event (which the reactor should ignore). */
	ommConsumerRole.base.channelEventCallback = channelEventCallbackWait;

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = const_cast<char*>("14011");
	rsslBindOpts.pingTimeout = 1;
	rsslBindOpts.minPingTimeout = 1;
	if (index == 1)
	{
		rsslBindOpts.wsOpts.protocols = const_cast<char*>("rssl.json.v2");
	}

	ASSERT_TRUE((pRsslServer = rsslBind(&rsslBindOpts, &rsslErrorInfo.rsslError)));

	connectOpts[index].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");

	/*** Test initialization on connection ***/

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	for (i = 0; i < 3; ++i)
	{
		ASSERT_TRUE(waitForConnection(pRsslServer, 1000));
		ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pRsslServer, &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Cons: Should get conn up/ready event */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
		pConsCh = pConsMon->mutMsg.pReactorChannel;
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Prov: Should get conn up/ready event */
		ASSERT_TRUE(dispatchEvent(pProvMon, 1000) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
		pProvCh = pProvMon->mutMsg.pReactorChannel;
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Prov: Close channel. */
		removeConnection(pProvMon, pProvCh);

		/* Prov: No message(close ack) */
		ASSERT_TRUE(dispatchEvent(pProvMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: One channel-down/reconnecting event. */
		ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
		ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == i+1);
		ASSERT_TRUE(pConsMon->channelDownEventCount == 0);

		/* Cons: Redudnant channel-down event from the worker (should not be passed to consumer). */
		if (index != 1)
		{
			ASSERT_TRUE(dispatchEvent(pConsMon, 1000) >= RSSL_RET_SUCCESS);
			ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		}
	}

	removeConnection(pConsMon, pConsCh);

	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	ASSERT_TRUE(rsslCloseServer(pRsslServer, &rsslErrorInfo.rsslError) == RSSL_RET_SUCCESS); 
}

void reactorUnitTests_ReconnectAttemptLimit(RsslConnectionTypes connectionType)
{
	/* Test a nonzero reconnectAttemptLimit to ensure correct number of down_reconnecting/down events are received.
	 * Test also uses an invalid hostname to ensure rsslConnect fails (as opposed to the ReactorWorker failing to initialize it). See ETA-2613. */
	RsslReactorChannel *pConsCh;
	RsslBindOptions rsslBindOpts;
	RsslReactorDispatchOptions dispatchOpts;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	rsslClearReactorDispatchOptions(&dispatchOpts);
	dispatchOpts.maxMessages = 1;

	connectOpts[index].reconnectAttemptLimit = 2;
	connectOpts[index].reconnectMinDelay = 500;
	connectOpts[index].reconnectMaxDelay = 500;
	pConsMon->closeConnections = RSSL_FALSE;

	/* Callback sleeps for a second when the channel goes down. This gives the worker a chance detect failure on ping and send an extra 
	 * channel event (which the reactor should ignore). */
	ommConsumerRole.base.channelEventCallback = channelEventCallbackWait;

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = const_cast<char*>("14012");

	connectOpts[index].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("invalid hostname!");
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");

	/*** Test initialization on connection ***/

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

	/* Cons: Channel-down/reconnecting event. */
	/* Don't dispatch -- the channel event callback was already run by rsslReactorConnect */
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 1);
	ASSERT_TRUE(pConsMon->channelDownEventCount == 0);
	pConsCh = pConsMon->mutMsg.pReactorChannel;

	/* Cons: Channel-down/reconnecting event. */
	while(dispatchEvent(pConsMon, 800) != RSSL_RET_SUCCESS);;
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 2);
	ASSERT_TRUE(pConsMon->channelDownEventCount == 0);

	/* Cons: Channel-down event. */
	while(dispatchEvent(pConsMon, 800) != RSSL_RET_SUCCESS);;
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
	ASSERT_TRUE(pConsMon->channelDownReconnectingEventCount == 2);
	ASSERT_TRUE(pConsMon->channelDownEventCount == 1);

	removeConnection(pConsMon, pConsCh);

	ASSERT_TRUE(dispatchEvent(pConsMon, 100) >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
}

#ifdef COMPILE_64BITS
void reactorUnitTests_ManyConnections(RsslConnectionTypes connectionType)
{

	/* Test using a very large number of connections between two reactors -- open them, exchange messages between them, and close them.
	 * Ensures that the reactor and notification can handle this. 
	 * 
	 * NOTE: A lot of the notification triggering in this test is going to the receipt of ping messages. The test calls dispatchEvents() 
	 * instead of dispatchEvent() so that rsslReactorDispatch can be called with a high enough maxMessages parameter that we still receive the desired events amid the pings. 
	 */

	int i;
	int numConnections = 1500; /* The number of connections must be set according to the performance of testing machine. */
	MyReactorChannel *myConsumerChannels;
	MyReactorChannel *myProviderChannels;
	RsslRet rsslRet;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	if (index == 1) numConnections -= 500;

#ifndef WIN32
	struct rlimit rlimit;
	ASSERT_TRUE(getrlimit(RLIMIT_NOFILE, &rlimit) == 0);

	if (rlimit.rlim_cur < 50)
	{
		printf("Warning: File descriptor limit very low; skipping test.\n");
		return;
	}
	else if (rlimit.rlim_cur < numConnections * 2 + 50)
	{
		printf("  Warning: Total number of connections reduced due to file descriptor limit. Test should work but is intended for a limit of at least %d files.\n", numConnections * 2 + 50);
		numConnections = rlimit.rlim_cur / 2 - 50;
	}

	printf("  File descriptor limit is %d. Test will open %d reactor connections between consumer & provider.\n", rlimit.rlim_cur, numConnections);
#else
	numConnections > MAX_REACTOR_CONS ? MAX_REACTOR_CONS : numConnections;
	printf("  FD_SETSIZE is %d. Test will open %d reactor connections between consumer & provider.\n", FD_SETSIZE, numConnections);
#endif

	myConsumerChannels = (MyReactorChannel*)calloc(sizeof(MyReactorChannel), numConnections);
	myProviderChannels = (MyReactorChannel*)calloc(sizeof(MyReactorChannel), numConnections);
	ASSERT_TRUE(myConsumerChannels != NULL);
	ASSERT_TRUE(myProviderChannels != NULL);

	clearObjects();

	/* Create notifiers. */
	pProvMon->pNotifier = rsslCreateNotifier(1024);
	ASSERT_TRUE(pProvMon->pNotifier != NULL);
	pConsMon->pNotifier = rsslCreateNotifier(1024);
	ASSERT_TRUE(pConsMon->pNotifier != NULL);

	/* Add notification for cons/prov reactor's event queue. */
	pConsMon->pReactorNotifierEvent = rsslCreateNotifierEvent();
	ASSERT_TRUE(pConsMon->pReactorNotifierEvent != NULL);
	ASSERT_TRUE(rsslNotifierAddEvent(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent, pConsMon->pReactor->eventFd, pConsMon) == 0);
	ASSERT_TRUE(rsslNotifierRegisterRead(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent) == 0);
	pProvMon->pReactorNotifierEvent = rsslCreateNotifierEvent();
	ASSERT_TRUE(pProvMon->pReactorNotifierEvent != NULL);
	ASSERT_TRUE(rsslNotifierAddEvent(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent, pProvMon->pReactor->eventFd, pProvMon) == 0);
	ASSERT_TRUE(rsslNotifierRegisterRead(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent) == 0);


	/* Open connections */
	for (i = 0; i < numConnections; ++i)
	{
		/* Cons: Connect client */
		connectOpts[index].rsslConnectOptions.userSpecPtr = &myConsumerChannels[i];
		ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Prov: Accept client connection */
		while( waitForConnection(pServer[index], 200) == false);;
		acceptOpts.rsslAcceptOptions.userSpecPtr = &myProviderChannels[i];
		ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Prov: dispatch; last received event should be conn ready */
		do { rsslRet = dispatchEvents(pProvMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || RSSL_RET_READ_WOULD_BLOCK);} while (pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Cons: dispatch; last received event should be conn ready */
		do { rsslRet = dispatchEvents(pConsMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || RSSL_RET_READ_WOULD_BLOCK);} while (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		ASSERT_TRUE(myConsumerChannels[i].pReactorChannel != NULL);
		ASSERT_TRUE(myProviderChannels[i].pReactorChannel != NULL);
	}

	/* Send an RsslGenericMsg over each connection, in each direction. */
	for (i = 0; i < numConnections; ++i)
	{
		RsslReactorSubmitMsgOptions submitMsgOpts;
		RsslGenericMsg genericMsg, *pGenericMsg;

		/* Consumer to provider */
		rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
		rsslClearGenericMsg(&genericMsg);
		genericMsg.msgBase.streamId = i;
		genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		submitMsgOpts.pRsslMsg = (RsslMsg*)&genericMsg;
		ASSERT_TRUE(rsslReactorSubmitMsg(pConsMon->pReactor, myConsumerChannels[i].pReactorChannel, &submitMsgOpts, &rsslErrorInfo) >= RSSL_RET_SUCCESS);

		do { rsslRet = dispatchEvents(pProvMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || rsslRet == RSSL_RET_READ_WOULD_BLOCK);} while (pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_RSSL );
		ASSERT_TRUE(pProvMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC );
		pGenericMsg = (RsslGenericMsg*)pProvMon->mutMsg.pRsslMsg;
		ASSERT_TRUE(pGenericMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pGenericMsg->msgBase.containerType == RSSL_DT_NO_DATA);

		/* Provider to consumer */
		rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
		rsslClearGenericMsg(&genericMsg);
		genericMsg.msgBase.streamId = i;
		genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		submitMsgOpts.pRsslMsg = (RsslMsg*)&genericMsg;
		ASSERT_TRUE(rsslReactorSubmitMsg(pProvMon->pReactor, myProviderChannels[i].pReactorChannel, &submitMsgOpts, &rsslErrorInfo) >= RSSL_RET_SUCCESS);

		do {rsslRet = dispatchEvents(pConsMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || rsslRet == RSSL_RET_READ_WOULD_BLOCK);} while (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_RSSL );
		ASSERT_TRUE(pConsMon->mutMsg.pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC );
		pGenericMsg = (RsslGenericMsg*)pConsMon->mutMsg.pRsslMsg;
		ASSERT_TRUE(pGenericMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pGenericMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	}

	/* Close connections */
	for (i = 0; i < numConnections; ++i)
	{
		/* Prov: close (& ack close)  */
		removeConnection(pProvMon, myProviderChannels[i].pReactorChannel);
		do { rsslRet = dispatchEvents(pProvMon, 200, 1000); } while ( rsslRet == RSSL_RET_READ_WOULD_BLOCK);
		ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

		/* Cons: Conn down */
		do { rsslRet = dispatchEvents(pConsMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || rsslRet == RSSL_RET_READ_WOULD_BLOCK);} while (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);
	}

	rsslNotifierRemoveEvent(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent);
	rsslNotifierRemoveEvent(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent);

	rsslDestroyNotifierEvent(pProvMon->pReactorNotifierEvent);
	rsslDestroyNotifierEvent(pConsMon->pReactorNotifierEvent);
	rsslDestroyNotifier(pConsMon->pNotifier);
	rsslDestroyNotifier(pProvMon->pNotifier);

	free(myConsumerChannels);
	free(myProviderChannels);
}

void reactorUnitTests_EventPoolSize(RsslConnectionTypes connectionType)
{
	int i;
	int numConnections = 100; /* The number of connections must be set according to the performance of testing machine. */
	MyReactorChannel myConsumerChannels[100] = {0};
	MyReactorChannel myProviderChannels[100] = {0};
	RsslRet rsslRet;
	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	///*Set size of pool*/
	rsslClearCreateReactorOptions(&mOpts);
	mOpts.maxEventsInPool = 1;
	initReactors(&mOpts, RSSL_FALSE);

	/* Create notifiers. */
	pProvMon->pNotifier = rsslCreateNotifier(1024);
	ASSERT_TRUE(pProvMon->pNotifier != NULL);
	pConsMon->pNotifier = rsslCreateNotifier(1024);
	ASSERT_TRUE(pConsMon->pNotifier != NULL);

	/* Add notification for cons/prov reactor's event queue. */
	pConsMon->pReactorNotifierEvent = rsslCreateNotifierEvent();
	ASSERT_TRUE(pConsMon->pReactorNotifierEvent != NULL);
	ASSERT_TRUE(rsslNotifierAddEvent(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent, pConsMon->pReactor->eventFd, pConsMon) == 0);
	ASSERT_TRUE(rsslNotifierRegisterRead(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent) == 0);
	pProvMon->pReactorNotifierEvent = rsslCreateNotifierEvent();
	ASSERT_TRUE(pProvMon->pReactorNotifierEvent != NULL);
	ASSERT_TRUE(rsslNotifierAddEvent(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent, pProvMon->pReactor->eventFd, pProvMon) == 0);
	ASSERT_TRUE(rsslNotifierRegisterRead(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent) == 0);

	MyReactorImpl *pMyConsReactorImpl = (MyReactorImpl*)pConsMon->pReactor;
	MyRsslReactorWorker *myConsReacotrWorker = &(pMyConsReactorImpl->reactorWorker);
	RsslQueue *evtPoolCons = &(myConsReacotrWorker->workerQueue.eventPool);

	/*Check pool size before connection*/
	ASSERT_TRUE((RsslInt32)evtPoolCons->count > mOpts.maxEventsInPool);

	/* Open connections */
	for (i = 0; i < numConnections; ++i)
	{
		/* Cons: Connect client */
		connectOpts[index].rsslConnectOptions.userSpecPtr = &myConsumerChannels[i];
		ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Prov: Accept client connection */
		while (waitForConnection(pServer[index], 200) == false);;
		acceptOpts.rsslAcceptOptions.userSpecPtr = &myProviderChannels[i];
		ASSERT_TRUE(rsslReactorAccept(pProvMon->pReactor, pServer[index], &acceptOpts, (RsslReactorChannelRole*)&ommProviderRole, &rsslErrorInfo) == RSSL_RET_SUCCESS);

		/* Prov: dispatch; last received event should be conn ready */
		do { rsslRet = dispatchEvents(pProvMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || RSSL_RET_READ_WOULD_BLOCK); } while (pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_CONN && pProvMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		/* Cons: dispatch; last received event should be conn ready */
		do { rsslRet = dispatchEvents(pConsMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || RSSL_RET_READ_WOULD_BLOCK); } while (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
		ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

		ASSERT_TRUE(myConsumerChannels[i].pReactorChannel != NULL);
		ASSERT_TRUE(myProviderChannels[i].pReactorChannel != NULL);
	}

	/*Check pool size after connection*/
	ASSERT_TRUE((RsslInt32)evtPoolCons->count <= mOpts.maxEventsInPool);

	/* Close connections */
	for (i = 0; i < numConnections; ++i)
	{
		removeConnection(pProvMon, myProviderChannels[i].pReactorChannel);
	}

	/*Check pool size after disconnection*/
	ASSERT_TRUE((RsslInt32)evtPoolCons->count <= mOpts.maxEventsInPool);

	do { rsslRet = dispatchEvents(pProvMon, 200, 1000); } while (rsslRet == RSSL_RET_READ_WOULD_BLOCK);
	ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS);
	ASSERT_TRUE(pProvMon->mutMsg.mutMsgType == MUT_MSG_NONE);

	///* Cons: Conn down */
	do { rsslRet = dispatchEvents(pConsMon, 200, 1000); ASSERT_TRUE(rsslRet >= RSSL_RET_SUCCESS || rsslRet == RSSL_RET_READ_WOULD_BLOCK); } while (pConsMon->mutMsg.mutMsgType == MUT_MSG_NONE);
	ASSERT_TRUE(pConsMon->mutMsg.mutMsgType == MUT_MSG_CONN && pConsMon->mutMsg.channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	/*Check pool size after dispaticing*/
	ASSERT_TRUE((RsslInt32)evtPoolCons->count <= mOpts.maxEventsInPool);

	rsslNotifierRemoveEvent(pConsMon->pNotifier, pConsMon->pReactorNotifierEvent);
	rsslNotifierRemoveEvent(pProvMon->pNotifier, pProvMon->pReactorNotifierEvent);

	rsslDestroyNotifierEvent(pProvMon->pReactorNotifierEvent);
	rsslDestroyNotifierEvent(pConsMon->pReactorNotifierEvent);
	rsslDestroyNotifier(pConsMon->pNotifier);
	rsslDestroyNotifier(pProvMon->pNotifier);

	pConsMon->pNotifier = pProvMon->pNotifier = NULL;
	pProvMon->pReactorNotifierEvent = pConsMon->pReactorNotifierEvent = NULL;

	/*Set value back to default*/
	mOpts.maxEventsInPool = -1;
	initReactors(&mOpts, RSSL_FALSE);
}
#endif

void reactorUtilTest_ConnectDeepCopy(RsslConnectionTypes connectionType)
{
	RsslConnectOptions inOpts, outOpts;
	
	rsslClearConnectOpts(&inOpts);
	rsslClearConnectOpts(&outOpts);
	
	inOpts.hostName = const_cast<char*>("testHost");
	inOpts.serviceName = const_cast<char*>("14000");
	inOpts.objectName = const_cast<char*>("testName");
	inOpts.connectionType = connectionType;
	inOpts.connectionInfo.segmented.recvAddress = const_cast<char*>("123.456.789");
	inOpts.connectionInfo.segmented.recvServiceName = const_cast<char*>("12343");
	inOpts.connectionInfo.segmented.interfaceName = const_cast<char*>("firstNIC");
	inOpts.connectionInfo.segmented.sendAddress = const_cast<char*>("987.654.321");
	inOpts.connectionInfo.segmented.sendServiceName = const_cast<char*>("54321");
	inOpts.compressionType = RSSL_COMP_LZ4;
	inOpts.blocking = RSSL_TRUE;
	inOpts.tcp_nodelay = RSSL_TRUE;
	inOpts.pingTimeout = 120;
	inOpts.guaranteedOutputBuffers = 25;
	inOpts.numInputBuffers = 100;
	inOpts.majorVersion = 3;
	inOpts.minorVersion = 4;
	inOpts.protocolType = 123;
	inOpts.userSpecPtr = (void*)&myReactors[0];
	inOpts.tcpOpts.tcp_nodelay = RSSL_TRUE;
	inOpts.multicastOpts.flags = 0x01;
	inOpts.multicastOpts.disconnectOnGaps = RSSL_TRUE;
	inOpts.multicastOpts.packetTTL = 3;
	inOpts.multicastOpts.ndata = 1;
	inOpts.multicastOpts.nrreq = 4;
	inOpts.multicastOpts.tdata = 5;
	inOpts.multicastOpts.trreq = 6;
	inOpts.multicastOpts.twait = 6;	
	inOpts.multicastOpts.tbchold = 2;
	inOpts.multicastOpts.tpphold = 1;
	inOpts.multicastOpts.userQLimit = 12345;
	inOpts.multicastOpts.nmissing = 12;
	inOpts.multicastOpts.pktPoolLimitHigh = 19;
	inOpts.multicastOpts.pktPoolLimitLow = 18;
	inOpts.multicastOpts.hsmInterface = const_cast<char*>("nic3");
	inOpts.multicastOpts.hsmMultAddress = const_cast<char*>("321.654.987");
	inOpts.multicastOpts.hsmPort = const_cast<char*>("1233");
	inOpts.multicastOpts.hsmInterval = 9;
	inOpts.multicastOpts.tcpControlPort = const_cast<char*>("4321");
	inOpts.multicastOpts.portRoamRange = 4;
	inOpts.shmemOpts.maxReaderLag = 8;
	inOpts.sysSendBufSize = 1;
	inOpts.sysRecvBufSize = 2;
	inOpts.seqMulticastOpts.maxMsgSize = 12345;
	inOpts.seqMulticastOpts.instanceId = 8;
	inOpts.proxyOpts.proxyHostName = const_cast<char*>("proxy");
	inOpts.proxyOpts.proxyPort = const_cast<char*>("1234");
	inOpts.componentVersion = const_cast<char*>("5");
	inOpts.encryptionOpts.encryptionProtocolFlags = RSSL_ENC_TLSV1_2;

	if (connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		inOpts.wsOpts.protocols = const_cast<char*>("rssl.rwf, rssl.json.v2, tr_json2");
		inOpts.wsOpts.maxMsgSize = 55555;
	}
	
	rsslDeepCopyConnectOpts(&outOpts, &inOpts);
	
	ASSERT_TRUE(inOpts.hostName != outOpts.hostName);
	ASSERT_TRUE(strcmp((const char*)inOpts.hostName, (const char*)outOpts.hostName) == 0);
	ASSERT_TRUE(inOpts.serviceName != outOpts.serviceName);
	ASSERT_TRUE(strcmp((const char*)inOpts.serviceName, (const char*)outOpts.serviceName) == 0);
	ASSERT_TRUE(inOpts.objectName != outOpts.objectName);
	ASSERT_TRUE(strcmp((const char*)inOpts.objectName, (const char*)outOpts.objectName) == 0);
	ASSERT_TRUE(inOpts.connectionType == outOpts.connectionType);
	ASSERT_TRUE(inOpts.connectionInfo.segmented.recvAddress != outOpts.connectionInfo.segmented.recvAddress);
	ASSERT_TRUE(strcmp((const char*)inOpts.connectionInfo.segmented.recvAddress, (const char*)outOpts.connectionInfo.segmented.recvAddress) == 0);
	ASSERT_TRUE(inOpts.connectionInfo.segmented.recvServiceName != outOpts.connectionInfo.segmented.recvServiceName);
	ASSERT_TRUE(strcmp((const char*)inOpts.connectionInfo.segmented.recvServiceName, (const char*)outOpts.connectionInfo.segmented.recvServiceName) == 0);
	ASSERT_TRUE(inOpts.connectionInfo.segmented.interfaceName != outOpts.connectionInfo.segmented.interfaceName);
	ASSERT_TRUE(strcmp((const char*)inOpts.connectionInfo.segmented.interfaceName, (const char*)outOpts.connectionInfo.segmented.interfaceName) == 0);
	ASSERT_TRUE(inOpts.connectionInfo.segmented.sendAddress != outOpts.connectionInfo.segmented.sendAddress);
	ASSERT_TRUE(strcmp((const char*)inOpts.connectionInfo.segmented.sendAddress, (const char*)outOpts.connectionInfo.segmented.sendAddress) == 0);
	ASSERT_TRUE(inOpts.connectionInfo.segmented.sendServiceName != outOpts.connectionInfo.segmented.sendServiceName);
	ASSERT_TRUE(strcmp((const char*)inOpts.connectionInfo.segmented.sendServiceName, (const char*)outOpts.connectionInfo.segmented.sendServiceName) == 0);
	ASSERT_TRUE(inOpts.compressionType == outOpts.compressionType);
	ASSERT_TRUE(inOpts.blocking == outOpts.blocking);
	ASSERT_TRUE(inOpts.tcp_nodelay == outOpts.tcp_nodelay);
	ASSERT_TRUE(inOpts.pingTimeout == outOpts.pingTimeout);
	ASSERT_TRUE(inOpts.guaranteedOutputBuffers == outOpts.guaranteedOutputBuffers);
	ASSERT_TRUE(inOpts.numInputBuffers == outOpts.numInputBuffers);
	ASSERT_TRUE(inOpts.majorVersion == outOpts.majorVersion);
	ASSERT_TRUE(inOpts.minorVersion == outOpts.minorVersion);
	ASSERT_TRUE(inOpts.protocolType == outOpts.protocolType);
	ASSERT_TRUE(inOpts.userSpecPtr == outOpts.userSpecPtr);
	ASSERT_TRUE(inOpts.tcpOpts.tcp_nodelay == outOpts.tcpOpts.tcp_nodelay);
	ASSERT_TRUE(inOpts.multicastOpts.flags == outOpts.multicastOpts.flags);
	ASSERT_TRUE(inOpts.multicastOpts.disconnectOnGaps == outOpts.multicastOpts.disconnectOnGaps);
	ASSERT_TRUE(inOpts.multicastOpts.packetTTL == outOpts.multicastOpts.packetTTL);
	ASSERT_TRUE(inOpts.multicastOpts.ndata == outOpts.multicastOpts.ndata);
	ASSERT_TRUE(inOpts.multicastOpts.nrreq == outOpts.multicastOpts.nrreq);
	ASSERT_TRUE(inOpts.multicastOpts.tdata == outOpts.multicastOpts.tdata);
	ASSERT_TRUE(inOpts.multicastOpts.trreq == outOpts.multicastOpts.trreq);
	ASSERT_TRUE(inOpts.multicastOpts.twait == outOpts.multicastOpts.twait);
	ASSERT_TRUE(inOpts.multicastOpts.tbchold == outOpts.multicastOpts.tbchold);
	ASSERT_TRUE(inOpts.multicastOpts.tpphold == outOpts.multicastOpts.tpphold);
	ASSERT_TRUE(inOpts.multicastOpts.userQLimit == outOpts.multicastOpts.userQLimit);
	ASSERT_TRUE(inOpts.multicastOpts.nmissing == outOpts.multicastOpts.nmissing);
	ASSERT_TRUE(inOpts.multicastOpts.pktPoolLimitHigh == outOpts.multicastOpts.pktPoolLimitHigh);
	ASSERT_TRUE(inOpts.multicastOpts.pktPoolLimitLow == outOpts.multicastOpts.pktPoolLimitLow);
	ASSERT_TRUE(inOpts.multicastOpts.hsmInterface != outOpts.multicastOpts.hsmInterface);
	ASSERT_TRUE(strcmp((const char*)inOpts.multicastOpts.hsmInterface, (const char*)outOpts.multicastOpts.hsmInterface) == 0);
	ASSERT_TRUE(inOpts.multicastOpts.hsmMultAddress != outOpts.multicastOpts.hsmMultAddress);
	ASSERT_TRUE(strcmp((const char*)inOpts.multicastOpts.hsmMultAddress, (const char*)outOpts.multicastOpts.hsmMultAddress) == 0);
	ASSERT_TRUE(inOpts.multicastOpts.hsmPort != outOpts.multicastOpts.hsmPort);
	ASSERT_TRUE(strcmp((const char*)inOpts.multicastOpts.hsmPort, (const char*)outOpts.multicastOpts.hsmPort) == 0);
	ASSERT_TRUE(inOpts.multicastOpts.hsmInterval == outOpts.multicastOpts.hsmInterval);
	ASSERT_TRUE(inOpts.multicastOpts.tcpControlPort != outOpts.multicastOpts.tcpControlPort);
	ASSERT_TRUE(strcmp((const char*)inOpts.multicastOpts.tcpControlPort, (const char*)outOpts.multicastOpts.tcpControlPort) == 0);
	ASSERT_TRUE(inOpts.multicastOpts.portRoamRange == outOpts.multicastOpts.portRoamRange);
	ASSERT_TRUE(inOpts.shmemOpts.maxReaderLag == outOpts.shmemOpts.maxReaderLag);	
	ASSERT_TRUE(inOpts.sysSendBufSize == outOpts.sysSendBufSize);
	ASSERT_TRUE(inOpts.sysRecvBufSize == outOpts.sysRecvBufSize);
	ASSERT_TRUE(inOpts.seqMulticastOpts.maxMsgSize == outOpts.seqMulticastOpts.maxMsgSize);
	ASSERT_TRUE(inOpts.seqMulticastOpts.instanceId == outOpts.seqMulticastOpts.instanceId);
	ASSERT_TRUE(inOpts.proxyOpts.proxyHostName != outOpts.proxyOpts.proxyHostName);
	ASSERT_TRUE(strcmp((const char*)inOpts.proxyOpts.proxyHostName, (const char*)outOpts.proxyOpts.proxyHostName) == 0);
	ASSERT_TRUE(inOpts.proxyOpts.proxyPort != outOpts.proxyOpts.proxyPort);
	ASSERT_TRUE(strcmp((const char*)inOpts.proxyOpts.proxyPort, (const char*)outOpts.proxyOpts.proxyPort) == 0);
	ASSERT_TRUE(inOpts.componentVersion != outOpts.componentVersion);
	ASSERT_TRUE(strcmp((const char*)inOpts.componentVersion, (const char*)outOpts.componentVersion) == 0);
	ASSERT_TRUE(inOpts.encryptionOpts.encryptionProtocolFlags == outOpts.encryptionOpts.encryptionProtocolFlags);

	if (connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		ASSERT_TRUE(inOpts.wsOpts.protocols != outOpts.wsOpts.protocols);
		ASSERT_STREQ(inOpts.wsOpts.protocols, outOpts.wsOpts.protocols);
		ASSERT_TRUE(inOpts.wsOpts.maxMsgSize == outOpts.wsOpts.maxMsgSize);
	}
	
	rsslFreeConnectOpts(&outOpts);
}

void reactorUnitTests_InvalidNetworkInterface(RsslConnectionTypes connectionType)
{
	RsslBindOptions rsslBindOpts;

	int index = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? 1 : 0;

	clearObjects();

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = const_cast<char*>("443");

	connectOpts[index].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_ENCRYPTED;
	connectOpts[index].rsslConnectOptions.encryptionOpts.encryptedProtocol = (connectionType == RSSL_CONN_TYPE_WEBSOCKET) ? RSSL_CONN_TYPE_WEBSOCKET : RSSL_CONN_TYPE_SOCKET;

	connectOpts[index].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("untrusted-root.badssl.com");
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("443");

	/*The interface name is valid but does not allow access remote address.*/
	connectOpts[index].rsslConnectOptions.connectionInfo.unified.interfaceName = const_cast<char*>("192.168.56.1");

	connectOpts[index].connectionCount = 0;
	connectOpts[index].reactorConnectionList = 0;
	connectOpts[index].reconnectAttemptLimit = 0;

	ASSERT_TRUE(rsslReactorConnect(pConsMon->pReactor, &connectOpts[index], (RsslReactorChannelRole*)&ommConsumerRole, &rsslErrorInfo) == RSSL_RET_FAILURE);
	ASSERT_TRUE(rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_FAILURE);
}
