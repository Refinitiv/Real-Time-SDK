///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "XmlItemParser.h"

#include "AppUtil.h"

#include <string.h>
#include <iostream>

using namespace std;
using namespace perftool::common;
using namespace refinitiv::ema::access;

XmlItemParser::XmlItemParser() :
 _count(0),
 _pXmlItemList(0),
 _parsingState(INIT_STATE)
{
}

static int _saxParseXmlItemInfo(const xmlChar** attrList, XmlItem* pXmlItem)
{
	int foundDomainType = 0, foundName = 0,  foundPost = 0, foundIsSnapshot = 0, foundIsGenMsg = 0;
	pXmlItem->post = false;
	pXmlItem->snapshot = false;
	if(!attrList)
	{
		EmaString text("Error: Function _saxParseXmlItemInfo-> Item has missing attributes.");
		AppUtil::logError(text);
		return -1;
	}
	for(; *attrList; attrList += 2)
	{
		if(!strcmp((char*)attrList[0], "domain"))
		{
			if(foundDomainType)
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Item has duplicate domain attribute.");
				AppUtil::logError(text);
				return -1;
			}
			foundDomainType = 1;
			if(!strcmp((char*)attrList[1], "MarketPrice"))
				pXmlItem->domain = XmlItem::MARKET_PRICE_DOMAIN;
			else if(!strcmp((char*)attrList[1], "MarketByOrder"))
				pXmlItem->domain = XmlItem::MARKET_BY_ORDER_DOMAIN;
			else
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Unknown item domainType.");
				AppUtil::logError(text);
				return -1;
			}
		}
		else if(!strcmp((char*)attrList[0], "name"))
		{
			if(foundName)
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Item has duplicate name attribute.");
				AppUtil::logError(text);
				return -1;
			}
			foundName = 1;
			pXmlItem->name.set((char*)attrList[1]);
		}
		else if(!strcmp((char*)attrList[0], "post"))
		{
			if(foundPost)
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Item has duplicate post attribute.");
				AppUtil::logError(text);
				return -1;
			}
			foundPost = 1;
			if(!strcmp((char*)attrList[1], "true"))
				pXmlItem->post = true;
			else if(!strcmp((char*)attrList[1], "false"))
				pXmlItem->post = false;
		}
		else if(!strcmp((char*)attrList[0], "generic"))
		{
			if(foundIsGenMsg)
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Item has duplicate generic attribute.");
				AppUtil::logError(text);
				return -1;
			}
			foundIsGenMsg = 1;
			if(!strcmp((char*)attrList[1], "true"))
				pXmlItem->genMsg = true;
			else if(!strcmp((char*)attrList[1], "false"))
				pXmlItem->genMsg = false;
		}
		else if(!strcmp((char*)attrList[0], "snapshot"))
		{
			if(foundIsSnapshot)
			{
				EmaString text("Error: Function _saxParseXmlItemInfo-> Item has duplicate snapshot attribute.");
				AppUtil::logError(text);
				return -1;
			}
			foundIsSnapshot = 1;
			if(!strcmp((char*)attrList[1], "true"))
				pXmlItem->snapshot = true;
			else if(!strcmp((char*)attrList[1], "false"))
				pXmlItem->snapshot = false;
		}
	}
	return (foundDomainType && foundName) ? 0: -1;
}

static void _saxStartElement(void* pData, const xmlChar* name, const xmlChar** attrList)
{
	XmlItemParser* pParser = (XmlItemParser*)pData;
	XmlItemList* pXmlItemList = pParser->_pXmlItemList;

	switch(pParser->_parsingState)
	{
	case XmlItemParser::INIT_STATE:
			// Parsing session element -- look for item list element.
			if (0 == strcmp((char*)name, "itemList"))
			{
				pParser->_parsingState = XmlItemParser::ITEM_LIST_STATE;
			}
			else
			{
				EmaString text("Error: Function _saxStartElement-> Unknown element '");
				text += (char *) name;
				text += "' while parsing root node.";
				AppUtil::logError(text);
				pParser->_parsingState = XmlItemParser::ERROR_STATE;
			}
			return;
	case XmlItemParser::ITEM_LIST_STATE:
			// Parsing item list. This should be a list of elements named "item".
			if (0 == strcmp((char*)name, "item"))
			{
				// Parse the item and create an XmlItemInfo, unless we've already parsed as many items as the caller requested.
				if(pXmlItemList->size() < pParser->_count)
				{
					XmlItem *pNewXmlItem = new XmlItem;
					pXmlItemList->push_back(pNewXmlItem);
					if(_saxParseXmlItemInfo(attrList, (*pXmlItemList)[pXmlItemList->size() - 1]))
					{
						pParser->_parsingState = XmlItemParser::ERROR_STATE;
						return;
					}
				}
				pParser->_parsingState = XmlItemParser::ITEM_STATE;
			}
			else
			{
				EmaString text("Error: Function _saxStartElement-> Unknown element '");
				text += (char *) name;
				text += "' while parsing item list.";
				AppUtil::logError(text);
				pParser->_parsingState = XmlItemParser::ERROR_STATE;
			}
			return;
		case XmlItemParser::ERROR_STATE:
			return;
		default:
			{
			EmaString text("Error: Function _saxStartElement-> Unexpected parsing state ");
			text.append( (UInt32) pParser->_parsingState);
			text += " while processing start of element '";
			text += (char *) name;
			text += "'.";
			AppUtil::logError(text);
			pParser->_parsingState = XmlItemParser::ERROR_STATE;
			return;
			}
	}
}

