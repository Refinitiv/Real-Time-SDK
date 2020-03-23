/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef __ripcssljit_h
#define __ripcssljit_h

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

/* This file is intended to provide an abstraction layer for both OpenSSL 1.1.0 and OpenSSL 1.0.2's API */

#define RSSL_SSL_VERIFY_NONE	0x00		// SSL_VERIFY_NONE
#define RSSL_10_SSL_OP_ALL		0x80000BFFL	// SSL_OP_ALL in 1.0.2
#define RSSL_10_SSL_OP_NO_SSLv2 0x01000000L // SSL_OP_NO_SSLv2 in 1.0.2
#define RSSL_10_SSL_OP_NO_SSLv3 0x02000000L // SSL_OP_NO_SSLv3 in 1.0.2
#define RSSL_10_SSL_OP_NO_TLSv1 0x04000000L // SSL_OP_NO_SSLv2 in 1.0.2
#define RSSL_10_SSL_OP_NO_TLSv1_1 0x10000000L // SSL_OP_NO_SSLv3 in 1.0.2
#define RSSL_SSL_FILETYPE_PEM  1			// SSL_FILETYPE_PEM
#define RSSL_SSL_VERIFY_PEER	0x1		//  SSL_VERIFY_PEER
#define RSSL_SSL_VERIFY_FAIL_IF_NO_PEER_CERT 0x02 //SSL_VERIFY_FAIL_IF_NO_PEER_CERT
#define RSSL_10_SSL_CTRL_OPTIONS 32		//SSL_CTRL_OPTIONS for 1.0.2
#define RSSL_SSL_ERROR_NONE		0		// SSL_ERROR_NONE
#define RSSL_SSL_ERROR_WANT_WRITE 3	//SSL_ERROR_WANT_WRITE
#define RSSL_SSL_ERROR_WANT_READ  2 //SSL_ERROR_WANT_READ
#define RSSL_SSL_ERROR_WANT_X509_LOOKUP 4 //SSL_ERROR_WANT_X509_LOOKUP
#define RSSL_SSL_ERROR_SYSCALL 5 //SSL_ERROR_SYSCALL
#define RSSL_SSL_ERROR_SSL 1 //SSL_ERROR_SSL
#define RSSL_SSL_ERROR_ZERO_RETURN 6 //SSL_ERROR_ZERO_RETURN
#define RSSL_SSL_SENT_SHUTDOWN 1	//SSL_SENT_SHUTDOWN
#define RSSL_SSL_RECEIVED_SHUTDOWN 2 //SSL_RECEIVED_SHUTDOWN
#define RSSL_X509_V_OK 0 //X509_V_OK
#define RSSL_10_SSL_ST_OK 0x03	//SSL_ST_OK from 1.0.2
#define RSSL_BIO_NOCLOSE 0x00	//BIO_NOCLOSE
#define RSSL_10_SSL_CTRL_MODE 33 //SSL_CTRL_MODE for 1.0.2
#define RSSL_SSL_CTRL_SET_TMP_ECDH 4 //SSL_CTRL_SET_TMP_ECDH

#define RSSL_SSL_CTRL_SET_TMP_DH 3	// SSL_CTRL_SET_TMP_DH 
#define RSSL_SSL_MODE_ENABLE_PARTIAL_WRITE 0x00000001L //SSL_MODE_ENABLE_PARTIAL_WRITE
#define RSSL_SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER 0x00000002L //SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER

#define RSSL_X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT 18 //X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
#define RSSL_X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN 19 //X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN
#define RSSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY 20 //X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
#define RSSL_X509_V_ERR_CERT_UNTRUSTED 27 //X509_V_ERR_CERT_UNTRUSTED
#define RSSL_X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE 21 //X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE

#define RSSL_SSL_ERR_TXT_STRING 0x02 //ERR_TXT_STRING

#define RSSL_11_SSL_OP_ALL 0x80000854U // Combination of flags from SSL_OP_ALL in 1.1.0	

#define RSSL_NID_subject_alt_name 85 //NID_subject_alt_name
#define RSSL_NID_commonName 13 // NID_commonName

