/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_ClientSession_h
#define __thomsonreuters_ema_access_ClientSession_h

#include "Common.h"
#include "HashTable.h"
#include "ItemInfo.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "rtr/rsslReactor.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmServerBaseImpl;

class ClientSession : public ListLinks<ClientSession>
{
public:

	static ClientSession* create(OmmServerBaseImpl*);

	static void destroy(ClientSession*&);

	UInt64 getClientHandle() const;

	UInt64 getLoginHandle() const;

	void setLoginHandle(UInt64 handle);

	RsslReactorChannel* getChannel() const;

	ItemInfo* getItemInfo(Int32) const;

	void setChannel(RsslReactorChannel*);

	void addItemInfo(ItemInfo* itemInfo);

	void removeItemInfo(ItemInfo* itemInfo);

	bool checkingExistingReq(ItemInfo * itemInfo);

	void closeAllItemInfo();

	bool isLogin();

	void setLogin(bool isLogin);

	bool isADHSession() const;

	void setADHSession(bool adhSession = true);

	const EmaString& toString() const;

	const EmaList< ItemInfo* >& getItemInfoList();

	class UInt64rHasher
	{
	public:
		size_t operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	class EmaBufferHasher
	{
	public:
		size_t operator()(const EmaBuffer&) const;
	};

	class EmaBufferEqual_To
	{
	public:
		bool operator()(const EmaBuffer&, const EmaBuffer&) const;
	};

	size_t static hashCodeBuffer(const EmaBuffer& value);

	typedef HashTable< EmaBuffer, EmaVector<ItemInfo*>*, EmaBufferHasher, EmaBufferEqual_To > GroupIdToInfoHash;

	typedef HashTable< UInt64, GroupIdToInfoHash*, UInt64rHasher, UInt64Equal_To > ServiceGroupToItemInfoHash;

	EmaVector<GroupIdToInfoHash*>&		getServiceGroupToItemInfoList();

	ServiceGroupToItemInfoHash&			getServiceGroupToItemInfoHash();

	EmaVector<EmaVector<ItemInfo*>*>&			getItemInfoVectorList();

private:

	ClientSession(OmmServerBaseImpl*);

	virtual ~ClientSession();

	class Int32rHasher
	{
	public:
		Int32 operator()(const Int32&) const;
	};

	class Int32Equal_To
	{
	public:
		bool operator()(const Int32&, const Int32&) const;
	};

	typedef HashTable< Int32, ItemInfo*, Int32rHasher, Int32Equal_To > StreamIdToItemInfoHash;

	class ItemInfoHasher
	{
	public:
		Int32 operator()(const ItemInfo*) const;
	};

	class ItemInfoEqual_To
	{
	public:
		bool operator()(const ItemInfo*, const ItemInfo*) const;
	};

	typedef HashTable< ItemInfo*, ItemInfo*, ItemInfoHasher, ItemInfoEqual_To > ItemInfoToItemInfoHash;

	EmaVector<GroupIdToInfoHash*>		_serviceGroupToItemInfoList;
	ServiceGroupToItemInfoHash			_serviceGroupToItemInfoHash;

	EmaVector<EmaVector<ItemInfo*>*>	_itemInfoVectorList;

	StreamIdToItemInfoHash		_streamIdToItemInfoHash;

	EmaList< ItemInfo* >		_itemInfoList;

	ItemInfoToItemInfoHash*		_pItemInfoItemInfoHash;

	RsslReactorChannel* _pChannel;

	mutable bool		_toStringSet;
	mutable EmaString	_toString;
	bool				_isLogin;
	OmmServerBaseImpl*  _pOmmServerBaseImpl;
	bool                _removingInCloseAll;
	bool				_isADHSession;
	UInt64				_loginHandle;
};

}

}

}

#endif // __thomsonreuters_ema_access_ClientSession_h
