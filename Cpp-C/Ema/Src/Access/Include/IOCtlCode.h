/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_IOCtlCode_h
#define __thomsonreuters_ema_access_IOCtlCode_h

 /**
	 \class thomsonreuters::ema::access::IOCtlCode IOCtlCode.h "Access/Include/IOCtlCode.h"
	 \brief IOCtlCode class provides enumeration representing I/O codes for modifying I/O values programmatically using the modifyIOCtl() method
	 of provided by OmmProvider and OmmConsumer classes.

	 IOCtlCode::IOCtlCodeEnum is a numeric representation of I/O codes to modify option for a particular channel or server.

	 \code
	 OmmProvider* pOmmProvider; // This provider variable is created for Interactive Provider applications.

	 pOmmProvider->modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, 500 , event.getHandle()); // Modifies the number of guaranteed buffers for the underlying channel.

	 \endcode

	 @see OmmProvider,
		OmmConsumer
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API IOCtlCode
{
public:

	/** @enum IOCtlCodeEnum
		An enumeration representing IOCtl code.
	*/
	enum IOCtlCodeEnum
	{
		MaxNumBuffersEnum = 1,         /*!< Used for changing the max number of buffers. */

		NumGuaranteedBuffersEnum = 2,  /*!< Used for changing the number of guaranteed buffers. */

		HighWaterMarkEnum = 3,         /*!< Used to set the upper buffer usage threshold. */

		ServerNumPoolBuffersEnum = 8,  /*!< Used to increase or decrease the number of server shared pool buffers. This option is used for Interactive Provider applications only. */

		CompressionThresholdEnum = 9,  /*!< When compression is on, this value is the smallest size packet that will be compressed. */
	};

private:

	IOCtlCode();
	IOCtlCode( const IOCtlCode& );
	IOCtlCode& operator=( const IOCtlCode& );
};

}

}

}

#endif // __thomsonreuters_ema_access_IOCtlCode_h
