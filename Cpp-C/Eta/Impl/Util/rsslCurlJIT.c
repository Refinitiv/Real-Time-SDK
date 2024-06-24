/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2019 LSEG. All rights reserved.                 --
*|-----------------------------------------------------------------------------
*/
#include "rtr/rsslSocketTransportImpl.h"
#include "rtr/rsslCurlJIT.h"
#include "rtr/rtratomic.h"
#include "rtr/rsslLoadInitTransport.h"
#include "rtr/rsslThread.h"

#ifdef WIN32
static HMODULE curlHandle = 0;
#else
static void* curlHandle = 0;
#endif

#ifdef LINUX
static char* defaultCurlLibName = "libcurl.so";
static char* defaultCurl4LibName = "libcurl.so.4";
#else
#ifdef NDEBUG
static char* defaultCurlLibName = "libcurl.dll";
#else
static char* defaultCurlLibName = "libcurl-d.dll";
#endif
#endif

static RsslCurlJITFuncs curlJITFuncs = INIT_RSSL_CURL_API_FUNCS;
static rtr_atomic_val rsslCurlInitCount = 0;

/* Lock for internal Curl thread safety */
RsslMutex curlLock;
CURLSH* curlShare = NULL;

static RsslCurlJITFuncs* curlLoadError(RsslError* error)
{
	error->rsslErrorId = RSSL_RET_FAILURE;
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Unable to load libcurl.  Please check that the library is present on the system and that it is the correct version.\n", __FILE__, __LINE__);

	RSSL_LI_DLCLOSE(curlHandle);
	curlHandle = 0;

	return NULL;
}

void curlLockCb(CURL* handle, curl_lock_data data, curl_lock_access access, void* useptr)
{
    RSSL_MUTEX_LOCK(&curlLock);
}

void curlUnlockCb(CURL* handle, curl_lock_data data, curl_lock_access access, void* useptr)
{
    RSSL_MUTEX_UNLOCK(&curlLock);
}

#ifdef WIN32
static HMODULE openLibCurlLib(char* libCurlName, RsslError *error)
#else
static void* openLibCurlLib(char* libCurlName, RsslError *error)
#endif
{
	/* try given name first */
	if (libCurlName != NULL && (*libCurlName != '\0'))
	{
		RSSL_LI_RESET_DLERROR;
		if ((curlHandle = RSSL_LI_DLOPEN(libCurlName)) == 0)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0012 Libcurl intialization failed. Curl library: %s not found.\n", __FILE__, __LINE__, libCurlName);
		}
		return curlHandle;
	}

	/* try defaultCurlLibName name then */
	RSSL_LI_RESET_DLERROR;
	if ((curlHandle = RSSL_LI_DLOPEN(defaultCurlLibName)) != 0)
	{
		return curlHandle;
	}

#ifdef LINUX
	/* try defaultCurl4LibName name then */
	RSSL_LI_RESET_DLERROR;
	if ((curlHandle = RSSL_LI_DLOPEN(defaultCurl4LibName)) != 0)
	{
		return curlHandle;
	}

#endif

	if (curlHandle == 0)
	{
		error->rsslErrorId = RSSL_RET_FAILURE;
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0012 Libcurl intialization failed. Curl library: %s not found.\n", __FILE__, __LINE__, defaultCurlLibName);
	}

	return curlHandle;
}

