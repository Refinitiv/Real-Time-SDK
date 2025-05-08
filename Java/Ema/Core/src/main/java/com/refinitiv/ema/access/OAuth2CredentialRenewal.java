///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2022, 2025 LSEG. All rights reserved.
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
     * Sets the user name required to authorize with the LDP token service. Mandatory for V1 oAuth Password Credentials logins
     * 
     * @param userName the userName for this request.
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal userName(String userName);
    
    /**
     * Sets the password for user name used to get an access token and a refresh token. Mandatory, used for V1 oAuth Password Credential logins.
     * If the password has changed, this will be the previous password.
     * 
     * @param password the password associated with the user name
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal password(String password);
    
    /**
     *  Sets the new Password.  This is only used for V1 oAuth Password Credentials only if the password has changed since the last login attempt. /p
	 *	If the password has changed, the previous password should be specified with OAuth2CredentialRenewal::password, and the 
	 *	new password should be set with this method.
	 *
     * @param newPassword the new password associated with the user name
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal newPassword(String newPassword);
    
    /**
     * Sets the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
     *
     * @param clientId the unique identifier for the application
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal clientId(String clientId);
    
    /**
     * Sets the clientSecret, also known as the Service Account password, used to authenticate with LDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
     *
     * @param clientSecret the client secret
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal clientSecret(String clientSecret);
    
    /**
     * Sets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the LDP token service. Mandatory for V2 logins with client JWT logins
     *
     * @param clientJwk the client JWK string, encoded in  JSON format.
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal clientJWK(String clientJwk);
    
    /**
     * Sets the token scope to limit the scope of generated token. Optional.
     *
     * @param tokenScope the token scope
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal tokenScope(String tokenScope);
    
    /**
     * Specifies the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
     *
     * @param audience the audience string
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal audience(String audience);
    
    /**
     * Sets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     * 
     * @return - reference to this object
     */
    public OAuth2CredentialRenewal takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl);
    
}
