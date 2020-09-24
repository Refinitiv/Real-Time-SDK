/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmOpaqueDecoder_h
#define __rtsdk_ema_access_OmmOpaqueDecoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmOpaqueDecoder : public Decoder
{
public :

	OmmOpaqueDecoder();

	virtual ~OmmOpaqueDecoder();

	bool setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	const EmaString& getString();

	const EmaBuffer& getBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer				_rsslBuffer;

	EmaStringInt			_toString;

	EmaStringInt			_getString;

	EmaBufferInt			_getBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif // __rtsdk_ema_access_OmmOpaqueDecoder_h
