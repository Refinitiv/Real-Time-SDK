/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2023 Refinitiv. All rights reserved.          --
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

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslThread.h"
#include "rtr/ripcutils.h"
#include "rtr/rsslEventSignal.h"
#include "rtr/ripcsslutils.h"
#include "rtr/rsslGetTime.h"

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

/* Simple deadlock detection thread.  This thread will set a 15 second timer.  If a test is not 
 * completed(either success or failure) during that time, then the function calls abort.  
 * All tests will need to trigger the rssl_pipe deadlockPipe in order to stop this timer.
 * PRECONDITIONS: Pipe has already been created */
RSSL_THREAD_DECLARE(deadlockThread, pArg)
{
	struct timeval selectTime;
	int selRet;

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
		selectTime.tv_sec = 150;
		selectTime.tv_usec = 0;

		useread = readfds;

		selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

		if (selRet <= 0)
		{
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

	EventSignal* signalForWrite; // This is an event-signal to write-loop that read-lop is ready for reading next chunk

	ReadChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		readCount = NULL;
		lock = NULL;
		lockReadWrite = NULL;
		signalForWrite = NULL;
	}

	/*	Read from pChnl.  Increments the read counter whenever rsslRead returns a buffer.
		If blocking is set to RSSL_TRUE, the function hard loops on rsslRead.  If not, the function will 
		sit on select and only call rsslRead if pChnl's FD is set.
		Note that this function does not validate the data, as the rsslRead function is not thread 
		aware and does not guarantee any message order for a given thread.  
	*/
	void readLoop(RsslBool blocking)
	{
		fd_set readfds;
		fd_set useread;
		RsslRet readRet;
		RsslError err;
		int readmsg = 0;
		bool readIncrement = false;
		bool shouldForceSelect = false;

		FD_ZERO(&readfds);
		FD_ZERO(&useread);

		RsslBuffer* readBuf;

		struct timeval selectTime;
		int selRet;

		if (pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE)
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

		FD_SET(pChnl->socketId, &readfds);

		while ( !shutdownTest && !failTest )
		{
			if ( blocking == RSSL_FALSE || shouldForceSelect )
			{
				useread = readfds;
				selectTime.tv_sec = 0L;
				selectTime.tv_usec = 500000;
				selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

				ASSERT_GE(selRet, 0) << "Reader select failed.";

				/* We're shutting down, so there is no failure case here */
				if ( shutdownTest || failTest )
				{
					break;
				}
				if (!FD_ISSET(pChnl->socketId, &useread))
				{
					continue;
				}
			}

			/* Special tests use an additional lock on entire read operation */
			/* It allows read and write operations to be performed step by step */
			if (lockReadWrite != NULL)
			{
				RSSL_MUTEX_LOCK(lockReadWrite);

				if (shutdownTest || failTest)
					break;
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
				}
				else
				{
					if (readRet < RSSL_RET_SUCCESS && readRet != RSSL_RET_READ_PING && readRet != RSSL_RET_READ_IN_PROGRESS && readRet != RSSL_RET_READ_WOULD_BLOCK)
					{
						if (lockReadWrite != NULL)
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
						}

						/* We're shutting down, so there is no failure case here */
						if (shutdownTest)
							return;
						failTest = true;
						ASSERT_TRUE(false) << "rsslRead failed. Return code:" << readRet << " Error info: " << err.text << " readCount: " << *(readCount);
					}
				}

				/* Checks the error case */
				if ( pChnl-> connectionType == RSSL_CONN_TYPE_WEBSOCKET
					&& (readRet == RSSL_RET_READ_WOULD_BLOCK || readRet > 0) )
				{
					rsslChannelImpl* rsslChnlImpl = (rsslChannelImpl*)pChnl;
					RsslSocketChannel* rsslSocketChannel = (RsslSocketChannel*)rsslChnlImpl->transportInfo;
					if (rsslSocketChannel->inputBufCursor > rsslSocketChannel->inputBuffer->length)
					{
						if (lockReadWrite != NULL)
						{
							RSSL_MUTEX_UNLOCK(lockReadWrite);
						}

						/* We're shutting down, so there is no failure case here */
						if (shutdownTest)
							return;
						failTest = true;
						ASSERT_TRUE(false) << "rsslRead: error case. Return code:" << readRet << " Error info: " << err.text << " readCount: " << *(readCount);
					}
				}
			} while ( readRet > 0 && !shutdownTest && !failTest );

			/* Special tests use an additional lock on entire read operation */
			/* It allows read and write operations to be performed step by step */
			if (lockReadWrite != NULL)
			{
				RSSL_MUTEX_UNLOCK(lockReadWrite);

				/* Sets the event-signal for a write loop: the read loop is ready for reading next chunk */
				signalForWrite->setSignal();
			}

			/* readIncrement stops us from resetting the deadlock timer more than once per readCount */
			if (*(readCount) % 10000 == 1 && readIncrement == true)
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
			while (!shutdownTest && writeCount < writeMax)
			{
				/* Hardlooping on Flush and getBuffer to make absolutely sure we get a buffer.  */
				writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);

				while (writeBuf == NULL)
				{
					if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
					}
					writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);
				}

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

				while (needFlush)
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
			while (!shutdownTest && writeCount < writeMax)
			{
				/* Hardlooping on Flush and getBuffer to make absolutely sure we get a buffer.  */
				writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);

				while (writeBuf == NULL)
				{
					if ((ret = rsslFlush(pChnl, &err)) < RSSL_RET_SUCCESS)
					{
						failTest = true;
						ASSERT_TRUE(false) << "Flush failed.  Error: " << err.text;
					}
					writeBuf = rsslGetBuffer(pChnl, bufferlen + 50, RSSL_FALSE, &err);
				}

				memcpy(writeBuf->data, testBuffer, (size_t)bufferlen);
				writeBuf->length = bufferlen;

				ret = rsslWriteEx(pChnl, writeBuf, &inArgs, &outArgs, &err);
				if (ret == RSSL_RET_WRITE_CALL_AGAIN)
				{
					while (ret == RSSL_RET_WRITE_CALL_AGAIN)
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

				while (needFlush)
				{
					FD_SET(pChnl->socketId, &writefds);
					selectTime.tv_sec = 0L;
					selectTime.tv_usec = 500000;
					selRet = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &selectTime);

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
	int				writeCount;					/* Counter of write operations. See writeLoop(). */

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

		int kn = snprintf(writeBuf->data + k, (fillLength - k), "%d:", writeCount);
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
		RsslRet ret;
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
			ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslWriteEx failed.  Error: " << pError->text;
		}
		else
		{
			needFlush = true;
		}

		while ( needFlush && !shutdownTest && !failTest )
		{
			if ( (ret = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS )
			{
				if ( lockReadWrite != NULL )
				{
					RSSL_MUTEX_UNLOCK(lockReadWrite);
				}
				failTest = true;
				ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslFlush failed. attempts: " << kAttemptsCallFlush << ", Error: " << pError->text;
			}

			if ( ret == RSSL_RET_SUCCESS )
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
				ASSERT_TRUE(false) << "writeBuffer_Blocking. rsslFlush failed many times. attempts: " << kAttemptsCallFlush << ", Error: " << pError->text;
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

		RsslRet ret;
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
					ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Flush failed.  Error: " << pError->text;
				}

				if ( shutdownTest || failTest )
					return;

				ret = rsslWriteEx(chnl, buffer, writeInArgs, writeOutArgs, pError);
				if (ret < RSSL_RET_SUCCESS)
					needFlush = true;
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
			ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Write failed.  Error: " << pError->text;
		}

		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_ZERO(&writefds);

		while ( needFlush && !shutdownTest && !failTest )
		{
			FD_SET(pChnl->socketId, &writefds);
			FD_SET(pChnl->socketId, &exceptfds);
			selectTime.tv_sec = 0L;
			selectTime.tv_usec = 20000;
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
				ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Select failed. attempts: " << kAttemptsCallFlush << ", Errno: " << errno;
			}

			if ( FD_ISSET(pChnl->socketId, &writefds) )
			{
				/* Hard looping on Flush to ensure that the call is made. */
				if ( (ret = rsslFlush(pChnl, pError)) < RSSL_RET_SUCCESS )
				{
					if ( lockReadWrite != NULL )
					{
						RSSL_MUTEX_UNLOCK(lockReadWrite);
					}
					failTest = true;
					ASSERT_TRUE(false) << "writeBuffer_NonBlocking. Flush failed. attempts: " << kAttemptsCallFlush << ", Error: " << pError->text;
				}
				if ( ret == RSSL_RET_SUCCESS )
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
				ASSERT_TRUE(false) << "writeBuffer_NonBlocking. rsslFlush failed many times. attempts: " << kAttemptsCallFlush << " ret:" << ret;
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

				if (!shutdownTest && writeCount < writeMax && !failTest)
				{
					// Waits for an event-signal that a read loop is ready for reading next chunk
					eventSignalReadWrite->waitSignal(0, 50000);
				}
			}

			if ( writeCount % 10000 == 1 )
				resetDeadlockTimer();
			//if ( writeCount % 2000 == 0 )
			//{
			//	std::cout << channelTitle << ":{" << writeCount << "} ";
			//}
		}
		//std::cout << "writeLoop Finished. " << channelTitle << " writeCount: " << writeCount << std::endl;
	}

};

