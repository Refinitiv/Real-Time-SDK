/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The Reactor channel event callback is used to communicate ReactorChannel and
 * connection state information to the application.
 */
public interface ReactorChannelEventCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * {@link ReactorChannel} and connection state information to the
     * application.
     * 
     * @param event A ReactorChannelEvent containing event information. The
     *            ReactorChannelEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see ReactorChannelEvent
     * @see ReactorCallbackReturnCodes
     */
    public int reactorChannelEventCallback(ReactorChannelEvent event);
}
