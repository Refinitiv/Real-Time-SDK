package com.rtsdk.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;

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
		_tokenScope.data("trapi.streaming.pricing.read");
		_takeExclusiveSignOnControl = true;
		_oAuthCredentialEventCallback = null;
	}
	
	 /**
     * The user name that was used when sending the authorization request.
     * 
     * @return - User name buffer.
     */
    public Buffer userName()
    {
    	return _userName;
    }

    /**
     * Sets userName to authorize with the token service. Mandatory
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
     * The password that was used when sending the authorization request.
     * 
     * @return - Password buffer.
     */
    public Buffer password()
    {
    	return _password;
    }
    
    /**
     * Sets password to authorize with the token service. Mandatory
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
     * The unique identifier that was used when sending the authorization request.
     * 
     * @return - Client ID buffer.
     */
    public Buffer clientId()
    {
    	return _clientId;
    }
    
    /**
     * Sets unique identifier defined for the application or user making a request to the token service. Mandatory
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
     * The secret that was used by OAuth Client to authenticate with the token service. 
     * 
     * @return - Client Secret buffer.
     */
    public Buffer clientSecret()
    {
    	return _clientSecret;
    }
    
    /**
     * Sets client secret to authorize with the token service. Optional
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
     * The token scope that was used to limit the scope of generated token. 
     * 
     * @return - Token Scope buffer.
     */
    public Buffer tokenScope()
    {
    	return _tokenScope;
    }
    
    /**
     * Sets token scope to limit the scope of generated token. Optional
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
