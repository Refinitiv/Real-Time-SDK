
/*
 * This is the login consumer for both the rsslConsumer and
 * rsslNIProvider applications.  It only supports a single
 * login instance for an application.  It provides functions
 * for sending a login request and processing the response.
 * A functions for closing the login stream is also provided.
 */

#include "rsslLoginConsumer.h"
#include "rsslLoginEncodeDecode.h"
#include "rsslSendMessage.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

/* rsslLoginConsumer only supports a single login instance for an application */

/* login response information */
static RsslLoginResponseInfo loginResponseInfo;

/* default user name  */
static const char *defaultUsername = "user";
/* application id */
static const char *applicationId = "256";
/* password */
static const char *pword = "mypassword";
/* instance id */
static const char *instanceId = "instance1";

/* user name requested by application */
static char cmdLineUsername[MAX_LOGIN_INFO_STRLEN];

/* login success callback function pointer */
LoginSuccessCallback loginSuccessCallback = NULL;

/* tracks state returned by status and refresh messages */
static RsslBool isClosed = RSSL_FALSE;
static RsslBool isClosedRecoverable = RSSL_FALSE;
static RsslBool isSuspect = RSSL_TRUE;
static RsslBool isProvDicDownloadSupported = RSSL_FALSE;

/* returns whether login stream is closed */
RsslBool isLoginStreamClosed()
{
	return isClosed;
}

/* returns whether login stream is closed recoverable */
RsslBool isLoginStreamClosedRecoverable()
{
	return isClosedRecoverable;
}

/* returns whether login stream is suspect */
RsslBool isLoginStreamSuspect()
{
	return isSuspect;
}

/* returns whether provider dictionary download is supported */
RsslBool isProviderDictionaryDownloadSupported()
{
	return isProvDicDownloadSupported;
}

/*
 * Sets the user name requested by the application.
 * username - The user name requested by the application
 */
void setUsername(char* username)
{
	snprintf(cmdLineUsername, MAX_LOGIN_INFO_STRLEN, "%s", username);
}

/*
 * Returns the login response information structure
 */
RsslLoginResponseInfo* getLoginResponseInfo()
{
	return &loginResponseInfo;
}

/*
 * Sends a login request to a channel.  This consists of getting
 * a message buffer, setting the login request information, encoding
 * the login request, and sending the login request to the server.
 * Returns success if send login request succeeds or failure if it fails.
 * chnl - The channel to send a login request to
 * appName - The application name of the login request
 * role - The role of the login request
 * loginSuccessCB - The login success callback function pointer
 */
