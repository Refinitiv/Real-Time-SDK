/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.valueadd.common.VaNode;

/* Used by the watchlist to handle post message timeouts. */
class WlPostTimeoutInfo extends VaNode
{
    PostMsg _postMsg = (PostMsg)CodecFactory.createMsg();
    long _timeout;
    
    
    /* Returns the post message. */
    PostMsg postMsg()
    {
        return _postMsg;
    }
    
    /* Sets the post message. */
    void postMsg(PostMsg postMsg)
    {
        postMsg.copy(_postMsg, CopyMsgFlags.ALL_FLAGS);
    }

    /* Returns the timeout value. */
    long timeout()
    {
        return _timeout;
    }

    /* Sets the timeout value. */
    void timeout(long timeout)
    {
        _timeout = timeout;
    }
    
    /* Clears the object for re-use. */
    void clear()
    {
        _postMsg.clear();
        _timeout = 0;
    }
}
