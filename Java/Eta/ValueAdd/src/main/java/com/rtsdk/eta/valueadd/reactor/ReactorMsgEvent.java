package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.transport.TransportBuffer;

/**
 * ReactorMsgEvent base class. Used by all other message event classes.
 * 
 * @see ReactorEvent
 */
public class ReactorMsgEvent extends ReactorEvent
{
    TransportBuffer _transportBuffer = null;
    Msg _msg = null;
    WatchlistStreamInfo _streamInfo = new WatchlistStreamInfo();

    void transportBuffer(TransportBuffer transportBuffer)
    {
        _transportBuffer = transportBuffer;
    }

    /**
     * The TransportBuffer associated with this message event.
     * 
     * @return TransportBuffer
     */
    public TransportBuffer transportBuffer()
    {
        return _transportBuffer;
    }

    void msg(Msg msg)
    {
        _msg = msg;
    }

    /**
     * The Msg associated with this message event.
     * 
     * @return Msg
     */
    public Msg msg()
    {
        return _msg;
    }

    /**
     * The WatchlistStreamInfo associated with this message event.
     * Only used when a watchlist is enabled.
     * 
     * @return WatchlistStreamInfo
     */
    public WatchlistStreamInfo streamInfo()
    {
        return _streamInfo;
    }

    /**
     * Clears {@link ReactorMsgEvent}.
     */
    public void clear()
    {
        super.clear();
        _transportBuffer = null;
        _msg = null;
        _streamInfo.clear();
    }

    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return super.toString() + ", " + (_transportBuffer != null ? "TransportBuffer present" : "TransportBuffer null") 
                + ", " + (_msg != null ? "Msg present" : "Msg null");
    }
}
