/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBufferDecoder.h"
#include "ExceptionTranslator.h"
#include <stdlib.h>

using namespace thomsonreuters::ema::access;

OmmBufferDecoder::OmmBufferDecoder() :
 _rsslBuffer(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmBufferDecoder::~OmmBufferDecoder()
{
}

Data::DataCode OmmBufferDecoder::getCode() const
{
	return _dataCode;
}

void OmmBufferDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmBufferDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmBufferDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	if ( rsslDecodeBuffer( dIter, &_rsslBuffer ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
		_dataCode = Data::BlankEnum;
}

const EmaString& OmmBufferDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	const char* tempStr = getBuffer().operator const char *();
	UInt32 length = strlen( tempStr );
	_toString.setInt( tempStr, length, true );
	return _toString.toString();
}

const EmaBuffer& OmmBufferDecoder::getBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );
	
	return _hexBuffer.toBuffer();
}
