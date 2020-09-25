/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaUnitTestConnect_h
#define __refinitiv_ema_access__EmaUnitTestConnect_h

#include "RmtesBufferImpl.h"
#include "RmtesBuffer.h"

namespace rtsdk {

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
#endif // __refinitiv_ema_access_EmaUnitTestConnect_h
