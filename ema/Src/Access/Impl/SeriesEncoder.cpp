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
#include "Series.h"

using namespace thomsonreuters::ema::access;

SeriesEncoder::SeriesEncoder() :
 _containerInitialized( false ),
 _rsslSeries(),
 _rsslSeriesEntry()
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

	_containerInitialized = false;
}

void SeriesEncoder::initEncode( UInt8 dataType )
{
	_rsslSeries.containerType = dataType;

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

void SeriesEncoder::addEncodedEntry( const char* methodName, RsslBuffer& rsslBuffer )
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
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(complexType).getEncoder() ).convertDataType( complexType.getDataType() );

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( dataType );
	}

	if ( static_cast<const ComplexType&>(complexType).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( "add()", static_cast<const ComplexType&>(complexType).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(complexType).getEncoder() ) );
		startEncodingEntry( "add()" );
	}
}

void SeriesEncoder::complete()
{
	if ( !hasEncIterator() )
	{
		EmaString temp( "Cannot complete an empty Series" );
		throwIueException( temp );
		return;
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
		if ( static_cast<const ComplexType&>(data).getEncoder().isComplete() )
		{
			rsslSeriesApplyHasSummaryData( &_rsslSeries );
			_rsslSeries.encSummaryData = static_cast<const ComplexType&>(data).getEncoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Invalid attempt to pass not completed container to summaryData()." );
			throwIueException( temp );
		}
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is not empty." );
		throwIueException( temp );
	}
}