/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmLoginCredentialConsumerClient_h
#define __refinitiv_ema_access_OmmLoginCredentialConsumerClient_h

/**
	@class refinitiv::ema::access::OmmLoginCredentialConsumerClient OmmLoginCredentialConsumerClient.h "Access/Include/OmmLoginCredentialConsumerClient.h"
	@brief OmmLoginCredentialConsumerClient class provides callback interfaces for OAuth interactions.

	Application may implement an application client class inheriting from OmmLoginCredentialConsumerClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by OmmLoginCredentialConsumerClient class.

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	The following code snippet shows basic usage of OmmLoginCredentialConsumerClient class to print recevied messages to screen.

	\code

	class AppClient : public OmmLoginCredentialConsumerClient
	{
		void onLoginCredentialRenewal( const OmmConsumerEvent& );
	};

	void AppClient::onLoginCredentialRenewal( const OmmConsumerEvent& event ) 
	{
		//Retrieve the credentials, then call OmmConsumer::SubmitOAuthCredentialRenewal to submit the renewed credentials. 

		LoginMsgCredentialRenewal credentials;
		// Need the OmmConsumer object to call SubmitOAuthCredentialRenewal
		OmmConsumer& consumer = event.getClosure().getConsumer();

		credentials.userName("<USER NAME>");

		consumer.renewLoginCredentials(credentials);
	}



	\endcode

*/

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmConsumerEvent;
class OmmLoginCredentialConsumerClient;


class EMA_ACCESS_API OmmLoginCredentialConsumerClient
{
public :

	///@name Callbacks
	//@{
	/** Invoked upon receiving a credentials request for Login credential renewal interactions.
		
		@param[out] ommConsumer The ommConsumer associated with this callback.  The user must call OmmConsumer::renewLoginCredentials 
					in this function, even if the credentials have not changed.
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onLoginCredentialRenewal( const OmmConsumerEvent& consumerEvent );


	//@}

protected :

	OmmLoginCredentialConsumerClient();
	virtual ~OmmLoginCredentialConsumerClient();

private :

	OmmLoginCredentialConsumerClient( const OmmLoginCredentialConsumerClient& );
	OmmLoginCredentialConsumerClient& operator=( const OmmLoginCredentialConsumerClient& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerClient_h
