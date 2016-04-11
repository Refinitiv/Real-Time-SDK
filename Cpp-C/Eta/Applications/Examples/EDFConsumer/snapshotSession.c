/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "snapshotSession.h"
#include "itemList.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rtr/rsslRDMLoginMsg.h"
#include <stdlib.h>

/* Requests the symbol list from the ref data server. */
static RsslRet refDataSessionSendSymbolListRequest(SnapshotSession *pSession);

/* Processes a symbol list response. */
static RsslRet processSymbolListResponse(SnapshotSession* pSession, RsslMsg* msg,
								  RsslDecodeIterator* dIter);


/* Decodes messages received from the snapshot server. */
static RsslRet snapshotSessionProcessMessage(SnapshotSession *pSession, RsslBuffer *pBuffer);

/* Writes messages. */
static RsslRet snapshotSessionWrite(SnapshotSession *pSession, RsslBuffer *pBuffer);

static RsslRet snapshotSessionSendSymbolListRequest(SnapshotSession *pSession);

static RsslRet encodeSymbolListRequest(SnapshotSession* pSession, RsslBuffer* msgBuf, RsslInt32 streamId);

/* ID to use for the login stream. */
static const RsslInt32 LOGIN_STREAM_ID = 1;
static const RsslInt32 SYMBOL_LIST_STREAM_ID = 2;
RsslInt32 SymbolListCounter = 0;

RsslRet snapshotSessionConnect(SnapshotSession *pSession, RsslConnectOptions *pOpts, RsslRDMService* service)
{
	RsslError rsslError;

	pSession->pRsslChannel = rsslConnect(pOpts, &rsslError);

	if (!pSession->pRsslChannel)
	{
		printf("<%s> rsslConnect() failed: %d (%s -- %s)\n\n", pSession->name,
				rsslError.rsslErrorId, rsslRetCodeToString(rsslError.rsslErrorId), rsslError.text);
		return rsslError.rsslErrorId;
	}
	
	printf("<%s> Channel connected with FD %d.\n\n", pSession->name, pSession->pRsslChannel->socketId);

	/* If we need to initialize the channel, make our first call to rsslInitChannel() when
	 * triggered to write. */
	if (pSession->pRsslChannel->state == RSSL_CH_STATE_INITIALIZING)
		pSession->setWriteFd = RSSL_TRUE;
	else
	{
		printf("<%s> Channel is active!\n\n", pSession->name);
		pSession->setWriteFd = RSSL_FALSE;
		if (snapshotSessionProcessChannelActive(pSession) != RSSL_RET_SUCCESS)
			exit(-1);
	}

	pSession->pService = service;

	return RSSL_RET_SUCCESS;

}

RsslRet snapshotSessionInitializeChannel(SnapshotSession *pSession)
{
	RsslInProgInfo inProg;
	RsslRet ret;
	RsslError rsslError;

	ret = rsslInitChannel(pSession->pRsslChannel, &inProg, &rsslError);

	switch(ret)
	{
		case RSSL_RET_SUCCESS: 
			/* Success indicates that initialization is complete. */
			pSession->setWriteFd = RSSL_FALSE;
			printf("<%s> Channel is active!\n\n", pSession->name);
			if ((ret = snapshotSessionProcessChannelActive(pSession)) != RSSL_RET_SUCCESS)
				return ret;
			return RSSL_RET_SUCCESS;

		case RSSL_RET_CHAN_INIT_IN_PROGRESS:

			if (inProg.flags & RSSL_IP_FD_CHANGE)
			{
				/* Indicates that the file descriptor for this channel changed. */
				printf("<%s> Channel FD changed while initializing (from %d to %d)\n\n", pSession->name,
						inProg.oldSocket, inProg.newSocket);

				/* This normally means that the channel was reconnected while initializing,
				 * so trigger ourselves to call rsslInitChannel on write notification
				 * again.  */
				pSession->setWriteFd = RSSL_TRUE;
			}
			else
				pSession->setWriteFd = RSSL_FALSE;

			return RSSL_RET_SUCCESS;

		default:
			printf("<%s> rsslInitChannel() failed: %d(%s).\n\n",
					pSession->name, ret, rsslRetCodeToString(ret));
			return ret;
	}
}

