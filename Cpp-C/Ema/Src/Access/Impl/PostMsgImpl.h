/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_PostMsgImpl_h
#define __refinitiv_ema_access_PostMsgImpl_h

#include "MsgImpl.h"

#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class PostMsgImpl final : public MsgImpl
{
public :

	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslPostMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_POST;

	/// Special members
	//@{

	PostMsgImpl();

	PostMsgImpl& operator=(const PostMsgImpl&);

	PostMsgImpl(const PostMsgImpl&) = delete;
	PostMsgImpl(PostMsgImpl&&) = delete;
	PostMsgImpl& operator=(PostMsgImpl&&) = delete;

	~PostMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslPostMsgCheckHasMsgKey( &_pRsslMsg->postMsg ) == RSSL_TRUE
			? true : false;
	};

	bool hasPostId() const;
	UInt32 getPostId() const;
	void setPostId( UInt32 );

	bool hasPostUserRights() const;
	UInt16 getPostUserRights() const;
	void setPostUserRights( UInt16 );

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader( const EmaBuffer& );

	bool hasSeqNum() const;
	UInt32 getSeqNum() const;
	void setSeqNum( UInt32 );

	bool hasSecondarySeqNum() const;

	bool hasPartNum() const;
	UInt16 getPartNum() const;
	void setPartNum( UInt16 );

	bool hasPermissionData() const;
	const EmaBuffer& getPermissionData() const;
	void setPermissionData( const EmaBuffer& );

	void setPublisherId( UInt32, UInt32 );
	UInt32 getPublisherIdUserId() const;
	UInt32 getPublisherIdUserAddress() const;

	bool getSolicitAck() const;
	void setSolicitAck( bool );

	bool getComplete() const;
	void setComplete( bool );

	void resetRsslInt() { };

	RsslPostMsg* getRsslPostMsg() const
	{
		return &_pRsslMsg->postMsg;
	};

	MsgDecoder<PostMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void releaseMsgBuffers() final;

	void clearRsslPostMsg();

	mutable EmaBufferInt			_extHeader;
	EmaBuffer _extHeaderData;

	mutable EmaBufferInt			_permission;
	EmaBuffer			_permissionData;
};

}

}

}

#endif // __refinitiv_ema_access_PostMsgImpl_h