const unsigned int WriteChannelTransport::MAX_LIMIT_CALL_FLUSH = 20U;

const char* WriteChannelTransport::testBuffer = "TestDataInfo\0";
const RsslUInt32 WriteChannelTransport::testBufferLength = (RsslUInt32)strlen(testBuffer);

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
			msgLengths = new RsslUInt32[12]; // { 100, 100, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000 };
			nLen = 12;
			msgLengths[0] = 100; msgLengths[1] = 100;
			msgLengths[2] = 3000; msgLengths[3] = 3000; msgLengths[4] = 3000; msgLengths[5] = 3000;
			msgLengths[6] = 3000; msgLengths[7] = 3000; msgLengths[8] = 3000; msgLengths[9] = 3000;
			msgLengths[10] = 3000; msgLengths[11] = 3000;
		}
		else if (configIndex == 1)
		{
			msgLengths = new RsslUInt32[14]; // { 100, 100, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 2918, 2500, 3000, 3000 };
			nLen = 14;
			msgLengths[0] = 100; msgLengths[1] = 100;
			msgLengths[2] = 3000; msgLengths[3] = 3000; msgLengths[4] = 3000; msgLengths[5] = 3000;
			msgLengths[6] = 3000; msgLengths[7] = 3000; msgLengths[8] = 3000; msgLengths[9] = 3000;
			msgLengths[10] = 2918; msgLengths[11] = 2500; msgLengths[12] = 3000; msgLengths[13] = 3000;
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
				//printf("writeLoop. writeCount=%d\n", writeCount);
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
			//if ( writeCount % 2000 == 0 )
			//{
			//	std::cout << channelTitle << ":{" << writeCount << "} ";
			//}
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

		testBuffer = new unsigned char[fileSize+32];
		if (testBuffer == NULL)
		{
			*bufferLength = 0U;
			*numMessagesInBuffer = 0U;
			if (errorText != NULL && szErrorText > 0)
#ifdef WIN32
				snprintf(errorText, szErrorText, "%s fileSize: %llu", TB_ERR_MEM_ALLOC_FAIL, fileSize);
#else
				snprintf(errorText, szErrorText, "%s fileSize: %lu", TB_ERR_MEM_ALLOC_FAIL, fileSize);
#endif
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
				*numMessagesInBuffer = 0;
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
			// "inpbuf_01.bin". 4 messages inside.
			return new MsgFileBuffer("inpbuf_01.bin");
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
	int				writeCount;					/* Counter of write operations. See writeLoop(). */

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


		if (pChnl == NULL || pChnl->state != RSSL_CH_STATE_ACTIVE)
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

			if (blocking == RSSL_FALSE)
			{
				waitReadyForWrite(totalSend, bufferlen);
			}

			// Sends the message buffer by chunks
			while ( totalSend < bufferlen && !shutdownTest && !failTest )
			{
				// Calculate the length of this chunk
				if (nChunkLengths > 0)
				{
					chunkLength = arrChunkLengths[(chunkIndex % nChunkLengths)];
				}
				if (chunkLength == 0)
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
							break;
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
							std::cout << "Write failed!!!" << std::endl;
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

				if (chunkSend > 0)
				{
					totalSend += chunkSend;
				}
				chunkIndex++;

			}  // while ( totalSend < bufferlen )

			writeCount += numMessagesInBuffer;

			if (writeCount % 10000 == 1)
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

	if (server == NULL)
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

RSSL_THREAD_DECLARE(nonBlockingWriteSystemThread, pArg)
{
	((WriteChannelSystem*)pArg)->writeLoop(RSSL_FALSE);

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
			else if ( (curTime - startTime) > 5000 )
			{
				failTest = true;
				isCheckLongTimeNoRead = true;
			}
		}
		return isCheckLongTimeNoRead;
	}
};