#define RSSL_10_TLS1_VERSION                    0x0301
#define RSSL_10_TLS1_1_VERSION                  0x0302
#define RSSL_10_TLS1_2_VERSION                  0x0303

#define RSSL_11_TLS1_VERSION                    0x0301
#define RSSL_11_TLS1_1_VERSION                  0x0302
#define RSSL_11_TLS1_2_VERSION                  0x0303

# define RSSL_11_SSL_CTRL_SET_MIN_PROTO_VERSION          123
# define RSSL_11_SSL_CTRL_SET_MAX_PROTO_VERSION          124


/* OpenSSL 1.1.X state table for connections. */
typedef enum {
	RSSL_TLS_ST_BEFORE,
	RSSL_TLS_ST_OK,
	RSSL_DTLS_ST_CR_HELLO_VERIFY_REQUEST,
	RSSL_TLS_ST_CR_SRVR_HELLO,
	RSSL_TLS_ST_CR_CERT,
	RSSL_TLS_ST_CR_CERT_STATUS,
	RSSL_TLS_ST_CR_KEY_EXCH,
	RSSL_TLS_ST_CR_CERT_REQ,
	RSSL_TLS_ST_CR_SRVR_DONE,
	RSSL_TLS_ST_CR_SESSION_TICKET,
	RSSL_TLS_ST_CR_CHANGE,
	RSSL_TLS_ST_CR_FINISHED,
	RSSL_TLS_ST_CW_CLNT_HELLO,
	RSSL_TLS_ST_CW_CERT,
	RSSL_TLS_ST_CW_KEY_EXCH,
	RSSL_TLS_ST_CW_CERT_VRFY,
	RSSL_TLS_ST_CW_CHANGE,
	RSSL_TLS_ST_CW_NEXT_PROTO,
	RSSL_TLS_ST_CW_FINISHED,
	RSSL_TLS_ST_SW_HELLO_REQ,
	RSSL_TLS_ST_SR_CLNT_HELLO,
	RSSL_DTLS_ST_SW_HELLO_VERIFY_REQUEST,
	RSSL_TLS_ST_SW_SRVR_HELLO,
	RSSL_TLS_ST_SW_CERT,
	RSSL_TLS_ST_SW_KEY_EXCH,
	RSSL_TLS_ST_SW_CERT_REQ,
	RSSL_TLS_ST_SW_SRVR_DONE,
	RSSL_TLS_ST_SR_CERT,
	RSSL_TLS_ST_SR_KEY_EXCH,
	RSSL_TLS_ST_SR_CERT_VRFY,
	RSSL_TLS_ST_SR_NEXT_PROTO,
	RSSL_TLS_ST_SR_CHANGE,
	RSSL_TLS_ST_SR_FINISHED,
	RSSL_TLS_ST_SW_SESSION_TICKET,
	RSSL_TLS_ST_SW_CERT_STATUS,
	RSSL_TLS_ST_SW_CHANGE,
	RSSL_TLS_ST_SW_FINISHED
} RSSL_11_OSSL_HANDSHAKE_STATE;

# define RSSL_11_X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS 0x4 /* X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS in OpenSSL 1.1.0 */


typedef struct openssl_x509_store_ctx OPENSSL_X509_STORE_CTX;
typedef struct open_ssl OPENSSL_SSL;
typedef struct openssl_11_dh OPENSSL_11_DH;
typedef struct openssl_dh_method OPENSSL_DH_METHOD;
typedef struct openssl_bn_mont_ctx OPENSSL_10_BN_MONT_CTX;
typedef struct openssl_ctx OPENSSL_SSL_CTX;
typedef struct openssl_bio OPENSSL_BIO;
typedef struct openssl_engine OPENSSL_ENGINE;
/* Windows defined X509_NAME in crypto.h. To avoid a type redefinition(since we are only passing pointers that have been allocated by OpenSSL), 
  this is renamed to OPENSSL_X509_NAME */
typedef struct openSSL_x509_name OPENSSL_X509_NAME;
typedef struct openSSL_x509_name_entry OPENSSL_X509_NAME_ENTRY;
typedef struct openssl_x509 OPENSSL_X509;
typedef struct openssl_method OPENSSL_SSL_METHOD;
typedef struct openssl_bignum OPENSSL_BIGNUM;
typedef struct openssl_x509_store	OPENSSL_X509_STORE;
typedef struct openssl_x509_verify_param OPENSSL_X509_VERIFY_PARAM;

