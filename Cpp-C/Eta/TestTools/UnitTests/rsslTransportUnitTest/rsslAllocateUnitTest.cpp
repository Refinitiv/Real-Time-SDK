/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 /************************************************************************
  *	 Server allocate/deallocate Unit Test
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

const char* pServerKey = "localhost.key";
const char* pServerCert = "localhost.crt";
const char* pOpenSSLCAStore = "RootCA.crt";

class ServerStartStopTests : public ::testing::Test {
protected:
	RsslBool _testBlockingIO;
	RsslConnectionTypes	_testConnectionType;

	virtual void SetUp()
	{
		_testBlockingIO = RSSL_FALSE;
		_testConnectionType = RSSL_CONN_TYPE_SOCKET;
	}

	virtual void TearDown()
	{
		rsslUninitialize();
	}
};

/* Run Server on a standard TCP socket
 * Test should verify:
 * 1) Server creating stage (rsslBind).
 * - Allocates rsslServerImpl instance - using the pool of preallocated objects FreeServerList;
 * - Allocates RsslServerSocketChannel - using the pool of preallocated objects FreeServerSocketChannelList.
 * 2) Server close stage (rsslCloseServer).
 * - Deallocates rsslServerImpl instance;
 * - Deallocates RsslServerSocketChannel instance;
 * - Calls close_socket API for RsslServerSocketChannel->stream.
 */
