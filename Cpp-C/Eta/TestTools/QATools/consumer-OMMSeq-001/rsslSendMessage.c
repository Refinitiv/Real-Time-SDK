/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2018,2020 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */



/*
 * This utility file is used by all example applications. It provides
 * functions for sending an encoded message, sending a ping message,
 * and sending a not supported status message.
 */

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#endif
#include "rsslSendMessage.h"
#include "rsslJsonSession.h"

RsslUInt32 outBytesTotal;
RsslUInt32 uncompOutBytesTotal;
extern fd_set wrtfds; /* located in application */

/*
 * Sends a message buffer to a channel.  
 * chnl - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* chnl, RsslBuffer* msgBuf)
{
	RsslError error;
	RsslRet	retval = 0;
	RsslUInt32 outBytes = 0;
    RsslUInt32 uncompOutBytes = 0;
    RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
	RsslBuffer* tempBuf;

	if (chnl->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		tempBuf = rsslJsonSessionMsgConvertToJson((RsslJsonSession*)(chnl->userSpecPtr), chnl, msgBuf, &error);

		if (tempBuf == NULL)
		{
			printf("\nRsslJson conversion failed with text: %s\n", error.text);
			rsslReleaseBuffer(msgBuf, &error);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		tempBuf = msgBuf;
	}

    /* send the request */
    if ((retval = rsslWrite(chnl, tempBuf, RSSL_HIGH_PRIORITY, writeFlags, &outBytes, &uncompOutBytes, &error)) > RSSL_RET_FAILURE)

	{
		/* set write fd if there's still data queued */
		/* flush is done by application */
		if (retval > RSSL_RET_SUCCESS)
		{
			FD_SET(chnl->socketId, &wrtfds);
		}
		if (showTransportDetails == RSSL_TRUE)
		{
           outBytesTotal += outBytes;
            uncompOutBytesTotal += uncompOutBytes;
            printf("\nCompression Stats Bytes Out: %d Uncompressed Bytes Out: %d\n", outBytesTotal, uncompOutBytesTotal);
		}
	}
	else
	{
		if (retval == RSSL_RET_WRITE_CALL_AGAIN)
		{
			/* call flush and write again */
			while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			{
				if ((retval = rsslFlush(chnl, &error)) < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
               retval = rsslWrite(chnl, tempBuf, RSSL_HIGH_PRIORITY, writeFlags, &outBytes, &uncompOutBytes, &error);
				if (showTransportDetails == RSSL_TRUE)
				{
					outBytesTotal += outBytes;
					uncompOutBytesTotal += uncompOutBytes;
					printf("\nCompression Stats Bytes Out: %d Uncompressed Bytes Out: %d\n", outBytesTotal, uncompOutBytesTotal);
				}
			}
			/* set write fd if there's still data queued */
			/* flush is done by application */
			if (retval > RSSL_RET_SUCCESS)
			{
				FD_SET(chnl->socketId, &wrtfds);
			}
		}
		else if (retval == RSSL_RET_WRITE_FLUSH_FAILED && chnl->state != RSSL_CH_STATE_CLOSED)
		{
			/* set write fd if flush failed */
			/* flush is done by application */
			FD_SET(chnl->socketId, &wrtfds);
		}
		else	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			printf("rsslWrite() failed with return code %d - <%s>\n", retval, error.text);
			rsslReleaseBuffer(tempBuf, &error);
			return RSSL_RET_FAILURE;
		}
	}

	if (chnl->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		rsslReleaseBuffer(msgBuf, &error);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a message buffer to a channel.  
 * chnl - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessageEx(RsslChannel* chnl, RsslBuffer* msgBuf, RsslWriteInArgs *writeInArgs)
{
	RsslError error;
	RsslRet	retval = 0;
	RsslWriteOutArgs writeOutArgs;
	RsslBuffer* tempBuf;

	if (chnl->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		tempBuf = rsslJsonSessionMsgConvertToJson((RsslJsonSession*)(chnl->userSpecPtr), chnl, msgBuf, &error);

		if (tempBuf == NULL)
		{
			printf("\nRsslJson conversion failed with text: %s\n", error.text);
			rsslReleaseBuffer(msgBuf, &error);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		tempBuf = msgBuf;
	}

	/* send the request */
	rsslClearWriteOutArgs(&writeOutArgs);
	if ((retval = rsslWriteEx(chnl, tempBuf, writeInArgs, &writeOutArgs, &error)) > RSSL_RET_FAILURE)
	{
		/* set write fd if there's still data queued */
		/* flush is done by application */
		if (retval > RSSL_RET_SUCCESS)
		{
			FD_SET(chnl->socketId, &wrtfds);
		}
		if (showTransportDetails == RSSL_TRUE)
		{
			outBytesTotal += writeOutArgs.bytesWritten;
			uncompOutBytesTotal += writeOutArgs.uncompressedBytesWritten;
			printf("\nCompression Stats Bytes Out: %d Uncompressed Bytes Out: %d\n", outBytesTotal, uncompOutBytesTotal);
		}
	}
	else
	{
		if (retval == RSSL_RET_WRITE_CALL_AGAIN)
		{
			/* call flush and write again */
			while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			{
				if ((retval = rsslFlush(chnl, &error)) < RSSL_RET_SUCCESS)
				{
					printf("rsslFlush() failed with return code %d - <%s>\n", retval, error.text);
				}
				retval = rsslWriteEx(chnl, tempBuf, writeInArgs, &writeOutArgs, &error);
				if (showTransportDetails == RSSL_TRUE)
				{
					outBytesTotal += writeOutArgs.bytesWritten;
					uncompOutBytesTotal += writeOutArgs.uncompressedBytesWritten;
					printf("\nCompression Stats Bytes Out: %d Uncompressed Bytes Out: %d\n", outBytesTotal, uncompOutBytesTotal);
				}
			}
			/* set write fd if there's still data queued */
			/* flush is done by application */
			if (retval > RSSL_RET_SUCCESS)
			{
				FD_SET(chnl->socketId, &wrtfds);
			}
		}
		else if (retval == RSSL_RET_WRITE_FLUSH_FAILED && chnl->state != RSSL_CH_STATE_CLOSED)
		{
			/* set write fd if flush failed */
			/* flush is done by application */
			FD_SET(chnl->socketId, &wrtfds);
		}
		else	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			printf("rsslWrite() failed with return code %d - <%s>\n", retval, error.text);
			rsslReleaseBuffer(tempBuf, &error);
			return RSSL_RET_FAILURE;
		}
	}

	if (chnl->protocolType == RSSL_JSON_PROTOCOL_TYPE)
	{
		rsslReleaseBuffer(msgBuf, &error);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a ping to a channel.  Returns success if
 * send ping succeeds or failure if it fails.
 * chnl - The channel to send a ping to
 */
RsslRet sendPing(RsslChannel* chnl)
{
	RsslError error;
	RsslRet ret = 0;

	if ((ret = rsslPing(chnl, &error)) < RSSL_RET_SUCCESS)
	{
		printf("\nrsslPing(): Failed on fd="SOCKET_PRINT_TYPE" with code %d\n", chnl->socketId, ret);
		return ret;
	}
	else if (ret > RSSL_RET_SUCCESS)
	{
		/* set write fd if there's still data queued */
		/* flush is done by application */
		FD_SET(chnl->socketId, &wrtfds);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the not supported status message for a channel.  Returns success
 * if send not supported status message succeeds or failure if it fails.
 * chnl - The channel to send not supported status message to
 * msg - The partially decoded request message
 */
RsslRet sendNotSupportedStatus(RsslChannel* chnl, RsslMsg* requestMsg)
{
	RsslRet ret = 0;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the not supported status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode not supported status */
		if ((ret = encodeNotSupportedStatus(chnl, requestMsg, msgBuf)) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeNotSupportedStatus() failed with return code: %d\n", ret);
			return ret;
		}

		printf("\nRejecting Item Request with streamId=%d.  Reason: Domain %d Not Supported\n", requestMsg->msgBase.streamId,  requestMsg->msgBase.domainType);

		/* send not supported status */
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
 * Encodes the not supported status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send not supported status message to
 * msg - The partially decoded request message
 * msgBuf - The message buffer to encode the not supported status into
 */
RsslRet encodeNotSupportedStatus(RsslChannel* chnl, RsslMsg* requestMsg, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_MSG_SIZE];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = requestMsg->msgBase.streamId;
	msg.msgBase.domainType = requestMsg->msgBase.domainType;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_USAGE_ERROR;
	snprintf(stateText, 4096, "Request rejected for stream id %d - domain type %d is not supported", requestMsg->msgBase.streamId, requestMsg->msgBase.domainType);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;

	/* encode message */
	if((rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
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
