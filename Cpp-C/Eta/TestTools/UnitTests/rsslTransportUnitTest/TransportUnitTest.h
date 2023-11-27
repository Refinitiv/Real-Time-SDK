/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's
 * LICENSE.md for details.
 * Copyright (C) 2020-2023 Refinitiv. All rights reserved.
*/

/* TransportUnitTest.h
 * Configures the Transport unit tests application. */

#pragma once

#ifndef _TRANSPORT_UNIT_TEST_H_
#define _TRANSPORT_UNIT_TEST_H_

#include "rtr/rsslChanManagement.h"
#include "rtr/rsslEventSignal.h"

rsslServerCountersInfo* rsslGetServerCountersInfo(RsslServer* pServer);

/* Provides configuration options for creating server in Transport Unit-test */
typedef struct {
	RsslBool			blocking;
	RsslUInt32			maxFragmentSize;
	char				portNo[32];			/* Port number. Service name */
	RsslConnectionTypes	connType;			/* Connection type for this provider */
	char				wsProtocolList[64];	/* List of WebSocket supported/preferred protocols */

	char				serverCert[64];		/* Server certificate file location */
	char				serverKey[64];		/* Server private key file location */
	char				cipherSuite[64];	/* Server cipher suite */

	RsslCompTypes		compressionType;	/* Type of the compression */
	RsslUInt32			compressionLevel;	/* The compression level. Currently only zlib supports */
} TUServerConfig;

void clearTUServerConfig(TUServerConfig* pServerConfig);
void constructTUServerConfig(
	int configIndex,
	TUServerConfig& serverConfig,
	RsslBool blocking,
	RsslConnectionTypes connType,
	RsslCompTypes compressType,
	RsslUInt32 compressLevel
);

RsslServer* bindRsslServer(TUServerConfig* pServerConfig);

bool checkCertificateFiles();
bool checkClientCertificateFiles();

const char* getPathServerKey();   // describes the path to Server key for creating a server on an encrypted connection
const char* getPathServerCert();  // describes the path to Server certificate for creating a server on an encrypted connection
const char* getOpenSSLCAStore();  // describes the path to the CAStore: certificate for creating a client on an encrypted connection

class GLobalLockFragmentedTestParams {
protected:
	GLobalLockFragmentedTestParams() {};

public:
	RsslConnectionTypes		connType;		// the connection type
	RsslUInt32				msgLength;		// the specific value for length of each separated message
	RsslUInt32				maxFragmentSize;// the max fragment size before fragmentation, 0 - default
	RsslUInt32				msgWriteCount;	// the count of writing messages in this test, 0 - default, MAX_MSG_WRITE_COUNT
	RsslConnectionTypes		encryptedProtocol;
	RsslUInt8				wsProtocolType;
	RsslCompTypes			compressionType;	// the compression type for the connection.
	RsslUInt32				compressionLevel;	// the compression level. Currently only zlib supports.

	GLobalLockFragmentedTestParams(
		RsslConnectionTypes cnType, RsslUInt32 msgLen, RsslUInt32 maxFragSz, RsslUInt32 msgCount,
		RsslConnectionTypes encrProt, RsslUInt8 wsProt,
		RsslCompTypes compressType = RSSL_COMP_NONE, RsslUInt32 compressLevel = 0U)
		:
		connType(cnType),
		msgLength(msgLen),
		maxFragmentSize(maxFragSz),
		msgWriteCount(msgCount),
		encryptedProtocol(encrProt),
		wsProtocolType(wsProt),
		compressionType(compressType),
		compressionLevel(compressLevel)
	{};

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend std::ostream& operator<<(std::ostream& out, const GLobalLockFragmentedTestParams& params)
	{
		out << "["
			"connType:" << params.connType << ","
			"msgLength:" << params.msgLength << ","
			"maxFragmentSize:" << params.maxFragmentSize << ","
			"msgWriteCount:" << params.msgWriteCount << ","
			"encryptedProtocol:" << params.encryptedProtocol << ","
			"wsProtocolType:" << (unsigned)params.wsProtocolType << ","
			"compressionType:" << params.compressionType << ","
			"compressionLevel:" << params.compressionLevel
			<< "]";
		return out;
	}
};

class GLobalLockSystemTestParams {
public:
	int						configIndex;		// test buffer configuartion index

	RsslConnectionTypes		connType;			// the connection type
	RsslUInt32				msgLength;			// the specific value for length of each separated message
	RsslUInt32				maxFragmentSize;	// the max fragment size before fragmentation, 0 - default
	RsslUInt32				msgWriteCount;		// the count of writing messages in this test, 0 - default, MAX_MSG_WRITE_COUNT
	RsslConnectionTypes		encryptedProtocol;
	RsslUInt8				wsProtocolType;
	RsslCompTypes			compressionType;	// the compression type for the connection.
	RsslUInt32				compressionLevel;	// the compression level. Currently only zlib supports.

