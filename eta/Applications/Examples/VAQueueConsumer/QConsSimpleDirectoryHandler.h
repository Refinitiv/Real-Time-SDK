/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_QCONS_SIMPLE_DIRECTORY_HANDLER_H
#define _RTR_QCONS_SIMPLE_HANDLER_H

#include "rtr/rsslReactor.h"
#include "QConsChannelStorage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Directory callback function */
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent);

/* Checks to see if the domain is available on the selected service for this channel */
RsslBool getSourceDirectoryCapabilities(ChannelStorage *pCommand, RsslUInt32 domainId);

#ifdef __cplusplus
};
#endif

#endif

