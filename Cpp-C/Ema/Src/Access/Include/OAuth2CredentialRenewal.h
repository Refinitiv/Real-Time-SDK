/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OAuth2CredentialRenewal_h
#define __refinitiv_ema_access_OAuth2CredentialRenewal_h

/**
	@class refinitiv::ema::access::OAuth2CredentialRenewal OAuth2CredentialRenewal.h "Access/Include/OAuth2CredentialRenewal.h"
	@brief OAuth2CredentialRenewal class is used with OmmConsumer::SubmitOAuthCredentialRenewal to supply EMA with changed credentials

*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"


namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class OAuth2CredentialRenewal;


class EMA_ACCESS_API OAuth2CredentialRenewal
{
public :

	///@name Constructor
	//@{
	/** Create an OAuth2CredentialRenewal for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OAuth2CredentialRenewal();
	//@}
	
	///@name Destructor
	//@{
	/** Clear out all contained EmaString by zeroing out the memory, then free everything.
	*/
	~OAuth2CredentialRenewal();
	//@}

	///@name Operations
	//@{
		
	/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clear();
	
	/** Specifies the user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins
		
		@param[in] userName the user name
		@return reference to this object
	*/
	OAuth2CredentialRenewal& userName( EmaString& userName );
	
	/** Specifies the password for user name used to get an access token and a refresh token. Mandatory, used for V1 oAuth Password Credential logins.
		If the password has changed, this will be the previous password.
		
		@param[in] password the password
		@return reference to this object
	*/
	OAuth2CredentialRenewal& password( EmaString& password );
	
	/** Specifies the new Password.  This is only used for V1 oAuth Password Credentials only if the password has changed since the last login attempt. /p
		If the password has changed, the previous password should be specified with OAuth2CredentialRenewal::password, and the 
		new password should be set with this function.
		
		@param[in] newPassword the password
		@return reference to this object
	*/
	OAuth2CredentialRenewal& newPassword( EmaString& newPassword );
	
	/** Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		
		@param[in] clientId the clientId
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientId( EmaString& clientId );
	
	/** Specifies the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		
		@param[in] clientSecret specifies the clientSecret
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientSecret( EmaString& clientSecret );


	/** Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 

		@param[in] clientJWK the client JWK
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientJWK(EmaString& clientJWK);
	
	/** Specifies the token scope to limit the scope of generated token from the token service. Optional.
		
		@param[in] clientSecret the clientSecret.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& tokenScope( EmaString& tokenScope );
	
	/** Specifies the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
		
		@param[in] takeExclusiveSignOnControl the takeExclusiveSignOnControl value
		@return reference to this object
	*/
	OAuth2CredentialRenewal& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl );
	//@}
	
	///@name Accessors
	//@{
	/**  Gets the user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins
	
	@return user name
	*/
	const EmaString& getUserName();
	
	/** Gets the password for user name used to get an access token and a refresh token. Mandatory, used for V1 oAuth Password Credential logins.
		
		@return password
	*/
	const EmaString& getPassword();
	
	/** Gets the newPassword.  This is only used for V1 oAuth Password Credentials only if the password has changed since the last login attempt. /p
		If the password has changed, the previous password should be specified with OAuth2CredentialRenewal::password, and the 
		new password should be set with this function.
		
		@return new password
	*/
	const EmaString& getNewPassword();
	
	/** Gets the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		
		@return client id
	*/
	const EmaString& getClientId();
	
	/** Gets the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		
		@return client secret
	*/
	const EmaString& getClientSecret();
	
	/** Gets the JWK formatted private key used to create the JWT.  The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 

	@return client JWK
	*/
	const EmaString& getClientJWK();

	/** Specifies the token scope to limit the scope of generated token from the token service. Optional.
		
		@return token scope
	*/
	const EmaString& getTokenScope();
	
	/** Gets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.

		@return takeExclusiveSignOnControl value
	*/
	const bool getTakeExclusiveSignOnControl();
	
	//@}

private :

	OAuth2CredentialRenewal( const OAuth2CredentialRenewal& );
	OAuth2CredentialRenewal& operator=( const OAuth2CredentialRenewal& );
	
	EmaString					_userName;					/*!< The user name to authorize with the RDP token service. This is used to get sensitive information
															 *   for the user name in the RsslReactorOAuthCredentialEventCallback. */
	EmaString					_password;					/*!< The password for user name used to get an access token and a refresh token. */
	EmaString					_newPassword;				/*!< The new password to change the password associated with this user name.
															 *   Both current and new passwords will be required in order to authenticate and change password. Optional.*/
	EmaString					_clientId;					/*!< The clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins. */
	EmaString					_clientSecret;				/*!< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	EmaString					_clientJWK;					/*!< The JWK used for V2 JWT authentication. Required for V2 JWT interactions. */
	EmaString					_tokenScope;					/*!< A user can optionally limit the scope of generated token. Optional. */
	bool						_takeExclusiveSignOnControl;	/*!< The exclusive sign on control to force sign-out of other applications using the same credentials. Optional */
	
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerClient_h
