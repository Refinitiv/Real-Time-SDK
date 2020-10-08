/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "DictionaryHandler.h"
#include "OmmServerBaseImpl.h"
#include "ItemInfo.h"
#include "ClientSession.h"
#include "StaticDecoder.h"
#include "OmmProviderClient.h"
#include "OmmIProviderActiveConfig.h"
#include "DirectoryServiceStore.h"
#include "Decoder.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

#include <new>

using namespace refinitiv::ema::access;

const int DictionaryHandler::INITIAL_DICTIONARY_STATUS_MSG_SIZE(256);

const EmaString DictionaryHandler::_clientName("DictionaryHandler");

static struct
{
	RsslInt32 rdmMsgType;
	const char* strMsgType;
}  ConverterRdmDictMsgTypeToStr[] =
{
	{ RDM_DC_MT_UNKNOWN, "Unknown" },
	{ RDM_DC_MT_REQUEST, "Request" },
	{ RDM_DC_MT_CLOSE, "Close" },
	{ RDM_DC_MT_REFRESH, "Refresh" },
	{ RDM_DC_MT_STATUS, "Status" }
};

DictionaryPayload::DictionaryPayload(Dictionary* dictionary, DictionaryType dictionaryType, bool ownDictionary)
{
	_pDictionary = dictionary;
	_dictionaryType = dictionaryType;
	_ownDictionary = ownDictionary;
}

DictionaryPayload::~DictionaryPayload()
{
	if ( _ownDictionary && _pDictionary )
	{
		if ( _pDictionary->getType() == Dictionary::FileDictionaryEnum)
		{
			LocalDictionary* localDictionary = (LocalDictionary*)_pDictionary;
			LocalDictionary::destroy(localDictionary);
		}
	}

	_pDictionary = 0;
}

DictionaryPayload::DictionaryType DictionaryPayload::getDictionaryType() const
{
	return _dictionaryType;
}

Dictionary* DictionaryPayload::getDictionary() const
{
	return _pDictionary;
}

DictionaryHandler::DictionaryHandler(OmmServerBaseImpl* ommServerBaseImpl) :
	_pOmmServerBaseImpl(ommServerBaseImpl)
{
	_apiAdminControl = _pOmmServerBaseImpl->getActiveConfig().getDictionaryAdminControl() == OmmIProviderConfig::ApiControlEnum ? true : false;

	_pDefaultLocalDictionary = LocalDictionary::create(*_pOmmServerBaseImpl, _pOmmServerBaseImpl->getActiveConfig());  // Creates the default local dictionary
}

DictionaryHandler::~DictionaryHandler()
{
	DictionaryPayload* dictionaryPayload = _dictionaryInfoList.pop_back();

	while (dictionaryPayload)
	{
		delete dictionaryPayload;

		dictionaryPayload = _dictionaryInfoList.pop_back();
	}

	// Frees the default LocalDictionary if it isn't used for the first local dictionary in the list.
	if (_pDefaultLocalDictionary)
	{
		LocalDictionary::destroy(_pDefaultLocalDictionary);
	}
}

Dictionary* DictionaryHandler::getDefaultDictionary()
{
	return _pDefaultLocalDictionary;
}

const EmaVector< ItemInfo* >&	DictionaryHandler::getDictionaryItemList()
{
	return _itemInfoList;
}