RsslCurlJITFuncs* rsslInitCurlApi(char* curlLibName, RsslError *error)
{
	CURLcode ret;
	CURLSHcode shret;
	RSSL_LI_ERR_T dlErr = 0;

	if (rsslCurlInitCount == 0)
	{
		memset(&curlJITFuncs, 0, sizeof(RsslCurlJITFuncs));
		RSSL_LI_RESET_DLERROR;
		if ((curlHandle = openLibCurlLib(curlLibName, error)) == 0)
		{
			return NULL;
		}

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_init = (CURL* (*)())RSSL_LI_DLSYM(curlHandle, "curl_easy_init");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_init, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_cleanup = (void (*)(CURL*))RSSL_LI_DLSYM(curlHandle, "curl_easy_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_cleanup, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_setopt = (CURLcode (*)(CURL*, CURLoption, ...))RSSL_LI_DLSYM(curlHandle, "curl_easy_setopt");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_setopt, dlErr))
			return curlLoadError(error);
	
		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_perform = (CURLcode (*)(CURL*))RSSL_LI_DLSYM(curlHandle, "curl_easy_perform");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_perform, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_global_init = (CURLcode (*)(long))RSSL_LI_DLSYM(curlHandle, "curl_global_init");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_global_init, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_global_cleanup = (void (*)(void))RSSL_LI_DLSYM(curlHandle, "curl_global_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_global_cleanup, dlErr))
			return curlLoadError(error);
		
		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_getinfo = (CURLcode (*)(CURL*, CURLINFO, ...))RSSL_LI_DLSYM(curlHandle, "curl_easy_getinfo");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_getinfo, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_easy_strerror = (const char* (*)(CURLcode))RSSL_LI_DLSYM(curlHandle, "curl_easy_strerror");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_easy_strerror, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_slist_append = (struct curl_slist* (*)(struct curl_slist *, const char *))RSSL_LI_DLSYM(curlHandle, "curl_slist_append");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_slist_append, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_slist_free_all = (void (*)(struct curl_slist *))RSSL_LI_DLSYM(curlHandle, "curl_slist_free_all");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_slist_free_all, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_init = (CURLM* (*)(void))RSSL_LI_DLSYM(curlHandle, "curl_multi_init");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_init, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_add_handle = (CURLMcode (*)(CURLM *multi_handle, CURL *curl_handle))RSSL_LI_DLSYM(curlHandle, "curl_multi_add_handle");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_add_handle, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_remove_handle = (CURLMcode (*)(CURLM *multi_handle, CURL *curl_handle))RSSL_LI_DLSYM(curlHandle, "curl_multi_remove_handle");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_remove_handle, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_perform = (CURLMcode (*)(CURLM *multi_handle, int *running_handles))RSSL_LI_DLSYM(curlHandle, "curl_multi_perform");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_perform, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_poll = (CURLMcode (*)(CURLM *multi_handle, struct curl_waitfd extra_fds[], unsigned int extra_nfds, int timeout_ms, int *numFds))RSSL_LI_DLSYM(curlHandle, "curl_multi_poll");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_poll, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_info_read = (CURLMsg* (*)(CURLM *multi_handle, int *msgs_in_queue))RSSL_LI_DLSYM(curlHandle, "curl_multi_info_read");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_info_read, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_cleanup = (CURLMcode (*)(CURLM *multi_handle))RSSL_LI_DLSYM(curlHandle, "curl_multi_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_cleanup, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_multi_strerror = (const char* (*)(CURLMcode))RSSL_LI_DLSYM(curlHandle, "curl_multi_strerror");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_multi_strerror, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_share_init = (CURLSH* (*)())RSSL_LI_DLSYM(curlHandle, "curl_share_init");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_share_init, dlErr))
			return curlLoadError(error);

		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_share_setopt = (CURLSHcode (*)(CURLSH*, CURLSHoption, ...))RSSL_LI_DLSYM(curlHandle, "curl_share_setopt");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_share_setopt, dlErr))
			return curlLoadError(error);
		
		RSSL_LI_RESET_DLERROR;
		curlJITFuncs.curl_share_cleanup = (CURLSHcode (*)(CURLSH*))RSSL_LI_DLSYM(curlHandle, "curl_share_cleanup");
		if (dlErr = RSSL_LI_CHK_DLERROR(curlJITFuncs.curl_share_cleanup, dlErr))
			return curlLoadError(error);

        if ((ret = (*(curlJITFuncs.curl_global_init))(CURL_GLOBAL_NOTHING)) != CURLE_OK)
		{
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Libcurl intialization failed.  CURL error number: %d\n", __FILE__, __LINE__, ret);
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;

			return NULL;
		}

        /* Init mutex, initialize share interface, and set lock callbacks. */
        RSSL_MUTEX_INIT(&curlLock);
        
        if((curlShare = (*(curlJITFuncs.curl_share_init))()) == NULL)
        {
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Libcurl intialization failed. Unable to get CURLSH object.\n", __FILE__, __LINE__);
			(*(curlJITFuncs.curl_global_cleanup))();
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;
            return NULL;
        }

        if((shret = (*(curlJITFuncs.curl_share_setopt))(curlShare, CURLSHOPT_LOCKFUNC, curlLockCb)) != CURLSHE_OK)
        {
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Libcurl intialization failed. Unable to set lock callback. CurlSH error number: %i\n", __FILE__, __LINE__, shret);
			(*(curlJITFuncs.curl_share_cleanup))(curlShare);
            curlShare = NULL;
            (*(curlJITFuncs.curl_global_cleanup))();
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;
            return NULL;
        }
        
        if((shret = (*(curlJITFuncs.curl_share_setopt))(curlShare, CURLSHOPT_UNLOCKFUNC, curlUnlockCb)) != CURLSHE_OK)
        {
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Libcurl intialization failed. Unable to set unlock callback. CurlSH error number: %i\n", __FILE__, __LINE__, shret);
			(*(curlJITFuncs.curl_share_cleanup))(curlShare);
            curlShare = NULL;
			(*(curlJITFuncs.curl_global_cleanup))();
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;
            return NULL;
        }

        if((shret = (*(curlJITFuncs.curl_share_setopt))(curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS)) != CURLSHE_OK)
        {
			error->rsslErrorId = RSSL_RET_FAILURE;
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 0002 Libcurl intialization failed. Unable to set DNS share. CurlSH error number: %i\n", __FILE__, __LINE__, shret);
			(*(curlJITFuncs.curl_share_cleanup))(curlShare);
            curlShare = NULL;
			(*(curlJITFuncs.curl_global_cleanup))();
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;
            return NULL;
        }

		/* Increment our refcounted value, we've successfully loaded and initialized libcurl */
		
	}
    curlJITFuncs.curlShare = curlShare;	
	RTR_ATOMIC_INCREMENT(rsslCurlInitCount);

	return &curlJITFuncs;
}

RsslRet rsslUninitCurlApi()
{
	RTR_ATOMIC_DECREMENT(rsslCurlInitCount);
	if (rsslCurlInitCount < 0)
	{
		RTR_ATOMIC_SET(rsslCurlInitCount, 0);
		return RSSL_RET_SUCCESS;
	}

	if (rsslCurlInitCount == 0)
	{		
		/* If the handle's present, initialization has succeeded, and we call curl's global cleanup function and close out the loaded shared lib. */
		if (curlHandle != 0)
		{
			(*(curlJITFuncs.curl_share_cleanup))(curlShare);
            curlShare = NULL;
			(*(curlJITFuncs.curl_global_cleanup))();
			RSSL_LI_DLCLOSE(curlHandle);
			curlHandle = 0;
		}
	}

    memset(&curlJITFuncs, 0, sizeof(RsslCurlJITFuncs));

	return RSSL_RET_SUCCESS;
}

RsslBool rsslCurlIsInitialized()
{
	if (rsslCurlInitCount == 0)
		return RSSL_FALSE;
	else
		return RSSL_TRUE;
}
RsslCurlJITFuncs* rsslGetCurlFuncs()
{
	if (rsslCurlInitCount == 0)
		return NULL;
	else
		return &curlJITFuncs;
}
