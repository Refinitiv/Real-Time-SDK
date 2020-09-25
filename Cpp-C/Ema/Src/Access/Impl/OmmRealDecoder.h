/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmRealDecoder_h
#define __refinitiv_ema_access_OmmRealDecoder_h

#include "Decoder.h"
#include "OmmReal.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmRealDecoder : public Decoder
{
public :

	OmmRealDecoder();

	virtual ~OmmRealDecoder();

	Int64 getMantissa() const; 

	OmmReal::MagnitudeType getMagnitudeType() const; 

	double toDouble();

	const EmaString& toString();

	Data::DataCode getCode() const;

	Int8 getRsslIsBlank() const;

	UInt8 getRsslHint() const;

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslReal				_rsslReal;

	double					_toDouble;

	EmaString				_toString;

	EmaBufferInt			_hexBuffer;

	bool					_toDoubleSet;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif // __refinitiv_ema_access_OmmRealDecoder_h
