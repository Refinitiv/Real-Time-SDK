/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __ripcsslutils_h
#define __ripcsslutils_h

#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/os.h"
#include "rtr/ripch.h"
#include "rtr/ripcutils.h"
#include "rtr/ripcssljit.h"

#include <sys/types.h>

#define USE_SOCKETS


#ifdef __cplusplus
extern "C" {
#endif


#ifdef LINUX
#include <unistd.h>
#include <dlfcn.h>
#endif

// define the default cipher list.  This is taken from OWASP's 'B' tier recommendations of broad compatability as of June, 2019:
// https://github.com/OWASP/CheatSheetSeries/blob/master/cheatsheets/TLS_Cipher_String_Cheat_Sheet.md
#define CIPHER_LIST "DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ECDSA:!ADH:!IDEA:!3DES"

/* our Server structure */
typedef struct {
	RsslSocket socket;
	OPENSSL_SSL_CTX *ctx;
	RsslInt32 verifyDepth;
	RsslServerSocketChannel* chnl;
	char* cipherSuite;	
} ripcSSLServer;


typedef enum
{
	SSL_INITIALIZING		= 0,
	SSL_ACTIVE				= 2
} ripcSSLStates;


/* our client structure */
typedef struct {
	RsslSocket				socket;
	ripcSSLServer			*server;  // used to keep track of which server this session came from (if its server side)
	OPENSSL_SSL_CTX			*ctx;  
	OPENSSL_SSL				*connection;
	OPENSSL_BIO				*bio;
	char					clientConnState;
	RsslBool				blocking;
} ripcSSLSession;

ripcSSLProtocolFlags ripcGetSupportedProtocolFlags();

/* our transport read function -
 this will read from the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLRead( void *sslSess, char *buf, RsslInt32 max_len, ripcRWFlags flags, RsslError *error );

/* our transport write function -
   this will write to the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLWrite( void *sslSess, char *buf, RsslInt32 len, ripcRWFlags flags, RsslError *error);

/* shutdown the SSL and the socket with shutdown() */
RsslInt32 ripcShutdownSSLSocket(void *session);

/* shutdown SSL and close the socket with sock_close() */
RsslInt32 ripcCloseSSLSocket(void *session);

/* accept a connecting socket */
RsslInt32 ripcSSLInit(void *session,  ripcSessInProg *inPr, RsslError *error);

/* creates a new server side session */
void *ripcNewSSLSocket(void *server, RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* creates and initializes new server structure */
ripcSSLServer *ripcInitializeSSLServer(RsslServerSocketChannel* chnl, RsslError  *error);

/* creates and initializes new client/session structure */
ripcSSLSession *ripcInitializeSSLSession(RsslSocket fd, RsslInt32 SSLProtocolVersion, RsslSocketChannel* chnl, RsslError *error);

/* connects session to a server */
void *ripcSSLConnectTLSv1(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* connects session to a server */
void *ripcSSLConnectTLSv11(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* connects session to a server */
void *ripcSSLConnectTLSv12(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* connects session to a server */
void *ripcSSLConnectTLS(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* reconnects for proxy keep alive */
RsslInt32 ripcSSLReconnection(void *session, RsslError *error);

/* release the SSL Session */
RsslInt32 ripcReleaseSSLSession(void* session, RsslError *error);

/* release the SSL Server */
RsslInt32 ripcReleaseSSLServer(void* srvr, RsslError *error);

/* handle errors */
void ripcSSLErrors(RsslError *error, RsslInt32 initPos);

/* initialize the SSL library - we should only need to do this once per ripc initialization.  Also sets the function pointers. */
RsslInt32 ripcInitializeSSL(char* libsslName, char* libcryptoName);

void ripcUninitializeSSL(void);

ripcSSLProtocolFlags ripcGetSupportedSSLVersion();

ripcSSLProtocolFlags ripcRemoveHighestSSLVersionFlag(ripcSSLProtocolFlags protoFlags);

ripcSSLServer *ripcSSLNewServer(RsslServerSocketChannel* chnl, RsslError *error);

/* server can be null if this is a client side connection */
ripcSSLSession *ripcSSLNewSession(RsslSocket fd, RsslSocketChannel* chnl, ripcSSLServer *server, RsslError *error);


RSSL_API RsslBool ripcCompareHostNameLen(char* host1, char* host2, unsigned int length);

RSSL_API RsslBool ripcVerifyCertHost(char* inputPattern, unsigned int patternLen, char* hostName, RsslError* err);

OPENSSL_SSL_CTX* ripcSSLSetupCTXClient(ripcSSLProtocolFlags version, RsslSocketChannel* chnl, RsslError *error);

OPENSSL_SSL_CTX* ripcSSLSetupCTXServer(ripcSSLProtocolFlags version, RsslServerSocketChannel* chnl, RsslError *error);

RsslInt32 ripcGetCountInitializeSSLServer();

RsslInt32 ripcGetCountReleaseSSLServer();

ripcSSLApiFuncs* ripcGetOpenSSLApiFuncs();

ripcCryptoApiFuncs* ripcGetOpenSSLCryptoFuncs();



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __ripcsslutils_h

