/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_StatusMsgImpl_h
#define __refinitiv_ema_access_StatusMsgImpl_h

#include "Access/Include/Msg.h"
#include "Access/Include/OmmState.h"

#include "MsgImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class StatusMsgImpl final : public MsgImpl
{
public:

	// This is needed to know how much to copy (some classess pass a pointer to
	// RsslStatusMsg instead of RsslMsg when the StaticDecoder is asked to create an Ema
	// message for it
	using RsslMsgType = RsslStatusMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_STATUS;

	/// Special members
	//@{

	StatusMsgImpl();

	StatusMsgImpl& operator=(const StatusMsgImpl&);

	StatusMsgImpl(const StatusMsgImpl&) = delete;
	StatusMsgImpl(StatusMsgImpl&&) = delete;
	StatusMsgImpl& operator=(StatusMsgImpl&&) = delete;

	~StatusMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslStatusMsgCheckHasMsgKey( &_pRsslMsg->statusMsg ) == RSSL_TRUE
			? true : false;
	};

	DataType::DataTypeEnum getDataType() const;

	Data::DataCode getCode() const;

	bool hasItemGroup() const;
	const EmaBuffer& getItemGroup() const;
	void setItemGroup(const EmaBuffer& itemGroup);

	bool hasPermissionData() const;
	const EmaBuffer& getPermissionData() const;
	void setPermissionData(const EmaBuffer& permissionData);

	bool hasPublisherId() const;
	UInt32 getPublisherIdUserId() const;
	UInt32 getPublisherIdUserAddress() const;
	void setPublisherId(UInt32 userId, UInt32 userAddress);

	bool hasState() const;
	const OmmState& getState() const;
	void setState(OmmState::StreamState streamState = OmmState::OpenEnum,
				  OmmState::DataState dataState = OmmState::OkEnum,
				  UInt8 statusCode = OmmState::NoneEnum,
				  const EmaString& statusText = EmaString());

	bool getClearCache() const;
	void setClearCache(bool clearCache = false);

	bool getPrivateStream() const;
	void setPrivateStream(bool privateStream = false);

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader(const EmaBuffer& buffer);

	void resetRsslInt() { _stateSet = false; };

	RsslStatusMsg* getRsslStatusMsg() const
	{
		return &_pRsslMsg->statusMsg;
	};

	MsgDecoder<StatusMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;

	void setStatusText(const char* str, UInt32 len);

	void releaseMsgBuffers() final;

	void clearRsslStatusMsg();

	mutable bool				_stateSet;

	mutable EmaBufferInt		_extHeader;
	EmaBuffer					_extHeaderData;

	mutable EmaBufferInt		_permission;
	EmaBuffer					_permissionData;

	mutable EmaBufferInt		_itemGroup;
	EmaBuffer					_itemGroupData;

	mutable OmmState			_state;

	EmaString					_statusTextData;
};

}

}

}

#endif // __refinitiv_ema_access_StatusMsgImpl_h
