/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#include "DirectoryHandler.h"
#include "ClientSession.h"
#include "ItemInfo.h"
#include "OmmProviderClient.h"
#include "OmmServerBaseImpl.h"
#include "DirectoryServiceStore.h"
#include "OmmServerBaseImpl.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "Utilities.h"

#include <new>

using namespace rtsdk::ema::access;

const EmaString DirectoryHandler::_clientName("DirectoryHandler");

static struct
{
	RsslInt32 rdmMsgType;
	const char* strMsgType;
}  ConverterRdmDirMsgTypeToStr[] =
{
	{ RDM_DR_MT_UNKNOWN, "Unknown" },
	{ RDM_DR_MT_REQUEST, "Request" },
	{ RDM_DR_MT_CLOSE, "Close" },
	{ RDM_DR_MT_CONSUMER_STATUS, "Consumer Status" },
	{ RDM_DR_MT_REFRESH, "Refresh" },
	{ RDM_DR_MT_UPDATE, "Update" },
	{ RDM_DR_MT_STATUS, "Status" }
};

DirectoryHandler::DirectoryHandler(OmmServerBaseImpl* ommServerBaseImpl) :
	_pOmmServerBaseImpl(ommServerBaseImpl)
{
	_apiAdminControl = _pOmmServerBaseImpl->getActiveConfig().getDirectoryAdminControl() == OmmIProviderConfig::ApiControlEnum  ? true : false;

	_refreshText.data = (char *)"Refresh Complete";
	_refreshText.length = 16;
}

DirectoryHandler::~DirectoryHandler()
{
	if (_rsslMsgBuffer.data)
	{
		free(_rsslMsgBuffer.data);
		_rsslMsgBuffer.data = 0;
	}
}

void DirectoryHandler::initialize(EmaConfigServerImpl* configImpl)
{
	_rsslMsgBuffer.length = 2048;
	_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

	if (!_rsslMsgBuffer.data)
	{
		_pOmmServerBaseImpl->handleMee("Failed to allocate memory in DirectoryHandler::initialize()");
		return;
	}
}

