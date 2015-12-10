/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmRmtesDecoder.h"
#include "ExceptionTranslator.h"
#include "RmtesBufferImpl.h"

using namespace thomsonreuters::ema::access;

OmmRmtesDecoder::OmmRmtesDecoder() :
 _rmtesBuffer(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmRmtesDecoder::~OmmRmtesDecoder()
{
}

Data::DataCode OmmRmtesDecoder::getCode() const
{
	return _dataCode;
}

void OmmRmtesDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmRmtesDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmRmtesDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( _rmtesBuffer._pImpl->_applyToCache )
		_rmtesBuffer._pImpl->clear();

	if ( rsslDecodeBuffer( dIter, &_rmtesBuffer._pImpl->_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

const EmaString& OmmRmtesDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		_rmtesBuffer._pImpl->_toString.setInt( "(blank data)", 12, true );
		return _rmtesBuffer._pImpl->_toString.toString();
	}

	return _rmtesBuffer._pImpl->toString();
}

const EmaBuffer& OmmRmtesDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rmtesBuffer._pImpl->_rsslBuffer.data, _rmtesBuffer._pImpl->_rsslBuffer.length );

	return _hexBuffer.toBuffer();
}

const RmtesBuffer& OmmRmtesDecoder::getRmtes()
{
	return _rmtesBuffer;
}
