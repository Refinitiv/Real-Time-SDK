/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "StatusMsgEncoder.h"
#include "ComplexType.h"
#include "OmmStateDecoder.h"

using namespace thomsonreuters::ema::access;

StatusMsgEncoder::StatusMsgEncoder() :
 MsgEncoder(),
 _rsslStatusMsg(),
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
 _userId( 0 ),
 _userAddress( 0 ),
 _identifierSet( false ),
 _filterSet( false ),
 _nameTypeSet( false ),
 _serviceIdSet( false ),
 _publisherIdSet( false ),
 _clearCache( false ),
 _privateStream( false )
{
	clearRsslStatusMsg();
}

StatusMsgEncoder::~StatusMsgEncoder()
{
}

void StatusMsgEncoder::clear()
{
	MsgEncoder::clear();

	clearRsslStatusMsg();

	_domainType = RSSL_DMT_MARKET_PRICE;
	_streamId = 0;
	_identifierSet = false;
	_filterSet = false;
	_nameTypeSet = false;
	_serviceIdSet = false;
#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = false;
	_itemGroupSet = false;
#else
	_pPermissionData = 0;
	_pItemGroup = 0;
	_pStatusText = 0;
#endif
	_publisherIdSet = false;
	_clearCache = false;
	_privateStream = false;
	_stateSet = false;
}

void StatusMsgEncoder::clearRsslStatusMsg()
{
	rsslClearStatusMsg( &_rsslStatusMsg );
	_rsslStatusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	_rsslStatusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
}

void StatusMsgEncoder::domainType( UInt8 value )
{
	acquireEncIterator();

	_domainType = value;
}

void StatusMsgEncoder::streamId( Int32 value )
{
	acquireEncIterator();

	_streamId = value;
}

void StatusMsgEncoder::serviceId( UInt16 value )
{
	if ( hasServiceName() )
	{
		EmaString text( "Attempt to set serviceId while service name is already set." );
		throwIueException( text );
		return;
	}

	acquireEncIterator();

	_serviceId = value;
	_serviceIdSet = true;;
}

bool StatusMsgEncoder::hasServiceId() const
{
	return _serviceIdSet;
}

void StatusMsgEncoder::name( const EmaString& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_nameSet = true;
	_name = value;
#else
	_pName = &value;
#endif
}

void StatusMsgEncoder::nameType( UInt8 value )
{
	acquireEncIterator();

	_nameTypeSet = true;
	_nameType = value;
}

void StatusMsgEncoder::filter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter = value;
}

void StatusMsgEncoder::addFilter( UInt32 value )
{
	acquireEncIterator();

	_filterSet = true;
	_filter |= value;
}

void StatusMsgEncoder::identifier( Int32 value )
{
	acquireEncIterator();

	_identifierSet = true;
	_identifier = value;
}

