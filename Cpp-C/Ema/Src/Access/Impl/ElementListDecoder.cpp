/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ElementListDecoder.h"
#include "StaticDecoder.h"
#include "Encoder.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

ElementListDecoder::ElementListDecoder() :
 _rsslElementList(),
 _rsslElementListBuffer(),
 _rsslElementEntry(),
 _decodeIter(),
 _pLoadPool( 0 ),
 _pLoad( 0 ),
 _pRsslDictionary( 0 ),
 _rsslLocalELSetDefDb( 0 ),
 _name(),
 _hexBuffer(),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION ),
 _errorCode( OmmError::NoErrorEnum ),
 _decodingStarted( false ),
 _atEnd( false )
{
	createLoadPool( _pLoadPool );
}

ElementListDecoder::~ElementListDecoder()
{
	destroyLoadPool( _pLoadPool );
}

bool ElementListDecoder::hasInfo() const
{
	return _rsslElementList.flags & RSSL_ELF_HAS_ELEMENT_LIST_INFO ? true : false;
}

Int16 ElementListDecoder::getInfoElementListNum() const
{
	if ( !hasInfo() )
	{
		EmaString temp( "Attempt to getInfoElementListNum() while it is NOT set." );

		throwIueException( temp );
	}

	return _rsslElementList.elementListNum;
}

void ElementListDecoder::clone( const ElementListDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslElementListBuffer = other._rsslElementListBuffer;

	_pRsslDictionary = other._pRsslDictionary;

	_rsslLocalELSetDefDb = other._rsslLocalELSetDefDb;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslElementListBuffer );
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

	retCode = rsslDecodeElementList( &_decodeIter, &_rsslElementList, _rsslLocalELSetDefDb );

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

bool ElementListDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

void ElementListDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslElementListBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = !_rsslElementListBuffer.length ? true : false;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeElementList( &_decodeIter, &_rsslElementList, _rsslLocalELSetDefDb );

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

bool ElementListDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool ElementListDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer,
									 const RsslDataDictionary* rsslDictionary, void* localElSeDefDb )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslElementListBuffer = *rsslBuffer;

	_pRsslDictionary = rsslDictionary;

	_rsslLocalELSetDefDb = (RsslLocalElementSetDefDb*)localElSeDefDb;

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

	retCode = rsslDecodeElementList( &_decodeIter, &_rsslElementList, _rsslLocalELSetDefDb );

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

bool ElementListDecoder::getNextData()
{
	if ( _atEnd ) return true;

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeElementEntry( &_decodeIter, &_rsslElementEntry );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
		_pLoad = Decoder::setRsslData( _pLoadPool, _rsslElementEntry.dataType,
								&_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
		return false;
	}
}

bool ElementListDecoder::getNextData( const EmaString& name )
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	bool matchName = false;
	EmaStringInt tempName;

	do {
		if ( _atEnd ) return true;

		_decodingStarted = true;

		retCode = rsslDecodeElementEntry( &_decodeIter, &_rsslElementEntry );

		if ( retCode == RSSL_RET_END_OF_CONTAINER )
		{
			_atEnd = true;
			return true;
		}

		tempName.setInt(  _rsslElementEntry.name.data, _rsslElementEntry.name.length, true );
		if ( name == tempName.toString() )
			matchName = true;

	} while ( !matchName );

	switch ( retCode )
	{
	case RSSL_RET_SUCCESS :
		_pLoad = Decoder::setRsslData( _pLoadPool, _rsslElementEntry.dataType,
								&_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
		return false;
	}
}