	GLobalLockSystemTestParams(
		int configTestBufferIndex,
		RsslConnectionTypes cnType,
		RsslUInt32 msgLen, RsslUInt32 maxFragSz, RsslUInt32 msgCount,
		RsslConnectionTypes encrProt, RsslUInt8 wsProt,
		RsslCompTypes compressType = RSSL_COMP_NONE, RsslUInt32 compressLevel = 0U)
		:
		configIndex(configTestBufferIndex),
		connType(cnType),
		msgLength(msgLen),
		maxFragmentSize(maxFragSz),
		msgWriteCount(msgCount),
		encryptedProtocol(encrProt),
		wsProtocolType(wsProt),
		compressionType(compressType),
		compressionLevel(compressLevel)
	{};

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend std::ostream& operator<<(std::ostream& out, const GLobalLockSystemTestParams& params)
	{
		out << "["
			"configIndex:" << params.configIndex << ","
			"connType:" << params.connType << ","
			"msgLength:" << params.msgLength << ","
			"maxFragmentSize:" << params.maxFragmentSize << ","
			"msgWriteCount:" << params.msgWriteCount << ","
			"encryptedProtocol:" << params.encryptedProtocol << ","
			"wsProtocolType:" << (unsigned)params.wsProtocolType << ","
			"compressionType:" << params.compressionType << ","
			"compressionLevel:" << params.compressionLevel
			<< "]";
		return out;
	}
};

/* Provides configuration options for creating a client in Transport Unit-test */
typedef struct {
	RsslBool			blocking;
	RsslUInt32			maxFragmentSize;
	char				portNo[32];			/* Port number. Service name */
	RsslConnectionTypes	connType;			/* Connection type for this provider */
	char				wsProtocolList[64];	/* List of WebSocket supported/preferred protocols */
	RsslCompTypes		compressionType;	/* Type of the compression */
	// RsslEncryptionOpts - Options for encrypted connections
	RsslUInt32			encryptionProtocolFlags;
	RsslConnectionTypes	encryptedProtocol;
	char				openSSLCAStore[64];	/* CAStore location */
} TUClientConfig;

void clearTUClientConfig(TUClientConfig* pTUClientConfig);
void constructTUClientConfig(
	int configIndex,
	TUClientConfig& clientConfig,
	RsslBool blocking,
	RsslConnectionTypes connType,
	RsslConnectionTypes encryptedConnType,
	RsslUInt8 wsProtocolType,
	RsslCompTypes compressType
);

/* Provides data structure for connection between Server and Client */
class TUConnection {
public:
	TUConnection() : pServerChannel(NULL), pClientChannel(NULL), pServer(NULL) {};

	RsslChannel*	pServerChannel;
	RsslChannel*	pClientChannel;
	RsslServer*		pServer;
};

void clearTUConnection(TUConnection* pConnection);

class EventSignal {
public:
	EventSignal()
	{
		RSSL_MUTEX_INIT(&lockEventSignal);
	}

	~EventSignal()
	{
		rsslCleanupEventSignal(&rsslEventSignal);
		RSSL_MUTEX_DESTROY(&lockEventSignal);
	}

	// invoke after rsslInitialize()
	void init()
	{
		rsslInitEventSignal(&rsslEventSignal);
	}

	/* Wait the signal with timeout
	*/
	RsslRet waitSignal(long wait_sec, long wait_usec)
	{
		RsslRet ret = RSSL_RET_SUCCESS;

		struct timeval selectTime;
		fd_set readfds;
		fd_set useread;

		FD_ZERO(&readfds);
		FD_ZERO(&useread);

		FD_SET(rsslGetEventSignalFD(&rsslEventSignal), &readfds);

		{
			selectTime.tv_sec = wait_sec;
			selectTime.tv_usec = wait_usec;

			useread = readfds;

			int selRet = select(FD_SETSIZE, &useread, NULL, NULL, &selectTime);

			if (selRet < 0)
			{
				ret = RSSL_RET_FAILURE;
			}
			else if (selRet == 0)
			{
				ret = RSSL_RET_READ_WOULD_BLOCK;
			}
			else
			{
				if (FD_ISSET(rsslEventSignal._fds[0], &useread))
				{
					RSSL_MUTEX_LOCK(&lockEventSignal);
					rsslResetEventSignal(&rsslEventSignal);
					RSSL_MUTEX_UNLOCK(&lockEventSignal);
					ret = RSSL_RET_SUCCESS;
				}
			}
		}
		return ret;
	}

	void setSignal()
	{
		RSSL_MUTEX_LOCK(&lockEventSignal);
		rsslSetEventSignal(&rsslEventSignal);
		RSSL_MUTEX_UNLOCK(&lockEventSignal);
	}

private:
	RsslEventSignal	rsslEventSignal;
	RsslMutex		lockEventSignal;
};

/* Handles received data and another events for running tests */
class TestHandler {
public:
	/* Handles the data buffer */
	/* pReadBuffer - pointer to the tested data buffer */
	/* Return: true - the data was processed successfully */
	/* false - when the test discovered the fail condition, the invoker should set Fail status */
	/* pErrorText  - the char array for the error text when detected Fail */
	/* szErrorText - the size of char array for the error text */
	virtual bool handleDataBuffer(RsslBuffer* pReadBuffer, char* pErrorText, size_t szErrorText) = 0;
};

#endif  // _TRANSPORT_UNIT_TEST_H_
