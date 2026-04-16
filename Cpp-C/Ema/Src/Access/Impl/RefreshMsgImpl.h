/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_RefreshMsgImpl_h
#define __refinitiv_ema_access_RefreshMsgImpl_h

#include "RefreshMsg.h"
#include "MsgImpl.h"

#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class RefreshMsgImpl final : public MsgImpl
{
public :

	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslRefreshMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_REFRESH;

	/// Special members
	//@{

	RefreshMsgImpl();

	RefreshMsgImpl& operator=(const RefreshMsgImpl&);

	RefreshMsgImpl(const RefreshMsgImpl&) = delete;
	RefreshMsgImpl(RefreshMsgImpl&&) = delete;
	RefreshMsgImpl& operator=(RefreshMsgImpl&&) = delete;

	~RefreshMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslRefreshMsgCheckHasMsgKey( &_pRsslMsg->refreshMsg ) == RSSL_TRUE
			? true : false;
	};

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader( const EmaBuffer& );

	bool hasQos() const;
	const OmmQos& getQos() const;
	void setQos( UInt32 timeliness , UInt32 rate );

	bool hasSeqNum() const;
	UInt32 getSeqNum() const;
	void setSeqNum( UInt32 );

	bool hasPartNum() const;
	UInt16 getPartNum() const;
	void setPartNum( UInt16 );

	bool hasPermissionData() const;
	const EmaBuffer& getPermissionData() const;
	void setPermissionData( const EmaBuffer& );

	bool hasPublisherId() const;
	void setPublisherId( UInt32 , UInt32 );
	UInt32 getPublisherIdUserId() const;
	UInt32 getPublisherIdUserAddress() const;

	const OmmState& getState() const;
	void setState( OmmState::StreamState , OmmState::DataState , UInt8 , const EmaString& );

	const EmaBuffer& getItemGroup() const;
	void setItemGroup( const EmaBuffer& );

	bool getDoNotCache() const;
	void setDoNotCache( bool );

	bool getSolicited() const;
	void setSolicited( bool );

	bool getComplete() const;
	void setComplete( bool );

	bool getClearCache() const;
	void setClearCache( bool );

	bool getPrivateStream() const;
	void setPrivateStream( bool );

	void resetRsslInt() { _stateSet = false; _qosSet = false; };

	RsslRefreshMsg* getRsslRefreshMsg() const
	{
		return &_pRsslMsg->refreshMsg;
	};

	MsgDecoder<RefreshMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void releaseMsgBuffers() final;

	void clearRsslRefreshMsg();

	void setStatusText(const char* str, UInt32 len);

	mutable bool					_stateSet;
	mutable bool					_qosSet;

	mutable EmaBufferInt			_extHeader;
	EmaBuffer 						_extHeaderData;

	mutable EmaBufferInt			_permission;

	mutable EmaBufferInt			_itemGroup;
	mutable EmaBuffer				_itemGroupData;

	mutable OmmState				_state;

	mutable OmmQos					_qos;

	EmaBuffer						_permissionData;

	EmaString						_statusText;
};

}

}

}

#endif // __refinitiv_ema_access_RefreshMsgImpl_h
