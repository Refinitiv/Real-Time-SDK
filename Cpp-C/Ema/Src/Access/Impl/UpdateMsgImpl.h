/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_UpdateMsgImpl_h
#define __refinitiv_ema_access_UpdateMsgImpl_h

#include "UpdateMsg.h"
#include "MsgImpl.h"

#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class UpdateMsgImpl final : public MsgImpl
{
public :

	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslUpdateMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_UPDATE;

	/// Special members
	//@{

	UpdateMsgImpl();

	UpdateMsgImpl& operator=( const UpdateMsgImpl& );

	UpdateMsgImpl(const UpdateMsgImpl&) = delete;
	UpdateMsgImpl(UpdateMsgImpl&&) = delete;
	UpdateMsgImpl& operator=(UpdateMsgImpl &&) = delete;

	~UpdateMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslUpdateMsgCheckHasMsgKey( &_pRsslMsg->updateMsg ) == RSSL_TRUE
			? true : false;
	};

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader( const EmaBuffer& );

	bool hasSeqNum() const;
	UInt32 getSeqNum() const;
	void setSeqNum( UInt32 );

	bool hasPermissionData() const;
	const EmaBuffer& getPermissionData() const;
	void setPermissionData( const EmaBuffer& );

	bool hasConflated() const;
	UInt16 getConflatedTime() const;
	UInt16 getConflatedCount() const;
	void setConflated( UInt16 count, UInt16 time );

	bool getDoNotConflate() const;
	void setDoNotConflate( bool );

	bool hasPublisherId() const;
	UInt32 getPublisherIdUserId() const;
	UInt32 getPublisherIdUserAddress() const;
	void setPublisherId( UInt32 , UInt32 );

	UInt8 getUpdateTypeNum() const;
	void setUpdateTypeNum( UInt8 );

	bool getDoNotCache() const;
	void setDoNotCache( bool );

	bool getDoNotRipple() const;
	void setDoNotRipple( bool );

	void resetRsslInt() { };

	RsslUpdateMsg* getRsslUpdateMsg() const
	{
		return &_pRsslMsg->updateMsg;
	};

	MsgDecoder<UpdateMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void releaseMsgBuffers() final;

	void clearRsslUpdateMsg();

	mutable EmaBufferInt		_extHeader;
	EmaBuffer					_extHeaderData;

	mutable EmaBufferInt		_permission;
	EmaBuffer					_permissionData;
};

}

}

}

#endif // __refinitiv_ema_access_UpdateMsgImpl_h