RsslRet sendLoginRequest(RsslChannel* chnl, const char *appName, RsslUInt64 role, LoginSuccessCallback loginSuccessCB)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;
	RsslLoginRequestInfo loginReqInfo;
	RsslUInt32 ipAddress = 0;
	char userName[256], hostName[256], positionStr[32];
	RsslBuffer userNameBuf, hostNameBuf;

	/* initialize login request info to default values */
	initLoginReqInfo(&loginReqInfo);

	/* set login success callback function */
	loginSuccessCallback = loginSuccessCB;

	/* get a buffer for the login request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* provide login request information */
		/* StreamId */
		loginReqInfo.StreamId = LOGIN_STREAM_ID;
		/* Username */
		if (strlen(cmdLineUsername) == 0) /* no command line username */
		{
			userNameBuf.data = userName;
			userNameBuf.length = sizeof(userName);
			if (rsslGetUserName(&userNameBuf) == RSSL_RET_SUCCESS)
			{
				snprintf(loginReqInfo.Username, 128, "%s", userName);
			}
			else
			{
				snprintf(loginReqInfo.Username, 128, "%s", defaultUsername);
			}
		}
		else /* use command line username */
		{
			snprintf(loginReqInfo.Username, 128, "%s", cmdLineUsername);
		}
		/* ApplicationId */
		snprintf(loginReqInfo.ApplicationId, 128, "%s", applicationId);
		/* ApplicationName */
		snprintf(loginReqInfo.ApplicationName, 128, "%s", appName);
		/* Position */
		if (gethostname(hostName, sizeof(hostName)) != 0)
		{
			snprintf(hostName, 256, "localhost");
		}
		hostNameBuf.data = hostName;
		hostNameBuf.length = sizeof(hostName);
		if (rsslHostByName(&hostNameBuf, &ipAddress) == RSSL_RET_SUCCESS)
		{
			rsslIPAddrUIntToString(ipAddress, positionStr);
			strcat(positionStr, "/net");
			snprintf(loginReqInfo.Position, 128, "%s", positionStr);
		}
		else
		{
			snprintf(loginReqInfo.Position, 128, "localhost");
		}
		/* Password */
		snprintf(loginReqInfo.Password, 128, "%s", pword);
		/* InstanceId */
		snprintf(loginReqInfo.InstanceId, 128, "%s", instanceId);
		/* if provider, change role and single open from default values */
		if (role == RSSL_PROVIDER)
		{
			/* this provider does not support SingleOpen behavior */
			loginReqInfo.SingleOpen = 0;
			/* provider role */
			loginReqInfo.Role = RSSL_PROVIDER; 
		}
		/* keep default values for all others */

		/* encode login request */
		if ((ret = encodeLoginRequest(chnl, &loginReqInfo, msgBuf)) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeLoginRequest() failed with return code: %d\n", ret);
			return ret;
		}

		/* send login request */
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
 * Processes a login response.  This consists of calling
 * decodeLoginResponse() to decode the response and calling
 * loginSuccessCallback() if login is successful.  Returns
 * success if process login response succeeds or failure
 * if it fails.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processLoginResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslState *pState = 0;
	char tempData[1024];
	RsslBuffer tempBuffer;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
		/* first clear response info structure */
		clearLoginRespInfo(&loginResponseInfo);
		/* decode login response */
		if ((ret = decodeLoginResponse(&loginResponseInfo, msg, dIter)) != RSSL_RET_SUCCESS)
		{
			printf("\ndecodeLoginResponse() failed with return code: %d\n", ret);
			return ret;
		}

		printf("\nReceived Login Response for Username: %s\n", loginResponseInfo.Username);

		if (loginResponseInfo.SupportProviderDictionaryDownload)
			isProvDicDownloadSupported = RSSL_TRUE;

		/* get state information */
		pState = &msg->refreshMsg.state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		/* call login success callback if login okay and is solicited */
		if (loginResponseInfo.isSolicited && pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_OK)
		{
			isClosed = RSSL_FALSE;
			isClosedRecoverable = RSSL_FALSE;
			isSuspect = RSSL_FALSE;
			loginSuccessCallback(chnl);
		}
		else /* handle error cases */
		{
			if (pState->streamState == RSSL_STREAM_CLOSED_RECOVER)
			{
				printf("\nLogin stream is closed recover\n");
				isClosedRecoverable = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (pState->streamState == RSSL_STREAM_CLOSED)
			{
				printf("\nLogin attempt failed (stream closed)\n");
				isClosed = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_SUSPECT)
			{
				printf("\nLogin stream is suspect\n");
				isSuspect = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
		}
		break;

	case RSSL_MC_STATUS:
		printf("\nReceived Login StatusMsg\n");
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    	{
			/* get state information */
			pState = &msg->statusMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
			/* handle error cases */
			if (pState->streamState == RSSL_STREAM_CLOSED_RECOVER)
			{
				printf("\nLogin stream is closed recover\n");
				isClosedRecoverable = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (pState->streamState == RSSL_STREAM_CLOSED)
			{
				printf("\nLogin stream is closed\n");
				isClosed = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
			else if (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_SUSPECT)
			{
				printf("\nLogin stream is suspect\n");
				isSuspect = RSSL_TRUE;
				return RSSL_RET_FAILURE;
			}
    	}
		break;

	case RSSL_MC_UPDATE:
		printf("\nReceived Login Update\n");
		break;

	case RSSL_MC_CLOSE:
		printf("\nReceived Login Close\n");
		isClosed = RSSL_TRUE;
		return RSSL_RET_FAILURE;
		break;

	default:
		printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
		return RSSL_RET_FAILURE;
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Close the login stream.  Note that closing login stream
 * will automatically close all other streams at the provider.
 * Returns success if close login stream succeeds or failure
 * if it fails.
 * chnl - The channel to send a login close to
 */
RsslRet closeLoginStream(RsslChannel* chnl)
{
	RsslRet ret;
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the login close */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode login close */
		if ((ret = encodeLoginClose(chnl, msgBuf, LOGIN_STREAM_ID)) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeLoginClose() failed with return code: %d\n", ret);
			return ret;
		}

		/* send close */
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
