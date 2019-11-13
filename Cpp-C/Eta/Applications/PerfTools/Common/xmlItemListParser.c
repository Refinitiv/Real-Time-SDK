/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "xmlItemListParser.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
	static void _saxStartElement(void *pData, const xmlChar *xmlName, const xmlChar **attrList);
	static void _saxEndElement(void *pData, const xmlChar *name);
}
#endif

/* Internal states for the XML parser. */
typedef enum
{
	XML_PARSE_ST_INIT,
	XML_PARSE_ST_ITEM_LIST,
	XML_PARSE_ST_ITEM,
	XML_PARSE_ST_COMPLETE,
	XML_PARSE_ST_ERROR
} XmlParseState;


/* State machine for parsing the item list. */
typedef struct
{
	XmlItemInfoList *pItemInfoList;		/* The itemInfoList being loaded */
	int				totalItemCount;		/* Total number of items to load */
	XmlParseState	saxParsingState;	/* Current parser state */
} XmlItemListParser;

/* Parse & store item attributes. */
static int _saxParseXmlItemInfo(const xmlChar **xmlAttrList, XmlItemInfo *pItemInfo)
{
	int foundDomainType = 0, foundName = 0, foundIsPost = 0, foundIsGenMsg = 0,
		foundIsLatency = 0, foundIsSnapshot = 0;

	char **attrList = (char**)xmlAttrList;


	if (!attrList)
	{
		printf("Item missing attributes.\n");
		return -1;
	}

	clearXmlItemInfo(pItemInfo);

	for (; *attrList; attrList += 2)
	{
		if (!strcmp(attrList[0], "domain"))
		{
			if (foundDomainType)
			{
				printf("Item has duplicate domain attribute.\n");
				return -1;
			}
			foundDomainType = 1;
				
			if (!strcmp(attrList[1], "MarketPrice"))
				pItemInfo->domainType = XML_ITEM_DMT_MARKET_PRICE;
			else if (!strcmp(attrList[1], "MarketByOrder"))
				pItemInfo->domainType = XML_ITEM_DMT_MARKET_BY_ORDER;
			else
			{
				printf("Unknown item domainType '%s' .\n", attrList[1]);
				return -1;
			}
		}

		else if (!strcmp(attrList[0], "name"))
		{
			unsigned int nameLen;

			if (foundName)
			{
				printf("Item has duplicate name attribute.\n");
				return -1;
			}
			foundName = 1;

			nameLen = (unsigned int)strlen(attrList[1]);

			if (nameLen > sizeof(pItemInfo->name) - 1)
			{
				printf("Item name is too long: '%s' .\n", attrList[1]);
				return -1;
			}

			strncpy(pItemInfo->name, attrList[1], 256);
			pItemInfo->nameLength = nameLen;
		}
		else if (!strcmp(attrList[0], "post"))
		{
			if (foundIsPost)
			{
				printf("Item has duplicate post attribute.\n");
				return -1;
			}
			foundIsPost = 1;

			if (!strcmp(attrList[1], "true"))
				pItemInfo->isPost = 1;
			else if (!strcmp(attrList[1], "false"))
				pItemInfo->isPost = 0;
			else
			{
				printf("Unknown post attribute setting '%s'\n",
						attrList[1]);
				return -1;
			}
		}
		else if (!strcmp(attrList[0], "generic"))
		{
			if (foundIsGenMsg)
			{
				printf("Item has duplicate generic attribute.\n");
				return -1;
			}
			foundIsGenMsg = 1;

			if (!strcmp(attrList[1], "true"))
				pItemInfo->isGenMsg = 1;
			else if (!strcmp(attrList[1], "false"))
				pItemInfo->isGenMsg = 0;
			else
			{
				printf("Unknown generic attribute setting '%s'\n",
						attrList[1]);
				return -1;
			}
		}
		else if (!strcmp(attrList[0], "snapshot"))
		{
			if (foundIsSnapshot)
			{
				printf("Item has duplicate snapshot attribute.\n");
				return -1;
			}
			foundIsSnapshot = 1;

			if (!strcmp(attrList[1], "true"))
				pItemInfo->isSnapshot = 1;
			else if (!strcmp(attrList[1], "false"))
				pItemInfo->isSnapshot = 0;
			else
			{
				printf("Unknown snapshot attribute setting '%s'\n",
						attrList[1]);
				return -1;
			}
		}

	}

	if (!(foundDomainType && foundName))
	{
		printf("Item missing required attributes. 'domain' and 'name' are required.\n");
		return -1;
	}

	if (pItemInfo->isPost && pItemInfo->isSnapshot)
	{
		printf("Item '%s' is set for both posting and snapshot.\n", pItemInfo->name);
		return -1;
	}

	if (pItemInfo->isGenMsg && pItemInfo->isSnapshot)
	{
		printf("Item '%s' is set for both generic and snapshot.\n", pItemInfo->name);
		return -1;
	}

	return 0;
}

