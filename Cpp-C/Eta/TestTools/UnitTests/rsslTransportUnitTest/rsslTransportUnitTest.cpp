/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2023, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */
 
/************************************************************************
 *	Transport Unit Test
 *
 *  Unit testing for the ETA Transport.
 *  Includes multithreaded tests.
 *
 /**********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <iostream>
#include <atomic>
#include <sstream>
#include <vector>
#include <iterator>

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"
#include "rtr/ripcutils.h"
#include "rtr/rsslEventSignal.h"
#include "rtr/ripcsslutils.h"
#include "rtr/rsslGetTime.h"
#include "rtr/rsslCurlJIT.h"
#include "rtr/ripcflip.h"
#include "rtr/ripc_int.h"
#include <curl/curl.h>

#include "TransportUnitTest.h"

#if defined(_WIN32)
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h> 
#include <pthread.h>
#include <signal.h>
#endif

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

// Select group of tests
#define RUN_TEST_ENCRYPTED 1

#define RUN_TEST_WEBSOCKET_NONBLOCK 1
#define RUN_TEST_WEBSOCKET_BLOCK 1

// Uncomment when debug, it will increase timeouts for detecting deadlock and a case when no data update receive
//#define DEBUGMODE_TRANSPORTTEST

// TIMEOUT_DEADLOCK_SEC  Select deadlock timeout (sec)
// MAX_TIMEWAIT_NOUPDATES_MS  Select time limit (ms) to wait any updates on client side: checkLongTimeNoRead()
#ifndef DEBUGMODE_TRANSPORTTEST
#define TIMEOUT_DEADLOCK_SEC 15
#define MAX_TIMEWAIT_NOUPDATES_MS 4000
#else
#define TIMEOUT_DEADLOCK_SEC 1000
#define MAX_TIMEWAIT_NOUPDATES_MS 1000000
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

 
/**************************************************************************
 *	Global test suite variables 
 *	Google Test only runs one test at a time, so these are safe to reuse 
 *	as cross-thread global variables.
 /************************************************************************/
RsslEventSignal deadlockPipe;
RsslMutex pipeLock;
bool testComplete;
bool failTest;
bool shutdownTest;

#define MAX_MSG_WRITE_COUNT 100000
#define MAX_THREADS 20
#define TEST_PROTOCOL_TYPE 10

#define SZ_Text 64000

/* Simple deadlock detection thread.  This thread will set a 15 second timer.  If a test is not 
 * completed(either success or failure) during that time, then the function calls abort.  
 * All tests will need to trigger the rssl_pipe deadlockPipe in order to stop this timer.
 * PRECONDITIONS: Pipe has already been created */
RSSL_THREAD_DECLARE(deadlockThread, pArg)
{
	struct timeval selectTime;
	int selRet;
	int nDetected = 0;

/* This is starting up prior to rsslInitialize getting called, so we will need to initialize Winsock here.
 * WSAStartup and WSACleanup are refcounted, so this should not cause any further issues.
 * The thread should live through all tests, all of which should WSACleanup as a part of their teardown, 
 * so this makes sure the pipe stays active through the entire run. */
#ifdef WIN32
	WSADATA	wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "WSAStartup failed." << std::endl;
		abort();
	}
	if ((LOBYTE(wsaData.wVersion) != 2) || (LOBYTE(wsaData.wHighVersion) != 2))
	{
		std::cout << "Wrong winsock version." << std::endl;
		WSACleanup();
		abort();
	}
#endif
	rsslInitEventSignal(&deadlockPipe);

	fd_set readfds;
	fd_set useread;

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(rsslGetEventSignalFD(&deadlockPipe), &readfds);

	while (!testComplete)
	{
		selectTime.tv_sec = TIMEOUT_DEADLOCK_SEC;
		selectTime.tv_usec = 0;

		useread = readfds;

		selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

		if (selRet <= 0)
		{
			if (++nDetected < 2)
			{	// Under Windows when the console is waited in QuickEdit mode and return to work
				// the deadlock monitor gets a signal. It's a negative case.
				// So, the monitor skips the first signal.
				std::cout << "." << std::endl;
				continue;
			}
			std::cout << "POSSIBLE DEADLOCK SITUATION.  ABORTING" << std::endl;
			abort();
		}
		else
		{
			if (FD_ISSET(deadlockPipe._fds[0], &useread))
			{
	            RSSL_MUTEX_LOCK(&pipeLock);
				rsslResetEventSignal(&deadlockPipe);
	            RSSL_MUTEX_UNLOCK(&pipeLock);
				nDetected = 0;
				continue;
			}
		}
	}

	rsslCleanupEventSignal(&deadlockPipe);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

void resetDeadlockTimer()
{
	RSSL_MUTEX_LOCK(&pipeLock);
	rsslSetEventSignal(&deadlockPipe);
	RSSL_MUTEX_UNLOCK(&pipeLock);
}

/*
 * Read packed messages
 * Calculate the expected number of messages that will be read.
 * When WebSocket + JSON case then packed JSON messages combine to items of JSON array.
 * rsslRead returns only message - the JSON array.
*/
static
int calcExpectedReadMessagesCount(bool isBlockedConn, int msgWriteCount,
	RsslConnectionTypes connType, RsslConnectionTypes encryptedProtocol, RsslUInt8 wsProtocolType,
	RsslUInt32 bufferLength, RsslUInt32 msgLength)
{
	// The expected number of messages that will be read
	int nReadMsgsExpect = msgWriteCount;
	if (connType == RSSL_CONN_TYPE_WEBSOCKET
		|| (connType == RSSL_CONN_TYPE_ENCRYPTED && encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET
			&& !isBlockedConn)
		)
	{	// When WebSocket + JSON: all the packed messages send as JSON array, and read as one message
		if (wsProtocolType == RSSL_JSON_PROTOCOL_TYPE)
		{
			int nMsgs = bufferLength / msgLength;
			// packed JSON messages in array divide by comma: [..],[..],[..]
			if ((msgLength + 1) * nMsgs - 1 > bufferLength)
				nMsgs--;

			nReadMsgsExpect = ((msgWriteCount / nMsgs) + (msgWriteCount % nMsgs > 0 ? 1 : 0));
			//std::cout << "msgWriteCount: " << msgWriteCount << " nMsgs: " << nMsgs << " nReadMsgsExpect: " << nReadMsgsExpect << std::endl;
		}
	}
	return nReadMsgsExpect;
}

/* Starts up and initializes a channel created with rsslAccept */
class ServerChannel
{
public:
	RsslThreadId* pThreadId;		/* Current Thread Id.  Useful for debugging */
	RsslChannel* pChnl;				/* Channel to be created.  This should be NULL when calling startServerChannel */
	RsslServer* pServer;			/* Server for channel creation */
	TUServerConfig* pServerConfig;	/* Configuration options for creating server */

	ServerChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		pServer = NULL;
		pServerConfig = NULL;
	}

	ServerChannel(TUServerConfig* pTUConfig) :
		pThreadId(NULL),
		pChnl(NULL),
		pServer(NULL),
		pServerConfig(pTUConfig)
	{
	}

	void setTUServerConfig(TUServerConfig* pTUConfig)
	{
		pServerConfig = pTUConfig;
	}

	/* If blocking is set to RSSL_TRUE, accept is set to block.  Otherwise, wait on notification for a new incomming channel, then accept and initialize. */
	void channelStart(RsslBool blocking)
	{
		RsslAcceptOptions acceptOpts;
		rsslClearAcceptOpts(&acceptOpts);
		struct timeval selectTime;
		RsslError err;
		int selRet;
		RsslChannel* serverChnl = pChnl;

		fd_set readfds;

		FD_ZERO(&readfds);

		/* Protect against pChnl being already set */
		ASSERT_EQ(serverChnl, (RsslChannel*)NULL) << "Server Channel has already been created, this is not supported.";


		if (blocking == RSSL_TRUE)
		{
			while (shutdownTest != true && serverChnl == NULL && !failTest)
			{
				serverChnl = rsslAccept(pServer, &acceptOpts, &err);

				if (serverChnl == NULL)
				{
					/* This happens primarily in the manyChannels test cases. */
					if (err.sysError == EWOULDBLOCK)
					{
						continue;
					}
					failTest = true;
					ASSERT_NE(serverChnl, (RsslChannel*)NULL) << "rsslAccept failed.  Error info: " << err.text;
				}
			}

			if (shutdownTest && serverChnl == NULL)
				return;

			if (serverChnl->state != RSSL_CH_STATE_ACTIVE)
			{
				failTest = true;
				ASSERT_EQ(serverChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel is not active!";
			}
		}
		else
		{
			selectTime.tv_sec = 0L;
			selectTime.tv_usec = 0L;
			while (shutdownTest != true && serverChnl == NULL && !failTest)
			{
				FD_SET(pServer->socketId, &readfds);
				selRet = select(FD_SETSIZE, &readfds, NULL, NULL, &selectTime);

				selectTime.tv_sec = 0L;
				selectTime.tv_usec = 10L;

				ASSERT_GE(selRet, 0) << "Select failed, system errno: " << errno;
				if (FD_ISSET(pServer->socketId, &readfds))
				{
					acceptOpts.nakMount = RSSL_FALSE;

					serverChnl = rsslAccept(pServer, &acceptOpts, &err);

					if (serverChnl == NULL)
					{
						/* This happens primarily in the manyChannels test cases. */
						if (err.sysError == EWOULDBLOCK)
						{
							continue;
						}
						failTest = true;
						ASSERT_NE(serverChnl, (RsslChannel*)NULL) << "rsslAccept failed.  Error info: " << err.text;
					}
					else
					{
						FD_SET(serverChnl->socketId, &readfds);
					}
				}
			}

			/* Hard looping on select and initChannel for this testing */
			while (shutdownTest != true && serverChnl->state != RSSL_CH_STATE_ACTIVE && !failTest)
			{
				RsslInProgInfo inProg;
				rsslClearInProgInfo(&inProg);
				RsslRet ret;

				ret = rsslInitChannel(serverChnl, &inProg, &err);

				/* This is not necessarily a failure condition, but we should print this out for record keeping*/
				if (ret < RSSL_RET_SUCCESS)
				{
					std::cout << "rsslInitChannel failed.  Error info: " << err.text << std::endl;
					failTest = true;
					ret = rsslCloseChannel(serverChnl, &err);
					ASSERT_TRUE(false) << "Init channel failed.";
				}

				if (inProg.flags & RSSL_IP_FD_CHANGE)
				{
					FD_CLR(inProg.oldSocket, &readfds);
					FD_SET(inProg.newSocket, &readfds);
				}
			}
		}

		pChnl = serverChnl;
	}

};

class ClientChannel
{
public:
	RsslThreadId* pThreadId;		/* Current Thread Id.  Useful for debugging */
	RsslChannel* pChnl;				/* Channel to be created.  This should be NULL when calling startServerChannel */
	TUClientConfig* pClientConfig;	/* Configuration options for creating client */

	ClientChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		pClientConfig = NULL;
	}

	ClientChannel(TUClientConfig* pTUConfig) :
		pThreadId(NULL),
		pChnl(NULL),
		pClientConfig(pTUConfig)
	{
	}

	void setTUClientConfig(TUClientConfig* pTUConfig)
	{
		pClientConfig = pTUConfig;
	}

	/* If blocking is set to RSSL_TRUE, attempt to connect using rsslConnect.  This will either return an active channel or error out.
	   Otherwise, loop on rsslInitChannel until the connection succeeds or fails. */
	void channelStart(RsslBool blocking)
	{
		RsslConnectOptions connectOpts;
		rsslClearConnectOpts(&connectOpts);
		RsslError err;
		RsslChannel* pClientChnl;

		ASSERT_EQ(pChnl, (RsslChannel*)NULL) << "Client channel already exists, this is not supported";

		if (pClientConfig == NULL)
		{
			connectOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
			connectOpts.connectionInfo.unified.address = (char*)"localhost";
			connectOpts.connectionInfo.unified.serviceName = (char*)"15000";
			connectOpts.protocolType = TEST_PROTOCOL_TYPE;
			connectOpts.tcp_nodelay = true;
			connectOpts.blocking = blocking;
		}
		else
		{
			connectOpts.connectionType = pClientConfig->connType;
			connectOpts.connectionInfo.unified.address = (char*)"localhost";
			connectOpts.connectionInfo.unified.serviceName = pClientConfig->portNo;
			connectOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
			connectOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
			connectOpts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
			connectOpts.tcp_nodelay = true;
			connectOpts.blocking = blocking;
			connectOpts.compressionType = pClientConfig->compressionType;

			if (connectOpts.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
			{
				connectOpts.encryptionOpts.encryptionProtocolFlags = pClientConfig->encryptionProtocolFlags;
				connectOpts.encryptionOpts.encryptedProtocol = pClientConfig->encryptedProtocol;
				connectOpts.encryptionOpts.openSSLCAStore = pClientConfig->openSSLCAStore;
			}

			if (connectOpts.connectionType == RSSL_CONN_TYPE_ENCRYPTED && pClientConfig->encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET
				|| connectOpts.connectionType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				connectOpts.wsOpts.protocols = pClientConfig->wsProtocolList;
			}
		}

		pClientChnl = rsslConnect(&connectOpts, &err);

		if (pClientChnl == NULL)
		{
			failTest = true;
			ASSERT_TRUE(false) << "rsslConnect failed with error text: " << err.text;
		}
		if (blocking == RSSL_FALSE)
		{
			/* Hard looping on rsslInitChannel */
			while (shutdownTest != true && pClientChnl->state != RSSL_CH_STATE_ACTIVE && !failTest)
			{

				RsslInProgInfo inProg;
				rsslClearInProgInfo(&inProg);
				RsslRet ret;

				ret = rsslInitChannel(pClientChnl, &inProg, &err);

				/* Failure condition */
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_CHAN_INIT_IN_PROGRESS)
				{
					failTest = true;
					ASSERT_TRUE(false) << "rsslInitChannel failed with return: " << ret << " And error text: " << err.text;
				}
			}
		}

		if (pClientChnl->state != RSSL_CH_STATE_ACTIVE)
		{
			failTest = true;
			ASSERT_EQ(pClientChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		pChnl = pClientChnl;
	}
};

class PingChannel
{
public:
	RsslThreadId* pThreadId;		/* Current Thread Id.  Useful for debugging */
	RsslChannel* pChnl;				/* Channel to be created.  This should be NULL when calling startServerChannel */

	PingChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
	}

	void pingLoop()
	{
		RsslError err;
		RsslRet ret;

		while (!shutdownTest && !failTest)
		{
			time_sleep(10);
			ret = rsslPing(pChnl, &err);

			if (ret < RSSL_RET_SUCCESS)
			{
				failTest = true;
				ASSERT_LT(ret, RSSL_RET_SUCCESS) << "Ping failed!";
			}
		}
	}
};

class ReadChannel
{
public:
	RsslThreadId* pThreadId;
	RsslChannel* pChnl;
	int* readCount;
	RsslMutex* lock;           // Protect access to the read counter: readCount
	RsslMutex* lockReadWrite;  // Protect access to whole read section, if need

	EventSignal* signalForWrite; // This is an event-signal to write-loop that read-loop is ready for reading next chunk
	TestHandler* pSystemTestHandler;  // SystemTests can handle some event, for example to test received data

	int expectedReadCount;     // If > 0, there is a limit on the number of read operations
	
	int selectCount;
	//const static size_t szArrSelectCount = 32;
	//int arrSelectCount[szArrSelectCount];

	ReadChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		readCount = NULL;
		lock = NULL;
		lockReadWrite = NULL;
		signalForWrite = NULL;
		pSystemTestHandler = NULL;
		expectedReadCount = 0;
		selectCount = 0;
		//memset(arrSelectCount, 0, sizeof(arrSelectCount));
	}

	/*	Read from pChnl.  Increments the read counter whenever rsslRead returns a buffer.
		If blocking is set to RSSL_TRUE, the function hard loops on rsslRead.  If not, the function will 
		sit on select and only call rsslRead if pChnl's FD is set.
		Note that this function does not validate the data, as the rsslRead function is not thread 
		aware and does not guarantee any message order for a given thread.  
	*/
	void readLoop(RsslBool blocking)
	{
		//fd_set readfds;
		fd_set useRead;
		fd_set useExcept;
		RsslRet readRet;
		RsslError err;
		int readmsg = 0;
		bool readIncrement = false;
		bool shouldForceSelect = false;
		char errorText[256] = "";

		//FD_ZERO(&readfds);
		//FD_ZERO(&useread);

		RsslBuffer* readBuf;

		struct timeval selectTime;
		int selRet;
		//bool selectReturnedNoData = false;

		if ( pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE )
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		if ( blocking == RSSL_TRUE )
		{
			/* It synchronizes with write-thread. We need to wait for available data on select at the first time. */
			if ( lockReadWrite != NULL )
				shouldForceSelect = true;
		}

		//FD_SET(pChnl->socketId, &readfds);

		while ( !shutdownTest && !failTest )
		{
			if ( expectedReadCount > 0 && expectedReadCount <= readmsg )
				break;

			//selectReturnedNoData = false;

			if ( blocking == RSSL_FALSE || shouldForceSelect )
			{
				FD_ZERO(&useRead);
				FD_ZERO(&useExcept);
				FD_SET(pChnl->socketId, &useRead);
				FD_SET(pChnl->socketId, &useExcept);
				//useRead = readfds;

				selectTime.tv_sec = 0L;
				selectTime.tv_usec = 40000; // 500000;
				selRet = select(FD_SETSIZE, &useRead, NULL, &useExcept, &selectTime);
				selectCount++;

				/* We're shutting down, so there is no failure case here */
				if ( shutdownTest || failTest )
				{
					break;
				}

				ASSERT_GE(selRet, 0) << "Reader select failed, system errno: " << errno;

				if ( !FD_ISSET(pChnl->socketId, &useRead) && !FD_ISSET(pChnl->socketId, &useExcept) )
				{
					//selectReturnedNoData = true;
					if ( (selectCount % 4) != 0 )
						continue;
				}
			}

			/* Special tests use an additional lock on entire read operation */
			/* It allows read and write operations to be performed step by step */
			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_LOCK(lockReadWrite);

				if ( shutdownTest || failTest )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
					break;
				}
			}

			do
			{
				readBuf = rsslRead(pChnl, &readRet, &err);
				if (readBuf)
				{
					++readmsg;
					readIncrement = true;
					RSSL_MUTEX_LOCK(lock);
					++(*(readCount));
					RSSL_MUTEX_UNLOCK(lock);

					//if ( selectReturnedNoData )
					//{
					//	std::cout << "rsslRead read a message, but select returned NoData selectCount: " << selectCount
					//		<< ", readmsg: " << readmsg << ", readBuf.len:" << readBuf->length << std::endl;
					//}
					//arrSelectCount[readmsg % szArrSelectCount] = selectCount;
					selectCount = 0;
				}
				else
				{
					if (readRet < RSSL_RET_SUCCESS && readRet != RSSL_RET_READ_PING && readRet != RSSL_RET_READ_IN_PROGRESS && readRet != RSSL_RET_READ_WOULD_BLOCK)
					{
						if ( lockReadWrite != NULL )
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
						}

						/* We're shutting down, so there is no failure case here */
						if ( shutdownTest )
							return;
						failTest = true;
						ASSERT_TRUE(false) << "rsslRead failed. Return code:" << readRet << " Error info: " << err.text << " readCount: " << *(readCount) << " readmsg: " << readmsg;
					}
				}

				/* Checks the error case */
				if ( pChnl->connectionType == RSSL_CONN_TYPE_WEBSOCKET
					&& (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet > 0) )
				{
					rsslChannelImpl* rsslChnlImpl = (rsslChannelImpl*)pChnl;
					RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;
					if (rsslSocketChannel->inputBufCursor > rsslSocketChannel->inputBuffer->length)
					{
						if ( lockReadWrite != NULL )
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
						}

						/* We're shutting down, so there is no failure case here */
						if ( shutdownTest )
							return;
						failTest = true;
						ASSERT_TRUE(false) << "rsslRead: error case. Return code:" << readRet << " Error info: " << err.text << " readCount: " << *(readCount) << " readmsg: " << readmsg;
					}
				}

				// Notify the handler: allow to do tests on the received data
				if ( pSystemTestHandler != NULL && readBuf != NULL )
				{
					if ( !pSystemTestHandler->handleDataBuffer(readBuf, errorText, sizeof(errorText)) )
					{
						if ( lockReadWrite != NULL )
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
						}

						failTest = true;
						ASSERT_TRUE(false) << "ReadChannel. handleDataBuffer failed. Error:" << errorText << " readCount: " << *(readCount) << " readmsg: " << readmsg;
					}
				}

				// Check the limit on the number of read operations
				if ( expectedReadCount > 0 && expectedReadCount <= readmsg )
					break;
			} while ( readRet > 0 && !shutdownTest && !failTest );

			/* Special tests use an additional lock on entire read operation */
			/* It allows read and write operations to be performed step by step */
			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);

				/* Sets the event-signal for a write loop: the read loop is ready for reading next chunk */
				signalForWrite->setSignal();
			}

			/* readIncrement stops us from resetting the deadlock timer more than once per readCount */
			if ( readmsg % 5000 == 1 && readIncrement == true )
			{
				resetDeadlockTimer();
				readIncrement = false;
			}
		}
	}
};

class WriteChannel
{
public:
	RsslThreadId* pThreadId;
	RsslChannel* pChnl;
	int msgCount;

	WriteChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		msgCount = MAX_MSG_WRITE_COUNT;
	}


	/*	Write to pChnl.
		This will send a small buffer msgCount times. This function also will flush.
		If blocking is set to RSSL_TRUE, the function will attempt to get a buffer, 
		then it will write the buffer to the wire and flush using rsslWrite and rsslFlush. 
		If blocking is set to RSSL_FALSE, the function will attempt to get a buffer,
		then it will write teh buffer to the wire.  If rsslWrite returns WRITE_CALL_AGAIN,
		the function will hard loop on rsslFlush and rsslWrite until the write succeeds.
		If rsslWrite returns a positive value, the function will select on the writing FD and 
		will call rsslFlush when triggered to do so.
	*/
	void writeLoop(RsslBool blocking)
	{
		fd_set readfds;
		fd_set exceptfds;
		fd_set writefds;
		RsslRet ret;
		RsslError err;
		bool needFlush = false;
		struct timeval selectTime;
		int selRet;
		int writeCount = 0;
		int writeMax = msgCount;

		RsslWriteInArgs inArgs;
		RsslWriteOutArgs outArgs;

		rsslClearWriteInArgs(&inArgs);

		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_ZERO(&writefds);

		RsslBuffer* writeBuf;

		const char* testBuffer = "TestDataInfo\0";
		RsslUInt32 bufferlen = (RsslUInt32)strlen(testBuffer);

		FD_SET(pChnl->socketId, &exceptfds);

		if (blocking == RSSL_TRUE)
		{
			/* Write messages until the maximum. */
			while (!shutdownTest && writeCount < writeMax && !failTest)
			{
				/* Hardlooping on Flush and getBuffer to make absolutely sure we get a buffer.  */
				writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);

				while (writeBuf == NULL && !shutdownTest && !failTest)
				{
					if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
					}
					writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);
				}

				if (shutdownTest || failTest)
					break;

				memcpy(writeBuf->data, testBuffer, (size_t)bufferlen);
				writeBuf->length = bufferlen;

				ret = rsslWriteEx(pChnl, writeBuf, &inArgs, &outArgs, &err);
				if (ret < RSSL_RET_SUCCESS)
				{
					failTest = true;
					ASSERT_TRUE(false) << "Write failed.  Error: " << err.text;
				}
				else
				{
					needFlush = true;
				}

				while (needFlush && !shutdownTest && !failTest)
				{
					if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
					}
					if (ret == RSSL_RET_SUCCESS)
						needFlush = false;
				}

				++writeCount;

				if (writeCount % 10000 == 1)
					resetDeadlockTimer();
			}
		}
		else
		{
			/* Write messages until the maximum. */
			while (!shutdownTest && writeCount < writeMax && !failTest)
			{
				/* Hardlooping on Flush and getBuffer to make absolutely sure we get a buffer.  */
				writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);

				while (writeBuf == NULL && !shutdownTest && !failTest)
				{
					if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
					}
					writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);
				}

				if (shutdownTest || failTest)
					break;

				memcpy(writeBuf->data, testBuffer, (size_t)bufferlen);
				writeBuf->length = bufferlen;

				ret = rsslWriteEx(pChnl, writeBuf, &inArgs, &outArgs, &err);
				if (ret == RSSL_RET_WRITE_CALL_AGAIN)
				{
					while (ret == RSSL_RET_WRITE_CALL_AGAIN && !shutdownTest && !failTest)
					{
						if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
						{
							failTest = true;
							ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
						}

						ret = rsslWriteEx(pChnl, writeBuf, &inArgs, &outArgs, &err);
						if (ret < RSSL_RET_SUCCESS)
							needFlush = true;
					}
				}
				else if (ret == RSSL_RET_WRITE_FLUSH_FAILED || ret >= RSSL_RET_SUCCESS)
				{
					needFlush = true;
				}
				else
				{
					failTest = true;
					ASSERT_TRUE(false) << "Write failed.  Error: " << err.text;
				}

				while (needFlush && !shutdownTest && !failTest)
				{
					FD_SET(pChnl->socketId, &writefds);
					selectTime.tv_sec = 0L;
					selectTime.tv_usec = 500000;
					selRet = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &selectTime);

					if (shutdownTest || failTest)
					{
						break;
					}

					if (selRet < 0)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Select failed.  Errno: " << errno;
					}
					if (FD_ISSET(pChnl->socketId, &writefds))
					{
						/* Hard looping on Flush to ensure that the call is made. */
						if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
						{
							failTest = true;
							ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
						}
						if (ret == RSSL_RET_SUCCESS)
							needFlush = false;
						FD_CLR(pChnl->socketId, &writefds);
					}

				}

				++writeCount;

				if (writeCount % 10000 == 1)
					resetDeadlockTimer();
			}
		}
	}
};

