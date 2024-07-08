/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmOpaqueEncoder_h
#define __refinitiv_ema_access_OmmOpaqueEncoder_h

#include "OmmNonRwfEncoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmOpaqueEncoder : public OmmNonRwfEncoder
{
public :

	OmmOpaqueEncoder();

	virtual ~OmmOpaqueEncoder();

	void set( const EmaBuffer& );

	void set( const EmaString& );
};

class OmmOpaqueEncoderPool : public EncoderPool< OmmOpaqueEncoder >
{
public :

	OmmOpaqueEncoderPool( unsigned int size = 5 ) : EncoderPool< OmmOpaqueEncoder >( size ) {};

	virtual ~OmmOpaqueEncoderPool() {}

private :

	OmmOpaqueEncoderPool( const OmmOpaqueEncoderPool& );
	OmmOpaqueEncoderPool& operator=( const OmmOpaqueEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmOpaqueEncoder_h
