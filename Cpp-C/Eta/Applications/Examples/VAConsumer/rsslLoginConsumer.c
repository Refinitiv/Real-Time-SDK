/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the implementation of the callback for Login responses
 * received by the rsslVAConsumer application.
 */

#include "rsslLoginConsumer.h"
#include "rsslVASendMessage.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"
#include "rsslConsumer.h"
#if defined(_WIN32)
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif

/*
 * Returns the login response information structure
 */
RsslRDMLoginRefresh* getLoginRefreshInfo(ChannelCommand *pCommand)
{
	return &pCommand->loginRefresh;
}

/*
 * Processes the information contained in Login responses.
 * Copies the refresh so we can use it to know what features are
 * supported(for example, whether posting is supported
 * by the provider).
 */
RsslReactorCallbackRet loginMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pChannel, RsslRDMLoginMsgEvent *pLoginMsgEvent)
{
	RsslState *pState = 0;
	RsslBuffer tempBuffer = { 1024, (char*)alloca(1024) };
	ChannelCommand *pCommand = (ChannelCommand*)pChannel->userSpecPtr;
	RsslRDMLoginMsg *pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;

	if (!pLoginMsg)
	{
		RsslErrorInfo *pError = pLoginMsgEvent->baseMsgEvent.pErrorInfo;
		printf("loginMsgCallback: %s(%s)\n", pError->rsslError.text, pError->errorLocation);
		closeConnection(pReactor, pChannel, pCommand);
		return RSSL_RC_CRET_SUCCESS;
	}
	switch(pLoginMsg->rdmMsgBase.rdmMsgType)
	{
	case RDM_LG_MT_REFRESH:
	{
		RsslRDMLoginRefresh *pLoginRefresh = &pLoginMsg->refresh;

		/* Store the login refresh that we received so we can remember what features the provider supports. */
		pCommand->loginRefreshMemory.data = pCommand->loginRefreshMemoryArray;
		pCommand->loginRefreshMemory.length = sizeof(pCommand->loginRefreshMemoryArray);
		if (rsslCopyRDMLoginRefresh(&pCommand->loginRefresh, pLoginRefresh, &pCommand->loginRefreshMemory) != RSSL_RET_SUCCESS)
		{
			printf("Failed to copy login response.\n");
			closeConnection(pReactor, pChannel, pCommand);
			return RSSL_RC_CRET_SUCCESS;
		}
		
		printf("\nReceived Login Response\n");

		/* get state information */
		pState = &pLoginRefresh->state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
		
		/* If authentication information is present, print it out */

		if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
			printf("	Authn TT Reissue: %llu\n", pLoginRefresh->authenticationTTReissue);
		
		if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
			printf("	Authn Extended Response: %.*s\n", pLoginRefresh->authenticationExtendedResp.length, pLoginRefresh->authenticationExtendedResp.data);
		
		if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
			printf("	Authn Error Code: %llu\n", pLoginRefresh->authenticationErrorCode);
		
		if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
			printf("	Authn Error Text: %.*s\n", pLoginRefresh->authenticationErrorText.length, pLoginRefresh->authenticationErrorText.data);


		/* Print out the list of servers available, if one was provided 
		 * in the login response.  Normally this is only given if
		 * the login request asked for it via the downloadConnectionConfig
		 * member. */
		if (pLoginRefresh->serverCount)
		{
			RsslUInt32 i;
			printf("List of available servers:\n");
			for(i = 0; i < pLoginRefresh->serverCount; ++i)
			{
				RsslRDMServerInfo *pServerInfo = &pLoginRefresh->serverList[i];

				printf("	%.*s:%llu(load factor %llu, type %hu)\n", pServerInfo->hostname.length, pServerInfo->hostname.data, pServerInfo->port, pServerInfo->loadFactor, pServerInfo->serverType); 
			}
		}

		if (pState->streamState != RSSL_STREAM_OPEN)
		{
			printf("\nLogin attempt failed\n");
			closeConnection(pReactor, pChannel, pCommand);
			return RSSL_RC_CRET_SUCCESS;
		}

		// get login reissue time from authenticationTTReissue
		if (pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
		{
			pCommand->loginReissueTime = pLoginRefresh->authenticationTTReissue;
			pCommand->canSendLoginReissue = RSSL_TRUE;
		}
		break;
	}
	case RDM_LG_MT_STATUS:
		printf("\nReceived Login StatusMsg\n");
		if (pLoginMsg->status.flags & RDM_LG_STF_HAS_STATE)
    	{
			/* get state information */
			rsslStateToString(&tempBuffer, &pLoginMsg->status.state);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    	}
		
		if(pLoginMsg->status.flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
			printf("	Authn Error Code: %llu\n", pLoginMsg->status.authenticationErrorCode);
		
		if(pLoginMsg->status.flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
			printf("	Authn Error Text: %.*s\n", pLoginMsg->status.authenticationErrorText.length, pLoginMsg->status.authenticationErrorText.data);
		break;

	case RDM_LG_MT_RTT:
		printf("Received login RTT message from Provider "SOCKET_PRINT_TYPE".\n", pChannel->socketId);
		printf("\tTicks: %llu\n", pLoginMsg->RTT.ticks);
		if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_LATENCY)
			printf("\tLast Latency: %llu\n", pLoginMsg->RTT.lastLatency);
		if (pLoginMsg->RTT.flags & RDM_LG_RTT_HAS_TCP_RETRANS)
			printf("\tProvider side TCP Retransmissions: %llu\n", pLoginMsg->RTT.tcpRetrans);
		if (pLoginMsgEvent->flags & RSSL_RDM_LG_LME_RTT_RESPONSE_SENT)
			printf("RTT Response sent to provider by reactor.\n");
		break;
	default:
		printf("\nReceived Unhandled Login Msg Class: %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
    	break;
	}

	return RSSL_RC_CRET_SUCCESS;
}
