/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2024 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/ripcsslutils.h"
#include "rtr/ripcflip.h"
#include "rtr/ripcssljit.h"
#include "rtr/rsslErrors.h"
#include "rtr/rsslLoadInitTransport.h"
#include <sys/stat.h>

static ripcSSLApiFuncs sslFuncs;
static ripcCryptoApiFuncs cryptoFuncs;

/* This should only get populated once upon lib load.  If it's none, that's an error case */
static ripcSSLProtocolFlags supportedProtocols = RIPC_PROTO_SSL_NONE;

#ifdef WIN32
#include <wincrypt.h>
#include <WinSock2.h>
static HMODULE sslHandle = 0;
static HMODULE cryptoHandle = 0;
#define getpid _getpid
#else
#include <arpa/inet.h>
static void* sslHandle = 0;
static void* cryptoHandle = 0;
#endif

#if defined(_WIN32)
static RsslUInt32			shutdownFlag = SD_SEND;
#else
static RsslUInt32			shutdownFlag = SHUT_WR;
#endif

#ifdef LINUX
static char* default10SslLibName = "libssl.so.10";
static char* default10CryptoLibName = "libcrypto.so.10";
static char* default11SslLibName = "libssl.so.1.1";
static char* default11CryptoLibName = "libcrypto.so.1.1";
static char* default3SslLibName = "libssl.so.3";
static char* default3CryptoLibName = "libcrypto.so.3";
#else
static char* default10SslLibName = "ssleay32.dll";
static char* default10CryptoLibName = "libeay32.dll";
static char* default11SslLibName = "libssl-1_1-x64.dll";
static char* default11CryptoLibName = "libcrypto-1_1-x64.dll";
static char* default3SslLibName = "libssl-3-x64.dll";
static char* default3CryptoLibName = "libcrypto-3-x64.dll";
#endif

static RsslOpenSSLAPIVersion openSSLAPI = RSSL_OPENSSL_VNONE;

// for monitoring calls of initialization and releasing
static RsslInt32 countInitializeSSLServer = 0;
static RsslInt32 countReleaseSSLServer = 0;

ripcSSLApiFuncs* ripcGetOpenSSLApiFuncs()
{
	return &sslFuncs;
}

ripcCryptoApiFuncs* ripcGetOpenSSLCryptoFuncs()
{
	return &cryptoFuncs;
}

