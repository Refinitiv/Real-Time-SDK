/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmJsonEncoder_h
#define __refinitiv_ema_access_OmmJsonEncoder_h

#include "OmmNonRwfEncoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmJsonEncoder : public OmmNonRwfEncoder
{
public:

	OmmJsonEncoder();

	virtual ~OmmJsonEncoder();

	void set( const EmaBuffer& );

	void set( const EmaString& );
};

class OmmJsonEncoderPool : public EncoderPool< OmmJsonEncoder >
{
public :

	OmmJsonEncoderPool( unsigned int size = 5 ) : EncoderPool< OmmJsonEncoder >( size ) {};

	virtual ~OmmJsonEncoderPool() {}

private :

	OmmJsonEncoderPool( const OmmJsonEncoderPool& );
	OmmJsonEncoderPool& operator=( const OmmJsonEncoderPool& );
};

}

}

}
#endif // __refinitiv_ema_access_OmmJsonEncoder_h
