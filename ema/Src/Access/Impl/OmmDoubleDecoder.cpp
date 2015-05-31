/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDoubleDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmDoubleDecoder::OmmDoubleDecoder() :
 _rsslBuffer(),
 _rsslDouble( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
}

OmmDoubleDecoder::~OmmDoubleDecoder()
{
}

Data::DataCode OmmDoubleDecoder::getCode() const
{
	return _dataCode;
}

void OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmDoubleDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeDouble( dIter, &_rsslDouble );

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
			EmaString temp( "Failed to decode OmmDouble. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmDoubleDecoder::toString()
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
		RsslRet retCode = rsslPrimitiveToString( &_rsslDouble, RSSL_DT_DOUBLE, &tempBuffer );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString text( "Failed to convert OmmDouble to string. Reason: " );
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

double OmmDoubleDecoder::getDouble() const
{
	return _rsslDouble;
}

const EmaBuffer& OmmDoubleDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
