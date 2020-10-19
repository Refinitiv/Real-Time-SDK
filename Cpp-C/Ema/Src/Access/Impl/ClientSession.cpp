/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the projects LICENSE.md for details.                   --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "ClientSession.h"
#include "OmmServerBaseImpl.h"
#include "DictionaryHandler.h"
#include "DirectoryHandler.h"
#include "LoginHandler.h"
#include "Utilities.h"

using namespace refinitiv::ema::access;


ClientSession::ClientSession(OmmServerBaseImpl* ommServerBaseImpl) :
	_pChannel(0),
	_toStringSet(false),
	_isLogin(false),
	_toString(),
	_pOmmServerBaseImpl(ommServerBaseImpl),
	_removingInCloseAll(false),
	_isADHSession(false),
	_loginHandle(0)
{
	_streamIdToItemInfoHash.rehash(_pOmmServerBaseImpl->getActiveConfig().itemCountHint);

	if (!_pOmmServerBaseImpl->getActiveConfig().acceptMessageSameKeyButDiffStream)
	{
		_pItemInfoItemInfoHash = new ItemInfoToItemInfoHash(_pOmmServerBaseImpl->getActiveConfig().itemCountHint);
	}
	else
	{
		_pItemInfoItemInfoHash = 0;
	}
}

ClientSession::~ClientSession()
{
	if (_pItemInfoItemInfoHash)
	{
		delete _pItemInfoItemInfoHash;
		_pItemInfoItemInfoHash = 0;
	}

	for (UInt32 index = 0; index < _serviceGroupToItemInfoList.size(); index++)
	{
		delete _serviceGroupToItemInfoList[index];
	}

	for (UInt32 index = 0; index < _itemInfoVectorList.size(); index++)
	{
		delete _itemInfoVectorList[index];
	}
}

ClientSession* ClientSession::create(OmmServerBaseImpl* ommServerBaseimpl)
{
	try {
		return new ClientSession(ommServerBaseimpl);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseimpl->handleMee("Failed to create ClientSession");
	}

	return NULL;
}

void ClientSession::destroy(ClientSession*& pClientSession)
{
	if (pClientSession)
	{
		delete pClientSession;
		pClientSession = 0;
	}
}

UInt64 ClientSession::getClientHandle() const
{
	return (UInt64)_pChannel;
}

UInt64 ClientSession::getLoginHandle() const
{
	return _loginHandle;
}

void ClientSession::setLoginHandle(UInt64 handle)
{
	_loginHandle = handle;
}

void ClientSession::setChannel(RsslReactorChannel* channel)
{
	_pChannel = channel;
}

RsslReactorChannel* ClientSession::getChannel() const
{
	return _pChannel;
}

void ClientSession::addItemInfo(ItemInfo* itemInfo)
{
	if (_streamIdToItemInfoHash.insert(itemInfo->getStreamId(), itemInfo))
	{
		_itemInfoList.push_back(itemInfo);
	}
}

void ClientSession::removeItemInfo(ItemInfo* itemInfo)
{
	if (_removingInCloseAll == false)
	{
		if (_streamIdToItemInfoHash.erase(itemInfo->getStreamId()) == 1)
		{
			_itemInfoList.remove(itemInfo);

			if (_pItemInfoItemInfoHash)
			{
				_pItemInfoItemInfoHash->erase(itemInfo);
			}
		}
	}
}

bool ClientSession::checkingExistingReq(ItemInfo * itemInfo)
{
	if (!_pItemInfoItemInfoHash)
	{
		return false;
	}

	_pOmmServerBaseImpl->getUserMutex().lock();

	if (_pItemInfoItemInfoHash->insert(itemInfo, 0))
	{
		_pOmmServerBaseImpl->getUserMutex().unlock();
		return false;
	}
	else
	{
		_pOmmServerBaseImpl->getUserMutex().unlock();
		return true;
	}
}

 ItemInfo* ClientSession::getItemInfo(Int32 streamId) const
{
	ItemInfo** itemInfoPtr = _streamIdToItemInfoHash.find(streamId);

	return itemInfoPtr ? *itemInfoPtr : 0;
}

