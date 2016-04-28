package com.thomsonreuters.upa.valueadd.reactor;

/**
 * TunnelStreamSubmitOptions to be used in the
 * {@link TunnelStream#submit(com.thomsonreuters.upa.transport.TransportBuffer, TunnelStreamSubmitOptions, ReactorErrorInfo)} call.
 */
public class TunnelStreamSubmitOptions
{
    int _containerType;
    
    /**
     * Returns the container type of the submitted buffer.
     */
    public int containerType()
    {
        return _containerType;
    }

    /**
     * Sets the container type of the submitted buffer.
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
