/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#include "OAuth2CredentialRenewal.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

/** Create an OAuth2CredentialRenewal for use with OmmConsumer::SubmitOAuthCredentialRenewal */
OAuth2CredentialRenewal::OAuth2CredentialRenewal() :
	_userName(),
	_password(),
	_newPassword(),
	_clientId(),
	_clientSecret(),
	_clientJWK(),
	_tokenScope()
{
	_takeExclusiveSignOnControl = false;
}

/** Clear out all contained EmaString by zeroing out the memory, then free everything. */
OAuth2CredentialRenewal::~OAuth2CredentialRenewal()
{
	clear();
}

/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object */
OAuth2CredentialRenewal& OAuth2CredentialRenewal::clear()
{
	_userName.secureClear();
	_password.secureClear();
	_newPassword.secureClear();
	_clientId.secureClear();
	_clientSecret.secureClear();
	_clientJWK.secureClear();
	_tokenScope.secureClear();
	_takeExclusiveSignOnControl = false;
	return *this;
}

/** Specifies the user name. This is only used for login V1
	
	@param[in] userName specifies the user name for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::userName( EmaString& userName )
{
	_userName = userName;
	return *this;
}

/** Specifies the password. This is only used for login V1
	
	@param[in] password specifies the password for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::password( EmaString& password )
{
	_password = password;
	return *this;
}

/** Specifies the new newPassword.  This is only used for login V1 only if the password has changed since the last login attempt. /p
	If the password has changed, the previous password should be specified with OAuth2CredentialRenewal::password, and the 
	new password should be set in 
	
	@param[in] newPassword specifies the password for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::newPassword( EmaString& newPassword )
{
	_newPassword = newPassword;
	return *this;
}

/** Specifies the clientId.  This is used for login V1 and login V2
	
	@param[in] clientId specifies the clientId  for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::clientId( EmaString& clientId )
{
	_clientId = clientId;
	return *this;
}

/** Specifies the clientSecret.  This is used for login V2
	
	@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::clientSecret( EmaString& clientSecret )
{
	_clientSecret = clientSecret;
	return *this;
}

/** Specifies the client JWK.  This is used for login V2

	@param[in] clientJWK specifies the client JWK for oAuth2 V2 JWT interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::clientJWK(EmaString& clientJWK)
{
	_clientJWK = clientJWK;
	return *this;
}

/** Specifies the tokenScope.  This is optionally used for login V1 and login V2
	
	@param[in] clientSecret specifies the clientSecret for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::tokenScope( EmaString& tokenScope )
{
	_tokenScope = tokenScope;
	return *this;
}

/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.
	
	@param[in] takeExclusiveSignOnControl specifies the takeExclusiveSignOnControl for oAuth2 interactions.
	@return reference to this object
*/
OAuth2CredentialRenewal& OAuth2CredentialRenewal::takeExclusiveSignOnControl( bool takeExclusiveSignOnControl )
{
	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	return *this;
}

/** Gets the user name. This is only used for login V1

@return user name
*/
const EmaString& OAuth2CredentialRenewal::getUserName()
{
	return _userName;
}

/** Gets the password. This is only used for login V1

@return password
*/
const EmaString& OAuth2CredentialRenewal::getPassword()
{
	return _password;
}

/** Gets the new newPassword.  This is only used for login V1 only if the password has changed since the last login attempt.

@return new password
*/
const EmaString& OAuth2CredentialRenewal::getNewPassword()
{
	return _newPassword;
}

/** Gets the clientId.  This is used for login V1 and login V2

@return client id
*/
const EmaString& OAuth2CredentialRenewal::getClientId()
{
	return _clientId;
}

/** Gets the clientSecret.  This is used for login V2

@return client secret
*/
const EmaString& OAuth2CredentialRenewal::getClientSecret()
{
	return _clientSecret;
}


/** Gets the clientJWK.  This is used for login V2

@return client JWK
*/
const EmaString& OAuth2CredentialRenewal::getClientJWK()
{
	return _clientJWK;
}

/** Gets the tokenScope.  This is optionally used for login V1 and login V2

@return token scope
*/
const EmaString& OAuth2CredentialRenewal::getTokenScope()
{
	return _tokenScope;
}

/** Specifies the takeExclusiveSignOnControl feature.  This is optionally used for login V1.

@return takeExclusiveSignOnControl value
*/
const bool OAuth2CredentialRenewal::getTakeExclusiveSignOnControl()
{
	return _takeExclusiveSignOnControl;
}