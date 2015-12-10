package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * Reactor connection information for use in {@link ReactorConnectOptions}.
 */
public class ReactorConnectInfo
{
    ConnectOptions _connectOptions = null;
    int DEFAULT_TIMEOUT = 60;
    int _initTimeout = DEFAULT_TIMEOUT;

    ReactorConnectInfo()
    {
        _connectOptions = TransportFactory.createConnectOptions();
    }

    /**
     * Returns the {@link ConnectOptions}, which is the ConnectOptions
     * associated with the underlying
     * {@link Transport#connect(ConnectOptions, com.thomsonreuters.upa.transport.Error)
     * Transport.connect} method. This includes information about the host or
     * network to connect to, the type of connection to use, and other transport
     * specific configuration information. This is described in more detail in
     * the UPA Developers Guide.
     * 
     * @return the {@link ConnectOptions}
     */
    public ConnectOptions connectOptions()
    {
        return _connectOptions;
    }

    /**
     * The amount of time (in seconds) to wait for the successful initialization
     * of a {@link ReactorChannel}. If initialization does not complete in time,
     * an event is dispatched to the application to indicate that the ReactorChannel
     * is down. Timeout must be greater than zero. Default is 60 seconds.
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

        _initTimeout = timeout;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the initialization timeout value.
     * 
     * @return the initialization timeout value
     */
    public int initTimeout()
    {
        return _initTimeout;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _connectOptions.clear();
        _initTimeout = DEFAULT_TIMEOUT;
    }
}
