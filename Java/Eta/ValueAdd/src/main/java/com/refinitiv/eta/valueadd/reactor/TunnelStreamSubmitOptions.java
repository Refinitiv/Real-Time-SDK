package com.refinitiv.eta.valueadd.reactor;

/**
 * TunnelStreamSubmitOptions to be used in the
 * {@link TunnelStream#submit(com.refinitiv.eta.transport.TransportBuffer, TunnelStreamSubmitOptions, ReactorErrorInfo)} call.
 */
public class TunnelStreamSubmitOptions
{
    int _containerType;
    
    /**
     * Returns the container type of the submitted buffer.
     *
     * @return the int
     */
    public int containerType()
    {
        return _containerType;
    }

    /**
     * Sets the container type of the submitted buffer.
     *
     * @param containerType the container type
     */
    public void containerType(int containerType)
    {
        _containerType = containerType;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _containerType = 0;
    }
}
