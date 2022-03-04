/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The Reactor OAuth credential event callback is used to communicate {@link ReactorOAuthCredentialRenewal} in order to get
 * sensitive information from the application.
 */
public interface ReactorOAuthCredentialEventCallback 
{
	/**
     * A callback function that the {@link Reactor} will use to get sensitive information from the application
     * The reactorSubmitOAuthCredentialRenewal() method is used to submit sensitive information.
     * 
     * @param reactorOAuthCredentialEvent containing event information. The
     *            ReactorOAuthCredentialEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see ReactorOAuthCredentialEvent
     * @see ReactorCallbackReturnCodes
     */
	public int reactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent);
}
