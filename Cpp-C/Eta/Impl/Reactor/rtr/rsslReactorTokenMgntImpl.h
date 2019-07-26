/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2019. All rights reserved.
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
	RsslUInt32						httpStausCode; /* the latest HTTP status code */
	RsslReactorTokenSessionState	tokenSessionState;
	RsslInt32						tokenMgntEventType; /* The value defined in RsslReactorTokenMgntEventType */
	RsslReactorOAuthCredential		*pOAuthCredential; /* OAuth credential for this session */
	RsslRestHandle					*pRestHandle;
	RsslInt							lastTokenUpdatedTime; /* Keep track the latest updated time in ms for the token information */

	/* Handling token refresh */
	RsslInt							nextExpiresTime; /* the next expires time in millisecond */
	RsslBool						resendFromFailure; /* Indicates to resend the request after a response failure */
	rtr_atomic_val					sendTokenRequest;
	RsslErrorInfo					tokenSessionWorkerCerr;

	RsslQueue						reactorChannelList; /* Keeps a list of RsslReactorChannelImpl for notifying the access token */
	RsslHashLink					hashLinkNameAndClientId; /* This is used for the sessionByNameAndClientId hash table */
	RsslBuffer						userNameAndClientId; /* This is the key for the hash table. The token session owns the memory buffer */
	RsslQueueLink					sessionLink; /* Keeps in the RsslReactorTokenManagementImpl's list */
	RsslReactor						*pReactor; /* The reactor for the token management */
	RsslMutex						accessTokenMutex; /* This is used to synchronized for requesting the token information from application's thread */
	rtr_atomic_val					requestingAccessToken;

	/* Use the proxy information from RsslReactorChannel if any */
	RsslConnectOptions				proxyConnectOpts; /* The proxy options */
	RsslBool						copiedProxyOpts;

	/* This is used for OAuth credential renewal */
	RsslReactorOAuthCredentialRenewalImpl *pOAuthCredentialRenewalImpl;

	RsslInt32					reissueTokenAttemptLimit; /* Keeping track of token renewal attempt */

	rtr_atomic_val				numberOfWaitingChannels; /* Keeps the number of RsslReactorChannelImpl waiting to register to the RsslReactorTokenSessionImpl*/

	RsslBuffer					temporaryURL; /* Used the memory location from the temporaryURLBuffer */
	RsslBuffer					temporaryURLBuffer;

} RsslReactorTokenSessionImpl;

RTR_C_INLINE void rsslClearReactorTokenSessionImpl(RsslReactorTokenSessionImpl *pTokenSessionImpl)
{
	memset(pTokenSessionImpl, 0, sizeof(RsslReactorTokenSessionImpl));
	rsslInitQueue(&pTokenSessionImpl->reactorChannelList);
	RSSL_MUTEX_INIT(&pTokenSessionImpl->accessTokenMutex);
}

RTR_C_INLINE void rsslFreeReactorTokenSessionImpl(RsslReactorTokenSessionImpl *pTokenSessionImpl)
{
	free(pTokenSessionImpl->tokenInformationBuffer.data);
	free(pTokenSessionImpl->rsslAccessTokenRespBuffer.data);
	free(pTokenSessionImpl->rsslServiceDiscoveryRespBuffer.data);
	free(pTokenSessionImpl->rsslPostDataBodyBuf.data);
	free(pTokenSessionImpl->userNameAndClientId.data);
	free(pTokenSessionImpl->pOAuthCredential);
	RSSL_MUTEX_DESTROY(&pTokenSessionImpl->accessTokenMutex);

	rsslFreeConnectOpts(&pTokenSessionImpl->proxyConnectOpts);
	free(pTokenSessionImpl->temporaryURLBuffer.data);

	memset(pTokenSessionImpl, 0, sizeof(RsslReactorTokenSessionImpl));

	free(pTokenSessionImpl);
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


#ifdef __cplusplus
};
#endif

#endif // _RTR_RSSL_REACTOR_TOKEN_MGNT_IMPL_H
