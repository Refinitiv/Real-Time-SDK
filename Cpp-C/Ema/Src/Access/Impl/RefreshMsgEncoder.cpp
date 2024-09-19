/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 201, 20249 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "RefreshMsgEncoder.h"
#include "ComplexType.h"
#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

RefreshMsgEncoder::RefreshMsgEncoder() :
 MsgEncoder(),
 _rsslRefreshMsg(),
#ifdef __EMA_COPY_ON_SET__
 _permissionData(),
 _itemGroup(),
 _statusText(),
 _permissionDataSet( false ),
 _itemGroupSet( false ),
#else
 _pPermissionData( 0 ),
 _pItemGroup( 0 ),
 _pStatusText( 0 ),
#endif
 _nameType( 0 ),
 _domainType( RSSL_DMT_MARKET_PRICE ),
 _serviceId( 0 ),
 _filter( 0 ),
 _identifier( 0 ),
 _streamId( 0 ),
 _sequenceNum( 0 ),
 _partNum( 0 ),
 _conflatedCount( 0 ),
 _conflatedTime( 0 ),
 _userId( 0 ),
 _userAddress( 0 ),
 _timeliness( 0 ),
 _rate( 0 ),
 _identifierSet( false ),
 _filterSet( false ),
 _nameTypeSet( false ),
 _serviceIdSet( false ),
 _sequenceNumSet( false ),
 _partNumSet( false ),
 _conflatedSet( false ),
 _publisherIdSet( false ),
 _doNotCache( false ),
 _solicited( false ),
 _clearCache( false ),
 _complete( false ),
 _doNotRipple( false ),
 _privateStream( false ),
 _qosSet( false ),
 _stateSet( false )
{
	clearRsslRefreshMsg();
}

RefreshMsgEncoder::~RefreshMsgEncoder()
{
}

void RefreshMsgEncoder::clear()
{
	MsgEncoder::clear();

	clearRsslRefreshMsg();

	_domainType = RSSL_DMT_MARKET_PRICE;
	_streamId = 0;
	_identifierSet = false;
	_filterSet = false;
	_nameTypeSet = false;
	_serviceIdSet = false;
	_sequenceNumSet = false;
	_partNumSet = false;
#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = false;
	_itemGroupSet = false;
#else
	_pPermissionData = 0;
	_pItemGroup = 0;
	_pStatusText = 0;
#endif
	_conflatedSet = false;
	_publisherIdSet = false;
	_doNotCache = false;
	_solicited = false;
	_clearCache = false;
	_complete = false;
	_doNotRipple = false;
	_privateStream = false;
	_qosSet = false;
	_stateSet = false;
}

void RefreshMsgEncoder::release()
{
	MsgEncoder::release();

	clearRsslRefreshMsg();

	_domainType = RSSL_DMT_MARKET_PRICE;
	_streamId = 0;
	_identifierSet = false;
	_filterSet = false;
	_nameTypeSet = false;
	_serviceIdSet = false;
	_sequenceNumSet = false;
	_partNumSet = false;
#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = false;
	_itemGroupSet = false;
#else
	_pPermissionData = 0;
	_pItemGroup = 0;
	_pStatusText = 0;
#endif
	_conflatedSet = false;
	_publisherIdSet = false;
	_doNotCache = false;
	_solicited = false;
	_clearCache = false;
	_complete = false;
	_doNotRipple = false;
	_privateStream = false;
	_qosSet = false;
	_stateSet = false;
}

void RefreshMsgEncoder::clearRsslRefreshMsg()
{
	rsslClearRefreshMsg( &_rsslRefreshMsg );
	_rsslRefreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslRefreshMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void RefreshMsgEncoder::domainType( UInt8 value )
{
	acquireEncIterator();

	_domainType = value;
}

void RefreshMsgEncoder::streamId( Int32 value )
{
	acquireEncIterator();

	_streamId = value;
}

void RefreshMsgEncoder::serviceId( UInt16 value )
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

bool RefreshMsgEncoder::hasServiceId() const
{
	return _serviceIdSet;
}

void RefreshMsgEncoder::name( const EmaString& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = true;
	_name = value;
#else
	_pName = &value;
#endif
}

void RefreshMsgEncoder::nameType( UInt8 value )
{
	acquireEncIterator();

	_nameTypeSet = true;
	_nameType = value;
}

void RefreshMsgEncoder::filter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter = value;
}

void RefreshMsgEncoder::addFilter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter |= value;
}

void RefreshMsgEncoder::identifier( Int32 value )
{
	acquireEncIterator();

	_identifierSet = true;
	_identifier = value;
}

void RefreshMsgEncoder::qos( UInt32 timeliness , UInt32 rate )
{
	acquireEncIterator();

	_qosSet = true;
	_timeliness = timeliness;
	_rate = rate;
}

