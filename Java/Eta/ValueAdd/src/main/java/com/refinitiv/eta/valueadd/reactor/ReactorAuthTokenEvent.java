package com.refinitiv.eta.valueadd.reactor;


/**
 * Event provided to ReactorAuthTokenEventCallback methods.
 * 
 * @see ReactorAuthTokenInfo
 */
public class ReactorAuthTokenEvent extends ReactorEvent
{
	ReactorAuthTokenInfo _reactorAuthTokenInfo;
	
	ReactorAuthTokenEvent()
    {
        super();
    }

    void reactorAuthTokenInfo(ReactorAuthTokenInfo reactorAuthTokenInfo)
    {
    	_reactorAuthTokenInfo = reactorAuthTokenInfo;
    }   
    
    /**
     * The Authorization Token Info associated with this message event.
     * 
     * @return ReactorAuthTokenInfo
     */
    public ReactorAuthTokenInfo reactorAuthTokenInfo()
    {
        return _reactorAuthTokenInfo;
    }  

}