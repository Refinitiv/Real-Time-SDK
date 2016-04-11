/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "refDataSession.h"
#include "rtr/rsslRDMLoginMsg.h"
#include <stdlib.h>

RsslBool fieldDictionaryLoaded;
RsslBool enumDictionaryLoaded;
RsslBool globalSetDefDictionaryLoaded;

RsslDataDictionary	dictionary;

/* Decodes messages received from the ref data server. */
static RsslRet refDataSessionProcessMessage(RefDataSession *pSession, RsslBuffer *pBuffer);

/* Processes a dictionary message. */
static RsslRet processDictionaryResponse(RefDataSession *pSession, RsslMsg* msg,
		RsslDecodeIterator* dIter);


/* Writes messages. */
static RsslRet refDataSessionWrite(RefDataSession *pSession, RsslBuffer *pBuffer);

/* ID to use for the login stream. */
static const RsslInt32 LOGIN_STREAM_ID = 1;

static RsslInt32 fieldDictionaryStreamId = 0, enumDictionaryStreamId = 0, globalSetDefDictionaryStreamId = 0;


RsslRet refDataSessionConnect(RefDataSession *pSession, RsslConnectOptions *pOpts)
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
		if (refDataSessionProcessChannelActive(pSession) != RSSL_RET_SUCCESS)
			exit(-1);
	}

	return RSSL_RET_SUCCESS;

}

RsslRet refDataSessionInitializeChannel(RefDataSession *pSession)
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
			if ((ret = refDataSessionProcessChannelActive(pSession)) != RSSL_RET_SUCCESS)
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

RsslRet refDataSessionProcessChannelActive(RefDataSession *pSession)
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
	if ((ret = refDataSessionWrite(pSession, pBuffer)) != RSSL_RET_SUCCESS)
		return ret;

        pSession->state = REF_DATA_STATE_LOGIN_REQUESTED;

	return RSSL_RET_SUCCESS;

}

RsslRet refDataSessionProcessReadReady(RefDataSession *pSession)
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
				if ((ret = refDataSessionProcessMessage(pSession, pBuffer)) != RSSL_RET_SUCCESS)
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
			return refDataSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;
	}
}

