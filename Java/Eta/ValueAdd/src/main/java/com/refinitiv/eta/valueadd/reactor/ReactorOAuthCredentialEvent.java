/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * An event that has occurred on a token session to get sensitive information from the application.
 * 
 * @see ReactorEvent
 */

public class ReactorOAuthCredentialEvent extends ReactorEvent 
{
	ReactorOAuthCredentialRenewal _oAuthCredentialRenewal;
	Reactor _reactor;
	Object _userSpecObj = null;
	
	ReactorOAuthCredentialEvent()
	{
		super();
	}
	
	void reactorOAuthCredentialRenewal(ReactorOAuthCredentialRenewal oAuthCredentialRenewal)
	{
		_oAuthCredentialRenewal = oAuthCredentialRenewal;
	}
	
	/**
     * The OAuth credential renewal information associated with this event.
     * 
     * @return ReactorOAuthCredentialRenewal
     */
	public ReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal()
	{
		return _oAuthCredentialRenewal;
	}
	
	/**
	 * The Reactor associated with this event
	 * 
	 * @return The Reactor
	 */
	public Reactor reactor()
	{
		return _reactor;
	}
	
	/**
     * Returns the userSpecObj specified in {@link ReactorOAuthCredential}.
     * 
     * @return the userSpecObj.
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    @Override
    public void returnToPool()
    {
    	/* Clears user-specified object given when the stream was opened by users.*/
    	_userSpecObj = null;
    	
    	_reactor = null;
    	_oAuthCredentialRenewal = null;
    	
    	super.returnToPool();
    }
}
