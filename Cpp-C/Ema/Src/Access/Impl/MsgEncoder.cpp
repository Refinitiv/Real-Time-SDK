/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ExceptionTranslator.h"

#include "MsgImpl.h"
#include "MsgEncoder.h"

using namespace refinitiv::ema::access;

MsgEncoder::MsgEncoder(MsgImpl* pMsgImpl) :
  Encoder(),
  _pMsgImpl(pMsgImpl)
{
}

MsgEncoder::~MsgEncoder()
{
	releaseEncIterator();
}

RsslBuffer& MsgEncoder::getEncodedBuffer() const
{
	UInt32 msgSize = rsslSizeOfMsg( _pMsgImpl->getRsslMsg(), RSSL_CMF_ALL_FLAGS );

	_pEncodeIter->clear( msgSize );

	RsslRet retCode = rsslEncodeMsg( &_pEncodeIter->_rsslEncIter, _pMsgImpl->getRsslMsg() );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "Failed to encode message. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'" );

		throwIueException( temp, retCode );
		return Encoder::getEncodedBuffer();
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	return Encoder::getEncodedBuffer();
}