class WriteChannelTransport
{
public:
	RsslThreadId*	pThreadId;
	RsslChannel*	pChnl;
	std::atomic<int>	writeCount;				/* Counter of write operations. See writeLoop(). */

	RsslUInt32		requiredMsgLength;			/* Length of each separated message that packed into the buffer. */
	RsslUInt32		msgWriteCount;				/* Count of writing messages in this test, 0 - default, MAX_MSG_WRITE_COUNT */

	static const unsigned int MAX_LIMIT_CALL_FLUSH;

	static const char* testBuffer;				// data for test
	static const RsslUInt32 testBufferLength;	// length of the testBuffer

	const char* channelTitle;					/* Name of the channel */

	RsslMutex* lockReadWrite;				/* Protect access to whole read section, if need */

	EventSignal* eventSignalReadWrite;		/* Synchronization signal to wait for Read-loop is ready */
											/* Write loop waits for the signal */
											/* When Read-loop has completed processing a previous data chunk it sends this signal */


	WriteChannelTransport(RsslUInt32 msgLength, RsslUInt32 msgWriteCount, const char* title,
				RsslMutex* lockRW = NULL, EventSignal* eventSignalRW = NULL) :
		pThreadId(NULL),
		pChnl(NULL),
		writeCount(0),
		requiredMsgLength(msgLength),
		msgWriteCount(msgWriteCount),
		channelTitle(title),
		lockReadWrite(lockRW),
		eventSignalReadWrite(eventSignalRW)
	{
	}

	void getBuffer(RsslChannel* chnl, RsslUInt32 size, RsslBool packedBuffer, RsslError* pError, RsslBuffer** writeBuf)
	{
		RsslRet ret;
		unsigned kAttemptsCallFlush = 0;

		/* Hardlooping on Flush and getBuffer to make absolutely sure we get a buffer. */
		*writeBuf = NULL;
		while ( (*writeBuf = rsslGetBuffer(pChnl, size, packedBuffer, pError)) == NULL && !shutdownTest && !failTest )
		{
			if (pError->rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
			{
				failTest = true;
				ASSERT_TRUE(false) << "getBuffer. rsslGetBuffer failed. attempts: " << kAttemptsCallFlush << ", Error: (" << pError->rsslErrorId << ") " << pError->text;
			}

			if ((ret = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS)
			{
				failTest = true;
				ASSERT_TRUE(false) << "getBuffer. rsslFlush failed. attempts: " << kAttemptsCallFlush << ", Error: " << pError->text;
			}
			if (++kAttemptsCallFlush >= MAX_LIMIT_CALL_FLUSH)
			{
				failTest = true;
				ASSERT_TRUE(false) << "getBuffer. rsslGetBuffer failed many times. attempts: " << kAttemptsCallFlush << ", Error: " << pError->text;
			}
		}
		return;
	}

	/* Fill up the buffer by simulated data */
	RsslUInt32 fillBuffer(RsslBuffer* writeBuf, RsslUInt32 fillLength)
	{
		RsslUInt32 partlen = 0;
		RsslUInt32 k = 0;

		if (fillLength > writeBuf->length)
		{
			fillLength = writeBuf->length;
		}

		*(writeBuf->data) = '[';
		++k;
		if (fillLength > 1)
			fillLength -= 1;
		else
			fillLength = 0;

		int kn = snprintf(writeBuf->data + k, (fillLength - k), "%d:", writeCount.load());
		if (kn > 0)
			k += kn;

		while ( k < fillLength )
		{
			partlen = (testBufferLength < (fillLength - k) ? testBufferLength : (fillLength - k));

			memcpy(writeBuf->data + k, testBuffer, (size_t)partlen);
			k += partlen;
		}

		*(writeBuf->data + k) = ']';
		++k;

		return k;
	}

	void dumpBuffer(RsslBuffer* outBuf)
	{
		char ch;
		char buf[512];
		unsigned i, pos;
		int k = 0;
		const unsigned DUMP_LEN = 32;
		const unsigned POS_MID = (DUMP_LEN - (DUMP_LEN / 4) - 1);
		int isPrintable;

		k = snprintf(buf, sizeof(buf), "[%s] lenBuf: %u\n", channelTitle, outBuf->length);

		for (i = 0, pos = 0; i < DUMP_LEN && pos < outBuf->length; i++, pos++)
		{
			ch = outBuf->data[pos];
			k += snprintf((buf + k), sizeof(buf) - k, "%2.2X ", (ch & 0xFF));
			if (i == POS_MID)
			{
				k += snprintf((buf + k), sizeof(buf) - k, "| ");
				pos = outBuf->length - (DUMP_LEN / 4) - 1;
			}
		}
		k += snprintf((buf + k), sizeof(buf) - k, "\n");

		for (i = 0, pos = 0; i < DUMP_LEN && pos < outBuf->length; i++, pos++)
		{
			ch = outBuf->data[pos];
			isPrintable = isprint(ch);
			k += snprintf((buf + k), sizeof(buf) - k, "%c", (isPrintable != 0 ? ch : '.'));
			if (i == POS_MID)
			{
				k += snprintf((buf + k), sizeof(buf) - k, "[..]");
				pos = outBuf->length - (DUMP_LEN / 4) - 1;
			}
		}
		printf("%s\n", buf);

		return;
	}

	void writeBuffer_Blocking(RsslChannel* chnl, RsslBuffer* buffer, RsslWriteInArgs* writeInArgs, RsslWriteOutArgs* writeOutArgs, RsslError* pError)
	{
		RsslRet ret, retFlush;
		bool needFlush = false;
		unsigned kAttemptsCallFlush = 0;

		if ( shutdownTest || failTest )
			return;

		ret = rsslWriteEx(chnl, buffer, writeInArgs, writeOutArgs, pError);
		if ( ret < RSSL_RET_SUCCESS )
		{
			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);
			}
			failTest = true;
			ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslWriteEx failed. [" << channelTitle << "] ret: " << ret << ", Error: " << pError->text;
		}
		else
		{
			needFlush = true;
		}

		while ( needFlush && !shutdownTest && !failTest )
		{
			if ( (retFlush = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS )
			{
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
				}
				failTest = true;
				ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslFlush failed. [" << channelTitle << "] attempts: " << kAttemptsCallFlush
					<< ", ret:" << ret << ", retFlush:" << retFlush << ", Error: " << pError->text;
			}

			if ( retFlush == RSSL_RET_SUCCESS )
			{
				needFlush = false;
			}
			else if ( ++kAttemptsCallFlush >= MAX_LIMIT_CALL_FLUSH )
			{
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
				}
				failTest = true;
				ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslFlush failed many times. [" << channelTitle << "] attempts:" << kAttemptsCallFlush
					<< ", ret:" << ret << ", retFlush:" << retFlush << ", Error: " << pError->text;
			}
		}

		return;
	}

	void writeBuffer_NonBlocking(RsslChannel* chnl, RsslBuffer* buffer, RsslWriteInArgs* writeInArgs, RsslWriteOutArgs* writeOutArgs, RsslError* pError)
	{
		fd_set readfds;
		fd_set exceptfds;
		fd_set writefds;
		int selRet;
		struct timeval selectTime;

		RsslRet ret, retFlush;
		bool needFlush = false;
		unsigned kAttemptsCallFlush = 0;

		if ( shutdownTest || failTest )
			return;

		ret = rsslWriteEx(chnl, buffer, writeInArgs, writeOutArgs, pError);
		if ( ret == RSSL_RET_WRITE_CALL_AGAIN )
		{
			while ( ret == RSSL_RET_WRITE_CALL_AGAIN && !shutdownTest && !failTest )
			{
				if ( (ret = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS )
				{
					if ( lockReadWrite != NULL )
					{
						RSSL_MUTEX_UNLOCK(lockReadWrite);
					}
					failTest = true;
					ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Flush failed. [" << channelTitle << "] ret: " << ret << ", Error: " << pError->text;
				}

				if ( shutdownTest || failTest )
					return;

				ret = rsslWriteEx(chnl, buffer, writeInArgs, writeOutArgs, pError);
				if ( ret < RSSL_RET_SUCCESS )
				{
					needFlush = true;
					if ( ret != RSSL_RET_WRITE_CALL_AGAIN && !shutdownTest && !failTest )
					{
						std::cout << "writeBuffer_NonBlocking. rsslWriteEx failed. [" << channelTitle << "] ret: " << ret << ", Error: " << pError->text << std::endl;
					}
				}
			}
		}
		else if ( ret == RSSL_RET_WRITE_FLUSH_FAILED || ret >= RSSL_RET_SUCCESS )
		{
			needFlush = true;
		}
		else
		{
			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);
			}
			failTest = true;
			ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Write failed. [" << channelTitle << "] ret: " << ret << ", Error: " << pError->text;
		}

		retFlush = RSSL_RET_SUCCESS;
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_ZERO(&writefds);

		while ( needFlush && !shutdownTest && !failTest )
		{
			FD_SET(pChnl->socketId, &writefds);
			FD_SET(pChnl->socketId, &exceptfds);
			selectTime.tv_sec = 0L;
			selectTime.tv_usec = 100000;
			selRet = select(FD_SETSIZE, NULL, &writefds, &exceptfds, &selectTime);

			if ( shutdownTest || failTest )
			{
				break;
			}

			if ( selRet < 0 )
			{
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
				}
				failTest = true;
				ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Select failed. [" << channelTitle << "] attempts: " << kAttemptsCallFlush
					<< ", ret:" << ret << ", retFlush:" << retFlush << ", Errno: " << errno;
			}

			if ( FD_ISSET(pChnl->socketId, &writefds) )
			{
				/* Hard looping on Flush to ensure that the call is made. */
				if ( (retFlush = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS )
				{
					if ( lockReadWrite != NULL )
					{
						RSSL_MUTEX_UNLOCK(lockReadWrite);
					}
					failTest = true;
					ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Flush failed. [" << channelTitle << "] attempts: " << kAttemptsCallFlush
						<< ", ret:" << ret << ", retFlush:" << retFlush << ", Error: " << pError->text;
				}
				if ( retFlush == RSSL_RET_SUCCESS )
					needFlush = false;
				FD_CLR(pChnl->socketId, &writefds);
			}
			if ( FD_ISSET(pChnl->socketId, &exceptfds) )
			{
				int optVal;
				socklen_t lenVal = sizeof(optVal);

				int retGetSockopt = getsockopt(pChnl->socketId, SOL_SOCKET, SO_ERROR, (char*)&optVal, &lenVal);
				std::cout << "writeBuffer_NonBlocking. exceptfds. getsockopt: " << optVal << std::endl;
				FD_CLR(pChnl->socketId, &exceptfds);
			}

			if ( needFlush && ++kAttemptsCallFlush >= MAX_LIMIT_CALL_FLUSH )
			{
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
				}
				failTest = true;
				ASSERT_TRUE(false) << "writeBuffer_NonBlocking. rsslFlush failed many times. [" << channelTitle << "] attempts:" << kAttemptsCallFlush
					<< ", ret:" << ret << ", retFlush:" << retFlush << ", selRet:" << selRet << ", writeCount:" << writeCount.load();
				//std::cout << "writeBuffer_NonBlocking. rsslFlush failed many times. [" << channelTitle << "] attempts:" << kAttemptsCallFlush << ", ret:" << ret << ", retFlush:" << retFlush << ", selRet:" << selRet << ", writeCount:" << writeCount.load() << std::endl;
				//needFlush = false;
			}
		}
	}

	/*	Write to pChnl.
		This will send a small buffer msgCount times. This function also will flush.
		If blocking is set to RSSL_TRUE, the function will attempt to get a buffer,
		then it will write the buffer to the wire and flush using rsslWrite and rsslFlush.
		If blocking is set to RSSL_FALSE, the function will attempt to get a buffer,
		then it will write the buffer to the wire.  If rsslWrite returns WRITE_CALL_AGAIN,
		the function will hard loop on rsslFlush and rsslWrite until the write succeeds.
		If rsslWrite returns a positive value, the function will select on the writing FD and
		will call rsslFlush when triggered to do so.
	*/
	void writeLoop(RsslBool blocking)
	{
		RsslError err;

		int writeMax = (msgWriteCount != 0 ? msgWriteCount : MAX_MSG_WRITE_COUNT);

		RsslWriteInArgs inArgs;
		RsslWriteOutArgs outArgs;

		rsslClearWriteInArgs(&inArgs);

		RsslBuffer* writeBuf;

		const RsslUInt32 bufferlen = requiredMsgLength;
		RsslUInt32 filledLength;

		if ( pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE )
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		writeCount = 0;
		/* Write messages until the maximum. */
		while ( !shutdownTest && writeCount < writeMax && !failTest )
		{
			getBuffer(pChnl, requiredMsgLength, RSSL_FALSE, &err, &writeBuf);

			if ( shutdownTest || failTest )
				break;

			filledLength = fillBuffer(writeBuf, bufferlen);

			if ( filledLength < writeBuf->length )
				writeBuf->length = filledLength;

			if ( shutdownTest || failTest )
				break;

			// Special tests use an additional lock on entire read operation
			// It allows read and write operations to be performed step by step
			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_LOCK(lockReadWrite);

				if ( shutdownTest || failTest )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
					break;
				}
			}

			//dumpBuffer(writeBuf);

			if ( blocking == RSSL_TRUE )
			{
				writeBuffer_Blocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
			}
			else
			{
				writeBuffer_NonBlocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
			}

			++writeCount;

			if ( lockReadWrite != NULL )
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);

				if ( !shutdownTest && writeCount < writeMax && !failTest )
				{
					// Waits for an event-signal that a read loop is ready for reading next chunk
					eventSignalReadWrite->waitSignal(0, 50000);
				}
			}

			if ( writeCount % 10000 == 1 )
				resetDeadlockTimer();
		}
		//std::cout << "writeLoop Finished. " << channelTitle << " writeCount: " << writeCount << std::endl;
	}

};

const unsigned int WriteChannelTransport::MAX_LIMIT_CALL_FLUSH = 20U;

const char* WriteChannelTransport::testBuffer = "TestDataInfo\0";
const RsslUInt32 WriteChannelTransport::testBufferLength = (RsslUInt32)strlen(testBuffer);

class WriteChannelPacked : public WriteChannelTransport
{
public:
	RsslBuffer*		pWritingBuffer;				/* Current buffer in use by this channel. */
	RsslInt32		packedBufferCount;			/* Total number of buffers currently packed in pWritingBuffer */

	RsslUInt32		requiredBufferLength;		/* Length of the buffer for sending packed messages. */

	bool			lastMsgMustWrite;			/* When true the last message must be write to transport instead of packing to the buffer */
												/* otherwise the last message should be pack into the buffer */

	WriteChannelPacked(RsslUInt32 msgLength, RsslUInt32 bufferLength, RsslUInt32 msgWriteCount, const char* title,
				RsslMutex* lockRW = NULL, EventSignal* eventSignalRW = NULL) :
		WriteChannelTransport(msgLength, msgWriteCount, title, lockRW, eventSignalRW),
		pWritingBuffer(NULL),
		packedBufferCount(0),
		requiredBufferLength(bufferLength),
		lastMsgMustWrite(true)
	{
	}

	/*	Write to pChnl.
		This will send a small buffer msgCount times. This function also will flush.
		If blocking is set to RSSL_TRUE, the function will attempt to get a buffer,
		then it will write the buffer to the wire and flush using rsslWrite and rsslFlush.
		If blocking is set to RSSL_FALSE, the function will attempt to get a buffer,
		then it will write teh buffer to the wire.  If rsslWrite returns WRITE_CALL_AGAIN,
		the function will hard loop on rsslFlush and rsslWrite until the write succeeds.
		If rsslWrite returns a positive value, the function will select on the writing FD and
		will call rsslFlush when triggered to do so.
	*/
	void writeLoop(RsslBool blocking)
	{
		RsslError err;

		int writeBufCount = 0;
		int writeMax = (msgWriteCount != 0 ? msgWriteCount : MAX_MSG_WRITE_COUNT);

		RsslWriteInArgs inArgs;
		RsslWriteOutArgs outArgs;

		rsslClearWriteInArgs(&inArgs);

		RsslBuffer* writeBuf = NULL;
		RsslBuffer* origWriteBuf = NULL;

		RsslUInt32 packedBufferLen = requiredBufferLength;
		RsslUInt32 maxPackedMessagesInBuffer = 10;

		RsslUInt32 filledLength;

		bool shouldWriteBuffer = false;
		bool isLastMsgPackedIntoBuffer = false;

		if ( pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE )
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		if ( requiredMsgLength > requiredBufferLength )
		{
			failTest = true;
			FAIL() << "The message length (" << requiredMsgLength << ") should be less or equal then requiredBufferLength (" << requiredBufferLength << ")";
		}

		writeCount = 0;
		/* Write messages until the maximum. */
		while ( !shutdownTest && writeCount < writeMax && !failTest )
		{
			if ( writeBuf == NULL )
			{
				getBuffer(pChnl, packedBufferLen, RSSL_TRUE, &err, &writeBuf);
				origWriteBuf = writeBuf;
			}

			if ( shutdownTest || failTest )
				break;

			if ( requiredMsgLength <= writeBuf->length )
			{
				filledLength = fillBuffer(writeBuf, requiredMsgLength);

				if ( shutdownTest || failTest )
					break;

				RsslUInt32 lengthBufferRemain = (filledLength < writeBuf->length ? writeBuf->length - filledLength : 0);

				if ( filledLength < writeBuf->length )
					writeBuf->length = filledLength;

				if ( lastMsgMustWrite && (lengthBufferRemain <= requiredMsgLength || (writeCount + 1) == writeMax) )
				{
					shouldWriteBuffer = true;
					isLastMsgPackedIntoBuffer = false;
				}
				else
				{
					writeBuf = rsslPackBuffer(pChnl, writeBuf, &err);
					if ( writeBuf == NULL )
					{
						failTest = true;
						ASSERT_TRUE(false) << "writeLoop. rsslPackBuffer failed. writeCount: " << writeCount << ", Error: " << err.text;
					}
					isLastMsgPackedIntoBuffer = true;
				}
				++writeCount;
			}
			else
			{
				shouldWriteBuffer = true;
			}

			if ( shouldWriteBuffer || writeCount == writeMax )
			{
				if ( isLastMsgPackedIntoBuffer )
				{
					writeBuf->length = 0;
				}

				if ( shutdownTest || failTest )
					break;

				// Special tests use an additional lock on entire read operation
				// It allows read and write operations to be performed step by step
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_LOCK(lockReadWrite);

					if ( shutdownTest || failTest )
					{
						RSSL_MUTEX_UNLOCK(lockReadWrite);
						break;
					}
				}

				if ( blocking == RSSL_TRUE )
				{
					writeBuffer_Blocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
				}
				else
				{
					writeBuffer_NonBlocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
				}

				shouldWriteBuffer = false;
				writeBuf = NULL;
				++writeBufCount;

				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);

					if ( !shutdownTest && writeCount < writeMax && !failTest )
					{
						// Waits for an event-signal that a read loop is ready for reading next chunk
						eventSignalReadWrite->waitSignal(0, 50000);
					}
				}
			}

			if ( writeCount % 10000 == 1 )
				resetDeadlockTimer();
		}
		//std::cout << "writeLoop Finished. " << channelTitle << ":{" << writeBufCount << "," << writeCount << "}" << std::endl;
	}
};


class WriteChannelSeveralBufferTransport : public WriteChannelTransport {
public:

	int					configIndex;
	RsslChannelInfo*	readChannelInfo;

	WriteChannelSeveralBufferTransport(RsslUInt32 msgLength, RsslUInt32 msgWriteCount, int configIdx, const char* title,
		RsslMutex* lockRW, EventSignal* eventSignalRW) :
		WriteChannelTransport(msgLength, msgWriteCount, title, lockRW, eventSignalRW),
		configIndex(configIdx),
		readChannelInfo(NULL)
	{
	}


	void writeLoop(RsslBool blocking)
	{
		RsslError err;

		int writeMax = (msgWriteCount != 0 ? msgWriteCount : MAX_MSG_WRITE_COUNT);

		RsslWriteInArgs inArgs;
		RsslWriteOutArgs outArgs;

		rsslClearWriteInArgs(&inArgs);

		RsslBuffer* writeBuf;

		const RsslUInt32 bufferlen = requiredMsgLength;
		RsslUInt32 filledLength;

		if (pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE)
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		rsslChannelImpl* rsslChnlImpl = (rsslChannelImpl*)pChnl;
		RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;

		RsslUInt32 *msgLengths = NULL;
		int len = bufferlen;
		unsigned nLen = 0;
		bool isMutexLocked = false;

		if (!isMutexLocked  && lockReadWrite != NULL)
		{
			RSSL_MUTEX_LOCK(lockReadWrite);
			isMutexLocked = true;
		}

		// maxLength = 30250, readSize = 27225
		if (configIndex == 0)
		{
			msgLengths = new RsslUInt32[12];
			nLen = 12;
			msgLengths[0] = 100; msgLengths[1] = 100;
			msgLengths[2] = 3000; msgLengths[3] = 3000; msgLengths[4] = 3000; msgLengths[5] = 3000;
			msgLengths[6] = 3000; msgLengths[7] = 3000; msgLengths[8] = 3000; msgLengths[9] = 3000;
			msgLengths[10] = 3000; msgLengths[11] = 3000;
		}
		else if (configIndex == 1)
		{
			msgLengths = new RsslUInt32[14];
			nLen = 14;
			msgLengths[0] = 100; msgLengths[1] = 100;
			msgLengths[2] = 3000; msgLengths[3] = 3000; msgLengths[4] = 3000; msgLengths[5] = 3000;
			msgLengths[6] = 3000; msgLengths[7] = 3000; msgLengths[8] = 3000; msgLengths[9] = 3000;
			msgLengths[10] = 2918; msgLengths[11] = 2500; msgLengths[12] = 3000; msgLengths[13] = 3000;
		}
		else if (configIndex == 2)  // RWF
		{
			msgLengths = new RsslUInt32[18];
			nLen = 18;
			msgLengths[0] = 1179; msgLengths[1] = 1250;
			msgLengths[2] = 1250; msgLengths[3] = 1250; msgLengths[4] = 1250; msgLengths[5] = 1250;
			msgLengths[6] = 1250; msgLengths[7] = 1250; msgLengths[8] = 1250; msgLengths[9] = 1250;
			msgLengths[10] = 1250; msgLengths[11] = 1250; msgLengths[12] = 1250; msgLengths[13] = 1250;
			msgLengths[14] = 1250; msgLengths[15] = 1257; msgLengths[16] = 1246; msgLengths[17] = 1247;
		}
		else if (configIndex == 3)  // RWF
		{
			msgLengths = new RsslUInt32[14];
			nLen = 14;
			msgLengths[0] = 1250; msgLengths[1] = 1250;
			msgLengths[2] = 1250; msgLengths[3] = 1250; msgLengths[4] = 1250; msgLengths[5] = 1250;
			msgLengths[6] = 1250; msgLengths[7] = 1250; msgLengths[8] = 1250; msgLengths[9] = 1250;
			msgLengths[10] = 1250; msgLengths[11] = 1250; msgLengths[12] = 1250; msgLengths[13] = 1250;
		}

		if (nLen == 0 || msgLengths == NULL)
		{
			failTest = true;
			if (isMutexLocked && lockReadWrite != NULL)
			{
				isMutexLocked = false;
				RSSL_MUTEX_UNLOCK(lockReadWrite);
			}

			ASSERT_NE(msgLengths, (RsslUInt32*)NULL) << "Control data for the test do not initialized. msgLengths == NULL";
			ASSERT_NE(nLen, 0) << "Control data for the test do not initialized. nLen == 0";
			return;
		}

		writeCount = 0;
		/* Write messages until the maximum. */
		while (!shutdownTest && writeCount < writeMax && !failTest)
		{
			// Special tests use an additional lock on entire read operation
			// It allows read and write operations to be performed step by step
			if (!isMutexLocked && writeCount % nLen == 0 && lockReadWrite != NULL)
			{
				RSSL_MUTEX_LOCK(lockReadWrite);
				isMutexLocked = true;

				if (shutdownTest || failTest)
					break;
			}

			len = msgLengths[writeCount % nLen];
			getBuffer(pChnl, len, RSSL_FALSE, &err, &writeBuf);

			if (shutdownTest || failTest)
				break;

			filledLength = fillBuffer(writeBuf, len);

			if (filledLength < writeBuf->length)
				writeBuf->length = filledLength;

			if (shutdownTest || failTest)
				break;

			//dumpBuffer(writeBuf);

			if (blocking == RSSL_TRUE)
			{
				writeBuffer_Blocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
			}
			else
			{
				writeBuffer_NonBlocking(pChnl, writeBuf, &inArgs, &outArgs, &err);
			}

			++writeCount;

			if (writeCount % nLen == 0 && lockReadWrite != NULL)
			{
				isMutexLocked = false;
				RSSL_MUTEX_UNLOCK(lockReadWrite);

				if (!shutdownTest && writeCount < writeMax && !failTest)
				{
					// Waits for an event-signal that a read loop is ready for reading next chunk
					eventSignalReadWrite->waitSignal(0, 50000);
				}
			}

			if (writeCount % 10000 == 1)
				resetDeadlockTimer();
		}

		if (isMutexLocked && lockReadWrite != NULL)
		{
			isMutexLocked = false;
			RSSL_MUTEX_UNLOCK(lockReadWrite);
		}

		//std::cout << "writeLoop Finished. " << channelTitle << " writeCount: " << writeCount << std::endl;
		if (msgLengths != NULL)
		{
			delete[] msgLengths;
		}
		return;
	}

};

