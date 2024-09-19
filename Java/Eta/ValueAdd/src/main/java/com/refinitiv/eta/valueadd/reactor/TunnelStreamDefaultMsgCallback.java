/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The tunnel stream default message callback is used to communicate tunnel
 * stream message events to the application.
 */
public interface TunnelStreamDefaultMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * tunnel stream message events to the application.
     * 
     * @param event A TunnelStreamMsgEvent containing event information. The
     *            TunnelStreamMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see TunnelStream
     * @see TunnelStreamMsgEvent
     * @see ReactorCallbackReturnCodes
     */
    public int defaultMsgCallback(TunnelStreamMsgEvent event);
}