void DictionaryHandler::addItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();

	_itemInfoList.push_back(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

void DictionaryHandler::removeItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();

	_itemInfoList.removeValue(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

RsslReactorCallbackRet DictionaryHandler::dictionaryCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pRDMDictionaryMsgEvent)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)pReactor->userSpecPtr;
	ClientSession* clientSession = (ClientSession*)pReactorChannel->userSpecPtr;
	ommServerBaseImpl->eventReceived();

	RsslRDMDictionaryMsg *pDictionaryMsg = pRDMDictionaryMsgEvent->pRDMDictionaryMsg;
	RsslErrorInfo* errorInfo = &ommServerBaseImpl->getDictionaryHandler()._errorInfo;
	clearRsslErrorInfo(errorInfo);

	if (!pDictionaryMsg)
	{
		sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, DICTIONARY_INVALID_MESSAGE, errorInfo, false);

		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Dictionary message rejected - invalid dictionary domain message.");
			temp.append(CR).append("Stream Id ").append(pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if (!ommServerBaseImpl->getActiveConfig().acceptMessageWithoutBeingLogin && !clientSession->isLogin())
	{
		sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, USER_IS_NOT_LOGGED_IN, errorInfo);

		return RSSL_RC_CRET_SUCCESS;
	}

	switch (pDictionaryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DC_MT_REQUEST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString name(pRDMDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.data, pRDMDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.length);
				EmaString temp("Received dictionary request message.");
				temp.append(CR).append("Dictionary name ").append(name)
					.append(CR).append("Stream Id ").append(pRDMDictionaryMsgEvent->pRDMDictionaryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			if (ommServerBaseImpl->getDictionaryHandler()._apiAdminControl == false)
			{
				RsslMsg* pRsslMsg = pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg;

				const RsslDataDictionary* rsslDataDictonary = 0;

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();

				if (pRsslMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(pRsslMsg->requestMsg.msgBase.msgKey.serviceId);

					if (!serviceNamePtr)
					{
						sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, SERVICE_ID_NOT_FOUND, errorInfo);
						return RSSL_RC_CRET_SUCCESS;
					}

					Dictionary* pDictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId(pRsslMsg->requestMsg.msgBase.msgKey.serviceId);

					if (pDictionary)
					{
						rsslDataDictonary = pDictionary->getRsslDictionary();
					}

					StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRsslMsg,
						pReactorChannel->majorVersion,
						pReactorChannel->minorVersion,
						rsslDataDictonary);

					ommServerBaseImpl->_reqMsg.getDecoder().setServiceName((*serviceNamePtr)->c_str(), (*serviceNamePtr)->length());
				}
				else
				{
					StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRsslMsg,
						pReactorChannel->majorVersion,
						pReactorChannel->minorVersion,
						rsslDataDictonary);
				}
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pDictionaryMsg->rdmMsgBase.streamId);

			if (!itemInfo)
			{
				ItemInfo* itemInfo = ItemInfo::create(*ommServerBaseImpl);

				if (!itemInfo || !itemInfo->setRsslRequestMsg(pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
					return RSSL_RC_CRET_SUCCESS;

				itemInfo->setClientSession(clientSession);

				ommServerBaseImpl->getDictionaryHandler().addItemInfo(itemInfo);
				ommServerBaseImpl->addItemInfo(itemInfo);

				if (ommServerBaseImpl->getDictionaryHandler()._apiAdminControl == false)
				{
					ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
					ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
					ommServerBaseImpl->_pOmmProviderClient->onReqMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				}
				else
				{
					if (sendDictionaryResponse(pReactor, pReactorChannel, pRDMDictionaryMsgEvent) == false)
					{
						ommServerBaseImpl->getDictionaryHandler().removeItemInfo(itemInfo);
						ommServerBaseImpl->removeItemInfo(itemInfo, false);
					}
				}
			}
			else
			{
				if ( !itemInfo->setRsslRequestMsg(pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
				{
					return RSSL_RC_CRET_SUCCESS;
				}

				if (ommServerBaseImpl->getDictionaryHandler()._apiAdminControl == false)
				{
					ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
					ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
					ommServerBaseImpl->_pOmmProviderClient->onReissue(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				}
				else
				{
					if (sendDictionaryResponse(pReactor, pReactorChannel, pRDMDictionaryMsgEvent) == false)
					{
						ommServerBaseImpl->getDictionaryHandler().removeItemInfo(itemInfo);
						ommServerBaseImpl->removeItemInfo(itemInfo, false);
					}
				}
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_DC_MT_CLOSE:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received dictionary close message.");
				temp.append(CR).append("Stream Id ").append(pDictionaryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

					ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pDictionaryMsg->rdmMsgBase.streamId);

			ommServerBaseImpl->_reqMsg.clear();
			ommServerBaseImpl->_reqMsg.initialImage(false);
			ommServerBaseImpl->_reqMsg.interestAfterRefresh(false);
			ommServerBaseImpl->_reqMsg.streamId(pDictionaryMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				if (ommServerBaseImpl->getDictionaryHandler()._apiAdminControl == false)
				{
					if (itemInfo->hasNameType())
					{
						ommServerBaseImpl->_reqMsg.nameType(itemInfo->getNameType());
					}

					if (itemInfo->hasName())
					{
						ommServerBaseImpl->_reqMsg.name(itemInfo->getName());
					}

					ommServerBaseImpl->_reqMsg.domainType(itemInfo->getDomainType());

					if (itemInfo->hasServiceId())
					{
						const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(itemInfo->getServiceId());

						ommServerBaseImpl->_reqMsg.serviceId(itemInfo->getServiceId());
						StaticDecoder::setData(&ommServerBaseImpl->_reqMsg, 0);

						if (serviceNamePtr)
						{
							ommServerBaseImpl->_reqMsg.getDecoder().setServiceName((*serviceNamePtr)->c_str(), (*serviceNamePtr)->length());
						}
					}
					else
					{
						StaticDecoder::setData(&ommServerBaseImpl->_reqMsg, 0);
					}

					ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
					ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
					ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();
					ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;

					ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
					ommServerBaseImpl->_pOmmProviderClient->onClose(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
				}

				ommServerBaseImpl->getDictionaryHandler().removeItemInfo(itemInfo);
				ommServerBaseImpl->removeItemInfo(itemInfo, false);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_DC_MT_REFRESH:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received dictionary refresh message.");
				temp.append(CR).append("Stream Id ").append(pDictionaryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			const RsslDataDictionary* rsslDataDictionary = 0;

			RsslRefreshMsg& refreshMsg = pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg->refreshMsg;

			if ( ( refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY) && ( refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
			{
				Dictionary* pDictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId( refreshMsg.msgBase.msgKey.serviceId );

				if (pDictionary)
				{
					rsslDataDictionary = pDictionary->getRsslDictionary();
				}
			}

			ommServerBaseImpl->getItemCallbackClient().processIProviderMsgCallback( pReactor, pReactorChannel, &pRDMDictionaryMsgEvent->baseMsgEvent, 
				rsslDataDictionary );

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_DC_MT_STATUS:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received dictionary status message.");
				temp.append(CR).append("Stream Id ").append(pDictionaryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			const RsslDataDictionary* rsslDataDictionary = 0;

			RsslStatusMsg& statusMsg = pRDMDictionaryMsgEvent->baseMsgEvent.pRsslMsg->statusMsg;

			if ( ( statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) && ( statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
			{
				Dictionary* pDictionary = ommServerBaseImpl->getDictionaryHandler().getDictionaryByServiceId( statusMsg.msgBase.msgKey.serviceId );

				if (pDictionary)
				{
					rsslDataDictionary = pDictionary->getRsslDictionary();
				}
			}

			ommServerBaseImpl->getItemCallbackClient().processIProviderMsgCallback( pReactor, pReactorChannel, &pRDMDictionaryMsgEvent->baseMsgEvent,
				rsslDataDictionary );

			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			EmaString temp("Rejected unhandled dictionary message type: ");
			temp.append(ConverterRdmDictMsgTypeToStr[pDictionaryMsg->rdmMsgBase.rdmMsgType].strMsgType);

			ItemInfo* itemInfo = clientSession->getItemInfo(pDictionaryMsg->rdmMsgBase.streamId);

			if (!itemInfo)
			{
				sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, DICTIONARY_INVALID_MESSAGE, errorInfo, false);
			}

			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				temp.append(CR).append("Stream Id ").append(pDictionaryMsg->rdmMsgBase.streamId);
				temp.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append(CR).append("Client handle ").append(clientSession->getClientHandle());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

bool DictionaryHandler::sendDictionaryResponse(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pRDMDictionaryMsgEvent)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)pReactor->userSpecPtr;
	RsslErrorInfo* errorInfo = &ommServerBaseImpl->getDictionaryHandler()._errorInfo;

	EmaString dictNameAndServiceId;

	dictNameAndServiceId.set(pRDMDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.data, pRDMDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.length);
	dictNameAndServiceId.append(pRDMDictionaryMsgEvent->pRDMDictionaryMsg->request.serviceId);

	DictionaryPayload* dictionaryPayload = ommServerBaseImpl->getDictionaryHandler().getDictionaryPayload(dictNameAndServiceId);

	if (!dictionaryPayload)
	{
		sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, DICTIONARY_NAME_NOT_FOUND, errorInfo);

		return false;
	}
	else
	{
		if (dictionaryPayload->getDictionaryType() == DictionaryPayload::RDM_FIELD_DICTIONARY)
		{
			if (sendFieldDictionaryResponse(pReactor, pReactorChannel, pRDMDictionaryMsgEvent->pRDMDictionaryMsg, dictionaryPayload->getDictionary(), errorInfo) != RSSL_RET_SUCCESS)
			{
				sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, DICTIONARY_ENCODING_FAILED, errorInfo);
				return false;
			}
		}
		else if (dictionaryPayload->getDictionaryType() == DictionaryPayload::ENUM_TYPE)
		{
			if (sendEnumTypeDictionaryResponse(pReactor, pReactorChannel, pRDMDictionaryMsgEvent->pRDMDictionaryMsg, dictionaryPayload->getDictionary(), errorInfo) != RSSL_RET_SUCCESS)
			{
				sendRequestReject(pReactor, pReactorChannel, pRDMDictionaryMsgEvent, DICTIONARY_ENCODING_FAILED, errorInfo);
				return false;
			}
		}
	}

	return true;
}

void DictionaryHandler::loadDictionaryFromFile()
{
	const EmaList<ServiceDictionaryConfig*>& serviceDictionaryConfigList = _pOmmServerBaseImpl->getActiveConfig().getServiceDictionaryConfigList();
	LocalDictionary* plocalDictionary = 0;
	EmaString fieldNameAndServiceId;
	EmaString enumTypeAndServiceId;
	bool existingFieldName;
	bool existingEnumName;
	DictionaryPayload* dictionaryPayload;

	ServiceDictionaryConfig* serviceDictionaryConfig = serviceDictionaryConfigList.front();

	while (serviceDictionaryConfig)
	{
		UInt64 serviceId = serviceDictionaryConfig->serviceId;

		const EmaList<DictionaryConfig*> dictionaryConfigList = serviceDictionaryConfig->getDictionaryProvidedList();

		DictionaryConfig* dictionaryConfig = dictionaryConfigList.back();

		while (dictionaryConfig)
		{
			fieldNameAndServiceId.set(dictionaryConfig->rdmFieldDictionaryItemName.c_str());
			fieldNameAndServiceId.append(serviceId);

			enumTypeAndServiceId.set(dictionaryConfig->enumTypeDefItemName.c_str());
			enumTypeAndServiceId.append(serviceId);

			existingFieldName = getDictionaryPayload(fieldNameAndServiceId) != 0 ? true : false;
			existingEnumName = getDictionaryPayload(enumTypeAndServiceId) != 0 ? true: false;

			if ( existingFieldName && existingEnumName)
			{
				dictionaryConfig = dictionaryConfig->previous();
				continue;
			}

			if (_pDefaultLocalDictionary)
			{
				plocalDictionary = _pDefaultLocalDictionary;
				_pDefaultLocalDictionary = NULL;
			}
			else
			{
				plocalDictionary = LocalDictionary::create(*_pOmmServerBaseImpl, _pOmmServerBaseImpl->getActiveConfig());
			}

			RsslRet retCode;
			if (plocalDictionary->load(dictionaryConfig->rdmfieldDictionaryFileName, dictionaryConfig->enumtypeDefFileName, retCode) == false)
			{
				EmaString temp("DictionaryHandler::loadDictionaryFromFile() failed while initializing DictionaryHandler.");
					temp.append(CR).append("Unable to load RDMFieldDictionary from file named ").append(dictionaryConfig->rdmfieldDictionaryFileName)
						.append(CR).append("or load enumtype.def from file named ").append(dictionaryConfig->enumtypeDefFileName);

				if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
				throwIueException(temp, retCode);
				return;
			}

			if ( !existingFieldName )
			{
				dictionaryPayload = new DictionaryPayload(plocalDictionary, DictionaryPayload::RDM_FIELD_DICTIONARY, !existingFieldName);

				addDictionary(fieldNameAndServiceId, dictionaryPayload);
			}

			if ( !existingEnumName )
			{
				dictionaryPayload = new DictionaryPayload(plocalDictionary, DictionaryPayload::ENUM_TYPE, existingFieldName );

				addDictionary(enumTypeAndServiceId, dictionaryPayload);
			}

			if (_serviceDictionaryByIdHash.find(serviceId) == 0)
			{
				_serviceDictionaryByIdHash.insert(serviceId, plocalDictionary);
			}
		
			dictionaryConfig = dictionaryConfig->previous();
		}

		serviceDictionaryConfig = serviceDictionaryConfig->next();
	}
}

Dictionary* DictionaryHandler::getDictionaryByServiceId(UInt64 serviceId)
{
	Dictionary** pDictionaryPtr = _serviceDictionaryByIdHash.find(serviceId);

	if (pDictionaryPtr)
	{
		return *pDictionaryPtr;
	}
	else
	{
		return 0;
	}
}

bool DictionaryHandler::addDictionary(const EmaString& dictNameAndServiceId, DictionaryPayload* dictionaryPayload)
{
	bool result = _dictionaryInfoHash.insert(dictNameAndServiceId, dictionaryPayload);

	if (result)
	{
		_dictionaryInfoList.push_back(dictionaryPayload);
	}

	return result;
}

void DictionaryHandler::removeDictionary(const EmaString& dictNameAndServiceId)
{
	DictionaryPayload** dictionaryPayloadPtr = _dictionaryInfoHash.find(dictNameAndServiceId);

	if (dictionaryPayloadPtr)
	{
		_dictionaryInfoHash.erase(dictNameAndServiceId);
		_dictionaryInfoList.remove(*dictionaryPayloadPtr);
	}
}

DictionaryPayload* DictionaryHandler::getDictionaryPayload(const EmaString& serviceIdAndName) const
{
	DictionaryPayload** dictionaryPayloadPtr = _dictionaryInfoHash.find(serviceIdAndName);

	if (dictionaryPayloadPtr)
	{
		return *dictionaryPayloadPtr;
	}
	else
	{
		return 0;
	}
}

void DictionaryHandler::logReactorReleaseBufferError(const char* errorMethod, OmmServerBaseImpl* ommServerBaseImpl, ClientSession* clientSession, RsslErrorInfo* rsslErrorInfo)
{
	if (ommServerBaseImpl != NULL && clientSession!= NULL && rsslErrorInfo!= NULL
		&& OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
	{
		EmaString errorText;
		errorText.set("Internal error. Failed to encode dictionary message in DictionaryHandler::")
			.append(errorMethod)
			.append(CR).append("Client handle ").append(clientSession->getClientHandle())
			.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
			.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo->rsslError.text);

		ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
	}
}

RsslReturnCodes DictionaryHandler::sendFieldDictionaryResponse(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsg* dictionaryRequest, Dictionary* dictionary, RsslErrorInfo* rsslErrorInfo)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)reactor->userSpecPtr;
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	RsslRDMDictionaryMsg& dictionaryRefresh = ommServerBaseImpl->getDictionaryHandler()._rdmDictionaryRefresh;
	EmaString stateText;

	rsslClearRDMDictionaryMsg(&dictionaryRefresh);

	dictionaryRefresh.rdmMsgBase.rdmMsgType = RDM_DC_MT_REFRESH;
	dictionaryRefresh.rdmMsgBase.streamId = dictionaryRequest->rdmMsgBase.streamId;
	dictionaryRefresh.refresh.type = RDM_DICTIONARY_FIELD_DEFINITIONS;
	dictionaryRefresh.refresh.pDictionary = (RsslDataDictionary*)dictionary->getRsslDictionary();
	dictionaryRefresh.refresh.verbosity = dictionaryRequest->request.verbosity;
	dictionaryRefresh.refresh.serviceId = dictionaryRequest->request.serviceId;
	dictionaryRefresh.refresh.dictionaryName = dictionaryRequest->request.dictionaryName;
	dictionaryRefresh.refresh.flags = RDM_DC_RFF_SOLICITED;

	dictionaryRefresh.refresh.state.streamState = RSSL_STREAM_OPEN;
	dictionaryRefresh.refresh.state.dataState = RSSL_DATA_OK;
	dictionaryRefresh.refresh.state.code = RSSL_SC_NONE;

	bool firstPartMultiPartRefresh = true;

	while (true)
	{
		if (firstPartMultiPartRefresh)
		{
			dictionaryRefresh.refresh.flags |= RDM_DC_RFF_CLEAR_CACHE;
			firstPartMultiPartRefresh = false;
			dictionaryRefresh.refresh.startFid = dictionary->getRsslDictionary()->minFid;
		}
		else
		{
			dictionaryRefresh.refresh.flags &= ~RDM_DC_RFF_CLEAR_CACHE;
		}

		clearRsslErrorInfo(rsslErrorInfo);
		RsslBuffer* msgBuf = rsslReactorGetBuffer(reactorChannel, ommServerBaseImpl->getDictionaryHandler()._maxFieldDictFragmentSize, RSSL_FALSE, rsslErrorInfo);

		if (!msgBuf)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to get buffer in DictionaryHandler::sendFieldDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo->rsslError.text);
					
				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			return RSSL_RET_BUFFER_NO_BUFFERS;
		}

		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator(&encodeIter);

		RsslRet retCode = rsslSetEncodeIteratorRWFVersion(&encodeIter, reactorChannel->majorVersion, reactorChannel->minorVersion);
		if (retCode != RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to set encode iterator rwf version in DictionaryHandler::sendFieldDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendFieldDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		retCode = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
		if (retCode != RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to set encode iterator buffer in DictionaryHandler::sendFieldDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendFieldDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		stateText.set("Field Dictionary Refresh (starting fid ");
		stateText.append(dictionaryRefresh.refresh.startFid).append(")");
		dictionaryRefresh.refresh.state.text.data = (char*)stateText.c_str();
		dictionaryRefresh.refresh.state.text.length = stateText.length();

		clearRsslErrorInfo(rsslErrorInfo);
		retCode = rsslEncodeRDMDictionaryMsg(&encodeIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, rsslErrorInfo);

		if (retCode < RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to encode dictionary message in DictionaryHandler::sendFieldDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo->rsslError.text);

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendFieldDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		RsslReactorSubmitOptions submitOpts;
		rsslClearReactorSubmitOptions(&submitOpts);

		clearRsslErrorInfo(rsslErrorInfo);
		int ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);
		if (ret < RSSL_RET_SUCCESS)
		{
			while (ret == RSSL_RET_WRITE_CALL_AGAIN)
				ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);

			if (ret < RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString errorText;
					errorText.set("Internal error. Failed to submit dicitonary message in DictionaryHandler::sendFieldDictionaryResponse()")
						.append(CR).append("Client handle ").append(clientSession->getClientHandle())
						.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
						.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo->rsslError.text);


					ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
				}

				if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					logReactorReleaseBufferError("sendFieldDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
				}
			}

			return RSSL_RET_FAILURE;
		}
		
		if (retCode == RSSL_RET_SUCCESS)
		{
			break;
		}
	}

	if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
	{
		EmaString name(dictionaryRequest->request.dictionaryName.data, dictionaryRequest->request.dictionaryName.length);
		EmaString temp("Successfully sent field dictionary type.");
		temp.append(CR).append("Dictionary name ").append(name)
			.append(CR).append("Stream Id ").append(dictionaryRequest->rdmMsgBase.streamId)
			.append(CR).append("Client handle ").append(clientSession->getClientHandle());

		ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}

	return RSSL_RET_SUCCESS;
}

RsslReturnCodes DictionaryHandler::sendEnumTypeDictionaryResponse(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsg* dictionaryRequest, Dictionary* dictionary, RsslErrorInfo* rsslErrorInfo)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)reactor->userSpecPtr;
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	RsslRDMDictionaryMsg& dictionaryRefresh = ommServerBaseImpl->getDictionaryHandler()._rdmDictionaryRefresh;

	rsslClearRDMDictionaryMsg(&dictionaryRefresh);

	dictionaryRefresh.rdmMsgBase.rdmMsgType = RDM_DC_MT_REFRESH;
	dictionaryRefresh.rdmMsgBase.streamId = dictionaryRequest->rdmMsgBase.streamId;
	dictionaryRefresh.refresh.type = RDM_DICTIONARY_ENUM_TABLES;
	dictionaryRefresh.refresh.pDictionary = (RsslDataDictionary*)dictionary->getRsslDictionary();
	dictionaryRefresh.refresh.verbosity = dictionaryRequest->request.verbosity;
	dictionaryRefresh.refresh.serviceId = dictionaryRequest->request.serviceId;
	dictionaryRefresh.refresh.dictionaryName = dictionaryRequest->request.dictionaryName;
	dictionaryRefresh.refresh.flags = RDM_DC_RFF_SOLICITED;

	dictionaryRefresh.refresh.state.streamState = RSSL_STREAM_OPEN;
	dictionaryRefresh.refresh.state.dataState = RSSL_DATA_OK;
	dictionaryRefresh.refresh.state.code = RSSL_SC_NONE;

	EmaString stateText;

	bool firstPartMultiPartRefresh = true;

	while (true)
	{
		if (firstPartMultiPartRefresh)
		{
			dictionaryRefresh.refresh.flags |= RDM_DC_RFF_CLEAR_CACHE;
			firstPartMultiPartRefresh = false;
		}
		else
		{
			dictionaryRefresh.refresh.flags &= ~RDM_DC_RFF_CLEAR_CACHE;
		}

		clearRsslErrorInfo(rsslErrorInfo);
		RsslBuffer* msgBuf = rsslReactorGetBuffer(reactorChannel, ommServerBaseImpl->getDictionaryHandler()._maxEnumTypeFragmentSize, RSSL_FALSE, rsslErrorInfo);

		if (!msgBuf)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to get buffer in DictionaryHandler::sendEnumTypeDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo->rsslError.text);

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}
			return RSSL_RET_BUFFER_NO_BUFFERS;
		}

		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator(&encodeIter);

		RsslRet retCode = rsslSetEncodeIteratorRWFVersion(&encodeIter, reactorChannel->majorVersion, reactorChannel->minorVersion);
		if (retCode != RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to set encode iterator rwf version in DictionaryHandler::sendEnumTypeDictionaryResponse().")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendEnumTypeDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		retCode = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
		if (retCode != RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to set encode iterator buffer in DictionaryHandler::sendEnumTypeDictionaryResponse().")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendEnumTypeDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		stateText.set("Enum Dictionary Refresh (starting enum fid ");
		stateText.append(dictionaryRefresh.refresh.enumStartFid).append(")");
		dictionaryRefresh.refresh.state.text.data = (char*)stateText.c_str();
		dictionaryRefresh.refresh.state.text.length = stateText.length();

		clearRsslErrorInfo(rsslErrorInfo);
		retCode = rsslEncodeRDMDictionaryMsg(&encodeIter, (RsslRDMDictionaryMsg*)&dictionaryRefresh, &msgBuf->length, rsslErrorInfo);

		if (retCode < RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to encode dictionary message in DictionaryHandler::sendEnumTypeDictionaryResponse()")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo->rsslError.text);

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendEnumTypeDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}
			return RSSL_RET_FAILURE;
		}

		RsslReactorSubmitOptions submitOpts;
		rsslClearReactorSubmitOptions(&submitOpts);

		clearRsslErrorInfo(rsslErrorInfo);
		int ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);
		if (ret < RSSL_RET_SUCCESS)
		{
			while (ret == RSSL_RET_WRITE_CALL_AGAIN)
				ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);

			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString errorText;
				errorText.set("Internal error. Failed to submit dictionary message in DictionaryHandler::sendEnumTypeDictionaryResponse().")
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
					.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo->rsslError.text);

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
			}

			if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				logReactorReleaseBufferError("sendEnumTypeDictionaryResponse()", ommServerBaseImpl, clientSession, rsslErrorInfo);
			}

			return RSSL_RET_FAILURE;
		}

		if (retCode == RSSL_RET_SUCCESS)
		{
			break;
		}
	}

	if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
	{
		EmaString name(dictionaryRequest->request.dictionaryName.data, dictionaryRequest->request.dictionaryName.length);
		EmaString temp("Successfully sent enumeration dictionary type.");
		temp.append(CR).append("Dictionary name ").append(name)
			.append(CR).append("Stream Id ").append(dictionaryRequest->rdmMsgBase.streamId)
			.append(CR).append("Client handle ").append(clientSession->getClientHandle());
		
		ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}

	return RSSL_RET_SUCCESS;
}

