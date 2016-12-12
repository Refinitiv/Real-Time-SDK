package com.thomsonreuters.upa.valueadd.reactor;

/**
 * An event that has occurred on a ReactorChannel.
 * 
 * @see ReactorEvent
 */
public class ReactorChannelEvent extends ReactorEvent
{
    int _eventType = 0;

    /**
     * The event type.
     * 
     * @see ReactorChannelEventTypes
     * 
     * @return the event type
     */
    public int eventType()
    {
        return _eventType;
    }

    void eventType(int eventType)
    {
        _eventType = eventType;
    }
    
    void clear()
    {
        super.clear();
        _eventType = 0;
    }
    
    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return super.toString() + ", "
                + (_reactorChannel == null ? "ReactorChannel null" : _reactorChannel.toString())
                + ", " + ReactorChannelEventTypes.toString(_eventType);
    }
}