RsslRet snapshotSessionProcessChannelActive(SnapshotSession *pSession)
{
	RsslEncodeIterator encodeIter;
	RsslRDMLoginRequest loginRequest;
	RsslBuffer *pBuffer;
	RsslErrorInfo rsslErrorInfo;
	RsslError rsslError;
	RsslRet ret = RSSL_RET_FAILURE;


	/* Get a buffer from the channel for writing. */
	if (!(pBuffer = rsslGetBuffer(pSession->pRsslChannel, 1024, RSSL_FALSE, &rsslError)))
	{
		printf("<%s> rsslGetBuffer() failed while sending login request: %d (%s -- %s).\n\n",
				pSession->name,
				rsslError.rsslErrorId, rsslRetCodeToString(rsslError.rsslErrorId), rsslError.text);
		return rsslError.rsslErrorId;
	}

	/* Populate the login request with some default information. */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, LOGIN_STREAM_ID)) != RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslInitDefaultRDMLoginRequest() failed: %d(%s).\n\n",
				pSession->name, ret, rsslRetCodeToString(ret));
		rsslReleaseBuffer(pBuffer, &rsslError);
		return ret;
	}

	/* Encode the login request using the RDM package encoder utility. This will
	 * translate the login request structure to an encoded message and set the proper length
	 * on the buffer. */
	rsslClearEncodeIterator(&encodeIter);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pSession->pRsslChannel->majorVersion,
			pSession->pRsslChannel->minorVersion);
	rsslSetEncodeIteratorBuffer(&encodeIter, pBuffer);
	if ((ret = rsslEncodeRDMLoginMsg(&encodeIter, (RsslRDMLoginMsg*)&loginRequest, &pBuffer->length,
					&rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeRDMLoginMsg() failed: %d (%s -- %s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret), rsslErrorInfo.rsslError.text);
		rsslReleaseBuffer(pBuffer, &rsslError);
		return ret;
	}

	/* Write the message. */
	if ((ret = snapshotSessionWrite(pSession, pBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	pSession->state = SNAPSHOT_STATE_LOGIN_REQUESTED;

	return RSSL_RET_SUCCESS;

}

RsslRet snapshotSessionProcessReadReady(SnapshotSession *pSession)
{
	RsslRet readRet;
	RsslRet ret = RSSL_RET_FAILURE;
	RsslError rsslError;


	RsslBuffer *pBuffer;
	RsslReadInArgs readInArgs;
	RsslReadOutArgs readOutArgs;

	switch(pSession->pRsslChannel->state)
	{
		case RSSL_CH_STATE_ACTIVE:

			rsslClearReadInArgs(&readInArgs);
			rsslClearReadOutArgs(&readOutArgs);

			pBuffer = rsslReadEx(pSession->pRsslChannel, &readInArgs, &readOutArgs, &readRet, 
					&rsslError);

			if (pBuffer)
			{
				if ((ret = snapshotSessionProcessMessage(pSession, pBuffer)) != RSSL_RET_SUCCESS)
					return ret;
			}

			if (readRet >= RSSL_RET_SUCCESS)
				return readRet;

			/* Read values less than RSSL_RET_SUCCESS are codes to handle. */
			switch (readRet)
			{
				case RSSL_RET_READ_FD_CHANGE:
					/* Channel's file descriptor was changed. */
					printf("<%s> Channel FD changed while reading (from %d to %d).\n\n",
							pSession->name, pSession->pRsslChannel->oldSocketId,
							pSession->pRsslChannel->socketId);
					return RSSL_RET_SUCCESS;

				case RSSL_RET_READ_PING:
					/* Received a ping message. */
					printf("<%s> Received ping.\n\n", pSession->name);
					return RSSL_RET_SUCCESS;

				case RSSL_RET_READ_WOULD_BLOCK:
					/* Nothing to read. */
					return RSSL_RET_SUCCESS;

				case RSSL_RET_FAILURE:
				default:
					/* Channel failed or unhandled code; close the channel. */
					printf("<%s> rsslReadEx() failed: %d(%s -- %s).\n\n", pSession->name,
							readRet, rsslRetCodeToString(readRet), rsslError.text);
					rsslCloseChannel(pSession->pRsslChannel, &rsslError);
					pSession->pRsslChannel = NULL;
					return readRet;
			}

		case RSSL_CH_STATE_INITIALIZING:
			return snapshotSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;


	}

}

static RsslRet snapshotSessionProcessMessage(SnapshotSession *pSession, RsslBuffer *pBuffer)
{
	RsslDecodeIterator decodeIter;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslMsg rsslMsg;
	char tempMem[1024];
	RsslBuffer tempBuffer;
	Item *pItem;

	printf("<%s> Received message:\n", pSession->name);

	/* Decode the message header. */
	rsslClearDecodeIterator(&decodeIter);
	rsslSetDecodeIteratorRWFVersion(&decodeIter, pSession->pRsslChannel->majorVersion,
			pSession->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&decodeIter, pBuffer);
	if ((ret = rsslDecodeMsg(&decodeIter, &rsslMsg)) != RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslDecodeMsg() failed: %d(%s).\n\n", 
				pSession->name, ret, rsslRetCodeToString(ret));
		return ret;
	}

	switch(rsslMsg.msgBase.domainType)
	{
		case RSSL_DMT_LOGIN:
		{
			/* Decode the message using the RDM package decoder utility. */
			RsslRDMLoginMsg loginMsg;

			tempBuffer.data = tempMem;
			tempBuffer.length = sizeof(tempMem);

			if ((ret = rsslDecodeRDMLoginMsg(&decodeIter, &rsslMsg, &loginMsg, &tempBuffer, 
							&rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf(	"<%s> rsslDecodeRDMLoginMsg() failed: %d (%s -- %s)\n"
						"  at %s.\n\n",
						pSession->name,
						ret, rsslRetCodeToString(ret), rsslErrorInfo.rsslError.text,
						rsslErrorInfo.errorLocation);
				return ret;
			}

			switch(loginMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_LG_MT_REFRESH:
					printf("<%s> Received login refresh.  %u (%s) Error text: %s \n\n",
							pSession->name,
							ret, rsslRetCodeToString(ret), rsslErrorInfo.rsslError.text);

					if (pSession->state == SNAPSHOT_STATE_LOGIN_REQUESTED)
					{
						if (snapshotSessionSendSymbolListRequest(pSession) != RSSL_RET_SUCCESS)
							return ret;
					}

					break;

				default:
					printf("<%s> Received unhandled RDM Login Message Type %d.\n\n",
							pSession->name, loginMsg.rdmMsgBase.rdmMsgType);
					return RSSL_RET_FAILURE;
			}
			return RSSL_RET_SUCCESS;
		}

		case RSSL_DMT_SYMBOL_LIST:
		{
			processSymbolListResponse(pSession, &rsslMsg, &decodeIter);

			if (rsslMsg.refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				pSession->state = SNAPSHOT_STATE_SYMBOL_LIST_RECEIVED;
			}
		}
		break;

		default:
		{
			if ((pItem = itemListGetItemBySnapStreamID(&itemList, rsslMsg.msgBase.streamId, rsslMsg.msgBase.domainType))
					== NULL)
			{
				printf("<%s> Received unhandled message with snapshot stream ID %d, and domain %d(%s).\n\n",
						pSession->name,
						rsslMsg.msgBase.streamId,
						rsslMsg.msgBase.domainType,
						rsslDomainTypeToString(rsslMsg.msgBase.domainType));

				return RSSL_RET_SUCCESS;
			}

			return itemProcessMsg(pItem, &decodeIter, &rsslMsg, RSSL_TRUE);
		}

	}

	return RSSL_RET_SUCCESS;
}

RsslRet snapshotSessionRequestItems(SnapshotSession *pSession)
{
	RsslUInt32 i;
	RsslEncodeIterator encodeIter;
	RsslRequestMsg requestMsg;
	RsslBuffer *pBuffer;
	RsslError rsslError;
	RsslRet ret;

	for (i = 0; i < itemList.itemCount; ++i)
	{
		Item *pItem = &itemList.items[i];

		if (!(pBuffer = rsslGetBuffer(pSession->pRsslChannel, 128, RSSL_FALSE, &rsslError)))
		{
			printf("<%s> rsslGetBuffer() failed while sending item request: %d (%s -- %s).\n\n",
					pSession->name,
					rsslError.rsslErrorId, rsslRetCodeToString(rsslError.rsslErrorId), rsslError.text);
			return ret;
		}

		rsslClearRequestMsg(&requestMsg);
		requestMsg.flags = RSSL_RQMF_HAS_QOS;
		requestMsg.msgBase.streamId = pItem->snapshotServerStreamId;
		requestMsg.msgBase.domainType = pItem->domainType;
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;

		/* This system uses the real-time feed's stream ID as the name. */
		requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_IDENTIFIER;
		requestMsg.msgBase.msgKey.identifier = pItem->feedStreamId;
		requestMsg.msgBase.msgKey.serviceId = exampleConfig.serviceId;

		rsslClearEncodeIterator(&encodeIter);
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pSession->pRsslChannel->majorVersion,
				pSession->pRsslChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&encodeIter, pBuffer);
		if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&requestMsg)) != RSSL_RET_SUCCESS)
		{
			printf("<%s> rsslEncodeMsg() failed while sending item request: %d (%s).\n\n",
					pSession->name,
					ret, rsslRetCodeToString(ret));
			rsslReleaseBuffer(pBuffer, &rsslError);
			return ret;
		}

		pBuffer->length = rsslGetEncodedBufferLength(&encodeIter);

		/* Write the message. */
		if ((ret = snapshotSessionWrite(pSession, pBuffer)) != RSSL_RET_SUCCESS)
			return ret;

		printf("<%s> Sent request for item %s, %s on stream %d.\n\n",
				pSession->name, pItem->symbolName,
				rsslDomainTypeToString(pItem->domainType), pItem->snapshotServerStreamId);

	}

	return RSSL_RET_SUCCESS;
}

