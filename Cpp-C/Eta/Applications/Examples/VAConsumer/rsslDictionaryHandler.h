/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_RSSL_DICTIONARY_HANDLER_H
#define _RTR_RSSL_DICTIONARY_HANDLER_H

#include "rsslConsumer.h"
#include "rtr/rsslReactor.h"
#include "rsslChannelCommand.h"

#ifdef __cplusplus
extern "C" {
#endif

void loadDictionary(ChannelCommand *pCommand);
RsslBool isDictionaryLoaded(ChannelCommand *pCommand);
RsslDataDictionary* getDictionary(ChannelCommand *pCommand);
void freeDictionary(ChannelCommand *pCommand);
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsgEvent *pDictionaryMsgEvent);

#ifdef __cplusplus
};
#endif

#endif
