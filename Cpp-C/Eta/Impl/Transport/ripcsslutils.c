/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef WIN32

#include "rtr/ripcsslutils.h"
#include "openssl/ssl.h"
#include "rtr/ripcflip.h"
#include "rtr/ripcssljit.h"
#include "rtr/rsslErrors.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/bn.h>

static ripcSSLApiFuncs sslFuncs = INIT_SSL_API_FUNCS;
static ripcSSLCTXApiFuncs ctxFuncs = INIT_SSL_CTX_FUNCS;
static ripcCryptoApiFuncs cryptoFuncs = INIT_CRYPTO_API_FUNCS;
static ripcSSLBIOApiFuncs bioFuncs = INIT_SSL_BIO_API_FUNCS;

/* This should only get populated once upon lib load.  If it's none, that's an error case */
static ripcSSLProtocolFlags supportedProtocols = RIPC_PROTO_SSL_NONE;

#ifdef WIN32
static HMODULE sslHandle = 0;
static HMODULE cryptoHandle = 0;
#else
static void* sslHandle = 0;
static void* cryptoHandle = 0;
#endif

#ifdef LINUX
static char* defaultSslLibName = "libssl.so.10";
static char* defaultCryptoLibName = "libcrypto.so.10";
#endif

RsslInt32 ripcInitializeSSL(char* libsslName, char* libcryptoName)
{
	char* sslLib;
	char* cryptoLib;
	RsslInt32 retVal;
	
	ripcTransportFuncs SSLfuncs;
	ripcSSLFuncs funcs; 

	supportedProtocols = RIPC_PROTO_SSL_NONE;

	if(libsslName == NULL)
		sslLib = defaultSslLibName;
	else
		sslLib = libsslName;
		
	if(libcryptoName == NULL)
		cryptoLib = defaultCryptoLibName;
	else
		cryptoLib = libcryptoName;
	if(sslHandle == 0)
	{
		if((sslHandle = RIPC_DLOPEN(sslLib)) == 0)
			return -1;
		
		if((sslFuncs.library_init = RIPC_DLSYM(sslHandle, "SSL_library_init")) == 0)
			goto sslLoadError;
		
		if((sslFuncs.load_error_strings = RIPC_DLSYM(sslHandle, "SSL_load_error_strings")) == 0)
			goto sslLoadError;
				
		if((sslFuncs.get_ex_data = RIPC_DLSYM(sslHandle, "SSL_get_ex_data")) == 0)
			goto sslLoadError;
		
		if((sslFuncs.TLSv1_client_method = RIPC_DLSYM(sslHandle, "TLSv1_client_method")) == 0)
			goto sslLoadError;
		else
			supportedProtocols |= RIPC_PROTO_SSL_TLS_V1;
			
		if((sslFuncs.TLSv1_1_client_method = RIPC_DLSYM(sslHandle, "TLSv1_1_client_method")) == 0)
			goto sslLoadError;
		else
			supportedProtocols |= RIPC_PROTO_SSL_TLS_V1_1;
		
		if((sslFuncs.TLSv1_2_client_method = RIPC_DLSYM(sslHandle, "TLSv1_2_client_method")) == 0)
			goto sslLoadError;
		else
			supportedProtocols |= RIPC_PROTO_SSL_TLS_V1_2;
			
		if((sslFuncs.ssl_read = RIPC_DLSYM(sslHandle, "SSL_read")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.get_error = RIPC_DLSYM(sslHandle, "SSL_get_error")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_write = RIPC_DLSYM(sslHandle, "SSL_write")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.set_shutdown = RIPC_DLSYM(sslHandle, "SSL_set_shutdown")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_accept = RIPC_DLSYM(sslHandle, "SSL_accept")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.get_verify_result = RIPC_DLSYM(sslHandle, "SSL_get_verify_result")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_state = RIPC_DLSYM(sslHandle, "SSL_state")) == 0)
			goto sslLoadError;
		
		if((sslFuncs.ssl_connect = RIPC_DLSYM(sslHandle, "SSL_connect")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_new = RIPC_DLSYM(sslHandle, "SSL_new")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_clear = RIPC_DLSYM(sslHandle, "SSL_clear")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.set_cipher_list = RIPC_DLSYM(sslHandle, "SSL_set_cipher_list")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.set_bio = RIPC_DLSYM(sslHandle, "SSL_set_bio")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ctrl = RIPC_DLSYM(sslHandle, "SSL_ctrl")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.set_connect_state = RIPC_DLSYM(sslHandle, "SSL_set_connect_state")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.set_ex_data = RIPC_DLSYM(sslHandle, "SSL_set_ex_data")) == 0)
			goto sslLoadError;
		
		if((sslFuncs.set_accept_state = RIPC_DLSYM(sslHandle, "SSL_set_accept_state")) == 0)
			goto sslLoadError;
			
		if((sslFuncs.ssl_free = RIPC_DLSYM(sslHandle, "SSL_free")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_new = RIPC_DLSYM(sslHandle, "SSL_CTX_new")) == 0)
			goto sslLoadError;
		
		if((ctxFuncs.ctx_set_quiet_shutdown = RIPC_DLSYM(sslHandle, "SSL_CTX_set_quiet_shutdown")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_set_cipher_list = RIPC_DLSYM(sslHandle, "SSL_CTX_set_cipher_list")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_load_verify_location = RIPC_DLSYM(sslHandle, "SSL_CTX_load_verify_locations")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_set_default_verify_paths = RIPC_DLSYM(sslHandle, "SSL_CTX_set_default_verify_paths")) == 0)
			goto sslLoadError;
		
		if((ctxFuncs.ctx_use_cert_chain_file = RIPC_DLSYM(sslHandle, "SSL_CTX_use_certificate_chain_file")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_use_privatekey_file = RIPC_DLSYM(sslHandle, "SSL_CTX_use_PrivateKey_file")) == 0)
			goto sslLoadError;

		if((ctxFuncs.ctx_set_verify = RIPC_DLSYM(sslHandle, "SSL_CTX_set_verify")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_ctrl = RIPC_DLSYM(sslHandle, "SSL_CTX_ctrl")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_set_tmp_dh_callback = RIPC_DLSYM(sslHandle, "SSL_CTX_set_tmp_dh_callback")) == 0)
			goto sslLoadError;
			
		if((ctxFuncs.ctx_free = RIPC_DLSYM(sslHandle, "SSL_CTX_free")) == 0)
			goto sslLoadError;
		
		if((ctxFuncs.ctx_set_ex_data = RIPC_DLSYM(sslHandle, "SSL_CTX_set_ex_data")) == 0)
			goto sslLoadError;
	}
	
	if(cryptoHandle == 0)
	{
		if((cryptoHandle = RIPC_DLOPEN(cryptoLib)) == 0)
			return -1;
		
		if((sslFuncs.load_crypto_strings = RIPC_DLSYM(cryptoHandle, "ERR_load_crypto_strings")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.err_free_strings = RIPC_DLSYM(cryptoHandle, "ERR_free_strings")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.v3_add_standard_extensions = RIPC_DLSYM(cryptoHandle, "X509V3_add_standard_extensions")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.thread_id = RIPC_DLSYM(cryptoHandle, "CRYPTO_thread_id")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.get_error_line_data = RIPC_DLSYM(cryptoHandle, "ERR_get_error_line_data")) == 0)
			goto cryptoLoadError;
		
		if((cryptoFuncs.error_string_n = RIPC_DLSYM(cryptoHandle, "ERR_error_string_n")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.read_bio_dhparams = RIPC_DLSYM(cryptoHandle, "PEM_read_bio_DHparams")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.rand_seed = RIPC_DLSYM(cryptoHandle, "RAND_seed")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.dh_free = RIPC_DLSYM(cryptoHandle, "DH_free")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.dh_new = RIPC_DLSYM(cryptoHandle, "DH_new")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.bin2bn = RIPC_DLSYM(cryptoHandle, "BN_bin2bn")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.X509_get_ex_data = RIPC_DLSYM(cryptoHandle, "X509_STORE_CTX_get_ex_data")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.get_current_cert = RIPC_DLSYM(cryptoHandle, "X509_STORE_CTX_get_current_cert")) == 0)
			goto cryptoLoadError;
		
		if((cryptoFuncs.get_error_depth = RIPC_DLSYM(cryptoHandle, "X509_STORE_CTX_get_error_depth")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.get_error = RIPC_DLSYM(cryptoHandle, "X509_STORE_CTX_get_error")) == 0)
			goto cryptoLoadError;
		
		if((cryptoFuncs.verify_cert_error_string = RIPC_DLSYM(cryptoHandle, "X509_verify_cert_error_string")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.name_oneline = RIPC_DLSYM(cryptoHandle, "X509_NAME_oneline")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.get_issuer_name = RIPC_DLSYM(cryptoHandle, "X509_get_issuer_name")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.get_subject_name = RIPC_DLSYM(cryptoHandle, "X509_get_subject_name")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.err_remove_state = RIPC_DLSYM(cryptoHandle, "ERR_remove_state")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.engine_cleanup = RIPC_DLSYM(cryptoHandle, "ENGINE_cleanup")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.crypto_cleanup_all_ex_data = RIPC_DLSYM(cryptoHandle, "CRYPTO_cleanup_all_ex_data")) == 0)
			goto cryptoLoadError;
			
		if((cryptoFuncs.evp_cleanup = RIPC_DLSYM(cryptoHandle, "EVP_cleanup")) == 0)
			goto cryptoLoadError;
			
		if((bioFuncs.new_file = RIPC_DLSYM(cryptoHandle, "BIO_new_file")) == 0)
			goto cryptoLoadError;
			
		if((bioFuncs.bio_free = RIPC_DLSYM(cryptoHandle, "BIO_free")) == 0)
			goto cryptoLoadError;
			
		if((bioFuncs.sock_should_retry = RIPC_DLSYM(cryptoHandle, "BIO_sock_should_retry")) == 0)
			goto cryptoLoadError;
			
		if((bioFuncs.new_socket = RIPC_DLSYM(cryptoHandle, "BIO_new_socket")) == 0)
			goto cryptoLoadError;
	}
		
	SSLfuncs.bindSrvr = 0;
	SSLfuncs.newSrvrConnection = ripcNewSSLSocket; //should be used after accept is called on the server side
	SSLfuncs.connectSocket = ipcConnectSocket;
	SSLfuncs.newClientConnection = 0;
	SSLfuncs.initializeTransport = ripcSSLAccept; // calls accept on server side
	SSLfuncs.shutdownTransport = ripcShutdownSSLSocket; // shuts down socket on client or server side
	SSLfuncs.readTransport = ripcSSLRead; // read function on client or server side
	SSLfuncs.writeTransport = ripcSSLWrite; // write function on client or server side
	SSLfuncs.writeVTransport = 0;
	SSLfuncs.reconnectClient = ripcSSLReconnection;
	SSLfuncs.acceptSocket = 0;
	SSLfuncs.shutdownSrvrError = 0;
	SSLfuncs.sessIoctl = 0;

	SSLfuncs.getSockName = 0;
	SSLfuncs.setSockOpts = 0;
	SSLfuncs.getSockOpts = 0;
	SSLfuncs.connected = 0;
	SSLfuncs.shutdownServer = 0;
	SSLfuncs.uninitialize = ripcUninitializeSSL;

	if((supportedProtocols & RIPC_PROTO_SSL_TLS_V1) != 0)
	{
		SSLfuncs.newClientConnection = ripcSSLConnectTLSv1; // calls SSL connect on client side

		retVal = ipcSetSSLTransFunc(RIPC_SSL_TLS_V1, &SSLfuncs);
		if(retVal == 0)
			goto cryptoLoadError;
	}
	
	if((supportedProtocols & RIPC_PROTO_SSL_TLS_V1_1) != 0)
	{
		SSLfuncs.newClientConnection = ripcSSLConnectTLSv11; // calls SSL connect on client side

		retVal = ipcSetSSLTransFunc(RIPC_SSL_TLS_V1_1, &SSLfuncs);
		if(retVal == 0)
			goto cryptoLoadError;
	}
	
	if((supportedProtocols & RIPC_PROTO_SSL_TLS_V1_2) != 0)
	{
		SSLfuncs.newClientConnection = ripcSSLConnectTLSv12; // calls SSL connect on client side

		retVal = ipcSetSSLTransFunc(RIPC_SSL_TLS_V1_2, &SSLfuncs);
		if(retVal == 0)
			goto cryptoLoadError;
	}
	
	funcs.newSSLServer = (void*)ripcInitializeSSLServer;
	funcs.freeSSLServer = ripcReleaseSSLServer;

	retVal = ipcSetSSLFuncs(&funcs);
	
	(*(sslFuncs.library_init))();

	(*(sslFuncs.load_error_strings))();
	(*(sslFuncs.load_crypto_strings))();

	(*(cryptoFuncs.v3_add_standard_extensions))();
	
	return 1;

cryptoLoadError:
	if(cryptoHandle != 0)
	{
		RIPC_DLCLOSE(cryptoHandle);
		cryptoHandle = 0;
	}
sslLoadError:
	if(sslHandle != 0)
	{
		RIPC_DLCLOSE(sslHandle);
		sslHandle = 0;
	}
	
	sslFuncs.initialized = 0;
	ctxFuncs.initialized = 0;
	cryptoFuncs.initialized = 0;
	bioFuncs.initialized = 0;
	return -1;
}

