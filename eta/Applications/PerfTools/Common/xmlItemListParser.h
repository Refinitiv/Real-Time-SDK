/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* xmlItemListParser.h
 * Builds a list of items from an XML file. */

#ifndef _XML_ITEM_LIST_PARSER_H
#define _XML_ITEM_LIST_PARSER_H

#define LIBXML_STATIC

#include "libxml/tree.h"
#include "libxml/parser.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Domains that items in the list may be. These should correspond to their domain model
 * counterparts. */
typedef enum
{
	XML_ITEM_DMT_MARKET_PRICE = 6,		/* Market Price (6) */
	XML_ITEM_DMT_MARKET_BY_ORDER = 7	/* Market By Order (7) */
} XmlItemDomain;


/* Contains information about an item in the item list. */
typedef struct {
	XmlItemDomain		domainType;		/* Domain Type of the item */
	char				name[256];		/* Name of the item */
	unsigned int		nameLength;		/* Length of the item name */
	int					isPost;			/* Whether the item is to be used for consumer posting. */
	int					isGenMsg;		/* Whether the item is to be used for consumer sending generic messages. */
	int					isSnapshot;		/* Whether the item should be requested as a snapshot. */
} XmlItemInfo;

/* Clears an XmlItemInfo structure. */
static void clearXmlItemInfo(XmlItemInfo *pXmlItemInfo)
{
	memset(pXmlItemInfo, 0, sizeof(XmlItemInfo));
}


/* Builds a list of items from an XML file.  
 * This may be used by applications to determine what items to
 * open when staring a session. */
typedef struct {
	int				itemInfoCount;		/* The total number of items in the list */
	int				postItemCount;		/* Number of items in list for posting. */
	int				genMsgItemCount;	/* Number of items in list for sending generic messages. */
	XmlItemInfo		*itemInfoList;		/* The list of items */
} XmlItemInfoList;

/* Creates a list of items from the specified XML file. */
XmlItemInfoList *createXmlItemList(const char *filename, unsigned int count);

/* Cleans up an XmlItemInfoList. */
void destroyXmlItemList(XmlItemInfoList *pInfoList);

#ifdef __cplusplus
};
#endif

#endif
