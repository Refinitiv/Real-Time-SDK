/*
 * This file contains all login encode and decode functions required
 * by the rsslConsumer, rsslProvider, and rsslNIProvider applications.
 */

#include "rsslLoginEncodeDecode.h"
#include "rsslSendMessage.h"
#include "rtr/rsslGetTime.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

static char *blankUserName = "\0";

/*
 * Encodes the login request.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * chnl - The channel to send a login request to
 * loginReqInfo - The login request information to be encoded
 * msgBuf - The message buffer to encode the login request into
 */
RsslRet encodeLoginRequest(RsslChannel* chnl, RsslLoginRequestInfo* loginReqInfo, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslMsgKey key = RSSL_INIT_MSG_KEY;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer applicationId, applicationName, position, password, instanceId, authenticationToken, authenticationExtended;
	RsslEncodeIterator encodeIter;
	RsslUInt tmp;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = loginReqInfo->StreamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_STREAMING;


	/* set msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME;
	/* Username */
	/* If the nameType is USER_AUTHN_TOKEN, set the msgKey.name buffer to one character of '/0' */
	if(loginReqInfo->NameType == RDM_LOGIN_USER_AUTHN_TOKEN)
	{
		msg.msgBase.msgKey.name.data = (char*)blankUserName;
		msg.msgBase.msgKey.name.length = 1;
		msg.msgBase.msgKey.nameType = RDM_LOGIN_USER_AUTHN_TOKEN;
	}
	else
	{	
		/* Otherwise, encode the userName as normal */
		msg.msgBase.msgKey.name.data =  (char*)loginReqInfo->Username;
		msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(loginReqInfo->Username);
		msg.msgBase.msgKey.nameType = RDM_LOGIN_USER_NAME;
	}
	msg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;


	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	/* since our msgKey has opaque that we want to encode, we need to use rsslEncodeMsgInit */
	/* rsslEncodeMsgInit should return and inform us to encode our key opaque */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* encode our msgKey opaque */
	/* encode the element list */
	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* If the user nameType is RDM_LOGIN_USER_AUTHN_TOKEN, encode the authentication token information */
	if(loginReqInfo->NameType == RDM_LOGIN_USER_AUTHN_TOKEN)
	{
		authenticationToken.data = (char*)loginReqInfo->AuthenticationToken;
		authenticationToken.length = (RsslUInt32)strlen(loginReqInfo->AuthenticationToken);
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_AUTHN_TOKEN;
		if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &authenticationToken)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
		
		/* Encode the authentication extended information, if present */
		if(strlen(loginReqInfo->AuthenticationExtended) != 0)
		{
			authenticationExtended.data = (char*)loginReqInfo->AuthenticationExtended;
			authenticationExtended.length = (RsslUInt32)strlen(loginReqInfo->AuthenticationExtended);
			element.dataType = RSSL_DT_BUFFER;
			element.name = RSSL_ENAME_AUTHN_EXTENDED;
			if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &authenticationExtended)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}

	/* ApplicationId */
	applicationId.data = (char*)loginReqInfo->ApplicationId;
	applicationId.length = (RsslUInt32)strlen(loginReqInfo->ApplicationId);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPID;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &applicationId)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ApplicationName */
	applicationName.data = (char*)loginReqInfo->ApplicationName;
	applicationName.length = (RsslUInt32)strlen(loginReqInfo->ApplicationName);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPNAME;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &applicationName)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Position */
	position.data = (char*)loginReqInfo->Position;
	position.length = (RsslUInt32)strlen(loginReqInfo->Position);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_POSITION;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &position)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Password */
	password.data = (char*)loginReqInfo->Password;
	password.length = (RsslUInt32)strlen(loginReqInfo->Password);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_PASSWORD;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &password)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ProvidePermissionProfile */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_PROV_PERM_PROF;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->ProvidePermissionProfile)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ProvidePermissionExpressions */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_PROV_PERM_EXP;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->ProvidePermissionExpressions)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SingleOpen */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SINGLE_OPEN;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->SingleOpen)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* AllowSuspectData */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_ALLOW_SUSPECT_DATA;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->AllowSuspectData)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* InstanceId */
	instanceId.data = (char*)loginReqInfo->InstanceId;
	instanceId.length = (RsslUInt32)strlen(loginReqInfo->InstanceId);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_INST_ID;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &instanceId)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Role */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_ROLE;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->Role)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* DownloadConnectionConfig */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_DOWNLOAD_CON_CONFIG;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginReqInfo->DownloadConnectionConfig)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* RTT support */
	if (loginReqInfo->RTT == RSSL_TRUE)
	{
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_RTT;
		tmp = RDM_LOGIN_RTT_ELEMENT;
		if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &tmp)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	   for us to encode our container/msg payload */
	if ((ret = rsslEncodeMsgKeyAttribComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgKeyAttribComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the login request into the RsslLoginRequestInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * loginReqInfo - The login request information structure
 * msg - The partially decoded message
 * dIter - The decode iterator
 * requestKey - The request key with the user name
 */
RsslRet decodeLoginRequest(RsslLoginRequestInfo* loginReqInfo, RsslMsg* msg, RsslDecodeIterator* dIter, RsslMsgKey* requestKey)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslUInt tmp;

	/* get StreamId */
	loginReqInfo->StreamId = msg->requestMsg.msgBase.streamId;

	/* get Username */
	if (requestKey->name.length < MAX_LOGIN_INFO_STRLEN)
	{
		strncpy(loginReqInfo->Username, requestKey->name.data, requestKey->name.length);
		loginReqInfo->Username[requestKey->name.length] = '\0';
	}
	else
	{
		strncpy(loginReqInfo->Username, requestKey->name.data, MAX_LOGIN_INFO_STRLEN - 1);
		loginReqInfo->Username[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
	}

	/* decode key opaque data */
	if ((ret = rsslDecodeMsgKeyAttrib(dIter, requestKey)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMsgKeyAttrib() failed with return code: %d\n", ret);
		return ret;
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
				/* Authentication Token.  For the Authentication token, the application should take additional steps to verify the token(the token introspection step in oauth2, for example)
				 * In this case, we are just accepting the login. */
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_TOKEN))
				{
					if (element.encData.length < AUTH_TOKEN_LENGTH)
					{
						strncpy(loginReqInfo->AuthenticationToken, element.encData.data, element.encData.length);
						loginReqInfo->AuthenticationToken[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->AuthenticationToken, element.encData.data, AUTH_TOKEN_LENGTH - 1);
						loginReqInfo->AuthenticationToken[AUTH_TOKEN_LENGTH - 1] = '\0';
					}
				}
				/* Authentication Extended */
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_EXTENDED))
				{
					if (element.encData.length < AUTH_TOKEN_LENGTH)
					{
						strncpy(loginReqInfo->AuthenticationToken, element.encData.data, element.encData.length);
						loginReqInfo->AuthenticationToken[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->AuthenticationToken, element.encData.data, AUTH_TOKEN_LENGTH - 1);
						loginReqInfo->AuthenticationToken[AUTH_TOKEN_LENGTH - 1] = '\0';
					}
				}
				
				/* ApplicationId */
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPID))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginReqInfo->ApplicationId, element.encData.data, element.encData.length);
						loginReqInfo->ApplicationId[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->ApplicationId, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginReqInfo->ApplicationId[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* ApplicationName */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginReqInfo->ApplicationName, element.encData.data, element.encData.length);
						loginReqInfo->ApplicationName[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->ApplicationName, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginReqInfo->ApplicationName[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* Position */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginReqInfo->Position, element.encData.data, element.encData.length);
						loginReqInfo->Position[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->Position, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginReqInfo->Position[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* Password */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PASSWORD))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginReqInfo->Password, element.encData.data, element.encData.length);
						loginReqInfo->Password[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->Password, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginReqInfo->Password[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* ProvidePermissionProfile */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_PROF))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->ProvidePermissionProfile);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* ProvidePermissionExpressions */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_EXP))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->ProvidePermissionExpressions);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SingleOpen */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SINGLE_OPEN))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->SingleOpen);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportProviderDictionaryDownload */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->SupportProviderDictionaryDownload);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* AllowSuspectData */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ALLOW_SUSPECT_DATA))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->AllowSuspectData);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* InstanceId */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_INST_ID))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginReqInfo->InstanceId, element.encData.data, element.encData.length);
						loginReqInfo->InstanceId[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginReqInfo->InstanceId, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginReqInfo->InstanceId[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* Role */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ROLE))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->Role);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* DownloadConnectionConfig */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DOWNLOAD_CON_CONFIG))
				{
					ret = rsslDecodeUInt(dIter, &loginReqInfo->DownloadConnectionConfig);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT))
				{
					ret = rsslDecodeUInt(dIter, &tmp);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					
					if (tmp == 2)
						loginReqInfo->RTT = RSSL_TRUE;
					else
						loginReqInfo->RTT = RSSL_FALSE;

				}
			}
			else
			{
				printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the login response.  Returns success if encoding
 * succeeds or failure if encoding fails.
 * chnl - The channel to send a login response to
 * loginRespInfo - The login response information to be encoded
 * msgBuf - The message buffer to encode the login response into
 */
RsslRet encodeLoginResponse(RsslChannel* chnl, RsslLoginResponseInfo* loginRespInfo, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslRefreshMsg msg = RSSL_INIT_REFRESH_MSG;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer applicationId, applicationName, position;
	RsslUInt tmp;
	char hostName[256], stateText[MAX_LOGIN_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REFRESH;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;
	msg.state.streamState = RSSL_STREAM_OPEN;
	msg.state.dataState = RSSL_DATA_OK;
	msg.state.code = RSSL_SC_NONE;
	snprintf(stateText, 128, "Login accepted by host ");
	if (gethostname(hostName, sizeof(hostName)) != 0)
	{
		snprintf(hostName, 256, "localhost");
	}
	strcat(stateText, hostName);
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

	/* StreamId */
	msg.msgBase.streamId = loginRespInfo->StreamId;

	/* set msgKey members */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME;
	/* Username */
	msg.msgBase.msgKey.name.data = loginRespInfo->Username;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(loginRespInfo->Username);
	msg.msgBase.msgKey.nameType = RDM_LOGIN_USER_NAME;
	msg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* encode message */
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	/* since our msgKey has opaque that we want to encode, we need to use rsslEncodeMsgInit */
	/* rsslEncodeMsgInit should return and inform us to encode our key opaque */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* encode our msgKey opaque */
	/* encode the element list */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}
	/* ApplicationId */
	applicationId.data = (char*)loginRespInfo->ApplicationId;
	applicationId.length = (RsslUInt32)strlen(loginRespInfo->ApplicationId);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPID;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &applicationId)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ApplicationName */
	applicationName.data = (char*)loginRespInfo->ApplicationName;
	applicationName.length = (RsslUInt32)strlen(loginRespInfo->ApplicationName);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_APPNAME;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &applicationName)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* Position */
	position.data = (char*)loginRespInfo->Position;
	position.length = (RsslUInt32)strlen(loginRespInfo->Position);
	element.dataType = RSSL_DT_ASCII_STRING;
	element.name = RSSL_ENAME_POSITION;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &position)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ProvidePermissionProfile */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_PROV_PERM_PROF;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->ProvidePermissionProfile)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* ProvidePermissionExpressions */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_PROV_PERM_EXP;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->ProvidePermissionExpressions)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SingleOpen */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SINGLE_OPEN;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SingleOpen)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* AllowSuspectData */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_ALLOW_SUSPECT_DATA;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->AllowSuspectData)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportPauseResume */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_PR;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportPauseResume)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportOptimizedPauseResume */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_OPR;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportOptimizedPauseResume)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportOMMPost */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_POST;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportOMMPost)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportViewRequests */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_VIEW;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportViewRequests)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportBatchRequests */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_BATCH;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportBatchRequests)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}
	/* SupportStandby */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SUPPORT_STANDBY;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &loginRespInfo->SupportStandby)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* RTT support */
	if (loginRespInfo->RTT == RSSL_TRUE)
	{
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_RTT;
		tmp = RDM_LOGIN_RTT_ELEMENT;
		if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &tmp)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	   for us to encode our container/msg payload */
	if ((ret = rsslEncodeMsgKeyAttribComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgKeyAttribComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the login response into the RsslLoginResponseInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * loginRespInfo - The login response information structure
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet decodeLoginResponse(RsslLoginResponseInfo* loginRespInfo, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslMsgKey* key = 0;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslUInt tmp;

	/* set stream id */
	loginRespInfo->StreamId = msg->msgBase.streamId;

	/* set isSolicited flag if a solicited refresh */
	if (msg->msgBase.msgClass == RSSL_MC_REFRESH)
	{
		if (msg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
		{
			loginRespInfo->isSolicited = RSSL_TRUE;
		}
	}

	/* get key */
	key = (RsslMsgKey *)rsslGetMsgKey(msg);

	/* get Username */
	if (key)
	{
		if (key->name.length < MAX_LOGIN_INFO_STRLEN)
		{
			strncpy(loginRespInfo->Username, key->name.data, key->name.length);
			loginRespInfo->Username[key->name.length] = '\0';
		}
		else
		{
			strncpy(loginRespInfo->Username, key->name.data, MAX_LOGIN_INFO_STRLEN - 1);
			loginRespInfo->Username[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
		}
	}
	else 
	{
		loginRespInfo->Username[0] = '\0';
	}

	/* decode key opaque data */
	if ((ret = rsslDecodeMsgKeyAttrib(dIter, key)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMsgKeyAttrib() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
	{
		/* decode each element entry in list */
		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret == RSSL_RET_SUCCESS)
			{
				/* get login response information */
				/* AuthenticationTTReissue */
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_TT_REISSUE))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->AuthenticationTTReissue);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* AuthenticationExtendedResponse */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_EXTENDED_RESP))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginRespInfo->AuthenticationExtendedResponse, element.encData.data, element.encData.length);
						loginRespInfo->AuthenticationExtendedResponse[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginRespInfo->AuthenticationExtendedResponse, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginRespInfo->AuthenticationExtendedResponse[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* AuthenticationStatusErrorText */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_TEXT))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginRespInfo->AuthenticationStatusErrorText, element.encData.data, element.encData.length);
						loginRespInfo->AuthenticationStatusErrorText[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginRespInfo->AuthenticationStatusErrorText, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginRespInfo->AuthenticationStatusErrorText[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* AuthenticationStatusErrorCode */
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_CODE))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->AuthenticationStatusErrorCode);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					loginRespInfo->HasAuthenticationStatusErrorCode = RSSL_TRUE;
				}
				/* AllowSuspectData */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ALLOW_SUSPECT_DATA))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->AllowSuspectData);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* ApplicationId */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPID))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginRespInfo->ApplicationId, element.encData.data, element.encData.length);
						loginRespInfo->ApplicationId[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginRespInfo->ApplicationId, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginRespInfo->ApplicationId[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* ApplicationName */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginRespInfo->ApplicationName, element.encData.data, element.encData.length);
						loginRespInfo->ApplicationName[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginRespInfo->ApplicationName, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginRespInfo->ApplicationName[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* Position */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
				{
					if (element.encData.length < MAX_LOGIN_INFO_STRLEN)
					{
						strncpy(loginRespInfo->Position, element.encData.data, element.encData.length);
						loginRespInfo->Position[element.encData.length] = '\0';
					}
					else
					{
						strncpy(loginRespInfo->Position, element.encData.data, MAX_LOGIN_INFO_STRLEN - 1);
						loginRespInfo->Position[MAX_LOGIN_INFO_STRLEN - 1] = '\0';
					}
				}
				/* ProvidePermissionExpressions */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_EXP))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->ProvidePermissionExpressions);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* ProvidePermissionProfile */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_PROF))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->ProvidePermissionProfile);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SingleOpen */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SINGLE_OPEN))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SingleOpen);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportOMMPost */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_POST))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportOMMPost);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportPauseResume */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_PR))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportPauseResume);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportStandby */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_STANDBY))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportStandby);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportProviderDictionaryDownload */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportProviderDictionaryDownload);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportBatchRequests */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_BATCH))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportBatchRequests);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportViewRequests */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_VIEW))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportViewRequests);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* SupportOptimizedPauseResume */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_OPR))
				{
					ret = rsslDecodeUInt(dIter, &loginRespInfo->SupportOptimizedPauseResume);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				/* RTT */
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT))
				{
					ret = rsslDecodeUInt(dIter, &tmp);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}

					if (tmp == 2)
						loginRespInfo->RTT = RSSL_TRUE;
					else
						loginRespInfo->RTT = RSSL_FALSE;

				}

			}
			else
			{
				printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Encodes the login close.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a login close to
 * msgBuf - The message buffer to encode the login close into
 * streamId - The stream id of the login close
 */
RsslRet encodeLoginClose(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslCloseMsg msg = RSSL_INIT_CLOSE_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_CLOSE;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	
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

/*
 * Encodes the login close status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send close status message to
 * msgBuf - The message buffer to encode the login close status into
 * streamId - The stream id of the login close status
 */
RsslRet encodeLoginCloseStatus(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_LOGIN_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	msg.state.code = RSSL_SC_NONE;
	snprintf(stateText, 128, "Login stream closed");
	msg.state.text.data = stateText;
	msg.state.text.length = (RsslUInt32)strlen(stateText);

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

/*
 * Encodes the login request reject status.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send request reject status message to
 * streamId - The stream id of the request
 * reason - The reason for the reject
 * msgBuf - The message buffer to encode the login request reject into
 */
RsslRet encodeLoginRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslLoginRejectReason reason, RsslBuffer* msgBuf)
{
	RsslRet ret = 0;
	RsslStatusMsg msg = RSSL_INIT_STATUS_MSG;
	char stateText[MAX_LOGIN_INFO_STRLEN];
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_STATUS;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_STMF_HAS_STATE;
	msg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	msg.state.dataState = RSSL_DATA_SUSPECT;
	switch(reason)
	{
	case MAX_LOGIN_REQUESTS_REACHED:
		msg.state.code = RSSL_SC_TOO_MANY_ITEMS;
		snprintf(stateText, 128, "Login request rejected for stream id %d - max request count reached", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	case NO_USER_NAME_IN_REQUEST:
		msg.state.code = RSSL_SC_USAGE_ERROR;
		snprintf(stateText, 128, "Login request rejected for stream id %d - request does not contain user name", streamId);
		msg.state.text.data = stateText;
		msg.state.text.length = (RsslUInt32)strlen(stateText) + 1;
		break;
	default:
		break;
	}

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

/* Encodes the server side RTT message. Rturns success if
 * encoding succeeds or failure if encoding fails. 
 * chnl - The channel to send request reject status message to
 * msgBuf - The message buffer to encode the login request reject into
 * streamId - The stream id of the request
 * lastLatency - the latency, in microseconds, of the last RTT message response.  If this is 0, no latency will be encoded.
 */
RsslRet encodeLoginRTTServer(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslUInt lastLatency)
{
	RsslRet ret = 0;
	RsslGenericMsg msg = RSSL_INIT_GENERIC_MSG;
	RsslEncodeIterator encodeIter;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslChannelStats stats;
	RsslTimeValue ticks;
	RsslError error;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);
	
	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_GENERIC;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	msg.flags = RSSL_GNMF_PROVIDER_DRIVEN;

	if ((ret = rsslGetChannelStats(chnl, &stats, &error)) != RSSL_RET_SUCCESS)
	{
		printf("rsslGetChannelStats() failed with return code: %d\n", ret);
		return ret;
	}

	/* Get the current ticks. */
	ticks = rsslGetTicks();

	/* encode message */
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	/* since we have a payload we want to encode, we need to use rsslEncodeMsgInit */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}

	rsslClearElementEntry(&element);
	element.name = RSSL_ENAME_RTT_TICKS;
	element.dataType = RSSL_DT_UINT;

	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, (RsslUInt*)&ticks)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	rsslClearElementEntry(&element);
	element.name = RSSL_ENAME_RTT_TCP_RETRANS;
	element.dataType = RSSL_DT_UINT;

	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &stats.tcpStats.tcpRetransmitCount)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	if (lastLatency != 0)
	{
		rsslClearElementEntry(&element);
		element.name = RSSL_ENAME_RTT;
		element.dataType = RSSL_DT_UINT;

		if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &lastLatency)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/* Encodes the and sends the client side RTT message. Rturns success if
* encoding succeeds or failure if encoding fails.
* chnl - The channel to send request reject status message to
* streamId - The stream id of the request
* ticks - ticks received from the server
*/
RsslRet encodeAndSendLoginRTTClient(RsslChannel* chnl, RsslInt32 streamId, RsslUInt ticks)
{
	RsslRet ret = 0;
	RsslGenericMsg msg = RSSL_INIT_GENERIC_MSG;
	RsslEncodeIterator encodeIter;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslChannelStats stats;
	RsslBuffer* msgBuf;
	RsslError error;
	RsslWriteInArgs args;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);
	rsslClearWriteInArgs(&args);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_GENERIC;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_LOGIN;
	msg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
	msg.flags = RSSL_GNMF_PROVIDER_DRIVEN;

	if((msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("rsslGetBuffer() failed with return code: %d\n", error.rsslErrorId);
		return error.rsslErrorId;
	}

	if (ret = rsslGetChannelStats(chnl, &stats, &error))
	{
		printf("rsslGetChannelStats() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode message */
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	/* since we have a payload we want to encode, we need to use rsslEncodeMsgInit */
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}

	rsslClearElementEntry(&element);
	element.name = RSSL_ENAME_RTT_TICKS;
	element.dataType = RSSL_DT_UINT;

	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, (RsslUInt*)&ticks)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	if (stats.tcpStats.flags &= RSSL_TCP_STATS_RETRANSMIT)
	{
		rsslClearElementEntry(&element);
		element.name = RSSL_ENAME_RTT_TCP_RETRANS;
		element.dataType = RSSL_DT_UINT;

		if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &stats.tcpStats.tcpRetransmitCount)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	if((ret = sendMessageEx(chnl, msgBuf, &args))  == RSSL_RET_FAILURE)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

/*
* Decodes the login RTT message for the server side;
* Returns success if decoding succeeds or failure if decoding fails.
* loginRespInfo - The login response information structure
* msg - The partially decoded message
* dIter - The decode iterator
* latency - this sill be filled with the current message's round trip latency
*/
RsslRet decodeLoginRTTForServer(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter, RsslUInt* latency)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslUInt tmp;
	RsslInt32 streamId;
	RsslUInt oldTicks;
	RsslTimeValue currTicks;

	/* get StreamId */
	streamId = msg->genericMsg.msgBase.streamId;

	printf("Received login RTT message from Consumer "SOCKET_PRINT_TYPE".\n", chnl->socketId);

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
	{
		/* decode each element entry in list */
		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret == RSSL_RET_SUCCESS)
			{
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT_TICKS))
				{
					ret = rsslDecodeUInt(dIter, &oldTicks);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					printf("\tRTT Tick value is " RTR_LLU "us.\n", oldTicks);
				}
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT_TCP_RETRANS))
				{
					ret = rsslDecodeUInt(dIter, &tmp);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					printf("\tConsumer side TCP retransmissions: " RTR_LLU "\n", tmp);

				}
			}
			else
			{
				printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	currTicks = rsslGetTicks();

	*latency = (RsslUInt)(((RsslDouble)currTicks - (RsslDouble)oldTicks) / rsslGetTicksPerMicro());

	printf("\tLast RTT message latency is " RTR_LLU "us.\n", *latency);

	return RSSL_RET_SUCCESS;
}