void ripcUninitializeSSL(void)
{
	if (cryptoHandle)
	{
		(*(cryptoFuncs.evp_cleanup))();
		(*(cryptoFuncs.engine_cleanup))();
		(*(cryptoFuncs.crypto_cleanup_all_ex_data))();
		(*(cryptoFuncs.err_remove_state))(0);
		(*(cryptoFuncs.err_free_strings))();
		RIPC_DLCLOSE(cryptoHandle);
		cryptoHandle = 0;
	}

	if(sslHandle != 0)
	{
		RIPC_DLCLOSE(sslHandle);
		sslHandle = 0;
	}
	
	sslFuncs.initialized = 0;
	ctxFuncs.initialized = 0;
	cryptoFuncs.initialized = 0;
	bioFuncs.initialized = 0;
}

void ripcSSLErrors(RsslError *error, RsslInt32 initPos)
{
	RsslUInt32 l = 0;
	char buf[1024];
	const char *file, *data;
	RsslInt32 line = 0;
	RsslInt32 flags = 0;
	RsslInt32 cUrl = initPos;
	RsslUInt32 es= (*(cryptoFuncs.thread_id))();

	buf[0] = '\0';

	while ((l=(*(cryptoFuncs.get_error_line_data))(&file, &line, &data, &flags)) != 0)
	{
		(*(cryptoFuncs.error_string_n))(l,buf,1024 - strlen(buf));
		if ((strlen(buf) + strlen(file) + cUrl + 30 ) > MAX_RSSL_ERROR_TEXT)
			break;
		
		snprintf(error->text + cUrl, MAX_RSSL_ERROR_TEXT,  "%u:%s:%s:%d:\n", es, buf, file, line);
		cUrl = strlen(error->text);
	}
}

