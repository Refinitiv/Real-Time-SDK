/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "MarketItemHandler.h"
#include "ClientSession.h"
#include "OmmServerBaseImpl.h"
#include "OmmProviderClient.h"
#include "DictionaryHandler.h"
#include "Decoder.h"
#include "DirectoryServiceStore.h"
#include "StaticDecoder.h"

#include "OmmIProviderImpl.h"

#include <new>

using namespace refinitiv::ema::access;

const EmaString MarketItemHandler::_clientName("MarketItemHandler");

static struct
{
	RsslUInt8 msgType;
	const char* strMsgType;
}  ConverterMsgTypeToStr[] =
{
	{ RSSL_MC_REFRESH, "Refresh" },
	{ RSSL_MC_STATUS, "State" },
	{ RSSL_MC_UPDATE, "Update" },
	{ RSSL_MC_ACK, "Ack" },
};

MarketItemHandler::MarketItemHandler(OmmServerBaseImpl* ommServerBaseImpl) :
	_pOmmServerBaseImpl(ommServerBaseImpl)
{
	_isDirectoryApiControl = _pOmmServerBaseImpl->getActiveConfig().getDirectoryAdminControl() == OmmIProviderConfig::ApiControlEnum ? true : false;
}

MarketItemHandler::~MarketItemHandler()
{
	if (_rsslMsgBuffer.data)
	{
		free(_rsslMsgBuffer.data);
		_rsslMsgBuffer.data = 0;
	}	

	if (_rsslQosStringBuffer.data)
	{
		free(_rsslQosStringBuffer.data);
		_rsslQosStringBuffer.data = 0;
	}
}

void MarketItemHandler::initialize()
{
	_rsslMsgBuffer.length = 1024;
	_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

	_rsslQosStringBuffer.length = 128;
	_rsslQosStringBuffer.data = (char*)malloc(sizeof(char) * _rsslQosStringBuffer.length);

	if ( (!_rsslMsgBuffer.data) || (!_rsslQosStringBuffer.data) )
	{
		_pOmmServerBaseImpl->handleMee("Failed to allocate memory in MarketItemHandler::initialize()");
		return;
	}
}

