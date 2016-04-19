/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmArrayDecoder.h"
#include "StaticDecoder.h"
#include "Utilities.h"

using namespace thomsonreuters::ema::access;

OmmArrayDecoder::OmmArrayDecoder() :
 _rsslArray(),
 _rsslArrayBuffer(),
 _rsslEntryBuffer(),
 _decodeIter(),
 _load(),
 _hexBuffer(),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION ),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum ),
 _decodingStarted( false ),
 _atEnd( false )
{
}

OmmArrayDecoder::~OmmArrayDecoder()
{
	StaticDecoder::morph( &_load, DataType::NoDataEnum );
}

Data::DataCode OmmArrayDecoder::getCode() const
{
	return _dataCode;
}

void OmmArrayDecoder::reset()
{
	_decodingStarted = false;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslArrayBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeArray( &_decodeIter, &_rsslArray );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_atEnd = true;
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorOverrunEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IncompleteDataEnum;
		break;
	default :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}
}

void OmmArrayDecoder::clone( const OmmArrayDecoder& other )
{
	_decodingStarted = false;

	_rsslMajVer = other._rsslMajVer;

	_rsslMinVer = other._rsslMinVer;

	_rsslArrayBuffer = other._rsslArrayBuffer;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &other._rsslArrayBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return;
	}

	retCode = rsslDecodeArray( &_decodeIter, &_rsslArray );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_atEnd = true;
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		break;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorOverrunEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IncompleteDataEnum;
		break;
	default :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::UnknownErrorEnum;
		break;
	}
}

bool OmmArrayDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_decodingStarted = false;

	_rsslMajVer = dIter->_majorVersion;

	_rsslMinVer = dIter->_minorVersion;

	_rsslArrayBuffer = *rsslBuffer;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslArrayBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslDecodeArray( &_decodeIter, &_rsslArray );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_atEnd = true;
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
}

bool OmmArrayDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_dataCode = Data::BlankEnum;
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmArrayDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* , void* )
{
	_decodingStarted = false;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	_rsslArrayBuffer = *rsslBuffer;

	rsslClearDecodeIterator( &_decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &_decodeIter, &_rsslArrayBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &_decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslDecodeArray( &_decodeIter, &_rsslArray );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_atEnd = true;
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_SUCCESS :
		_atEnd = false;
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_ITERATOR_OVERRUN :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default :
		_atEnd = false;
		_dataCode = _rsslArrayBuffer.length ? Data::NoCodeEnum : Data::BlankEnum;
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
}

bool OmmArrayDecoder::hasFixedWidth() const
{
	return _rsslArray.itemLength ? true : false;
}

UInt16 OmmArrayDecoder::getFixedWidth() const
{
	return _rsslArray.itemLength;
}

RsslPrimitiveType OmmArrayDecoder::getRsslDataType() const
{
	return _rsslArray.primitiveType;
}

bool OmmArrayDecoder::getNextData()
{
	if ( _atEnd ) return true;

	_decodingStarted = true;

	RsslRet retCode = rsslDecodeArrayEntry( &_decodeIter, &_rsslEntryBuffer );
	
	switch ( retCode )
	{
	case RSSL_RET_END_OF_CONTAINER :
		_atEnd = true;
		return true;
	case RSSL_RET_SUCCESS :
		Decoder::setRsslData( &_load, _rsslArray.primitiveType, &_decodeIter, &_rsslEntryBuffer, 0, 0 ); 
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		Decoder::setRsslData( &_load, OmmError::IncompleteDataEnum, &_decodeIter, &_rsslEntryBuffer ); 
		return false;
	case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		Decoder::setRsslData( &_load, OmmError::UnsupportedDataTypeEnum, &_decodeIter, &_rsslEntryBuffer ); 
		return false;
	default :
		Decoder::setRsslData( &_load, OmmError::UnknownErrorEnum, &_decodeIter, &_rsslEntryBuffer );
		return false;
	}
}

const Data& OmmArrayDecoder::getLoad() const
{
	return _load;
}

void OmmArrayDecoder::setAtExit()
{
}

DataType::DataTypeEnum OmmArrayDecoder::getLoadDataType() const
{
	return dataType[ _rsslArray.primitiveType ];
}

bool OmmArrayDecoder::decodingStarted() const
{
	return _decodingStarted;
}

const EmaBuffer& OmmArrayDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslArrayBuffer.data, _rsslArrayBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmArrayDecoder::getRsslBuffer() const
{
	return _rsslArrayBuffer;
}

OmmError::ErrorCode OmmArrayDecoder::getErrorCode() const
{
	return _errorCode;
}
