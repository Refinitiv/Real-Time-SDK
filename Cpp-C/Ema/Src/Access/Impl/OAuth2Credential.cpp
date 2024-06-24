/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#include "OAuth2Credential.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal */
OAuth2Credential::OAuth2Credential() :
	_userName(),
	_password(),
	_clientId(),
	_clientSecret(),
	_clientJWK(),
	_audience(),
	_tokenScope(),
	_channelList()
{
	_takeExclusiveSignOnControl = true;
}

/** Clear out all contained EmaString by zeroing out the memory, then free everything. */
OAuth2Credential::~OAuth2Credential()
{
	clear();
}

OAuth2Credential::OAuth2Credential(const OAuth2Credential& credentials) :
	_userName(credentials._userName),
	_password(credentials._password),
	_clientId(credentials._clientId),
	_clientSecret(credentials._clientSecret),
	_clientJWK(credentials._clientJWK),
	_audience(credentials._audience),
	_tokenScope(credentials._tokenScope),
	_channelList(credentials._channelList)
{
	_takeExclusiveSignOnControl = credentials._takeExclusiveSignOnControl;
}

/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object */
OAuth2Credential& OAuth2Credential::clear()
{
	_userName.secureClear();
	_password.secureClear();
	_clientId.secureClear();
	_clientSecret.secureClear();
	_clientJWK.secureClear();
	_audience.secureClear();
	_tokenScope.secureClear();
	_channelList.secureClear();
	_takeExclusiveSignOnControl = true;
	return *this;
}

/** Specifies the user name. This is only used for login V1
	
	@param[in] userName specifies the user name for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::userName(const EmaString& userName )
{
	_userName = userName;
	return *this;
}

/** Specifies the password. This is only used for login V1
	
	@param[in] password specifies the password for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::password(const EmaString& password )
{
	_password = password;
	return *this;
}

/** Specifies the clientId.  This is used for login V1 and login V2
	
	@param[in] clientId specifies the clientId  for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::clientId(const EmaString& clientId )
{
	_clientId = clientId;
	return *this;
}

/** Specifies the clientSecret.  This is used for login V2
	
	@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::clientSecret(const EmaString& clientSecret )
{
	_clientSecret = clientSecret;
	return *this;
}

/** Specifies the client JWK.  This is used for login V2

	@param[in] clientJWK specifies the client JWK for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::clientJWK(const EmaString& clientJWK)
{
	_clientJWK = clientJWK;
	return *this;
}

/** Specifies the audience claim for the JWT usage.  This is used for login V2

	@param[in] clientJWK specifies the client JWK for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::audience(const EmaString& audience)
{
	_audience = audience;
	return *this;
}

/** Specifies the tokenScope.  This is optionally used for login V1 and login V2
	
	@param[in] tokenScope specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::tokenScope(const EmaString& tokenScope )
{
	_tokenScope = tokenScope;
	return *this;
}

/** Specifies the channelList.  This is used for both login V1 and login V2

	@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::channelList(const EmaString& channelList)
{
	_channelList = channelList;
	return *this;
}

/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
	
	@param[in] takeExclusiveSignOnControl specifies the takeExclusiveSignOnControl for oAuth2 interactions.
	@return reference to this object
*/
OAuth2Credential& OAuth2Credential::takeExclusiveSignOnControl( bool takeExclusiveSignOnControl )
{
	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	return *this;
}

/** Gets the user name. This is only used for login V1

@return user name
*/
const EmaString& OAuth2Credential::getUserName()
{
	return _userName;
}

/** Gets the password. This is only used for login V1

@return password
*/
const EmaString& OAuth2Credential::getPassword()
{
	return _password;
}

/** Gets the clientId.  This is used for login V1 and login V2

@return client id
*/
const EmaString& OAuth2Credential::getClientId()
{
	return _clientId;
}

/** Gets the clientSecret.  This is used for login V2

@return client secret
*/
const EmaString& OAuth2Credential::getClientSecret()
{
	return _clientSecret;
}

/** Gets the client JWK.  This is used for login V2

	@return client WJK
*/
const EmaString& OAuth2Credential::getClientJWK()
{
	return _clientJWK;
}

/** Gets the audience.  This is used for login V2

	@return audience
*/
const EmaString& OAuth2Credential::getAudience()
{
	return _audience;
}

/** Gets the tokenScope.  This is optionally used for login V1 and login V2

@return token scope
*/
const EmaString& OAuth2Credential::getTokenScope()
{
	return _tokenScope;
}

/** Specifies the connections associated with this credential set.  This is a comma separated string with the name of the connections.
		If this is blank, then these credentials will apply to all channels that have session management enabled.

	@param[in] connectionList specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
	*/
const EmaString& OAuth2Credential::getChannelList()
{
	return _channelList;
}

/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.

@return takeExclusiveSignOnControl value
*/
const bool OAuth2Credential::getTakeExclusiveSignOnControl()
{
	return _takeExclusiveSignOnControl;
}