class GlobalLockTestsFragmentedFixture : public GlobalLockTests, public ::testing::WithParamInterface<GLobalLockFragmentedTestParams>
{
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
	} while (readCount < (MAX_MSG_WRITE_COUNT * 2));

	shutdownTest = true;
	RSSL_THREAD_KILL(&clientWriteThread);
	RSSL_THREAD_KILL(&serverWriteThread);
	RSSL_THREAD_KILL(&clientReadThread);
	RSSL_THREAD_KILL(&serverReadThread);
	rsslCloseChannel(serverChannel, &err);
	rsslCloseChannel(clientChannel, &err);

	ASSERT_FALSE(failTest) << "Test failed.";

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
	serverWriteOpts.pThreadId = &serverWriteThread;
	serverWriteOpts.pChnl = serverChannel;

	clientReadOpts.pThreadId = &clientReadThread;
	clientReadOpts.pChnl = clientChannel;
	clientReadOpts.readCount = &msgsRead;
	clientReadOpts.lock = &readLock;
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

	RSSL_THREAD_KILL(&clientWriteThread);
	RSSL_THREAD_KILL(&serverWriteThread);
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

	WriteChannelTransport serverWriteOpts(testParams.msgLength, testParams.msgWriteCount, "SrvW");
	WriteChannelTransport clientWriteOpts(testParams.msgLength, testParams.msgWriteCount, "CliW");

	RsslError err;
	int readCount = 0;
	int readCountPrev = 0;
	int msgWriteCount = (testParams.msgWriteCount != 0 ? testParams.msgWriteCount : MAX_MSG_WRITE_COUNT);

	startupServerAndConections(0, RSSL_TRUE,
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

	RSSL_THREAD_START(&serverReadThread, blockingReadThread, (void*)&serverReadOpts);
	RSSL_THREAD_START(&serverWriteThread, blockingWriteTransportThread, (void*)&serverWriteOpts);

	RSSL_THREAD_START(&clientReadThread, blockingReadThread, (void*)&clientReadOpts);
	RSSL_THREAD_START(&clientWriteThread, blockingWriteTransportThread, (void*)&clientWriteOpts);

	std::cout << "GlobalLockTestsFragmentedFixture  Run test. msgWriteCount: " << msgWriteCount << std::endl;

	//int m = 0;
	do
	{
		time_sleep(200);
		//		std::cout << "GlobalLockTestsFragmentedFixture  locking..." << std::endl;
		RSSL_MUTEX_LOCK(&readLock);
		//		std::cout << "GlobalLockTestsFragmentedFixture  Locked." << std::endl;
		readCount = msgsRead;
		RSSL_MUTEX_UNLOCK(&readLock);
		//std::cout << "GlobalLockTestsFragmentedFixture  Unlocked. readCount: " << readCount << " m: " << m << std::endl;
		//if (m % 10 == 0)
		//	std::cout << "  R " << m << ":" << readCount << "  ";
		//++m;

		EXPECT_FALSE(checkLongTimeNoRead(readCountPrev, readCount)) << "Long time no read data. readCount: " << readCount;
	} while (readCount < (msgWriteCount * 2) && !failTest);

	std::cout << "GlobalLockTestsFragmentedFixture  Loop finished. failTest: " << failTest << " readCount: " << readCount << std::endl;

	shutdownTest = true;
	time_sleep(50);

	std::cout << "GlobalLockTestsFragmentedFixture  Before Join Write-threads." << std::endl;
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

	std::cout << "GlobalLockTestsFragmentedFixture  Before rsslCloseChannel." << std::endl;

	EXPECT_EQ(rsslCloseChannel(serverChannel, &err), RSSL_RET_SUCCESS) << "serverChannel close error: " << err.text;
	EXPECT_EQ(rsslCloseChannel(clientChannel, &err), RSSL_RET_SUCCESS) << "clientChannel close error: " << err.text;

	RSSL_THREAD_KILL(&clientReadThread);
	RSSL_THREAD_KILL(&serverReadThread);

	//std::cout << "GlobalLockTestsFragmentedFixture  Before Join clientReadThread." << std::endl;
	//RSSL_THREAD_JOIN(clientReadThread);
	//std::cout << "GlobalLockTestsFragmentedFixture  Before Join serverReadThread." << std::endl;
	//RSSL_THREAD_JOIN(serverReadThread);

	std::cout << "GlobalLockTestsFragmentedFixture  FINISH." << std::endl;

	ASSERT_FALSE(failTest) << "Test failed.";
}

