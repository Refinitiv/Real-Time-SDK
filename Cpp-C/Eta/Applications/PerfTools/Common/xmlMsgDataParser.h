/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

/* xmlMsgDataParser.h
 * Builds message payloads from an XML file for use in providing item data. */

#ifndef _XML_MSG_DATA_PARSER_H
#define _XML_MSG_DATA_PARSER_H

#include "testUtils.h"
#include "rtr/rsslMessagePackage.h"
#include "libxml/tree.h"
#include "libxml/parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ORDER_ID_STRLEN 64

#define ORDER_ID_FID	3426

#define TIM_TRK_1_FID 3902 /* Field TIM_TRK_1 is used to send update latency. */
#define TIM_TRK_2_FID 3903 /* Field TIM_TRK_2 is used to send post latency. */
#define TIM_TRK_3_FID 3904 /* Field TIM_TRK_3 is used to send generic message latency. */

/*** Structures representing data ***/

RTR_C_INLINE void clearRsslPrimitive(RsslPrimitive *pPrimitive)
{
	memset(pPrimitive, 0, sizeof(RsslPrimitive));
}

/* MarketField
   *   Represents one field in a fieldList(used by MarketPrice and MarketByOrder messages)
   *   Stores the FieldID and dataType of the desired data using a FieldEntry,
   *   as well as the value to use */
typedef struct {
	RsslFieldEntry fieldEntry;
	RsslPrimitive primitive;
	RsslBool isBlank;
} MarketField;

RTR_C_INLINE void clearMarketField(MarketField *pField)
{
	memset(pField, 0, sizeof(MarketField));
}
	
/* Represents one order in a MarketByOrder message.  Each corresponds to
 * a map entry */
typedef struct {
	RsslBuffer orderId;
	char orderIdString[MAX_ORDER_ID_STRLEN];
	RsslMapEntryActions action;
	RsslInt32 setId;
	RsslInt32 fieldEntriesCount;
	MarketField *fieldEntries;
} MarketOrder;

RTR_C_INLINE void clearMarketOrder(MarketOrder *pOrder)
{
	memset(pOrder, 0, sizeof(MarketOrder));
	pOrder->setId = -1;
}

/* Represents one MarketByOrder message */
typedef struct{
	RsslInt32				orderCount;					/* Number of orders in the list. */
	MarketOrder				*orders;					/* List of orders. */
	RsslLocalFieldSetDefDb	*setDefDb;					/* Set definitions to encode, if any. */
	RsslUInt32				estimatedContentLength;		/* Estimated size of payload. */
} MarketByOrderMsg;

RTR_C_INLINE void clearMarketByOrderMsg(MarketByOrderMsg *pMsg)
{
	memset(pMsg, 0, sizeof(MarketByOrderMsg));
}

/* Stores list of market by order messages. */
typedef struct {
	MarketByOrderMsg	refreshMsg;			/* Refresh message. */
	RsslInt32			updateMsgCount;		/* Number of updates in list. */
	MarketByOrderMsg	*updateMsgs;		/* List of updates. */
	RsslInt32			postMsgCount;		/* Number of posts in list. */
	MarketByOrderMsg	*postMsgs;			/* List of posts. */
	RsslInt32			genMsgCount;		/* Number of generic messages in list. */
	MarketByOrderMsg	*genMsgs;			/* List of generic messages. */
} MarketByOrderMsgList;

/* Clears a MarketByOrderMsgList. */
RTR_C_INLINE void clearMarketByOrderMsgList(MarketByOrderMsgList *pMsgList)
{
	memset(pMsgList, 0, sizeof(MarketByOrderMsgList));
}

/* Represents one MarketPrice message */
typedef struct {
	RsslInt32	fieldEntriesCount;		/* Number of fields in list. */
	MarketField *fieldEntries;			/* List of fields. */
	RsslUInt32	estimatedContentLength;	/* Estimated size of payload. */
} MarketPriceMsg;

RTR_C_INLINE void clearMarketPriceMsg(MarketPriceMsg *pMsg)
{
	memset(pMsg, 0, sizeof(MarketPriceMsg));
}

/* Stores the initial refresh to send and the list of update messages. */
typedef struct {
	MarketPriceMsg	refreshMsg;		/* Refresh message. */
	RsslInt32		updateMsgCount;	/* Number of updates in list. */
	MarketPriceMsg	*updateMsgs;	/* List of updates. */
	RsslInt32		postMsgCount;	/* Number of posts in list. */
	MarketPriceMsg	*postMsgs;		/* List of posts. */
	RsslInt32		genMsgCount;	/* Number of generic messages in list. */
	MarketPriceMsg	*genMsgs;		/* List of generic messages. */
} MarketPriceMsgList;

/* Clears a MarketPriceMsgList. */
RTR_C_INLINE void clearMarketPriceMsgList(MarketPriceMsgList *pMsgList)
{
	memset(pMsgList, 0, sizeof(MarketPriceMsgList));
}

/* Defines the payload of MarketPrice responses */
extern RsslBool xmlMsgDataHasMarketPrice;
extern MarketPriceMsgList xmlMarketPriceMsgs;

/* Defines the payload of MarketByOrder responses */
extern RsslBool xmlMsgDataHasMarketByOrder;
extern MarketByOrderMsgList xmlMarketByOrderMsgs;

/* Contains the globally-used dictionary. */
extern RsslDataDictionary dictionary; 

/*** XML Parsing ***/

/* Builds the MarketPrice message list. */
RsslRet parseXMLMarketPrice(xmlNode *pXmlMsgList);

/* Builds the MarketByOrder message list */
RsslRet parseXMLMarketByOrder(xmlNode *pXmlMsgList);

/* Whether XML data has been loaded. */
extern RsslBool xmlMsgDataLoaded;

/* Builds payloads for messages from XML. Loads dictionary */
RsslRet xmlMsgDataInit(char *filename);

/* Cleans up memory associated with message payloads. Frees dictionary. */
void xmlMsgDataCleanup();


#ifdef __cplusplus
};
#endif

#endif
