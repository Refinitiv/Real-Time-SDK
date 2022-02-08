/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2021 Refinitiv. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __refinitiv_ema_access_RestLoggingCallbackClient_h
#define __refinitiv_ema_access_RestLoggingCallbackClient_h

#include "OmmRestLoggingClient.h"
#include "OmmConsumerRestLoggingEvent.h"

/*
* EMA internal callback client.
* Supports to invoke an user callback implementation based on OmmRestLoggingClient.
* User should derived from OmmRestLoggingClient base class.
*/

namespace refinitiv {

namespace ema {

namespace access {

class OmmBaseImpl;

class RestLoggingCallbackClient
{
public:
	RestLoggingCallbackClient(OmmRestLoggingClient* pOmmRestLoggingClient, void* closure);

	virtual ~RestLoggingCallbackClient();

	void onRestLoggingEvent(const OmmConsumerRestLoggingEvent& ommLogRestEvent);

	void* getRestLoggingClosure() const;

private:
	RestLoggingCallbackClient();
	RestLoggingCallbackClient(const RestLoggingCallbackClient&);
	RestLoggingCallbackClient& operator=(const RestLoggingCallbackClient&);

	OmmRestLoggingClient* _pOmmRestLoggingClient;
	void* _pRestLoggingClosure;
};  // class RestLoggingCallbackClient

}

}

}

#endif  // __refinitiv_ema_access_RestLoggingCallbackClient_h
