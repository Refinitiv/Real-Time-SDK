/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ADHSimulator.h"

// Debug output macro
//#define ADH_SIMULATOR_DEBUG_OUTPUT 1

static EmaString dictionaryLocation("./TestRDMFieldDictionary");

ADHSimulator::ADHSimulator(ADHSimulatorOptions& adhOptions) :
	options(adhOptions),
	pReactor(nullptr),
	rsslSrvr(nullptr),
	stopThreadFlag(false),
	isRunningFlag(false),
	counters()
{
	char tmpErrorBuffer[1024];
	RsslBuffer errorBuffer = { sizeof(tmpErrorBuffer), tmpErrorBuffer };
	RsslCreateReactorOptions* pReactorOptions = options.pReactorOptions;

	RsslCreateReactorOptions reactorOpts;
	if (!pReactorOptions)
	{
		rsslClearCreateReactorOptions(&reactorOpts);
		pReactorOptions = &reactorOpts;
	}

	if ( !(pReactor = rsslCreateReactor(pReactorOptions, &rsslErrorInfo)) )
	{
		std::cout << "Reactor creation failed: " << rsslErrorInfo.rsslError.text << std::endl;
	}

	rsslClearDataDictionary(&dictionary);
	
	if(rsslLoadFieldDictionary(dictionaryLocation.c_str(), &dictionary, &errorBuffer) != RSSL_RET_SUCCESS)
	{
		std::cout << "Failed to load field dictionary: " << errorBuffer.data << std::endl;
	}

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);
}

ADHSimulator::~ADHSimulator()
{
	stop();

	if (pReactor != nullptr)
	{
		if ( rsslDestroyReactor(pReactor, &rsslErrorInfo) != RSSL_RET_SUCCESS )
		{
			std::cout << "Failed to destroy reactor: " << rsslErrorInfo.rsslError.text << std::endl;
		}
		pReactor = nullptr;
	}

	// Free the message list and clear the message queue.
	while(_messageList.size() > 0)
	{
		RsslMsg* pMsg = _messageList[0];
		if (pMsg)
		{
			rsslReleaseCopiedMsg(pMsg);
		}
		_messageList.removePosition(0);
	}

	rsslDeleteDataDictionary(&dictionary);
}

void ADHSimulator::closeChannels()
{
	int i;
	/* clean up client sessions */
	for (i = 0; i < clientList.size(); i++)
	{
		if (clientList[i].pReactorChannel != nullptr)
		{
			closeChannel(clientList[i].pReactorChannel);
			clientList[i].clear();
		}
	}
	clientList.clear();
}

