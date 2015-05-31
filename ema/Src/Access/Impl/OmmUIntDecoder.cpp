/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUIntDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmUIntDecoder::OmmUIntDecoder() :
 _rsslBuffer(),
 _rsslUInt( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
}

OmmUIntDecoder::~OmmUIntDecoder()
{
}

Data::DataCode OmmUIntDecoder::getCode() const
{
	return _dataCode;
}

void OmmUIntDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmUIntDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmUIntDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeUInt( dIter, &_rsslUInt );

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
			EmaString temp( "Failed to decode OmmUInt. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmUIntDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _dataCode == Data::BlankEnum )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char uintegerString[256];
		RsslBuffer tempRsslBuffer;
		tempRsslBuffer.data = uintegerString;
		tempRsslBuffer.length = 256;
		RsslRet retCode = rsslPrimitiveToString( &_rsslUInt, RSSL_DT_UINT, &tempRsslBuffer );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString temp( "Failed to convert OmmUInt to string. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		else
		{
			_toStringSet = true;
			_toString.set( uintegerString );
		}
	}

	return _toString;
}

UInt64 OmmUIntDecoder::getUInt() const
{
	return _rsslUInt;
}

const EmaBuffer& OmmUIntDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