/* Write class for tests of rsslEncryptBuffer */
class WriteChannelEncryptBufferTransport : public WriteChannelTransport {
public:

	WriteChannelEncryptBufferTransport(RsslUInt32 msgLength, RsslUInt32 msgWriteCount, const char* title,
		RsslMutex* lockRW, EventSignal* eventSignalRW) :
		WriteChannelTransport(msgLength, msgWriteCount, title, lockRW, eventSignalRW)
	{
	}

	void writeLoop(RsslBool blocking)
	{
		RsslError err;

		int writeMax = (msgWriteCount != 0 ? msgWriteCount : MAX_MSG_WRITE_COUNT);

		RsslWriteInArgs inArgs;
		RsslWriteOutArgs outArgs;

		rsslClearWriteInArgs(&inArgs);

		RsslBuffer* dataBuf;
		RsslBuffer* writeEncryptedBuf;

		const RsslUInt32 bufferlen = requiredMsgLength;
		RsslUInt32 filledLength;
		RsslUInt32 encryptedSize;
		RsslRet ret;

		if (pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE)
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		writeCount = 0;
		/* Write messages until the maximum. */
		while (!shutdownTest && writeCount < writeMax && !failTest)
		{
			getBuffer(pChnl, requiredMsgLength, RSSL_FALSE, &err, &dataBuf);

			if (shutdownTest || failTest)
				break;

			filledLength = fillBuffer(dataBuf, bufferlen);

			if (filledLength < dataBuf->length)
				dataBuf->length = filledLength;

			if (shutdownTest || failTest)
				break;

			// Calculate the size of required buffer for encryption
			encryptedSize = rsslCalculateEncryptedSize(dataBuf);

			// Get the buffer
			getBuffer(pChnl, encryptedSize, RSSL_FALSE, &err, &writeEncryptedBuf);

			if (shutdownTest || failTest)
				break;

			// Encrypt the buffer
			ret = rsslEncryptBuffer(pChnl, dataBuf, writeEncryptedBuf, &err);
			if (ret != RSSL_RET_SUCCESS)
			{
				failTest = true;
				ASSERT_TRUE(false) << "rsslEncryptBuffer failed. ret: " << ret << ", Error: (" << err.rsslErrorId << ") " << err.text;
			}

			ret = rsslReleaseBuffer(dataBuf, &err);
			if (ret != RSSL_RET_SUCCESS)
			{
				failTest = true;
				ASSERT_TRUE(false) << "rsslReleaseBuffer failed. ret: " << ret << ", Error: (" << err.rsslErrorId << ") " << err.text;
			}

			if (shutdownTest || failTest)
				break;

			// Special tests use an additional lock on entire read operation
			// It allows read and write operations to be performed step by step
			if (lockReadWrite != NULL)
			{
				RSSL_MUTEX_LOCK(lockReadWrite);

				if (shutdownTest || failTest)
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
					break;
				}
			}

			//dumpBuffer(writeEncryptedBuf);

			if (blocking == RSSL_TRUE)
			{
				writeBuffer_Blocking(pChnl, writeEncryptedBuf, &inArgs, &outArgs, &err);
			}
			else
			{
				writeBuffer_NonBlocking(pChnl, writeEncryptedBuf, &inArgs, &outArgs, &err);
			}

			++writeCount;

			if (lockReadWrite != NULL)
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);

				if (!shutdownTest && writeCount < writeMax && !failTest)
				{
					// Waits for an event-signal that a read loop is ready for reading next chunk
					eventSignalReadWrite->waitSignal(0, 50000);
				}
			}

			if (writeCount % 10000 == 1)
				resetDeadlockTimer();
		}
		//std::cout << "writeLoop Finished. " << channelTitle << " writeCount: " << writeCount << std::endl;
	}
};


class AbstractTransportBuffer {
public:
	virtual const unsigned char* getTestBuffer(
		RsslUInt32 requiredMsgLength, int configIndex,
		RsslUInt32* bufferLength, RsslUInt32* numMessagesInBuffer,
		char* errorText, size_t szErrorText) = 0;

	virtual size_t getChunkLengths(int configIndex, int writeCount, unsigned* arrChunkLengths, size_t szArrChunkLengths)
	{
		return 0;
	}
};

#define TB_ERR_FILE_NOT_OPEN "File is not open:"
#define TB_ERR_MEM_ALLOC_FAIL "Memory allocation failed."
#define TB_ERR_FILE_READ_FAIL "Read buffer failed."

class MsgFileBuffer : public AbstractTransportBuffer
{
	const char* fileName;
	unsigned char* testBuffer;

public:
	MsgFileBuffer(const char* fName) : fileName(fName), testBuffer(NULL)
	{}

	~MsgFileBuffer()
	{
		if (testBuffer)
			delete[] testBuffer;
		testBuffer = NULL;
	}

	const unsigned char* getTestBuffer(
		RsslUInt32 requiredMsgLength, int configIndex,
		RsslUInt32* bufferLength, RsslUInt32* numMessagesInBuffer,
		char* errorText, size_t szErrorText)
	{
		int statResult;
#ifdef WIN32
		struct _stat statBuffer;
		statResult = _stat(fileName, &statBuffer);
#else
		struct stat statBuffer;
		statResult = stat(fileName, &statBuffer);
#endif
		if (statResult < 0)
		{
			*bufferLength = 0U;
			*numMessagesInBuffer = 0U;
			if (errorText != NULL && szErrorText > 0)
				snprintf(errorText, szErrorText, "%s %s", TB_ERR_FILE_NOT_OPEN, fileName);
			return NULL;
		}

		size_t fileSize = statBuffer.st_size;

		testBuffer = new unsigned char[fileSize + 32];
		if (testBuffer == NULL)
		{
			*bufferLength = 0U;
			*numMessagesInBuffer = 0U;
			if (errorText != NULL && szErrorText > 0)
				snprintf(errorText, szErrorText, "%s fileSize: %zu", TB_ERR_MEM_ALLOC_FAIL, fileSize);
			return NULL;
		}

		FILE* finp = fopen(fileName, "rb");
		if (finp == NULL)
		{
			*bufferLength = 0U;
			*numMessagesInBuffer = 0U;
			if (errorText != NULL && szErrorText > 0)
				snprintf(errorText, szErrorText, "%s %s", TB_ERR_FILE_NOT_OPEN, fileName);
			return NULL;
		}
		else
		{
			size_t sizeToRead = (0 < requiredMsgLength && requiredMsgLength <= fileSize ? requiredMsgLength : fileSize);
			size_t sz = fread(testBuffer, sizeof(unsigned char), sizeToRead, finp);
			if (sz > 0)
			{
				*bufferLength = ((RsslUInt32)sz);
				*numMessagesInBuffer = 4;
			}
			else
			{
				*bufferLength = 0U;
				*numMessagesInBuffer = 0U;
				if (errorText != NULL && szErrorText > 0)
					snprintf(errorText, szErrorText, "%s %s", TB_ERR_FILE_READ_FAIL, fileName);
			}
			fclose(finp);
		}

		return testBuffer;
	}

	size_t getChunkLengths(int configIndex, int writeCount, unsigned* arrChunkLengths, size_t szArrChunkLengths)
	{
		int ind = writeCount % 50000;
		int k;

		switch (configIndex) {
		case 0:
			{
				// WebSocket + JSON
				// "inpbuf_01.bin". 4 messages inside. size: 14673
				// size of mess: 5668 2430 5804 771
				// 0..5667, 5668..8097, 8098..13901, 13902..14672
				// 
				//memset(arrChunkLengths, 0, szArrChunkLengths);
				if (writeCount >= 50000)
				{
					if (ind <= 40)
					{
						k = ind / 4;
						arrChunkLengths[0] = (k + 1);
						arrChunkLengths[1] = 14673 - (k + 1);
						return 2;
					}
					else if (100 <= ind && ind <= 140)
					{
						k = (ind - 100) / 4;
						arrChunkLengths[0] = (k + 1);
						arrChunkLengths[1] = 5668 - (k + 1);
						arrChunkLengths[2] = (k + 1);
						arrChunkLengths[3] = 2430 - (k + 1);
						arrChunkLengths[4] = (k + 1);
						arrChunkLengths[5] = 5804 - (k + 1);
						arrChunkLengths[6] = (k + 1);
						arrChunkLengths[7] = 771 - (k + 1);
						return 8;
					}
					if (1000 <= ind && ind <= 1040)
					{
						k = (ind - 1000) / 4;
						arrChunkLengths[0] = 14673 - (k + 1);
						arrChunkLengths[1] = (k + 1);
						return 2;
					}
					else if (2000 <= ind && ind <= 2040)
					{
						k = (ind - 2000) / 4;
						arrChunkLengths[0] = 5668 - (k + 1);
						arrChunkLengths[1] = (k + 1);
						arrChunkLengths[2] = 2430 - (k + 1);
						arrChunkLengths[3] = (k + 1);
						arrChunkLengths[4] = 5804 - (k + 1);
						arrChunkLengths[5] = (k + 1);
						arrChunkLengths[6] = 771 - (k + 1);
						arrChunkLengths[7] = (k + 1);
						return 8;
					}
					else if (3000 <= ind && ind <= 3040)
					{
						k = (ind - 3000) / 4;
						arrChunkLengths[0] = (k + 1);
						arrChunkLengths[1] = 5668;    // {0..k}, {(k+1)..5667, 2-nd: 0..k}
						arrChunkLengths[2] = (k + 1);
						arrChunkLengths[3] = 2430;
						arrChunkLengths[4] = (k + 1);
						arrChunkLengths[5] = 5804;
						arrChunkLengths[6] = (k + 1);
						arrChunkLengths[7] = 771 - 4 * (k + 1);
						return 8;
					}
					else if (4000 <= ind && ind <= 4040)
					{
						k = (ind - 4000) / 4;
						arrChunkLengths[0] = 5668 - (k + 1);
						arrChunkLengths[1] = 2430 - (k + 1);
						arrChunkLengths[2] = 5804 - (k + 1);
						arrChunkLengths[3] = 771 - (k + 1);
						arrChunkLengths[4] = 4 * (k + 1);
						return 5;
					}
				}
			}
			break;
		case 1:
			{
				k = ind / 4;
				arrChunkLengths[0] = (k + 1);
				arrChunkLengths[1] = 14673 - (k + 1);
				return 2;
			}
			break;
		case 2:
			{
				k = ind / 4;
				arrChunkLengths[0] = (k + 1);
				arrChunkLengths[1] = 5668 - (k + 1);
				arrChunkLengths[2] = (k + 1);
				arrChunkLengths[3] = 2430 - (k + 1);
				arrChunkLengths[4] = (k + 1);
				arrChunkLengths[5] = 5804 - (k + 1);
				arrChunkLengths[6] = (k + 1);
				arrChunkLengths[7] = 771 - (k + 1);
				return 8;
			}
			break;
		case 3:
			{
				k = ind / 4;
				arrChunkLengths[0] = 14673 - (k + 1);
				arrChunkLengths[1] = (k + 1);
				return 2;
			}
			break;
		case 4:
			{
				k = ind / 4;
				arrChunkLengths[0] = 5668 - (k + 1);
				arrChunkLengths[1] = (k + 1);
				arrChunkLengths[2] = 2430 - (k + 1);
				arrChunkLengths[3] = (k + 1);
				arrChunkLengths[4] = 5804 - (k + 1);
				arrChunkLengths[5] = (k + 1);
				arrChunkLengths[6] = 771 - (k + 1);
				arrChunkLengths[7] = (k + 1);
				return 8;
			}
			break;
		case 5:
			{
				k = ind / 4;
				arrChunkLengths[0] = (k + 1);
				arrChunkLengths[1] = 5668;    // {0..k}, {(k+1)..5667, 2-nd: 0..k}
				arrChunkLengths[2] = (k + 1);
				arrChunkLengths[3] = 2430;
				arrChunkLengths[4] = (k + 1);
				arrChunkLengths[5] = 5804;
				arrChunkLengths[6] = (k + 1);
				arrChunkLengths[7] = 771 - 4 * (k + 1);
				return 8;
			}
			break;
		case 6:
			{
				k = ind / 4;
				arrChunkLengths[0] = 5668 - (k + 1);
				arrChunkLengths[1] = 2430 - (k + 1);
				arrChunkLengths[2] = 5804 - (k + 1);
				arrChunkLengths[3] = 771 - (k + 1);
				arrChunkLengths[4] = 4 * (k + 1);
				return 5;
			}
			break;

			// WebSocket + RWF
			// inpbuf_ws_rwf_01.bin
			// 5 messges inside. size: 526.
			// Size of mess: 107, 112, 111, 110, 86
			// 0 .. 106, 107 .. 218, 219 .. 329, 330 .. 429, 430 .. 525 

		case 7:
		{
			k = ind / 5;
			arrChunkLengths[0] = (k + 1);
			arrChunkLengths[1] = 526 - (k + 1);
			return 2;
		}
		break;
		case 8:
		{
			k = ind / 5;
			arrChunkLengths[0] = (k + 1);
			arrChunkLengths[1] = 107 - (k + 1);
			arrChunkLengths[2] = (k + 1);
			arrChunkLengths[3] = 112 - (k + 1);
			arrChunkLengths[4] = (k + 1);
			arrChunkLengths[5] = 111 - (k + 1);
			arrChunkLengths[6] = (k + 1);
			arrChunkLengths[7] = 110 - (k + 1);
			arrChunkLengths[8] = (k + 1);
			arrChunkLengths[9] = 86 - (k + 1);
			return 10;
		}
		break;
		case 9:
		{
			k = ind / 5;
			arrChunkLengths[0] = 526 - (k + 1);
			arrChunkLengths[1] = (k + 1);
			return 2;
		}
		break;
		case 10:
		{
			k = ind / 5;
			arrChunkLengths[0] = 107 - (k + 1);
			arrChunkLengths[1] = (k + 1);
			arrChunkLengths[2] = 112 - (k + 1);
			arrChunkLengths[3] = (k + 1);
			arrChunkLengths[4] = 111 - (k + 1);
			arrChunkLengths[5] = (k + 1);
			arrChunkLengths[6] = 110 - (k + 1);
			arrChunkLengths[7] = (k + 1);
			arrChunkLengths[8] = 86 - (k + 1);
			arrChunkLengths[9] = (k + 1);
			return 10;
		}
		break;
		case 11:
		{
			k = ind / 5;
			arrChunkLengths[0] = (k + 1);
			arrChunkLengths[1] = 107;    // {0..k}, {(k+1)..107, 2-nd: 0..k}
			arrChunkLengths[2] = (k + 1);
			arrChunkLengths[3] = 112;
			arrChunkLengths[4] = (k + 1);
			arrChunkLengths[5] = 111;
			arrChunkLengths[6] = (k + 1);
			arrChunkLengths[7] = 110;
			arrChunkLengths[8] = (k + 1);
			arrChunkLengths[9] = 86 - 5 * (k + 1);
			return 10;
		}
		break;
		case 12:
		{
			k = ind / 5;
			arrChunkLengths[0] = 107 - (k + 1);
			arrChunkLengths[1] = 112 - (k + 1);
			arrChunkLengths[2] = 111 - (k + 1);
			arrChunkLengths[3] = 110 - (k + 1);
			arrChunkLengths[4] = 86  - (k + 1);
			arrChunkLengths[5] = 5 * (k + 1);
			return 6;
		}
		break;
		}

		return 0;
	}
};

class AbstractTransportBufferFactory
{
public:
	static AbstractTransportBuffer* getAbstractTransportBuffer(int configIndex)
	{
		switch (configIndex)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			// "inpbuf_01.bin". 4 messages inside. WebSocket + JSON
			return new MsgFileBuffer("inpbuf_01.bin");
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			// "inpbuf_ws_rwf_01.bin". 5 messages inside. WebSocket + RWF
			return new MsgFileBuffer("inpbuf_ws_rwf_01.bin");
		default:
			return NULL;
		}
	}
};

class WriteChannelSystem
{
public:
	RsslThreadId*	pThreadId;
	RsslChannel*	pChnl;
	std::atomic<int>	writeCount;				/* Counter of write operations. See writeLoop(). */

	RsslUInt32		requiredMsgLength;			/* Length of each separated message that packed into the buffer. */
	RsslUInt32		msgWriteCount;				/* Count of writing messages in this test, 0 - default, MAX_MSG_WRITE_COUNT */

	int				configIndex;				/* Configuration index - for getting test buffer */

	RsslMutex*		lockReadWrite;				/* Protect access to whole read section, if need */

	EventSignal*	eventSignalReadWrite;		/* Synchronization signal to wait for Read-loop is ready */

	WriteChannelSystem(RsslUInt32 msgLength, RsslUInt32 msgWriteCount, int cfgIndex, RsslMutex* lockRW, EventSignal* eventSignalRW) :
		pThreadId(NULL),
		pChnl(NULL),
		writeCount(0),
		requiredMsgLength(msgLength),
		msgWriteCount(msgWriteCount),
		configIndex(cfgIndex),
		lockReadWrite(lockRW),
		eventSignalReadWrite(eventSignalRW)
	{
	}

	/* Waits on select during a timeout.
	*  Checks that the socket is ready for write.
	*/
	void waitReadyForWrite(int totalSend, int bufferlen)
	{
		fd_set writefds;
		struct timeval selectTime;

		FD_ZERO(&writefds);
		FD_SET(pChnl->socketId, &writefds);

		selectTime.tv_sec = 0L;
		selectTime.tv_usec = 200000;

		int selRet = select(FD_SETSIZE, NULL, &writefds, NULL, &selectTime);

		if ( !shutdownTest && !failTest )
		{
			if (selRet > 0 && FD_ISSET(pChnl->socketId, &writefds))
			{
				FD_CLR(pChnl->socketId, &writefds);
			}
			else if (selRet < 0)
			{
				failTest = true;
				ASSERT_TRUE(false) << "Select failed. errno=" << errno << "; writeCount=" << writeCount << " totalSend=" << totalSend << " (" << bufferlen << ")";
			}
			else
			{
				failTest = true;
				ASSERT_TRUE(false) << "Select timeout (the socket is not ready to write) or unexpected socket. writeCount=" << writeCount << " totalSend=" << totalSend << " (" << bufferlen << ")";
			}
		}
		return;
	}

	/*	Write to the system socket.
	*/
	void writeLoop(RsslBool blocking)
	{
		int writeCountRetZeroBytes = 0;
		const int MAX_LIMIT_SEND_RET_ZERO = 1;

		int numBytes;
		int totalSend = 0;

		int writeMax = (msgWriteCount != 0 ? msgWriteCount : MAX_MSG_WRITE_COUNT);


		if ( pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE )
		{
			failTest = true;
			ASSERT_NE(pChnl, (RsslChannel*)NULL) << "Channel should not equal to NULL";
			ASSERT_EQ(pChnl->state, RSSL_CH_STATE_ACTIVE) << "Channel state is not active";
		}

		// Getter - returns a test buffer
		AbstractTransportBuffer* pTransportBuffer = AbstractTransportBufferFactory::getAbstractTransportBuffer(configIndex);
		if (pTransportBuffer == NULL)
		{
			failTest = true;
			ASSERT_TRUE(false) << "Write failed. Absent getter of test buffer. configIndex=" << configIndex;
		}

		// Gets the test buffer
		RsslUInt32 numMessagesInBuffer = 0;
		RsslUInt32 testbuflen = 0;
		char errorText[256] = "";
		char* testBuffer = (char*)pTransportBuffer->getTestBuffer(
					requiredMsgLength, configIndex, &testbuflen, &numMessagesInBuffer, errorText, sizeof(errorText));
		if (testBuffer == NULL || testbuflen == 0 || numMessagesInBuffer == 0)
		{
			failTest = true;
			ASSERT_TRUE(false) << "Write failed. Test buffer is NULL or test-config error. [" << errorText << "] configIndex=" << configIndex << "; testbuflen=" << testbuflen << "; numMessagesInBuffer=" << numMessagesInBuffer;
		}

		int bufferlen = testbuflen;

		int chunkLength = 0;
		int chunkSend = 0;
		unsigned chunkIndex = 0;

		// Gets the length for each chunk, i.e. when it sends the test buffer chunk by chunk.
		unsigned arrChunkLengths[10];
		size_t nChunkLengths = 0;

		/* Write messages until the maximum. */
		while ( !shutdownTest && writeCount < writeMax && !failTest )
		{
			// Sends message buffer
			totalSend = 0;

			nChunkLengths = pTransportBuffer->getChunkLengths(configIndex, writeCount, arrChunkLengths, 10);
			if (nChunkLengths > 0 && arrChunkLengths[0] == 0)
				nChunkLengths = 0;

			chunkLength = bufferlen;
			chunkIndex = 0;

			if ( blocking == RSSL_FALSE )
			{
				waitReadyForWrite(totalSend, bufferlen);
			}

			// Sends the message buffer by chunks
			while ( totalSend < bufferlen && !shutdownTest && !failTest )
			{
				// Calculate the length of this chunk
				if ( nChunkLengths > 0 )
				{
					chunkLength = arrChunkLengths[(chunkIndex % nChunkLengths)];
				}
				if ( chunkLength == 0 )
				{
					chunkLength = bufferlen;
				}

				chunkSend = 0;
				// Sends a chunk (next part of the message buffer)
				while ( chunkSend < chunkLength && (totalSend + chunkSend) < bufferlen && !shutdownTest && !failTest )
				{
					// Special tests use an additional lock on entire read operation
					// It allows read and write operations to be performed step by step
					if (lockReadWrite != NULL)
					{
						RSSL_MUTEX_LOCK(lockReadWrite);

						if (shutdownTest || failTest)
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
							break;
						}
					}

					numBytes = send(pChnl->socketId, (testBuffer + (totalSend + chunkSend)), (chunkLength - chunkSend), 0);

					if (lockReadWrite != NULL)
					{
						RSSL_MUTEX_UNLOCK(lockReadWrite);

						if (numBytes > 0)
						{
							// Waits for an event-signal that a read loop is ready for reading next chunk
							eventSignalReadWrite->waitSignal(0, 50000);
						}
					}

					if (numBytes > 0)
					{
						chunkSend += numBytes;
						writeCountRetZeroBytes = 0;
					}
					else if (numBytes <= 0)
					{
						if (numBytes < 0) // send return an error
						{
							if (!((errno == _IPC_WOULD_BLOCK) || (errno == EINTR)))
							{
								failTest = true;
								ASSERT_TRUE(false) << "Write failed. errno=" << errno << "; writeCount=" << writeCount << " totalSend=" << (totalSend + chunkSend) << " (" << bufferlen << ")";
							}
						}

						if (writeCountRetZeroBytes++ > MAX_LIMIT_SEND_RET_ZERO)
						{
							failTest = true;
							ASSERT_TRUE(false) << "Write failed. send returned zero bytes more than " << MAX_LIMIT_SEND_RET_ZERO << " times" << "; writeCount=" << writeCount << " totalSend=" << (totalSend + chunkSend) << " (" << bufferlen << ")";
						}

						if (blocking == RSSL_FALSE)
						{
							waitReadyForWrite(chunkSend, chunkLength);
						}
					}
				}  // while ( chunkSend < chunkLength )

				if ( chunkSend > 0 )
				{
					totalSend += chunkSend;
				}
				chunkIndex++;

			}  // while ( totalSend < bufferlen )

			writeCount += numMessagesInBuffer;

			if (writeCount % 10000 <= (int)numMessagesInBuffer)
				resetDeadlockTimer();
		}
		return;
	}
};

