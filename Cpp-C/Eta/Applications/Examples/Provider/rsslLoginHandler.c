/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



/*
 * This is the login handler for the rsslProvider application.
 * Only one login stream per channel is allowed by this simple
 * provider.  It provides functions for processing login requests
 * from consumers and sending back the responses.  Functions for
 * sending login request reject/close status messages, initializing
 * the login handler, and closing login streams are also provided.
 */

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#endif
#include "rsslLoginHandler.h"
#include "rsslSendMessage.h"

/* login request information list */
static RsslLoginRequestInfo loginRequestInfoList[NUM_CLIENT_SESSIONS];

/* application id */
static const char *applicationId = "256";
/* application name */
static const char *applicationName = "rsslProvider";

static RsslBool supportRTT = RSSL_FALSE;
/* cookie name */
static const char *nameCookieAuthToken = "AuthToken";
/* cookie name */
static const char *nameCookiePosition = "AuthPosition";
/* cookie name */
static const char *nameCookieApplicationId = "applicationId";

/* authentication token retrived from cookies*/
static char valueCookieAuthToken[AUTH_TOKEN_LENGTH];

/* position retrived from cookies*/
static char valueCookiePosition[MAX_LOGIN_INFO_STRLEN];

/* applicationId token retrived from cookies*/
static char valueCookieApplicationId[MAX_LOGIN_INFO_STRLEN];

/* use cookies for login */
static RsslBool loginWithCookies = RSSL_FALSE;

/*
 * Initializes login information fields.
 */
void initLoginHandler(RsslBool rttSupport)
{
	int i;

	for(i = 0; i < NUM_CLIENT_SESSIONS; ++i)
	{
		clearLoginReqInfo(&loginRequestInfoList[i]);
	}

	supportRTT = rttSupport;
}

/*
 * Gets a login request information structure for a channel.
 * chnl - The channel to get the login request information structure for
 * msg - The partially decoded message
 */
static RsslLoginRequestInfo* getLoginReqInfo(RsslChannel* chnl, RsslMsg* msg)
{
	int i;
	RsslLoginRequestInfo* loginReqInfo = NULL;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (loginRequestInfoList[i].IsInUse &&
			loginRequestInfoList[i].Chnl == chnl)
		{
			loginReqInfo = &loginRequestInfoList[i];
			/* if stream id is different from last request, this is an invalid request */
			if (loginReqInfo->StreamId != msg->msgBase.streamId)
			{
				return NULL;
			}
			break;
		}
	}

	/* get a new one if one is not already in use */
	if (!loginReqInfo)
	{
		for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
		{
			if(loginRequestInfoList[i].IsInUse == RSSL_FALSE)
			{
				loginRequestInfoList[i].Chnl = chnl;
				loginRequestInfoList[i].IsInUse = RSSL_TRUE;
				loginReqInfo = &loginRequestInfoList[i];
				break;
			}
		}
	}

	return loginReqInfo;
}

/*
 * find a login request information structure for a channel.
 * chnl - The channel to get the login request information structure for
 * msg - The partially decoded message
 */
RsslLoginRequestInfo* findLoginReqInfo(RsslChannel* chnl)
{
	int i;
	RsslLoginRequestInfo* loginReqInfo = NULL;

	/* first check if one already in use for this channel */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if (loginRequestInfoList[i].IsInUse && loginRequestInfoList[i].Chnl == chnl)
		{
			loginReqInfo = &loginRequestInfoList[i];
			break;
		}
	}
	return loginReqInfo;
}

