/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "ItemInfo.h"
#include "OmmServerBaseImpl.h"
#include "rtr/rsslRDM.h"

#include "OmmIProviderImpl.h"

#include <new>

using namespace thomsonreuters::ema::access;

ItemInfo::ItemInfo(OmmServerBaseImpl& ommServerBaseimpl) :
_streamId(0),
_name(),
_flags(None),
_itemGroup(),
_pClientSession(0),
_sentRefresh(false),
_ommServerBaseimpl(ommServerBaseimpl),
_pPostIdHash(0)
{
	rsslClearMsgKey(&_rsslMsgKey);

	if (static_cast<OmmIProviderActiveConfig&>(_ommServerBaseimpl.getActiveConfig()).getEnforceAckIDValidation())
	{
		_pPostIdHash = new PostIdHash();
	}
}

ItemInfo::~ItemInfo()
{
	if (_pPostIdHash)
	{
		delete _pPostIdHash;
	}

	if (_rsslMsgKey.encAttrib.data)
	{
		free(_rsslMsgKey.encAttrib.data);
	}

	if (_rsslMsgKey.name.data)
	{
		free(_rsslMsgKey.name.data);
	}
}

ItemInfo* ItemInfo::create(OmmServerBaseImpl& ommServerBaseimpl)
{
	try {
		return new ItemInfo(ommServerBaseimpl);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseimpl.handleMee("Failed to create ItemInfo");
	}

	return NULL;
}

void ItemInfo::destroy(ItemInfo*& itemInfo)
{
	if (itemInfo)
	{
		delete itemInfo;
		itemInfo = 0;
	}
}

bool ItemInfo::hasItemGroup() const
{
	return ( (_flags & ItemGroupFlag ) == ItemGroupFlag );
}

bool ItemInfo::isPrivateStream() const
{
	return ((_flags & PrivateStreamFlag) == PrivateStreamFlag);
}

