/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* dictionaryProvider.h
 * Provides sending of the interactive provider's dictionary, if requested. */

#ifndef _DICTIONARY_PROVIDER_H
#define _DICTIONARY_PROVIDER_H

#include "channelHandler.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDMDictionaryMsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* reasons a dictionary request is rejected */
typedef enum {
	DICTIONARY_REJECT_UNKNOWN_NAME
} DictionaryRejectReason;

/* Initializes the Dictionary Provider. */
void initDictionaryProvider();

/* Process a request for a dictionary. */
RsslRet processDictionaryRequest(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslMsg* msg, RsslDecodeIterator* dIter);

/* Send the appropriate dictionary. */
static RsslRet sendDictionaryResponse(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslRDMDictionaryRequest *pDictionaryRequest, RDMDictionaryTypes type);

/* Reject a dictionary request. */
static RsslRet sendDictionaryRequestReject(ChannelHandler *pChannelHandler, ChannelInfo *pChannelInfo, 
		RsslInt32 streamId, DictionaryRejectReason reason);

#ifdef __cplusplus
};
#endif

#endif