/* Close the RsslReactorChannel. Cleans up associated resources */
void ADHSimulator::closeChannel( RsslReactorChannel* pReactorChannel )
{
	RsslErrorInfo rsslErrorInfo;

	FD_CLR(pReactorChannel->socketId, &readFds);
	FD_CLR(pReactorChannel->socketId, &exceptFds);

	if (rsslReactorCloseChannel(pReactor, pReactorChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		std::cout << "rsslReactorCloseChannel() failed: " << rsslErrorInfo.rsslError.text << std::endl;
	}
	return;
}

/* Close a client session for a channel */
void ADHSimulator::closeClientSessionForChannel(RsslReactorChannel* pReactorChannel)
{
	int i;

	/* Locate for the client session */
	for (i = 0; i < clientList.size(); i++)
	{
		if (clientList[i].pReactorChannel == pReactorChannel)
		{
			clientList[i].clear();
			break;
		}
	}

	closeChannel(pReactorChannel);
	return;
}


bool ADHSimulator::start()
{
	if ( !pReactor )
	{
		std::cout << "start() failed. pReactor: " << (pReactor ? "ok" : "(null)") << std::endl;
		return false;
	}

	stopThreadFlag = false;

	tr = std::thread(&ADHSimulator::runThreadADH, this);

#ifdef ADH_SIMULATOR_DEBUG_OUTPUT
	std::ostringstream oss;
	oss << "ADH Simulator start(). port: " << options.portNo;
	std::cout << oss.str() << std::endl;
#endif // ADH_SIMULATOR_DEBUG_OUTPUT

	return true;
}

void ADHSimulator::stop()
{
	if ( isRunningFlag.load() && !stopThreadFlag.load() )
	{
		stopThreadFlag = true;  // signal thread to stop
		tr.join();

		isRunningFlag = false;
	}

#ifdef ADH_SIMULATOR_DEBUG_OUTPUT
	std::ostringstream oss;
	oss << "ADH Simulator stop(). port: " << options.portNo
		<< "; isRunningFlag: " << isRunningFlag
		<< "; stopThreadFlag: " << stopThreadFlag;
	std::cout << oss.str() << std::endl;
#endif // ADH_SIMULATOR_DEBUG_OUTPUT
}

void ADHSimulator::runThreadADH()
{
	if ( !pReactor )
	{
		std::cout << "runThreadADH() failed. pReactor is null." << std::endl;
		return;
	}

	RsslBindOptions sopts;
	rsslClearBindOpts(&sopts);

	sopts = RSSL_INIT_BIND_OPTS;
	sopts.guaranteedOutputBuffers = 2000;
	sopts.serviceName = options.portNo;
	sopts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	sopts.minorVersion = RSSL_RWF_MINOR_VERSION;
	sopts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	sopts.connectionType = RSSL_CONN_TYPE_SOCKET;
	sopts.compressionType = options.compressionType;
	sopts.compressionLevel = options.compressionLevel;

	if ( !(rsslSrvr = rsslBind(&sopts, &rsslErrorInfo.rsslError)) )
	{
		std::cout << "rsslBind failed. Server could not be created: " << rsslErrorInfo.rsslError.text << std::endl;
		return;
	}

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	FD_SET(rsslSrvr->socketId, &readFds);
	FD_SET(rsslSrvr->socketId, &writeFds);
	FD_SET(rsslSrvr->socketId, &exceptFds);

	FD_SET(pReactor->eventFd, &readFds);
	FD_SET(pReactor->eventFd, &writeFds);

	struct timeval time_interval;
	fd_set	useRead;
	fd_set	useExcept;

	int selRet = 0;
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslReactorDispatchOptions dispatchOpts;
	RsslReactorOMMProviderRole providerRole;

	rsslClearOMMProviderRole(&providerRole);
	providerRole.base.channelEventCallback = channelEventCallback;
	providerRole.base.defaultMsgCallback = defaultMsgCallback;
	providerRole.loginMsgCallback = loginMsgCallback;

	FD_ZERO(&useRead);
	FD_ZERO(&useExcept);

	isRunningFlag = true;

#ifdef ADH_SIMULATOR_DEBUG_OUTPUT
	std::ostringstream oss;
	oss << "ADH Simulator runThreadADH(). port " << options.portNo
		<< "; isRunningFlag: " << isRunningFlag
		<< "; stopThreadFlag: " << stopThreadFlag;
	std::cout << oss.str() << std::endl;
#endif // ADH_SIMULATOR_DEBUG_OUTPUT

	/* Main thread of ADH Simulator */
	while ( !stopThreadFlag.load() )
	{
		useRead = readFds;
		useExcept = exceptFds;

		time_interval.tv_sec = 0;
		time_interval.tv_usec = 200000;

		selRet = select(FD_SETSIZE, &useRead, NULL, &useExcept, &time_interval);

		if ( stopThreadFlag.load() )
			break;

		if ( selRet > 0 )
		{
			// Accept connection here
			if ( FD_ISSET(rsslSrvr->socketId, &useRead) )
			{
				RsslReactorAcceptOptions acceptOpts;
				rsslClearReactorAcceptOptions(&acceptOpts);

				ADHClientSessionInfo* pClientSessionInfo = getNewClientSession();

				/* Accept or Reject the channel */
				if (pClientSessionInfo)
					acceptOpts.rsslAcceptOptions.userSpecPtr = pClientSessionInfo;
				else
					acceptOpts.rsslAcceptOptions.nakMount = RSSL_TRUE;

				if ( (ret = rsslReactorAccept(pReactor, rsslSrvr, &acceptOpts, (RsslReactorChannelRole*)&providerRole, &rsslErrorInfo))
						!= RSSL_RET_SUCCESS )
				{
					std::cout << "rsslReactorAccept failed: " << rsslErrorInfo.rsslError.text << std::endl;
					break;
				}

				std::cout << "Accepted new connection. port: " << options.portNo << std::endl;
			}
		}
		else if ( selRet == 0 )
		{
			// Timeout occurred
		}
		else
		{
			// select() failure
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR)
				continue;
			std::cout << "select() failed. " << WSAGetLastError() << std::endl;
#else
			if (errno == EINTR)
				continue;
			std::cout << "select() failed. errno: " << errno << std::endl;
#endif
			break;
		}

		rsslClearReactorDispatchOptions(&dispatchOpts);

		while (!stopThreadFlag.load()
			&& (ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS)
			;

		if (ret < RSSL_RET_SUCCESS)
		{
			std::cout << "rsslReactorDispatch(). failed: " << rsslErrorInfo.rsslError.text
				<< "; ret: " << ret << "; selRet: " << selRet << std::endl;
			break;
		}
	}

	FD_CLR(rsslSrvr->socketId, &readFds);
	FD_CLR(rsslSrvr->socketId, &exceptFds);

	int i;
	/* clean up client sessions */
	for (i = 0; i < clientList.size(); i++)
	{
		if ( clientList[i].pReactorChannel != nullptr )
		{
			closeChannel(clientList[i].pReactorChannel);
			clientList[i].clear();
		}
	}
	clientList.clear();

	if (rsslSrvr)
	{
		if ( (ret = rsslCloseServer(rsslSrvr, &rsslErrorInfo.rsslError)) != RSSL_RET_SUCCESS )
		{
			std::cout << "rsslCloseServer() failed: " << rsslErrorInfo.rsslError.text << "; ret:" << ret << std::endl;
		}
		rsslSrvr = nullptr;
		//std::cout << "runThreadADH()  after rsslCloseServer. ret:" << ret << "; port " << options.portNo << std::endl;
	}

	isRunningFlag = false;
}

ADHClientSessionInfo* ADHSimulator::getNewClientSession()
{
	ADHClientSessionInfo* pClientSessionInfo = nullptr;
	int i;

	/* find an available client session */
	for (i = 0; i < clientList.size(); i++)
	{
		if ( !clientList[i].isInUse )
		{
			pClientSessionInfo = &clientList[i];
			pClientSessionInfo->isInUse = true;
			break;
		}
	}

	/* no available client session, create a new one */
	if (pClientSessionInfo == nullptr)
	{
		//ADHClientSessionInfo newClientSessionInfo(this, nullptr);
		clientList.emplace_back(this, nullptr);
		pClientSessionInfo = &clientList.back();
		pClientSessionInfo->isInUse = true;
	}

	return pClientSessionInfo;
}


RsslRet ADHSimulator::sendMessage(RsslReactor* pReactor, RsslReactorChannel* chnl, RsslBuffer* msgBuf)
{
	RsslErrorInfo rsslErrorInfo;
	RsslRet	retval = 0;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
	RsslReactorSubmitOptions submitOpts;

	rsslClearReactorSubmitOptions(&submitOpts);

	/* send the request */
	if ((retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
	{
		while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo);

		if (retval < RSSL_RET_SUCCESS)	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			std::cout << "rsslReactorSubmit() failed with return code: " << retval << "; " << rsslErrorInfo.rsslError.text << std::endl;
			if ((retval = rsslReactorReleaseBuffer(chnl, msgBuf, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
				std::cout << "rsslReactorReleaseBuffer() failed with return code: " << retval << "; " << rsslErrorInfo.rsslError.text << std::endl;

			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslRet ADHSimulator::sendMessage(RsslReactor* pReactor, RsslReactorChannel* chnl, RsslMsg* pMsg)
{
	RsslErrorInfo rsslErrorInfo;
	RsslRet	retval = 0;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
	RsslReactorSubmitMsgOptions submitOpts;

	rsslClearReactorSubmitMsgOptions(&submitOpts);

	submitOpts.majorVersion = chnl->majorVersion;
	submitOpts.minorVersion = chnl->minorVersion;
	submitOpts.pRsslMsg = pMsg;

	/* send the request */
	if ((retval = rsslReactorSubmitMsg(pReactor, chnl, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
	{
		while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			retval = rsslReactorSubmitMsg(pReactor, chnl, &submitOpts, &rsslErrorInfo);

		if (retval < RSSL_RET_SUCCESS)	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			std::cout << "rsslReactorSubmit() failed with return code: " << retval << "; " << rsslErrorInfo.rsslError.text << std::endl;

			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/* Callbacks*/
RsslReactorCallbackRet
ADHSimulator::loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent)
{
	ADHClientSessionInfo* pClientSessionInfo = static_cast<ADHClientSessionInfo*>(pChannel->userSpecPtr);
	ADHSimulator* pAdhSim = pClientSessionInfo->pADHSimulator;

	char appIdBuf[3] = { '1', '0', '0' };
	char appAppNameBuf[6] = { 'A', 'd', 'h', 'S', 'i', 'm' };

	/* Accept login */
	RsslRDMLoginMsg* pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;
	RsslRDMLoginRequest* pLoginRequest = &pLoginMsg->request;

	if (pAdhSim->options.saveMsgs.load() == true)
	{
		RsslMsg* pRsslMsg = nullptr;

		pRsslMsg = rsslCopyMsg(pLoginMsgEvent->baseMsgEvent.pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, nullptr);

		if (pRsslMsg == nullptr)
		{
			std::cout << "Error on message copy" << std::endl;
			return RSSL_RC_CRET_SUCCESS;
		}

		pAdhSim->_messageQueue.push_back(pRsslMsg);
		pAdhSim->_messageList.push_back(pRsslMsg);

	}

	//std::cout << "ADHSimulator::loginMsgCallback: "
	//	<< "Received login message of type " << std::to_string(pLoginMsg->rdmMsgBase.rdmMsgType)
	//	<< " on channel " << pChannel->socketId
	//	<< std::endl;
	if (pLoginMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST)
	{
		RsslErrorInfo rsslErrorInfo;
		RsslBuffer* msgBuf = 0;
		RsslUInt32 ipAddress = 0;
		RsslRet ret;

		/* get a buffer for the login response */
		msgBuf = rsslReactorGetBuffer(pChannel, 4096, RSSL_FALSE, &rsslErrorInfo);

		if (msgBuf != NULL)
		{
			if (pAdhSim->options.rejectLogin == false)
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
				loginRefresh.applicationId.data = appIdBuf;
				loginRefresh.applicationId.length = 3;

				loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
				loginRefresh.applicationName.data = appAppNameBuf;
				loginRefresh.applicationName.length = 6;

				loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;
				loginRefresh.position = pLoginRequest->position;


				if (pAdhSim->options.supportProviderDictionaryDownload.load() == true)
				{
					/* Indicate support for provider dictionary download */
					loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD;
					loginRefresh.supportProviderDictionaryDownload = 1;
				}
				else
				{
					/* Indicate support for provider dictionary download */
					loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD;
					loginRefresh.supportProviderDictionaryDownload = 0;
				}

				/* Leave all other parameters as default values. */

				/* Encode the refresh. */
				rsslClearEncodeIterator(&eIter);
				rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
				if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					std::cout << "rsslSetEncodeIteratorBuffer() failed with return code: " << ret << std::endl;
					return RSSL_RC_CRET_FAILURE;
				}
				if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					std::cout << "rsslEncodeRDMLoginRefresh() failed: " << rsslErrorInfo.rsslError.text << " (" << rsslErrorInfo.errorLocation << ")" << std::endl;
					return RSSL_RC_CRET_FAILURE;
				}

				/* Send the refresh. */
				if (sendMessage(pReactor, pChannel, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RC_CRET_FAILURE;
			}
			else
			{
				RsslRDMLoginStatus loginStatus;
				RsslEncodeIterator eIter;

				rsslClearRDMLoginStatus(&loginStatus);


				/* Set state information */
				loginStatus.flags |= RDM_LG_STF_HAS_STATE;
				loginStatus.state.streamState = RSSL_STREAM_CLOSED;
				loginStatus.state.dataState = RSSL_DATA_SUSPECT;
				loginStatus.state.code = RSSL_SC_NONE;

				/* Set stream ID */
				loginStatus.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;

				/* Leave all other parameters as default values. */

				/* Encode the refresh. */
				rsslClearEncodeIterator(&eIter);
				rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
				if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					std::cout << "rsslSetEncodeIteratorBuffer() failed with return code: " << ret << std::endl;
					return RSSL_RC_CRET_FAILURE;
				}
				if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginStatus, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					std::cout << "rsslEncodeRDMLoginStatus() failed: " << rsslErrorInfo.rsslError.text << " (" << rsslErrorInfo.errorLocation << ")" << std::endl;
					return RSSL_RC_CRET_FAILURE;
				}

				/* Send the refresh. */
				if (sendMessage(pReactor, pChannel, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RC_CRET_FAILURE;
			}
		}
		else
		{
			std::cout << "rsslReactorGetBuffer(): Failed: " << rsslErrorInfo.rsslError.text << std::endl;
			return RSSL_RC_CRET_FAILURE;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet
ADHSimulator::defaultMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pRsslMsgEvent)
{
	ADHClientSessionInfo* pClientSessionInfo = static_cast<ADHClientSessionInfo*>(pReactorChannel->userSpecPtr);
	ADHSimulator* pAdhSim = pClientSessionInfo->pADHSimulator;
	RsslRet ret = RSSL_RET_SUCCESS;

	//char tmpChar[255];
	//RsslBuffer tmpBuf = { sizeof(tmpChar), tmpChar };

	//std::cout << "ADHSimulator::defaultMsgCallback: "
	//	<< "Received message of class: " << std::to_string(pRsslMsgEvent->pRsslMsg->msgBase.msgClass)
	//	<< " " << rsslMsgClassToString(pRsslMsgEvent->pRsslMsg->msgBase.msgClass)
	//	<< ", domain: " << rsslDomainTypeToString(pRsslMsgEvent->pRsslMsg->msgBase.domainType)
	//	<< ", streamId: " << std::to_string(pRsslMsgEvent->pRsslMsg->msgBase.streamId)
	//	<< "  on channel " << pReactorChannel->socketId
	//	<< std::endl;

	if (pAdhSim->options.saveMsgs.load() == true)
	{
		RsslMsg* pRsslMsg = nullptr;

		pRsslMsg = rsslCopyMsg(pRsslMsgEvent->pRsslMsg, RSSL_CMF_ALL_FLAGS, 0, nullptr);

		if (pRsslMsg == nullptr)
		{
			std::cout << "Error on message copy" << std::endl;
			return RSSL_RC_CRET_SUCCESS;
		}

		pAdhSim->_messageQueue.push_back(pRsslMsg);
		pAdhSim->_messageList.push_back(pRsslMsg);
	
	}

	switch ( pRsslMsgEvent->pRsslMsg->msgBase.msgClass )
	{
	case RSSL_MC_REQUEST:	pAdhSim->counters.countRequest++; break;
	case RSSL_MC_REFRESH:	pAdhSim->counters.countRefresh++; break;
	case RSSL_MC_STATUS:	pAdhSim->counters.countStatus++; break;
	case RSSL_MC_UPDATE:	pAdhSim->counters.countUpdate++; break;
	case RSSL_MC_CLOSE:		pAdhSim->counters.countClose++; break;
	case RSSL_MC_ACK:		pAdhSim->counters.countAck++; break;
	case RSSL_MC_GENERIC:	pAdhSim->counters.countGeneric++; break;
	}

	switch ( pRsslMsgEvent->pRsslMsg->msgBase.msgClass )
	{
	case RSSL_MC_REQUEST:
		if (pRsslMsgEvent->pRsslMsg->msgBase.domainType == RSSL_DMT_DICTIONARY)
		{
			RsslErrorInfo errorInfo;
			RsslRDMDictionaryRequest dictRequest;
			rsslClearRDMDictionaryRequest(&dictRequest);
			char tmpMemBuffer[1024];
			RsslBuffer memBuffer = { sizeof(tmpMemBuffer), tmpMemBuffer };

			RsslDecodeIterator decodeIter;
			rsslClearDecodeIterator(&decodeIter);

			rsslSetDecodeIteratorRWFVersion(&decodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
			rsslSetDecodeIteratorBuffer(&decodeIter, &pRsslMsgEvent->pRsslMsg->msgBase.encDataBody);

			rsslDecodeRDMDictionaryMsg(&decodeIter, pRsslMsgEvent->pRsslMsg, (RsslRDMDictionaryMsg*)&dictRequest, &memBuffer, &errorInfo);

			RsslRDMDictionaryRefresh dictRefresh;
			rsslClearRDMDictionaryRefresh(&dictRefresh);

			dictRefresh.pDictionary = &pAdhSim->dictionary;
			dictRefresh.rdmMsgBase.streamId = pRsslMsgEvent->pRsslMsg->msgBase.streamId;
			dictRefresh.dictionaryName = pRsslMsgEvent->pRsslMsg->msgBase.msgKey.name;
			dictRefresh.type = RDM_DICTIONARY_FIELD_DEFINITIONS;
			dictRefresh.state.streamState = RSSL_STREAM_OPEN;
			dictRefresh.state.dataState = RSSL_DATA_OK;
			dictRefresh.state.code = RSSL_SC_NONE;
			dictRefresh.verbosity = dictRequest.verbosity;
			dictRefresh.serviceId = dictRequest.serviceId;
			dictRefresh.flags = RDM_DC_RFF_SOLICITED;

			RsslReactorSubmitMsgOptions submitOpts;

			rsslClearReactorSubmitMsgOptions(&submitOpts);
			submitOpts.majorVersion = pReactorChannel->majorVersion;
			submitOpts.minorVersion = pReactorChannel->minorVersion;
			submitOpts.pRDMMsg = (RsslRDMMsg*)&dictRefresh;

			rsslReactorSubmitMsg(pReactor, pReactorChannel, &submitOpts, &errorInfo);
		}
		break;
	case RSSL_MC_REFRESH:
		//ret = rsslRefreshFlagsToOmmString(&tmpBuf, pRsslMsgEvent->pRsslMsg->msgBase.msgKey.flags);
		//std::cout << " Flags: " << tmpBuf.data << std::endl;
		break;
	case RSSL_MC_GENERIC:
	{
		if (pAdhSim->options.sendGenericMsg.load())
		{
			ret = pAdhSim->sendGenericMessageLogin(pReactor, pReactorChannel, pRsslMsgEvent);
			//std::cout << "ADHSimulator::defaultMsgCallback: Sent generic message on LOGIN domain, ret = " << ret << std::endl;
		}
		break;
	}
	default:
		break;
	}
	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet
ADHSimulator::channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pChannelEvent)
{
	ADHClientSessionInfo* pClientSessionInfo = static_cast<ADHClientSessionInfo*>(pReactorChannel->userSpecPtr);
	ADHSimulator* pAdhSim = pClientSessionInfo->pADHSimulator;

	switch (pChannelEvent->channelEventType)
	{
	case RSSL_RC_CET_CHANNEL_UP:
		pClientSessionInfo->pReactorChannel = pReactorChannel;
		FD_SET(pReactorChannel->socketId, &pAdhSim->readFds);
		FD_SET(pReactorChannel->socketId, &pAdhSim->exceptFds);
		break;
	case RSSL_RC_CET_CHANNEL_DOWN:
		pAdhSim->closeClientSessionForChannel(pReactorChannel);
		break;
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		FD_CLR(pReactorChannel->socketId, &pAdhSim->readFds);
		FD_CLR(pReactorChannel->socketId, &pAdhSim->exceptFds);
		break;
	case RSSL_RC_CET_FD_CHANGE:
		FD_CLR(pReactorChannel->oldSocketId, &pAdhSim->readFds);
		FD_CLR(pReactorChannel->oldSocketId, &pAdhSim->exceptFds);
		FD_SET(pReactorChannel->socketId, &pAdhSim->readFds);
		FD_SET(pReactorChannel->socketId, &pAdhSim->exceptFds);
		break;
	}
	return RSSL_RC_CRET_SUCCESS;
}

/*
* Send generic message on the LOGIN domain
* ADH send RTT generic messages on the LOGIN domain
*/
RsslRet ADHSimulator::sendGenericMessageLogin(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pRsslMsgEvent)
{
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret = RSSL_RET_SUCCESS;

	RsslMsg* pRsslMsg = pRsslMsgEvent->pRsslMsg;

	RsslGenericMsg genericMsg;
	RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;

	char nameChar[128];
	char nameValue[20];
	RsslBuffer* msgBuf = nullptr;

	char valUInt[4] = "2";
	RsslBuffer pEncUInt;
	pEncUInt.data = valUInt;
	pEncUInt.length = 2;

	RsslInt      eData = 1;
	RsslElementList elemList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry elemEntry = RSSL_INIT_ELEMENT_ENTRY;

	/* get a buffer for the item request reject status */
	msgBuf = rsslReactorGetBuffer(pReactorChannel, 4096, RSSL_FALSE, &rsslErrorInfo);

	if (msgBuf != NULL)
	{
		/* encode generic msg login */
		rsslClearGenericMsg(&genericMsg);
		genericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
		genericMsg.msgBase.domainType = RSSL_DMT_LOGIN;
		genericMsg.msgBase.streamId = pRsslMsg->msgBase.streamId;
		genericMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
		genericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;

		if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME
			&& pRsslMsg->msgBase.msgKey.name.data != nullptr
			&& pRsslMsg->msgBase.msgKey.name.length > 0)
		{
			size_t nameLen = pRsslMsg->msgBase.msgKey.name.length < (sizeof(nameChar) - 1) ?
				pRsslMsg->msgBase.msgKey.name.length : (sizeof(nameChar) - 1);
			memcpy(nameChar, pRsslMsg->msgBase.msgKey.name.data, nameLen);
			nameChar[nameLen] = '\0';
		}
		else
		{
			snprintf(nameChar, sizeof(nameChar), "%s", "genericMsgInGeneral");
		}

		genericMsg.msgBase.msgKey.name.data = nameChar;
		genericMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen(nameChar);
		genericMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;

		snprintf(nameValue, sizeof(nameValue), "%s", "Ticks");

		rsslClearEncodeIterator(&encodeIter);
		rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);

		if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
		{
			rsslReactorReleaseBuffer(pReactorChannel, msgBuf, &rsslErrorInfo);
			std::cout << "ADHSimulator::sendGenericMessageLogin rsslSetEncodeIteratorBuffer() failed with return code: " << ret << std::endl;
			return RSSL_RET_FAILURE;
		}

		if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&genericMsg, 0)) < RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin rsslEncodeMsgInit() failed with return code: " << ret << std::endl;
			return ret;
		}
		rsslClearElementList(&elemList);
		elemList.elementListNum = 1;
		elemList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		ret = rsslEncodeElementListInit(&encodeIter, &elemList, 0, 0);
		if (ret < RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin Error "
				<< rsslRetCodeToString(ret) << " (" << ret
				<< ") encountered with rsslEncodeElementListInit. Error Text: " << rsslRetCodeInfo(ret)
				<< std::endl;
		}

		elemEntry.dataType = RSSL_DT_UINT;
		elemEntry.name.data = nameValue;
		elemEntry.name.length = (RsslUInt32)strlen(nameValue);
		RsslUInt tempUInt = 2;
		ret = rsslEncodeElementEntry(&encodeIter, &elemEntry, &tempUInt);
		if (ret < RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin Error "
				<< rsslRetCodeToString(ret) << " (" << ret
				<< ") encountered with rsslEncodeElementEntry. Error Text: " << rsslRetCodeInfo(ret)
				<< std::endl;
		}

		ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE);
		if (ret < RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin Error "
				<< rsslRetCodeToString(ret) << " (" << ret
				<< ") encountered with rsslEncodeElementListComple. Error Text: " << rsslRetCodeInfo(ret)
				<< std::endl;				
		}

		/* complete encode message */
		if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin rsslEncodeMsgComplete() failed with return code: " << ret << std::endl;
			return ret;
		}

		msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);
		
		//std::cout << "ADHSimulator::sendGenericMessageLogin msgBuf->length=" << msgBuf->length << std::endl;

		/* send generic message login */
		if (sendMessage(pReactor, pReactorChannel, msgBuf) != RSSL_RET_SUCCESS)
		{
			std::cout << "ADHSimulator::sendGenericMessageLogin Error submitting generic message on LOGIN domain streamId=" << pRsslMsgEvent->pRsslMsg->msgBase.streamId << std::endl;
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		std::cout << "ADHSimulator::sendGenericMessageLogin rsslReactorGetBuffer(): Failed. " << rsslErrorInfo.rsslError.text << std::endl;
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslMsg* ADHSimulator::popMsg()
{
	RsslMsg* tmp = NULL;
	if (_messageQueue.size() > 0)
	{
		tmp = _messageQueue[0];
		_messageQueue.removePosition(0);
	}

	return tmp;
}