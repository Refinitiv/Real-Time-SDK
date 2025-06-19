/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The tunnel stream status event callback is used to communicate tunnel
 * stream status events to the application.
 */
public interface TunnelStreamStatusEventCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * tunnel stream status events to the application.
     * 
     * @param event A TunnelStreamStatusEvent containing event information. The
     *            TunnelStreamStatusEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see TunnelStream
     * @see TunnelStreamStatusEvent
     * @see ReactorCallbackReturnCodes
     */
    public int statusEventCallback(TunnelStreamStatusEvent event);
}
