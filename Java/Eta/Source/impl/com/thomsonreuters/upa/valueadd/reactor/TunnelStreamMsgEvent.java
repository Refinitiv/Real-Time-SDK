package com.thomsonreuters.upa.valueadd.reactor;

/**
 * Event provided to TunnelStreamMsgCallback methods.
 * 
 * @see TunnelStream
 * @see ReactorMsgEvent
 */
public class TunnelStreamMsgEvent extends ReactorMsgEvent
{
    TunnelStream _tunnelStream;
    int _containerType;

    void tunnelStream(TunnelStream tunnelStream)
    {
        _tunnelStream = tunnelStream;
    }

    /**
     * The tunnel stream associated with this message event.
     * 
     * @return TunnelStream
     */
    public TunnelStream tunnelStream()
    {
        return _tunnelStream;
    }
    
    void containerType(int containerType)
    {
        _containerType = containerType;
    }

    /**
     * Returns the container type associated with this message event's buffer.
     */
    public int containerType()
    {
        return _containerType;
    }
    
    /**
     * Clears {@link TunnelStreamMsgEvent}.
     */
    public void clear()
    {
        super.clear();
        _tunnelStream = null;
        _containerType = 0;
    }
}