void RefreshMsgEncoder::state( OmmState::StreamState streamState,
						   OmmState::DataState dataState,
						   UInt8 stateCode, const EmaString& text )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_statusText = text;
#else
	_pStatusText = &text;
#endif

	_streamState = streamState;
	_dataState = dataState;
	_stateCode = stateCode;

	_stateSet = true;
}

void RefreshMsgEncoder::seqNum( UInt32 value )
{
	acquireEncIterator();

	_sequenceNumSet = true;
	_sequenceNum = value;
}

void RefreshMsgEncoder::partNum( UInt16 value )
{
	acquireEncIterator();

	_partNumSet = true;
	_partNum = value;
}

void RefreshMsgEncoder::publisherId( UInt32 userId, UInt32 userAddress )
{
	acquireEncIterator();

	_publisherIdSet = true;
	_userId = userId;
	_userAddress = userAddress;
}

void RefreshMsgEncoder::conflated( UInt16 count, UInt16 time )
{
	acquireEncIterator();

	_conflatedSet = true;
	_conflatedCount = count;
	_conflatedTime = time;
}

void RefreshMsgEncoder::itemGroup( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_itemGroupSet = true;
	_itemGroup = value;
#else
	_pItemGroup = &value;
#endif
}

void RefreshMsgEncoder::permissionData( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = true;
	_permissionData = value;
#else
	_pPermissionData = &value;
#endif
}

void RefreshMsgEncoder::payload( const ComplexType& load )
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

void RefreshMsgEncoder::attrib( const ComplexType& attrib )
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

void RefreshMsgEncoder::extendedHeader( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeaderSet = true;
	_extendedHeader = value;
#else
	_pExtendedHeader = &value;
#endif
}

void RefreshMsgEncoder::doNotCache( bool value )
{
	acquireEncIterator();

	_doNotCache = value;
}

void RefreshMsgEncoder::solicited( bool value )
{
	acquireEncIterator();

	_solicited = value;
}

void RefreshMsgEncoder::clearCache( bool value )
{
	acquireEncIterator();

	_clearCache = value;
}

void RefreshMsgEncoder::complete( bool value )
{
	acquireEncIterator();

	_complete = value;
}

void RefreshMsgEncoder::doNotRipple( bool value )
{
	acquireEncIterator();

	_doNotRipple = value;
}

void RefreshMsgEncoder::privateStream( bool value )
{
	acquireEncIterator();

	_privateStream = value;
}

