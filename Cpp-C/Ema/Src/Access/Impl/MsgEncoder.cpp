/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "MsgEncoder.h"
#include "rtr/rsslMsgEncoders.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

MsgEncoder::MsgEncoder() :
#ifdef __EMA_COPY_ON_SET__
 _name(),
 _serviceName(),
 _attrib(),
 _payload(),
 _extendedHeader(),
 _nameSet( false ),
 _serviceNameSet( false ),
 _serviceListNameSet( false ),
 _extendedHeaderSet( false ),
 _serviceListName(),
#else
 _pName( 0 ),
 _pServiceName( 0 ),
 _pServiceListName( 0 ),
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
	Encoder::clearEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = false;
	_serviceNameSet = false;
	_extendedHeaderSet = false;
	_serviceListNameSet = false;
#else
	_pName = 0;
	_pServiceName = 0;
	_pServiceListName = 0;
	_pAttrib = 0;
	_pPayload = 0;
	_pExtendedHeader = 0;
#endif
	_attribDataType = RSSL_DT_NO_DATA;
	_payloadDataType = RSSL_DT_NO_DATA;
}

void MsgEncoder::release()
{
	Encoder::releaseEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = false;
	_serviceNameSet = false;
	_extendedHeaderSet = false;
	_serviceListNameSet = false;
#else
	_pName = 0;
	_pServiceName = 0;
	_pServiceListName = 0;
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
	if (hasServiceId() || hasServiceListName())
	{
		EmaString text( "Attempt to set serviceName while service id or service list name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

#ifdef __EMA_COPY_ON_SET__
	_serviceNameSet = serviceName.length() ? true : false;
	_serviceName = serviceName;
#else
	_pServiceName = &serviceName;
#endif
}

void MsgEncoder::serviceListName(const EmaString& serviceListName)
{
	if (hasServiceId() || hasServiceName())
	{
		EmaString text("Attempt to set serviceListName while service id or service name is already set.");
		throwIueException(text, OmmInvalidUsageException::InvalidOperationEnum);
		return;
	}

#ifdef __EMA_COPY_ON_SET__
	_serviceListNameSet = serviceListName.length() ? true : false;
	_serviceListName = serviceListName;
#else
	_pServiceListName = &serviceListName;
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

const EmaString& MsgEncoder::getServiceListName() const
{
#ifdef __EMA_COPY_ON_SET__
	return _serviceListName;
#else
	return *_pServiceListName;
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


bool MsgEncoder::hasServiceListName() const
{
#ifdef __EMA_COPY_ON_SET__
	return _serviceListNameSet;
#else
	return _pServiceListName && !_pServiceListName->empty() ? true : false;
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

		throwIueException( temp, retCode );
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
