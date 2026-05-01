/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_AckMsgImpl_h
#define __refinitiv_ema_access_AckMsgImpl_h

#include "AckMsg.h"
#include "MsgImpl.h"

#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class AckMsgImpl final : public MsgImpl
{
public :
	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslAckMsg;
	typedef RsslBuffer RsslAckMsg::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_ACK;

	/// Special members
	//@{

	AckMsgImpl();
	AckMsgImpl& operator=(const AckMsgImpl&);

	AckMsgImpl(const AckMsgImpl&) = delete;
	AckMsgImpl(AckMsgImpl&&) = delete;
	AckMsgImpl& operator=(AckMsgImpl&&) = delete;

	~AckMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslAckMsgCheckHasMsgKey( &_pRsslMsg->ackMsg ) == RSSL_TRUE
			? true : false;
	};

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader(const EmaBuffer& buffer);

	bool hasSeqNum() const;
	UInt32 getSeqNum() const;
	void setSeqNum( UInt32 );

	bool hasNackCode() const;
	UInt8 getNackCode() const;
	void setNackCode( UInt8 );

	bool hasText() const;
	const EmaString& getText() const;
	void setText( const EmaString& );

	UInt32 getAckId() const;
	void setAckId( UInt32 );

	bool getPrivateStream() const;
	void setPrivateStream( bool );

	void resetRsslInt() { };

	RsslAckMsg* getRsslAckMsg() const
	{
		return &_pRsslMsg->ackMsg;
	};

	MsgDecoder<AckMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void releaseMsgBuffers() final;

	void clearRsslAckMsg();

	mutable EmaStringInt	_text;
	EmaString				_textData;

	mutable EmaBufferInt	_extHeader;
	EmaBuffer				_extHeaderData;

};

}

}

}

#endif // __refinitiv_ema_access_AckMsgImpl_h