RsslReactorCallbackRet MarketItemHandler::itemCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pMsgEvent)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)pReactor->userSpecPtr;
	ClientSession*	clientSession = (ClientSession*)pReactorChannel->userSpecPtr;
	ommServerBaseImpl->eventReceived();

	RsslMsg *pRsslMsg = pMsgEvent->pRsslMsg;

	if (!pRsslMsg)
	{
		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			RsslErrorInfo *pError = pMsgEvent->pErrorInfo;
			EmaString temp("Received error message.");
			temp.append(CR).append("Error Text ").append(pError->rsslError.text)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());
				
			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if (!ommServerBaseImpl->getActiveConfig().acceptMessageWithoutBeingLogin && !clientSession->isLogin())
	{
		EmaString temp("Message rejected - there is no logged in user for this session.");

		ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

		if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch (pRsslMsg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received request message.");
				temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			const RsslDataDictionary* rsslDataDictionary = 0;

			ItemInfo* itemInfo = clientSession->getItemInfo(pRsslMsg->msgBase.streamId);

			if (pRsslMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
			{
				const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(pRsslMsg->requestMsg.msgBase.msgKey.serviceId);

				if (!serviceNamePtr)
				{
					EmaString temp("Request message rejected - the service Id = ");
					temp.append(pRsslMsg->msgBase.msgKey.serviceId).append(" does not exist in the source directory");

					ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

					if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
							.append(CR).append("Client handle ").append(clientSession->getClientHandle())
							.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

						ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
					}

					if (itemInfo)
					{
						ommServerBaseImpl->getMarketItemHandler().notifyOnClose(pRsslMsg, itemInfo);
					}

					break;
				}

				Dictionary* pDictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(pRsslMsg->requestMsg.msgBase.msgKey.serviceId);

				if (pDictionary)
				{
					rsslDataDictionary = pDictionary->getRsslDictionary();
				}

				StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					rsslDataDictionary);

				ommServerBaseImpl->_reqMsg.getDecoder().setServiceName((*serviceNamePtr)->c_str(), (*serviceNamePtr)->length());

				OmmIProviderDirectoryStore& ommIProviderDirectoryStore = static_cast<OmmIProviderDirectoryStore&>(ommServerBaseImpl->getDirectoryServiceStore());

				if ( !ommServerBaseImpl->getActiveConfig().acceptMessageWithoutAcceptingRequests && ommServerBaseImpl->getMarketItemHandler()._isDirectoryApiControl 
					&& !ommIProviderDirectoryStore.IsAcceptingRequests(pRsslMsg->requestMsg.msgBase.msgKey.serviceId))
				{
					EmaString temp("Request message rejected - the service Id = ");
					temp.append(pRsslMsg->msgBase.msgKey.serviceId).append(" does not accept any requests.");

					ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

					if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
							.append(CR).append("Client handle ").append(clientSession->getClientHandle())
							.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());
							
						ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
					}

					if (itemInfo)
					{
						ommServerBaseImpl->getMarketItemHandler().notifyOnClose(pRsslMsg, itemInfo);
					}

					break;
				}

				if (!ommServerBaseImpl->getActiveConfig().acceptMessageWithoutQosInRange && ommServerBaseImpl->getMarketItemHandler()._isDirectoryApiControl
					&& !ommIProviderDirectoryStore.IsValidQosRange(pRsslMsg->requestMsg.msgBase.msgKey.serviceId, pRsslMsg->requestMsg))
				{
					EmaString temp("Request message rejected - the service Id = ");
					temp.append(pRsslMsg->msgBase.msgKey.serviceId).append(" does not support the specified QoS(");

					RsslBuffer tempBuffer;
					tempBuffer.data = ommServerBaseImpl->getMarketItemHandler()._rsslQosStringBuffer.data;
					tempBuffer.length = ommServerBaseImpl->getMarketItemHandler()._rsslQosStringBuffer.length;

					if (pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_QOS)
					{
						rsslQosToString(&tempBuffer, &pRsslMsg->requestMsg.qos);
						EmaString qosText(tempBuffer.data, tempBuffer.length);
						temp.append(qosText).append(")");
					}

					tempBuffer.data = ommServerBaseImpl->getMarketItemHandler()._rsslQosStringBuffer.data;
					tempBuffer.length = ommServerBaseImpl->getMarketItemHandler()._rsslQosStringBuffer.length;

					if (pRsslMsg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS)
					{
						rsslQosToString(&tempBuffer, &pRsslMsg->requestMsg.worstQos);
						EmaString worstQosText(tempBuffer.data, tempBuffer.length);
						temp.append(" and Worst QoS(").append(worstQosText).append(").");
					}
					else
					{
						temp.append(".");
					}

					ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

					if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
							.append(CR).append("Client handle ").append(clientSession->getClientHandle())
							.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

						ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
					}

					if (itemInfo)
					{
						ommServerBaseImpl->getMarketItemHandler().notifyOnClose(pRsslMsg, itemInfo);
					}

					break;
				}
			}
			else
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					rsslDataDictionary);
			}

			ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
			ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
			ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();

			if (!itemInfo)
			{
				ItemInfo* itemInfo = ItemInfo::create(*ommServerBaseImpl);

				if (!itemInfo || !itemInfo->setRsslRequestMsg(pRsslMsg->requestMsg) )
					return RSSL_RC_CRET_SUCCESS;

				itemInfo->setClientSession(clientSession);

				if (!ommServerBaseImpl->getActiveConfig().acceptMessageSameKeyButDiffStream)
				{
					if (clientSession->checkingExistingReq(itemInfo))
					{
						EmaString temp("Request Message rejected - Item already open with exact same message key on another stream.");

						ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

						if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						{
							temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
								.append(CR).append("Client handle ").append(clientSession->getClientHandle())
								.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

							ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
						}

						ItemInfo::destroy(itemInfo);

						return RSSL_RC_CRET_SUCCESS;
					}
				}

				ommServerBaseImpl->addItemInfo(itemInfo);

				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onReqMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
			}
			else
			{
				bool setMessageKey = false;

				if (pRsslMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					if (itemInfo->hasServiceId())
					{
						if (itemInfo->getServiceId() != pRsslMsg->requestMsg.msgBase.msgKey.serviceId)
						{
							if (!ommServerBaseImpl->getActiveConfig().acceptMessageThatChangesService)
							{
								EmaString temp("Request Message rejected - Attempt to reissue the service Id from ");
								temp.append(itemInfo->getServiceId()).append(" to ").append(pRsslMsg->requestMsg.msgBase.msgKey.serviceId)
									.append(" while this is not supported.");

								ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);

								if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
								{
									temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
										.append(CR).append("Client handle ").append(clientSession->getClientHandle())
										.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

									ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
								}

								if (itemInfo)
								{
									ommServerBaseImpl->getMarketItemHandler().notifyOnClose(pRsslMsg, itemInfo);
								}

								break;
							}
							else
							{
								if (itemInfo->hasItemGroup())
								{
									ommServerBaseImpl->removeItemGroup(itemInfo);

									if (!itemInfo->setRsslRequestMsg(pRsslMsg->requestMsg))
										return RSSL_RC_CRET_SUCCESS;

									setMessageKey = true;

									ommServerBaseImpl->addItemGroup(itemInfo, itemInfo->getItemGroup());
								}
							}
						}
					}
				}

				if (!setMessageKey && !itemInfo->setRsslRequestMsg(pRsslMsg->requestMsg))
					return RSSL_RC_CRET_SUCCESS;

				ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
				ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				ommServerBaseImpl->_pOmmProviderClient->onReissue(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
			}

			break;
		}
		case RSSL_MC_CLOSE:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received close message.");
				temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pRsslMsg->msgBase.streamId);

			if (itemInfo)
			{
				ommServerBaseImpl->getMarketItemHandler().notifyOnClose(pRsslMsg, itemInfo);
			}

			break;
		}
		case RSSL_MC_POST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received post message.");
				temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pRsslMsg->msgBase.streamId);

			if (itemInfo)
			{
				const RsslDataDictionary* rsslDataDictionary = 0;
				Dictionary* dictionary = NULL;

				/* Get the default dictionary */
				if (ommServerBaseImpl->getDictionaryHandler().getDefaultDictionaryUse())
				{
					rsslDataDictionary = ommServerBaseImpl->getDictionaryHandler().getDefaultDictionaryUse()->getRsslDictionary();
				}

				if (pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(pRsslMsg->msgBase.msgKey.serviceId);
					
					dictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(pRsslMsg->msgBase.msgKey.serviceId);

					if (dictionary)
					{
						rsslDataDictionary = dictionary->getRsslDictionary();
					}
					else
					{	/* Gets the dictionary from service ID of the item stream */
						if (itemInfo->hasServiceId())
						{
							dictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(itemInfo->getServiceId());

							if (dictionary)
							{
								rsslDataDictionary = dictionary->getRsslDictionary();
							}
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
		case RSSL_MC_GENERIC:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received generic message.");
				temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pRsslMsg->msgBase.streamId);

			if (itemInfo)
			{
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
			EmaString temp("Rejected unhandled message type ");
			temp.append(ConverterMsgTypeToStr[pRsslMsg->msgBase.msgClass].strMsgType);

			ItemInfo* itemInfo = clientSession->getItemInfo(pRsslMsg->msgBase.streamId);

			if (!itemInfo)
			{
				ommServerBaseImpl->getMarketItemHandler().sendRejectMessage(pReactorChannel, pRsslMsg, RSSL_SC_USAGE_ERROR, temp);
			}

			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				temp.append(CR).append("Stream Id ").append(pRsslMsg->msgBase.streamId)
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append(CR).append("Client handle ").append(clientSession->getClientHandle());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void MarketItemHandler::notifyOnClose(RsslMsg* pRsslMsg, ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_reqMsg.clear();
	_pOmmServerBaseImpl->_reqMsg.initialImage(false);
	_pOmmServerBaseImpl->_reqMsg.interestAfterRefresh(false);
	_pOmmServerBaseImpl->_reqMsg.streamId(pRsslMsg->msgBase.streamId);

	if (itemInfo->hasNameType())
	{
		_pOmmServerBaseImpl->_reqMsg.nameType(itemInfo->getNameType());
	}

	if (itemInfo->hasName())
	{
		_pOmmServerBaseImpl->_reqMsg.name(itemInfo->getName());
	}

	_pOmmServerBaseImpl->_reqMsg.domainType(itemInfo->getDomainType());

	if (itemInfo->hasServiceId())
	{
		const EmaString** serviceNamePtr = _pOmmServerBaseImpl->getDirectoryServiceStore().getServiceNameById(itemInfo->getServiceId());

		_pOmmServerBaseImpl->_reqMsg.serviceId(itemInfo->getServiceId());
		StaticDecoder::setData(&_pOmmServerBaseImpl->_reqMsg, 0);

		if (serviceNamePtr)
		{
			_pOmmServerBaseImpl->_reqMsg.getDecoder().setServiceName((*serviceNamePtr)->c_str(), (*serviceNamePtr)->length());
		}
	}
	else
	{
		StaticDecoder::setData(&_pOmmServerBaseImpl->_reqMsg, 0);
	}

	_pOmmServerBaseImpl->ommProviderEvent._clientHandle = itemInfo->getClientSession()->getClientHandle();
	_pOmmServerBaseImpl->ommProviderEvent._closure = _pOmmServerBaseImpl->_pClosure;
	_pOmmServerBaseImpl->ommProviderEvent._provider = _pOmmServerBaseImpl->getProvider();
	_pOmmServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

	_pOmmServerBaseImpl->_pOmmProviderClient->onAllMsg(_pOmmServerBaseImpl->_reqMsg, _pOmmServerBaseImpl->ommProviderEvent);
	_pOmmServerBaseImpl->_pOmmProviderClient->onClose(_pOmmServerBaseImpl->_reqMsg, _pOmmServerBaseImpl->ommProviderEvent);

	_pOmmServerBaseImpl->removeItemInfo(itemInfo, true);
}

void MarketItemHandler::sendRejectMessage(RsslReactorChannel* reactorChannel, RsslMsg* pRsslMsg, RsslStateCodes stateCode, EmaString& stateText)
{
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	rsslClearStatusMsg(&_rsslStatusMsg);

	_rsslStatusMsg.msgBase.msgClass = RSSL_MC_STATUS;
	_rsslStatusMsg.msgBase.streamId = pRsslMsg->msgBase.streamId;
	_rsslStatusMsg.msgBase.domainType = pRsslMsg->msgBase.domainType;
	_rsslStatusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	_rsslStatusMsg.flags = RSSL_STMF_HAS_STATE;

	if (pRsslMsg->msgBase.msgClass == RSSL_MC_REQUEST)
	{
		if (pRsslMsg->requestMsg.flags & RSSL_RQMF_PRIVATE_STREAM)
		{
			_rsslStatusMsg.flags |= RSSL_STMF_PRIVATE_STREAM;
		}
	}

	_rsslStatusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	_rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
	_rsslStatusMsg.state.code = stateCode;
	_rsslStatusMsg.state.text.data = (char*)stateText.c_str();
	_rsslStatusMsg.state.text.length = stateText.length();

	rsslClearEncodeIterator(&_rsslEncodeIter);

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion(&_rsslEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator version in MarketItemHandler::sendRejectMessage()");
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
			EmaString temp("Internal error. Failed to set encode iterator buffer in MarketItemHandler::sendRejectMessage()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslErrorInfo rsslErrorInfo;
	RsslRet retCode;
	retCode = rsslEncodeMsg(&_rsslEncodeIter, (RsslMsg*)&_rsslStatusMsg);

	while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(rsslBuffer.data);

		_rsslMsgBuffer.length += _rsslMsgBuffer.length;
		_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

		if (!rsslBuffer.data)
		{
			if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to allocate memory in MarketItemHandler::sendRejectMessage()");
				temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

				_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}

			return;
		}

		rsslBuffer.data = _rsslMsgBuffer.data;
		rsslBuffer.length = _rsslMsgBuffer.length;

		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeMsg(&_rsslEncodeIter, (RsslMsg*)&_rsslStatusMsg);
	}

	rsslBuffer.length = rsslGetEncodedBufferLength(&_rsslEncodeIter);

	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: failed to encode rsslEncodeMsg in MarketItemHandler::sendRejectMessage()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle()).append(CR)
				.append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
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
			EmaString temp("IInternal error. Failed to set decode iterator version in MarketItemHandler::sendRejectMessage()");
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
			EmaString temp("Internal error. Failed to set decode iterator buffer in MarketItemHandler::sendRejectMessage()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	if (rsslDecodeMsg(&_rsslDecodeIter, (RsslMsg*)&_rsslStatusMsg) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to decode message in MarketItemHandler::sendRejectMessage()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)&_rsslStatusMsg;

	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pOmmServerBaseImpl->getRsslReactor(), reactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in MarketItemHandler::sendRejectMessage().");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle()).append(CR)
				.append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}
}

MarketItemHandler* MarketItemHandler::create(OmmServerBaseImpl* ommServerBaseImpl)
{
	try
	{
		return new MarketItemHandler(ommServerBaseImpl);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseImpl->handleMee("Failed to allocate memory in MarketItemHandler::create()");
	}

	return NULL;
}

void MarketItemHandler::destroy(MarketItemHandler*& marketItemHandler)
{
	if (marketItemHandler)
	{
		delete marketItemHandler;
		marketItemHandler = 0;
	}
}

