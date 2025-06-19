/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_LoginMsgCredentialRenewal_h
#define __refinitiv_ema_access_LoginMsgCredentialRenewal_h

/**
	@class refinitiv::ema::access::LoginMsgCredentialRenewal LoginMsgCredentialRenewal.h "Access/Include/LoginMsgCredentialRenewal.h"
	@brief LoginMsgCredentialRenewal class is used with OmmConsumer::SubmitOAuthCredentialRenewal to supply EMA with changed credentials

*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaBuffer.h"


namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class LoginMsgCredentialRenewal;


class EMA_ACCESS_API LoginMsgCredentialRenewal
{
public :

	///@name Constructor
	//@{
	/** Create an LoginMsgCredentialRenewal for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	LoginMsgCredentialRenewal();
	//@}
	
	///@name Destructor
	//@{
	/** Clear out all contained EmaString by zeroing out the memory, then free everything.
	*/
	~LoginMsgCredentialRenewal();
	//@}

	///@name Operations
	//@{
		
	/** Zeros out and clears all allocated EMAStrings in the class.
		
		@return reference to this object
	*/
	LoginMsgCredentialRenewal& clear();

	/** Specifies the user name that will be updated.

		@param[in] userName specifies the new user name for this login.
		@return reference to this object
	*/
	LoginMsgCredentialRenewal& userName(EmaString& userName);

	/** Specifies the extended authentication information that will be updated.

		@param[in] userName specifies the user name for oAuth2 interactions.
		@return reference to this object
	*/
	LoginMsgCredentialRenewal& authenticationExtended(EmaBuffer& authenticationExtended);
	
	//@}
	
	///@name Accessors
	//@{
	/** Gets the user name.
	
	@return user name
	*/
	const EmaString& getUserName();

	/** Gets the authentication extended buffer.

	@return authentication extended
	*/
	const EmaBuffer& getAuthenticationExtended();

	
	//@}

private :

	LoginMsgCredentialRenewal( const LoginMsgCredentialRenewal& );
	LoginMsgCredentialRenewal& operator=( const LoginMsgCredentialRenewal& );
	
	EmaString					_userName;					/*!< The updated user name for the new login request. */
	EmaBuffer					_authenticationExtended;
	
};

}

}

}

#endif // __refinitiv_ema_access_LoginMsgCredentialRenewal_h
