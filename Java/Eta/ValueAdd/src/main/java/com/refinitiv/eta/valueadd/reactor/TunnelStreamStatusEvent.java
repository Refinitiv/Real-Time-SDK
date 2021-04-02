package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;

/**
 * Event provided to TunnelStreamStatusEventCallback methods.
 * 
 * @see TunnelStream
 * @see ReactorMsgEvent
 */
public class TunnelStreamStatusEvent extends ReactorMsgEvent
{
    TunnelStream _tunnelStream;
    State _state = CodecFactory.createState();
    TunnelStreamAuthInfo _authInfo;
    
    /**
     * Returns the state of this event.
     * 
     * @return TunnelStream
     */
    public State state()
    {
        return _state;
    }

    void tunnelStream(TunnelStream tunnelStream)
    {
        _tunnelStream = tunnelStream;
    }

    /**
     * The tunnel stream associated with this event.
     * 
     * @return TunnelStream
     */
    public TunnelStream tunnelStream()
    {
        return _tunnelStream;
    }
    
    /**
     * (Consumers only) Provides information about a received authentication response.
     * 
     * @return TunnelStreamAuthInfo
     */
    public TunnelStreamAuthInfo authInfo()
    {
        return _authInfo;
    }

    void authInfo(TunnelStreamAuthInfo authInfo)
    {
        _authInfo = authInfo;
    }

    /**
     * Clears {@link TunnelStreamStatusEvent}.
     */
    public void clear()
    {
        super.clear();
        _state.clear();
        _tunnelStream = null;
        _authInfo = null;
    }
    
    @Override
    public void returnToPool()
    {
    	_tunnelStream = null;
        _authInfo = null;
    	
    	super.returnToPool();
    }
}
