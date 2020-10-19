/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_ItemInfo_h
#define __refinitiv_ema_access_ItemInfo_h

#include "EmaString.h"
#include "EmaBuffer.h"
#include "EmaList.h"
#include "Rdm/Include/EmaRdm.h"
#include "rtr/rsslMsg.h"

#include "HashTable.h"

namespace refinitiv {

namespace ema {

namespace access {

class ClientSession;
class OmmServerBaseImpl;

class ItemInfo : public ListLinks<ItemInfo>
{
public:

	enum ItemInfoFlags
	{
		None = 0x0000,
		StreamingFlag = 0x0001,
		PrivateStreamFlag = 0x0002,
		ItemGroupFlag = 0x0004,
		MsgKeyFlag = 0x0008
	};

	static ItemInfo* create(OmmServerBaseImpl&);

	static void destroy(ItemInfo*&);

	bool hasItemGroup() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasServiceId() const;

	bool hasFilter() const;

	UInt64 getHandle() const;

	Int32 getStreamId() const;

	UInt16 getServiceId() const;

	UInt32 getFilter() const;

	const EmaString& getName() const;

	UInt8 getNameType() const;

	UInt32 getFlags() const;

	UInt8 getDomainType() const;

	const EmaBuffer& getItemGroup() const;

	const RsslMsgKey& getRsslMsgKey() const;

	ClientSession* getClientSession() const;

	bool isPrivateStream() const;

	bool isSentRefresh() const;

	void setStreamId(Int32 streamId);

	void setServiceId(UInt16 serviceId);

	void setNameType(UInt8 nameType);

	void setFlags(UInt32 flags);

	void setDomainType(UInt8 domainType);

	void setItemGroup(const RsslBuffer& itemGroup);

	void setFilter(UInt32 filter);

	void setClientSession(ClientSession* clientSession);

	bool setRsslRequestMsg(RsslRequestMsg&);

	void setSentRefresh();

	bool operator==(const ItemInfo&) const;

	void addPostId(UInt32 postId);
	bool removePostId(UInt32 postId);

private:

	ItemInfo(OmmServerBaseImpl& ommServerBaseimpl);

	virtual ~ItemInfo();

	bool _sentRefresh;
	Int32 _streamId;
	EmaString _name;
	UInt32 _flags;
	UInt8 _domainType;
	EmaBuffer _itemGroup;
	RsslMsgKey _rsslMsgKey;
	ClientSession* _pClientSession;
	OmmServerBaseImpl& _ommServerBaseimpl;

	class UInt32rHasher
	{
	public:
		size_t operator()(const UInt32&) const;
	};

	class UInt32Equal_To
	{
	public:
		bool operator()(const UInt32&, const UInt32&) const;
	};

	typedef HashTable< UInt32, UInt32, UInt32rHasher, UInt32Equal_To > PostIdHash;
	PostIdHash* _pPostIdHash;
};

}

}

}

#endif // __refinitiv_ema_access_ItemInfo_h
