///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __INCLUDED_XML_ITEM_PARSER_H__
#define __INCLUDED_XML_ITEM_PARSER_H__

// Class for mapping XML data constaining information about items in 'itemFile' to 
// a list of structures, one for each item, containing OMM Domain, item name, and flag 
// to indicate item can be used for posting.

#include "Ema.h"
#include "AppVector.h"

#include "libxml/tree.h"
#include "libxml/parser.h"

struct XmlItem
{
	enum Domain {MARKET_PRICE_DOMAIN = 6, MARKET_BY_ORDER_DOMAIN = 7};
	Domain										domain;
	thomsonreuters::ema::access::EmaString		name;
	bool										post;
	bool										snapshot;
	bool										genMsg;
};

typedef perftool::common::AppVector<XmlItem*> XmlItemList;

class XmlItemParser
{
public:
	XmlItemParser();
	~XmlItemParser();

	XmlItemList* create(const char* filename, thomsonreuters::ema::access::UInt32 count);
	enum ParsingState {INIT_STATE, ITEM_LIST_STATE, ITEM_STATE, COMPLETE_STATE, ERROR_STATE};

	thomsonreuters::ema::access::UInt64				_count;
	XmlItemList*									_pXmlItemList;
	ParsingState									_parsingState;
};
#endif
