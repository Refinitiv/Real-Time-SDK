package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;

public class ReactorOAuthCredentialRenewalOptions 
{
	private Buffer _proxyHostName = CodecFactory.createBuffer();
	private Buffer _proxyPort = CodecFactory.createBuffer();
	private Buffer _proxyUserName = CodecFactory.createBuffer();
	private Buffer _proxyPassword = CodecFactory.createBuffer();
	private Buffer _proxyDomain = CodecFactory.createBuffer();
	private Buffer _proxyLocalHostName = CodecFactory.createBuffer();
	private Buffer _proxyKrb5ConfigFile = CodecFactory.createBuffer();
	private int _renewalModes = 0;
	private ReactorAuthTokenEventCallback _reactorAuthTokenEventCallback;
	
	 /**
     * OAuth credential renewal mode enumerations.
     */
    public static class RenewalModes
    {
        // OAuthCredentialRenewalModes class cannot be instantiated
        private RenewalModes()
        {
            throw new AssertionError();
        }

        /** None */
        public static final int NONE = 0;

        /** Renew access token with password only */
        public static final int PASSWORD = 1;
        
        /** Renew access token with changing password*/
        public static final int PASSWORD_CHANGE = 2;
    }
    
    ReactorOAuthCredentialRenewalOptions()
    {
    	clear();
    }
	
    public void clear()
    {
    	_renewalModes = RenewalModes.NONE;
    	_proxyHostName.clear();
    	_proxyPort.clear();
    	_proxyUserName.clear();
    	_proxyPassword.clear();
    	_proxyDomain.clear();
    }
    
    /**
     * Specifies the modes for the OAuth credential renewal.
     * 
     * @param renewalModes the renewal modes.
     */	
    public void renewalModes(int renewalModes)
    {
    	_renewalModes = renewalModes;
    }

    /**
     * Returns the modes for the OAuth credential renewal.
     * 
     * @return the renewal modes.
     */
    public int renewalModes()
    {
    	return _renewalModes;
    }
    
    /**
     * Specifies a Callback function that receives ReactorAuthTokenEvents when the submitOAuthCredentialRenewal() method is called
     * outside of the {@link ReactorOAuthCredentialEventCallback}.
     * 
     * @param callback the auth token event callback.
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the callback is not
     *         null, otherwise {@link ReactorReturnCodes#PARAMETER_INVALID}.
     *         
     * @see ReactorAuthTokenEventCallback
     * @see ReactorAuthTokenEvent         
     */
    public int reactorAuthTokenEventCallback(ReactorAuthTokenEventCallback callback)
    {
        if (callback == null)
            return ReactorReturnCodes.PARAMETER_INVALID;

        _reactorAuthTokenEventCallback = callback;
        return ReactorReturnCodes.SUCCESS;
    }
    
    /** A callback function for processing AuthTokenEvents received.
     * 
     * @return the reactorAuthTokenEventCallback
     * @see ReactorAuthTokenEventCallback
     * @see ReactorAuthTokenEvent
     */
    public ReactorAuthTokenEventCallback reactorAuthTokenEventCallback()
    {
        return _reactorAuthTokenEventCallback;
    } 
    
    /**
     * Specifies the address or hostname of the HTTP proxy server.
     * 
     * @param proxyHostName the proxy hostname.
     */	
	public void proxyHostName(Buffer proxyHostName)
	{
		assert(proxyHostName != null) : "proxyHostName can not be null";
		proxyHostName().data(proxyHostName.data(), proxyHostName.position(), proxyHostName.length());
	}
	
    /**
     * Returns the address or hostname of the HTTP proxy server.
     * 
     * @return the proxy hostname.
     */		
	public Buffer proxyHostName()
	{
		return _proxyHostName;
	}
	
	/**
     * Specifies the Port Number of the HTTP proxy server.
     * 
     * @param proxyPort the proxy port.
     */	
	public void proxyPort(Buffer proxyPort)
	{
		_proxyPort = proxyPort;
	}
	
    /**
     * Returns the Port Number of the HTTP proxy server.
     * 
     * @return the proxy port
     */		
	public Buffer proxyPort()
	{
		return _proxyPort;
	}
	
	/**
     * Specifies the proxy user name to authenticate.
     * 
     * @param proxyUserName the proxy user name.
     */	
	public void proxyUserName(Buffer proxyUserName)
	{
		assert(proxyUserName != null) : "proxyUserName can not be null";
		proxyUserName().data(proxyUserName.data(), proxyUserName.position(), proxyUserName.length());
	}
	
    /**
     * Returns the proxy user name to authenticate.
     * 
     * @return the proxy user name.
     */		
	public Buffer proxyUserName()
	{
		return _proxyUserName;
	}
	
	/**
     * Specifies the proxy password to authenticate.
     * 
     * @param proxyPassword the proxy password.
     */	
	public void proxyPassword(Buffer proxyPassword)
	{
		assert(proxyPassword != null) : "proxyPassword can not be null";
		proxyPassword().data(proxyPassword.data(), proxyPassword.position(), proxyPassword.length());
	}
	
    /**
     * Returns the proxy password to authenticate.
     * 
     * @return the proxy password.
     */		
	public Buffer proxyPassword()
	{
		return _proxyPassword;
	}

	/**
     * Specifies the proxy domain of the user to authenticate. 
     * 
     * @param proxyDomain the proxy domain.
     */	
	public void proxyDomain(Buffer proxyDomain)
	{
		assert(proxyDomain != null) : "proxyDomain can not be null";
		proxyDomain().data(proxyDomain.data(), proxyDomain.position(), proxyDomain.length());
	}
	
    /**
     * Returns the proxy domain of the user to authenticate.
     * 
     * @return the proxy domain.
     */		
	public Buffer proxyDomain()
	{
		return _proxyDomain;
	}
	
	/**
     * Specifies the local hostname of the user to authenticate. 
     * 
     * @param proxyLocalHostName the local hostname.
     */	
	public void proxyLocalHostName(Buffer proxyLocalHostName)
	{
		assert(proxyLocalHostName != null) : "proxyLocalHostName can not be null";
		proxyLocalHostName().data(proxyLocalHostName.data(), proxyLocalHostName.position(), proxyLocalHostName.length());
	}
	
    /**
     * Returns the proxy local hostname of the user to authenticate with NTLM protocol only.
     * 
     * @return the local hostname.
     */		
	public Buffer proxyLocalHostName()
	{
		return _proxyLocalHostName;
	}
	
	/**
     * Specifies the complete path of the Keberos5 configuration file.
     * <p>Needed for Negotiate/Kerberos and Kerberos authentications.</p> 
     * 
     * @param proxyKRB5ConfigFile the Keberos5 configuration file.
     */	
	public void proxyKRB5ConfigFile(Buffer proxyKRB5ConfigFile)
	{
		assert(proxyKRB5ConfigFile != null) : "proxyKRB5ConfigFile can not be null";
		proxyKRB5ConfigFile().data(proxyKRB5ConfigFile.data(), proxyKRB5ConfigFile.position(), proxyKRB5ConfigFile.length());
	}
	
    /**
     * Returns the complete path of the Keberos5 configuration file.
     * 
     * @return the Keberos5 configuration file.
     */		
	public Buffer proxyKRB5ConfigFile()
	{
		return _proxyKrb5ConfigFile;
	}
}
