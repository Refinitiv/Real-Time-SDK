/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmUtf8Decoder_h
#define __thomsonreuters_ema_access_OmmUtf8Decoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmUtf8Decoder : public Decoder
{
public :

	OmmUtf8Decoder();

	virtual ~OmmUtf8Decoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	const EmaBuffer& getUtf8();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer				_rsslBuffer;

	EmaStringInt			_toString;

	EmaBufferInt			_utf8Buffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmUtf8Decoder_h