static RsslRet snapshotSessionWrite(SnapshotSession *pSession, RsslBuffer *pBuffer)
{
	RsslError rsslError;
	RsslRet ret;

	RsslWriteInArgs writeInArgs;
	RsslWriteOutArgs writeOutArgs;

	/* Write the message. */
	rsslClearWriteInArgs(&writeInArgs);
	rsslClearWriteOutArgs(&writeOutArgs);
	if ((ret = rsslWriteEx(pSession->pRsslChannel, pBuffer, &writeInArgs, &writeOutArgs,
					&rsslError)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslWriteEx() failed: %d(%s -- %s).\n\n",
				pSession->name, ret, rsslRetCodeToString(ret), rsslError.text);
		rsslReleaseBuffer(pBuffer, &rsslError);
		rsslCloseChannel(pSession->pRsslChannel, &rsslError);
		pSession->pRsslChannel = NULL;
		return ret;
	}

	/* Positive values indicate that data is in the outbound queue and should be flushed. */
	if (ret > RSSL_RET_SUCCESS)
		pSession->setWriteFd = RSSL_TRUE;

	return RSSL_RET_SUCCESS;

}

RsslRet snapshotSessionProcessWriteReady(SnapshotSession *pSession)
{
	RsslRet ret = RSSL_RET_FAILURE;
	RsslError rsslError;

	switch(pSession->pRsslChannel->state)
	{
		case RSSL_CH_STATE_ACTIVE:
			ret = rsslFlush(pSession->pRsslChannel, &rsslError);

			/* Read values less than RSSL_RET_SUCCESS are codes to handle. */
			if (ret < RSSL_RET_SUCCESS)
			{
				/* Flush failed; close the channel. */
				printf("<%s> rsslFlush() failed: %d(%s).\n\n", pSession->name,
						ret, rsslRetCodeToString(ret));
				rsslCloseChannel(pSession->pRsslChannel, &rsslError);
				pSession->pRsslChannel = NULL;
				return ret;
			}
			else if (ret == 0)
				pSession->setWriteFd = RSSL_FALSE; /* Done flushing. */

			return RSSL_RET_SUCCESS;

		case RSSL_CH_STATE_INITIALIZING:
			return snapshotSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;
	}
}