class ManyThreadChannel
{
public:
	RsslThreadId* pThreadId;
	RsslServer* pServer;
	bool serverChannel;  /* Select if this test is with a server channel or client channel */
	int msgCount;		 /* msgCount for a writer thread */
	int* readCount;		 /* Count of messages read */
	RsslMutex* lock;	 /* Reading lock, used to protect the counts */

	ManyThreadChannel()
	{
		pThreadId = NULL;
		pServer = NULL;
		serverChannel = false;
		msgCount = 10;
		readCount = NULL;
		lock = NULL;
	}

	/*	Starts up a reader channel and then starts the readLoop.  Once the 
		test has been signalled to end with shutdownTest, closes the channel with 
		rsslCloseChannel.*/
	void manyChannelReader(RsslBool blocking)
	{
		RsslChannel* chnl;
		RsslError err;
		ClientChannel clientOpts;
		ServerChannel serverChnl;

		if (serverChannel)
		{
			serverChnl.pThreadId = pThreadId;
			serverChnl.pServer = pServer;

			serverChnl.channelStart(blocking);
			if (serverChnl.pChnl == NULL)
			{
				failTest = true;
				ASSERT_NE(serverChnl.pChnl, (RsslChannel*)NULL) << "Server channel creation failed";
			}
			chnl = serverChnl.pChnl;
		}
		else
		{
			clientOpts.pThreadId = pThreadId;

			clientOpts.channelStart(blocking);

			if (clientOpts.pChnl == NULL)
			{
				failTest = true;
				ASSERT_NE(clientOpts.pChnl, (RsslChannel*)NULL) << "Client channel creation failed";
			}
			chnl = clientOpts.pChnl;
		}

		ReadChannel readOpts;

		readOpts.pThreadId = pThreadId;
		readOpts.pChnl = chnl;
		readOpts.lock = lock;
		readOpts.readCount = readCount;

		readOpts.readLoop(blocking);

		rsslCloseChannel(chnl, &err);
	}


	/*	Starts up a reader channel and then starts the writeLoop, which will 
		write msgCount number of messages to the wire. Once the messages are written, 
		waits for shutdownTest to be true, and then closes the channel with	rsslCloseChannel.*/
	void manyChannelWriter(RsslBool blocking)
	{
		RsslChannel* chnl;
		RsslError err;
		ServerChannel serverChnl;
		ClientChannel clientOpts;

		if (serverChannel)
		{
			serverChnl.pThreadId = pThreadId;
			serverChnl.pServer = pServer;

			serverChnl.channelStart(blocking);
			if (serverChnl.pChnl == NULL)
			{
				failTest = true;
				ASSERT_NE(serverChnl.pChnl, (RsslChannel*)NULL) << "Server channel creation failed";
			}
			chnl = serverChnl.pChnl;
		}
		else
		{
			clientOpts.pThreadId = pThreadId;

			clientOpts.channelStart(blocking);

			if (clientOpts.pChnl == NULL)
			{
				failTest = true;
				ASSERT_NE(clientOpts.pChnl, (RsslChannel*)NULL) << "Client channel creation failed";
			}
			chnl = clientOpts.pChnl;
		}

		WriteChannel writeOpts;

		writeOpts.pThreadId = pThreadId;
		writeOpts.pChnl = chnl;
		writeOpts.msgCount = msgCount;

		writeOpts.writeLoop(blocking);

        resetDeadlockTimer();

		while (!shutdownTest)
		    time_sleep(500);	

		rsslCloseChannel(chnl, &err);
	}

};

/* This function starts up the RsslServer. */
RsslServer* startupServer(RsslBool blocking)
{
	RsslError err;
	RsslBindOptions bindOpts;
	RsslServer* server;
	rsslClearBindOpts(&bindOpts);

	bindOpts.serviceName = (char*)"15000";
	bindOpts.protocolType = TEST_PROTOCOL_TYPE;  /* These tests are just sending a pre-set string across the wire, so protocol type should not be RWF */
	bindOpts.channelsBlocking = blocking;
	bindOpts.serverBlocking = blocking;

	server = rsslBind(&bindOpts, &err);

	if ( server == NULL )
	{
		failTest = true;
		std::cout << "Could not start rsslServer.  Error text: " << err.text << std::endl;
	}

	return server;

}

/* This function starts up the RsslServer. */
RsslServer* bindRsslServer(TUServerConfig* pServerConfig)
{
	RsslServer* server;
	RsslError err;
	RsslBindOptions bindOpts;

	rsslClearBindOpts(&bindOpts);

	bindOpts.guaranteedOutputBuffers = 5000;

	bindOpts.interfaceName = (char*)"localhost";

	bindOpts.maxOutputBuffers = 5000;
	bindOpts.serviceName = pServerConfig->portNo;

	bindOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	bindOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
	bindOpts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	bindOpts.tcp_nodelay = RSSL_TRUE;
	bindOpts.sysSendBufSize = 0;
	bindOpts.sysRecvBufSize = 0;
	bindOpts.maxFragmentSize = pServerConfig->maxFragmentSize;
	bindOpts.wsOpts.protocols = pServerConfig->wsProtocolList;

	bindOpts.channelsBlocking = pServerConfig->blocking;
	bindOpts.serverBlocking = pServerConfig->blocking;

	bindOpts.connectionType = pServerConfig->connType;

	if (bindOpts.connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		bindOpts.encryptionOpts.serverCert = pServerConfig->serverCert;
		bindOpts.encryptionOpts.serverPrivateKey = pServerConfig->serverKey;
		bindOpts.encryptionOpts.cipherSuite = pServerConfig->cipherSuite;
	}

	bindOpts.compressionType = pServerConfig->compressionType;
	bindOpts.compressionLevel = pServerConfig->compressionLevel;

	server = rsslBind(&bindOpts, &err);

	if ( server == NULL )
	{
		failTest = true;
		std::cout << "Could not start rsslServer. Port: " << pServerConfig->portNo << std::endl << "Error text: " << err.text << std::endl;
	}

	return server;
}

void clearTUServerConfig(TUServerConfig* pServerConfig)
{
	pServerConfig->blocking = RSSL_FALSE;
	pServerConfig->maxFragmentSize = RSSL_MAX_MSG_SIZE;
	snprintf(pServerConfig->portNo, sizeof(pServerConfig->portNo), "%s", "");
	pServerConfig->connType = RSSL_CONN_TYPE_INIT;
	snprintf(pServerConfig->wsProtocolList, sizeof(pServerConfig->wsProtocolList), "%s", "");

	snprintf(pServerConfig->serverCert, sizeof(pServerConfig->serverCert), "%s", "");
	snprintf(pServerConfig->serverKey, sizeof(pServerConfig->serverKey), "%s", "");
	snprintf(pServerConfig->cipherSuite, sizeof(pServerConfig->cipherSuite), "%s", "");

	pServerConfig->compressionType = RSSL_COMP_NONE;
	pServerConfig->compressionLevel = 0U;
}

void clearTUClientConfig(TUClientConfig* pClientConfig)
{
	pClientConfig->blocking = RSSL_FALSE;
	pClientConfig->maxFragmentSize = RSSL_MAX_MSG_SIZE;
	snprintf(pClientConfig->portNo, sizeof(pClientConfig->portNo), "%s", "");
	pClientConfig->connType = RSSL_CONN_TYPE_INIT;
	snprintf(pClientConfig->wsProtocolList, sizeof(pClientConfig->wsProtocolList), "%s", "");

	pClientConfig->encryptionProtocolFlags = RSSL_ENC_TLSV1_2;
	pClientConfig->encryptedProtocol = RSSL_CONN_TYPE_SOCKET;

	snprintf(pClientConfig->openSSLCAStore, sizeof(pClientConfig->openSSLCAStore), "%s", "");

	pClientConfig->compressionType = RSSL_COMP_NONE;
}

void constructTUServerConfig(
	int configIndex,
	TUServerConfig& serverConfig,
	RsslBool blocking,
	RsslUInt32 maxFragmentSize,
	RsslConnectionTypes connType,
	RsslCompTypes compressType,
	RsslUInt32 compressLevel
	)
{
	clearTUServerConfig(&serverConfig);

	serverConfig.blocking = blocking;
	serverConfig.maxFragmentSize = maxFragmentSize;
	serverConfig.connType = connType;
	if (configIndex == 0)
	{
		strncpy(serverConfig.portNo, "15000", sizeof(serverConfig.portNo));
	}
	else
	{
		strncpy(serverConfig.portNo, "15001", sizeof(serverConfig.portNo));
	}
	snprintf(serverConfig.wsProtocolList, sizeof(serverConfig.wsProtocolList), "rssl.json.v2, rssl.rwf, tr_json2");

	if (connType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		snprintf(serverConfig.serverKey, sizeof(serverConfig.serverKey), "%s", getPathServerKey());
		snprintf(serverConfig.serverCert, sizeof(serverConfig.serverCert), "%s", getPathServerCert());
		snprintf(serverConfig.cipherSuite, sizeof(serverConfig.cipherSuite), "%s", "");
	}

	serverConfig.compressionType = compressType;
	serverConfig.compressionLevel = compressLevel;

	return;
}

void constructTUClientConfig(
	int configIndex,
	TUClientConfig& clientConfig,
	RsslBool blocking,
	RsslConnectionTypes connType,
	RsslConnectionTypes encryptedConnType,
	RsslUInt8 wsProtocolType,
	RsslCompTypes compressType
	)
{
	clearTUClientConfig(&clientConfig);

	clientConfig.blocking = blocking;
	clientConfig.connType = connType;
	if (configIndex == 0)
	{
		strncpy(clientConfig.portNo, "15000", sizeof(clientConfig.portNo));
	}
	else
	{
		strncpy(clientConfig.portNo, "15001", sizeof(clientConfig.portNo));
	}

	if (connType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		clientConfig.encryptedProtocol = encryptedConnType;
		snprintf(clientConfig.openSSLCAStore, sizeof(clientConfig.openSSLCAStore), "%s", getOpenSSLCAStore());
	}

	if (connType == RSSL_CONN_TYPE_WEBSOCKET
		|| connType == RSSL_CONN_TYPE_ENCRYPTED && encryptedConnType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		if (wsProtocolType == RSSL_JSON_PROTOCOL_TYPE)
		{
			snprintf(clientConfig.wsProtocolList, sizeof(clientConfig.wsProtocolList), "%s", "rssl.json.v2");
		}
		else
		{
			snprintf(clientConfig.wsProtocolList, sizeof(clientConfig.wsProtocolList), "%s", "rssl.rwf");
		}
	}

	clientConfig.compressionType = compressType;
	return;
}

void clearTUConnection(TUConnection* pConnection)
{
	pConnection->pServerChannel = NULL;
	pConnection->pClientChannel = NULL;
	pConnection->pServer = NULL;
}


/* Google Test requires that all functions that have test usage(i.e. using the ASSERT or EXPECT macros) have a void return type.
	Since RSSL_THREAD declaration functions require a void* return type, they will need to wrap the actual tested functionality in another function. */

RSSL_THREAD_DECLARE(nonBlockingServerConnectThread, pArg)
{
	((ServerChannel*)pArg)->channelStart(RSSL_FALSE);
	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingClientConnectThread, pArg)
{
	((ClientChannel*)pArg)->channelStart(RSSL_FALSE);
	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingReadThread, pArg)
{
	((ReadChannel*)pArg)->readLoop(RSSL_FALSE);
	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWriteThread, pArg)
{
	((WriteChannel*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWriteTransportThread, pArg)
{
	((WriteChannelTransport*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWritePackedThread, pArg)
{
	((WriteChannelPacked*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWriteSystemThread, pArg)
{
	((WriteChannelSystem*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWriteSeveralBufferTransportThread, pArg)
{
	((WriteChannelSeveralBufferTransport*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingWriteEncryptBufferTransportThread, pArg)
{
	((WriteChannelEncryptBufferTransport*)pArg)->writeLoop(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingManyChannelWriteThread, pArg)
{
	((ManyThreadChannel*)pArg)->manyChannelWriter(RSSL_FALSE);

	return 0;
}

RSSL_THREAD_DECLARE(nonBlockingManyChannelReadThread, pArg)
{
	((ManyThreadChannel*)pArg)->manyChannelReader(RSSL_FALSE);
	return 0;
}

RSSL_THREAD_DECLARE(pingThread, pArg)
{
	((PingChannel*)pArg)->pingLoop();
	return 0;
}

RSSL_THREAD_DECLARE(blockingServerConnectThread, pArg)
{
	((ServerChannel*)pArg)->channelStart(RSSL_TRUE);
	return 0;
}

RSSL_THREAD_DECLARE(blockingClientConnectThread, pArg)
{
	((ClientChannel*)pArg)->channelStart(RSSL_TRUE);
	return 0;
}

RSSL_THREAD_DECLARE(blockingReadThread, pArg)
{
	((ReadChannel*)pArg)->readLoop(RSSL_TRUE);
	return 0;
}

RSSL_THREAD_DECLARE(blockingWriteThread, pArg)
{
	((WriteChannel*)pArg)->writeLoop(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingWriteTransportThread, pArg)
{
	((WriteChannelTransport*)pArg)->writeLoop(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingWritePackedThread, pArg)
{
	((WriteChannelPacked*)pArg)->writeLoop(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingWriteSystemThread, pArg)
{
	((WriteChannelSystem*)pArg)->writeLoop(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingWriteSeveralBufferTransportThread, pArg)
{
	((WriteChannelSeveralBufferTransport*)pArg)->writeLoop(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingManyChannelWriteThread, pArg)
{
	((ManyThreadChannel*)pArg)->manyChannelWriter(RSSL_TRUE);

	return 0;
}

RSSL_THREAD_DECLARE(blockingManyChannelReadThread, pArg)
{
	((ManyThreadChannel*)pArg)->manyChannelReader(RSSL_TRUE);
	return 0;
}

// GlobalLockTests use RSSL_LOCK_GLOBAL lock
class GlobalLockTests : public ::testing::Test {
protected:
	RsslChannel* serverChannel;
	RsslChannel* clientChannel;
	RsslServer* server;
	int msgsRead;
	RsslMutex readLock;

	RsslTimeValue startTime;
	RsslTimeValue curTime;
	RsslUInt64	maxTimeWait;

	virtual void SetUp()
	{
		RsslError err;

		shutdownTest = false;
		failTest = false;
		server = NULL;
		serverChannel = NULL;
		clientChannel = NULL;

		RSSL_MUTEX_INIT(&readLock);

		msgsRead = 0;

		startTime = 0;
		curTime = 0;
		maxTimeWait = 0;

		rsslInitialize(RSSL_LOCK_GLOBAL, &err);
	}

	virtual void TearDown()
	{
		RsslError err;
		rsslCloseServer(server, &err);
		rsslUninitialize();
		server = NULL;
		serverChannel = NULL;
		clientChannel = NULL;
		RSSL_MUTEX_DESTROY(&readLock);
		resetDeadlockTimer();
	}

	void startupServerAndConections(RsslBool blocking)
	{
		RsslThreadId serverThread, clientThread;
		ClientChannel clientOpts;
		ServerChannel serverChnl;
		serverChnl.pThreadId = &serverThread;
		clientOpts.pThreadId = &clientThread;

		server = startupServer(blocking);
		
		ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";

		serverChnl.pServer = server;
		
		if (blocking == RSSL_FALSE)
		{
			RSSL_THREAD_START(&serverThread, nonBlockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, nonBlockingClientConnectThread, &clientOpts);
		}
		else
		{
			RSSL_THREAD_START(&serverThread, blockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, blockingClientConnectThread, &clientOpts);
		}

		RSSL_THREAD_JOIN(serverThread);
		RSSL_THREAD_JOIN(clientThread);

		serverChannel = serverChnl.pChnl;
		clientChannel = clientOpts.pChnl;

		if (!serverChannel || !clientChannel || serverChannel->state != RSSL_CH_STATE_ACTIVE || clientChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
	}

	void startupServerAndConections(
		int configIndex,
		RsslBool blocking,
		RsslUInt32 maxFragmentSize,
		RsslConnectionTypes connType,
		RsslConnectionTypes encryptedProtocol,
		RsslUInt8 wsProtocolType,
		RsslCompTypes compressType,
		RsslUInt32 compressLevel
	)
	{
		RsslThreadId serverThread, clientThread;
		ClientChannel clientOpts;
		ServerChannel serverChnl;
		serverChnl.pThreadId = &serverThread;
		clientOpts.pThreadId = &clientThread;

		TUServerConfig serverConfig;
		constructTUServerConfig(configIndex, serverConfig, blocking, maxFragmentSize, connType, compressType, compressLevel);

		server = bindRsslServer(&serverConfig);

		ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";

		serverChnl.pServer = server;
		serverChnl.setTUServerConfig(&serverConfig);

		TUClientConfig clientConfig;
		constructTUClientConfig(configIndex, clientConfig, blocking, connType, encryptedProtocol, wsProtocolType, compressType);
		clientOpts.setTUClientConfig(&clientConfig);

		if (blocking == RSSL_FALSE)
		{
			RSSL_THREAD_START(&serverThread, nonBlockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, nonBlockingClientConnectThread, &clientOpts);
		}
		else
		{
			RSSL_THREAD_START(&serverThread, blockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, blockingClientConnectThread, &clientOpts);
		}

		RSSL_THREAD_JOIN(serverThread);
		RSSL_THREAD_JOIN(clientThread);

		RSSL_THREAD_DETACH(&serverThread);
		RSSL_THREAD_DETACH(&clientThread);

		serverChannel = serverChnl.pChnl;
		clientChannel = clientOpts.pChnl;

		if (!serverChannel || !clientChannel || serverChannel->state != RSSL_CH_STATE_ACTIVE || clientChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
	}

	bool checkLongTimeNoRead(int& readCountPrev, int readCount)
	{
		bool isCheckLongTimeNoRead = false;
		if (readCountPrev < readCount)
		{
			if (startTime > 0 && curTime > 0)
			{
				if ( (curTime - startTime) > maxTimeWait )
					maxTimeWait = (curTime - startTime);
			}
			readCountPrev = readCount;
			startTime = 0;
		}
		else
		{
			curTime = rsslGetTimeMilli();
			if (startTime == 0)
			{
				startTime = curTime;
			}
			else if ( (curTime - startTime) > MAX_TIMEWAIT_NOUPDATES_MS )
			{
				if ( (curTime - startTime) > maxTimeWait )
					maxTimeWait = (curTime - startTime);
				failTest = true;
				isCheckLongTimeNoRead = true;
			}
		}
		return isCheckLongTimeNoRead;
	}
};

class SimpleBufferChecker : public TestHandler
{
public:
	std::vector<int> indices;  // indices of messages read

	SimpleBufferChecker() {
		indices.reserve(MAX_MSG_WRITE_COUNT);
	}

	bool handleDataBuffer(RsslBuffer* pReadBuffer, char* pErrorText, size_t szErrorText)
	{
		char* p = pReadBuffer->data;
		RsslUInt32 buflen = pReadBuffer->length;
		unsigned k = 0;

		int ind;
		const char* testBuffer = "TestDataInfo";
		size_t lenTestBuffer = strlen(testBuffer);
		size_t len;

		bool isTextCorrect = false;

		do {
			if (*p != '[')
				break;
			while (k < buflen && *(p + k) == '[')
				k++;
			if (k >= buflen)
				break;

			if (!isdigit(*(p + k)))
				break;

			ind = atoi(p + k);
			while (k < buflen && *(p + k) != ':')
				k++;

			k++;  // skip ':'
			if (k >= buflen)
				break;

			if (buflen - k > lenTestBuffer)
				len = lenTestBuffer;
			else
				len = buflen - k;

			if (strncmp(p + k, testBuffer, len) != 0)
				break;

			indices.push_back(ind);

			isTextCorrect = true;
		} while (false);

		return isTextCorrect;
	}

	void dumpIndices(std::ostringstream& outputStream, RsslInt32 n)
	{
		outputStream << "{sz:" << indices.size() << "}[";
		if (!indices.empty()) {
			std::vector<int>::iterator it;
			if (n > 0)
			{
				if (n < indices.size())
					it = std::next(indices.begin(), n);
				else
					it = indices.begin();
			}
			else if (n < 0)
			{
				if (-n < indices.size())
					it = std::prev(indices.end(), -n);
				else
					it = indices.begin();
			}
			else
			{
				it = indices.begin();
			}

			auto end = indices.end();

			while (it != end) {
				outputStream << *it++;
				if (it != end)
					outputStream << ',';
			}
		}
		outputStream << ']';
		return;
	}

	void dumpLast100Indices(const char* title)
	{
		std::ostringstream outputStream;
		dumpIndices(outputStream, -100);
		std::cout << "Test failed. [" << title << "]. indices: " << outputStream.str() << std::endl;
		return;
	}
};

class GlobalLockTestsPackedFixture : public GlobalLockTests, public ::testing::WithParamInterface<GLobalLockPackedTestParams>
{
	virtual void SetUp()
	{
		GlobalLockTests::SetUp();
		const GLobalLockPackedTestParams& testParams = GetParam();
		// Check conditions to skip the test
#if (RUN_TEST_ENCRYPTED == 0)
		if (testParams.connType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			FAIL() << "Test on an encrypted connection - skipped.";
			return;
		}
#endif
	}

public:
	// Calculate the value of packed buffer length given the limit maxFragmentSize
	// Here, it do not check that required length can be more than the limit maxFragmentSize
	// because it could check inside unit-tests
	RsslUInt32 getPackedBufferLength()
	{
		const GLobalLockPackedTestParams& testParams = GetParam();
		RsslUInt32 requiredPackedBufferLength = testParams.bufferLength;
		RsslUInt32 packedBufferLength = 0;
		RsslUInt32 maxSize = (testParams.maxFragmentSize > 0 ? testParams.maxFragmentSize : RSSL_MAX_MSG_SIZE);

		if (testParams.wsProtocolType == RSSL_JSON_PROTOCOL_TYPE &&
			(testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
			|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET))
		{
			packedBufferLength = requiredPackedBufferLength;
		}
		else
		{	// For packed buffer (Socket, WebSocket+RWF) reserves additional 2 bytes for length.
			if (requiredPackedBufferLength >= maxSize)
			{
				packedBufferLength = requiredPackedBufferLength - 2;
			}
			else if (requiredPackedBufferLength == maxSize - 1)
			{
				packedBufferLength = requiredPackedBufferLength - 1;
			}
			else
			{
				packedBufferLength = requiredPackedBufferLength;
			}
		}
		return packedBufferLength;
	}

	// Read packed messages
	// Calculate the expected number of messages that will be read.
	// When WebSocket + JSON case then packed JSON messages combine to items of JSON array.
	// rsslRead returns only message - the JSON array.
	int calculateExpectedReadMessagesCount(bool isBlockedConn, int msgWriteCount)
	{
		const GLobalLockPackedTestParams& testParams = GetParam();

		return calcExpectedReadMessagesCount(isBlockedConn, msgWriteCount,
			testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
			testParams.bufferLength, testParams.msgLength);
	}
};

class GlobalLockTestsFragmentedFixture : public GlobalLockTests,
	public ::testing::WithParamInterface<GLobalLockFragmentedTestParams>
{
	virtual void SetUp()
	{
		GlobalLockTests::SetUp();
		const GLobalLockFragmentedTestParams& testParams = GetParam();
		// Check conditions to skip the test
#if (RUN_TEST_ENCRYPTED == 0)
		if (testParams.connType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			FAIL() << "Test on an encrypted connection - skipped.";
			return;
		}
#endif
	}
};

/*	Test kicks off one writer and one reader thread for the client and server.  Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish.
	This test has both sides reading and writing in different threads.*/
TEST_F(GlobalLockTests, NonBlockingTwoWayClientServer)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;
	WriteChannel serverWriteOpts, clientWriteOpts;
	RsslError err;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	shutdownTest = true;
	time_sleep(50);

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(serverWriteThread);

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(clientReadThread);
	RSSL_THREAD_JOIN(serverReadThread);

	ASSERT_FALSE(failTest) << "Test failed.";

}

TEST_P(GlobalLockTestsFragmentedFixture, NonBlockingTwoWayClientServerTransport)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockFragmentedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_NONBLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Non-Blocking connection - skipped.";
		return;
	}
#endif

	WriteChannelTransport serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, "SrvW");
	WriteChannelTransport clientWriteOpts(testParams.msgLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteTransportThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteTransportThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(200);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();
		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (msgWriteCount * 2) && !failTest);

	shutdownTest = true;
	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientReadThread);
		RSSL_THREAD_KILL(&serverReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientReadThread);
		RSSL_THREAD_JOIN(serverReadThread);
	}

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

TEST_P(GlobalLockTestsPackedFixture, NonBlockingTwoWayClientServerPacked)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockPackedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_NONBLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Non-Blocking connection - skipped.";
		return;
	}
#endif

	RsslUInt32 packedBufferLength = getPackedBufferLength();

	WriteChannelPacked serverWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "SrvW");
	WriteChannelPacked clientWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// The expected number of messages that will be read
	int nReadMsgsExpect = calculateExpectedReadMessagesCount(false, msgWriteCount);

	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWritePackedThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWritePackedThread, (void*)&clientWriteOpts);

	RsslTimeValue startTime = 0;
	RsslTimeValue curTime = 0;

	do
	{
		time_sleep(20);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();
		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2)
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (nReadMsgsExpect * 2) && !failTest);

//	std::cout << "GlobalLockTestsPackedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2) << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;

	shutdownTest = true;
	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientReadThread);
		RSSL_THREAD_KILL(&serverReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientReadThread);
		RSSL_THREAD_JOIN(serverReadThread);
	}

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

/*	Test kicks off one writer and one reader thread for the client and server.  Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish.  
	This test has both sides reading and writing in different threads.*/
TEST_F(GlobalLockTests, BlockingTwoWayClientServer)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;
	WriteChannel serverWriteOpts, clientWriteOpts;
	RsslError err;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = MAX_MSG_WRITE_COUNT;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = MAX_MSG_WRITE_COUNT;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWriteThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWriteThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);
	shutdownTest = true;
	time_sleep(50);

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(serverWriteThread);
	RSSL_THREAD_KILL(&clientReadThread);
	RSSL_THREAD_KILL(&serverReadThread);

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

	ASSERT_FALSE(failTest) << "Test failed.";
}

TEST_P(GlobalLockTestsFragmentedFixture, BlockingTwoWayClientServerTransport)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockFragmentedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_BLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Blocking connection - skipped.";
		return;
	}
#endif

	WriteChannelTransport serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, "SrvW");
	WriteChannelTransport clientWriteOpts(testParams.msgLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = msgWriteCount;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = msgWriteCount;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWriteTransportThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWriteTransportThread, (void*)&clientWriteOpts);

	//std::cout << "GlobalLockTestsFragmentedFixture  Run test. msgWriteCount: " << msgWriteCount << std::endl;

	do
	{
		time_sleep(200);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (msgWriteCount * 2) && !failTest);

//	std::cout << "GlobalLockTestsFragmentedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;

	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockTestsFragmentedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

//	std::cout << "GlobalLockTestsFragmentedFixture  Before rsslCloseChannel." << std::endl;

	EXPECT_EQ( rsslCloseChannel(serverChannel, &err), RSSL_RET_SUCCESS ) << "serverChannel close error: " << err.text;
	EXPECT_EQ( rsslCloseChannel(clientChannel, &err), RSSL_RET_SUCCESS ) << "clientChannel close error: " << err.text;

	RSSL_THREAD_KILL(&clientReadThread);
	RSSL_THREAD_KILL(&serverReadThread);

	//std::cout << "GlobalLockTestsFragmentedFixture  Before Join clientReadThread." << std::endl;
	//RSSL_THREAD_JOIN(clientReadThread);
	//std::cout << "GlobalLockTestsFragmentedFixture  Before Join serverReadThread." << std::endl;
	//RSSL_THREAD_JOIN(serverReadThread);

//	std::cout << "GlobalLockTestsFragmentedFixture  FINISH." << std::endl;

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

TEST_P(GlobalLockTestsPackedFixture, BlockingTwoWayClientServerPacked)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockPackedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_BLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Blocking connection - skipped.";
		return;
	}
#endif
	RsslUInt32 packedBufferLength = getPackedBufferLength();

	WriteChannelPacked serverWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "SrvW");
	WriteChannelPacked clientWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// The expected number of messages that will be read
	int nReadMsgsExpect = calculateExpectedReadMessagesCount(true, msgWriteCount);

	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = nReadMsgsExpect;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = nReadMsgsExpect;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWritePackedThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWritePackedThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(20);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect * 2)
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (nReadMsgsExpect * 2) && !failTest);

