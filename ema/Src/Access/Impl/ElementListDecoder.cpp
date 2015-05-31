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
 _load(),
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
	rsslClearElementList( &_rsslElementList );
 }

ElementListDecoder::~ElementListDecoder()
{
	StaticDecoder::morph( &_load, DataType::NoDataEnum );
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

void ElementListDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
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

void ElementListDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void ElementListDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer,
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

bool ElementListDecoder::getNextData()
{
	if ( _atEnd ) return true;

	if ( !_decodingStarted && _errorCode != OmmError::NoErrorEnum )
	{
		_atEnd = true;
		_decodingStarted = true;
		Decoder::setRsslData( &_load, _errorCode, &_decodeIter, &_rsslElementListBuffer ); 
		return false;
	}

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeElementEntry( &_decodeIter, &_rsslElementEntry );

	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
		Decoder::setRsslData( &_load, _rsslElementEntry.dataType,
								&_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
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

		if ( !_decodingStarted && _errorCode != OmmError::NoErrorEnum )
		{
			_atEnd = true;
			_decodingStarted = true;
			Decoder::setRsslData( &_load, _errorCode, &_decodeIter, &_rsslElementListBuffer ); 
			return false;
		}

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
		Decoder::setRsslData( &_load, _rsslElementEntry.dataType,
								&_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
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

		if ( !_decodingStarted && _errorCode != OmmError::NoErrorEnum )
		{
			_atEnd = true;
			_decodingStarted = true;
			Decoder::setRsslData( &_load, _errorCode, &_decodeIter, &_rsslElementListBuffer );  
			return false;
		}

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
		Decoder::setRsslData( &_load, _rsslElementEntry.dataType,
								&_decodeIter, &_rsslElementEntry.encData, _pRsslDictionary, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslElementEntry.encData ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslElementEntry.encData );
		return false;
	}
}

bool ElementListDecoder::getNextData( const Data& data )
{
	if ( data.getDataType() != DataType::ElementListEnum )
	{
		_atEnd = true;
		return true;
	}

	RsslDataType listDataType = RSSL_DT_NO_DATA;
	EmaVector< EmaString > stringList;	

	if ( !decodeViewList( &data.getEncoder().getRsslBuffer(), listDataType, stringList ) )
	{
		_atEnd = true;
		return true;
	}

	switch ( listDataType )
	{
	case RSSL_DT_ASCII_STRING :
		return getNextData( stringList );
	default :
		_atEnd = true;
		return true;
	}
}

bool ElementListDecoder::decodeViewList( RsslBuffer* rsslBuffer, RsslDataType& rsslDataType, EmaVector< EmaString >& stringList )
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
	bool searchListCompiled = false;

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
						EmaString temp( "Error decoding OmmArray with strings while compiling search list." );

						// todo ...   logger output
					}
					else
					{
						searchListCompiled = true;
						rsslDataType = RSSL_DT_ASCII_STRING;
					}
				}
			}
			break;
		default :
			break;
		}

		retCode = rsslDecodeElementEntry( &dIter, &rsslElementEntry );
	}

	if ( retCode != RSSL_RET_END_OF_CONTAINER )
	{
		EmaString temp( "Error decoding ElementList while compiling a search list." );

		// todo logger output to be provided
		
		return false;
	}

	if ( !searchListCompiled )
	{
		EmaString temp( "Search list was not compiled." );

		// todo ... logger output to be provided
		return false;
	}

	return true;
}

const Data& ElementListDecoder::getLoad() const
{
	return _load;
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
