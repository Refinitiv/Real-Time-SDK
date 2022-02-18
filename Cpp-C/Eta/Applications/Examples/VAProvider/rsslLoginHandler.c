/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the implementation of the callback for Login requests
 * received by the rsslVAProvider application.
 */

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#endif
#include "rsslProvider.h"
#include "rsslLoginHandler.h"
#include "rsslVASendMessage.h"
#include "rtr/rsslGetTime.h"

/*
 * Clears a LoginRequestInfo structure.
 */
RTR_C_INLINE void clearLoginRequestInfo(LoginRequestInfo *pInfo)
{
	pInfo->isInUse = RSSL_FALSE;
	pInfo->chnl = 0;
	rsslClearRDMLoginRequest(&pInfo->loginRequest);
	pInfo->memoryBuffer.data = pInfo->memory;
	pInfo->memoryBuffer.length = sizeof(pInfo->memory);
	pInfo->lastLatency = 0;
}

/*
 * The list of open logins.
 */
static LoginRequestInfo loginRequests[NUM_CLIENT_SESSIONS];

/* application id */
static const char *applicationId = "256";
/* application name */
static const char *applicationName = "rsslProvider";
static RsslBool rttSupport = RSSL_FALSE;

void setRTTSupport(RsslBool rtt)
{
	rttSupport = rtt;
}

/*
 * Initializes login information fields.
 */
void initLoginHandler()
{
	int i;

	for(i = 0; i < NUM_CLIENT_SESSIONS; ++i)
	{
		clearLoginRequestInfo(&loginRequests[i]);
	}
}

/*
 * Retrieves LoginRequestInfo structure to use with a consumer that is logging in.
 */
static RsslRet getLoginRequestInfo(RsslReactorChannel* pReactorChannel, RsslRDMLoginRequest* pRequest, LoginRequestInfo** loginRequestInfo)
{
	int i;
	LoginRequestInfo* pLoginRequestInfo = NULL;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (loginRequests[i].isInUse &&
			loginRequests[i].chnl == pReactorChannel)
		{
			pLoginRequestInfo = &loginRequests[i];
			/* if stream id is different from last request, this is an invalid request */
			if (pLoginRequestInfo->loginRequest.rdmMsgBase.streamId != pRequest->rdmMsgBase.streamId)
			{
				*loginRequestInfo = NULL;
			}
			break;
		}
	}

	/* get a new one if one is not already in use */
	if (!pLoginRequestInfo)
	{
		for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
		{
			if(loginRequests[i].isInUse == RSSL_FALSE)
			{
				RsslRet ret;
				loginRequests[i].chnl = pReactorChannel;
				loginRequests[i].isInUse = RSSL_TRUE;
				pLoginRequestInfo = &loginRequests[i];

				/* Copy the RsslRDMLoginRequest. */
				if ((ret = rsslCopyRDMLoginRequest(&pLoginRequestInfo->loginRequest, pRequest, &pLoginRequestInfo->memoryBuffer)) != RSSL_RET_SUCCESS)
				{
					printf("Failure: rsslCopyRDMLoginRequest\n");
					return RSSL_RET_FAILURE;
				}
				break;
			}
		}
	}

	*loginRequestInfo = pLoginRequestInfo;

	return RSSL_RET_SUCCESS;
}

/*
 * find a login request information structure for a channel.
 */
LoginRequestInfo* findLoginRequestInfo(RsslReactorChannel* pReactorChannel)
{
	int i;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (loginRequests[i].isInUse &&	loginRequests[i].chnl == pReactorChannel)
		{
			return &loginRequests[i];
		}
	}
	return NULL;
}

