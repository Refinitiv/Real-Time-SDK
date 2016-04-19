/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "MsgEncoder.h"
#include "rtr/rsslMsgEncoders.h"

using namespace thomsonreuters::ema::access;

MsgEncoder::MsgEncoder() :
#ifdef __EMA_COPY_ON_SET__
 _name(),
 _serviceName(),
 _attrib(),
 _payload(),
 _extendedHeader(),
 _nameSet( false ),
 _serviceNameSet( false ),
 _extendedHeaderSet( false ),
#else
 _pName( 0 ),
 _pServiceName( 0 ),
 _pAttrib( 0 ),
 _pPayload( 0 ),
 _pExtendedHeader( 0 ),
#endif
 _attribDataType( RSSL_DT_NO_DATA ),
 _payloadDataType( RSSL_DT_NO_DATA )
{
}

MsgEncoder::~MsgEncoder()
{
}

void MsgEncoder::clear()
{
	Encoder::releaseEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = false;
	_serviceNameSet = false;
	_extendedHeaderSet = false;
#else
	_pName = 0;
	_pServiceName = 0;
	_pAttrib = 0;
	_pPayload = 0;
	_pExtendedHeader = 0;
#endif
	_attribDataType = RSSL_DT_NO_DATA;
	_payloadDataType = RSSL_DT_NO_DATA;
}

bool MsgEncoder::ownsIterator() const
{
	return false;
}

void MsgEncoder::serviceName( const EmaString& serviceName )
{
	if ( hasServiceId() )
	{
		EmaString text( "Attempt to set serviceName while service id is already set." );
		throwIueException( text );
		return;
	}

#ifdef __EMA_COPY_ON_SET__
	_serviceNameSet = serviceName.length() ? true : false;
	_serviceName = serviceName;
#else
	_pServiceName = &serviceName;
#endif
}

const EmaString& MsgEncoder::getServiceName() const
{
#ifdef __EMA_COPY_ON_SET__
	return _serviceName;
#else
	return *_pServiceName;
#endif
}

bool MsgEncoder::hasServiceName() const
{
#ifdef __EMA_COPY_ON_SET__
	return _serviceNameSet;
#else
	return _pServiceName && !_pServiceName->empty() ? true : false;
#endif
}

bool MsgEncoder::hasName() const
{
#ifdef __EMA_COPY_ON_SET__
	return _nameSet;
#else
	return _pName ? true : false;
#endif
}

RsslBuffer& MsgEncoder::getRsslBuffer() const
{
	RsslMsg* pMsg = getRsslMsg();

	UInt32 msgSize = rsslSizeOfMsg( pMsg, RSSL_CMF_ALL_FLAGS );

	_pEncodeIter->reallocate( msgSize );

	RsslRet retCode = rsslEncodeMsg( &_pEncodeIter->_rsslEncIter, pMsg );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "Failed to encode message. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'" );

		throwIueException( temp );
		return Encoder::getRsslBuffer();
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	return Encoder::getRsslBuffer();
}

void MsgEncoder::endEncodingEntry() const
{
}

bool MsgEncoder::isComplete() const
{
	return true;
}