void ClientSession::closeAllItemInfo()
{
	ItemInfo* prevItemInfo = _itemInfoList.back();
	ItemInfo* itemInfo;

	_removingInCloseAll = true;

	while (prevItemInfo)
	{
		itemInfo = prevItemInfo;

		switch (itemInfo->getDomainType())
		{
		case ema::rdm::MMT_DICTIONARY:
			_pOmmServerBaseImpl->getDictionaryHandler().removeItemInfo(itemInfo);
			break;
		case ema::rdm::MMT_DIRECTORY:
			_pOmmServerBaseImpl->getDirectoryHandler().removeItemInfo(itemInfo);
			break;
		case ema::rdm::MMT_LOGIN:
			setLoginHandle(0);
			_pOmmServerBaseImpl->getLoginHandler().removeItemInfo(itemInfo);
			break;
		}

		prevItemInfo = itemInfo->previous();

		_pOmmServerBaseImpl->removeItemInfo(itemInfo, false);
	}

	_streamIdToItemInfoHash.clear();

	if (_pItemInfoItemInfoHash)
	{
		_pItemInfoItemInfoHash->clear();
	}

	for (UInt32 index = 0; index < _serviceGroupToItemInfoList.size(); index++)
	{
		delete _serviceGroupToItemInfoList[index];
	}

	_serviceGroupToItemInfoList.clear();

	for (UInt32 index = 0; index < _itemInfoVectorList.size(); index++)
	{
		delete _itemInfoVectorList[index];
	}

	_itemInfoVectorList.clear();

	_serviceGroupToItemInfoHash.clear();

	_removingInCloseAll = false;
}

const EmaList< ItemInfo* >& ClientSession::getItemInfoList()
{
	return _itemInfoList;
}

EmaVector< ClientSession::GroupIdToInfoHash*>& ClientSession::getServiceGroupToItemInfoList()
{
	return _serviceGroupToItemInfoList;
}

ClientSession::ServiceGroupToItemInfoHash& ClientSession::getServiceGroupToItemInfoHash()
{
	return _serviceGroupToItemInfoHash;
}

EmaVector<EmaVector<ItemInfo*>*>& ClientSession::getItemInfoVectorList()
{
	return _itemInfoVectorList;
}

const EmaString& ClientSession::toString() const
{
	if (!_toStringSet)
	{
		_toStringSet = true;
		_toString.set("\tClient handle = ").append(getClientHandle());
	}
	return _toString;
}

bool ClientSession::isLogin()
{
	return _isLogin;
}

void ClientSession::setLogin(bool isLogin)
{
	_isLogin = isLogin;
}

bool ClientSession::isADHSession() const
{
	return _isADHSession;
}

void ClientSession::setADHSession(bool adhClient)
{
	_isADHSession = adhClient;
}

Int32 ClientSession::Int32rHasher::operator()(const Int32& value) const
{
	return value;
}

bool ClientSession::Int32Equal_To::operator()(const Int32& x, const Int32& y) const
{
	return x == y ? true : false;
}

Int32 ClientSession::ItemInfoHasher::operator()(const ItemInfo* value) const
{
	return rsslMsgKeyHash(&value->getRsslMsgKey());
}

bool ClientSession::ItemInfoEqual_To::operator()(const ItemInfo* x, const ItemInfo* y) const
{
	return *x == *y;
}

size_t ClientSession::hashCodeBuffer(const EmaBuffer& value)
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value.c_buf();
	UInt32 n = value.length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)* s++;
	return result;
}

size_t ClientSession::EmaBufferHasher::operator()(const EmaBuffer& value) const
{
	return hashCodeBuffer(value);
}

bool ClientSession::EmaBufferEqual_To::operator()(const EmaBuffer& x, const EmaBuffer& y) const
{
	return hashCodeBuffer(x) == hashCodeBuffer(y) ? true : false;
}

size_t ClientSession::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool ClientSession::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y ? true : false;
}

