/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
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
    CURLSH* (*curl_share_init)();
    CURLSHcode (*curl_share_setopt)(CURLSH*, CURLSHoption, ...);
    CURLSHcode (*curl_share_cleanup)(CURLSH*);

    CURLSH* curlShare;

} RsslCurlJITFuncs;

#define INIT_RSSL_CURL_API_FUNCS {0,0,0,0,0,0,0}

RsslCurlJITFuncs* rsslInitCurlApi(char* curlLibName, RsslError *error);
RsslRet rsslUninitCurlApi();

RsslBool rsslCurlIsInitialized();
RsslCurlJITFuncs* rsslGetCurlFuncs();


#endif

