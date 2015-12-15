/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/



#include "rsslVASymbolListItems.h"
#include "rsslVASendMessage.h"


/* 
 * This handles the generation of example symbol lists, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a symbol list message and payload.
 */

#define MAX_SYMBOL_LIST_SIZE 100
#define SYMBOL_LIST_REFRESH 0
#define SYMBOL_LIST_UPDATE_ADD 1
#define SYMBOL_LIST_UPDATE_DELETE 2

/*item information list*/
static RsslSymbolListItem symbolListItemList[MAX_SYMBOL_LIST_SIZE];

static RsslUInt32 itemCount = 0;

/*
 * Encodes the symbol list response.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pReactorChannel - The channel to send a market price response to
 * itemInfo - The item information
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the market price response into
 * streamId - The stream id of the market price response
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the market price response
 * dictionary - The dictionary used for encoding
 * itemName - the name of the item to be encoded (this value is NULL if a refresh msg is being encoded)
 * responseType- The type of response to be encoded: refresh, add update, or delete update
 */
RsslRet encodeSymbolListResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslUInt16 serviceId, RsslDataDictionary* dictionary, char* itemName, RsslUInt8 responseType)
{
	RsslRet ret = 0;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslUpdateMsg updateMsg = RSSL_INIT_UPDATE_MSG;
	RsslMsgBase* msgBase;
	RsslMsg* msg;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	char stateText[MAX_ITEM_INFO_STRLEN];
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslEncodeIterator encodeIter;
	RsslBuffer tmpBuf;
	RsslUInt32 i = 0;

	rsslClearEncodeIterator(&encodeIter);
	
	/*set-up message*/
	/*set message depending on whether refresh or update*/
	if (responseType == SYMBOL_LIST_REFRESH) /*this is a refresh message*/
	{
		msgBase = &refreshMsg.msgBase;
		msgBase->msgClass = RSSL_MC_REFRESH;
		refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		if (isStreaming)
		{
			refreshMsg.state.dataState = RSSL_DATA_OK;
		}
		else
		{
			refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
		} 
		refreshMsg.state.code = RSSL_SC_NONE;

		if (isSolicited)
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}
		else
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}
		sprintf(stateText, "Item Refresh Completed");
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME;
		/* ServiceId*/
		msgBase->msgKey.serviceId = serviceId;
		/* Itemname */
		msgBase->msgKey.name.data = itemInfo->Itemname;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
		/* Qos */
		refreshMsg.qos.dynamic = RSSL_FALSE;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		msg = (RsslMsg *)&refreshMsg;

	}
	else /*this is an update message */
	{
		msgBase = &updateMsg.msgBase;
		msgBase->msgClass = RSSL_MC_UPDATE;
		msg = (RsslMsg *)&updateMsg;
	}

	msgBase->domainType = RSSL_DMT_SYMBOL_LIST;
	msgBase->containerType = RSSL_DT_MAP;

	/* StreamId*/
	msgBase->streamId = streamId;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter,msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/*encode map*/
	map.flags = 0;
	map.containerType = RSSL_DT_NO_DATA;

	
	map.keyPrimitiveType = RSSL_DT_BUFFER;

	if ((ret = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* encode map entry */
	rsslClearMapEntry(&mapEntry);
	mapEntry.flags = RSSL_MPEF_NONE;

	switch(responseType)
	{
		/* this is a refresh message, so begin encoding the entire symbol list */
		case SYMBOL_LIST_REFRESH:
			tmpBuf.data = symbolListItemList[i].itemName;
			mapEntry.action = RSSL_MPEA_ADD_ENTRY;
			break;
		/* this is an update message adding a name, so only encode the item being added to the list */
		case SYMBOL_LIST_UPDATE_ADD:	
			tmpBuf.data = itemName;
			mapEntry.action = RSSL_MPEA_ADD_ENTRY;
			break;

		/* this is an update message deleting a name */
		case SYMBOL_LIST_UPDATE_DELETE:
			tmpBuf.data = itemName;
			mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
			break;
	}
	tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
	if((ret = rsslEncodeMapEntryInit(&encodeIter, &mapEntry, &tmpBuf, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
		return ret;
	}
	if ((ret = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)))
		{
			printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}

	/* if this is a refresh message, finish encoding the entire symbol list in the response */
	if (responseType == SYMBOL_LIST_REFRESH) 
	{
		for(i = 1; i < MAX_SYMBOL_LIST_SIZE; i++) 
		{
			if(symbolListItemList[i].isInUse == RSSL_TRUE)
			{
				tmpBuf.data = symbolListItemList[i].itemName;
				tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
				if ((ret = rsslEncodeMapEntry(&encodeIter, &mapEntry, &tmpBuf)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
	}
	
	/* complete map */
	if ((ret = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);
	
	return RSSL_RET_SUCCESS;
}

/*
* Clears out a symbol list item field
* itemNum - the item number to be cleared
*/
void clearSymbolListItem(RsslUInt32 itemNum) 
{
	symbolListItemList[itemNum].isInUse = RSSL_FALSE;
	symbolListItemList[itemNum].interestCount = 0;
	memset(&symbolListItemList[itemNum].itemName[0], 0, sizeof(symbolListItemList[itemNum].itemName));
}
/*
* Returns the status of an item in the symbol list
* itemNum - An index into the symbol list array
*/
RsslBool getSymbolListItemStatus(int itemNum)
{
	return symbolListItemList[itemNum].isInUse;
}

/*
* Returns a pointer to the name of an item in the symbol list
* itemNum - An index into the symbol list array
*/
char* getSymbolListItemName(int itemNum)
{
	return symbolListItemList[itemNum].itemName;
}

/*
* Sets the status and and name of a symbol list item
* itemName - the name that the symbol list item will be set to
* itemNum - An index into the symbol list array
*/
void setSymbolListItemInfo(char* itemName, int itemNum)
{
	snprintf(symbolListItemList[itemNum].itemName, 128, "%s", itemName);
	symbolListItemList[itemNum].isInUse = RSSL_TRUE;
}

/*
* Returns the current number of items in the symbol list
*/
RsslUInt32 getItemCount()
{
	return itemCount;
}

/*
* Increments the number of items in the symbol list
*/
void incrementItemCount()
{
	itemCount++;
}

/*
* Decrements the number of items in the symbol list
*/
void decrementItemCount()
{
	itemCount--;
}

/*
* Increments the interest count of an item in the symbol list
* itemNum - An index into the symbol list array
*/
void incrementInterestCount(int itemNum)
{
	symbolListItemList[itemNum].interestCount++;
}

/*
* Decrements the interest count of an item in the symbol list
* itemNum - An index into the symbol list array
*/
void decrementInterestCount(int itemNum)
{
	symbolListItemList[itemNum].interestCount--;
}

/*
* Returns the current interest count of an item in the symbol list
* itemNum - An index into the symbol list array
*/
RsslUInt32 getInterestCount(int itemNum)
{
	return symbolListItemList[itemNum].interestCount;
}

/*
*Initializes the symbol list item fields
*/
void initSymbolListItemList() 
{
	int i;

	symbolListItemList[0].isInUse = RSSL_TRUE;
	snprintf(symbolListItemList[0].itemName, 128, "%s", "TRI");
	symbolListItemList[1].isInUse = RSSL_TRUE;
	snprintf(symbolListItemList[1].itemName, 128, "%s", "RES-DS");
	itemCount = 2;

	/*clear out the rest of the entries*/
	for(i = 2; i < MAX_SYMBOL_LIST_SIZE; i++)
	{
		clearSymbolListItem(i);
	}
}

