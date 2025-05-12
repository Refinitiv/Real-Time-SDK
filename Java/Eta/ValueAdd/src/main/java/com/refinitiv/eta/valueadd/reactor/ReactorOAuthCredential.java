/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

/**
 * This class represents the OAuth credential for authorization with the token service.
 * 
 * @see ConsumerRole
 */
public class ReactorOAuthCredential
{
	private Buffer _userName = CodecFactory.createBuffer();
	private Buffer _password = CodecFactory.createBuffer();
	private Buffer _clientId = CodecFactory.createBuffer();
	private Buffer _clientSecret = CodecFactory.createBuffer();
	private Buffer _audience = CodecFactory.createBuffer();
	private Buffer _clientJwk = CodecFactory.createBuffer();
	private Buffer _tokenScope = CodecFactory.createBuffer();
	private boolean	_takeExclusiveSignOnControl = true;
	private ReactorOAuthCredentialEventCallback _oAuthCredentialEventCallback;
	Object _userSpecObj = null;
	
	/**
     * Instantiates ReactorOAuthCredential.
     */
	ReactorOAuthCredential()
	{
		clear();
	}
	
	 /**
     * Clears to defaults
     */
	public void clear()
	{
		_userName.clear();
		_password.clear();
		_clientId.clear();
		_clientSecret.clear();
		_audience.clear();
		_clientJwk.clear();
		_tokenScope.data("trapi.streaming.pricing.read");
		_takeExclusiveSignOnControl = true;
		_oAuthCredentialEventCallback = null;
	}
	
	 /**
     * Gets user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins
     * 
     * @return - User name buffer.
     */
    public Buffer userName()
    {
    	return _userName;
    }

    /**
     * Sets the user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins
     *
     * @param userName the user name
     */
    public void userName(Buffer userName)
    {
    	assert(userName != null) : "userName can not be null";
    	_userName.data(userName.data(), userName.position(),
    			userName.length());
    }
    
    /**
     * Gets the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins
     * 
     * @return - Password buffer.
     */
    public Buffer password()
    {
    	return _password;
    }
    
    /**
     * Sets the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins
     *
     * @param password the password associated with the user name
     */
    public void password(Buffer password)
    {
    	assert(password != null) : "password can not be null";
    	_password.data(password.data(), password.position(),
    			password.length());
    }
    
    /**
     * Gets the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
     * 
     * @return - Client ID buffer.
     */
    public Buffer clientId()
    {
    	return _clientId;
    }
    
    /**
     * Sets the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
     *
     * @param clientId the unique identifier for the application
     */
    public void clientId(Buffer clientId)
    {
    	assert(clientId != null) : "clientId can not be null";
    	_clientId.data(clientId.data(), clientId.position(),
    			clientId.length());	
    }
    
    /**
     * Gets the clientSecret, also known as the Service Account password, used to authenticate with LDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
     * 
     * @return - Client Secret buffer.
     */
    public Buffer clientSecret()
    {
    	return _clientSecret;
    }
    
    
    /**
     * Sets the clientSecret, also known as the Service Account password, used to authenticate with LDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
     *
     * @param clientSecret the client secret
     */
    public void clientSecret(Buffer clientSecret)
    {
    	assert(clientSecret != null) : "clientSecret can not be null";
    	_clientSecret.data(clientSecret.data(), clientSecret.position(),
    			clientSecret.length());
    }
    
    /**
     * JWK formatted private key used to create the JWT. The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins.
     * 
     * @return - Client JWK buffer.
     */
    public Buffer clientJwk()
    {
    	return _clientJwk;
    }
    
    /**
     * Sets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins.
     *
     * @param clientJwk the JWK formatted private key 
     */
    public void clientJwk(Buffer clientJwk)
    {
    	assert(clientJwk != null) : "clientJwk can not be null";
    	_clientJwk.data(clientJwk.data(), clientJwk.position(),
    			clientJwk.length());
    }
    
    /**
     * Gets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     * 
     * @return - Client Secret buffer.
     */
    public Buffer audience()
    {
    	return _audience;
    }
    