bool ElementListDecoder::getNextData( const EmaVector< EmaString >& stringList )
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	bool matchName = false;
	EmaStringInt tempName;

	do {
		if ( _atEnd ) return true;

		_decodingStarted = true;

		retCode = rsslDecodeElementEntry( &_decodeIter, &_rsslElementEntry );

		if ( retCode == RSSL_RET_END_OF_CONTAINER )
		{
			_atEnd = true;
			return true;
		}

		tempName.setInt(  _rsslElementEntry.name.data, _rsslElementEntry.name.length, true );
			
		UInt32 size = stringList.size();
		for ( UInt32 idx = 0; idx < size; ++idx )
		{
			if ( stringList[idx] == tempName.toString() )
			{
				matchName = true;
				break;
			}
		}
	} while ( !matchName );

	switch ( retCode )
	{
	case RSSL_RET_SUCCESS :
		_pLoad = Decoder::setRsslData( _pLoadPool, _rsslElementEntry.dataType, &_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		_pLoad = Decoder::setRsslData( _pLoadPool[DataType::ErrorEnum], OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
		return false;
	}
}

bool ElementListDecoder::getNextData( const Data& data )
{
	if ( data.getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Wrong container type used for passing search list. Expecting ElementList. Passed in container is " );
		temp += getDTypeAsString( data.getDataType() );
		throwIueException( temp );
		return true;
	}

	RsslDataType listDataType = RSSL_DT_UNKNOWN;
	EmaVector< EmaString > stringList;	

	decodeViewList( &data.getEncoder().getRsslBuffer(), listDataType, stringList );

	switch ( listDataType )
	{
	case RSSL_DT_ASCII_STRING :
		return getNextData( stringList );
	default :
		throwIueException( EmaString( "Passed in search list contains wrong data type." ) );
		return true;
	}
}

void ElementListDecoder::decodeViewList( RsslBuffer* rsslBuffer, RsslDataType& rsslDataType, EmaVector< EmaString >& stringList )
{
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator( &dIter );
	rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetDecodeIteratorBuffer( &dIter, rsslBuffer );

	RsslElementList rsslElementList;
	rsslClearElementList( &rsslElementList );

	rsslDecodeElementList( &dIter, &rsslElementList, 0 );
	
	RsslElementEntry rsslElementEntry;
	rsslClearElementEntry( &rsslElementEntry );

	RsslRet retCode = rsslDecodeElementEntry( &dIter, &rsslElementEntry );

	while ( retCode == RSSL_RET_SUCCESS )
	{
		switch ( rsslElementEntry.dataType )
		{
		case RSSL_DT_ARRAY :
			{
				RsslArray rsslArray;
				rsslClearArray( &rsslArray );

				RsslDecodeIterator dIter;
				rsslClearDecodeIterator( &dIter );
				rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
				rsslSetDecodeIteratorBuffer( &dIter, &rsslElementEntry.encData );

				rsslDecodeArray( &dIter, &rsslArray );
				if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
				{
					RsslBuffer rsslBuffer;
					rsslClearBuffer( & rsslBuffer );
					RsslRet retCode = rsslDecodeArrayEntry( &dIter, &rsslBuffer );
					
					while ( retCode == RSSL_RET_SUCCESS )
					{
						RsslBuffer stringValue;
						rsslDecodeBuffer( &dIter, &stringValue );
						EmaString temp( stringValue.data, stringValue.length );
						if ( 0 > stringList.getPositionOf( temp ) )
							stringList.push_back( temp );
						retCode = rsslDecodeArrayEntry( &dIter, &rsslBuffer );
					}

					if ( retCode != RSSL_RET_END_OF_CONTAINER )
					{
						EmaString temp( "Error decoding OmmArray with Ascii while compiling search list." );
						throwIueException( temp );
						return;
					}
				}

				rsslDataType = rsslArray.primitiveType;
			}
			break;
		default :
			EmaString temp( "Search list does not contain expected OmmArray of Ascii." );
			throwIueException( temp );
			return;
		}

		retCode = rsslDecodeElementEntry( &dIter, &rsslElementEntry );
	}

	if ( retCode != RSSL_RET_END_OF_CONTAINER )
	{
		EmaString temp( "Error decoding ElementList while compiling a search list." );
		throwIueException( temp );
	}
}

const Data& ElementListDecoder::getLoad() const
{
	return *_pLoad;
}

Data** ElementListDecoder::getLoadPtr()
{
	return &_pLoad;
}

void ElementListDecoder::setAtExit()
{
}

const EmaString& ElementListDecoder::getName()
{
	_name.setInt( _rsslElementEntry.name.data, _rsslElementEntry.name.length, false );

	return _name.toString();
}

bool ElementListDecoder::decodingStarted() const
{
	return _decodingStarted;
}

const EmaBuffer& ElementListDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslElementListBuffer.data, _rsslElementListBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& ElementListDecoder::getRsslBuffer() const
{
	return _rsslElementListBuffer;
}

OmmError::ErrorCode ElementListDecoder::getErrorCode() const
{
	return _errorCode;
}
