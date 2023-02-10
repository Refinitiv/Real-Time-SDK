/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
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
	
	/** Specifies the user name. This is only used for login V1
		
		@param[in] userName specifies the user name for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& userName(const EmaString& userName );
	
	/** Specifies the password. This is only used for login V1
		
		@param[in] password specifies the password for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& password(const EmaString& password );
	
	/** Specifies the clientId.  This is used for login V1 and login V2
		
		@param[in] clientId specifies the clientId  for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& clientId(const EmaString& clientId );
	
	/** Specifies the clientSecret.  This is used for login V2
		
		@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& clientSecret(const EmaString& clientSecret );

	/** Specifies the client JWK.  This is used for login V2

		@param[in] clientJWK specifies the client JWK for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& clientJWK(const EmaString& clientJWK);

	/** Optionally specifies the audience claim for the JWT usage.  This is used for login V2

		@param[in] audience specifies the audience claim string for JWT oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& audience(const EmaString& audience);
	
	/** Specifies the tokenScope.  This is optionally used for login V1 and login V2
		
		@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& tokenScope(const EmaString& tokenScope );
	
	/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
		
		@param[in] takeExclusiveSignOnControl specifies the takeExclusiveSignOnControl for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl );
	
	/** Specifies the connections associated with this credential set.  This is a comma separated string with the name of the connections.
		If this is blank, then these credentials will apply to all channels that have session management enabled that do not match with any other configured oAuth2 credentials.
	
		@param[in] connectionList specifies the clientSecret for oAuth2 interactions.
		@return reference to this object
	*/
	OAuth2Credential& channelList(const EmaString& channelList );
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
	
	/** Gets the clientId.  This is used for login V1 and login V2
		
		@return client id
	*/
	const EmaString& getClientId();
	
	/** Gets the clientSecret.  This is used for login V2
		
		@return client secret
	*/
	const EmaString& getClientSecret();

	/** Gets the client JWK.  This is used for login V2

		@return client WJK
	*/
	const EmaString& getClientJWK();

	/** Gets the audience.  This is used for login V2

		@return audience
	*/
	const EmaString& getAudience();
	
	/** Gets the tokenScope.  This is optionally used for login V1 and login V2
		
		@return token scope
	*/
	const EmaString& getTokenScope();

	/** Gets the channelList.  This is optionally used for login V1 and login V2

		@return channe list
	*/
	const EmaString& getChannelList();
	
	/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
		
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
	
	EmaString					_clientId;					/*!< A unique ID defined for an application marking the request. Optional */
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
