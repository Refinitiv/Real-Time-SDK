/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_LOGIN_CONSUMER_H
#define _RTR_RSSL_LOGIN_CONSUMER_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rsslLoginEncodeDecode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* rsslLoginConsumer only supports a single login instance for an application */

#define RSSL_CONSUMER 0
#define RSSL_PROVIDER 1
#define LOGIN_STREAM_ID 1

typedef void (*LoginSuccessCallback)(RsslChannel* chnl);

/* allows the application to set a username */
void setUsername(char* username);
/* allows the application to set an authentication token */
void setAuthenticationToken(char* authenticationToken);
/* allows the application to set authentication extended information */
void setAuthenticationExtended(char* authenticationExtended);
/* allows the application to set an application id */
void setApplicationId(char* applicationId);

/* sends a login request */
RsslRet sendLoginRequest(RsslChannel* chnl, const char *appName, RsslUInt64 role, LoginSuccessCallback loginSuccessCB);
/* processes login response messages */
RsslRet processLoginResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* closes the stream associated with the login */
RsslRet closeLoginStream(RsslChannel* chnl);
/* returns attributes from the login response - useful to determine features supported by provider */
RsslLoginResponseInfo* getLoginResponseInfo();
/* returns whether login stream is closed */
RsslBool isLoginStreamClosed();
/* returns whether login stream is closed recoverable */
RsslBool isLoginStreamClosedRecoverable();
/* returns whether login stream is suspect */
RsslBool isLoginStreamSuspect();
/* returns whether provider dictionary download is supported */
RsslBool isProviderDictionaryDownloadSupported();

#ifdef __cplusplus
};
#endif

#endif
