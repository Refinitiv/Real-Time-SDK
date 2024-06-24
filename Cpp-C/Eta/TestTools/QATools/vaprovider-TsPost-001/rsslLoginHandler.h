/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_LOGIN_HANDLER_H
#define _RTR_RSSL_LOGIN_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rsslProvider.h"
#include "rtr/rsslReactor.h"

#define MAX_LOGIN_INFO_STRLEN 128
#define LOGIN_REQ_MEM_BUF_SIZE 256

/* reasons a login request is rejected */
typedef enum {
	MAX_LOGIN_REQUESTS_REACHED	= 0,
	LOGIN_RDM_DECODER_FAILED	= 1
} RsslLoginRejectReason;

/*
 * Returns a string for the reject reason code.
 * rejectReason - The RsslLoginRejectReason enum code
 */
RTR_C_INLINE const char* rejectReasonToString(RsslLoginRejectReason rejectReason)
{
	switch (rejectReason)
	{
	case MAX_LOGIN_REQUESTS_REACHED:
		return "ITEM_COUNT_REACHED";
		break;
	case LOGIN_RDM_DECODER_FAILED:
		return "DECODING_FAILED";
		break;
	default:
		return "Unknown reason";
	}
}

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
} LoginRequestInfo;
void initLoginHandler();
static RsslRet sendLoginRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginRequest* pLoginRequest);
static RsslRet sendLoginRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslLoginRejectReason reason, RsslErrorInfo *pError);
void closeLoginStreamForChannel(RsslReactorChannel* pReactorChannel);
static void closeLoginStream(RsslInt32 streamId);
LoginRequestInfo* findLoginRequestInfo(RsslReactorChannel* pReactorChannel);
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent);
void setRejectLogin();

#ifdef __cplusplus
};
#endif

#endif
