/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "SeriesEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "Series.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

SeriesEncoder::SeriesEncoder() :
 _rsslSeries(),
 _rsslSeriesEntry(),
 _emaLoadType( DataType::NoDataEnum ),
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

	_emaLoadType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void SeriesEncoder::initEncode( UInt8 rsslDataType, DataType::DataTypeEnum emaDataType )
{
	if ( !_rsslSeries.containerType )
	{
		_rsslSeries.containerType = rsslDataType;
		_emaLoadType = emaDataType;
	}
	else if ( _rsslSeries.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a DataType different than summaryData's DataType. Passed in ComplexType has DataType of " );
		temp += DataType( emaDataType ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
		throwIueException( temp, retCode );
	}

	_containerInitialized = true;
}

void SeriesEncoder::addEncodedEntry( const char* methodName, const RsslBuffer& rsslBuffer )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_SERIES ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add SeriesEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

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
		throwIueException( temp, retCode );
	}
}

void SeriesEncoder::startEncodingEntry( const char* methodName )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_SERIES ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		EmaString temp("Attemp to add SeriesEntry while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

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
		throwIueException( temp, retCode );
	}
}

void SeriesEncoder::endEncodingEntry() const
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_SERIES ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRY_INIT &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Series while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeSeriesEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	/* Reallocate does not need here. The data is placed in already allocated memory */

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Series. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void SeriesEncoder::add( const ComplexType& complexType )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
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
		temp += DataType( _emaLoadType );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	if ( complexType.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( "add()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to add() a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( "add()" );
	}
}

void SeriesEncoder::add()
{
	if (_containerComplete)
	{
		EmaString temp("Attempt to add an entry after complete() was called.");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	UInt8 rsslDataType = RSSL_DT_NO_DATA;

	if (!hasEncIterator())
	{
		acquireEncIterator();

		initEncode(rsslDataType, DataType::NoDataEnum);
	}
	else if (_rsslSeries.containerType != rsslDataType)
	{
		EmaString temp("Attempt to add an entry with a different DataType. Encode DataType as ");
		temp += DataType(DataType::NoDataEnum).toString();
		temp += EmaString(" while the expected DataType is ");
		temp += DataType( _emaLoadType );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslBuffer rsslBuffer;
	rsslClearBuffer(&rsslBuffer);
	addEncodedEntry("add()", rsslBuffer);
}

void SeriesEncoder::complete()
{
	if ( _containerComplete ) return;

	if (!hasEncIterator())
	{
		acquireEncIterator();

		initEncode(convertDataType( _emaLoadType ), _emaLoadType);
	}

	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_SERIES ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRIES &&
		 _levelInfo->_encodingState != RSSL_EIS_SET_DEFINITIONS &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Series while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeSeriesComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Series encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
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
		EmaString temp( "Invalid attempt to call totalCountHint() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
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
				throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		_emaLoadType = data.getDataType();
		_rsslSeries.containerType = enc.convertDataType( _emaLoadType );
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is initialized." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}
