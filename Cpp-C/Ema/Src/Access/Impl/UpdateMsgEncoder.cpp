/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "UpdateMsgEncoder.h"
#include "ComplexType.h"
#include "Decoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

UpdateMsgEncoder::UpdateMsgEncoder() :
 MsgEncoder(),
 _rsslUpdateMsg(),
#ifdef __EMA_COPY_ON_SET__
 _permissionData(),
 _permissionDataSet( false ),
#else
 _pPermissionData( 0 ),
#endif
 _updateTypeNum( 0 ),
 _nameType( 0 ),
 _domainType( RSSL_DMT_MARKET_PRICE ),
 _serviceId( 0 ),
 _filter( 0 ),
 _identifier( 0 ),
 _streamId( 0 ),
 _sequenceNum( 0 ),
 _conflatedCount( 0 ),
 _conflatedTime( 0 ),
 _userId( 0 ),
 _userAddress( 0 ),
 _identifierSet( false ),
 _filterSet( false ),
 _nameTypeSet( false ),
 _serviceIdSet( false ),
 _sequenceNumSet( false ),
 _conflatedSet( false ),
 _publisherIdSet( false ),
 _doNotCache( false ),
 _doNotConflate( false ),
 _doNotRipple( false )
{
	clearRsslUpdateMsg();
}

UpdateMsgEncoder::~UpdateMsgEncoder()
{
}

void UpdateMsgEncoder::clear()
{
	MsgEncoder::clear();

	clearRsslUpdateMsg();

	_domainType = RSSL_DMT_MARKET_PRICE;
	_streamId = 0;
	_identifierSet = false;
	_filterSet = false;
	_updateTypeNum = 0;
	_nameTypeSet = false;
	_serviceIdSet = false;
	_sequenceNumSet = false;
#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = false;
#else
	_pPermissionData = 0;
#endif
	_conflatedSet = false;
	_publisherIdSet = false;
	_doNotCache = false;
	_doNotConflate = false;
	_doNotRipple = false;
}

void UpdateMsgEncoder::clearRsslUpdateMsg()
{
	rsslClearUpdateMsg( &_rsslUpdateMsg );
	_rsslUpdateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslUpdateMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void UpdateMsgEncoder::domainType( UInt8 value )
{
	acquireEncIterator();

	_domainType = value;
}

void UpdateMsgEncoder::streamId( Int32 value )
{
	acquireEncIterator();

	_streamId = value;
}

void UpdateMsgEncoder::serviceId( UInt16 value )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	acquireEncIterator();

	_serviceId = value;
	_serviceIdSet = true;;
}

bool UpdateMsgEncoder::hasServiceId() const
{
	return _serviceIdSet;
}

void UpdateMsgEncoder::name( const EmaString& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = true;
	_name = value;
#else
	_pName = &value;
#endif
}

void UpdateMsgEncoder::nameType( UInt8 value )
{
	acquireEncIterator();

	_nameTypeSet = true;
	_nameType = value;
}

void UpdateMsgEncoder::filter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter = value;
}

void UpdateMsgEncoder::addFilter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter |= value;
}

void UpdateMsgEncoder::identifier( Int32 value )
{
	acquireEncIterator();

	_identifierSet = true;
	_identifier = value;
}

void UpdateMsgEncoder::updateTypeNum( UInt8 value )
{
	acquireEncIterator();

	_updateTypeNum = value;
}

void UpdateMsgEncoder::seqNum( UInt32 value )
{
	acquireEncIterator();

	_sequenceNumSet = true;
	_sequenceNum = value;
}

void UpdateMsgEncoder::publisherId( UInt32 userId, UInt32 userAddress )
{
	acquireEncIterator();

	_publisherIdSet = true;
	_userId = userId;
	_userAddress = userAddress;
}

void UpdateMsgEncoder::conflated( UInt16 count, UInt16 time )
{
	acquireEncIterator();

	_conflatedSet = true;
	_conflatedCount = count;
	_conflatedTime = time;
}

void UpdateMsgEncoder::permissionData( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = true;
	_permissionData = value;
#else
	_pPermissionData = &value;
#endif
}

void UpdateMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_payloadDataType = convertDataType( load.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
	{
		const RsslBuffer& rsslBuf = load.getEncoder().getRsslBuffer();
		_payload.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else if ( load.hasDecoder() )
	{
		const RsslBuffer& rsslBuf = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
		_payload.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}
#else
	if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
		_pPayload = &load.getEncoder().getRsslBuffer();
	else if ( load.hasDecoder() )
		_pPayload = &const_cast<RsslBuffer&>( const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer());
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}
#endif
}

void UpdateMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_attribDataType = convertDataType( attrib.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
	{
		const RsslBuffer& rsslBuf = attrib.getEncoder().getRsslBuffer();
		_attrib.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else if ( attrib.hasDecoder() )
	{
		const RsslBuffer& rsslBuf = const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer();
		_attrib.setFrom( rsslBuf.data, rsslBuf.length );
	}
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}
#else
	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
		_pAttrib = &static_cast<const Data&>(attrib).getEncoder().getRsslBuffer();
	else if ( attrib.hasDecoder() )
		_pAttrib = &const_cast<RsslBuffer&>( const_cast<ComplexType&>( attrib ).getDecoder().getRsslBuffer());
	else
	{
		EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}
#endif
}

