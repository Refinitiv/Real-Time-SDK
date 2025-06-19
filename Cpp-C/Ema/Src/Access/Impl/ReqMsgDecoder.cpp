/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ReqMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmQos.h"
#include "ReqMsg.h"
#include "OmmInvalidUsageException.h"

#include "rtr/rsslMsgDecoders.h"

using namespace refinitiv::ema::access;

ReqMsgDecoder::ReqMsgDecoder() :
 MsgDecoder(),
 _name(),
 _serviceName(),
 _extHeader(),
 _qos(),
 _hexBuffer(),
 _serviceNameSet( false ),
 _errorCode( OmmError::NoErrorEnum )
{
}

ReqMsgDecoder::~ReqMsgDecoder()
{
}

bool ReqMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary )
{
	_serviceNameSet = false;

	_pRsslMsg = rsslMsg;

	_pRsslDictionary = rsslDictionary;

	_rsslMajVer = majVer;

	_rsslMinVer = minVer;

	StaticDecoder::setRsslData( &_attrib, &_pRsslMsg->msgBase.msgKey.encAttrib,
		hasAttrib() ? _pRsslMsg->msgBase.msgKey.attribContainerType : RSSL_DT_NO_DATA, majVer, minVer, _pRsslDictionary );

	StaticDecoder::setRsslData( &_payload, &_pRsslMsg->msgBase.encDataBody, _pRsslMsg->msgBase.containerType, majVer, minVer, _pRsslDictionary );

	setQosInt();

	_errorCode = OmmError::NoErrorEnum;

	return true;
}

bool ReqMsgDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void* )
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
		setQosInt();
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

bool ReqMsgDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool ReqMsgDecoder::hasMsgKey() const
{
	return true; 
}

bool ReqMsgDecoder::hasName() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) ? true : false;
}

bool ReqMsgDecoder::hasNameType() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE ) ? true : false;
}

bool ReqMsgDecoder::hasServiceId() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
}

bool ReqMsgDecoder::hasId() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_IDENTIFIER ) ? true : false;
}

bool ReqMsgDecoder::hasFilter() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) ? true : false;
}

bool ReqMsgDecoder::hasAttrib() const
{
	return ( _pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) ? true : false;
}

bool ReqMsgDecoder::hasPayload() const
{
	return _pRsslMsg->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

bool ReqMsgDecoder::hasExtendedHeader() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER ) ? true : false;
}

bool ReqMsgDecoder::hasServiceName() const
{
	return _serviceNameSet;
}

Int32 ReqMsgDecoder::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

UInt16 ReqMsgDecoder::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

const EmaString& ReqMsgDecoder::getName() const
{
	if ( !hasName() )
	{
		EmaString temp( "Attempt to getName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_name.setInt( _pRsslMsg->msgBase.msgKey.name.data, _pRsslMsg->msgBase.msgKey.name.length, false );

	return _name.toString();
}

UInt8 ReqMsgDecoder::getNameType() const
{
	if ( !hasNameType() )
	{
		EmaString temp( "Attempt to getNameType() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.nameType;
}

UInt32 ReqMsgDecoder::getServiceId() const
{
	if ( !hasServiceId() )
	{
		EmaString temp( "Attempt to getServiceId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.serviceId;
}

const EmaString& ReqMsgDecoder::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to getServiceName() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceName.toString();
}

Int32 ReqMsgDecoder::getId() const
{
	if ( !hasId() )
	{
		EmaString temp( "Attempt to getId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.identifier;
}

UInt32 ReqMsgDecoder::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp( "Attempt to getFilter() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->msgBase.msgKey.filter;
}

const EmaBuffer& ReqMsgDecoder::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( _pRsslMsg->requestMsg.extendedHeader.data, _pRsslMsg->requestMsg.extendedHeader.length );

	return _extHeader.toBuffer();
}

bool ReqMsgDecoder::hasPriority() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_PRIORITY ) ? true : false;
}

bool ReqMsgDecoder::hasQos() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_QOS ) ? true : false;
}

bool ReqMsgDecoder::hasView() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_VIEW ) ? true : false;
}

bool ReqMsgDecoder::hasBatch() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_BATCH ) ? true : false;
}

