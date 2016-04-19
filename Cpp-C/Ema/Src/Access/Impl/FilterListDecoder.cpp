/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "FilterListDecoder.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;

FilterListDecoder::FilterListDecoder() :
 _rsslFilterList(),
 _rsslFilterListBuffer(),
 _rsslFilterEntry(),
 _decodeIter(),
 _load(),
 _hexBuffer(),
 _pRsslDictionary( 0 ),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION ),
 _errorCode( OmmError::NoErrorEnum ),
 _decodingStarted( false ),
 _atEnd( false )
{
}

FilterListDecoder::~FilterListDecoder()
{
	StaticDecoder::morph( &_load, DataType::NoDataEnum );
}

void FilterListDecoder::clone( const FilterListDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslFilterListBuffer = other._rsslFilterListBuffer;

	_pRsslDictionary = other._pRsslDictionary;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslFilterListBuffer );
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

	retCode = rsslDecodeFilterList( &_decodeIter, &_rsslFilterList );

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
	case RSSL_RET_SET_SKIPPED :
		_atEnd = false;
		_errorCode = OmmError::NoSetDefinitionEnum;
		break;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}
}

bool FilterListDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

void FilterListDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslFilterListBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = !_rsslFilterListBuffer.length ? true : false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeFilterList( &_decodeIter, &_rsslFilterList );

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
	case RSSL_RET_SET_SKIPPED :
		_atEnd = false;
		_errorCode = OmmError::NoSetDefinitionEnum;
		break;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}
}

bool FilterListDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool FilterListDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslFilterListBuffer = *rsslBuffer;

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

	retCode = rsslDecodeFilterList( &_decodeIter, &_rsslFilterList );

	switch ( retCode )
	{
	case RSSL_RET_NO_DATA :
		_atEnd = true;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	case RSSL_RET_SET_SKIPPED :
		_atEnd = false;
		_errorCode = OmmError::NoSetDefinitionEnum;
		return false;
	default :
		_atEnd = false;
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
}

bool FilterListDecoder::getNextData()
{
	if ( _atEnd ) return true;

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeFilterEntry( &_decodeIter, &_rsslFilterEntry );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
		Decoder::setRsslData( &_load, ( _rsslFilterEntry.action == RSSL_FTEA_CLEAR_ENTRY ? RSSL_DT_NO_DATA : ( ( _rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) ?
								_rsslFilterEntry.containerType : _rsslFilterList.containerType ) ),
								&_decodeIter, &_rsslFilterEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslFilterEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslFilterEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslFilterEntry.encData );
		return false;
	}
}

bool FilterListDecoder::getNextData( UInt8 id )
{
	RsslRet retCode = RSSL_RET_SUCCESS;

	do {
		if ( _atEnd ) return true;

		_decodingStarted = true;

		retCode = rsslDecodeFilterEntry( &_decodeIter, &_rsslFilterEntry );

		if ( retCode == RSSL_RET_END_OF_CONTAINER )
		{
			_atEnd = true;
			return true;
		}
	}

	while (	_rsslFilterEntry.id != id );

	switch ( retCode )
	{
	case RSSL_RET_SUCCESS :
		Decoder::setRsslData( &_load, ( _rsslFilterEntry.action == RSSL_FTEA_CLEAR_ENTRY ? RSSL_DT_NO_DATA : ( ( _rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) ?
								_rsslFilterEntry.containerType : _rsslFilterList.containerType ) ),
								&_decodeIter, &_rsslFilterEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslFilterEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslFilterEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslFilterEntry.encData );
		return false;
	}
}

FilterEntry::FilterAction FilterListDecoder::getAction() const
{
	return static_cast< FilterEntry::FilterAction >( _rsslFilterEntry.action );
}

bool FilterListDecoder::hasPermissionData() const
{
	return ( _rsslFilterList.flags & RSSL_FTF_HAS_PER_ENTRY_PERM_DATA ) ? true : false; 
}

bool FilterListDecoder::hasEntryPermissionData() const
{
	return ( _rsslFilterEntry.flags & RSSL_FTEF_HAS_PERM_DATA ) ? true : false; 
}

const EmaBuffer& FilterListDecoder::getEntryPermissionData()
{
	_permissionData.setFromInt( _rsslFilterEntry.permData.data, _rsslFilterEntry.permData.length );

	return _permissionData.toBuffer();
}

const Data& FilterListDecoder::getLoad() const
{
	return _load;
}

void FilterListDecoder::setAtExit()
{
}

UInt8 FilterListDecoder::getFilterId() const
{
	return _rsslFilterEntry.id;
}

bool FilterListDecoder::decodingStarted() const
{
	return _decodingStarted;
}

bool FilterListDecoder::hasTotalCountHint() const
{
	return ( _rsslFilterList.flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT ) ? true : false;
}

UInt32 FilterListDecoder::getTotalCountHint() const
{
	if ( !hasTotalCountHint() )
	{
		EmaString temp( "Attempt to getTotalCountHint() while it is NOT set." );
		throwIueException( temp );
	}

	return _rsslFilterList.totalCountHint;
}

const EmaBuffer& FilterListDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslFilterListBuffer.data, _rsslFilterListBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& FilterListDecoder::getRsslBuffer() const
{
	return _rsslFilterListBuffer;
}

OmmError::ErrorCode FilterListDecoder::getErrorCode() const
{
	return _errorCode;
}
