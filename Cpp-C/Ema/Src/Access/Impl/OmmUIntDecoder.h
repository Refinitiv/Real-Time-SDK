/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmUIntDecoder_h
#define __thomsonreuters_ema_access_OmmUIntDecoder_h

#include "Decoder.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmUIntDecoder : public Decoder
{
public :

	OmmUIntDecoder();

	virtual ~OmmUIntDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	UInt64 getUInt() const;

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslUInt				_rsslUInt;

	EmaString				_toString;

	EmaBufferInt			_hexBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmUIntDecoder_h
