/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MsgImpl_h
#define __refinitiv_ema_access_MsgImpl_h

#include "ComplexType.h"
#include "NoDataImpl.h"

#include "MsgDecoder.h"
#include "MsgEncoder.h"

#include "EmaStringInt.h"
#include "FixedBuffer.h"

#include <cstdint>
#include <type_traits>

namespace refinitiv {

namespace ema {

namespace access {

class Msg;

class MsgImpl
{
public :

	// See applyHasMsgKey in the "private" section below.  hasMsgKey flag is raised in the
	// concrete message struct, while the key itslef is in the base msg
	virtual bool hasMsgKey() const = 0;

	/// Getters and Setters
	//{

	bool hasFilter() const;
	UInt32 getFilter() const;
	void setFilter(UInt32);
	void addFilter( UInt32 filter );

	template<class MsgImplT>
	bool hasAttrib() const
	{
		return ( static_cast<MsgImplT*>(this)->hasMsgKey()
				 && rsslMsgKeyCheckHasAttrib( getRsslMsgKey() ) == RSSL_TRUE
				 && getRsslMsgKey()->attribContainerType != RSSL_DT_NO_DATA )
			? true : false;
	};

	void setAttrib(const ComplexType&);
	const Data& getAttribData() const;

	bool hasPayload() const;
	void setPayload(const ComplexType&);
	const Data& getPayloadData() const;

	Int32 getStreamId() const;
	void setStreamId(Int32);

	UInt16 getDomainType() const;
	void setDomainType(UInt16);

	bool hasName() const;
	const EmaString& getName() const;
	void setName(const EmaString&);

	bool hasNameType() const;
	UInt8 getNameType() const;
	void setNameType(UInt8);

	bool hasServiceId() const;
	UInt32 getServiceId() const;
	void setServiceId(UInt16);

	bool hasServiceName() const;
	const EmaString& getServiceName() const;
	/** Performs a deep copy of the provided string into the private member variable. */
	void copyServiceName(const EmaString&);
	/** Adjusts internal pointers to the provided string, does not claim ownership over
		provided string's buffers. */
	void setServiceNameInt(const EmaString& serviceName)
	{
		_serviceNameSet = serviceName.length() ? true : false;
		_serviceName.setInt(serviceName.c_str(), serviceName.length(), true);
	};
	/** Marks this message as having no service name */
	void resetServiceName() { _serviceNameSet = false; };

	bool hasId() const;
	Int32 getId() const;
	void setId(Int32);

	bool hasServiceListName() const;
	const EmaString& getServiceListName() const;
	void setServiceListName(const EmaString&);

	virtual bool hasExtendedHeader() const = 0;
	virtual const EmaBuffer& getExtendedHeader() const = 0;

	//}

	void setAtExit();

	const RsslDataDictionary* getRsslDictionary();

	UInt8 getMajorVersion();
	UInt8 getMinorVersion();

	RsslMsg* getRsslMsg() const { return _pRsslMsg; };
	RsslMsgKey* getRsslMsgKey() const { return &_pRsslMsg->msgBase.msgKey; };

	const EmaBuffer& getAsHex() const;

	FixedBuffer& Arena() { return _arena; };

	// negate the message-specific or RsslMsgKey flag
	template<typename T, typename U>
	void applyHasNoFlag(T* msg, const U flag)
	{
		msg->flags &= ~flag;
	}

	void clearBaseMsgImpl();

	// clears the message impl instance to make it ready to be recycled by another
	// messsage, releases encoder while this impl is idle in the pool
	template<typename MsgImplT>
	void deallocateBuffers()
	{
		static_cast<MsgImplT*>(this)->clear();

		_encoder.release();
	}

	/// Proxies
	//{

	// Proxy functions providing library code access to the private implementation
	// details of different Message types without flooding public header files with
	// "friend class" declarations
	//
	// T is concrete message type (AckMsg, StatusMsg, etc.) derived from Msg
	// returns pointer to concrete impl  (AckMsgImpl, StatusMsgImpl, etc.)
	template<typename T,
			 typename std::enable_if<std::is_base_of<Msg, T>::value, bool>::type = true >
	constexpr static auto getImpl(T& msg) -> decltype(msg.impl()) { return static_cast<decltype(msg.impl())>(msg._pImpl); };

