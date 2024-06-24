/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2022 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_REACTOR_TOKEN_MGNT_IMPL_H
#define _RTR_RSSL_REACTOR_TOKEN_MGNT_IMPL_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslVAUtils.h"
#include "rtr/rsslRestClientImpl.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslThread.h"
#include "rtr/rtratomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RsslReactorOAuthCredentialRenewalImpl RsslReactorOAuthCredentialRenewalImpl;

typedef struct _RsslReactorExplicitServiceDiscoveryInfo RsslReactorExplicitServiceDiscoveryInfo;

/* RsslReactorChannelInfoImplState
* - Represents states for token management and requesting service discovery */
typedef enum
{
	RSSL_RC_TOKEN_SESSION_IMPL_STOP_REQUESTING = -5,
	RSSL_RC_TOKEN_SESSION_IMPL_MEM_ALLOCATION_FAILURE = -4,
	RSSL_RC_TOKEN_SESSION_IMPL_PARSE_RESP_FAILURE = -3,
	RSSL_RC_TOKEN_SESSION_IMPL_REQUEST_FAILURE = -2,
	RSSL_RC_TOKEN_SESSION_IMPL_BUFFER_TOO_SMALL = -1,
	RSSL_RC_TOKEN_SESSION_IMPL_INIT = 0,
	RSSL_RC_TOKEN_SESSION_IMPL_REQ_INIT_AUTH_TOKEN = 1,
	RSSL_RC_TOKEN_SESSION_IMPL_REQ_AUTH_TOKEN_REISSUE = 2,
	RSSL_RC_TOKEN_SESSION_IMPL_RECEIVED_AUTH_TOKEN = 3,
	RSSL_RC_TOKEN_SESSION_IMPL_REQ_SENSITIVE_INFO = 4,
	RSSL_RC_TOKEN_SESSION_IMPL_WAITING_TO_REQ_AUTH_TOKEN = 5,
} RsslReactorTokenSessionState;

typedef struct
{
	/* For storing and handling token information and service discovery */
	RsslReactorAuthTokenInfo		tokenInformation;
	RsslBuffer						tokenInformationBuffer;
	RsslBuffer						rsslAccessTokenRespBuffer;
	RsslBuffer						rsslServiceDiscoveryRespBuffer;
	RsslBuffer						rsslPostDataBodyBuf;
	RsslUInt32						httpStatusCode; /* the latest HTTP status code */
	RsslReactorTokenSessionState	tokenSessionState;
	RsslInt32						tokenMgntEventType; /* The value defined in RsslReactorTokenMgntEventType */
	RsslReactorOAuthCredential		*pOAuthCredential; /* OAuth credential for this session */
	RsslRestHandle					*pRestHandle;
	RsslInt							lastTokenUpdatedTime; /* Keep track the latest updated time in ms for the token information */

	/* Handling token refresh */
	RsslInt							nextExpiresTime; /* the next expires time in millisecond */
	rtr_atomic_val64				tokenExpiresTime; /* the next expires time in millisecond */
	RsslBool						resendFromFailure; /* Indicates to resend the request after a response failure */
	rtr_atomic_val					sendTokenRequest;
	rtr_atomic_val					stopTokenRequest; /* Indicates that the worker thread is no longer sending token reissue. */
	RsslErrorInfo					tokenSessionWorkerCerr;
	RsslInt							originalExpiresIn; /* The original expires in seconds from sending token request using the password grant type.*/

	RsslQueue						reactorChannelList; /* Keeps a list of RsslReactorChannelImpl for notifying the access token */
	RsslHashLink					hashLinkNameAndClientId; /* This is used for the sessionByNameAndClientId hash table */
	RsslBuffer						userNameAndClientId; /* This is the key for the hash table. The token session owns the memory buffer */
	RsslQueueLink					sessionLink; /* Keeps in the RsslReactorTokenManagementImpl's list */
	RsslReactor						*pReactor; /* The reactor for the token management */
	RsslMutex						accessTokenMutex; /* This is used to synchronized for requesting the token information from application's thread */
	rtr_atomic_val					requestingAccessToken;
	RsslBool						initialized; /* This is used to indicate that the token session has success fully initialized in  rsslReactorConnect() */

	/* Use the proxy information from RsslReactorChannel if any */
	RsslConnectOptions				proxyConnectOpts; /* The proxy options */
	RsslBool						copiedProxyOpts;

	/* This is used for OAuth credential renewal */
	RsslReactorOAuthCredentialRenewalImpl *pOAuthCredentialRenewalImpl;

	RsslInt32					reissueTokenAttemptLimit; /* Keeping track of token renewal attempt */

	rtr_atomic_val				numberOfWaitingChannels; /* Keeps the number of RsslReactorChannelImpl waiting to register to the RsslReactorTokenSessionImpl*/

	RsslBuffer					sessionAuthUrl;
	RsslBuffer					temporaryURL; /* Used the memory location from the temporaryURLBuffer */
	RsslBuffer					temporaryURLBuffer;

	RsslReactorSessionMgmtVersion sessionVersion;
	void*						userSpecPtr;

	RsslBool					initialTokenRetrival;  /* Flag used to determine if it's the initial connection to make sure we don't call the user back unnecessarily and properly clean up any sensitive data once we have a token */

	RsslReactorExplicitServiceDiscoveryInfo* pExplicitServiceDiscoveryInfo; /* This is set when performing non-blocking explicit service discovery */
	rtr_atomic_val				waitingForResponseOfExplicitSD; /* Keeps number of explicit service discovery requests which still refers to this RsslReactorTokenSessionImpl */
	RsslQueueLink				invalidSessionLink; /* Keeps in the RsslReactorWorker.freeInvalidTokenSessions for freeing it later when the token session is no longer needed. */

} RsslReactorTokenSessionImpl;

