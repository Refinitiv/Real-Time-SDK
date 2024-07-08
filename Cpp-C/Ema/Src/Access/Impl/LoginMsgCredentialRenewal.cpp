/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#include "LoginMsgCredentialRenewal.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

/** Create an LoginMsgCredentialRenewal for use with OmmConsumer::SubmitOAuthCredentialRenewal */
LoginMsgCredentialRenewal::LoginMsgCredentialRenewal() :
	_userName()
{
}

/** Clear out all contained EmaString by zeroing out the memory, then free everything. */
LoginMsgCredentialRenewal::~LoginMsgCredentialRenewal()
{
	clear();
}

/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object */
LoginMsgCredentialRenewal& LoginMsgCredentialRenewal::clear()
{
	_userName.secureClear();
	return *this;
}

/** Specifies the user name. This is only used for login V1
	
	@param[in] userName specifies the user name for oAuth2 interactions.
	@return reference to this object
*/
LoginMsgCredentialRenewal& LoginMsgCredentialRenewal::userName( EmaString& userName )
{
	_userName = userName;
	return *this;
}

LoginMsgCredentialRenewal& LoginMsgCredentialRenewal::authenticationExtended(EmaBuffer& authenticationExtended)
{
	_authenticationExtended = authenticationExtended;
	return *this;
}
/** Gets the user name.

@return user name
*/
const EmaString& LoginMsgCredentialRenewal::getUserName()
{
	return _userName;
}

const EmaBuffer& LoginMsgCredentialRenewal::getAuthenticationExtended()
{
	return _authenticationExtended;
}
