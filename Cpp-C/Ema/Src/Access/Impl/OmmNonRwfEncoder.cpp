/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNonRwfEncoder.h"
#include "rtr/rsslDataUtils.h"

using namespace thomsonreuters::ema::access;

OmmNonRwfEncoder::OmmNonRwfEncoder()
{
}

OmmNonRwfEncoder::~OmmNonRwfEncoder()
{
}

void OmmNonRwfEncoder::endEncodingEntry() const
{
}

void OmmNonRwfEncoder::encodeBuffer( const char* data, UInt32 length )
{
	if ( !_pEncodeIter )
	{
		acquireEncIterator();
	}
	else
	{
		_pEncodeIter->clear();
	}

	RsslBuffer rsslBuffer;
	RsslRet retCode = rsslEncodeNonRWFDataTypeInit( &_pEncodeIter->_rsslEncIter, &rsslBuffer );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal failure #1 in OmmNonRwfEncoder::encodeBuffer(). Reason = " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return;
	}

	while ( rsslBuffer.length < length )
	{
		_pEncodeIter->reallocate();
		retCode = rsslEncodeNonRWFDataTypeInit( &_pEncodeIter->_rsslEncIter, &rsslBuffer );
	}

	memcpy( rsslBuffer.data, data, length );

	rsslBuffer.length = length;

	retCode = rsslEncodeNonRWFDataTypeComplete( &_pEncodeIter->_rsslEncIter, &rsslBuffer, RSSL_TRUE );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeNonRWFDataTypeComplete( &_pEncodeIter->_rsslEncIter, &rsslBuffer, RSSL_TRUE );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal failure #2 in OmmNonRwfEncoder::encodeBuffer(). Reason = " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &_pEncodeIter->_rsslEncIter ) );
}
