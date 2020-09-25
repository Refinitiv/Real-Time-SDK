/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmTimeDecoder_h
#define __refinitiv_ema_access_OmmTimeDecoder_h

#include "Decoder.h"
#include "EmaBufferInt.h"
#include "DateTimeStringFormat.h"


namespace rtsdk {

namespace ema {

namespace access {

class OmmTimeDecoder : public Decoder
{
public :

	OmmTimeDecoder();

	virtual ~OmmTimeDecoder();

	UInt8 getHour() const;

	UInt8 getMinute() const;

	UInt8 getSecond() const;

	UInt16 getMillisecond() const;

	UInt16 getMicrosecond() const;

	UInt16 getNanosecond() const;

	const EmaString& toString();

	Data::DataCode getCode() const;

	DateTimeStringFormat::DateTimeStringFormatTypes setDateTimeStringFormatType(DateTimeStringFormat::DateTimeStringFormatTypes format);

	DateTimeStringFormat::DateTimeStringFormatTypes getDateTimeStringFormatType();


	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslTime				_rsslTime;

	DateTimeStringFormat::DateTimeStringFormatTypes _format;

	EmaString				_toString;

	EmaBufferInt			_hexBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif //__refinitiv_ema_access_OmmTimeDecoder_h