static RsslRet processSymbolListResponse(SnapshotSession* pSession, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;

	RsslMsgKey* key = 0;
	RsslMap rsslMap;
	RsslMapEntry mapEntry;
	RsslVector rsslVector;
	RsslVectorEntry vectorEntry;
	char tempData[1024];
	char mapKeyData[32];
	RsslBuffer tempBuffer;
	RsslBuffer fidBufferValue;
	RsslLocalFieldSetDefDb fieldSetDefDb;
	RsslLocalElementSetDefDb elemSetDefDb;
	RsslElementList elemList;
	RsslElementEntry elemEntry;
	RsslFieldEntry fieldEntry;
	RsslFieldList fieldList;
	RsslBuffer stringBuf;
	char data[24];

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:

		printf("\n<%s> Received symbol list refresh.\n\n", pSession->name);
	case RSSL_MC_UPDATE:

		/* decode symbol list response */
		/* get key*/
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		if(msg->msgBase.msgClass == RSSL_MC_REFRESH)
		{
			rsslStateToString(&tempBuffer, &msg->refreshMsg.state);
			printf("%.*s\n", tempBuffer.length, tempBuffer.data);
		}

		rsslClearMap(&rsslMap);

		if ((ret = rsslDecodeMap(dIter, &rsslMap)) != RSSL_RET_SUCCESS)
		{
			printf("<%s> rsslDecodeMap() failed with return code: %d\n", pSession->name, ret);
			return RSSL_RET_FAILURE;
		}

		if (rsslMap.flags & RSSL_MPF_HAS_SET_DEFS)
		{
			/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
			if (rsslMap.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDefDb);
				if ((ret = rsslDecodeLocalFieldSetDefDb(dIter, &fieldSetDefDb)) < RSSL_RET_SUCCESS)
				{
					/* decoding failures tend to be unrecoverable */
					printf("<%s> Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
						pSession->name,
						rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
					return ret;
				}
			}
		}

		/* decode the map */
		while ((ret = rsslDecodeMapEntry(dIter, &mapEntry, mapKeyData)) != RSSL_RET_END_OF_CONTAINER)
		{
			if(ret != RSSL_RET_SUCCESS)
			{
				printf("<%s> rsslDecodeMapEntry() failed with return code: %d\n", pSession->name, ret);
				return RSSL_RET_FAILURE;
			}

			stringBuf.data = data;
			stringBuf.length = 24;
			rsslPrimitiveToString(&mapKeyData,rsslMap.keyPrimitiveType,&stringBuf);

			printf("\nID: %s, ", stringBuf.data);
			pSession->symbolListEntry[SymbolListCounter] = (SymbolListEntry*)malloc(sizeof(SymbolListEntry));
			pSession->symbolListEntry[SymbolListCounter]->id = atoi(stringBuf.data);

			if ((ret = rsslDecodeFieldList(dIter, &fieldList, &fieldSetDefDb)) != RSSL_RET_SUCCESS)
			{
				printf("<%s> rsslDecodeMap() failed with return code: %d\n", pSession->name, ret);
				return RSSL_RET_FAILURE;
			}


			/* The following fields are needed to uniquely identify a symbol on the realtime and gap fill streams:
			FID		Name						Type
			3422	Provider Symbol				RMTES_STRING
			8746	Provider Symbol 2			RMTES_STRING
			32639	Multicast channel(RT)		Vector of Element Lists
			32640	Multicast Channel(Gapfill)	Vector of Element Lists
			*/


			while ((ret = rsslDecodeFieldEntry(dIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret != RSSL_RET_SUCCESS)
				{
					printf("<%s> rsslDecodeFieldEntry() failed with return code: %d\n", pSession->name, ret);
					return RSSL_RET_FAILURE;
				}

				if(fieldEntry.fieldId == 8746)
				{
					if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
					{
						//snprintf(&pSession->symbolListEntry[SymbolListCounter]->name[0], 128, "%.*s", fidBufferValue.length, fidBufferValue.data);
						printf("SYMBOL2: %s", fidBufferValue.data);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("<%s> Error: %s (%d) encountered with rsslDecodeBuffer(). Error Text: %s\n",
							pSession->name,
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}
				}
				else if (fieldEntry.fieldId == 3422)
				{
					if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
					{
						snprintf(&pSession->symbolListEntry[SymbolListCounter]->name[0], 128, "%.*s", fidBufferValue.length, fidBufferValue.data);
						printf("SYMBOL: %s", pSession->symbolListEntry[SymbolListCounter]->name);
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("<%s> Error: %s (%d) encountered with rsslDecodeBuffer(). Error Text: %s\n",
							pSession->name,
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}
				}
				else if (fieldEntry.fieldId == 32639)
				{

					int countStreamingChan = 0;

					rsslClearVector(&rsslVector);
					if ((ret = rsslDecodeVector(dIter, &rsslVector)) != RSSL_RET_SUCCESS)
					{
						printf("<%s> rsslDecodeVector() failed with return code: %d\n", pSession->name, ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslVector.flags & RSSL_VTF_HAS_SET_DEFS)
					{
						/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
						if (rsslVector.containerType == RSSL_DT_ELEMENT_LIST)
						{
							rsslClearLocalElementSetDefDb(&elemSetDefDb);
							if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elemSetDefDb)) < RSSL_RET_SUCCESS)
							{
								/* decoding failures tend to be unrecoverable */
								printf("<%s> Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									pSession->name,
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
								return ret;
							}
						}
					}

					/* decode the vector */
					while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
					{
						if(ret != RSSL_RET_SUCCESS)
						{
							printf("<%s> rsslDecodeVectorEntry() failed with return code: %d\n", pSession->name, ret);
							return RSSL_RET_FAILURE;
						}

						rsslClearElementList(&elemList);
						if ((ret = rsslDecodeElementList(dIter, &elemList, &elemSetDefDb)) != RSSL_RET_SUCCESS)
						{
							printf("<%s> rsslDecodeElementList() failed with return code: %d\n", pSession->name, ret);
							return RSSL_RET_FAILURE;
						}

						while ((ret = rsslDecodeElementEntry(dIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
						{
							if (ret != RSSL_RET_SUCCESS)
							{
								printf("<%s> rsslDecodeElementEntry() failed with return code: %d\n", pSession->name, ret);
								return RSSL_RET_FAILURE;
							}

							if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_CHANNEL_ID))
							{
								if ((ret = rsslDecodeUInt(dIter, &pSession->symbolListEntry[SymbolListCounter]->streamingChannels[countStreamingChan].channelId)) == RSSL_RET_SUCCESS)
								{	
									printf(" StreamingChanId: "RTR_LLU, pSession->symbolListEntry[SymbolListCounter]->streamingChannels[countStreamingChan].channelId);
								}
								else
								{
									printf("<%s> Error: %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
										pSession->name,
										rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
									return ret;
								}
							}
							else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_DOMAIN))
							{
								if ((ret = rsslDecodeUInt(dIter, &pSession->symbolListEntry[SymbolListCounter]->streamingChannels[countStreamingChan].domain)) == RSSL_RET_SUCCESS)
								{	
									printf(" StreamingChanDom: "RTR_LLU, pSession->symbolListEntry[SymbolListCounter]->streamingChannels[countStreamingChan].domain);
								}
								else
								{
									printf("<%s> Error: %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
										pSession->name,
										rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
									return ret;
								}
							}

						}
						countStreamingChan++;

					}
				}
				else if (fieldEntry.fieldId == 32640)
				{
					int countGapChan = 0;

					rsslClearVector(&rsslVector);
					if ((ret = rsslDecodeVector(dIter, &rsslVector)) != RSSL_RET_SUCCESS)
					{
						printf("<%s> rsslDecodeVector() failed with return code: %d\n", pSession->name, ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslVector.flags & RSSL_VTF_HAS_SET_DEFS)
					{
						/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
						if (rsslVector.containerType == RSSL_DT_ELEMENT_LIST)
						{
							rsslClearLocalElementSetDefDb(&elemSetDefDb);
							if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elemSetDefDb)) < RSSL_RET_SUCCESS)
							{
								/* decoding failures tend to be unrecoverable */
								printf("<%s> Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									pSession->name,
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
								return ret;
							}

						}
					}

					/* decode the vector */
					while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
					{
						if(ret != RSSL_RET_SUCCESS)
						{
							printf("<%s> rsslDecodeVectorEntry() failed with return code: %d\n", pSession->name, ret);
							return RSSL_RET_FAILURE;
						}

						rsslClearElementList(&elemList);
						if ((ret = rsslDecodeElementList(dIter, &elemList, &elemSetDefDb)) != RSSL_RET_SUCCESS)
						{
							printf("<%s> rsslDecodeElementList() failed with return code: %d\n", pSession->name, ret);
							return RSSL_RET_FAILURE;
						}

						while ((ret = rsslDecodeElementEntry(dIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
						{
							if (ret != RSSL_RET_SUCCESS)
							{
								printf("<%s> rsslDecodeElementEntry() failed with return code: %d\n", pSession->name, ret);
								return RSSL_RET_FAILURE;
							}
							if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_CHANNEL_ID))
							{
								if ((ret = rsslDecodeUInt(dIter, &pSession->symbolListEntry[SymbolListCounter]->gapChannels[countGapChan].channelId)) == RSSL_RET_SUCCESS)
								{	
									printf(" GapChanId: "RTR_LLU, pSession->symbolListEntry[SymbolListCounter]->gapChannels[countGapChan].channelId);
								}
								else
								{
									printf("<%s> Error: %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
										pSession->name,
										rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
									return ret;
								}
							}
							else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_DOMAIN))
							{
								if ((ret = rsslDecodeUInt(dIter, &pSession->symbolListEntry[SymbolListCounter]->gapChannels[countGapChan].domain)) == RSSL_RET_SUCCESS)
								{	
									printf(" GapChanlDom: "RTR_LLU, pSession->symbolListEntry[SymbolListCounter]->gapChannels[countGapChan].domain);
								}
								else
								{
									printf("<%s> Error: %s (%d) encountered with rsslDecodeUInt(). Error Text: %s\n",
										pSession->name,
										rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
									return ret;
								}
							}
						}
					}
					countGapChan++;
				}
			}
			SymbolListCounter++;
		}



		break;

	case RSSL_MC_STATUS:
		printf("\n<%s> Received Item StatusMsg for stream %i \n", pSession->name, msg->statusMsg.msgBase.streamId);
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
		{
			rsslStateToString(&tempBuffer, &msg->statusMsg.state);
			printf("        %.*s\n\n", tempBuffer.length, tempBuffer.data);
		}

		break;

	case RSSL_MC_ACK:
		/* although this application only posts on MP (Market Price), 
		* ACK handler is provided for other domains to allow user to extend 
		* and post on MBO (Market By Order), MBP (Market By Price), SymbolList, and Yield Curve domains */
		printf("\n<%s> Received AckMsg for stream %i \n", pSession->name, msg->msgBase.streamId);

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
		printf("\n<%s> Recieved Unhandled Item Msg Class: %d\n", pSession->name, msg->msgBase.msgClass);
		break;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet snapshotSessionSendSymbolListRequest(SnapshotSession *pSession)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the item request*/
	msgBuf = rsslGetBuffer(pSession->pRsslChannel, 128, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/*encode symbol list request*/
		if (encodeSymbolListRequest(pSession, msgBuf, SYMBOL_LIST_STREAM_ID) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\n<%s> Symbol List encodeSymbolListRequest() failed\n", pSession->name);
			return RSSL_RET_FAILURE;
		}

		if ((ret = snapshotSessionWrite(pSession, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;
	}
	else
	{
		printf("<%s> rsslGetBuffer(): Failed <%s>\n", pSession->name, error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet encodeSymbolListRequest(SnapshotSession* pSession, RsslBuffer* msgBuf, RsslInt32 streamId)
{                       
	RsslRet ret = 0;
	RsslRequestMsg msg;
	RsslEncodeIterator encodeIter;
	RsslQos	*pQos;

	/* clear encode iterator*/
	rsslClearEncodeIterator(&encodeIter);

	/*set-up message*/
	rsslClearRequestMsg(&msg);

	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_HAS_PRIORITY;

	msg.priorityClass = 1;
	msg.priorityCount = 1;

	/* Use a QoS from the service, if one is given. */
	if ((pSession->pService->flags & RDM_SVCF_HAS_INFO) && 
		(pSession->pService->info.flags & RDM_SVC_IFF_HAS_QOS) && 
		(pSession->pService->info.qosCount > 0))
	{
		pQos = &pSession->pService->info.qosList[0];
	}
	else
	{
		pQos = NULL;
	}

	if (pQos)
	{
		msg.flags |= RSSL_RQMF_HAS_QOS;
		msg.qos = *pQos;
	}	

	/* specify msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.serviceId = (RsslUInt16)(pSession->pService->serviceId);

	/* if the there was no symbol list name
	included in the source directory, set the name data to NULL */
	if ((pSession->pService->flags & RDM_SVCF_HAS_INFO) && 
		(pSession->pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST ) && 
		(pSession->pService->info.itemList.length > 0))
	{
		msg.msgBase.msgKey.name.length = pSession->pService->info.itemList.length;
		msg.msgBase.msgKey.name.data = pSession->pService->info.itemList.data;
	}
	else
	{
		msg.msgBase.msgKey.name.data = NULL;
	}

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeIteratorBuffer() failed with return code: %d\n", pSession->name, ret);
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion(&encodeIter, pSession->pRsslChannel->majorVersion, 
		pSession->pRsslChannel->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMsgInit() failed with return code: %d\n", pSession->name, ret);
		return ret;
	}

	if((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMessageComplete() failed with return code: %d\n", pSession->name, ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);
	return RSSL_RET_SUCCESS;
} 
