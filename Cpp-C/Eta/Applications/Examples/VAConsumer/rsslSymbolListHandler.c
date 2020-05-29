/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/


/*
 * This is the symbol list handler for the rsslVAConsumer application.
 * It provides functions for sending the symbol list request(s) to
 * a provider and processing the response(s).  Functions for decoding
 * a map containing symbol list items is are also provided.
 */

#include "rsslSymbolListHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslVASendMessage.h"

/* Send a snapshot request to the provider */
static RsslBool snapshotRequest = RSSL_FALSE; 

/*
 * Publically visable - used to enable snapshot requesting 
 *	Send symbol list as a snapshot
 */
void setSLSnapshotRequest()
{
	snapshotRequest = RSSL_TRUE;
}

void setSymbolListName(ChannelCommand *pCommand, RsslBuffer slName)
{
	ItemRequest *pRequest = &pCommand->symbolListRequest;

	pRequest->itemName.data = pRequest->itemNameString;
	pRequest->itemName.length = snprintf(pRequest->itemNameString, sizeof(pRequest->itemNameString), "%.*s", slName.length, slName.data);
}

/*
 * Encodes the symbol list request.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pCommand - the ChannelCommand to send an item request to
 * msgBuf - The message buffer to encode the item request into
 * pRequest - Item request structure
 *
 * This function is only used within the Symbol List Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeSymbolListRequest(ChannelCommand *pCommand, RsslBuffer* msgBuf, ItemRequest *pRequest) 
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;


	/* clear encode iterator*/
	rsslClearEncodeIterator(&encodeIter);

	/*set-up message*/
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = pRequest->streamId;
	msg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	if (snapshotRequest)
	{
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_PRIORITY;
	}
	else
	{ 
		msg.flags = RSSL_RQMF_HAS_QOS | RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;
	} 
	msg.priorityClass = 1;
	msg.priorityCount = 1;

	/*copy the QoS information*/
	rsslCopyQos(&(msg.qos), &pCommand->qos);

	/*specify msgKey members*/
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)pCommand->serviceId;
	
	/*if the user specified the SL name on the command line, add it to the request */
	msg.msgBase.msgKey.name = pCommand->symbolListRequest.itemName;


	/* encode message*/
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pCommand->reactorChannel->majorVersion, pCommand->reactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
			printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
			return ret;
	}


	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);
	return RSSL_RET_SUCCESS;
}

/*
 * Publically visable symbol list request function 
 * Sends item requests to a channel.  For each item, this
 * consists of getting a message buffer, encoding the item
 * request, and sending the symbol list request to the server.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to send an item request to
 */
