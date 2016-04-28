package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * ReactorAcceptOptions to be used in the {@link Reactor#accept(com.thomsonreuters.upa.transport.Server,
 * ReactorAcceptOptions, ReactorRole, ReactorErrorInfo)} call.
 */
public class ReactorAcceptOptions
{
    AcceptOptions _acceptOptions = null;
    int DEFAULT_TIMEOUT = 60;
    int _timeout = DEFAULT_TIMEOUT;

    ReactorAcceptOptions()
    {
        _acceptOptions = TransportFactory.createAcceptOptions();
    }

    /**
     * Returns the {@link AcceptOptions}, which is the AcceptOptions associated
     * with the underlying Reactor#accept(Server, ReactorConnectOptions,
     * ReactorRole, ReactorErrorInfo)} method. This includes an option to reject
     * the connection as well as a userSpecObj. This is described in more detail
     * in the UPA Developers Guide.
     * 
     * @return acceptOptions
     */
    public AcceptOptions acceptOptions()
    {
        return _acceptOptions;
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
    }
}