RsslReactorCallbackRet DirectoryHandler::directoryCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pRDMDirectoryMsgEvent)
{
	OmmServerBaseImpl* ommServerBaseImpl = (OmmServerBaseImpl*)pReactor->userSpecPtr;
	ommServerBaseImpl->eventReceived();

	ClientSession* clientSession = (ClientSession*)pReactorChannel->userSpecPtr;

	RsslRDMDirectoryMsg* pDirectoryMsg = pRDMDirectoryMsgEvent->pRDMDirectoryMsg;

	if (!pDirectoryMsg)
	{
		if (pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.msgClass == RSSL_MC_GENERIC)
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received Generic message on directory stream.");
				temp.append(CR).append("Stream Id ").append(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId);

			if (itemInfo)
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_genericMsg, pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg,
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

		EmaString temp("Directory message rejected - invalid directory domain message.");

		ommServerBaseImpl->getDirectoryHandler().sendDirectoryReject(pReactorChannel, pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId, RSSL_SC_USAGE_ERROR, temp);

		if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			temp.append(CR).append("Stream Id ").append(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->msgBase.streamId)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if (!ommServerBaseImpl->getActiveConfig().acceptMessageWithoutBeingLogin && !clientSession->isLogin())
	{
		EmaString temp("Directory message rejected - there is no logged in user for this session.");

		ommServerBaseImpl->getDirectoryHandler().sendDirectoryReject(pReactorChannel, pDirectoryMsg->rdmMsgBase.streamId, RSSL_SC_USAGE_ERROR, temp);

		if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
				.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

			ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}
  
	switch (pDirectoryMsg->rdmMsgBase.rdmMsgType)
	{
		case RDM_DR_MT_REQUEST:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received directory request message.");
				temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pDirectoryMsg->rdmMsgBase.streamId);

			if ( ommServerBaseImpl->getActiveConfig().acceptDirMessageWithoutMinFilters || ((pDirectoryMsg->request.filter
				& (RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER) ) == (RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER)))
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_reqMsg, pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg,
					pReactorChannel->majorVersion,
					pReactorChannel->minorVersion,
					0);

				ommServerBaseImpl->ommProviderEvent._clientHandle = clientSession->getClientHandle();
				ommServerBaseImpl->ommProviderEvent._closure = ommServerBaseImpl->_pClosure;
				ommServerBaseImpl->ommProviderEvent._provider = ommServerBaseImpl->getProvider();

				if ( !itemInfo )
				{
					ItemInfo* itemInfo = ItemInfo::create(*ommServerBaseImpl);

					if (!itemInfo || !itemInfo->setRsslRequestMsg(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
						return RSSL_RC_CRET_SUCCESS;

					itemInfo->setClientSession(clientSession);

					ommServerBaseImpl->getDirectoryHandler().addItemInfo(itemInfo);
					ommServerBaseImpl->addItemInfo(itemInfo);

					if (ommServerBaseImpl->getDirectoryHandler()._apiAdminControl == false)
					{
						if (pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
						{
							const EmaString** serviceNamePtr = ommServerBaseImpl->getDirectoryServiceStore().getServiceNameById(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg.msgBase.msgKey.serviceId);

							if (serviceNamePtr)
							{
								ommServerBaseImpl->_reqMsg.getDecoder().setServiceName((*serviceNamePtr)->c_str(), (*serviceNamePtr)->length());
							}
						}

						ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
						ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
						ommServerBaseImpl->_pOmmProviderClient->onReqMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
					}
					else
					{
						ommServerBaseImpl->getDirectoryHandler().handleDirectoryRequest(pReactorChannel, pDirectoryMsg->request);
					}
				}
				else
				{
					if ( !itemInfo->setRsslRequestMsg(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg->requestMsg) )
					{
						return RSSL_RC_CRET_SUCCESS;
					}

					if (ommServerBaseImpl->getDirectoryHandler()._apiAdminControl == false)
					{
						ommServerBaseImpl->ommProviderEvent._handle = (UInt64)itemInfo;
						ommServerBaseImpl->_pOmmProviderClient->onAllMsg(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
						ommServerBaseImpl->_pOmmProviderClient->onReissue(ommServerBaseImpl->_reqMsg, ommServerBaseImpl->ommProviderEvent);
					}
					else
					{
						ommServerBaseImpl->getDirectoryHandler().handleDirectoryRequest(pReactorChannel, pDirectoryMsg->request);
					}
				}
			}
			else
			{
				EmaString temp("Source directory request rejected - request message must minimally have SERVICE_INFO_FILTER and SERVICE_STATE_FILTER filters");

				ommServerBaseImpl->getDirectoryHandler().sendDirectoryReject(pReactorChannel, pDirectoryMsg->rdmMsgBase.streamId, RSSL_SC_USAGE_ERROR, temp);

				if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
						.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName())
						.append(CR).append("Client handle ").append(clientSession->getClientHandle());

					ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
				}

				if (itemInfo)
				{
					if (ommServerBaseImpl->getDirectoryHandler()._apiAdminControl == false)
					{
						ommServerBaseImpl->getDirectoryHandler().notifyOnClose(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg, itemInfo);
					}

					ommServerBaseImpl->getDirectoryHandler().removeItemInfo(itemInfo);
					ommServerBaseImpl->removeItemInfo(itemInfo, false);
				}
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		case RDM_DR_MT_CONSUMER_STATUS:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received directory consumer status message.");
				temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pDirectoryMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				StaticDecoder::setRsslData(&ommServerBaseImpl->_genericMsg, pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg,
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
		case RDM_DR_MT_CLOSE:
		{
			if (OmmLoggerClient::VerboseEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Received directory close message.");
				temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			ItemInfo* itemInfo = clientSession->getItemInfo(pDirectoryMsg->rdmMsgBase.streamId);

			if (itemInfo)
			{
				if (ommServerBaseImpl->getDirectoryHandler()._apiAdminControl == false)
				{
					ommServerBaseImpl->getDirectoryHandler().notifyOnClose(pRDMDirectoryMsgEvent->baseMsgEvent.pRsslMsg, itemInfo);
				}

				ommServerBaseImpl->getDirectoryHandler().removeItemInfo(itemInfo);
				ommServerBaseImpl->removeItemInfo(itemInfo, false);
			}

			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			EmaString temp("Rejected unhandled directory message type: ");
			temp.append(ConverterRdmDirMsgTypeToStr[pDirectoryMsg->rdmMsgBase.rdmMsgType].strMsgType);

			ItemInfo* itemInfo = clientSession->getItemInfo(pDirectoryMsg->rdmMsgBase.streamId);

			if (!itemInfo)
			{
				ommServerBaseImpl->getDirectoryHandler().sendDirectoryReject(pReactorChannel, pDirectoryMsg->rdmMsgBase.streamId, RSSL_SC_USAGE_ERROR, temp);
			}

			if (OmmLoggerClient::ErrorEnum >= ommServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				temp.append(CR).append("Stream Id ").append(pDirectoryMsg->rdmMsgBase.streamId)
					.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(ommServerBaseImpl->getInstanceName());

				ommServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void DirectoryHandler::notifyOnClose(RsslMsg* pRsslMsg, ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_reqMsg.clear();
	_pOmmServerBaseImpl->_reqMsg.initialImage(false);
	_pOmmServerBaseImpl->_reqMsg.interestAfterRefresh(false);
	_pOmmServerBaseImpl->_reqMsg.streamId(pRsslMsg->msgBase.streamId);
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
}

DirectoryHandler* DirectoryHandler::create(OmmServerBaseImpl* ommServerBaseImpl)
{
	DirectoryHandler* directoryHandler = 0;

	try
	{
		directoryHandler = new DirectoryHandler(ommServerBaseImpl);
	}
	catch (std::bad_alloc&) {}

	if (!directoryHandler)
		ommServerBaseImpl->handleMee("Failed to allocate memory in DirectoryHandler::create()");

	return directoryHandler;
}

void DirectoryHandler::destroy(DirectoryHandler*& directoryHandler)
{
	if (directoryHandler)
	{
		delete directoryHandler;
		directoryHandler = 0;
	}
}

const EmaVector< ItemInfo* >&	DirectoryHandler::getDirectoryItemList()
{
	return _itemInfoList;
}

void DirectoryHandler::addItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();

	_itemInfoList.push_back(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

void DirectoryHandler::removeItemInfo(ItemInfo* itemInfo)
{
	_pOmmServerBaseImpl->_userLock.lock();

	_itemInfoList.removeValue(itemInfo);

	_pOmmServerBaseImpl->_userLock.unlock();
}

void DirectoryHandler::sendDirectoryReject(RsslReactorChannel* reactorChannel, RsslInt32 streamId, RsslStateCodes stateCode, EmaString& stateText)
{	
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	rsslClearRDMDirectoryStatus(&_rsslRdmDirectoryMsg.status);
	_rsslRdmDirectoryMsg.status.flags |= RDM_DR_STF_HAS_STATE;
	_rsslRdmDirectoryMsg.status.rdmMsgBase.streamId = streamId;
	_rsslRdmDirectoryMsg.status.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	_rsslRdmDirectoryMsg.status.state.dataState = RSSL_DATA_SUSPECT;
	_rsslRdmDirectoryMsg.status.state.code = stateCode;
	_rsslRdmDirectoryMsg.status.state.text.data = (char*)stateText.c_str();
	_rsslRdmDirectoryMsg.status.state.text.length = stateText.length();

	rsslClearEncodeIterator(&_rsslEncodeIter);

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion(&_rsslEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator version in DirectoryHandler::sendDirectoryReject()");
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
			EmaString temp("Internal error. Failed to set encode iterator buffer in DirectoryHandler::sendDirectoryReject()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}

	RsslErrorInfo rsslErrorInfo;
	RsslRet retCode;
	retCode = rsslEncodeRDMDirectoryMsg(&_rsslEncodeIter, (RsslRDMDirectoryMsg*)&_rsslRdmDirectoryMsg.status, &rsslBuffer.length, &rsslErrorInfo);

	while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(rsslBuffer.data);

		_rsslMsgBuffer.length += _rsslMsgBuffer.length;
		_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

		if (!_rsslMsgBuffer.data)
		{
			if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to allocate memory in DirectoryHandler::sendDirectoryReject()");
				temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

				_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}

			return;
		}

		rsslBuffer.data = _rsslMsgBuffer.data;
		rsslBuffer.length = _rsslMsgBuffer.length;

		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeRDMDirectoryMsg(&_rsslEncodeIter, (RsslRDMDirectoryMsg*)&_rsslRdmDirectoryMsg, &rsslBuffer.length, &rsslErrorInfo);
	}

	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: failed to encode RsslRDMDirectoryMsg in DirectoryHandler::sendDirectoryReject()");
				temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
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
			EmaString temp("IInternal error. Failed to set decode iterator version in DirectoryHandler::sendDirectoryReject()");
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
			EmaString temp("Internal error. Failed to set decode iterator buffer in DirectoryHandler::sendDirectoryReject()");
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
			EmaString temp("Internal error. Failed to decode message in DirectoryHandler::sendDirectoryReject()");
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
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in DirectoryHandler::sendDirectoryReject().");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return;
	}
}

void DirectoryHandler::handleDirectoryRequest(RsslReactorChannel* reactorChannel, RsslRDMDirectoryRequest& directoryRequest)
{
	ClientSession* clientSession = (ClientSession*)reactorChannel->userSpecPtr;

	rsslClearRDMDirectoryRefresh(&_rsslRdmDirectoryMsg.refresh);

	RsslBuffer rsslBuffer;
	rsslBuffer.data = _rsslMsgBuffer.data;
	rsslBuffer.length = _rsslMsgBuffer.length;

	_rsslRdmDirectoryMsg.refresh.flags = RDM_DR_RFF_CLEAR_CACHE;
	_rsslRdmDirectoryMsg.refresh.state.text.data = _refreshText.data;
	_rsslRdmDirectoryMsg.refresh.state.text.length = _refreshText.length;

	bool specificServiceId = directoryRequest.flags & RDM_DR_RQF_HAS_SERVICE_ID ? true : false;

	if (!DirectoryServiceStore::encodeDirectoryRefreshMsg(_pOmmServerBaseImpl->getDirectoryServiceStore().getDirectory(),
		_rsslRdmDirectoryMsg.refresh, directoryRequest.filter, specificServiceId, directoryRequest.serviceId))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Failed to allocate memory in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);
		
		return;
	}

	_rsslRdmDirectoryMsg.refresh.flags |= RDM_DR_RFF_SOLICITED;

	rsslClearEncodeIterator(&_rsslEncodeIter);

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion(&_rsslEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator version in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer(&_rsslEncodeIter, &rsslBuffer))
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set encode iterator buffer in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	RsslRet retCode = rsslEncodeRDMDirectoryMsg(&_rsslEncodeIter, (RsslRDMDirectoryMsg*)&_rsslRdmDirectoryMsg, &rsslBuffer.length, &rsslErrorInfo);

	while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(rsslBuffer.data);

		_rsslMsgBuffer.length += _rsslMsgBuffer.length;
		_rsslMsgBuffer.data = (char*)malloc(sizeof(char) * _rsslMsgBuffer.length);

		if (!_rsslMsgBuffer.data)
		{
			if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to allocate memory in DirectoryHandler::handleDirectoryRequest()");
				temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
					.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

				_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
			}

			DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

			return;
		}

		rsslBuffer.data = _rsslMsgBuffer.data;
		rsslBuffer.length = _rsslMsgBuffer.length;

		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeRDMDirectoryMsg(&_rsslEncodeIter, (RsslRDMDirectoryMsg*)&_rsslRdmDirectoryMsg, &rsslBuffer.length, &rsslErrorInfo);
	}

	if (retCode != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: failed to encode RsslRDMDirectoryMsg in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	rsslClearDecodeIterator(&_rsslDecodeIter);

	if (rsslSetDecodeIteratorRWFVersion(&_rsslDecodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set decode iterator version in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	if (rsslSetDecodeIteratorBuffer(&_rsslDecodeIter, &rsslBuffer) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to set decode iterator buffer in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	RsslRefreshMsg rsslRefreshMsg;
	rsslClearRefreshMsg(&rsslRefreshMsg);
	if (rsslDecodeMsg(&_rsslDecodeIter, (RsslMsg*)&rsslRefreshMsg) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. Failed to decode message in DirectoryHandler::handleDirectoryRequest()");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName());

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	rsslRefreshMsg.msgBase.streamId = directoryRequest.rdmMsgBase.streamId;

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)&rsslRefreshMsg;

	EmaString temp;

	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pOmmServerBaseImpl->getRsslReactor(), reactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _pOmmServerBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in DirectoryHandler::handleDirectoryRequest().");
			temp.append(CR).append("Client handle ").append(clientSession->getClientHandle())
				.append(CR).append("Instance Name ").append(_pOmmServerBaseImpl->getInstanceName())
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_pOmmServerBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);

		return;
	}

	DirectoryServiceStore::freeMemory(_rsslRdmDirectoryMsg.refresh, 0);
}
