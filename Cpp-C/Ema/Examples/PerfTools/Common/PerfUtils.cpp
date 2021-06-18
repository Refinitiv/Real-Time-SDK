///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "PerfUtils.h"
#include "../Common/XmlItemParser.h"

#include "Ema.h"

#include <iostream>

using namespace refinitiv::ema::access;
using namespace std;

bool PerfUtils::initializeItems(NIProvPerfConfig const& niProvPerfConfig, EmaList< ProvItemInfo* >& items, UInt32 itemListCount, UInt32 itemListStartIndex)
{
	if (!items.empty())
	{
		cout << "Error: list of the items should be empty." << endl;
		return false;
	}

	XmlItemParser xmlItemParser;
	
	XmlItemList* pXmlItemList = xmlItemParser.create(niProvPerfConfig.itemFilename.c_str(), niProvPerfConfig.itemCount);
	if ( pXmlItemList == NULL )
	{
		cout << "Failed to load item list from file '" << niProvPerfConfig.itemFilename << "'." << endl;
		return false;
	}

	UInt32 index = 0;
	UInt32 xmlItemListIndex = 0;

	// Copy item information from the XML list: prepare list of items that are published
	for (index = 0, xmlItemListIndex = itemListStartIndex; index < itemListCount; ++index, ++xmlItemListIndex)
	{
		ProvItemInfo* itemInfo = new ProvItemInfo;

		itemInfo->setDomain( (*pXmlItemList)[xmlItemListIndex]->domain );
		itemInfo->setName( (*pXmlItemList)[xmlItemListIndex]->name );
		itemInfo->setFlags( ITEM_IS_STREAMING_REQ );
		itemInfo->setIsPost( (*pXmlItemList)[xmlItemListIndex]->post );
		itemInfo->setIsGeneric( (*pXmlItemList)[xmlItemListIndex]->genMsg );

		itemInfo->setHandle( index );

		items.push_back(itemInfo);
	}

	cout << "Loaded: " << items.size() << " items." << endl;

	return true;
}
