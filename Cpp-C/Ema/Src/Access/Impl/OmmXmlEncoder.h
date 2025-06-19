/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmXmlEncoder_h
#define __refinitiv_ema_access_OmmXmlEncoder_h

#include "OmmNonRwfEncoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmXmlEncoder : public OmmNonRwfEncoder
{
public:

	OmmXmlEncoder();

	virtual ~OmmXmlEncoder();

	void set( const EmaBuffer& );

	void set( const EmaString& );
};

class OmmXmlEncoderPool : public EncoderPool< OmmXmlEncoder >
{
public :

	OmmXmlEncoderPool( unsigned int size = 5 ) : EncoderPool< OmmXmlEncoder >( size ) {};

	virtual ~OmmXmlEncoderPool() {}

private :

	OmmXmlEncoderPool( const OmmXmlEncoderPool& );
	OmmXmlEncoderPool& operator=( const OmmXmlEncoderPool& );
};

}

}

}
#endif // __refinitiv_ema_access_OmmXmlEncoder_h
