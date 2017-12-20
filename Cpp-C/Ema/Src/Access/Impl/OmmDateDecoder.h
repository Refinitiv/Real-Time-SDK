/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmDateDecoder_h
#define __thomsonreuters_ema_access_OmmDateDecoder_h

#include "Decoder.h"
#include "EmaBufferInt.h"
#include "DateTimeStringFormat.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmDateDecoder : public Decoder
{
public :

	OmmDateDecoder();

	virtual ~OmmDateDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	DateTimeStringFormat::DateTimeStringFormatTypes setDateTimeStringFormatType(DateTimeStringFormat::DateTimeStringFormatTypes format);

	DateTimeStringFormat::DateTimeStringFormatTypes getDateTimeStringFormatType();

	Data::DataCode getCode() const;

	const EmaString& toString();

	UInt16 getYear() const;

	UInt8 getMonth() const;

	UInt8 getDay() const;

	const EmaBuffer& getHexBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer*				_pRsslBuffer;

	RsslDate				_rsslDate;

	DateTimeStringFormat::DateTimeStringFormatTypes _format;

	EmaString				_toString;

	EmaBufferInt			_hexBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmDateDecoder_h
