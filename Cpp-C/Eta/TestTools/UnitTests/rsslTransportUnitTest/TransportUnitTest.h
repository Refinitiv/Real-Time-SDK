/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's
 * LICENSE.md for details.
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

/* TransportUnitTest.h
 * Configures the Transport unit tests application. */

#pragma once

#ifndef _TRANSPORT_UNIT_TEST_H_
#define _TRANSPORT_UNIT_TEST_H_

#include "rtr/rsslChanManagement.h"

rsslServerCountersInfo* rsslGetServerCountersInfo(RsslServer* pServer);

/* Provides configuration options for creating server in Transport Unit-test */
typedef struct {
	RsslBool			blocking;
	char 				portNo[32];			/* Port number. Service name */
	RsslConnectionTypes	connType;			/* Connection type for this provider */
	char				serverCert[64];		/* Server certificate file location */
	char				serverKey[64];		/* Server private key file location */
	char				cipherSuite[64];	/* Server cipher suite */
} TUServerConfig;

void clearTUConfig(TUServerConfig* serverConfig);

RsslServer* bindRsslServer(TUServerConfig* serverConfig);

bool checkCertificateFiles();

const char* getPathServerKey();   // describes the path to Server key for creating a server on an encrypted connection
const char* getPathServerCert();  // describes the path to Server certificate for creating a server on an encrypted connection

#endif  // _TRANSPORT_UNIT_TEST_H_
