/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "rsslNILoginProvider.h"
#include "rsslVASendMessage.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"

static const char *applicationId = "256";
static const char *instanceId = "VAInstance";

RsslRet setupLoginRequest(NIChannelCommand* chnlCommand, RsslInt32 streamId)
{
	RsslUInt32 ipAddress = 0;
	
	rsslInitDefaultRDMLoginRequest((RsslRDMLoginRequest*)&chnlCommand->loginRequest, streamId);
	
	chnlCommand->loginRequest.flags |= RDM_LG_RQF_HAS_ROLE;
	chnlCommand->loginRequest.role = 1;
	
	chnlCommand->loginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
	if(chnlCommand->applicationId.length)
	{
		chnlCommand->loginRequest.applicationId = chnlCommand->applicationId;
	}
	else
	{
		chnlCommand->loginRequest.applicationId.data = (char *)applicationId;
		chnlCommand->loginRequest.applicationId.length = (RsslUInt32)strlen(applicationId);
	}
	
	chnlCommand->loginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	
	if(chnlCommand->authenticationToken.length)
	{
		chnlCommand->loginRequest.userName = chnlCommand->authenticationToken;
		chnlCommand->loginRequest.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
		
		if(chnlCommand->authenticationExtended.length)
		{
			chnlCommand->loginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
			chnlCommand->loginRequest.authenticationExtended = chnlCommand->authenticationExtended;
		}
		
		printf("AuthenticationToken: %s\n", chnlCommand->loginRequest.userName.data);
		printf("AuthenticationExtended: %s\n", chnlCommand->loginRequest.authenticationExtended.data);
	}
	else
	{
		chnlCommand->loginRequest.userNameType = RDM_LOGIN_USER_NAME;
		if(chnlCommand->username.length)
		{
			chnlCommand->loginRequest.userName = chnlCommand->username;
		}
		
		printf("Username: %s\n", chnlCommand->loginRequest.userName.data);
	}
	
	return RSSL_RET_SUCCESS;
}

RsslReactorCallbackRet processLoginResponse(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent* pEvent )
{
	RsslState *pState = 0;
	RsslErrorInfo err;
	
	NIChannelCommand *chnlCommand = (NIChannelCommand*)pChannel->userSpecPtr;
	
	RsslBuffer tempBuffer =  { 1024, (char *)alloca(1024) };
	
	if(!pEvent->pRDMLoginMsg)
	{
		printf("No login response!\n");
		printf("processLoginResponse %s(%s)\n", pEvent->baseMsgEvent.pErrorInfo->rsslError.text, pEvent->baseMsgEvent.pErrorInfo->errorLocation);
		rsslReactorCloseChannel(pReactor, pChannel, &err);
		return RSSL_RC_CRET_SUCCESS;
	}
	
	switch(pEvent->pRDMLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REFRESH:
			{
			RsslRDMLoginRefresh *pLoginRefresh = &pEvent->pRDMLoginMsg->refresh;
			
			if(rsslCopyRDMLoginRefresh(&chnlCommand->loginRefresh, pLoginRefresh, &chnlCommand->loginRefreshBuffer) != RSSL_RET_SUCCESS)
			{
				printf("Failed to copy login response.\n");
				exit(-1);
			}
			
			printf("\nReceived Login Response\n");
			printf("streamId: %i\n", pEvent->pRDMLoginMsg->rdmMsgBase.streamId);
			
			pState = &pLoginRefresh->state;
			rsslStateToString(&tempBuffer, pState);
			printf("\t%.*s\n\n", tempBuffer.length, tempBuffer.data);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
				printf("  AuthenticationTTReissue: %llu\n", pLoginRefresh->authenticationTTReissue);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
				printf("  AuthenticationExtendedResp: %.*s\n", pLoginRefresh->authenticationExtendedResp.length, pLoginRefresh->authenticationExtendedResp.data);

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginRefresh->authenticationErrorCode);
			
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginRefresh->authenticationErrorText.length, pLoginRefresh->authenticationErrorText.data);
			
			if(pState->streamState == RSSL_STREAM_OPEN &&
			   pState->dataState == RSSL_DATA_OK &&
			   pState->code == RSSL_SC_NONE)
			{
				printf("Login Stream established, sending Source Directory\n");
			}
			else
			{
				printf("\nLogin failure, shutting down\n");
				return RSSL_RC_CRET_FAILURE;
			}

			// get login reissue time from authenticationTTReissue
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
			{
				chnlCommand->loginReissueTime = pLoginRefresh->authenticationTTReissue;
				chnlCommand->canSendLoginReissue = RSSL_TRUE;
			}

			break;
		}
		case RDM_LG_MT_STATUS:
		{
			RsslRDMLoginStatus *pLoginStatus = &pEvent->pRDMLoginMsg->status;
			printf("\nReceived Login Status\n");
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
				printf("  AuthenticationErrorCode: %llu\n", pLoginStatus->authenticationErrorCode);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
				printf("  AuthenticationErrorText: %.*s\n", pLoginStatus->authenticationErrorText.length, pLoginStatus->authenticationErrorText.data);
			
			if (pLoginStatus->flags & RDM_LG_STF_HAS_STATE)
			{
				/* get state information */
				
				rsslStateToString(&tempBuffer, &pLoginStatus->state);
				printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
			}
			break;
		}

		default:
			printf("\nReceived Unhandled Login Msg Class: %d\n", pEvent->baseMsgEvent.pRsslMsg->msgBase.msgClass);
			break;
	}
	
	return RSSL_RC_CRET_SUCCESS;
}