//	std::cout << "GlobalLockTestsPackedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2) << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;

	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockTestsPackedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

//	std::cout << "GlobalLockTestsPackedFixture  Before rsslCloseChannel." << std::endl;
	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

//	std::cout << "GlobalLockTestsPackedFixture  Before Join Read-threads." << std::endl;
	RSSL_THREAD_KILL(&clientReadThread);
	RSSL_THREAD_KILL(&serverReadThread);

	//RSSL_THREAD_JOIN(clientReadThread);
	//RSSL_THREAD_JOIN(serverReadThread);

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

INSTANTIATE_TEST_SUITE_P(
	MsgLength,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 1250, 6000, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 2377, 6000, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 5999, 6000, 0, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrypted,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2377, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5999, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWF,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2377, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSON,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2377, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockRWF,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2377, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockJSON,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2377, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz3000,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 1250, 3000, 3000, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 2377, 3000, 3000, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 2998, 3000, 3000, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz3000,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2377, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2998, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz3000,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2377, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz38750,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,   100,  6000, 38750,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  1250,  6000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  2377,  6000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  5999,  6000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 38000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz38750,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100,  6000, 38750,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2377,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 38000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz38750,
	GlobalLockTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100,  6000, 38750,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2377,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999,  6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 38000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);


//GLobalLockFragmentedTestParams(
//	RsslConnectionTypes cnType, RsslUInt32 msgLen, RsslUInt32 maxFragSz, RsslUInt32 msgCount,
//	RsslConnectionTypes encrProt, RsslUInt8 wsProt)

INSTANTIATE_TEST_SUITE_P(
	MsgLength,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 0,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 0, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 0, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz3000,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  2797, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  2979, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz38750,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 38750,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 38750,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrypted,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,   100, 0,     0, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  1250, 0,     0, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6135, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6145, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6150, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 12263, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWF,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 0, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz3000,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 3000, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2797, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2979, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz38750,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 38750, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSON,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 0, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz3000,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 3000,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2797, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  2979, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz38750,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 38750,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  5999, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockRWF,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,   100, 0, 40000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  1250, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  5999, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6135, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6145, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6150, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 12263, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockJSON,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,   100, 0, 40000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  1250, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  5999, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6135, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6145, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  6150, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 12263, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	Compress,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 2000, 0, 50000, RSSL_CONN_TYPE_INIT, 0, RSSL_COMP_ZLIB, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 2000, 0, 50000, RSSL_CONN_TYPE_INIT, 0, RSSL_COMP_ZLIB, 1 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 2000, 0, 50000, RSSL_CONN_TYPE_INIT, 0, RSSL_COMP_ZLIB, 2 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 2000, 0, 50000, RSSL_CONN_TYPE_INIT, 0, RSSL_COMP_ZLIB, 3 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 2000, 0, 50000, RSSL_CONN_TYPE_INIT, 0, RSSL_COMP_LZ4,  0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	CompressEncrypted,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2000, 0, 50000, RSSL_CONN_TYPE_SOCKET, 0, RSSL_COMP_ZLIB, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2000, 0, 50000, RSSL_CONN_TYPE_SOCKET, 0, RSSL_COMP_ZLIB, 1 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2000, 0, 50000, RSSL_CONN_TYPE_SOCKET, 0, RSSL_COMP_ZLIB, 2 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2000, 0, 50000, RSSL_CONN_TYPE_SOCKET, 0, RSSL_COMP_ZLIB, 3 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 2000, 0, 50000, RSSL_CONN_TYPE_SOCKET, 0, RSSL_COMP_LZ4,  0 )
	)
);


// GlobalLockChannelTests use RSSL_LOCK_GLOBAL_AND_CHANNEL lock
class GlobalLockChannelTests : public ::testing::Test {
protected:
	// two-way client-server connections
	TUConnection connectionA;  // Server-channel for writing : Client-channel for reading
	TUConnection connectionB;  // Server-channel for reading : Client-channel for writing

	int msgsRead;
	RsslMutex readLock;

	RsslTimeValue startTime;
	RsslTimeValue curTime;
	RsslUInt64	maxTimeWait;

	virtual void SetUp()
	{
		RsslError err;

		shutdownTest = false;
		failTest = false;

		clearTUConnection(&connectionA);
		clearTUConnection(&connectionB);

		RSSL_MUTEX_INIT(&readLock);

		msgsRead = 0;

		startTime = 0;
		curTime = 0;
		maxTimeWait = 0;

		rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);
	}

	virtual void TearDown()
	{
		RsslError err;
		rsslCloseServer(connectionA.pServer, &err);
		rsslCloseServer(connectionB.pServer, &err);

		rsslUninitialize();

		clearTUConnection(&connectionA);
		clearTUConnection(&connectionB);

		RSSL_MUTEX_DESTROY(&readLock);
		resetDeadlockTimer();
	}

	void startupServerAndConections(
		int configIndex,
		RsslBool blocking,
		RsslUInt32 maxFragmentSize,
		RsslConnectionTypes connType,
		RsslConnectionTypes encryptedProtocol,
		RsslUInt8 wsProtocolType,
		RsslCompTypes compressType,
		RsslUInt32 compressLevel,
		TUConnection* pConnection
	)
	{
		RsslThreadId serverThread, clientThread;
		ClientChannel clientChnl;
		ServerChannel serverChnl;
		RsslServer* server;
		serverChnl.pThreadId = &serverThread;
		clientChnl.pThreadId = &clientThread;

		TUServerConfig serverConfig;
		constructTUServerConfig(configIndex, serverConfig, blocking, maxFragmentSize, connType, compressType, compressLevel);

		server = bindRsslServer(&serverConfig);

		ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";

		serverChnl.pServer = server;
		serverChnl.setTUServerConfig(&serverConfig);

		TUClientConfig clientConfig;
		constructTUClientConfig(configIndex, clientConfig, blocking, connType, encryptedProtocol, wsProtocolType, compressType);
		clientChnl.setTUClientConfig(&clientConfig);

		if (blocking == RSSL_FALSE)
		{
			RSSL_THREAD_START(&serverThread, nonBlockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, nonBlockingClientConnectThread, &clientChnl);
		}
		else
		{
			RSSL_THREAD_START(&serverThread, blockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, blockingClientConnectThread, &clientChnl);
		}

		RSSL_THREAD_JOIN(serverThread);
		RSSL_THREAD_JOIN(clientThread);

		RSSL_THREAD_DETACH(&serverThread);
		RSSL_THREAD_DETACH(&clientThread);

		pConnection->pServer = server;
		pConnection->pServerChannel = serverChnl.pChnl;
		pConnection->pClientChannel = clientChnl.pChnl;

		if (!(pConnection->pServerChannel) || !(pConnection->pClientChannel)
			|| (pConnection->pServerChannel)->state != RSSL_CH_STATE_ACTIVE || (pConnection->pClientChannel)->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
	}

	bool checkLongTimeNoRead(int& readCountPrev, int readCount)
	{
		bool isCheckLongTimeNoRead = false;
		if (readCountPrev < readCount)
		{
			if (startTime > 0 && curTime > 0)
			{
				if ( (curTime - startTime) > maxTimeWait )
					maxTimeWait = (curTime - startTime);
			}
			readCountPrev = readCount;
			startTime = 0;
		}
		else
		{
			curTime = rsslGetTimeMilli();
			if (startTime == 0)
			{
				startTime = curTime;
			}
			else if ( (curTime - startTime) > MAX_TIMEWAIT_NOUPDATES_MS )
			{
				if ( (curTime - startTime) > maxTimeWait )
					maxTimeWait = (curTime - startTime);
				failTest = true;
				isCheckLongTimeNoRead = true;
			}
		}
		return isCheckLongTimeNoRead;
	}
};

class GlobalLockChannelTestsPackedFixture
	: public GlobalLockChannelTests, public ::testing::WithParamInterface<GLobalLockPackedTestParams>
{
	virtual void SetUp()
	{
		GlobalLockChannelTests::SetUp();
		const GLobalLockPackedTestParams& testParams = GetParam();
		// Check conditions to skip the test
#if (RUN_TEST_ENCRYPTED == 0)
		if (testParams.connType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			FAIL() << "Test on an encrypted connection - skipped.";
			return;
		}
#endif
	}

public:
	// Calculate the value of packed buffer length given the limit maxFragmentSize
	// Here, it do not check that required length can be more than the limit maxFragmentSize
	// because it could check inside unit-tests
	RsslUInt32 getPackedBufferLength()
	{
		const GLobalLockPackedTestParams& testParams = GetParam();
		RsslUInt32 requiredPackedBufferLength = testParams.bufferLength;
		RsslUInt32 packedBufferLength = 0;
		RsslUInt32 maxSize = (testParams.maxFragmentSize > 0 ? testParams.maxFragmentSize : RSSL_MAX_MSG_SIZE);

		if (testParams.wsProtocolType == RSSL_JSON_PROTOCOL_TYPE &&
			(testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
				|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET))
		{
			packedBufferLength = requiredPackedBufferLength;
		}
		else
		{	// For packed buffer (Socket, WebSocket+RWF) reserves additional 2 bytes for length.
			if (requiredPackedBufferLength >= maxSize)
			{
				packedBufferLength = requiredPackedBufferLength - 2;
			}
			else if (requiredPackedBufferLength == maxSize - 1)
			{
				packedBufferLength = requiredPackedBufferLength - 1;
			}
			else
			{
				packedBufferLength = requiredPackedBufferLength;
			}
		}
		return packedBufferLength;
	}

	// Read packed messages
	// Calculate the expected number of messages that will be read.
	// When WebSocket + JSON case then packed JSON messages combine to items of JSON array.
	// rsslRead returns only message - the JSON array.
	int calculateExpectedReadMessagesCount(bool isBlockedConn, int msgWriteCount)
	{
		const GLobalLockPackedTestParams& testParams = GetParam();

		return calcExpectedReadMessagesCount(isBlockedConn, msgWriteCount,
			testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
			testParams.bufferLength, testParams.msgLength);
	}
};

class GlobalLockChannelTestsFragmentedFixture
	: public GlobalLockChannelTests, public ::testing::WithParamInterface<GLobalLockFragmentedTestParams>
{
	virtual void SetUp()
	{
		GlobalLockChannelTests::SetUp();
		const GLobalLockFragmentedTestParams& testParams = GetParam();
		// Check conditions to skip the test
#if (RUN_TEST_ENCRYPTED == 0)
		if (testParams.connType == RSSL_CONN_TYPE_ENCRYPTED)
		{
			FAIL() << "Test on an encrypted connection - skipped.";
			return;
		}
#endif
	}
};

TEST_P(GlobalLockChannelTestsFragmentedFixture, NonBlockingTwoWayClientServerTransport)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockFragmentedTestParams& testParams = GetParam();

	WriteChannelTransport serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, "SrvW");
	WriteChannelTransport clientWriteOpts(testParams.msgLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// serverChannelWr, clientChannelRd are connected by port 15000
	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel,
		&connectionA);

	// serverChannelRd, clientChannelWr are connected by port 15001
	startupServerAndConections(1, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel,
		&connectionB);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionB.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionB.pClientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteTransportThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteTransportThread, (void*)&clientWriteOpts);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture.NonBlocking  Run test. msgWriteCount: " << msgWriteCount << std::endl;

	do
	{
		time_sleep(200);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (msgWriteCount * 2) && !failTest);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;

	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	//RSSL_THREAD_KILL(&clientReadThread);
	//RSSL_THREAD_KILL(&serverReadThread);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before rsslCloseChannel. Writers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionA.pServerChannel, &err), RSSL_RET_SUCCESS) << "connA serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionB.pClientChannel, &err), RSSL_RET_SUCCESS) << "connB clientChannel close error: " << err.text;

	//std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join clientReadThread." << std::endl;
	RSSL_THREAD_JOIN(clientReadThread);
	//std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join serverReadThread." << std::endl;
	RSSL_THREAD_JOIN(serverReadThread);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before rsslCloseChannel. Readers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionB.pServerChannel, &err), RSSL_RET_SUCCESS) << "connB serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionA.pClientChannel, &err), RSSL_RET_SUCCESS) << "connA clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  FINISH." << std::endl;

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

TEST_P(GlobalLockChannelTestsFragmentedFixture, BlockingTwoWayClientServerTransport)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockFragmentedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_BLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Blocking connection - skipped.";
		return;
	}
#endif

	WriteChannelTransport serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, "SrvW");
	WriteChannelTransport clientWriteOpts(testParams.msgLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// serverChannelWr, clientChannelRd are connected by port 15000
	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel,
		&connectionA);

	// serverChannelRd, clientChannelWr are connected by port 15001
	startupServerAndConections(1, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel,
		&connectionB);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionB.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = msgWriteCount;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = msgWriteCount;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionB.pClientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWriteTransportThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWriteTransportThread, (void*)&clientWriteOpts);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Run test. msgWriteCount: " << msgWriteCount << std::endl;
	do
	{
		time_sleep(200);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (msgWriteCount * 2) && !failTest);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;
	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	//RSSL_THREAD_KILL(&clientReadThread);
	//RSSL_THREAD_KILL(&serverReadThread);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before rsslCloseChannel. Writers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionA.pServerChannel, &err), RSSL_RET_SUCCESS) << "connA serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionB.pClientChannel, &err), RSSL_RET_SUCCESS) << "connB clientChannel close error: " << err.text;

	//std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join clientReadThread." << std::endl;
	RSSL_THREAD_JOIN(clientReadThread);
	//std::cout << "GlobalLockChannelTestsFragmentedFixture  Before Join serverReadThread." << std::endl;
	RSSL_THREAD_JOIN(serverReadThread);

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  Before rsslCloseChannel. Readers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionB.pServerChannel, &err), RSSL_RET_SUCCESS) << "connB serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionA.pClientChannel, &err), RSSL_RET_SUCCESS) << "connA clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsFragmentedFixture  FINISH." << std::endl;

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

TEST_P(GlobalLockChannelTestsPackedFixture, NonBlockingTwoWayClientServerPacked)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockPackedTestParams& testParams = GetParam();
	RsslUInt32 packedBufferLength = getPackedBufferLength();

	WriteChannelPacked serverWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "SrvW");
	WriteChannelPacked clientWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// The expected number of messages that will be read
	int nReadMsgsExpect = calculateExpectedReadMessagesCount(false, msgWriteCount);

	// serverChannelWr, clientChannelRd are connected by port 15000
	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U,
		&connectionA);

	// serverChannelRd, clientChannelWr are connected by port 15001
	startupServerAndConections(1, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U,
		&connectionB);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionB.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = nReadMsgsExpect;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = nReadMsgsExpect;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionB.pClientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWritePackedThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWritePackedThread, (void*)&clientWriteOpts);

//	std::cout << "GlobalLockChannelTestsPackedFixture.NonBlocking  Run test. msgWriteCount: " << msgWriteCount << std::endl;

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE(checkLongTimeNoRead(readCountPrev, readCount)) << "Long time no read data. readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect * 2)
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (nReadMsgsExpect * 2) && !failTest);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2) << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;
	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before rsslCloseChannel. Writers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionA.pServerChannel, &err), RSSL_RET_SUCCESS) << "connA serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionB.pClientChannel, &err), RSSL_RET_SUCCESS) << "connB clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before Join Read-threads." << std::endl;
	RSSL_THREAD_JOIN(clientReadThread);
	RSSL_THREAD_JOIN(serverReadThread);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before rsslCloseChannel. Readers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionB.pServerChannel, &err), RSSL_RET_SUCCESS) << "connB serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionA.pClientChannel, &err), RSSL_RET_SUCCESS) << "connA clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsPackedFixture  FINISH." << std::endl;

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

TEST_P(GlobalLockChannelTestsPackedFixture, BlockingTwoWayClientServerPacked)
{
	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;

	const GLobalLockPackedTestParams& testParams = GetParam();
#if (RUN_TEST_WEBSOCKET_BLOCK == 0)
	if (testParams.connType == RSSL_CONN_TYPE_WEBSOCKET
		|| testParams.connType == RSSL_CONN_TYPE_ENCRYPTED && testParams.encryptedProtocol == RSSL_CONN_TYPE_WEBSOCKET)
	{
		FAIL() << "Test on WebSocket Blocking connection - skipped.";
		return;
	}
#endif
	RsslUInt32 packedBufferLength = getPackedBufferLength();

	WriteChannelPacked serverWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "SrvW");
	WriteChannelPacked clientWriteOpts(testParams.msgLength, packedBufferLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);
	int writeCountSrvW = 0;
	int writeCountCliW = 0;

	// The expected number of messages that will be read
	int nReadMsgsExpect = calculateExpectedReadMessagesCount(true, msgWriteCount);

	// serverChannelWr, clientChannelRd are connected by port 15000
	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U,
		&connectionA);

	// serverChannelRd, clientChannelWr are connected by port 15001
	startupServerAndConections(1, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		RSSL_COMP_NONE, 0U,
		&connectionB);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionB.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = nReadMsgsExpect;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = nReadMsgsExpect;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionB.pClientChannel;

	// TestHandler checks the data buffer on Read (server and client) side
	SimpleBufferChecker checkerSrvR, checkerCliR;
	serverReadOpts.pSystemTestHandler = &checkerSrvR; // should have handleDataBuffer
	clientReadOpts.pSystemTestHandler = &checkerCliR;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWritePackedThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWritePackedThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		writeCountSrvW = serverWriteOpts.writeCount.load();
		writeCountCliW = clientWriteOpts.writeCount.load();

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2)
			<< " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")";
	} while (readCount < (nReadMsgsExpect * 2) && !failTest);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << " nReadMsgsExpect*2: " << (nReadMsgsExpect*2) << " writeCounts: " << (writeCountSrvW + writeCountCliW) << " (" << writeCountSrvW << ", " << writeCountCliW << ")" << std::endl;
	shutdownTest = true;
	time_sleep(50);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before Join Write-threads." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	//RSSL_THREAD_KILL(&clientReadThread);
	//RSSL_THREAD_KILL(&serverReadThread);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before rsslCloseChannel. Writers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionA.pServerChannel, &err), RSSL_RET_SUCCESS) << "connA serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionB.pClientChannel, &err), RSSL_RET_SUCCESS) << "connB clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before Join Read-threads." << std::endl;
	RSSL_THREAD_JOIN(clientReadThread);
	RSSL_THREAD_JOIN(serverReadThread);

