/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmErrorDecoder_h
#define __thomsonreuters_ema_access_OmmErrorDecoder_h

#include "Decoder.h"
#include "OmmError.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmErrorDecoder : public Decoder
{
public :

	OmmErrorDecoder();

	virtual ~OmmErrorDecoder();

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	OmmError::ErrorCode getErrorCode() const;

	const EmaBuffer& getAsHex();

private :

	RsslBuffer*					_pRsslBuffer;

	OmmError::ErrorCode			_errorCode;

	EmaBufferInt				_toHex;
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmErrorDecoder_h
