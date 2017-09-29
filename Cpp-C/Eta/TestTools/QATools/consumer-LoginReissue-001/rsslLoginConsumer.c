
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

/* authentication token requested by application */
static char cmdLineAuthenticationToken[255];

/* authentication extended requested by application */
static char cmdLineAuthenticationExtended[MAX_LOGIN_INFO_STRLEN];

/* applicationId requested by application */
static char cmdLineApplicationId[MAX_LOGIN_INFO_STRLEN];

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
 * Sets the authentication token requested by the application.
 * authenticationToken - The token requested by the application
 */
void setAuthenticationToken(char* authenticationToken)
{
	snprintf(cmdLineAuthenticationToken, 255, "%s", authenticationToken);
}

/*
 * Sets the authentication extended information requested by the application.
 * authenticationExtended - The extended information requested by the application
 */
void setAuthenticationExtended(char* authenticationExtended)
{
	snprintf(cmdLineAuthenticationExtended, 255, "%s", authenticationExtended);
}

/*
 * Sets the application Id information requested by the application.
 * applicationId - The application Id requested by the application
 */
void setApplicationId(char* applicationId)
{
	snprintf(cmdLineApplicationId, 255, "%s", applicationId);
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
// APIQA:  Adding a flag to add PAUSE flag accordingly
RsslRet sendLoginRequest(RsslChannel* chnl, const char *appName, RsslUInt64 role, LoginSuccessCallback loginSuccessCB, int shouldPause)
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
		/* If an authentication token is present, set the login request's name type to RDM_LOGIN_USER_AUTHN_TOKEN and use the authentication token */
		if(strlen(cmdLineAuthenticationToken) != 0)
		{
			snprintf(loginReqInfo.AuthenticationToken, 255, "%s", cmdLineAuthenticationToken);
			loginReqInfo.NameType = RDM_LOGIN_USER_AUTHN_TOKEN;
			
			if(strlen(cmdLineAuthenticationExtended) != 0)
			{
				snprintf(loginReqInfo.AuthenticationExtended, 255, "%s", cmdLineAuthenticationExtended);
			}
		}
		/* If the application Id is present, set on the request */
		if(strlen(cmdLineApplicationId) != 0)
		{
			snprintf(loginReqInfo.ApplicationId, 128, "%s", cmdLineApplicationId);
		}
		else /* Use default AppId */
		{
			snprintf(loginReqInfo.ApplicationId, 128, "%s", applicationId);
		}
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
		// APIQA:  Adding a boolean to add PAUSE flag accordingly
		if ((ret = encodeLoginRequest(chnl, &loginReqInfo, msgBuf, shouldPause)) != RSSL_RET_SUCCESS)
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
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslInt code;

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
		printf("	%.*s\n", tempBuffer.length, tempBuffer.data);
		
		if(strlen(loginResponseInfo.AuthenticationExtendedResponse) != 0)
		{
			printf("	Authentication Extended Response: %s\n", loginResponseInfo.AuthenticationExtendedResponse);
		}
		
		if(loginResponseInfo.AuthenticationTTReissue != 0)
		{
			printf("	Authentication Time To Reissue: " RTR_LLU "\n", loginResponseInfo.AuthenticationTTReissue);
		}
		if(loginResponseInfo.HasAuthenticationStatusErrorCode == RSSL_TRUE)
		{
			printf("	Authentication Error Code: " RTR_LLD "\n", loginResponseInfo.AuthenticationStatusErrorCode);
		}
		if(strlen(loginResponseInfo.AuthenticationStatusErrorText) != 0)
		{
			printf("	Authentication Status Text: %s\n", loginResponseInfo.AuthenticationStatusErrorText);
		}
		printf("\n");

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
			printf("	%.*s\n", tempBuffer.length, tempBuffer.data);
			
			/* Check for authentication attrib information */
			if(msg->statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB)
			{
				
				/* decode key opaque data */
				if ((ret = rsslDecodeMsgKeyAttrib(dIter, &msg->statusMsg.msgBase.msgKey)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeMsgKeyAttrib() failed with return code: %d\n", ret);
					return RSSL_RET_FAILURE;
				}

				/* decode element list */
				if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
				{
					/* decode each element entry in list */
					while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
					{
						if (ret == RSSL_RET_SUCCESS)
						{
							/* get login request information */
							/* Authentication Error code */
							if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_CODE))
							{
								ret = rsslDecodeUInt(dIter, &code);
								if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
								{
									printf("rsslDecodeUInt() failed with return code: %d\n", ret);
									return RSSL_RET_FAILURE;
								}
								printf("	Authentication Error Code: %llu\n", code);
							}
							/* Authentication Error Text */
							else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_TEXT))
							{
								printf("	Authentication Error Code: %.*s\n", element.encData.length, element.encData.data);
							}
						}
						else
						{
							printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
							return RSSL_RET_FAILURE;
						}
					}
				}
			}
			
			printf("\n");
							
				
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
