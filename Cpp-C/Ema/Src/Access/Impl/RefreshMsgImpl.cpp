/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "RefreshMsgImpl.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmState.h"
#include "OmmQos.h"
#include "OmmInvalidUsageException.h"

#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"

#include "rtr/rsslMsgDecoders.h"

#include <array>

using namespace refinitiv::ema::access;

static constexpr std::array<char,2> DEFAULT_GROUP_ID {{ '\0', '\0' }};

RefreshMsgImpl::RefreshMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _stateSet(false),
 _qosSet(false),
 _extHeader(),
 _permission(),
 _itemGroup(),
 _state(),
 _qos()
{
	clearRsslRefreshMsg();
}

RefreshMsgImpl& RefreshMsgImpl::operator=( const RefreshMsgImpl& other )
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslRefreshMsg));

	rsslClearBuffer(&(getRsslRefreshMsg()->state.text));
	rsslClearBuffer(&(getRsslRefreshMsg()->groupId));
	rsslClearBuffer(&(getRsslRefreshMsg()->permData));
	rsslClearBuffer(&(getRsslRefreshMsg()->extendedHeader));
	rsslClearBuffer(&(getRsslRefreshMsg()->reqMsgKey.name));
	rsslClearBuffer(&(getRsslRefreshMsg()->reqMsgKey.encAttrib));

	// adjust RsslBuffer values of the RsslRefreshMsg that are specific only to the statusMsg
	RsslRefreshMsg& dstMsg = *getRsslRefreshMsg();
	const RsslRefreshMsg& srcMsg = *other.getRsslRefreshMsg();

	// state.text
	if (srcMsg.state.text.length != 0)
	{
		if (! memberCopyInPlace( dstMsg.state.text, srcMsg.state.text,
								 dstMsg.msgBase.encMsgBuffer, srcMsg.msgBase.encMsgBuffer ) )
		{
			setStatusText(srcMsg.state.text.data, srcMsg.state.text.length);
		}
	}

	// groupId
	if (srcMsg.groupId.length != 0)
	{
		if (! memberCopyInPlace<RefreshMsgImpl>(&RsslRefreshMsg::groupId, dstMsg, srcMsg))
		{
			setItemGroup(other.getItemGroup());
		}
	}

	// permData
	if (other.hasPermissionData())
	{
		if (! memberCopyInPlace<RefreshMsgImpl>(&RsslRefreshMsg::permData, dstMsg, srcMsg))
		{
			setPermissionData(other.getPermissionData());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<RefreshMsgImpl>(&RsslRefreshMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	// reqMsgKey is not used in Ema
	applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_HAS_REQ_MSG_KEY);

	return *this;
}

RefreshMsgImpl::~RefreshMsgImpl()
{
}

void RefreshMsgImpl::applyHasMsgKey()
{
	rsslRefreshMsgApplyHasMsgKey(getRsslRefreshMsg());
}

bool RefreshMsgImpl::hasExtendedHeader() const
{
	return rsslRefreshMsgCheckHasExtendedHdr(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& RefreshMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslRefreshMsg()->extendedHeader.data, getRsslRefreshMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

bool RefreshMsgImpl::hasSeqNum() const
{
	return rsslRefreshMsgCheckHasSeqNum(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::hasPartNum() const
{
	return rsslRefreshMsgCheckHasPartNum(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::hasPermissionData() const
{
	return rsslRefreshMsgCheckHasPermData(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::hasPublisherId() const
{
	return rsslRefreshMsgCheckHasPostUserInfo(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

const OmmState& RefreshMsgImpl::getState() const
{
	if (! _stateSet)
	{
		StaticDecoder::setRsslData( &_state, &getRsslRefreshMsg()->state );
		_stateSet = true;
	}

	return static_cast<const OmmState&>( static_cast<const Data&>( _state ) );
}

bool RefreshMsgImpl::hasQos() const
{
	return rsslRefreshMsgCheckHasQos(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

const OmmQos& RefreshMsgImpl::getQos() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getQos() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	if (! _qosSet)
	{
		StaticDecoder::setRsslData( &_qos, &getRsslRefreshMsg()->qos );
		_qosSet = true;
	}

	return static_cast<const OmmQos&>( static_cast<const Data&>( _qos ) );
}

UInt32 RefreshMsgImpl::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRefreshMsg()->seqNum;
}

UInt16 RefreshMsgImpl::getPartNum() const
{
	if ( !hasPartNum() )
	{
		EmaString temp( "Attempt to getPartNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRefreshMsg()->partNum;
}

const EmaBuffer& RefreshMsgImpl::getItemGroup() const
{
	_itemGroup.setFromInt( getRsslRefreshMsg()->groupId.data, getRsslRefreshMsg()->groupId.length );

	return _itemGroup.toBuffer();
}

const EmaBuffer& RefreshMsgImpl::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( getRsslRefreshMsg()->permData.data, getRsslRefreshMsg()->permData.length );

	return _permission.toBuffer();
}

UInt32 RefreshMsgImpl::getPublisherIdUserId() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRefreshMsg()->postUserInfo.postUserId;
}

UInt32 RefreshMsgImpl::getPublisherIdUserAddress() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserAddress() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRefreshMsg()->postUserInfo.postUserAddr;
}

bool RefreshMsgImpl::getDoNotCache() const
{
	return rsslRefreshMsgCheckDoNotCache(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::getSolicited() const
{
	return rsslRefreshMsgCheckSolicited(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::getComplete() const
{
	return rsslRefreshMsgCheckRefreshComplete(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::getClearCache() const
{
	return rsslRefreshMsgCheckClearCache(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

bool RefreshMsgImpl::getPrivateStream() const
{
	return rsslRefreshMsgCheckPrivateStream(getRsslRefreshMsg()) == RSSL_TRUE
		? true : false;
}

void RefreshMsgImpl::clearRsslRefreshMsg()
{
	rsslClearRefreshMsg( getRsslRefreshMsg() );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;

	getRsslRefreshMsg()->state.streamState = RSSL_STREAM_OPEN;
	getRsslRefreshMsg()->state.dataState = RSSL_DATA_OK;
	getRsslRefreshMsg()->state.code = RSSL_SC_NONE;
	getRsslRefreshMsg()->state.text.data = nullptr;
	getRsslRefreshMsg()->state.text.length = 0;

	getRsslRefreshMsg()->groupId.data = (char*)DEFAULT_GROUP_ID.data();
	getRsslRefreshMsg()->groupId.length = (UInt32)DEFAULT_GROUP_ID.size();
}

void RefreshMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslRefreshMsg();
}

void RefreshMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
	_itemGroupData.release();
	_permissionData.release();
}

void RefreshMsgImpl::setQos( UInt32 timeliness , UInt32 rate )
{
	OmmQosDecoder::convertToRssl( &getRsslRefreshMsg()->qos, timeliness, rate );

	rsslRefreshMsgApplyHasQos(getRsslRefreshMsg());

	_qosSet = false;
}

void RefreshMsgImpl::setState( OmmState::StreamState streamState,
							   OmmState::DataState dataState,
							   UInt8 stateCode, const EmaString& text )
{
	getRsslRefreshMsg()->state.streamState = streamState;
	getRsslRefreshMsg()->state.dataState = dataState;
	getRsslRefreshMsg()->state.code = stateCode;

	setStatusText(text.c_str(), text.length());
}

void RefreshMsgImpl::setStatusText(const char* str, UInt32 len)
{
	if (! _arena.trySave(getRsslRefreshMsg()->state.text, str, len))
	{
		_statusText.set(str, len);

		getRsslRefreshMsg()->state.text.data = (char*)_statusText.c_str();
		getRsslRefreshMsg()->state.text.length = _statusText.length();
	}

	_stateSet = false;
}

void RefreshMsgImpl::setSeqNum( UInt32 value )
{
	getRsslRefreshMsg()->seqNum = value;

	rsslRefreshMsgApplyHasSeqNum(getRsslRefreshMsg());
}

void RefreshMsgImpl::setPartNum( UInt16 value )
{
	getRsslRefreshMsg()->partNum = value;

	rsslRefreshMsgApplyHasPartNum(getRsslRefreshMsg());
}

void RefreshMsgImpl::setPublisherId( UInt32 userId, UInt32 userAddress )
{
	getRsslRefreshMsg()->postUserInfo.postUserAddr = userAddress;
	getRsslRefreshMsg()->postUserInfo.postUserId = userId;

	rsslRefreshMsgApplyHasPostUserInfo(getRsslRefreshMsg());
}

void RefreshMsgImpl::setItemGroup( const EmaBuffer& groupId )
{
	if (groupId.length() == 0)
	{
		rsslClearBuffer(&getRsslRefreshMsg()->groupId);
	}
	else
	{
		if (! _arena.trySave(getRsslRefreshMsg()->groupId, groupId))
		{
			_itemGroupData = groupId;

			getRsslRefreshMsg()->groupId.data = (char*)_itemGroupData.c_buf();
			getRsslRefreshMsg()->groupId.length = _itemGroupData.length();
		}
	}
}

void RefreshMsgImpl::setPermissionData( const EmaBuffer& permData )
{
	if (permData.length() == 0)
	{
		rsslClearBuffer(&getRsslRefreshMsg()->permData);
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_HAS_PERM_DATA);
	}
	else
	{
		if (! _arena.trySave(getRsslRefreshMsg()->permData, permData))
		{
			_permissionData = permData;

			getRsslRefreshMsg()->permData.data = (char*)_permissionData.c_buf();
			getRsslRefreshMsg()->permData.length = _permissionData.length();
		}

		rsslRefreshMsgApplyHasPermData(getRsslRefreshMsg());
	}
}

void RefreshMsgImpl::setExtendedHeader( const EmaBuffer& extHeader )
{
	if (extHeader.length() == 0)
	{
		rsslClearBuffer(&getRsslRefreshMsg()->extendedHeader);
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslRefreshMsg()->extendedHeader, extHeader))
		{
			_extHeaderData = extHeader;

			getRsslRefreshMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslRefreshMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslRefreshMsgApplyHasExtendedHdr(getRsslRefreshMsg());
	}
}

void RefreshMsgImpl::setDoNotCache( bool value )
{
	if (value)
		rsslRefreshMsgApplyDoNotCache(getRsslRefreshMsg());
	else
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_DO_NOT_CACHE);
}

void RefreshMsgImpl::setSolicited( bool value )
{
	if (value)
		rsslRefreshMsgApplySolicited(getRsslRefreshMsg());
	else
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_SOLICITED);
}

void RefreshMsgImpl::setClearCache( bool value )
{
	if (value)
		rsslRefreshMsgApplyClearCache(getRsslRefreshMsg());
	else
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_CLEAR_CACHE);
}

void RefreshMsgImpl::setComplete( bool value )
{
	if (value)
		rsslRefreshMsgApplyRefreshComplete(getRsslRefreshMsg());
	else
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_REFRESH_COMPLETE);
}

void RefreshMsgImpl::setPrivateStream( bool value )
{
	if (value)
		rsslRefreshMsgApplyPrivateStream(getRsslRefreshMsg());
	else
		applyHasNoFlag(getRsslRefreshMsg(), RSSL_RFMF_PRIVATE_STREAM);
}