RTR_C_INLINE void rsslClearReactorTokenSessionImpl(RsslReactorTokenSessionImpl *pTokenSessionImpl)
{
	memset(pTokenSessionImpl, 0, sizeof(RsslReactorTokenSessionImpl));
	rsslInitQueue(&pTokenSessionImpl->reactorChannelList);
	RSSL_MUTEX_INIT(&pTokenSessionImpl->accessTokenMutex);
}

RTR_C_INLINE void rsslFreeServiceDiscoveryOptions(RsslReactorServiceDiscoveryOptions* pOpts)
{
	if (pOpts->userName.data != NULL)
		free(pOpts->userName.data);

	if (pOpts->password.data != NULL)
		free(pOpts->password.data);

	if (pOpts->clientId.data != NULL)
		free(pOpts->clientId.data);

	if (pOpts->clientSecret.data != NULL)
		free(pOpts->clientSecret.data);

	if (pOpts->tokenScope.data != NULL)
		free(pOpts->tokenScope.data);

	if (pOpts->proxyHostName.data != NULL)
		free(pOpts->proxyHostName.data);

	if (pOpts->proxyPort.data != NULL)
		free(pOpts->proxyPort.data);

	if (pOpts->proxyUserName.data != NULL)
		free(pOpts->proxyUserName.data);

	if (pOpts->proxyPasswd.data != NULL)
		free(pOpts->proxyPasswd.data);

	if (pOpts->proxyDomain.data != NULL)
		free(pOpts->proxyDomain.data);

	if (pOpts->clientJWK.data != NULL)
		free(pOpts->clientJWK.data);

	if (pOpts->audience.data != NULL)
		free(pOpts->audience.data);
}

/* RsslReactorOAuthCredentialRenewalImpl
 * - Handles allocated memory length and the association with RsslReactorChannelImpl for RsslReactorOAuthCredentialRenewal */
struct _RsslReactorOAuthCredentialRenewalImpl
{
	RsslReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal;
	RsslReactorTokenSessionImpl			*pParentTokenSession; /* Specify the owner of this type if any */

	/* Keeps track of memory allocation length */
	size_t		memoryLength;

	/* The following member variables is used only when submitting without a channel */
	RsslBuffer					rsslAccessTokenRespBuffer;
	RsslReactor					*pRsslReactor;
	RsslReactorAuthTokenInfo	tokenInformation;
	RsslBuffer					tokenInformationBuffer;
	RsslBuffer					rsslPostDataBodyBuf;
	RsslReactorAuthTokenEventCallback	*pAuthTokenEventCallback;
	RsslUInt32					httpStatusCode;
	RsslBuffer					proxyHostName;
	RsslBuffer					proxyPort;
	RsslBuffer					proxyUserName;
	RsslBuffer					proxyPasswd;
	RsslBuffer					proxyDomain;
	RsslReactorSessionMgmtVersion  sessionVersion;
};

RTR_C_INLINE void rsslClearReactorOAuthCredentialRenewalImpl(RsslReactorOAuthCredentialRenewalImpl *pInfo)
{
	memset(pInfo, 0, sizeof(RsslReactorOAuthCredentialRenewalImpl));
}

typedef struct
{
	RsslQueue					tokenSessionList; /* Keeps a list of RsslReactorTokenSessionImpl for token renewal */
	RsslMutex					tokenSessionMutex; /* Ensures function calls to access the token sessions is thread-safe */
	RsslHashTable				sessionByNameAndClientIdHt;

} RsslReactorTokenManagementImpl;

RTR_C_INLINE void rsslClearReactorTokenManagementImpl(RsslReactorTokenManagementImpl *pTokenManagementImpl)
{
	memset(pTokenManagementImpl, 0, sizeof(RsslReactorTokenManagementImpl));
	RSSL_MUTEX_INIT(&pTokenManagementImpl->tokenSessionMutex);
	rsslInitQueue(&pTokenManagementImpl->tokenSessionList);
}

