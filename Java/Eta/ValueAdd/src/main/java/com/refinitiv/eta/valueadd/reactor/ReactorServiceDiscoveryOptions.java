/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

public class ReactorServiceDiscoveryOptions
{
	private Buffer _username = CodecFactory.createBuffer(); 
	private Buffer _password = CodecFactory.createBuffer(); 
	private Buffer _clientId = CodecFactory.createBuffer();
	private Buffer _clientSecret = CodecFactory.createBuffer();
	private Buffer _tokenScope = CodecFactory.createBuffer();
	private int    _transport = ReactorDiscoveryTransportProtocol.RD_TP_INIT;
	private int    _dataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_INIT;
	private ReactorServiceEndpointEventCallback _reactorServiceEndpointEventCallback = null; 
	private Object _userSpecObject = null;
	private Buffer _proxyHostName = CodecFactory.createBuffer();
	private Buffer    _proxyPort = CodecFactory.createBuffer();
	private Buffer _proxyUserName = CodecFactory.createBuffer();
	private Buffer _proxyPassword = CodecFactory.createBuffer();
	private Buffer _proxyDomain = CodecFactory.createBuffer(); // Needed for NTLM and Kerberos authentication protocols
	private Buffer _proxyLocalHostName = CodecFactory.createBuffer();
	private Buffer _proxyKrb5ConfigFile = CodecFactory.createBuffer();
	private boolean	_takeExclusiveSignOnControl = true;
	
	ReactorServiceDiscoveryOptions ()
	{
		clear();
	}
	
	public void clear()
	{
		_username.clear();
		_password.clear();
		_clientId.clear();
		_clientSecret.clear();
		_tokenScope.data("trapi.streaming.pricing.read");
		_transport = ReactorDiscoveryTransportProtocol.RD_TP_INIT;
		_dataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_INIT;
		_reactorServiceEndpointEventCallback = null;		
		_userSpecObject = null;
		_proxyHostName.clear();
		_proxyPort.clear();
		_proxyUserName.clear();
		_proxyPassword.clear();
		_proxyDomain.clear();
		_proxyLocalHostName.clear();
		_proxyKrb5ConfigFile.clear();
		_takeExclusiveSignOnControl = true;
	}
	
