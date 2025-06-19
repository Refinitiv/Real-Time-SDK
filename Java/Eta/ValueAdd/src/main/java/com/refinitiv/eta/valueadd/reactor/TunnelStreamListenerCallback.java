/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The tunnel stream listener callback is used to communicate tunnel
 * stream request information to a tunnel stream provider application.
 * 
 * The tunnel stream provider application must either accept or reject
 * the request within the callback.
 * 
 * @see TunnelStreamRequestEvent
 * @see ReactorChannel#acceptTunnelStream(TunnelStreamRequestEvent, TunnelStreamAcceptOptions, ReactorErrorInfo)
 * @see ReactorChannel#rejectTunnelStream(TunnelStreamRequestEvent, TunnelStreamRejectOptions, ReactorErrorInfo)
 */
public interface TunnelStreamListenerCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * tunnel stream request information to a tunnel stream provider application.
     * 
     * @param event TunnelStreamRequestEvent containing request information. The
     *            TunnelStreamRequestEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see TunnelStream
     * @see TunnelStreamRequestEvent
     * @see ReactorCallbackReturnCodes
     */
    public int listenerCallback(TunnelStreamRequestEvent event);
}
