/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ReqMsgImpl_h
#define __refinitiv_ema_access_ReqMsgImpl_h

#include "ReqMsg.h"
#include "MsgImpl.h"

#include "OmmQos.h"
#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class ReqMsgImpl final : public MsgImpl
{
public :

	// RsslMsg type needed by MsgImpl
	using RsslMsgType = RsslRequestMsg;
	typedef RsslBuffer RsslMsgType::*BufMemberType;
	constexpr static RsslMsgClasses RSSL_MSG_CLASS = RSSL_MC_REQUEST;

	/// Special members
	//@{

	ReqMsgImpl();

	ReqMsgImpl& operator=( const ReqMsgImpl& );

	ReqMsgImpl(const ReqMsgImpl&) = delete;
	ReqMsgImpl(ReqMsgImpl&&) = delete;
	ReqMsgImpl& operator=( ReqMsgImpl&& ) = delete;

	~ReqMsgImpl();

	//@}

	void clear();

	bool hasMsgKey() const final
	{
		// note: msgKey is *required* for an RsslRequestMsg (which implies it is always present)
		return true;
	};

	bool hasExtendedHeader() const final;
	const EmaBuffer& getExtendedHeader() const final;
	void setExtendedHeader( const EmaBuffer& );

	bool hasPriority() const;
	UInt8 getPriorityClass() const;
	UInt16 getPriorityCount() const;
	void setPriority( UInt8 , UInt16 );

	bool hasQos() const;
	UInt32 getTimeliness() const;
	UInt32 getRate() const;
	void setQos( UInt32 timeliness, UInt32 rate );

	bool hasView() const;

	bool hasBatch() const;

	bool getInitialImage() const;
	void setInitialImage( bool initialImage );

	bool getInterestAfterRefresh() const;
	void setInterestAfterRefresh( bool interestAfterRefresh );

	bool getConflatedInUpdates() const;
	void setConflatedInUpdates( bool conflatedInUpdates );

	bool getPause() const;
	void setPause( bool pause );

	bool getPrivateStream() const;
	void setPrivateStream( bool privateStream );

	// Fills the EmaVector with the batch item names, if found.
	// Returns the number of items in the batch request
	UInt32 getBatchItemList(EmaVector<EmaString>*) const;

	// Fills the EmaBuffer with the View elements.
	// This will only get the view out of the payload, not any batch item names.
	bool getViewPayload(EmaBuffer&) const;

	bool isDomainTypeSet() const;

	void resetRsslInt() { };

	RsslRequestMsg* getRsslRequestMsg() const
	{
		return &_pRsslMsg->requestMsg;
	};

	MsgDecoder<ReqMsgImpl> _decoder;

private :

	void applyHasMsgKey() final;
	void adjustPayload() final;
	void releaseMsgBuffers() final;

	void checkBatchView( RsslBuffer* );

	void clearRsslRequestMsg();

	mutable EmaBufferInt			_extHeader;
	EmaBuffer						_extHeaderData;

	bool							_domainTypeSet;
};

}

}

}

#endif // __refinitiv_ema_access_ReqMsgImpl_h
