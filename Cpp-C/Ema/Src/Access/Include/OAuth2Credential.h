/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OAuth2Credential_h
#define __refinitiv_ema_access_OAuth2Credential_h

/**
	@class refinitiv::ema::access::OAuth2Credential OAuth2Credential.h "Access/Include/OAuth2Credential.h"
	@brief OAuth2Credential class is used with OmmConsumer::SubmitOAuthCredentialRenewal to supply EMA with changed credentials

*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"


namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class OAuth2Credential;


class EMA_ACCESS_API OAuth2Credential
{
public :

	///@name Constructor
	//@{
	/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OAuth2Credential();
	//@}
	
	///@name Destructor
	//@{
	/** Clear out all contained EmaString by zeroing out the memory, then free everything.
	*/
	~OAuth2Credential();
	//@}

	///@name Operations
	//@{
		
	/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object
	*/
	OAuth2Credential& clear();
	
	/** Specifies the user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins.
		
		@param[in] userName specifies the user name
		@return reference to this object
	*/
	OAuth2Credential& userName(const EmaString& userName );
	
	/** Specifies the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins 
		
		@param[in] password specifies the password 
		@return reference to this object
	*/
	OAuth2Credential& password(const EmaString& password );
	
	/**  Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		
		@param[in] clientId specifies the clientId
		@return reference to this object
	*/
	OAuth2Credential& clientId(const EmaString& clientId );
	
	/** Specifies the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		
		@param[in] clientSecret specifies the clientSecret
		@return reference to this object
	*/
	OAuth2Credential& clientSecret(const EmaString& clientSecret );

	/** Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 

		@param[in] clientJWK specifies the JWK formatted private key
		@return reference to this object
	*/
	OAuth2Credential& clientJWK(const EmaString& clientJWK);

	/** Specifies the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.

		@param[in] audience specifies the audience claim string
		@return reference to this object
	*/
	OAuth2Credential& audience(const EmaString& audience);
	
	/** Specifies the token scope to limit the scope of generated token from the token service. Optional.
		
		@param[in] clientSecret specifies the clientSecret
		@return reference to this object
	*/
	OAuth2Credential& tokenScope(const EmaString& tokenScope );
	
	/** Specifies the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
				
		@param[in] takeExclusiveSignOnControl specifies the takeExclusiveSignOnControl value
		@return reference to this object
	*/
	OAuth2Credential& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl );
	
	/** Specifies the connections associated with this credential set.  This is a comma separated string with the name of the connections.
		If this is blank, then these credentials will apply to all channels that have session management enabled that do not match with any other configured oAuth2 credentials.
	
		@param[in] connectionList specifies the list of connections associated with this set of credentials. 
		@return reference to this object
	*/
	OAuth2Credential& channelList(const EmaString& channelList );
	//@}
	
	///@name Accessors
	//@{
	/** Gets the user name required to authorize with the RDP token service. Mandatory for V1 oAuth Password Credentials logins.
	
	@return user name
	*/
	const EmaString& getUserName();
	
	/** Gets the password for user name used to get access token. Mandatory for V1 oAuth Password Credentials logins 
		
		@return password
	*/
	const EmaString& getPassword();
	
	/** Gets the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		
		@return client id
	*/
	const EmaString& getClientId();
	
	/** Gets the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		
		@return client secret
	*/
	const EmaString& getClientSecret();

	/** Gets the JWK formatted private key used to create the JWT.  The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 

		@return client WJK
	*/
	const EmaString& getClientJWK();

	/** Gets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.

		@return audience
	*/
	const EmaString& getAudience();
	
	/** Gets the token scope to limit the scope of generated token from the token service. Optional.
		
		@return token scope
	*/
	const EmaString& getTokenScope();

	/** Gets the connections associated with this credential set.  This is a comma separated string with the name of the connections.
		If this is blank, then these credentials will apply to all channels that have session management enabled that do not match with any other configured oAuth2 credentials.

		@return channe list
	*/
	const EmaString& getChannelList();
	
	/** Gets the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
		
		@return takeExclusiveSignOnControl value
	*/
	const bool getTakeExclusiveSignOnControl();
	
	//@}

private :
	friend class OmmOAuth2CredentialImpl;

	OAuth2Credential( const OAuth2Credential& );
	OAuth2Credential& operator=( const OAuth2Credential& );
	
	EmaString					_userName;					/*!< The user name to authorize with the RDP token service. This is used to get sensitive information
															 *   for the user name in the RsslReactorOAuthCredentialEventCallback. */
	EmaString					_password;					/*!< The password for user name used to get an access token and a refresh token. */
	
	EmaString					_clientId;					/*!< The clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.*/
	EmaString					_clientSecret;				/*!< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	EmaString					_tokenScope;					/*!< A user can optionally limit the scope of generated token. Optional. */

	EmaString					_clientJWK;
	EmaString					_audience;
	
	EmaString					_channelList;				/* Comma separated list of configured channel names that this set of credentials is associated with*/

	bool						_takeExclusiveSignOnControl;	/*!< The exclusive sign on control to force sign-out of other applications using the same credentials. Optional */
	
};

}

}

}

#endif // __refinitiv_ema_access_OAuth2Credential_h
