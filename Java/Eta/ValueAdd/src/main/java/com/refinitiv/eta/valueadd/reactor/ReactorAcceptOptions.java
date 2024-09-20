/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.TransportFactory;

/**
 * ReactorAcceptOptions to be used in the {@link Reactor#accept(com.refinitiv.eta.transport.Server,
 * ReactorAcceptOptions, ReactorRole, ReactorErrorInfo)} call.
 */
public class ReactorAcceptOptions
{
    AcceptOptions _acceptOptions = null;
    int DEFAULT_TIMEOUT = 60;
    int _timeout = DEFAULT_TIMEOUT;
    ReactorWSocketAcceptOptions _wsocketAcceptOptions;

    ReactorAcceptOptions()
    {
        _acceptOptions = TransportFactory.createAcceptOptions();
        _wsocketAcceptOptions = new ReactorWSocketAcceptOptions();
    }

    /**
     * Returns the {@link AcceptOptions}, which is the AcceptOptions associated
     * with the underlying Reactor#accept(Server, ReactorConnectOptions,
     * ReactorRole, ReactorErrorInfo)} method. This includes an option to reject
     * the connection as well as a userSpecObj. This is described in more detail
     * in the ETA Developers Guide.
     *
     * @return acceptOptions
     */
    public AcceptOptions acceptOptions()
    {
        return _acceptOptions;
    }

    /**
     * Returns the {@link ReactorWSocketAcceptOptions} to specify whether to send a ping message to clients.
     *
     * @return ReactorWSocketAcceptOptions
     */
    public ReactorWSocketAcceptOptions websocketAcceptOptions()
    {
        return _wsocketAcceptOptions;
    }

    /**
     * The amount of time (in seconds) to wait for the successful connection
     * establishment of a {@link ReactorChannel}. If a timeout occurs, an event
     * is dispatched to the application to indicate that the ReactorChannel is
     * down. Timeout must be greater than zero. Default is 60 seconds.
     *
     * @param timeout the initialization timeout in seconds
     *
     * @return {@link ReactorReturnCodes#SUCCESS} if the timeout is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the timeout is out of range
     */
    public int initTimeout(int timeout)
    {
        if (timeout < 1)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        _timeout = timeout;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the initialization timeout value.
     *
     * @return the initialization timeout value
     */
    public int initTimeout()
    {
        return _timeout;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _acceptOptions.clear();
        _timeout = DEFAULT_TIMEOUT;
        _wsocketAcceptOptions.clear();
    }
}
