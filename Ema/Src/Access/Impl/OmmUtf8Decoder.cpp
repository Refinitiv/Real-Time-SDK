/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUtf8Decoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmUtf8Decoder::OmmUtf8Decoder() :
 _rsslBuffer(),
 _toString(),
 _utf8Buffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmUtf8Decoder::~OmmUtf8Decoder()
{
}

Data::DataCode OmmUtf8Decoder::getCode() const
{
	return _dataCode;
}

void OmmUtf8Decoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmUtf8Decoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmUtf8Decoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( rsslDecodeBuffer( dIter, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

const EmaString& OmmUtf8Decoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
		_toString.setInt( "(blank data)", 12, true );
	else
		_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaBuffer& OmmUtf8Decoder::getUtf8()
{
	_utf8Buffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _utf8Buffer.toBuffer();
}
