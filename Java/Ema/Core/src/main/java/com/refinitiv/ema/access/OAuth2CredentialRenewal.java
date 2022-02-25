///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * This class represents the OAuth credential renewal information.
 * It is to be used with Consumer.
 * 
**/

public interface OAuth2CredentialRenewal {
	
	/**
	 * Clears the OAuth2CredentialRenewal and sets all the defaults.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public OAuth2CredentialRenewal clear();
	
	/**
     * Sets the user name that will be used when sending the authorization request.  Mandatory for V1 oAuthPasswordGrant.
     * 
     * @param userName the userName for this request.
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal userName(String userName);
    
    /**
     * Sets password that will be used when sending the authorization request. Mandatory for V1 oAuthPasswordGrant. 
     * If the password has changed, this will be the previous password.
     * 
     * @param password the password associated with the user name
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal password(String password);
    
    /**
     * Sets password to authorize with the token service. Mandatory for V1 oAuthPasswordGrant
     *
     * @param newPassword the new password associated with the user name
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal newPassword(String newPassword);
    
    /**
     * Sets unique identifier defined for the application or user making a request to the token service. Mandatory for V1 oAuthPasswordGrant and V2 oAuthClientCred
     *
     * @param clientId the unique identifier for the application
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal clientId(String clientId);
    
    /**
     * Sets client secret to authorize with the token service. Required for V2 oAuthClientCred
     *
     * @param clientSecret the client secret
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal clientSecret(String clientSecret);
    
    /**
     * Sets token scope to limit the scope of generated token. Optional
     *
     * @param tokenScope the token scope
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal tokenScope(String tokenScope);
    
    /**
     * Sets the exclusive sign on control to force sign-out of other applications using the same credentials. Not used with V2.
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl);
    
}
