/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
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
	
	/** Specifies the user name. This is only used for login V1
		
		@param[in] userName specifies the user name for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& userName( EmaString& userName );
	
	/** Specifies the password. This is only used for login V1
		
		@param[in] password specifies the password for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& password( EmaString& password );
	
	/** Specifies the new newPassword.  This is only used for login V1 only if the password has changed since the last login attempt. /p
		If the password has changed, the previous password should be specified with OAuth2CredentialRenewal::password, and the 
		new password should be set with this function.
		
		@param[in] newPassword specifies the password for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& newPassword( EmaString& newPassword );
	
	/** Specifies the clientId.  This is used for login V1 and login V2
		
		@param[in] clientId specifies the clientId  for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientId( EmaString& clientId );
	
	/** Specifies the clientSecret.  This is used for login V2
		
		@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientSecret( EmaString& clientSecret );


	/** Specifies the clientJWK.  This is used for login V2

		@param[in] clientJWK specifies the client JWK for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& clientJWK(EmaString& clientJWK);
	
	/** Specifies the tokenScope.  This is optionally used for login V1 and login V2
		
		@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& tokenScope( EmaString& tokenScope );
	
	/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
		
		@param[in] takeExclusiveSignOnControl specifies the takeExclusiveSignOnControl for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2CredentialRenewal& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl );
	//@}
	
	///@name Accessors
	//@{
	/** Gets the user name. This is only used for login V1
	
	@return user name
	*/
	const EmaString& getUserName();
	
	/** Gets the password. This is only used for login V1
		
		@return password
	*/
	const EmaString& getPassword();
	
	/** Gets the new newPassword.  This is only used for login V1 only if the password has changed since the last login attempt.
		
		@return new password
	*/
	const EmaString& getNewPassword();
	
	/** Gets the clientId.  This is used for login V1 and login V2
		
		@return client id
	*/
	const EmaString& getClientId();
	
	/** Gets the clientSecret.  This is used for login V2
		
		@return client secret
	*/
	const EmaString& getClientSecret();
	
	/** Gets the clientJWK.  This is used for login V2

	@return client JWK
	*/
	const EmaString& getClientJWK();

	/** Gets the tokenScope.  This is optionally used for login V1 and login V2
		
		@return token scope
	*/
	const EmaString& getTokenScope();
	
	/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
		
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
	EmaString					_clientId;					/*!< A unique ID defined for an application marking the request. Optional */
	EmaString					_clientSecret;				/*!< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	EmaString					_clientJWK;					/*!< The JWK used for V2 JWT authentication. Required for V2 JWT interactions. */
	EmaString					_tokenScope;					/*!< A user can optionally limit the scope of generated token. Optional. */
	bool						_takeExclusiveSignOnControl;	/*!< The exclusive sign on control to force sign-out of other applications using the same credentials. Optional */
	
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerClient_h
