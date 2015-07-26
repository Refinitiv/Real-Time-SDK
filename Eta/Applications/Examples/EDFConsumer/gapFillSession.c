/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "gapFillSession.h"
#include "itemList.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"

/* Handles an incoming message from the gap fill data feed. */
static RsslRet gapFillSessionProcessMessage(GapFillSession *pSession, RsslBuffer *pBuffer, RsslUInt32 seqNum);

RsslRet gapFillSessionConnect(GapFillSession *pSession, RsslConnectOptions *pOpts)
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

	/* Normally channel initialization may need to be considered, however this channel 
	 * initializes immediately. */
	if (pSession->pRsslChannel->state == RSSL_CH_STATE_ACTIVE)
	{
		printf("<%s> Channel is active!\n\n", pSession->name);
		return RSSL_RET_SUCCESS;
	}
	else
	{
		printf("<%s> Unhandled channel state %u.\n\n", pSession->name,
				pSession->pRsslChannel->state);
		return RSSL_RET_FAILURE;
	}

}

RsslRet gapFillSessionProcessReadReady(GapFillSession *pSession)
{

	RsslRet readRet, ret;
	RsslError rsslError;

	RsslBuffer *pBuffer;
	RsslReadInArgs readInArgs;
	RsslReadOutArgs readOutArgs;

	rsslClearReadInArgs(&readInArgs);
	rsslClearReadOutArgs(&readOutArgs);

	pBuffer = rsslReadEx(pSession->pRsslChannel, &readInArgs, &readOutArgs, &readRet, &rsslError);

	if (pBuffer)
	{
		if ((ret = gapFillSessionProcessMessage(pSession, pBuffer, readOutArgs.seqNum)) != RSSL_RET_SUCCESS)
			return ret;

		return readRet;
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
			if (readOutArgs.readOutFlags & RSSL_READ_OUT_SEQNUM)
				printf("<%s> Received ping with sequence number %u.\n\n", pSession->name,
						readOutArgs.seqNum);
			else
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
}

RsslRet gapFillSessionProcessMessage(GapFillSession *pSession, RsslBuffer *pBuffer, RsslUInt32 seqNum)
{
	RsslDecodeIterator decodeIter;
	RsslRet ret;
	RsslMsg rsslMsg;
	Item *pItem;
	Item item;


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

	if ((pItem = itemListGetItem(&itemList, rsslMsg.msgBase.streamId, rsslMsg.msgBase.domainType))
			== NULL)
	{
		if ( exampleConfig.gapFillServerVerboseOutput )
		{
			pItem = &item;
			printf("<%s> VERBOSE Received message SEQ.NO.: %u:\n", pSession->name, seqNum);
			return itemDecodeMsgData(pItem, &decodeIter, &rsslMsg);
		}
		else
			return RSSL_RET_SUCCESS; /* Discard. */
	}
	else
	{
		printf("<%s> Received message SEQ.NO.: %u:\n", pSession->name, seqNum);
	}

	return itemDecodeMsgData(pItem, &decodeIter, &rsslMsg);
}

