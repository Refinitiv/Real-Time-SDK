/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_POST_HANDLER_H
#define _RTR_RSSL_POST_HANDLER_H

#include "rsslConsumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sends a post message if enabled, alternates between post message 
   containing another message and a post message containing a container type */
RsslRet handlePosts(RsslChannel* chnl, RsslBool connectionRecovery);
/* initializes timing mechanism associated with sending posts */
void initPostHandler();
/* sends an on-stream post message if needed, can be told to send post 
   with message or post with data */
RsslRet sendOnstreamPostMsg(RsslChannel* chnl, RsslBool postWithMsg);
/* sends an off-stream post message if needed */
RsslRet sendOffstreamPostMsg(RsslChannel* chnl);
/* closes any offstream posts that were made */
RsslRet closeOffstreamPost(RsslChannel* chnl);
/* enables offstream posting mode */
void enableOffstreamPost();
/* disables offstream posting mode */
void disableOffstreamPost();
/* enables onstream posting mode */
void enableOnstreamPost();
/* disables onstream posting mode */
void disableOnstreamPost();


#ifdef __cplusplus
};
#endif

#endif
