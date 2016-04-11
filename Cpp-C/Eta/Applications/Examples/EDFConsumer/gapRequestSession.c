/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "gapRequestSession.h"
#include "itemList.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rtr/rsslRDMLoginMsg.h"
#include <stdlib.h>

#define GAP_FILL_CUST_DOMAIN 128
static const RsslBuffer CELOXICA_ENAME_ADDRESS 	= { 7 , (char*)"Address" };
static const RsslBuffer CELOXICA_ENAME_PORT 	= { 4 , (char*)"Port" };
static const RsslBuffer CELOXICA_ENAME_GAPSTART  	= { 8 , (char*)"GapStart" };
static const RsslBuffer CELOXICA_ENAME_GAPEND    	= { 6 , (char*)"GapEnd" };

/* Decodes messages received from the gap server. */
static RsslRet gapRequestSessionProcessMessage(GapRequestSession *pSession, RsslBuffer *pBuffer);

/* Writes messages. */
static RsslRet gapRequestSessionWrite(GapRequestSession *pSession, RsslBuffer *pBuffer);

/* ID to use for the login stream. */
static const RsslInt32 LOGIN_STREAM_ID = 1;

RsslRet gapRequestSessionConnect(GapRequestSession *pSession, RsslConnectOptions *pOpts)
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
		if (gapRequestSessionProcessChannelActive(pSession) != RSSL_RET_SUCCESS)
			exit(-1);
	}

	return RSSL_RET_SUCCESS;

}

RsslRet gapRequestSessionInitializeChannel(GapRequestSession *pSession)
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
			if ((ret = gapRequestSessionProcessChannelActive(pSession)) != RSSL_RET_SUCCESS)
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

RsslRet gapRequestSessionProcessChannelActive(GapRequestSession *pSession)
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
	if ((ret = gapRequestSessionWrite(pSession, pBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;

}

RsslRet gapRequestSessionProcessReadReady(GapRequestSession *pSession)
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
				if ((ret = gapRequestSessionProcessMessage(pSession, pBuffer)) != RSSL_RET_SUCCESS)
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
			return gapRequestSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;
	}
}

static RsslRet gapRequestSessionProcessMessage (GapRequestSession *pSession, RsslBuffer *pBuffer)
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
		case GAP_FILL_CUST_DOMAIN:
		{
			RsslState *pState;

			tempBuffer.data = tempMem;
			tempBuffer.length = sizeof(tempMem);

			switch (rsslMsg.msgBase.msgClass)
			{
				case RSSL_MC_STATUS:
					if (rsslMsg.statusMsg.flags & RSSL_STMF_HAS_STATE)
					{
						printf("<%s> Received Gap Fill Status message for StreamId: %d.\n\n", 
							pSession->name, rsslMsg.msgBase.streamId);
						
						pState = &rsslMsg.statusMsg.state;

						rsslStateToString(&tempBuffer, pState);
						printf("      %.*s\n\n", tempBuffer.length, tempBuffer.data);
					}
					break;

				default:
					printf("<%s> Received unhandled Message Type %d.\n\n",
							pSession->name, rsslMsg.msgBase.msgClass);
					return RSSL_RET_FAILURE;
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

					pSession->state = GAP_REQUEST_STATE_LOGGED_IN;
					break;

				default:
					printf("<%s> Received unhandled RDM Login Message Type %d.\n\n",
							pSession->name, loginMsg.rdmMsgBase.rdmMsgType);
					return RSSL_RET_FAILURE;
			}
			return RSSL_RET_SUCCESS;
		}

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


RsslRet gapRequestSessionRequestFill(GapRequestSession *pSession, gap_info_t *info)
{
	RsslRequestMsg requestMsg;
	RsslElementEntry element;
	RsslElementList elementList;
	RsslBuffer *pBuffer;
	RsslError rsslError;
	RsslEncodeIterator encodeIter;	
	RsslBuffer address;
	RsslRet ret = RSSL_RET_FAILURE;

	if (!(pSession->state == GAP_REQUEST_STATE_LOGGED_IN))
	{
		/* if we are not logged in yet do not make a request */
		printf("<%s> Can't send a gap fill request because not logged in yet to the gap request server!\n\n",
				pSession->name);
		return RSSL_RET_SUCCESS;
	}

	if (!(pBuffer = rsslGetBuffer(pSession->pRsslChannel, 128, RSSL_FALSE, &rsslError)))
	{
		printf("<%s> rsslGetBuffer() failed while sending gap fill request: %d (%s -- %s).\n\n",
				pSession->name,
				rsslError.rsslErrorId, rsslRetCodeToString(rsslError.rsslErrorId), rsslError.text);
		return rsslError.rsslErrorId;
	}

	// Set-up message.
        rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	requestMsg.msgBase.streamId = pSession->streamId++;
	requestMsg.msgBase.domainType = GAP_FILL_CUST_DOMAIN;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	// Set msgKey members.
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_ATTRIB;
	requestMsg.msgBase.msgKey.serviceId = exampleConfig.serviceId;
	requestMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;


	// Clear the encode iterator.
	rsslClearEncodeIterator (&encodeIter);

	// Bind iterator to the buffer.
	if ((ret = rsslSetEncodeIteratorBuffer (&encodeIter, pBuffer)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslSetEncodeIteratorBuffer() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	rsslSetEncodeIteratorRWFVersion (&encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	if ((ret = rsslEncodeMsgInit (&encodeIter, (RsslMsg*)&requestMsg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMsgInit() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Encode the element list.
	rsslClearElementList (&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementListInit() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Address
	address.data = &info->address[0];
	address.length = (RsslUInt32)strlen(info->address);

	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = CELOXICA_ENAME_ADDRESS;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &address)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementEntry() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Port
	element.dataType = RSSL_DT_UINT;
	element.name = CELOXICA_ENAME_PORT;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &info->port)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementEntry() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}


	// GapStart
	element.dataType = RSSL_DT_UINT;
	element.name = CELOXICA_ENAME_GAPSTART;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &info->start)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementEntry() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// GapEnd
	element.dataType = RSSL_DT_UINT;
	element.name = CELOXICA_ENAME_GAPEND;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &info->end)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementEntry() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Complete element list.
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeElementListComplete() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Complete encode key.
	if ((ret = rsslEncodeMsgKeyAttribComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMsgKeyAttribComplete() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	// Complete encode message
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("<%s> rsslEncodeMsgComplete() failed while sending gap fill request: %d (%s).\n\n",
				pSession->name,
				ret, rsslRetCodeToString(ret));
		return ret;
	}

	pBuffer->length = rsslGetEncodedBufferLength (&encodeIter);

	/* Write the message. */
	if ((ret = gapRequestSessionWrite(pSession, pBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	printf("<%s> Sent gap fill request for message SEQ.NO. Start-End: " RTR_LLU "-" RTR_LLU " on StreamId %d\n\n",
			pSession->name, info->start, info->end, requestMsg.msgBase.streamId);

	return RSSL_RET_SUCCESS;
}

static RsslRet gapRequestSessionWrite(GapRequestSession *pSession, RsslBuffer *pBuffer)
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

RsslRet gapRequestSessionProcessWriteReady(GapRequestSession *pSession)
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
			return gapRequestSessionInitializeChannel(pSession);

		default:
			printf("<%s> Unhandled channel state %u (%s).\n\n", pSession->name,
					ret, rsslRetCodeToString(ret));
			exit(-1);
			return RSSL_RET_FAILURE;
	}
}
