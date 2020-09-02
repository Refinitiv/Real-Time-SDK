/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_EmaBufferInt_h
#define __thomsonreuters_ema_access_EmaBufferInt_h

#include "EmaBuffer.h"

namespace rtsdk {
	
namespace ema {

namespace access {

class EmaBufferInt : protected EmaBuffer
{
public :

	EmaBufferInt();

	virtual ~EmaBufferInt();

	void clear();

	void setFromInt( const char* , UInt32 );

	const EmaBuffer& toBuffer() const;
};

}

}

}

#endif //__thomsonreuters_ema_access_EmaBufferInt_h
