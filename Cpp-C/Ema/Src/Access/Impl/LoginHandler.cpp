/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "LoginHandler.h"
#include "OmmServerBaseImpl.h"
#include "OmmProviderClient.h"
#include "ServerChannelHandler.h"
#include "DictionaryHandler.h"
#include "DirectoryServiceStore.h"
#include "StaticDecoder.h"
#include "ClientSession.h"
#include "EmaRdm.h"

#include "OmmIProviderImpl.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

const EmaString LoginHandler::_clientName("LoginHandler");

static struct
{
	RsslInt32 rdmMsgType;
	const char* strMsgType;
}  ConverterRdmLoginMsgTypeToStr[] =
{
	{ RDM_LG_MT_UNKNOWN, "Unknown" },
	{ RDM_LG_MT_REQUEST, "Request" },
	{ RDM_LG_MT_CLOSE, "Close" },
	{ RDM_LG_MT_CONSUMER_CONNECTION_STATUS, "Consumer Connection Status" },
	{ RDM_LG_MT_REFRESH, "Refresh" },
	{ RDM_LG_MT_STATUS, "Status" },
	{ RDM_LG_MT_POST, "Post" },
	{ RDM_LG_MT_ACK, "Ack" }
};

LoginHandler::LoginHandler(OmmServerBaseImpl* ommServerBaseImpl) :
	_pOmmServerBaseImpl(ommServerBaseImpl)
{
}

LoginHandler::~LoginHandler()
{
	if (_rsslMsgBuffer.data)
	{
		free(_rsslMsgBuffer.data);
		_rsslMsgBuffer.data = 0;
	}
}

void LoginHandler::initialize()
{
	_rsslMsgBuffer.length = 1024;
	_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

	if (!_rsslMsgBuffer.data)
	{
		_pOmmServerBaseImpl->handleMee("Failed to allocate memory in LoginHandler::initialize()");
		return;
	}
}

const EmaVector< ItemInfo* >&	LoginHandler::getLoginItemList()
{
	return _itemInfoList;
}

