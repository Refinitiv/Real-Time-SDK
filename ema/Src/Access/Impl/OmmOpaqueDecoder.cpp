/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmOpaqueDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmOpaqueDecoder::OmmOpaqueDecoder() :
 _rsslBuffer(),
 _toString(),
 _getString(),
 _getBuffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmOpaqueDecoder::~OmmOpaqueDecoder()
{
}

Data::DataCode OmmOpaqueDecoder::getCode() const
{
	return _dataCode;
}

void OmmOpaqueDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( rsslDecodeBuffer( dIter, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

void OmmOpaqueDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmOpaqueDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* pRsslBuffer,
								const RsslDataDictionary* rsslDictionary , void* )
{
	RsslDecodeIterator decodeIterator;
	rsslClearDecodeIterator( &decodeIterator );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIterator, pRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_dataCode = Data::BlankEnum;
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIterator, majVer, minVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_dataCode = Data::BlankEnum;
		return;
	}

	retCode = rsslDecodeBuffer( &decodeIterator, &_rsslBuffer );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		break;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		break;
	default :
		_dataCode = Data::BlankEnum;
		break;
	}
}

const EmaString& OmmOpaqueDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
		_toString.setInt( "(blank data)", 12, true );
	else
		_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmOpaqueDecoder::getString()
{
	_getString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _getString.toString();
}

const EmaBuffer& OmmOpaqueDecoder::getBuffer()
{
	_getBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _getBuffer.toBuffer();
}