RsslReturnCodes DictionaryHandler::sendRequestReject(RsslReactor* reactor, RsslReactorChannel* reactorChannel, RsslRDMDictionaryMsgEvent* rdmDictionaryMsgEvent, DictionaryRejectEnum reason, RsslErrorInfo* rsslErrorInfo, bool traceMessage)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)reactor->userSpecPtr;
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;
	RsslUInt32 statusMessageSize = INITIAL_DICTIONARY_STATUS_MSG_SIZE;

	if (rdmDictionaryMsgEvent->pRDMDictionaryMsg)
	{
		statusMessageSize += rdmDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.length;
	}

	clearRsslErrorInfo(rsslErrorInfo);
	RsslBuffer* msgBuf = rsslReactorGetBuffer(reactorChannel, statusMessageSize, RSSL_FALSE, rsslErrorInfo);

	if (!msgBuf)
	{
		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString errorText;
			errorText.set("Internal error. Failed to get buffer in DictionaryHandler::sendRequestReject().")
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo->rsslError.text);

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}

		return RSSL_RET_BUFFER_NO_BUFFERS;
	}

	RsslRDMDictionaryMsg& dictionaryStatus = ommServerBaseImpl->getDictionaryHandler()._rdmDictionaryStatus;

	rsslClearRDMDictionaryMsg(&dictionaryStatus);

	dictionaryStatus.rdmMsgBase.rdmMsgType = RDM_DC_MT_STATUS;
	dictionaryStatus.rdmMsgBase.streamId = rdmDictionaryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId;
	dictionaryStatus.status.flags = RDM_DC_STF_HAS_STATE;
	dictionaryStatus.status.state.dataState = RSSL_DATA_SUSPECT;
	dictionaryStatus.status.state.code = RSSL_SC_ERROR;
	dictionaryStatus.status.state.streamState = RSSL_STREAM_CLOSED_RECOVER;

	EmaString stateText;

	switch (reason)
	{
	case DICTIONARY_INVALID_MESSAGE:
	{
		stateText.set("Dictionary message rejected - invalid dictionary message.");
		break;
	}
	case DICTIONARY_NOT_LOADED:
	{
		stateText.set("Dictionary request message rejected - dictionary is not loaded in provider.");
		break;
	}
	case DICTIONARY_ENCODING_FAILED:
	{
		stateText.set("Dictionary request message rejected - failed to encode dictionary information.");
		break;
	}
	case USER_IS_NOT_LOGGED_IN:
	{
		stateText.set("Dictionary message rejected - there is no logged in user for this session.");
		break;
	}
	case DICTIONARY_NAME_NOT_FOUND:
	{
		stateText.set("Dictionary request message rejected - the reqesting dictionary name '");
		stateText.append(EmaString(rdmDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.data, rdmDictionaryMsgEvent->pRDMDictionaryMsg->request.dictionaryName.length));
		stateText.append("' not found.");
		break;
	}
	case SERVICE_ID_NOT_FOUND:
	{
		stateText.set("Dictionary request message rejected - the service Id = ");
		stateText.append(rdmDictionaryMsgEvent->pRDMDictionaryMsg->request.serviceId).append(" does not exist in the source directory");
		break;
	}
	default:
		break;
	}

	if ( traceMessage && ( OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity ) )
	{
		EmaString errorText(stateText);
		errorText.append(CR).append("Stream Id ").append(rdmDictionaryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
			.append(CR).append("Client handle ").append(clientSession->getClientHandle())
			.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

		ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, errorText);
	}

	dictionaryStatus.status.state.text.data = (char*)stateText.c_str();
	dictionaryStatus.status.state.text.length = stateText.length();

	RsslEncodeIterator encodeIter;
	rsslClearEncodeIterator(&encodeIter);

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion(&encodeIter, reactorChannel->majorVersion, reactorChannel->minorVersion);
	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString errorText;
			errorText.set("Internal error. Failed to set encode iterator rwf version in DictionaryHandler::sendRequestReject()")
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}

		if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			logReactorReleaseBufferError("sendRequestReject()", ommServerBaseImpl, clientSession, rsslErrorInfo);
		}
		return RSSL_RET_FAILURE;
	}

	retCode = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString errorText;
			errorText.set("Internal error. Failed to set encode iterator buffer in DictionaryHandler::sendRequestReject()")
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}

		if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			logReactorReleaseBufferError("sendRequestReject()", ommServerBaseImpl, clientSession, rsslErrorInfo);
		}
		return RSSL_RET_FAILURE;
	}

	clearRsslErrorInfo(rsslErrorInfo);
	retCode = rsslEncodeRDMDictionaryMsg(&encodeIter, (RsslRDMDictionaryMsg*)&dictionaryStatus, &msgBuf->length, rsslErrorInfo);
	if (retCode < RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString errorText;
			errorText.set("Internal error. Failed to encode status message in DictionaryHandler::sendRequestReject()")
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo->rsslError.text);

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}

		if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			logReactorReleaseBufferError("sendRequestReject()", ommServerBaseImpl, clientSession, rsslErrorInfo);
		}
		return RSSL_RET_FAILURE;
	}

	RsslReactorSubmitOptions submitOpts;
	rsslClearReactorSubmitOptions(&submitOpts);

	clearRsslErrorInfo(rsslErrorInfo);
	int ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);
	if (ret < RSSL_RET_SUCCESS)
	{
		while (ret == RSSL_RET_WRITE_CALL_AGAIN)
			ret = rsslReactorSubmit(reactor, reactorChannel, msgBuf, &submitOpts, rsslErrorInfo);

		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString errorText;
			errorText.set("Internal error. Failed to submit status message in DictionaryHandler::sendRequestReject()")
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo->rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo->rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo->errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo->rsslError.text);

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}

		if (rsslReactorReleaseBuffer(reactorChannel, msgBuf, rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			logReactorReleaseBufferError("sendRequestReject()", ommServerBaseImpl, clientSession, rsslErrorInfo);
		}
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

size_t DictionaryHandler::EmaStringPtrHasher::operator()(const EmaString& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value.c_str();
	UInt32 n = value.length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)* s++;
	return result;
}

