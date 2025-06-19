/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the dictionary handler for the rsslConsumer application.
 * It provides functions for loading the field/enumType dictionaries
 * from a file and sending requests for those dictionaries to a
 * provider.  Functions for processing the dictionary response and
 * closing a dictionary stream are also provided.
 */

#include "rsslDictionaryHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslYieldCurveHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslSendMessage.h"

/* data dictionary */
static RsslDataDictionary dictionary;
// APIQA: ETA-2660
static RsslDataDictionary dictionaryNetwork;
// END APIQA  

/* dictionary loaded flag */
static RsslBool fieldDictionaryLoaded = RSSL_FALSE;
/* dictionary loaded from file flag */
static RsslBool fieldDictionaryLoadedFromFile = RSSL_FALSE;
/* dictionary file name  */
static const char *dictionaryFileName = "RDMFieldDictionary";
/* dictionary download name */
static const char *dictionaryDownloadName = "RWFFld";

/* enum table loaded flag */
static RsslBool enumTypeDictionaryLoaded = RSSL_FALSE;
/* enum table loaded from file flag */
static RsslBool enumTypeDictionaryLoadedFromFile = RSSL_FALSE;
/* enum table file name */
static const char *enumTableFileName = "enumtype.def";
/* enum table download name */
static const char *enumTableDownloadName = "RWFEnum";

/* remember the StreamID of the field/enum refreshes and use that to determine the dictionary type */
static RsslInt32 fieldDictionaryStreamId = 0, enumDictionaryStreamId = 0;

/*
 * Loads the field/enumType dictionaries from a file.
 */
