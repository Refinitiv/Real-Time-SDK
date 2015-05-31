/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmIntDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmIntDecoder::OmmIntDecoder() :
 _rsslBuffer(),
 _rsslInt( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
}

OmmIntDecoder::~OmmIntDecoder()
{
}

Data::DataCode OmmIntDecoder::getCode() const
{
	return _dataCode;
}

void OmmIntDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmIntDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmIntDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeInt( dIter, &_rsslInt );

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
			EmaString temp( "Failed to decode OmmInt. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmIntDecoder::toString()
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
		RsslRet retCode = rsslPrimitiveToString( &_rsslInt, RSSL_DT_INT, &tempBuffer );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString text( "Failed to convert OmmInt to string. Reason: " );
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

Int64 OmmIntDecoder::getInt() const
{
	return _rsslInt;
}

const EmaBuffer& OmmIntDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
