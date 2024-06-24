/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmOAuth2CredentialImpl_h
#define __refinitiv_ema_access_OmmOAuth2CredentialImpl_h

/**
	@class refinitiv::ema::access::OAuth2Credential OAuth2Credential.h "Access/Include/OAuth2Credential.h"
	@brief OmmOAuth2CredentialImpl 

*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/OAuth2Credential.h"
#include "Access/Include/OmmOAuth2ConsumerClient.h"
#include "rtr/rsslReactor.h"


namespace refinitiv {

namespace ema {

namespace access {


class EMA_ACCESS_API OmmOAuth2CredentialImpl: public OAuth2Credential
{
public :

	///@name Constructor
	//@{
	/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OmmOAuth2CredentialImpl(OAuth2Credential&);
	//@}
	
	///@name Constructor
	//@{
	/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OmmOAuth2CredentialImpl(OAuth2Credential&, OmmOAuth2ConsumerClient&);
	//@}

	///@name Constructor
	//@{
	/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OmmOAuth2CredentialImpl(OAuth2Credential&, OmmOAuth2ConsumerClient&, void*);
	//@}

	///@name Copy Constructor
	//@{
	/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal
	*/
	OmmOAuth2CredentialImpl(const OmmOAuth2CredentialImpl&);
	//@}
	
	///@name Destructor
	//@{
	/** Clear out all contained EmaString by zeroing out the memory, then free everything.
	*/
	~OmmOAuth2CredentialImpl();
	//@}

	bool isOAuth2ClientSet();

	void oAuth2ArrayIndex(UInt8);

	UInt8 getOAuth2ArrayIndex();
	
	
	OmmOAuth2ConsumerClient& getClient();

	void* getClosure();



private :
	
	OmmOAuth2ConsumerClient& _client;
	bool _hasOAuth2Client;
	void* _closure;

	RsslReactorOAuthCredential _oAuthCredential;
	UInt8 _oAuthArrayIndex;
	
};

}

}

}

#endif // __refinitiv_ema_access_OAuth2Credential_h
