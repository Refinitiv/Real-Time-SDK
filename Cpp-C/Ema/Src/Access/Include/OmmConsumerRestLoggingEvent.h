/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2021 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __refinitiv_ema_access_OmmConsumerRestLoggingEvent_h
#define __refinitiv_ema_access_OmmConsumerRestLoggingEvent_h

 /**
	 @class refinitiv::ema::access::OmmConsumerRestLoggingEvent OmmConsumerRestLoggingEvent.h "Access/Include/OmmConsumerRestLoggingEvent.h"
	 @brief OmmConsumerRestLoggingEvent encapsulates Reactor REST logging message.

	 OmmConsumerRestLoggingEvent is used to convey Reactor REST logging message to application.
	 OmmConsumerRestLoggingEvent is returned through OmmConsumerImpl::OmmBaseImpl callback method.

	 \remark OmmConsumerRestLoggingEvent is a read only class. This class is used for receiving REST logging message.
	 \remark All methods in this class are \ref SingleThreaded.

	 @see OmmConsumerImpl,
		 OmmConsumerConfig
 */

#include "Common.h"
#include "EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmConsumerRestLoggingEvent
{
public:

	///@name Accessors
	//@{
	/** Returns received REST logging message for this event.
		@return REST logging message for this event.
	*/
	const EmaString& getRestLoggingMessage() const;

	/** Returns an identifier (a.k.a., closure) associated with a user callback client by consumer application.
		Application associates the closure with the user callback client on OmmConsumerConfig::restLoggingCallback( OmmRestLoggingClient& , void* closure ).
		@return closure value.
	*/
	void* getClosure() const;
	//@}

private:

	friend class OmmBaseImpl;

	EmaString _logRestMessage;
	void* _pRestLoggingClosure;

	OmmConsumerRestLoggingEvent( const char* str, refinitiv::ema::access::UInt32 length, void* closure );
	virtual ~OmmConsumerRestLoggingEvent();

	OmmConsumerRestLoggingEvent(const OmmConsumerRestLoggingEvent&);
	OmmConsumerRestLoggingEvent& operator=(const OmmConsumerRestLoggingEvent&);
};

}

}

}

#endif  // __refinitiv_ema_access_OmmConsumerRestLoggingEvent_h
