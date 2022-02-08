/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2021 Refinitiv. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "RestLoggingCallbackClient.h"

using namespace refinitiv::ema::access;

RestLoggingCallbackClient::RestLoggingCallbackClient(OmmRestLoggingClient* pOmmRestLoggingClient, void* closure) :
	_pOmmRestLoggingClient(pOmmRestLoggingClient),
	_pRestLoggingClosure(closure)
{
}

RestLoggingCallbackClient::~RestLoggingCallbackClient()
{
}

void RestLoggingCallbackClient::onRestLoggingEvent(const OmmConsumerRestLoggingEvent& ommLogRestEvent)
{
	if (_pOmmRestLoggingClient)
	{
		_pOmmRestLoggingClient->onRestLoggingEvent(ommLogRestEvent);
	}
	return;
}

void* RestLoggingCallbackClient::getRestLoggingClosure() const
{
	return _pRestLoggingClosure;
}