static void _saxStartElement(void *pData, const xmlChar *xmlName, const xmlChar **attrList)
{
	XmlItemListParser *pParser = (XmlItemListParser*)pData;
	XmlItemInfoList *pItemInfoList = pParser->pItemInfoList;

	char *name = (char*)xmlName;

	switch(pParser->saxParsingState)
	{
		case XML_PARSE_ST_INIT:
			/* Look for the root element. */
			if (0 == strcmp(name, "itemList"))
				pParser->saxParsingState = XML_PARSE_ST_ITEM_LIST;
			else
			{
				printf("Unknown element '%s' while parsing root.\n", name);
				pParser->saxParsingState = XML_PARSE_ST_ERROR;
			}
			return;

		case XML_PARSE_ST_ITEM_LIST:
			/* Parsing item list. This should be a list of elements named "item". */
			if (0 == strcmp(name, "item"))
			{
				/* Parse the item and create an XmlItemInfo, unless
				 * we've already parsed as many items as the caller requested. */
				if (pItemInfoList->itemInfoCount < pParser->totalItemCount)
				{
					if (_saxParseXmlItemInfo(attrList, &pItemInfoList->itemInfoList[pItemInfoList->itemInfoCount]) != 0)
					{
						pParser->saxParsingState = XML_PARSE_ST_ERROR;
						return;
					}

					if (pItemInfoList->itemInfoList[pItemInfoList->itemInfoCount].isPost)
						++pItemInfoList->postItemCount;

					if (pItemInfoList->itemInfoList[pItemInfoList->itemInfoCount].isGenMsg)
						++pItemInfoList->genMsgItemCount;

					++pItemInfoList->itemInfoCount;
				}

				pParser->saxParsingState = XML_PARSE_ST_ITEM;
			}
			else
			{
				printf("Unknown element '%s' while parsing item list.\n", name);
				pParser->saxParsingState = XML_PARSE_ST_ERROR;
			}
			return;
		case XML_PARSE_ST_ERROR:
			return;
		default:
			printf("Unexpected parsing state %d while processing start of element '%s'\n", pParser->saxParsingState, name); 
			pParser->saxParsingState = XML_PARSE_ST_ERROR;
			return;
	}
	
}

static void _saxEndElement(void *pData, const xmlChar *name)
{
	XmlItemListParser *pParser = (XmlItemListParser*)pData;
	XmlItemInfoList *pItemInfoList = pParser->pItemInfoList;

	switch(pParser->saxParsingState)
	{
		case XML_PARSE_ST_ITEM_LIST: pParser->saxParsingState = XML_PARSE_ST_COMPLETE; return;
		case XML_PARSE_ST_ITEM: pParser->saxParsingState = XML_PARSE_ST_ITEM_LIST; return;
		case XML_PARSE_ST_ERROR: return;
		default: 
			printf("Unexpected parsing state %d while processing end of element '%s'\n", pParser->saxParsingState, name); 
			pParser->saxParsingState = XML_PARSE_ST_ERROR;
			return;
	}
}

XmlItemInfoList *createXmlItemList(const char *filename, unsigned int count)
{
	XmlItemInfoList *pItemInfoList = NULL;
	xmlSAXHandler saxHandler;
	XmlItemListParser xmlItemListParser;

	if (count < 1 )
	{
		printf("Count was not at least 1.\n");
		return NULL;
	}


	pItemInfoList = (XmlItemInfoList*)malloc(sizeof(XmlItemInfoList));
	pItemInfoList->itemInfoList = (XmlItemInfo*)malloc((count) * sizeof(XmlItemInfo));

	pItemInfoList->itemInfoCount = 0;
	pItemInfoList->postItemCount = 0;
	pItemInfoList->genMsgItemCount = 0;

	memset(&saxHandler, 0, sizeof(saxHandler));
	memset(&xmlItemListParser, 0, sizeof(xmlItemListParser));

	xmlItemListParser.pItemInfoList = pItemInfoList;
	xmlItemListParser.totalItemCount = count;
	xmlItemListParser.saxParsingState = XML_PARSE_ST_INIT;

	saxHandler.startElement = _saxStartElement;
	saxHandler.endElement = _saxEndElement;

	if (xmlSAXUserParseFile(&saxHandler, &xmlItemListParser, filename) < 0)
	{
		printf("xmlSAXUserParseFile() failed with parsing state: %d.\n", xmlItemListParser.saxParsingState);
		goto createXmlItemList_failure;
	}
	else if	(xmlItemListParser.saxParsingState != XML_PARSE_ST_COMPLETE)
	{
		printf("xmlSAXUserParseFile() returned with unexpected parsing state: %d\n", xmlItemListParser.saxParsingState);
		goto createXmlItemList_failure;
	}

	return pItemInfoList;

	createXmlItemList_failure:
	free(pItemInfoList->itemInfoList);
	free(pItemInfoList);
	return NULL;
}

void destroyXmlItemList(XmlItemInfoList *pInfoList)
{
	free(pInfoList->itemInfoList);
	free(pInfoList);
}
