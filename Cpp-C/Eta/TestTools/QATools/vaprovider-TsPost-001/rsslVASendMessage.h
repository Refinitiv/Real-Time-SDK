/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_SENDMSG_H
#define _RTR_RSSL_SENDMSG_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MSG_SIZE 4096
#define NUM_CLIENT_SESSIONS 5

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

RsslRet sendMessage(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslBuffer* msgBuf);
RsslRet sendNotSupportedStatus(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslMsg* requestMsg);
RsslRet encodeNotSupportedStatus(RsslReactorChannel* chnl, RsslMsg* requestMsg, RsslBuffer* msgBuf);

#ifdef __cplusplus
};
#endif

#endif