/*
* Decodes the login RTT message for the client side.  When decoding is complete, it will attempt to send an RTT response immediately.
* Returns success if decoding succeeds or failure if decoding fails.
* loginRespInfo - The login response information structure
* msg - The partially decoded message
* dIter - The decode iterator
*/
RsslRet decodeLoginRTTForClient(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslUInt tmp;
	RsslInt32 streamId;
	RsslUInt ticks;

	/* get StreamId */
	streamId = msg->genericMsg.msgBase.streamId;

	printf("Received login RTT message from Provider "SOCKET_PRINT_TYPE".\n", chnl->socketId);

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) == RSSL_RET_SUCCESS)
	{
		/* decode each element entry in list */
		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret == RSSL_RET_SUCCESS)
			{
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT_TICKS))
				{
					ret = rsslDecodeUInt(dIter, &ticks);
					printf("\tTicks: %llu\n", ticks);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
				}
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT_TCP_RETRANS))
				{
					ret = rsslDecodeUInt(dIter, &tmp);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					printf("\tProvider side TCP Retransmissions: %llu\n", tmp);

				}
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT))
				{
					ret = rsslDecodeUInt(dIter, &tmp);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeUInt() failed with return code: %d\n", ret);
						return ret;
					}
					printf("\tLast Latency: %llu\n", tmp);

				}
			}
			else
			{
				printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
				return ret;
			}
		}
	}
	else
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	printf("Sending RTT response to server.\n");

	encodeAndSendLoginRTTClient(chnl, streamId, ticks);

	return RSSL_RET_SUCCESS;
}