bool ItemInfo::hasName() const
{
	if (_rsslMsgKey.flags & RSSL_MKF_HAS_NAME)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ItemInfo::hasNameType() const
{
	if (_rsslMsgKey.flags & RSSL_MKF_HAS_NAME_TYPE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ItemInfo::hasServiceId() const
{
	if (_rsslMsgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ItemInfo::hasFilter() const
{
	if (_rsslMsgKey.flags & RSSL_MKF_HAS_FILTER)
	{
		return true;
	}
	else
	{
		return false;
	}
}

const RsslMsgKey& ItemInfo::getRsslMsgKey() const
{
	return _rsslMsgKey;
}

UInt64 ItemInfo::getHandle() const
{
	return (UInt64)this;
}

Int32 ItemInfo::getStreamId() const
{
	return _streamId;
}

UInt16 ItemInfo::getServiceId() const
{
	return _rsslMsgKey.serviceId;
}

UInt32 ItemInfo::getFilter() const
{
	return _rsslMsgKey.filter;
}

const EmaString& ItemInfo::getName() const
{
	return _name;
}

UInt8 ItemInfo::getNameType() const
{
	return _rsslMsgKey.nameType;
}

UInt32 ItemInfo::getFlags() const
{
	return _flags;
}

UInt8 ItemInfo::getDomainType() const
{
	return _domainType;
}

const EmaBuffer& ItemInfo::getItemGroup() const
{
	return _itemGroup;
}

ClientSession* ItemInfo::getClientSession() const
{
	return _pClientSession;
}

bool ItemInfo::isSentRefresh() const
{
	return _sentRefresh;
}

bool ItemInfo::setRsslRequestMsg(RsslRequestMsg& requestMsg)
{
	_streamId = requestMsg.msgBase.streamId;
	_domainType = requestMsg.msgBase.domainType;

	if ((requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) && (requestMsg.msgBase.msgKey.encAttrib.length > 0) )
	{
		if (_rsslMsgKey.encAttrib.length < requestMsg.msgBase.msgKey.encAttrib.length )
		{
			if (_rsslMsgKey.encAttrib.data)
			{
				free(_rsslMsgKey.encAttrib.data);
			}

			_rsslMsgKey.encAttrib.length = requestMsg.msgBase.msgKey.encAttrib.length;
			_rsslMsgKey.encAttrib.data = (char*)malloc(sizeof(char) * _rsslMsgKey.encAttrib.length);

			if (!_rsslMsgKey.encAttrib.data)
			{
				_ommServerBaseimpl.handleMee("Failed to allocate memory in ItemInfo::setRsslRequestMsg()");
				return false;
			}
		}
	}

	if ((requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) && (requestMsg.msgBase.msgKey.name.length > 0))
	{
		if (_rsslMsgKey.name.length < requestMsg.msgBase.msgKey.name.length)
		{
			if (_rsslMsgKey.name.data)
			{
				free(_rsslMsgKey.name.data);
			}

			_rsslMsgKey.name.length = requestMsg.msgBase.msgKey.name.length;
			_rsslMsgKey.name.data = (char*)malloc(sizeof(char) * _rsslMsgKey.name.length);

			if (!_rsslMsgKey.name.data)
			{
				_ommServerBaseimpl.handleMee("Failed to allocate memory in ItemInfo::setRsslRequestMsg()");
				return false;
			}
		}
	}

	if (rsslCopyMsgKey(&_rsslMsgKey, &requestMsg.msgBase.msgKey) != RSSL_RET_SUCCESS)
	{
		return false;
	}

	if (requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
	{
		_name.set(requestMsg.msgBase.msgKey.name.data, requestMsg.msgBase.msgKey.name.length);
	}

	if (requestMsg.flags & RSSL_RQMF_STREAMING)
	{
		_flags |= ItemInfo::StreamingFlag;
	}

	if (requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
	{
		_flags |= ItemInfo::PrivateStreamFlag;
	}

	return true;
}

void ItemInfo::setStreamId(Int32 streamId)
{
	_streamId = streamId;
}

void ItemInfo::setServiceId(UInt16 serviceId)
{
	_rsslMsgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	_rsslMsgKey.serviceId = serviceId;
}

void ItemInfo::setNameType(UInt8 nameType)
{
	_rsslMsgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;

	_rsslMsgKey.nameType = nameType;
}

void ItemInfo::setFlags(UInt32 flags)
{
	_flags = flags;
}

void ItemInfo::setDomainType(UInt8 domainType)
{
	_domainType = domainType;
}

void ItemInfo::setFilter(UInt32 filter)
{
	_rsslMsgKey.flags |= RSSL_MKF_HAS_FILTER;

	_rsslMsgKey.filter = filter;
}

void ItemInfo::setItemGroup(const RsslBuffer& itemGroup)
{
	_flags |= ItemGroupFlag;

	_itemGroup.setFrom(itemGroup.data, itemGroup.length);
}

void ItemInfo::setClientSession(ClientSession* clientSession)
{
	_pClientSession = clientSession;
}

void ItemInfo::setSentRefresh()
{
	_sentRefresh = true;
}

bool ItemInfo::operator==(const ItemInfo& other) const
{
	if (this == &other) return true;

	if (_domainType != other._domainType)
	{
		return false;
	}

	if ((_flags & ItemInfo::PrivateStreamFlag) != (other._flags & ItemInfo::PrivateStreamFlag))
	{
		return false;
	}

	if ( rsslCompareMsgKeys(&_rsslMsgKey, &other._rsslMsgKey) != RSSL_RET_SUCCESS )
	{
		return false;
	}

	return true;
}

size_t ItemInfo::UInt32rHasher::operator()(const UInt32& value) const
{
	return value;
}

bool ItemInfo::UInt32Equal_To::operator()(const UInt32& x, const UInt32& y) const
{
	return x == y ? true : false;
}

void ItemInfo::addPostId(UInt32 postId)
{
	if (_pPostIdHash)
	{
		UInt32* count = _pPostIdHash->find(postId);
		if (count)
		{
			++(*count);
		}
		else
		{
			_pPostIdHash->insert(postId, 0);
		}
	}
}

bool ItemInfo::removePostId(UInt32 postId)
{
	if (_pPostIdHash)
	{
		UInt32* count = _pPostIdHash->find(postId);
		if (count)
		{
			if (*count > 0)
			{
				--(*count);
			}
			else
			{
				_pPostIdHash->erase(postId);
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}
