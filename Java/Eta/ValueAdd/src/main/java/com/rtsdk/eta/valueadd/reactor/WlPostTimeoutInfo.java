package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CopyMsgFlags;
import com.rtsdk.eta.codec.PostMsg;
import com.rtsdk.eta.valueadd.common.VaNode;

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