typedef struct openssl_stack OPENSSL_STACK;
#define _STACK OPENSSL_STACK;

typedef struct openssl_othername OPENSSL_OTHERNAME;   /* otherName */
typedef struct openssl_asn1_ia5string OPENSSL_ASN1_IA5STRING;
typedef struct openssl_asn1_type OPENSSL_ASN1_TYPE;
typedef struct openssl_edipartyname OPENSSL_EDIPARTYNAME;
typedef struct openssl_asn1_octet_string OPENSSL_ASN1_OCTET_STRING;
typedef struct openssl_asn1_object OPENSSL_ASN1_OBJECT;
typedef struct openssl_asn1_string OPENSSL_ASN1_STRING;

typedef struct openssl_ec_key_st OPENSSL_EC_KEY;

typedef int (*verifyCallback)(int, OPENSSL_X509_STORE_CTX*);

typedef int pem_password_cb(char*, int, int, void*);


/* OpenSSL 1.0.X elliptic curve default */
#define RSSL_10_NID_X9_62_prime256v1 415

/* Types for GENERAL_NAME struct*/
# define RSSL_SSL_GEN_OTHERNAME   0
# define RSSL_SSL_GEN_EMAIL       1
# define RSSL_SSL_GEN_DNS         2
# define RSSL_SSL_GEN_X400        3
# define RSSL_SSL_GEN_DIRNAME     4
# define RSSL_SSL_GEN_EDIPARTY    5
# define RSSL_SSL_GEN_URI         6
# define RSSL_SSL_GEN_IPADD       7
# define RSSL_SSL_GEN_RID         8

/* Definitions for TLS SNI support */
#define RSSL_SSL_CTRL_SET_TLSEXT_HOSTNAME  55
#define RSSL_TLSEXT_NAMETYPE_host_name     0

typedef struct OPENSSL_10_CRYPTO_EX_DATA_st {
	OPENSSL_STACK *sk;
	int dummy;
}OPENSSL_10_CRYPTO_EX_DATA;

typedef struct OPENSSL_GENERAL_NAME_st {

	int type;
	union {
		char *ptr;
		OPENSSL_OTHERNAME *otherName;   /* otherName */
		OPENSSL_ASN1_IA5STRING *rfc822Name;
		OPENSSL_ASN1_IA5STRING *dNSName;
		OPENSSL_ASN1_TYPE *x400Address;
		OPENSSL_X509_NAME *directoryName;
		OPENSSL_EDIPARTYNAME *ediPartyName;
		OPENSSL_ASN1_IA5STRING *uniformResourceIdentifier;
		OPENSSL_ASN1_OCTET_STRING *iPAddress;
		OPENSSL_ASN1_OBJECT *registeredID;
		/* Old names */
		OPENSSL_ASN1_OCTET_STRING *ip;  /* iPAddress */
		OPENSSL_X509_NAME *dirn;        /* dirn */
		OPENSSL_ASN1_IA5STRING *ia5;    /* rfc822Name, dNSName,
										* uniformResourceIdentifier */
		OPENSSL_ASN1_OBJECT *rid;       /* registeredID */
		OPENSSL_ASN1_TYPE *other;       /* x400Address */
	} d;
} OPENSSL_GENERAL_NAME;

typedef struct OPENSSL_10_DH_st
{
	/*
	* This first argument is used to pick up errors when a DH is passed
	* instead of a EVP_PKEY
	*/
	int pad;
	int version;
	OPENSSL_BIGNUM *p;
	OPENSSL_BIGNUM *g;
	long length;                /* optional */
	OPENSSL_BIGNUM *pub_key;            /* g^x % p */
	OPENSSL_BIGNUM *priv_key;           /* x */
	int flags;
	OPENSSL_10_BN_MONT_CTX *method_mont_p;
	OPENSSL_BIGNUM *q;
	OPENSSL_BIGNUM *j;
	unsigned char *seed;
	int seedlen;
	OPENSSL_BIGNUM *counter;
	int references;
	OPENSSL_10_CRYPTO_EX_DATA ex_data;
	const OPENSSL_DH_METHOD *meth;
	OPENSSL_ENGINE *engine;
} OPENSSL_10_DH;