static void _saxEndElement(void* pData, const xmlChar* name)
{
	XmlItemParser* pParser = (XmlItemParser*)pData;
	switch(pParser->_parsingState)
	{
		case XmlItemParser::ITEM_LIST_STATE: pParser->_parsingState = XmlItemParser::COMPLETE_STATE; return;
		case XmlItemParser::ITEM_STATE:      pParser->_parsingState = XmlItemParser::ITEM_LIST_STATE; return;
		case XmlItemParser::ERROR_STATE:     return;
		default: 
			{
			EmaString text("Error: Function _saxStartElement-> Unexpected parsing state ");
			text.append( (UInt32) pParser->_parsingState);
			text += " while processing end of element '";
			text += (char *) name;
			text += "'.";
			AppUtil::logError(text);
			pParser->_parsingState = XmlItemParser::ERROR_STATE;
			return;
			}
	}
}

extern "C" {
	void _saxStartElementExternC(void* pData, const xmlChar* name, const xmlChar** attrList)
	{
		_saxStartElement(pData, name, attrList);
	}
	void _saxEndElementExternC(void* pData, const xmlChar* name)
	{
		_saxEndElement(pData, name);
	}
}
XmlItemParser::~XmlItemParser()
{
	if(_pXmlItemList)
	{
		for( refinitiv::ema::access::UInt64 i = 0; i <_pXmlItemList->size(); i++)
		{
			if( (*_pXmlItemList)[i] )
			{
				delete (*_pXmlItemList)[i];
				(*_pXmlItemList)[i] = NULL;
			}
		}
	}
	AppUtil::log("XmlItemParser Destroyed\n");
}

XmlItemList* XmlItemParser::create(const char* filename, unsigned int count)
{
	xmlParserCtxtPtr pParserCtxt = NULL;
	xmlSAXHandler saxHandler;

	if(count < 1)
	{
		EmaString text("Error: Function XmlItemParser::create-> Count was not at least 1.");
		AppUtil::logError(text);
		return NULL;
	}
	_count = count;
	_pXmlItemList = new XmlItemList(_count);

	memset(&saxHandler, 0, sizeof(saxHandler));
	_parsingState = INIT_STATE;

	saxHandler.startElement = _saxStartElementExternC;
	saxHandler.endElement = _saxEndElementExternC;

	pParserCtxt = xmlNewSAXParserCtxt(&saxHandler, (void*)this);
	if (pParserCtxt == NULL)
	{
		EmaString text("Error: Function XmlItemParser::create-> xmlNewSAXParserCtxt() failed to allocate parser context.");
		AppUtil::logError(text);
		if( _pXmlItemList )
			delete _pXmlItemList;
		_pXmlItemList = 0;

		return NULL;
	}

	(void)xmlCtxtReadFile(pParserCtxt, filename, NULL, XML_PARSE_COMPACT | XML_PARSE_BIG_LINES);
	if (pParserCtxt->myDoc != NULL)
	{
		EmaString text("Error: Function XmlItemParser::create-> xmlCtxtReadFile() failed with parsing state: ");
		text.append((UInt32)_parsingState);
		text += ".";
		AppUtil::logError(text);
		if (_pXmlItemList)
			delete _pXmlItemList;
		_pXmlItemList = 0;
	}
	else if (_parsingState != COMPLETE_STATE)
	{
		EmaString text("Error: Function XmlItemParser::create-> xmlCtxtReadFile() returned with unexpected parsing state: ");
		text.append((UInt32)_parsingState);
		text += ".";
		AppUtil::logError(text);
		if (_pXmlItemList)
			delete _pXmlItemList;
		_pXmlItemList = 0;
	}

	xmlFreeParserCtxt(pParserCtxt);

	return _pXmlItemList;
}
