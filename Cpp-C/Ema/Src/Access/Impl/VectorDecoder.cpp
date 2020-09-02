/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "VectorDecoder.h"
#include "GlobalPool.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace rtsdk::ema::access;

VectorDecoder::VectorDecoder() :
 _rsslVector(),
 _rsslVectorBuffer(),
 _rsslVectorEntry(),
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

VectorDecoder::~VectorDecoder()
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

void VectorDecoder::clone( const VectorDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslVectorBuffer = other._rsslVectorBuffer;

	_pRsslDictionary = other._pRsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslVectorBuffer );
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

	retCode = rsslDecodeVector( &_decodeIter, &_rsslVector );

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
		if ( _rsslVector.flags & RSSL_VTF_HAS_SET_DEFS )
		{
			switch ( _rsslVector.containerType )
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
			( _rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA ) ? _rsslVector.containerType : RSSL_DT_NO_DATA,
			&_decodeIter, &_rsslVector.encSummaryData, _pRsslDictionary, _localSetDefDb );
	}
}

bool VectorDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

void VectorDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslVectorBuffer );
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

	retCode = rsslDecodeVector( &_decodeIter, &_rsslVector );

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
		( _rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA ) ? _rsslVector.containerType : RSSL_DT_NO_DATA,
		&_decodeIter, &_rsslVector.encSummaryData, _pRsslDictionary, _localSetDefDb );
}

bool VectorDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool VectorDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer,
								const RsslDataDictionary* rsslDictionary , void* )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslVectorBuffer = *rsslBuffer;

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

	retCode = rsslDecodeVector( &_decodeIter, &_rsslVector );

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

	if ( _rsslVector.flags & RSSL_VTF_HAS_SET_DEFS )
	{
		switch ( _rsslVector.containerType )
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
		( _rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA ) ? _rsslVector.containerType : RSSL_DT_NO_DATA,
		&_decodeIter, &_rsslVector.encSummaryData, _pRsslDictionary, _localSetDefDb );

	return true;
}

bool VectorDecoder::getNextData()
{
	if ( _atEnd ) return true;

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeVectorEntry( &_decodeIter, &_rsslVectorEntry );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
			Decoder::setRsslData( &_load,
						_rsslVectorEntry.action != RSSL_VTEA_DELETE_ENTRY ? _rsslVector.containerType : RSSL_DT_NO_DATA,
						&_decodeIter, &_rsslVectorEntry.encData, _pRsslDictionary, _localSetDefDb ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslVectorEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslVectorEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslVectorEntry.encData );
		return false;
	}
}

bool VectorDecoder::hasTotalCountHint() const
{
	return ( _rsslVector.flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT ) ? true : false;
}

UInt32 VectorDecoder::getTotalCountHint() const
{
	if ( !hasTotalCountHint() )
	{
		EmaString temp( "Attempt to getTotalCountHint() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _rsslVector.totalCountHint;
}

VectorEntry::VectorAction VectorDecoder::getAction() const
{
	return static_cast<VectorEntry::VectorAction>( _rsslVectorEntry.action );
}

const Data& VectorDecoder::getLoad() const
{
	return _load;
}

bool VectorDecoder::hasSummary() const
{
	return ( _rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA ) ? true : false;
}

const Data* VectorDecoder::getSummaryData() const
{
	return &_summary;	
}

bool VectorDecoder::hasPermissionData() const
{
	return ( _rsslVector.flags & RSSL_VTF_HAS_PER_ENTRY_PERM_DATA ) ? true : false; 
}

bool VectorDecoder::hasEntryPermissionData() const
{
	return ( _rsslVectorEntry.flags & RSSL_VTEF_HAS_PERM_DATA ) ? true : false; 
}

const EmaBuffer& VectorDecoder::getEntryPermissionData()
{
	_permissionData.setFromInt( _rsslVectorEntry.permData.data, _rsslVectorEntry.permData.length );

	return _permissionData.toBuffer();
}

void VectorDecoder::setAtExit()
{
	_atExit = true;
}

bool VectorDecoder::decodingStarted() const
{
	return _decodingStarted;
}

bool VectorDecoder::getSortable() const
{
	return ( _rsslVector.flags & RSSL_VTF_SUPPORTS_SORTING ) ? true : false;
}

UInt32 VectorDecoder::getEntryIndex() const
{
	return _rsslVectorEntry.index;
}

const EmaBuffer& VectorDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslVectorBuffer.data, _rsslVectorBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& VectorDecoder::getRsslBuffer() const
{
	return _rsslVectorBuffer;
}

OmmError::ErrorCode VectorDecoder::getErrorCode() const
{
	return _errorCode;
}