	template<typename T,
			 typename std::enable_if<std::is_base_of<Msg, T>::value, bool>::type = true >
	constexpr static auto getImpl(const T& msg) -> decltype(msg.impl()) { return static_cast<decltype(msg.impl())>(msg._pImpl); };

	// Msg base class is different - it doesn't have the impl() member function
	template<typename T,
			 typename std::enable_if<std::is_same<Msg, T>::value, bool>::type = true >
	constexpr static MsgImpl* getImpl(const T& msg) { return msg._pImpl; };

	//}

	MsgEncoder _encoder;

protected :

	template<typename T> friend class MsgDecoder;

	MsgImpl();

	MsgImpl(const MsgImpl&) = delete;
	MsgImpl(MsgImpl&&) = delete;
	MsgImpl& operator=( MsgImpl&& ) = delete;

	virtual ~MsgImpl();

	RsslMsg*						_pRsslMsg;
	const RsslDataDictionary*		_pRsslDictionary;

	UInt8							_rsslMajVer;
	UInt8							_rsslMinVer;

	bool							_serviceNameSet;
	bool							_serviceListNameSet;

	RsslMsg							_rsslMsg;

	Data*							_attrib;
	Data*							_payload;

	mutable EmaStringInt			_name;
	EmaString						_nameData;

	EmaBuffer						_attribData;
	EmaBuffer						_payloadData;

	mutable EmaBufferInt			_hexBuffer;

	EmaStringInt					_serviceName;
	EmaString						_serviceNameData;

	EmaString						_serviceListName;

	FixedBuffer						_arena;

	/// Perform deep copy for the msgBase part of the message from the other message
	void copyMsgBaseFrom(const MsgImpl& other, size_t msgSize);

	/// returns true when the needle is completely within the given region, like when a
	/// decoded message RsslBuffer name field points into encMsgBuffer
	static constexpr bool isContainedWithin(const RsslBuffer& needle, const RsslBuffer& region)
	{
		return (region.data != nullptr
				&& ((uintptr_t)needle.data >= (uintptr_t)region.data
					&& (uintptr_t)needle.data + (uintptr_t)needle.length <= (uintptr_t)region.data + (uintptr_t)region.length));
	};

	/// adjusts .data pointer into the encoded message buffer encDstBuf at the same offset
	/// as the original srcBuf in encSrcBuf
	bool memberCopyInPlace(RsslBuffer& dstBuf, const RsslBuffer& srcBuf, const RsslBuffer& encDstBuf, const RsslBuffer& encSrcBuf)
	{
		if (isContainedWithin(srcBuf, encSrcBuf))
		{
			ptrdiff_t offset = srcBuf.data - encSrcBuf.data;

			dstBuf.data = encDstBuf.data + offset;
			dstBuf.length = srcBuf.length;

			return true;
		}
		return false;
	};

	template <typename T>
	bool memberCopyInPlace(typename T::BufMemberType memberPtr, typename T::RsslMsgType& dstMsg, const typename T::RsslMsgType& srcMsg)
	{
		return memberCopyInPlace(dstMsg.*memberPtr, srcMsg.*memberPtr,
								 dstMsg.msgBase.encMsgBuffer, srcMsg.msgBase.encMsgBuffer);
	};

private :

	void copyName(const MsgImpl& other);
	void copyAttrib(const MsgImpl& other);
	void copyPayload(const MsgImpl& other);

	// raise message specific HAS_MSG_KEY flag, like RSSL_AKMF_HAS_MSG_KEY
	virtual void applyHasMsgKey() = 0;

	// release memory used by message-specific buffers
	virtual void releaseMsgBuffers() = 0;

	// perform optional message-specific adjustments after payload is set, like ReqMsg
	// handling batch requests
	virtual void adjustPayload() { };

	char _attrib_placeholder[sizeof(NoDataImpl)];
	char _payload_placeholder[sizeof(NoDataImpl)];
};

}

}

}

#endif //__refinitiv_ema_access_MsgImpl_h