/* RsslReactorErrorInfoImpl is used to capture error information for a token session for one or multiple channels and it is 
 * able to reuse it from the pool. */
typedef struct
{
	RsslQueueLink				poolLink;		/* For keeping in the RsslReactorWorker.errorInfoPool or RsslReactorWorker.errorInfoInUsedPool member */
	RsslErrorInfo				rsslErrorInfo;	/* Keeps the error message and its location */
	RsslUInt32					referenceCount; /* Number of events that refers to this structure */
} RsslReactorErrorInfoImpl;

RTR_C_INLINE void rsslClearReactorErrorInfoImpl(RsslReactorErrorInfoImpl *pErrorInfoImpl)
{
	memset(&pErrorInfoImpl->rsslErrorInfo, 0, sizeof(RsslErrorInfo));
	pErrorInfoImpl->referenceCount = 0;
}

typedef enum
{
	RSSL_RCIMPL_ET_REST_REQ_UNKNOWN = 0x00,
	RSSL_RCIMPL_ET_REST_REQ_AUTH_SERVICE = 0x01,
	RSSL_RCIMPL_ET_REST_REQ_SERVICE_DISCOVERY = 0x02
} RsslReactorRestRequestType;

/* This is used to keep the query service discovery options. */
struct _RsslReactorExplicitServiceDiscoveryInfo
{
	RsslReactorRestRequestType restRequestType;
	RsslRestRequestArgs* pRestRequestArgs;
	RsslBuffer restRequestBuf;
	RsslBuffer restResponseBuf;
	RsslReactorServiceDiscoveryOptions serviceDiscoveryOptions;
	int sessionMgntVersion; /* either RSSL_RC_SESSMGMT_V1 or RSSL_RC_SESSMGMT_V2 */
	RsslReactor* pReactor;
	RsslBuffer parseRespBuffer;
	RsslUInt32 httpStatusCode;
	RsslReactorTokenSessionImpl* pTokenSessionImpl;
	void* pUserSpec;
	rtr_atomic_val assignedToChannel; /* This is used to indicate whether the token session is assigned to ReactorChannel. */
};

RTR_C_INLINE void rsslClearReactorExplicitServiceDiscoveryInfo(RsslReactorExplicitServiceDiscoveryInfo* pServiceDiscoveryInfo)
{
	memset(pServiceDiscoveryInfo, 0, sizeof(RsslReactorExplicitServiceDiscoveryInfo));
}

RTR_C_INLINE void rsslFreeReactorExplicitServiceDiscoveryInfo(RsslReactorExplicitServiceDiscoveryInfo* pExplicitServiceDiscoveryInfo)
{
	free(pExplicitServiceDiscoveryInfo->restRequestBuf.data);
	free(pExplicitServiceDiscoveryInfo->restResponseBuf.data);
	free(pExplicitServiceDiscoveryInfo->parseRespBuffer.data);

	rsslFreeServiceDiscoveryOptions(&pExplicitServiceDiscoveryInfo->serviceDiscoveryOptions);

	memset(pExplicitServiceDiscoveryInfo, 0, sizeof(pExplicitServiceDiscoveryInfo));

	free(pExplicitServiceDiscoveryInfo);
}

RTR_C_INLINE void rsslFreeReactorTokenSessionImpl(RsslReactorTokenSessionImpl* pTokenSessionImpl)
{
	free(pTokenSessionImpl->tokenInformationBuffer.data);
	free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
	free(pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data);
	free(pTokenSessionImpl->rsslPostDataBodyBuf.data);
	free(pTokenSessionImpl->userNameAndClientId.data);
	free(pTokenSessionImpl->pOAuthCredential);
	free(pTokenSessionImpl->sessionAuthUrl.data);
	free(pTokenSessionImpl->pOAuthCredentialRenewalImpl);
	RSSL_MUTEX_DESTROY(&pTokenSessionImpl->accessTokenMutex);

	rsslFreeConnectOpts(&pTokenSessionImpl->proxyConnectOpts);
	free(pTokenSessionImpl->temporaryURLBuffer.data);

	if (pTokenSessionImpl->pExplicitServiceDiscoveryInfo)
	{
		rsslFreeReactorExplicitServiceDiscoveryInfo(pTokenSessionImpl->pExplicitServiceDiscoveryInfo);
	}

	memset(pTokenSessionImpl, 0, sizeof(RsslReactorTokenSessionImpl));

	free(pTokenSessionImpl);
}

#ifdef __cplusplus
};
#endif

#endif // _RTR_RSSL_REACTOR_TOKEN_MGNT_IMPL_H
