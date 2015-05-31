/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmRealDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmRealDecoder::OmmRealDecoder() :
 _rsslBuffer(),
 _rsslReal(),
 _toDouble( 0 ),
 _toString(),
 _hexBuffer(),
 _toDoubleSet( false ),
 _toStringSet( false )
{
	rsslClearReal( &_rsslReal );
}

OmmRealDecoder::~OmmRealDecoder()
{
}

Data::DataCode OmmRealDecoder::getCode() const
{
	return _rsslReal.isBlank == RSSL_TRUE ? Data::BlankEnum : Data::NoCodeEnum;
}

Int8 OmmRealDecoder::getRsslIsBlank() const
{
	return _rsslReal.isBlank;
}

UInt8 OmmRealDecoder::getRsslHint() const
{
	return _rsslReal.hint;
}

void OmmRealDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmRealDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmRealDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toDoubleSet = false;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeReal( dIter, &_rsslReal );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_rsslReal.isBlank = RSSL_TRUE;
		break;
	case RSSL_RET_SUCCESS :
		_rsslReal.isBlank = RSSL_FALSE;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
	default :
		{
			_rsslReal.isBlank = RSSL_TRUE;
			EmaString temp( "Failed to decode OmmReal. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmRealDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _rsslReal.isBlank == RSSL_TRUE )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char realString[256];
		RsslBuffer tempRsslBuffer;
		tempRsslBuffer.data = realString;
		tempRsslBuffer.length = 256;
		RsslRet retCode = rsslRealToString( &tempRsslBuffer, &_rsslReal );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString temp( "Failed to convert OmmReal to string. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		else
		{
			_toStringSet = true;
			_toString.set( realString );
		}
	}

	return _toString;
}

Int64 OmmRealDecoder::getMantissa() const
{
	return _rsslReal.value;
}

OmmReal::MagnitudeType OmmRealDecoder::getMagnitudeType() const
{
	return static_cast< OmmReal::MagnitudeType >( _rsslReal.hint );
}

double OmmRealDecoder::toDouble()
{
	if ( !_toDoubleSet )
	{
		_toDoubleSet = true;

		rsslRealToDouble( &_toDouble, &_rsslReal );
	}

	return _toDouble;
}

const EmaBuffer& OmmRealDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
