/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "SeriesEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "Series.h"

using namespace thomsonreuters::ema::access;

SeriesEncoder::SeriesEncoder() :
 _rsslSeries(),
 _rsslSeriesEntry(),
 _emaDataType( DataType::NoDataEnum ),
 _containerInitialized( false )
{
}

SeriesEncoder::~SeriesEncoder()
{
}

void SeriesEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearSeries( &_rsslSeries );
	rsslClearSeriesEntry( &_rsslSeriesEntry );

	_emaDataType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void SeriesEncoder::initEncode( UInt8 rsslDataType, DataType::DataTypeEnum emaDataType )
{
	if ( !_rsslSeries.containerType )
	{
		_rsslSeries.containerType = rsslDataType;
		_emaDataType = emaDataType;
	}
	else if ( _rsslSeries.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a DataType different than summaryData's DataType. Passed in ComplexType has DataType of " );
		temp += DataType( emaDataType ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaDataType );
		throwIueException( temp );
		return;
	}

	RsslRet retCode = rsslEncodeSeriesInit( &(_pEncodeIter->_rsslEncIter), &_rsslSeries, 0, 0 );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeSeriesComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeSeriesInit( &(_pEncodeIter->_rsslEncIter), &_rsslSeries, 0, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize Series encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void SeriesEncoder::addEncodedEntry( const char* methodName, const RsslBuffer& rsslBuffer )
{
	_rsslSeriesEntry.encData = rsslBuffer;

	RsslRet retCode = rsslEncodeSeriesEntry( &_pEncodeIter->_rsslEncIter, &_rsslSeriesEntry );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeSeriesEntry( &_pEncodeIter->_rsslEncIter, &_rsslSeriesEntry );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding Series. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void SeriesEncoder::startEncodingEntry( const char* methodName )
{
	_rsslSeriesEntry.encData.data = 0;
	_rsslSeriesEntry.encData.length = 0;

	RsslRet retCode = rsslEncodeSeriesEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslSeriesEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeSeriesEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslSeriesEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in Series::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void SeriesEncoder::endEncodingEntry() const
{
	RsslRet retCode = rsslEncodeSeriesEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Series. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void SeriesEncoder::add( const ComplexType& complexType )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = complexType.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( complexType.getDataType() );

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( rsslDataType, complexType.getDataType() );
	}
	else if ( _rsslSeries.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( complexType.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaDataType );
		throwIueException( temp );
		return;
	}

	if ( complexType.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( "add()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to add() a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( complexType.hasDecoder() )
	{
		addEncodedEntry( "add()", const_cast<ComplexType&>( complexType ).getDecoder().getRsslBuffer() );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( "add()" );
	}
}

void SeriesEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !hasEncIterator() )
	{
		if ( _rsslSeries.containerType )
		{
			acquireEncIterator();

			initEncode( _rsslSeries.containerType, _emaDataType );
		}
		else
		{
			EmaString temp( "Attempt to complete() while no Series::add() or Series::summaryData() were called yet." );
			throwIueException( temp );
			return;
		}
	}

	RsslRet retCode = rsslEncodeSeriesComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Series encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}

void SeriesEncoder::totalCountHint( UInt32 totalCountHint )
{
	if ( !_containerInitialized )
	{
		rsslSeriesApplyHasTotalCountHint( &_rsslSeries );
		_rsslSeries.totalCountHint = totalCountHint;
	}
	else
	{
		EmaString temp( "Invalid attempt to call totalCountHint() when container is not empty." );
		throwIueException( temp );
	}
}

void SeriesEncoder::summaryData( const ComplexType& data )
{
	if ( !_containerInitialized )
	{
		const Encoder& enc = data.getEncoder();

		if ( data.hasEncoder() && enc.ownsIterator() )
		{
			if ( enc.isComplete() )
			{
				rsslSeriesApplyHasSummaryData( &_rsslSeries );
				_rsslSeries.encSummaryData = enc.getRsslBuffer();
			}
			else
			{
				EmaString temp( "Attempt to set summaryData() with a ComplexType while complete() was not called on this ComplexType." );
				throwIueException( temp );
			}
		}
		else if ( data.hasDecoder() )
		{
			rsslSeriesApplyHasSummaryData( &_rsslSeries );
			_rsslSeries.encSummaryData = const_cast<ComplexType&>(data).getDecoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Attempt to pass an empty ComplexType to summaryData() while it is not supported." );
			throwIueException( temp );
			return;
		}

		_emaDataType = data.getDataType();
		_rsslSeries.containerType = enc.convertDataType( _emaDataType );
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is not empty." );
		throwIueException( temp );
	}
}
