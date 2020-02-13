/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/



#include "rsslItemHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryProvider.h"
#include "rsslVASendMessage.h"
#include "rsslLoginHandler.h"
#include "rsslVAMarketPriceItems.h"

#include "rsslVACacheHandler.h"

#define MAX_ITEM_LIST_SIZE OPEN_LIMIT * NUM_CLIENT_SESSIONS
#define MAX_ITEM_REQ_INFO_LIST_SIZE OPEN_LIMIT * NUM_CLIENT_SESSIONS
#define MAX_SYMBOL_LIST_SIZE 100
#define SYMBOL_LIST_REFRESH 0
#define SYMBOL_LIST_UPDATE_ADD 1
#define SYMBOL_LIST_UPDATE_DELETE 2

/*
 * This is the item handler for the rsslVAProvider application.
 * It uses a simple array based watch list for its implementation and
 * is limited to OPEN_LIMIT items per channel.  It provides functions
 * for processing item requests from consumers and sending back
 * the refresh/update messages.  Functions for sending 
 * request reject/close status messages, initializing the item handler, 
 * checking if the item count per channel has been reached,
 * checking if an item is already opened on a channel, checking if a
 * stream is already in use, and closing item streams are also
 * provided.
 * 
 * This handler provides data for MarketPrice, MarketByOrder and 
 * MarketByPrice item requests.
 */

/* item information list */
static RsslItemInfo itemInfoList[MAX_ITEM_LIST_SIZE];
/* item request information list */
static RsslItemRequestInfo itemRequestInfoList[MAX_ITEM_REQ_INFO_LIST_SIZE];
/* rsslProvider QoS */
RsslQos rsslProviderQos = RSSL_INIT_QOS;

static RsslRet sendMarketByPriceResponse(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo, RsslVACacheInfo *pCacheInfo);
static RsslRet sendMarketByPriceMultiPart(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo, RsslVACacheInfo *pCacheInfo, RsslBool withUpdates);

static RsslRet buildRefreshResponse(RsslItemRequestInfo* itemReqInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo);
static RsslRet buildUpdateResponse(RsslItemRequestInfo* itemReqInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo);

/*
 * Initializes item handler internal structures.
 */
void initItemHandler()
{
	int i;

	/* set Qos for rsslProvider */
	rsslProviderQos.dynamic = RSSL_FALSE;
	rsslProviderQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	rsslProviderQos.timeliness = RSSL_QOS_TIME_REALTIME;

	/* clear item information list */
	for (i = 0; i < MAX_ITEM_LIST_SIZE; i++)
	{
		clearItemInfo(&itemInfoList[i]);
	}

	/* clear item request information list */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		clearItemReqInfo(&itemRequestInfoList[i]);
	}
}

/*
 * Checks if a request matches an item already open on the request's channel.
 *   (or rejects the request if it is invalid).
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to get the item request information structure for
 * msg - The partially decoded message
 * key - The message key
 * rejectReason - if the request is invalid in some way, the specific reason is returned.
 */
static RsslItemRequestInfo* getMatchingItemReqInfo(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == pReactorChannel )
		{ 
			if (itemRequestInfoList[i].domainType == msg->msgBase.domainType
					&& RSSL_RET_SUCCESS == rsslCompareMsgKeys(&itemRequestInfoList[i].MsgKey, key))
			{
				/* The request has the same domain and key as one currently open for this channel. */

				if ( itemRequestInfoList[i].StreamId != stream )
				{
					/* The request has a different stream ID, meaning it would open the same item on another stream.
					 * This is not allowed(except for private streams). */
					if (!(msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
							&& !(itemRequestInfoList[i].IsPrivateStreamRequest))
					{
						*rejectReason = ITEM_ALREADY_OPENED;
						return NULL;
					}

					/* Otherwise continue checking the list. */
				}
				else
				{
					/* Check that the private stream flag matches correctly. */
					if ((msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM) && !itemRequestInfoList[i].IsPrivateStreamRequest
							|| !(msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM) && itemRequestInfoList[i].IsPrivateStreamRequest)
					{
						/* This item would be a match except that the private stream flag does not match. */
						*rejectReason = PRIVATE_STREAM_MISMATCH;
						return NULL;
					}

					/* The domain, key, stream ID, and private stream flag all match, so this item is a match, and the request is a reissue. */
					return &itemRequestInfoList[i];
				}
			}
			else if (itemRequestInfoList[i].StreamId == stream)
			{
				/* This stream ID is already in use for a different item. */
				*rejectReason = STREAM_ALREADY_IN_USE;
				return NULL;
			}
		}
	}
	
	*rejectReason = ITEM_REJECT_NONE;
	return NULL;
}

/*
 * Initializes a new item request information structure for a channel
 *   (or rejects the request if its channel has too many items open).
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to get the item request information structure for
 * msg - The partially decoded message
 * key - The message key
 * rejectReason - if the request is invalid in some way, the specific reason is returned.
 */
static RsslRet getNewItemReqInfo(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason, RsslItemRequestInfo** newItemRequestInfo)
{

	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;
	int count = 0;

	/* Find an available item request info structure to use,
	 * and check that the channel has not reached its allowed limit of open items. */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse)
		{
			if (itemRequestInfoList[i].Chnl == pReactorChannel)
			{
				++count;
				if (count >= OPEN_LIMIT)
				{
					/* Consumer has requested too many items. */
					*rejectReason = ITEM_COUNT_REACHED;
					*newItemRequestInfo = NULL;
				}
			}
		}
		else if(!itemRequestInfo)
		{
			itemRequestInfo = &itemRequestInfoList[i];
		}
	}

	if (!itemRequestInfo)
	{
		printf("\nFailed to get storage for item request.\n");
		return RSSL_RET_FAILURE;
	}

	itemRequestInfo->Chnl = pReactorChannel;
	itemRequestInfo->IsInUse = RSSL_TRUE;
	if (rsslCopyMsgKey(&itemRequestInfo->MsgKey, key) == RSSL_RET_FAILURE)
	{
		*rejectReason = ITEM_NOT_SUPPORTED;
		*newItemRequestInfo = NULL;
	}

	itemRequestInfo->domainType = msg->msgBase.domainType;

	itemRequestInfo->StreamId = stream;

	/* get IsPrivateStreamRequest */
	if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
	{
		itemRequestInfo->IsPrivateStreamRequest = RSSL_TRUE;
	}

	/* get IsStreamingRequest */
	if (msg->requestMsg.flags & RSSL_RQMF_STREAMING)
	{
		itemRequestInfo->IsStreamingRequest = RSSL_TRUE;
	}

	/* get IncludeKeyInUpdates */
	if (msg->requestMsg.flags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
	{
		itemRequestInfo->IncludeKeyInUpdates = RSSL_TRUE;
	}

	/* get item information */
	itemRequestInfo->Itemname[itemRequestInfo->MsgKey.name.length] = '\0';
	if (getItemInfo(pReactor, pReactorChannel, itemRequestInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (!itemRequestInfo->ItemInfo)
	{
		printf("\nFailed to get storage for item info.\n");
		return RSSL_RET_FAILURE;
	}

	/* increment item interest count if new request */
	itemRequestInfo->ItemInfo->InterestCount++;

	/* Provide a refresh if one was requested. */
	itemRequestInfo->ItemInfo->IsRefreshComplete = (msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH) ? RSSL_TRUE : RSSL_FALSE;

	*newItemRequestInfo = itemRequestInfo;

	return RSSL_RET_SUCCESS;
}

/*
 * finds the item request info associated with a channel and stream
 */
RsslItemRequestInfo *findItemReqInfo(RsslReactorChannel* pReactorChannel, RsslInt32 streamId)
{
	int i;

	for (i=0; i<MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].Chnl == pReactorChannel && itemRequestInfoList[i].StreamId == streamId)
			return &itemRequestInfoList[i];
	}
	return NULL;
}

/*
 * Frees an item request information structure.
 * itemReqInfo - The item request information structure to free
 */
