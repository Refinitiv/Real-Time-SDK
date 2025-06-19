/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
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
void setSymbolListName(char *slName);

/* called by rsslConsumer to set the symbol list request information */
void setSymbolListInfo(RsslBool slNameGiven, RsslBool slInfo); 

/* called by rsslConsumer to enable snapshot requesting symbol lists */
void setSLSnapshotRequest();

/* called to send all symbol list requests */
RsslRet sendSymbolListRequests(RsslChannel* chnl);

/* called to process all symbol list responses */
RsslRet processSymbolListResponse(RsslMsg* msg, RsslDecodeIterator* dIter);

/* called to close all symbol list streams */
RsslRet closeSymbolListStream(RsslChannel* chnl);

/* called to return the value of slNameGivenInfo */
RsslBool getSlNameGivenInfo();

#ifdef __cplusplus
};
#endif

#endif