//	std::cout << "GlobalLockChannelTestsPackedFixture  Before rsslCloseChannel. Readers side." << std::endl;
	EXPECT_EQ(rsslCloseChannel(connectionB.pServerChannel, &err), RSSL_RET_SUCCESS) << "connB serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(connectionA.pClientChannel, &err), RSSL_RET_SUCCESS) << "connA clientChannel close error: " << err.text;

//	std::cout << "GlobalLockChannelTestsPackedFixture  FINISH." << std::endl;

	if (failTest)
	{
		checkerSrvR.dumpLast100Indices("serverRead");
		checkerCliR.dumpLast100Indices("clientRead");
	}
	if (maxTimeWait > 200)
		std::cout << "maxTimeWait: " << maxTimeWait << " ms" << std::endl;

	ASSERT_FALSE(failTest) << "Test failed. readCount: " << msgsRead
		<< ", lastSelectCount: " << serverReadOpts.selectCount << ", " << clientReadOpts.selectCount
		<< ", writeCount [" << serverWriteOpts.channelTitle << "]: " << serverWriteOpts.writeCount
		<< ", [" << clientWriteOpts.channelTitle << "]: " << clientWriteOpts.writeCount;
}

//GLobalLockFragmentedTestParams(
//	RsslConnectionTypes cnType, RsslUInt32 msgLen, RsslUInt32 maxFragSz, RsslUInt32 msgCount,
//	RsslConnectionTypes encrProt, RsslUInt8 wsProt)

INSTANTIATE_TEST_SUITE_P(
	MsgLength,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 0,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 0, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz3000,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 3000, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz38750,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,   100, 38750,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  1250, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  5999, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6135, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6145, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET,  6150, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_SOCKET, 12263, 38750, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrypted,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 0,     0, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWF,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSON,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5999, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockRWF,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5999, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockJSON,
	GlobalLockChannelTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5999, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);


INSTANTIATE_TEST_SUITE_P(
	MsgLength,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 5999, 6000, 0, 20000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrypted,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5999, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWF,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSON,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 0,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockRWF,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthEncrWebSockJSON,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED,  100, 6000, 0,     0, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 1250, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_ENCRYPTED, 5993, 6000, 0, 20000, RSSL_CONN_TYPE_WEBSOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 3000
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz3000,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 1250, 3000, 3000, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 2998, 3000, 3000, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz3000,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 2998, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz3000,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 3000, 3000,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 3000, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

// maxFragmentSize = 38750
INSTANTIATE_TEST_SUITE_P(
	MsgLengthMSz38750,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET,  100, 6000, 38750,     0, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 1250, 6000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_SOCKET, 5999, 6000, 38750, 40000, RSSL_CONN_TYPE_INIT, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockRWFMSz38750,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 38750,     0, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5999, 6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONMSz38750,
	GlobalLockChannelTestsPackedFixture,
	::testing::Values(
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  100, 6000, 38750,     0, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 1250, 6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockPackedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 5999, 6000, 38750, 20000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

class SystemTests : public ::testing::Test {
protected:
	// one-way client-server connections
	TUConnection connectionA;  // Server-channel for writing : Client-channel for reading

	int msgsRead;
	RsslMutex readLock;       // Protect access to the read counter: readCount
	RsslMutex readwriteLock;  // Protect access to whole read section, if need

	EventSignal eventSignalRW;	// The signal sends from Read-loop to Write-loop when it is ready for reading next chunk. Uses in step-by-step tests.

	RsslTimeValue startTime;
	RsslTimeValue curTime;

	virtual void SetUp()
	{
		RsslError err;

		shutdownTest = false;
		failTest = false;

		clearTUConnection(&connectionA);

		RSSL_MUTEX_INIT(&readLock);
		RSSL_MUTEX_INIT(&readwriteLock);

		msgsRead = 0;

		startTime = 0;
		curTime = 0;

		rsslInitialize(RSSL_LOCK_GLOBAL, &err);

		eventSignalRW.init();
	}

	virtual void TearDown()
	{
		RsslError err;
		rsslCloseServer(connectionA.pServer, &err);

		rsslUninitialize();

		clearTUConnection(&connectionA);

		RSSL_MUTEX_DESTROY(&readwriteLock);
		RSSL_MUTEX_DESTROY(&readLock);
		resetDeadlockTimer();
	}

	void startupServerAndConections(
		int configIndex,
		RsslBool blocking,
		RsslUInt32 maxFragmentSize,
		RsslConnectionTypes connType,
		RsslConnectionTypes encryptedProtocol,
		RsslUInt8 wsProtocolType,
		RsslCompTypes compressType,
		RsslUInt32 compressLevel,
		TUConnection* pConnection
	)
	{
		RsslThreadId serverThread, clientThread;
		ClientChannel clientChnl;
		ServerChannel serverChnl;
		RsslServer* server;
		serverChnl.pThreadId = &serverThread;
		clientChnl.pThreadId = &clientThread;

		TUServerConfig serverConfig;
		constructTUServerConfig(configIndex, serverConfig, blocking, maxFragmentSize, connType, compressType, compressLevel);

		server = bindRsslServer(&serverConfig);

		ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";

		serverChnl.pServer = server;
		serverChnl.setTUServerConfig(&serverConfig);

		TUClientConfig clientConfig;
		constructTUClientConfig(configIndex, clientConfig, blocking, connType, encryptedProtocol, wsProtocolType, compressType);
		clientChnl.setTUClientConfig(&clientConfig);

		if (blocking == RSSL_FALSE)
		{
			RSSL_THREAD_START(&serverThread, nonBlockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, nonBlockingClientConnectThread, &clientChnl);
		}
		else
		{
			RSSL_THREAD_START(&serverThread, blockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, blockingClientConnectThread, &clientChnl);
		}

		RSSL_THREAD_JOIN(serverThread);
		RSSL_THREAD_JOIN(clientThread);

		RSSL_THREAD_DETACH(&serverThread);
		RSSL_THREAD_DETACH(&clientThread);

		pConnection->pServer = server;
		pConnection->pServerChannel = serverChnl.pChnl;
		pConnection->pClientChannel = clientChnl.pChnl;

		if (!(pConnection->pServerChannel) || !(pConnection->pClientChannel)
			|| (pConnection->pServerChannel)->state != RSSL_CH_STATE_ACTIVE || (pConnection->pClientChannel)->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
	}

	bool checkLongTimeNoRead(int& readCountPrev, int readCount)
	{
		bool isCheckLongTimeNoRead = false;
		if (readCountPrev < readCount)
		{
			readCountPrev = readCount;
			startTime = 0;
		}
		else
		{
			curTime = rsslGetTimeMilli();
			if (startTime == 0)
			{
				startTime = curTime;
			}
			else if ( (curTime - startTime) > MAX_TIMEWAIT_NOUPDATES_MS )
			{
				failTest = true;
				isCheckLongTimeNoRead = true;
			}
		}
		return isCheckLongTimeNoRead;
	}
};

class SystemTestsFixture
	: public SystemTests, public ::testing::WithParamInterface<GLobalLockSystemTestParams>
{
};


TEST_P(SystemTestsFixture, NonBlockingClientServer)
{
	RsslThreadId serverWriteThread, clientReadThread;
	ReadChannel clientReadOpts;

	const GLobalLockSystemTestParams& testParams = GetParam();

	RsslMutex* pReadWriteLock = NULL;
	EventSignal* pReadWriteSignal = NULL;
	if (1 <= testParams.configIndex && testParams.configIndex <= 6		// inpbuf_01.bin WebSocket + JSON
		|| 7 <= testParams.configIndex && testParams.configIndex <= 12	// inpbuf_ws_rwf_01.bin WebSocket + RWF
		)
	{   // all the tests 1 .. 6, 7 .. 12 are: step-by-step
		// i.e. after each send the test waits for read is completed
		// and only then sends a next chunk 
		pReadWriteLock = &readwriteLock;
		pReadWriteSignal = &eventSignalRW;
	}

	WriteChannelSystem serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, testParams.configIndex, pReadWriteLock, pReadWriteSignal);

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel, &connectionA);

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.lockReadWrite = pReadWriteLock;
	clientReadOpts.signalForWrite = pReadWriteSignal;

	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteSystemThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount) && !failTest);

	shutdownTest = true;
	//std::cout << "SystemTestsFragmentedFixture  Loop finished. failTest: " << failTest << " readCount/writeCount: " << readCount << "/" << msgWriteCount << std::endl;

	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	rsslCloseChannel(connectionA.pServerChannel, &err);
	rsslCloseChannel(connectionA.pClientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientReadThread);
	}

	ASSERT_FALSE(failTest) << "Test failed.";
}

INSTANTIATE_TEST_SUITE_P(
	MsgLengthWebSockJSONFile,
	SystemTestsFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 2000000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	ReadWriteStepsWebSockJSON,
	SystemTestsFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 1, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 2, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 3, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 4, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 5, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 6, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	ReadWriteStepsWebSockRWF,
	SystemTestsFixture,
	::testing::Values(
		GLobalLockSystemTestParams(  7, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams(  8, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams(  9, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 10, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 11, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 12, RSSL_CONN_TYPE_WEBSOCKET, 526, 0, 20, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

class SystemTestsSendBuffersFixture
	: public SystemTests, public ::testing::WithParamInterface<GLobalLockSystemTestParams>
{
};

TEST_P(SystemTestsSendBuffersFixture, BlockingClientSendsMessagesAndFillFullInternalBuffer)
{
	RsslThreadId serverReadThread, clientWriteThread;

	const GLobalLockSystemTestParams& testParams = GetParam();

	// Synchronization objects for step-by-step test
	// i.e. after send several buffers the test waits for read is completed
	// and only then sends a next chunk
	RsslMutex* pReadWriteLock = &readwriteLock;
	EventSignal* pReadWriteSignal = &eventSignalRW;

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	WriteChannelSeveralBufferTransport
		clientWriteOpts(testParams.msgLength, msgWriteCount,testParams.configIndex, "CliW", pReadWriteLock, pReadWriteSignal);

	ReadChannel serverReadOpts;

	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel, &connectionA);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionA.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.expectedReadCount = msgWriteCount;
	serverReadOpts.lockReadWrite = pReadWriteLock;
	serverReadOpts.signalForWrite = pReadWriteSignal;

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionA.pClientChannel;

	RsslChannelInfo readChannelInfo;

	ASSERT_EQ(rsslGetChannelInfo(connectionA.pServerChannel, &readChannelInfo, &err), RSSL_RET_SUCCESS) << "rsslGetChannelInfo returned error.";
	clientWriteOpts.readChannelInfo = &readChannelInfo;

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);

	RSSL_THREAD_START(&clientWriteThread, blockingWriteSeveralBufferTransportThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);

		//EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount) && !failTest);

	shutdownTest = true;
	//std::cout << "SystemTestsFrameFixture  Loop finished. failTest: " << failTest << " readCount/writeCount: " << readCount << "/" << msgWriteCount << std::endl;

	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
	}

	rsslCloseChannel(connectionA.pServerChannel, &err);
	rsslCloseChannel(connectionA.pClientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&serverReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(serverReadThread);
	}

	ASSERT_FALSE(failTest) << "Test failed.";
}

TEST_P(SystemTestsSendBuffersFixture, NonBlockingClientSendsMessagesAndFillFullInternalBuffer)
{
	RsslThreadId serverReadThread, clientWriteThread;

	const GLobalLockSystemTestParams& testParams = GetParam();

	// Synchronization objects for step-by-step test
	// i.e. after send several buffers the test waits for read is completed
	// and only then sends a next chunk
	RsslMutex* pReadWriteLock = &readwriteLock;
	EventSignal* pReadWriteSignal = &eventSignalRW;

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	WriteChannelSeveralBufferTransport
		clientWriteOpts(testParams.msgLength, msgWriteCount, testParams.configIndex, "CliW", pReadWriteLock, pReadWriteSignal);

	ReadChannel serverReadOpts;

	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel, &connectionA);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = connectionA.pServerChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverReadOpts.lockReadWrite = pReadWriteLock;
	serverReadOpts.signalForWrite = pReadWriteSignal;

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = connectionA.pClientChannel;

	RsslChannelInfo readChannelInfo;

	ASSERT_EQ(rsslGetChannelInfo(connectionA.pServerChannel, &readChannelInfo, &err), RSSL_RET_SUCCESS) << "rsslGetChannelInfo returned error.";
	clientWriteOpts.readChannelInfo = &readChannelInfo;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);

	RSSL_THREAD_START(&clientWriteThread, blockingWriteSeveralBufferTransportThread, (void*)&clientWriteOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);

		//EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount) && !failTest);

	shutdownTest = true;
	//std::cout << "SystemTestsFrameFixture  Loop finished. failTest: " << failTest << " readCount/writeCount: " << readCount << "/" << msgWriteCount << std::endl;

	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientWriteThread);
	}

	rsslCloseChannel(connectionA.pServerChannel, &err);
	rsslCloseChannel(connectionA.pClientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&serverReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(serverReadThread);
	}

	ASSERT_FALSE(failTest) << "Test failed.";
}

TEST_P(SystemTestsSendBuffersFixture, BlockingServerSendsMessagesAndFillFullInternalBuffer)
{
	RsslThreadId serverWriteThread, clientReadThread;

	const GLobalLockSystemTestParams& testParams = GetParam();

	// Synchronization objects for step-by-step test
	// i.e. after send several buffers the test waits for read is completed
	// and only then sends a next chunk
	RsslMutex* pReadWriteLock = &readwriteLock;
	EventSignal* pReadWriteSignal = &eventSignalRW;

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	WriteChannelSeveralBufferTransport
		serverWriteOpts(testParams.msgLength, msgWriteCount, testParams.configIndex, "SrvW", pReadWriteLock, pReadWriteSignal);

	ReadChannel clientReadOpts;

	startupServerAndConections(0, RSSL_TRUE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel, &connectionA);

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = msgWriteCount;
	clientReadOpts.lockReadWrite = pReadWriteLock;
	clientReadOpts.signalForWrite = pReadWriteSignal;

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	RsslChannelInfo readChannelInfo;

	ASSERT_EQ(rsslGetChannelInfo(connectionA.pServerChannel, &readChannelInfo, &err), RSSL_RET_SUCCESS) << "rsslGetChannelInfo returned error.";
	serverWriteOpts.readChannelInfo = &readChannelInfo;

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);

	RSSL_THREAD_START(&serverWriteThread, blockingWriteSeveralBufferTransportThread, (void*)&serverWriteOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount) && !failTest);

	shutdownTest = true;
//	std::cout << "SystemTestsSendBuffersFixture  Loop finished. failTest: " << failTest << " readCount/writeCount: " << readCount << "/" << msgWriteCount << std::endl;

	time_sleep(50);

//	std::cout << "SystemTestsSendBuffersFixture  Before Join Write-thread." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(serverWriteThread);
	}

//	std::cout << "SystemTestsSendBuffersFixture  Before rsslCloseChannel. Writer side." << std::endl;
	rsslCloseChannel(connectionA.pServerChannel, &err);
//	std::cout << "SystemTestsSendBuffersFixture  Before rsslCloseChannel. Reader side." << std::endl;
	rsslCloseChannel(connectionA.pClientChannel, &err);

//	std::cout << "SystemTestsSendBuffersFixture  Before Join Read-thread." << std::endl;
	if (failTest)
	{
		RSSL_THREAD_KILL(&clientReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientReadThread);
	}

	ASSERT_FALSE(failTest) << "Test failed.";
}

INSTANTIATE_TEST_SUITE_P(
	WebSockJSONClients,
	SystemTestsSendBuffersFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 24, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 1, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 28, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_SUITE_P(
	WebSockRWFClients,
	SystemTestsSendBuffersFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 2, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 36, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 3, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 14, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

class TestEncrDecrFixture
	: public SystemTests, public TestHandler, public ::testing::WithParamInterface<GLobalLockSystemTestParams>
{
private:
	char text[SZ_Text];
public:
	bool handleDataBuffer(RsslBuffer* pReadBuffer, char* pErrorText, size_t szErrorText)
	{
		RsslBuffer decryptedBuffer;
		RsslError error;

		decryptedBuffer.data = text;
		decryptedBuffer.length = SZ_Text;

		// Decrypt the buffer
		RsslRet ret = rsslDecryptBuffer(connectionA.pClientChannel, pReadBuffer, &decryptedBuffer, &error);
		if (ret != RSSL_RET_SUCCESS)
		{
			if (pErrorText != NULL && szErrorText > 0)
				snprintf(pErrorText, szErrorText, "rsslDecryptBuffer failed. ret: %d. Error: %d. %s",
					ret, error.rsslErrorId, error.text);
			return false;
		}

		// Check the text
		char* p = decryptedBuffer.data;
		RsslUInt32 buflen = decryptedBuffer.length;
		unsigned k = 0;

		int ind;
		const char* testBuffer = "TestDataInfo";
		size_t lenTestBuffer = strlen(testBuffer);
		size_t len;

		bool isTextCorrect = false;

		do {
			if (*p != '[')
				break;
			while (k < buflen && *(p + k) == '[')
				k++;
			if (k >= buflen)
				break;

			if (!isdigit(*(p+k)))
				break;

			ind = atoi(p+k);
			while (k < buflen && *(p + k) != ':')
				k++;

			k++;  // skip ':'
			if (k >= buflen)
				break;

			if (buflen - k > lenTestBuffer)
				len = lenTestBuffer;
			else
				len = buflen - k;

			if (strncmp(p + k, testBuffer, len) != 0)
				break;

			isTextCorrect = true;
		} while (false);

		return isTextCorrect;
	}
};

TEST_P(TestEncrDecrFixture, SendReceiveMessages)
{
	RsslThreadId clientReadThread, serverWriteThread;

	const GLobalLockSystemTestParams& testParams = GetParam();

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	WriteChannelEncryptBufferTransport
		serverWriteOpts(testParams.msgLength, msgWriteCount, "SrvW", NULL, NULL);

	ReadChannel clientReadOpts;

	startupServerAndConections(0, RSSL_FALSE,
		testParams.maxFragmentSize,
		testParams.connType, testParams.encryptedProtocol, testParams.wsProtocolType,
		testParams.compressionType, testParams.compressionLevel, &connectionA);

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = connectionA.pClientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientReadOpts.expectedReadCount = msgWriteCount;
	clientReadOpts.pSystemTestHandler = this; // should have handleDataBuffer

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = connectionA.pServerChannel;

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);

	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteEncryptBufferTransportThread, (void*)&serverWriteOpts);

	do
	{
		time_sleep(50);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);

		EXPECT_FALSE( checkLongTimeNoRead(readCountPrev, readCount) ) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount) && !failTest);

	shutdownTest = true;
//	std::cout << "TestEncrDecrFixture  Loop finished. failTest: " << failTest << " readCount/writeCount: " << readCount << "/" << msgWriteCount << std::endl;

	time_sleep(50);

	if (failTest)
	{
		RSSL_THREAD_KILL(&serverWriteThread);
	}
	else
	{
		RSSL_THREAD_JOIN(serverWriteThread);
	}

	rsslCloseChannel(connectionA.pServerChannel, &err);
	rsslCloseChannel(connectionA.pClientChannel, &err);

	if (failTest)
	{
		RSSL_THREAD_KILL(&clientReadThread);
	}
	else
	{
		RSSL_THREAD_JOIN(clientReadThread);
	}

	ASSERT_FALSE(failTest) << "Test failed.";
}

INSTANTIATE_TEST_SUITE_P(
	SocketClient,
	TestEncrDecrFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_SOCKET, 3000,  100, 1024, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_SOCKET, 3000, 3000, 1024, RSSL_CONN_TYPE_SOCKET, 0 ),
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_SOCKET, 3000,    0, 1024, RSSL_CONN_TYPE_SOCKET, 0 )
	)
);

INSTANTIATE_TEST_SUITE_P(
	WebSockRWFClient,
	TestEncrDecrFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 3000,  100, 1024, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 1024, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 3000,    0, 1024, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);


class AllLockTests : public ::testing::Test {
protected:
	RsslChannel* serverChannel;
	RsslChannel* clientChannel;
	RsslServer* server;
	int msgsRead;
	RsslMutex readLock;
	RsslError err;

	virtual void SetUp()
	{
		shutdownTest = false;
		failTest = false;
		server = NULL;
		serverChannel = NULL;
		clientChannel = NULL;

		RSSL_MUTEX_INIT(&readLock);

		msgsRead = 0;

		rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);
	}

	virtual void TearDown()
	{
		rsslCloseServer(server, &err);
		rsslUninitialize();
		server = NULL;
		serverChannel = NULL;
		clientChannel = NULL;
		RSSL_MUTEX_DESTROY(&readLock);
		resetDeadlockTimer();
	}

	void startupServerAndConections(RsslBool blocking)
	{
		RsslThreadId serverThread, clientThread;
		ClientChannel clientOpts;
		ServerChannel serverChnl;
		serverChnl.pThreadId = &serverThread;
		clientOpts.pThreadId = &clientThread;

		server = startupServer(blocking);
		ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";
		serverChnl.pServer = server;
		if (blocking == RSSL_FALSE)
		{
			RSSL_THREAD_START(&serverThread, nonBlockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, nonBlockingClientConnectThread, &clientOpts);
		}
		else
		{
			RSSL_THREAD_START(&serverThread, blockingServerConnectThread, &serverChnl);
			RSSL_THREAD_START(&clientThread, blockingClientConnectThread, &clientOpts);
		}

		RSSL_THREAD_JOIN(serverThread);
		RSSL_THREAD_JOIN(clientThread);

		serverChannel = serverChnl.pChnl;
		clientChannel = clientOpts.pChnl;

		if (!serverChannel || !clientChannel || serverChannel->state != RSSL_CH_STATE_ACTIVE || clientChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
	}
};

/*	Test kicks off one writer and one reader thread for the client and server each.  Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish. */
TEST_F(AllLockTests, NonBlockingTwoWayClientServer)
{

	RsslThreadId serverReadThread, serverWriteThread, clientReadThread, clientWriteThread;
	ReadChannel serverReadOpts, clientReadOpts;
	WriteChannel serverWriteOpts, clientWriteOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);
	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);
	
	ASSERT_FALSE(failTest) << "Test Failed!";
	
	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(serverWriteThread);
	RSSL_THREAD_JOIN(clientReadThread);
	RSSL_THREAD_JOIN(serverReadThread);

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

}

/*	Test kicks off one writer and one reader thread for the client and server each. Also 
	starts a thread for pinging both the client and server connections.  Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish. */
TEST_F(AllLockTests, NonBlockingTwoWayClientServerPing)
{

	RsslThreadId serverReadThread, serverWriteThread, serverPingThread, clientReadThread, clientWriteThread, clientPingThread;
	ReadChannel serverReadOpts, clientReadOpts;
	WriteChannel serverWriteOpts, clientWriteOpts;
	PingChannel serverPingOpts, clientPingOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverReadOpts.pThreadId = &serverReadThread;
	serverReadOpts.pChnl = serverChannel;
	serverReadOpts.readCount = &msgsRead;
	serverReadOpts.lock = &readLock;
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;
	serverPingOpts.pThreadId = &serverPingThread;
	serverPingOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;
	clientPingOpts.pThreadId = &clientPingThread;
	clientPingOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread, nonBlockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteThread, (void*)&serverWriteOpts);
	RSSL_THREAD_START(&serverPingThread, pingThread, (void*)&serverPingOpts);

	RSSL_THREAD_START(&clientReadThread, nonBlockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);
	RSSL_THREAD_START(&clientPingThread, pingThread, (void*)&clientPingOpts);
	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test Failed!";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(serverWriteThread);
	RSSL_THREAD_JOIN(serverPingThread);
	RSSL_THREAD_JOIN(clientPingThread);
	RSSL_THREAD_JOIN(clientReadThread);
	RSSL_THREAD_JOIN(serverReadThread);

	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

}

