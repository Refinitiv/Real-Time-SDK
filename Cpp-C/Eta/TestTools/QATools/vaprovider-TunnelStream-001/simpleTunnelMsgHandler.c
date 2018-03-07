/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This is a basic tunnel stream messages handler for the UPA Value Add consumer application.
 * It handles exchange of simple generic messages over the tunnel stream.
 */

//APIQA this file is QATools standalone no need to merge
/* This file alters VAProvider to process a generic message with a nested 
 * market price request and respond with a generic messagae which contains a market 
 * price refresh message. 
 */

#include "simpleTunnelMsgHandler.h"

#define TUNNEL_MSG_STREAM_ID 2000
#define TUNNEL_MSG_FREQUENCY 2
#define TUNNEL_DOMAIN_TYPE 199

#define MAX_STREAMID 1024

RsslInt			_inited = 0;
RsslUInt8		_domainType[MAX_STREAMID];
RsslUInt8		_msgClass[MAX_STREAMID];
RsslUInt		_updateCount[MAX_STREAMID];

void initTunnelStream()
{
	RsslUInt32 pos = 0;

	if ( _inited > 0 ) return;

	_inited = 1;

	for ( pos = 0; pos < MAX_STREAMID; ++pos )
	{
		_domainType[pos] = 0;
		_msgClass[pos] = RSSL_MC_REFRESH;
		_updateCount[pos] = 0;
	}
}

void encodeRefreshMsg( RsslUInt8 domainType, RsslInt32 streamId, RsslBuffer* pRsslBuffer )
{
	RsslEncodeIterator eIter;
	RsslRefreshMsg refreshMsg;
	RsslFieldList fList;
	RsslFieldEntry fEntry;
	RsslBuffer payload;
	char payloadBuffer[1024];
	RsslUInt value;
	RsslReal real;
	RsslDate date;
	RsslTime time;

	payload.data = &payloadBuffer[0];
	payload.length = 1024;

	rsslClearEncodeIterator( &eIter );
	rsslClearRefreshMsg( &refreshMsg );
	rsslClearFieldList( &fList );
	rsslClearFieldEntry( &fEntry );

	refreshMsg.flags |= RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE;

	refreshMsg.state.code = RSSL_SC_NONE;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.text.data = (char*)"RefreshComplete";
	refreshMsg.state.text.length = 15;

	refreshMsg.msgBase.domainType = domainType;
	refreshMsg.msgBase.streamId = streamId;

	refreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;

	refreshMsg.msgBase.msgKey.name.data = (char*)"item";
	refreshMsg.msgBase.msgKey.name.length = 4;

	refreshMsg.msgBase.msgKey.serviceId = 1;

	if ( domainType = RSSL_DMT_MARKET_PRICE )
	{
		refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

		rsslSetEncodeIteratorBuffer( &eIter, &payload );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		fList.flags |= RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		fList.fieldListNum = 1;
		fList.dictionaryId = 1;

		rsslEncodeFieldListInit( &eIter, &fList, 0, 0 );

		fEntry.dataType = RSSL_DT_UINT;
		fEntry.fieldId = 1;
		value = 1;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&value );

		fEntry.dataType = RSSL_DT_REAL;
		fEntry.fieldId = 6;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&real );

		fEntry.dataType = RSSL_DT_DATE;
		fEntry.fieldId = 16;
		rsslClearDate(&date);
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&date );

		fEntry.dataType = RSSL_DT_TIME;
		fEntry.fieldId = 18;
		rsslClearTime(&time);
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&time );

		rsslEncodeFieldListComplete( &eIter, RSSL_TRUE );

		payload.length = rsslGetEncodedBufferLength( &eIter );

		refreshMsg.msgBase.encDataBody = payload;

		rsslClearEncodeIterator( &eIter );
		rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		rsslEncodeMsg( &eIter, (RsslMsg*)&refreshMsg );

		pRsslBuffer->length = rsslGetEncodedBufferLength( &eIter );

	}
	else if ( domainType = RSSL_DMT_MARKET_BY_ORDER )
	{
		refreshMsg.msgBase.containerType = RSSL_DT_MAP;
		rsslSetEncodeIteratorBuffer( &eIter, &payload );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	}
	else
	{
	}
}