void loadDictionary()
{
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	/* Load dictionary from file if possible. Otherwise we will try to download it. */
	rsslClearDataDictionary(&dictionary);
	if (rsslLoadFieldDictionary(dictionaryFileName, &dictionary, &errorText) < 0)
		printf("\nUnable to load field dictionary.  Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
	{
// APIQA: ETA-2660
		fieldDictionaryLoaded = RSSL_FALSE;
		fieldDictionaryLoadedFromFile = RSSL_FALSE;
// END APIQA  
	}

	if (rsslLoadEnumTypeDictionary(enumTableFileName, &dictionary, &errorText) < 0)
		printf("\nUnable to load enum type dictionary.  Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
	{
// APIQA: ETA-2660
		enumTypeDictionaryLoaded = RSSL_FALSE;
		enumTypeDictionaryLoadedFromFile = RSSL_FALSE;
// END APIQA  
	}
}

/*
 * Returns whether or not field dictionary has been loaded
 */
RsslBool isFieldDictionaryLoaded()
{
	return fieldDictionaryLoaded;
}

/*
 * Returns whether or not field dictionary has been loaded from a file.
 */
RsslBool isFieldDictionaryLoadedFromFile()
{
	return fieldDictionaryLoadedFromFile;
}

/*
 * Returns whether or not the enumeration types dictionary has been loaded
 */
RsslBool isEnumTypeDictionaryLoaded()
{
	return enumTypeDictionaryLoaded;
}

/*
 * Returns whether or not the enumeration types dictionary has been loaded from a file.
 */
RsslBool isEnumTypeDictionaryLoadedFromFile()
{
	return enumTypeDictionaryLoadedFromFile;
}

/*
 * Returns the data dictionary.
 */
RsslDataDictionary* getDictionary()
{
	return &dictionary;
}

/*
 * Sends a dictionary request to a channel.  This consists of
 * getting a message buffer, encoding the dictionary request,
 * and sending the dictionary request to the server.
 * chnl - The channel to send a dictionary request to
 * dictionaryName - The name of the dictionary to request
 * streamId - The stream id of the dictionary request 
 */
RsslRet sendDictionaryRequest(RsslChannel* chnl, const char *dictionaryName, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary request */
		if (encodeDictionaryRequest(chnl, msgBuf, dictionaryName, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeDictionaryRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send request */
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
 * Sends both field and enumType dictionary requests to a channel. 
 * chnl - The channel to send a dictionary requests to
 */
RsslRet sendDictionaryRequests(RsslChannel* chnl)
{
	if (!isFieldDictionaryLoaded())
	{
		if (sendDictionaryRequest(chnl, dictionaryDownloadName, FIELD_DICTIONARY_STREAM_ID) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}


	if (!isEnumTypeDictionaryLoaded())
	{
		if (sendDictionaryRequest(chnl, enumTableDownloadName, ENUM_TYPE_DICTIONARY_STREAM_ID) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the dictionary request.  Returns success
 * if encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a dictionary request to
 * msgBuf - The message buffer to encode the dictionary request into
 * dictionaryName - The name of the dictionary to request
 * streamId - The stream id of the dictionary request 
 */
static RsslRet encodeDictionaryRequest(RsslChannel* chnl, RsslBuffer* msgBuf, const char *dictionaryName, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_NONE;

	/* set msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.name.data = (char *)dictionaryName;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(dictionaryName);
	msg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	msg.msgBase.msgKey.filter = RDM_DICTIONARY_VERBOSE;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Processes a dictionary response.  This consists of calling
 * rsslDecodeFieldDictionary() to decode the dictionary and calling
 * sendItemRequest() to send the item request if the dictionary
 * has been fully loaded.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processDictionaryResponse(RsslChannel *chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	RsslState *pState = 0;
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	char tempData[1024];
	RsslBuffer tempBuffer;
	RDMDictionaryTypes dictionaryType = (RDMDictionaryTypes)0;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
		/* decode dictionary response */

		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		if (key)
		{
			printf("\nReceived Dictionary Response: %.*s\n", key->name.length, key->name.data);
		}
		else 
		{
			printf("\nReceived Dictionary Response\n");
		}

		pState = &msg->refreshMsg.state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		if ((msg->msgBase.streamId != fieldDictionaryStreamId) && (msg->msgBase.streamId != enumDictionaryStreamId))
		{
			/* The first part of a dictionary refresh should contain information about its type.
			 * Save this information and use it as subsequent parts arrive. */

			if (rsslExtractDictionaryType(dIter, &dictionaryType, &errorText) != RSSL_RET_SUCCESS)
    		{
    			printf("rsslGetDictionaryType() failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_FAILURE;
    		}

			switch (dictionaryType)
			{
			case RDM_DICTIONARY_FIELD_DEFINITIONS:
    			fieldDictionaryStreamId = msg->msgBase.streamId; 
				break;
			case RDM_DICTIONARY_ENUM_TABLES:
    			enumDictionaryStreamId = msg->msgBase.streamId;
				break;
			default:
				printf("Unknown dictionary type %llu from message on stream %d\n", (RsslUInt)dictionaryType, msg->msgBase.streamId);
				return RSSL_RET_SUCCESS;
			}
		}

		if (msg->msgBase.streamId == fieldDictionaryStreamId)
		{
// APIQA: ETA-2660
    		if (rsslDecodeFieldDictionary(dIter, &dictionaryNetwork, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
// END APIQA  
			{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_FAILURE;
    		}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				fieldDictionaryLoaded = RSSL_TRUE;
				fieldDictionaryStreamId = 0;
				if (!isEnumTypeDictionaryLoaded())
					printf("Field Dictionary complete, waiting for Enum Table...\n");
				else
					printf("Field Dictionary complete.\n");
			}
		} 
		else if (msg->msgBase.streamId == enumDictionaryStreamId)
		{
// APIQA: ETA-2660
    		if (rsslDecodeEnumTypeDictionary(dIter, &dictionaryNetwork, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
// END APIQA      		
			{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
    			return RSSL_RET_FAILURE;
    		}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				enumTypeDictionaryLoaded = RSSL_TRUE;
				enumDictionaryStreamId = 0;
				if (!isFieldDictionaryLoaded())
					printf("Enumerated Types Dictionary complete, waiting for Field Dictionary...\n");
				else
					printf("Enumerated Types Dictionary complete.\n");
			}
		}
		else
		{
			printf("Received unexpected dictionary message on stream %d\n", msg->msgBase.streamId);
			return RSSL_RET_SUCCESS;
		}

		if (isFieldDictionaryLoaded() && isEnumTypeDictionaryLoaded())
		{
			printf("Dictionary ready, requesting item...\n\n");
// APIQA: ETA-2660 This codes compares the network dictionary with local dictionary
			if (dictionary.numberOfEntries == dictionaryNetwork.numberOfEntries &&
				dictionary.enumTableCount == dictionaryNetwork.enumTableCount)
			{
				printf("Dictionary from file and from network are the same\n\n");
			}
			else
			{
				printf("ERROR: Dictionary from file is different than from the network\n\n");
			}
// END APIQA  

			/* send item request */
    		if (sendMarketPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketByOrderItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketByPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendYieldCurveItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			
			if (sendSymbolListRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
    	break;

	case RSSL_MC_STATUS:
		printf("\nReceived StatusMsg for dictionary\n");
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    	{
    		RsslState *pState = &msg->statusMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    	}
		break;

	default:
		printf("\nReceived Unhandled Dictionary MsgClass: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Close the dictionary stream if there is one.
 * chnl - The channel to send a dictionary close to
 * streamId - The stream id of the dictionary stream to close 
 */
RsslRet closeDictionaryStream(RsslChannel* chnl, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the dictionary close */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary close */
		if (encodeDictionaryClose(chnl, msgBuf, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeDictionaryClose() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close */
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
 * Encodes the dictionary close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a dictionary close to
 * msgBuf - The message buffer to encode the dictionary close into
 * streamId - The stream id of the dictionary stream to close 
 */
static RsslRet encodeDictionaryClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg = RSSL_INIT_CLOSE_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	
	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Calls the function to delete the dictionary, freeing all memory associated with it.
 */
void freeDictionary()
{
	rsslDeleteDataDictionary(&dictionary);
// APIQA: ETA-2660
	rsslDeleteDataDictionary(&dictionaryNetwork);
// END APIQA  
}

void resetDictionaryStreamId()
{
	fieldDictionaryStreamId = 0;
	enumDictionaryStreamId = 0;
}

/* if the dictionary stream IDs are non-zero, it indicates we downloaded the dictionaries.  Because of this, we want to free the memory before recovery 
since we will download them again upon recovery.  If the stream IDs are zero, it implies no dictionary or dictionary was loaded from file, so we only 
want to release when we are cleaning up the entire app. */
RsslBool needToDeleteDictionary()
{
	if ((fieldDictionaryStreamId != 0) || (enumDictionaryStreamId != 0))
		return RSSL_TRUE;
	else
		return RSSL_FALSE;
}