RsslRefreshMsg* RefreshMsgEncoder::getRsslRefreshMsg() const
{
	if ( _streamId ) _rsslRefreshMsg.msgBase.streamId = _streamId;

	_rsslRefreshMsg.msgBase.domainType = _domainType;

	if ( hasName() || _nameTypeSet || _serviceIdSet ||
		_filterSet || _identifierSet ||
		_attribDataType != RSSL_DT_NO_DATA )
	{
		if ( hasName() )
		{
#ifdef __EMA_COPY_ON_SET__
			_rsslRefreshMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
			_rsslRefreshMsg.msgBase.msgKey.name.length = _name.length();
#else
			_rsslRefreshMsg.msgBase.msgKey.name.data = (char*)_pName->c_str();
			_rsslRefreshMsg.msgBase.msgKey.name.length = _pName->length();
#endif
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
		}

		if ( _nameTypeSet )
		{
			_rsslRefreshMsg.msgBase.msgKey.nameType = _nameType;
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
		}

		if ( _serviceIdSet )
		{
			_rsslRefreshMsg.msgBase.msgKey.serviceId = _serviceId;
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}

		if ( _filterSet )
		{
			_rsslRefreshMsg.msgBase.msgKey.filter = _filter;
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
		}

		if ( _identifierSet )
		{
			_rsslRefreshMsg.msgBase.msgKey.identifier = _identifier;
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
		}
		
		if ( _attribDataType != RSSL_DT_NO_DATA )
		{
			_rsslRefreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
			_rsslRefreshMsg.msgBase.msgKey.attribContainerType = _attribDataType;

#ifdef __EMA_COPY_ON_SET__
			_rsslRefreshMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
			_rsslRefreshMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
			_rsslRefreshMsg.msgBase.msgKey.encAttrib = *_pAttrib;
#endif
		}

		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
	}

	if ( _privateStream ) _rsslRefreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;
	if ( _complete ) _rsslRefreshMsg.flags |= RSSL_RFMF_REFRESH_COMPLETE;
	if ( _doNotCache ) _rsslRefreshMsg.flags |= RSSL_RFMF_DO_NOT_CACHE;
	if ( _clearCache ) _rsslRefreshMsg.flags |= RSSL_RFMF_CLEAR_CACHE;
	if ( _solicited ) _rsslRefreshMsg.flags |= RSSL_RFMF_SOLICITED;

#ifdef __EMA_COPY_ON_SET__
	if ( _extendedHeaderSet )
	{
		_rsslRefreshMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
		_rsslRefreshMsg.extendedHeader.length = _extendedHeader.length();
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_EXTENDED_HEADER;
	}

	if ( _permissionDataSet )
	{
		_rsslRefreshMsg.permData.data = (char*)_permissionData.c_buf();
		_rsslRefreshMsg.permData.length = _permissionData.length();
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_PERM_DATA;
	}

	if ( _itemGroupSet )
	{
		_rsslRefreshMsg.groupId.data = (char*)_itemGroup.c_buf();
		_rsslRefreshMsg.groupId.length = _itemGroup.length();
	}
	else
	{
		_itemGroup.clear().append( '\0' ).append( '\0' );
		_rsslRefreshMsg.groupId.data = (char*)_itemGroup.c_buf();
		_rsslRefreshMsg.groupId.length = _itemGroup.length();
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslRefreshMsg.msgBase.containerType = _payloadDataType;
		_rsslRefreshMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
		_rsslRefreshMsg.msgBase.encDataBody.length = _payload.length();
	}

	if ( _stateSet )
	{
		_rsslRefreshMsg.state.streamState = _streamState;
		_rsslRefreshMsg.state.dataState = _dataState;
		_rsslRefreshMsg.state.code = _stateCode;
		_rsslRefreshMsg.state.text.data = (char*)_statusText.c_str();
		_rsslRefreshMsg.state.text.length = _statusText.length();
	}
	else
	{
		_rsslRefreshMsg.state.streamState = RSSL_STREAM_OPEN;
		_rsslRefreshMsg.state.dataState = RSSL_DATA_OK;
		_rsslRefreshMsg.state.code = RSSL_SC_NONE;
		_rsslRefreshMsg.state.text.data = 0;
		_rsslRefreshMsg.state.text.length = 0;
	}
#else
	if ( _pExtendedHeader )
	{
		_rsslRefreshMsg.extendedHeader.data = (char*)_pExtendedHeader->c_buf();
		_rsslRefreshMsg.extendedHeader.length = _pExtendedHeader->length();
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_EXTENDED_HEADER;
	}

	if ( _pPermissionData )
	{
		_rsslRefreshMsg.permData.data = (char*)_pPermissionData->c_buf();
		_rsslRefreshMsg.permData.length = _pPermissionData->length();
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_PERM_DATA;
	}

	if ( _pItemGroup )
	{
		_rsslRefreshMsg.groupId.data = (char*)_pItemGroup->c_buf();
		_rsslRefreshMsg.groupId.length = _pItemGroup->length();
	}
	else
	{
		_rsslRefreshMsg.groupId.data = (char*)"0";
		_rsslRefreshMsg.groupId.length = 1;
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslRefreshMsg.msgBase.containerType = _payloadDataType;
		_rsslRefreshMsg.msgBase.encDataBody = *_pPayload;
	}

	if ( _stateSet )
	{
		_rsslRefreshMsg.state.streamState = _streamState;
		_rsslRefreshMsg.state.dataState = _dataState;
		_rsslRefreshMsg.state.code = _stateCode;
		_rsslRefreshMsg.state.text.data = (char*)_pStatusText->c_str();
		_rsslRefreshMsg.state.text.length = _pStatusText->length();
	}
	else
	{
		_rsslRefreshMsg.state.streamState = RSSL_STREAM_OPEN;
		_rsslRefreshMsg.state.dataState = RSSL_DATA_OK;
		_rsslRefreshMsg.state.code = RSSL_SC_NONE;
		_rsslRefreshMsg.state.text.data = 0;
		_rsslRefreshMsg.state.text.length = 0;
	}
#endif

	if ( _sequenceNumSet )
	{
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_SEQ_NUM;
		_rsslRefreshMsg.seqNum = _sequenceNum;
	}

	if ( _partNumSet )
	{
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_PART_NUM;
		_rsslRefreshMsg.partNum = _partNum;
	}

	if ( _publisherIdSet )
	{
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_POST_USER_INFO;
		_rsslRefreshMsg.postUserInfo.postUserAddr = _userAddress;
		_rsslRefreshMsg.postUserInfo.postUserId = _userId;
	}

	if ( _qosSet )
	{
		_rsslRefreshMsg.flags |= RSSL_RFMF_HAS_QOS;
		OmmQosDecoder::convertToRssl( &_rsslRefreshMsg.qos, _timeliness, _rate );
	}

	return &_rsslRefreshMsg;
}

RsslMsg* RefreshMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*)getRsslRefreshMsg();
}
