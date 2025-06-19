/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The RDM dictionary message callback is used to communicate dictionary
 * message events to the application.
 */
public interface RDMDictionaryMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * dictionary message events to the application.
     * 
     * @param event A ReactorMsgEvent containing event information. The
     *            ReactorMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see RDMDictionaryMsgEvent
     * @see ReactorCallbackReturnCodes
     */
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event);
}
