/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2022 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_LOGIN_HANDLER_H
#define _RTR_RSSL_LOGIN_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rsslProvider.h"
#include "rtr/rsslReactor.h"

#define MAX_LOGIN_INFO_STRLEN 128
#define LOGIN_REQ_MEM_BUF_SIZE 4096

/* reasons a login request is rejected */
typedef enum {
	MAX_LOGIN_REQUESTS_REACHED	= 0,
	LOGIN_RDM_DECODER_FAILED	= 1,
	NO_USER_NAME_IN_REQUEST		= 2
} RsslLoginRejectReason;

/*
 * Stores information about a consumer's login.
 */
typedef struct LoginRequestInfo
{
	RsslBool isInUse;
	RsslReactorChannel *chnl;
	RsslRDMLoginRequest loginRequest;
	char memory[LOGIN_REQ_MEM_BUF_SIZE];
	RsslBuffer memoryBuffer;
	RsslUInt lastLatency;
} LoginRequestInfo;
void initLoginHandler();
static RsslRet sendLoginRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginRequest* pLoginRequest);
static RsslRet sendLoginRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslLoginRejectReason reason, RsslErrorInfo *pError);
void closeLoginStreamForChannel(RsslReactorChannel* pReactorChannel);
static void closeLoginStream(RsslInt32 streamId);
LoginRequestInfo* findLoginRequestInfo(RsslReactorChannel* pReactorChannel);
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent);
RsslRet sendLoginRTT(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel);

void setRTTSupport(RsslBool rtt);

#ifdef __cplusplus
};
#endif

#endif
