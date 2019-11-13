/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_SOCKET_TRANSPORT_H
#define __RTR_RSSL_SOCKET_TRANSPORT_H

/* Contains function declarations necessary to hook in the
 * bi-directional socket (and HTTP/HTTPS) connection type 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "rtr/rsslTypes.h"
#include "rtr/rsslChanManagement.h"
#include "rtr/rsslThread.h"

#if defined(RSSL_RSSL_SOCKET_EXPORTS)
	#define 	RSSL_RSSL_SOCKET_API			RTR_API_EXPORT
	#define 	RSSL_RSSL_SOCKET_FAST(ret)		RTR_API_EXPORT ret RTR_FASTCALL
#elif defined(RSSL_RSSL_SOCKET_DLL)
	#define 	RSSL_RSSL_SOCKET_API			RTR_API_IMPORT
	#define 	RSSL_RSSL_SOCKET_FAST(ret)		RTR_API_IMPORT ret RTR_FASTCALL
#else
	#define 	RSSL_RSSL_SOCKET_API
	#define 	RSSL_RSSL_SOCKET_FAST(ret)		ret RTR_FASTCALL
#endif

/* Initializes socket transport and sets up function pointers */
RsslRet rsslSocketInitialize(RsslInitializeExOpts *rsslInitOpts, RsslError *error);

/* Uninitializes socket transport */
RsslRet rsslSocketUninitialize();

#ifdef __cplusplus
};
#endif


#endif
