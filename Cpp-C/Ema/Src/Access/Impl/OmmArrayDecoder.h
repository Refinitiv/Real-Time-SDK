/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmArrayDecoder_h
#define __refinitiv_ema_access_OmmArrayDecoder_h

#include "Decoder.h"
#include "OmmError.h"
#include "NoDataImpl.h"
#include "EmaBufferInt.h"
#include "EmaPool.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmArrayDecoder : public Decoder
{
public :

	OmmArrayDecoder();

	virtual ~OmmArrayDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );
		
	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	void reset();

	const Data& getLoad() const;

	Data::DataCode getCode() const;

	DataType::DataTypeEnum getLoadDataType() const;

	bool hasFixedWidth() const;

	UInt16 getFixedWidth() const;

	RsslPrimitiveType getRsslDataType() const;

	const EmaBuffer& getHexBuffer();

	void clone( const OmmArrayDecoder& );

	bool decodingStarted() const;

	const RsslBuffer& getRsslBuffer() const;

	void setAtExit();

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslArray					_rsslArray;

	mutable RsslBuffer			_rsslArrayBuffer;

	RsslBuffer					_rsslEntryBuffer;

	RsslDecodeIterator			_decodeIter;

	NoDataImpl					_load;

	EmaBufferInt				_hexBuffer;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	Data::DataCode				_dataCode;

	OmmError::ErrorCode			_errorCode;
	
	bool						_decodingStarted;

	bool						_atEnd;
};

class OmmArrayDecoderPool : public DecoderPool< OmmArrayDecoder >
{
public :

	OmmArrayDecoderPool( unsigned int size = 5 ) : DecoderPool< OmmArrayDecoder >( size ) {};

	virtual ~OmmArrayDecoderPool() {}

private :

	OmmArrayDecoderPool( const OmmArrayDecoderPool& );
	OmmArrayDecoderPool& operator=( const OmmArrayDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmArrayDecoder_h