typedef enum
{
	RSSL_OPENSSL_VNONE = 0,
	RSSL_OPENSSL_V1_0 = 1,
	RSSL_OPENSSL_V1_1 = 2
} RsslOpenSSLAPIVersion;

typedef struct
{
	int initialized;
	unsigned long (*ssl_10_version)(void); /* OpenSSL V1.0.X version SSLeay */
	unsigned long (*ssl_11_version)();	/* OpenSSL V1.1.X version OpenSSL_version_num */
	int (*library_init)(void);  		/* V1.0.X SSL_library_init */
	void (*load_error_strings)(); 		/* V1.0.X SSL_load_error_strings */
	OPENSSL_SSL* (*ssl_new)(OPENSSL_SSL_CTX*);			/* SSL_new */
	int (*ssl_clear)(OPENSSL_SSL*);				/* SSL_clear */
	int (*ssl_connect)(OPENSSL_SSL*);			/* SSL_connect */
	int (*ssl_accept)(OPENSSL_SSL*);			/* SSL_accept */
	int (*ssl_read)(OPENSSL_SSL*, void*, int);	/* SSL_read */
	int (*get_error)(const OPENSSL_SSL*, int);	/* SSL_get_error */
	int (*ssl_write)(OPENSSL_SSL*, void*, int);	/* SSL_write */
	long (*get_verify_result)(OPENSSL_SSL*);	/* SSL_get_verify_result */
	void (*set_shutdown)(OPENSSL_SSL*, int);	/* SSL_set_shutdown */
	int (*set_cipher_list)(OPENSSL_SSL*, const char*);	/* SSL_set_cipher_list */
	long (*ctrl)(OPENSSL_SSL*, int, long, void*);	/* SSL_ctrl */
	long (*set_options)(OPENSSL_SSL*, long);		/* SSL_set_options */
	void (*set_bio)(OPENSSL_SSL*, OPENSSL_BIO*, OPENSSL_BIO*);		/* SSL_set_bio */
	void (*set_connect_state)(OPENSSL_SSL*);		/* SSL_set_connect_state */
	int (*set_ex_data)(OPENSSL_SSL*, int, void*);	/* SSL_set_ex_data */
	void* (*get_ex_data)(OPENSSL_SSL*, int);			/* SSL_get_ex_data */
	void (*set_accept_state)(OPENSSL_SSL*);			/* SSL_set_accept_state */
	void (*ssl_free)(OPENSSL_SSL*);				/* SSL_free */
	int (*ssl_state)(const OPENSSL_SSL*);		/* SSL_state */
	int (*ssl_set1_host)(OPENSSL_SSL*, const char*); /* V1.1.X SSL_set1_host for host validation.*/
	OPENSSL_X509_VERIFY_PARAM* (*ssl_get0_param)(OPENSSL_SSL*);	/* V1.1 SSL_get0_param for hostname verification */
	void (*set_hostflags)(OPENSSL_SSL*, unsigned int); /* V1.1.X SSL_set_hostflags*/
	OPENSSL_X509* (*ssl_get_peer_cert)(const OPENSSL_SSL*); /* SSL_get_peer_certificate*/
	RSSL_11_OSSL_HANDSHAKE_STATE (*ssl_get_state)(const OPENSSL_SSL*); /* 1.1 SSL_get_state*/
	const OPENSSL_SSL_METHOD* (*TLS_client_method)();    /* V1.1.X auto-negotiated method */
	const OPENSSL_SSL_METHOD* (*TLSv1_client_method)();  /* TLSv1_client_method */
	const OPENSSL_SSL_METHOD* (*TLSv1_1_client_method)();
	const OPENSSL_SSL_METHOD* (*TLSv1_2_client_method)();
	const OPENSSL_SSL_METHOD* (*SSLv23_server_method)(); /* V1.0.X auto-negoitated server method */
	const OPENSSL_SSL_METHOD* (*TLS_server_method)(); /* V1.1.X auto-negotiated server method*/


	OPENSSL_SSL_CTX* (*ctx_new)(const OPENSSL_SSL_METHOD*);									/* SSL_CTX_new */
	void (*ctx_set_quiet_shutdown)(OPENSSL_SSL_CTX*, int);							/* SSL_CTX_set_quiet_shutdown */
	int (*ctx_set_cipher_list)(OPENSSL_SSL_CTX*, const char*);						/* SSL_CTX_set_cipher_list */
	int (*ctx_load_verify_location)(OPENSSL_SSL_CTX*, const char*, const char*); 	/* SSL_CTX_load_verify_locations */
	int (*ctx_set_default_verify_paths)(OPENSSL_SSL_CTX*);							/* SSL_CTX_set_default_verify_paths */
	void (*ctx_set_cert_store)(OPENSSL_SSL_CTX*, OPENSSL_X509_STORE*);				/* SSL_CTX_set_cert_store */
	OPENSSL_X509_STORE* (*ctx_get_cert_store)(OPENSSL_SSL_CTX*);					/* SSL_CTX_get_cert_store */
	int (*ctx_use_cert_chain_file)(OPENSSL_SSL_CTX*, const char*);					/* SSL_CTX_use_certificate_chain_file */
	int (*ctx_use_privatekey_file)(OPENSSL_SSL_CTX*, const char*, int);				/* SSL_CTX_use_PrivateKey_file */
	void (*ctx_set_verify)(OPENSSL_SSL_CTX*, int, verifyCallback);	/* SSL_CTX_set_verify */
	int (*ctx_set_ex_data)(OPENSSL_SSL_CTX*, int, void*);		/* SSL_CTX_set_ex_data */
	void (*ctx_free)(OPENSSL_SSL_CTX*);												/* SSL_CTX_free */
	long (*ctx_ctrl)(OPENSSL_SSL_CTX*, int, long, void*);							/* SSL_CTX_ctrl */
	long (*ctx_set_options)(OPENSSL_SSL_CTX*, unsigned long);					/* SSL_CTX_set_options */


	OPENSSL_BIO* (*BIO_new_socket)(int, int);						/* BIO_new_socket */
	int(*BIO_sock_should_retry)(int);						/* BIO_sock_should_retry */
	OPENSSL_BIO* (*BIO_new_file)(const char*, const char*);			/* BIO_new_file */
	int(*BIO_free)(OPENSSL_BIO*);								/* BIO_free */
} ripcSSLApiFuncs;


