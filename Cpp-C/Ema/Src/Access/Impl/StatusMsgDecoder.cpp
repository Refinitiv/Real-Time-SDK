/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright Thomson Reuters 2015, 2019. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#include "StatusMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmState.h"
#include "OmmInvalidUsageException.h"

#include "rtr/rsslMsgDecoders.h"

using namespace thomsonreuters::ema::access;

StatusMsgDecoder::StatusMsgDecoder() :
 MsgDecoder(),
 _name(),
 _serviceName(),
 _extHeader(),
 _permission(),
 _itemGroup(),
 _state(),
 _hexBuffer(),
 _serviceNameSet( false ),
 _stateSet( false ),
 _errorCode( OmmError::NoErrorEnum )
{
}

StatusMsgDecoder::~StatusMsgDecoder()
{
}

bool StatusMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary )
{
	_serviceNameSet = false;

	_stateSet = false;

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

bool StatusMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	_serviceNameSet = false;

	_stateSet = false;

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

bool StatusMsgDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool StatusMsgDecoder::hasMsgKey() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) ? true : false;
}

bool StatusMsgDecoder::hasName() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool StatusMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool StatusMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool StatusMsgDecoder::hasId() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool StatusMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool StatusMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool StatusMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool StatusMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool StatusMsgDecoder::hasServiceName() const
{
	return _serviceNameSet;
}

Int32 StatusMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 StatusMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& StatusMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 StatusMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 StatusMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

const EmaString& StatusMsgDecoder::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to getServiceName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceName.toString();
}

Int32 StatusMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 StatusMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

const EmaBuffer& StatusMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( _pRsslMsg->statusMsg.extendedHeader.data, _pRsslMsg->statusMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

bool StatusMsgDecoder::hasItemGroup() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID ) ? true : false;
}

bool StatusMsgDecoder::hasPermissionData() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA ) ? true : false;
}

bool StatusMsgDecoder::hasPublisherId() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO ) ? true : false;
}

bool StatusMsgDecoder::hasState() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE ) ? true : false;
}

void StatusMsgDecoder::setStateInt() const
{
	if ( _stateSet ) return;

	_stateSet = true;

	RsslState rsslState;

	if ( rsslStatusMsgCheckHasState( &_pRsslMsg->statusMsg ) )
		rsslState = _pRsslMsg->statusMsg.state;
	else
	{
		rsslState.code = RSSL_SC_NONE;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslClearBuffer( &rsslState.text );
	}

	StaticDecoder::setRsslData( &_state, &rsslState );
}

const OmmState& StatusMsgDecoder::getState() const
{
	if ( !hasState() )
	{
		EmaString temp( "Attempt to getState() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	setStateInt();

	return static_cast<const OmmState&>( static_cast<const Data&>( _state ) );
}

const EmaBuffer& StatusMsgDecoder::getItemGroup() const
{
	if ( !hasItemGroup() )
	{
		EmaString temp( "Attempt to getItemGroup() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_itemGroup.setFromInt( _pRsslMsg->statusMsg.groupId.data, _pRsslMsg->statusMsg.groupId.length );

	return _itemGroup.toBuffer();
}

const EmaBuffer& StatusMsgDecoder::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( _pRsslMsg->statusMsg.permData.data, _pRsslMsg->statusMsg.permData.length );

	return _permission.toBuffer();
}

UInt32 StatusMsgDecoder::getPublisherIdUserId() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->statusMsg.postUserInfo.postUserId;
}

UInt32 StatusMsgDecoder::getPublisherIdUserAddress() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserAddress() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->statusMsg.postUserInfo.postUserAddr;
}

bool StatusMsgDecoder::getClearCache() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_CLEAR_CACHE ) ? true : false;
}

bool StatusMsgDecoder::getPrivateStream() const
{
	return ( _pRsslMsg->statusMsg.flags & RSSL_STMF_PRIVATE_STREAM ) ? true : false;
}

void StatusMsgDecoder::setServiceName( const char* serviceName, UInt32 length, bool nullTerm )
{
	_serviceNameSet = length ? true : false;

	_serviceName.setInt( serviceName, length, nullTerm );
}

const EmaBuffer& StatusMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& StatusMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode StatusMsgDecoder::getErrorCode() const
{
	return _errorCode;
}
