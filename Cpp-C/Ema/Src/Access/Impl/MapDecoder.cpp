/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "MapDecoder.h"
#include "GlobalPool.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;

MapDecoder::MapDecoder() :
 _rsslMap(),
 _rsslMapBuffer(),
 _rsslMapEntry(),
 _decodeIter(),
 _summary(),
 _load(),
 _key(),
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

MapDecoder::~MapDecoder()
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

	StaticDecoder::morph( &_key, DataType::NoDataEnum );
	StaticDecoder::morph( &_load, DataType::NoDataEnum );
	StaticDecoder::morph( &_summary, DataType::NoDataEnum );
}

void MapDecoder::clone( const MapDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslMapBuffer = other._rsslMapBuffer;

	_pRsslDictionary = other._pRsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslMapBuffer );
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

	retCode = rsslDecodeMap( &_decodeIter, &_rsslMap );

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
		if ( rsslMapCheckHasSetDefs( &_rsslMap ) )
		{
			switch ( _rsslMap.containerType )
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
			( _rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA ) ? _rsslMap.containerType : RSSL_DT_NO_DATA,
			&_decodeIter, &_rsslMap.encSummaryData, _pRsslDictionary, _localSetDefDb );
	}
}

void MapDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
}

void MapDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslMapBuffer );
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

	retCode = rsslDecodeMap( &_decodeIter, &_rsslMap );

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
		( _rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA ) ? _rsslMap.containerType : RSSL_DT_NO_DATA,
		&_decodeIter, &_rsslMap.encSummaryData, _pRsslDictionary, _localSetDefDb );
}

void MapDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void MapDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslMapBuffer = *rsslBuffer;

	_pRsslDictionary = rsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, rsslBuffer );
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

	retCode = rsslDecodeMap( &_decodeIter, &_rsslMap );

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
		if ( rsslMapCheckHasSetDefs( &_rsslMap ) )
		{
			switch ( _rsslMap.containerType )
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
			( _rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA ) ? _rsslMap.containerType : RSSL_DT_NO_DATA,
			&_decodeIter, &_rsslMap.encSummaryData, _pRsslDictionary, _localSetDefDb );
	}
}

bool MapDecoder::getNextData()
{
	if ( _atEnd ) return true;

	if ( !_decodingStarted && _errorCode != OmmError::NoErrorEnum )
	{
		_atEnd = true;
		_decodingStarted = true;
		Decoder::setRsslData( &_load, _errorCode, &_decodeIter, &_rsslMapBuffer ); 
		return false;
	}

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeMapEntry( &_decodeIter, &_rsslMapEntry, 0 );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
		{
			RsslDecodeIterator tempDecodeIter;
			
			rsslClearDecodeIterator( &tempDecodeIter );

			RsslRet retCode = rsslSetDecodeIteratorBuffer( &tempDecodeIter, &_rsslMapEntry.encKey );
			if ( RSSL_RET_SUCCESS != retCode )
			{
				_atEnd = false;
				_errorCode = OmmError::IteratorSetFailureEnum;
				return false;
			}

			retCode = rsslSetDecodeIteratorRWFVersion( &tempDecodeIter, _rsslMajVer, _rsslMinVer );
			if ( RSSL_RET_SUCCESS != retCode )
			{
				_atEnd = false;
				_errorCode = OmmError::IteratorSetFailureEnum;
				return false;
			}

			Decoder::setRsslData( &_key,
						_rsslMap.keyPrimitiveType,
						&tempDecodeIter, &_rsslMapEntry.encKey, _pRsslDictionary, 0 ); 
	
			Decoder::setRsslData( &_load,
						_rsslMapEntry.action != RSSL_MPEA_DELETE_ENTRY ? _rsslMap.containerType : RSSL_DT_NO_DATA,
						&_decodeIter, &_rsslMapEntry.encData, _pRsslDictionary, _localSetDefDb ); 
		}
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslMapEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslMapEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslMapEntry.encData );
		return false;
	}
}

bool MapDecoder::hasKeyFieldId() const
{
	return ( _rsslMap.flags & RSSL_MPF_HAS_KEY_FIELD_ID ) ? true : false;
}

Int16 MapDecoder::getKeyFieldId() const
{
	if ( !hasKeyFieldId() )
	{
		EmaString temp( "Attempt to getKeyFieldId() while it is not set." );
		throwIueException( temp );
	}

	return _rsslMap.keyFieldId;
}

bool MapDecoder::hasTotalCountHint() const
{
	return ( _rsslMap.flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT ) ? true : false;
}

UInt32 MapDecoder::getTotalCountHint() const
{
	if ( !hasTotalCountHint() )
	{
		EmaString temp( "Attempt to getTotalCountHint() while it is NOT set." );
		throwIueException( temp );
	}

	return _rsslMap.totalCountHint;
}

MapEntry::MapAction MapDecoder::getAction() const
{
	return static_cast< MapEntry::MapAction >( _rsslMapEntry.action );
}

const Data& MapDecoder::getKeyData() const
{
	return _key;
}

const Data& MapDecoder::getLoad() const
{
	return _load;
}

bool MapDecoder::hasSummary() const
{
	return ( _rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA ) ? true : false;
}

const Data* MapDecoder::getSummaryData() const
{
	return &_summary;	
}

bool MapDecoder::hasPermissionData() const
{
	return ( _rsslMap.flags & RSSL_MPF_HAS_PER_ENTRY_PERM_DATA ) ? true : false;
}

bool MapDecoder::hasEntryPermissionData() const
{
	return ( _rsslMapEntry.flags & RSSL_MPEF_HAS_PERM_DATA ) ? true : false; 
}

const EmaBuffer& MapDecoder::getEntryPermissionData()
{
	_permissionData.setFromInt( _rsslMapEntry.permData.data, _rsslMapEntry.permData.length );

	return _permissionData.toBuffer();
}

void MapDecoder::setAtExit()
{
	_atExit = true;
}

bool MapDecoder::decodingStarted() const
{
	return _decodingStarted;
}

const EmaBuffer& MapDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslMapBuffer.data, _rsslMapBuffer.length );

	return _hexBuffer.toBuffer();
}
