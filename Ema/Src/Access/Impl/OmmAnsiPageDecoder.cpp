/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmAnsiPageDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmAnsiPageDecoder::OmmAnsiPageDecoder() :
_rsslBuffer(),
_toString(),
_getString(),
_getBuffer(),
_dataCode( Data::BlankEnum )
{
}

OmmAnsiPageDecoder::~OmmAnsiPageDecoder()
{
}

Data::DataCode OmmAnsiPageDecoder::getCode() const
{
	return _dataCode;
}

void OmmAnsiPageDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( rsslDecodeBuffer( dIter, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

void OmmAnsiPageDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmAnsiPageDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer,
								const RsslDataDictionary* rsslDictionary , void* )
{
	RsslDecodeIterator decodeIterator;
	rsslClearDecodeIterator( &decodeIterator );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIterator, rsslBuffer );
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

	if ( rsslDecodeBuffer( &decodeIterator, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

const EmaString& OmmAnsiPageDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmAnsiPageDecoder::getString()
{
	_getString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _getString.toString();
}

const EmaBuffer& OmmAnsiPageDecoder::getBuffer()
{
	_getBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );
	
	return _getBuffer.toBuffer();
}
