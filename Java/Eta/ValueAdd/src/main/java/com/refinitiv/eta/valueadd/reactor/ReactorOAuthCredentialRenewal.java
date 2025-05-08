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
import com.refinitiv.eta.codec.CodecReturnCodes;

/**
 * This class represents the OAuth credential renewal information.
 * 
**/
public class ReactorOAuthCredentialRenewal
{
	Buffer _userName = CodecFactory.createBuffer();
	Buffer _password = CodecFactory.createBuffer();
	Buffer _newPassword = CodecFactory.createBuffer();
	Buffer _clientId = CodecFactory.createBuffer();
	Buffer _clientSecret = CodecFactory.createBuffer();
	Buffer _audience = CodecFactory.createBuffer();
	Buffer _clientJwk = CodecFactory.createBuffer();
	Buffer _tokenScope = CodecFactory.createBuffer();
	private boolean	_takeExclusiveSignOnControl = true;

	ReactorOAuthCredentialRenewal()
	{
		clear();
	}
	
	/**
	 * Clears all values to default
	 */
	public void clear()
	{
		_userName.clear();
		_password.clear();
		_newPassword.clear();
		_clientId.clear();
		_clientSecret.clear();
		_audience.clear();
		_clientJwk.clear();
		_tokenScope.clear();
		_takeExclusiveSignOnControl = true;
	}
	
	 /**
     * Gets the user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins
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
    	userName.copy(_userName);
    }
    
    /**
     * Gets the password for user name used to get an access token and a refresh token. Mandatory, used for V1 oAuth Password Credential logins.
     * 
     * @return - Password buffer.
     */
    public Buffer password()
    {
    	return _password;
    }
    
    /**
     * Sets the password for user name used to get an access token and a refresh token. Mandatory, used for V1 oAuth Password Credential logins.
     *
     * @param password the password associated with the user name
     */
    public void password(Buffer password)
    {
    	password.copy(_password);
    }
    
    /**
     * Gets the newPassword.  This is only used for V1 oAuth Password Credentials only if the password has changed since the last login attempt. /p
	 *	If the password has changed, the previous password should be specified with ReactorOAuth2CredentialRenewal::password, and the 
	 *	new password should be set with this function.
     * 
     * @return - Password buffer.
     */
    public Buffer newPassword()
    {
    	return _newPassword;
    }
    
    /**
     * Sets the newPassword.  This is only used for V1 oAuth Password Credentials only if the password has changed since the last login attempt. /p
	 *	If the password has changed, the previous password should be specified with ReactorOAuth2CredentialRenewal::password, and the 
	 *	new password should be set with this function.
     *
     * @param newPassword the password associated with the user name
     */
    public void newPassword(Buffer newPassword)
    {
    	newPassword.copy(_newPassword);
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
    	clientId.copy(_clientId);
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
    	clientSecret.copy(_clientSecret);
    }
    
    /**
     * Gets the JWK formatted private key used to create the JWT.  The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins
     * 
     * @return - Client JWK buffer.
     */
    public Buffer clientJWK()
    {
    	return _clientJwk;
    }
    
    /**
     * Sets the JWK formatted private key used to create the JWT.  The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins
     *
     * @param clientJwk the client JWK
     */
    public void clientJWK(Buffer clientJwk)
    {
    	clientJwk.copy(_clientJwk);
    }
    
    
    /**
     * Gets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     * 
     * @return - audience buffer.
     */
    public Buffer audience()
    {
    	return _audience;
    }
    
    /**
     * Sets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     *
     * @param audience the audience claim.
     */
    public void audience(Buffer audience)
    {
    	audience.copy(_audience);
    }
    
    /**
     * Gets the scope of generated token. Optional.
     * 
     * @return - Token Scope buffer.
     */
    public Buffer tokenScope()
    {
    	return _tokenScope;
    }
    
    /**
     * Sets the scope of generated token. Optional.
     *
     * @param tokenScope the token scope
     */
    public void tokenScope(Buffer tokenScope)
    {
    	tokenScope.copy(_tokenScope);
    }
    
    /**
     * Gets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins
     * 
     * @return - true to force sign-out using the same credential otherwise false.
     */
    public boolean takeExclusiveSignOnControl()
    {
    	return _takeExclusiveSignOnControl;
    }
    
    /**
     * Sets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     */
    public void takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl)
    {
    	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
    }
    
    /**
     * Performs a deep copy of {@link ReactorOAuthCredentialRenewal} object.
     * 
     * @param destReactorOAuthCredentialRenewal to copy OAuth credential renewal object into. It
     *            cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(ReactorOAuthCredentialRenewal destReactorOAuthCredentialRenewal)
    {
    	assert (destReactorOAuthCredentialRenewal != null) : "destReactorOAuthCredentialRenewal must be non-null";
    	
    	if(destReactorOAuthCredentialRenewal == null)
    		return CodecReturnCodes.FAILURE;
    	
    	if(_userName.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_userName.length());
    		_userName.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.userName().data(byteBuffer);
    	}
    	
    	if(_password.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_password.length());
    		_password.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.password().data(byteBuffer);
    	}
    	
    	if(_newPassword.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_newPassword.length());
    		_newPassword.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.newPassword().data(byteBuffer);
    	}
    	
    	if(_clientId.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientId.length());
    		_clientId.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.clientId().data(byteBuffer);
    	}
    	
    	if(_clientSecret.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientSecret.length());
    		_clientSecret.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.clientSecret().data(byteBuffer);
    	}
    	
    	if(_clientJwk.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_clientJwk.length());
    		_clientJwk.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.clientJWK().data(byteBuffer);
    	}
    	
    	if(_tokenScope.length() != 0)
    	{
    		ByteBuffer byteBuffer = ByteBuffer.allocate(_tokenScope.length());
    		_tokenScope.copy(byteBuffer);
    		destReactorOAuthCredentialRenewal.tokenScope().data(byteBuffer);
    	}
    	
    	destReactorOAuthCredentialRenewal._takeExclusiveSignOnControl = _takeExclusiveSignOnControl;
    	
    	return CodecReturnCodes.SUCCESS;
    }
}
