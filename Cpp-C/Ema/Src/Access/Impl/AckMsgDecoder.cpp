/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "AckMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

AckMsgDecoder::AckMsgDecoder() :
	MsgDecoder(),
	_name(),
	_serviceName(),
	_text(),
	_extHeader(),
	_hexBuffer(),
	_serviceNameSet( false ),
	_errorCode( OmmError::NoErrorEnum )
{
}

AckMsgDecoder::~AckMsgDecoder()
{
}

bool AckMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* pRsslDictionary )
{
	_serviceNameSet = false;

	_pRsslMsg = rsslMsg;

	_pRsslDictionary = pRsslDictionary;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	StaticDecoder::setRsslData( &_attrib, &_pRsslMsg->msgBase.msgKey.encAttrib,
	                            hasAttrib() ? _pRsslMsg->msgBase.msgKey.attribContainerType : RSSL_DT_NO_DATA, majVer, minVer, _pRsslDictionary );

	StaticDecoder::setRsslData( &_payload, &_pRsslMsg->msgBase.encDataBody,
	                            _pRsslMsg->msgBase.containerType, majVer, minVer, _pRsslDictionary );

	_errorCode = OmmError::NoErrorEnum;

	return true;
}

bool AckMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* pRsslDictionary, void* )
{
	_serviceNameSet = false;

	rsslClearMsg( &_rsslMsg );

	_pRsslMsg = &_rsslMsg;

	_pRsslDictionary = pRsslDictionary;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator( &decodeIter );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, rsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, _rsslMajVer, _rsslMinVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslDecodeMsg( &decodeIter, _pRsslMsg );

	switch ( retCode )
	{
	case RSSL_RET_SUCCESS :
		_errorCode = OmmError::NoErrorEnum;
		StaticDecoder::setRsslData( &_attrib, &_pRsslMsg->msgBase.msgKey.encAttrib,
		                            hasAttrib() ? _pRsslMsg->msgBase.msgKey.attribContainerType : RSSL_DT_NO_DATA, majVer, minVer, _pRsslDictionary );
		StaticDecoder::setRsslData( &_payload, &_pRsslMsg->msgBase.encDataBody, _pRsslMsg->msgBase.containerType, majVer, minVer, _pRsslDictionary );
		return true;
	case RSSL_RET_ITERATOR_OVERRUN :
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA :
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default :
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
}

bool AckMsgDecoder::setRsslData( RsslDecodeIterator*, RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool AckMsgDecoder::hasMsgKey() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) ? true : false;
}

bool AckMsgDecoder::hasName() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool AckMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool AckMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool AckMsgDecoder::hasId() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool AckMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool AckMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) &&
	       ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool AckMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool AckMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool AckMsgDecoder::hasSeqNum() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM ) ? true : false;
}

bool AckMsgDecoder::hasNackCode() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE ) ? true : false;
}

bool AckMsgDecoder::hasText() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT ) ? true : false;
}

bool AckMsgDecoder::hasServiceName() const
{
	return _serviceNameSet;
}

Int32 AckMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 AckMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& AckMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 AckMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 AckMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

const EmaString& AckMsgDecoder::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to getServiceName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceName.toString();
}

Int32 AckMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 AckMsgDecoder::getAckId() const
{
	return _pRsslMsg->ackMsg.ackId;
}

UInt32 AckMsgDecoder::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->ackMsg.seqNum;
}

UInt32 AckMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

UInt8 AckMsgDecoder::getNackCode() const
{
	if ( !hasNackCode() )
	{
		EmaString temp( "Attempt to getNackCode() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->ackMsg.nakCode;
}

const EmaBuffer& AckMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( _pRsslMsg->ackMsg.extendedHeader.data, _pRsslMsg->ackMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

void AckMsgDecoder::setServiceName( const char* serviceName, UInt32 length, bool nullTerm )
{
	_serviceNameSet = length ? true : false;

	_serviceName.setInt( serviceName, length, nullTerm );
}

void AckMsgDecoder::setServiceId(UInt16 serviceId)
{
	_pRsslMsg->ackMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
	_pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	_pRsslMsg->msgBase.msgKey.serviceId = serviceId;
}

const EmaString& AckMsgDecoder::getText() const
{
	if ( !hasText() )
	{
		EmaString temp( "Attempt to getText() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_text.setInt( _pRsslMsg->ackMsg.text.data, _pRsslMsg->ackMsg.text.length, false );

	return _text.toString();
}

bool AckMsgDecoder::getPrivateStream() const
{
	return ( _pRsslMsg->ackMsg.flags & RSSL_AKMF_PRIVATE_STREAM ) ? true : false;
}

const EmaBuffer& AckMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& AckMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode AckMsgDecoder::getErrorCode() const
{
	return _errorCode;
}