void StatusMsgEncoder::state( OmmState::StreamState streamState,
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

void StatusMsgEncoder::publisherId( UInt32 userId, UInt32 userAddress )
{
	acquireEncIterator();

	_publisherIdSet = true;
	_userId = userId;
	_userAddress = userAddress;
}

void StatusMsgEncoder::itemGroup( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_itemGroupSet = true;
	_itemGroup = value;
#else
	_pItemGroup = &value;
#endif
}

void StatusMsgEncoder::permissionData( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_permissionDataSet = true;
	_permissionData = value;
#else
	_pPermissionData = &value;
#endif
}

void StatusMsgEncoder::payload( const ComplexType& load )
{
	acquireEncIterator();

	_payloadDataType = convertDataType( load.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	_payload.setFrom( static_cast<const Data&>(load).getEncoder().getRsslBuffer().data,
					static_cast<const Data&>(load).getEncoder().getRsslBuffer().length );
#else
	_pPayload = &static_cast<const Data&>(load).getEncoder().getRsslBuffer();
#endif
}

void StatusMsgEncoder::attrib( const ComplexType& attrib )
{
	acquireEncIterator();

	_attribDataType = convertDataType( attrib.getDataType() );

#ifdef __EMA_COPY_ON_SET__
	_attrib.setFrom( static_cast<const Data&>(attrib).getEncoder().getRsslBuffer().data,
					static_cast<const Data&>(attrib).getEncoder().getRsslBuffer().length );
#else
	_pAttrib = &static_cast<const Data&>(attrib).getEncoder().getRsslBuffer();
#endif
}

void StatusMsgEncoder::extendedHeader( const EmaBuffer& value )
{
	acquireEncIterator();

#ifdef __EMA_COPY_ON_SET__
	_extendedHeaderSet = true;
	_extendedHeader = value;
#else
	_pExtendedHeader = &value;
#endif
}

void StatusMsgEncoder::clearCache( bool value )
{
	acquireEncIterator();

	_clearCache = value;
}

void StatusMsgEncoder::privateStream( bool value )
{
	acquireEncIterator();

	_privateStream = value;
}

RsslStatusMsg* StatusMsgEncoder::getRsslStatusMsg() const
{
	if ( _streamId ) _rsslStatusMsg.msgBase.streamId = _streamId;

	_rsslStatusMsg.msgBase.domainType = _domainType;

	if ( hasName() || _nameTypeSet || _serviceIdSet ||
		_filterSet || _identifierSet ||
		_attribDataType != RSSL_DT_NO_DATA )
	{
		if ( hasName() )
		{
#ifdef __EMA_COPY_ON_SET__
			_rsslStatusMsg.msgBase.msgKey.name.data = (char*)_name.c_str();
			_rsslStatusMsg.msgBase.msgKey.name.length = _name.length();
#else
			_rsslStatusMsg.msgBase.msgKey.name.data = (char*)_pName->c_str();
			_rsslStatusMsg.msgBase.msgKey.name.length = _pName->length();
#endif
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
		}

		if ( _nameTypeSet )
		{
			_rsslStatusMsg.msgBase.msgKey.nameType = _nameType;
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
		}

		if ( _serviceIdSet )
		{
			_rsslStatusMsg.msgBase.msgKey.serviceId = _serviceId;
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}

		if ( _filterSet )
		{
			_rsslStatusMsg.msgBase.msgKey.filter = _filter;
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_FILTER;
		}

		if ( _identifierSet )
		{
			_rsslStatusMsg.msgBase.msgKey.identifier = _identifier;
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_IDENTIFIER;
		}
		
		if ( _attribDataType != RSSL_DT_NO_DATA )
		{
			_rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
			_rsslStatusMsg.msgBase.msgKey.attribContainerType = _attribDataType;

#ifdef __EMA_COPY_ON_SET__
			_rsslStatusMsg.msgBase.msgKey.encAttrib.data = (char*)_attrib.c_buf();
			_rsslStatusMsg.msgBase.msgKey.encAttrib.length = _attrib.length();
#else
			_rsslStatusMsg.msgBase.msgKey.encAttrib = *_pAttrib;
#endif
		}

		_rsslStatusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
	}

	if ( _privateStream ) _rsslStatusMsg.flags |= RSSL_STMF_PRIVATE_STREAM;
	if ( _clearCache ) _rsslStatusMsg.flags |= RSSL_STMF_CLEAR_CACHE;

#ifdef __EMA_COPY_ON_SET__
	if ( _extendedHeaderSet )
	{
		_rsslStatusMsg.extendedHeader.data = (char*)_extendedHeader.c_buf();
		_rsslStatusMsg.extendedHeader.length = _extendedHeader.length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_EXTENDED_HEADER;
	}

	if ( _permissionDataSet )
	{
		_rsslStatusMsg.permData.data = (char*)_permissionData.c_buf();
		_rsslStatusMsg.permData.length = _permissionData.length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_PERM_DATA;
	}

	if ( _itemGroupSet )
	{
		_rsslStatusMsg.groupId.data = (char*)_itemGroup.c_buf();
		_rsslStatusMsg.groupId.length = _itemGroup.length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_GROUP_ID;
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslStatusMsg.msgBase.containerType = _payloadDataType;
		_rsslStatusMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
		_rsslStatusMsg.msgBase.encDataBody.length = _payload.length();
	}

	if ( _stateSet )
	{
		_rsslStatusMsg.state.streamState = _streamState;
		_rsslStatusMsg.state.dataState = _dataState;
		_rsslStatusMsg.state.code = _stateCode;
		_rsslStatusMsg.state.text.data = (char*)_statusText.c_str();
		_rsslStatusMsg.state.text.length = _statusText.length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;
	}
	else
	{
		_rsslStatusMsg.flags &= ~RSSL_STMF_HAS_STATE;
	}
#else
	if ( _pExtendedHeader )
	{
		_rsslStatusMsg.extendedHeader.data = (char*)_pExtendedHeader->c_buf();
		_rsslStatusMsg.extendedHeader.length = _pExtendedHeader->length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_EXTENDED_HEADER;
	}

	if ( _pPermissionData )
	{
		_rsslStatusMsg.permData.data = (char*)_pPermissionData->c_buf();
		_rsslStatusMsg.permData.length = _pPermissionData->length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_PERM_DATA;
	}

	if ( _pItemGroup )
	{
		_rsslStatusMsg.groupId.data = (char*)_pItemGroup->c_buf();
		_rsslStatusMsg.groupId.length = _pItemGroup->length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_GROUP_ID;
	}

	if ( _payloadDataType != RSSL_DT_NO_DATA )
	{
		_rsslStatusMsg.msgBase.containerType = _payloadDataType;
		_rsslStatusMsg.msgBase.encDataBody = *_pPayload;
	}

	if ( _stateSet )
	{
		_rsslStatusMsg.state.streamState = _streamState;
		_rsslStatusMsg.state.dataState = _dataState;
		_rsslStatusMsg.state.code = _stateCode;
		_rsslStatusMsg.state.text.data = (char*)_pStatusText->c_str();
		_rsslStatusMsg.state.text.length = _pStatusText->length();
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;
	}
	else
	{
		_rsslStatusMsg.flags &= ~RSSL_STMF_HAS_STATE;
	}
#endif

	if ( _publisherIdSet )
	{
		_rsslStatusMsg.flags |= RSSL_STMF_HAS_POST_USER_INFO;
		_rsslStatusMsg.postUserInfo.postUserAddr = _userAddress;
		_rsslStatusMsg.postUserInfo.postUserId = _userId;
	}

	return &_rsslStatusMsg;
}

RsslMsg* StatusMsgEncoder::getRsslMsg() const
{
	return (RsslMsg*)getRsslStatusMsg();
}
