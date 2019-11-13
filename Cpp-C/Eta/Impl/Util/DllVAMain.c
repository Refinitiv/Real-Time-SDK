/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

//
// Defines the entry point for the DLL 
//
#ifdef WIN32
#ifdef RSSL_VA_EXPORTS	// only include this in shared library builds

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

 BOOL WINAPI DllMain(
	HINSTANCE	hinstDLL,		// handle to DLL module
	DWORD		reason,			// reason for calling function
	LPVOID		lpReserved)		// reserved
{
	BOOL ret = 0;
	DWORD errCode = 0;

    switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			ret = DisableThreadLibraryCalls((HINSTANCE)hinstDLL);
			if (ret == 0) errCode = GetLastError();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
		default:
			break;
    }
    return TRUE;	// the the process wont load if we return FALSE
}

#ifdef __cplusplus
}
#endif
#endif 
#endif 
