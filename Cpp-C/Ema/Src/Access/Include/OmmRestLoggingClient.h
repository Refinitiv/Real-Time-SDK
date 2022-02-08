/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2021 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __refinitiv_ema_access_OmmRestLoggingClient_h
#define __refinitiv_ema_access_OmmRestLoggingClient_h

 /**
	@class refinitiv::ema::access::OmmRestLoggingClient OmmRestLoggingClient.h "Access/Include/OmmRestLoggingClient.h"
	@brief OmmRestLoggingClient class provides callback mechanism to receive REST logging messages.

	By default OmmConsumer class setup to print all the REST debug messages to output stream when enabled.
	Specifying OmmRestLoggingClient in OmmConsumerConfig overwrites this behaviour.

	\remark Thread safety of all the methods in this class depends on user's implementation.

	The following code snippet shows basic usage of OmmRestLoggingClient class in a simple consumer type app.
	User declares new class as derived from class OmmRestLoggingClient.
	Then registers the callback client (instance of the derived class) in OmmConsumerConfig.

	\code

	// create an implementation for OmmRestLoggingClient to receive REST logging event that encapsulated REST logging messages
	class AppRestClient : public OmmRestLoggingClient
	{
		void onRestLoggingEvent( const OmmConsumerRestLoggingEvent& ommLogRestEvent );
	};

	AppRestClient restClient;

	// instantiate OmmConsumer object and set up the Rest callback client using OmmConsumerConfig
	OmmConsumer consumer( OmmConsumerConfig().restLoggingCallback( restClient ) );

	// User can provide into OmmConsumerConfig an optional closure that specified application defined identifier.
	OmmConsumer consumer( OmmConsumerConfig().restLoggingCallback( restClient, closure ) );

	\endcode

	@see OmmConsumer,
	     OmmConsumerConfig,
	     OmmConsumerRestLoggingEvent
 */

#include "Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmConsumerRestLoggingEvent;

class EMA_ACCESS_API OmmRestLoggingClient
{
public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving REST logging message. Requires setting up the OmmRestLoggingClient into OmmConsumerConfig.
		@param[out] ommLogRestEvent encapsulates REST logging message.
		@return void
	*/
	virtual void onRestLoggingEvent( const OmmConsumerRestLoggingEvent& ommLogRestEvent ) = 0;
	//@}

protected:
	///@name Constructor
	//@{
	OmmRestLoggingClient() {};
	//@}

	///@name Destructor
	//@{
	virtual ~OmmRestLoggingClient() {};
	//@}

private:

	OmmRestLoggingClient(const OmmRestLoggingClient&);
	OmmRestLoggingClient& operator=(const OmmRestLoggingClient&);

};

}

}

}

#endif  // __refinitiv_ema_access_OmmRestLoggingClient_h
