/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_POST_HANDLER_H
#define _RTR_RSSL_POST_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sends a post message if enabled, alternates between post message 
   containing another message and a post message containing a contaner type */
RsslRet handlePosts(RsslReactor *pReactor, ChannelCommand* pCommand);
/* initializes timing mechanism associated with sending posts */
void initPostHandler(ChannelCommand *pCommand);
/* sends an on-stream post message if needed, can be told to send post with message 
   or post with data */
RsslRet sendOnstreamPostMsg(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslBool postWithMsg);
/* sends an off-stream post message if needed */
RsslRet sendOffStreamPostMsg(RsslReactor *pReactor, RsslReactorChannel* chnl);
/* closes any offstream posts that were made */
RsslRet closeOffstreamPost(RsslReactor *pReactor, ChannelCommand* pCommand);

#ifdef __cplusplus
};
#endif

#endif
