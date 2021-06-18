///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "PerfMessageData.h"

using namespace refinitiv::ema::access;

PerfMessageData::PerfMessageData(const EmaString& msgFilename, bool preEncItems)
{
	pMessageData = MessageData::getInstance(msgFilename);

	MarketPriceMsgList& mpMsgList = pMessageData->getMarketPriceMsgList();
	MarketByOrderMsgList& mboMsgList = pMessageData->getMarketByOrderMsgList();

	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();
	msgDataUtil->setMessageData(pMessageData);

	// Pre encode data
	if (preEncItems)
	{
		/////////////////////////////////////////////////
		// Market Price.
		/////////////////////////////////////////////////
		// fills up FieldList by template for RefreshMsg
		msgDataUtil->fillMarketPriceFieldListRefreshMsg(mpFieldListRefresh);

		// fills up FieldList by template for UpdateMsg
		for (Int32 i = 0; i < mpMsgList.updateMsgCount; ++i)
		{
			FieldList* pFieldList = new FieldList();
			UpdateMsg* updateMsg = new UpdateMsg();

			msgDataUtil->fillMarketPriceFieldListUpdateMsg(*pFieldList);
			mpFieldListUpdates.push_back(pFieldList);

			updateMsg->payload(*pFieldList);
			mpUpdateMessagesPreEncoded.push_back(updateMsg);
		}

		// fills up FieldList by template for GenericMsg
		for (Int32 i = 0; i < mpMsgList.genMsgCount; ++i)
		{
			FieldList* pFieldList = new FieldList();
			GenericMsg* genericMsg = new GenericMsg();

			msgDataUtil->fillMarketPriceFieldListGenericMsg(*pFieldList);
			mpFieldListGenerics.push_back(pFieldList);

			genericMsg->payload(*pFieldList);
			mpGenericMessagesPreEncoded.push_back(genericMsg);
		}

		/////////////////////////////////////////////////
		// Market by Order.
		/////////////////////////////////////////////////
		// Fills up map by template for RefreshMsg
		msgDataUtil->fillMarketByOrderMapRefreshMsg(mboMapOrdersRefresh);

		// Fills up list of map by template for UpdateMsg
		for (Int32 i = 0; i < mboMsgList.updateMsgCount; ++i)
		{
			Map* pMap = new Map();
			UpdateMsg* updateMsg = new UpdateMsg();

			msgDataUtil->fillMarketByOrderMapUpdateMsg(*pMap);
			mboMapOrdersUpdates.push_back(pMap);

			updateMsg->payload(*pMap);
			mboUpdateMessagesPreEncoded.push_back(updateMsg);
		}

		// Fills up list of map by template for GenericMsg
		for (Int32 i = 0; i < mboMsgList.genMsgCount; ++i)
		{
			Map* pMap = new Map();
			GenericMsg* genericMsg = new GenericMsg();

			msgDataUtil->fillMarketByOrderMapGenericMsg(*pMap);
			mboMapOrdersGenerics.push_back(pMap);

			genericMsg->payload(*pMap);
			mboGenericMessagesPreEncoded.push_back(genericMsg);
		}
	}
}

PerfMessageData::~PerfMessageData()
{
	UInt32 i;

	// Cleans all the pre-encoded data
	for (i = 0; i < mpFieldListUpdates.size(); ++i)
	{
		FieldList* pFieldList = mpFieldListUpdates[i];
		if (pFieldList != NULL)
			delete pFieldList;
		mpFieldListUpdates[i] = NULL;
	}

	for (i = 0; i < mpUpdateMessagesPreEncoded.size(); ++i)
	{
		UpdateMsg* updMsg = mpUpdateMessagesPreEncoded[i];
		if (updMsg != NULL)
			delete updMsg;
		mpUpdateMessagesPreEncoded[i] = NULL;
	}

	for (i = 0; i < mboMapOrdersUpdates.size(); ++i)
	{
		Map* pMap = mboMapOrdersUpdates[i];
		if (pMap != NULL)
			delete pMap;
		mboMapOrdersUpdates[i] = NULL;
	}

	for (i = 0; i < mboUpdateMessagesPreEncoded.size(); ++i)
	{
		UpdateMsg* updMsg = mboUpdateMessagesPreEncoded[i];
		if (updMsg != NULL)
			delete updMsg;
		mboUpdateMessagesPreEncoded[i] = NULL;
	}

	for (i = 0; i < mpFieldListGenerics.size(); ++i)
	{
		FieldList* pFieldList = mpFieldListGenerics[i];
		if (pFieldList != NULL)
			delete pFieldList;
		mpFieldListGenerics[i] = NULL;
	}

	for (i = 0; i < mpGenericMessagesPreEncoded.size(); ++i)
	{
		GenericMsg* genericMsg = mpGenericMessagesPreEncoded[i];
		if (genericMsg != NULL)
			delete genericMsg;
		mpGenericMessagesPreEncoded[i] = NULL;
	}

	for (i = 0; i < mboMapOrdersGenerics.size(); ++i)
	{
		Map* pMap = mboMapOrdersGenerics[i];
		if (pMap != NULL)
			delete pMap;
		mboMapOrdersGenerics[i] = NULL;
	}

	for (i = 0; i < mboGenericMessagesPreEncoded.size(); ++i)
	{
		GenericMsg* genericMsg = mboGenericMessagesPreEncoded[i];
		if (genericMsg != NULL)
			delete genericMsg;
		mboGenericMessagesPreEncoded[i] = NULL;
	}
}
