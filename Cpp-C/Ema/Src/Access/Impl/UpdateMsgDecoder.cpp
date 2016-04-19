/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "UpdateMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"

using namespace thomsonreuters::ema::access;

UpdateMsgDecoder::UpdateMsgDecoder() :
 MsgDecoder(),
 _rsslMsg(),
 _pRsslMsg( 0 ),
 _name(),
 _serviceName(),
 _extHeader(),
 _permission(),
 _hexBuffer(),
 _serviceNameSet( false ),
 _rsslMajVer( RSSL_RWF_MAJOR_VERSION ),
 _rsslMinVer( RSSL_RWF_MINOR_VERSION ),
 _errorCode( OmmError::NoErrorEnum )
{
}

UpdateMsgDecoder::~UpdateMsgDecoder()
{
}

bool UpdateMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary )
{
	_serviceNameSet = false;

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

bool UpdateMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
{
	_serviceNameSet = false;

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

bool UpdateMsgDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool UpdateMsgDecoder::hasMsgKey() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) ? true : false;
}

bool UpdateMsgDecoder::hasName() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool UpdateMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool UpdateMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool UpdateMsgDecoder::hasId() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool UpdateMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool UpdateMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) &&
			( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool UpdateMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool UpdateMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool UpdateMsgDecoder::hasServiceName() const
{
	return _serviceNameSet;
}

Int32 UpdateMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 UpdateMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& UpdateMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 UpdateMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 UpdateMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

const EmaString& UpdateMsgDecoder::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to getServiceName() while it is NOT set." );
		throwIueException( temp );
	}

	return _serviceName.toString();
}

Int32 UpdateMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 UpdateMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

const EmaBuffer& UpdateMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp );
	}

	_extHeader.setFromInt( _pRsslMsg->updateMsg.extendedHeader.data, _pRsslMsg->updateMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

bool UpdateMsgDecoder::hasSeqNum() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM ) ? true : false;
}

bool UpdateMsgDecoder::hasPermissionData() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA ) ? true : false;
}

bool UpdateMsgDecoder::hasConflated() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_CONF_INFO ) ? true : false;
}

bool UpdateMsgDecoder::hasPublisherId() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO ) ? true : false;
}

UInt8 UpdateMsgDecoder::getUpdateTypeNum() const
{
	return _pRsslMsg->updateMsg.updateType; 
}

UInt32 UpdateMsgDecoder::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->updateMsg.seqNum;
}

const EmaBuffer& UpdateMsgDecoder::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp );
	}

	_permission.setFromInt( _pRsslMsg->updateMsg.permData.data, _pRsslMsg->updateMsg.permData.length );

	return _permission.toBuffer();
}

UInt16 UpdateMsgDecoder::getConflatedTime() const
{
	if ( !hasConflated() )
	{
		EmaString temp( "Attempt to getConflatedTime() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->updateMsg.conflationTime;
}

UInt16 UpdateMsgDecoder::getConflatedCount() const
{
	if ( !hasConflated() )
	{
		EmaString temp( "Attempt to getConflatedCount() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->updateMsg.conflationCount;
}

UInt32 UpdateMsgDecoder::getPublisherIdUserId() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserId() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->updateMsg.postUserInfo.postUserId;
}

UInt32 UpdateMsgDecoder::getPublisherIdUserAddress() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserAddress() while it is NOT set." );
		throwIueException( temp );
	}

	return _pRsslMsg->updateMsg.postUserInfo.postUserAddr;
}

bool UpdateMsgDecoder::getDoNotCache() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_DO_NOT_CACHE ) ? true : false;
}

bool UpdateMsgDecoder::getDoNotConflate() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_DO_NOT_CONFLATE ) ? true : false;
}

bool UpdateMsgDecoder::getDoNotRipple() const
{
	return ( _pRsslMsg->updateMsg.flags & RSSL_UPMF_DO_NOT_RIPPLE ) ? true : false;
}

void UpdateMsgDecoder::setServiceName( const char* serviceName, UInt32 length, bool nullTerm )
{
	_serviceNameSet = length ? true : false;

	_serviceName.setInt( serviceName, length, nullTerm );
}

const EmaBuffer& UpdateMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& UpdateMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode UpdateMsgDecoder::getErrorCode() const
{
	return _errorCode;
}

