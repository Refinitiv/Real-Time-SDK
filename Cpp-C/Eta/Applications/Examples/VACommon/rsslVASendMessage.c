/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 * This utility file is used by all VA example applications. It provides
 * functions for sending an encoded message, and sending a not supported 
 * status message.
 *
 * The ETA Reactor handles all pings on the users behalf, so none of the 
 * VA examples should require any ping handling or ping sending. 
 */

#ifdef _WIN32
#else
#include <sys/types.h>
#endif
#include "rsslVASendMessage.h"

/* Sends a message buffer to a channel
 * pReactor - RsslReactor associated with the application
 * chnl - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslBuffer* msgBuf)
{
	RsslErrorInfo rsslErrorInfo;
	RsslRet	retval = 0;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
	RsslReactorSubmitOptions submitOpts;

	rsslClearReactorSubmitOptions(&submitOpts);

	/* send the request */
	if ((retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
	{
		while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo);

		if (retval < RSSL_RET_SUCCESS)	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			printf("rsslReactorSubmit() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);
			if (retval = rsslReactorReleaseBuffer(chnl, msgBuf, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				printf("rsslReactorReleaseBuffer() failed with return code %d - <%s>\n", retval, rsslErrorInfo.rsslError.text);

			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends the not supported status message for a channel.  Returns success
 * if send not supported status message succeeds or failure if it fails.
 * pReactor - RsslReactor associated with the application
 * chnl - The channel to send not supported status message to
 * requestMsg - The partially decoded request message
 */
RsslRet sendNotSupportedStatus(RsslReactor *pReactor, RsslReactorChannel* chnl, RsslMsg* requestMsg)
{
	RsslRet ret = 0;
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the not supported status */
	msgBuf = rsslReactorGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode not supported status */
		if ((ret = encodeNotSupportedStatus(chnl, requestMsg, msgBuf)) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(chnl, msgBuf, &rsslErrorInfo);
			printf("\nencodeNotSupportedStatus() failed with return code: %d\n", ret);
			return ret;
		}

		printf("\nRejecting Item Request with streamId=%d.  Reason: Domain %d Not Supported\n", requestMsg->msgBase.streamId,  requestMsg->msgBase.domainType);

		/* send not supported status */
		if (sendMessage(pReactor, chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslReactorGetBuffer(): Failed <%s>\n", rsslErrorInfo.rsslError.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the not supported status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send not supported status message to
 * requestMsg - The partially decoded request message
 * msgBuf - The message buffer to encode the not supported status into
 */
RsslRet encodeNotSupportedStatus(RsslReactorChannel* chnl, RsslMsg* requestMsg, RsslBuffer* msgBuf)
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
	sprintf(stateText, "Request rejected for stream id %d - domain type %d is not supported", requestMsg->msgBase.streamId, requestMsg->msgBase.domainType);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
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
