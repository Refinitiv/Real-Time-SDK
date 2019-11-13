/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#include "ServerChannelHandler.h"
#include "OmmServerBaseImpl.h"

#include "LoginHandler.h"
#include "DirectoryHandler.h"
#include "DictionaryHandler.h"
#include "MarketItemHandler.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;

const EmaString ServerChannelHandler::_clientName("ServerChannelHandler");

ServerChannelHandler::ServerChannelHandler(OmmServerBaseImpl* ommServerBaseImpl) :
	_pOmmServerBaseImpl(ommServerBaseImpl)
{
}

ServerChannelHandler::~ServerChannelHandler()
{
}

RsslReactorCallbackRet ServerChannelHandler::channelEventCallback(RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pRsslReactorChannelEvent)
{
	ClientSession* clientSession = (ClientSession*)pRsslReactorChannel->userSpecPtr;
	OmmServerBaseImpl* ommServerBase = (OmmServerBaseImpl*)pRsslReactor->userSpecPtr;
	ommServerBase->eventReceived();

	switch (pRsslReactorChannelEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
		{
			RsslReactorChannelInfo channelInfo;
			RsslErrorInfo rsslErrorInfo;
			clearRsslErrorInfo(&rsslErrorInfo);

			clientSession->setChannel(pRsslReactorChannel);

			RsslRet retChanInfo;
			retChanInfo = rsslReactorGetChannelInfo(pRsslReactorChannel, &channelInfo, &rsslErrorInfo);
			EmaString componentInfo("Connected component version: ");
			for (unsigned int i = 0; i < channelInfo.rsslChannelInfo.componentInfoCount; ++i)
			{
				componentInfo.append(channelInfo.rsslChannelInfo.componentInfo[i]->componentVersion.data);
				if (i < (channelInfo.rsslChannelInfo.componentInfoCount - 1))
					componentInfo.append(", ");
			}

			if ( componentInfo.find("adh") != -1  )
			{
				clientSession->setADHSession(true);
			}

			if (OmmLoggerClient::SuccessEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received ChannelUp event on client handle ");
				temp.append(clientSession->getClientHandle()).append(CR)
					.append("Instance Name ").append(ommServerBase->getInstanceName());
				if (channelInfo.rsslChannelInfo.componentInfoCount > 0)
					temp.append(CR).append(componentInfo);

				ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::SuccessEnum, temp);
			}

			ommServerBase->getServerChannelHandler().addClientSession(clientSession);
			ommServerBase->addSocket(pRsslReactorChannel->socketId);

			if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &ommServerBase->getActiveConfig().pServerConfig->sysSendBufSize , &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Failed to set send buffer size on client handle ");
					temp.append(clientSession->getClientHandle()).append(CR)
						.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
						.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
						.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error text ").append(rsslErrorInfo.rsslError.text);

					ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
				}

				ommServerBase->getServerChannelHandler().closeChannel(pRsslReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &ommServerBase->getActiveConfig().pServerConfig->sysRecvBufSize, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Failed to set receive buffer size on client handle ");
					temp.append(clientSession->getClientHandle()).append(CR)
						.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
						.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
						.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error text ").append(rsslErrorInfo.rsslError.text);

					ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
				}

				ommServerBase->getServerChannelHandler().closeChannel(pRsslReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_COMPRESSION_THRESHOLD, &ommServerBase->getActiveConfig().pServerConfig->compressionThreshold, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Failed to set compression threshold on client handle ");
					temp.append(clientSession->getClientHandle()).append(CR)
						.append("Consumer Name ").append(ommServerBase->getInstanceName()).append(CR)
						.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
						.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error text ").append(rsslErrorInfo.rsslError.text);

					ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
				}

				ommServerBase->getServerChannelHandler().closeChannel(pRsslReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}

			if ( ommServerBase->getActiveConfig().pServerConfig->highWaterMark )
			{
				if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_HIGH_WATER_MARK, &ommServerBase->getActiveConfig().pServerConfig->highWaterMark, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					if (OmmLoggerClient::ErrorEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Failed to set the high water mark on client handle ");
						temp.append(clientSession->getClientHandle()).append(CR)
							.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
							.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
							.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
							.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
							.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
							.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
							.append("Error Text ").append(rsslErrorInfo.rsslError.text);
						ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
					}
				}
				else if (OmmLoggerClient::VerboseEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("high water mark set on client handle ");
					temp.append(clientSession->getClientHandle()).append(CR)
						.append("Instance Name ").append(ommServerBase->getInstanceName());
					ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
				}
			}

			ActiveServerConfig& activeConfig = ommServerBase->getActiveConfig();
			if (activeConfig.xmlTraceToFile || activeConfig.xmlTraceToStdout)
			{
				EmaString fileName(activeConfig.xmlTraceFileName);
				fileName.append("_");

				RsslTraceOptions traceOptions;
				rsslClearTraceOptions(&traceOptions);

				traceOptions.traceMsgFileName = (char*)fileName.c_str();

				if (activeConfig.xmlTraceToFile)
					traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE;

				if (activeConfig.xmlTraceToStdout)
					traceOptions.traceFlags |= RSSL_TRACE_TO_STDOUT;

				if (activeConfig.xmlTraceToMultipleFiles)
					traceOptions.traceFlags |= RSSL_TRACE_TO_MULTIPLE_FILES;

				if (activeConfig.xmlTraceWrite)
					traceOptions.traceFlags |= RSSL_TRACE_WRITE;

				if (activeConfig.xmlTraceRead)
					traceOptions.traceFlags |= RSSL_TRACE_READ;

				if (activeConfig.xmlTracePing)
					traceOptions.traceFlags |= RSSL_TRACE_PING;

				if (activeConfig.xmlTraceHex)
					traceOptions.traceFlags |= RSSL_TRACE_HEX;

				traceOptions.traceMsgMaxFileSize = activeConfig.xmlTraceMaxFileSize;

				if (RSSL_RET_SUCCESS != rsslReactorChannelIoctl(pRsslReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void*)&traceOptions, &rsslErrorInfo))
				{
					if (OmmLoggerClient::ErrorEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Failed to enable Xml Tracing on client handle ");
						temp.append(clientSession->getClientHandle()).append(CR)
							.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
							.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
							.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
							.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
							.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
							.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
							.append("Error Text ").append(rsslErrorInfo.rsslError.text);
						ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
					}
				}
				else if (OmmLoggerClient::VerboseEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Xml Tracing enabled on client handle ");
					temp.append(clientSession->getClientHandle()).append(CR)
						.append("Instance Name ").append(ommServerBase->getInstanceName());
					ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
				}
			}
			ommServerBase->addConnectedChannel(pRsslReactorChannel);
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received ChannelReady event on client handle ");
				temp.append(clientSession->getClientHandle()).append(CR)
					.append("Instance Name ").append(ommServerBase->getInstanceName());
				ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received FD Change event on client handle ");
				temp.append(clientSession->getClientHandle()).append(CR)
					.append("Instance Name ").append(ommServerBase->getInstanceName());
				ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ommServerBase->removeSocket(pRsslReactorChannel->oldSocketId);
			ommServerBase->addSocket(pRsslReactorChannel->socketId);

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			if (OmmLoggerClient::WarningEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received Channel warning event on client handle ");
				temp.append(clientSession->getClientHandle()).append(CR)
					.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
					.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(pRsslReactorChannelEvent->pError->rsslError.channel)).append(CR)
					.append("Error Id ").append(pRsslReactorChannelEvent->pError->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(pRsslReactorChannelEvent->pError->rsslError.sysError).append(CR)
					.append("Error Location ").append(pRsslReactorChannelEvent->pError->errorLocation).append(CR)
					.append("Error Text ").append(pRsslReactorChannelEvent->pError->rsslError.rsslErrorId ? pRsslReactorChannelEvent->pError->rsslError.text : "");

				ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace());
			}
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			if (OmmLoggerClient::WarningEnum >= ommServerBase->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received ChannelDown event on client handle ");
				temp.append(clientSession->getClientHandle()).append(CR)
					.append("Instance Name ").append(ommServerBase->getInstanceName()).append(CR)
					.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(pRsslReactorChannelEvent->pError->rsslError.channel)).append(CR)
					.append("Error Id ").append(pRsslReactorChannelEvent->pError->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(pRsslReactorChannelEvent->pError->rsslError.sysError).append(CR)
					.append("Error Location ").append(pRsslReactorChannelEvent->pError->errorLocation).append(CR)
					.append("Error Text ").append(pRsslReactorChannelEvent->pError->rsslError.rsslErrorId ? pRsslReactorChannelEvent->pError->rsslError.text : "");

				ommServerBase->getOmmLoggerClient().log(_clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace());
			}

			if (ommServerBase->getState() != OmmServerBaseImpl::UnInitializingEnum)
			{
				ommServerBase->getLoginHandler().notifyChannelDown(clientSession);
				ommServerBase->processChannelEvent(pRsslReactorChannelEvent);
			}

			ommServerBase->getServerChannelHandler().closeChannel(pRsslReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

ServerChannelHandler* ServerChannelHandler::create(OmmServerBaseImpl* ommServerBaseImpl)
{
	ServerChannelHandler* serverChannelHandler = 0;

	try
	{
		serverChannelHandler = new ServerChannelHandler(ommServerBaseImpl);
	}
	catch (std::bad_alloc) {}

	if (!serverChannelHandler)
		ommServerBaseImpl->handleMee("Failed to create ServerChannelHandler");

	return serverChannelHandler;
}

void ServerChannelHandler::destroy(ServerChannelHandler*& serverChannelHandler)
{
	if (serverChannelHandler)
	{
		delete serverChannelHandler;
		serverChannelHandler = 0;
	}
}

void ServerChannelHandler::initialize()
{

}

void ServerChannelHandler::closeActiveSessions()
{
	_pOmmServerBaseImpl->getUserMutex().lock();

	if (!_clientSessionList.empty())
	{
		ClientSession* clientSession = _clientSessionList.pop_front();

		while (clientSession)
		{
			_clientSessionHash.erase(clientSession->getClientHandle());
			clientSession->closeAllItemInfo();
			ClientSession::destroy(clientSession);

			clientSession = _clientSessionList.pop_front();
		}
	}

	_pOmmServerBaseImpl->getUserMutex().unlock();
}

ClientSessionPtr ServerChannelHandler::getClientSession(UInt64 clientHandle) const
{
	_pOmmServerBaseImpl->getUserMutex().lock();

	ClientSessionPtr* clientSessionPtr = _clientSessionHash.find(clientHandle);

	_pOmmServerBaseImpl->getUserMutex().unlock();

	return clientSessionPtr ? *clientSessionPtr : 0;
}

void ServerChannelHandler::addClientSession(ClientSession* clientSession)
{
	_pOmmServerBaseImpl->getUserMutex().lock();
	if (!_clientSessionHash.find(clientSession->getClientHandle()))
	{
		_clientSessionHash.insert(clientSession->getClientHandle(), clientSession);
		_clientSessionList.push_back(clientSession);
	}

	_pOmmServerBaseImpl->getUserMutex().unlock();
}

void ServerChannelHandler::removeClientSession(ClientSession* clientSession)
{
	_pOmmServerBaseImpl->getUserMutex().lock();

	if (!_clientSessionList.empty())
	{
		_clientSessionHash.erase(clientSession->getClientHandle());
		_clientSessionList.remove(clientSession);
		clientSession->closeAllItemInfo();

		ClientSession::destroy(clientSession);
	}

	_pOmmServerBaseImpl->getUserMutex().unlock();
}

void ServerChannelHandler::closeChannel(RsslReactorChannel* pRsslReactorChannel)
{
	if (!pRsslReactorChannel) return;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);

	removeChannel(pRsslReactorChannel);

	if (pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET)
		_pOmmServerBaseImpl->removeSocket(pRsslReactorChannel->socketId);

	if (rsslReactorCloseChannel(_pOmmServerBaseImpl->getRsslReactor(), pRsslReactorChannel, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Failed to close reactor channel (rsslReactorCloseChannel).");
			temp.append("' RsslChannel='").append((UInt64)rsslErrorInfo.rsslError.channel)
				.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
				.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
				.append("' Error Location='").append(rsslErrorInfo.errorLocation)
				.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");

			_pOmmServerBaseImpl->getUserMutex().lock();

			if (&_pOmmServerBaseImpl->getOmmLoggerClient()) 
				_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			_pOmmServerBaseImpl->getUserMutex().unlock();
		}
	}
}

void ServerChannelHandler::removeChannel(RsslReactorChannel* pRsslReactorChannel)
{
	ClientSession* clientSession = (ClientSession*)pRsslReactorChannel->userSpecPtr;

	_pOmmServerBaseImpl->getUserMutex().lock();

	removeClientSession( clientSession );

	_pOmmServerBaseImpl->getUserMutex().unlock();

	_pOmmServerBaseImpl->removeConnectedChannel(pRsslReactorChannel);
}

const EmaList<ClientSession*>& ServerChannelHandler::getClientSessionList()
{
	return _clientSessionList;
}

size_t ServerChannelHandler::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool ServerChannelHandler::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y ? true : false;
}

ClientSessionPtr ServerChannelHandler::getClientSessionForDictReq() const
{
	ClientSessionPtr clientSession = 0;

	if (!_clientSessionList.empty())
	{
		ClientSessionPtr clientSessionTemp = _clientSessionList.front();

		while (clientSessionTemp)
		{
			if ( clientSessionTemp->isADHSession() )
			{
				clientSession = clientSessionTemp;
				break;
			}

			clientSessionTemp = clientSessionTemp->next();
		}

		if ( !clientSession )
		{
			clientSession = _clientSessionList.front();
		}
	}

	return clientSession;
}