static RsslRet freeItemReqInfo(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo)
{
	if (itemReqInfo)
	{
		/* decrement item interest count */
		if (itemReqInfo->ItemInfo && itemReqInfo->ItemInfo->InterestCount > 0)
		{
			itemReqInfo->ItemInfo->InterestCount--;
		}

		if(itemReqInfo->domainType != RSSL_DMT_SYMBOL_LIST && itemReqInfo->ItemInfo->InterestCount == 0)
		{
			if (deleteSymbolListItem(pReactor, itemReqInfo) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}



		/* free item information if no more interest */
		if (itemReqInfo->ItemInfo->InterestCount == 0)
		{
			freeItemInfo(itemReqInfo->ItemInfo);
		}

		/* free item request information */
		clearItemReqInfo(itemReqInfo);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Gets item information structure associated with the item name.
 */
static RsslRet getItemInfo(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslItemRequestInfo* itemReqInfo)
{
	int i;
	RsslItemInfo* rsslItemInfo = NULL;
	char* itemName = itemReqInfo->Itemname;

	/* first check for existing item */
	for (i = 0; i < MAX_ITEM_LIST_SIZE; i++)
	{
		if (!strcmp(itemName, itemInfoList[i].Itemname)
				&& itemReqInfo->domainType == itemInfoList[i].domainType
				&& itemReqInfo->IsPrivateStreamRequest == itemInfoList[i].IsPrivateStream)
		{
			rsslItemInfo = &itemInfoList[i];
			break;
		}
	}

	/* if no existing item, get new one */
	if (!rsslItemInfo)
	{
		for (i = 0; i < MAX_ITEM_LIST_SIZE; i++)
		{
			if (itemInfoList[i].InterestCount == 0)
			{
				snprintf(itemInfoList[i].Itemname, 128, "%s", itemName);
				rsslItemInfo = &itemInfoList[i];
				rsslItemInfo->domainType = itemReqInfo->domainType;
				rsslItemInfo->IsPrivateStream = itemReqInfo->IsPrivateStreamRequest;
				switch(rsslItemInfo->domainType)
				{
				case RSSL_DMT_MARKET_PRICE:
					rsslItemInfo->itemData = (void*)getMarketPriceItem(itemName);
					break;
				case RSSL_DMT_MARKET_BY_ORDER:
					rsslItemInfo->itemData = (void*)getMarketByOrderItem(itemName);
					break;
				case RSSL_DMT_MARKET_BY_PRICE:
					rsslItemInfo->itemData = (void*)getMarketByPriceItem(itemName);
					break;
				case RSSL_DMT_SYMBOL_LIST:
					break;
				}
				
				if (!(rsslItemInfo->itemData) && (rsslItemInfo->domainType != RSSL_DMT_SYMBOL_LIST))
				{
					printf("\nFailed to get storage for item data.\n");
					return RSSL_RET_FAILURE;

				}

				if(rsslItemInfo->domainType != RSSL_DMT_SYMBOL_LIST)
				{
					if (addSymbolListItem(pReactor, pReactorChannel, itemReqInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				break;
			}
		}
	}

	itemReqInfo->ItemInfo = rsslItemInfo;

	return RSSL_RET_SUCCESS;
}

/*
 * finds an item info given the item name and domain
 */
RsslItemInfo *findItemInfo(RsslBuffer *name, RsslUInt8 domainType)
{
	int i;

	for (i=0; i<MAX_ITEM_LIST_SIZE; i++)
	{
		if (itemInfoList[i].domainType == domainType && strncmp(itemInfoList[i].Itemname, name->data, name->length) == 0)
			return &itemInfoList[i];
	}

	return NULL;
}

/*
 * Frees an item information structure.
 * itemInfo - The item information structure to free
 */
static void freeItemInfo(RsslItemInfo* itemInfo)
{
	if (itemInfo)
	{
		/* free if no more interest */
		if (itemInfo->InterestCount == 0)
		{
			switch(itemInfo->domainType)
			{
			case RSSL_DMT_MARKET_PRICE:
				freeMarketPriceItem((RsslMarketPriceItem*)itemInfo->itemData);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				freeMarketByOrderItem((RsslMarketByOrderItem*)itemInfo->itemData);
				break;
			case RSSL_DMT_MARKET_BY_PRICE:
				freeMarketByPriceItem((RsslMarketByPriceItem*)itemInfo->itemData);
				break;
			case RSSL_DMT_SYMBOL_LIST:
				break;
			}
			clearItemInfo(itemInfo);
		}
	}
}

/*
 * Updates any item information that's currently in use.
 */
void updateItemInfo()
{
	int i;

	for (i = 0; i < MAX_ITEM_LIST_SIZE; i++)
	{
		if (itemInfoList[i].InterestCount > 0)
		{
			switch(itemInfoList[i].domainType)
			{
			case RSSL_DMT_MARKET_PRICE:
				updateMarketPriceItemFields((RsslMarketPriceItem*)itemInfoList[i].itemData);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				updateMarketByOrderItemFields((RsslMarketByOrderItem*)itemInfoList[i].itemData);
				break;
			case RSSL_DMT_MARKET_BY_PRICE:
				updateMarketByPriceItemFields((RsslMarketByPriceItem*)itemInfoList[i].itemData);
				break;
			case RSSL_DMT_SYMBOL_LIST:
				break;
			default:
				printf("\nUnknown domain\n");
			}
		}
	}
}

/* 
 * update the item info from the post based on market domain
 * This example supports posting on the market Price domain
 * a more fully functional implementation would support additional domains
 */
RsslRet updateItemInfoFromPost(RsslItemInfo *itemInfo, RsslMsg *msg, RsslDecodeIterator* dIter, RsslErrorInfo *error)
{
	RsslRet ret;

	switch(itemInfo->domainType)
	{
	case RSSL_DMT_MARKET_PRICE:
		ret = updateMarketPriceItemFieldsFromPost((RsslMarketPriceItem*)itemInfo->itemData, dIter, error);
		break;

	case RSSL_DMT_MARKET_BY_ORDER:
	case RSSL_DMT_MARKET_BY_PRICE:
	case RSSL_DMT_YIELD_CURVE:
	default:
		sprintf(error->rsslError.text, "Unsupported domain(%d) in post message update/refresh\n", itemInfo->domainType);
		ret = error->rsslError.rsslErrorId = RSSL_RET_FAILURE;
	}
	return ret;
}

/*
 * Closes an item stream.  This consists of finding the original
 * item request information associated with the streamId and freeing
 * the item request information.
 * streamId - The stream id of the item to be closed
 */
static RsslRet closeItemStream(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslInt32 streamId)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	/* find original item request information associated with streamId */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if(itemRequestInfoList[i].StreamId == streamId && itemRequestInfoList[i].Chnl == pReactorChannel)
		{
			itemRequestInfo = &itemRequestInfoList[i];
			break;
		}
	}

	/* remove original item request information */
	if (itemRequestInfo)
	{
		printf("Closing item stream id %d with item name: %.*s\n", itemRequestInfo->StreamId, (int)strlen(itemRequestInfo->Itemname), itemRequestInfo->Itemname);
		if (freeItemReqInfo(pReactor, itemRequestInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("No item found for StreamId: %d\n", streamId);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Closes all open item streams for a channel.  This consists of finding
 * all original item request information associated with the channel and freeing
 * the item request information.
 * pReactorChannel - The channel of the items to be closed
 */
void closeItemChnlStreams(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	/* find original item request information associated with streamId */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if(itemRequestInfoList[i].Chnl == pReactorChannel)
		{
			itemRequestInfo = &itemRequestInfoList[i];
			/* remove original item request information */
			printf("Closing item stream id %d with item name: %.*s and domainType: %u\n", itemRequestInfo->StreamId, (int)strlen(itemRequestInfo->Itemname), itemRequestInfo->Itemname, itemRequestInfo->domainType);
			freeItemReqInfo(pReactor, itemRequestInfo);
		}
	}
}

RsslRet sendAck(RsslReactor *pReactor, RsslReactorChannel *chnl, RsslPostMsg *postMsg, RsslUInt8 nakCode, char *errText)
{
	RsslBuffer *ackBuf;
	RsslErrorInfo error;
	RsslRet ret;

	// send an ack if it was requested
	if (postMsg->flags & RSSL_PSMF_ACK)
	{
		if ((ackBuf = rsslReactorGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
		{
			printf("\n rsslReactorGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslError.rsslErrorId);
			return RSSL_RET_FAILURE;
		}
		
		if ((ret = encodeAck(chnl, ackBuf, postMsg, nakCode, errText)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(chnl, ackBuf, &error);
			printf("\n encodeAck() Failed (ret = %d)\n", ret);
			return RSSL_RET_FAILURE;
		}
		
		if (sendMessage(pReactor, chnl, ackBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}
//API QA
RsslRet decodeFieldEntry(RsslDataDictionary *dictionary, RsslFieldEntry* fEntry, RsslDecodeIterator *dIter)
{
	RsslRet ret = 0;
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslUInt64 fidUIntValue = 0;
	RsslInt64 fidIntValue = 0;
	RsslFloat tempFloat = 0;
	RsslDouble tempDouble = 0;
	RsslReal fidRealValue = RSSL_INIT_REAL;
	RsslEnum fidEnumValue;
	RsslFloat fidFloatValue = 0;
	RsslDouble fidDoubleValue = 0;
	RsslQos fidQosValue = RSSL_INIT_QOS; 
	RsslDateTime fidDateTimeValue;
	RsslState fidStateValue;
	RsslBuffer fidBufferValue;
	RsslBuffer fidDateTimeBuf;
	RsslBuffer fidRealBuf;
	RsslBuffer fidStateBuf;
	RsslBuffer fidQosBuf;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* get dictionary entry */
	if (!dictionary->isInitialized)
	{
		//dumpHexBuffer(&fEntry->encData);
		return RSSL_RET_SUCCESS;
	}
	else
		dictionaryEntry = dictionary->entriesArray[fEntry->fieldId];

	/* return if no entry found */
	if (!dictionaryEntry) 
    {
		printf("\tFid %d not found in dictionary\n", fEntry->fieldId);
		//dumpHexBuffer(&fEntry->encData);
		return RSSL_RET_SUCCESS;
    }

	/* print out fid name */
	printf("\t%-20s", dictionaryEntry->acronym.data);
	/* decode and print out fid value */
	dataType = dictionaryEntry->rwfType;
	switch (dataType)
	{
		case RSSL_DT_UINT:
			if ((ret = rsslDecodeUInt(dIter, &fidUIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLU "\n", fidUIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_INT:
			if ((ret = rsslDecodeInt(dIter, &fidIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLD "\n", fidIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeInt() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_FLOAT:
			if ((ret = rsslDecodeFloat(dIter, &fidFloatValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f\n", fidFloatValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeFloat() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DOUBLE:
			if ((ret = rsslDecodeDouble(dIter, &fidDoubleValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f\n", fidDoubleValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDouble() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_REAL:
			if ((ret = rsslDecodeReal(dIter, &fidRealValue)) == RSSL_RET_SUCCESS)
			{
				fidRealBuf.data = (char*)alloca(35);
				fidRealBuf.length = 35;
				rsslRealToString(&fidRealBuf, &fidRealValue);
				printf("%s\n", fidRealBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeReal() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_ENUM:
			if ((ret = rsslDecodeEnum(dIter, &fidEnumValue)) == RSSL_RET_SUCCESS)
			{
				RsslEnumType *pEnumType = getFieldEntryEnumType(dictionaryEntry, fidEnumValue);
				if (pEnumType)
    				printf("%.*s(%d)\n", pEnumType->display.length, pEnumType->display.data, fidEnumValue);
				else
    				printf("%d\n", fidEnumValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeEnum() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATE:
			if ((ret = rsslDecodeDate(dIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDate() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_TIME:
			if ((ret = rsslDecodeTime(dIter, &fidDateTimeValue.time)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_TIME, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeTime() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATETIME:
			if ((ret = rsslDecodeDateTime(dIter, &fidDateTimeValue)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(50);
				fidDateTimeBuf.length = 50;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATETIME, &fidDateTimeValue);
				printf("%s\n", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDateTime() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_QOS:
			if((ret = rsslDecodeQos(dIter, &fidQosValue)) == RSSL_RET_SUCCESS) {
				fidQosBuf.data = (char*)alloca(100);
				fidQosBuf.length = 100;
				rsslQosToString(&fidQosBuf, &fidQosValue);
				printf("%s\n", fidQosBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeQos() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		case RSSL_DT_STATE:
			if((ret = rsslDecodeState(dIter, &fidStateValue)) == RSSL_RET_SUCCESS) {
				int stateBufLen = 80;
				if (fidStateValue.text.data)
					stateBufLen += fidStateValue.text.length;
				fidStateBuf.data = (char*)alloca(stateBufLen);
				fidStateBuf.length = stateBufLen;
				rsslStateToString(&fidStateBuf, &fidStateValue);
				printf("%.*s\n", fidStateBuf.length, fidStateBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeState() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		
		/*For an example of array decoding, see fieldListEncDec.c*/
		case RSSL_DT_ARRAY:
		break;
		case RSSL_DT_BUFFER:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			if((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
			{
				printf("%.*s\n", fidBufferValue.length, fidBufferValue.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA) 
			{
				printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
				return ret;
			}
			break;
		default:
			printf("Unsupported data type (%d) for fid value\n", dataType);
			break;
	}
	if (ret == RSSL_RET_BLANK_DATA)
	{
		printf("<blank data>\n");
	}

	return RSSL_RET_SUCCESS;
}
/*
 * decodeFieldList
 */
RsslRet decodeFieldList(RsslDataDictionary *dictionary, RsslDecodeIterator *dIter)
{
	RsslRet ret;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;

	if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) == RSSL_RET_SUCCESS)
	{
		/* decode each field entry in list */
		while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret == RSSL_RET_SUCCESS)
			{
				/* decode field entry info */
				if (decodeFieldEntry(dictionary, &fEntry, dIter) != RSSL_RET_SUCCESS)
				{
					printf("\ndecodeFieldEntry() failed\n");
					return RSSL_RET_FAILURE;
				}
			}
			else
			{
				printf("rsslDecodeFieldEntry() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}
		}
		if (ret == RSSL_RET_END_OF_CONTAINER)
			return RSSL_RET_SUCCESS;
	}
	else
	{
		printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
		return RSSL_RET_FAILURE;
	}

	return ret;
}
//END API QA

/*
 * Processes a posting request.  
 * This consists of decoding the status/update/refresh information and sending it out to all clients
 * the posting request may be on-stream or off-stream
 *
 * In this example, we will send the post to any stream which has the item open.
 * We will also update the item's field values with the post values.
 * if the item name in an off-steam post is not open on any streams, then no updates will be sent or made.
 * a more complete implementation might choose to add unknown(or new) items to the item cache if the client has sufficient postUserRights
 * a more complete implementation might also choose to add unknown(or new) fields on a posting refresh to the item cache.
 *
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processPost(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	LoginRequestInfo *loginRequestInfo;
	RsslPostMsg			*postMsg = &msg->postMsg;
	RsslItemRequestInfo	*itemReqInfo;
	RsslItemInfo		*itemInfo;
	RsslMsg				nestedMsg;
	RsslUpdateMsg		updateMsg;
	RsslErrorInfo		error;
	RsslRet				ret;
	char				*errText = NULL;
	int					i;
	RsslVACacheInfo *pCacheInfo;

	// get the login stream so that we can see if the post was an off-stream post
	if ((loginRequestInfo =  findLoginRequestInfo(chnl)) == NULL)
	{
		errText = (char *)"Received a post message request from client before login\n";
		if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		printf(errText);
		return RSSL_RET_SUCCESS;
	}

	// if the post is on the login stream, then it's an off-stream post
	if (loginRequestInfo->loginRequest.rdmMsgBase.streamId == msg->msgBase.streamId)
	{
		// the msg key must be specified to provide the item name
		if ((postMsg->flags & RSSL_PSMF_HAS_MSG_KEY) == 0)
		{
			errText = (char *)"Received an off-stream post message request from client without a msgkey\n";
			if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf(errText);
			return RSSL_RET_SUCCESS;
		}
		printf("Received an off-stream item post (item=%.*s)\n", postMsg->msgBase.msgKey.name.length, postMsg->msgBase.msgKey.name.data);
		// look up the item name
		// for this example, we will treat an unknown item as an error
		// However, other providers may choose to add the item to their cache
		if ((itemInfo = findItemInfo(&postMsg->msgBase.msgKey.name, postMsg->msgBase.domainType)) == NULL)
		{
			errText = (char *)"Received an off-stream post message for an item that doesnt exist\n";
			if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_SYMBOL_UNKNOWN, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf(errText);
			return RSSL_RET_SUCCESS;
		}
	}
	else
	{
		// the msgkey is not required for on-stream post
		// get the item request associated with this on-stream post
		if ((itemReqInfo = findItemReqInfo(chnl, postMsg->msgBase.streamId)) == NULL)
		{
			errText = (char *)"Received an on-stream post message on a stream that does not have an item open\n";
			if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf(errText);
			return RSSL_RET_SUCCESS;
		}

		itemInfo = itemReqInfo->ItemInfo;
		printf("Received an on-stream post for item=%s\n", itemInfo->Itemname);
	    // API QA
	    /* get key */
		printf("Post Msg Key ContainerType=%d\n", postMsg->msgBase.msgKey.attribContainerType);

		if(postMsg->msgBase.msgKey.attribContainerType == RSSL_DT_FIELD_LIST)
		{
			RsslDecodeIterator decodeIt;
			rsslClearDecodeIterator(&decodeIt);
			rsslSetDecodeIteratorRWFVersion(&decodeIt, chnl->majorVersion, chnl->minorVersion);
			rsslSetDecodeIteratorBuffer(&decodeIt, &postMsg->msgBase.msgKey.encAttrib);

			decodeFieldList(getDictionary(), &decodeIt);
		}

		//printf("Post Msg Key Data=\n%.*s", postMsg->msgBase.msgKey.encAttrib.data);
		// END API QA
	}

	// if the post message contains another message, then use the "contained" message as the update/refresh/status
	if (postMsg->msgBase.containerType == RSSL_DT_MSG)
	{	
		rsslClearMsg(&nestedMsg);
		rsslDecodeMsg(dIter, &nestedMsg);
		switch(nestedMsg.msgBase.msgClass)
		{
		case RSSL_MC_REFRESH:
			nestedMsg.refreshMsg.postUserInfo = postMsg->postUserInfo;
			nestedMsg.refreshMsg.flags |= RSSL_RFMF_HAS_POST_USER_INFO;
			if (updateItemInfoFromPost(itemInfo, &nestedMsg, dIter, &error) != RSSL_RET_SUCCESS)
			{
				if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.rsslError.text) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				return RSSL_RET_SUCCESS;
			}
			break;

		case RSSL_MC_UPDATE:
			nestedMsg.updateMsg.postUserInfo = postMsg->postUserInfo;
			nestedMsg.updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
			if (updateItemInfoFromPost(itemInfo, &nestedMsg, dIter, &error) != RSSL_RET_SUCCESS)
			{
				if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.rsslError.text) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				return RSSL_RET_SUCCESS;
			}
			break;

		case RSSL_MC_STATUS:
			nestedMsg.statusMsg.postUserInfo = postMsg->postUserInfo;
			nestedMsg.statusMsg.flags |= RSSL_STMF_HAS_POST_USER_INFO;
			if ((nestedMsg.statusMsg.flags & RSSL_STMF_HAS_STATE) != 0 && nestedMsg.statusMsg.state.streamState == RSSL_STREAM_CLOSED)
			{
				// check if the user has the rights to send a post that closes an item
				if ((postMsg->flags & RSSL_PSMF_HAS_POST_USER_RIGHTS) == 0 || (postMsg->postUserRights & RSSL_PSUR_DELETE) == 0)
				{
					errText = (char *)"client has insufficient rights to close/delete an item";
					if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					printf(errText);
					return RSSL_RET_SUCCESS;
				}
			}
			break;
		}
	}
	else
	{	
		rsslClearUpdateMsg(&updateMsg);
		updateMsg.msgBase = postMsg->msgBase;
		updateMsg.msgBase.msgClass = RSSL_MC_UPDATE;
		updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
		updateMsg.postUserInfo = postMsg->postUserInfo;

		if (postMsg->flags & RSSL_PSMF_HAS_MSG_KEY)
			updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;

		if (updateItemInfoFromPost(itemInfo, msg, dIter, &error) != RSSL_RET_SUCCESS)
		{
			if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.rsslError.text) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			return RSSL_RET_SUCCESS;
		}
	}

	if (sendAck(pReactor, chnl, postMsg, RSSL_NAKC_NONE, NULL) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	// apply post to cache entry for this item
	pCacheInfo = getCacheInfo();
	if (pCacheInfo->useCache)
	{
		applyMsgToCache(&itemInfo->cacheEntryHandle, pCacheInfo, msg, dIter);
	}

	// send the post to all streams with this item open
	for (i=0; i<MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].ItemInfo == itemInfo)
		{
			RsslBuffer *sendBuf;
			RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;

			if ((sendBuf = rsslReactorGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
			{
				printf("\n rsslReactorGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslError.rsslErrorId);
				return RSSL_RET_FAILURE;
			}
			if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, sendBuf)) < RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(chnl, sendBuf, &error);
				printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}
			rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

			if (postMsg->msgBase.containerType == RSSL_DT_MSG)
			{
				// send the contained/embedded message if there was one.
				nestedMsg.msgBase.streamId = itemRequestInfoList[i].StreamId;

				if ((ret = rsslEncodeMsg(&encodeIter, &nestedMsg)) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(chnl, sendBuf, &error);
					printf("\n rsslEncodeMsg() Failed (ret = %d)\n", ret);
					return RSSL_RET_FAILURE;
				}
				sendBuf->length = rsslGetEncodedBufferLength(&encodeIter);
				
				if (sendMessage(pReactor, itemRequestInfoList[i].Chnl, sendBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				// check if its a status close and close any open streams if it is
				if (nestedMsg.msgBase.msgClass == RSSL_MC_STATUS && (nestedMsg.statusMsg.flags & RSSL_STMF_HAS_STATE) != 0 && nestedMsg.statusMsg.state.streamState == RSSL_STREAM_CLOSED)
				{
					if (closeItemStream(pReactor, itemRequestInfoList[i].Chnl, nestedMsg.msgBase.streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}
			else
			{
				// send an update message if the post contained data
				updateMsg.msgBase.streamId = itemRequestInfoList[i].StreamId;
				if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg *)&updateMsg)) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(chnl, sendBuf, &error);
					printf("\n rsslEncodeMsg() Failed (ret = %d)\n", ret);
					return RSSL_RET_FAILURE;
				}
				sendBuf->length = rsslGetEncodedBufferLength(&encodeIter);
				
				if (sendMessage(pReactor, itemRequestInfoList[i].Chnl, sendBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
* Processes a single item request.  This consist of storing the request information,
* then calling sendItemResponse() to send the response
* pReactorChannel - The channel of the response
* msg - the partially decoded message
* dIter - the decode iterator
*/
RsslRet processSingleItemRequest(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* key, RsslBool isPrivateStream)

{
	RsslItemRequestInfo* itemReqInfo;
	RsslUInt8 domainType = msg->msgBase.domainType;
	RsslItemRejectReason rejectReason;

	/* check for private stream special item name without private stream flag set */
	if (!isPrivateStream &&	rsslBufferIsEqual(&key->name, &RSSL_SPECIAL_PRIVATE_STREAM_ITEM))
	{
		if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, PRIVATE_STREAM_REDIRECT, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	/* check for invalid symbol list request */
	if((domainType == RSSL_DMT_SYMBOL_LIST) && (msg->msgBase.msgKey.name.length != 0))
	{
		RsslBuffer upaItemListName;
		upaItemListName.data = (char *)"_UPA_ITEM_LIST";
		upaItemListName.length = (RsslUInt32)strlen("_UPA_ITEM_LIST");

		/* if the consumer specified symbol list name isn't "_UPA_ITEM_LIST", reject it */
		if (!rsslBufferIsEqual(&msg->msgBase.msgKey.name, &upaItemListName))
		{
			if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, ITEM_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			return RSSL_RET_SUCCESS;
		}
	}

	/* check if item already opened with exact same key on another stream */
	itemReqInfo = getMatchingItemReqInfo(pReactor, pReactorChannel, msg, msg->msgBase.streamId, key, &rejectReason); /* Check for reissue request. */
	if (!itemReqInfo && rejectReason == ITEM_REJECT_NONE)
	{
			if (getNewItemReqInfo(pReactor, pReactorChannel, msg, msg->msgBase.streamId, key, &rejectReason, &itemReqInfo) != RSSL_RET_SUCCESS) /* No matching items. This is a new request. */
				return RSSL_RET_FAILURE;
	}

	if (!itemReqInfo)
	{
		if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
	{
		printf("\nReceived Private Stream Item Request for %.*s (streamId=%d) on domain %s\n", (int)strlen(itemReqInfo->Itemname), itemReqInfo->Itemname, msg->msgBase.streamId,  rsslDomainTypeToString(itemReqInfo->domainType));
	}
	else
	{
	printf("\nReceived Item Request for %.*s (streamId=%d) on domain %s\n", (int)strlen(itemReqInfo->Itemname), itemReqInfo->Itemname, msg->msgBase.streamId,  rsslDomainTypeToString(itemReqInfo->domainType));
	}

	/* send item response */
	if (!(msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH))
	{
		itemReqInfo->ItemInfo->IsRefreshComplete = RSSL_FALSE;
		if (sendItemResponse(pReactor, itemReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	/* if this is a streaming request, set the refresh complete flag */
	/* if this is a snapshot request, free the request info structure */
	if (itemReqInfo->IsStreamingRequest) /* streaming request */
	{
		itemReqInfo->ItemInfo->IsRefreshComplete = RSSL_TRUE;
	}
	else /* snapshot request */
	{
		if (freeItemReqInfo(pReactor, itemReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
*Process a batch request.  This consist of storing the request information,
*then calling sendItemResponse() to send a response for each item requested.
*pReactorChannel - The channel of the response
*msg - The partially decoded message
*dIter - The decode iterator
*/
RsslRet processBatchRequest(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* key, RsslBool isPrivateStream)
{
	RsslBuffer *msgBuf = NULL;
	RsslUInt8 dataState = RSSL_DATA_OK;
	RsslItemRequestInfo* itemReqInfo;
	RsslUInt8 domainType = msg->msgBase.domainType;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslUInt32		itemStream;
	RsslArray	array;
	RsslBuffer	arrayEntry;
	RsslErrorInfo	error;
	RsslRet		ret;
	RsslUInt32 numOfItemsProcessed = 0;

	printf("\nReceived a batch item request (streamId=%d) on domain %s\n", msg->msgBase.streamId, rsslDomainTypeToString(domainType));

	/* check if batch stream already in use with a different key */
	if (isStreamInUse(pReactorChannel, msg, key))
	{
		if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, STREAM_ALREADY_IN_USE, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}
		
	// The payload of a batch request contains an elementList
	if((ret = rsslDecodeElementList(dIter, &elementList, 0)) < RSSL_RET_SUCCESS)
	{
		if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, REQUEST_DECODE_ERROR, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	while((ret = rsslDecodeElementEntry(dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST))
		{
			// The list of items names is in an array
			if((ret = rsslDecodeArray(dIter, &array)) < RSSL_RET_SUCCESS)
			{
				printf("\nrsslDecodeArray() Failed for batch request(ret = %d)\n", ret);
				break;
			}

			// Get each requested item name
			// We will assign consecutive stream IDs for each item
			// starting with the stream following the one from the batch request made on
			itemStream = msg->msgBase.streamId;
			while ((ret = rsslDecodeArrayEntry(dIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				RsslItemRejectReason rejectReason;
				if(ret < RSSL_RET_SUCCESS)
				{
					printf("\nrsslDecodeArrayEntry() Failed for batch request(ret = %d)\n", ret);
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}

				/* check if stream already in use with a different key */
				itemStream++;			// bump the stream ID with each item we find in the batch request
				
				numOfItemsProcessed++;

				/* check for private stream special item name without private stream flag set */
				if (!isPrivateStream &&	rsslBufferIsEqual(&arrayEntry, &RSSL_SPECIAL_PRIVATE_STREAM_ITEM))
				{
					if (sendItemRequestReject(pReactor, pReactorChannel, itemStream, domainType, PRIVATE_STREAM_REDIRECT, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_OK;	//  Data is Ok for PRIVATE_STREAM_REDIRECT use cases 
					continue;
				}

				
				// all of the items requested have the same key.  They use the key of the batch request.
				// The only difference is the name
				key->flags |= RSSL_MKF_HAS_NAME;
				key->name.data = arrayEntry.data;
				key->name.length = arrayEntry.length;

				itemReqInfo = getMatchingItemReqInfo(pReactor, pReactorChannel, msg, itemStream, key, &rejectReason);

				if(!itemReqInfo && rejectReason != ITEM_REJECT_NONE)
				{
					if (sendItemRequestReject(pReactor, pReactorChannel, itemStream, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}
				else if(itemReqInfo)
				{
					/* Batch requests should not be used to reissue item requests. */
					if (sendItemRequestReject(pReactor, pReactorChannel, itemStream, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}
				
				if (getNewItemReqInfo(pReactor, pReactorChannel, msg, itemStream, key, &rejectReason, &itemReqInfo) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				if(!itemReqInfo)
				{
					/* Batch requests should not be used to reissuse item reqeusts */
					if (sendItemRequestReject(pReactor, pReactorChannel, itemStream, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}

				if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
				{
					printf("\nReceived Private Stream Item Request for %.*s (streamId=%d) on domain %s\n", (int)strlen(itemReqInfo->Itemname), itemReqInfo->Itemname, itemStream,  rsslDomainTypeToString(itemReqInfo->domainType));
				}
				else
				{
					printf("\nReceived Item Request for %.*s (streamId=%d) on domain %s\n", (int)strlen(itemReqInfo->Itemname), itemReqInfo->Itemname, itemStream,  rsslDomainTypeToString(itemReqInfo->domainType));
				}

				/* send item response/refresh if required */
				if(!(msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH))
				{
					if (sendItemResponse(pReactor, itemReqInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}

				/* if this is a streaming request, set the refresh complete flag*/
				/* if this is a snapshot request, free the request info structure */
				if (itemReqInfo->IsStreamingRequest) /* streaming request*/
				{
					itemReqInfo->ItemInfo->IsRefreshComplete = RSSL_TRUE; // we've sent the refresh
				}
				else /* snapshot request - so we don't have to send updates */
				{
					if (freeItemReqInfo(pReactor, itemReqInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}
		}
	}
	// now that we have processed the batch request and sent the responses for all the items, send a response for the batch request itself
	/* get a buffer for the batch status close */
	if ((msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("\rsslReactorGetBuffer() Failed (rsslErrorID = %d)\n", error.rsslError.rsslErrorId);
		return RSSL_RET_FAILURE;
	}
	
	// we close the stream the batch request was made on (and later send the item responses on different streams)
	if ((ret = encodeCloseStatusToBatch(pReactor, pReactorChannel, domainType, msgBuf, msg->msgBase.streamId, dataState, numOfItemsProcessed, BATCH_REQUEST)) < RSSL_RET_SUCCESS)
	{
		rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &error);
		printf("\nencodeBatchCloseStatus() Failed (ret = %d)\n", ret);
		return RSSL_RET_FAILURE;
	}

	/* send the batch close */
	if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Processes an batch close message.  This consists of storing the request information,
 * then calling sendItemResponse() to send a response for each item requested.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processBatchClose(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslBuffer *msgBuf = NULL;
	RsslUInt8 dataState = RSSL_DATA_OK;
	RsslUInt8 domainType = msg->msgBase.domainType;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslInt64	streamId64 = 0;
	RsslInt32	itemStreamId = 0;
	RsslArray	valArray;
	RsslBuffer	arrayEntry;
	RsslErrorInfo	error;
	RsslRet		ret;
	RsslUInt32 numOfItemsProcessed = 0;

	printf("\nReceived batch item close (streamId=%d) on domain %s\n", msg->msgBase.streamId, rsslDomainTypeToString(domainType));
	
	rsslClearDecodeIterator(dIter);

	// The payload of a batch request contains an elementList
	if ((ret = rsslDecodeElementList(dIter, &elementList, 0)) < RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeElementList() Failed for batch close(ret = %d)\n", ret);
		return RSSL_RET_FAILURE;
	}

	// The list of stream ids being requested is in an elementList entry with the element name of ":StreamIdList"
	while ((ret = rsslDecodeElementEntry(dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_BATCH_STREAMID_LIST))
		{
			if (elementEntry.dataType != RSSL_DT_ARRAY)
			{
				printf("\relementEntry should have an Aarray type for batch close(ret = %d)\n", ret);
				return RSSL_RET_FAILURE;
				}

			// The list of items names is in an array
			if ((ret = rsslDecodeArray(dIter, &valArray)) < RSSL_RET_SUCCESS)
			{
				printf("\nrsslDecodeArray() Failed for batch close(ret = %d)\n", ret);
				return RSSL_RET_FAILURE;
			}

			rsslClearBuffer(&arrayEntry);

			while ((ret = rsslDecodeArrayEntry(dIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret < RSSL_RET_SUCCESS)
				{
					printf("\rsslDecodeArrayEntry() Failed for batch close(ret = %d)\n", ret);
					return RSSL_RET_FAILURE;
				}

				if (valArray.primitiveType == RSSL_DT_INT)
				{
					streamId64 = 0;
					if (((ret = rsslDecodeInt(dIter, &streamId64)) != RSSL_RET_SUCCESS) && (ret != RSSL_RET_BLANK_DATA))
					{
						printf("rsslDecodeInt() failed with return code: %d\n", ret);
						return RSSL_RET_FAILURE;
					}

					itemStreamId = (RsslInt32)streamId64;
				} 

				/* close individual item stream */
				if (closeItemStream(pReactor, pReactorChannel, itemStreamId) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				else
					numOfItemsProcessed++;
			}
		}
	}

	// now that we have processed the batch close and sent the responses for all the items, send a response for the batch close itself
	/* get a buffer for the batch status close */
	if ((msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("\rsslReactorGetBuffer() Failed (rsslErrorID = %d)\n", error.rsslError.rsslErrorId);
		return RSSL_RET_FAILURE;
	}
	
	// we close the stream the batch close stream was made on
	if ((ret = encodeCloseStatusToBatch(pReactor, pReactorChannel, domainType, msgBuf, msg->msgBase.streamId, dataState, numOfItemsProcessed, BATCH_CLOSE)) < RSSL_RET_SUCCESS)
	{
		rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &error);
		printf("\nencodeCloseStatusToBatch() Failed (ret = %d)\n", ret);
		return RSSL_RET_FAILURE;
	}

	/* send batch status close to the batch close request stream */
	if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Processes an item request.  This consists of storing the request information,
 * then calling sendItemResponse() to send the response.
 * pReactorChannel - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processItemRequest(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	RsslBool isPrivateStream = RSSL_FALSE;

	RsslUInt8 domainType = msg->msgBase.domainType;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REQUEST:
		/* set isPrivateStream flag */
		isPrivateStream = (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM) > 0 ? RSSL_TRUE : RSSL_FALSE;

		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		/* check if service id correct */
		if (key->serviceId != getServiceId())
		{
			if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, INVALID_SERVICE_ID, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		/* check if QoS supported */
		if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_WORST_QOS &&
			((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsInRange(&((RsslRequestMsg *)msg)->qos, &((RsslRequestMsg *)msg)->worstQos, &rsslProviderQos))
			{
				if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, QOS_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}
		}
		else if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsEqual(&((RsslRequestMsg *)msg)->qos, &rsslProviderQos))
			{
				if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, QOS_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}
		}
		/* check for unsupported key attribute information */
		if (rsslMsgKeyCheckHasAttrib(key))
		{
			if (sendItemRequestReject(pReactor, pReactorChannel, msg->msgBase.streamId, domainType, KEY_ENC_ATTRIB_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_BATCH)
		{
			if (processBatchRequest(pReactor,pReactorChannel, msg, dIter, key, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			if (processSingleItemRequest(pReactor, pReactorChannel, msg, dIter, key, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_MC_CLOSE:
		if ( msg->closeMsg.flags & RSSL_CLMF_HAS_BATCH ) /* Batch Close */ 
		{
			printf("\nReceived Batch Close for StreamId %d\n", msg->msgBase.streamId);

			if (processBatchClose(pReactor, pReactorChannel, msg, dIter) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;				
		}
		else
		{
			printf("\nReceived Item Close for StreamId %d\n", msg->msgBase.streamId);

			/* close item stream */
			if (closeItemStream(pReactor, pReactorChannel, msg->msgBase.streamId) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_MC_POST:
		if (processPost(pReactor, pReactorChannel, msg, dIter) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		break;

	default:
		printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
		break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends an item response to a channel.  This consists of getting
 * a message buffer, encoding the response, and sending the
 * response to the consumer.
 * pReactorChannel - The channel to send a response to
 * itemReqInfo - The item request information
 */
static RsslRet sendItemResponse(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslVACacheInfo* pCacheInfo;

	pCacheInfo = getCacheInfo();

	/* market by price is handled separately due to multi-part refresh */
	if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_PRICE)
	{
		if (sendMarketByPriceResponse(pReactor, itemReqInfo, pCacheInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	/* get a buffer for the response */
	msgBuf = rsslReactorGetBuffer(itemReqInfo->Chnl, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);
	if (msgBuf == NULL)
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	/* Encode the message with data appropriate for the domain */
	switch(itemReqInfo->domainType)
	{
	case RSSL_DMT_MARKET_PRICE:
	case RSSL_DMT_MARKET_BY_ORDER:
		if (itemReqInfo->ItemInfo->IsRefreshComplete)
			ret = buildUpdateResponse(itemReqInfo, msgBuf, pCacheInfo);
		else
			ret = buildRefreshResponse(itemReqInfo, msgBuf, pCacheInfo);

		if (ret != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &rsslErrorInfo);
			return ret;
		}
		break;
		case RSSL_DMT_SYMBOL_LIST:
			/*encode symbol list response*/
			/* only encode refresh responses for the symbol list from this function.  symbol list update responses are handled separately*/
			if (!(itemReqInfo->ItemInfo->IsRefreshComplete))
			{
				if (encodeSymbolListResponse(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, (RsslUInt16)getServiceId(), getDictionary(), NULL, SYMBOL_LIST_REFRESH) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &rsslErrorInfo);
					printf("\nencodeSymbolListResponse() failed\n");
					return RSSL_RET_FAILURE;
				}
			}
			else
			{
				rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &rsslErrorInfo);
				return RSSL_RET_SUCCESS;
			}
			break;
		default:
			printf("\nReceived unhandled domain %u for item\n", itemReqInfo->domainType);
			return RSSL_RET_FAILURE;
			break;
		}

		/* send item response */
		if (sendMessage(pReactor, itemReqInfo->Chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Sends update responses for the symbol list domain. 
 * pReactorChannel - The channel to send a symbol list response to
 * itemInfo - The item information
 * responseType - The type of update response, either delete or add
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the symbol list response into
 * streamId - The stream id of the symbol list response
 * isStreaming - Flag for streaming or snapshot
 */
RsslRet sendSLItemUpdates(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslItemRequestInfo* itemReqInfo, RsslUInt8 responseType, RsslInt32 streamId, RsslBool isStreaming)
{
	RsslErrorInfo error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the response */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		if (encodeSymbolListResponse(pReactorChannel, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, streamId, isStreaming, (RsslUInt16)getServiceId(), getDictionary(), itemReqInfo->Itemname, responseType) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &error);
				printf("\nencodeSymbolListResponse() failed\n");
				return RSSL_RET_FAILURE;
			}
	}

	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", error.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
* Adds new items to the symbol list
* itemReqInfo - information about the item to be added
*/
RsslRet addSymbolListItem(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslItemRequestInfo* itemReqInfo)
{
	RsslUInt32 i, itemVacancy;
	char* newItem = itemReqInfo->Itemname;
	RsslBool foundVacancy = RSSL_FALSE;


	/* TRI and RES-DS are added to our symbol list at initialization, and are always present
	so they never need to be added again */
	if ((!(strcmp(newItem, "TRI"))) || (!(strcmp(newItem, "RES-DS"))) || (getItemCount() >= MAX_SYMBOL_LIST_SIZE))
	{
		return RSSL_RET_SUCCESS;
	}

	/* check to see if this item is already in the item list*/
	for(i = 2; i < MAX_SYMBOL_LIST_SIZE; i++)
	{
		/* if the item is already present, increment the interest count */
		if (!(strcmp(newItem, getSymbolListItemName(i))))
		{
			incrementInterestCount(i);
			return RSSL_RET_SUCCESS;
		}
		if ((getSymbolListItemStatus(i) == RSSL_FALSE) && foundVacancy == RSSL_FALSE)
		{	
			/*store the index of the first vacancy in the symbol list*/
			foundVacancy = RSSL_TRUE;
			itemVacancy = i;
		}
	}
	
	/* add the new item name to the symbol list */
	setSymbolListItemInfo(newItem, itemVacancy);
	incrementInterestCount(itemVacancy);
	incrementItemCount();
	
	/* find all consumers currently using the symbol list domain, and send them updates */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if(itemRequestInfoList[i].domainType == RSSL_DMT_SYMBOL_LIST)
		{
			if (sendSLItemUpdates(pReactor, itemRequestInfoList[i].Chnl, itemReqInfo, SYMBOL_LIST_UPDATE_ADD, itemRequestInfoList[i].StreamId, itemRequestInfoList[i].IsStreamingRequest) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
* Deletes items from the symbol list
* itemReqInfo - information about the item to be deleted
*/
RsslRet deleteSymbolListItem(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo)
{
	RsslUInt32 i;
	char* delItem = itemReqInfo->Itemname;

	/* TRI and RES-DS are always present in our symbol list and should never be deleted */
	if ((!(strcmp(delItem, "TRI"))) || (!(strcmp(delItem, "RES-DS"))))
	{
		return RSSL_RET_SUCCESS;
	}

	/* search the symbol list, and delete the item if the interest count is 0 */
	for(i = 2; i < MAX_SYMBOL_LIST_SIZE; i++)
	{
		if(!(strcmp(delItem, getSymbolListItemName(i))))
		{
			decrementInterestCount(i);

			/* no more interest in the item, so remove it from the symbol list */
			if(getInterestCount(i) == 0)
			{
				clearSymbolListItem(i);
				decrementItemCount();
				/* find all consumers using the symbol list domain and send them updates */
				for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
				{
					/* Only send Symbol List updates to active channels that have made requests */
					if(itemRequestInfoList[i].domainType == RSSL_DMT_SYMBOL_LIST && itemRequestInfoList[i].Chnl->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
					{
						if (sendSLItemUpdates(pReactor, itemRequestInfoList[i].Chnl, itemReqInfo, SYMBOL_LIST_UPDATE_DELETE, itemRequestInfoList[i].StreamId, itemRequestInfoList[i].IsStreamingRequest) != RSSL_RET_SUCCESS)
							return RSSL_RET_FAILURE;
					}
				}
			}
			break;
		}
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Builds a refresh response message for the item by encoding a message
 * buffer. This method supports Market By Order and Market Price.
 * itemReqInfo - the request for the item from the client
 * msgBuf - the buffer where the response will be encoded
 * pCacheInfo - cache information to support optional use of cache while encoding message
 */
static RsslRet buildRefreshResponse(RsslItemRequestInfo* itemReqInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;
	RsslEncodeIterator eIter;
	RsslBool applyToCache = RSSL_FALSE;

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion);
	if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	if (itemReqInfo->domainType == RSSL_DMT_MARKET_PRICE)
		ret = encodeMarketPriceResponseMsgInit(itemReqInfo->ItemInfo, &eIter, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId());
	else if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
		ret = encodeMarketByOrderResponseMsgInit(itemReqInfo->ItemInfo, &eIter, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId());
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nFailed to encode response message init\n");
		return RSSL_RET_FAILURE;
	}

	if (pCacheInfo->useCache && itemReqInfo->ItemInfo->cacheEntryHandle && rsslPayloadEntryGetDataType(itemReqInfo->ItemInfo->cacheEntryHandle) != RSSL_DT_UNKNOWN)
	{
		printf("Encoding item %s from cache.\n", itemReqInfo->Itemname);
		if ((ret = retrieveFromCache(&eIter, itemReqInfo->ItemInfo->cacheEntryHandle, pCacheInfo)) != RSSL_RET_SUCCESS)
		{
			printf("Error %d retrieving payload from cache\n", ret);
			return ret;
		}
	}
	else
	{
		/* encode from (simulated) data source */
		if (itemReqInfo->domainType == RSSL_DMT_MARKET_PRICE)
			ret = encodeMPFieldList(itemReqInfo->ItemInfo, &eIter, itemReqInfo->IsPrivateStreamRequest, getDictionary());
		else if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
			ret = encodeMBOMap(itemReqInfo->ItemInfo, &eIter, itemReqInfo->IsPrivateStreamRequest, getDictionary());
		else
			ret = RSSL_RET_FAILURE;

		if (ret != RSSL_RET_SUCCESS)
		{
			printf("Error encoding Refresh payload\n");
			return RSSL_RET_FAILURE;
		}
		applyToCache = RSSL_TRUE;
	}

	if (encodeResponseMsgComplete(&eIter) != RSSL_RET_SUCCESS)
	{
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	if (pCacheInfo->useCache && applyToCache)
	{
		printf("Applying item %s to cache.\n", itemReqInfo->Itemname);
		applyMsgBufferToCache(eIter._majorVersion, eIter._minorVersion, &itemReqInfo->ItemInfo->cacheEntryHandle, getCacheInfo(), msgBuf);
	}

	return ret;
}

/*
 * Builds an update response for the item by encoding a message
 * buffer. This method supports Market By Order and Market Price.
 * itemReqInfo - the request for the item from the client
 * msgBuf - the buffer where the response will be encoded
 * pCacheInfo - cache information to support optional use of cache while encoding message
 */
static RsslRet buildUpdateResponse(RsslItemRequestInfo* itemReqInfo, RsslBuffer *msgBuf, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;
	RsslEncodeIterator eIter;
	RsslBool applyToCache = RSSL_FALSE;

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion);

	if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}

	if (itemReqInfo->domainType == RSSL_DMT_MARKET_PRICE)
		ret = encodeMarketPriceResponseMsgInit(itemReqInfo->ItemInfo, &eIter, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId());
	else if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
		ret = encodeMarketByOrderResponseMsgInit(itemReqInfo->ItemInfo, &eIter, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId());
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nFailed to encode response message init\n");
		return RSSL_RET_FAILURE;
	}

	if (itemReqInfo->domainType == RSSL_DMT_MARKET_PRICE)
	{
		ret = encodeMPFieldList(itemReqInfo->ItemInfo, &eIter, itemReqInfo->IsPrivateStreamRequest, getDictionary());
		if (itemReqInfo->ItemInfo->cacheUpdateCount < ((RsslMarketPriceItem*)(itemReqInfo->ItemInfo->itemData))->updateCount )
		{
			applyToCache = RSSL_TRUE; /* apply update to cache 1x per item */
			itemReqInfo->ItemInfo->cacheUpdateCount++;
		}
	}
	else if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_ORDER)
	{
		ret = encodeMBOMap(itemReqInfo->ItemInfo, &eIter, itemReqInfo->IsPrivateStreamRequest, getDictionary());
		if (itemReqInfo->ItemInfo->cacheUpdateCount < ((RsslMarketByOrderItem*)(itemReqInfo->ItemInfo->itemData))->updateCount )
		{
			applyToCache = RSSL_TRUE; /* apply update to cache 1x per item */
			itemReqInfo->ItemInfo->cacheUpdateCount++;
		}
	}
	else
		ret = RSSL_RET_FAILURE;

	if (ret != RSSL_RET_SUCCESS)
	{
		printf("Error encoding Update payload\n");
		return RSSL_RET_FAILURE;
	}

	if (encodeResponseMsgComplete(&eIter) != RSSL_RET_SUCCESS)
	{
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&eIter);

	if (pCacheInfo->useCache && applyToCache)
	{
		applyMsgBufferToCache(eIter._majorVersion, eIter._minorVersion, &itemReqInfo->ItemInfo->cacheEntryHandle, getCacheInfo(), msgBuf);
	}

	return ret;
}


/*
 * Sends a market by price item response.  This consists
 * of getting a message buffer, encoding the response, and sending the
 * response to the consumer.  The refresh message is sent as multi-part
 * with an update between each part.
 * pReactorChannel - The channel to send a response to
 * itemReqInfo - The item request information
 * pCacheInfo - information for optional cache usage when encoding and sending message
 */
static RsslRet sendMarketByPriceResponse(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo, RsslVACacheInfo *pCacheInfo)
{
	RsslRet ret;
	RsslErrorInfo error;
	RsslBuffer* msgBuf = 0;
	int multiPartNo;
	RsslEncodeIterator eIter;
	RsslUInt32 bufferSize = 200;


	if (!itemReqInfo->ItemInfo->IsRefreshComplete) /* multi-part refresh */
	{
		if (itemReqInfo->ItemInfo->cacheEntryHandle == 0 || rsslPayloadEntryGetDataType(itemReqInfo->ItemInfo->cacheEntryHandle) == RSSL_DT_UNKNOWN)
		{
			/* Entry is not cached: encode from source.
			 * If cache use is enabled, this will populate cache with initial refresh.
			 */
			ret = sendMarketByPriceMultiPart(pReactor, itemReqInfo, pCacheInfo, RSSL_TRUE);
		}
		else
		{
			/* Cache entry exists, so retrieve response data from cache */

			rsslPayloadCursorClear(pCacheInfo->cursorHandle); /* Clear before a multi-part retrieval */

			/* Retrieve refresh parts until cursor indicates retrieval complete */
			multiPartNo = 0;
			while (!rsslPayloadCursorIsComplete(pCacheInfo->cursorHandle))
			{
				/* adjust buffer size to control size of refresh (and total number of parts) */
				msgBuf = rsslReactorGetBuffer(itemReqInfo->Chnl, bufferSize, RSSL_FALSE, &error);
				if (msgBuf == NULL)
				{
					printf("rsslReactorGetBuffer failed.\n\tError (%d): %s\n", error.rsslErrorInfoCode, error.errorLocation);
					ret = RSSL_RET_FAILURE;
					break;
				}
				rsslClearEncodeIterator(&eIter);
				rsslSetEncodeIteratorRWFVersion(&eIter, itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion);
				if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
				{
					printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
					ret = RSSL_RET_FAILURE;
					break;
				}

				/* Encode message header */
				if ((ret = encodeMarketByPriceRefreshMsgInit(itemReqInfo->ItemInfo, &eIter, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), multiPartNo, "Cache Refresh Part")) != RSSL_RET_SUCCESS)
				{
					printf("encodeMarketByPriceRefreshMsgInit error %d\n", ret);
					break;
				}

				/* Encode data in cache into the message payload */
				if ((ret = retrieveFromCache(&eIter, itemReqInfo->ItemInfo->cacheEntryHandle, pCacheInfo)) != RSSL_RET_SUCCESS)
				{
					printf("Retrieve payload from cache failed (%d)\n\tError (%d): %s\n",
							ret, pCacheInfo->cacheErrorInfo.rsslErrorId, pCacheInfo->cacheErrorInfo.text);
					break;
				}

				msgBuf->length = rsslGetEncodedBufferLength(&eIter);

				/* If this is the last part, set the refresh complete flag in the encoded message */
				if (rsslPayloadCursorIsComplete(pCacheInfo->cursorHandle))
					rsslSetRefreshCompleteFlag(&eIter);

				encodeMarketByPriceResponseMsgComplete(&eIter);

				/* send refresh part */
				if ((ret = sendMessage(pReactor, itemReqInfo->Chnl, msgBuf)) != RSSL_RET_SUCCESS)
				{
					printf("sendMessage failed: Error %d\n", ret);
					break;
				}
				msgBuf = 0;

				++multiPartNo;
			}

			if (ret != RSSL_RET_SUCCESS && msgBuf != NULL)
				rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &error);
		}
	}
	else /* update */
	{
		/* get a buffer for the response */
		msgBuf = rsslReactorGetBuffer(itemReqInfo->Chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			/* encode market by price update */
			if (encodeMarketByPriceUpdate(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &error);
				printf("\nencodeMarketByPriceUpdate() failed\n");
				return RSSL_RET_FAILURE;
			}

			if (pCacheInfo->useCache)
			{
				if (itemReqInfo->ItemInfo->cacheUpdateCount < ((RsslMarketByPriceItem*)(itemReqInfo->ItemInfo->itemData))->updateCount )
				{

					itemReqInfo->ItemInfo->cacheUpdateCount++;
					applyMsgBufferToCache(itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion, &itemReqInfo->ItemInfo->cacheEntryHandle, pCacheInfo, msgBuf);
				}
			}

			/* send item response */
			if (sendMessage(pReactor, itemReqInfo->Chnl, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			printf("rsslReactorGetBuffer(): Failed <%s>\n", error.rsslError.text);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encode and send a multi-part Market By Price refresh, with updates between refresh parts.
 * Optionally apply the refresh/updates to cache entry.
 * pReactor - reactor channel for sending message
 * itemReqInfo - item request information from the client
 * pCacheInfo - information for applying item payload to cache
 * withUpdates - option to send updates between refresh parts
 */
static RsslRet sendMarketByPriceMultiPart(RsslReactor *pReactor, RsslItemRequestInfo* itemReqInfo, RsslVACacheInfo *pCacheInfo, RsslBool withUpdates)
{
	RsslErrorInfo error;
	RsslBuffer* msgBuf = 0;
	int i;
	RsslMarketByPriceItem *mbpItem;

	mbpItem = (RsslMarketByPriceItem *)itemReqInfo->ItemInfo->itemData;
	for (i = 0; i < MAX_REFRESH_PARTS; i++)
	{
		/* get a buffer for the response */
		msgBuf = rsslReactorGetBuffer(itemReqInfo->Chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			/* encode market by price refresh */
			if (encodeMarketByPriceRefresh(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary(), i) != RSSL_RET_SUCCESS)
			{
				rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &error);
				printf("\nencodeMarketByPriceRefresh() failed\n");
				return RSSL_RET_FAILURE;
			}

			if (pCacheInfo->useCache)
			{
				printf("Applying item %s to cache\n", itemReqInfo->Itemname);
				applyMsgBufferToCache(itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion, &itemReqInfo->ItemInfo->cacheEntryHandle, pCacheInfo, msgBuf);
			}

			/* send item response */
			if (sendMessage(pReactor, itemReqInfo->Chnl, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			printf("rsslReactorGetBuffer(): Failed <%s>\n", error.rsslError.text);
			return RSSL_RET_FAILURE;
		}

		/* send an update between each part of the refresh */
		if (withUpdates && (i < MAX_REFRESH_PARTS - 1))
		{
			mbpItem->orders[0].ORDER_SIZE.value += i + 1; // change order size for update
			mbpItem->orders[1].ORDER_SIZE.value += i + 1; // change order size for update
			mbpItem->orders[2].ORDER_SIZE.value += i + 1; // change order size for update
			/* get a buffer for the response */
			msgBuf = rsslReactorGetBuffer(itemReqInfo->Chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

			if (msgBuf != NULL)
			{
				/* encode market by price update */
				if (encodeMarketByPriceUpdate(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(itemReqInfo->Chnl, msgBuf, &error);
					printf("\nencodeMarketByPriceUpdate() failed\n");
					return RSSL_RET_FAILURE;
				}

				if (pCacheInfo->useCache)
				{
					applyMsgBufferToCache(itemReqInfo->Chnl->majorVersion, itemReqInfo->Chnl->minorVersion, &itemReqInfo->ItemInfo->cacheEntryHandle, pCacheInfo, msgBuf);
				}

				/* send item response */
				if (sendMessage(pReactor, itemReqInfo->Chnl, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslReactorGetBuffer(): Failed <%s>\n", error.rsslError.text);
				return RSSL_RET_FAILURE;
			}
			mbpItem->orders[0].ORDER_SIZE.value -= i + 1; // change order size back for refresh
			mbpItem->orders[1].ORDER_SIZE.value -= i + 1; // change order size back for refresh
			mbpItem->orders[2].ORDER_SIZE.value -= i + 1; // change order size back for refresh
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item update(s) for a channel.  This consists
 * of finding all request information for this channel, and sending
 * the responses to the channel.
 * pReactorChannel - The channel to send update(s) to
 */
RsslRet sendItemUpdates(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	int i;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == pReactorChannel)
		{
			if (sendItemResponse(pReactor, &itemRequestInfoList[i]) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item close status message(s) for a channel.
 * This consists of finding all request information for this channel
 * and sending the close status messages to the channel.
 * pReactorChannel - The channel to send close status message(s) to
 */
void sendItemCloseStatusMsgs(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	int i;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == pReactorChannel)
		{
			sendItemCloseStatusMsg(pReactor, pReactorChannel, &itemRequestInfoList[i]);
		}
	}
}

/*
 * Sends the item close status message for a channel.
 * pReactorChannel - The channel to send close status message to
 * itemReqInfo - The original item request information
 */
static RsslRet sendItemCloseStatusMsg(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslItemRequestInfo* itemReqInfo)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the close status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode close status */
		if (encodeItemCloseStatus(pReactorChannel, itemReqInfo->ItemInfo, msgBuf, itemReqInfo->StreamId) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nencodeItemCloseStatus() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}
	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item request reject status message for a channel.
 * pReactorChannel - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendItemRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslUInt8 domainType, RsslItemRejectReason reason, RsslBool isPrivateStream)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode request reject status */
		if (encodeItemRequestReject(pReactorChannel, streamId, reason, msgBuf, domainType, isPrivateStream) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nencodeItemRequestReject() failed\n");
			return RSSL_RET_FAILURE;
		}

		printf("\nRejecting Item Request with streamId=%d and domain %s.  Reason: %s\n", streamId,  rsslDomainTypeToString(domainType), itemRejectReasonToString(reason));

		/* send request reject status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Is item count reached for this channel.
 * pReactorChannel - The channel to check item count for
 */
static RsslBool isItemCountReached(RsslReactorChannel* pReactorChannel)
{
	int i, count = 0;
	RsslBool itemCountReached = RSSL_FALSE;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == pReactorChannel)
		{
			count++;
		}
	}

	if (count >= OPEN_LIMIT)
	{
		itemCountReached = RSSL_TRUE;
	}

	return itemCountReached;
}

/*
 * Is stream already in use with a different key for this channel.
 * pReactorChannel - The channel to check stream already in use for
 * msg - The partially decoded message
 * key - The message key
*/
static RsslBool isStreamInUse(RsslReactorChannel* pReactorChannel, RsslMsg* msg, RsslMsgKey* key)
{
	int i;
	RsslBool streamInUse = RSSL_FALSE;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse &&
			itemRequestInfoList[i].Chnl == pReactorChannel &&
			itemRequestInfoList[i].StreamId == msg->msgBase.streamId)
		{
			if (rsslCompareMsgKeys(&itemRequestInfoList[i].MsgKey, key) != RSSL_RET_SUCCESS)
			{
				streamInUse = RSSL_TRUE;
				break;
			}
		}
	}

	return streamInUse;
}
