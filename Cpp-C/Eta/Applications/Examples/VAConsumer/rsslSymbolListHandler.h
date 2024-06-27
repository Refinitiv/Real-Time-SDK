/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/


#ifndef _RTR_RSSL_SYMBOL_LIST_HANDLER_H
#define _RTR_RSSL_SYMBOL_LIST_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* called to convert map entry actions to their string representation */
const char * mapEntryActionToString(RsslUInt8 mapEntryAction);

/* called by rsslDirectoryHandler to set the symbol list name */
void setSymbolListName(ChannelCommand *pCommand, RsslBuffer slName);

/* called by rsslConsumer to enable snapshot requesting symbol lists */
void setSLSnapshotRequest();

/* called to send all symbol list requests */
RsslRet sendSymbolListRequests(RsslReactor *pReactor, RsslReactorChannel* chnl);

/* called to process all symbol list responses */
RsslRet processSymbolListResponse(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter);

#ifdef __cplusplus
};
#endif

#endif
