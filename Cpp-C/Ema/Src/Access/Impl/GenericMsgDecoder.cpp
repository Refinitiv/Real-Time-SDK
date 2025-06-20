/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GenericMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

GenericMsgDecoder::GenericMsgDecoder() :
 MsgDecoder(),
 _name(),
 _extHeader(),
 _permission(),
 _hexBuffer(),
 _errorCode( OmmError::NoErrorEnum )
{
}

GenericMsgDecoder::~GenericMsgDecoder()
{
}

void GenericMsgDecoder::cloneMsgKey(const Msg& other)
{
	RsslGenericMsg* pRsslGenericMsg = (RsslGenericMsg*)_pRsslMsg;

	rsslClearMsgKey(&pRsslGenericMsg->msgBase.msgKey);

	pRsslGenericMsg->flags |= RSSL_GNMF_HAS_MSG_KEY;

	MsgDecoder::cloneMsgKey(other, &pRsslGenericMsg->msgBase.msgKey);
}

bool GenericMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary )
{
	_pRsslMsg = rsslMsg;

	_pRsslDictionary = rsslDictionary;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	StaticDecoder::setRsslData( &_attrib, &_pRsslMsg->msgBase.msgKey.encAttrib,
		hasAttrib() ? _pRsslMsg->msgBase.msgKey.attribContainerType : RSSL_DT_NO_DATA, majVer, minVer, _pRsslDictionary );

	StaticDecoder::setRsslData( &_payload, &_pRsslMsg->msgBase.encDataBody, _pRsslMsg->msgBase.containerType, majVer, minVer, _pRsslDictionary );

	_errorCode = OmmError::NoErrorEnum;

	return true;
}

bool GenericMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	rsslClearMsg( &_rsslMsg );

	_pRsslMsg = &_rsslMsg;

	_pRsslDictionary = rsslDictionary;

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

bool GenericMsgDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool GenericMsgDecoder::hasMsgKey() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) ? true : false;
}

bool GenericMsgDecoder::hasName() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool GenericMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool GenericMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool GenericMsgDecoder::hasId() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool GenericMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool GenericMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool GenericMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool GenericMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool GenericMsgDecoder::hasSeqNum() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_SEQ_NUM ) ? true : false;
}

bool GenericMsgDecoder::hasSecondarySeqNum() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM ) ? true : false;
}

bool GenericMsgDecoder::hasPermissionData() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA ) ? true : false;
}

bool GenericMsgDecoder::hasPartNum() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_HAS_PART_NUM ) ? true : false;
}

Int32 GenericMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 GenericMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& GenericMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 GenericMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 GenericMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

Int32 GenericMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 GenericMsgDecoder::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->genericMsg.seqNum;
}

UInt32 GenericMsgDecoder::getSecondarySeqNum() const
{
	if ( !hasSecondarySeqNum() )
	{
		EmaString temp( "Attempt to getSecondarySeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->genericMsg.secondarySeqNum;
}

UInt16 GenericMsgDecoder::getPartNum() const
{
	if ( !hasPartNum() )
	{
		EmaString temp( "Attempt to getPartNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->genericMsg.partNum;
}

bool GenericMsgDecoder::getComplete() const
{
	return ( _pRsslMsg->genericMsg.flags & RSSL_GNMF_MESSAGE_COMPLETE ) ? true : false;
}

const EmaBuffer& GenericMsgDecoder::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( _pRsslMsg->genericMsg.permData.data, _pRsslMsg->genericMsg.permData.length );

	return _permission.toBuffer();
}

UInt32 GenericMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

const EmaBuffer& GenericMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( _pRsslMsg->genericMsg.extendedHeader.data, _pRsslMsg->genericMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

const EmaBuffer& GenericMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& GenericMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode GenericMsgDecoder::getErrorCode() const
{
	return _errorCode;
}
