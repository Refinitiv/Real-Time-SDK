/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
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
		selectTime.tv_sec = 15;
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
				rsslResetEventSignal(&deadlockPipe);
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

	ServerChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		pServer = NULL;
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
			while (shutdownTest != true && serverChnl == NULL)
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
			while (shutdownTest != true && serverChnl == NULL)
			{
				selectTime.tv_sec = 0L;
				selectTime.tv_usec = 0L;
				FD_SET(pServer->socketId, &readfds);
				selRet = select(FD_SETSIZE, &readfds, NULL, NULL, &selectTime);

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
			while (shutdownTest != true && serverChnl->state != RSSL_CH_STATE_ACTIVE)
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

	ClientChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
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

		connectOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
		connectOpts.connectionInfo.unified.address = (char*)"localhost";
		connectOpts.connectionInfo.unified.serviceName = (char*)"15000";
		connectOpts.protocolType = TEST_PROTOCOL_TYPE;
		connectOpts.tcp_nodelay = true;
		connectOpts.blocking = blocking;

		pClientChnl = rsslConnect(&connectOpts, &err);

		if (pClientChnl == NULL)
		{
			failTest = true;
			ASSERT_TRUE(false) << "rsslConnect failed with error text: " << err.text;
		}
		if (blocking == RSSL_FALSE)
		{
			/* Hard looping on rsslInitChannel */
			while (shutdownTest != true && pClientChnl->state != RSSL_CH_STATE_ACTIVE)
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
	RsslMutex* lock;

	ReadChannel()
	{
		pThreadId = NULL;
		pChnl = NULL;
		readCount = NULL;
		lock = NULL;
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

		FD_ZERO(&readfds);
		FD_ZERO(&useread);

		RsslBuffer* readBuf;

		struct timeval selectTime;
		int selRet;

		FD_SET(pChnl->socketId, &readfds);

		while (!shutdownTest)
		{
			if (blocking == RSSL_FALSE)
			{
				useread = readfds;
				selectTime.tv_sec = 0L;
				selectTime.tv_usec = 500000;
				selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

				ASSERT_GE(selRet, 0) << "Reader select failed.";

				if (!FD_ISSET(pChnl->socketId, &useread))
				{
					continue;
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
				}
				else
				{
					if (readRet < RSSL_RET_SUCCESS && readRet != RSSL_RET_READ_PING && readRet != RSSL_RET_READ_IN_PROGRESS && readRet != RSSL_RET_READ_WOULD_BLOCK)
					{
						/* We're shutting down, so there is no failure case here */
						if (shutdownTest)
							return;
						failTest = true;
						ASSERT_TRUE(false) << "rsslRead failed. Return code:" << readRet << " Error info: " << err.text;
					}
				}
			} while (readRet > 0);

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

		while (!shutdownTest)
			continue;

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

	if(server == NULL)
		std::cout << "Could not start rsslServer.  Error text: " << err.text << std::endl;

	return server;

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

class GlobalLockTests : public ::testing::Test {
protected:
	RsslChannel* serverChannel;
	RsslChannel* clientChannel;
	RsslServer* server;
	int msgsRead;
	RsslMutex readLock;

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

		serverChannel = serverChnl.pChnl;
		clientChannel = clientOpts.pChnl;

		if (!serverChannel || !clientChannel || serverChannel->state != RSSL_CH_STATE_ACTIVE || clientChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ASSERT_TRUE(false) << "Channel creation failed!";
		}
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

int main(int argc, char* argv[])
{
	int ret;
	::testing::InitGoogleTest(&argc, argv);
	RsslThreadId dlThread;
	RSSL_MUTEX_INIT(&pipeLock);

	RSSL_THREAD_START(&dlThread, deadlockThread, &dlThread);

	ret =  RUN_ALL_TESTS();
	testComplete = true;
	resetDeadlockTimer();
	RSSL_THREAD_JOIN(dlThread);
	
	return ret;
}
