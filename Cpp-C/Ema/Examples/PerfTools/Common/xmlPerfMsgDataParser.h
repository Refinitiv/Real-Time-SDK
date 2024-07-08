///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2021 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#pragma once

#ifndef _XML_PERF_MSGDATA_PARSER_H_
#define _XML_PERF_MSGDATA_PARSER_H_

#include "xmlMsgDataParser.h"
#include "Mutex.h"


class MessageData {
public:
	static MessageData* getInstance(const char* msgDataFileName);

	~MessageData();


	bool getXmlMsgDataLoaded() const { return (xmlMsgDataLoaded != 0); }

	bool getXmlMsgDataHasMarketPrice() const { return (xmlMsgDataHasMarketPrice != 0); }

	MarketPriceMsgList& getMarketPriceMsgList() { return xmlMarketPriceMsgs; }

	bool getXmlMsgDataHasMarketByOrder() const { return (xmlMsgDataHasMarketByOrder != 0); }

	MarketByOrderMsgList& getMarketByOrderMsgList() { return xmlMarketByOrderMsgs; }

	RsslDataDictionary& getDictionary() { return dictionary; }

private:

	static MessageData* instance;

	static perftool::common::Mutex mutex;  // creating stage mutex

	//MessageData();
	MessageData(const char* msgDataFileName);
	MessageData(MessageData const&);
	MessageData operator=(MessageData const&);
};

#endif // _XML_PERF_MSGDATA_PARSER_H_
