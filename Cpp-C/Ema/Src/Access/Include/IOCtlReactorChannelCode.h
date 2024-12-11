/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license 
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose. 
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.   
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __refinitiv_ema_access_IOCtlReactorChannelCode_h
#define __refinitiv_ema_access_IOCtlReactorChannelCode_h

 /**
	 \class refinitiv::ema::access::IOCtlReactorChannelCode IOCtlReactorChannelCode.h "Access/Include/IOCtlReactorChannelCode.h"
	 \brief IOCtlReactorChannelCode class provides enumeration representing I/O codes for modifying Reactor channel I/O values programmatically using the modifyReactorChannelIOCtl() method
	 of provided by OmmProvider and OmmConsumer classes.

	 IOCtlReactorChannelCode::IOCtlReactorChannelCodeEnum is a numeric representation of I/O codes to modify option for Reactor channel.

	 \code
	 OmmConsumer* pOmmConsumer; // This consumer variable is created for Consumer applications.

	 pOmmConsumer->modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelDirectWrite, 1); // enable direct write to the socket

	 \endcode

	 @see OmmProvider,
		OmmConsumer
*/

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API IOCtlReactorChannelCode
{
public:

	/** @enum IOCtlReactorChannelCodeEnum
		An enumeration representing Reactor channel IOCtl codes.
	*/
	enum IOCtlReactorChannelCodeEnum {
		ReactorChannelDirectWrite = 200,  /* (200) ReactorChannel: Used for attempting to pass the data directly to the transport, avoiding the queuing for this channel. It will be set flag RSSL_WRITE_DIRECT_SOCKET_WRITE for rsslWrite. */

		ReactorChannelPreferredHost = 201 /* (201) ReactorChannel: Used to dynamically change the preferred host options. See RsslPreferredHostOptions */
	};

private:

	IOCtlReactorChannelCode();
	IOCtlReactorChannelCode(const IOCtlReactorChannelCode&);
	IOCtlReactorChannelCode& operator=(const IOCtlReactorChannelCode&);
};

}

}

}

#endif  // __refinitiv_ema_access_IOCtlReactorChannelCode_h