ripcSSLProtocolFlags ripcGetSupportedSSLVersion()
{
	return supportedProtocols;
}

ripcSSLProtocolFlags ripcRemoveHighestSSLVersionFlag(ripcSSLProtocolFlags protoFlags)
{
	if((protoFlags & RIPC_PROTO_SSL_TLS_V1_2) != 0)
	{
		return (protoFlags & (~RIPC_PROTO_SSL_TLS_V1_2));
	}

	if((protoFlags & RIPC_PROTO_SSL_TLS_V1_1) != 0)
	{
		return (protoFlags & (~RIPC_PROTO_SSL_TLS_V1_1));
		}

	if((protoFlags & RIPC_PROTO_SSL_TLS_V1) != 0)
	{
		return (protoFlags & (~RIPC_PROTO_SSL_TLS_V1));
	}
}

// this is a temporary diffie-hellman callback that allows us
// to use ephemeral diffie-hellman (EDH) for key exchange.  
static DH *ripcDHCallback(SSL *ssl, RsslInt32 is_export, RsslInt32 keylength)
{
	DH *ret = 0;
	ripcSSLSession *sess = 0;

	sess = (ripcSSLSession*)(*(sslFuncs.get_ex_data))(ssl, 0);

	switch (keylength)
	{
	case 512:
			ret = (DH*)sess->server->keys[RIPC_SSL_DH512].key;
		break;
	
	case 1024:
			ret = (DH*)sess->server->keys[RIPC_SSL_DH1024].key;
		break;

	case 2048:
	default:
			ret = (DH*)sess->server->keys[RIPC_SSL_DH2048].key;
		break;
	}

	return ret;
}