INSTANTIATE_TEST_CASE_P(
	MsgLengthWebSockRWF,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 0, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_RWF_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_CASE_P(
	MsgLengthWebSockJSON,
	GlobalLockTestsFragmentedFixture,
	::testing::Values(
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,   100, 0, 40000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  1250, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6135, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6145, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET,  6150, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockFragmentedTestParams( RSSL_CONN_TYPE_WEBSOCKET, 12263, 0, 10000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
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
			else if ((curTime - startTime) > 1000)
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
	if (1 <= testParams.configIndex && testParams.configIndex <= 6)
	{   // all the tests 1 .. 6 are: step-by-step
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
		time_sleep(200);
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

INSTANTIATE_TEST_CASE_P(
	MsgLengthWebSockJSONFile,
	SystemTestsFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 14673, 0, 2000000, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
	)
);

INSTANTIATE_TEST_CASE_P(
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

		//EXPECT_FALSE(checkLongTimeNoRead(readCountPrev, readCount)) << "Long time no read data. readCount: " << readCount;
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

		//EXPECT_FALSE(checkLongTimeNoRead(readCountPrev, readCount)) << "Long time no read data. readCount: " << readCount;
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

INSTANTIATE_TEST_CASE_P(
	WebSockJSONClients,
	SystemTestsSendBuffersFixture,
	::testing::Values(
		GLobalLockSystemTestParams( 0, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 24, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE ),
		GLobalLockSystemTestParams( 1, RSSL_CONN_TYPE_WEBSOCKET, 3000, 3000, 28, RSSL_CONN_TYPE_SOCKET, RSSL_JSON_PROTOCOL_TYPE )
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

	::testing::InitGoogleTest(&argc, argv);

	// Run ServerStartStopTests after all the other tests.
	// ServerStartStopTests create and destroy the transport library and its internal members many times.
	// It will interfere with other tests that could run parallel.
	if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter)=="*")
	{
		::testing::GTEST_FLAG(filter) = "-ServerStartStopTests.*";
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
			std::cout << "The tests ServerSSLStartStop*Test will be skip." << std::endl
				<< "Creation of server on an encrypted connection requires key-file \"" << getPathServerKey() << "\" and certificate-file \"" << getPathServerCert() << "\"." << std::endl;
			::testing::GTEST_FLAG(filter) = "ServerStartStopTests.*:-ServerStartStopTests.ServerSSL*";
		}

		ret = RUN_ALL_TESTS();
	}
	return ret;
}