TEST_F(ServerStartStopTests, ServerTCPStartStopTest)
{
	RsslServer* server = NULL;
	RsslError err;
	TUServerConfig serverConfig;
	rsslServerCountersInfo* pServerCountersInfo;

	rsslInitialize(RSSL_LOCK_NONE, &err);

	_testConnectionType = RSSL_CONN_TYPE_SOCKET;

	clearTUServerConfig(&serverConfig);
	serverConfig.blocking = _testBlockingIO;
	serverConfig.connType = _testConnectionType;
	strncpy(serverConfig.portNo, "15010", sizeof(serverConfig.portNo));

	server = bindRsslServer(&serverConfig);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";

	pServerCountersInfo = rsslGetServerCountersInfo(server);

	EXPECT_EQ(1, pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(9, pServerCountersInfo->countOfFreeServerList);
	EXPECT_EQ(1, pServerCountersInfo->countOfActiveServerSocketChannelList);
	EXPECT_EQ(9, pServerCountersInfo->countOfFreeServerSocketChannelList);

	RsslRet ret = rsslCloseServer(server, &err);

	EXPECT_EQ(RSSL_RET_SUCCESS, ret);

	EXPECT_EQ(0,  pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(10, pServerCountersInfo->countOfFreeServerList);
	EXPECT_EQ(0,  pServerCountersInfo->countOfActiveServerSocketChannelList);
	EXPECT_EQ(10, pServerCountersInfo->countOfFreeServerSocketChannelList);
}

TEST_F(ServerStartStopTests, ServerTCPStartStop100Test)
{
	RsslError err;
	const int NumConfigs = 100;
	const int basePortIndex = 15100;
	RsslServer* server[NumConfigs];
	TUServerConfig serverConfig[NumConfigs];
	int i;
	RsslRet ret;

	rsslServerCountersInfo* pServerCountersInfo;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	// Run the first 10 servers
	// Instances for the first 10 servers have already created and pushed into queues:
	// rsslServerImpl -> FreeServerList
	// RsslServerSocketChannel -> FreeServerSocketChannelList
	for (i = 0; i < 10; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		server[i] = NULL;
		server[i] = bindRsslServer( &(serverConfig[i]) );
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed!";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 1 .. 10
		// count of free list should be: 9 .. 0
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ( (10 - (i + 1)), pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ( (10 - (i + 1)), pServerCountersInfo->countOfFreeServerSocketChannelList);
	}

	// All the next instances (more than 10) are not allocated now
	// and will create when a server run
	for (i = 10; i < NumConfigs; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		server[i] = NULL;
		server[i] = bindRsslServer(&(serverConfig[i]));
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed!";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 11 .. NumConfigs
		// count of free list should be: 0
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ( 0, pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ( 0, pServerCountersInfo->countOfFreeServerSocketChannelList);
	}

	// Verify close server
	for (i = 0; i < NumConfigs; ++i)
	{
		ret = rsslCloseServer(server[i], &err);
		EXPECT_EQ(RSSL_RET_SUCCESS, ret);

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: (NumConfigs-1) .. 0
		// count of free list should be: 1 .. NumConfigs
		EXPECT_EQ( (NumConfigs - (i + 1)), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ( (NumConfigs - (i + 1)), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ( (i + 1), pServerCountersInfo->countOfFreeServerSocketChannelList);
	}
}


const char* getPathServerKey()
{
	return pServerKey;
}

const char* getPathServerCert()
{
	return pServerCert;
}

const char* getOpenSSLCAStore()
{
	return pOpenSSLCAStore;
}

bool checkCertificateFiles()
{
	struct stat buffer;
	bool isExistServerKey = (stat(pServerKey, &buffer) == 0);
	bool isExistServerCert = (stat(pServerCert, &buffer) == 0);
	return (isExistServerKey && isExistServerCert);
}

bool checkClientCertificateFiles()
{
	struct stat buffer;
	bool isExistOpenSSLCAStore = (stat(pOpenSSLCAStore, &buffer) == 0);
	return (isExistOpenSSLCAStore);
}

/* Run Server on a encrypted socket
 * Test should verify:
 * 1) Server creating stage (rsslBind).
 * - Allocates rsslServerImpl instance - using the pool of preallocated objects FreeServerList;
 * - Allocates RsslServerSocketChannel - using the pool of preallocated objects FreeServerSocketChannelList.
 * 2) Server close stage (rsslCloseServer).
 * - Deallocates rsslServerImpl instance;
 * - Deallocates RsslServerSocketChannel instance;
 * - Calls close_socket API for RsslServerSocketChannel->stream;
 * +++ Encrypted socket specific +++
 * - Calls freeSSLServer method for RsslServerSocketChannel->transportInfo.
 */
TEST_F(ServerStartStopTests, ServerSSLStartStopTest)
{
	RsslServer* server = NULL;
	RsslError err;
	TUServerConfig serverConfig;

	rsslServerCountersInfo* pServerCountersInfo;

	//DWORD length = 1024;
	//char* temp = new char[length];
	//DWORD dwRet = GetCurrentDirectory(length, temp);

	// Run Server on a encrypted socket
	_testConnectionType = RSSL_CONN_TYPE_ENCRYPTED;

	rsslInitialize(RSSL_LOCK_NONE, &err);

	clearTUServerConfig(&serverConfig);
	serverConfig.blocking = _testBlockingIO;
	serverConfig.connType = _testConnectionType;
	strncpy(serverConfig.portNo, "15020", sizeof(serverConfig.portNo));

	snprintf(serverConfig.serverKey, sizeof(serverConfig.serverKey), "%s", pServerKey);
	snprintf(serverConfig.serverCert, sizeof(serverConfig.serverCert), "%s", pServerCert);
	snprintf(serverConfig.cipherSuite, sizeof(serverConfig.cipherSuite), "%s", "");

	server = bindRsslServer(&serverConfig);
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed! Creation of server on an encrypted connection"
		<< " requires key-file \"" << pServerKey << "\" and certificate-file \"" << pServerCert << "\".";

	pServerCountersInfo = rsslGetServerCountersInfo(server);

	EXPECT_EQ(1, pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(9, pServerCountersInfo->countOfFreeServerList);
	EXPECT_EQ(1, pServerCountersInfo->countOfActiveServerSocketChannelList);
	EXPECT_EQ(9, pServerCountersInfo->countOfFreeServerSocketChannelList);

	RsslUInt32 numReleaseSSLServer0 = pServerCountersInfo->numberCallsOfReleaseSSLServer;

	RsslRet ret = rsslCloseServer(server, &err);

	EXPECT_EQ(RSSL_RET_SUCCESS, ret);

	EXPECT_EQ(0, pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(10, pServerCountersInfo->countOfFreeServerList);
	EXPECT_EQ(0, pServerCountersInfo->countOfActiveServerSocketChannelList);
	EXPECT_EQ(10, pServerCountersInfo->countOfFreeServerSocketChannelList);

	EXPECT_EQ(numReleaseSSLServer0 + 1, pServerCountersInfo->numberCallsOfReleaseSSLServer);
}

TEST_F(ServerStartStopTests, ServerSSLStartStop100Test)
{
	RsslError err;
	const int NumConfigs = 100;
	const int basePortIndex = 15300;
	RsslServer* server[NumConfigs];
	TUServerConfig serverConfig[NumConfigs];
	int i;
	RsslRet ret;

	rsslServerCountersInfo* pServerCountersInfo;

	// Run Server on a encrypted socket
	_testConnectionType = RSSL_CONN_TYPE_ENCRYPTED;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	// Run the first 10 servers
	// Instances for the first 10 servers have already created and pushed into queues:
	// rsslServerImpl -> FreeServerList
	// RsslServerSocketChannel -> FreeServerSocketChannelList
	for (i = 0; i < 10; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		snprintf(serverConfig[i].serverKey, sizeof(serverConfig[i].serverKey), "%s", pServerKey);
		snprintf(serverConfig[i].serverCert, sizeof(serverConfig[i].serverCert), "%s",  pServerCert);
		snprintf(serverConfig[i].cipherSuite, sizeof(serverConfig[i].cipherSuite), "%s", "");

		server[i] = NULL;
		server[i] = bindRsslServer(&(serverConfig[i]));
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed! Creation of server on an encrypted connection"
			<< " requires key-file \"" << pServerKey << "\" and certificate-file \"" << pServerCert << "\".";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 1 .. 10
		// count of free list should be: 9 .. 0
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ((10 - (i + 1)), pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ((10 - (i + 1)), pServerCountersInfo->countOfFreeServerSocketChannelList);
	}

	// All the next instances (more than 10) are not allocated now
	// and will create when a server run
	for (i = 10; i < NumConfigs; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		snprintf(serverConfig[i].serverKey, sizeof(serverConfig[i].serverKey), "%s", pServerKey);
		snprintf(serverConfig[i].serverCert, sizeof(serverConfig[i].serverCert), "%s", pServerCert);
		snprintf(serverConfig[i].cipherSuite, sizeof(serverConfig[i].cipherSuite), "%s", "");

		server[i] = NULL;
		server[i] = bindRsslServer(&(serverConfig[i]));
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed! Creation of server on an encrypted connection"
			<< " requires key-file \"" << pServerKey << "\" and certificate-file \"" << pServerCert << "\".";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 11 .. NumConfigs
		// count of free list should be: 0
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ(0, pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ(0, pServerCountersInfo->countOfFreeServerSocketChannelList);
	}

	RsslUInt32 numReleaseSSLServer0 = pServerCountersInfo->numberCallsOfReleaseSSLServer;

	// Verify close server
	for (i = 0; i < NumConfigs; ++i)
	{
		ret = rsslCloseServer(server[i], &err);
		EXPECT_EQ(RSSL_RET_SUCCESS, ret);

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: (NumConfigs-1) .. 0
		// count of free list should be: 1 .. NumConfigs
		EXPECT_EQ((NumConfigs - (i + 1)), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfFreeServerList);
		EXPECT_EQ((NumConfigs - (i + 1)), pServerCountersInfo->countOfActiveServerSocketChannelList);
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfFreeServerSocketChannelList);

		EXPECT_EQ(numReleaseSSLServer0 + (i + 1), pServerCountersInfo->numberCallsOfReleaseSSLServer);
	}
}

/* Run Server on a shared memory connection
 * Test should verify:
 * 1) Server creating stage (rsslBind).
 * - Allocates rsslServerImpl instance - using the pool of preallocated objects FreeServerList;
 * 2) Server close stage (rsslCloseServer).
 * - Deallocates rsslServerImpl instance;
 * +++ Shared memory specific +++
 * - Calls rtrShmTransDestroy method for rsslServerImpl->trasportInfo.
 */
TEST_F(ServerStartStopTests, ServerSHMemStartStopTest)
{
	RsslServer* server = NULL;
	RsslError err;
	TUServerConfig serverConfig;

	rsslServerCountersInfo* pServerCountersInfo;

	//DWORD length = 1024;
	//char* temp = new char[length];
	//DWORD dwRet = GetCurrentDirectory(length, temp);

	// Run Server a shared memory connection
	_testConnectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;

	rsslInitialize(RSSL_LOCK_NONE, &err);

	clearTUServerConfig(&serverConfig);
	serverConfig.blocking = _testBlockingIO;
	serverConfig.connType = _testConnectionType;
	strncpy(serverConfig.portNo, "15037", sizeof(serverConfig.portNo));

	server = bindRsslServer(&serverConfig);
#if defined(_WIN32)
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed!";
#else
	ASSERT_NE(server, (RsslServer*)NULL) << "Server creation failed! Linux Shared Memory test requires root privilege.";
#endif

	pServerCountersInfo = rsslGetServerCountersInfo(server);

	EXPECT_EQ(1, pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(9, pServerCountersInfo->countOfFreeServerList);

	RsslUInt32 numCallsOfShmTransDestroy0 = pServerCountersInfo->numberCallsOfShmTransDestroy;

	RsslRet ret = rsslCloseServer(server, &err);

	EXPECT_EQ(RSSL_RET_SUCCESS, ret);

	EXPECT_EQ(0, pServerCountersInfo->countOfActiveServerList);
	EXPECT_EQ(10, pServerCountersInfo->countOfFreeServerList);

	EXPECT_EQ(numCallsOfShmTransDestroy0 + 1, pServerCountersInfo->numberCallsOfShmTransDestroy);
}

#if defined(_WIN32)
TEST_F(ServerStartStopTests, ServerSHMemStartStop100Test)
{
	RsslError err;
	const int NumConfigs = 100;
	const int basePortIndex = 15500;
	RsslServer* server[NumConfigs];
	TUServerConfig serverConfig[NumConfigs];
	int i;
	RsslRet ret;

	rsslServerCountersInfo* pServerCountersInfo;

	// Run Server a shared memory connection
	_testConnectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;

	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &err);

	// Run the first 10 servers
	// Instances for the first 10 servers have already created and pushed into queues:
	// rsslServerImpl -> FreeServerList
	for (i = 0; i < 10; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		snprintf(serverConfig[i].serverKey, sizeof(serverConfig[i].serverKey), "localhost.key");
		snprintf(serverConfig[i].serverCert, sizeof(serverConfig[i].serverCert), "localhost.crt");
		snprintf(serverConfig[i].cipherSuite, sizeof(serverConfig[i].cipherSuite), "%s", "");

		server[i] = NULL;
		server[i] = bindRsslServer(&(serverConfig[i]));
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed!";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 1 .. 10
		// count of free list should be: 9 .. 0
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ((10 - (i + 1)), pServerCountersInfo->countOfFreeServerList);
	}

	// All the next instances (more than 10) are not allocated now
	// and will create when a server run
	for (i = 10; i < NumConfigs; ++i)
	{
		clearTUServerConfig(&serverConfig[i]);
		serverConfig[i].blocking = _testBlockingIO;
		serverConfig[i].connType = _testConnectionType;
		snprintf(serverConfig[i].portNo, sizeof(serverConfig[i].portNo), "%d", (basePortIndex + i));

		snprintf(serverConfig[i].serverKey, sizeof(serverConfig[i].serverKey), "localhost.key");
		snprintf(serverConfig[i].serverCert, sizeof(serverConfig[i].serverCert), "localhost.crt");
		snprintf(serverConfig[i].cipherSuite, sizeof(serverConfig[i].cipherSuite), "%s", "");

		server[i] = NULL;
		server[i] = bindRsslServer(&(serverConfig[i]));
		ASSERT_NE(server[i], (RsslServer*)NULL) << "Server creation failed!";

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: 11 .. NumConfigs
		// count of free list should be: 0
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ(0, pServerCountersInfo->countOfFreeServerList);
	}

	RsslUInt32 numCallsOfShmTransDestroy0 = pServerCountersInfo->numberCallsOfShmTransDestroy;

	// Verify close server
	for (i = 0; i < NumConfigs; ++i)
	{
		ret = rsslCloseServer(server[i], &err);
		EXPECT_EQ(RSSL_RET_SUCCESS, ret);

		pServerCountersInfo = rsslGetServerCountersInfo(server[i]);

		// count of active list should be: (NumConfigs-1) .. 0
		// count of free list should be: 1 .. NumConfigs
		EXPECT_EQ((NumConfigs - (i + 1)), pServerCountersInfo->countOfActiveServerList);
		EXPECT_EQ((i + 1), pServerCountersInfo->countOfFreeServerList);

		EXPECT_EQ(numCallsOfShmTransDestroy0 + (i + 1), pServerCountersInfo->numberCallsOfShmTransDestroy);
	}
}
#endif