void LoginHandler::addItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();

	_itemInfoList.push_back(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

void LoginHandler::removeItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();
	
	_itemInfoList.removeValue(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

RsslReactorCallbackRet LoginHandler::loginCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent* pRDMLoginMsgEvent)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)pReactor->userSpecPtr;
	ommServerBaseImpl->eventReceived();

	ClientSession* clientSession = (ClientSession*)pReactorChannel->userSpecPtr;
	RsslRDMLoginMsg *pLoginMsg = pRDMLoginMsgEvent->pRDMLoginMsg;

	if (!pLoginMsg)
	{
		if (pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC)
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received Generic message on login stream.");
				temp.append(CR).append("Stream Id ").append(pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId);

			if (itemInfo)
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_genericMsg, pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					0);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onGenericMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
			}

			return RSSL_RC_CRET_SUCCESS;
		}

		EmaString temp("Login message rejected - invalid login domain message.");

		ommServerBaseImpl->getLoginHandler().sendLoginReject(pReactorChannel, pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId, RSSL_SC_USAGE_ERROR, temp);

		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			temp.append(CR).append("Stream Id ").append(pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch (pLoginMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_LG_MT_REQUEST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received login request message.");
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg,
				pReactorChannel->majorVersion,
				pReactorChannel->minorVersion,
				0);

			ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
			ommServerBaseImpl->ommProviderEvent._channel = pReactorChannel;
			ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
			ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			if (!itemInfo )
			{
				ItemInfo* itemInfo = ItemInfo::create(*ommServerBaseImpl);

				if (!itemInfo || !itemInfo->setRsslRequestMsg(pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
					return RSSL_RC_CRET_SUCCESS;

				itemInfo->setClientSession(clientSession);
				clientSession->setLoginHandle((UInt64)itemInfo);

				ommServerBaseImpl->getLoginHandler().addItemInfo(itemInfo);
				ommServerBaseImpl->addItemInfo(itemInfo);

				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onReqMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
			}
			else
			{
				if ( !itemInfo->setRsslRequestMsg(pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
					return RSSL_RC_CRET_SUCCESS;

				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onReissue(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_LG_MT_CONSUMER_CONNECTION_STATUS:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received Consumer Connection Status message.");
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_genericMsg, pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					0);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onGenericMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_LG_MT_POST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received post message on login domain.");
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				RsslMsg *pRsslMsg = pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg;
				const RsslDataDictionary* rsslDataDictionary = 0;

				if (pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(pRsslMsg->msgBase.msgKey.serviceId);

					Dictionary* dictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(pRsslMsg->msgBase.msgKey.serviceId);

					if (dictionary)
					{
						rsslDataDictionary = dictionary->getRsslDictionary();
					}
					else
					{
						if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						{
							EmaString temp("Data dictionary information is not available for service Id = ");
							temp.append(pRsslMsg->msgBase.msgKey.serviceId)
								.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
								.append(CR).append("Client handle ").append(clientSession->getClientHandle())
								.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

							ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
						}
					}

					if (serviceNamePtr)
					{
						ommServerBaseImpl->_postMsg.serviceName(**serviceNamePtr);
					}
					else
					{
						if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						{
							EmaString temp("Post message has an invalid service Id = ");
							temp.append(pRsslMsg->msgBase.msgKey.serviceId)
								.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
								.append(CR).append("Client handle ").append(clientSession->getClientHandle())
								.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

							ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
						}
					}
				}

				StaticDecoder::setRsslData(&ommServerBaseImpl->_postMsg, pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					rsslDataDictionary);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				if (static_cast<OmmIProviderActiveConfig&>(ommServerBaseImpl->getActiveConfig()).getEnforceAckIDValidation())
				{
					if (itemInfo && ommServerBaseImpl->_postMsg.hasPostId())
					{
						itemInfo->addPostId(ommServerBaseImpl->_postMsg.getPostId());
					}
				}

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_postMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onPostMsg(ommServerBaseImpl->_postMsg, ommServerBaseImpl->ommProviderEvent);
			}

			break;
		}
		case RDM_LG_MT_CLOSE:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received login close message.");
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			ommServerBaseImpl->_reqMsg.clear();
			ommServerBaseImpl->_reqMsg.initialImage(false);
			ommServerBaseImpl->_reqMsg.interestAfterRefresh(false);
			ommServerBaseImpl->_reqMsg.streamId(pLoginMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				if (itemInfo->hasNameType())
				{
					ommServerBaseImpl->_reqMsg.nameType(itemInfo->getNameType());
				}

				if (itemInfo->hasName())
				{
					ommServerBaseImpl->_reqMsg.name(itemInfo->getName());
				}

				ommServerBaseImpl->_reqMsg.name(itemInfo->getName());
				ommServerBaseImpl->_reqMsg.domainType(itemInfo->getDomainType());

				StaticDecoder::setData(&ommServerBaseImpl->_reqMsg, 0);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onClose(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				clientSession->setLoginHandle(0); /* Unset login handle when EMA receives login close message. */

				ommServerBaseImpl->getServerChannelHandler().closeChannel(pReactorChannel);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_LG_MT_RTT:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received RTT message on login domain.");
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				RsslMsg *pRsslMsg = pRDMLoginMsgEvent->baseMsgEvent.pRsslMsg;
				const RsslDataDictionary* rsslDataDictionary = 0;

				if (itemInfo->hasServiceId())
				{
					Dictionary* dictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(itemInfo->getServiceId());

					if (dictionary)
					{
						rsslDataDictionary = dictionary->getRsslDictionary();
					}
				}

				StaticDecoder::setRsslData(&ommServerBaseImpl->_genericMsg, pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					rsslDataDictionary);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onGenericMsg(ommServerBaseImpl->_genericMsg, ommServerBaseImpl->ommProviderEvent);
			}

			break;
		}
		default:
		{
			EmaString temp("Rejected unhandled login message type ");
			temp.append(ConverterRdmLoginMsgTypeToStr[pLoginMsg->rdmMsgBase.rdmMsgType].strMsgType);

			ItemInfo* itemInfo = clientSession->getItemInfo(pLoginMsg->rdmMsgBase.streamId);

			if (!itemInfo)
			{
				ommServerBaseImpl->getLoginHandler().sendLoginReject(pReactorChannel, pLoginMsg->rdmMsgBase.streamId, RSSL_SC_USAGE_ERROR, temp);
			}

			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				temp.append(CR).append("Stream Id ").append(pLoginMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void LoginHandler::sendLoginReject(RsslReactorChannel* reactorChannel, RsslInt32 streamId, RsslStateCodes stateCode, EmaString& stateText)
{
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	rsslClearRDMLoginStatus(&_rsslRdmLoginMsg.status);
	_rsslRdmLoginMsg.status.flags |= RDM_LG_STF_HAS_STATE;
	_rsslRdmLoginMsg.status.rdmMsgBase.streamId = streamId;
	_rsslRdmLoginMsg.status.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	_rsslRdmLoginMsg.status.state.dataState = RSSL_DATA_SUSPECT;
	_rsslRdmLoginMsg.status.state.code = stateCode;
	_rsslRdmLoginMsg.status.state.text.data = (char*)stateText.c_str();
	_rsslRdmLoginMsg.status.state.text.length = stateText.length();

	rsslClearEncodeIterator(&_rsslEncodeIter);

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion(&_rsslEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator version in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslBuffer rsslBuffer;
	rsslBuffer.data = _rsslMsgBuffer.data;
	rsslBuffer.length = _rsslMsgBuffer.length;

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer(&_rsslEncodeIter, &rsslBuffer))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator buffer in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslErrorInfo rsslErrorInfo;
	RsslRet retCode;
	retCode = rsslEncodeRDMLoginMsg(&_rsslEncodeIter, (RsslRDMLoginMsg*)&_rsslRdmLoginMsg.status, &rsslBuffer.length, &rsslErrorInfo);

	while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(rsslBuffer.data);

		_rsslMsgBuffer.length += _rsslMsgBuffer.length;
		_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

		if (!_rsslMsgBuffer.data)
		{
			if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to allocate memory in LoginHandler::sendLoginReject()");
				temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

				_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}

			return;
		}

		rsslBuffer.data = _rsslMsgBuffer.data;
		rsslBuffer.length = _rsslMsgBuffer.length;

		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeRDMLoginMsg(&_rsslEncodeIter, (RsslRDMLoginMsg*)&_rsslRdmLoginMsg, &rsslBuffer.length, &rsslErrorInfo);
	}

	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: failed to encode RsslRDMDirectoryMsg in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName()).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	rsslClearDecodeIterator(&_rsslDecodeIter);

	if (rsslSetDecodeIteratorRWFVersion(&_rsslDecodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("IInternal error. Failed to set decode iterator version in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	if (rsslSetDecodeIteratorBuffer(&_rsslDecodeIter, &rsslBuffer) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set decode iterator buffer in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslStatusMsg rsslStatusMsg;
	rsslClearStatusMsg(&rsslStatusMsg);
	if (rsslDecodeMsg(&_rsslDecodeIter, (RsslMsg*)&rsslStatusMsg) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to decode message in LoginHandler::sendLoginReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	rsslStatusMsg.msgBase.streamId = streamId;

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)&rsslStatusMsg;

	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pOmmServerBaseImpl->getRsslReactor(), reactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in LoginHandler::sendLoginReject().");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName()).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}
}

LoginHandler* LoginHandler::create(OmmServerBaseImpl* ommServerBaseImpl)
{
	try
	{
		return new LoginHandler(ommServerBaseImpl);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseImpl->handleMee("Failed to allocate memory in LoginHandler::create()");
	}

	return NULL;
}

void LoginHandler::destroy(LoginHandler*& loginHandler)
{
	if (loginHandler)
	{
		delete loginHandler;
		loginHandler = 0;
	}
}

void LoginHandler::notifyChannelDown(ClientSession* clientSession)
{
	_pOmmServerBaseImpl->_userLock.lock();

	ItemInfo* itemInfo;

	for (UInt32 index = 0; index < _itemInfoList.size(); index++)
	{
		itemInfo = _itemInfoList[index];

		if (clientSession == itemInfo->getClientSession())
		{
			_pOmmServerBaseImpl->_reqMsg.clear();
			_pOmmServerBaseImpl->_reqMsg.initialImage(false);
			_pOmmServerBaseImpl->_reqMsg.interestAfterRefresh(false);
			_pOmmServerBaseImpl->_reqMsg.streamId(itemInfo->getStreamId());

			if (itemInfo->hasNameType())
			{
				_pOmmServerBaseImpl->_reqMsg.nameType(itemInfo->getNameType());
			}

			if (itemInfo->hasName())
			{
				_pOmmServerBaseImpl->_reqMsg.name(itemInfo->getName());
			}

			_pOmmServerBaseImpl->_reqMsg.domainType(MMT_LOGIN);

			StaticDecoder::setData(&_pOmmServerBaseImpl->_reqMsg, 0);

			_pOmmServerBaseImpl->ommProviderEvent._clientHandle = itemInfo->getClientSession()->getClientHandle();
			_pOmmServerBaseImpl->ommProviderEvent._closure = _pOmmServerBaseImpl->_pClosure;
			_pOmmServerBaseImpl->ommProviderEvent._provider = _pOmmServerBaseImpl->getProvider();
			_pOmmServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

			_pOmmServerBaseImpl->_pOmmProviderClient->onAllMsg(_pOmmServerBaseImpl->_reqMsg, _pOmmServerBaseImpl->ommProviderEvent);
			_pOmmServerBaseImpl->_pOmmProviderClient->onClose(_pOmmServerBaseImpl->_reqMsg, _pOmmServerBaseImpl->ommProviderEvent);

			clientSession->setLoginHandle(0); /* Unset login handle when EMA receives login close message. */

			break;
		}
	}

	_pOmmServerBaseImpl->_userLock.unlock();
}
