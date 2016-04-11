/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "realTimeSession.h"
#include "gapRequestSession.h"
#include "itemList.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"

/* Decodes messages received from the realtime data server. */
static RsslRet realTimeSessionProcessMessage(RealTimeSession *pSession, RsslBuffer *pBuffer,
		RsslReadOutArgs* readOutArgs);

RsslRet realTimeSessionConnect(RealTimeSession *pSession, RsslConnectOptions *pOpts)
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

RsslRet realTimeSessionProcessReadReady(RealTimeSession *pSession)
{
	RsslRet readRet, ret;
	RsslError rsslError;

	RsslBuffer *pBuffer;
	RsslReadInArgs readInArgs;
	RsslReadOutArgs readOutArgs;

	rsslClearReadInArgs(&readInArgs);
	rsslClearReadOutArgs(&readOutArgs);

	pBuffer = rsslReadEx(pSession->pRsslChannel, &readInArgs, &readOutArgs, &readRet, &rsslError);


	/* Perform simple gap detection. */
        if(readOutArgs.readOutFlags & RSSL_READ_OUT_SEQNUM)
                if (readOutArgs.seqNum > pSession->gapInfo.start + 1 && pSession->gapInfo.start != 0)
                {
                        printf("<%s> Gap Detected, received sequence: %d expected sequence: "RTR_LLU" \n\n",
                                pSession->name, readOutArgs.seqNum, pSession->gapInfo.start + 1);

                        pSession->gapDetected = RSSL_TRUE;
                        pSession->gapInfo.start = pSession->gapInfo.start + 1;
                        pSession->gapInfo.end = readOutArgs.seqNum - 1;
                }
                else
                {
                        pSession->gapInfo.start = readOutArgs.seqNum;
                        pSession->gapInfo.end = readOutArgs.seqNum;
                }

	if (pBuffer)
	{
		if ((ret = realTimeSessionProcessMessage(pSession, pBuffer, &readOutArgs)) != RSSL_RET_SUCCESS)
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


RsslRet realTimeSessionProcessMessage(RealTimeSession *pSession, RsslBuffer *pBuffer, RsslReadOutArgs* readOutArgs)
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
                if ( exampleConfig.realTimeDataVerboseOutput )
                {
                        pItem = &item;
			
                        printf("<%s> VERBOSE Received message SEQ.NO.: %u:\n", pSession->name, readOutArgs->seqNum);
			ret = itemDecodeMsgData(pItem, &decodeIter, &rsslMsg);
			return ret;
                }
                else
                        return RSSL_RET_SUCCESS; /* Discard. */

	}
	else
	{
		const RsslUInt32 *pSeqNum;

		/* If not reodering messages, or message has no sequence number, process now. */
		pSeqNum = rsslGetSeqNum(&rsslMsg);

		printf("<%s> Received message SEQ.NO.: %u Item: %s StreamId: %d Seq: %d\n", 
					pSession->name, readOutArgs->seqNum, pItem->symbolName, rsslMsg.msgBase.streamId, *pSeqNum);
		ret = itemProcessMsg(pItem, &decodeIter, &rsslMsg, RSSL_FALSE);
		return ret;
	}

}
