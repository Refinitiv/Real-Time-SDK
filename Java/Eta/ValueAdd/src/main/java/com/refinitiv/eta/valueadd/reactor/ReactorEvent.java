package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaNode;

/** ReactorEvent base class. Used by all other event classes. */ 
public class ReactorEvent extends VaNode
{
    ReactorChannel _reactorChannel = null;
    ReactorErrorInfo _errorInfo = null;

    ReactorEvent()
    {
        _errorInfo = ReactorFactory.createReactorErrorInfo();
    }

    /**
     * The ReactorChannel associated with this event.
     * 
     * @return ReactorChannel
     */
    public ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }
    
    void reactorChannel(ReactorChannel channel)
    {
        _reactorChannel = channel;
    }

    /**
     * The ReactorErrorInfo associated with this event.
     * 
     * @return ReactorErrorInfo
     */
    public ReactorErrorInfo errorInfo()
    {
        return _errorInfo;
    }

    void clear()
    {
        _reactorChannel = null;
        _errorInfo.clear();
    }
    
    @Override
    public void returnToPool()
    {
    	_reactorChannel = null;
    	
    	super.returnToPool();
    }
    
    @Override
    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return (_reactorChannel != null ? _reactorChannel.toString() : "ReactorChannel null")
                + ", " + _errorInfo.toString(); 
    }
}
