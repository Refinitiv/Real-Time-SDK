/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_EmaUnitTestConnect_h
#define __thomsonreuters_ema_access__EmaUnitTestConnect_h

#include "RmtesBufferImpl.h"
#include "RmtesBuffer.h"

namespace thomsonreuters {

	namespace ema {

		namespace access {

			class EmaUnitTestConnect
			{
			public:
				static RmtesBufferImpl* getRmtesBufferImpl(RmtesBuffer& rmtesBuffer)
				{
					return rmtesBuffer._pImpl;
				}
			};
		}
	}
}
#endif // __thomsonreuters_ema_access_EmaUnitTestConnect_h
