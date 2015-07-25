/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is the queue messaging handler for the UPA Value Add consumer application.
 * It handles any queue message delivery.
 */

#include "queueMsgHandler.h"
#include "tunnelStreamHandler.h"

#define TUNNEL_STREAM_STREAM_ID 1000
#define QUEUE_MSG_STREAM_ID 2000
#define QUEUE_MSG_DOMAIN 199
#define QUEUE_MSG_FREQUENCY 2

static RsslDataDictionary fdmDictionary;

void queueMsgHandlerInit(QueueMsgHandler *pQueueMsgHandler, char *consumerName, RsslUInt8 domainType,
		RsslBool useAuthentication)
{
	int i;
	tunnelStreamHandlerInit(&pQueueMsgHandler->tunnelStreamHandler,
			consumerName, domainType, useAuthentication, RSSL_TRUE /* Queue messaging */,
			queueMsgHandlerProcessTunnelOpened,
			queueMsgHandlerProcessTunnelClosed,
			queueMsgHandlerDefaultMsgCallback,
			queueMsgHandlerQueueMsgCallback);

	rsslClearBuffer(&pQueueMsgHandler->sourceName);
	for (i = 0; i < MAX_DEST_NAMES; ++i)
		rsslClearBuffer(&pQueueMsgHandler->destNames[i]);
	pQueueMsgHandler->destNameCount = 0;
	pQueueMsgHandler->isQueueStreamOpen = RSSL_FALSE;
	pQueueMsgHandler->identifier = 0;
}

