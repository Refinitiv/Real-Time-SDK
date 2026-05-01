/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GenericMsgImpl_h
#define __refinitiv_ema_access_GenericMsgImpl_h

#include "GenericMsg.h"
#include "MsgImpl.h"

#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class GenericMsgImpl final : public MsgImpl
{
public :

	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslGenericMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_GENERIC;

	/// Special members
	//@{

	GenericMsgImpl();

	GenericMsgImpl& operator=(const GenericMsgImpl&);

	GenericMsgImpl(const GenericMsgImpl&) = delete;
	GenericMsgImpl(GenericMsgImpl&&) = delete;
	GenericMsgImpl& operator=( GenericMsgImpl&& ) = delete;

	~GenericMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		return rsslGenericMsgCheckHasMsgKey( &_pRsslMsg->genericMsg ) == RSSL_TRUE
			? true : false;
	};

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader( const EmaBuffer& );

	bool hasSeqNum() const;
	UInt32 getSeqNum() const;
	void setSeqNum( UInt32 );

	bool hasSecondarySeqNum() const;
	UInt32 getSecondarySeqNum() const;
	void setSecondarySeqNum( UInt32 );

	bool hasPartNum() const;
	UInt16 getPartNum() const;
	void setPartNum( UInt16 );

	bool hasPermissionData() const;
	const EmaBuffer& getPermissionData() const;
	void setPermissionData( const EmaBuffer& );

	bool getComplete() const;
	void setComplete( bool );

	void setProviderDriven( bool );

	void resetRsslInt() { };

	RsslGenericMsg* getRsslGenericMsg() const
	{
		return &_pRsslMsg->genericMsg;
	};

	MsgDecoder<GenericMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void releaseMsgBuffers() final;

	void clearRsslGenericMsg();

	mutable EmaBufferInt	_extHeader;
	EmaBuffer				_extHeaderData;

	mutable EmaBufferInt	_permission;
	EmaBuffer				_permissionData;

};

}

}

}

#endif // __refinitiv_ema_access_GenericMsgImpl_h
