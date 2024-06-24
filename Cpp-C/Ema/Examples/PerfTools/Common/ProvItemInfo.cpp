///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2021 LSEG. All rights reserved.                    --
///*|-----------------------------------------------------------------------------

#include "ProvItemInfo.h"

using namespace refinitiv::ema::access;

RefreshItems::RefreshItems() : itemsA(), itemsB()
{
	mutex.lock();
	curIndexWrite = 0;
	mutex.unlock();
}

RefreshItems::~RefreshItems()
{
	ProvItemInfo* itemInfo;

	mutex.lock();

	itemInfo = itemsA.front();
	while (itemInfo != NULL)
	{
		delete itemInfo;
		itemInfo = itemInfo->next();
	}
	itemsA.clear();

	itemInfo = itemsB.front();
	while (itemInfo != NULL)
	{
		delete itemInfo;
		itemInfo = itemInfo->next();
	}
	itemsB.clear();

	curIndexWrite = 0;

	mutex.unlock();
}

EmaList< ProvItemInfo* >& RefreshItems::prepareReadList()
{
	mutex.lock();
	UInt8 readIndex = curIndexWrite;

	if (++curIndexWrite > 1)
		curIndexWrite = 0;
	mutex.unlock();

	EmaList< ProvItemInfo* >& currentListForRead = (readIndex == 0 ? itemsA : itemsB);
	return currentListForRead;
}

void RefreshItems::add(ProvItemInfo* itemInfo)
{
	mutex.lock();
	if (curIndexWrite == 0)
		itemsA.push_back(itemInfo);
	else
		itemsB.push_back(itemInfo);
	mutex.unlock();
}