bool DictionaryHandler::EmaStringPtrEqual_To::operator()(const EmaString& x, const EmaString& y) const
{
	return x == y;
}

UInt64 DictionaryHandler::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool DictionaryHandler::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y ? true : false;
}

DictionaryHandler* DictionaryHandler::create(OmmServerBaseImpl* ommServerBaseImpl)
{
	DictionaryHandler* dictionaryHandler = 0;

	try
	{
		dictionaryHandler = new DictionaryHandler(ommServerBaseImpl);
	}
	catch (std::bad_alloc&) {}

	if (!dictionaryHandler)
		ommServerBaseImpl->handleMee("Failed to allocate memory in DictionaryHandler::create()");

	return dictionaryHandler;
}

void DictionaryHandler::destroy(DictionaryHandler*& dictionaryHandler)
{
	if (dictionaryHandler)
	{
		delete dictionaryHandler;
		dictionaryHandler = 0;
	}
}

void DictionaryHandler::initialize()
{
	if (_apiAdminControl)
	{
		_maxFieldDictFragmentSize = static_cast<OmmIProviderActiveConfig&>(_pOmmServerBaseImpl->getActiveConfig()).getMaxFieldDictFragmentSize();
		_maxEnumTypeFragmentSize = static_cast<OmmIProviderActiveConfig&>(_pOmmServerBaseImpl->getActiveConfig()).getMaxEnumTypeFragmentSize();

		loadDictionaryFromFile();
	}
}
