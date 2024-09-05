/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_DispatchError_h
#define __refinitiv_ema_access_DispatchError_h

 /**
	 @class refinitiv::ema::access::OmmDispatchError OmmDispatchError.h "Access/Include/DispatchError.h"

	 @see RsslErrorInfoCode,
 */

#include "Access/Include/OmmException.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API DispatchError
{
public :

	/** @enum ErrorCode
		An enumeration representing error codes for handling the exception. 
	*/
	enum ErrorCode
	{
		FailureEnum = -1,                       /*!< General failure. */

		ShutdownEnum = -10,                     /*!<  Reactor is shutdown. */
	};
};

}

}

}

#endif // __refinitiv_ema_access_DispatchError_h