ripcSSLProtocolFlags ripcGetSupportedProtocolFlags()
{
	if (openSSLAPI == RSSL_OPENSSL_V1_0 || openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
		return supportedProtocols;
	else
		return RIPC_PROTO_SSL_NONE;
}

int ripcSSLInitError()
{
	if (cryptoHandle != 0)
	{
		RSSL_LI_DLCLOSE(cryptoHandle);
		cryptoHandle = 0;
	}
	if (sslHandle != 0)
	{
		RSSL_LI_DLCLOSE(sslHandle);
		sslHandle = 0;
	}

	memset(&sslFuncs, 0, sizeof(ripcSSLApiFuncs));
	memset(&cryptoFuncs, 0, sizeof(ripcCryptoApiFuncs));

	sslFuncs.initialized = 0;
	cryptoFuncs.initialized = 0;
	return -1;
}

#ifdef WIN32
static HMODULE openSslLib(char* libsslName)
#else
static void* openSslLib(char* libsslName)
#endif
{
	/* try given name first */
	if (libsslName != NULL && (*libsslName != '\0'))
	{
		RSSL_LI_RESET_DLERROR;
		sslHandle = RSSL_LI_DLOPEN(libsslName);
		return sslHandle;
	}

	/* try OPENSSL 3 name then */
	RSSL_LI_RESET_DLERROR;
	if ((sslHandle = RSSL_LI_DLOPEN(default3SslLibName)) != 0)
	{
		return sslHandle;
	}

	/* try OPENSSL 1.1 name then */
	RSSL_LI_RESET_DLERROR;
	if ((sslHandle = RSSL_LI_DLOPEN(default11SslLibName)) != 0)
	{
		return sslHandle;
	}

	/* try OPENSSL 1.0 name then */
	RSSL_LI_RESET_DLERROR;
	if ((sslHandle = RSSL_LI_DLOPEN(default10SslLibName)) != 0)
	{
		return sslHandle;
	}

	return sslHandle;
}

#ifdef WIN32
static HMODULE openCryptoLib(char* libcryptoName)
#else
static void* openCryptoLib(char* libcryptoName)
#endif
{
	/* try given name first */
	if (libcryptoName != NULL && (*libcryptoName != '\0'))
	{
		RSSL_LI_RESET_DLERROR;
		cryptoHandle = RSSL_LI_DLOPEN(libcryptoName);
		return cryptoHandle;
	}

	/* try OPENSSL 3 name then */
	RSSL_LI_RESET_DLERROR;
	if ((cryptoHandle = RSSL_LI_DLOPEN(default3CryptoLibName)) != 0)
	{
		return cryptoHandle;
	}

	/* try OPENSSL 1.1 name then */
	RSSL_LI_RESET_DLERROR;
	if ((cryptoHandle = RSSL_LI_DLOPEN(default11CryptoLibName)) != 0)
	{
		return cryptoHandle;
	}

	/* try OPENSSL 1.0 name then */
	RSSL_LI_RESET_DLERROR;
	if ((cryptoHandle = RSSL_LI_DLOPEN(default10CryptoLibName)) != 0)
	{
		return cryptoHandle;
	}

	return cryptoHandle;
}

RsslInt32 ripcInitializeSSL(char* libsslName, char* libcryptoName)
{
	RsslInt32 retVal;
	
	ripcTransportFuncs SSLfuncs;
	ripcSSLFuncs funcs;
	RSSL_LI_ERR_T dlErr = 0;

	RsslBool openSSL_V1_1_0 = RSSL_FALSE;

	memset(&sslFuncs, 0, sizeof(ripcSSLApiFuncs));
	memset(&cryptoFuncs, 0, sizeof(ripcCryptoApiFuncs));

	supportedProtocols = RIPC_PROTO_SSL_NONE;

	/* load libcrypto first */
	if ((cryptoHandle = openCryptoLib(libcryptoName)) == 0)
	{
		return -1;
	}

	if ((sslHandle = openSslLib(libsslName)) == 0)
	{
		RSSL_LI_DLCLOSE(cryptoHandle);
		return -1;
	}

	/* This function is in the 1.0.X versions, but not 1.1.0 */
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_10_version = (unsigned long (*)(void))RSSL_LI_DLSYM(cryptoHandle, "SSLeay");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_10_version, dlErr))
	{
		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_11_version = (unsigned long(*)())RSSL_LI_DLSYM(cryptoHandle, "OpenSSL_version_num");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_11_version, dlErr))
		{
			return ripcSSLInitError();
		}
		else
		{
			unsigned long version = (*(sslFuncs.ssl_11_version))();
			/* version 0xMNNFFPPS: major minor fix patch (the letter) status (development or betas or release) */
			if ((version & 0xf0000000) == 0x30000000)
			{
				openSSLAPI = RSSL_OPENSSL_V3;
				sslFuncs.version = RSSL_OPENSSL_V3;
				cryptoFuncs.version = RSSL_OPENSSL_V3;
			}
			/* version 0x1010114f is for 1.1.1t release */
			else if ((version & 0xf0000000) == 0x10000000)
			{
				/* version 0x101000bf is for 1.1.0k release */
				if ((version & 0x0ffff000) == 0x00100000)
				{
					openSSL_V1_1_0 = RSSL_TRUE;
				}
				openSSLAPI = RSSL_OPENSSL_V1_1;
				sslFuncs.version = RSSL_OPENSSL_V1_1;
				cryptoFuncs.version = RSSL_OPENSSL_V1_1;
			}
			else
			{
				return ripcSSLInitError();
			}
		}
	}
	else
	{
		openSSLAPI = RSSL_OPENSSL_V1_0;
		sslFuncs.version = RSSL_OPENSSL_V1_0;
		cryptoFuncs.version = RSSL_OPENSSL_V1_0;
	}

	/* These functions are only used with OpenSSL 1.0.X's interface */
	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		RSSL_LI_RESET_DLERROR;
		sslFuncs.library_init = (int (*)(void))RSSL_LI_DLSYM(sslHandle, "SSL_library_init");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.library_init, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.load_error_strings = (void (*)())RSSL_LI_DLSYM(sslHandle, "SSL_load_error_strings");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.load_error_strings, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_state = (int (*)(const OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_state");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_state, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.load_crypto_strings = (void (*)())RSSL_LI_DLSYM(cryptoHandle, "ERR_load_crypto_strings");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.load_crypto_strings, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.err_free_strings = (void (*)(void))RSSL_LI_DLSYM(cryptoHandle, "ERR_free_strings");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_free_strings, dlErr))
				return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.thread_id = (unsigned long (*)())RSSL_LI_DLSYM(cryptoHandle, "CRYPTO_thread_id");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.thread_id, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.engine_cleanup = (void (*)(void))RSSL_LI_DLSYM(cryptoHandle, "ENGINE_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.engine_cleanup, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.crypto_cleanup_all_ex_data = (void (*)(void))RSSL_LI_DLSYM(cryptoHandle, "CRYPTO_cleanup_all_ex_data");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.crypto_cleanup_all_ex_data, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.ASN1_STRING_data = (unsigned char* (*)(OPENSSL_ASN1_STRING*))RSSL_LI_DLSYM(cryptoHandle, "ASN1_STRING_data");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.ASN1_STRING_data, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.sk_free = (void* (*)(OPENSSL_STACK*))RSSL_LI_DLSYM(cryptoHandle, "sk_free");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.sk_free, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.sk_num = (int (*)(const OPENSSL_STACK*))RSSL_LI_DLSYM(cryptoHandle, "sk_num");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.sk_num, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.sk_value = (void* (*)(const OPENSSL_STACK*, int))RSSL_LI_DLSYM(cryptoHandle, "sk_value");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.sk_value, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.evp_cleanup = (void (*)(void))RSSL_LI_DLSYM(cryptoHandle, "EVP_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.evp_cleanup, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.TLSv1_2_client_method = (const OPENSSL_SSL_METHOD* (*)())RSSL_LI_DLSYM(sslHandle, "TLSv1_2_client_method");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.TLSv1_2_client_method, dlErr))
			sslFuncs.TLSv1_2_client_method = 0;
		else
			supportedProtocols |= RIPC_PROTO_SSL_TLS_V1_2;

		RSSL_LI_RESET_DLERROR;
		sslFuncs.SSLv23_server_method = (const OPENSSL_SSL_METHOD* (*)())RSSL_LI_DLSYM(sslHandle, "SSLv23_server_method");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.SSLv23_server_method, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.dh_free_10 = (void(*)(OPENSSL_10_DH*))RSSL_LI_DLSYM(cryptoHandle, "DH_free");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.dh_free_10, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.dh_new_10 = (OPENSSL_10_DH* (*)())RSSL_LI_DLSYM(cryptoHandle, "DH_new");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.dh_new_10, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.read_bio_dhparams_10 = (OPENSSL_10_DH* (*)(OPENSSL_BIO*, OPENSSL_10_DH**, pem_password_cb*, void*))RSSL_LI_DLSYM(cryptoHandle, "PEM_read_bio_DHparams");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.read_bio_dhparams_10, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.PEM_write_bio_RSAPrivateKey_10 = (int (*)(OPENSSL_BIO*, OPENSSL_10_rsa*, const OPENSSL_EVP_CIPHER*, 
						unsigned char*, int, pem_password_cb*, void*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_RSAPrivateKey");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_RSAPrivateKey_10, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.PEM_write_bio_RSAPublicKey_10 = (int (*)(OPENSSL_BIO*, OPENSSL_10_rsa*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_RSAPublicKey");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_RSAPublicKey_10, dlErr))
			return ripcSSLInitError();
	}
	
	/* These functions are only defined on the OpenSSL 1.1.X interface */
	if (openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
	{
		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_get_state = (RSSL_11_OSSL_HANDSHAKE_STATE (*)(const OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_get_state");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_get_state, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.set_options = (long (*)(OPENSSL_SSL*, long))RSSL_LI_DLSYM(sslHandle, "SSL_set_options");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_options, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_set1_host = (int (*)(OPENSSL_SSL*, const char*))RSSL_LI_DLSYM(sslHandle, "SSL_set1_host");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_set1_host, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_get0_param = (OPENSSL_X509_VERIFY_PARAM*(*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_get0_param");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_get0_param, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.x509_verify_param_set1_ip_asc = (int(*)(OPENSSL_X509_VERIFY_PARAM*, const char*))RSSL_LI_DLSYM(cryptoHandle, "X509_VERIFY_PARAM_set1_ip_asc");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.x509_verify_param_set1_ip_asc, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.set_hostflags = (void (*)(OPENSSL_SSL*, unsigned int))RSSL_LI_DLSYM(sslHandle, "SSL_set_hostflags");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_hostflags, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.ctx_set_options = (long(*)(OPENSSL_SSL_CTX*, unsigned long))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_options");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_options, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		sslFuncs.TLS_client_method = (const OPENSSL_SSL_METHOD* (*)())RSSL_LI_DLSYM(sslHandle, "TLS_client_method");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.TLS_client_method, dlErr))
			return ripcSSLInitError();
		else
		{
			/* OpenSSL 1.1.0 does not support TLS 1.3 */
			if (openSSL_V1_1_0 == RSSL_TRUE)
			{
				supportedProtocols = RIPC_PROTO_SSL_TLS_V1_2;
			}
			else
			{
				supportedProtocols = RIPC_PROTO_SSL_TLS_V1_2 | RIPC_PROTO_SSL_TLS_V1_3;
			}
		}

		RSSL_LI_RESET_DLERROR;
		sslFuncs.TLS_server_method = (const OPENSSL_SSL_METHOD* (*)())RSSL_LI_DLSYM(sslHandle, "TLS_server_method");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.TLS_server_method, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.dh_free_11 = (void(*)(OPENSSL_11_DH*))RSSL_LI_DLSYM(cryptoHandle, "DH_free");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.dh_free_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.dh_new_11 = (OPENSSL_11_DH* (*)())RSSL_LI_DLSYM(cryptoHandle, "DH_new");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.dh_new_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.read_bio_dhparams_11 = (OPENSSL_11_DH* (*)(OPENSSL_BIO*, OPENSSL_11_DH**, pem_password_cb*, void*))RSSL_LI_DLSYM(cryptoHandle, "PEM_read_bio_DHparams");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.read_bio_dhparams_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.dh_set0_pqg_11 = (int (*)(OPENSSL_11_DH *dh, OPENSSL_BIGNUM *p, OPENSSL_BIGNUM *q, OPENSSL_BIGNUM *g))RSSL_LI_DLSYM(cryptoHandle, "DH_set0_pqg");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.dh_set0_pqg_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.PEM_write_bio_RSAPrivateKey_11 = (int (*)(OPENSSL_BIO*, OPENSSL_11_rsa*, const OPENSSL_EVP_CIPHER*, 
					unsigned char*, int, pem_password_cb*, void*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_RSAPrivateKey");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_RSAPrivateKey_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.PEM_write_bio_RSAPublicKey_11 = (int (*)(OPENSSL_BIO*, OPENSSL_11_rsa*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_RSAPublicKey");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_RSAPublicKey_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.RSA_new_11 = (OPENSSL_11_rsa* (*)())RSSL_LI_DLSYM(cryptoHandle, "RSA_new");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.RSA_new_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.RSA_free_11 = (void (*)(OPENSSL_11_rsa*))RSSL_LI_DLSYM(cryptoHandle, "RSA_free");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.RSA_free_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.RSA_set0_key_11 = (int (*)(OPENSSL_11_rsa*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "RSA_set0_key");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.RSA_set0_key_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.RSA_set0_factors_11 = (int (*)(OPENSSL_11_rsa*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "RSA_set0_factors");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.RSA_set0_factors_11, dlErr))
			return ripcSSLInitError();

		RSSL_LI_RESET_DLERROR;
		cryptoFuncs.RSA_set0_crt_params_11 = (int (*)(OPENSSL_11_rsa*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "RSA_set0_crt_params");
		if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.RSA_set0_crt_params_11, dlErr))
			return ripcSSLInitError();
	}

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_version = (int (*)(const OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_version");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_version, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.get_ex_data = (void* (*)(OPENSSL_SSL*, int))RSSL_LI_DLSYM(sslHandle, "SSL_get_ex_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.get_ex_data, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_read = (int (*)(OPENSSL_SSL*, void*, int))RSSL_LI_DLSYM(sslHandle, "SSL_read");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_read, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.get_error = (int (*)(const OPENSSL_SSL*, int))RSSL_LI_DLSYM(sslHandle, "SSL_get_error");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.get_error, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_write = (int (*)(OPENSSL_SSL*, void*, int))RSSL_LI_DLSYM(sslHandle, "SSL_write");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_write, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_shutdown = (void (*)(OPENSSL_SSL*, int))RSSL_LI_DLSYM(sslHandle, "SSL_set_shutdown");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_shutdown, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_accept = (int (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_accept");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_accept, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.get_verify_result = (long (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_get_verify_result");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.get_verify_result, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_connect = (int (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_connect");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_connect, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_new = (OPENSSL_SSL* (*)(OPENSSL_SSL_CTX*))RSSL_LI_DLSYM(sslHandle, "SSL_new");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_new, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_clear = (int (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_clear");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_clear, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_cipher_list = (int (*)(OPENSSL_SSL*, const char*))RSSL_LI_DLSYM(sslHandle, "SSL_set_cipher_list");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_cipher_list, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_bio = (void (*)(OPENSSL_SSL*, OPENSSL_BIO*, OPENSSL_BIO*))RSSL_LI_DLSYM(sslHandle, "SSL_set_bio");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_bio, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctrl = (long (*)(OPENSSL_SSL*, int, long, void*))RSSL_LI_DLSYM(sslHandle, "SSL_ctrl");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctrl, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_connect_state = (void (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_set_connect_state");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_connect_state, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_ex_data = (int (*)(OPENSSL_SSL*, int, void*))RSSL_LI_DLSYM(sslHandle, "SSL_set_ex_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_ex_data, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.set_accept_state = (void (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_set_accept_state");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.set_accept_state, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ssl_free = (void (*)(OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_free, dlErr))
		return ripcSSLInitError();
	
	/* SSL_get0_peer_certificate() and SSL_get1_peer_certificate() were added in 3.0.0.
	   SSL_get_peer_certificate() is an alias of SSL_get1_peer_certificate().
	   SSL_get_peer_certificate() was deprecated in 3.0.0. */
	if (openSSLAPI == RSSL_OPENSSL_V3)
	{
		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_get_peer_cert = (OPENSSL_X509* (*)(const OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_get1_peer_certificate");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_get_peer_cert, dlErr))
			return ripcSSLInitError();
	}
	else
	{
		RSSL_LI_RESET_DLERROR;
		sslFuncs.ssl_get_peer_cert = (OPENSSL_X509* (*)(const OPENSSL_SSL*))RSSL_LI_DLSYM(sslHandle, "SSL_get_peer_certificate");
		if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ssl_get_peer_cert, dlErr))
			return ripcSSLInitError();
	}

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_new = (OPENSSL_SSL_CTX* (*)(const OPENSSL_SSL_METHOD*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_new");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_new, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_quiet_shutdown = (void (*)(OPENSSL_SSL_CTX*, int))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_quiet_shutdown");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_quiet_shutdown, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_cipher_list = (int (*)(OPENSSL_SSL_CTX*, const char*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_cipher_list");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_cipher_list, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_load_verify_location = (int (*)(OPENSSL_SSL_CTX*, const char*, const char*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_load_verify_locations");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_load_verify_location, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_default_verify_paths = (int (*)(OPENSSL_SSL_CTX*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_default_verify_paths");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_load_verify_location, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_use_cert_chain_file = (int (*)(OPENSSL_SSL_CTX*, const char*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_use_certificate_chain_file");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_use_cert_chain_file, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_use_privatekey_file = (int (*)(OPENSSL_SSL_CTX*, const char*, int))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_use_PrivateKey_file");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_use_privatekey_file, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_cert_store = (void (*)(OPENSSL_SSL_CTX*, OPENSSL_X509_STORE*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_cert_store");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_cert_store, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_get_cert_store = (OPENSSL_X509_STORE* (*)(OPENSSL_SSL_CTX*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_get_cert_store");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_get_cert_store, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_verify = (void (*)(OPENSSL_SSL_CTX*, int, verifyCallback))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_verify");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_verify, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_ctrl = (long (*)(OPENSSL_SSL_CTX*, int, long, void*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_ctrl");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_ctrl, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_free = (void (*)(OPENSSL_SSL_CTX*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.ctx_set_ex_data = (int (*)(OPENSSL_SSL_CTX*, int, void*))RSSL_LI_DLSYM(sslHandle, "SSL_CTX_set_ex_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.ctx_set_ex_data, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_new_file = (OPENSSL_BIO* (*)(const char*, const char*))RSSL_LI_DLSYM(cryptoHandle, "BIO_new_file");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_new_file, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_free = (int (*)(OPENSSL_BIO*))RSSL_LI_DLSYM(cryptoHandle, "BIO_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_sock_should_retry = (int (*)(int))RSSL_LI_DLSYM(cryptoHandle, "BIO_sock_should_retry");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_new_socket = (OPENSSL_BIO* (*)(int, int))RSSL_LI_DLSYM(cryptoHandle, "BIO_new_socket");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_new_socket, dlErr))
		return ripcSSLInitError();
	
	/* libcrypto calls */
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_error_line_data = (unsigned long (*)(const char**, int*, const char**, int*))RSSL_LI_DLSYM(cryptoHandle, "ERR_get_error_line_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.get_error_line_data, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.err_peek_error = (unsigned long (*)(void))RSSL_LI_DLSYM(cryptoHandle, "ERR_peek_error");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_peek_error, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.err_get_error = (unsigned long (*)(void))RSSL_LI_DLSYM(cryptoHandle, "ERR_get_error");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_get_error, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.err_error_string = (char* (*)(unsigned long, char*))RSSL_LI_DLSYM(cryptoHandle, "ERR_error_string");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_error_string, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.err_print_errors_fp = (void (*)(FILE*))RSSL_LI_DLSYM(cryptoHandle, "ERR_print_errors_fp");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_print_errors_fp, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.rand_seed = (void (*)(const void*, int))RSSL_LI_DLSYM(cryptoHandle, "RAND_seed");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.rand_seed, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.bin2bn = (OPENSSL_BIGNUM* (*)(const unsigned char*, int, OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "BN_bin2bn");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.bin2bn, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_get_ex_data = (void* (*)(OPENSSL_X509_STORE_CTX*, int))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_CTX_get_ex_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_get_ex_data, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_store_new = (OPENSSL_X509_STORE* (*)(void))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_new");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_store_new, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_free = (void (*)(OPENSSL_X509*))RSSL_LI_DLSYM(cryptoHandle, "X509_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.d2i_X509 = (OPENSSL_X509* (*)(OPENSSL_X509**, const unsigned char**, long len))RSSL_LI_DLSYM(cryptoHandle, "d2i_X509");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.d2i_X509, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_STORE_add_cert = (int (*)(OPENSSL_X509_STORE*, OPENSSL_X509*))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_add_cert");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_STORE_add_cert, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_current_cert = (OPENSSL_X509* (*)(OPENSSL_X509_STORE_CTX*))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_CTX_get_current_cert");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_STORE_add_cert, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_error_depth = (int (*)(OPENSSL_X509_STORE_CTX*))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_CTX_get_error_depth");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_STORE_add_cert, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_error = (int (*)(OPENSSL_X509_STORE_CTX*))RSSL_LI_DLSYM(cryptoHandle, "X509_STORE_CTX_get_error");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.get_error, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.verify_cert_error_string = (const char* (*)(long))RSSL_LI_DLSYM(cryptoHandle, "X509_verify_cert_error_string");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.verify_cert_error_string, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.name_oneline = (char* (*)(OPENSSL_X509_NAME*, char*, int))RSSL_LI_DLSYM(cryptoHandle, "X509_NAME_oneline");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.name_oneline, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_issuer_name = (OPENSSL_X509_NAME* (*)(OPENSSL_X509*))RSSL_LI_DLSYM(cryptoHandle, "X509_get_issuer_name");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.get_issuer_name, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.get_subject_name = (OPENSSL_X509_NAME* (*)(OPENSSL_X509*))RSSL_LI_DLSYM(cryptoHandle, "X509_get_subject_name");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.get_subject_name, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_get_ext_d2i = (void* (*)(const OPENSSL_X509*, int, int*, int*))RSSL_LI_DLSYM(cryptoHandle, "X509_get_ext_d2i");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_get_ext_d2i, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_NAME_get_index_by_NID = (int (*)(OPENSSL_X509_NAME*, int, int))RSSL_LI_DLSYM(cryptoHandle, "X509_NAME_get_index_by_NID");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_NAME_get_index_by_NID, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_NAME_get_entry = (OPENSSL_X509_NAME_ENTRY* (*)(const OPENSSL_X509_NAME*, int))RSSL_LI_DLSYM(cryptoHandle, "X509_NAME_get_entry");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_NAME_get_entry, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.X509_NAME_ENTRY_get_data = (OPENSSL_ASN1_STRING* (*)(OPENSSL_X509_NAME_ENTRY*))RSSL_LI_DLSYM(cryptoHandle, "X509_NAME_ENTRY_get_data");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.X509_NAME_ENTRY_get_data, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.ASN1_STRING_length = (int (*)(OPENSSL_ASN1_STRING*))RSSL_LI_DLSYM(cryptoHandle, "ASN1_STRING_length");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.ASN1_STRING_length, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.err_remove_state = (void (*)(int))RSSL_LI_DLSYM(cryptoHandle, "ERR_remove_state");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.err_remove_state, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_new_by_curve_name = (OPENSSL_EC_KEY* (*)(int))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_new_by_curve_name");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_new_by_curve_name, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_f_base64 = (OPENSSL_BIO_METHOD* (*)())RSSL_LI_DLSYM(cryptoHandle, "BIO_f_base64");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_f_base64, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_s_mem = (OPENSSL_BIO_METHOD* (*)())RSSL_LI_DLSYM(cryptoHandle, "BIO_s_mem");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_s_mem, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_new = (OPENSSL_BIO* (*)(OPENSSL_BIO_METHOD*))RSSL_LI_DLSYM(cryptoHandle, "BIO_new");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_new, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_push = (OPENSSL_BIO* (*)(OPENSSL_BIO*, OPENSSL_BIO*))RSSL_LI_DLSYM(cryptoHandle, "BIO_push");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_push, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_ctrl = (long (*)(OPENSSL_BIO*, int, long, void*))RSSL_LI_DLSYM(cryptoHandle, "BIO_ctrl");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_ctrl, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_write = (int (*)(OPENSSL_BIO*, const void*, int))RSSL_LI_DLSYM(cryptoHandle, "BIO_write");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_write, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_new_mem_buf = (OPENSSL_BIO * (*)(const void*, int))RSSL_LI_DLSYM(cryptoHandle, "BIO_new_mem_buf");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_new_mem_buf, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_read = (int (*)(OPENSSL_BIO*, void*, int))RSSL_LI_DLSYM(cryptoHandle, "BIO_read");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_read, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_set_flags = (void (*)(OPENSSL_BIO*, int))RSSL_LI_DLSYM(cryptoHandle, "BIO_set_flags");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_set_flags, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	sslFuncs.BIO_free_all = (void (*)(OPENSSL_BIO*))RSSL_LI_DLSYM(cryptoHandle, "BIO_free_all");
	if (dlErr = RSSL_LI_CHK_DLERROR(sslFuncs.BIO_free_all, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_set_private_key = (int (*)(OPENSSL_EC_KEY*, const OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_set_private_key");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_set_private_key, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_get0_group = (const OPENSSL_EC_GROUP* (*)(const OPENSSL_EC_KEY*))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_get0_group");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_get0_group, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_POINT_new = (OPENSSL_EC_POINT* (*)(const OPENSSL_EC_GROUP*))RSSL_LI_DLSYM(cryptoHandle, "EC_POINT_new");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_POINT_new, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_POINT_set_affine_coordinates_GFp = (int (*)(const OPENSSL_EC_GROUP*, OPENSSL_EC_POINT*, const OPENSSL_BIGNUM*, const OPENSSL_BIGNUM*, OPENSSL_BN_CTX*))RSSL_LI_DLSYM(cryptoHandle, "EC_POINT_set_affine_coordinates_GFp");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_POINT_set_affine_coordinates_GFp, dlErr))
		return ripcSSLInitError();
	
	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_set_public_key_affine_coordinates = (int (*)(OPENSSL_EC_KEY*, OPENSSL_BIGNUM*, OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_set_public_key_affine_coordinates");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_set_public_key_affine_coordinates, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_set_public_key = (int (*)(OPENSSL_EC_KEY*, const OPENSSL_EC_POINT*))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_set_public_key");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_set_public_key, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_POINT_clear_free = (void (*)(OPENSSL_EC_POINT*))RSSL_LI_DLSYM(cryptoHandle, "EC_POINT_clear_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_POINT_clear_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.PEM_write_bio_ECPrivateKey = (int (*)(OPENSSL_BIO*, OPENSSL_EC_KEY*, const OPENSSL_EVP_CIPHER*, unsigned char*, int, pem_password_cb*, void*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_ECPrivateKey");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_ECPrivateKey, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.PEM_write_bio_EC_PUBKEY = (int (*)(OPENSSL_BIO*, OPENSSL_EC_KEY*))RSSL_LI_DLSYM(cryptoHandle, "PEM_write_bio_EC_PUBKEY");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.PEM_write_bio_EC_PUBKEY, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_free = (void (*)(OPENSSL_EC_KEY*))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.bn_free = (void(*)(OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "BN_free");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.bn_free, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.bn_clear = (void(*)(OPENSSL_BIGNUM*))RSSL_LI_DLSYM(cryptoHandle, "BN_clear");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.bn_clear, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_POINT_mul = (int(*)(const OPENSSL_EC_GROUP*, OPENSSL_EC_POINT*, const OPENSSL_BIGNUM*, const OPENSSL_EC_POINT*, const OPENSSL_BIGNUM*, OPENSSL_BN_CTX*))RSSL_LI_DLSYM(cryptoHandle, "EC_POINT_mul");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_POINT_mul, dlErr))
		return ripcSSLInitError();

	RSSL_LI_RESET_DLERROR;
	cryptoFuncs.EC_KEY_set_asn1_flag = (void(*)(OPENSSL_EC_KEY*, int))RSSL_LI_DLSYM(cryptoHandle, "EC_KEY_set_asn1_flag");
	if (dlErr = RSSL_LI_CHK_DLERROR(cryptoFuncs.EC_KEY_set_asn1_flag, dlErr))
		return ripcSSLInitError();

	SSLfuncs.bindSrvr = ipcSrvrBind;
	SSLfuncs.newSrvrConnection = ripcNewSSLSocket; //should be used after accept is called on the server side
	SSLfuncs.connectSocket = ipcConnectSocket;
	SSLfuncs.newClientConnection = 0;
	SSLfuncs.initializeTransport = ripcSSLInit; // calls accept on server side
	SSLfuncs.shutdownTransport = ripcCloseSSLSocket; // shuts down socket on client or server side
	SSLfuncs.readTransport = ripcSSLRead; // read function on client or server side
	SSLfuncs.writeTransport = ripcSSLWrite; // write function on client or server side
	SSLfuncs.writeVTransport = 0;
	SSLfuncs.reconnectClient = ripcSSLReconnection;
	SSLfuncs.acceptSocket = ipcSrvrAccept;
	SSLfuncs.shutdownSrvrError = 0;
	SSLfuncs.sessIoctl = 0;

	SSLfuncs.getSockName = 0;
	SSLfuncs.setSockOpts = 0;
	SSLfuncs.getSockOpts = 0;
	SSLfuncs.connected = ipcIsConnected;
	SSLfuncs.shutdownServer = 0;
	SSLfuncs.uninitialize = ripcUninitializeSSL;

	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		if ((supportedProtocols & RIPC_PROTO_SSL_TLS_V1_2) != 0)
		{
			SSLfuncs.newClientConnection = ripcSSLConnectTLSv12; // calls SSL connect on client side

			retVal = ipcSetSSLTransFunc(&SSLfuncs);
			if (retVal == 0)
				return ripcSSLInitError();

		}
	}
	else if (openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
	{
		SSLfuncs.newClientConnection = ripcSSLConnectTLS; // calls SSL connect on client side

		retVal = ipcSetSSLTransFunc(&SSLfuncs);
		if (retVal == 0)
			return ripcSSLInitError();
	}

	/* Set the encrypted functions for server usage to ENCRYPTED*/
	ipcSetTransFunc(RSSL_CONN_TYPE_ENCRYPTED, &SSLfuncs);
	
	funcs.newSSLServer = (void*)ripcInitializeSSLServer;
	funcs.freeSSLServer = ripcReleaseSSLServer;

	retVal = ipcSetSSLFuncs(&funcs);
	
	/* These functions only need to be called for OpenSSLv1.0.X.  With OpenSSL 1.1.X, initalization and cleanup are handled automatically */
	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		(*(sslFuncs.library_init))();

		(*(sslFuncs.load_error_strings))();
		(*(cryptoFuncs.load_crypto_strings))();
	}

	return 1;

}

void ripcUninitializeSSL(void)
{
	if (cryptoHandle)
	{
		if (openSSLAPI == RSSL_OPENSSL_V1_0)
		{
			(*(cryptoFuncs.evp_cleanup))();
			(*(cryptoFuncs.engine_cleanup))();
			(*(cryptoFuncs.crypto_cleanup_all_ex_data))();
			(*(cryptoFuncs.err_remove_state))(0);
			(*(cryptoFuncs.err_free_strings))();
		}
	}

	ripcSSLInitError();
}

void ripcSSLErrors(RsslError *error, RsslInt32 initPos)
{
	RsslUInt32 l = 0;
	char buf[1024];
	const char *file, *data;
	RsslInt32 line = 0;
	RsslInt32 flags = 0;
	RsslInt32 cUrl = initPos;
	buf[0] = '\0';

	while ((l=(*(cryptoFuncs.get_error_line_data))(&file, &line, &data, &flags)) != 0)
	{
		if ((strlen(buf) + strlen(file) + cUrl + 30 ) >= MAX_RSSL_ERROR_TEXT)
			break;

		if (data != NULL && flags & RSSL_SSL_ERR_TXT_STRING)
		{
			snprintf(error->text + cUrl, MAX_RSSL_ERROR_TEXT, "OpenSSL Error string: %s:%s:%s:%d:\n", (*(cryptoFuncs.err_error_string))(l, NULL), data, file, line);
		}
		else
			snprintf(error->text + cUrl, MAX_RSSL_ERROR_TEXT, "OpenSSL Error string: %s:%s:%d:\n", (*(cryptoFuncs.err_error_string))(l, NULL), file, line);

		cUrl = (RsslInt32)strlen(error->text);
	}
}

ripcSSLProtocolFlags ripcGetSupportedSSLVersion()
{
	return supportedProtocols;
}

ripcSSLProtocolFlags ripcRemoveHighestSSLVersionFlag(ripcSSLProtocolFlags protoFlags)
{
	ripcSSLProtocolFlags tmp = protoFlags;
    
    if((protoFlags & RIPC_PROTO_SSL_TLS_V1_3) != 0)
	{
		tmp = protoFlags & (~RIPC_PROTO_SSL_TLS_V1_3);
	}
	else if((protoFlags & RIPC_PROTO_SSL_TLS_V1_2) != 0)
	{
		tmp = protoFlags & (~RIPC_PROTO_SSL_TLS_V1_2);
	}

	return tmp;
}

/* case insensitive string comparison.  Inputs are assumed to not be null, 
  and any difference prior to lenght will result in a negative response. */
RsslBool ripcCompareHostNameLen(char* host1, char* host2, unsigned int length)
{
	int i = length;
	char *tmp1 = host1;
	char *tmp2 = host2;
	while (i > 0)
	{
		/* Check to see if either input is a null character, then compare */
		if (*tmp1 == '\0' || *tmp2 == '\0' || toupper((int)*tmp1) != toupper((int)*tmp2))
			break;

		tmp1++;
		tmp2++;
		i--;
	}

	if (i == 0)
		return RSSL_TRUE;
	else
		return RSSL_FALSE;
}

/* Verify the input certificate name according to the wildcarding rules from the following RFC:
	https://tools.ietf.org/html/rfc6125#section-6.4.3 

	This code will not allow for partial wildcarding(i.e. f*.bar.com).  IP Address verification will not be handled 
	here, and should be handled in any above calls.

	Unit tests are located in rsslTransportUnitTest
*/
RsslBool ripcVerifyCertHost(char* inputPattern, unsigned int patternLen, char* hostName, RsslError* err)
{
	const char  *wildcardPos, *patternFirstDotPos, *hostFirstDotPos;
	unsigned int hostNameLen, patternEndLen;
	const char* temp;

	wildcardPos = strchr(inputPattern, '*');
	if (wildcardPos == NULL)
		return ripcCompareHostNameLen(inputPattern, hostName, patternLen);
	
	/* Found a wildcard character, now verify it */
	/* First check to see if two '.' characters are present in 
	  the input pattern to avoid overbroad wildcarded certificates(i.e. *.com).
	  Also check to see if the wildcard is before the first '.' character */
	patternFirstDotPos = strchr(inputPattern, '.');
	if (patternFirstDotPos == NULL)
		return RSSL_FALSE;

	if ((temp = strchr(patternFirstDotPos + 1, '.')) == NULL)
		return RSSL_FALSE;

	if (wildcardPos > patternFirstDotPos)
		return RSSL_FALSE;

	/* Verfiy that the wildcard character is the first character(nothing prior to it), and that a '.' immediately follows */
	if (wildcardPos != inputPattern || patternFirstDotPos != (wildcardPos + 1))
		return RSSL_FALSE;

	patternEndLen = patternLen - (unsigned int)(patternFirstDotPos - inputPattern);

	hostNameLen = (unsigned int)strlen(hostName);
	/* Find the first '.' in the hostName.  Make sure that there is something prior to the first '.' */
	hostFirstDotPos = strchr(hostName, '.');
	if (hostFirstDotPos == NULL || hostName == hostFirstDotPos)
		return RSSL_FALSE;

	/* Now match the remainder of the hostName to the remainder of the inputPattern */
	return ripcCompareHostNameLen((char*)patternFirstDotPos, (char*)hostFirstDotPos, patternEndLen);
}

static RsslBool ripcVerify10Hostname(OPENSSL_X509* cert, RsslSocketChannel* chnl, RsslError* err)
{
	RsslBool match = RSSL_FALSE;
	RsslBool hasChecked = RSSL_FALSE;  /* This ensures that we have checked against an DNS or IP Addess Type*/
	unsigned char* hostName = NULL;
	unsigned int hostNameLen = 0;
	int locCN = -1;
	int hostAddressType = RSSL_SSL_GEN_DNS;  /* Type of the hostname provided by the user.  Defaults to DNS. */

	OPENSSL_X509_NAME* name;
	OPENSSL_X509_NAME_ENTRY* nameEntry;
	OPENSSL_ASN1_STRING* asn1CN;

	OPENSSL_STACK* certAltNames; /* OPENSSL _STACK of alternative names.  OpenSSL provides very strict compile-time type checking.
								 Unfortunately, this code does not have a direct dependency on any OpenSSL headers, so we will maintain
								 these as _STACK pointers */
	int numAltNames;
	OPENSSL_GENERAL_NAME* certName;
	
	int i;

	struct in_addr addr;

	/* Check to see if the hostname on the channel is an IP Address*/
	if (inet_pton(AF_INET, chnl->hostName, &addr))
	{
		hostAddressType = RSSL_SSL_GEN_IPADD;
	}

	certAltNames = (OPENSSL_STACK*)(*cryptoFuncs.X509_get_ext_d2i)(cert, RSSL_NID_subject_alt_name, NULL, NULL);

	if (certAltNames)
	{
		/* There are anternative name structures in the certificate, pull them out, and check against them all.
		If any match, the check is successful*/
		/* Get the number of alternatives */
		numAltNames = (*cryptoFuncs.sk_num)(certAltNames);
		for (i = 0; i < numAltNames; i++)
		{
			certName = (OPENSSL_GENERAL_NAME*)(cryptoFuncs.sk_value)(certAltNames, i);

			if (certName->type == RSSL_SSL_GEN_DNS || certName->type == RSSL_SSL_GEN_IPADD)
				hasChecked = RSSL_TRUE;

			if (certName->type == hostAddressType)
			{
				hostName = (char*)(*cryptoFuncs.ASN1_STRING_data)((OPENSSL_ASN1_STRING*)certName->d.ia5);
				hostNameLen = (unsigned int)(*cryptoFuncs.ASN1_STRING_length)((OPENSSL_ASN1_STRING*)certName->d.ia5);

				switch (hostAddressType)
				{
				case RSSL_SSL_GEN_DNS:
					/* DNS name comparison.  Make sure that the length is correct(there are no embedded \0 characters) and check the hostname */
					if (hostNameLen == (unsigned int)strlen(hostName) && ripcVerifyCertHost(hostName, hostNameLen, chnl->hostName, err) == RSSL_TRUE)
						match = RSSL_TRUE;
					break;
				case RSSL_SSL_GEN_IPADD:
					/* IP address comparison.  Make sure that the value here is an ipV4 address and that the addresses match */
					if (hostNameLen == (unsigned int)sizeof(struct in_addr) && memcmp(hostName, &addr, hostNameLen) == 0)
						match = RSSL_TRUE;
					break;
				}
			}
		}
		(*cryptoFuncs.sk_free)(certAltNames);
		/* Error out if there has not been a match */
		if (match == RSSL_FALSE && hasChecked == RSSL_TRUE)
		{
			_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
			snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  No subject name match in host certificate's alternate name list.", __FILE__, __LINE__);
			return RSSL_FALSE;
		}
		else if (match == RSSL_TRUE && hasChecked == RSSL_TRUE)
		{
			return RSSL_TRUE;
		}
	}

	/* Get the common name in C string form from the certificate's subject name */
	name = (*cryptoFuncs.get_subject_name)(cert);
	if (name == NULL)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  No subject name in host certificate.", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	locCN = (*cryptoFuncs.X509_NAME_get_index_by_NID)(name, RSSL_NID_commonName, -1);
	if (locCN < 0)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  No common name in host certificate.", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	nameEntry = (*cryptoFuncs.X509_NAME_get_entry)(name, locCN);
	if (nameEntry == NULL)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  Unable to retieve common name from certificate.", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	asn1CN = (*cryptoFuncs.X509_NAME_ENTRY_get_data)(nameEntry);
	if (asn1CN == NULL)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  Unable to retieve common name from certificate.", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	/* Get the C string from the ASN1_STRING, and verify that the certificate's length is equal to the length of the C string*/
	hostName = (char*)(*cryptoFuncs.ASN1_STRING_data)(asn1CN);
	hostNameLen = (unsigned int)(*cryptoFuncs.ASN1_STRING_length)(asn1CN);
	if (strlen(hostName) != (size_t)hostNameLen)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  Certificate hostname does not match the connected DNS name", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	if (ripcVerifyCertHost(hostName, hostNameLen, chnl->hostName, err) == RSSL_FALSE)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  Certificate hostname does not match the connected DNS name", __FILE__, __LINE__);
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

/* Verify the server's certificate */
static RsslBool ripcVerifyCert(OPENSSL_SSL* ssl, RsslSocketChannel* chnl, RsslError* err)
{
	OPENSSL_X509* cert = NULL;
	long ret;
	RsslBool certHostMatch = RSSL_TRUE;

	cert = (*sslFuncs.ssl_get_peer_cert)((const OPENSSL_SSL*)ssl);

	if (!cert)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 No certificate from server.", __FILE__, __LINE__);
		ripcSSLErrors(err, (RsslInt32)strlen(err->text));
		return RSSL_FALSE;
	}

	ret = (*sslFuncs.get_verify_result)(ssl);

	if(RSSL_X509_V_OK != ret)
	{
		_rsslSetError(err, NULL, RSSL_RET_FAILURE, 0);
		snprintf(err->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 Certificate validation error.  OpenSSL Return code: %s", __FILE__, __LINE__, (*cryptoFuncs.verify_cert_error_string)(ret));
		ripcSSLErrors(err, (RsslInt32)strlen(err->text));
		return RSSL_FALSE;
	}

	/* Perform hostname validation on OpenSSL 1.0.X.  This is automatically processed in OpenSSL 1.1 onwards. */
	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		certHostMatch = ripcVerify10Hostname(cert, chnl, err);
	}
	
	(*cryptoFuncs.X509_free)(cert);
	
	return certHostMatch;
}

ripcSSLServer *ripcSSLNewServer(RsslServerSocketChannel* chnl, RsslError *error)
{
	RsslInt32 i = 0;
	ripcSSLServer *server = (ripcSSLServer*)_rsslMalloc(sizeof(ripcSSLServer));

	if (server == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1001 Could not allocate space for ripcSSLServer.", __FILE__, __LINE__);
		return 0;
	}

	/* Clear out the SSL Server object */
	memset((void*)server, 0, sizeof(ripcSSLServer));

	server->socket = (RsslUInt32)chnl->stream;
	server->ctx = 0;
	server->chnl = chnl;
	server->verifyDepth = 9;
	server->cipherSuite = chnl->cipherSuite;

	return server;
}

/* server can be null if this is a client side connection */
ripcSSLSession *ripcSSLNewSession(RsslSocket fd, RsslSocketChannel* chnl, ripcSSLServer *server, RsslError *error)
{
	ripcSSLSession *session = (ripcSSLSession*)_rsslMalloc(sizeof(ripcSSLSession));

	if (session == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1001 Could not allocate space for ripcSSLSession.", __FILE__, __LINE__);
		return 0;
	}

	/* Clear out the ssl session object */
	memset((void*)session, 0, sizeof(ripcSSLSession));

	session->socket = (RsslUInt32)fd;
	session->server = server;
	session->bio = 0;
	session->ctx = 0;
	session->connection = 0;
	session->blocking = chnl->blocking;
	session->clientConnState = SSL_INITIALIZING;
	
	return session;
}

OPENSSL_SSL_CTX* ripcSSLCreateCTXClientV1_0(RsslError *error)
{
	OPENSSL_SSL_CTX *ctx = 0;

	if ((ctx = (*(sslFuncs.ctx_new))((*(sslFuncs.TLSv1_2_client_method))())) == NULL)
	{
		/* populate error  and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTXClient() failed to create new context (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		return NULL;
	}

	return ctx;
}

OPENSSL_SSL_CTX* ripcSSLCreateCTXClientV1_1(RsslUInt32 sslProtocolBitmap, RsslError *error)
{
	OPENSSL_SSL_CTX *ctx = 0;

	if ((ctx = (*(sslFuncs.ctx_new))((*(sslFuncs.TLS_client_method))())) == NULL)
	{
		/* populate error  and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTXClient() failed to create new context (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		return NULL;
	}

	/* Setup the TLS version here, starting with the minimum version */
	if (sslProtocolBitmap & RIPC_PROTO_SSL_TLS_V1_2)
	{
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MIN_PROTO_VERSION, RSSL_11_TLS1_2_VERSION, NULL);
	}
	else if (sslProtocolBitmap & RIPC_PROTO_SSL_TLS_V1_3)
	{
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MIN_PROTO_VERSION, RSSL_11_TLS1_3_VERSION, NULL);
	}

	/* Set the maximum version according to the bitmap */
	if (sslProtocolBitmap & RIPC_PROTO_SSL_TLS_V1_3)
	{
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MAX_PROTO_VERSION, RSSL_11_TLS1_3_VERSION, NULL);
	}
	else if (sslProtocolBitmap & RIPC_PROTO_SSL_TLS_V1_2)
	{
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MAX_PROTO_VERSION, RSSL_11_TLS1_2_VERSION, NULL);
	}

	return ctx;
}

OPENSSL_SSL_CTX* ripcSSLSetupCTXClient(OPENSSL_SSL_CTX *ctx, RsslSocketChannel* chnl, RsslError *error)
{
	RsslInt32 retVal = 0;
	/* since we dont need to use certificates by default... */
 	RsslInt32 perm = RSSL_SSL_VERIFY_NONE;

	/* Turn off SSLv2 and SSLv3, since both are insecure.  See:
		https://www.openssl.org/~bodo/ssl-poodle.pdf
	*/

	(*(sslFuncs.ctx_set_quiet_shutdown))(ctx, 1);

	if ((retVal = (*(sslFuncs.ctx_set_cipher_list))(ctx, CIPHER_LIST)) != 1)
	{
		/* populate error and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> Error: 2001 ripcSSLSetupCTXClient() error setting up cipher list (no valid ciphers) (errno %d)", __FILE__,__LINE__,errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		(*(sslFuncs.ctx_free))(ctx);
		return NULL;
	}
	
	(*(sslFuncs.ctx_set_verify))(ctx, perm, NULL);

	/* Setup the Certificate trust store here. */
	/* Check to see if a certificate store location has been provided by the user and pass it to OpenSSL */
	if (chnl->sslCAStore != NULL)
	{
		struct stat filestat;
		int isDirectory; // 1 if path is a directory, 0 if file 
		if (stat(chnl->sslCAStore, &filestat) != 0)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXClient() error finding certificate store", __FILE__, __LINE__);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}

		if (filestat.st_mode & S_IFDIR)
			isDirectory = 1;
		else if (filestat.st_mode & S_IFREG)
			isDirectory = 0;
		else
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXClient() error certificate store not a file or directory", __FILE__, __LINE__);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}

		if (isDirectory)
		{
			if ((*(sslFuncs.ctx_load_verify_location))(ctx, NULL, chnl->sslCAStore) != 1)
			{
				/* populate error and return failure */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTX() error loading certifcate store", __FILE__, __LINE__);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}
		}
		else
		{
			if ((*(sslFuncs.ctx_load_verify_location))(ctx, chnl->sslCAStore, NULL) != 1)
			{
				/* populate error and return failure */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTX() error loading certifcate store", __FILE__, __LINE__);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}
		}
	}
	else
	{
#ifdef LINUX
		/* Verify that the default OpenSSL install location is present */
		if ((retVal = (*(sslFuncs.ctx_set_default_verify_paths))(ctx)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTX() error loading CA file (errno %d)", __FILE__, __LINE__, errno);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}
#elif WIN32
		/* Load the OS CA store into OpenSSL.  We're using the crypto library here to pull the system's CA root certificates, translate them into OpenSSL's format, and add them to the current context's X509 store. */
		HCERTSTORE     hCertStore;
		PCCERT_CONTEXT pContext = NULL;
		OPENSSL_X509_STORE    *store;
		size_t         count = 0;
		OPENSSL_X509* x509 = NULL;

		char** tmpPtr;

		if ((store = (*cryptoFuncs.X509_store_new)()) == NULL)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTX() Error creating new X509_store", __FILE__, __LINE__);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}

		if ((hCertStore = CertOpenSystemStore(0, "ROOT")) == NULL)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTX() Error loading Windows ROOT certificate store", __FILE__, __LINE__);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}


		/* For each certificate in the Windows store, add to the X509_store*/
		while ((pContext = CertEnumCertificatesInStore(hCertStore, pContext)) != NULL)
		{
			tmpPtr = (char**)&pContext->pbCertEncoded;
			x509 = (*cryptoFuncs.d2i_X509)(NULL, tmpPtr, (long)pContext->cbCertEncoded);

			if (x509)
			{
				(*cryptoFuncs.X509_STORE_add_cert)(store, x509);
				(*cryptoFuncs.X509_free)(x509);
			}
		}

		CertFreeCertificateContext(pContext);
		CertCloseStore(hCertStore, 0);

		(*sslFuncs.ctx_set_cert_store)(ctx, store);

#endif
	}

	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		RsslInt32 opts = RSSL_10_SSL_OP_ALL | RSSL_10_SSL_OP_NO_SSLv2 | RSSL_10_SSL_OP_NO_SSLv3;
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_10_SSL_CTRL_OPTIONS, opts, NULL);
	}
	else
	{
		RsslInt32 opts = RSSL_11_SSL_OP_ALL;
		(*(sslFuncs.ctx_set_options))(ctx, opts);
	}
	
	return ctx;
}


OPENSSL_SSL_CTX* ripcSSLSetupCTXServer(RsslServerSocketChannel* chnl, RsslError *error)
{
	OPENSSL_SSL_CTX *ctx = 0;
	RsslInt32 retVal = 0;
	RsslInt32 perm = RSSL_SSL_VERIFY_NONE;
	char* cipherList = 0;
	RsslInt32 opts;


	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		opts = RSSL_10_SSL_OP_ALL | RSSL_10_SSL_OP_NO_SSLv2 | RSSL_10_SSL_OP_NO_SSLv3 | RSSL_10_SSL_OP_NO_TLSv1 | RSSL_10_SSL_OP_NO_TLSv1_1;
		if ((ctx = (*(sslFuncs.ctx_new))((*(sslFuncs.SSLv23_server_method))())) == NULL)
		{
			/* populate error  and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTXServer() failed to create new context (errno %d)", __FILE__, __LINE__, errno);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			return NULL;
		}

		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_10_SSL_CTRL_OPTIONS, opts, NULL);
	}
	else
	{
		opts = RSSL_11_SSL_OP_ALL;

		if ((ctx = (*(sslFuncs.ctx_new))((*(sslFuncs.TLS_server_method))())) == NULL)
		{
			/* populate error  and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLSetupCTXServer() failed to create new context (errno %d)", __FILE__, __LINE__, errno);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			return NULL;
		}

		/* Setup the TLS version here, starting with the minimum version */
		if (chnl->encryptionProtocolFlags & RIPC_PROTO_SSL_TLS_V1_2)
		{
			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MIN_PROTO_VERSION, RSSL_11_TLS1_2_VERSION, NULL);
		}
		else if (chnl->encryptionProtocolFlags & RIPC_PROTO_SSL_TLS_V1_3)
		{
			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MIN_PROTO_VERSION, RSSL_11_TLS1_3_VERSION, NULL);
		}

		/* Set the maximum version according to the bitmap */
		if (chnl->encryptionProtocolFlags & RIPC_PROTO_SSL_TLS_V1_3)
		{
			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MAX_PROTO_VERSION, RSSL_11_TLS1_3_VERSION, NULL);
		}
		else if (chnl->encryptionProtocolFlags & RIPC_PROTO_SSL_TLS_V1_2)
		{
			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_11_SSL_CTRL_SET_MAX_PROTO_VERSION, RSSL_11_TLS1_2_VERSION, NULL);
		}

		(*(sslFuncs.ctx_set_options))(ctx, opts);
	}


	/* if they pass in a key file, use that instead */
	if (chnl->dhParams)
	{
		if (openSSLAPI == RSSL_OPENSSL_V1_0)
		{
			/* set up diffie-hellman temp keys for EDH */
			OPENSSL_BIO *bio;
			OPENSSL_10_DH *dh;

			if ((bio = (*(sslFuncs.BIO_new_file))(chnl->dhParams, "r")) == NULL)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 Unable to load DH parameter file %s.", __FILE__, __LINE__, chnl->dhParams);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}
			dh = (*(cryptoFuncs.read_bio_dhparams_10))(bio, NULL, NULL, NULL);
			(*(sslFuncs.BIO_free))(bio);

			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_SSL_CTRL_SET_TMP_DH, 0, (void*)dh);

			(*(cryptoFuncs.dh_free_10))(dh);
		}
		else
		{
			/* set up diffie-hellman temp keys for EDH */
			OPENSSL_BIO *bio;
			OPENSSL_11_DH *dh;

			if ((bio = (*(sslFuncs.BIO_new_file))(chnl->dhParams, "r")) == NULL)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 Unable to load DH parameter file %s.", __FILE__, __LINE__, chnl->dhParams);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}
			dh = (*(cryptoFuncs.read_bio_dhparams_11))(bio, NULL, NULL, NULL);
			(*(sslFuncs.BIO_free))(bio);

			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_SSL_CTRL_SET_TMP_DH, 0, (void*)dh);

			(*(cryptoFuncs.dh_free_11))(dh);
		}

	}
	else // no key passed in, using default keys from ripcssldh.c file 
	{
		if (openSSLAPI == RSSL_OPENSSL_V1_0)
		{
			/* set up diffie-hellman temp keys for EDH */
			OPENSSL_10_DH *dh = ripcSSL10DHGetParam(&sslFuncs, &cryptoFuncs);

			if (dh == NULL)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 Unable to load default DH parameters %s.", __FILE__, __LINE__, chnl->dhParams);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}

			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_SSL_CTRL_SET_TMP_DH, 0, (void*)dh);

			(*(cryptoFuncs.dh_free_10))(dh);
		}
		else
		{
			/* set up diffie-hellman temp keys for EDH */
			OPENSSL_11_DH *dh = ripcSSL11DHGetParam(&sslFuncs, &cryptoFuncs);

			if (dh == NULL)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 Unable to load default DH parameters %s.", __FILE__, __LINE__, chnl->dhParams);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				(*(sslFuncs.ctx_free))(ctx);
				return NULL;
			}

			(*(sslFuncs.ctx_ctrl))(ctx, RSSL_SSL_CTRL_SET_TMP_DH, 0, (void*)dh);

			(*(cryptoFuncs.dh_free_11))(dh);
		}
	}

	/* Setup the elliptic curve information for 1.0.X only.  1.1 automatically selects the proper curve */
	if (openSSLAPI == RSSL_OPENSSL_V1_0)
	{
		OPENSSL_EC_KEY* ecdh;
		ecdh = (*(cryptoFuncs.EC_KEY_new_by_curve_name))(RSSL_10_NID_X9_62_prime256v1);    //  secp256r1 curve - referred as prime256v1
		(*(sslFuncs.ctx_ctrl))(ctx, RSSL_SSL_CTRL_SET_TMP_ECDH, 0, ecdh);
	}
	

	/* Turn off SSLv2 and SSLv3, since both are insecure.  See:
	https://www.openssl.org/~bodo/ssl-poodle.pdf
	*/

	(*(sslFuncs.ctx_set_quiet_shutdown))(ctx, 1);

	if (chnl->cipherSuite && ((const char*)chnl->cipherSuite) > 0)
		cipherList = chnl->cipherSuite;
	else
		cipherList = CIPHER_LIST;

	if ((retVal = (*(sslFuncs.ctx_set_cipher_list))(ctx, cipherList)) != 1)
	{
		/* populate error and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXServer() error setting up cipher list (no valid ciphers) (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		(*(sslFuncs.ctx_free))(ctx);
		return NULL;
	}

	/* need to get CERTFILE from config */
	if (chnl->serverCert)
	{
		if ((retVal = (*(sslFuncs.ctx_use_cert_chain_file))(ctx, chnl->serverCert)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXServer() error loading certificate from file %s", __FILE__, __LINE__, chnl->serverCert);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}

		/* need to get CERTFILE from config */
		if ((retVal = (*(sslFuncs.ctx_use_privatekey_file))(ctx, chnl->serverPrivateKey, RSSL_SSL_FILETYPE_PEM)) != 1)
		{
			/* populate error and return failure */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXServer() error loading private key from file %s", __FILE__, __LINE__, chnl->serverPrivateKey);
			ripcSSLErrors(error, (RsslInt32)strlen(error->text));
			(*(sslFuncs.ctx_free))(ctx);
			return NULL;
		}
	}
	else
	{
		/* populate error and return failure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLSetupCTXServer() No certificate files provided.", __FILE__, __LINE__);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		(*(sslFuncs.ctx_free))(ctx);
		return NULL;
	}

	(*(sslFuncs.ctx_set_verify))(ctx, perm, NULL);

	return ctx;
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
			case RSSL_SSL_ERROR_NONE:
				totalBytes += numBytes;
				if (sess->blocking)
					return totalBytes;
			break;
			case RSSL_SSL_ERROR_WANT_WRITE:
			case RSSL_SSL_ERROR_WANT_READ:
			case RSSL_SSL_ERROR_WANT_X509_LOOKUP:
					/* these are a would block/retry read situation */
				return totalBytes;
			break;
			case RSSL_SSL_ERROR_SYSCALL:
			case RSSL_SSL_ERROR_SSL: /* OpenSSL3.0 returns SSL_ERROR_SSL for recoverable error codes which ETA can try to read again */
				/* OpenSSL does provide a way to get last error(get_last_socket_error() */
				if((errno == EAGAIN) || (errno == EINTR) || (errno == _IPC_WOULD_BLOCK))
					return totalBytes;
				else
				{
					error->text[0] = '\0';
					return (-1);
				}
			break;
			case RSSL_SSL_ERROR_ZERO_RETURN:
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
			case RSSL_SSL_ERROR_NONE:
				totalOut += numBytes;
				if (sess->blocking)
					return totalOut;
			break;
			case RSSL_SSL_ERROR_WANT_WRITE:
			case RSSL_SSL_ERROR_WANT_READ:
			case RSSL_SSL_ERROR_WANT_X509_LOOKUP:
				/* these are a would block/retry read situation */
				return totalOut;
			break;
			case RSSL_SSL_ERROR_SYSCALL:
			case RSSL_SSL_ERROR_SSL: /* OpenSSL3.0 returns SSL_ERROR_SSL for recoverable error codes which ETA can try to write again */
				/* OpenSSL does provide a way to get last error(get_last_socket_error() */
				if ((errno == EAGAIN) || (errno == EINTR) || (errno == _IPC_WOULD_BLOCK))
					return totalOut;
				else
				{
					error->text[0] = '\0';
					return (-1);
				}
			break;
			case RSSL_SSL_ERROR_ZERO_RETURN:
				/* this may actually be a transport error worthy of disconnection... */
				error->text[0] = '\0';
				return (-2);
			break;
		}
	}
	
	return totalOut;
}

RsslInt32 ripcCloseSSLSocket(void *session)
{
	ripcSSLSession *sess = (ripcSSLSession*)session;

	if (session == NULL)
		return 1;

	/* If the socket is invalid, it already has been closed by curl */
	if(sess->socket != RIPC_INVALID_SOCKET)
		sock_close(sess->socket);
	/* close the SSL connection */
	if (sess->connection)
		(*(sslFuncs.set_shutdown))(sess->connection, RSSL_SSL_SENT_SHUTDOWN| RSSL_SSL_RECEIVED_SHUTDOWN);

	ripcReleaseSSLSession(sess, 0);

	return 1;
}

/* This is used to call socket shutdown instead of sock_close.  Shutdown will gracefully close the channel, allowing any in-flight messages 
   to be read by the other side prior to closing the connection. */
RsslInt32 ripcShutdownSSLSocket(void *session)
{
	ripcSSLSession *sess = (ripcSSLSession*)session;

	if (session == NULL)
		return 1;

	/* If the socket is invalid, it already has been closed by curl */
	if (sess->socket != RIPC_INVALID_SOCKET)
		shutdown(sess->socket, shutdownFlag);
	/* close the SSL connection */
	if(sess->connection)
		(*(sslFuncs.set_shutdown))(sess->connection, RSSL_SSL_SENT_SHUTDOWN | RSSL_SSL_RECEIVED_SHUTDOWN);

	ripcReleaseSSLSession(sess, 0);

	return 1;
}

static int ripcGetProtocolVersion(const OPENSSL_SSL* ssl)
{
	int protocol = (*(sslFuncs.ssl_version))(ssl);
	switch (protocol)
	{
	case RSSL_11_TLS1_2_VERSION:
		return RSSL_ENC_TLSV1_2;
		break;
	case RSSL_11_TLS1_3_VERSION:
		return RSSL_ENC_TLSV1_3;
		break;
	default:
		return RSSL_ENC_NONE;
		break;
	}
}

/* Initializes the OpenSSL session, calling either ssl_accept or ssl_connect depending on if the channel is a server or client, respectively*/
RsslInt32 ripcSSLInit(void *session, ripcSessInProg *inPr, RsslError *error)
{
	RsslInt32 retVal = 0;
	ripcSSLSession *sess = (ripcSSLSession*)session;
	long ret;

	if (sess->server)
	{
		if ((retVal = (*(sslFuncs.ssl_accept))(sess->connection)) <= 0)
		{
			if ((*(sslFuncs.BIO_sock_should_retry))(retVal))
				return 0;

			error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

			if ((retVal == -1) && ((error->sysError == RSSL_SSL_ERROR_WANT_READ) || (error->sysError == RSSL_SSL_ERROR_WANT_WRITE)))
				return 0;
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, error->sysError );
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLInit error on SSL_accept. The client may be attempting to connect with a non-encrypted connection. ", __FILE__, __LINE__);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				return -1;
			}
		}

		/* what happens if this is not true??? */
		if (openSSLAPI == RSSL_OPENSSL_V1_0)
		{
			if ((*(sslFuncs.ssl_state))(sess->connection) == RSSL_10_SSL_ST_OK)
			{
				RsslSocketChannel* chnl = (RsslSocketChannel*)(*(sslFuncs.get_ex_data))(sess->connection, 2);
				if (chnl != NULL)
				{
					chnl->sslCurrentProtocol = ripcGetProtocolVersion(sess->connection);
				}
				return 1;
			}
			else
			{
				/* what do we do here? */
				/* do we need to do something here */
				return 0;
			}
		}
		else if(openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
		{
			if ((*(sslFuncs.ssl_get_state))(sess->connection) == RSSL_TLS_ST_OK)
			{
				RsslSocketChannel* chnl = (RsslSocketChannel*)(*(sslFuncs.get_ex_data))(sess->connection, 2);
				if (chnl != NULL)
				{
					chnl->sslCurrentProtocol = ripcGetProtocolVersion(sess->connection);
				}
				return 1;
			}
			else
			{
				/* what do we do here? */
				/* do we need to do something here */
				return 0;
			}
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
			/* Check to see if the channel has connected, if so, proceed with ssl_connect.  Otherwise, set the state the SSL_INITIALIZNIG,
			and initComplete to 0.  ssl_connect will be called when initTransport is called again.*/
			if ((retVal = ipcConnected(sess->socket)) == -1)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 ripcSSLInit client connect() failed. System errno: %i", __FILE__, __LINE__, errno);
				return -1;
			}

			if (retVal == 0)
			{
				sess->clientConnState = SSL_INITIALIZING;
				return 0;
			}

			if ((retVal = (*(sslFuncs.ssl_connect))(sess->connection)) <= 0)
			{
				/* this would happen if its nonblocking and it needs more action to be taken */
				error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

				if ((retVal == -1) && ((error->sysError == RSSL_SSL_ERROR_WANT_READ) || (error->sysError == RSSL_SSL_ERROR_WANT_WRITE)))
					return 0;
				else
				{
					/* We're in a failure condition here.  First check to see if certificate validation failed.
					  SSL_get_verify_result returns X509_v_OK if there is no certificate, allowing us to drop down to other error handling */
					ret = (*sslFuncs.get_verify_result)(sess->connection);

					if (RSSL_X509_V_OK != ret)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 ripcSSLInit Certificate validation error.  OpenSSL Verification return: %s", __FILE__, __LINE__, (*cryptoFuncs.verify_cert_error_string)(ret));
						ripcSSLErrors(error, (RsslInt32)strlen(error->text));
						return -2;
					}
					else
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2002 ripcSSLInit error on SSL_connect SSL.  Please verify that the server can accept TLS encrypted connections. Error: %i errno: %i ", __FILE__, __LINE__, error->sysError, errno);
						ripcSSLErrors(error, (RsslInt32)strlen(error->text));
						return -1;
					}
				}
			}
			else
			{
				RsslSocketChannel* chnl = (RsslSocketChannel*)(*(sslFuncs.get_ex_data))(sess->connection, 2);
				if (chnl != NULL)
				{
					chnl->sslCurrentProtocol = ripcGetProtocolVersion(sess->connection);
				}

				if (ripcVerifyCert(sess->connection, chnl, error) == RSSL_FALSE)
				{
					return -2;
				}
				sess->clientConnState = SSL_ACTIVE;
				inPr->intConnState = (sess->clientConnState << 8);  
				return 1;
			}
		}
	}

	return 0;
}

void *ripcSSLConnectInt(ripcSSLSession *sess, RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	RsslInt32 retVal = 0;
	long ret;
	struct in_addr addr;
	OPENSSL_X509_VERIFY_PARAM* params;
	RsslSocketChannel* chnl = (RsslSocketChannel*)userSpecPtr;

	if (sess == NULL)
		return 0;

	if ((sess->bio = (*(sslFuncs.BIO_new_socket))((int)sess->socket, RSSL_BIO_NOCLOSE)) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLConnect error creating new BIO socket", __FILE__, __LINE__);
		return 0;
	}

	if ((sess->connection = (*(sslFuncs.ssl_new))(sess->ctx)) == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2000 ripcSSLConnect error creating new SSL connection", __FILE__, __LINE__);
		return 0;
	}

	(*(sslFuncs.ssl_clear))(sess->connection);

	if ((*(sslFuncs.set_cipher_list))(sess->connection, CIPHER_LIST) < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2001 ripcSSLConnect error setting up cipher list (no valid ciphers) (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		return 0;
	}

	/* Setup SNI extension.  Note that we're just going to pass in the hostName as is. */
	if ((*(sslFuncs.ctrl))(sess->connection, RSSL_SSL_CTRL_SET_TLSEXT_HOSTNAME, RSSL_TLSEXT_NAMETYPE_host_name, chnl->hostName) < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2001 ripcSSLConnect Error setting up SNI extension (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		return 0;
	}

	/* allows us to write part of a buffer and not be required to pass in the same address with the next write call */
	if (openSSLAPI == RSSL_OPENSSL_V1_0)
		(*(sslFuncs.ctrl))(sess->connection, RSSL_10_SSL_CTRL_MODE, RSSL_SSL_MODE_ENABLE_PARTIAL_WRITE | RSSL_SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);
	else if (openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
		(*(sslFuncs.set_options))(sess->connection, RSSL_SSL_MODE_ENABLE_PARTIAL_WRITE | RSSL_SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	
	(*(sslFuncs.set_bio))(sess->connection, sess->bio, sess->bio);
	(*(sslFuncs.set_connect_state))(sess->connection);

	(*(sslFuncs.set_ex_data))(sess->connection, 2, userSpecPtr);  // Set the rsslSocketChannel for this connection.  This contains information for certificate verification and set the negotiated TLS protocol version.

	
	/* Setup hostname validation here */
	if (openSSLAPI == RSSL_OPENSSL_V1_1 || openSSLAPI == RSSL_OPENSSL_V3)
	{
		/* Check to see if the hostname on the channel is an IP Address*/
		if (inet_pton(AF_INET, ((RsslSocketChannel*)userSpecPtr)->hostName, &addr))
		{
			params = (*sslFuncs.ssl_get0_param)(sess->connection);

			if (params == NULL)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2000 ripcSSLConnect Unable to get ssl certificate verification parameters", __FILE__, __LINE__);
				return 0;
			}

			if ((*cryptoFuncs.x509_verify_param_set1_ip_asc)(params, ((RsslSocketChannel*)userSpecPtr)->hostName) != 1)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2001 ripcSSLConnect Unable to set IP address verification on SSL object", __FILE__, __LINE__);
				return 0;
			}
		}
		else
		{
			if ((*sslFuncs.ssl_set1_host)(sess->connection, ((RsslSocketChannel*)userSpecPtr)->hostName) != 1)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 2001 ripcSSLConnect Unable to set hostName verification on ssl object", __FILE__, __LINE__);
				return 0;
			}
			(*sslFuncs.set_hostflags)(sess->connection, RSSL_11_X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
		}
	}
	/* Check to see if the channel has connected, if so, proceed with ssl_connect.  Otherwise, set the state the SSL_INITIALIZNIG,
	  and initComplete to 0.  ssl_connect will be called when initTransport is called again.*/
	if ((retVal = ipcConnected(fd)) == -1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d>  Error: 1002 ripcSSLConnect() client connect() failed.  System errno: (%d)\n", __FILE__, __LINE__, errno);
		return 0;
	}

	if (retVal == 0)
	{
		sess->clientConnState = SSL_INITIALIZING;

		*initComplete = 0;
		return sess;
	}

	if ((retVal = (*(sslFuncs.ssl_connect))(sess->connection)) <= 0)
	{
		/* this would happen if its nonblocking and it needs more action to be taken */
		error->sysError = (*(sslFuncs.get_error))(sess->connection, retVal);

		if ((retVal == -1) && ((error->sysError == RSSL_SSL_ERROR_WANT_READ) || (error->sysError == RSSL_SSL_ERROR_WANT_WRITE)))
		{
			/* This is not an error condition, ssl_connect has started, and further triggers will be either on a read or write. */
			*initComplete = 0;
			return sess;
		}
		else
		{
			/* We're in a failure condition here.  First check to see if certificate validation failed.
			SSL_get_verify_result returns X509_v_OK if there is no certificate, allowing us to drop down to other error handling */
			ret = (*sslFuncs.get_verify_result)(sess->connection);

			if (RSSL_X509_V_OK != ret)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 Certificate validation error.  OpenSSL Verification return: %s", __FILE__, __LINE__, (*cryptoFuncs.verify_cert_error_string)(ret));
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				return 0;
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errno);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 error on SSL_connect SSL Error: %i errno: %i ", __FILE__, __LINE__, error->sysError, errno);
				ripcSSLErrors(error, (RsslInt32)strlen(error->text));
				return 0;
			}
		}

	}
	else /* This else implies that we are doing blocking. */
	{
		if(ripcVerifyCert(sess->connection, (RsslSocketChannel*)userSpecPtr, error) == RSSL_FALSE)
		{
			return 0;
		}

		if (chnl != NULL)
		{
			chnl->sslCurrentProtocol = ripcGetProtocolVersion(sess->connection);
		}

		sess->clientConnState = SSL_ACTIVE;
		*initComplete = 1;
	}

	return sess;
}

void *ripcSSLConnectTLSv12(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	ripcSSLSession *sess = ripcSSLNewSession(fd, (RsslSocketChannel*)userSpecPtr, NULL, error);

	if (sess == NULL)
		return NULL;

	sess->ctx = ripcSSLCreateCTXClientV1_0(error);

	if (sess->ctx == NULL)
		return NULL;

	/* setup the CTX - we are the client */
	sess->ctx = ripcSSLSetupCTXClient(sess->ctx, (RsslSocketChannel*)userSpecPtr, error);

	if (sess->ctx == NULL)
		return NULL;

	(*(sslFuncs.ctx_set_ex_data))(sess->ctx, 0, sess);

	return ripcSSLConnectInt(sess, fd, initComplete, userSpecPtr, error);
}

void* ripcSSLConnectTLS(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	ripcSSLSession *sess = ripcSSLNewSession(fd, (RsslSocketChannel*)userSpecPtr, NULL, error);

	if (sess == NULL)
		return NULL;

	sess->ctx = ripcSSLCreateCTXClientV1_1(((RsslSocketChannel*)userSpecPtr)->sslProtocolBitmap, error);

	if (sess->ctx == NULL)
		return NULL;

	/* setup the CTX - we are the client */
	sess->ctx = ripcSSLSetupCTXClient(sess->ctx, (RsslSocketChannel*)userSpecPtr, error);

	if (sess->ctx == NULL)
		return NULL;

	(*(sslFuncs.ctx_set_ex_data))(sess->ctx, 0, sess);

	return ripcSSLConnectInt(sess, fd, initComplete, userSpecPtr, error);
}


RsslInt32  ripcSSLReconnection(void *session, RsslError *error)
{
	return 1;
}

void *ripcNewSSLSocket(void *server, RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr,  RsslError *error)
{
	ripcSSLSession *newsess = (ripcSSLSession*)ripcSSLNewSession(fd, (RsslSocketChannel*)userSpecPtr, server, error);
	ripcSSLServer *srvr = (ripcSSLServer*)server;
	char* cipherList = 0;
	
	if (newsess == 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1001 ripcSSLNewSocket could not allocate ripcSSLSession.", __FILE__, __LINE__);
		return 0;
	}

	*initComplete = 0;

	newsess->connection = (*(sslFuncs.ssl_new))(srvr->ctx);
	if (newsess->connection == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLNewSocket could get SSL object", __FILE__, __LINE__);
		ripcReleaseSSLSession((void*)newsess, error);
		return 0;
	}


	(*(sslFuncs.ssl_clear))(newsess->connection);

	if (srvr->cipherSuite == NULL)
		cipherList = CIPHER_LIST;
	else
		cipherList = srvr->cipherSuite;

	if ((*(sslFuncs.set_cipher_list))(newsess->connection, cipherList) < 1)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2001 ripcSSLNewSocket error setting up cipher list (no valid ciphers) (errno %d)", __FILE__, __LINE__, errno);
		ripcSSLErrors(error, (RsslInt32)strlen(error->text));
		ripcReleaseSSLSession((void*)newsess, error);
		return 0;
	}

	/* allows us to write part of a buffer and not be required to pass in the same address with the next write call */
	(*(sslFuncs.ctrl))(newsess->connection, RSSL_10_SSL_CTRL_MODE, RSSL_SSL_MODE_ENABLE_PARTIAL_WRITE | RSSL_SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);

	(*(sslFuncs.set_ex_data))(newsess->connection, 1, newsess);
	(*(sslFuncs.set_ex_data))(newsess->connection, 0, newsess);

	/* Set the rsslSocketChannel for this connection. This is used to set the negotiated TLS protocol version.*/
	(*(sslFuncs.set_ex_data))(newsess->connection, 2, userSpecPtr);

	newsess->bio = (*(sslFuncs.BIO_new_socket))((int)fd, RSSL_BIO_NOCLOSE);
	if (newsess->bio == NULL)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 2000 ripcSSLNewSocket could not get new socket BIO", __FILE__, __LINE__);
		ripcReleaseSSLSession((void*)newsess, error);
		return 0;
	}

	(*(sslFuncs.set_bio))(newsess->connection, newsess->bio, newsess->bio);
	(*(sslFuncs.set_accept_state))(newsess->connection);

	*initComplete = RSSL_FALSE;

	return newsess;
}

RsslInt32 ripcReleaseSSLServer(void* srvr, RsslError *error)
{
	if (srvr)
	{
		ripcSSLServer *server=(ripcSSLServer*)srvr;
		
		if (server->ctx)
			(*(sslFuncs.ctx_free))(server->ctx);

		_rsslFree(server);

		++countReleaseSSLServer;
	}

	return 1;
}

RsslInt32 ripcReleaseSSLSession(void* session, RsslError *error)
{
	if (session)
	{

		ripcSSLSession *sess=(ripcSSLSession*)session;
		/* release the SSL* */
		if (sess->connection)
		{
            (*(sslFuncs.set_ex_data))(sess->connection, 0, NULL);
			(*(sslFuncs.ssl_free))(sess->connection);
			sess->connection = 0;
            sess->bio = 0;
		}
		if (sess->ctx)
		{
			(*(sslFuncs.ctx_free))(sess->ctx);
			sess->ctx = 0;
		}
        
	
		sess->server = 0;

		_rsslFree(sess);
	}
	return 1;
}

ripcSSLServer *ripcInitializeSSLServer(RsslServerSocketChannel* chnl, RsslError  *error)
{
	/* how do we pass in the connect options here??? */
	ripcSSLServer *server = ripcSSLNewServer(chnl, error);

	if (server == 0)
		return 0;

	/* setup the CTX - we are the server */
	server->ctx = ripcSSLSetupCTXServer(chnl,  error);
	if (server->ctx == 0)
	{
		/* release server */
		ripcReleaseSSLServer(server, 0);
		return 0;
	}

	(*(sslFuncs.ctx_set_ex_data))(server->ctx, 0, server);

	/* not setting the cache size - right now that will allow 20000 sessions 
	   - this is extreme, need a better idea of actual number of sessions to allow */

	++countInitializeSSLServer;
	return server;
}

RsslInt32 ripcGetCountInitializeSSLServer()
{
	return countInitializeSSLServer;
}

RsslInt32 ripcGetCountReleaseSSLServer()
{
	return countReleaseSSLServer;
}