RsslInt32 ripcInitSSLConnectOpts(ripcSSLConnectOpts *config, RsslError *error)
{
	config->CAfile = 0;
	config->CApath = 0;
	config->cert_file = 0;
	config->cipher = CIPHER_LIST;
	config->key_file = 0;
	config->verifyDepth = 9;
	config->verifyLevel = ripcSSLNone;
	config->blocking = 0;
	config->address = 0;
	config->id = 0;
	config->pID = 0;

	return 1;
}

ripcSSLServer *ripcSSLNewServer(RsslSocket fd, char* name, RsslError *error)
{
	RsslInt32 i = 0;
	ripcSSLServer *server = (ripcSSLServer*)_rsslMalloc(sizeof(ripcSSLServer));

	if (server == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Could not allocate space for ripcSSLServer.", __FILE__, __LINE__);
		return 0;
	}

	server->socket = (RsslUInt32)fd;
	server->ctx = 0;

	for (i = 0; i <= RIPC_SSL_MAX_KEY; i++)
	{
		server->keys[i].nData = 0;
		server->keys[i].cipherData = 0;
		server->keys[i].key = 0;
	}

	if (ripcInitSSLConnectOpts(&server->config, error) == 0)
	{
		_rsslFree(server);
		return 0;
	}

	return server;
}

/* server can be null if this is a client side connection */
ripcSSLSession *ripcSSLNewSession(RsslSocket fd, char* name, ripcSSLServer *server, RsslError *error)
{
	ripcSSLSession *session = (ripcSSLSession*)_rsslMalloc(sizeof(ripcSSLSession));

	if (session == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Could not allocate space for ripcSSLSession.", __FILE__, __LINE__);
		return 0;
	}

	session->socket = (RsslUInt32)fd;
	session->server = server;
	session->bio = 0;
	session->ctx = 0;
	session->connection = 0;
	session->clientConnState = SSL_INITIALIZING;
	
	/* we only need to initialize the connect opts for client side configs - if its 
	   a server side channel, just point it to the servers config */
	if (server == 0)
	{
		if (ripcInitSSLConnectOpts(&session->config, error) == 0)
		{
            _rsslFree(session);
			return 0;
		}
	}
	else
	{
		// set to point at servers config
		session->config.CAfile = session->server->config.CAfile;
		session->config.CApath = session->server->config.CApath;
		session->config.cert_file = session->server->config.cert_file;
		session->config.key_file = session->server->config.key_file;
		session->config.cipher = session->server->config.cipher;
		session->config.verifyDepth = session->server->config.verifyDepth;
		session->config.verifyLevel = session->server->config.verifyLevel;		
	}

	return session;
}

void ripcFreeSSLConnectOpts(ripcSSLConnectOpts *config)
{
	if (config->cert_file)
	{
		_rsslFree(config->cert_file);
		config->cert_file = 0;
	}

	if (config->key_file)
	{
		_rsslFree(config->key_file);
		config->key_file = 0;
	}

	if (config->CAfile)
	{
		_rsslFree(config->CAfile);
		config->CAfile = 0;
	}

	if (config->CApath)
	{
		_rsslFree(config->CApath);
		config->CApath = 0;
	}

	/*if (config->cipher && (config->cipher != CIPHER_LIST))
	{
		_rsslFree(config->cipher);
		config->cipher = 0;
	} */
}


static int ssl_rand_choosenum(int l, int h)
{
	int i = 0;
	char buf[50];
	srand((unsigned int)time(NULL));
	snprintf(buf, 50, "%.0f", (((double)(rand()%RAND_MAX)/RAND_MAX)*(h-l)));
	i = atoi(buf) + 1;
	if (i < 1)
		i = l;
	if (i > h)
		i = h;
	return i;
}

static void ripcSSLRandSeed()
{
	time_t t;
	int l = 0;
	unsigned short pid;
	unsigned char stackdata[256];

	/* seed in the current time */
	t = time(NULL);
	(*(cryptoFuncs.rand_seed))((unsigned char*)&t, sizeof(time_t));

	/* seed in the PID */
	pid = getpid();
	(*(cryptoFuncs.rand_seed))((unsigned char*)&pid, sizeof(unsigned short));

	/* seed in some current state of the run-time stack */
	l = ssl_rand_choosenum(0, sizeof(stackdata)-128-1);
	(*(cryptoFuncs.rand_seed))(stackdata + l, 128);
}