/*	Test kicks off two writer threads on the client and two reader threads on the server. Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish. */
TEST_F(AllLockTests, NonBlockingClientTwoWriterServerTwoReader)
{
	RsslThreadId serverReadThread1, serverReadThread2, clientWriteThread1, clientWriteThread2;
	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts1, clientWriteOpts2;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	clientWriteOpts1.pThreadId = &clientWriteThread1;
	clientWriteOpts1.pChnl = clientChannel;
	clientWriteOpts2.pThreadId = &clientWriteThread2;
	clientWriteOpts2.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread1, nonBlockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, nonBlockingReadThread, (void*)&serverReadOpts2);

	RSSL_THREAD_START(&clientWriteThread1, nonBlockingWriteThread, (void*)&clientWriteOpts1);
	RSSL_THREAD_START(&clientWriteThread2, nonBlockingWriteThread, (void*)&clientWriteOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";
	
	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread1);
	RSSL_THREAD_JOIN(clientWriteThread2);

	rsslCloseChannel(clientChannel, &err);
	
	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

/*  Test runs two writer threads on the server, and two reader threads on the client. Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientTwoReaderServerTwoWriter)
{
	RsslThreadId serverWriteThread1, serverWriteThread2, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts1, serverWriteOpts2;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverWriteOpts1.pThreadId = &serverWriteThread1;
	serverWriteOpts1.pChnl = serverChannel;
	serverWriteOpts2.pThreadId = &serverWriteThread2;
	serverWriteOpts2.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread1, nonBlockingWriteThread, (void*)&serverWriteOpts1);
	RSSL_THREAD_START(&serverWriteThread2, nonBlockingWriteThread, (void*)&serverWriteOpts2);

	RSSL_THREAD_START(&clientReadThread1, nonBlockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, nonBlockingReadThread, (void*)&clientReadOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";
	
	shutdownTest = true;
	
	rsslCloseChannel(serverChannel, &err);
	
	RSSL_THREAD_JOIN(serverWriteThread1);
	RSSL_THREAD_JOIN(serverWriteThread2);
	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Test runs two writer threads on the server, and two reader threads on the client, and 
	a pinging thread on the client channel. Once the writer threads have each written their 
	full data, sets the shutdown boolean and waits for the reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientTwoWriterServerTwoReaderPing)
{
	RsslThreadId serverReadThread1, serverReadThread2, clientWriteThread1, clientWriteThread2, clientPingThread;
	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts1, clientWriteOpts2;
	PingChannel clientPingOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	clientWriteOpts1.pThreadId = &clientWriteThread1;
	clientWriteOpts1.pChnl = clientChannel;
	clientWriteOpts2.pThreadId = &clientWriteThread2;
	clientWriteOpts2.pChnl = clientChannel;
	clientPingOpts.pThreadId = &clientPingThread;
	clientPingOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread1, nonBlockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, nonBlockingReadThread, (void*)&serverReadOpts2);

	RSSL_THREAD_START(&clientWriteThread1, nonBlockingWriteThread, (void*)&clientWriteOpts1);
	RSSL_THREAD_START(&clientWriteThread2, nonBlockingWriteThread, (void*)&clientWriteOpts2);
	RSSL_THREAD_START(&clientPingThread, pingThread, (void*)&clientPingOpts);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread1);
	RSSL_THREAD_JOIN(clientWriteThread2);
	RSSL_THREAD_JOIN(clientPingThread);

	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

/*  Test runs two reader threads on the server, and two writer threads on the client, and a pinging thread
	on the server channel. Once the writer threads have each written their full data, sets 
	the shutdown boolean and waits for the reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientTwoReaderServerTwoWriterPing)
{
	RsslThreadId serverWriteThread1, serverWriteThread2, serverPingThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts1, serverWriteOpts2;
	PingChannel serverPingOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverWriteOpts1.pThreadId = &serverWriteThread1;
	serverWriteOpts1.pChnl = serverChannel;
	serverWriteOpts2.pThreadId = &serverWriteThread2;
	serverWriteOpts2.pChnl = serverChannel;
	serverPingOpts.pThreadId = &serverPingThread;
	serverPingOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread1, nonBlockingWriteThread, (void*)&serverWriteOpts1);
	RSSL_THREAD_START(&serverWriteThread2, nonBlockingWriteThread, (void*)&serverWriteOpts2);
	RSSL_THREAD_START(&serverPingThread, pingThread, (void*)&serverPingOpts);

	RSSL_THREAD_START(&clientReadThread1, nonBlockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, nonBlockingReadThread, (void*)&clientReadOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(serverWriteThread1);
	RSSL_THREAD_JOIN(serverWriteThread2);
	RSSL_THREAD_JOIN(serverPingThread);
	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Multiple reader client threads from a single writer server. Once the writer threads 
	have each written their full data, sets the shutdown boolean and waits for the 
	reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientTwoReaderServerWriter)
{
	RsslThreadId serverWriteThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread1, nonBlockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, nonBlockingReadThread, (void*)&clientReadOpts1);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";
	
	shutdownTest = true;

	RSSL_THREAD_JOIN(serverWriteThread);

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/* Multiple reader server threads from a single writer client. Once the writer threads 
	have each written their full data, sets the shutdown boolean and waits for the 
	reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientWriterServerTwoReader)
{
	RsslThreadId clientWriteThread, serverReadThread1, serverReadThread2;

	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);

	RSSL_THREAD_START(&serverReadThread1, nonBlockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, nonBlockingReadThread, (void*)&serverReadOpts2);


	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";
	
	shutdownTest = true;
	
	RSSL_THREAD_JOIN(clientWriteThread);

	rsslCloseChannel(clientChannel, &err);
	
	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

/*  Multiple reader client threads from a single writer server, and with a pinging thread.
	Once the writer threads have each written their full data, sets the shutdown boolean 
	and waits for the reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientTwoReaderServerWriterPing)
{
	RsslThreadId serverWriteThread, serverPingThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts;
	PingChannel serverPingOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;
	serverPingOpts.pThreadId = &serverPingThread;
	serverPingOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread, nonBlockingWriteThread, (void*)&serverWriteOpts);
	RSSL_THREAD_START(&serverPingThread, pingThread, (void*)&serverPingOpts);

	RSSL_THREAD_START(&clientReadThread1, nonBlockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, nonBlockingReadThread, (void*)&clientReadOpts1);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(serverWriteThread);
	RSSL_THREAD_JOIN(serverPingThread);

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Multiple reader server threads from a single writer client, and with a pinging thread.
	Once the writer threads have each written their full data, sets the shutdown boolean
	and waits for the reader threads to finish.*/
TEST_F(AllLockTests, NonBlockingClientWriterServerTwoReaderPing)
{
	RsslThreadId clientWriteThread, clientPingThread, serverReadThread1, serverReadThread2;


	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts;
	PingChannel clientPingOpts;
	int readCount;

	startupServerAndConections(RSSL_FALSE);

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;
	clientPingOpts.pThreadId = &clientPingThread;
	clientPingOpts.pChnl = clientChannel;

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);
	RSSL_THREAD_START(&clientPingThread, pingThread, (void*)&clientPingOpts);

	RSSL_THREAD_START(&serverReadThread1, nonBlockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, nonBlockingReadThread, (void*)&serverReadOpts2);


	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(clientPingThread);

	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

/*	Test kicks off one writer and one reader thread for the client and server. Also
	starts a thread for pinging both the client and server connections.  Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish. */
TEST_F(AllLockTests, BlockingClientTwoWriterServerTwoReader)
{
	RsslThreadId serverReadThread1, serverReadThread2, clientWriteThread1, clientWriteThread2;

	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts1, clientWriteOpts2;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	clientWriteOpts1.pThreadId = &clientWriteThread1;
	clientWriteOpts1.pChnl = clientChannel;
	clientWriteOpts2.pThreadId = &clientWriteThread2;
	clientWriteOpts2.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread1, blockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, blockingReadThread, (void*)&serverReadOpts2);

	RSSL_THREAD_START(&clientWriteThread1, blockingWriteThread, (void*)&clientWriteOpts1);
	RSSL_THREAD_START(&clientWriteThread2, blockingWriteThread, (void*)&clientWriteOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread1);
	RSSL_THREAD_JOIN(clientWriteThread2);

	rsslCloseChannel(clientChannel, &err);
	
	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);
}

/*	Test kicks off two writer threads on the client and two reader threads on the server. Once
	the writer threads have each written their full data, sets the shutdown boolean and
	waits for the reader threads to finish. */
TEST_F(AllLockTests, BlockingClientTwoReaderServerTwoWriter)
{
	RsslThreadId serverWriteThread1, serverWriteThread2, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts1, serverWriteOpts2;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverWriteOpts1.pThreadId = &serverWriteThread1;
	serverWriteOpts1.pChnl = serverChannel;
	serverWriteOpts2.pThreadId = &serverWriteThread2;
	serverWriteOpts2.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread1, blockingWriteThread, (void*)&serverWriteOpts1);
	RSSL_THREAD_START(&serverWriteThread2, blockingWriteThread, (void*)&serverWriteOpts2);

	RSSL_THREAD_START(&clientReadThread1, blockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, blockingReadThread, (void*)&clientReadOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;
	
	RSSL_THREAD_JOIN(serverWriteThread1);
	RSSL_THREAD_JOIN(serverWriteThread2);

	rsslCloseChannel(serverChannel, &err);
	
	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Test runs two writer threads on the server, and two reader threads on the client, and
	a pinging thread on the server channel. Once the writer threads have each written their
	full data, sets the shutdown boolean and waits for the reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientTwoWriterServerTwoReaderPing)
{
	RsslThreadId serverReadThread1, serverReadThread2, clientWriteThread1, clientWriteThread2, clientPingThread;
	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts1, clientWriteOpts2;
	PingChannel clientPingOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	clientWriteOpts1.pThreadId = &clientWriteThread1;
	clientWriteOpts1.pChnl = clientChannel;
	clientWriteOpts2.pThreadId = &clientWriteThread2;
	clientWriteOpts2.pChnl = clientChannel;
	clientPingOpts.pThreadId = &clientPingThread;
	clientPingOpts.pChnl = clientChannel;

	RSSL_THREAD_START(&serverReadThread1, blockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, blockingReadThread, (void*)&serverReadOpts2);

	RSSL_THREAD_START(&clientWriteThread1, blockingWriteThread, (void*)&clientWriteOpts1);
	RSSL_THREAD_START(&clientWriteThread2, blockingWriteThread, (void*)&clientWriteOpts2);
	RSSL_THREAD_START(&clientPingThread, pingThread, (void*)&clientPingOpts);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread1);
	RSSL_THREAD_JOIN(clientWriteThread2);
	RSSL_THREAD_JOIN(clientPingThread);

	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

/*  Test runs two reader threads on the server, and two writer threads on the client, and a pinging thread
	on the client channel. Once the writer threads have each written their full data, sets
	the shutdown boolean and waits for the reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientTwoReaderServerTwoWriterPing)
{
	RsslThreadId serverWriteThread1, serverWriteThread2, serverPingThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts1, serverWriteOpts2;
	PingChannel serverPingOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverWriteOpts1.pThreadId = &serverWriteThread1;
	serverWriteOpts1.pChnl = serverChannel;
	serverWriteOpts2.pThreadId = &serverWriteThread2;
	serverWriteOpts2.pChnl = serverChannel;
	serverPingOpts.pThreadId = &serverPingThread;
	serverPingOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread1, blockingWriteThread, (void*)&serverWriteOpts1);
	RSSL_THREAD_START(&serverWriteThread2, blockingWriteThread, (void*)&serverWriteOpts2);
	RSSL_THREAD_START(&serverPingThread, pingThread, (void*)&serverPingOpts);

	RSSL_THREAD_START(&clientReadThread1, blockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, blockingReadThread, (void*)&clientReadOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2) && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(serverWriteThread1);
	RSSL_THREAD_JOIN(serverWriteThread2);
	RSSL_THREAD_JOIN(serverPingThread);
	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Multiple reader client threads from a single writer server. Once the writer threads
	have each written their full data, sets the shutdown boolean and waits for the
	reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientTwoReaderServerWriter)
{
	RsslThreadId serverWriteThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread, blockingWriteThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread1, blockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, blockingReadThread, (void*)&clientReadOpts1);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(serverWriteThread);

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Multiple reader server threads from a single writer client. Once the writer threads
	have each written their full data, sets the shutdown boolean and waits for the
	reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientWriterServerTwoReader)
{
	RsslThreadId clientWriteThread, serverReadThread1, serverReadThread2;

	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&clientWriteThread, blockingWriteThread, (void*)&clientWriteOpts);

	RSSL_THREAD_START(&serverReadThread1, blockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, blockingReadThread, (void*)&serverReadOpts2);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);
	
	ASSERT_FALSE(failTest) << "Test failed.";
	
	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread);

	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);
}

/*  Multiple reader client threads from a single writer server, and with a pinging thread.
	Once the writer threads have each written their full data, sets the shutdown boolean
	and waits for the reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientTwoReaderServerWriterPing)
{
	RsslThreadId serverWriteThread, serverPingThread, clientReadThread1, clientReadThread2;

	ReadChannel clientReadOpts1, clientReadOpts2;
	WriteChannel serverWriteOpts;
	PingChannel serverPingOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;
	serverPingOpts.pThreadId = &serverPingThread;
	serverPingOpts.pChnl = serverChannel;

	clientReadOpts1.pThreadId = &clientReadThread1;
	clientReadOpts1.pChnl = clientChannel;
	clientReadOpts1.readCount = &msgsRead;
	clientReadOpts1.lock = &readLock;
	clientReadOpts2.pThreadId = &clientReadThread2;
	clientReadOpts2.pChnl = clientChannel;
	clientReadOpts2.readCount = &msgsRead;
	clientReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&serverWriteThread, blockingWriteThread, (void*)&serverWriteOpts);
	RSSL_THREAD_START(&serverPingThread, pingThread, (void*)&serverPingOpts);

	RSSL_THREAD_START(&clientReadThread1, blockingReadThread, (void*)&clientReadOpts1);
	RSSL_THREAD_START(&clientReadThread2, blockingReadThread, (void*)&clientReadOpts1);

	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(serverWriteThread);
	RSSL_THREAD_JOIN(serverPingThread);

	rsslCloseChannel(serverChannel, &err);

	RSSL_THREAD_JOIN(clientReadThread1);
	RSSL_THREAD_JOIN(clientReadThread2);

	rsslCloseChannel(clientChannel, &err);
}

/*  Multiple reader server threads from a single writer client, and with a pinging thread.
	Once the writer threads have each written their full data, sets the shutdown boolean
	and waits for the reader threads to finish.*/
TEST_F(AllLockTests, BlockingClientWriterServerTwoReaderPing)
{
	RsslThreadId clientWriteThread, clientPingThread, serverReadThread1, serverReadThread2;


	ReadChannel serverReadOpts1, serverReadOpts2;
	WriteChannel clientWriteOpts;
	PingChannel clientPingOpts;
	int readCount;

	startupServerAndConections(RSSL_TRUE);

	clientWriteOpts.pThreadId = &clientWriteThread;
	clientWriteOpts.pChnl = clientChannel;
	clientPingOpts.pThreadId = &clientPingThread;
	clientPingOpts.pChnl = clientChannel;

	serverReadOpts1.pThreadId = &serverReadThread1;
	serverReadOpts1.pChnl = serverChannel;
	serverReadOpts1.readCount = &msgsRead;
	serverReadOpts1.lock = &readLock;
	serverReadOpts2.pThreadId = &serverReadThread2;
	serverReadOpts2.pChnl = serverChannel;
	serverReadOpts2.readCount = &msgsRead;
	serverReadOpts2.lock = &readLock;

	RSSL_THREAD_START(&clientWriteThread, nonBlockingWriteThread, (void*)&clientWriteOpts);
	RSSL_THREAD_START(&clientPingThread, pingThread, (void*)&clientPingOpts);

	RSSL_THREAD_START(&serverReadThread1, blockingReadThread, (void*)&serverReadOpts1);
	RSSL_THREAD_START(&serverReadThread2, blockingReadThread, (void*)&serverReadOpts2);


	do
	{
		time_sleep(500);
		RSSL_MUTEX_LOCK(&readLock);
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
	} while (readCount < MAX_MSG_WRITE_COUNT && !failTest);

	ASSERT_FALSE(failTest) << "Test failed.";

	shutdownTest = true;

	RSSL_THREAD_JOIN(clientWriteThread);
	RSSL_THREAD_JOIN(clientPingThread);

	rsslCloseChannel(clientChannel, &err);

	RSSL_THREAD_JOIN(serverReadThread1);
	RSSL_THREAD_JOIN(serverReadThread2);

	rsslCloseChannel(serverChannel, &err);

}

class ManyThreadConnectionTests : public ::testing::Test {
protected:
	RsslServer* server;
	int writeMsgs;
	RsslError err;
	bool threadsCreated;

	RsslThreadId clientThreads[MAX_THREADS];
	RsslThreadId serverThreads[MAX_THREADS];
	ManyThreadChannel clientOpts[MAX_THREADS];
	ManyThreadChannel serverOpts[MAX_THREADS];
	int threadReadCount[MAX_THREADS];
	RsslMutex readLock[MAX_THREADS];

	virtual void SetUp()
	{
		writeMsgs = 1;
		threadsCreated = false;
		shutdownTest = false;
		failTest = false;

		for (int i = 0; i < MAX_THREADS; ++i)
		{
			RSSL_MUTEX_INIT(&readLock[i]);
			threadReadCount[i] = 0;
		}
	}

	virtual void TearDown()
	{

		if(failTest && threadsCreated)
		{
			for (int i = 0; i < MAX_THREADS; i++)
			{
				RSSL_THREAD_KILL(&clientThreads[i]);
				RSSL_THREAD_KILL(&serverThreads[i]);
			}
		} 
		rsslUninitialize();
		server = NULL;
		resetDeadlockTimer();
	}
};

/*  Starts up a nonblocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.  
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, NonBlockingAllLockClientReadServerWrite)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	server = startupServer(RSSL_FALSE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].readCount = &(threadReadCount[i]);
		clientOpts[i].lock = &readLock[i];
		RSSL_THREAD_START(&clientThreads[i], nonBlockingManyChannelReadThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&serverThreads[i], nonBlockingManyChannelWriteThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}	
	
	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
	

}

/*  Starts up a nonblocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, NonBlockingAllLockClientWriteServerRead)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	server = startupServer(RSSL_FALSE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&clientThreads[i], nonBlockingManyChannelWriteThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].lock = &readLock[i];
		serverOpts[i].readCount = &(threadReadCount[i]);
		RSSL_THREAD_START(&serverThreads[i], nonBlockingManyChannelReadThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}

	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a nonblocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, NonBlockingGlobalLockClientReadServerWrite)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL, &err);

	server = startupServer(RSSL_FALSE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].readCount = &(threadReadCount[i]);
		clientOpts[i].lock = &readLock[i];
		RSSL_THREAD_START(&clientThreads[i], nonBlockingManyChannelReadThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&serverThreads[i], nonBlockingManyChannelWriteThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}

	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a nonblocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, NonBlockingGlobalLockClientWriteServerRead)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL, &err);

	server = startupServer(RSSL_FALSE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&clientThreads[i], nonBlockingManyChannelWriteThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].lock = &readLock[i];
		serverOpts[i].readCount = &(threadReadCount[i]);
		RSSL_THREAD_START(&serverThreads[i], nonBlockingManyChannelReadThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}

	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a blocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, BlockingAllLockClientReadServerWrite)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	server = startupServer(RSSL_TRUE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].readCount = &(threadReadCount[i]);
		clientOpts[i].lock = &readLock[i];
		RSSL_THREAD_START(&clientThreads[i], blockingManyChannelReadThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&serverThreads[i], blockingManyChannelWriteThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}

	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a blocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, BlockingAllLockClientWriteServerRead)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	server = startupServer(RSSL_TRUE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&clientThreads[i], blockingManyChannelWriteThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].lock = &readLock[i];
		serverOpts[i].readCount = &(threadReadCount[i]);
		RSSL_THREAD_START(&serverThreads[i], blockingManyChannelReadThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}

	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a blocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, BlockingGlobalLockClientReadServerWrite)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;
	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL, &err);

	server = startupServer(RSSL_TRUE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].readCount = &(threadReadCount[i]);
		clientOpts[i].lock = &readLock[i];
		RSSL_THREAD_START(&clientThreads[i], blockingManyChannelReadThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&serverThreads[i], blockingManyChannelWriteThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}
	
	ASSERT_FALSE(failTest) << "Test failed.";
	
	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/*  Starts up a blocking server, and then creates MAX_THREADS reader and MAX_THREADS writer threads.
	Each thread creates a new RsslChannel and either reads or sends a message, then quits. */
TEST_F(ManyThreadConnectionTests, BlockingGlobalLockClientWriteServerRead)
{

	RsslServer* server = NULL;
	int i;
	int readMax = 0;

	int threadsDone = 0;

	rsslInitialize(RSSL_LOCK_GLOBAL, &err);

	server = startupServer(RSSL_TRUE);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";


	for (i = 0; i < MAX_THREADS; i++)
	{
		clientOpts[i].pThreadId = &clientThreads[i];
		clientOpts[i].serverChannel = false;
		clientOpts[i].msgCount = writeMsgs;
		RSSL_THREAD_START(&clientThreads[i], blockingManyChannelWriteThread, (void*)&clientOpts[i]);

		serverOpts[i].pThreadId = &serverThreads[i];
		serverOpts[i].serverChannel = true;
		serverOpts[i].pServer = server;
		serverOpts[i].lock = &readLock[i];
		serverOpts[i].readCount = &(threadReadCount[i]);
		RSSL_THREAD_START(&serverThreads[i], blockingManyChannelReadThread, (void*)&serverOpts[i]);
	}

	threadsCreated = true;

	while (!shutdownTest && !failTest)
	{
		time_sleep(500);
		threadsDone = 0;
		for (i = 0; i < MAX_THREADS; i++)
		{
			RSSL_MUTEX_LOCK(&readLock[i]);
			if (threadReadCount[i] == writeMsgs)
				++threadsDone;
			RSSL_MUTEX_UNLOCK(&readLock[i]);
		}

		if (threadsDone == MAX_THREADS)
		{
			shutdownTest = true;
		}
	}
	
	ASSERT_FALSE(failTest) << "Test failed.";

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(serverThreads[i]);
	}

	for (i = 0; i < MAX_THREADS; i++)
	{
		RSSL_THREAD_JOIN(clientThreads[i]);
	}

	rsslCloseServer(server, &err);
}

/* Testing the host name validation functions */

