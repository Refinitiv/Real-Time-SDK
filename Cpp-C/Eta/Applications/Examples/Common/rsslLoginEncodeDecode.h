/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



#ifndef _RTR_RSSL_LOGIN_ENCDEC_H
#define _RTR_RSSL_LOGIN_ENCDEC_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LOGIN_INFO_STRLEN 128
#define AUTH_TOKEN_LENGTH 1024

/* login request information */
typedef struct {
	RsslInt32	StreamId;
	RsslUInt8	NameType;
	char		Username[MAX_LOGIN_INFO_STRLEN];
	char		ApplicationId[MAX_LOGIN_INFO_STRLEN];
	char		ApplicationName[MAX_LOGIN_INFO_STRLEN];
	char		Position[MAX_LOGIN_INFO_STRLEN];
	char		Password[MAX_LOGIN_INFO_STRLEN];
	char		AuthenticationToken[AUTH_TOKEN_LENGTH];
	char		AuthenticationExtended[AUTH_TOKEN_LENGTH];
	RsslUInt64	ProvidePermissionProfile;
	RsslUInt64	ProvidePermissionExpressions;
	RsslUInt64	SingleOpen;
	RsslUInt64	SupportProviderDictionaryDownload;
	RsslUInt64	AllowSuspectData;
	char		InstanceId[MAX_LOGIN_INFO_STRLEN];
	RsslUInt64	Role;
	RsslUInt64	DownloadConnectionConfig;
	RsslChannel*	Chnl;
	RsslBool		IsInUse;
	RsslBool		RTT;
	RsslUInt		RTTLatency;
} RsslLoginRequestInfo;

/* reasons a login request is rejected */
typedef enum {
	MAX_LOGIN_REQUESTS_REACHED	= 0,
	NO_USER_NAME_IN_REQUEST		= 1,
	ALREADY_LOGGED_WITH_COOKIE	= 2
} RsslLoginRejectReason;

/* login response information */
typedef struct {
	RsslInt32	StreamId;
	char		Username[MAX_LOGIN_INFO_STRLEN];
	char		ApplicationId[MAX_LOGIN_INFO_STRLEN];
	char		ApplicationName[MAX_LOGIN_INFO_STRLEN];
	char		Position[MAX_LOGIN_INFO_STRLEN];
	char		AuthenticationExtendedResponse[MAX_LOGIN_INFO_STRLEN];
	char		AuthenticationStatusErrorText[MAX_LOGIN_INFO_STRLEN];
	RsslUInt64  AuthenticationTTReissue;
	RsslBool	HasAuthenticationStatusErrorCode;
	RsslUInt64   AuthenticationStatusErrorCode;
	RsslUInt64	ProvidePermissionProfile;
	RsslUInt64	ProvidePermissionExpressions;
	RsslUInt64	SingleOpen;
	RsslUInt64	AllowSuspectData;
	RsslUInt64	SupportPauseResume;
	RsslUInt64	SupportOptimizedPauseResume;
	RsslUInt64	SupportOMMPost;
	RsslUInt64	SupportViewRequests;
	RsslUInt64	SupportBatchRequests;
	RsslUInt64  SupportProviderDictionaryDownload;
	RsslUInt64	SupportStandby;
	RsslBool	isSolicited;
	RsslBool	RTT;
} RsslLoginResponseInfo;

RsslRet encodeLoginRequest(RsslChannel* chnl, RsslLoginRequestInfo* loginReqInfo, RsslBuffer* msgBuf);
RsslRet decodeLoginRequest(RsslLoginRequestInfo* loginReqInfo, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* requestKey);
RsslRet encodeLoginResponse(RsslChannel* chnl, RsslLoginResponseInfo* loginRespInfo, RsslBuffer* msgBuf);
RsslRet decodeLoginResponse(RsslLoginResponseInfo* loginRespInfo, RsslMsg* msg, RsslDecodeIterator* dIter);
RsslRet encodeLoginClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId);
RsslRet encodeLoginCloseStatus(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId);
RsslRet encodeLoginRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslLoginRejectReason reason, RsslBuffer* msgBuf);
RsslRet encodeLoginRTTServer(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt LastLatency);
RsslRet encodeAndSendLoginRTTClient(RsslChannel* chnl, RsslInt32 streamId, RsslUInt ticks);
RsslRet decodeLoginRTTForServer(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter, RsslUInt *latency);
RsslRet decodeLoginRTTForClient(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);


/*
 * Clears the login request information.
 * loginReqInfo - The login request information to be cleared
 */
