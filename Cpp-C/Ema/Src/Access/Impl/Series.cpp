/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Series.h"
#include "SeriesDecoder.h"
#include "SummaryData.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Series::Series() :
 _toString(),
 _entry(),
 _summary(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

Series::~Series()
{
	if ( _pEncoder )
		g_pool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool.returnItem( _pDecoder );
}

Series& Series::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode Series::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum Series::getDataType() const
{
	return DataType::SeriesEnum;
}

bool Series::forth() const
{
	return !_pDecoder->getNextData();
}

void Series::reset() const
{
	_pDecoder->reset();
}

const EmaString& Series::toString() const
{
	return toString( 0 );
}

const EmaString& Series::toString( UInt64 indent ) const
{
	if ( !_pDecoder )
		return _toString.clear().append( "\nDecoding of just encoded object in the same application is not supported\n" );

	SeriesDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "Series" );
			
	if ( tempDecoder.hasTotalCountHint() )
		_toString.append( " totalCountHint=\"" ).append( tempDecoder.getTotalCountHint() );

	if ( tempDecoder.hasSummary() )
	{
		++indent;
		addIndent( _toString.append( "\n" ), indent )
			.append( "SummaryData dataType=\"" ).append( getDTypeAsString( tempDecoder.getSummaryData()->getDataType() ) ).append( "\"\n" );

		++indent;
		_toString += tempDecoder.getSummaryData()->toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "SummaryDataEnd" );
		--indent;
	}

	++indent;
		
	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "SeriesEntry dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) ).append( "\"\n" );
		
		++indent;
		_toString += tempDecoder.getLoad().toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "SeriesEntryEnd" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "SeriesEnd\n" );

	return _toString;
}

const EmaBuffer& Series::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& Series::getDecoder()
{
	if ( !_pDecoder )
	{
		_summary._pDecoder = _entry._pDecoder = _pDecoder = g_pool.getSeriesDecoderItem();
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

bool Series::hasDecoder() const
{
	return _pDecoder ? true : false;
}

const Encoder& Series::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getSeriesEncoderItem();

	return *_pEncoder;
}

const SummaryData& Series::getSummaryData() const
{
	return _summary;
}

UInt32 Series::getTotalCountHint() const
{
	return _pDecoder->getTotalCountHint();
}

bool Series::hasTotalCountHint() const
{
	return _pDecoder->hasTotalCountHint();
}

const SeriesEntry& Series::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _entry;
}

Series& Series::add( const ComplexType& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getSeriesEncoderItem();

	_pEncoder->add( value );

	return *this;
}

Series& Series::add()
{
	if (!_pEncoder)
		_pEncoder = g_pool.getSeriesEncoderItem();

	_pEncoder->add();

	return *this;
}

const Series& Series::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getSeriesEncoderItem();

	_pEncoder->complete();

	return *this;
}

Series& Series::totalCountHint( UInt32 totalCountHint )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getSeriesEncoderItem();

	_pEncoder->totalCountHint( totalCountHint );

	return *this;
}

Series& Series::summaryData( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getSeriesEncoderItem();

	_pEncoder->summaryData( data );

	return *this;
}

bool Series::hasEncoder() const
{
	return _pEncoder ? true : false;
}