typedef struct
{
	int initialized;
	void (*load_crypto_strings)(); 		/* V1.0.X crypto::ERR_load_crypto_strings */
	unsigned long (*thread_id)();						/* CRYPTO_thread_id */
	unsigned long (*get_error_line_data)(const char**, int*, const char**, int*); /* ERR_get_error_line_data */
	unsigned long (*err_get_error)(void);				/* ERR_get_error */
	unsigned long (*err_peek_error)(void);			/* ERR_peek_error */
	void (*err_print_errors_fp)(FILE*);				/* ERR_print_errors_fp */				
	char* (*err_error_string)(unsigned long, char*);	/* ERR_error_string */
	void* (*X509_get_ex_data)(OPENSSL_X509_STORE_CTX*, int);		/* X509_STORE_CTX_get_ex_data */
	OPENSSL_X509* (*get_current_cert)(OPENSSL_X509_STORE_CTX*);		/* X509_STORE_CTX_get_current_cert */
	OPENSSL_X509_STORE* (*X509_store_new)(void);			/* X509_STORE_new */
	void (*X509_free)(OPENSSL_X509*);						/* X509_free */
	OPENSSL_X509* (*d2i_X509)(OPENSSL_X509**, const unsigned char**, long len); /* d2i_X509 */
	int (*X509_STORE_add_cert)(OPENSSL_X509_STORE*, OPENSSL_X509*); /* X509_STORE_add_cert */
	int (*get_error_depth)(OPENSSL_X509_STORE_CTX*);		/* X509_STORE_CTX_get_error_depth */
	int (*get_error)(OPENSSL_X509_STORE_CTX*);				/* X509_STORE_CTX_get_error */
	char* (*name_oneline)(OPENSSL_X509_NAME*, char*, int);	/* X509_NAME_oneline */
	OPENSSL_X509_NAME* (*get_subject_name)(OPENSSL_X509*);			/* X509_get_subject_name */
	OPENSSL_X509_NAME* (*get_issuer_name)(OPENSSL_X509*);		/* X509_get_issuer_name */
	const char* (*verify_cert_error_string)(long);	/* X509_verify_cert_error_string */
	OPENSSL_10_DH* (*read_bio_dhparams_10)(OPENSSL_BIO*, OPENSSL_10_DH**, pem_password_cb*, void*); /* PEM_read_bio_DHparams */
	OPENSSL_10_DH* (*dh_new_10)();							/* DH_new */
	void (*dh_free_10)(OPENSSL_10_DH*);						/* DH_free */
	OPENSSL_11_DH* (*read_bio_dhparams_11)(OPENSSL_BIO*, OPENSSL_11_DH**, pem_password_cb*, void*); /* PEM_read_bio_DHparams */
	OPENSSL_11_DH* (*dh_new_11)();							/* DH_new */
	int (*dh_set0_pqg_11)(OPENSSL_11_DH *dh, OPENSSL_BIGNUM *p, OPENSSL_BIGNUM *q, OPENSSL_BIGNUM *g); /* DH_set0_pqg for 1.1.X */
	void (*dh_free_11)(OPENSSL_11_DH*);						/* DH_free */
	OPENSSL_BIGNUM* (*bin2bn)(const unsigned char*, int, OPENSSL_BIGNUM*);	/* BN_bin2bn */
	void (*bn_free)(OPENSSL_BIGNUM*);			/* BN_free */
	void (*rand_seed)(const void*, int);		/* RAND_seed */
	void (*err_free_strings)(void);				/* ERR_free_strings */
	void (*err_remove_state)(int);				/* ERR_remove_state */
	void (*engine_cleanup)(void);				/* ENGINE_cleanup */
	void (*crypto_cleanup_all_ex_data)(void);  /* CRYPTO_cleanup_all_ex_data */
	void (*evp_cleanup)(void);					/* EVP_cleanup */
	void* (*X509_get_ext_d2i)(const OPENSSL_X509*, int, int*, int*); /* X509_get_ext_d2i */
	int (*X509_NAME_get_index_by_NID)(OPENSSL_X509_NAME*, int, int); /* X509_NAME_get_index_by_NID*/
	OPENSSL_X509_NAME_ENTRY* (*X509_NAME_get_entry)(const OPENSSL_X509_NAME*, int); /* X509_NAME_get_entry*/
	OPENSSL_ASN1_STRING* (*X509_NAME_ENTRY_get_data)(OPENSSL_X509_NAME_ENTRY*); /* X509_NAME_ENTRY_get_data */
	int(*x509_verify_param_set1_ip_asc)(OPENSSL_X509_VERIFY_PARAM*, const char*);		/* 1.1 X509_verify_Param */
	unsigned char* (*ASN1_STRING_data)(OPENSSL_ASN1_STRING*); /* ASN1_STRING_data for 1.0.X.  This is only called in the 1.0.X certificate validation */
	int (*ASN1_STRING_length)(OPENSSL_ASN1_STRING*); /* ASN1_STRING_length */
	int (*sk_num)(const OPENSSL_STACK*);		/* sk_num */
	void*(*sk_value)(const OPENSSL_STACK*, int); /* sk_value */
	void*(*sk_free)(OPENSSL_STACK*);			/* sk_free for 1.0.X */
	OPENSSL_EC_KEY* (*EC_KEY_new_by_curve_name)(int); /* EC_KEY_new_by_curve_name */
}ripcCryptoApiFuncs;


OPENSSL_10_DH *ripcSSL10DHGetParam(ripcSSLApiFuncs* sslFuncs, ripcCryptoApiFuncs* cryptoFuncs);

OPENSSL_11_DH *ripcSSL11DHGetParam(ripcSSLApiFuncs* sslFuncs, ripcCryptoApiFuncs* cryptoFuncs);

#endif

