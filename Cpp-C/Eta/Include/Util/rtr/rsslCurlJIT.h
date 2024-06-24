/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef __rsslcurljit_h
#define __rsslcurljit_h

#include "rtr/os.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslTypes.h"
#include "curl/curl.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

typedef struct
{
	CURL* (*curl_easy_init)();
	void(*curl_easy_cleanup)(CURL*);
	CURLcode (*curl_easy_setopt)(CURL*, CURLoption, ...);
	CURLcode (*curl_easy_perform)(CURL*);
	CURLcode (*curl_global_init)(long);
	void (*curl_global_cleanup)(void);
	CURLcode (*curl_easy_getinfo)(CURL*, CURLINFO, ...);
	const char* (*curl_easy_strerror)(CURLcode);
	struct curl_slist* (*curl_slist_append)(struct curl_slist *, const char *);
	void (*curl_slist_free_all)(struct curl_slist *);
	CURLM* (*curl_multi_init)(void);
	CURLMcode (*curl_multi_add_handle)(CURLM *multi_handle, CURL *curl_handle);
	CURLMcode (*curl_multi_remove_handle)(CURLM *multi_handle, CURL *curl_handle);
	CURLMcode (*curl_multi_perform)(CURLM *multi_handle, int *running_handles);
	CURLMcode (*curl_multi_poll)(CURLM *multi_handle, struct curl_waitfd extra_fds[], unsigned int extra_nfds, int timeout_ms, int *numFds);
	CURLMsg* (*curl_multi_info_read)(CURLM *multi_handle, int *msgs_in_queue);
	CURLMcode (*curl_multi_cleanup)(CURLM *multi_handle);
	const char* (*curl_multi_strerror)(CURLMcode);
    CURLSH* (*curl_share_init)();
    CURLSHcode (*curl_share_setopt)(CURLSH*, CURLSHoption, ...);
    CURLSHcode (*curl_share_cleanup)(CURLSH*);

    CURLSH* curlShare;

} RsslCurlJITFuncs;

#define INIT_RSSL_CURL_API_FUNCS {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

RSSL_API RsslCurlJITFuncs* rsslInitCurlApi(char* curlLibName, RsslError *error);
RSSL_API RsslRet rsslUninitCurlApi();

RSSL_API RsslBool rsslCurlIsInitialized();
RSSL_API RsslCurlJITFuncs* rsslGetCurlFuncs();


#endif