    /**
     * Sets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     *
     * @param audience the audience claim
     */
    public void audience(Buffer audience)
    {
    	assert(audience != null) : "audience can not be null";
    	_audience.data(audience.data(), audience.position(),
    			audience.length());
    }
    
    /**
     * Gets the token scope to limit the scope of generated token from the token service. Optional.
     * 
     * @return - Token Scope buffer.
     */
    public Buffer tokenScope()
    {
    	return _tokenScope;
    }
    
    /**
     * Sets the token scope to limit the scope of generated token from the token service. Optional.
     *
     * @param tokenScope the token scope
     */
    public void tokenScope(Buffer tokenScope)
    {
    	assert(tokenScope != null) : "tokenScope can not be null";
    	_tokenScope.data(tokenScope.data(), tokenScope.position(),
    			tokenScope.length());
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
    
    /**
     * Specifies the {@link ReactorOAuthCredentialEventCallback} to submit OAuth sensitive information.
     * 
     * <p>The Reactor will not copy password and client secret if the callback is specified.</p>
     * 
     * @param oAuthCredentialEventCallback the OAuth credential event callback.
     */
    public void reactorOAuthCredentialEventCallback(ReactorOAuthCredentialEventCallback oAuthCredentialEventCallback)
    {
    	_oAuthCredentialEventCallback = oAuthCredentialEventCallback;
    }
    
    /**
     * The callback to submit OAuth sensitive information.
     * 
     * @return The {@link ReactorOAuthCredentialEventCallback}.
     */
    public ReactorOAuthCredentialEventCallback reactorOAuthCredentialEventCallback()
    {
    	return _oAuthCredentialEventCallback;
    }
    
    /**
     * Specifies a user defined object that can be useful for retrieving sensitive information 
     * via in the {@link ReactorOAuthCredentialEventCallback}.
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

        _userSpecObj = userSpecObj;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the userSpecObj.
     * 
     * @return the userSpecObj.
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    
    /**
     * Performs a deep copy of {@link ReactorOAuthCredential} object.
     * 
     * @param destReactorOAuthCredential to copy OAuth credential object into. It
     *            cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(ReactorOAuthCredential destReactorOAuthCredential)
    {
    	assert (destReactorOAuthCredential != null) : "destReactorOAuthCredential must be non-null";
    	
    	if(destReactorOAuthCredential == null)
    		return CodecReturnCodes.FAILURE;
    	
    	if(_userName.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_userName.length());
    		_userName.copy(byteBuffer);
    		destReactorOAuthCredential.userName().data(byteBuffer);
    	}
    	
    	if(_password.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_password.length());
    		_password.copy(byteBuffer);
    		destReactorOAuthCredential.password().data(byteBuffer);
    	}
    	
    	if(_clientId.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientId.length());
    		_clientId.copy(byteBuffer);
    		destReactorOAuthCredential.clientId().data(byteBuffer);
    	}
    	
    	if(_clientSecret.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientSecret.length());
    		_clientSecret.copy(byteBuffer);
    		destReactorOAuthCredential.clientSecret().data(byteBuffer);
    	}
    	
    	if(_clientJwk.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientJwk.length());
    		_clientJwk.copy(byteBuffer);
    		destReactorOAuthCredential.clientJwk().data(byteBuffer);
    	}
    	
    	if(_audience.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_audience.length());
    		_audience.copy(byteBuffer);
    		destReactorOAuthCredential.audience().data(byteBuffer);
    	}
    	
    	if(_tokenScope.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_tokenScope.length());
    		_tokenScope.copy(byteBuffer);
    		destReactorOAuthCredential.tokenScope().data(byteBuffer);
    	}
    	
    	destReactorOAuthCredential._takeExclusiveSignOnControl = _takeExclusiveSignOnControl;
    	destReactorOAuthCredential._oAuthCredentialEventCallback = _oAuthCredentialEventCallback;
    	destReactorOAuthCredential._userSpecObj = _userSpecObj;
    	
    	return CodecReturnCodes.SUCCESS;
    }
}
