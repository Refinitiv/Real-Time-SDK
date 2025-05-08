/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022, 2025 LSEG. All rights reserved.
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
	private Buffer _audience = CodecFactory.createBuffer();
	private Buffer _clientJWK = CodecFactory.createBuffer();
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
		_clientJWK.clear();
		_audience.data("https://login.ciam.refinitiv.com/as/token.oauth2");
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
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_audience.length());
        	_audience.copy(byteBuffer);
        	dest.audience().data(byteBuffer);  
        } 
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_clientJWK.length());
        	_clientJWK.copy(byteBuffer);
        	dest.clientJWK().data(byteBuffer);  
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
     * Sets the user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins.
     * 
     * @param userName the username.
     */	
	public void userName(Buffer userName)
	{
		assert(userName != null) : "userName can not be null";
        userName().data(userName.data(), userName.position(), userName.length());
	}
	
    /**
     * Gets the user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins.
     * 
     * @return the userName
     */		
	public Buffer userName()
	{
		return _username;
	}
	
    /**
     * Sets the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins 
     * 
     * @param password the password.
     */		
	public void password(Buffer password)
	{
		assert(password != null) : "password can not be null";
		password().data(password.data(), password.position(), password.length());
	}
	
    /**
     * Gets the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins 
     * 
     * @return the password
     */			
	public Buffer password()
	{
		return _password;
	}
	
    /**
     * Sets the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
     * 
     * @param clientId the client Id
     */			
	public void clientId(Buffer clientId)
	{
		assert(clientId != null) : "clientId can not be null";
		clientId().data(clientId.data(), clientId.position(), clientId.length());
	}

    /**
     * Gets the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
     * 
     * @return clientId.
     */		
	public Buffer clientId()
	{
		return _clientId;
	}
	
	/**
     * Sets the clientSecret, also known as the Service Account password, used to authenticate with LDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
     * 
     * @param clientSecret the client secret
     */			
	public void clientSecret(Buffer clientSecret)
	{
		assert(clientSecret != null) : "clientSecret can not be null";
		clientSecret().data(clientSecret.data(), clientSecret.position(), clientSecret.length());
	}

    /**
     * Gets the clientSecret, also known as the Service Account password, used to authenticate with LDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
     * 
     * @return clientSecret.
     */		
	public Buffer clientSecret()
	{
		return _clientSecret;
	}
	
	/**
     * Sets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins
     * 
     * @param clientJWK the client JWK
     */			
	public void clientJWK(Buffer clientJWK)
	{
		assert(clientJWK != null) : "clientSecret can not be null";
		clientJWK().data(clientJWK.data(), clientJWK.position(), clientJWK.length());
	}
	
	/**
     * Gets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins
     * 
     * @return clientJWK.
     */		
	public Buffer clientJWK()
	{
		return _clientJWK;
	}
	
	/**
     * Sets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     * 
     * @param audience the audience claim
     */			
	public void audience(Buffer audience)
	{
		assert(audience != null) : "clientSecret can not be null";
		audience().data(audience.data(), audience.position(), audience.length());
	}
	
	/**
     * Gets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     * 
     * @return audience.
     */		
	public Buffer audience()
	{
		return _audience;
	}
	
	
	/**
     * Sets the token scope to limit the scope of generated token from the token service. Optional.
     * 
     * @param tokenScope the token scope
     */			
	public void tokenScope(Buffer tokenScope)
	{
		assert(tokenScope != null) : "tokenScope can not be null";
		tokenScope().data(tokenScope.data(), tokenScope.position(), tokenScope.length());
	}

    /**
     * Gets the token scope to limit the scope of generated token from the token service. Optional.
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
     * Gets the Port Number of the HTTP proxy server.
     * 
     * @return the proxy port
     */		
	public Buffer proxyPort()
	{
		return _proxyPort;
	}
	
	/**
     * Sets the proxy user name to authenticate.
     * 
     * @param proxyUserName the proxy user name.
     */	
	public void proxyUserName(Buffer proxyUserName)
	{
		assert(proxyUserName != null) : "proxyUserName can not be null";
		proxyUserName().data(proxyUserName.data(), proxyUserName.position(), proxyUserName.length());
	}
	
    /**
     * Gets the proxy user name to authenticate.
     * 
     * @return the proxy user name.
     */		
	public Buffer proxyUserName()
	{
		return _proxyUserName;
	}
	
	/**
     * Sets the proxy password to authenticate.
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
     * Gets the proxy domain of the user to authenticate.
     * 
     * @return the proxy domain.
     */		
	public Buffer proxyDomain()
	{
		return _proxyDomain;
	}
	
	/**
     * Sets the local hostname of the user to authenticate. 
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
     * Sets the complete path of the Keberos5 configuration file.
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
     * Gets the complete path of the Keberos5 configuration file.
     * 
     * @return the Keberos5 configuration file.
     */		
	public Buffer proxyKRB5ConfigFile()
	{
		return _proxyKrb5ConfigFile;
	}
	
    /**
     * Sets the Reactor Service Endpoint Event Callback.
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
     * Gets the user specified object.
     * 
     * @return the ReactorServiceEndpointEventCallback
     */
    public ReactorServiceEndpointEventCallback reactorServiceEndpointEventCallback()
    {
    	return _reactorServiceEndpointEventCallback;
    }	
	
    /**
     * Sets the user defined object.
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
     * Gets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
     * 
     * @return - true to force sign-out using the same credential otherwise false.
     */
    public boolean takeExclusiveSignOnControl()
    {
    	return _takeExclusiveSignOnControl;
    }
    
    /**
     * Sets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     */
    public void takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl)
    {
    	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
    }
}


