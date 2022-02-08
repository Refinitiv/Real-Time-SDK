/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv.   All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __refinitiv_ema_access_IOCtlReactorCode_h
#define __refinitiv_ema_access_IOCtlReactorCode_h

 /**
	 \class refinitiv::ema::access::IOCtlReactorCode IOCtlReactorCode.h "Access/Include/IOCtlReactorCode.h"
	 \brief IOCtlReactorCode class provides enumeration representing I/O codes for modifying Reactor I/O values programmatically using the modifyReactorIOCtl() method
	 of provided by OmmProvider and OmmConsumer classes.

	 IOCtlReactorCode::IOCtlReactorCodeEnum is a numeric representation of I/O codes to modify option for Reactor.

	 \code
	 OmmConsumer* pOmmConsumer; // This consumer variable is created for Consumer applications.

	 pOmmConsumer->modifyReactorIOCtl(IOCtlReactorCode::EnableRestLoggingEnum, 1); // Turn on the printing REST messages to output stream.

	 \endcode

	 @see OmmProvider,
		OmmConsumer
*/

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API IOCtlReactorCode
{
public:

	/** @enum IOCtlReactorCodeEnum
		An enumeration representing Reactor IOCtl codes.
	*/
	enum IOCtlReactorCodeEnum
	{
		EnableRestLoggingEnum = 1,          /*!< Used for enabling or disabling the printing REST interaction debug messages to output stream. */

		EnableRestCallbackLoggingEnum = 2,  /*!< Used for enabling or disabling the receiving REST logging messages via callback specified by user. */
	};

private:

	IOCtlReactorCode();
	IOCtlReactorCode(const IOCtlReactorCode&);
	IOCtlReactorCode& operator=(const IOCtlReactorCode&);
};

}

}

}

#endif  // __refinitiv_ema_access_IOCtlReactorCode_h
