/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmErrorDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmErrorDecoder::OmmErrorDecoder() :
 _pRsslBuffer( 0 ),
 _errorCode( OmmError::NoErrorEnum ),
 _toHex()
{
}

OmmErrorDecoder::~OmmErrorDecoder()
{
}

OmmError::ErrorCode OmmErrorDecoder::getErrorCode() const
{
	return _errorCode;
}

void OmmErrorDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmErrorDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* pRsslBuffer, const RsslDataDictionary* error , void* )
{
	_pRsslBuffer = pRsslBuffer;

	_errorCode = (OmmError::ErrorCode)(UInt64)error;
}

void OmmErrorDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
}

const EmaBuffer& OmmErrorDecoder::getAsHex()
{
	_toHex.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _toHex.toBuffer();
}