void encodeStatusMsg( RsslUInt8 domainType, RsslInt32 streamId, RsslBuffer* pRsslBuffer )
{
	RsslEncodeIterator eIter;
	RsslStatusMsg statusMsg;

	rsslClearEncodeIterator( &eIter );
	rsslClearStatusMsg( &statusMsg );

	statusMsg.flags |= RSSL_STMF_HAS_STATE;

	statusMsg.state.code = RSSL_SC_NONE;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.text.data = (char*)"Closed";
	statusMsg.state.text.length = 6;

	statusMsg.msgBase.domainType = domainType;
	statusMsg.msgBase.streamId = streamId;

	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer );
	rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

	rsslEncodeMsg( &eIter, (RsslMsg*)&statusMsg );

	pRsslBuffer->length = rsslGetEncodedBufferLength( &eIter );
}

void encodeUpdateMsg( RsslUInt8 domainType, RsslInt32 streamId, RsslBuffer* pRsslBuffer )
{
	RsslEncodeIterator eIter;
	RsslUpdateMsg updateMsg;
	RsslFieldList fList;
	RsslFieldEntry fEntry;
	RsslBuffer payload;
	char payloadBuffer[1024];
	RsslUInt value;
	RsslReal real;
	RsslDate date;
	RsslTime time;

	payload.data = &payloadBuffer[0];
	payload.length = 1024;

	rsslClearEncodeIterator( &eIter );
	rsslClearUpdateMsg( &updateMsg );
	rsslClearFieldList( &fList );
	rsslClearFieldEntry( &fEntry );

	updateMsg.flags |= RSSL_UPMF_DO_NOT_CONFLATE;

	updateMsg.msgBase.domainType = domainType;
	updateMsg.msgBase.streamId = streamId;

	if ( domainType = RSSL_DMT_MARKET_PRICE )
	{
		updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

		rsslSetEncodeIteratorBuffer( &eIter, &payload );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		fList.flags |= RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		fList.fieldListNum = 1;
		fList.dictionaryId = 1;

		rsslEncodeFieldListInit( &eIter, &fList, 0, 0 );

		fEntry.dataType = RSSL_DT_UINT;
		fEntry.fieldId = 1;
		value = 1;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&value );

		fEntry.dataType = RSSL_DT_REAL;
		fEntry.fieldId = 6;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&real );

		fEntry.dataType = RSSL_DT_DATE;
		fEntry.fieldId = 16;
		rsslClearDate(&date);
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&date );

		fEntry.dataType = RSSL_DT_TIME;
		fEntry.fieldId = 18;
		rsslClearTime(&time);
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &eIter, &fEntry, (void*)&time );

		rsslEncodeFieldListComplete( &eIter, RSSL_TRUE );

		payload.length = rsslGetEncodedBufferLength( &eIter );

		updateMsg.msgBase.encDataBody = payload;

		rsslClearEncodeIterator( &eIter );
		rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		rsslEncodeMsg( &eIter, (RsslMsg*)&updateMsg );

		pRsslBuffer->length = rsslGetEncodedBufferLength( &eIter );

	}
	else if ( domainType = RSSL_DMT_MARKET_BY_ORDER )
	{
		updateMsg.msgBase.containerType = RSSL_DT_MAP;

		rsslSetEncodeIteratorBuffer( &eIter, &payload );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );


		updateMsg.msgBase.encDataBody = payload;

		rsslClearEncodeIterator( &eIter );
		rsslSetEncodeIteratorBuffer( &eIter, &payload );
		rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

		rsslEncodeMsg( &eIter, (RsslMsg*)&updateMsg );

		pRsslBuffer->length = rsslGetEncodedBufferLength( &eIter );
	}
	else
	{
	}
}