void UpdateMsgEncoder::extendedHeader( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeaderSet = true;
	_extendedHeader = value;
#else
	_pExtendedHeader = &value;
#endif
}

void UpdateMsgEncoder::doNotCache( bool value )
{
	acquireEncIterator();

	_doNotCache = value;
}

void UpdateMsgEncoder::doNotConflate( bool value )
{
	acquireEncIterator();

	_doNotConflate = value;
}

void UpdateMsgEncoder::doNotRipple( bool value )
{
	acquireEncIterator();

	_doNotRipple = value;
}

RsslUpdateMsg* UpdateMsgEncoder::getRsslUpdateMsg() const
{
	if ( _streamId ) _rsslUpdateMsg.msgBase.streamId = _streamId;

	_rsslUpdateMsg.msgBase.domainType = _domainType;

	_rsslUpdateMsg.updateType = _updateTypeNum;

	if ( hasName() || _nameTypeSet || _serviceIdSet ||
		_filterSet || _identifierSet ||
		_attribDataType != RSSL_DT_NO_DATA )
	{
		if ( hasName() )
		{
#ifdef __EMA_COPY_ON_SET__
			_rsslUpdateMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
			_rsslUpdateMsg.msgBase.msgKey.name.length = _name.length();
#else
			_rsslUpdateMsg.msgBase.msgKey.name.data = (char*)_pName->c_str();
			_rsslUpdateMsg.msgBase.msgKey.name.length = _pName->length();
#endif
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
		}

		if ( _nameTypeSet )
		{
			_rsslUpdateMsg.msgBase.msgKey.nameType = _nameType;
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
		}

		if ( _serviceIdSet )
		{
			_rsslUpdateMsg.msgBase.msgKey.serviceId = _serviceId;
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}

		if ( _filterSet )
		{
			_rsslUpdateMsg.msgBase.msgKey.filter = _filter;
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
		}

		if ( _identifierSet )
		{
			_rsslUpdateMsg.msgBase.msgKey.identifier = _identifier;
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
		}

		if ( _attribDataType != RSSL_DT_NO_DATA )
		{
			_rsslUpdateMsg.msgBase.msgKey.encAttrib.data;
			_rsslUpdateMsg.msgBase.msgKey.encAttrib.length;
			_rsslUpdateMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
		}
		
		if ( _attribDataType != RSSL_DT_NO_DATA )
		{
			_rsslUpdateMsg.msgBase.msgKey.attribContainerType = _attribDataType;

#ifdef __EMA_COPY_ON_SET__
			_rsslUpdateMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
			_rsslUpdateMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
			_rsslUpdateMsg.msgBase.msgKey.encAttrib = *_pAttrib;
#endif
		}

		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
	}

	if ( _doNotCache ) _rsslUpdateMsg.flags |= RSSL_UPMF_DO_NOT_CACHE;
	if ( _doNotConflate ) _rsslUpdateMsg.flags |= RSSL_UPMF_DO_NOT_CONFLATE;
	if ( _doNotRipple ) _rsslUpdateMsg.flags |= RSSL_UPMF_DO_NOT_RIPPLE;

#ifdef __EMA_COPY_ON_SET__
	if ( _extendedHeaderSet )
	{
		_rsslUpdateMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
		_rsslUpdateMsg.extendedHeader.length = _extendedHeader.length();
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_EXTENDED_HEADER;
	}

	if ( _permissionDataSet )
	{
		_rsslUpdateMsg.permData.data = (char*)_permissionData.c_buf();
		_rsslUpdateMsg.permData.length = _permissionData.length();
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_PERM_DATA;
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslUpdateMsg.msgBase.containerType = _payloadDataType;
		_rsslUpdateMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
		_rsslUpdateMsg.msgBase.encDataBody.length = _payload.length();
	}
#else
	if ( _pExtendedHeader )
	{
		_rsslUpdateMsg.extendedHeader.data = (char*)_pExtendedHeader->c_buf();
		_rsslUpdateMsg.extendedHeader.length = _pExtendedHeader->length();
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_EXTENDED_HEADER;
	}

	if ( _pPermissionData )
	{
		_rsslUpdateMsg.permData.data = (char*)_pPermissionData->c_buf();
		_rsslUpdateMsg.permData.length = _pPermissionData->length();
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_PERM_DATA;
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslUpdateMsg.msgBase.containerType = _payloadDataType;
		_rsslUpdateMsg.msgBase.encDataBody = *_pPayload;
	}
#endif

	if ( _sequenceNumSet )
	{
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_SEQ_NUM;
		_rsslUpdateMsg.seqNum = _sequenceNum;
	}

	if ( _conflatedSet )
	{
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_CONF_INFO;
		_rsslUpdateMsg.conflationCount = _conflatedCount;
		_rsslUpdateMsg.conflationTime = _conflatedTime;
	}

	if ( _publisherIdSet )
	{
		_rsslUpdateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
		_rsslUpdateMsg.postUserInfo.postUserAddr = _userAddress;
		_rsslUpdateMsg.postUserInfo.postUserId = _userId;
	}

	return &_rsslUpdateMsg;
}

RsslMsg* UpdateMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*)getRsslUpdateMsg();
}
