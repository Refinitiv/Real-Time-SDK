/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmStateDecoder_h
#define __thomsonreuters_ema_access_OmmStateDecoder_h

#include "Decoder.h"
#include "OmmState.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmStateDecoder : public Decoder
{
public :

	OmmStateDecoder();

	virtual ~OmmStateDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	void setRsslData( RsslState* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	OmmState::StreamState getStreamState() const;

	OmmState::DataState getDataState() const;

	UInt8 getStatusCode() const;

	const EmaString& getStatusText();

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslState				_rsslState;

	EmaString				_toString;

	EmaStringInt			_statusText;

	EmaBufferInt			_hexBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmStateDecoder_h