/* initialize the temporary DH keys */
RsslInt32 ripcInitKeys(ripcSSLServer *server, RsslError *error)
{
	DH *dh = 0;

	ripcSSLRandSeed();

	/* if they pass in a key file, use that instead */
	if (server->config.key_file)
	{
		/* set up diffie-hellman temp keys for EDH */
		if ((dh = ripcSSLDHGetParamFile(server->config.key_file, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 512 bit DH parameters from file %s.", __FILE__, __LINE__, server->config.key_file);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}
		server->keys[RIPC_SSL_DH512].key = (void*)dh;

		if ((dh = ripcSSLDHGetParamFile(server->config.key_file, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 1024 bit DH parameters from file %s.", __FILE__, __LINE__, server->config.key_file);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}
		server->keys[RIPC_SSL_DH1024].key = (void*)dh;

		if ((dh = ripcSSLDHGetParamFile(server->config.key_file, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 2048 bit DH parameters from file %s.", __FILE__, __LINE__, server->config.key_file);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}
		server->keys[RIPC_SSL_DH2048].key = (void*)dh;
	}
	else // no key passed in, using default keys from ripcssldh.c file 
	{
		/* set up diffie-hellman temp keys for EDH */
		if ((dh = ripcSSLDHGetTmpParam(512, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 512 bit DH parameters.", __FILE__, __LINE__);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}
		server->keys[RIPC_SSL_DH512].key = (void*)dh;

		if ((dh = ripcSSLDHGetTmpParam(1024, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 1024 bit DH parameters.", __FILE__, __LINE__);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}
		server->keys[RIPC_SSL_DH1024].key = (void*)dh;

		if ((dh = ripcSSLDHGetTmpParam(2048, &bioFuncs, &cryptoFuncs)) == NULL)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Failed to import temporary 2048 bit DH parameters.", __FILE__, __LINE__);
			ripcSSLErrors(error, strlen(error->text));
			return -1;
		}

		server->keys[RIPC_SSL_DH2048].key = (void*)dh;
	}

	return 1;
}

void ripcFreeKeys(ripcSSLServer *server)
{
	if (server->keys[RIPC_SSL_DH512].key)
	{
		(*(cryptoFuncs.dh_free))((DH*)server->keys[RIPC_SSL_DH512].key);
		server->keys[RIPC_SSL_DH512].key = 0;
	}
	if (server->keys[RIPC_SSL_DH1024].key)
	{
		(*(cryptoFuncs.dh_free))((DH*)server->keys[RIPC_SSL_DH1024].key);
		server->keys[RIPC_SSL_DH1024].key = 0;
	}
	if (server->keys[RIPC_SSL_DH2048].key)
	{
		(*(cryptoFuncs.dh_free))((DH*)server->keys[RIPC_SSL_DH2048].key);
		server->keys[RIPC_SSL_DH2048].key = 0;
	}
}

//user is either 0 for client or 1 for server - we set up a few options differently for the server
SSL_CTX* ripcSSLSetupCTX(RsslInt32 user, ripcSSLProtocolFlags version, ripcSSLConnectOpts *config, RsslError *error)
{
	SSL_CTX *ctx = 0;
	RsslInt32 retVal = 0;
	/* since we dont need to use certificates by default... */
	RsslInt32 perm = SSL_VERIFY_NONE;
	RsslInt32 opts = SSL_OP_ALL | SSL_OP_NO_SSLv2;

	switch(version)
	{
		case RIPC_PROTO_SSL_TLS_V1:
		if ((ctx = (*(ctxFuncs.ctx_new))( (*(sslFuncs.TLSv1_client_method))())) == NULL)
	{
		/* populate error  and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() failed to create new context (errno %d)", __FILE__,__LINE__,errno);
		ripcSSLErrors(error, strlen(error->text));
		return NULL;
	}
		break;
		case RIPC_PROTO_SSL_TLS_V1_1:
		if ((ctx = (*(ctxFuncs.ctx_new))( (*(sslFuncs.TLSv1_1_client_method))())) == NULL)
		{
			/* populate error  and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() failed to create new context (errno %d)", __FILE__,__LINE__,errno);
			ripcSSLErrors(error, strlen(error->text));
			return NULL;
		}
		break;
		case RIPC_PROTO_SSL_TLS_V1_2:
		if ((ctx = (*(ctxFuncs.ctx_new))( (*(sslFuncs.TLSv1_2_client_method))())) == NULL)
		{
			/* populate error  and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() failed to create new context (errno %d)", __FILE__,__LINE__,errno);
			ripcSSLErrors(error, strlen(error->text));
			return NULL;
		}
		break;
	}
		
	
	/* Turn off SSLv2 and SSLv3, since both are insecure.  See:
		https://www.openssl.org/~bodo/ssl-poodle.pdf
	*/

	(*(ctxFuncs.ctx_set_quiet_shutdown))(ctx, 1);

	//if ((retVal = SSL_CTX_set_cipher_list(ctx, config->cipher)) != 1)
	if ((retVal = (*(ctxFuncs.ctx_set_cipher_list))(ctx, CIPHER_LIST)) != 1)
	{
		/* populate error and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcSSLSetupCTX() error setting up cipher list (no valid ciphers) (errno %d)", __FILE__,__LINE__,errno);
		ripcSSLErrors(error, strlen(error->text));
		goto ripcsetupctxend;
	}

	// need to get CAFILE or CADIR from config 
	if (config->CAfile || config->CApath)
	{
		if ((retVal = (*(ctxFuncs.ctx_load_verify_location))(ctx, config->CAfile, config->CApath)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() error loading CA file (errno %d)", __FILE__,__LINE__,errno);
			ripcSSLErrors(error, strlen(error->text));
			goto ripcsetupctxend;
		}

		if ((retVal = (*(ctxFuncs.ctx_set_default_verify_paths))(ctx)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() error loading CA file (errno %d)", __FILE__,__LINE__,errno);
			ripcSSLErrors(error, strlen(error->text));
			goto ripcsetupctxend;
		}
	}
	
	/* need to get CERTFILE from config */
	if (config->cert_file)
	{
		if ((retVal = (*(ctxFuncs.ctx_use_cert_chain_file))(ctx, config->cert_file)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() error loading certificate from file %s", __FILE__,__LINE__,config->cert_file);
			ripcSSLErrors(error, strlen(error->text));
			goto ripcsetupctxend;
		}

		/* need to get CERTFILE from config */
		if ((retVal = (*(ctxFuncs.ctx_use_privatekey_file))(ctx, config->cert_file, SSL_FILETYPE_PEM)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLSetupCTX() error loading private key from file %s", __FILE__,__LINE__,config->cert_file);
			ripcSSLErrors(error, strlen(error->text));
			goto ripcsetupctxend;
		}
	}

	if (config->verifyLevel == ripcSSLRequireCA)
		perm |= SSL_VERIFY_PEER;

	/* set up options differently if we are server */
	if (user)
	{
		opts |= SSL_OP_SINGLE_DH_USE;
		if (config->verifyLevel == ripcSSLRequireCA)
			perm |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	}

	/* need to get CERTFILE from config */
	(*(ctxFuncs.ctx_set_verify))(ctx, perm, verify_callback);

	(*(ctxFuncs.ctx_ctrl))(ctx, SSL_CTRL_OPTIONS, opts,NULL);
	
	(*(ctxFuncs.ctx_set_tmp_dh_callback))(ctx, ripcDHCallback);

	return ctx;

ripcsetupctxend:
	_rsslSetError(error,0,RIPC_SYS_ERR,retVal);
	(*(ctxFuncs.ctx_free))(ctx);
	return NULL;
}

/* our transport read function -
 this will read from the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLRead( void *sslSess, char *buf, RsslInt32 max_len, ripcRWFlags flags, RsslError *error )
{
	RsslInt32 numBytes = 0;
	RsslInt32 totalBytes = 0;
	ripcSSLSession *sess = (ripcSSLSession*)sslSess;

	while(totalBytes < max_len)
	{
		numBytes = (*(sslFuncs.ssl_read))(sess->connection, (buf + totalBytes), (max_len - totalBytes));

		switch ((*(sslFuncs.get_error))(sess->connection, numBytes))
		{
			case SSL_ERROR_NONE:
				totalBytes += numBytes;
				if (sess->config.blocking)
					return totalBytes;
			break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_X509_LOOKUP:
					/* these are a would block/retry read situation */
				return totalBytes;
			break;
			case SSL_ERROR_SYSCALL:
				/* OpenSSL does provide a way to get last error(get_last_socket_error() */
				if(errno == EAGAIN || errno == EINTR)
					return totalBytes;
				else
					return (-1);
				break;
			case SSL_ERROR_SSL:
				error->text[0] = '\0';
				return (-1);
			break;
			case SSL_ERROR_ZERO_RETURN:
				/* this may actually be a transport error worthy of disconnection... */
				if (totalBytes)
					return totalBytes;
				else
				{
					error->text[0] = '\0';
					return (-2);
				}
			break;
		}
	}

	return totalBytes;
}

/* our transport write function -
   this will write to the network using SSL and return the appropriate value to the ripc layer */
RsslInt32 ripcSSLWrite( void *sslSess, char *buf, RsslInt32 len, ripcRWFlags flags, RsslError *error)
{
	RsslInt32 numBytes = 0;
	RsslInt32 totalOut = 0;
	ripcSSLSession *sess = (ripcSSLSession*)sslSess;

	while (totalOut < len)
	{
		numBytes = (*(sslFuncs.ssl_write))(sess->connection, (buf + totalOut), (len - totalOut));
	
		switch ((*(sslFuncs.get_error))(sess->connection, numBytes))
		{
			case SSL_ERROR_NONE:
				totalOut += numBytes;
				if (sess->config.blocking)
					return totalOut;
			break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_X509_LOOKUP:
				/* these are a would block/retry read situation */
				return totalOut;
			break;
			case SSL_ERROR_SYSCALL:
				/* OpenSSL does provide a way to get last error(get_last_socket_error() */
				if(errno == EAGAIN || errno == EINTR)
					return totalOut;
				else
					return (-1);
				break;
			case SSL_ERROR_SSL:
				error->text[0] = '\0';
				return (-1);
			break;
			case SSL_ERROR_ZERO_RETURN:
				/* this may actually be a transport error worthy of disconnection... */
				error->text[0] = '\0';
				return (-2);
			break;
		}
	}
	
	return totalOut;
}

RsslInt32 ripcShutdownSSLSocket(void *session)
{
	ripcSSLSession *sess = (ripcSSLSession*)session;

	/* close the socket */
	sock_close(sess->socket);
	/* close the SSL connection */
	(*(sslFuncs.set_shutdown))(sess->connection, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);

	ripcReleaseSSLSession(sess, 0);

	return 1;
}

RsslInt32 ripcSSLAccept(void *session, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32 retVal = 0;
	RsslUInt32 verify_error;
	ripcSSLSession *sess = (ripcSSLSession*)session;

	if (sess->server)
	{
		if ((retVal = (*(sslFuncs.ssl_accept))(sess->connection)) <= 0)
		{
			if ((*(bioFuncs.sock_should_retry))(retVal))
				return 0;

			error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

			if (sess->config.verifyLevel > ripcSSLNone)
			{
				verify_error = (*(sslFuncs.get_verify_result))(sess->connection);

				if (verify_error != X509_V_OK)
				{
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLAccept Verify Error: %s", __FILE__, __LINE__, (*(cryptoFuncs.verify_cert_error_string))(verify_error));
				}
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, error->sysError );
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLAccept Verify Error ", __FILE__, __LINE__);
					ripcSSLErrors(error, strlen(error->text));
					return -1;
				}
			}

			if ((retVal == -1) && ((error->sysError == SSL_ERROR_WANT_READ) || (error->sysError == SSL_ERROR_WANT_WRITE)))
				return 0;
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, error->sysError );
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLAccept error on SSL_accept ", __FILE__, __LINE__);
				ripcSSLErrors(error, strlen(error->text));
				return -1;
			}
		}

		/* what happens if this is not true??? */
		if ((*(sslFuncs.ssl_state))(sess->connection) == SSL_ST_OK)
		{
			return 1;
		}
		else
		{
			/* what do we do here? */
			/* do we need to do something here */
			return 0;
		}
	}
	else
	{
		/* set internal state before to cover error returns */
		/* put this into the second byte; standard handshake gets first byte; top two are unused for now */
		inPr->intConnState = (sess->clientConnState << 8);  
		/* Make sure to reset this if we change state before returning */

		/* should be client side */
		if (sess->clientConnState == SSL_INITIALIZING)
		{
			if ((retVal = (*(sslFuncs.ssl_connect))(sess->connection)) <= 0)
			{
				/* this would happen if its nonblocking and it needs more action to be taken */
				error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

				if ((retVal == -1) && ((error->sysError == SSL_ERROR_WANT_READ) || (error->sysError == SSL_ERROR_WANT_WRITE)))
					return 0;
				else
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLAccept error on SSL_connect SSL Error: %i retVal: %i errno: %i ", __FILE__, __LINE__, error->sysError, retVal, errno);
					ripcSSLErrors(error, strlen(error->text));
					return -1;
				}
			}
			else
			{
				sess->clientConnState = SSL_ACTIVE;
				inPr->intConnState = (sess->clientConnState << 8);  
				return 1;
			}
		}
	}
}

void *ripcSSLConnectInt(RsslSocket fd, RsslInt32 SSLProtocolVersion, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	RsslInt32 retVal = 0;
	ripcSSLSession *sess = ripcInitializeSSLSession(fd, SSLProtocolVersion, 0, error);

	if ((sess->bio = (*(bioFuncs.new_socket))(sess->socket, BIO_NOCLOSE)) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLConnect error creating new socket", __FILE__, __LINE__);
		return 0;
	}

	if ((sess->connection = (*(sslFuncs.ssl_new))(sess->ctx)) == NULL)
	{
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLConnect error creating new SSL connection", __FILE__, __LINE__);
		return 0;
	}

	(*(sslFuncs.ssl_clear))(sess->connection);

	if ((*(sslFuncs.set_cipher_list))(sess->connection, CIPHER_LIST) < 1)
	{
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLConnect error setting cipher list", __FILE__, __LINE__);
		return 0;
	}

	/* allows us to write part of a buffer and not be required to pass in the same address with the next write call */
	(*(sslFuncs.ctrl))(sess->connection, SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);

	(*(sslFuncs.set_bio))(sess->connection, sess->bio, sess->bio);
	(*(sslFuncs.set_connect_state))(sess->connection);

	
	(*(sslFuncs.set_ex_data))(sess->connection, 0, sess);  // Set the user spec pointer for this connection 

	if ((retVal = (*(sslFuncs.ssl_connect))(sess->connection)) <= 0)
	{
		/* this would happen if its nonblocking and it needs more action to be taken */
		error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

		if ((retVal == -1) && ((error->sysError == SSL_ERROR_WANT_READ) || (error->sysError == SSL_ERROR_WANT_WRITE)))
		{
			sess->clientConnState = SSL_INITIALIZING;

			*initComplete = 0;
		}
		else
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, error->sysError);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLConnect error on SSL_connect", __FILE__, __LINE__);
			ripcSSLErrors(error, strlen(error->text));
			return 0;
		}
	}
	else /* This else implies that we are doing blocking. */
	{
		sess->clientConnState = SSL_ACTIVE;
			*initComplete = 1;
		}

	return sess;
}



void *ripcSSLConnectTLSv1(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	return ripcSSLConnectInt(fd, RIPC_PROTO_SSL_TLS_V1, initComplete, userSpecPtr, error);
}

void *ripcSSLConnectTLSv11(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	return ripcSSLConnectInt(fd, RIPC_PROTO_SSL_TLS_V1_1, initComplete, userSpecPtr, error);
}

void *ripcSSLConnectTLSv12(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	return ripcSSLConnectInt(fd, RIPC_PROTO_SSL_TLS_V1_2, initComplete, userSpecPtr, error);
}


RsslInt32  ripcSSLReconnection(void *session, RsslError *error)
{
	return 1;
}

void *ripcNewSSLSocket(void *server, RsslSocket fd, RsslInt32 *initComplete,void* userSpecPtr,  RsslError *error)
{
	ripcSSLSession *newsess = (ripcSSLSession*)ripcSSLNewSession(fd, 0, server, error);
	ripcSSLServer *srvr = (ripcSSLServer*)server;

	
	if (newsess == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLNewSocket could not allocate ripcSSLSession.", __FILE__, __LINE__);
		return 0;
	}

	*initComplete = 0;

	ripcSSLRandSeed();

	newsess->connection = (*(sslFuncs.ssl_new))(srvr->ctx);
	
	(*(sslFuncs.ssl_clear))(newsess->connection);

	if ((*(sslFuncs.set_cipher_list))(newsess->connection, CIPHER_LIST) < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcSSLNewSocket could not set cipher", __FILE__, __LINE__);
		return 0;
	}

	/* allows us to write part of a buffer and not be required to pass in the same address with the next write call */
	(*(sslFuncs.ctrl))(newsess->connection, SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);

	(*(sslFuncs.set_ex_data))(newsess->connection, 1, newsess);
	(*(sslFuncs.set_ex_data))(newsess->connection, 0, newsess);

	newsess->bio = (*(bioFuncs.new_socket))(fd, BIO_NOCLOSE);
	(*(sslFuncs.set_bio))(newsess->connection, newsess->bio, newsess->bio);
	(*(sslFuncs.set_accept_state))(newsess->connection);

	return newsess;
}

RsslInt32 ripcReleaseSSLServer(void* srvr, RsslError *error)
{
	if (srvr)
	{
		ripcSSLServer *server=(ripcSSLServer*)srvr;
		
		if (server->ctx)
			(*(ctxFuncs.ctx_free))(server->ctx);

		ripcFreeKeys(server);
		ripcFreeSSLConnectOpts(&server->config);
		_rsslFree(server);
	}

	return 1;
}

RsslInt32 ripcReleaseSSLSession(void* session, RsslError *error)
{
	if (session)
	{

		ripcSSLSession *sess=(ripcSSLSession*)session;
		if (sess->ctx)
		{
			(*(ctxFuncs.ctx_free))(sess->ctx);
			sess->ctx = 0;
		}

		// if the server exists, the config is just pointing to the server 
		// if it doesnt, we need to clean up the memory we point to
		if (sess->server)
		{
			// just reset pointers 
			sess->config.CAfile = 0;
			sess->config.CApath = 0;
			sess->config.cert_file = 0;
			sess->config.cipher = 0;
			sess->config.verifyDepth = 0;
			sess->config.verifyLevel = 0;
			sess->config.blocking = 0;
			sess->config.address = 0;
			sess->config.id = 0;
			sess->config.pID = 0;
		}
		else
			ripcFreeSSLConnectOpts(&sess->config);
		
		sess->server = 0;

		/* release the SSL* */
		if (sess->connection)
		{
			(*(sslFuncs.ssl_free))(sess->connection);
			sess->connection = 0;
		}

		_rsslFree(sess);
	}
	return 1;
}

ripcSSLServer *ripcInitializeSSLServer(RsslSocket fd, char* name, RsslError  *error)
{
	/* how do we pass in the connect options here??? */
	ripcSSLServer *server = ripcSSLNewServer(fd, name, error);

	if (server == 0)
		return 0;

	/* setup temporary keys */
	if (ripcInitKeys(server, error) <= 0)
	{
		ripcReleaseSSLServer(server, 0);
		return 0;
	}

	/* setup the CTX - we are the server */
	server->ctx = ripcSSLSetupCTX(1, RIPC_PROTO_SSL_TLS_V1_2,  &server->config, error);
	if (server->ctx == 0)
	{
		/* release server */
		ripcReleaseSSLServer(server, 0);
		return 0;
	}

	(*(ctxFuncs.ctx_set_ex_data))(server->ctx, 0, server);

	/* not setting the cache size - right now that will allow 20000 sessions 
	   - this is extreme, need a better idea of actual number of sessions to allow */

	return server;
}

ripcSSLSession *ripcInitializeSSLSession(RsslSocket fd, RsslInt32 SSLProtocolVersion, char* name, RsslError *error)
{
	ripcSSLSession *sess = ripcSSLNewSession(fd, name, NULL, error);

	if (sess == 0)
		return 0;

	/* setup the CTX - we are the client */
	sess->ctx = ripcSSLSetupCTX(0, SSLProtocolVersion, &sess->config, error);

	(*(ctxFuncs.ctx_set_ex_data))(sess->ctx, 0, sess);

	return sess;
}


RsslInt32 verify_callback(RsslInt32 ok, X509_STORE_CTX *store)
{
	char data[256];
	RsslInt32 depth = 0;
	RsslInt32 err = 0;
	ripcSSLSession *sess = 0;
	SSL *ssl = 0;
	X509 *cert = 0;

	ssl = (SSL*)(*(cryptoFuncs.X509_get_ex_data))(store, 0);
	sess = (ripcSSLSession*)(*(sslFuncs.get_ex_data))(ssl, 0);

	cert = (*(cryptoFuncs.get_current_cert))(store);
	depth = (*(cryptoFuncs.get_error_depth))(store);
	err = (*(cryptoFuncs.get_error))(store);

	(*(cryptoFuncs.name_oneline))((*(cryptoFuncs.get_subject_name))(cert), data, 256);

	if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
		err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN ||
		err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
		err == X509_V_ERR_CERT_UNTRUSTED ||
		err == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE)
	{
		if (sess->config.verifyLevel == ripcSSLRequireCA) 
		{
			if (!((*(cryptoFuncs.name_oneline))((*(cryptoFuncs.get_issuer_name))(cert), data, 256)))
			{
				return 0;
		}
		}
		else
			return 1;
	}

	if (ok)
	{
		err = (*(cryptoFuncs.get_error))(store);
	}
	else
	{
		/* we have a verify error */
		if (sess->config.verifyDepth >= depth)
		{
			/* if we hit the depth, we probably shouldnt treat this as OK */
			ok = 1;
		}
		else
		{
			ok = 0;
		}
	}

	return ok;
}

#endif
