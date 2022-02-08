/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2021 Refinitiv. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerRestLoggingEvent.h"

using namespace refinitiv::ema::access;

OmmConsumerRestLoggingEvent::OmmConsumerRestLoggingEvent(const char* str, UInt32 length, void* closure) :
	_logRestMessage(str, length),
	_pRestLoggingClosure(closure)
{
}

OmmConsumerRestLoggingEvent::~OmmConsumerRestLoggingEvent()
{
}

const EmaString& OmmConsumerRestLoggingEvent::getRestLoggingMessage() const
{
	return _logRestMessage;
}

void* OmmConsumerRestLoggingEvent::getClosure() const
{
	return _pRestLoggingClosure;
}
