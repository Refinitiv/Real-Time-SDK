package com.thomsonreuters.upa.valueadd.reactor;


class RestEvent {

	int _eventType = RestEventTypes.CANCELLED;
	Object _closure = null;
	ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
	ReactorAuthTokenInfo _reactorAuthTokenInfo;
	
	RestEvent(int eventType, Object closure)
	{
		_closure = closure;
		_eventType = eventType;
	}

    public int eventType()
    {
        return _eventType;
    }

    void eventType(int eventType)
    {
        _eventType = eventType;
    }
    
    public Object userSpecObj()
    {
        return _closure;
    }
    
    public ReactorErrorInfo errorInfo()
    {
    	return _errorInfo;
    }
    
    void userSpecObj(Object userSpecObj)
    {
        _closure = userSpecObj;
    }
    
    void clear()
    {
        _eventType = RestEventTypes.CANCELLED;
        _closure = 0;
        _errorInfo.clear();
    }
    
	/**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return (_closure == null ? "\nclosure null" : "\nclosure: " + _closure)
                + "\nevent type: " + RestEventTypes.toString(_eventType)
                + "\nerror info: " + _errorInfo.toString();
    }
}
