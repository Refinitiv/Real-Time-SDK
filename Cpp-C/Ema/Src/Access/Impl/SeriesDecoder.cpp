/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "SeriesDecoder.h"
#include "GlobalPool.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace thomsonreuters::ema::access;

SeriesDecoder::SeriesDecoder() :
 _rsslSeries(),
 _rsslSeriesBuffer(),
 _rsslSeriesEntry(),
 _decodeIter(),
 _summary(),
 _load(),
 _hexBuffer(),
 _pRsslDictionary( 0 ),
 _elementListSetDef( 0 ),
 _fieldListSetDef( 0 ),
 _localSetDefDb( 0 ),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION ),
 _errorCode( OmmError::NoErrorEnum ),
 _decodingStarted( false ),
 _atEnd( false ),
 _atExit( false )
{
}

SeriesDecoder::~SeriesDecoder()
{
	if ( _atExit )
	{
		if ( _elementListSetDef ) 
			delete _elementListSetDef;

		if ( _fieldListSetDef )
			delete _fieldListSetDef;
	}
	else
	{
		if ( _elementListSetDef ) 
			g_pool._elementListSetDefPool.returnItem( _elementListSetDef );

		if ( _fieldListSetDef )
			g_pool._fieldListSetDefPool.returnItem( _fieldListSetDef );
	}

	StaticDecoder::morph( &_load, DataType::NoDataEnum );
	StaticDecoder::morph( &_summary, DataType::NoDataEnum );
}

void SeriesDecoder::clone( const SeriesDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslSeriesBuffer = other._rsslSeriesBuffer;

	_pRsslDictionary = other._pRsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslSeriesBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeSeries( &_decodeIter, &_rsslSeries );

	switch ( retCode )
	{
	case RSSL_RET_NO_DATA :
		_atEnd = true;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_errorCode = OmmError::IteratorOverrunEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_errorCode = OmmError::IncompleteDataEnum;
		break;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}

	if ( _errorCode == OmmError::NoErrorEnum )
	{
		if ( _rsslSeries.flags & RSSL_SRF_HAS_SET_DEFS )
		{
			switch ( _rsslSeries.containerType )
			{
			case RSSL_DT_FIELD_LIST :
				_fieldListSetDef = g_pool._fieldListSetDefPool.getItem();
				rsslDecodeLocalFieldSetDefDb( &_decodeIter, _fieldListSetDef->getSetDefDb() );
				_localSetDefDb = _fieldListSetDef->getSetDefDb();
				break;
			case RSSL_DT_ELEMENT_LIST :
				_elementListSetDef = g_pool._elementListSetDefPool.getItem();
				rsslDecodeLocalElementSetDefDb( &_decodeIter, _elementListSetDef->getSetDefDb() );
				_localSetDefDb = _elementListSetDef->getSetDefDb();
				break;
			default :
				_localSetDefDb = 0;
				_errorCode = OmmError::UnsupportedDataTypeEnum;
				return;
			}
		}
		else
			_localSetDefDb = 0;

		Decoder::setRsslData( &_summary,
			( _rsslSeries.flags & RSSL_SRF_HAS_SUMMARY_DATA ) ? _rsslSeries.containerType : RSSL_DT_NO_DATA,
			&_decodeIter, &_rsslSeries.encSummaryData, _pRsslDictionary, _localSetDefDb );
	}
}

bool SeriesDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

void SeriesDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslSeriesBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeSeries( &_decodeIter, &_rsslSeries );

	switch ( retCode )
	{
	case RSSL_RET_NO_DATA :
		_atEnd = true;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_errorCode = OmmError::IteratorOverrunEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_errorCode = OmmError::IncompleteDataEnum;
		break;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}

	Decoder::setRsslData( &_summary,
		(_rsslSeries.flags & RSSL_SRF_HAS_SUMMARY_DATA) ? _rsslSeries.containerType : RSSL_DT_NO_DATA,
		&_decodeIter, &_rsslSeries.encSummaryData, _pRsslDictionary, _localSetDefDb );
}

bool SeriesDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool SeriesDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer,
								const RsslDataDictionary* rsslDictionary , void* )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslSeriesBuffer = *rsslBuffer;

	_pRsslDictionary = rsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, rsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslDecodeSeries( &_decodeIter, &_rsslSeries );

	switch ( retCode )
	{
	case RSSL_RET_NO_DATA :
		_atEnd = true;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}

	if ( _rsslSeries.flags & RSSL_SRF_HAS_SET_DEFS )
	{
		switch ( _rsslSeries.containerType )
		{
		case RSSL_DT_FIELD_LIST :
			_fieldListSetDef = g_pool._fieldListSetDefPool.getItem();
			rsslDecodeLocalFieldSetDefDb( &_decodeIter, _fieldListSetDef->getSetDefDb() );
			_localSetDefDb = _fieldListSetDef->getSetDefDb();
			break;
		case RSSL_DT_ELEMENT_LIST :
			_elementListSetDef = g_pool._elementListSetDefPool.getItem();
			rsslDecodeLocalElementSetDefDb( &_decodeIter, _elementListSetDef->getSetDefDb() );
			_localSetDefDb = _elementListSetDef->getSetDefDb();
			break;
		default :
			_localSetDefDb = 0;
			_errorCode = OmmError::UnsupportedDataTypeEnum;
			return false;
		}
	}
	else
		_localSetDefDb = 0;

	Decoder::setRsslData( &_summary,
		( _rsslSeries.flags & RSSL_SRF_HAS_SUMMARY_DATA ) ? _rsslSeries.containerType : RSSL_DT_NO_DATA,
		&_decodeIter, &_rsslSeries.encSummaryData, _pRsslDictionary, _localSetDefDb );
	
	return true;
}

bool SeriesDecoder::getNextData()
{
	if ( _atEnd ) return true;

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeSeriesEntry( &_decodeIter, &_rsslSeriesEntry );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
			Decoder::setRsslData( &_load, _rsslSeries.containerType,
						&_decodeIter, &_rsslSeriesEntry.encData, _pRsslDictionary, _localSetDefDb );
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslSeriesEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslSeriesEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslSeriesEntry.encData );
		return false;
	}
}

const Data& SeriesDecoder::getLoad() const
{
	return _load;
}

bool SeriesDecoder::hasSummary() const
{
	return ( _rsslSeries.flags & RSSL_SRF_HAS_SUMMARY_DATA ) ? true : false;
}

const Data* SeriesDecoder::getSummaryData() const
{
	return &_summary;	
}

bool SeriesDecoder::hasTotalCountHint() const
{
	return ( _rsslSeries.flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT ) ? true : false;
}

UInt32 SeriesDecoder::getTotalCountHint() const
{
	if ( !hasTotalCountHint() )
	{
		EmaString temp( "Attempt to getTotalCountHint() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _rsslSeries.totalCountHint;
}

void SeriesDecoder::setAtExit()
{
	_atExit = true;
}

bool SeriesDecoder::decodingStarted() const
{
	return _decodingStarted;
}

const EmaBuffer& SeriesDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslSeriesBuffer.data, _rsslSeriesBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& SeriesDecoder::getRsslBuffer() const
{
	return _rsslSeriesBuffer;
}

OmmError::ErrorCode SeriesDecoder::getErrorCode() const
{
	return _errorCode;
}
