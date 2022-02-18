/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2021 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmOAuth2ConsumerClient_h
#define __refinitiv_ema_access_OmmOAuth2ConsumerClient_h

/**
	@class refinitiv::ema::access::OmmOAuth2ConsumerClient OmmOAuth2ConsumerClient.h "Access/Include/OmmOAuth2ConsumerClient.h"
	@brief OmmOAuth2ConsumerClient class provides callback interfaces for OAuth interactions.

	Application may implement an application client class inheriting from OmmOAuth2ConsumerClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by OmmOAuth2ConsumerClient class.

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	The following code snippet shows basic usage of OmmOAuth2ConsumerClient class to print recevied messages to screen.

	\code

	class AppClient : public OmmOAuth2ConsumerClient
	{
		void onCredentialRenewal( const OmmConsumerEvent& );
	};

	void AppClient::onCredentialRenewal( const OmmConsumerEvent& event ) 
	{
		//Retrieve the credentials, then call OmmConsumer::SubmitOAuthCredentialRenewal to submit the renewed credentials. 

		OAuth2CredentialRenewal credentials;
		// Need the OmmConsumer object to call SubmitOAuthCredentialRenewal
		OmmConsumer& consumer = event.getClosure().getConsumer();

		credentials.clientId("<CLIENT ID>");
		credentials.clientSecret("<CLIENT_SECRET>");

		consumer.SubmitOAuthCredentialRenewal(credentials);
	}



	\endcode

*/

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmConsumerEvent;
class OmmOAuth2ConsumerClient;


class EMA_ACCESS_API OmmOAuth2ConsumerClient
{
public :

	///@name Callbacks
	//@{
	/** Invoked upon receiving a credentials request for OAuth interactions.
		
		@param[out] ommConsumer The ommConsumer associated with this callback.  The user will need to call OmmConsumer::renewOAuth2Credentials 
					in this function to provide the updated credentials
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onCredentialRenewal( const OmmConsumerEvent& consumerEvent );


	//@}

protected :

	OmmOAuth2ConsumerClient();
	virtual ~OmmOAuth2ConsumerClient();

private :

	OmmOAuth2ConsumerClient( const OmmOAuth2ConsumerClient& );
	OmmOAuth2ConsumerClient& operator=( const OmmOAuth2ConsumerClient& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerClient_h
