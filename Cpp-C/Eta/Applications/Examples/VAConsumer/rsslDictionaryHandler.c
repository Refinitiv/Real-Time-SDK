/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This is the Dictionary handler for the rsslVAConsumer
 * application. It provides functionality for loading
 * the dictionary from file when possible, and a callback 
 * for receiving dictionaries from the provider.
 */

#include "rtr/rsslRDMDictionaryMsg.h"
#include "rsslDictionaryHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslVASendMessage.h"

#include "rtr/rsslPayloadCache.h"

/* field dictionary file name */
static const char *fieldDictionaryFileName = "RDMFieldDictionary";
/* enum table file name */
static const char *enumTableFileName = "enumtype.def";

/*
 * Loads the field/enumType dictionaries from a file.
 */
void loadDictionary(ChannelCommand *pCommand)
{
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	RsslBool fieldDictionaryLoaded = RSSL_FALSE, enumTableLoaded = RSSL_FALSE;

	/* Load dictionary from file if possible. Otherwise we will try to download it. */
	rsslClearDataDictionary(&pCommand->dictionary);
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &pCommand->dictionary, &errorText) < 0)
		printf("\nUnable to load field dictionary.  Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
		fieldDictionaryLoaded = RSSL_TRUE;
			
	if (rsslLoadEnumTypeDictionary(enumTableFileName, &pCommand->dictionary, &errorText) < 0)
		printf("\nUnable to load enum type dictionary.  Will attempt to download from provider.\n\tError Text: %s\n", errorText.data);
	else
		enumTableLoaded = RSSL_TRUE;

	if (fieldDictionaryLoaded && enumTableLoaded)
	{
		/* Dictionary was successfully loaded from files. */
		pCommand->dictionariesLoadedFromFile = RSSL_TRUE;
		pCommand->dictionariesLoaded = RSSL_TRUE;

		if (pCommand->cacheInfo.useCache == RSSL_TRUE)
		{
			if ( RSSL_RET_SUCCESS != rsslPayloadCacheSetDictionary( pCommand->cacheInfo.cacheHandle,
										&pCommand->dictionary,
										pCommand->cacheInfo.cacheDictionaryKey,
										&pCommand->cacheInfo.cacheErrorInfo) )
			{
				pCommand->cacheInfo.useCache = RSSL_FALSE;
				printf("\nUnable to bind dictionary to cache. Disabling cache for this connection.\n\tError (%d): %s\n",
						pCommand->cacheInfo.cacheErrorInfo.rsslErrorId, pCommand->cacheInfo.cacheErrorInfo.text);
			}
		}
	}
	else
	{
		/* Dictionary could not be loaded from files. We will need to get them from the provider. */
		pCommand->dictionariesLoaded = RSSL_FALSE;
		pCommand->dictionariesLoadedFromFile = RSSL_FALSE;

		/* Make sure dictionary is cleaned up */
		rsslDeleteDataDictionary(&pCommand->dictionary);
		rsslClearDataDictionary(&pCommand->dictionary);
	}
}

/*
 * Calls the function to delete the dictionary, freeing all memory associated with it.
 */
void freeDictionary(ChannelCommand *pCommand)
{
	rsslDeleteDataDictionary(&pCommand->dictionary);
}

/*
 * Returns whether or not field dictionary has been loaded
 */
RsslBool isDictionaryLoaded(ChannelCommand *pCommand)
{
	return pCommand->dictionariesLoaded;
}

/*
 * Returns the data dictionary.
 */
RsslDataDictionary* getDictionary(ChannelCommand *pCommand)
{
	return &pCommand->dictionary;
}

/*
 * Processes information contained in Dictionary responses.
 * Takes the payload of the messages and caches them
 * using the RSSL utilities for decoding dictionary info.
 */
RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDictionaryMsgEvent *pDictionaryMsgEvent)
{
	char	errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslDecodeIterator dIter;
	ChannelCommand *pCommand = (ChannelCommand*)pReactorChannel->userSpecPtr;
	RsslRDMDictionaryMsg *pDictionaryMsg = pDictionaryMsgEvent->pRDMDictionaryMsg;


	if (!pDictionaryMsg)
	{
		RsslErrorInfo *pError = pDictionaryMsgEvent->baseMsgEvent.pErrorInfo;
		printf("dictionaryResponseCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		closeConnection(pReactor, pReactorChannel, pCommand);
		return RSSL_RC_CRET_SUCCESS;
	}


	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_DC_MT_REFRESH:
	{
		RsslRDMDictionaryRefresh *pRefresh = &pDictionaryMsg->refresh;

		if (pRefresh->flags & RDM_DC_RFF_HAS_INFO)
		{
			/* The first part of a dictionary refresh should contain information about its type.
			 * Save this information and use it as subsequent parts arrive. */

			switch(pRefresh->type)
			{
				case RDM_DICTIONARY_FIELD_DEFINITIONS: pCommand->fieldDictionaryStreamId = pRefresh->rdmMsgBase.streamId; break;
				case RDM_DICTIONARY_ENUM_TABLES: pCommand->enumDictionaryStreamId = pRefresh->rdmMsgBase.streamId; break;
				default: 
					printf("Unknown dictionary type %llu from message on stream %d\n", pRefresh->type, pRefresh->rdmMsgBase.streamId);
					closeConnection(pReactor, pReactorChannel, pCommand);
					return RSSL_RC_CRET_SUCCESS;
			}
		}

		/* decode dictionary response */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

		/* Set the iterator to the payload of the response and pass it to
		 * the appropriate dictionary decoder. */
		rsslSetDecodeIteratorBuffer(&dIter, &pRefresh->dataBody);

		printf("\nReceived Dictionary Response: %.*s\n", pRefresh->dictionaryName.length, pRefresh->dictionaryName.length ? pRefresh->dictionaryName.data : "");

		if (pRefresh->rdmMsgBase.streamId == pCommand->fieldDictionaryStreamId)
		{
    		if (rsslDecodeFieldDictionary(&dIter, &pCommand->dictionary, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS)
			{
				if (pRefresh->flags & RDM_DC_RFF_IS_COMPLETE)
				{
					printf("Field Dictionary complete.\n");

					if (pCommand->cacheInfo.useCache == RSSL_TRUE)
					{
						if ( RSSL_RET_SUCCESS != rsslPayloadCacheSetDictionary( pCommand->cacheInfo.cacheHandle,
													&pCommand->dictionary,
													pCommand->cacheInfo.cacheDictionaryKey,
													&pCommand->cacheInfo.cacheErrorInfo) )
						{
							pCommand->cacheInfo.useCache = RSSL_FALSE;
							printf("\nUnable to bind dictionary to cache. Disabling cache for this connection.\n\tError (%d): %s\n",
									pCommand->cacheInfo.cacheErrorInfo.rsslErrorId, pCommand->cacheInfo.cacheErrorInfo.text);
						}
					}
				}
			}
			else
    		{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
				closeConnection(pReactor, pReactorChannel, pCommand);
    		}
		}
		else if (pRefresh->rdmMsgBase.streamId == pCommand->enumDictionaryStreamId)
		{
    		if (rsslDecodeEnumTypeDictionary(&dIter, &pCommand->dictionary, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS)
			{
				if (pRefresh->flags & RDM_DC_RFF_IS_COMPLETE)
					printf("Enumerated Types Dictionary complete.\n");
			}
			else
    		{
    			printf("Decoding Dictionary failed: %.*s\n", errorText.length, errorText.data);
    			closeConnection(pReactor, pReactorChannel, pCommand);
    		}

		}
		else
		{
			printf("Received unexpected dictionary message on stream %d\n", pDictionaryMsg->rdmMsgBase.streamId);
		}
		return RSSL_RC_CRET_SUCCESS;
	}

	case RDM_DC_MT_STATUS:
		printf("\nReceived StatusMsg for dictionary\n");
		return RSSL_RC_CRET_SUCCESS;

	default:
		printf("\nReceived Unhandled Dictionary Msg Type: %d\n", pDictionaryMsg->rdmMsgBase.rdmMsgType);
		return RSSL_RC_CRET_SUCCESS;
	}

}

