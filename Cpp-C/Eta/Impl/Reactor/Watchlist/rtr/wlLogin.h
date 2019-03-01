/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2018. All rights reserved.
*/

#ifndef WL_LOGIN_H
#define WL_LOGIN_H

#include "rtr/wlBase.h"
#include "rtr/rsslHashTable.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
	
/* Represents a User Token, if the AAA API or TREP Authentication is in use. Used to update the login 
 * request as the token changes. */
typedef struct
{
	RsslBuffer		buffer;
	RsslBool		needRefresh;
	RsslUInt8		nameType;
	RsslBuffer		extBuffer;
} WlUserToken;

/* Represents the login request. */
typedef struct
{
	WlRequestBase			base;
	RsslRDMLoginRequest		*pLoginReqMsg;
	RsslBuffer				rdmMsgMemoryBuffer;
	WlUserToken				*pCurrentToken;			/* Current open request token */
	WlUserToken				*pNextToken;			/* Pending next request token */
} WlLoginRequest;

/* Creates a login request. */
WlLoginRequest *wlLoginRequestCreate(RsslRDMLoginRequest *pLoginReqMsg, void *pUserSpec,
		RsslErrorInfo *pErrorInfo);

/* Destroys a login request. */
void wlLoginRequestDestroy(WlBase *pBase, WlLoginRequest *pLoginRequest);

/* Flags associated with the login stream. */
typedef enum
{
	WL_LSF_NONE			= 0x0,	/* None. */
	WL_LSF_ESTABLISHED	= 0x1	/* Stream is established and can accept post messages. */
} WlLoginStreamFlags;

/* Represents the login stream. */
typedef struct
{
	WlStreamBase	base;
	RsslUInt8		flags;
} WlLoginStream;


/* Represents login domain handling. */
typedef struct
{
	WlLoginStream	*pStream;
	WlLoginRequest	**pRequest; /* Represent a list of WlLoginRequest */
	RsslInt32		index; /* Represents current WlLoginRequest */
	RsslUInt32		count;
} WlLogin;

/* Creates & opens a login stream. */
WlLoginStream *wlLoginStreamCreate(WlBase *pBase, WlLogin *pLogin, RsslErrorInfo *pErrorInfo);

/* Destroys a login stream. */
void wlLoginStreamDestroy(WlLoginStream *pLoginStream);

/* Closes a login stream. */
void wlLoginStreamClose(WlBase *pBase, WlLogin *pLogin, RsslBool sendCloseMsg);


/* Indicates action the watchlist needs to update its item list, if any,
 * based on the provider's login response. */
typedef enum
{
	WL_LGPA_NONE,		/* No action. */
	WL_LGPA_RECOVER,	/* Recover all items. */
	WL_LGPA_CLOSE		/* Close all items. */
} WlLoginProviderAction;

/* Process a Login RsslMsg from the provider. */
RsslRet wlLoginProcessProviderMsg(WlLogin *pLogin, WlBase *pBase, 
		RsslDecodeIterator *pIter, RsslMsg *iRsslMsg, RsslRDMLoginMsg *oLoginMsg, WlLoginProviderAction *oAction,
		RsslErrorInfo *pErrorInfo);

/* Indicates action the watchlist needs to update its item list, if any, based on
 * the consumer's request. */
typedef enum
{
	WL_LGCA_NONE,		/* No action. */
	WL_LGCA_PAUSE_ALL,	/* Set all items to paused. */
	WL_LGCA_RESUME_ALL,	/* Set all items as not paused. */
	WL_LGCA_CLOSE		/* Close all items. */
} WlLoginConsumerAction;

/* Process a Login message from the consumer. */
RsslRet wlLoginProcessConsumerMsg(WlLogin *pLogin, WlBase *pBase,
		RsslRDMLoginMsg *pLoginMsg, void *pUserSpec, WlLoginConsumerAction *pAction, RsslErrorInfo *pErrorInfo);

/* Updates the login request to the next user token, if the AAA API is in use. */
void wlLoginSetNextUserToken(WlLogin *pLogin, WlBase *pBase);

/* Handle channel-down for login stream (send status if appropriate). */
RsslRet wlLoginChannelDown(WlLogin *pLogin, WlBase *pBase, RsslErrorInfo *pErrorInfo);

#ifdef __cplusplus
}
#endif


#endif

