///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2021 LSEG. All rights reserved.                    --
///*|-----------------------------------------------------------------------------

/* PerfMessageData.h
 * Provides the logic that fill up Refresh/Update/Post/Generic messages by templates.
 * Pre-encoded data for sending messages. */

#pragma once

#ifndef _PERF_MESSAGE_DATA_H_
#define _PERF_MESSAGE_DATA_H_

#include "Ema.h"
#include "MessageDataUtil.h"

class PerfMessageData {
public:
	PerfMessageData(const refinitiv::ema::access::EmaString& msgFilename, bool preEncItems);
	~PerfMessageData();

	refinitiv::ema::access::FieldList& getRefreshFieldList() { return mpFieldListRefresh; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::FieldList*>& getUpdatesFieldList() { return mpFieldListUpdates; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::UpdateMsg*>& getMpUpdatesPreEncoded() { return mpUpdateMessagesPreEncoded; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::FieldList*>& getGenericsFieldList() { return mpFieldListGenerics; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::GenericMsg*>& getMpGenericsPreEncoded() { return mpGenericMessagesPreEncoded; }

	refinitiv::ema::access::Map& getRefreshMboMapOrders() { return mboMapOrdersRefresh; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::Map*>& getUpdatesMboMapOrders() { return mboMapOrdersUpdates; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::UpdateMsg*>& getMboUpdatesPreEncoded() { return mboUpdateMessagesPreEncoded; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::Map*>& getGenericsMboMapOrders() { return mboMapOrdersGenerics; }
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::GenericMsg*>& getMboGenericsPreEncoded() { return mboGenericMessagesPreEncoded; }

private:
	// Helper methods to access to template messages data.
	MessageData* pMessageData;

	// Pre-encoded data.
	// Market Price. Field-lists for refreshes, updates and generics.
	refinitiv::ema::access::FieldList mpFieldListRefresh;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::FieldList*> mpFieldListUpdates;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::UpdateMsg*> mpUpdateMessagesPreEncoded;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::FieldList*> mpFieldListGenerics;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::GenericMsg*> mpGenericMessagesPreEncoded;

	// Market By Orders. Map of orders for refreshes, updates and generics.
	refinitiv::ema::access::Map mboMapOrdersRefresh;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::Map*> mboMapOrdersUpdates;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::UpdateMsg*> mboUpdateMessagesPreEncoded;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::Map*> mboMapOrdersGenerics;
	refinitiv::ema::access::EmaVector<refinitiv::ema::access::GenericMsg*> mboGenericMessagesPreEncoded;

};  // class PerfMessageData

#endif  // _PERF_MESSAGE_DATA_H_