RTR_C_INLINE void clearLoginReqInfo(RsslLoginRequestInfo* loginReqInfo)
{
	loginReqInfo->StreamId = 0;
	loginReqInfo->NameType = RDM_LOGIN_USER_NAME;
	loginReqInfo->Username[0] = '\0';
	loginReqInfo->ApplicationId[0] = '\0';
	loginReqInfo->ApplicationName[0] = '\0';
	loginReqInfo->AuthenticationToken[0] = '\0';
	loginReqInfo->AuthenticationExtended[0] = '\0';
	loginReqInfo->Position[0] = '\0';
	loginReqInfo->Password[0] = '\0';
	loginReqInfo->ProvidePermissionProfile = 0;
	loginReqInfo->ProvidePermissionExpressions = 0;
	loginReqInfo->SingleOpen = 0;
	loginReqInfo->SupportProviderDictionaryDownload = 0;
	loginReqInfo->AllowSuspectData = 0;
	loginReqInfo->InstanceId[0] = '\0';
	loginReqInfo->Role = 0;
	loginReqInfo->DownloadConnectionConfig = 0;
	loginReqInfo->Chnl = 0;
	loginReqInfo->IsInUse = RSSL_FALSE;
	loginReqInfo->RTT = RSSL_FALSE;
	loginReqInfo->RTTLatency = 0;
}

/*
 * Initializes the login request information.
 * loginReqInfo - The login request information to be initialized
 */
RTR_C_INLINE void initLoginReqInfo(RsslLoginRequestInfo* loginReqInfo)
{
	clearLoginReqInfo(loginReqInfo);
	loginReqInfo->StreamId = 0;
	loginReqInfo->Username[0] = '\0';
	loginReqInfo->ApplicationId[0] = '\0';
	loginReqInfo->ApplicationName[0] = '\0';
	loginReqInfo->AuthenticationToken[0] = '\0';
	loginReqInfo->AuthenticationExtended[0] = '\0';
	loginReqInfo->Position[0] = '\0';
	loginReqInfo->Password[0] = '\0';
	loginReqInfo->ProvidePermissionProfile = 1;
	loginReqInfo->ProvidePermissionExpressions = 1;
	loginReqInfo->SingleOpen = 1;
	loginReqInfo->SupportProviderDictionaryDownload = 0;
	loginReqInfo->AllowSuspectData = 1;
	loginReqInfo->InstanceId[0] = '\0';
	loginReqInfo->Role = 0;
	loginReqInfo->DownloadConnectionConfig = 0;
}

/*
 * Clears the login response information.
 * loginRespInfo - The login response information to be cleared
 */
RTR_C_INLINE void clearLoginRespInfo(RsslLoginResponseInfo* loginRespInfo)
{
	loginRespInfo->StreamId = 0;
	loginRespInfo->Username[0] = '\0';
	loginRespInfo->ApplicationId[0] = '\0';
	loginRespInfo->ApplicationName[0] = '\0';
	loginRespInfo->Position[0] = '\0';
	loginRespInfo->AuthenticationExtendedResponse[0] = '\0';
	loginRespInfo->AuthenticationStatusErrorText[0] = '\0';
	loginRespInfo->AuthenticationTTReissue = 0;
	loginRespInfo->HasAuthenticationStatusErrorCode = RSSL_FALSE;
	loginRespInfo->AuthenticationStatusErrorCode = 0;
	loginRespInfo->ProvidePermissionProfile = 0;
	loginRespInfo->ProvidePermissionExpressions = 0;
	loginRespInfo->SingleOpen = 0;
	loginRespInfo->AllowSuspectData = 0;
	loginRespInfo->SupportPauseResume = 0;
	loginRespInfo->SupportOptimizedPauseResume = 0;
	loginRespInfo->SupportOMMPost = 0;
	loginRespInfo->SupportViewRequests = 0;
	loginRespInfo->SupportBatchRequests = 0;
	loginRespInfo->SupportProviderDictionaryDownload = 0;
	loginRespInfo->SupportStandby = 0;
	loginRespInfo->isSolicited = RSSL_FALSE;
}

/*
 * Initializes the login response information.
 * loginRespInfo - The login response information to be initialized
 */
RTR_C_INLINE void initLoginRespInfo(RsslLoginResponseInfo* loginRespInfo)
{
	loginRespInfo->StreamId = 0;
	loginRespInfo->Username[0] = '\0';
	loginRespInfo->ApplicationId[0] = '\0';
	loginRespInfo->ApplicationName[0] = '\0';
	loginRespInfo->Position[0] = '\0';
	loginRespInfo->AuthenticationExtendedResponse[0] = '\0';
	loginRespInfo->AuthenticationStatusErrorText[0] = '\0';
	loginRespInfo->AuthenticationTTReissue = 0;
	loginRespInfo->HasAuthenticationStatusErrorCode = RSSL_FALSE;
	loginRespInfo->AuthenticationStatusErrorCode = 0;
	loginRespInfo->ProvidePermissionProfile = 1;
	loginRespInfo->ProvidePermissionExpressions = 1;
	loginRespInfo->SingleOpen = 1;
	loginRespInfo->AllowSuspectData = 1;
	loginRespInfo->SupportPauseResume = 0;
	loginRespInfo->SupportOptimizedPauseResume = 0;
	loginRespInfo->SupportOMMPost = 0;
	loginRespInfo->SupportViewRequests = 0;
	loginRespInfo->SupportBatchRequests = 0;
	loginRespInfo->SupportProviderDictionaryDownload = 0;
	loginRespInfo->SupportStandby = 0;
	loginRespInfo->isSolicited = RSSL_FALSE;
}

#ifdef __cplusplus
};
#endif

#endif
