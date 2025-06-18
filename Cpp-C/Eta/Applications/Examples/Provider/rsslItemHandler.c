/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */



#include "rsslItemHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryProvider.h"
#include "rsslSendMessage.h"
#include "rsslLoginHandler.h"

#define MAX_ITEM_LIST_SIZE OPEN_LIMIT * NUM_CLIENT_SESSIONS
#define MAX_ITEM_REQ_INFO_LIST_SIZE OPEN_LIMIT * NUM_CLIENT_SESSIONS
#define MAX_SYMBOL_LIST_SIZE 100
#define SYMBOL_LIST_REFRESH 0
#define SYMBOL_LIST_UPDATE_ADD 1
#define SYMBOL_LIST_UPDATE_DELETE 2

/*
 * This is the item handler for the rsslProvider application.
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
 * This handler provides data for MarketPrice, MarketByOrder,
 * MarketByPrice, SymbolList
 * item requests.
 */

/* item information list */
static RsslItemInfo itemInfoList[MAX_ITEM_LIST_SIZE];
/* item request information list */
static RsslItemRequestInfo itemRequestInfoList[MAX_ITEM_REQ_INFO_LIST_SIZE];
/* rsslProvider QoS */
RsslQos rsslProviderQos = RSSL_INIT_QOS;

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
 * chnl - The channel to get the item request information structure for
 * msg - The partially decoded message
 * key - The message key
 * rejectReason - if the request is invalid in some way, the specific reason is returned.
 */
static RsslItemRequestInfo* getMatchingItemReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == chnl )
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
 * chnl - The channel to get the item request information structure for
 * msg - The partially decoded message
 * key - The message key
 * rejectReason - if the request is invalid in some way, the specific reason is returned.
 */
static RsslRet getNewItemReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslInt32 stream, RsslMsgKey* key, RsslItemRejectReason *rejectReason, RsslItemRequestInfo** newItemRequestInfo)
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
			if (itemRequestInfoList[i].Chnl == chnl)
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

	itemRequestInfo->Chnl = chnl;
	itemRequestInfo->IsInUse = RSSL_TRUE;
	if (rsslCopyMsgKey(&itemRequestInfo->MsgKey, key) == RSSL_RET_FAILURE)
	{
		*rejectReason = ITEM_NOT_SUPPORTED;
		*newItemRequestInfo = NULL;
	}

	itemRequestInfo->domainType = msg->msgBase.domainType;

	/* get item information */
	itemRequestInfo->Itemname[itemRequestInfo->MsgKey.name.length] = '\0';
	if (getItemInfo(itemRequestInfo, itemRequestInfo->domainType, &itemRequestInfo->ItemInfo) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if (!itemRequestInfo->ItemInfo)
	{
		printf("\nFailed to get storage for item info.\n");
		return RSSL_RET_FAILURE;
	}

	/* get IsStreamingRequest */
	if (msg->requestMsg.flags & RSSL_RQMF_STREAMING)
	{
		itemRequestInfo->IsStreamingRequest = RSSL_TRUE;
	}

	/* get IsPrivateStreamRequest */
	if (msg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
	{
		itemRequestInfo->IsPrivateStreamRequest = RSSL_TRUE;
	}

	/* get IncludeKeyInUpdates */
	if (msg->requestMsg.flags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
	{
		itemRequestInfo->IncludeKeyInUpdates = RSSL_TRUE;
	}

	/* increment item interest count if new request */
	itemRequestInfo->ItemInfo->InterestCount++;
	itemRequestInfo->StreamId = stream;

	/* Provide a refresh if one was requested. */
	itemRequestInfo->ItemInfo->IsRefreshComplete = (msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH) ? RSSL_TRUE : RSSL_FALSE;

	*newItemRequestInfo = itemRequestInfo;

	return RSSL_RET_SUCCESS;
}

/*
 * finds the item request info associated with a channel and stream
 */
RsslItemRequestInfo *findItemReqInfo(RsslChannel *chnl, RsslInt32 streamId)
{
	int i;

	for (i=0; i<MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].Chnl == chnl && itemRequestInfoList[i].StreamId == streamId)
			return &itemRequestInfoList[i];
	}
	return NULL;
}

/*
 * Frees an item request information structure.
 * itemReqInfo - The item request information structure to free
 */
