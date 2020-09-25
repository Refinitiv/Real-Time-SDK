/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_SeriesDecoder_h
#define __refinitiv_ema_access_SeriesDecoder_h

#include "Decoder.h"
#include "EmaBufferInt.h"
#include "ElementListSetDef.h"
#include "FieldListSetDef.h"
#include "NoDataImpl.h"

namespace rtsdk {

namespace ema {

namespace access {

class SeriesDecoder : public Decoder
{
public :

	SeriesDecoder();

	virtual ~SeriesDecoder();

	bool setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	bool getNextData();

	void reset();

	const Data& getLoad() const;

	const Data* getSummaryData() const;

	const EmaBuffer& getHexBuffer();

	bool hasSummary() const;

	bool hasTotalCountHint() const; 

	UInt32 getTotalCountHint() const;

	void clone( const SeriesDecoder& );

	bool decodingStarted() const;

	const RsslBuffer& getRsslBuffer() const;

	void setAtExit();

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslSeries					_rsslSeries;

	mutable RsslBuffer			_rsslSeriesBuffer;
		
	RsslSeriesEntry				_rsslSeriesEntry;

	RsslDecodeIterator			_decodeIter;

	NoDataImpl					_summary;

	NoDataImpl					_load;

	EmaBufferInt				_hexBuffer;

	const RsslDataDictionary*	_pRsslDictionary;

	ElementListSetDef*			_elementListSetDef;

	FieldListSetDef*			_fieldListSetDef;

	void*						_localSetDefDb;

	UInt8						_rsslMajVer;

	UInt8						_rsslMinVer;

	OmmError::ErrorCode			_errorCode;

	bool						_decodingStarted;

	bool						_atEnd;

	bool						_atExit;
};

class SeriesDecoderPool : public DecoderPool< SeriesDecoder >
{
public :

	SeriesDecoderPool( unsigned int size = 5 ) : DecoderPool< SeriesDecoder >( size ) {};

	virtual ~SeriesDecoderPool() {}

private :

	SeriesDecoderPool( const SeriesDecoderPool& );
	SeriesDecoderPool& operator=( const SeriesDecoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_SeriesDecoder_h