void simpleTunnelMsgHandlerInit(SimpleTunnelMsgHandler *pMsgHandler,
		char *consumerName, RsslUInt8 domainType, RsslBool useAuthentication, RsslBool isProvider)
{
	initTunnelStream();

	tunnelStreamHandlerInit(&pMsgHandler->tunnelStreamHandler, consumerName, domainType, useAuthentication,
			RSSL_FALSE /* Not queue messaging */,
			simpleTunnelMsgHandlerProcessTunnelOpened,
			simpleTunnelMsgHandlerProcessTunnelClosed,
			simpleTunnelMsgHandlerConsumerMsgCallback,
			NULL);

	pMsgHandler->isProvider = isProvider;
}

void handleSimpleTunnelMsgHandler(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler)
{
	time_t currentTime = 0;
	RsslTunnelStream *pTunnelStream = pSimpleTunnelMsgHandler->tunnelStreamHandler.pTunnelStream;

	handleTunnelStreamHandler(pReactor, pReactorChannel, &pSimpleTunnelMsgHandler->tunnelStreamHandler);

	if (pTunnelStream == NULL)
		return;

	/* Don't try to send messages if tunnel is not established. */
	if (pTunnelStream->state.streamState != RSSL_STREAM_OPEN
			|| pTunnelStream->state.dataState != RSSL_DATA_OK)
		return;

	/* If authentication was requested and this is a provider, ensure the authentication response
	 * is sent before sending other messages. */
	if (pSimpleTunnelMsgHandler->waitingForAuthenticationRequest)
		return;


	/* If tunnel is open and some time has passed, send a message. */
	time(&currentTime);
	if (currentTime >= pSimpleTunnelMsgHandler->nextMsgTime)
	{
		simpleTunnelMsgHandlerSendMessage(pSimpleTunnelMsgHandler);
		pSimpleTunnelMsgHandler->nextMsgTime = currentTime + (time_t)TUNNEL_MSG_FREQUENCY;
	}
}

static void simpleTunnelMsgHandlerSendMessage(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler)
{
	RsslTunnelStream *pTunnelStream = pSimpleTunnelMsgHandler->tunnelStreamHandler.pTunnelStream;
	RsslReactorChannel *pReactorChannel = pTunnelStream->pReactorChannel;

	RsslTunnelStreamSubmitOptions submitOpts;
	RsslTunnelStreamGetBufferOptions bufferOpts;
	RsslEncodeIterator eIter;
	RsslBuffer *pBuffer;
	RsslRet ret, ret2;
	RsslErrorInfo errorInfo;
	RsslInt32 pos;

	for ( pos = 0; pos < MAX_STREAMID; ++pos )
	{
		if ( !_domainType[pos] ) continue;

		rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
		bufferOpts.size = 1024;
		if ((pBuffer = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferOpts, &errorInfo)) == NULL)
		{
			printf("rsslTunnelStreamGetBuffer failed: %s(%s)\n", rsslRetCodeToString(errorInfo.rsslError.rsslErrorId), &errorInfo.rsslError.text);
			return;
		}

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelStream->classOfService.common.protocolMajorVersion,
				pTunnelStream->classOfService.common.protocolMinorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, pBuffer);

		switch ( _msgClass[pos] )
		{
		case RSSL_MC_REFRESH :
			encodeRefreshMsg( _domainType[pos], pos, pBuffer );
			_msgClass[pos] = RSSL_MC_UPDATE;
			break;
		case RSSL_MC_UPDATE :
			encodeUpdateMsg( _domainType[pos], pos, pBuffer );
			if ( ++_updateCount[pos] > 10 )
			{
				_msgClass[pos] = RSSL_MC_STATUS;
				_updateCount[pos] = 0;
			}
			break;
		case RSSL_MC_STATUS :
			encodeStatusMsg( _domainType[pos], pos, pBuffer );
			_domainType[pos] = 0;
			break;
		default :
			continue;
		}

		/* Message encoding complete; submit it. */
		rsslClearTunnelStreamSubmitOptions(&submitOpts);
		submitOpts.containerType = RSSL_DT_MSG;
		if ((ret = rsslTunnelStreamSubmit(pTunnelStream, pBuffer, &submitOpts, &errorInfo)) != RSSL_RET_SUCCESS)
		{
			printf("rsslTunnelStreamSubmit(): Failed <%s>\n", errorInfo.rsslError.text);
			if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
				printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
			return;
		}

		++pSimpleTunnelMsgHandler->msgCount;
	}
}

