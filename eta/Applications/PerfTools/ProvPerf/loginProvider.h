/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* loginProvider.h
 * Determines the information a provider needs for accepting logins, and provides
 * encoding of appropriate responses. */

#ifndef _LOGIN_PROVIDER_H
#define _LOGIN_PROVIDER_H

#include "channelHandler.h"
#include "rtr/rsslRDMLoginMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Provides configuration for handling the login domain. */
typedef struct {
	RsslBuffer applicationName;
	RsslBuffer applicationId;
	RsslBuffer position;
	char positionChar[256];
} LoginConfig;

extern LoginConfig loginConfig;

/* Clear the LoginConfig. */
void clearLoginConfig();

/* Set the application position in the LoginConfig to the application's IP address. */
void setLoginConfigPosition();

/* Decodes a login request and sends an appropriate response. */
RsslRet processLoginRequest(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslMsg* msg, RsslDecodeIterator* dIter);

#ifdef __cplusplus
};
#endif

#endif
