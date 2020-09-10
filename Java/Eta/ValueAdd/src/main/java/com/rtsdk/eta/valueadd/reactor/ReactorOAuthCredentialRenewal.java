package com.rtsdk.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;

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
		_tokenScope.clear();
		_takeExclusiveSignOnControl = true;
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
    	userName.copy(_userName);
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
    	password.copy(_password);
    }
    
    /**
     * The password that was used when sending the authorization request.
     * 
     * @return - Password buffer.
     */
    public Buffer newPassword()
    {
    	return _newPassword;
    }
    
    /**
     * Sets password to authorize with the token service. Mandatory
     *
     * @param newPassword the password associated with the user name
     */
    public void newPassword(Buffer newPassword)
    {
    	newPassword.copy(_newPassword);
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
    	clientId.copy(_clientId);
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
    	clientSecret.copy(_clientSecret);
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
    	tokenScope.copy(_tokenScope);
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
