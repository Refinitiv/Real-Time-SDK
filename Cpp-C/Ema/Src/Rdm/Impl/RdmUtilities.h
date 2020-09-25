/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_RdmUtilities_h
#define __refinitiv_ema_rdm_RdmUtilities_h

#include "Access/Include/Common.h"

namespace rtsdk {
	namespace ema {
		namespace access {
			class EmaString;
		}
	}
}

const rtsdk::ema::access::EmaString& rdmDomainToString( rtsdk::ema::access::UInt16 domain );
const rtsdk::ema::access::EmaString& loginNameTypeToString( rtsdk::ema::access::UInt8 nameType );

#endif // __refinitiv_ema_rdm_RdmUtilities_h