	public int copy(ReactorServiceDiscoveryOptions dest)
	{
		if (dest == null)
			return ReactorReturnCodes.FAILURE;
		
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_username.length());
        	_username.copy(byteBuffer);
        	dest.userName().data(byteBuffer);
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_password.length());
        	_password.copy(byteBuffer);
        	dest.password().data(byteBuffer);        
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_clientId.length());
        	_clientId.copy(byteBuffer);
        	dest.clientId().data(byteBuffer);  
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_clientSecret.length());
        	_clientSecret.copy(byteBuffer);
        	dest.clientSecret().data(byteBuffer);  
        } 
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_tokenScope.length());
        	_tokenScope.copy(byteBuffer);
        	dest.tokenScope().data(byteBuffer);  
        } 
		
		dest._transport = _transport;
		dest._dataFormat = _dataFormat;
		dest._userSpecObject = _userSpecObject;
		dest._reactorServiceEndpointEventCallback = _reactorServiceEndpointEventCallback;
		
		return ReactorReturnCodes.SUCCESS;
	}
	
    /**
     * Specifies the user name for authorization with the token service
     * 
     * @param userName the username.
     */	
	public void userName(Buffer userName)
	{
		assert(userName != null) : "userName can not be null";
        userName().data(userName.data(), userName.position(), userName.length());
	}
	
    /**
     * Returns the user name for authorization with the token service
     * 
     * @return the userName
     */		
	public Buffer userName()
	{
		return _username;
	}
	
    /**
     * Specifies the password for authorization with the token service.
     * 
     * @param password the password.
     */		
	public void password(Buffer password)
	{
		assert(password != null) : "password can not be null";
		password().data(password.data(), password.position(), password.length());
	}
	
    /**
     * Returns the password for authorization with the token service
     * 
     * @return the password
     */			
	public Buffer password()
	{
		return _password;
	}
	
    /**
     * Specifies the unique ID defined for an application making a request to the token service.
     * 
     * @param clientId the client Id
     */			
	public void clientId(Buffer clientId)
	{
		assert(clientId != null) : "clientId can not be null";
		clientId().data(clientId.data(), clientId.position(), clientId.length());
	}

    /**
     * Returns a unique ID for application making the request to RDP token service, also known as AppKey generated using an AppGenerator.
     * 
     * @return clientId.
     */		
	public Buffer clientId()
	{
		return _clientId;
	}
	
	/**
     * Specifies the client secret defined for an application making a request to the token service.
     * 
     * @param clientSecret the client secret
     */			
	public void clientSecret(Buffer clientSecret)
	{
		assert(clientSecret != null) : "clientSecret can not be null";
		clientSecret().data(clientSecret.data(), clientSecret.position(), clientSecret.length());
	}

    /**
     * Returns the client secret for application making the request to RDP token service.
     * 
     * @return clientSecret.
     */		
	public Buffer clientSecret()
	{
		return _clientSecret;
	}
	
	/**
     * Specifies an optional token scope defined for an application making a request to the token service.
     * 
     * @param tokenScope the token scope
     */			
	public void tokenScope(Buffer tokenScope)
	{
		assert(tokenScope != null) : "tokenScope can not be null";
		tokenScope().data(tokenScope.data(), tokenScope.position(), tokenScope.length());
	}

    /**
     * Returns the token scope for application making the request to RDP token service.
     * 
     * @return tokenScope.
     */		
	public Buffer tokenScope()
	{
		return _tokenScope;
	}
	
    /**
     * This is an optional parameter to specify the desired transport protocol to get
	 * service endpoints from the service discovery. 
	 * 
	 * @param transport protocol
	 */	
	public void transport(int transport)
	{
		_transport = transport;
	}
	
    /**
     * This is an optional parameter to specify the desired transport protocol to get
	 * service endpoints from the service discovery. 
	 * 
	 * @return transport protocol
	 */	
	public int transport()
	{
		return _transport;
	}

    /**
     * This is an optional parameter to specify the desired data format protocol to get
	 * service endpoints from the service discovery. 
	 * 
	 * @param dataFormat the data format
	 */
	
	public void dataFormat(int dataFormat)
	{
		_dataFormat = dataFormat;
	}
	
    /**
     * This is an optional parameter to specify the desired data format protocol to get
	 * service endpoints from the service discovery. 
	 * 
	 * @return dataFormat 
	 */	
	
	public int dataFormat()
	{
		return _dataFormat;
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
	
    /**
     * Specifies a Reactor Service Endpoint Event Callback.
     * 
     * @param callback the reactor service endpoint event callback.
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the callback is not
     *         null, otherwise {@link ReactorReturnCodes#PARAMETER_INVALID}.
     */
    public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEventCallback callback)
    {
        if (callback == null)
            return ReactorReturnCodes.PARAMETER_INVALID;

        _reactorServiceEndpointEventCallback = callback;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * A user specified object.
     * 
     * @return the ReactorServiceEndpointEventCallback
     */
    public ReactorServiceEndpointEventCallback reactorServiceEndpointEventCallback()
    {
    	return _reactorServiceEndpointEventCallback;
    }	
	
    /**
     * Specifies a user defined object.
     * 
     * @param userSpecObj the userSpecObj
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the userSpecObj is not
     *         null, otherwise {@link ReactorReturnCodes#PARAMETER_INVALID}.
     */
    public int userSpecObj(Object userSpecObj)
    {
        if (userSpecObj == null)
            return ReactorReturnCodes.PARAMETER_INVALID;

        _userSpecObject = userSpecObj;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * A user specified object.
     * 
     * @return the userSpecObject
     */
    public Object userSpecObject()
    {
    	return _userSpecObject;
    }
	
    /**
     * The exclusive sign on control to force sign-out.
     * 
     * @return - true to force sign-out using the same credential otherwise false.
     */
    public boolean takeExclusiveSignOnControl()
    {
    	return _takeExclusiveSignOnControl;
    }
    
    /**
     * Sets the exclusive sign on control to force sign-out of other applications using the same credentials.
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     */
    public void takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl)
    {
    	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
    }
}


