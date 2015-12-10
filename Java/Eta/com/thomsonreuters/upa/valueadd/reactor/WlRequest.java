package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.valueadd.common.VaNode;

/* Watchlist user request that contains user request information. */
public class WlRequest extends VaNode
{
    RequestMsg _requestMsg = (RequestMsg)CodecFactory.createMsg();  
    WlHandler _handler;
    WlStream _stream;
    WatchlistStreamInfo _streamInfo = new WatchlistStreamInfo();
    State _state = State.PENDING_REQUEST;
    boolean _hasStaticQos;
    Qos _matchedQos = CodecFactory.createQos();
    
    WlRequest()
    {
        _requestMsg.msgClass(MsgClasses.REQUEST);
    }
    
    /* The state of the watchlist request. */
    enum State
    {
        PENDING_REQUEST,
        REFRESH_PENDING,
        REFRESH_COMPLETE_PENDING, // multi-part
        OPEN
    }
    
    /* Returns the request message. */
    RequestMsg requestMsg()
    {
        return _requestMsg;
    }
    
    /* Handler associated with request. */
    WlHandler handler()
    {
        return _handler;
    }
    
    /* Set the handler associated with request. */
    void handler(WlHandler handler)
    {
        _handler = handler;
    }

    /* Stream associated with request. */
    WlStream stream()
    {
        return _stream;
    }
    
    /* Set the stream associated with request. */
    void stream(WlStream stream)
    {
        _stream = stream;
    }

    /* Returns whether or not request has a static Qos. */
    boolean hasStaticQos()
    {
        return _hasStaticQos;
    }

    /* Sets whether or not request has a static Qos. If this is set,
     * then the matchedQos is the static Qos. */
    void hasStaticQos(boolean hasStaticQos)
    {
        _hasStaticQos = hasStaticQos;
    }

    /* Matched Qos of the request. This is the static Qos if
     * hasStaticQos is set. */
    Qos matchedQos()
    {
        return _matchedQos;
    }
    
    /* Stream information for the stream. */
    WatchlistStreamInfo streamInfo()
    {
        return _streamInfo;
    }

    /* Sets the state of the user request. */
    void state(State state)
    {
        _state = state;
    }

    /* Returns the state of the user request. */
    State state()
    {
        return _state;
    }

    /* Clears the object for re-use. */
    void clear()
    {
        _requestMsg.clear();
        _requestMsg.msgClass(MsgClasses.REQUEST);
        _handler = null;
        _stream = null;
        _streamInfo.clear();
        _state = State.PENDING_REQUEST;
        _hasStaticQos = false;
        _matchedQos.clear();
    }
}
