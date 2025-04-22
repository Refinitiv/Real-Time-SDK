/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019, 2025 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "RefreshMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmState.h"
#include "OmmQos.h"
#include "OmmInvalidUsageException.h"

#include "rtr/rsslMsgDecoders.h"

using namespace refinitiv::ema::access;

RefreshMsgDecoder::RefreshMsgDecoder() :
 MsgDecoder(),
 _name(),
 _serviceName(),
 _extHeader(),
 _permission(),
 _itemGroup(),
 _state(),
 _qos(),
 _hexBuffer(),
 _serviceNameSet( false ),
 _stateSet( false ),
 _qosSet( false ),
 _errorCode( OmmError::NoErrorEnum )
{
}

RefreshMsgDecoder::~RefreshMsgDecoder()
{
}

bool RefreshMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary )
{
	_serviceNameSet = false;

	_stateSet = false;

	_qosSet = false;

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

bool RefreshMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	_serviceNameSet = false;

	_stateSet = false;

	_qosSet = false;

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

bool RefreshMsgDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool RefreshMsgDecoder::hasMsgKey() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) ? true : false;
}

bool RefreshMsgDecoder::hasName() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool RefreshMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool RefreshMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool RefreshMsgDecoder::hasId() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool RefreshMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool RefreshMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool RefreshMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool RefreshMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool RefreshMsgDecoder::hasServiceName() const
{
	return _serviceNameSet;
}

Int32 RefreshMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 RefreshMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& RefreshMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 RefreshMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 RefreshMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

const EmaString& RefreshMsgDecoder::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to getServiceName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceName.toString();
}

Int32 RefreshMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 RefreshMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

const EmaBuffer& RefreshMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( _pRsslMsg->refreshMsg.extendedHeader.data, _pRsslMsg->refreshMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

bool RefreshMsgDecoder::hasQos() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_QOS ) ? true : false;
}

bool RefreshMsgDecoder::hasSeqNum() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM ) ? true : false;
}

bool RefreshMsgDecoder::hasPartNum() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM ) ? true : false;
}

bool RefreshMsgDecoder::hasPermissionData() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA ) ? true : false;
}

bool RefreshMsgDecoder::hasPublisherId() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO ) ? true : false;
}

void RefreshMsgDecoder::setStateInt() const
{
	if ( _stateSet ) return;

	_stateSet = true;

	StaticDecoder::setRsslData( &_state, &_pRsslMsg->refreshMsg.state );
}

void RefreshMsgDecoder::setQosInt() const
{
	if ( _qosSet ) return;

	_qosSet = true;

	StaticDecoder::setRsslData( &_qos, &_pRsslMsg->refreshMsg.qos );
}

const OmmState& RefreshMsgDecoder::getState() const
{
	setStateInt();

	return static_cast<const OmmState&>( static_cast<const Data&>( _state ) );
}

const OmmQos& RefreshMsgDecoder::getQos() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getQos() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	setQosInt();

	return static_cast<const OmmQos&>( static_cast<const Data&>( _qos ) );
}

UInt32 RefreshMsgDecoder::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->refreshMsg.seqNum;
}

UInt16 RefreshMsgDecoder::getPartNum() const
{
	if ( !hasPartNum() )
	{
		EmaString temp( "Attempt to getPartNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->refreshMsg.partNum;
}

const EmaBuffer& RefreshMsgDecoder::getItemGroup() const
{
	_itemGroup.setFromInt( _pRsslMsg->refreshMsg.groupId.data, _pRsslMsg->refreshMsg.groupId.length );

	return _itemGroup.toBuffer();
}

const EmaBuffer& RefreshMsgDecoder::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( _pRsslMsg->refreshMsg.permData.data, _pRsslMsg->refreshMsg.permData.length );

	return _permission.toBuffer();
}

UInt32 RefreshMsgDecoder::getPublisherIdUserId() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->refreshMsg.postUserInfo.postUserId;
}

UInt32 RefreshMsgDecoder::getPublisherIdUserAddress() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserAddress() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->refreshMsg.postUserInfo.postUserAddr;
}

bool RefreshMsgDecoder::getDoNotCache() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_DO_NOT_CACHE ) ? true : false;
}

bool RefreshMsgDecoder::getSolicited() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_SOLICITED ) ? true : false;
}

bool RefreshMsgDecoder::getComplete() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE ) ? true : false;
}

bool RefreshMsgDecoder::getClearCache() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_CLEAR_CACHE ) ? true : false;
}

bool RefreshMsgDecoder::getPrivateStream() const
{
	return ( _pRsslMsg->refreshMsg.flags & RSSL_RFMF_PRIVATE_STREAM ) ? true : false;
}

void RefreshMsgDecoder::setServiceName( const char* serviceName, UInt32 length, bool nullTerm )
{
	_serviceNameSet = length ? true : false;

	_serviceName.setInt( serviceName, length, nullTerm );
}

void RefreshMsgDecoder::setServiceId(UInt16 serviceId)
{
	_pRsslMsg->refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
	_pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	_pRsslMsg->msgBase.msgKey.serviceId = serviceId;
}


const EmaBuffer& RefreshMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& RefreshMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode RefreshMsgDecoder::getErrorCode() const
{
	return _errorCode;
}
