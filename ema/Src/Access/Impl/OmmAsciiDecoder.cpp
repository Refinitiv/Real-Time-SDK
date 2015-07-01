/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmAsciiDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmAsciiDecoder::OmmAsciiDecoder() :
 _rsslBuffer(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmAsciiDecoder::~OmmAsciiDecoder()
{
}

Data::DataCode OmmAsciiDecoder::getCode() const
{
	return _dataCode;
}

void OmmAsciiDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmAsciiDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmAsciiDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( rsslDecodeBuffer( dIter, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

const EmaString& OmmAsciiDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
		_toString.setInt( "(blank data)", 12, true );
	else
		_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmAsciiDecoder::getAscii()
{
	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaBuffer& OmmAsciiDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
