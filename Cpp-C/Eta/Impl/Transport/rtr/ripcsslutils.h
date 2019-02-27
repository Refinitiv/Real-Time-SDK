/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
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


//TODO: Figure out if we need this.
#ifdef LINUX
#include <unistd.h>
#include <dlfcn.h>
#endif

OPENSSL_DH *ripcSSLDHGetTmpParam(RsslInt32 nKeyLen, ripcSSLApiFuncs* sslFuncs , ripcCryptoApiFuncs* cryptoFuncs);
OPENSSL_DH *ripcSSLDHGetParamFile(char *file, ripcSSLApiFuncs* sslFuncs, ripcCryptoApiFuncs* cryptoFuncs );

// define the cipher list - this will allow all
// ciphers except anonymous (ADH), keys of 64 or 56 bits (LOW), 
// keys that are export limited (EXP) - this may need to change depending on export laws
// any MD5 keys (MD5) and then sets the strength keyword which sorts and orders keys based on strength
#define CIPHER_LIST "ALL:!LOW:!EXP:!MD5:@STRENGTH"

/* define our different types of key exchange - either diffie-hellman or RSA */
#define RIPC_SSL_DH512		0
#define RIPC_SSL_DH1024		1
#define RIPC_SSL_DH2048		2
#define RIPC_SSL_MAX_KEY	RIPC_SSL_DH2048

typedef enum {
	ripcSSLNone   = 0,  /* dont require any certificate */
	ripcSSLRequireCA = 1  /* require a client certificate */
} ripcSSLVerifyTypes;

/* SSL Configuration structure */
typedef struct 
{
	ripcSSLVerifyTypes verifyLevel;
	RsslInt32 verifyDepth;
	RsslInt32 blocking;
	RsslUInt32 address;
	RsslUInt32 id;
	RsslUInt16 pID; 
	char* cert_file;	/* certificate file */
	char* key_file;		/* file containing DH key if we want to use a different one */
	char* CApath;		/* Certificate Authority certificate path */
	char* CAfile;		/* Certificate Authority file  */
	char* cipher;		/* cipher list - defaults to CIPHER_LIST */
} ripcSSLConnectOpts;

/* architecture has said that Dacs should provide the user authentication and we should not need certificate exchange */
#define RIPC_INIT_SSL_CONNECT_OPTS { ripcSSLNone, 9, 0, 0, 0, 0, 0, 0, 0, 0, CIPHER_LIST }

/* our SSL Key structure */
typedef struct {
	RsslUInt32 nData;  // I dont think we need this
	RsslUInt8 *cipherData;  // I dont think we need this
	void *key;
} ripcSSLKey;

#define RIPC_INIT_SSL_KEY { 0, 0, 0 }

/* our Server structure */
typedef struct {
	RsslSocket socket;
	OPENSSL_SSL_CTX *ctx;
	ripcSSLConnectOpts  config;  // this is used for the case where we need to check the config off the server
	ripcSSLKey		keys[RIPC_SSL_MAX_KEY + 1];	
} ripcSSLServer;

#define RIPC_INIT_SSL_SERVER { 0, 0, RIPC_INIT_SSL_CONNECT_OPTS, 0 }

typedef enum
{
	SSL_INITIALIZING		= 0,
	SSL_WAITING_FOR_ACK		= 1,
	SSL_ACTIVE				= 2
} ripcSSLStates;


/* our client structure */
typedef struct {
	RsslSocket socket;
	ripcSSLServer *server;  // used to keep track of which server this session came from (if its server side)
	OPENSSL_SSL_CTX		  *ctx;  
	OPENSSL_SSL			  *connection;
	OPENSSL_BIO			  *bio;
	char		   clientConnState;
	ripcSSLProtocolFlags sessionProtocol;
	ripcSSLConnectOpts  config;  // this holds the config for the clients (if this is server side, the config is copied from the servers 
} ripcSSLSession;

#define RIPC_INIT_SSL_SESSION { 0, 0, 0, 0, 0, SSL_INITIALIZING, RIPC_INIT_SSL_CONNECT_OPTS }

ripcSSLProtocolFlags ripcGetSupportedProtocolFlags();

/* our transport read function -
 this will read from the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLRead( void *sslSess, char *buf, RsslInt32 max_len, ripcRWFlags flags, RsslError *error );

/* our transport write function -
   this will write to the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLWrite( void *sslSess, char *buf, RsslInt32 len, ripcRWFlags flags, RsslError *error);

/* shutdown the SSL and the socket */
RsslInt32 ripcShutdownSSLSocket(void *session);

/* accept a connecting socket */
RsslInt32 ripcSSLAccept(void *session,  ripcSessInProg *inPr, RsslError *error);
/* creates a new server side session */
void *ripcNewSSLSocket(void *server, RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error);

/* creates and initializes new server structure */
ripcSSLServer *ripcInitializeSSLServer(RsslSocket fd, char* name, RsslError  *error);

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

/* returns 0 for failure and 1 for success */
RsslInt32 ripcInitSSLConnectOpts(ripcSSLConnectOpts *config, RsslError *error);

ripcSSLServer *ripcSSLNewServer(RsslSocket fd, char* name, RsslError *error);

/* server can be null if this is a client side connection */
ripcSSLSession *ripcSSLNewSession(RsslSocket fd, RsslSocketChannel* chnl, ripcSSLServer *server, RsslError *error);

void ripcFreeSSLConnectOpts(ripcSSLConnectOpts *config);

RSSL_API RsslBool ripcCompareHostNameLen(char* host1, char* host2, unsigned int length);

RSSL_API RsslBool ripcVerifyCertHost(char* inputPattern, unsigned int patternLen, char* hostName, RsslError* err);

/* initialize the temporary DH keys */
RsslInt32 ripcInitKeys(ripcSSLServer *server, RsslError *error);

void ripcFreeKeys(ripcSSLServer *server);

//user is either 0 for client or 1 for server - we set up a few options differently for the server
OPENSSL_SSL_CTX* ripcSSLSetupCTX(RsslInt32 user,  ripcSSLProtocolFlags version, RsslSocketChannel* chnl, ripcSSLConnectOpts *config, RsslError *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // __ripcsslutils_h

