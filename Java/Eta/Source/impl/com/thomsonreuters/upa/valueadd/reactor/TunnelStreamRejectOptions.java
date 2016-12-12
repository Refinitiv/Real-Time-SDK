package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.State;

/**
 * The options for rejecting a TunnelStream.
 * 
 * @see TunnelStream
 * @see ReactorChannel#rejectTunnelStream(TunnelStreamRequestEvent, TunnelStreamRejectOptions, ReactorErrorInfo)
 *
 */
public class TunnelStreamRejectOptions
{
    State _state = CodecFactory.createState();
    ClassOfService _expectedClassOfService;
    
    /**
     * Returns the expected class of service of the tunnel stream.
     * 
     * @see ClassOfService
     */
    public ClassOfService expectedClassOfService()
    {
        return _expectedClassOfService;
    }

    /**
     * Sets the expected class of service of the tunnel stream.
     * 
     * @see ClassOfService
     */
    public void expectedClassOfService(ClassOfService classOfService)
    {
        _expectedClassOfService = classOfService;
    }
    
    /**
     * Returns the the state of the rejected tunnel stream.
     * Use to retrieve and set the state.
     */
    public State state()
    {
        return _state;
    }
    
    /**
     * Clears the TunnelStreamRejectOptions for re-use.
     */
    public void clear()
    {
        _state.clear();
        _expectedClassOfService = null;
    }    
}
