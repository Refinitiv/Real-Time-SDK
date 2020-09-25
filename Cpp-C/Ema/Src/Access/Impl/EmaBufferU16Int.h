/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaBufferU16Int_h
#define __refinitiv_ema_access_EmaBufferU16Int_h

#include "EmaBufferU16.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class EmaBufferU16Int : protected EmaBufferU16
{
public :

	EmaBufferU16Int();

	virtual ~EmaBufferU16Int();

	void clear();

	void setFromInt( const UInt16* , UInt32 );

	const EmaBufferU16& toBuffer() const;
};

}

}

}

#endif //__refinitiv_ema_access_EmaBufferU16Int_h
