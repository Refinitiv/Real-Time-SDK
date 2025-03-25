/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019, 2025 LSEG. All rights reserved.
*/

#include "rtr/rsslRDMLoginMsg.h"


RSSL_VA_API RsslRet rsslEncodeRDMLoginMsg(RsslEncodeIterator *pEncodeIter, RsslRDMLoginMsg *pLoginMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	RsslRet ret;
	RsslMsg msg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	char blank = 0x00;

	switch (pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REQUEST:
		{
			RsslRequestMsg *pReqMsg = &msg.requestMsg;
			RsslRDMLoginRequest *pLoginRequest = &pLoginMsg->request;
			RsslUInt tmp = 0;

			/* Preconditions */
			if (!RSSL_ERROR_INFO_CHECK((pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE && pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN 
					&& pLoginRequest->userName.data && pLoginRequest->userName.length)
					|| (pLoginRequest->userName.data && pLoginRequest->userName.length), RSSL_RET_FAILURE, pError)) 
				return RSSL_RET_FAILURE;

			rsslClearRequestMsg(pReqMsg);
			/* Populate msgBase */
			pReqMsg->msgBase.msgClass = RSSL_MC_REQUEST;
			pReqMsg->msgBase.streamId = pLoginRequest->rdmMsgBase.streamId;
			pReqMsg->msgBase.domainType = RSSL_DMT_LOGIN;
			pReqMsg->msgBase.containerType = RSSL_DT_NO_DATA;
			pReqMsg->flags = RSSL_RQMF_STREAMING;

			if (pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH)
				pReqMsg->flags |= RSSL_RQMF_NO_REFRESH;

			if (pLoginRequest->flags & RDM_LG_RQF_PAUSE_ALL)
				pReqMsg->flags |= RSSL_RQMF_PAUSE;

			/* Populate key with userName */
			pReqMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;
			
			
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
			{
				pReqMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
				pReqMsg->msgBase.msgKey.nameType = pLoginRequest->userNameType;
			}
			
			if((pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE) && pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
			{
				pReqMsg->msgBase.msgKey.name.data = &blank;
				pReqMsg->msgBase.msgKey.name.length = 1;
			}
			else
				pReqMsg->msgBase.msgKey.name = pLoginRequest->userName;				
			
			pReqMsg->msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, &msg, 0)) == RSSL_RET_ENCODE_MSG_KEY_OPAQUE, ret, pError)) return ret;

			/*** Encode login attributes in msgKey.attrib ***/

			rsslClearElementList(&elementList);
			elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			rsslClearElementEntry(&elementEntry);
			
			if((pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE) && pLoginRequest->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN && pLoginRequest->userName.length != 0)
			{
				/* Authentication token.  This has already been verified to be present with the initial precondition check above */
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_AUTHN_TOKEN;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->userName)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			
				/* Authentication extended Info */
				if(pLoginRequest->flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED && pLoginRequest->authenticationExtended.length != 0)
				{
					elementEntry.dataType = RSSL_DT_BUFFER;
					elementEntry.name = RSSL_ENAME_AUTHN_EXTENDED;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->authenticationExtended)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}
			}

			/* ApplicationId */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_APPLICATION_ID && pLoginRequest->applicationId.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_APPID;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->applicationId)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ApplicationName */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_APPLICATION_NAME && pLoginRequest->applicationName.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_APPNAME;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->applicationName)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ApplicationAuthorization Token */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN && pLoginRequest->applicationAuthorizationToken.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_APPAUTH_TOKEN;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->applicationAuthorizationToken)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* Position */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_POSITION && pLoginRequest->position.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_POSITION;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->position)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* Password */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_PASSWORD && pLoginRequest->password.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_PASSWORD;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->password)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* InstanceId */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_INSTANCE_ID && pLoginRequest->instanceId.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_INST_ID;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->instanceId)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ProvidePermissionProfile */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_PROV_PERM_PROF;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->providePermissionProfile)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ProvidePermissionExpressions */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_PROV_PERM_EXP;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->providePermissionExpressions)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SingleOpen */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_SINGLE_OPEN)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SINGLE_OPEN;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->singleOpen)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportProviderDictionaryDownload */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->supportProviderDictionaryDownload)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* AllowSuspectData */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_ALLOW_SUSPECT_DATA;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->allowSuspectData)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* Role */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_ROLE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_ROLE;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->role)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* DownloadConnectionConfig */
			if (pLoginRequest->flags & RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_DOWNLOAD_CON_CONFIG;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRequest->downloadConnectionConfig)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (pLoginRequest->flags & RDM_LG_RQF_RTT_SUPPORT)
			{
				rsslClearElementEntry(&elementEntry);
				elementEntry.name = RSSL_ENAME_RTT;
				elementEntry.dataType = RSSL_DT_UINT;
				tmp = RDM_LOGIN_RTT_ELEMENT;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &tmp)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			/* complete encode key */
			/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
			   for us to encode our container/msg payload */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgKeyAttribComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;


			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		case RDM_LG_MT_CONSUMER_CONNECTION_STATUS:
		{
			RsslGenericMsg *pGenericMsg = &msg.genericMsg;
			RsslRDMLoginConsumerConnectionStatus *pConnectionStatus = &pLoginMsg->consumerConnectionStatus;

			RsslMap map;
			RsslMapEntry mapEntry;
			RsslElementList elementList;
			RsslElementEntry elementEntry;

			/* Encode ConsumerConnectionStatus Generic message.
			 * Used to send the WarmStandbyMode. */
			rsslClearGenericMsg(pGenericMsg);
			pGenericMsg->msgBase.msgClass = RSSL_MC_GENERIC;
			pGenericMsg->msgBase.streamId = pLoginMsg->rdmMsgBase.streamId;
			pGenericMsg->msgBase.domainType = RSSL_DMT_LOGIN;
			pGenericMsg->msgBase.containerType = RSSL_DT_MAP;
			pGenericMsg->flags = RSSL_GNMF_HAS_MSG_KEY;

			pGenericMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
			pGenericMsg->msgBase.msgKey.name = RSSL_ENAME_CONS_CONN_STATUS;
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, &msg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return ret;

			rsslClearMap(&map);
			map.flags = RSSL_MPF_NONE;
			map.keyPrimitiveType = RSSL_DT_ASCII_STRING;
			map.containerType = RSSL_DT_ELEMENT_LIST;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapInit(pEncodeIter, &map, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			if (pConnectionStatus->flags & RDM_LG_CCSF_HAS_WARM_STANDBY_INFO)
			{
				rsslClearMapEntry(&mapEntry);
				mapEntry.flags = RSSL_MPEF_NONE;
				mapEntry.action = pConnectionStatus->warmStandbyInfo.action;

				if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryInit(pEncodeIter, &mapEntry, &RSSL_ENAME_WARMSTANDBY_INFO, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					rsslClearElementEntry(&elementEntry);
					elementEntry.name = RSSL_ENAME_WARMSTANDBY_MODE;
					elementEntry.dataType = RSSL_DT_UINT;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pConnectionStatus->warmStandbyInfo.warmStandbyMode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}
				else
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapEntry(pEncodeIter, &mapEntry, &RSSL_ENAME_WARMSTANDBY_INFO)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}
			}

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMapComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;

		}
		case RDM_LG_MT_CLOSE:
		{
			RsslCloseMsg *pCloseMsg = &msg.closeMsg;
			rsslClearCloseMsg(pCloseMsg);
			pCloseMsg->msgBase.msgClass = RSSL_MC_CLOSE;
			pCloseMsg->msgBase.streamId = pLoginMsg->rdmMsgBase.streamId;
			pCloseMsg->msgBase.domainType = RSSL_DMT_LOGIN;
			pCloseMsg->msgBase.containerType = RSSL_DT_NO_DATA;
			pCloseMsg->flags = RSSL_CLMF_NONE;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pEncodeIter, &msg)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_LG_MT_REFRESH:
		{
			RsslRefreshMsg msg;
			RsslRDMLoginRefresh *pLoginRefresh = &pLoginMsg->refresh;
			RsslUInt tmp = 0;

			rsslClearRefreshMsg(&msg);
			/* Populate msgBase */
			msg.msgBase.msgClass = RSSL_MC_REFRESH;
			msg.msgBase.streamId = pLoginRefresh->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_LOGIN;
			msg.msgBase.containerType = (pLoginRefresh->flags & RDM_LG_RFF_HAS_CONN_CONFIG) ? RSSL_DT_ELEMENT_LIST : RSSL_DT_NO_DATA;
			msg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE;

			msg.state = pLoginRefresh->state;

			if (pLoginRefresh->flags & RDM_LG_RFF_SOLICITED)
				msg.flags |= RSSL_RFMF_SOLICITED;

			if (pLoginRefresh->flags & RDM_LG_RFF_CLEAR_CACHE)
				msg.flags |= RSSL_RFMF_CLEAR_CACHE;

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SEQ_NUM)
			{
				msg.flags |= RSSL_RFMF_HAS_SEQ_NUM;
				msg.seqNum = pLoginRefresh->sequenceNumber;
			}

			msg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB;

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME)
			{
				msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
				msg.msgBase.msgKey.name = pLoginRefresh->userName;
			}

			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE)
			{
				if((pLoginRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE) && pLoginRefresh->userNameType == RDM_LOGIN_USER_AUTHN_TOKEN)
				{
					msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
					msg.msgBase.msgKey.name.data = &blank;
					msg.msgBase.msgKey.name.length = 1;
				}
				
				msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
				msg.msgBase.msgKey.nameType = pLoginRefresh->userNameType;
			}

			msg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

			/* Begin encoding message */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, (RsslMsg*)&msg, 0)) == RSSL_RET_ENCODE_MSG_KEY_OPAQUE, ret, pError)) return ret;

			/*** Encode login attributes in msgKey.attrib ***/

			rsslClearElementList(&elementList);
			elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			rsslClearElementEntry(&elementEntry);

			/* ApplicationId */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID && pLoginRefresh->applicationId.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_APPID;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->applicationId)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ApplicationName */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_NAME && pLoginRefresh->applicationName.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_APPNAME;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->applicationName)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* Position */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_POSITION && pLoginRefresh->position.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_POSITION;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->position)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ProvidePermissionProfile */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_PROV_PERM_PROF;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->providePermissionProfile)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* ProvidePermissionExpressions */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_PROV_PERM_EXP;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->providePermissionExpressions)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SingleOpen */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SINGLE_OPEN;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->singleOpen)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* AllowSuspectData */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_ALLOW_SUSPECT_DATA;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->allowSuspectData)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportOMMPost */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_POST)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_POST;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportOMMPost)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportBatchRequests */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_BATCH;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportBatchRequests)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportProviderDictionaryDownload */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportProviderDictionaryDownload)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportAdvancedSymbolList */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_ENH_SL)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_ENH_SL;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportEnhancedSymbolList)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportViewRequests */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_VIEW;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportViewRequests)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportStandby */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_STANDBY;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportStandby)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportOptimizedPauseResume */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_OPR;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportOptimizedPauseResume)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* sequenceRetryInterval */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SEQUENCE_RETRY_INTERVAL;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->sequenceRetryInterval)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* updateBufferLimit */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_UPDATE_BUFFER_LIMIT;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->updateBufferLimit)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* sequenceNumberRecovery */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SEQUENCE_NUMBER_RECOVERY;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->sequenceNumberRecovery)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}
			
			/* Authentication time to reissue */
			if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_AUTHN_TT_REISSUE;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->authenticationTTReissue)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}
			
			/* Authentication extended response buffer */
			if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP && pLoginRefresh->authenticationExtendedResp.length != 0)
			{
				elementEntry.dataType = RSSL_DT_BUFFER;
				elementEntry.name = RSSL_ENAME_AUTHN_EXTENDED_RESP;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->authenticationExtendedResp)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}
			
			/* Authentication error code */
			if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_AUTHN_ERROR_CODE;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->authenticationErrorCode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}
			
			/* Authentication error text buffer */
			if(pLoginRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT && pLoginRefresh->authenticationErrorText.length != 0)
			{
				elementEntry.dataType = RSSL_DT_ASCII_STRING;
				elementEntry.name = RSSL_ENAME_AUTHN_ERROR_TEXT;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->authenticationErrorText)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (pLoginRefresh->flags & RDM_LG_RFF_RTT_SUPPORT)
			{
				rsslClearElementEntry(&elementEntry);
				elementEntry.name = RSSL_ENAME_RTT;
				elementEntry.dataType = RSSL_DT_UINT;
				tmp = RDM_LOGIN_RTT_ELEMENT;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &tmp)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			/* SupportStandbyMode */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE)
			{
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_SUPPORT_STANDBY_MODE;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->supportStandbyMode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			/* complete encode key */
			/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
			   for us to encode our container/msg payload */
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgKeyAttribComplete(pEncodeIter, RSSL_TRUE)) == ((pLoginRefresh->flags & RDM_LG_RFF_HAS_CONN_CONFIG) ? RSSL_RET_ENCODE_CONTAINER : RSSL_RET_SUCCESS), ret, pError)) return ret;

			/* Encode server list now, if specified */
			if (pLoginRefresh->flags & RDM_LG_RFF_HAS_CONN_CONFIG)
			{
				RsslVector vec;
				RsslVectorEntry vecEntry;
				RsslUInt32 i;

				/* Encode Element "ConnectionConfig" */
				rsslClearElementList(&elementList);
				elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				rsslClearElementEntry(&elementEntry);

				elementEntry.dataType = RSSL_DT_VECTOR;
				elementEntry.name = RSSL_ENAME_CONNECTION_CONFIG;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryInit(pEncodeIter, &elementEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* Encode Server Entries */
				rsslClearVector(&vec);
				vec.containerType = RSSL_DT_ELEMENT_LIST;
				vec.flags = RSSL_VTF_HAS_SUMMARY_DATA;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeVectorInit(pEncodeIter, &vec, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;


				/* Encode numStandbyServers in summary data. */
				rsslClearElementList(&elementList);
				elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				rsslClearElementEntry(&elementEntry);
				elementEntry.dataType = RSSL_DT_UINT;
				elementEntry.name = RSSL_ENAME_NUM_STANDBY_SERVERS;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginRefresh->numStandbyServers)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeVectorSummaryDataComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				for (i = 0; i < pLoginRefresh->serverCount; ++i)
				{
					RsslRDMServerInfo *pServer = &pLoginRefresh->serverList[i];

					rsslClearVectorEntry(&vecEntry);
					vecEntry.index = pServer->serverIndex;
					vecEntry.action = RSSL_VTEA_SET_ENTRY;


					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeVectorEntryInit(pEncodeIter, &vecEntry, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* Encode Element List describing server */
					rsslClearElementList(&elementList);
					elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					rsslClearElementEntry(&elementEntry);

					elementEntry.dataType = RSSL_DT_ASCII_STRING;
					elementEntry.name = RSSL_ENAME_HOSTNAME;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pServer->hostname)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					elementEntry.dataType = RSSL_DT_UINT;
					elementEntry.name = RSSL_ENAME_PORT;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pServer->port)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					if (pServer->flags & RDM_LG_SIF_HAS_LOAD_FACTOR)
					{
						elementEntry.dataType = RSSL_DT_UINT;
						elementEntry.name = RSSL_ENAME_LOAD_FACT;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pServer->loadFactor)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}

					if (pServer->flags & RDM_LG_SIF_HAS_TYPE)
					{
						elementEntry.dataType = RSSL_DT_ENUM;
						elementEntry.name = RSSL_ENAME_SERVER_TYPE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pServer->serverType)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeVectorEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}

				/* Complete */
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeVectorComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntryComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}

		case RDM_LG_MT_STATUS:
		{
			RsslStatusMsg msg;
			RsslRDMLoginStatus *pLoginStatus = &pLoginMsg->status;
			rsslClearStatusMsg(&msg);
			/* Populate msgBase */
			msg.msgBase.msgClass = RSSL_MC_STATUS;
			msg.msgBase.streamId = pLoginStatus->rdmMsgBase.streamId;
			msg.msgBase.domainType = RSSL_DMT_LOGIN;
			msg.msgBase.containerType = RSSL_DT_NO_DATA;
			
			if(pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE || (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT 
				&& pLoginStatus->authenticationErrorText.data != 0 && pLoginStatus->authenticationErrorText.length != 0))
			{
				msg.flags |= RSSL_STMF_HAS_MSG_KEY;
				msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB;
			}

			if (pLoginStatus->flags & RDM_LG_STF_HAS_STATE) 
			{
				msg.flags |= RSSL_STMF_HAS_STATE;
				msg.state = pLoginStatus->state;
			}

			if (pLoginStatus->flags & RDM_LG_STF_HAS_USERNAME)
			{

				msg.flags |= RSSL_STMF_HAS_MSG_KEY;
				msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
				msg.msgBase.msgKey.name = pLoginStatus->userName;

				if (pLoginStatus->flags & RDM_LG_STF_HAS_USERNAME_TYPE)
				{
					msg.flags |= RSSL_STMF_HAS_MSG_KEY;
					msg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME_TYPE;
					msg.msgBase.msgKey.nameType = pLoginStatus->userNameType;
				}
			}

			if((pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE) || (pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT && pLoginStatus->authenticationErrorText.length != 0))
			{
				msg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;
				
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, (RsslMsg*)&msg, 0) == RSSL_RET_ENCODE_MSG_KEY_OPAQUE), ret, pError)) return ret;

				/*** Encode login attributes in msgKey.attrib ***/

				rsslClearElementList(&elementList);
				elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				rsslClearElementEntry(&elementEntry);
				
				/* Authentication error code */
				if(pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
				{
					elementEntry.dataType = RSSL_DT_UINT;
					elementEntry.name = RSSL_ENAME_AUTHN_ERROR_CODE;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginStatus->authenticationErrorCode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}
				
				/* Authentication error text buffer */
				if(pLoginStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT && pLoginStatus->authenticationErrorText.length != 0)
				{
					elementEntry.dataType = RSSL_DT_ASCII_STRING;
					elementEntry.name = RSSL_ENAME_AUTHN_ERROR_TEXT;
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pLoginStatus->authenticationErrorText)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
				}

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* complete encode key */
				/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
				   for us to encode our container/msg payload */
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgKeyAttribComplete(pEncodeIter, RSSL_TRUE) == RSSL_RET_SUCCESS), ret, pError)) return ret;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}
			else
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsg(pEncodeIter, (RsslMsg*)&msg) == RSSL_RET_SUCCESS), ret, pError)) return ret;
			
			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);
			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		case RDM_LG_MT_RTT:
		{
			RsslGenericMsg *pGenericMsg = &msg.genericMsg;
			RsslRDMLoginRTT *pRTT = &pLoginMsg->RTT;

			RsslElementList elementList;
			RsslElementEntry elementEntry;

			/* Encode ConsumerConnectionStatus Generic message.
			* Used to send the WarmStandbyMode. */
			rsslClearGenericMsg(pGenericMsg);
			pGenericMsg->msgBase.msgClass = RSSL_MC_GENERIC;
			pGenericMsg->msgBase.streamId = pLoginMsg->rdmMsgBase.streamId;
			pGenericMsg->msgBase.domainType = RSSL_DMT_LOGIN;
			pGenericMsg->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
			pGenericMsg->flags = RSSL_GNMF_PROVIDER_DRIVEN;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgInit(pEncodeIter, &msg, 0)) == RSSL_RET_ENCODE_CONTAINER, ret, pError)) return ret;
			
			rsslClearElementList(&elementList);
			elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListInit(pEncodeIter, &elementList, 0, 0)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			/* Encode ticks */
			rsslClearElementEntry(&elementEntry);
			elementEntry.name = RSSL_ENAME_RTT_TICKS;
			elementEntry.dataType = RSSL_DT_UINT;
			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pRTT->ticks)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			
			if (pRTT->flags & RDM_LG_RTT_HAS_TCP_RETRANS)
			{
				rsslClearElementEntry(&elementEntry);
				elementEntry.name = RSSL_ENAME_RTT_TCP_RETRANS;
				elementEntry.dataType = RSSL_DT_UINT;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pRTT->tcpRetrans)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}

			if (pRTT->flags & RDM_LG_RTT_HAS_LATENCY)
			{
				rsslClearElementEntry(&elementEntry);
				elementEntry.name = RSSL_ENAME_RTT;
				elementEntry.dataType = RSSL_DT_UINT;
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementEntry(pEncodeIter, &elementEntry, &pRTT->lastLatency)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
			}


			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeElementListComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			if (!RSSL_ERROR_INFO_CHECK((ret = rsslEncodeMsgComplete(pEncodeIter, RSSL_TRUE)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

			*pBytesWritten = rsslGetEncodedBufferLength(pEncodeIter);

			pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
			return RSSL_RET_SUCCESS;
		}
		
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown login msg type %d\n", pLoginMsg->rdmMsgBase.rdmMsgType);
			return RSSL_RET_FAILURE;
	}
}