static RsslRet freeItemReqInfo(RsslItemRequestInfo* itemReqInfo)
{
	if (itemReqInfo)
	{
		/* decrement item interest count */
		if (itemReqInfo->ItemInfo && itemReqInfo->ItemInfo->InterestCount > 0)
		{
			itemReqInfo->ItemInfo->InterestCount--;
		}

		/* if this item is from the MP, MBO, or MBP domains, determine if it should be deleted the symbol list */
		if(itemReqInfo->domainType != RSSL_DMT_SYMBOL_LIST)
		{	
			if (deleteSymbolListItem(itemReqInfo) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}

		/* free item information if no more interest */
		if (itemReqInfo->ItemInfo && itemReqInfo->ItemInfo->InterestCount == 0)
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
 * itemName - The item name of the item information structure
 */
static RsslRet getItemInfo(RsslItemRequestInfo* itemReqInfo, RsslUInt8 domainType, RsslItemInfo** itemInfo)
{
	int i;
	RsslItemInfo* rsslItemInfo = NULL;
	char* itemName = itemReqInfo->Itemname;

	/* first check for existing item */
	for (i = 0; i < MAX_ITEM_LIST_SIZE; i++)
	{
		if (!strcmp(itemName, itemInfoList[i].Itemname) && domainType == itemInfoList[i].domainType)
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
				snprintf(itemInfoList[i].Itemname, MAX_ITEM_INFO_STRLEN + 1, "%s", itemName);
				rsslItemInfo = &itemInfoList[i];
				rsslItemInfo->domainType = domainType;
				switch(domainType)
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
				
				if (!(rsslItemInfo->itemData) && (domainType != RSSL_DMT_SYMBOL_LIST))
				{
					printf("\nFailed to get storage for item data.\n");
					return RSSL_RET_FAILURE;
				}

				break;
			}
		}
	}

	itemReqInfo->ItemInfo = rsslItemInfo;
	/* if this item is from the MP, MBO, or MBP domains, add it to the symbol list */
	if(domainType != RSSL_DMT_SYMBOL_LIST)
	{
		if (addSymbolListItem(itemReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	
	*itemInfo = rsslItemInfo;

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
 * This example supports posting on the Market Price domain
 * a more fully functional implementation would support additional domains
 */
RsslRet updateItemInfoFromPost(RsslItemInfo *itemInfo, RsslMsg *msg, RsslDecodeIterator* dIter, RsslError *error)
{
	RsslRet ret;

	switch(itemInfo->domainType)
	{
	case RSSL_DMT_MARKET_PRICE:
		ret = updateMarketPriceItemFieldsFromPost((RsslMarketPriceItem*)itemInfo->itemData, dIter, error);
		break;

	case RSSL_DMT_MARKET_BY_ORDER:
	case RSSL_DMT_MARKET_BY_PRICE:
	default:
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "Unsupported domain(%d) in post message update/refresh\n", itemInfo->domainType);
		ret = error->rsslErrorId = RSSL_RET_FAILURE;
	}
	return ret;
}
/*
 * Closes an item stream.  This consists of finding the original
 * item request information associated with the streamId and freeing
 * the item request information.
 * streamId - The stream id of the item to be closed
 */
static RsslRet closeItemStream(RsslChannel *chnl, RsslInt32 streamId)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	/* find original item request information associated with streamId */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if(itemRequestInfoList[i].StreamId == streamId && itemRequestInfoList[i].Chnl == chnl)
		{
			itemRequestInfo = &itemRequestInfoList[i];
			break;
		}
	}

	/* remove original item request information */
	if (itemRequestInfo)
	{
		printf("Closing item stream id %d with item name: %.*s\n", itemRequestInfo->StreamId, (int)strlen(itemRequestInfo->Itemname), itemRequestInfo->Itemname);
		if (freeItemReqInfo(itemRequestInfo) != RSSL_RET_SUCCESS)
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
 * chnl - The channel of the items to be closed
 */
void closeItemChnlStreams(RsslChannel* chnl)
{
	int i;
	RsslItemRequestInfo* itemRequestInfo = NULL;

	/* find original item request information associated with streamId */
	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if(itemRequestInfoList[i].Chnl == chnl)
		{
			itemRequestInfo = &itemRequestInfoList[i];
			/* remove original item request information */
			printf("Closing item stream id %d with item name: %.*s and domainType: %u\n", itemRequestInfo->StreamId, (int)strlen(itemRequestInfo->Itemname), itemRequestInfo->Itemname, itemRequestInfo->domainType);
			freeItemReqInfo(itemRequestInfo);
		}
	}
}
RsslRet sendAck(RsslChannel *chnl, RsslPostMsg *postMsg, RsslUInt8 nakCode, char *errText)
{
	RsslBuffer *ackBuf;
	RsslError error;
	RsslRet ret;

	// send an ack if it was requested
	if (postMsg->flags & RSSL_PSMF_ACK)
	{
		if ((ackBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
		{
			printf("\n rsslGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslErrorId);
			return RSSL_RET_FAILURE;
		}
		
		if ((ret = encodeAck(chnl, ackBuf, postMsg, nakCode, errText)) < RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(ackBuf, &error); 
			printf("\n encodeAck() Failed (ret = %d)\n", ret);
			return RSSL_RET_FAILURE;
		}
		
		if (sendMessage(chnl, ackBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

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
RsslRet processPost(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslLoginRequestInfo *loginRequestInfo;
	RsslPostMsg			*postMsg = &msg->postMsg;
	RsslItemRequestInfo	*itemReqInfo;
	RsslItemInfo		*itemInfo;
	RsslMsg				nestedMsg;
	RsslUpdateMsg		updateMsg;
	RsslError			error;
	RsslRet				ret;
	char				*errText = NULL;
	int					i;
	char				postUserAddrString[16];

	// get the login stream so that we can see if the post was an off-stream post
	if ((loginRequestInfo =  findLoginReqInfo(chnl)) == NULL)
	{
		errText = (char *)"Received a post message request from client before login\n";
		if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		printf("%s", errText);
		return RSSL_RET_SUCCESS;
	}

	// if the post is on the login stream, then it's an off-stream post
	if (loginRequestInfo->StreamId == msg->msgBase.streamId)
	{
		// the msg key must be specified to provide the item name
		if ((postMsg->flags & RSSL_PSMF_HAS_MSG_KEY) == 0)
		{
			errText = (char *)"Received an off-stream post message request from client without a msgkey\n";
			if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf("%s", errText);
			return RSSL_RET_SUCCESS;
		}
		printf("Received an off-stream item post (item=%.*s)", postMsg->msgBase.msgKey.name.length, postMsg->msgBase.msgKey.name.data);
		// look up the item name
		// for this example, we will treat an unknown item as an error
		// However, other providers may choose to add the item to their cache
		if ((itemInfo = findItemInfo(&postMsg->msgBase.msgKey.name, postMsg->msgBase.domainType)) == NULL)
		{
			errText = (char *)"Received an off-stream post message for an item that doesnt exist\n";
			if (sendAck(chnl, postMsg, RSSL_NAKC_SYMBOL_UNKNOWN, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf("%s", errText);
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
			if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			printf("%s", errText);
			return RSSL_RET_SUCCESS;
		}

		itemInfo = itemReqInfo->ItemInfo;
		printf("Received an on-stream post for item=%s", itemInfo->Itemname);
	}

	/* The Visible Publisher Identity (VPI) can be found within the RsslPostUserInfo. 
	 * This will provide both the publisher ID and publisher address. Providers may define this when publishing from the postMsg.
	 */
	rsslIPAddrUIntToString(postMsg->postUserInfo.postUserAddr, postUserAddrString);
	printf(" from client with publisher user ID: \"%u\" at user address: \"%s\"\n\n", postMsg->postUserInfo.postUserId, postUserAddrString);

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
				if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.text) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				return RSSL_RET_SUCCESS;
			}
			break;

		case RSSL_MC_UPDATE:
			nestedMsg.updateMsg.postUserInfo = postMsg->postUserInfo;
			nestedMsg.updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
			if (updateItemInfoFromPost(itemInfo, &nestedMsg, dIter, &error) != RSSL_RET_SUCCESS)
			{
				if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.text) != RSSL_RET_SUCCESS)
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
					if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, errText) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					printf("%s", errText);
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
			if (sendAck(chnl, postMsg, RSSL_NAKC_INVALID_CONTENT, error.text) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			return RSSL_RET_SUCCESS;
		}
	}

	if ((ret = sendAck(chnl, postMsg, RSSL_NAKC_NONE, NULL)) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	// send the post to all streams with this item open
	for (i=0; i<MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].ItemInfo == itemInfo)
		{
			RsslBuffer *sendBuf;
			RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;

			if ((sendBuf = rsslGetBuffer(itemRequestInfoList[i].Chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
			{
				printf("\n rsslGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslErrorId);
				return RSSL_RET_FAILURE;
			}
			if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, sendBuf)) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(sendBuf, &error); 
				printf("\n rsslSetEncodeIteratorBuffer() failed with ret code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}
			rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

			if (postMsg->msgBase.containerType == RSSL_DT_MSG)
			{
				// send the contained/embedded message if there was one.
				nestedMsg.msgBase.streamId = itemRequestInfoList[i].StreamId;

				if ((ret = rsslEncodeMsg(&encodeIter, &nestedMsg)) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(sendBuf, &error); 
					printf("\n rsslEncodeMsg() Failed (ret = %d)\n", ret);
					return RSSL_RET_FAILURE;
				}
				sendBuf->length = rsslGetEncodedBufferLength(&encodeIter);
				
				if (sendMessage(itemRequestInfoList[i].Chnl, sendBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				// check if its a status close and close any open streams if it is
				if (nestedMsg.msgBase.msgClass == RSSL_MC_STATUS && (nestedMsg.statusMsg.flags & RSSL_STMF_HAS_STATE) != 0 && nestedMsg.statusMsg.state.streamState == RSSL_STREAM_CLOSED)
				{
					if (closeItemStream(itemRequestInfoList[i].Chnl, nestedMsg.msgBase.streamId) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}
			else
			{
				// send an update message if the post contained data
				updateMsg.msgBase.streamId = itemRequestInfoList[i].StreamId;
				if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg *)&updateMsg)) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(sendBuf, &error);
					printf("\n rsslEncodeMsg() Failed (ret = %d)\n", ret); 
					return RSSL_RET_FAILURE;
				}
				sendBuf->length = rsslGetEncodedBufferLength(&encodeIter);

				if (sendMessage(itemRequestInfoList[i].Chnl, sendBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Processes a single item request.  This consists of storing the request information,
 * then calling sendItemResponse() to send the response.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processSingleItemRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* key, RsslBool isPrivateStream)
{
	RsslItemRequestInfo* itemReqInfo;
	RsslUInt8 domainType = msg->msgBase.domainType;
	RsslItemRejectReason rejectReason;

	/* check for private stream special item name without private stream flag set */
	if (!isPrivateStream &&	rsslBufferIsEqual(&key->name, &RSSL_SPECIAL_PRIVATE_STREAM_ITEM))
	{
		if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, PRIVATE_STREAM_REDIRECT, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	/* check for invalid symbol list request */
	if((domainType == RSSL_DMT_SYMBOL_LIST) && (msg->msgBase.msgKey.name.length != 0))
	{
		RsslBuffer symbolListName;
		symbolListName.data = (char *)"_ETA_ITEM_LIST";
		symbolListName.length = (RsslUInt32)strlen("_ETA_ITEM_LIST");
		/* if the consumer specified symbol list name isn't "_ETA_ITEM_LIST", reject it */
		if (!rsslBufferIsEqual(&(msg->msgBase.msgKey.name), &symbolListName))
		{
			if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, ITEM_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			return RSSL_RET_SUCCESS;
		}
	}

	/* get request info structure */
	itemReqInfo = getMatchingItemReqInfo(chnl, msg, msg->msgBase.streamId, key, &rejectReason); /* Check for reissue request. */
	if (!itemReqInfo && rejectReason == ITEM_REJECT_NONE)
	{
		if (getNewItemReqInfo(chnl, msg, msg->msgBase.streamId, key, &rejectReason, &itemReqInfo) != RSSL_RET_SUCCESS)	/* No matching items. This is a new request. */
			return RSSL_RET_FAILURE;
	}

	if (!itemReqInfo)
	{
		if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
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
		if (sendItemResponse(itemReqInfo) != RSSL_RET_SUCCESS)
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
		if (freeItemReqInfo(itemReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Processes an batch item request.  This consists of storing the request information,
 * then calling sendItemResponse() to send a response for each item requested.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processBatchRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* key, RsslBool isPrivateStream)
{
	RsslBuffer *msgBuf = NULL;
	RsslUInt8 dataState = RSSL_DATA_OK;
	RsslItemRequestInfo* itemReqInfo;
	RsslUInt8 domainType = msg->msgBase.domainType;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslUInt32	itemStream;
	RsslArray	array;
	RsslBuffer	arrayEntry;
	RsslError	error;
	RsslRet		ret;
	RsslUInt32 numOfItemsProcessed = 0;

	printf("\nReceived batch item request (streamId=%d) on domain %s\n", msg->msgBase.streamId, rsslDomainTypeToString(domainType));

	/* check if batch stream already in use with a different key */
	if (isStreamInUse(chnl, msg->msgBase.streamId, key))
	{
		if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, STREAM_ALREADY_IN_USE, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}
	
	// The payload of a batch request contains an elementList
	if ((ret = rsslDecodeElementList(dIter, &elementList, 0)) < RSSL_RET_SUCCESS)
	{
		if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, REQUEST_DECODE_ERROR, isPrivateStream) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	// The list of items being requested is in an elementList entry with the element name of ":ItemList"
	while ((ret = rsslDecodeElementEntry(dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST))
		{
			// The list of items names is in an array
			if ((ret = rsslDecodeArray(dIter, &array)) < RSSL_RET_SUCCESS)
			{
				printf("\nrsslDecodeArray() Failed for batch request(ret = %d)\n", ret);
				break;
			}

			// Get each requested item name
			// We will assign consecutive stream IDs for each item 
			// starting with the stream following the one the batch request was made on
			itemStream = msg->msgBase.streamId;
			while ((ret = rsslDecodeArrayEntry(dIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				RsslItemRejectReason rejectReason;
				if (ret < RSSL_RET_SUCCESS)
				{
					printf("\rsslDecodeArrayEntry() Failed for batch request(ret = %d)\n", ret);
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}

				/* check if stream already in use with a different key */
				itemStream++;			// bump the stream ID with each item we find in the batch request

				numOfItemsProcessed++;

				/* check for private stream special item name without private stream flag set */
				if (!isPrivateStream &&	rsslBufferIsEqual(&arrayEntry, &RSSL_SPECIAL_PRIVATE_STREAM_ITEM))
				{
					if (sendItemRequestReject(chnl, itemStream, domainType, PRIVATE_STREAM_REDIRECT, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_OK;	//  Data is Ok for PRIVATE_STREAM_REDIRECT use cases 
					continue;
				}


				// all of the items requested have the same key. They use the key of the batch request.
				// The only difference is the name
				key->flags |= RSSL_MKF_HAS_NAME;
				key->name.data = arrayEntry.data;
				key->name.length = arrayEntry.length;

				itemReqInfo = getMatchingItemReqInfo(chnl, msg, itemStream, key, &rejectReason);

				if (!itemReqInfo && rejectReason != ITEM_REJECT_NONE)
				{
					if (sendItemRequestReject(chnl, itemStream, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}
				else if(itemReqInfo)
				{
					/* Batch requests should not be used to reissue item requests. */
					if (sendItemRequestReject(chnl, itemStream, domainType, BATCH_ITEM_REISSUE, isPrivateStream) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
					dataState = RSSL_DATA_SUSPECT;
					continue;
				}

				if (getNewItemReqInfo(chnl, msg, itemStream, key, &rejectReason, &itemReqInfo) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;

				if(!itemReqInfo)
				{
					/* Batch requests should not be used to reissue item requests. */
					if (sendItemRequestReject(chnl, itemStream, domainType, rejectReason, isPrivateStream) != RSSL_RET_SUCCESS)
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
				if (!(msg->requestMsg.flags & RSSL_RQMF_NO_REFRESH))
				{
					if (sendItemResponse(itemReqInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}

				/* if this is a streaming request, set the refresh complete flag */
				/* if this is a snapshot request, free the request info structure */
				if (itemReqInfo->IsStreamingRequest) /* streaming request */
				{
					itemReqInfo->ItemInfo->IsRefreshComplete = RSSL_TRUE;	// we've sent the refresh
				}
				else /* snapshot request - so we dont have to send updates */
				{
					if (freeItemReqInfo(itemReqInfo) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
			}
		}
	}
	// now that we have processed the batch request and sent responses for all the items, send a response for the batch request itself
	/* get a buffer for the batch status close */
	if ((msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("\rsslGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslErrorId);
		return RSSL_RET_FAILURE;
	}

	// we close the stream the batch request was made on (and later send the item responses on different streams)
	if ((ret = encodeCloseStatusToBatch(chnl, domainType, msgBuf, msg->msgBase.streamId, dataState, numOfItemsProcessed, BATCH_REQUEST)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error); 
		printf("\nencodeCloseStatusToBatch() Failed (ret = %d)\n", ret);
		return RSSL_RET_FAILURE;
	}

	/* send batch status close */
	if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Processes an batch close message.  This consists of storing the request information,
 * then calling sendItemResponse() to send a response for each item requested.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processBatchClose(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
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
	RsslError	error;
	RsslRet		ret;
	RsslUInt32 numOfItemsProcessed = 0;

	printf("\nReceived batch item close (streamId=%d) on domain %s\n", msg->msgBase.streamId, rsslDomainTypeToString(domainType));

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
				if (closeItemStream(chnl, itemStreamId) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				else
					numOfItemsProcessed++;
			}
		}
	}
	// now that we have processed the batch close and sent responses for all the items, send a response for the batch close itself
	/* get a buffer for the batch status close */
	if ((msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("\rsslGetBuffer() Failed (rsslErrorId = %d)\n", error.rsslErrorId);
		return RSSL_RET_FAILURE;
	}

	// we close the stream the batch close stream was made on
	if ((ret = encodeCloseStatusToBatch(chnl, domainType, msgBuf, msg->msgBase.streamId, dataState, numOfItemsProcessed, BATCH_CLOSE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error); 
		printf("\nencodeCloseStatusToBatch() Failed (ret = %d)\n", ret);
		return RSSL_RET_FAILURE;
	}

	/* send batch status close to the batch close request stream */
	if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
 * Processes an item request.  This consists of storing the request information,
 * then calling sendItemResponse() to send the response.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processItemRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
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
			if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, INVALID_SERVICE_ID, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		/* check if QoS supported */
		if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_WORST_QOS &&
			((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsInRange(&((RsslRequestMsg *)msg)->qos, &((RsslRequestMsg *)msg)->worstQos, &rsslProviderQos))
			{
				if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, QOS_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}
		}
		else if (((RsslRequestMsg *)msg)->flags & RSSL_RQMF_HAS_QOS)
		{
			if (!rsslQosIsEqual(&((RsslRequestMsg *)msg)->qos, &rsslProviderQos))
			{
				if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, QOS_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			}
		}
		/* check for unsupported key attribute information */
		if (rsslMsgKeyCheckHasAttrib(key))
		{
			if (sendItemRequestReject(chnl, msg->msgBase.streamId, domainType, KEY_ENC_ATTRIB_NOT_SUPPORTED, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		
		if (msg->requestMsg.flags & RSSL_RQMF_HAS_BATCH)
		{
			if (processBatchRequest(chnl, msg, dIter, key, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			if (processSingleItemRequest(chnl, msg, dIter, key, isPrivateStream) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		break;

	case RSSL_MC_CLOSE:
		if ( msg->closeMsg.flags & RSSL_CLMF_HAS_BATCH ) /* Batch Close */ 
		{
			printf("\nReceived Batch Close for StreamId %d\n", msg->msgBase.streamId);

			if (processBatchClose(chnl, msg, dIter) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;				
		}
		else
		{
			printf("\nReceived Item Close for StreamId %d\n", msg->msgBase.streamId);

			/* close item stream */
			if (closeItemStream(chnl, msg->msgBase.streamId) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		break;
	case RSSL_MC_POST:
		if (processPost(chnl, msg, dIter) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		break;

	default:
		printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends an item response to a channel.  This consists of getting
 * a message buffer, encoding the response, and sending the
 * response to the consumer.
 * chnl - The channel to send a response to
 * itemReqInfo - The item request information
 */
static RsslRet sendItemResponse(RsslItemRequestInfo* itemReqInfo)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* market by price is handled separately due to multi-part refresh */
	if (itemReqInfo->domainType == RSSL_DMT_MARKET_BY_PRICE)
	{
		if (sendMBPItemResponse(itemReqInfo->Chnl, itemReqInfo) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
		return RSSL_RET_SUCCESS;
	}

	/* get a buffer for the response */
	msgBuf = rsslGetBuffer(itemReqInfo->Chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{

		/* Encode the message with data appopriate for the domain */
		switch(itemReqInfo->domainType)
		{
		case RSSL_DMT_MARKET_PRICE:
			/* encode market price response */
			if (encodeMarketPriceResponse(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("\nencodeMarketPriceResponse() failed\n"); 
				return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			/* encode market by order response */
			if (encodeMarketByOrderResponse(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("\nencodeMarketByOrderResponse() failed\n"); 
				return RSSL_RET_FAILURE;
			}
			break;
		case RSSL_DMT_SYMBOL_LIST:
			/*encode symbol list response*/
			/* only encode refresh responses for the symbol list from this function.  symbol list update responses are handled seperately*/
			if (!(itemReqInfo->ItemInfo->IsRefreshComplete))
			{
				if (encodeSymbolListResponse(itemReqInfo->Chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, (RsslUInt16)getServiceId(), getDictionary(), NULL, SYMBOL_LIST_REFRESH) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error);
					printf("\nencodeSymbolListResponse() failed\n"); 
					return RSSL_RET_FAILURE;
				}
			}
			else
			{
				rsslReleaseBuffer(msgBuf, &error); 
				return RSSL_RET_SUCCESS;
			}
			break;
		default:
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nReceived unhandled domain %u for item\n", itemReqInfo->domainType); 
			return RSSL_RET_FAILURE;
		}

		/* send item response */
		if (sendMessage(itemReqInfo->Chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends update responses for the symbol list domain. 
 * chnl - The channel to send a symbol list response to
 * itemInfo - The item information
 * responseType - The type of update response, either delete or add
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the symbol list response into
 * streamId - The stream id of the symbol list response
 * isStreaming - Flag for streaming or snapshot
 */
RsslRet sendSLItemUpdates(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo, RsslUInt8 responseType, RsslInt32 streamId, RsslBool isStreaming)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		if (encodeSymbolListResponse(chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, streamId, isStreaming, (RsslUInt16)getServiceId(), getDictionary(), itemReqInfo->Itemname, responseType) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error);
				printf("\nencodeSymbolListResponse() failed\n"); 
				return RSSL_RET_FAILURE;
			}
	}

	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text); 
		return RSSL_RET_FAILURE;
	}

	if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

/*
* Adds new items to the symbol list
* itemReqInfo - information about the item to be added
*/
RsslRet addSymbolListItem(RsslItemRequestInfo* itemReqInfo)
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
			if (sendSLItemUpdates(itemRequestInfoList[i].Chnl, itemReqInfo, SYMBOL_LIST_UPDATE_ADD, itemRequestInfoList[i].StreamId, itemRequestInfoList[i].IsStreamingRequest) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
* Deletes items from the symbol list
* itemReqInfo - information about the item to be deleted
*/
RsslRet deleteSymbolListItem(RsslItemRequestInfo* itemReqInfo)
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
					if(itemRequestInfoList[i].domainType == RSSL_DMT_SYMBOL_LIST && itemRequestInfoList[i].Chnl->state == RSSL_CH_STATE_ACTIVE)
					{
						if (sendSLItemUpdates(itemRequestInfoList[i].Chnl, itemReqInfo, SYMBOL_LIST_UPDATE_DELETE, itemRequestInfoList[i].StreamId, itemRequestInfoList[i].IsStreamingRequest) != RSSL_RET_SUCCESS)
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
 * Sends a market by price item response to a channel.  This consists
 * of getting a message buffer, encoding the response, and sending the
 * response to the consumer.  The refresh message is sent as multi-part
 * with an update between each part.
 * chnl - The channel to send a response to
 * itemReqInfo - The item request information
 */
static RsslRet sendMBPItemResponse(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	int i;
	RsslMarketByPriceItem *mbpItem;

	if (!itemReqInfo->ItemInfo->IsRefreshComplete) /* multi-part refresh */
	{
		mbpItem = (RsslMarketByPriceItem *)itemReqInfo->ItemInfo->itemData;
		for (i = 0; i < MAX_REFRESH_PARTS; i++)
		{
			/* get a buffer for the response */
			msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

			if (msgBuf != NULL)
			{
				/* encode market by price refresh */
				if (encodeMarketByPriceRefresh(chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary(), i) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error);
					printf("\nencodeMarketByPriceRefresh() failed\n"); 
					return RSSL_RET_FAILURE;
				}

				/* send item response */
				if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslGetBuffer(): Failed <%s>\n", error.text); 
				return RSSL_RET_FAILURE;
			}

			/* send an update between each part of the refresh */
			if (i < MAX_REFRESH_PARTS - 1)
			{
				mbpItem->orders[0].ORDER_SIZE.value += i + 1; // change order size for update
				mbpItem->orders[1].ORDER_SIZE.value += i + 1; // change order size for update
				mbpItem->orders[2].ORDER_SIZE.value += i + 1; // change order size for update
				/* get a buffer for the response */
				msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

				if (msgBuf != NULL)
				{
					/* encode market by price update */
					if (encodeMarketByPriceUpdate(chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
					{
						rsslReleaseBuffer(msgBuf, &error); 
						printf("\nencodeMarketByPriceUpdate() failed\n");
						return RSSL_RET_FAILURE;
					}

					/* send item response */
					if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
						return RSSL_RET_FAILURE;
				}
				else
				{
					printf("rsslGetBuffer(): Failed <%s>\n", error.text);
					return RSSL_RET_FAILURE;
				}

				mbpItem->orders[0].ORDER_SIZE.value -= i + 1; // change order size back for refresh
				mbpItem->orders[1].ORDER_SIZE.value -= i + 1; // change order size back for refresh
				mbpItem->orders[2].ORDER_SIZE.value -= i + 1; // change order size back for refresh
			}
		}
	}
	else /* update */
	{
		/* get a buffer for the response */
		msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

		if (msgBuf != NULL)
		{
			/* encode market by price update */
			if (encodeMarketByPriceUpdate(chnl, itemReqInfo->ItemInfo, msgBuf, RSSL_TRUE, itemReqInfo->StreamId, itemReqInfo->IsStreamingRequest, itemReqInfo->IsPrivateStreamRequest, (RsslUInt16)getServiceId(), getDictionary()) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error); 
				printf("\nencodeMarketByPriceUpdate() failed\n");
				return RSSL_RET_FAILURE;
			}

			/* send item response */
			if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			printf("rsslGetBuffer(): Failed <%s>\n", error.text);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item update(s) for a channel.  This consists
 * of finding all request information for this channel, and sending
 * the responses to the channel.
 * chnl - The channel to send update(s) to
 */
RsslRet sendItemUpdates(RsslChannel* chnl)
{
	int i;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == chnl)
		{
			if (sendItemResponse(&itemRequestInfoList[i]) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item close status message(s) for a channel.
 * This consists of finding all request information for this channel
 * and sending the close status messages to the channel.
 * chnl - The channel to send close status message(s) to
 */
void sendItemCloseStatusMsgs(RsslChannel* chnl)
{
	int i;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == chnl)
		{
			sendItemCloseStatusMsg(chnl, &itemRequestInfoList[i]);
		}
	}
}

/*
 * Sends the item close status message for a channel.
 * chnl - The channel to send close status message to
 * itemReqInfo - The original item request information
 */
static RsslRet sendItemCloseStatusMsg(RsslChannel* chnl, RsslItemRequestInfo* itemReqInfo)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the close status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode close status */
		if (encodeItemCloseStatus(chnl, itemReqInfo->ItemInfo, msgBuf, itemReqInfo->StreamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeItemCloseStatus() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close status */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text); 
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the item request reject status message for a channel.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendItemRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslUInt8 domainType, RsslItemRejectReason reason, RsslBool isPrivateStream)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item request reject status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode request reject status */
		if (encodeItemRequestReject(chnl, streamId, reason, msgBuf, domainType, isPrivateStream) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeItemRequestReject() failed\n"); 
			return RSSL_RET_FAILURE;
		}

		printf("\nRejecting Item Request with streamId=%d and domain %s.  Reason: %s\n", streamId,  rsslDomainTypeToString(domainType), itemRejectReasonToString(reason));

		/* send request reject status */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Is item count reached for this channel.
 * chnl - The channel to check item count for
 */
static RsslBool isItemCountReached(RsslChannel* chnl)
{
	int i, count = 0;
	RsslBool itemCountReached = RSSL_FALSE;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse && itemRequestInfoList[i].Chnl == chnl)
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
 * chnl - The channel to check stream already in use for
 * msg - The partially decoded message
 * key - The message key
*/
static RsslBool isStreamInUse(RsslChannel* chnl, RsslInt32 streamId, RsslMsgKey* key)
{
	int i;
	RsslBool streamInUse = RSSL_FALSE;

	for (i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
	{
		if (itemRequestInfoList[i].IsInUse &&
			itemRequestInfoList[i].Chnl == chnl &&
			itemRequestInfoList[i].StreamId == streamId)
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