/*
 * Processes information contained in Login requests, and provides responses.
 */
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent)
{
	RsslRDMLoginMsg *pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;
	int i;
	LoginRequestInfo* reqInfo = NULL;
	RsslTimeValue currTicks;

	if (!pLoginMsg)
	{
		RsslMsg *pRsslMsg = pLoginMsgEvent->baseMsgEvent.pRsslMsg;
		RsslErrorInfo *pError = pLoginMsgEvent->baseMsgEvent.pErrorInfo;

		printf("loginMsgCallback() received error: %s(%s)\n", pError->rsslError.text, pError->errorLocation);

		if (pRsslMsg)
		{
			if (sendLoginRequestReject(pReactor, pReactorChannel, pRsslMsg->msgBase.streamId, LOGIN_RDM_DECODER_FAILED, pError) != RSSL_RET_SUCCESS)
				removeClientSessionForChannel(pReactor, pReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}
		else
		{
			removeClientSessionForChannel(pReactor, pReactorChannel);
			return RSSL_RC_CRET_SUCCESS;
		}
	}

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REQUEST:
		{
			RsslRDMLoginRequest *pLoginRequest = &pLoginMsg->request;
			LoginRequestInfo *pLoginRequestInfo = 0;
		   
			if (getLoginRequestInfo(pReactorChannel, pLoginRequest, &pLoginRequestInfo) != RSSL_RET_SUCCESS)
			{
				removeClientSessionForChannel(pReactor, pReactorChannel);
				return RSSL_RC_CRET_SUCCESS;
			}

			if (!pLoginRequestInfo)
			{
				if (sendLoginRequestReject(pReactor, pReactorChannel, pLoginRequest->rdmMsgBase.streamId, MAX_LOGIN_REQUESTS_REACHED, NULL) != RSSL_RET_SUCCESS)
					removeClientSessionForChannel(pReactor, pReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
			{
				/* If this an authentication token user, the application needs to take the token and verify it against the token infrastructure.
				 * In this example, the VAProvider will just accept the token */
				printf("\nReceived Login Request for Token: %.*s\n", pLoginRequest->userName.length, pLoginRequest->userName.data);
			}
			else if(pLoginRequest->userName.length != 0 && pLoginRequest->userName.data != NULL)
			{
				printf("\nReceived Login Request for Username: %.*s\n", pLoginRequest->userName.length, pLoginRequest->userName.data);
			}
			else
			{
				if (sendLoginRequestReject(pReactor, pReactorChannel, pLoginRequest->rdmMsgBase.streamId, NO_USER_NAME_IN_REQUEST, NULL) != RSSL_RET_SUCCESS)
					removeClientSessionForChannel(pReactor, pReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			/* send login response */
			if (sendLoginRefresh(pReactor, pReactorChannel, pLoginRequest) != RSSL_RET_SUCCESS)
				removeClientSessionForChannel(pReactor, pReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}

		case RDM_LG_MT_CLOSE:
		{
			printf("\nReceived Login Close for StreamId %d\n", pLoginMsg->rdmMsgBase.streamId);

			/* close login stream */
			closeLoginStream(pLoginMsg->rdmMsgBase.streamId);
			return RSSL_RC_CRET_SUCCESS;
		}

		case RDM_LG_MT_RTT:
		{
			/* find original request information associated with pReactorChannel */
			for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
			{
				if (loginRequests[i].chnl == pReactorChannel)
				{
					if (loginRequests[i].loginRequest.flags & RDM_LG_RQF_RTT_SUPPORT)
					{
						reqInfo = &loginRequests[i];
						break;
					}
					else
						return RSSL_RET_SUCCESS;
				}
			}
			printf("Received login RTT message from Consumer "SOCKET_PRINT_TYPE".\n", pReactorChannel->socketId);
			printf("\tRTT Tick value is " RTR_LLU "us.\n", pLoginMsg->RTT.ticks);
			if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_TCP_RETRANS)
				printf("\tConsumer side TCP retransmissions: " RTR_LLU "\n", pLoginMsg->RTT.tcpRetrans);

			currTicks = rsslGetTicks();

			reqInfo->lastLatency = (RsslUInt)(((RsslDouble)currTicks - (RsslDouble)pLoginMsg->RTT.ticks) / rsslGetTicksPerMicro());

			printf("\tLast RTT message latency is " RTR_LLU "us.\n", reqInfo->lastLatency);

			return RSSL_RET_SUCCESS;
		}
		default:
			printf("\nReceived unhandled login msg type: %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RC_CRET_SUCCESS;
	}
}


/*
 * Sends a login refresh to a channel.  This consists of getting
 * a message buffer, initializing the RsslRDMLoginRefresh structure, 
 * encoding it, and sending the encoded message.
 * pReactorChannel - The channel to send a login response to
 * pLoginRequest - The login request that solicited this refresh
 */
static RsslRet sendLoginRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginRequest* pLoginRequest)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	RsslUInt32 ipAddress = 0;
	RsslRet ret;

	/* get a buffer for the login response */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslRDMLoginRefresh loginRefresh;
		RsslEncodeIterator eIter;

		rsslClearRDMLoginRefresh(&loginRefresh);

		/* Set state information */
		loginRefresh.state.streamState = RSSL_STREAM_OPEN;
		loginRefresh.state.dataState = RSSL_DATA_OK;
		loginRefresh.state.code = RSSL_SC_NONE;

		/* Set stream ID */
		loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;

		/* Mark refresh as solicited since it is a response to a request. */
		loginRefresh.flags = RDM_LG_RFF_SOLICITED;

		/* Echo the userName, applicationId, applicationName, and position */
		loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;
		loginRefresh.userName = pLoginRequest->userName;
		if (pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
		{
			loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
			loginRefresh.userNameType = pLoginRequest->userNameType;
		}

		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
		loginRefresh.applicationId = pLoginRequest->applicationId;

		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
		loginRefresh.applicationName = pLoginRequest->applicationName;

		loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;
		loginRefresh.position = pLoginRequest->position;

		/* This provider does not support Single-Open behavior. */
		loginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;
		loginRefresh.singleOpen = 0; 

		/* This provider supports posting. */
		loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;
		loginRefresh.supportOMMPost = 1; 

		/* This provider supports batch requests*/
		loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_BATCH;
		loginRefresh.supportBatchRequests = RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_CLOSES;     /* this provider supports batch requests and batch close */

		/* set the clear cache flag */
		loginRefresh.flags |= RDM_LG_RFF_CLEAR_CACHE;

		if (rttSupport && pLoginRequest->flags & RDM_LG_RQF_RTT_SUPPORT)
			loginRefresh.flags |= RDM_LG_RFF_RTT_SUPPORT;

		/* Leave all other parameters as default values. */

		/* Encode the refresh. */
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		if((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMLoginRefresh() failed:%s(%s)\n", rsslErrorInfo.rsslError.text, rsslErrorInfo.errorLocation);
			return RSSL_RET_FAILURE;
		}

		/* Send the refresh. */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
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
 * Sends the login request reject status message for a channel.
 * pReactorChannel - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendLoginRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslLoginRejectReason reason, RsslErrorInfo *pError)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the login request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslRet ret = 0;
		char stateText[MAX_LOGIN_INFO_STRLEN];
		RsslEncodeIterator encodeIter;
		RsslRDMLoginStatus loginStatus;
		RsslErrorInfo rsslErrorInfo;

		rsslClearRDMLoginStatus(&loginStatus);
		loginStatus.flags |= RDM_LG_STF_HAS_STATE;
		loginStatus.rdmMsgBase.streamId = streamId;
		loginStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		loginStatus.state.dataState = RSSL_DATA_SUSPECT;

		/* set-up message */
		switch(reason)
		{
			case MAX_LOGIN_REQUESTS_REACHED:
				loginStatus.state.code = RSSL_SC_TOO_MANY_ITEMS;
				snprintf(stateText, sizeof(stateText), "Login request rejected for stream id %d - max request count reached", streamId);
				loginStatus.state.text.data = stateText;
				loginStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			case LOGIN_RDM_DECODER_FAILED:
				/* The typed message decoder failed. Pass along the error text. */
				loginStatus.state.code = RSSL_SC_USAGE_ERROR;
				snprintf(stateText, sizeof(stateText), "Login request rejected for stream id %d - decoding failure: %s", streamId, pError->rsslError.text);
				loginStatus.state.text.data = stateText;
				loginStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			case NO_USER_NAME_IN_REQUEST:
				loginStatus.state.code = RSSL_SC_USAGE_ERROR;
				snprintf(stateText, 128, "Login request rejected for stream id %d - request does not contain user name", streamId);
				loginStatus.state.text.data = stateText;
				loginStatus.state.text.length = (RsslUInt32)strlen(stateText) + 1;
				break;
			default:
				break;
		}

		/* encode message */
		rsslClearEncodeIterator(&encodeIter);
		if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		if (ret = rsslEncodeRDMLoginMsg(&encodeIter, (RsslRDMLoginMsg*)&loginStatus, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMLoginMsg() failed\n");
			return RSSL_RET_FAILURE;
		}

		msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

		/* send request reject status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
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
* Sends the login RTT message for a channel.
* pReactorChannel - The channel to send RTT message to
* streamId - The stream id of the stream
*/
RsslRet sendLoginRTT(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel)
{
	RsslErrorInfo rsslErrorInfo;
	RsslBuffer* msgBuf = 0;
	LoginRequestInfo* reqInfo = NULL;
	int i;

	if (rttSupport == RSSL_FALSE)
		return RSSL_RET_SUCCESS;

	/* find original Login request information associated with pReactorChannel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (loginRequests[i].chnl == pReactorChannel)
		{
			if (loginRequests[i].loginRequest.flags & RDM_LG_RQF_RTT_SUPPORT)
			{
				reqInfo = &loginRequests[i];
				break;
			}
			else
				return RSSL_RET_SUCCESS;
		}
	}

	if (reqInfo == NULL)
		return RSSL_RET_SUCCESS;

	/* get a buffer for the login request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, MAX_MSG_SIZE, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		RsslRet ret = 0;
		RsslEncodeIterator encodeIter;
		RsslRDMLoginRTT loginRTT;
		RsslErrorInfo rsslErrorInfo;
		RsslChannelStats stats;
		RsslError error;
		RsslTimeValue ticks;

		/* Get the current ticks. */
		ticks = rsslGetTicks();

		rsslClearRDMLoginRTT(&loginRTT);

		loginRTT.rdmMsgBase.streamId = reqInfo->loginRequest.rdmMsgBase.streamId;
		
		if (reqInfo->lastLatency != 0)
		{
			loginRTT.flags |= RDM_LG_RTT_HAS_LATENCY;
			loginRTT.lastLatency = reqInfo->lastLatency;
		}

		if ((ret = rsslGetChannelStats(pReactorChannel->pRsslChannel, &stats, &error)) != RSSL_RET_SUCCESS)
		{
			printf("rsslGetChannelStats() failed with return code: %d\n", ret);
			return ret;
		}

		loginRTT.flags |= RDM_LG_RTT_HAS_TCP_RETRANS;
		loginRTT.tcpRetrans = stats.tcpStats.tcpRetransmitCount;

		loginRTT.ticks = ticks;

		/* encode message */
		rsslClearEncodeIterator(&encodeIter);
		if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
			return RSSL_RET_FAILURE;
		}
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		if (ret = rsslEncodeRDMLoginMsg(&encodeIter, (RsslRDMLoginMsg*)&loginRTT, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			printf("\nrsslEncodeRDMLoginMsg() failed\n");
			return RSSL_RET_FAILURE;
		}

		msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

		/* send request reject status */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
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
 * Closes the login stream for a channel. 
 * pReactorChannel - The channel to close the login stream for
 */
void closeLoginStreamForChannel(RsslReactorChannel* pReactorChannel)
{
	int i;
	LoginRequestInfo* loginReqInfo = NULL;

	/* find original request information associated with pReactorChannel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(loginRequests[i].chnl == pReactorChannel)
		{
			loginReqInfo = &loginRequests[i];
			/* clear original request information */
			printf("Closing login stream id %d with user name: %.*s\n", loginReqInfo->loginRequest.rdmMsgBase.streamId, loginReqInfo->loginRequest.userName.length, loginReqInfo->loginRequest.userName.data);
			clearLoginRequestInfo(loginReqInfo);
			break;
		}
	}
}

/* 
 * Closes a login stream. 
 * streamId - The stream id to close the login for
 */
static void closeLoginStream(RsslInt32 streamId)
{
	int i;
	LoginRequestInfo* loginReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(loginRequests[i].loginRequest.rdmMsgBase.streamId == streamId)
		{
			loginReqInfo = &loginRequests[i];
			/* clear original request information */
			printf("Closing login stream id %d with user name: %.*s\n", loginReqInfo->loginRequest.rdmMsgBase.streamId, loginReqInfo->loginRequest.userName.length, loginReqInfo->loginRequest.userName.data);
			clearLoginRequestInfo(loginReqInfo);
			break;
		}
	}
}