RSSL_VA_API RsslRet rsslDecodeRDMLoginMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMLoginMsg *pLoginMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	RsslRet ret = 0;
	RsslMsgKey* key = 0;
	RsslElementList	elementList;
	RsslElementEntry	element;
	RsslUInt RTTType = 0;
	

	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
		{
			RsslRDMLoginRefresh *pLoginRefresh = &pLoginMsg->refresh;

			rsslClearRDMLoginRefresh(pLoginRefresh);

			pLoginRefresh->state = pMsg->refreshMsg.state;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_SOLICITED)
				pLoginRefresh->flags |= RDM_LG_RFF_SOLICITED;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_CLEAR_CACHE)
				pLoginRefresh->flags |= RDM_LG_RFF_CLEAR_CACHE;

			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			{
				pLoginRefresh->flags |= RDM_LG_RFF_HAS_SEQ_NUM;
				pLoginRefresh->sequenceNumber = pMsg->refreshMsg.seqNum;
			}

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			if (key != NULL)
			{
				if (!RSSL_ERROR_INFO_CHECK(!(key->flags & RSSL_MKF_HAS_ATTRIB) || (key->attribContainerType == RSSL_DT_ELEMENT_LIST), RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				if (key->flags & RSSL_MKF_HAS_NAME)
				{
					pLoginRefresh->userName = key->name;
					pLoginRefresh->flags |= RDM_LG_RFF_HAS_USERNAME;
				}

				if (key->flags & RSSL_MKF_HAS_NAME_TYPE)
				{
					pLoginRefresh->userNameType = key->nameType;
					pLoginRefresh->flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
				}

				/* decode key attrib data, if present */
				if (key->flags & RSSL_MKF_HAS_ATTRIB)
				{
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMsgKeyAttrib(pIter, key)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* decode element list */
					if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* decode each element entry in list */
					while ((ret = rsslDecodeElementEntry(pIter, &element)) != RSSL_RET_END_OF_CONTAINER)
					{
						if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return ret;

						/* get login response information */
						/* AllowSuspectData */
						if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ALLOW_SUSPECT_DATA))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->allowSuspectData)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* ApplicationId */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPID))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
							pLoginRefresh->applicationId = element.encData;
						}
						/* ApplicationName */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
							pLoginRefresh->applicationName = element.encData;
						}
						/* Position */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_POSITION;
							pLoginRefresh->position = element.encData;
						}
						/* ProvidePermissionExpressions */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_EXP))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->providePermissionExpressions)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* ProvidePermissionProfile */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_PROF))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->providePermissionProfile)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SingleOpen */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SINGLE_OPEN))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->singleOpen)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportOMMPost */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_POST))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_POST;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportOMMPost)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportStandby */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_STANDBY))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_STANDBY;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportStandby)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportBatchRequests */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_BATCH))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_BATCH;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportBatchRequests)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportProviderDictionaryDownload */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportProviderDictionaryDownload)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportAdvancedSymbolList */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_ENH_SL))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_ENH_SL;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportEnhancedSymbolList)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportViewRequests */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_VIEW))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportViewRequests)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* SupportOptimizedPauseResume */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_OPR))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportOptimizedPauseResume)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* sequenceRetryInterval */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SEQUENCE_RETRY_INTERVAL))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->sequenceRetryInterval)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* updateBufferLimit */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_UPDATE_BUFFER_LIMIT))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->updateBufferLimit)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* sequenceNumberRecovery */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SEQUENCE_NUMBER_RECOVERY))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->sequenceNumberRecovery)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* Authentication TT Reissue */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_TT_REISSUE))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_AUTHN_TT_REISSUE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->authenticationTTReissue)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* Authentication extended response */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_EXTENDED_RESP))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP;
							pLoginRefresh->authenticationExtendedResp = element.encData;
						}
						/* Authentication Error Code */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_CODE))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_AUTHN_ERROR_CODE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->authenticationErrorCode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
						/* Authentication Error Text */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_TEXT))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_BUFFER || element.dataType == RSSL_DT_ASCII_STRING, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT;
							pLoginRefresh->authenticationErrorText = element.encData;	
						}
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &RTTType)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
							if (!RSSL_ERROR_INFO_CHECK((RTTType == RDM_LOGIN_RTT_ELEMENT), ret, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_RTT_SUPPORT;
						}
						/* SupportStandbyMode */
						else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_STANDBY_MODE))
						{
							if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
							pLoginRefresh->flags |= RDM_LG_RFF_HAS_SUPPORT_STANDBY_MODE;
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->supportStandbyMode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}
					}
				}
			}

			if (pMsg->msgBase.containerType == RSSL_DT_ELEMENT_LIST)
			{
				/* Decode payload */
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* decode each element entry in list */
				while ((ret = rsslDecodeElementEntry(pIter, &element)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* ConnectionConfig */
					if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_CONNECTION_CONFIG))
					{
						RsslVector vector;
						RsslVectorEntry vectorEntry;
						RsslElementList serverElementList;
						RsslElementEntry serverElementEntry;
						RsslLocalElementSetDefDb setDb;
						RsslBool foundNumStandbyServers = RSSL_FALSE;
						char setDefMemory[3825];

						pLoginRefresh->flags |= RDM_LG_RFF_HAS_CONN_CONFIG;

						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_VECTOR, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeVector(pIter, &vector)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

						if (!RSSL_ERROR_INFO_CHECK(vector.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

						if (rsslVectorCheckHasSetDefs(&vector))
						{
							rsslClearLocalElementSetDefDb(&setDb);
							setDb.entries.data = setDefMemory;
							setDb.entries.length = sizeof(setDefMemory);
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeLocalElementSetDefDb(pIter, &setDb)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						}

						/* Decode payload */
						if (rsslVectorCheckHasSummaryData(&vector))
						{
							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &serverElementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

							/* decode each element entry in list */
							while ((ret = rsslDecodeElementEntry(pIter, &serverElementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&serverElementEntry.name, &RSSL_ENAME_NUM_STANDBY_SERVERS))
								{
									if (!RSSL_ERROR_INFO_CHECK(serverElementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRefresh->numStandbyServers)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
								}
								foundNumStandbyServers = RSSL_TRUE;
							}

							if (!RSSL_ERROR_INFO_CHECK(foundNumStandbyServers, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						}

						rsslClearVectorEntry(&vectorEntry);
						if ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
						{

							/* Align buffer so we can start an array */
							if (!RSSL_ERROR_INFO_CHECK(pLoginRefresh->serverList = (RsslRDMServerInfo*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 0, sizeof(RsslRDMServerInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

							do
							{
								RsslRDMServerInfo* pNewServerInfo;
								RsslBool foundServerHostname = RSSL_FALSE;
								RsslBool foundServerPort = RSSL_FALSE;

								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (!RSSL_ERROR_INFO_CHECK(pNewServerInfo = (RsslRDMServerInfo*)rsslReserveBufferMemory(pMemoryBuffer, 1, sizeof(RsslRDMServerInfo)), RSSL_RET_BUFFER_TOO_SMALL, pError)) return RSSL_RET_BUFFER_TOO_SMALL;

								rsslClearRDMServerInfo(pNewServerInfo);

								if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &serverElementList, &setDb)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

								pNewServerInfo->serverIndex = vectorEntry.index;

								/* decode each element entry in list */
								while ((ret = rsslDecodeElementEntry(pIter, &serverElementEntry)) != RSSL_RET_END_OF_CONTAINER)
								{

									if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return ret;

									if (rsslBufferIsEqual(&serverElementEntry.name, &RSSL_ENAME_HOSTNAME))
									{
										if (!RSSL_ERROR_INFO_CHECK(serverElementEntry.dataType == RSSL_DT_ASCII_STRING || serverElementEntry.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
										pNewServerInfo->hostname = serverElementEntry.encData;
										foundServerHostname = RSSL_TRUE;
									}
									else if (rsslBufferIsEqual(&serverElementEntry.name, &RSSL_ENAME_PORT))
									{
										if (!RSSL_ERROR_INFO_CHECK(serverElementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pNewServerInfo->port)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
										foundServerPort = RSSL_TRUE;
									}
									else if (rsslBufferIsEqual(&serverElementEntry.name, &RSSL_ENAME_LOAD_FACT))
									{
										if (!RSSL_ERROR_INFO_CHECK(serverElementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pNewServerInfo->loadFactor)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
										pNewServerInfo->flags |= RDM_LG_SIF_HAS_LOAD_FACTOR;
									}
									else if (rsslBufferIsEqual(&serverElementEntry.name, &RSSL_ENAME_SERVER_TYPE))
									{
										if (!RSSL_ERROR_INFO_CHECK(serverElementEntry.dataType == RSSL_DT_ENUM, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
										if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeEnum(pIter, &pNewServerInfo->serverType)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
										pNewServerInfo->flags |= RDM_LG_SIF_HAS_TYPE;
									}

								}

								if (!RSSL_ERROR_INFO_CHECK(foundServerHostname, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
								if (!RSSL_ERROR_INFO_CHECK(foundServerPort, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
								++pLoginRefresh->serverCount;

								rsslClearVectorEntry(&vectorEntry);
							} while ((ret = rsslDecodeVectorEntry(pIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER);
						}
					}
				}
			}

			break;
		}
		case RSSL_MC_STATUS:
		{
			RsslRDMLoginStatus *pLoginStatus = &pLoginMsg->status;

			rsslClearRDMLoginStatus(pLoginStatus);

			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				pLoginStatus->flags |= RDM_LG_STF_HAS_STATE;
				pLoginStatus->state = pMsg->statusMsg.state;
			}

			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			if (key && key->flags & RSSL_MKF_HAS_NAME)
			{
				pLoginStatus->userName = key->name;
				pLoginStatus->flags |= RDM_LG_STF_HAS_USERNAME;

				if (key && key->flags & RSSL_MKF_HAS_NAME_TYPE)
				{
					pLoginStatus->userNameType = key->nameType;
					pLoginStatus->flags |= RDM_LG_STF_HAS_USERNAME_TYPE;
				}
			}
			
			if (key && key->flags & RSSL_MKF_HAS_ATTRIB)
			{
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMsgKeyAttrib(pIter, key)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* decode element list */
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* decode each element entry in list */
				while ((ret = rsslDecodeElementEntry(pIter, &element)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* Authentication Error Code */
					if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_CODE))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginStatus->flags |= RDM_LG_STF_HAS_AUTHN_ERROR_CODE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginStatus->authenticationErrorCode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* Authentication Error Text */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_ERROR_TEXT))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_BUFFER || element.dataType == RSSL_DT_ASCII_STRING, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginStatus->flags |= RDM_LG_STF_HAS_AUTHN_ERROR_TEXT;
						pLoginStatus->authenticationErrorText = element.encData;
					}
				}
			}
		
			break;
		}
		case RSSL_MC_POST:
		{
			pLoginMsg->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_POST;
			break;
		}
		case RSSL_MC_ACK:
		{
			pLoginMsg->rdmMsgBase.domainType = RSSL_DMT_LOGIN;
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_ACK;
			break;
		}
		case RSSL_MC_REQUEST:
		{
			RsslRDMLoginRequest *pLoginRequest = &pLoginMsg->request;
			RsslUInt RTTType = 0;

			rsslClearRDMLoginRequest(pLoginRequest);

			/* All login requests should be streaming */
			if (!RSSL_ERROR_INFO_CHECK(pMsg->requestMsg.flags & RSSL_RQMF_STREAMING, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			if (pMsg->requestMsg.flags & RSSL_RQMF_NO_REFRESH)
				pLoginRequest->flags |= RDM_LG_RQF_NO_REFRESH;

			if (pMsg->requestMsg.flags & RSSL_RQMF_PAUSE)
				pLoginRequest->flags |= RDM_LG_RQF_PAUSE_ALL;

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);


			if (!RSSL_ERROR_INFO_CHECK(key && (key->flags & RSSL_MKF_HAS_NAME), RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			/* get Username */
			pLoginRequest->userName = key->name;

			if (key->flags & RSSL_MKF_HAS_NAME_TYPE)
			{
				pLoginRequest->userNameType = key->nameType;
				pLoginRequest->flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
			}

			/* decode key attrib data, if present */
			if (key->flags & RSSL_MKF_HAS_ATTRIB)
			{
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMsgKeyAttrib(pIter, key)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* decode element list */
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				/* decode each element entry in list */
				while ((ret = rsslDecodeElementEntry(pIter, &element)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return ret;

					/* get login response information */
					/* AllowSuspectData */
					if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ALLOW_SUSPECT_DATA))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->allowSuspectData)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* ApplicationId */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPID))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
						pLoginRequest->applicationId = element.encData;
					}
					/* ApplicationName */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPNAME))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_NAME;
						pLoginRequest->applicationName = element.encData;
					}
					/* ApplicationAuthorizationToken */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_APPAUTH_TOKEN))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN;
						pLoginRequest->applicationAuthorizationToken = element.encData;
					}

					/* DownloadConnectionConfig */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DOWNLOAD_CON_CONFIG))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->downloadConnectionConfig)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* Position */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_POSITION))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_POSITION;
						pLoginRequest->position = element.encData;
					}
					/* Password */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PASSWORD))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_PASSWORD;
						pLoginRequest->password = element.encData;
					}
					/* InstanceId */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_INST_ID))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_INSTANCE_ID;
						pLoginRequest->instanceId = element.encData;
					}
					/* ProvidePermissionExpressions */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_EXP))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->providePermissionExpressions)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* ProvidePermissionProfile */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PROV_PERM_PROF))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->providePermissionProfile)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* Role */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ROLE))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_ROLE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->role)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* SingleOpen */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SINGLE_OPEN))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->singleOpen)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* SupportProviderDictionaryDownload */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pLoginRequest->supportProviderDictionaryDownload)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
					/* Authentication Token */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_TOKEN))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->userName = element.encData;
					}
					/* Authentication Token */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_AUTHN_EXTENDED))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_ASCII_STRING || element.dataType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
						pLoginRequest->authenticationExtended = element.encData;
					}
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_RTT))
					{
						if (!RSSL_ERROR_INFO_CHECK(element.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &RTTType)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;
						if (!RSSL_ERROR_INFO_CHECK((RTTType == RDM_LOGIN_RTT_ELEMENT), ret, pError)) return RSSL_RET_FAILURE;
						pLoginRequest->flags |= RDM_LG_RQF_RTT_SUPPORT;
					}
				}
			}
			break;
		}

		case RSSL_MC_GENERIC:
		{
			if (pMsg->msgBase.containerType == RSSL_DT_ELEMENT_LIST)
			{
				RsslElementList elementList; RsslElementEntry elementEntry;
				RsslRDMLoginRTT *pRTT = &pLoginMsg->RTT;

				rsslClearRDMLoginRTT(pRTT);
				
				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

				while ((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_RTT))
					{
						if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pRTT->lastLatency)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						pRTT->flags |= RDM_LG_RTT_HAS_LATENCY;
					}
					else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_RTT_TCP_RETRANS))
					{
						if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pRTT->tcpRetrans)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
						pRTT->flags |= RDM_LG_RTT_HAS_TCP_RETRANS;
					}
					else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_RTT_TICKS))
					{
						if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pRTT->ticks)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
					}
				}

				break;
			}

			/* get key */
			key = (RsslMsgKey *)rsslGetMsgKey(pMsg);

			if (!RSSL_ERROR_INFO_CHECK(key && (key->flags & RSSL_MKF_HAS_NAME), RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

			if (rsslBufferIsEqual(&key->name, &RSSL_ENAME_CONS_CONN_STATUS))
			{
				RsslMap map; RsslMapEntry mapEntry;
				RsslBuffer mapKey;
				RsslElementList elementList; RsslElementEntry elementEntry;
				RsslRDMLoginConsumerConnectionStatus *pConsumerConnectionStatus = &pLoginMsg->consumerConnectionStatus;

				rsslClearRDMLoginConsumerConnectionStatus(pConsumerConnectionStatus);

				if (!RSSL_ERROR_INFO_CHECK(pMsg->msgBase.containerType == RSSL_DT_MAP, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeMap(pIter, &map)) == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK(map.keyPrimitiveType == RSSL_DT_ASCII_STRING || map.keyPrimitiveType == RSSL_DT_BUFFER, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				if (!RSSL_ERROR_INFO_CHECK(map.containerType == RSSL_DT_ELEMENT_LIST, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;

				while((ret = rsslDecodeMapEntry(pIter, &mapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

					if (rsslBufferIsEqual(&mapKey, &RSSL_ENAME_WARMSTANDBY_INFO))
					{
						pConsumerConnectionStatus->flags |= RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
						pConsumerConnectionStatus->warmStandbyInfo.action = (RsslMapEntryActions)mapEntry.action;

						if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
						{
							RsslBool foundWarmStandbyMode = RSSL_FALSE;

							if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeElementList(pIter, &elementList, NULL)) == RSSL_RET_SUCCESS, ret, pError)) return ret;

							while((ret = rsslDecodeElementEntry(pIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (!RSSL_ERROR_INFO_CHECK(ret == RSSL_RET_SUCCESS, ret, pError)) return RSSL_RET_FAILURE;

								if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_WARMSTANDBY_MODE))
								{
									if (!RSSL_ERROR_INFO_CHECK(elementEntry.dataType == RSSL_DT_UINT, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
									if (!RSSL_ERROR_INFO_CHECK((ret = rsslDecodeUInt(pIter, &pConsumerConnectionStatus->warmStandbyInfo.warmStandbyMode)) == RSSL_RET_SUCCESS, ret, pError)) return ret;
									foundWarmStandbyMode = RSSL_TRUE;
								}
							}
							if (!RSSL_ERROR_INFO_CHECK(foundWarmStandbyMode, RSSL_RET_FAILURE, pError)) return RSSL_RET_FAILURE;
						}
					}

				}
			}
			else
			{
				rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown generic msg name \"%.*s\"\n", key->name.length, key->name.data);
				return RSSL_RET_FAILURE;
			}
			break;
		}
		case RSSL_MC_CLOSE:
		{
			rsslClearRDMLoginClose(&pLoginMsg->close);
			/* No additional information to decode */
			break;
		}
		default:
			pError->rsslErrorInfoCode = RSSL_EIC_FAILURE;
			rsslSetErrorInfo(pError, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, "Unknown msg class for login: %u\n", pMsg->msgBase.msgClass);
			return RSSL_RET_FAILURE;
	}

	pLoginMsg->rdmMsgBase.streamId = pMsg->msgBase.streamId;
	pError->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginRequest(RsslRDMLoginRequest *pNewRequest, RsslRDMLoginRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer)
{
	/* Copy members */
	*pNewRequest = *pOldRequest;

	/* Create separate copies of strings */
	if (!rsslCopyBufferMemory(&pNewRequest->userName, &pOldRequest->userName, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_APPLICATION_ID) && !rsslCopyBufferMemory(&pNewRequest->applicationId, &pOldRequest->applicationId, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_APPLICATION_NAME) && !rsslCopyBufferMemory(&pNewRequest->applicationName, &pOldRequest->applicationName, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN) && !rsslCopyBufferMemory(&pNewRequest->applicationAuthorizationToken, &pOldRequest->applicationAuthorizationToken, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_POSITION) && !rsslCopyBufferMemory(&pNewRequest->position, &pOldRequest->position, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_PASSWORD) && !rsslCopyBufferMemory(&pNewRequest->password, &pOldRequest->password, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRequest->flags & RDM_LG_RQF_HAS_INSTANCE_ID) && !rsslCopyBufferMemory(&pNewRequest->instanceId, &pOldRequest->instanceId, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	if ((pOldRequest->flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED) && !rsslCopyBufferMemory(&pNewRequest->authenticationExtended, &pOldRequest->authenticationExtended, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginClose(RsslRDMLoginClose *pNewClose, RsslRDMLoginClose *pOldClose, RsslBuffer *pNewMemoryBuffer)
{
	*pNewClose = *pOldClose;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginConsumerConnectionStatus(RsslRDMLoginConsumerConnectionStatus *pNewConnectionStatus, RsslRDMLoginConsumerConnectionStatus *pOldConnectionStatus, RsslBuffer *pNewMemoryBuffer)
{
	*pNewConnectionStatus = *pOldConnectionStatus;
	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginRefresh(RsslRDMLoginRefresh *pNewRefresh, RsslRDMLoginRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer)
{
	/* Copy members */
	*pNewRefresh = *pOldRefresh;

	/* Create separate copies of strings */
	if ((pOldRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID) && !rsslCopyBufferMemory(&pNewRefresh->applicationId, &pOldRefresh->applicationId, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_NAME) && !rsslCopyBufferMemory(&pNewRefresh->applicationName, &pOldRefresh->applicationName, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if ((pOldRefresh->flags & RDM_LG_RFF_HAS_POSITION) && !rsslCopyBufferMemory(&pNewRefresh->position, &pOldRefresh->position, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;

	if (!rsslCopyBufferMemory(&pNewRefresh->state.text, &pOldRefresh->state.text, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	if ((pOldRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP) && !rsslCopyBufferMemory(&pNewRefresh->authenticationExtendedResp, &pOldRefresh->authenticationExtendedResp, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	if ((pOldRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT) && !rsslCopyBufferMemory(&pNewRefresh->authenticationErrorText, &pOldRefresh->authenticationErrorText, pNewMemoryBuffer))
		return RSSL_RET_BUFFER_TOO_SMALL;


	if (pOldRefresh->flags & RDM_LG_RFF_HAS_CONN_CONFIG)
	{
		RsslRDMServerInfo *pNewServer, *pOldServer;
		RsslUInt32 i;

		/* Create copy of server list */
		if (!(pNewRefresh->serverList = (RsslRDMServerInfo*)rsslReserveAlignedBufferMemory(pNewMemoryBuffer, pNewRefresh->serverCount, sizeof(RsslRDMServerInfo))))
			return RSSL_RET_BUFFER_TOO_SMALL;

		pNewServer = pNewRefresh->serverList;
		pOldServer = pOldRefresh->serverList;
		for(i = 0; i < pNewRefresh->serverCount; ++i, ++pOldServer, ++pNewServer)
		{
			rsslClearRDMServerInfo(pNewServer);

			*pNewServer = *pOldServer;

			if (!rsslCopyBufferMemory(&pNewServer->hostname, &pOldServer->hostname, pNewMemoryBuffer))
				return RSSL_RET_BUFFER_TOO_SMALL;

		}
	}

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginStatus(RsslRDMLoginStatus *pNewStatus, RsslRDMLoginStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer)
{
	/* Copy members */
	*pNewStatus = *pOldStatus;

	/* Create separate copies of strings */
	if (pOldStatus->flags & RDM_LG_STF_HAS_STATE)
	{
		if (!rsslCopyBufferMemory(&pNewStatus->state.text, &pOldStatus->state.text, pNewMemoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}

	if (pOldStatus->flags & RDM_LG_STF_HAS_USERNAME)
	{
		if (!rsslCopyBufferMemory(&pNewStatus->userName, &pOldStatus->userName, pNewMemoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}
	
	if (pOldStatus->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
	{
		if (!rsslCopyBufferMemory(&pNewStatus->authenticationErrorText, &pOldStatus->authenticationErrorText, pNewMemoryBuffer))
			return RSSL_RET_BUFFER_TOO_SMALL;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_VA_API RsslRet rsslCopyRDMLoginRTT(RsslRDMLoginRTT *pNewRTT, RsslRDMLoginRTT *pOldRTT, RsslBuffer *pNewMemoryBuffer)
{
	*pNewRTT = *pOldRTT;
	return RSSL_RET_SUCCESS;
}


RSSL_VA_API RsslRet rsslCopyRDMLoginMsg(RsslRDMLoginMsg *pNewMsg, RsslRDMLoginMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer)
{
	switch(pOldMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REQUEST: return rsslCopyRDMLoginRequest(&pNewMsg->request, &pOldMsg->request, pNewMemoryBuffer);
		case RDM_LG_MT_CONSUMER_CONNECTION_STATUS: return rsslCopyRDMLoginConsumerConnectionStatus(&pNewMsg->consumerConnectionStatus, &pOldMsg->consumerConnectionStatus, pNewMemoryBuffer);
		case RDM_LG_MT_CLOSE: return rsslCopyRDMLoginClose(&pNewMsg->close, &pOldMsg->close, pNewMemoryBuffer);
		case RDM_LG_MT_REFRESH: return rsslCopyRDMLoginRefresh(&pNewMsg->refresh, &pOldMsg->refresh, pNewMemoryBuffer);
		case RDM_LG_MT_STATUS: return rsslCopyRDMLoginStatus(&pNewMsg->status, &pOldMsg->status, pNewMemoryBuffer);
		case RDM_LG_MT_RTT: return rsslCopyRDMLoginRTT(&pNewMsg->RTT, &pOldMsg->RTT, pNewMemoryBuffer);
		default: return RSSL_RET_INVALID_ARGUMENT;
	}

}