RsslReactorCallbackRet simpleTunnelMsgHandlerConsumerMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	switch(pEvent->containerType)
	{
		case RSSL_DT_OPAQUE:
		{
			/* Read the text contained. */
			printf("Tunnel Stream %d received OPAQUE data: %.*s\n\n", 
					pTunnelStream->streamId, pEvent->pRsslBuffer->length, pEvent->pRsslBuffer->data);
			break;
		}


		default:
		{
			printf("Received unhandled buffer containerType %d(%s) in tunnel stream %d\n\n", 
				pEvent->containerType, rsslDataTypeToString(pEvent->containerType), pTunnelStream->streamId);
			break;
		}
	}	

	return RSSL_RC_CRET_SUCCESS; 
}

RsslReactorCallbackRet simpleTunnelMsgHandlerProviderMsgCallback(RsslTunnelStream *pTunnelStream, RsslTunnelStreamMsgEvent *pEvent)
{
	RsslMsg *pRsslMsg = pEvent->pRsslMsg;
	RsslReactorChannel *pReactorChannel = pTunnelStream->pReactorChannel;
	SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler = (SimpleTunnelMsgHandler*)pTunnelStream->userSpecPtr;

	/* Inspect the message and handle it accordingly. This is basically
	 * the same as the consumer's message callback but will respond to the
	 * client's authentication login message if one is received. */

	switch(pEvent->containerType)
	{
		case RSSL_DT_OPAQUE:
		{
			/* Read the text contained. */
			printf("Tunnel Stream %d received OPAQUE data: %.*s\n\n", 
					pTunnelStream->streamId, pEvent->pRsslBuffer->length, pEvent->pRsslBuffer->data);
			break;

		}

		case RSSL_DT_MSG:
		{

			switch(pRsslMsg->msgBase.domainType)
			{
				case RSSL_DMT_LOGIN:
				{
					RsslDecodeIterator dIter;
					RsslRDMLoginMsg loginMsg;
					char tmpMemory[1024];
					RsslBuffer memoryBuffer;
					RsslRet ret;
					RsslErrorInfo errorInfo;

					/* Use the ValueAdd RDM Decoder to decode the login message. */
					rsslClearDecodeIterator(&dIter);
					rsslSetDecodeIteratorRWFVersion(&dIter, pTunnelStream->classOfService.common.protocolMajorVersion,
						pTunnelStream->classOfService.common.protocolMinorVersion);
					rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

					rsslClearBuffer(&memoryBuffer);
					memoryBuffer.length = sizeof(tmpMemory);
					memoryBuffer.data = tmpMemory;

					if ((ret = rsslDecodeRDMLoginMsg(&dIter, pRsslMsg, &loginMsg, &memoryBuffer, &errorInfo))
						!= RSSL_RET_SUCCESS)
					{
						printf("rsslDecodeRDMLoginMsg() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
						break;
					}

					switch(loginMsg.rdmMsgBase.rdmMsgType)
					{
					case RDM_LG_MT_REQUEST:
						{
							/* This is a login request, likely the client's authentication
							* request. Send a response to establish the tunnel stream. */

							RsslRDMLoginRefresh loginRefresh;
							RsslTunnelStreamSubmitOptions submitOpts;
							RsslTunnelStreamGetBufferOptions bufferOpts;
							RsslBuffer *pBuffer;
							RsslEncodeIterator eIter;
							RsslRet ret, ret2;
							RsslRDMLoginRequest *pLoginRequest = &loginMsg.request;

							printf("Received login request on tunnel stream(ID %d) with stream ID %d.\n", 
								pTunnelStream->streamId, pLoginRequest->rdmMsgBase.streamId);

							if (pLoginRequest->flags & RDM_LG_RQF_NO_REFRESH)
								break;

							rsslClearTunnelStreamGetBufferOptions(&bufferOpts);
							bufferOpts.size = 1024;
							if ((pBuffer = rsslTunnelStreamGetBuffer(pTunnelStream, &bufferOpts, &errorInfo))
								== NULL)
							{
								printf("rsslTunnelStreamGetBuffer failed: %s(%s)\n", rsslRetCodeToString(errorInfo.rsslError.rsslErrorId), &errorInfo.rsslError.text);
								break;
							}

							rsslClearRDMLoginRefresh(&loginRefresh);

							/* Set state information */
							loginRefresh.state.streamState = RSSL_STREAM_OPEN;
							loginRefresh.state.dataState = RSSL_DATA_OK;
							loginRefresh.state.code = RSSL_SC_NONE;
							loginRefresh.state.text.data = (char*)"Tunnel login accepted.";
							loginRefresh.state.text.length = (RsslUInt32)strlen(loginRefresh.state.text.data);

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

							/* set the clear cache flag */
							loginRefresh.flags |= RDM_LG_RFF_CLEAR_CACHE;

							/* Leave all other parameters as default values. */

							/* Encode the refresh. */
							rsslClearEncodeIterator(&eIter);
							rsslSetEncodeIteratorRWFVersion(&eIter, pTunnelStream->classOfService.common.protocolMajorVersion,
								pTunnelStream->classOfService.common.protocolMinorVersion);
							if((ret = rsslSetEncodeIteratorBuffer(&eIter, pBuffer)) < RSSL_RET_SUCCESS)
							{
								printf("rsslSetEncodeIteratorBuffer(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &pBuffer->length, &errorInfo) != RSSL_RET_SUCCESS)
							{
								printf("rsslEncodeRDMLoginMsg(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							/* Message encoding complete; submit it. */
							rsslClearTunnelStreamSubmitOptions(&submitOpts);
							submitOpts.containerType = RSSL_DT_MSG;
							if ((ret = rsslTunnelStreamSubmit(pTunnelStream, pBuffer, &submitOpts, &errorInfo)) != RSSL_RET_SUCCESS)
							{
								printf("rsslTunnelStreamSubmit(): Failed <%s>\n", errorInfo.rsslError.text);
								if ((ret2 = rsslTunnelStreamReleaseBuffer(pBuffer, &errorInfo)) != RSSL_RET_SUCCESS)
									printf("rsslTunnelStreamReleaseBuffer(): Failed <%d:%s>\n", ret2, errorInfo.rsslError.text);
								break;
							}

							printf("Sent response to tunnel login request.\n\n");

							pSimpleTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_FALSE;

							break;
						}

					case RDM_LG_MT_CLOSE:
						{
							/* Login close message. */
							RsslRDMLoginClose *pLoginClose = &loginMsg.close;

							printf("Received login close on tunnel stream(ID %d) with stream ID %d.\n", 
								pTunnelStream->streamId, pLoginClose->rdmMsgBase.streamId);
							break;
						}
					}


					break;
				}

				case RSSL_DMT_MARKET_BY_ORDER :
				case RSSL_DMT_MARKET_PRICE :
				case RSSL_DMT_MARKET_BY_PRICE :
				{
					_domainType[pRsslMsg->msgBase.streamId] = pRsslMsg->msgBase.domainType;
					_msgClass[pRsslMsg->msgBase.streamId] = RSSL_MC_REFRESH;
					break;
				}
				default:
				{
					/* Don't recognize this message. */
					printf("Received unhandled message in TunnelStream with stream ID %d, class %u(%s) and domainType %u(%s)\n\n",
						pRsslMsg->msgBase.streamId, 
						pRsslMsg->msgBase.msgClass, rsslMsgClassToString(pRsslMsg->msgBase.msgClass),
						pRsslMsg->msgBase.domainType, rsslDomainTypeToString(pRsslMsg->msgBase.domainType));
					break;
				}
			}
			break;
		}

		default:
		{
			printf("Received unhandled buffer containerType %d(%s) in tunnel stream %d\n\n", 
				pEvent->containerType, rsslDataTypeToString(pEvent->containerType), pTunnelStream->streamId);
			break;
		}
	}


	return RSSL_RC_CRET_SUCCESS; 
}

static void simpleTunnelMsgHandlerProcessTunnelOpened(RsslTunnelStream *pTunnelStream)
{
	SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler = (SimpleTunnelMsgHandler*)pTunnelStream->userSpecPtr;

	time(&pSimpleTunnelMsgHandler->nextMsgTime);
	pSimpleTunnelMsgHandler->nextMsgTime += TUNNEL_MSG_FREQUENCY;
	pSimpleTunnelMsgHandler->msgCount = 0;

	/* If authentication was requested and this is a provider, ensure the authentication response
	 * is sent before sending other messages. */
	if (pTunnelStream->classOfService.authentication.type != RDM_COS_AU_NOT_REQUIRED
			&& pSimpleTunnelMsgHandler->isProvider)
		pSimpleTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_TRUE;
	else
		pSimpleTunnelMsgHandler->waitingForAuthenticationRequest = RSSL_FALSE;


}

static void simpleTunnelMsgHandlerProcessTunnelClosed(RsslTunnelStream *pTunnelStream)
{
	/* Nothing to do. */
}

void simpleTunnelMsgHandlerProcessNewStream(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent)
{
	RsslErrorInfo errorInfo;
	RsslRet ret;
	RsslClassOfService cos;
	RsslBool rejectWithClassOfService = RSSL_FALSE;
	char *rejectString = NULL;

	printf("Received TunnelStream request on Stream ID %d.\n", pEvent->streamId);

	if (pSimpleTunnelMsgHandler->tunnelStreamHandler.tunnelStreamOpenRequested)
		rejectString = (char*)"Consumer already has a tunnel stream open. This provider example only supports one tunnel stream.";

	if (rejectString == NULL)
	{
		if ((rejectString = simpleTunnelMsgHandlerCheckRequestedClassOfService(pSimpleTunnelMsgHandler,
			pEvent, &cos)) != NULL)
			rejectWithClassOfService = RSSL_TRUE;
	}

	if (rejectString == NULL)
	{
		/* Accept the tunnel stream. */
		RsslReactorAcceptTunnelStreamOptions acceptOpts;

		rsslClearReactorAcceptTunnelStreamOptions(&acceptOpts);

		acceptOpts.statusEventCallback = tunnelStreamStatusEventCallback;
		acceptOpts.defaultMsgCallback = simpleTunnelMsgHandlerProviderMsgCallback;
		acceptOpts.userSpecPtr = (void*)pSimpleTunnelMsgHandler;

		acceptOpts.classOfService.authentication.type = cos.authentication.type;
		acceptOpts.classOfService.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
		acceptOpts.classOfService.dataIntegrity.type = RDM_COS_DI_RELIABLE;

		/* Use whichever protocol minor version is lower. */
		if (cos.common.protocolMinorVersion < acceptOpts.classOfService.common.protocolMinorVersion)
			acceptOpts.classOfService.common.protocolMinorVersion = cos.common.protocolMinorVersion;

		if ((ret = rsslReactorAcceptTunnelStream(pEvent, &acceptOpts, &errorInfo)) != RSSL_RET_SUCCESS)
			printf("rsslReactorAcceptTunnelStream() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);

		pSimpleTunnelMsgHandler->tunnelStreamHandler.tunnelStreamOpenRequested = RSSL_TRUE;
	}
	else
	{
		/* Something didn't match our expectations, so reject the stream. */
		RsslReactorRejectTunnelStreamOptions rejectOpts;
		RsslClassOfService expectedCos;
		rsslClearReactorRejectTunnelStreamOptions(&rejectOpts);

		rejectOpts.state.streamState = RSSL_STREAM_CLOSED;
		rejectOpts.state.dataState = RSSL_DATA_SUSPECT;
		rejectOpts.state.text.data = rejectString;
		rejectOpts.state.text.length = (RsslUInt32)strlen(rejectOpts.state.text.data);

		if (rejectWithClassOfService)
		{
			/* Since we're rejecting due a Class-of-Service mismatch, 
			 * send a redirect state to the consumer. */
			rejectOpts.state.streamState = RSSL_STREAM_REDIRECTED;
			rejectOpts.state.dataState = RSSL_DATA_SUSPECT;

			/* Set what the class of service is expected to be. */
			rsslClearClassOfService(&expectedCos);
			expectedCos.authentication.type = cos.authentication.type;
			expectedCos.flowControl.type = RDM_COS_FC_BIDIRECTIONAL;
			expectedCos.dataIntegrity.type = RDM_COS_DI_RELIABLE;
			rejectOpts.pCos = &expectedCos;
		}
		else
		{
			rejectOpts.state.streamState = RSSL_STREAM_CLOSED;
			rejectOpts.state.dataState = RSSL_DATA_SUSPECT;
		}

		if ((ret = rsslReactorRejectTunnelStream(pEvent, &rejectOpts, &errorInfo)) != RSSL_RET_SUCCESS)
			printf("rsslReactorRejectTunnelStream() failed: %s(%s)\n", rsslRetCodeToString(ret), errorInfo.rsslError.text);
	}
}

char* simpleTunnelMsgHandlerCheckRequestedClassOfService(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler,
		RsslTunnelStreamRequestEvent *pEvent, RsslClassOfService *pCos)
{
	RsslErrorInfo errorInfo;

	/* 
	 * The class of service filter can be inspected to detect early if
	 * certain members of the ClassOfService are not present (and therefore
	 * have their default values).
	 *
	 * if ((pEvent->classOfServiceFilter & RDM_COS_COMMON_PROPERTIES_FLAG) == 0)
	 *	return "This provider requires the common ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_AUTHENTICATION_FLAG) == 0)
	 *	return "This provider requires the authentication ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_FLOW_CONTROL_FLAG) == 0)
	 *	return "This provider requires the flow control ClassOfService filter.";
	 * else if ((pEvent->classOfServiceFilter & RDM_COS_DATA_INTEGRITY_FLAG) == 0)
	 *	return "This provider requires the data integrity ClassOfService filter.";
	 */

	/* Try to decode the class of service. */
	if (rsslTunnelStreamRequestGetCos(pEvent, pCos, &errorInfo) != RSSL_RET_SUCCESS)
	{
		printf("rsslTunnelStreamRequestGetCos failed: %s(%s)\n", rsslRetCodeToString(errorInfo.rsslError.rsslErrorId), &errorInfo.rsslError.text);
		return (char*)"Failed to decode class of service.";
	}

	/* Check if it matches what we require. */

	if (pCos->common.protocolType != RSSL_RWF_PROTOCOL_TYPE)
		return (char*)"This provider doesn't support this protocol type.";

	if (pCos->common.protocolMajorVersion != RSSL_RWF_MAJOR_VERSION)
		return (char*)"This provider doesn't support this wire format major version.";

	if (pCos->authentication.type != RDM_COS_AU_NOT_REQUIRED
			&& pCos->authentication.type != RDM_COS_AU_OMM_LOGIN)
		return (char*)"This provider doesn't support this type of authentication.";

	if (pCos->flowControl.type != RDM_COS_FC_BIDIRECTIONAL)
		return (char*)"This provider requires bidirectional flow control.";

	if (pCos->guarantee.type != RDM_COS_GU_NONE)
		return (char*)"This provider does not support guaranteed streams.";

	return NULL;
}

void simpleTunnelMsgHandlerCloseStreams(SimpleTunnelMsgHandler *pSimpleTunnelMsgHandler)
{
	tunnelStreamHandlerCloseStreams(&pSimpleTunnelMsgHandler->tunnelStreamHandler);
}

//END APIQA
