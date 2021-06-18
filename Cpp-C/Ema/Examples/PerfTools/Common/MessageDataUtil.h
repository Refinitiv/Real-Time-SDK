///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2021 Refinitiv.   All rights reserved.            --
///*|-----------------------------------------------------------------------------

#pragma once

#ifndef _MESSAGEDATA_UTIL_H_
#define _MESSAGEDATA_UTIL_H_

#include "Ema.h"
#include "xmlPerfMsgDataParser.h"
#include "GetTime.h"
struct TimeTrack;

/* Helps to fill up Refresh, Update, Generic messages by template xml file (MsgData.xml). */

class MessageDataUtil
{
public:
	static MessageDataUtil* getInstance();

	void setMessageData(MessageData* data) { pMessageData = data; }

	// Market Price.
	// Refresh message. Fills up the fieldList by template pMessageData.
	void fillMarketPriceFieldListRefreshMsg(refinitiv::ema::access::FieldList& fieldList);
	// Update message. Fills up the fieldList by template pMessageData.
	void fillMarketPriceFieldListUpdateMsg(refinitiv::ema::access::FieldList& fieldList, PerfTimeValue latencyStartTime = 0);
	// Generic message. Fills up the fieldList by template pMessageData.
	void fillMarketPriceFieldListGenericMsg(refinitiv::ema::access::FieldList& fieldList, PerfTimeValue latencyStartTime = 0);

	// Market By Order
	// Refresh message. Fills up the map of orders by template pMessageData.
	void fillMarketByOrderMapRefreshMsg(refinitiv::ema::access::Map& mapOrders);
	// Update message. Fills up the map of orders by template pMessageData.
	void fillMarketByOrderMapUpdateMsg(refinitiv::ema::access::Map& mapOrders, PerfTimeValue latencyStartTime = 0);
	// Generic message. Fills up the map of orders by template pMessageData.
	void fillMarketByOrderMapGenericMsg(refinitiv::ema::access::Map& mapOrders, PerfTimeValue latencyStartTime = 0);

	// Decode Update/Generic message.
	bool decodeUpdate(const refinitiv::ema::access::FieldList& fldList, refinitiv::ema::access::UInt16 msgtype,
		TimeTrack& timeTrack, refinitiv::ema::access::EmaString& errText);

private:
	MessageDataUtil() : updateMsgMPIdx(0), updateMsgMBOIdx(0), genericMsgMPIdx(0), genericMsgMBOIdx(0), pMessageData(NULL) {}
	~MessageDataUtil() {}
	MessageDataUtil(MessageDataUtil const&);
	MessageDataUtil& operator=(MessageDataUtil const&);

	static MessageDataUtil* instance;
	MessageData* pMessageData;

	void fillFieldList(refinitiv::ema::access::FieldList& fieldList, refinitiv::ema::access::Int32 fieldEntriesCount, const MarketField* fieldEntries);

	void fillMarketPriceFieldList(refinitiv::ema::access::FieldList& fieldList, MarketPriceMsg& mpMsg);
	void fillMarketByOrderMap(refinitiv::ema::access::Map& mapOrders, MarketByOrderMsg& mboMsg, refinitiv::ema::access::UInt16 msgtype, PerfTimeValue latencyStartTime = 0);

	void addRefereshLatency(refinitiv::ema::access::FieldList& fieldList);
	void addUpdateLatency(refinitiv::ema::access::FieldList& fieldList, PerfTimeValue latencyStartTime);
	void addGenericLatency(refinitiv::ema::access::FieldList& fieldList, PerfTimeValue latencyStartTime);

	refinitiv::ema::access::Int32 updateMsgMPIdx;	// Index of current template for Update message (Market Price) payload to send.
	refinitiv::ema::access::Int32 updateMsgMBOIdx;	// Index of current template for Update message (Market By Order) payload to send.

	refinitiv::ema::access::Int32 genericMsgMPIdx;	// Index of current template for Generic message (Market Price) payload to send.
	refinitiv::ema::access::Int32 genericMsgMBOIdx;	// Index of current template for Generic message (Market By Order) payload to send.

};  // class MessageDataUtil


struct TimeTrack {
	refinitiv::ema::access::UInt64 updTime;			// time tracks for UpdateMsg
	refinitiv::ema::access::UInt64 postTime;		// time tracks for PostMsg
	refinitiv::ema::access::UInt64 genTime;			// time tracks for GenericMsg
};

#endif // !_MESSAGEDATA_UTIL_H_

