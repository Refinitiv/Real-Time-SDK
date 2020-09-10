package com.rtsdk.proxy.authentication;

public interface IProxyAuthenticatorResponse
{
    
    /**
     * Checks if is proxy connection close.
     *
     * @return true, if is proxy connection close
     */
    public boolean isProxyConnectionClose();
	
    /**
     * Gets the proxy authorization.
     *
     * @return the proxy authorization
     */
    public String getProxyAuthorization();
}