RsslRet sendSymbolListRequests(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel) 
{
	RsslErrorInfo cbError;
	RsslBuffer* msgBuf = 0;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	ItemRequest *pRequest = &pCommand->symbolListRequest;

	/* initialize state management array */
	/* these will be updated as refresh and status messages are received */
	pRequest->itemState.dataState = RSSL_DATA_NO_CHANGE;
	pRequest->itemState.streamState = RSSL_STREAM_UNSPECIFIED;

	if(!pCommand->sendSymbolList || (!pCommand->foundItemList && !pCommand->userSpecSymbolList))
	{
		/* User did not request symbol list at all, or
		 * user did not specify a name and the directory response didn't provide one. */
		return RSSL_RET_SUCCESS;
	}

	if(getSourceDirectoryCapabilities(pCommand, RSSL_DMT_SYMBOL_LIST) == RSSL_FALSE)
	{
		printf("RSSL_DMT_SYMBOL_LIST is not supported by the indicated provider\n");
		return RSSL_RET_SUCCESS;
	}

	/* get a buffer for the item request*/
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &cbError);

	if (msgBuf != NULL)
	{
		pCommand->symbolListRequest.streamId = SYMBOL_LIST_STREAM_ID_START;

		/*encode symbol list request*/
		if (encodeSymbolListRequest(pCommand, msgBuf, pRequest) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &cbError);
			printf("\nSymbol List encodeSymbolListRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", cbError.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Publically visable symbol list response handler 
 *
 * Processes a symbol list response.  This consists of extracting the
 * key, printing out the item name contained in the key, decoding the
 * map and map entries.
 * pReactor - RsslReactor associated with the application
 * pReactorChannel - The channel to get the symbol list response from
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processSymbolListResponse(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslMsgEvent* pMsgEvent, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;

	RsslMsgKey* key = 0;
	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslBuffer mapKey = RSSL_INIT_BUFFER;
	char tempData[1024];
	RsslBuffer tempBuffer;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	ItemRequest *pRequest = &pCommand->symbolListRequest;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			/* update our item state list if its a refresh, then process just like update */

			pRequest->itemState.dataState = msg->refreshMsg.state.dataState;
			pRequest->itemState.streamState = msg->refreshMsg.state.streamState;

			/* refresh continued - process just like update */

		case RSSL_MC_UPDATE:
			/* decode symbol list response */
			/* get key*/
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* Print descriptor of the channel this message was received from. */
			printf("\n(Channel "SOCKET_PRINT_TYPE"): ", pReactorChannel->socketId);

			/* print the name of the symbolist and the domain */
			printf("\n%.*s\nDOMAIN: %s\n", pRequest->itemName.length, pRequest->itemName.data, rsslDomainTypeToString(msg->msgBase.domainType));
	
			if (msg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				/* When displaying update information, we should also display the updateType information. */
				printf("UPDATE TYPE: %u\n", msg->updateMsg.updateType);	
			}

			if(msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);
			}

			if ((ret = rsslDecodeMap(dIter, &rsslMap)) == RSSL_RET_SUCCESS)
			{
				/* decode the map */
				while ((ret = rsslDecodeMapEntry(dIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
				{
					if(ret == RSSL_RET_SUCCESS)
					{
						/* print the name and action for this symbol list entry */
						printf("%.*s\t%s\n", mapEntry.encKey.length, mapEntry.encKey.data, mapEntryActionToString(mapEntry.action));
					}
					else
					{
						printf("rsslDecodeMapEntry() failed with return code: %d\n", ret);
						return RSSL_RET_FAILURE;
					}
				}
			}
			else
			{
				printf("rsslDecodeMap() failed with return code: %d\n", ret);
				return RSSL_RET_FAILURE;
			}

			break;

		case RSSL_MC_STATUS:
			printf("\nReceived Item StatusMsg for stream %i \n", msg->statusMsg.msgBase.streamId);
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				rsslStateToString(&tempBuffer, &msg->statusMsg.state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

				/* update our state table for posting */
				pRequest->itemState.dataState = msg->statusMsg.state.dataState;
				pRequest->itemState.streamState = msg->statusMsg.state.streamState;
			}

			break;

		case RSSL_MC_ACK:
			/* although this application only posts on MP (Market Price), 
			   ACK handler is provided for other domains to allow user to extend 
			   and post on MBO (Market By Order), MBP (Market By Price), SymbolList, and Yield Curve domains */
			printf("\nReceived AckMsg for stream %i \n", msg->msgBase.streamId);

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* print out item name from key if it has it */
			if (key)
			{
				printf("%.*s\nDOMAIN: %s\n", key->name.length, key->name.data, rsslDomainTypeToString(msg->msgBase.domainType));
			}
			printf("\tackId=%u\n", msg->ackMsg.ackId);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				printf("\tseqNum=%u\n", msg->ackMsg.seqNum);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
				printf("\tnakCode=%u\n", msg->ackMsg.nakCode);
			if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				printf("\ttext=%.*s\n", msg->ackMsg.text.length, msg->ackMsg.text.data);

			break;

		default:
			printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
			break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the symbol list close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * pCommand - the ChannelCommand to send an item close to
 * pReactorChannel - The channel to send an item close to
 * msgBuf - The message buffer to encode the item close into
 * streamId - The stream id of the symbol list
 *
 * This function is only used within the Symbol List Handler
 * and each handler has its own implementation, although much is similar
 */
static RsslRet encodeSymbolListClose(ChannelCommand *pCommand, RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg = RSSL_INIT_CLOSE_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
* Takes in a map entry action's rssluint8 representation
* and returns the string value
* mapEntryAction - the action to be performed on a map entry 
*/
const char* mapEntryActionToString(RsslUInt8 mapEntryAction)
{
	switch(mapEntryAction)
	{
		case RSSL_MPEA_UPDATE_ENTRY:
			return "RSSL_MPEA_UPDATE_ENTRY";
		case RSSL_MPEA_ADD_ENTRY:
			return "RSSL_MPEA_ADD_ENTRY";
		case RSSL_MPEA_DELETE_ENTRY:
			return "RSSL_MPEA_DELETE_ENTRY";
		default:
			return "Unknown Map Entry Action";
	}
	
}


