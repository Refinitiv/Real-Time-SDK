/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef __ripcssljit_h
#define __ripcssljit_h
 
#ifndef WIN32
 
#define USE_SOCKETS
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/bn.h>

#include "rtr/os.h"
#include "rtr/ripch.h"
#include "rtr/ripcutils.h"

#ifdef _WIN32
/* we should pick up windows.h from ripch.h */
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined( WIN32 )

	#define RIPC_DLOPEN(FILENAME)				LoadLibrary(FILENAME)
	#define RIPC_DLCLOSE( HANDLE )				FreeLibrary( HANDLE )
	#define RIPC_DLSYM(MODULE,FUNCNAME)		GetProcAddress((MODULE), (FUNCNAME))

#elif defined(LINUX)

	#define RIPC_DLOPEN(FILENAME)				dlopen(FILENAME, RTLD_NOW | RTLD_GLOBAL)
	#define RIPC_DLCLOSE( HANDLE )				dlclose( HANDLE )
	#define RIPC_DLSYM(MODULE,FUNCNAME)			dlsym((MODULE), (FUNCNAME))

#else
	#error Not implemented
#endif

typedef int (*verifyCallback)(int, X509_STORE_CTX*);

typedef DH* (*dhVerifyCallback)(SSL*, int, int);

typedef struct
{
	int initialized;
	int (*library_init)(void);  		/* SSL_library_init */
	void (*load_error_strings)(); 		/* SSL_load_error_strings */
	void (*load_crypto_strings)(); 		/* crypto::ERR_load_crypto_strings */
	SSL* (*ssl_new)(SSL_CTX*);			/* SSL_new */
	int (*ssl_clear)(SSL*);				/* SSL_clear */
	int (*ssl_connect)(SSL*);			/* SSL_connect */
	int (*ssl_accept)(SSL*);			/* SSL_accept */
	int (*ssl_read)(SSL*, void*, int);	/* SSL_read */
	int (*get_error)(const SSL*, int);	/* SSL_get_error */
	int (*ssl_write)(SSL*, void*, int);	/* SSL_write */
	long (*get_verify_result)(SSL*);	/* SSL_get_verify_result */
	void (*set_shutdown)(SSL*, int);	/* SSL_set_shutdown */
	int (*set_cipher_list)(SSL*, char*);	/* SSL_set_cipher_list */
	long (*ctrl)(SSL*, int, long, void*);	/* SSL_ctrl */
	void (*set_bio)(SSL*, BIO*, BIO*);		/* SSL_set_bio */
	void (*set_connect_state)(SSL*);		/* SSL_set_connect_state */
	int (*set_ex_data)(SSL*, int, void*);	/* SSL_set_ex_data */
	void* (*get_ex_data)(SSL*, int);			/* SSL_get_ex_data */
	void (*set_accept_state)(SSL*);			/* SSL_set_accept_state */
	void (*ssl_free)(SSL*);				/* SSL_free */
	int (*ssl_state)(const SSL*);		/* SSL_state */
	const SSL_METHOD* (*TLSv1_client_method)();  /* TLSv1_client_method */
	const SSL_METHOD* (*TLSv1_1_client_method)();
	const SSL_METHOD* (*TLSv1_2_client_method)();
} ripcSSLApiFuncs;

#define INIT_SSL_API_FUNCS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

typedef struct
{
	int initialized;
	SSL_CTX* (*ctx_new)(const SSL_METHOD*);									/* SSL_CTX_new */
	void (*ctx_set_quiet_shutdown)(SSL_CTX*, int);							/* SSL_CTX_set_quiet_shutdown */
	int (*ctx_set_cipher_list)(SSL_CTX*, const char*);						/* SSL_CTX_set_cipher_list */
	int (*ctx_load_verify_location)(SSL_CTX*, const char*, const char*); 	/* SSL_CTX_load_verify_locations */
	int (*ctx_set_default_verify_paths)(SSL_CTX*);							/* SSL_CTX_set_default_verify_paths */
	int (*ctx_use_cert_chain_file)(SSL_CTX*, const char*);					/* SSL_CTX_use_certificate_chain_file */
	int (*ctx_use_privatekey_file)(SSL_CTX*, const char*, int);				/* SSL_CTX_use_PrivateKey_file */
	void (*ctx_set_verify)(SSL_CTX*, int, verifyCallback);	/* SSL_CTX_set_verify */
	void (*ctx_set_tmp_dh_callback)(SSL_CTX*, dhVerifyCallback);	/* SSL_CTX_set_tmp_dh_callback */
	void* (*ctx_set_ex_data)(SSL_CTX*, int, void*);		/* SSL_CTX_set_ex_data */
	void (*ctx_free)(SSL_CTX*);												/* SSL_CTX_free */
	long (*ctx_ctrl)(SSL_CTX*, int, long, void*);							/* SSL_CTX_ctrl */
}ripcSSLCTXApiFuncs;

#define INIT_SSL_CTX_FUNCS {0,0,0,0,0,0,0,0,0,0,0,0,0}

typedef struct
{
	int initialized;
	int (*v3_add_standard_extensions)();				/* X509V3_add_standard_extensions */
	unsigned long (*thread_id)();						/* CRYPTO_thread_id */
	unsigned long (*get_error_line_data)(const char**, int*, const char**, int*); /* ERR_get_error_line_data */
	void (*error_string_n)(unsigned long, char*, size_t);	/* ERR_error_string_n */
	void* (*X509_get_ex_data)(X509_STORE_CTX*, int);		/* X509_STORE_CTX_get_ex_data */
	X509* (*get_current_cert)(X509_STORE_CTX*);		/* X509_STORE_CTX_get_current_cert */
	int (*get_error_depth)(X509_STORE_CTX*);		/* X509_STORE_CTX_get_error_depth */
	int (*get_error)(X509_STORE_CTX*);				/* X509_STORE_CTX_get_error */
	char* (*name_oneline)(X509_NAME*, char*, int);	/* X509_NAME_oneline */
	X509_NAME* (*get_subject_name)(X509*);			/* X509_get_subject_name */
	X509_NAME* (*get_issuer_name)(X509*);		/* X509_get_issuer_name */
	const char* (*verify_cert_error_string)(long);	/* X509_verify_cert_error_string */
	DH* (*read_bio_dhparams)(BIO*, DH**, pem_password_cb*, void*); /* PEM_read_bio_DHparams */
	DH* (*dh_new)();							/* DH_new */
	BIGNUM* (*bin2bn)(const unsigned char*, int, BIGNUM*);	/* BN_bin2bn */
	void (*dh_free)(DH*);						/* DH_free */
	void (*rand_seed)(const void*, int);		/* RAND_seed */
	void (*err_free_strings)(void);				/* ERR_free_strings */
	void (*err_remove_state)(int);				/* ERR_remove_state */
	void (*engine_cleanup)(void);				/* ENGINE_cleanup */
	void (*crypto_cleanup_all_ex_data)(void);  /* CRYPTO_cleanup_all_ex_data */
	void (*evp_cleanup)(void);					/* EVP_cleanup */
}ripcCryptoApiFuncs;

#define INIT_CRYPTO_API_FUNCS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

typedef struct
{
	int initialized;
	BIO* (*new_socket)(int, int);						/* BIO_new_socket */
	int (*sock_should_retry)(int);						/* BIO_sock_should_retry */
	BIO* (*new_file)(const char*, const char*);			/* BIO_new_file */
	int (*bio_free)(BIO*);								/* BIO_free */
}ripcSSLBIOApiFuncs;

#define INIT_SSL_BIO_API_FUNCS {0,0,0,0,0}

#endif
#endif