UInt8 ReqMsgDecoder::getPriorityClass() const
{
	if ( !hasPriority() )
	{
		EmaString temp( "Attempt to getPriorityClass() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->requestMsg.priorityClass;
}

UInt16 ReqMsgDecoder::getPriorityCount() const
{
	if ( !hasPriority() )
	{
		EmaString temp( "Attempt to getPriorityCount() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pRsslMsg->requestMsg.priorityCount;
}

UInt32 ReqMsgDecoder::getTimeliness() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getTimeliness() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	UInt32 timeliness = ReqMsg::BestTimelinessEnum;

	if ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS )
	{
		if ( _pRsslMsg->requestMsg.qos.timeliness == _pRsslMsg->requestMsg.worstQos.timeliness )
		{
			switch ( _pRsslMsg->requestMsg.qos.timeliness )
			{
			case RSSL_QOS_TIME_REALTIME:
				timeliness = ReqMsg::RealTimeEnum;
				break;
			case RSSL_QOS_TIME_DELAYED_UNKNOWN:
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			case RSSL_QOS_TIME_DELAYED:
				if ( _pRsslMsg->requestMsg.qos.timeInfo == _pRsslMsg->requestMsg.worstQos.timeInfo )
					timeliness = _pRsslMsg->requestMsg.qos.timeInfo;
				else
					timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			}
		}
		else
		{
			if ( _pRsslMsg->requestMsg.qos.timeliness == RSSL_QOS_TIME_REALTIME )
				timeliness = ReqMsg::BestRateEnum;
			else
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
 		}
	}
	else
	{
		switch ( _pRsslMsg->requestMsg.qos.timeliness )
		{
			case RSSL_QOS_TIME_REALTIME:
				timeliness = ReqMsg::RealTimeEnum;
				break;
			case RSSL_QOS_TIME_DELAYED_UNKNOWN:
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			case RSSL_QOS_TIME_DELAYED:
					timeliness = _pRsslMsg->requestMsg.qos.timeInfo;
				break;
		}
	}

	return timeliness;
}

UInt32 ReqMsgDecoder::getRate() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getRate() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	UInt32 rate = ReqMsg::BestRateEnum;

	if ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS )
	{
		if ( _pRsslMsg->requestMsg.qos.rate == _pRsslMsg->requestMsg.worstQos.rate )
		{
			switch ( _pRsslMsg->requestMsg.qos.rate )
			{
			case RSSL_QOS_RATE_TICK_BY_TICK:
				rate = ReqMsg::TickByTickEnum;
				break;
			case RSSL_QOS_RATE_JIT_CONFLATED:
				rate = ReqMsg::JustInTimeConflatedEnum;
				break;
			case RSSL_QOS_RATE_TIME_CONFLATED:
				if ( _pRsslMsg->requestMsg.qos.rateInfo == _pRsslMsg->requestMsg.worstQos.rateInfo )
					rate = _pRsslMsg->requestMsg.qos.rateInfo;
				else
					rate = ReqMsg::BestConflatedRateEnum;
				break;
			}
		}
		else
		{
			if ( _pRsslMsg->requestMsg.qos.rate == RSSL_QOS_RATE_TICK_BY_TICK )
				rate = ReqMsg::BestRateEnum;
			else
				rate = ReqMsg::BestConflatedRateEnum;
		}
	}
	else
	{
		switch ( _pRsslMsg->requestMsg.qos.rate )
		{
		case RSSL_QOS_RATE_TICK_BY_TICK:
			rate = ReqMsg::TickByTickEnum;
			break;
		case RSSL_QOS_RATE_JIT_CONFLATED:
			rate = ReqMsg::JustInTimeConflatedEnum;
			break;
		case RSSL_QOS_RATE_TIME_CONFLATED:
			rate = _pRsslMsg->requestMsg.qos.rateInfo;
		}
	}
	
	return rate;
}

bool ReqMsgDecoder::getInitialImage() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_NO_REFRESH ) ? false : true;
}

bool ReqMsgDecoder::getInterestAfterRefresh() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_STREAMING ) ? true : false;
}

bool ReqMsgDecoder::getConflatedInUpdates() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_CONF_INFO_IN_UPDATES ) ? true : false;
}

bool ReqMsgDecoder::getPause() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_PAUSE ) ? true : false;
}

bool ReqMsgDecoder::getPrivateStream() const
{
	return ( _pRsslMsg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM ) ? true : false;
}

void ReqMsgDecoder::setQosInt()
{
	RsslQos* rsslQos = &_pRsslMsg->requestMsg.qos;

	StaticDecoder::setRsslData( &_qos, rsslQos );
}

void ReqMsgDecoder::setServiceName( const char* serviceName, UInt32 length, bool nullTerm )
{
	_serviceNameSet = length ? true : false;

	_serviceName.setInt( serviceName, length, nullTerm );
}

void ReqMsgDecoder::setServiceId(UInt16 serviceId)
{
	_pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	_pRsslMsg->msgBase.msgKey.serviceId = serviceId;
}


const EmaBuffer& ReqMsgDecoder::getHexBuffer() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data, _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& ReqMsgDecoder::getRsslBuffer() const
{
	return _pRsslMsg->msgBase.encMsgBuffer;
}

OmmError::ErrorCode ReqMsgDecoder::getErrorCode() const
{
	return _errorCode;
}

