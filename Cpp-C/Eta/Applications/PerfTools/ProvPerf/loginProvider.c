/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#include "loginProvider.h"
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#endif

/* Contains the global login configuration. */
LoginConfig loginConfig;

void clearLoginConfig()
{
	rsslClearBuffer(&loginConfig.applicationName);
	rsslClearBuffer(&loginConfig.applicationId);
	rsslClearBuffer(&loginConfig.position);
}

void setLoginConfigPosition()
{
	RsslUInt32 ipAddress = 0;
	char tmpHostNameBlock[256];
	RsslBuffer tmpHostName = { 256, tmpHostNameBlock };

	/* Populate provider position. */
	loginConfig.position.data = loginConfig.positionChar;
	loginConfig.position.length = sizeof(loginConfig.positionChar);

	if (gethostname(tmpHostNameBlock, sizeof(tmpHostNameBlock)) != 0)
		snprintf(tmpHostNameBlock, 256, "localhost");

	if (rsslHostByName(&tmpHostName, &ipAddress) == RSSL_RET_SUCCESS)
	{
		rsslIPAddrUIntToString(ipAddress, tmpHostNameBlock);
		snprintf(loginConfig.position.data, loginConfig.position.length, "%s/net", tmpHostNameBlock);
		loginConfig.position.length = (RsslUInt32)strlen(loginConfig.position.data);
	}
	else
	{
		snprintf(loginConfig.position.data, loginConfig.position.length, "localhost");
		loginConfig.position.length = (RsslUInt32)strlen(loginConfig.position.data);
	}
}

RsslRet processLoginRequest(ChannelHandler *pChannelHandler, ChannelInfo* pChannelInfo, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslRDMLoginMsg loginMsg;
	char loginMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, loginMsgChar };
	RsslErrorInfo errorInfo;

	if ((ret = rsslDecodeRDMLoginMsg(dIter, msg, &loginMsg, &memoryBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeRDMLoginMsg() failed: %d(%s)\n", ret, errorInfo.rsslError.text);
		return ret;
	}

	switch(loginMsg.rdmMsgBase.rdmMsgType)
	{
	case RDM_LG_MT_REQUEST:
	{
		RsslError error;
		RsslBuffer* msgBuf = 0;
		RsslRet ret;
		RsslChannel *pChannel = pChannelInfo->pChannel;
		RsslEncodeIterator eIter;
		RsslRDMLoginRefresh loginRefresh;

		printf(	"Received login request.\n"
				"  Username: %.*s\n",
				loginMsg.request.userName.length, loginMsg.request.userName.data);
		if (loginMsg.request.flags & RDM_LG_RQF_HAS_APPLICATION_NAME)
			printf("  ApplicationName: %.*s\n",
					loginMsg.request.applicationName.length, loginMsg.request.applicationName.data);
		printf("\n");

		if (loginMsg.request.flags & RDM_LG_RQF_HAS_ROLE && loginMsg.request.role != RDM_LOGIN_ROLE_CONS)
			printf("Warning: Connected client did not identify itself as a consumer.\n");

		/* get a buffer for the login response */
		if ((msgBuf = rsslGetBuffer(pChannel, 512, RSSL_FALSE, &error)) == NULL)
		{
			printf("sendLoginResponse(): rsslGetBuffer() failed: %d(%s)",
					error.rsslErrorId, error.text);
			return error.rsslErrorId;
		}

		rsslClearRDMLoginRefresh(&loginRefresh);

		/* provide login response information */
		/* StreamId */
		loginRefresh.rdmMsgBase.streamId = loginMsg.rdmMsgBase.streamId;

		/* Username */
		loginRefresh.userName = loginMsg.request.userName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;

		/* ApplicationId */
		loginRefresh.applicationId = loginConfig.applicationId;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;

		/* ApplicationName */
		loginRefresh.applicationName = loginConfig.applicationName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;

		/* Position */
		loginRefresh.position = loginConfig.position;
		loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;

		loginRefresh.singleOpen = 0; /* this provider does not support SingleOpen behavior */
		loginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;

		loginRefresh.supportOMMPost = 1;
		loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;

		loginRefresh.flags |= RDM_LG_RFF_SOLICITED;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMLoginMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		/* send login response */
		return channelHandlerWriteChannel(pChannelHandler, pChannelInfo, msgBuf, 0);
	}

	case RDM_LG_MT_CLOSE:
		printf("\nReceived Login Close for StreamId %d\n", loginMsg.rdmMsgBase.streamId);
		return RSSL_RET_SUCCESS;

	default:
		printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
    	return RSSL_RET_FAILURE;
	}
}

RsslRet processLoginRequestReactor(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMLoginMsg *pLoginMsg)
{
	RsslErrorInfo errorInfo;

	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_LG_MT_REQUEST:
	{
		RsslError error;
		RsslBuffer* msgBuf = 0;
		RsslRet ret;
		RsslEncodeIterator eIter;
		RsslRDMLoginRefresh loginRefresh;
		RsslReactorSubmitOptions submitOpts;

		printf(	"Received login request.\n"
				"  Username: %.*s\n",
				pLoginMsg->request.userName.length, pLoginMsg->request.userName.data);
		if (pLoginMsg->request.flags & RDM_LG_RQF_HAS_APPLICATION_NAME)
			printf("  ApplicationName: %.*s\n",
					pLoginMsg->request.applicationName.length, pLoginMsg->request.applicationName.data);
		printf("\n");

		if (pLoginMsg->request.flags & RDM_LG_RQF_HAS_ROLE && pLoginMsg->request.role != RDM_LOGIN_ROLE_CONS)
			printf("Warning: Connected client did not identify itself as a consumer.\n");

		/* get a buffer for the login response */
		if ((msgBuf = rsslReactorGetBuffer(pReactorChannel, 512, RSSL_FALSE, &errorInfo)) == NULL)
		{
			printf("sendLoginResponse(): rsslReactorGetBuffer() failed: %d(%s)",
					error.rsslErrorId, error.text);
			return error.rsslErrorId;
		}

		rsslClearRDMLoginRefresh(&loginRefresh);

		/* provide login response information */
		/* StreamId */
		loginRefresh.rdmMsgBase.streamId = pLoginMsg->rdmMsgBase.streamId;

		/* Username */
		loginRefresh.userName = pLoginMsg->request.userName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;

		/* ApplicationId */
		loginRefresh.applicationId = loginConfig.applicationId;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;

		/* ApplicationName */
		loginRefresh.applicationName = loginConfig.applicationName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;

		/* Position */
		loginRefresh.position = loginConfig.position;
		loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;

		loginRefresh.singleOpen = 0; /* this provider does not support SingleOpen behavior */
		loginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;

		loginRefresh.supportOMMPost = 1;
		loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;

		loginRefresh.flags |= RDM_LG_RFF_SOLICITED;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		if ((ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeRDMLoginMsg() failed: %d(%s)", ret, errorInfo.rsslError.text);
			return ret;
		}

		/* send login response */
		rsslClearReactorSubmitOptions(&submitOpts);
 		return rsslReactorSubmit(pReactor, pReactorChannel, msgBuf, &submitOpts, &errorInfo);
	}

	case RDM_LG_MT_CLOSE:
		printf("\nReceived Login Close for StreamId %d\n", pLoginMsg->rdmMsgBase.streamId);
		return RSSL_RET_SUCCESS;

	default:
		printf("\nReceived Unhandled Login Msg Type: %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
    	return RSSL_RET_FAILURE;
	}
}
