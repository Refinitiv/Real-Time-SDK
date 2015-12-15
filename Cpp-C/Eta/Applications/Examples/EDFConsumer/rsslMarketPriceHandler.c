/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the market price handler for the EDF example application.
 */

#include "rsslMarketPriceHandler.h"

// data dictionary
extern RsslDataDictionary dictionary;

void dumpHexBuffer(const RsslBuffer *buffer)
{
      unsigned int i;
      char * position = buffer->data;
      for (i = 0; i < buffer->length; i++, position++)
      {
            if (i % 32 == 0)
            {
                  if (i != 0)
                  {
                        printf("\n");
                  }
            }
            else if ((i != 0) && (i % 2 == 0))
            {
                  printf(" ");
            }
            printf("%2.2X", (unsigned char) *position);
      }
}

/*
 * Decodes the field entry data and prints out the field entry data
 * with help of the dictionary.  Returns success if decoding succeeds
 * or failure if decoding fails.
 * fEntry - The field entry data
 * dIter - The decode iterator
 */
RsslRet decodeFieldEntry(RsslFieldEntry* fEntry, RsslDecodeIterator *dIter)
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
	if (!dictionary.entriesArray)
	{
		dumpHexBuffer(&fEntry->encData);
		return RSSL_RET_SUCCESS;
	}
	else
		dictionaryEntry = dictionary.entriesArray[fEntry->fieldId];

	/* return if no entry found */
	if (!dictionaryEntry) 
	{
		printf("\tFid %d not found in dictionary\n", fEntry->fieldId);
		dumpHexBuffer(&fEntry->encData);
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
				printf(""RTR_LLU"\n", fidUIntValue);
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
				printf(""RTR_LLD"\n", fidIntValue);
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
 * Processes a market price response.
 */
RsslRet processMarketPriceResponse(RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslMsgKey* key = 0;
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
			/* decode market price response */
			
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

			/* decode field list */
			if ((ret = rsslDecodeFieldList(dIter, &fList, 0)) == RSSL_RET_SUCCESS)
			{
				/* decode each field entry in list */
				while ((ret = rsslDecodeFieldEntry(dIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret == RSSL_RET_SUCCESS)
					{
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
			break;
		case RSSL_MC_STATUS:
			if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    		{
				printf("Received StatusMsg for stream %i\n", msg->statusMsg.msgBase.streamId);
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