static RsslRet refDataSessionProcessMessage (RefDataSession *pSession, RsslBuffer *pBuffer)
{
	RsslDecodeIterator decodeIter;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	RsslMsg rsslMsg;
	char tempMem[1024];
	RsslBuffer tempBuffer;

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
		case RSSL_DMT_DICTIONARY:
		{
			processDictionaryResponse(pSession, &rsslMsg, &decodeIter);
		}
		break;
		case RSSL_DMT_SOURCE:
		{
			char memoryChar[16384];
			RsslBuffer memoryBuffer = { 16384, memoryChar };
			RsslRDMDirectoryMsg directoryMsg;
			RsslRDMService *pMsgServiceList;
			RsslUInt32 msgServiceCount, iMsgServiceList;
			RsslBool foundService = RSSL_FALSE;
			

			printf("Received source directory response.\n\n");

			if ((ret = rsslDecodeRDMDirectoryMsg(&decodeIter, &rsslMsg, &directoryMsg, &memoryBuffer, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
			{
				printf(	"<%s> rsslDecodeRDMDirectoryMsg() failed: %d (%s -- %s)\n",
						pSession->name,
						ret, rsslRetCodeToString(ret), rsslErrorInfo.rsslError.text);
				return RSSL_RET_FAILURE;
			}

			// Copy the directory message.  If our desired service is present inside, we will want to keep it. 
			if ((ret = rsslCopyRDMDirectoryMsg(&pSession->directoryMsgCopy, &directoryMsg, &pSession->directoryMsgCopyMemory)) != RSSL_RET_SUCCESS)
			{
				printf( "<%s> rsslCopyRDMDirectoryMsg failed:  %d (%s -- %s)\n",
						pSession->name,
						ret, rsslRetCodeToString(ret), rsslErrorInfo.rsslError.text);
				return RSSL_RET_FAILURE;
			}			

			switch(pSession->directoryMsgCopy.rdmMsgBase.rdmMsgType)
			{
				case RDM_DR_MT_REFRESH:
					pMsgServiceList = pSession->directoryMsgCopy.refresh.serviceList;
					msgServiceCount = pSession->directoryMsgCopy.refresh.serviceCount;
					break;
				case RDM_DR_MT_UPDATE:
					pMsgServiceList = pSession->directoryMsgCopy.update.serviceList;
					msgServiceCount = pSession->directoryMsgCopy.update.serviceCount;
					break;
				default:
					printf( "<%s> Error: Received unhandled directory message type %d.\n",
							pSession->name, pSession->directoryMsgCopy.rdmMsgBase.rdmMsgType);
					return RSSL_RET_FAILURE;
			}			

			// Loook for the service requested
			for (iMsgServiceList = 0; iMsgServiceList < msgServiceCount; ++iMsgServiceList)
			{
				RsslRDMService *pService = &pMsgServiceList[iMsgServiceList];
				
				if (pService->flags & RDM_SVCF_HAS_INFO &&
					pService->serviceId == exampleConfig.serviceId)
				{
					foundService = RSSL_TRUE;
					pSession->pService = &pMsgServiceList[iMsgServiceList];
					break;
				}
			}

			if (!foundService)
			{
				pSession->pService = &pMsgServiceList[0];
				printf("<%s> Error: Could not find service with service id %d. Will use (name: %.*s - serviceId: " RTR_LLU ") service instead.\n\n", pSession->name,
					exampleConfig.serviceId, pSession->pService->info.serviceName.length, pSession->pService->info.serviceName.data,
					pSession->pService->serviceId);
					exampleConfig.serviceId = (RsslUInt32)pSession->pService->serviceId;
			}

			if (globalSetDefDictionaryLoaded == RSSL_FALSE)
			{
				refDataSessionRequestGlobalSetDef(pSession, exampleConfig.setDefDictName, pSession->streamId);
				globalSetDefDictionaryStreamId = pSession->streamId++;
			}

			if (fieldDictionaryLoaded == RSSL_FALSE && enumDictionaryLoaded == RSSL_FALSE)
			{
				refDataSessionRequestGlobalSetDef(pSession, "RWFFld", pSession->streamId);
				fieldDictionaryStreamId = pSession->streamId++;
				refDataSessionRequestGlobalSetDef(pSession, "RWFEnum", pSession->streamId);
				enumDictionaryStreamId = pSession->streamId++;
			}

			if (globalSetDefDictionaryLoaded && fieldDictionaryLoaded && enumDictionaryLoaded)
			{
				pSession->state = REF_DATA_STATE_READY;
			}
		}
		break;
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

					printf("<%s> Received login refresh.\n\n", pSession->name);

					if (pSession->state == REF_DATA_STATE_LOGIN_REQUESTED)
					{					
						if (refDataSessionRequestSourceDirectory(pSession) != RSSL_RET_SUCCESS)
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
		break;

		default:
		{
			printf("<%s> Received unhandled message with stream ID %d and domain %d(%s).\n\n",
					pSession->name,
					rsslMsg.msgBase.streamId,
					rsslMsg.msgBase.domainType,
					rsslDomainTypeToString(rsslMsg.msgBase.domainType));

			return RSSL_RET_SUCCESS;
		}
	}

	return RSSL_RET_SUCCESS;
}
/*
* * Takes in a map entry action's rssluint8 representation
* * and returns the string value
* * mapEntryAction - the action to be performed on a map entry 
* */
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


RsslRet processDictionaryResponse(RefDataSession *pSession, RsslMsg* msg, RsslDecodeIterator* dIter)
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
			printf("<%s> Received Dictionary Response: %.*s\n", pSession->name, key->name.length, key->name.data);
		}
		else 
		{
			printf("<%s> Received Dictionary Response\n", pSession->name);
		}

		pState = &msg->refreshMsg.state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		if ((msg->msgBase.streamId != fieldDictionaryStreamId) && 
			(msg->msgBase.streamId != enumDictionaryStreamId) && 
			(msg->msgBase.streamId != globalSetDefDictionaryStreamId))
		{
			/* The first part of a dictionary refresh should contain information about its type.
			 * Save this information and use it as subsequent parts arrive. */

			if (rsslExtractDictionaryType(dIter, &dictionaryType, &errorText) != RSSL_RET_SUCCESS)
			{
				printf("<%s> rsslGetDictionaryType() failed: %.*s\n", pSession->name, errorText.length, errorText.data);
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
				case RDM_DICTIONARY_FIELD_SET_DEFINITION:
					globalSetDefDictionaryStreamId = msg->msgBase.streamId;
					break;
				default:
					printf("<%s> Unknown dictionary type %llu from message on stream %d\n", pSession->name,
						(RsslUInt)dictionaryType, msg->msgBase.streamId);
					return RSSL_RET_SUCCESS;
			}
		}

		if (msg->msgBase.streamId == globalSetDefDictionaryStreamId)
		{
			RsslBuffer version;
			RsslRet ret;
			version.length = 0;

			printf ("<%s> Received Global Set Def\n", pSession->name);

			if (!globalFieldSetDefDb.isInitialized)
				rsslClearFieldSetDb(&globalFieldSetDefDb);

			if ((ret = rsslDecodeFieldSetDefDictionary(dIter, &globalFieldSetDefDb, RDM_DICTIONARY_VERBOSE, &errorText)) != RSSL_RET_SUCCESS)
			{
				printf("<%s> rsslDecodeFieldSetDefDictionary() failed: %d(%s) %s.\n\n", 
						pSession->name, ret, rsslRetCodeToString(ret), errorText.data);
			}
			globalSetDefDictionaryLoaded = RSSL_TRUE;
		}
		else if (msg->msgBase.streamId == fieldDictionaryStreamId)
		{
			if (rsslDecodeFieldDictionary(dIter, &dictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
			{
				printf("<%s> Decoding Dictionary failed: %.*s\n", pSession->name, errorText.length, errorText.data);
				return RSSL_RET_FAILURE;
			}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				fieldDictionaryLoaded = RSSL_TRUE;
				fieldDictionaryStreamId = 0;
			}
		} 
		else if (msg->msgBase.streamId == enumDictionaryStreamId)
		{
			if (rsslDecodeEnumTypeDictionary(dIter, &dictionary, RDM_DICTIONARY_VERBOSE, &errorText) != RSSL_RET_SUCCESS)
			{
				printf("<%s> Decoding Dictionary failed: %.*s\n", pSession->name, errorText.length, errorText.data);
				return RSSL_RET_FAILURE;
			}

			if (msg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE)
			{
				enumDictionaryLoaded = RSSL_TRUE;
				enumDictionaryStreamId = 0;
			}
		}
		else
		{
			printf("<%s> Received unexpected dictionary message on stream %d\n", pSession->name, msg->msgBase.streamId);
			return RSSL_RET_SUCCESS;
		}

		if (globalSetDefDictionaryLoaded && enumDictionaryLoaded && fieldDictionaryLoaded)
		{
			pSession->state = REF_DATA_STATE_READY;
		}	

    	break;

	case RSSL_MC_STATUS:
		printf("<%s> Received StatusMsg for dictionary\n", pSession->name);
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
		{
			RsslState *pState = &msg->statusMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
		}
		break;

	default:
		printf("<%s> Received Unhandled Dictionary MsgClass: %d\n", pSession->name, msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet encodeDictionaryRequest(RefDataSession *pSession, RsslBuffer *msgBuf, const char *dictionaryName, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslRequestMsg msg;
	RsslEncodeIterator encodeIter;

	rsslClearRequestMsg(&msg);

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
        msg.msgBase.msgKey.serviceId = (RsslUInt16)(pSession->pService->serviceId);
	msg.msgBase.msgKey.filter = RDM_DICTIONARY_VERBOSE;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslSetEncodeIteratorBuffer() failed with return code: %d\n", pSession->name, ret);
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion(&encodeIter, pSession->pRsslChannel->majorVersion, 
		pSession->pRsslChannel->minorVersion);

	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMsg() failed with return code: %d\n", pSession->name, ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}


RsslRet refDataSessionRequestGlobalSetDef(RefDataSession *pSession, const char *dictionaryName, RsslInt32 streamId)
{
	RsslError error;
	RsslBuffer* msgBuf;
	RsslRet ret;

	/* get a buffer for the dictionary request */
	msgBuf = rsslGetBuffer(pSession->pRsslChannel, 128, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode dictionary request */
		if (encodeDictionaryRequest(pSession, msgBuf, dictionaryName, streamId) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\n<%s> encodeDictionaryRequest() failed\n", pSession->name);
			return RSSL_RET_FAILURE;
		}

		/* send request */
		if ((ret = refDataSessionWrite(pSession, msgBuf)) != RSSL_RET_SUCCESS)
			return ret;
	}
	else
	{
		printf("<%s> rsslGetBuffer(): Failed <%s>\n", pSession->name, error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;

}

RsslRet refDataSessionRequestSourceDirectory(RefDataSession *pSession)
{
        RsslRDMDirectoryRequest directoryRequest;
        RsslRet ret;
        RsslBuffer *msgBuf;
        RsslEncodeIterator encodeIter;
	RsslErrorInfo rsslErrorInfo;
	RsslError rsslError;

        /* Send Directory Request */
        if ((ret = rsslInitDefaultRDMDirectoryRequest(&directoryRequest, pSession->streamId++)) != RSSL_RET_SUCCESS)
        {
                printf("<%s> rsslInitDefaultRDMDirectoryRequest() failed: %d (%s).\n\n",
			pSession->name, ret, rsslRetCodeToString(ret));
                return ret;
        }

        if (!(msgBuf = rsslGetBuffer(pSession->pRsslChannel, 128, RSSL_FALSE, &rsslError)))
        {
		printf("<%s> rsslGetBuffer() failed while sending source directory request: %d (%s -- %s).\n\n",
			pSession->name,
			ret, rsslRetCodeToString(ret), rsslError.text);
		return ret;
        }

        directoryRequest.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | \
                                                                RDM_DIRECTORY_SERVICE_STATE_FILTER | \
                                                                RDM_DIRECTORY_SERVICE_GROUP_FILTER | \
                                                                RDM_DIRECTORY_SERVICE_LOAD_FILTER | \
                                                                RDM_DIRECTORY_SERVICE_DATA_FILTER | \
                                                                RDM_DIRECTORY_SERVICE_LINK_FILTER | \
								RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER;

        rsslClearEncodeIterator(&encodeIter);
        rsslSetEncodeIteratorRWFVersion(&encodeIter, pSession->pRsslChannel->majorVersion, 
		pSession->pRsslChannel->minorVersion);
        if ( (ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) != RSSL_RET_SUCCESS)
        {
		printf("<%s> rsslSetEncodeIteratorBuffer() failed while sending source directory request: %d (%s).\n\n",
			pSession->name,
			ret, rsslRetCodeToString(ret));
		return ret;
        }

        if ((ret = rsslEncodeRDMDirectoryMsg(&encodeIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
        {
		printf("<%s> rsslEncodeRDMDirectoryMsg() failed while sending source directory request: %d (%s).\n\n",
			pSession->name,
			ret, rsslRetCodeToString(ret));
		return ret;
        }

	// Write the message.
	if ((ret = refDataSessionWrite(pSession, msgBuf)) != RSSL_RET_SUCCESS)
		return ret;

	printf("<%s> Sent source directory request.\n\n", pSession->name);

	return RSSL_RET_SUCCESS;
}

static RsslRet refDataSessionWrite(RefDataSession *pSession, RsslBuffer *pBuffer)
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

RsslRet refDataSessionProcessWriteReady(RefDataSession *pSession)
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
			return refDataSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;
	}
}
