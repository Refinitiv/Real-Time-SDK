/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_NoDataImpl_h
#define __thomsonreuters_ema_access_NoDataImpl_h

#include "Data.h"
#include "Decoder.h"
#include "EmaBufferInt.h"

#define MAX_NODATA		20

namespace rtsdk {

namespace ema {

namespace access {

class NoDataImpl : public Data, public Decoder
{
public :

	NoDataImpl();

	virtual ~NoDataImpl();

	DataType::DataTypeEnum getDataType() const;

	Data::DataCode getCode() const;

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	const EmaString& toString( UInt64 indent ) const;

	const EmaBuffer& getAsHex() const;

	const EmaString& toString() const;

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer				_rsslBuffer;

	mutable EmaBufferInt	_hexBuffer;

	UInt8					_majVer;

	UInt8					_minVer;

	mutable EmaString		_toString;

	UInt64					_space[ MAX_NODATA ];

	OmmError::ErrorCode		_errorCode;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	NoDataImpl( const NoDataImpl& );
	NoDataImpl& operator=( const NoDataImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_NoDataImpl_h
