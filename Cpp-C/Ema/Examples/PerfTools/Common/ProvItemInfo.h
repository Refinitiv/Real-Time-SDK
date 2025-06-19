/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/* ProvItemInfo.h */
/* Provides data structures to work with list of items that published by Provider */

#pragma once

#ifndef _PROV_ITEM_INFO_
#define _PROV_ITEM_INFO_

#include "Ema.h"
#include "Mutex.h"

#include "Access/Impl/EmaList.h"


class ProvItemInfo : public refinitiv::ema::access::ListLinks< ProvItemInfo >
{
public:
	ProvItemInfo() : handle(0), clientHandle(0), domain(refinitiv::ema::rdm::MMT_MARKET_PRICE), flags(0), name(""), isPost(true), isGeneric(true) {}

	ProvItemInfo(
		refinitiv::ema::access::UInt64 _handle,
		refinitiv::ema::access::UInt64 _clientHandle,
		refinitiv::ema::access::UInt16 _domain,
		refinitiv::ema::access::UInt8 _flags,
		refinitiv::ema::access::EmaString const& _name)
		: handle(_handle), clientHandle(_clientHandle),
		domain(_domain), flags(_flags), name(_name), isPost(true), isGeneric(true) {}

	~ProvItemInfo() {}

	ProvItemInfo(ProvItemInfo const& itemInfo) : handle(itemInfo.handle), clientHandle(itemInfo.clientHandle),
		domain(itemInfo.domain), flags(itemInfo.flags), name(itemInfo.name), isPost(itemInfo.isPost), isGeneric(itemInfo.isGeneric) {}

	ProvItemInfo& operator=(ProvItemInfo const& itemInfo) {
		handle = itemInfo.handle;
		clientHandle = itemInfo.clientHandle;
		domain = itemInfo.domain;
		flags = itemInfo.flags;
		name = itemInfo.name;
		isPost = itemInfo.isPost;
		isGeneric = itemInfo.isGeneric;

		return *this;
	}

	bool operator==(ProvItemInfo const& itemInfo) const {
		if (handle == itemInfo.handle
			&& clientHandle == itemInfo.clientHandle
			&& domain == itemInfo.domain
			&& flags == itemInfo.flags
			&& name == itemInfo.name
			&& isPost == itemInfo.isPost
			&& isGeneric == itemInfo.isGeneric)
			return true;
		else
			return false;
	}


	void setHandle(refinitiv::ema::access::UInt64 _handle) {
		handle = _handle;
	}
	// get the stream handle - for sending via Provider
	refinitiv::ema::access::UInt64 getHandle() {
		return handle;
	}

	void setClientHandle(refinitiv::ema::access::UInt64 _clientHandle) {
		clientHandle = _clientHandle;
	}
	// get the handle of the client session (id of the customer) - for identifying the client
	refinitiv::ema::access::UInt64 getClientHandle() {
		return clientHandle;
	}

	void setDomain(refinitiv::ema::access::UInt16 _domain) {
		domain = _domain;
	}
	refinitiv::ema::access::UInt16 getDomain() {
		return domain;
	}

	void setFlags(refinitiv::ema::access::UInt8 _flags) {
		flags = _flags;
	}
	refinitiv::ema::access::UInt16 getFlags() {
		return flags;
	}

	void setName(refinitiv::ema::access::EmaString& nm) {
		name = nm;
	}
	refinitiv::ema::access::EmaString const& getName() {
		return name;
	}

	void setIsPost(bool _isPost) {
		isPost = _isPost;
	}
	bool getIsPost() {
		return isPost;
	}

	void setIsGeneric(bool _isGeneric) {
		isGeneric = _isGeneric;
	}
	bool getIsGeneric() {
		return isGeneric;
	}


private:
	refinitiv::ema::access::UInt64		handle;			// stream handle - for sending via Provider
	refinitiv::ema::access::UInt64		clientHandle;	// handle of the client session (id of the customer) - for identifying the client
	refinitiv::ema::access::UInt16		domain;
	refinitiv::ema::access::UInt8		flags;
	refinitiv::ema::access::EmaString	name;

	bool isPost;
	bool isGeneric;
};  // class ProvItemInfo


/** Provides guard-access to store new items
*   and no-guard-access to read items
*/
class RefreshItems {
public:
	RefreshItems();
	~RefreshItems();

	/** Prepares and returns a list for reading items (ProvItemInfo) */
	refinitiv::ema::access::EmaList< ProvItemInfo* >& prepareReadList();

	/** Adds new item-info element to the current list for writing operations under guard */
	void add(ProvItemInfo*);

private:
	perftool::common::Mutex				mutex;			// mutex to guard access to curIndexWrite and the current list for writing operations
	refinitiv::ema::access::UInt8		curIndexWrite;	// current index of the list for writing operations (0 focuses to itemsA, 1 to itemsB)
	refinitiv::ema::access::EmaList< ProvItemInfo* >	itemsA;
	refinitiv::ema::access::EmaList< ProvItemInfo* >	itemsB;
};  // class RefreshItems


typedef enum {
	ITEM_IS_STREAMING_REQ = 0x04,	/* Provider should send updates */
	ITEM_IS_SOLICITED = 0x10,		/* Item was requested(not published) */
	ITEM_IS_POST = 0x20,			/* Consumer should send posts */
	ITEM_IS_GEN_MSG = 0x40,			/* Consumer should send generic messages */
	ITEM_IS_PRIVATE = 0x80			/* Item should be requested on private stream */
} ItemFlags;


template <class T>
class RotateListUtil {
public:
	RotateListUtil(refinitiv::ema::access::EmaList< T >& items, T startItem)
		: itemList(items), item(startItem)
	{
	}

	const T& getNext() {
		if (item != NULL)
		{
			item = item->next();
			if (item == NULL)
				item = itemList.front();
		}
		else
		{
			item = itemList.front();
		}

		return item;
	}

private:
	refinitiv::ema::access::EmaList< T >& itemList;
	T item;
};  // class RotateListUtil


template <class T>
class RotateVectorUtil {
public:
	RotateVectorUtil(refinitiv::ema::access::EmaVector< T >& vec, refinitiv::ema::access::UInt32 startInd)
		: startIndex(startInd), currentIndex(startInd), data(vec)
	{
		if (startIndex >= data.size())
		{
			if (!data.empty())
				currentIndex = startIndex % data.size();
			else
				currentIndex = 0;
		}
	}

	refinitiv::ema::access::UInt32 getCurrentIndex() { return currentIndex; }

	const T& getNext() {
		refinitiv::ema::access::UInt32 index = currentIndex;
		if (++currentIndex >= data.size())
		{
			currentIndex = 0;
		}
		return data[index];
	}

	const T& operator[](refinitiv::ema::access::UInt32 index) const {
		return data[index];
	}

private:
	refinitiv::ema::access::UInt32 startIndex;
	refinitiv::ema::access::UInt32 currentIndex;

	refinitiv::ema::access::EmaVector< T >& data;
};  // class RotateVectorUtil

#endif  // _PROV_ITEM_INFO_
