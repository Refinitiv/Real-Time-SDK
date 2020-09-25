/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_SeriesEncoder_h
#define __refinitiv_ema_access_SeriesEncoder_h

#include "Encoder.h"
#include "ComplexType.h"
#include "EmaPool.h"
#include "rtr/rsslSeries.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmArray;
class ElementList;
class FieldList;
class FilterList;
class Map;
class Series;
class Vector;
class AckMsg;
class GenericMsg;
class PostMsg;
class ReqMsg;
class RespMsg;
class OmmAnsiPage;
class OmmOpaque;
class OmmXml;

class SeriesEncoder : public Encoder
{
public :

	SeriesEncoder();

	virtual ~SeriesEncoder();

	void clear();

	void add( const ComplexType& complexType );

	void add();

	void totalCountHint( UInt32 totalCountHint );

	void summaryData( const ComplexType& data );

	void complete();

private :

	void initEncode( UInt8 rsslDataType, DataType::DataTypeEnum emaDataType );

	void addEncodedEntry( const char* , const RsslBuffer& );

	void startEncodingEntry( const char* );

	void endEncodingEntry() const;

	RsslSeries				_rsslSeries;

	RsslSeriesEntry			_rsslSeriesEntry;

	DataType::DataTypeEnum	_emaDataType;

	bool					_containerInitialized;
};


class SeriesEncoderPool : public EncoderPool< SeriesEncoder >
{
public :

	SeriesEncoderPool( unsigned int size = 5 ) : EncoderPool< SeriesEncoder >( size ) {};

	virtual ~SeriesEncoderPool() {}

private :

	SeriesEncoderPool( const SeriesEncoderPool& );
	SeriesEncoderPool& operator=( const SeriesEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_SeriesEncoder_h
