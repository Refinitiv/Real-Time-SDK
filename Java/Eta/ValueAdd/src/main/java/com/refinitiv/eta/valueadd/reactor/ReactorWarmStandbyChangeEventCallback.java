/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The Reactor warm standby change event callback is used to communicate
 * information about the ongoing warm standby operation to the application.
 */
public interface ReactorWarmStandbyChangeEventCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * information about the ongoing warm standby operation to the application.
     *
     * @param event A ReactorWarmStandbyChangeEvent containing event information.
     *            The ReactorWarmStandbyChangeEvent is valid only during callback
     *
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *
     * @see ReactorWarmStandbyChangeEvent
     * @see ReactorCallbackReturnCodes
     */
    int reactorWarmStandbyChangeEventCallback(ReactorWarmStandbyChangeEvent event);
}
