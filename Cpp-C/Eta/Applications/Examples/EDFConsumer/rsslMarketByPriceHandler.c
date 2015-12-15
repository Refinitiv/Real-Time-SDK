/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the market by price handler for the EDF example application.
 */

#include "rsslMarketByPriceHandler.h"

extern void dumpHexBuffer(const RsslBuffer *buffer);
extern RsslRet decodeFieldEntry(RsslFieldEntry* fEntry, RsslDecodeIterator *dIter);

/*
 * Processes a market by price response.
 */
RsslRet processMarketByPriceResponse(RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
	const char* actionString;
	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslBuffer mapKey = RSSL_INIT_BUFFER;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;
	RsslBool isBatchRequest = RSSL_FALSE;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			printf("Received RefreshMsg for stream %i\n", msg->refreshMsg.msgBase.streamId);

			if (msg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
				printf("SEQ. NO.: %u\n", msg->refreshMsg.seqNum);

			/* process just like update */
		case RSSL_MC_UPDATE:
			/* decode market by price response */

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(msg);

			/* print out item name from key if it has it */
			if (key)
			{
				printf("%.*s\n", key->name.length, key->name.data);
			}
			if (msg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				printf("Received UpdateMsg for stream %i\n", msg->updateMsg.msgBase.streamId);
				/* When displaying update information, we should also display the updateType information. */
				printf("UPDATE TYPE: %u\n", msg->updateMsg.updateType);	

				if (msg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
					printf("SEQ. NO.: %u\n", msg->updateMsg.seqNum);
			}
			printf("DOMAIN: %s\n", rsslDomainTypeToString(msg->msgBase.domainType));

			if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
			{
				rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
				printf("%.*s\n", tempBuffer.length, tempBuffer.data);
			}

			/* level 2 market by price is a map of field lists */
			if ((ret = rsslDecodeMap(dIter, &rsslMap)) == RSSL_RET_SUCCESS)
			{
				/* decode any summary data - this should be a field list according to the domain model */ 
				if (rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA)
				{
					printf("SUMMARY DATA\n");
					// using XML file set definition database for decoding
					if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) == RSSL_RET_SUCCESS)
					{
						/* decode each field entry in list */
						while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
						{
							if (ret == RSSL_RET_SUCCESS)
					 		{
								/* decode field entry info */
								/* tab this over before decoding this so it looks better within this domain model */
								printf("\t");
								if ((ret = decodeFieldEntry(&fEntry, dIter)) != RSSL_RET_SUCCESS)
								{
									return ret;
								}
							}
							else
							{
								printf("rsslDecodeFieldEntry() failed with return code: %d\n", ret);
								return RSSL_RET_FAILURE;
							}
						}
					}
					else
					{
						printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
						return RSSL_RET_FAILURE;
					}
					printf("\n"); /* add blank line after summary data for readability */
				}

				/* decode the map */
				while ((ret = rsslDecodeMapEntry(dIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret == RSSL_RET_SUCCESS)
					{
						/* convert the action to a string for display purposes */
						switch(mapEntry.action)
						{
							case RSSL_MPEA_UPDATE_ENTRY:
								actionString = "RSSL_MPEA_UPDATE_ENTRY"; 
								break;
							case RSSL_MPEA_ADD_ENTRY:
								actionString = "RSSL_MPEA_ADD_ENTRY";
								break;
							case RSSL_MPEA_DELETE_ENTRY:
								actionString = "RSSL_MPEA_DELETE_ENTRY";
								break;
							default:
								actionString = "Unknown";

						}
						/* print out the key */
						if (mapKey.length)
						{
							printf("ORDER ID: ");
							dumpHexBuffer(&mapKey);
							printf("\n");
							printf("ACTION: %s\n", actionString);
						}
					
						/* there is not any payload in delete actions */
						if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
						{
							/* decode field list */
							// using XML file set definition database for decoding
							if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) == RSSL_RET_SUCCESS)
							{
								/* decode each field entry in list */
								while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
								{
									if (ret == RSSL_RET_SUCCESS)
						 			{
										/* decode field entry info */
										/* tab this over before decoding this so it looks better within this domain model */
										printf("\t");
										if ((ret = decodeFieldEntry(&fEntry, dIter)) != RSSL_RET_SUCCESS)
										{
											return ret;
										}
									}
									else
									{
										printf("rsslDecodeFieldEntry() failed with return code: %d\n", ret);
										return RSSL_RET_FAILURE;
									}
								}
							}
							else
							{
								printf("rsslDecodeFieldList() failed with return code: %d\n", ret);
								return RSSL_RET_FAILURE;
							}						
						}
						printf("\n"); /* add a space between end of order and beginning of next order for readability */
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
			}	

			break;
		default:
			printf("\nReceived Unhandled Item Msg Class: %d\n", msg->msgBase.msgClass);
    		break;
	}

	return RSSL_RET_SUCCESS;
}
