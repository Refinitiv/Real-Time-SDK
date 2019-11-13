/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_RdmUtilities_h
#define __thomsonreuters_ema_rdm_RdmUtilities_h

#include "Access/Include/Common.h"

namespace thomsonreuters {
	namespace ema {
		namespace access {
			class EmaString;
		}
	}
}

const thomsonreuters::ema::access::EmaString& rdmDomainToString( thomsonreuters::ema::access::UInt16 domain );
const thomsonreuters::ema::access::EmaString& loginNameTypeToString( thomsonreuters::ema::access::UInt8 nameType );

#endif // __thomsonreuters_ema_rdm_RdmUtilities_h