/*
 * Processes a login request.  This consists of calling
 * decodeLoginRequest() to decode the request and calling
 * sendLoginResponse() to send the login response.
 * chnl - The channel of the request
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processLoginRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslState *pState = 0;
	RsslMsgKey* key = 0;
	RsslLoginRequestInfo* loginRequestInfo;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REQUEST:

		if (loginWithCookies)
		{
			if (sendLoginRequestReject(chnl, msg->msgBase.streamId, ALREADY_LOGGED_WITH_COOKIE) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			loginWithCookies = RSSL_FALSE;
			break;
		}

		/* get key */
		key = (RsslMsgKey *)rsslGetMsgKey(msg);

		/* check if key has user name */
		/* user name is only login user type accepted by this application (user name is the default type) */
		if (!(key->flags & RSSL_MKF_HAS_NAME) || ((key->flags & RSSL_MKF_HAS_NAME_TYPE) && ((key->nameType != RDM_LOGIN_USER_NAME) && (key->nameType != RDM_LOGIN_USER_AUTHN_TOKEN))))
		{
			sendLoginRequestReject(chnl, msg->msgBase.streamId, NO_USER_NAME_IN_REQUEST);
			break;
		}

		loginRequestInfo = getLoginReqInfo(chnl, msg);

		if (!loginRequestInfo)
		{
			if (sendLoginRequestReject(chnl, msg->msgBase.streamId, MAX_LOGIN_REQUESTS_REACHED) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			break;
		}
		/* decode login request */
		if (decodeLoginRequest(loginRequestInfo, msg, dIter, key) != RSSL_RET_SUCCESS)
		{
			printf("\ndecodeLoginRequest() failed\n");
			return RSSL_RET_FAILURE;
		}

		if(key->nameType != RDM_LOGIN_USER_AUTHN_TOKEN)
			printf("\nReceived Login Request for Username: %.*s\n", (int)strlen(loginRequestInfo->Username), loginRequestInfo->Username);
		else
			printf("\nReceived Login Request with Token: %.*s\n", (int)strlen(loginRequestInfo->AuthenticationToken), loginRequestInfo->AuthenticationToken);

		/* Check to see if RTT is supported.  If it is not, make sure it's turned off in the response */
		if (supportRTT == RSSL_FALSE)
		{
			loginRequestInfo->RTT = RSSL_FALSE;
		}

		if (loginRequestInfo->RTT == RSSL_TRUE)
		{
			printf("Round Trip Time Latency messages requested by the client\n");
		}

		/* send login response */
		if (sendLoginResponse(chnl, loginRequestInfo) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		break;

	case RSSL_MC_CLOSE:
		printf("\nReceived Login Close for StreamId %d\n", msg->msgBase.streamId);

		/* close login stream */
		closeLoginStream(msg->msgBase.streamId);

		break;
	case RSSL_MC_GENERIC:
		if (supportRTT == RSSL_FALSE)
		{
			printf("RTT messages are not supported for this run.  Turn it on with -rtt.\n");
			return RSSL_RET_SUCCESS;
		}
		printf("\nReceived RTT response from client.\n");
		loginRequestInfo = getLoginReqInfo(chnl, msg);

		if (!loginRequestInfo)
		{
			printf("Received an RTT response on a closed login stream\n");
			return RSSL_RET_FAILURE;
		}

		decodeLoginRTTForServer(chnl, msg, dIter, &(loginRequestInfo->RTTLatency));
		break;

	default:
		printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
    	return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Sends a login response to a channel.  This consists of getting
 * a message buffer, setting the login response information, encoding
 * the login response, and sending the login response to the server.
 * chnl - The channel to send a login response to
 * loginReqInfo - The login request information
 */
static RsslRet sendLoginResponse(RsslChannel* chnl, RsslLoginRequestInfo* loginReqInfo)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslLoginResponseInfo loginRespInfo;
	RsslUInt32 ipAddress = 0;
	
	/* initialize login response info */
	initLoginRespInfo(&loginRespInfo);

	/* get a buffer for the login response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* provide login response information */
		/* StreamId */
		loginRespInfo.StreamId = loginReqInfo->StreamId;
		/* Username */
		snprintf(loginRespInfo.Username, 128, "%s", loginReqInfo->Username);
		/* ApplicationId */
		snprintf(loginRespInfo.ApplicationId, 128, "%s", applicationId);
		/* ApplicationName */
		snprintf(loginRespInfo.ApplicationName, 128, "%s", applicationName);
		/* Position */
		snprintf(loginRespInfo.Position, 128, "%s", loginReqInfo->Position);
		
		loginRespInfo.SingleOpen = 0;				/* this provider does not support SingleOpen behavior */
		loginRespInfo.SupportBatchRequests = RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_CLOSES;		/* this provider supports batch requests and batch close */
		loginRespInfo.SupportOMMPost = 1;
		/* keep default values for all others */

		/* encode login response */
		if (encodeLoginResponse(chnl, &loginRespInfo, msgBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeLoginResponse() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send login response */
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

RsslRet sendRTTLoginMsg(RsslChannel* chnl)
{
	RsslLoginRequestInfo *info = NULL;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	if ((info = findLoginReqInfo(chnl)) == NULL)
	{
		printf("Cannot send RTT message on a closed Login stream\n");
		return RSSL_RET_FAILURE;
	}

	if (info->RTT == RSSL_FALSE)
		return RSSL_RET_SUCCESS;

	/* get a buffer for the RTT message */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode login RTT */
		if (encodeLoginRTTServer(chnl, msgBuf, info->StreamId, info->RTTLatency) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeLoginRTTServer() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send login response */
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
 * Sends the login request reject status message for a channel.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 */
static RsslRet sendLoginRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslLoginRejectReason reason)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the login request reject status */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode login request reject status */
		if (encodeLoginRequestReject(chnl, streamId, reason, msgBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeLoginRequestReject() failed\n"); 
			return RSSL_RET_FAILURE;
		}

		/* send request reject status */
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
 * Sends the login response based on cookies data
 * chnl - The channel to send request reject status message to
 * reason - The reason for the reject
 */
RsslRet sendCoockiesLoginResponse(RsslChannel* chnl) 
{
	RsslLoginResponseInfo loginRespInfo;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	initLoginRespInfo(&loginRespInfo);

	/* get a buffer for the login response */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		loginRespInfo.StreamId = -1;
		snprintf(loginRespInfo.Username, AUTH_TOKEN_LENGTH, "%s", valueCookieAuthToken);
		snprintf(loginRespInfo.ApplicationId, MAX_LOGIN_INFO_STRLEN, "%s", valueCookieApplicationId);
		snprintf(loginRespInfo.Position, MAX_LOGIN_INFO_STRLEN, "%s", valueCookiePosition);

		loginRespInfo.SingleOpen = 0;				/* this provider does not support SingleOpen behavior */
		loginRespInfo.SupportBatchRequests = RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_CLOSES;		/* this provider supports batch requests and batch close */
		loginRespInfo.SupportOMMPost = 1;
		/* keep default values for all others */

		/* encode login response */
		if (encodeLoginResponse(chnl, &loginRespInfo, msgBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error);
			printf("\nencodeLoginResponse() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send login response */
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
 * Closes the login stream for a channel. 
 * chnl - The channel to close the login stream for
 */
void closeLoginChnlStream(RsslChannel* chnl)
{
	int i;
	RsslLoginRequestInfo* loginReqInfo = NULL;

	/* find original request information associated with chnl */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(loginRequestInfoList[i].Chnl == chnl)
		{
			loginReqInfo = &loginRequestInfoList[i];
			/* clear original request information */
			printf("Closing login stream id %d with user name: %.*s\n", loginReqInfo->StreamId, (int)strlen(loginReqInfo->Username), loginReqInfo->Username);
			clearLoginReqInfo(loginReqInfo);
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
	RsslLoginRequestInfo* loginReqInfo = NULL;

	/* find original request information associated with streamId */
	for (i = 0; i < NUM_CLIENT_SESSIONS; i++)
	{
		if(loginRequestInfoList[i].StreamId == streamId)
		{
			loginReqInfo = &loginRequestInfoList[i];
			/* clear original request information */
			printf("Closing login stream id %d with user name: %.*s\n", loginReqInfo->StreamId, (int)strlen(loginReqInfo->Username), loginReqInfo->Username);
			clearLoginReqInfo(loginReqInfo);
			break;
		}
	}
}

static RsslBool copyCookieValue(const char *cookies, char* value)
{
	RsslBool endOfLine = RSSL_FALSE;
	char* ptrCoockieEq = (char*)cookies, *ptrCoockieSc;
	size_t valueLength = 0;

	ptrCoockieEq = strchr(ptrCoockieEq, '=');
	if (ptrCoockieEq)
	{
		ptrCoockieSc = strchr(ptrCoockieEq, ';');
		if (ptrCoockieSc)
		{
			valueLength = (size_t)ptrCoockieSc - (size_t)ptrCoockieEq - 1; // ptrCoockieEq - 1 skip  in value ';' 
			strncpy(value, ptrCoockieEq + 1, valueLength); // ptrCoockieEq + 1 to point to the next symbol after '='
		}
		else
		{
			strcpy(value, ptrCoockieEq + 1); // ptrCoockieEq + 1 to point to the next symbol after '='
		}
	}
	else
	{
		printf("Cookies value is not valid: %s \n", cookies);
		endOfLine = RSSL_TRUE;
	}
	return endOfLine;
}

RsslBool checkLoginCockies(const char* coockies)
{
	static RsslUInt32 cookiesCnt = 0;
	const char* ptrCoockie = coockies;

	cookiesCnt = cookiesCnt > 3 ? 0 : cookiesCnt;

	while (*ptrCoockie)
	{
		if (!strncmp(ptrCoockie, nameCookieAuthToken, strlen(nameCookieAuthToken)))
		{
			cookiesCnt++;
			if (copyCookieValue(ptrCoockie, valueCookieAuthToken)) break;
		}
		if (!strncmp(ptrCoockie, nameCookiePosition, strlen(nameCookiePosition)))
		{
			cookiesCnt++;
			if (copyCookieValue(ptrCoockie, valueCookiePosition)) break;
		}
		if (!strncmp(ptrCoockie, nameCookieApplicationId, strlen(nameCookieApplicationId)))
		{
			cookiesCnt++;
			if (copyCookieValue(ptrCoockie, valueCookieApplicationId)) break;
		}
		ptrCoockie++;
	}

	return (loginWithCookies = (cookiesCnt >= 3 ? RSSL_TRUE : RSSL_FALSE));
}