TEST(OpenSSLHostNameValidation, HostNameValidation)
{
	char pattern[16];
	char hostName[16];
	RsslBool output = RSSL_TRUE;
	RsslError err;

	/* No wildcard success */
	strcpy(pattern, "www.foo.com");
	strcpy(hostName, "www.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_TRUE);

	/* No wildcard failure */
	strcpy(pattern, "www.foo.com");
	strcpy(hostName, "www.bar.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard success */
	strcpy(pattern, "*.foo.com");
	strcpy(hostName, "www.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_TRUE);

	/* Wildcard success, many tokens */
	strcpy(pattern, "*.foo.bar.com");
	strcpy(hostName, "www.foo.bar.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_TRUE);

	/* Wildcard failure */
	strcpy(pattern, "*.foo.com");
	strcpy(hostName, "www.bar.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard failure, additional tokens */
	strcpy(pattern, "*.foo.com");
	strcpy(hostName, "www.bar.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard failure, no leftmost token */
	strcpy(pattern, "*.foo.com");
	strcpy(hostName, "foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard failure, no leftmost token */
	strcpy(pattern, "*.foo.com");
	strcpy(hostName, ".foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard parse failure, too broad of a wildcard */
	strcpy(pattern, "*.com");
	strcpy(hostName, "foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard parse failure, partial wildcard */
	strcpy(pattern, "a*.foo.com");
	strcpy(hostName, "a.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard parse failure, partial wildcard */
	strcpy(pattern, "*a.foo.com");
	strcpy(hostName, "a.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard parse failure, partial wildcard */
	strcpy(pattern, "b*r.foo.com");
	strcpy(hostName, "bar.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	/* Wildcard parse failure, non-first wildcard */
	strcpy(pattern, "bar.*.com");
	strcpy(hostName, "bar.foo.com");
	output = ripcVerifyCertHost(pattern, (unsigned int)strlen(pattern), hostName, &err);
	EXPECT_TRUE(output == RSSL_FALSE);

	resetDeadlockTimer();
}


class BindSharedServerSocketOpt : public ::testing::Test {
protected:
	RsslServer* pServer;
	virtual void SetUp()
	{
		RsslError err;

		pServer = NULL;

		rsslInitialize(RSSL_LOCK_GLOBAL, &err);
	}

	virtual void TearDown()
	{
		RsslError err;
		if (pServer != NULL)
		{
			rsslCloseServer(pServer, &err);
		}
		rsslUninitialize();
		pServer = NULL;
		resetDeadlockTimer();
	}

	void runRsslBind(RsslBindOptions& bindOpts)
	{
		RsslError err;

		bindOpts.serviceName = (char*)"15000";
		bindOpts.protocolType = TEST_PROTOCOL_TYPE;  /* These tests are just sending a pre-set string across the wire, so protocol type should not be RWF */
		memset(&err, 0, sizeof(RsslError));

		pServer = rsslBind(&bindOpts, &err);

		ASSERT_NE(pServer, (RsslServer*)NULL) << "Server creation failed! " << err.text;
		ASSERT_NE(pServer->socketId, 0) << "The server socket should be not Null";
	}

	void testSocketOpt(int optname, bool isExpectedResultTrue, const char* sockOptName)
	{
		int optVal;
		socklen_t lenVal = sizeof(int);
		int iResult = getsockopt(pServer->socketId, SOL_SOCKET, optname, (char*)&optVal, &lenVal);
		ASSERT_EQ(iResult, 0) << "getsockopt " << sockOptName << ":  failed";
		ASSERT_EQ(lenVal, sizeof(int)) << "getsockopt " << sockOptName << ":  invalid length = " << lenVal;

		if (isExpectedResultTrue) {
			EXPECT_NE(optVal, 0) << "getsockopt " << sockOptName << ": should be True";
		}
		else {
			EXPECT_EQ(optVal, 0) << "getsockopt " << sockOptName << ": should be False";
		}
	}
};

/* Testing setup of the socket options for server call Bind */
#if defined(_WIN32) || defined(SO_REUSEPORT)
TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenDefaultInitValues)
{
	// Tests default init-values
	RsslBindOptions bindOpts = RSSL_INIT_BIND_OPTS;

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
#if defined(_WIN32)
	testSocketOpt(SO_REUSEADDR,			false,	"SO_REUSEADDR");
	testSocketOpt(SO_EXCLUSIVEADDRUSE,	true,	"SO_EXCLUSIVEADDRUSE");
#else  // Linux
	testSocketOpt(SO_REUSEADDR,			true,	"SO_REUSEADDR");
	testSocketOpt(SO_REUSEPORT,			false,	"SO_REUSEPORT");
#endif
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenClearBindOpts)
{
	// Tests rsslClearBindOpts
	RsslBindOptions bindOpts;
	rsslClearBindOpts(&bindOpts);

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
#if defined(_WIN32)
	testSocketOpt(SO_REUSEADDR, false, "SO_REUSEADDR");
	testSocketOpt(SO_EXCLUSIVEADDRUSE, true, "SO_EXCLUSIVEADDRUSE");
#else  // Linux
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
	testSocketOpt(SO_REUSEPORT, false, "SO_REUSEPORT");
#endif
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenSetFalse)
{
	// Tests serverSharedSocket off
	RsslBindOptions bindOpts;
	rsslClearBindOpts(&bindOpts);
	bindOpts.serverSharedSocket = RSSL_FALSE;

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
#if defined(_WIN32)
	testSocketOpt(SO_REUSEADDR, false, "SO_REUSEADDR");
	testSocketOpt(SO_EXCLUSIVEADDRUSE, true, "SO_EXCLUSIVEADDRUSE");
#else  // Linux
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
	testSocketOpt(SO_REUSEPORT, false, "SO_REUSEPORT");
#endif
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOnWhenSetTrue)
{
	// Tests serverSharedSocket on
	RsslBindOptions bindOpts;
	rsslClearBindOpts(&bindOpts);
	bindOpts.serverSharedSocket = RSSL_TRUE;

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket permits sharing
#if defined(_WIN32)
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
	testSocketOpt(SO_EXCLUSIVEADDRUSE, false, "SO_EXCLUSIVEADDRUSE");
#else  // Linux
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
	testSocketOpt(SO_REUSEPORT, true, "SO_REUSEPORT");
#endif
}

#else  // Linux && !defined(SO_REUSEPORT)  // LUP - Linux Unsopported SO_REUSEPORT

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenDefaultInitValuesLUP)
{
	// Tests default init-values
	RsslBindOptions bindOpts = RSSL_INIT_BIND_OPTS;

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenClearBindOptsLUP)
{
	// Tests rsslClearBindOpts
	RsslBindOptions bindOpts;
	rsslClearBindOpts(&bindOpts);

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeOffWhenSetFalseLUP)
{
	// Tests serverSharedSocket off
	RsslBindOptions bindOpts;
	rsslClearBindOpts(&bindOpts);
	bindOpts.serverSharedSocket = RSSL_FALSE;

	runRsslBind(bindOpts);
	ASSERT_NE(pServer, (RsslServer*)NULL);

	// Tests the socket options: the server socket does not permit sharing
	testSocketOpt(SO_REUSEADDR, true, "SO_REUSEADDR");
}

TEST_F(BindSharedServerSocketOpt, ServerSharedSocketShouldBeErrorOnRsslBindLUP)
{
	// Tests serverSharedSocket on
	RsslBindOptions bindOpts;
	RsslError err;
	rsslClearBindOpts(&bindOpts);
	bindOpts.serverSharedSocket = RSSL_TRUE;

	bindOpts.serviceName = (char*)"15000";
	bindOpts.protocolType = TEST_PROTOCOL_TYPE;  /* These tests are just sending a pre-set string across the wire, so protocol type should not be RWF */

	// Tests rsslBind returns NULL because SO_REUSEPORT option is unsupported
	pServer = rsslBind(&bindOpts, &err);

	ASSERT_EQ(pServer, (RsslServer*)NULL) << "rsslBind should return NULL when required set unsupported option SO_REUSEPORT";
	ASSERT_EQ(err.rsslErrorId, RSSL_RET_FAILURE) << "Error ccode should set to General Failure";
	ASSERT_EQ(err.sysError, 0) << "SysError should be equal to 0";
}

#endif

class WebsocketServerConnectionTest : public ::testing::Test {
protected:
	RsslServer* pServer;
	RsslChannel* pServerChannel;
	RsslCurlJITFuncs* curlFuncs;
	RsslSocket clientSocket;

	virtual void SetUp()
	{
		RsslError err;
		RsslBindOptions bindOpts;

		pServer = NULL;
		pServerChannel = NULL;

		rsslInitialize(RSSL_LOCK_GLOBAL, &err);

		// Start server
		rsslClearBindOpts(&bindOpts);
		bindOpts.serviceName = (char*)"20000";
		bindOpts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
		bindOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
		bindOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
		bindOpts.wsOpts.protocols = (char*)"rssl.json.v2";
		bindOpts.channelsBlocking = 1;
		memset(&err, 0, sizeof(RsslError));

		pServer = rsslBind(&bindOpts, &err);

		ASSERT_NE(pServer, (RsslServer*)NULL) << "Server creation failed! " << err.text;
		ASSERT_NE(pServer->socketId, 0) << "The server socket should be not Null";
	}

	virtual void TearDown()
	{
		RsslError err;
		if (pServerChannel != NULL)
		{
			rsslCloseChannel(pServerChannel, &err);
		}
		if (pServer != NULL)
		{
			rsslCloseServer(pServer, &err);
		}

		rsslUninitialize();
		pServer = NULL;
		pServerChannel = NULL;
		resetDeadlockTimer();
	}

	void connectClient()
	{
		RsslError			error;
		struct	sockaddr_in	toaddr;
		struct	sockaddr_in	baddr;
		int					sockRet;
		struct timeval		selectTime;
		int					selRet;
		RsslAcceptOptions	acceptOpts;
		bool				writeReadyBoth = false;
		RsslUInt32			addr;

		fd_set readfds;
		fd_set writefds;
		fd_set useread;
		fd_set usewrite;

		selectTime.tv_sec = 0L;
		selectTime.tv_usec = 500000;

		// Create a new socket, connect to the localhost server.
		clientSocket = socket(AF_INET, SOCK_STREAM, 6);
		ASSERT_NE(clientSocket, RIPC_INVALID_SOCKET) << "Invalid Socket";

		ipcSessSetMode(clientSocket, 0, 1, &error, __LINE__);

		baddr.sin_family = AF_INET;
		baddr.sin_addr.s_addr = host2net_u32(INADDR_ANY);
		baddr.sin_port = (RsslUInt16)0;

		sockRet = bind(clientSocket, (struct sockaddr*)&baddr, (int)sizeof(baddr));
		ASSERT_GE(sockRet, 0) << "bind failure";

		rsslGetHostByName((char*)"localhost", &addr);
		
		// Localhost connection, so inaddr_loopback should be correct.
		// Address and port needs to be in network byte order.
		toaddr.sin_family = AF_INET;
		toaddr.sin_addr.s_addr = addr;
		toaddr.sin_port = host2net_u16((RsslUInt16)20000);

		sockRet = connect(clientSocket, (struct sockaddr*)&toaddr, (int)sizeof(toaddr));
		if (sockRet < 0)
		{
#if defined(_WIN32)
			if ((errno != WSAEINPROGRESS) && (errno != WSAEWOULDBLOCK))
#else
			if ((errno != EINPROGRESS) && (errno != EALREADY))
#endif
			{
				ASSERT_TRUE(false) << "Connect failure";
			}
		}

		selRet = 0;
		FD_ZERO(&readfds);
		FD_ZERO(&useread);

		FD_SET(pServer->socketId, &readfds);

		do
		{
			useread = readfds;
			selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);
			ASSERT_GE(selRet, 0) << "Select failure";
		} while (selRet == 0);

		rsslClearAcceptOpts(&acceptOpts);
		pServerChannel = rsslAccept(pServer, &acceptOpts, &error);

		FD_ZERO(&writefds);
		FD_ZERO(&usewrite);
		FD_SET(pServerChannel->socketId, &writefds);
		FD_SET(clientSocket, &writefds);
		do
		{
			selectTime.tv_sec = 0L;
			selectTime.tv_usec = 500000;

			usewrite = writefds;
			selRet = select(FD_SETSIZE, NULL, &usewrite, NULL, &selectTime);
			ASSERT_GE(selRet, 0) << "Select failure";

			if (FD_ISSET(pServerChannel->socketId, &usewrite) && FD_ISSET(clientSocket, &usewrite))
			{
				writeReadyBoth = true;
			}

		} while (writeReadyBoth == false);
		resetDeadlockTimer();
	}
};

// This tests the server side checks required by RFC6455 for initial websocket client handshake parsing.
TEST_F(WebsocketServerConnectionTest, WebsocketServerHandshakeTest)
{
	char				writeBuff[1024];
	char				readBuff[1024];
	RsslInt32			cc;
	struct timeval		selectTime;
	int					selRet;
	int					numBytes;
	RsslError			error;
	RsslInProgInfo		inProgInfo;
	const char			*serverOpenHandShake =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Protocol: tr_json2\r\n"
		"Sec-WebSocket-Accept: gMwr65Y4mfQzKM+BbgkgXL43ZYA=\r\n";

	const char			*serverBadRequest =
		"HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Cache-Control: no-cache, private, no-store\r\n"
		"Transfer-Encoding: chunked\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Connection: close\r\n";

	fd_set readfds;
	fd_set useread;

	// Connect up the client to the server
	connectClient();

	// Send a PUT, this should error out.
	// Key is taken from a random run of a consumer connecting to a provider.
	cc = snprintf(writeBuff, 1000, "PUT /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;
	
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Channel would be in closed state
	ASSERT_EQ (error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET,  but without the host this should error out.
	cc = snprintf(writeBuff, 1000, "PUT /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET, but without the upgrade this should error out.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET, but without the connection this should error out.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET, but without the key this should error out.
	// Key is taken from a random run of a consumer connecting to a provider.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET, but without the version this should error out.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: yB0kpwchuxb9yF1Jbm3OTA==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Verify server response
	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(clientSocket, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	numBytes = SOCK_RECV(clientSocket, readBuff, 1024, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_RECV failed";

	ASSERT_TRUE(0 == strncmp(readBuff, serverBadRequest, strlen(serverBadRequest)));

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET, but an incorrect websocket version this should error out.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 14\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "User-Agent: Mozilla/5.0\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect failure here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_FAILURE);

	// Verify server response
	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(clientSocket, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	numBytes = SOCK_RECV(clientSocket, readBuff, 1024, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_RECV failed";

	ASSERT_TRUE(0 == strncmp(readBuff, serverBadRequest, strlen(serverBadRequest)));

	// Channel would be in closed state
	ASSERT_EQ(error.channel->state, RsslChannelState::RSSL_CH_STATE_CLOSED);

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send a GET,  without the origin, extensions, user-agent this pass, cause those fields are optional per RFC6455 
	// RFC6455 exception: Sec-WebSocket-Protocol is an obligatory our WS implementation.
	cc = snprintf(writeBuff, 1000, "GET /WebSocket HTTP/1.1\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: keep-alive, Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Host: localhost:14002\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Key: wSam/MfhQlzaI4qDu/lwVw==\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Version: 13\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(clientSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pServerChannel->socketId, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProgInfo);

	// Expect pass here.
	ASSERT_EQ(rsslInitChannel(pServerChannel, &inProgInfo, &error), RSSL_RET_SUCCESS);

	// Verify server response
	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(clientSocket, &readfds);
	useread = readfds;

	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	numBytes = SOCK_RECV(clientSocket, readBuff, 1024, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_RECV failed";

	ASSERT_TRUE(0==strncmp(readBuff, serverOpenHandShake, strlen(serverOpenHandShake)));

	rsslCloseChannel(pServerChannel, &error);
	pServerChannel = NULL;

	sock_close(clientSocket);
	clientSocket = RIPC_INVALID_SOCKET;
}


class WebsocketClientConnectionTest : public ::testing::Test {
protected:

	RsslSocket server;
	RsslSocket serverSocket;
	RsslChannel *pClientChannel;

	virtual void SetUp()
	{
		struct timeval		selectTime;
		RsslError			error;
		struct	sockaddr_in	baddr;
		int					sockRet;
		int					flag = 1;

		rsslInitialize(RSSL_LOCK_GLOBAL, &error);

		selectTime.tv_sec = 0L;
		selectTime.tv_usec = 500000;

		// Create a new socket, connect to the localhost server.
		server = socket(AF_INET, SOCK_STREAM, 6);
		ASSERT_NE(server, RIPC_INVALID_SOCKET) << "Invalid Socket";

		ipcSessSetMode(server, 0, 1, &error, __LINE__);

		sockRet = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, (int)sizeof(flag));
		ASSERT_GE(sockRet, 0) << "setsockopt failure";

		baddr.sin_family = AF_INET;
		baddr.sin_addr.s_addr = host2net_u32(INADDR_ANY);
		baddr.sin_port = host2net_u16((RsslUInt16)20000);

		sockRet = bind(server, (struct sockaddr*)&baddr, (int)sizeof(baddr));
		ASSERT_GE(sockRet, 0) << "bind failure";

		sockRet = listen(server, 10);
		ASSERT_GE(sockRet, 0) << "listen failure";
	}

	virtual void TearDown()
	{
		RsslError err;
		if (pClientChannel != NULL)
		{
			rsslCloseChannel(pClientChannel, &err);
		}

		if (server != RIPC_INVALID_SOCKET)
			sock_close(server);

		if (serverSocket != RIPC_INVALID_SOCKET)
			sock_close(serverSocket);

		rsslUninitialize();
		resetDeadlockTimer();
	}

	void connectClient()
	{
		RsslError			error;
		RsslConnectOptions copts = RSSL_INIT_CONNECT_OPTS;
		struct	sockaddr_in	toaddr;
		struct timeval		selectTime;
		int					selRet;
		bool				writeReadyBoth = false;
		RsslRet             ret;
		RsslInProgInfo		inProg = RSSL_INIT_IN_PROG_INFO;
#ifdef WIN32
		int					toaddrLen;
#else
		socklen_t			toaddrLen;
#endif // WIN32


		fd_set readfds;
		fd_set writefds;
		fd_set useread;
		fd_set usewrite;

		copts.connectionInfo.unified.address = (char*)"localhost";
		copts.connectionInfo.unified.serviceName = (char*)"20000";
		copts.connectionType = RSSL_CONN_TYPE_WEBSOCKET;
		copts.wsOpts.protocols = (char*)"rssl.json.v2";
		copts.majorVersion = RSSL_RWF_MAJOR_VERSION;
		copts.minorVersion = RSSL_RWF_MINOR_VERSION;
		copts.protocolType = RSSL_RWF_PROTOCOL_TYPE;

		pClientChannel = rsslConnect(&copts, &error);
		ASSERT_NE(pClientChannel, (RsslChannel*)NULL) << "Client failed to connect " << error.text;

		FD_ZERO(&readfds);
		FD_ZERO(&useread);
		FD_ZERO(&writefds);
		FD_ZERO(&usewrite);

		FD_SET(pClientChannel->socketId, &readfds);
		FD_SET(pClientChannel->socketId, &writefds);

		useread = readfds;
		usewrite = writefds;
		selRet = select(FD_SETSIZE, &useread, &usewrite, NULL, &selectTime);

		ASSERT_GE(selRet, 0) << "Select failure";

		FD_CLR(pClientChannel->socketId, &useread);
		FD_CLR(pClientChannel->socketId, &usewrite);

		rsslClearInProgInfo(&inProg);
		ret = rsslInitChannel(pClientChannel, &inProg, &error);
		ASSERT_GE(ret, RSSL_RET_SUCCESS) << "Client failed to init channel " << error.text;

		// Localhost connection, so inaddr_loopback should be correct.
		// Address and port needs to be in network byte order.
		toaddr.sin_family = AF_INET;
		toaddr.sin_addr.s_addr = host2net_u32(INADDR_ANY);
		toaddr.sin_port = host2net_u16((RsslUInt16)20000);

		selRet = 0;
		FD_ZERO(&readfds);
		FD_ZERO(&useread);
		
		FD_SET(server, &readfds);

		do
		{
			useread = readfds;
			selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);
			ASSERT_GE(selRet, 0) << "Select failure";
		} while (selRet == 0);

		toaddrLen = sizeof(toaddr);

		serverSocket = accept(server, (struct sockaddr*)&toaddr, &toaddrLen);
		ASSERT_NE(serverSocket, RIPC_INVALID_SOCKET) << "Invalid Socket";

		FD_ZERO(&writefds);
		FD_ZERO(&usewrite);
		FD_ZERO(&readfds);
		FD_ZERO(&useread);

		FD_SET(serverSocket, &writefds);
		FD_SET(pClientChannel->socketId, &writefds);
		do
		{
			selectTime.tv_sec = 0L;
			selectTime.tv_usec = 500000;
		
			usewrite = writefds;
			useread = readfds;
			selRet = select(FD_SETSIZE, &useread, &usewrite, NULL, &selectTime);
			ASSERT_GE(selRet, 0) << "Select failure";
		
			if (FD_ISSET(serverSocket, &usewrite) && FD_ISSET(pClientChannel->socketId, &usewrite))
			{
				writeReadyBoth = true;
			}		
		} while (writeReadyBoth == false);
		
		resetDeadlockTimer();
	}
};

TEST_F(WebsocketClientConnectionTest, WebsocketClientHandshakeTest)
{
	char				writeBuff[1024];
	RsslInt32			cc;
	struct timeval		selectTime;
	int					selRet;
	int					numBytes;
	RsslError			error;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;

	fd_set readfds;
	fd_set useread;

	// Connect up the client to the server
	connectClient();

	// Send response with wrong status line 
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 111 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "Invalid HTTP response, status code 111") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	// Connect up the client to the server
	connectClient();

	// Send response without status line
	cc = snprintf(writeBuff, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "Bad HTTP status/request line") != NULL);

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	// Connect up the client to the server
	connectClient();

	// Send response without upgrade key
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 101 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "No Upgrade: key received.") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	// Connect up the client to the server
	connectClient();

	// Send response without connection key 
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 101 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "No Connection: key received.") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	// Connect up the client to the server
	connectClient();

	// Send response without Sec-WebSocket-Protocol key
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 101 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "Unsupported Websocket protocol type received") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	// Connect up the client to the server
	connectClient();

	// Send response without Sec-WebSocket-Accept
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 101 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "No Sec-Websocket-Accept: key received, key expected") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;

	// Connect up the client to the server
	connectClient();

	// Send response with wrong Sec-WebSocket-Accept
	cc = snprintf(writeBuff, 1000, "HTTP/1.1 101 Switching Protocols\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Upgrade: websocket\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Connection: Upgrade\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Protocol: tr_json2\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n");
	cc += snprintf(writeBuff + cc, 1000 - cc, "\n");

	// This should be a single packet
	numBytes = SOCK_SEND(serverSocket, writeBuff, cc, 0);
	ASSERT_GE(numBytes, 1) << "SOCK_SEND failed";

	FD_ZERO(&readfds);
	FD_ZERO(&useread);

	FD_SET(pClientChannel->socketId, &readfds);

	useread = readfds;
	selectTime.tv_sec = 0L;
	selectTime.tv_usec = 200000;
	selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

	ASSERT_GE(selRet, 1) << "Select failure";

	rsslClearInProgInfo(&inProg);

	ASSERT_EQ(rsslInitChannel(pClientChannel, &inProg, &error), RSSL_RET_FAILURE);

	ASSERT_TRUE(strstr(error.text, "Key received 's3pPLMBiTxaQ9kYGzzhZRbK+xOo=' is not key expected") != NULL);

	sock_close(serverSocket);
	serverSocket = RIPC_INVALID_SOCKET;

	rsslCloseChannel(pClientChannel, &error);
	pClientChannel = NULL;
}

rsslServerCountersInfo* rsslGetServerCountersInfo(RsslServer* pServer)
{
	rsslServerImpl *srvrImpl = (rsslServerImpl *)pServer;
	rsslServerCountersInfo* serverCountersInfo = &srvrImpl->serverCountersInfo;
	return serverCountersInfo;
}

int main(int argc, char* argv[])
{
	int ret;
	bool isGtestFilter = false;

	try {
		::testing::InitGoogleTest(&argc, argv);

		// Run ServerStartStopTests after all the other tests.
		// ServerStartStopTests create and destroy the transport library and its internal members many times.
		// It will interfere with other tests that could run parallel.
		if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter)=="*")
		{
			if (checkCertificateFiles() && checkClientCertificateFiles())
			{
				::testing::GTEST_FLAG(filter) = "-ServerStartStopTests.*";
			}
			else
			{
				std::cout << "The tests that check encrypted connection will be skipped." << std::endl
					<< "Creation of server on an encrypted connection requires key-file \"" << getPathServerKey() << "\" and certificate-file \"" << getPathServerCert() << "\"." << std::endl
					<< "Creation of client on an encrypted connection requires certificate-file \"" << getOpenSSLCAStore() << "\"." << std::endl;
				::testing::GTEST_FLAG(filter) = "-ServerStartStopTests.*:*Encrypted*:*EncrWebSock*";
			}

		}
		else
		{
			isGtestFilter = true;
		}

		RsslThreadId dlThread;
		RSSL_MUTEX_INIT(&pipeLock);

		RSSL_THREAD_START(&dlThread, deadlockThread, &dlThread);

		ret =  RUN_ALL_TESTS();
		testComplete = true;
		resetDeadlockTimer();
		RSSL_THREAD_JOIN(dlThread);

		// Run ServerStartStopTests
		if (!isGtestFilter)
		{
			if (checkCertificateFiles())
			{
				::testing::GTEST_FLAG(filter) = "ServerStartStopTests.*";
			}
			else
			{
				std::cout << "The tests ServerSSLStartStop*Test will be skipped." << std::endl
					<< "Creation of server on an encrypted connection requires key-file \"" << getPathServerKey() << "\" and certificate-file \"" << getPathServerCert() << "\"." << std::endl;
				::testing::GTEST_FLAG(filter) = "ServerStartStopTests.*:-ServerStartStopTests.ServerSSL*";
			}

			ret = RUN_ALL_TESTS();
		}
	} catch (const std::exception& e) {
		std::cout << "GoogleTest failed: %s\n"<< e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cout << "GoogleTest failed: unknown error\n" << std::endl;
		return 1;
	}
	return ret;
}
 