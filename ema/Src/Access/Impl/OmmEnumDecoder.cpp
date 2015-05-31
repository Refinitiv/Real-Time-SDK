/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmEnumDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmEnumDecoder::OmmEnumDecoder() :
 _rsslBuffer(),
 _rsslEnum( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
}

OmmEnumDecoder::~OmmEnumDecoder()
{
}

Data::DataCode OmmEnumDecoder::getCode() const
{
	return _dataCode;
}

void OmmEnumDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmEnumDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmEnumDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeEnum( dIter, &_rsslEnum );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		break;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
	default :
		{
			_dataCode = Data::BlankEnum;
			EmaString temp( "Failed to decode OmmEnum. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmEnumDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _dataCode == Data::BlankEnum )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char temp[256];
		RsslBuffer tempBuffer;
		tempBuffer.length = 256;
		tempBuffer.data = temp;
		RsslRet retCode = rsslPrimitiveToString( &_rsslEnum, RSSL_DT_ENUM, &tempBuffer );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString text( "Failed to convert OmmEnum to string. Reason: " );
			text += rsslRetCodeToString( retCode );
			throwIueException( text );
		}
		else
		{
			_toStringSet = true;
			_toString.set( temp );
		}
	}

	return _toString;
}

UInt16 OmmEnumDecoder::getEnum() const
{
	return _rsslEnum;
}

const EmaBuffer& OmmEnumDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
