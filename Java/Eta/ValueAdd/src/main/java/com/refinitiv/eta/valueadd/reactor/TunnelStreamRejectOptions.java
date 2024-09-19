/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;

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
     * Instantiates a new tunnel stream reject options.
     */
    public TunnelStreamRejectOptions()
    {
        _state.streamState(StreamStates.CLOSED_RECOVER);
        _state.dataState(DataStates.SUSPECT);
    }
    
    /**
     * Returns the expected class of service of the tunnel stream.
     *
     * @return the class of service
     * @see ClassOfService
     */
    public ClassOfService expectedClassOfService()
    {
        return _expectedClassOfService;
    }

    /**
     * Sets the expected class of service of the tunnel stream.
     *
     * @param classOfService the class of service
     * @see ClassOfService
     */
    public void expectedClassOfService(ClassOfService classOfService)
    {
        _expectedClassOfService = classOfService;
    }
    
    /**
     * Returns the the state of the rejected tunnel stream.
     * Use to retrieve and set the state.
     *
     * @return the state
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
        _state.streamState(StreamStates.CLOSED_RECOVER);
        _state.dataState(DataStates.SUSPECT);
        _expectedClassOfService = null;
    }    
}