RsslBool loadFdmDictionary()
{
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	rsslClearDataDictionary(&fdmDictionary);
	if (rsslLoadFieldDictionary("FDMFixFieldDictionary", &fdmDictionary, &errorText) < 0)
	{
		printf("\nUnable to load FDMFixFieldDictionary.\n\tError Text: %s\n\n", errorText.data);
		rsslDeleteDataDictionary(&fdmDictionary);
		return RSSL_FALSE;
	}
			
	if (rsslLoadEnumTypeDictionary("FDMenumtypes.def", &fdmDictionary, &errorText) < 0)
	{
		printf("\nUnable to load FDMenumtypes.def.\n\tError Text: %s\n\n", errorText.data);
		rsslDeleteDataDictionary(&fdmDictionary);
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

void cleanupFdmDictionary()
{
	rsslDeleteDataDictionary(&fdmDictionary);
}

/* Callback for received tunnel stream messages. */
/* Processes a queue message event. */
RsslReactorCallbackRet queueMsgHandlerQueueMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamQueueMsgEvent *pEvent)
{
	RsslState *pState;
	RsslBuffer tempBuffer;
	char tempData[1024];
	QueueMsgHandler *pQueueMsgHandler = (QueueMsgHandler*)pTunnelStream->userSpecPtr;
	RsslReactorChannel *pReactorChannel = pTunnelStream->pReactorChannel;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch (pEvent->pQueueMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_QMSG_MT_DATA:
        {
			RsslBool sendAck = RSSL_FALSE;
			RsslRDMQueueData *pQueueData = (RsslRDMQueueData *)pEvent->pQueueMsg;
			RsslDecodeIterator dIter;

			printf("Received Msg on stream %d from %.*s to %.*s, ID: %lld\n",
					pQueueData->rdmMsgBase.streamId, pQueueData->sourceName.length, pQueueData->sourceName.data, pQueueData->destName.length, pQueueData->destName.data, pQueueData->identifier);

			if (rsslRDMQueueDataCheckIsPossibleDuplicate(pQueueData))
				printf("  (Message may be a duplicate of a previous message.)\n");


			printf("  Queue Depth: %u\n", pQueueData->queueDepth);
                

            switch (pQueueData->containerType)
            {

				case RSSL_DT_FIELD_LIST:
				{
                    // Received a buffer; decode it.
					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelStream->classOfService.common.protocolMajorVersion, 
							pTunnelStream->classOfService.common.protocolMajorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pQueueData->encDataBody);

					/* If queueMsgHandlerDecodeQueueData, this message is an order message so we will acknowledge it. */
					if (queueMsgHandlerDecodeQueueData(&dIter))
						ackQueueMsg(pQueueMsgHandler, &pQueueData->sourceName, pQueueData->rdmMsgBase.streamId);
    
                }
                default:
                    break;
            }                
            break;
        }

		case RDM_QMSG_MT_DATA_EXPIRED:
        {
			RsslBool sendAck = RSSL_FALSE;
			RsslRDMQueueDataExpired *pQueueDataExpired = (RsslRDMQueueDataExpired *)pEvent->pQueueMsg;
			RsslDecodeIterator dIter;

			printf("Received Msg on stream %d from %.*s to %.*s, ID: %lld (Undeliverable Message with code: %d(%s))\n",
					pQueueDataExpired->rdmMsgBase.streamId, 
					pQueueDataExpired->sourceName.length, pQueueDataExpired->sourceName.data, 
					pQueueDataExpired->destName.length, pQueueDataExpired->destName.data, 
					pQueueDataExpired->identifier, 
					pQueueDataExpired->undeliverableCode,
					rsslRDMQueueUndeliverableCodeToString(pQueueDataExpired->undeliverableCode));

			if (rsslRDMQueueDataExpiredCheckIsPossibleDuplicate(pQueueDataExpired))
				printf("  (Message may be a duplicate of a previous message.)");


			printf("  Queue Depth: %u\n", pQueueDataExpired->queueDepth);
                

            switch (pQueueDataExpired->containerType)
            {

				case RSSL_DT_FIELD_LIST:
				{
                    // Received a buffer; decode it.
					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelStream->classOfService.common.protocolMajorVersion, 
							pTunnelStream->classOfService.common.protocolMajorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pQueueDataExpired->encDataBody);
					queueMsgHandlerDecodeQueueData(&dIter);
    
                }
                default:
                    break;
            }                
            break;
        }
		case RDM_QMSG_MT_ACK:
		{
			RsslRDMQueueAck *pQueueAck = (RsslRDMQueueAck *)pEvent->pQueueMsg;
			printf("Received persistence Ack for submitted message with ID: %lld\n\n", pQueueAck->identifier);
			break;
		}
		case RDM_QMSG_MT_REFRESH:
		{
			RsslRDMQueueRefresh *pQueueRefresh = (RsslRDMQueueRefresh *)pEvent->pQueueMsg;
			pState = &pQueueRefresh->state;

			if (pState->streamState == RSSL_STREAM_OPEN
					&& pState->dataState == RSSL_DATA_OK)
				pQueueMsgHandler->isQueueStreamOpen = RSSL_TRUE;
			else
				pQueueMsgHandler->isQueueStreamOpen = RSSL_FALSE;

			rsslStateToString(&tempBuffer, pState);
			printf("Received QueueRefresh on stream %d for sourceName %.*s with %.*s\n  Queue Depth: %u\n\n", pQueueRefresh->rdmMsgBase.streamId, pQueueRefresh->sourceName.length, pQueueRefresh->sourceName.data, tempBuffer.length, tempBuffer.data, pQueueRefresh->queueDepth);

			time(&pQueueMsgHandler->nextMsgTime);
			pQueueMsgHandler->nextMsgTime += QUEUE_MSG_FREQUENCY;

			break;
		}
		case RDM_QMSG_MT_STATUS:
		{
			RsslRDMQueueStatus *pQueueStatus = (RsslRDMQueueStatus *)pEvent->pQueueMsg;
			if (pQueueStatus->flags & RDM_QMSG_STF_HAS_STATE)
			{
				pState = &pQueueStatus->state;

				if (pState->streamState == RSSL_STREAM_OPEN
						&& pState->dataState == RSSL_DATA_OK)
					pQueueMsgHandler->isQueueStreamOpen = RSSL_TRUE;
				else
					pQueueMsgHandler->isQueueStreamOpen = RSSL_FALSE;

				rsslStateToString(&tempBuffer, pState);
				printf("Received QueueStatus on stream %d with %.*s\n\n", pQueueStatus->rdmMsgBase.streamId, tempBuffer.length, tempBuffer.data);
			}
			else
			{
				printf("Received QueueStatus on stream %d\n\n", pQueueStatus->rdmMsgBase.streamId);
			}
			break;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

/* Decodes the content of a QueueData or QueueDataExpired message. */
RsslBool queueMsgHandlerDecodeQueueData(RsslDecodeIterator *pIter)
{
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslRet ret;
	RsslBool sendAck = RSSL_FALSE;

	rsslClearFieldList(&fieldList);
	ret = rsslDecodeFieldList(pIter, &fieldList, NULL);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("\nrsslDecodeFieldList failed: %d\n", ret);
		return RSSL_FALSE;
	}

	rsslClearFieldEntry(&fieldEntry);
	while ((ret = rsslDecodeFieldEntry(pIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
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

		if (ret != RSSL_RET_SUCCESS)
		{
			printf("\nrsslDecodeFieldEntry() failed: %d\n", ret);
			return RSSL_FALSE;
		}

		dictionaryEntry = fdmDictionary.entriesArray[fieldEntry.fieldId];

		/* return if no entry found */
		if (!dictionaryEntry) 
		{
			printf("\tFid %d not found in dictionary\n", fieldEntry.fieldId);
		}
		else
		{
			/* print out fid name */
			printf("\t%-20s", dictionaryEntry->acronym.data);
			/* decode and print out fid value */
			dataType = dictionaryEntry->rwfType;
			switch (dataType)
			{
				case RSSL_DT_UINT:
					if ((ret = rsslDecodeUInt(pIter, &fidUIntValue)) == RSSL_RET_SUCCESS)
					{
						printf(""RTR_LLU"\n", fidUIntValue);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_INT:
					if ((ret = rsslDecodeInt(pIter, &fidIntValue)) == RSSL_RET_SUCCESS)
					{
						printf(""RTR_LLD"\n", fidIntValue);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeInt() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_FLOAT:
					if ((ret = rsslDecodeFloat(pIter, &fidFloatValue)) == RSSL_RET_SUCCESS) 
					{
						printf("%f\n", fidFloatValue);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeFloat() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_DOUBLE:
					if ((ret = rsslDecodeDouble(pIter, &fidDoubleValue)) == RSSL_RET_SUCCESS) 
					{
						printf("%f\n", fidDoubleValue);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeDouble() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_REAL:
					if ((ret = rsslDecodeReal(pIter, &fidRealValue)) == RSSL_RET_SUCCESS)
					{
						fidRealBuf.data = (char*)alloca(35);
						fidRealBuf.length = 35;
						rsslRealToString(&fidRealBuf, &fidRealValue);
						printf("%s\n", fidRealBuf.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeReal() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_ENUM:
					if ((ret = rsslDecodeEnum(pIter, &fidEnumValue)) == RSSL_RET_SUCCESS)
					{
						RsslEnumType *pEnumType = getFieldEntryEnumType(dictionaryEntry, fidEnumValue);
						if (pEnumType)
							printf("\"%.*s\"(%d)\n", pEnumType->display.length, pEnumType->display.data, fidEnumValue);
						else
							printf("%d\n", fidEnumValue);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeEnum() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_DATE:
					if ((ret = rsslDecodeDate(pIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
					{
						fidDateTimeBuf.data = (char*)alloca(30);
						fidDateTimeBuf.length = 30;
						rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
						printf("%s\n", fidDateTimeBuf.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeDate() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_TIME:
					if ((ret = rsslDecodeTime(pIter, &fidDateTimeValue.time)) == RSSL_RET_SUCCESS)
					{
						fidDateTimeBuf.data = (char*)alloca(30);
						fidDateTimeBuf.length = 30;
						rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_TIME, &fidDateTimeValue);
						printf("%s\n", fidDateTimeBuf.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeTime() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_DATETIME:
					if ((ret = rsslDecodeDateTime(pIter, &fidDateTimeValue)) == RSSL_RET_SUCCESS)
					{
						fidDateTimeBuf.data = (char*)alloca(50);
						fidDateTimeBuf.length = 50;
						rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATETIME, &fidDateTimeValue);
						printf("%s\n", fidDateTimeBuf.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeDateTime() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_QOS:
					if((ret = rsslDecodeQos(pIter, &fidQosValue)) == RSSL_RET_SUCCESS) {
						fidQosBuf.data = (char*)alloca(100);
						fidQosBuf.length = 100;
						rsslQosToString(&fidQosBuf, &fidQosValue);
						printf("%s\n", fidQosBuf.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeQos() failed with return code: %d\n", ret);
						return RSSL_FALSE;
					}
					break;
				case RSSL_DT_STATE:
					if((ret = rsslDecodeState(pIter, &fidStateValue)) == RSSL_RET_SUCCESS) {
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
						return RSSL_FALSE;
					}
					break;

					/*For an example of array decoding, see fieldListEncDec.c*/
				case RSSL_DT_ARRAY:
					break;
				case RSSL_DT_BUFFER:
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_RMTES_STRING:
					{

						if((ret = rsslDecodeBuffer(pIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
						{
							RsslBuffer msgTypeBuf;
							msgTypeBuf.length = 1;
							msgTypeBuf.data = (char*)"D";

							printf("%.*s\n", fidBufferValue.length, fidBufferValue.data);

							/* Acknowledge orders. */
							if (fieldEntry.fieldId == 35 &&
									rsslBufferIsEqual(&msgTypeBuf, &fidBufferValue))
								sendAck = RSSL_TRUE;
						}
						else if (ret != RSSL_RET_BLANK_DATA) 
						{
							printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
							return RSSL_FALSE;
						}
						break;
					}
				default:
					printf("Unsupported data type (%d) for fid value\n", dataType);
					break;
			}
		}
		if (ret == RSSL_RET_BLANK_DATA)
		{
			printf("<blank data>\n");
		}

	}

	printf("\n");

	return sendAck;
}

RsslReactorCallbackRet queueMsgHandlerDefaultMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	RsslMsg *pRsslMsg = pEvent->pRsslMsg;

	printf("Received unhandled message in TunnelStream with stream ID %d, class %u(%s) and domainType %u(%s)\n\n",
		pRsslMsg->msgBase.streamId, 
		pRsslMsg->msgBase.msgClass, rsslMsgClassToString(pRsslMsg->msgBase.msgClass),
		pRsslMsg->msgBase.domainType, rsslDomainTypeToString(pRsslMsg->msgBase.domainType));

	return RSSL_RC_CRET_SUCCESS; 
}

/*
 * Encode and send a queue message ACK through the ReactorChannel.
 * This message will contain an field list as its payload.
 */
static void ackQueueMsg(QueueMsgHandler *pQueueMsgHandler, RsslBuffer *pDestName, RsslInt32 streamId)
{
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslEnum enumVal;
	RsslUInt uIntVal;
	RsslReal realVal;
	RsslTunnelStreamSubmitMsgOptions submitMsgOpts;
	RsslRDMQueueData queueData;
	RsslEncodeIterator eIter;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslBuffer fieldBuffer;
	RsslBuffer ackBuffer;
	char ackBufferMemory[1024];
	RsslTunnelStream *pTunnelStream = pQueueMsgHandler->tunnelStreamHandler.pTunnelStream;

	ackBuffer.data = ackBufferMemory;
	ackBuffer.length = sizeof(ackBufferMemory);

    // encode content into ackBuffer
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelStream->classOfService.common.protocolMajorVersion, 
			pTunnelStream->classOfService.common.protocolMajorVersion);
	rsslSetEncodeIteratorBuffer(&eIter, &ackBuffer);

	//// Start Content Encoding ////
	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(&eIter, &fieldList, NULL, 0)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit(): Failed <%d>\n", ret);
		return;
	}

	/* MsgType */
	rsslClearFieldEntry(&fieldEntry);
	rsslClearBuffer(&fieldBuffer);
	fieldEntry.fieldId = 35;
	fieldEntry.dataType = RSSL_DT_BUFFER;
	fieldBuffer.length = 1;
	fieldBuffer.data = (char*)"8";  // 8 for execution report
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* OrderId */
	rsslClearFieldEntry(&fieldEntry);
	rsslClearBuffer(&fieldBuffer);
	fieldEntry.fieldId = 37;
	fieldEntry.dataType = RSSL_DT_BUFFER;
	fieldBuffer.length = 18;
	fieldBuffer.data = (char*)"BATS-3456789-98765";
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* ClOrderId */
	rsslClearFieldEntry(&fieldEntry);
	rsslClearBuffer(&fieldBuffer);
	fieldEntry.fieldId = 11;
	fieldEntry.dataType = RSSL_DT_BUFFER;
	fieldBuffer.length = 12;
	fieldBuffer.data = (char*)"100000020998";
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* ExecID */
	rsslClearFieldEntry(&fieldEntry);
	rsslClearBuffer(&fieldBuffer);
	fieldEntry.fieldId = 17;
	fieldEntry.dataType = RSSL_DT_BUFFER;
	fieldBuffer.length = 10;
	fieldBuffer.data = (char*)"7654689076";
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* ExecType */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 150;
	fieldEntry.dataType = RSSL_DT_ENUM;
	enumVal = 0;  // 0 for New, 1 for partial fill, 2 for fill, 3 for Done
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* Order Status */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 39;
	fieldEntry.dataType = RSSL_DT_ENUM;
	enumVal = 0;  // 0 for new, 1 for partial fill, 2 for fill, 3 for done
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}
	
	/* Side */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 54;
	fieldEntry.dataType = RSSL_DT_ENUM;
	enumVal = 1;  // 1 for buy
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}
	
	/* LeavesQty */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 151;
	fieldEntry.dataType = RSSL_DT_UINT;
	uIntVal = 1000;  
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &uIntVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}
	
	/* CumQty */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 14;
	fieldEntry.dataType = RSSL_DT_UINT;
	uIntVal = 0;  
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &uIntVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}

	/* AvgPx */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 6;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&realVal);
	realVal.value = 0;  
	realVal.hint = RSSL_RH_EXPONENT0;
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &realVal)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}
	
	/* Symbol */
	rsslClearFieldEntry(&fieldEntry);
	rsslClearBuffer(&fieldBuffer);
	fieldEntry.fieldId = 55;
	fieldEntry.dataType = RSSL_DT_BUFFER;
	fieldBuffer.length = 3;
	fieldBuffer.data = (char*)"TRI";
	if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
		return;
	}
	
	if ((ret = rsslEncodeFieldListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete(): Failed <%d>\n", ret);
		return;
	}
    //// End Content Encoding ////

	// set ackBuffer encoded length
	ackBuffer.length = rsslGetEncodedBufferLength(&eIter);

    // initialize the QueueData with the ackBuffer set as the encodedDataBody
	rsslClearRDMQueueData(&queueData);
	queueData.rdmMsgBase.streamId = streamId;
	queueData.identifier = ++pQueueMsgHandler->identifier;
	queueData.rdmMsgBase.domainType = QUEUE_MSG_DOMAIN;
	queueData.sourceName = pQueueMsgHandler->sourceName;
	queueData.destName = *pDestName;
	queueData.timeout = RDM_QMSG_TC_INFINITE;
	queueData.containerType = RSSL_DT_FIELD_LIST;
	queueData.encDataBody = ackBuffer;

	// submit QueueData message with the encodedDataBody to the tunnel stream
	rsslClearTunnelStreamSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRDMMsg = (RsslRDMMsg *)&queueData;
	if ((ret = rsslTunnelStreamSubmitMsg(pQueueMsgHandler->tunnelStreamHandler.pTunnelStream, &submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslTunnelStreamSubmitMsg(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return;
	}

	printf("Submitted Exec Report message with ID %lld to %.*s.\n\n", pQueueMsgHandler->identifier, pDestName->length, pDestName->data);
}

void handleQueueMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, QueueMsgHandler *pQueueMsgHandler)
{
	time_t currentTime = 0;
	RsslTunnelStream *pTunnelStream = pQueueMsgHandler->tunnelStreamHandler.pTunnelStream;

	handleTunnelStreamHandler(pReactor, pReactorChannel, &pQueueMsgHandler->tunnelStreamHandler);

	if (pTunnelStream == NULL)
		return;

	/* Don't try to send messages if tunnel is not established. */
	if (pTunnelStream->state.streamState != RSSL_STREAM_OPEN
			|| pTunnelStream->state.dataState != RSSL_DATA_OK)
		return;

	/* If tunnel is open and some time has passed, send a message. */
	time(&currentTime);
	if (currentTime >= pQueueMsgHandler->nextMsgTime && pQueueMsgHandler->isQueueStreamOpen)
	{
		sendQueueMsg(pQueueMsgHandler);
		pQueueMsgHandler->nextMsgTime = currentTime + QUEUE_MSG_FREQUENCY;
	}
}

void queueMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream)
{
	TunnelStreamHandler *pTunnelHandler = (TunnelStreamHandler*)pTunnelStream->userSpecPtr;
	QueueMsgHandler *pQueueMsgHandler = (QueueMsgHandler*)pTunnelStream->userSpecPtr;
	RsslTunnelStreamSubmitMsgOptions tunnelStreamSubmitOptions;
	RsslRDMQueueRequest queueRequest;
	RsslRet ret;
	RsslErrorInfo errorInfo;

	pQueueMsgHandler->identifier = 0;
	pQueueMsgHandler->isQueueStreamOpen = RSSL_FALSE;

	// open a queue message sub-stream for this tunnel stream
	rsslClearRDMQueueRequest(&queueRequest);
	queueRequest.rdmMsgBase.streamId = QUEUE_MSG_STREAM_ID;
	queueRequest.rdmMsgBase.domainType = QUEUE_MSG_DOMAIN;
	queueRequest.sourceName =  pQueueMsgHandler->sourceName;

	rsslClearTunnelStreamSubmitMsgOptions(&tunnelStreamSubmitOptions);
	tunnelStreamSubmitOptions.pRDMMsg = (RsslRDMMsg *)&queueRequest;
	if ((ret = rsslTunnelStreamSubmitMsg(pTunnelStream, &tunnelStreamSubmitOptions, &errorInfo)) != RSSL_RET_SUCCESS)
		printf("rsslTunnelStreamSubmitMsg failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);

}

void queueMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream)
{
	QueueMsgHandler *pQueueMsgHandler = (QueueMsgHandler*)pTunnelStream->userSpecPtr;
	tunnelStreamHandlerClear(&pQueueMsgHandler->tunnelStreamHandler);
}


/*
 * Encode and send a queue message through the ReactorChannel.
 * This message will contain an field list as its payload.
 */
static void sendQueueMsg(QueueMsgHandler *pQueueMsgHandler)
{
	int i;
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret, ret2;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	for(i = 0; i < pQueueMsgHandler->destNameCount; i++)
	{
		RsslBuffer *pBuffer;
		RsslTunnelStreamSubmitOptions submitOpts;
		RsslTunnelStreamGetBufferOptions bufferOpts;
		RsslRDMQueueData queueData;
		RsslEncodeIterator eIter;
		RsslBuffer fieldBuffer;
		RsslDateTime dateTime;
		RsslReal real;
		RsslEnum enumVal;
		RsslUInt uintVal;
		RsslTunnelStream *pTunnelStream = pQueueMsgHandler->tunnelStreamHandler.pTunnelStream;

		rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
		bufferOpts.size = 1024;
		if ((pBuffer = rsslTunnelStreamGetBuffer(pQueueMsgHandler->tunnelStreamHandler.pTunnelStream, 
						&bufferOpts, &rsslErrorInfo)) == NULL)
		{
			printf("rsslTunnelStreamGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			return;
		}

        // initialize the QueueData encoding
		rsslClearRDMQueueData(&queueData);
		queueData.rdmMsgBase.streamId = QUEUE_MSG_STREAM_ID;
		queueData.identifier = ++pQueueMsgHandler->identifier;
		queueData.rdmMsgBase.domainType = QUEUE_MSG_DOMAIN;
		queueData.sourceName = pQueueMsgHandler->sourceName;
		queueData.destName = pQueueMsgHandler->destNames[i];
		queueData.timeout = RDM_QMSG_TC_INFINITE;
		queueData.containerType = RSSL_DT_FIELD_LIST;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelStream->classOfService.common.protocolMajorVersion, 
				pTunnelStream->classOfService.common.protocolMinorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

		if ((ret = rsslEncodeRDMQueueMsgInit(&eIter, (RsslRDMQueueMsg*)&queueData, &rsslErrorInfo)) != RSSL_RET_ENCODE_CONTAINER)
		{
			printf("rsslEncodeRDMQueueMsgInit(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		//// Start Content Encoding ////
		rsslClearFieldList(&fieldList);
		fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(&eIter, &fieldList, NULL, 0)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* MsgType */
		rsslClearFieldEntry(&fieldEntry);
		rsslClearBuffer(&fieldBuffer);
		fieldEntry.fieldId = 35;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		fieldBuffer.length = 1;
		fieldBuffer.data = (char*)"D";  // D for new single order
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* ClOrderId */
		rsslClearFieldEntry(&fieldEntry);
		rsslClearBuffer(&fieldBuffer);
		fieldEntry.fieldId = 11;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		fieldBuffer.length = 12;
		fieldBuffer.data = (char*)"100000020998";
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* Account */
		rsslClearFieldEntry(&fieldEntry);
		rsslClearBuffer(&fieldBuffer);
		fieldEntry.fieldId = 1;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		fieldBuffer.length = 10;
		fieldBuffer.data = (char*)"D6789-3456";
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* Handle instruction */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 21;
		fieldEntry.dataType = RSSL_DT_ENUM;
		
		enumVal = 1;  // 1 = automated, 2 = semi automated, 3 = manual
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* Symbol */
		rsslClearFieldEntry(&fieldEntry);
		rsslClearBuffer(&fieldBuffer);
		fieldEntry.fieldId = 55;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		fieldBuffer.length = 3;
		fieldBuffer.data = (char*)"TRI";
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &fieldBuffer)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* Side */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 54;
		fieldEntry.dataType = RSSL_DT_ENUM;
		
		enumVal = 1;  // 1 for Buy
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* TransactTime */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 60;
		fieldEntry.dataType = RSSL_DT_DATETIME;

		if ((ret = rsslDateTimeGmtTime(&dateTime)) != RSSL_RET_SUCCESS)
		{
			printf("rsslDateTimeGmtTime(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &dateTime)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* OrderQty */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 38;
		fieldEntry.dataType = RSSL_DT_UINT;
		uintVal = 1000;
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &uintVal)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		/* OrderType */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 40;
		fieldEntry.dataType = RSSL_DT_ENUM;		
		enumVal = 1;  // 2 for Limit Order
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &enumVal)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}
		
		
		/* Price */
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = 44;
		fieldEntry.dataType = RSSL_DT_REAL;		
		rsslClearReal(&real);
		real.value = 3835;    //38.35
		real.hint = RSSL_RH_EXPONENT_2;
		if ((ret = rsslEncodeFieldEntry(&eIter, &fieldEntry, &real)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}


		if ((ret = rsslEncodeFieldListComplete(&eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete(): Failed <%d>\n", ret);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}
        //// End Content Encoding ////

		// complete the QueueData encoding
		if ((ret = rsslEncodeRDMQueueMsgComplete(&eIter, RSSL_TRUE, &pBuffer->length, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMQueueMsgComplete(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		// submit the encoded data buffer of QueueData to the tunnel stream
		rsslClearTunnelStreamSubmitOptions(&submitOpts);
		submitOpts.containerType = RSSL_DT_MSG;
		if ((ret = rsslTunnelStreamSubmit(pQueueMsgHandler->tunnelStreamHandler.pTunnelStream, pBuffer, &submitOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslTunnelStreamSubmit(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, rsslErrorInfo.rsslError.text);
			return;
		}

		printf("Submitted Single Order message with ID %lld to %.*s.\n\n", pQueueMsgHandler->identifier, pQueueMsgHandler->destNames[i].length, pQueueMsgHandler->destNames[i].data);
	}
}

void queueMsgHandlerCloseStreams(QueueMsgHandler *pQueueMsgHandler)
{
	tunnelStreamHandlerCloseStreams(&pQueueMsgHandler->tunnelStreamHandler);
}
