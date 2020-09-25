/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_MfFieldType_h
#define __refinitiv_ema_rdm_MfFieldType_h

#include "Access/Include/Common.h"

/**
* Enumerations used to define Marketfeed field type
*
*/

namespace rtsdk {

namespace ema {

namespace rdm {

class EMA_ACCESS_API MfFieldType
{

public:

	enum MfFieldTypeEnum
	{
		NONE = -1,

		TIME_SECONDS = 0,

		INTEGER = 1,

		DATE = 3,

		PRICE = 4,

		ALPHANUMERIC = 5,

		ENUMERATED = 6,

		TIME = 7,

		BINARY = 8,

		LONG_ALPHANUMERIC = 9,

		OPAQUE = 10
	};

private:

	MfFieldType();

	MfFieldType(const MfFieldType&);

	MfFieldType& operator=(const MfFieldType&);

	virtual ~MfFieldType();
};

}

}

}

#endif // __refinitiv_ema_rdm_MfFieldType_h

