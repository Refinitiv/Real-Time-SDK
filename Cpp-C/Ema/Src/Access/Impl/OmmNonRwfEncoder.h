/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmNonRwfEncoder_h
#define __refinitiv_ema_access_OmmNonRwfEncoder_h

#include "Encoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmNonRwfEncoder : public Encoder
{
public :

	OmmNonRwfEncoder();

	virtual ~OmmNonRwfEncoder();

	void encodeBuffer( const char *, UInt32 );

	void endEncodingEntry() const;

	void release();
};

}

}

}

#endif // __refinitiv_ema_access_OmmNonRwfEncoder_h
