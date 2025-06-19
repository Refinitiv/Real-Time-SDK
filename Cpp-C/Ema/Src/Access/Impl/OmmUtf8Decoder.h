/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmUtf8Decoder_h
#define __refinitiv_ema_access_OmmUtf8Decoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace refinitiv {

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

#endif //__refinitiv_ema_access_OmmUtf8Decoder_h
