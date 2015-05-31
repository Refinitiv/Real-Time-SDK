/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmAnsiPageEncoder_h
#define __thomsonreuters_ema_access_OmmAnsiPageEncoder_h

#include "OmmNonRwfEncoder.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmAnsiPageEncoder : public OmmNonRwfEncoder
{
public:

	OmmAnsiPageEncoder();

	virtual ~OmmAnsiPageEncoder();

	void set( const EmaBuffer& );

	void set( const EmaString& );
};

class OmmAnsiPageEncoderPool : public EncoderPool< OmmAnsiPageEncoder >
{
public :

	OmmAnsiPageEncoderPool( unsigned int size = 5 ) : EncoderPool< OmmAnsiPageEncoder >( size ) {};

	virtual ~OmmAnsiPageEncoderPool() {}

private :

	OmmAnsiPageEncoderPool( const OmmAnsiPageEncoderPool& );
	OmmAnsiPageEncoderPool& operator=( const OmmAnsiPageEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmAnsiPageEncoder_h
