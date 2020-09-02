/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmQosDecoder_h
#define __thomsonreuters_ema_access_OmmQosDecoder_h

#include "Decoder.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmQosDecoder : public Decoder
{
public :

	static void convertToRssl( RsslQos* , UInt32 timeliness, UInt32 rate );

	OmmQosDecoder();

	virtual ~OmmQosDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	void setRsslData( RsslQos* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	UInt32 getTimeliness() const;

	UInt32 getRate() const;

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslQos					_rsslQos;

	EmaString				_toString;

	EmaBufferInt			_hexBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmQosDecoder_h
