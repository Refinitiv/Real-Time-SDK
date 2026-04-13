/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "PackedMsgImpl.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"
#include "OmmNiProviderImpl.h"
#include "OmmIProviderImpl.h"
#include "ChannelCallbackClient.h"
#include "ServerChannelHandler.h"
#include "NiProviderRoutingChannel.h"
#include "NiProviderRoutingSession.h"


using namespace refinitiv::ema::access;

#define MSG_PACKING_BUFFER_SIZE			6000

NiProvSessionTransportBuffer::NiProvSessionTransportBuffer()
{
	clear();
}

NiProvSessionTransportBuffer::~NiProvSessionTransportBuffer()
{
	releaseBuffer();
}

void NiProvSessionTransportBuffer::releaseBuffer()
{
	RsslErrorInfo errorInfo;
	if (pBuffer != NULL && pSessionChannel->pReactorChannel != NULL && pSessionChannel->pReactorChannel->pRsslChannel == pRsslChannel)
	{
		rsslReactorReleaseBuffer(pSessionChannel->pReactorChannel, pBuffer, &errorInfo);
	}

	pBuffer = NULL;
}

void NiProvSessionTransportBuffer::clear()
{
	pBuffer = NULL;
	pSessionChannel = NULL;
	pRsslChannel = NULL;
}

PackedMsgImpl::PackedMsgImpl(OmmProvider* ommProvider) :
	_ommProvider(ommProvider),
	_remainingSize(MSG_PACKING_BUFFER_SIZE),
	_maxSize(MSG_PACKING_BUFFER_SIZE),
	_packedMsgCount(0),
	_packedBuf(NULL),
	_clientHandle(0),
	_itemHandle(0),
	_reactorChannel(NULL),
	_ommIProviderImpl(NULL),
	_ommNiProviderImpl(NULL),
	_allocatedSize(0),
	_initialized(false)
{
	rsslClearEncodeIterator(&_eIter);
	rsslClearBuffer(&_encodeBuffer);

	if(_ommProvider->getProviderRole() == OmmProviderConfig::NonInteractiveEnum)
	{
		BaseRoutingSession* pRoutingSession = (static_cast<OmmNiProviderImpl*>(_ommProvider->_pImpl))->getRoutingSession();
		if (pRoutingSession != NULL)
		{
			for (UInt32 i = 0; i < pRoutingSession->routingChannelList.size(); ++i)
			{
				NiProvSessionTransportBuffer* pSessionBuffer = new NiProvSessionTransportBuffer();
				_rwfSessionBufferList.push_back(pSessionBuffer);
			}
		}
	}
}

PackedMsgImpl::~PackedMsgImpl() 
{
	if (_ommProvider->getProviderRole() == OmmProviderConfig::NonInteractiveEnum)
	{
		BaseRoutingSession* pRoutingSession = (static_cast<OmmNiProviderImpl*>(_ommProvider->_pImpl))->getRoutingSession();
		if (pRoutingSession != NULL)
		{
			// Release the buffers back to the rssl channels and delete the allocated session buffer objects.
			for (UInt32 i = 0; i < _rwfSessionBufferList.size(); ++i)
			{
				_rwfSessionBufferList[i]->releaseBuffer();
				
				delete _rwfSessionBufferList[i];
				_rwfSessionBufferList[i] = NULL;
			}

			_rwfSessionBufferList.clear();
		}
	}

	if (_encodeBuffer.data != NULL)
	{
		free(_encodeBuffer.data);
		_encodeBuffer.data = NULL;
	}
}

void PackedMsgImpl::initBuffer()
{
	clear();

	_maxSize = MSG_PACKING_BUFFER_SIZE;
	_remainingSize = MSG_PACKING_BUFFER_SIZE;

	if (_ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		EmaString temp("This method is used for Non-Interactive provider only. Setting a client handle is required when using Interactive Provider.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_ommNiProviderImpl = static_cast<OmmNiProviderImpl*>(_ommProvider->_pImpl);
	_reactorChannel = _ommNiProviderImpl->getRsslReactorChannel();

	if (_ommNiProviderImpl->getRoutingSession() == NULL && (!_reactorChannel || !_reactorChannel->pRsslChannel))
	{
		EmaString temp("initBuffer() failed because connection is not established.");
		throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
	}

	RsslErrorInfo rsslErrorInfo;

	if (_ommNiProviderImpl->getRoutingSession() == NULL)
	{
		if ((_packedBuf = rsslReactorGetBuffer(_reactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
		{
			EmaString temp("Failed to get packed buffer in initBuffer().");
			temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
				.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}
	}
	else
	{
		NiProviderRoutingSession* pRoutingSession = static_cast<NiProviderRoutingSession*>(_ommNiProviderImpl->getRoutingSession());

		// Allocate _encodeBuffer.data 
		if (_allocatedSize < _maxSize)
		{
			if (_encodeBuffer.data != NULL)
			{
				free(_encodeBuffer.data);
			}

			_encodeBuffer.data = static_cast<char*>(malloc(sizeof(char) * _maxSize + 7));

			if (_encodeBuffer.data == NULL)
			{
				const char* temp = "Failed to allocate encoded buffer in initBuffer().";
				throwMeeException(temp);
				return;
			}

			_allocatedSize = _maxSize;
		}

		_encodeBuffer.length = _maxSize;

		for (UInt32 i = 0; i < pRoutingSession->routingChannelList.size(); ++i)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pRoutingSession->routingChannelList[i]);
			
			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pRoutingChannel == NULL || pRoutingChannel->pReactorChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel->state != RSSL_CH_STATE_ACTIVE || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			NiProvSessionTransportBuffer* pSessionBuffer = _rwfSessionBufferList[i];

			if ((pSessionBuffer->pBuffer = rsslReactorGetBuffer(pReactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
			{
				// Release any previously acquired buffers back to the rssl channels.
				for(UInt32 j = 0; j < i; ++j)
				{
					_rwfSessionBufferList[j]->releaseBuffer();
					_rwfSessionBufferList[j]->clear();
				}

				EmaString temp("Failed to get packed buffer in initBuffer().");
				temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
					.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
				throwIueException(temp, OmmInvalidUsageException::NoBuffersEnum);
				return;
			}

			pSessionBuffer->pRsslChannel = pReactorChannel->pRsslChannel;
			pSessionBuffer->pSessionChannel = pRoutingChannel;
		}
	}

	_initialized = true;
}

void PackedMsgImpl::initBuffer(UInt32 maxSize)
{
	UInt32 previousMaxSize = _maxSize;
	clear();

	_maxSize = maxSize;
	_remainingSize = maxSize;

	if (_ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		EmaString temp("This method is used for Non-Interactive provider only. Setting a client handle is required when using Interactive Provider.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_ommNiProviderImpl = static_cast<OmmNiProviderImpl*>(_ommProvider->_pImpl);
	_reactorChannel = _ommNiProviderImpl->getRsslReactorChannel();

	if (_ommNiProviderImpl->getRoutingSession() == NULL && (!_reactorChannel || !_reactorChannel->pRsslChannel))
	{
		EmaString temp("initBuffer() failed because connection is not established.");
		throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
	}

	RsslErrorInfo rsslErrorInfo;


	if (_ommNiProviderImpl->getRoutingSession() == NULL)
	{
		if ((_packedBuf = rsslReactorGetBuffer(_reactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
		{
			EmaString temp("Failed to get packed buffer in initBuffer().");
			temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
				.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}
	}
	else
	{
		NiProviderRoutingSession* pRoutingSession = static_cast<NiProviderRoutingSession*>(_ommNiProviderImpl->getRoutingSession());

		// Allocate _encodeBuffer.data 
		if (_allocatedSize < _maxSize)
		{
			if (_encodeBuffer.data != NULL)
			{
				free(_encodeBuffer.data);
			}

			_encodeBuffer.data = static_cast<char*>(malloc(sizeof(char) * _maxSize + 7));

			if (_encodeBuffer.data == NULL)
			{
				const char* temp = "Failed to allocate encoded buffer in initBuffer().";
				throwMeeException(temp);
				return;
			}

			_allocatedSize = _maxSize;
		}

		_encodeBuffer.length = _maxSize;

		int packedBufferCount = 0;

		for (UInt32 i = 0; i < pRoutingSession->routingChannelList.size(); ++i)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pRoutingSession->routingChannelList[i]);

			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pRoutingChannel == NULL || pRoutingChannel->pReactorChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel->state != RSSL_CH_STATE_ACTIVE || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			NiProvSessionTransportBuffer* pSessionBuffer = _rwfSessionBufferList[i];

			if ((pSessionBuffer->pBuffer = rsslReactorGetBuffer(pReactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
			{
				// Release any previously acquired buffers back to the rssl channels.
				for (UInt32 j = 0; j < i; ++j)
				{
					_rwfSessionBufferList[j]->releaseBuffer();
					_rwfSessionBufferList[j]->clear();
				}

				EmaString temp("Failed to get packed buffer in initBuffer().");
				temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
					.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
				throwIueException(temp, OmmInvalidUsageException::NoBuffersEnum);
				return;
			}

			pSessionBuffer->pRsslChannel = pReactorChannel->pRsslChannel;
			pSessionBuffer->pSessionChannel = pRoutingChannel;
			packedBufferCount++;
		}

		if (packedBufferCount == 0)
		{
			EmaString temp("initBuffer() failed because connection is not established.");
			throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		}
	}

	_initialized = true;
}

void PackedMsgImpl::initBuffer(UInt64 clientHandle)
{
	clear();

	_maxSize = MSG_PACKING_BUFFER_SIZE;
	_remainingSize = MSG_PACKING_BUFFER_SIZE;
	_clientHandle = clientHandle;
	
	if (_ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		_ommIProviderImpl = static_cast<OmmIProviderImpl*>(_ommProvider->_pImpl);
		ClientSessionPtr pClientSession = _ommIProviderImpl->_pServerChannelHandler->getClientSession(_clientHandle);

		if (!pClientSession)
		{
			EmaString temp("Client handle is not valid.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}
		_reactorChannel = pClientSession->getChannel();
	}
	else
	{
		EmaString temp("This method is used for Interactive provider only. Setting a client handle is not required when using Non-Interactive Provider.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	if (!_reactorChannel || !_reactorChannel->pRsslChannel)
	{
		EmaString temp("initBuffer() failed because connection is not established.");
		throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
	}

	RsslErrorInfo rsslErrorInfo;
	
	if ((_packedBuf = rsslReactorGetBuffer(_reactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
	{
		EmaString temp("Failed to get packed buffer in initBuffer().");
		temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
			.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_initialized = true;
}

void PackedMsgImpl::initBuffer(UInt64 clientHandle, UInt32 maxSize)
{
	clear();

	_maxSize = maxSize;
	_remainingSize = maxSize;
	_clientHandle = clientHandle;

	if (_ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		_ommIProviderImpl = static_cast<OmmIProviderImpl*>(_ommProvider->_pImpl);
		ClientSessionPtr pClientSession = _ommIProviderImpl->_pServerChannelHandler->getClientSession(_clientHandle);

		if (!pClientSession)
		{
			EmaString temp("Client handle is not valid.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}
		_reactorChannel = pClientSession->getChannel();
	}
	else 
	{
		EmaString temp("This method is used for Interactive provider only. Setting a client handle is not required when using Non-Interactive Provider.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	if (!_reactorChannel || !_reactorChannel->pRsslChannel)
	{
		EmaString temp("initBuffer() failed because connection is not established.");
		throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
	}

	RsslErrorInfo rsslErrorInfo;

	if ((_packedBuf = rsslReactorGetBuffer(_reactorChannel, _maxSize, true, &rsslErrorInfo)) == NULL)
	{
		EmaString temp("Failed to get packed buffer in initBuffer().");
		temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
			.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_initialized = true;
}

void PackedMsgImpl::addMsg(const Msg& msg, UInt64 itemHandle)
{
	_itemHandle = itemHandle;

	if (!_initialized)
	{
		EmaString temp("addMsg() fails because initBuffer() was not called.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	// If this reactorChannel has no channel set, our connection is not established anymore
	// For a NiProvider session, this will be handled below.
	if (_ommNiProviderImpl == NULL || _ommNiProviderImpl->getRoutingSession() == NULL)
	{
		if (_reactorChannel != NULL && !_reactorChannel->pRsslChannel)
		{
			clear();

			EmaString temp("AddMsg() failed because connection is not established.");
			throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		}
	}

	// Item handle 0 is not valid for Interactive Providers
	if (_itemHandle == 0 && _ommIProviderImpl != NULL)
	{
		EmaString temp("Attempt to addMsg() while handle is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	RsslErrorInfo rsslErrorInfo;
	RsslRet ret = RSSL_RET_FAILURE;
	bool niProvHandleAdded = false;
	RsslInt32 streamId = 0;
	RsslMsg* rsslMsg = NULL;

	if (_ommIProviderImpl)
	{
		ItemInfo *itemInfo = _ommIProviderImpl->getItemInfo(_itemHandle);

		if (!itemInfo)
		{
			clear();

			EmaString temp("Incorrect handler incoming message.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		rsslMsg = MsgImpl::getImpl(msg)->getRsslMsg();

		rsslMsg->msgBase.streamId = itemInfo->getStreamId();

		if (MsgImpl::getImpl(msg)->hasServiceId())
		{
			if (!_ommIProviderImpl->getDirectoryServiceStore().getServiceNameById(MsgImpl::getImpl(msg)->getServiceId()))
			{
				EmaString temp(0, 512);
				temp.append("Attempt to add ");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append(" with service id of ");
				temp.append(MsgImpl::getImpl(msg)->getServiceId());
				temp.append(" that was not included in the SourceDirectory. Dropping this ");
				temp.append("\"");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append("\"").append(CR);

				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			}
		}
		else if (MsgImpl::getImpl(msg)->hasServiceName())
		{
			RsslUInt64* serviceId = _ommIProviderImpl->getDirectoryServiceStore().getServiceIdByName(&MsgImpl::getImpl(msg)->getServiceName());

			if (serviceId)
			{
				rsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				rsslMsg->msgBase.msgKey.serviceId = static_cast<RsslUInt16>(*serviceId);
			}
			else
			{
				EmaString temp(0, 512);
				temp.append("Attempt to add ");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append(" with service name of ");
				temp.append(MsgImpl::getImpl(msg)->getServiceName());
				temp.append(" that was not included in the SourceDirectory. Dropping this ");
				temp.append("\"");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append("\"").append(CR);

				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			}
		}
	}
	else
	{
		OmmNiProviderImpl::StreamInfoPtr* streamInfo = _ommNiProviderImpl->getStreamInfo(_itemHandle);
		rsslMsg = MsgImpl::getImpl(msg)->getRsslMsg();

		if (streamInfo)
		{	
			rsslMsg->msgBase.streamId = (*streamInfo)->_streamId;
		}
		else
		{
			try
			{
				streamId = _ommNiProviderImpl->getNextProviderStreamId();
				OmmNiProviderImpl::StreamInfoPtr pTemp = new OmmNiProviderImpl::StreamInfo(OmmNiProviderImpl::StreamInfo::ProvidingEnum, streamId);
				_ommNiProviderImpl->_handleToStreamInfo.insert(_itemHandle, pTemp);
				_ommNiProviderImpl->_streamInfoList.push_back(pTemp);
				rsslMsg->msgBase.streamId = streamId;
				niProvHandleAdded = true;
			}
			catch (std::bad_alloc&)
			{
				_ommNiProviderImpl->returnProviderStreamId(streamId);
				clear();

				EmaString temp("Failed to allocate memory in PackedMsgImpl::addMsg() for OmmNiProviderImpl::StreamInfo()");
				throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
			}
		}

		if (MsgImpl::getImpl(msg)->hasServiceId())
		{
			if (!_ommNiProviderImpl->getDirectoryServiceStore().getServiceNameById(MsgImpl::getImpl(msg)->getServiceId()))
			{
				EmaString temp(0, 512);
				temp.append("Attempt to add ");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append(" with service id of ");
				temp.append(MsgImpl::getImpl(msg)->getServiceId());
				temp.append(" that was not included in the SourceDirectory. Dropping this ");
				temp.append("\"");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append("\"").append(CR);

				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			}
		}
		else if (MsgImpl::getImpl(msg)->hasServiceName())
		{
			RsslUInt64* serviceId = _ommNiProviderImpl->getDirectoryServiceStore().getServiceIdByName(&MsgImpl::getImpl(msg)->getServiceName());

			if (serviceId)
			{
				rsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
				rsslMsg->msgBase.msgKey.serviceId = static_cast<RsslUInt16>(*serviceId);
			}
			else
			{
				EmaString temp(0, 512);
				temp.append("Attempt to add ");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append(" with service name of ");
				temp.append(MsgImpl::getImpl(msg)->getServiceName());
				temp.append(" that was not included in the SourceDirectory. Dropping this ");
				temp.append("\"");
				temp.append(rsslMsgClassToString(rsslMsg->msgBase.msgClass));
				temp.append("\"").append(CR);

				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			}
		}
	}

	// We've already verified everything, so if _ommIProviderImpl is NULL, _ommNiProviderImpl will not be NULL
	if (_ommIProviderImpl != NULL || _ommNiProviderImpl->getRoutingSession() == NULL)
	{
		rsslClearEncodeIterator(&_eIter);

		if ((ret = rsslSetEncodeIteratorRWFVersion(&_eIter, MsgImpl::getImpl(msg)->getMajorVersion(), MsgImpl::getImpl(msg)->getMinorVersion())) < RSSL_RET_SUCCESS)
		{
			clear();

			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}

			EmaString temp("Failed rsslSetEncodeIteratorRWFVersion() with code: ");
			temp.append(ret).append(CR);
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		if ((ret = rsslSetEncodeIteratorBuffer(&_eIter, _packedBuf)) < RSSL_RET_SUCCESS)
		{
			clear();

			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}

			EmaString temp("Failed rsslSetEncodeIteratorBuffer() with code: ");
			temp.append(ret).append(CR);
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		if ((ret = rsslEncodeMsg(&_eIter, rsslMsg)) < RSSL_RET_SUCCESS)
		{
			if (ret == RSSL_RET_BUFFER_TOO_SMALL)
			{
				if (niProvHandleAdded)
				{
					OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
					_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
					_ommNiProviderImpl->returnProviderStreamId(streamId);
				}

				EmaString temp("Failed rsslEncodeBuffer(). Buffer too small. Error code: ");
				temp.append(ret).append(CR);
				throwIueException(temp, OmmInvalidUsageException::BufferTooSmallEnum);
			}
			else
			{
				// This is fatal, there has been an encoding error, clear this object and throw exception.
				clear();

				if (niProvHandleAdded)
				{
					OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
					_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
					_ommNiProviderImpl->returnProviderStreamId(streamId);
				}

				EmaString temp("Failed rsslEncodeBuffer() with code: ");
				temp.append(ret).append(CR);
				throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
			}
		}

		_packedBuf->length = rsslGetEncodedBufferLength(&_eIter);

		RsslBuffer* packedBuf = rsslReactorPackBuffer(_reactorChannel, _packedBuf, &rsslErrorInfo);
		if (packedBuf != NULL)
		{
			_packedBuf = packedBuf;

			_remainingSize = _packedBuf->length;
			_packedMsgCount++;
		}
		else
		{
			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}

			EmaString temp("Failed to pack buffer during addMsg().");
			OmmInvalidUsageException::ErrorCode errorCode;

			if (rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE && rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL)
			{
				errorCode = OmmInvalidUsageException::BufferTooSmallEnum;
				temp.append(" Buffer too small.");
			}
			else
			{
				clear();
				switch (rsslErrorInfo.rsslErrorInfoCode)
				{
				case RSSL_EIC_FAILURE:
					errorCode = OmmInvalidUsageException::FailureEnum;
					break;
				case RSSL_EIC_SHUTDOWN:
					errorCode = OmmInvalidUsageException::NoActiveChannelEnum;
					break;
				default:
					errorCode = OmmInvalidUsageException::FailureEnum;
				}
			}

			temp.append("Msg: ").append(msg.toString()).append(CR)
				.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
				.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
			throwIueException(temp, errorCode);
		}
	}
	else
	{
		// Routing Session case.
		// Iterate through _rwfSessionBufferList, encoding at the first connected opportunity.
		int packedMessages = 0;
		bool encodedData = false;

		for(UInt32 i = 0; i < _rwfSessionBufferList.size(); ++i)
		{
			NiProvSessionTransportBuffer* pSessionTransportBuffer = _rwfSessionBufferList[i];
			NiProviderRoutingSessionChannel* pRoutingChannel = pSessionTransportBuffer->pSessionChannel;
			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// The reactor channel's current rssl channel does not match the saved SessionTransportBuffer's pRsslChannel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pRoutingChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum || pRoutingChannel->pReactorChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel == NULL || pRoutingChannel->pReactorChannel->pRsslChannel->state != RSSL_CH_STATE_ACTIVE || pRoutingChannel->pReactorChannel->pRsslChannel != pSessionTransportBuffer->pRsslChannel)
			{
				// Don't need to release the buffer here, because it's already been cleaned up, or will be cleaned up shortly
				pSessionTransportBuffer->clear();
				continue;
			}

			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			if(!encodedData)
			{
				rsslClearEncodeIterator(&_eIter);
				
				_encodeBuffer.length = _remainingSize;

				if ((ret = rsslSetEncodeIteratorRWFVersion(&_eIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion)) < RSSL_RET_SUCCESS)
				{
					if (niProvHandleAdded)
					{
						OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
						_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
						delete* pTempStreamInfoPtr;
						_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
						_ommNiProviderImpl->returnProviderStreamId(streamId);
					}

					clear();

					EmaString temp("Failed rsslSetEncodeIteratorRWFVersion() with code: ");
					temp.append(ret).append(CR);
					throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
				}

				if ((ret = rsslSetEncodeIteratorBuffer(&_eIter, &_encodeBuffer)) < RSSL_RET_SUCCESS)
				{
					if (niProvHandleAdded)
					{
						OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
						_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
						delete* pTempStreamInfoPtr;
						_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
						_ommNiProviderImpl->returnProviderStreamId(streamId);
					}

					clear();

					EmaString temp("Failed rsslSetEncodeIteratorBuffer() with code: ");
					temp.append(ret).append(CR);
					throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
				}

				if ((ret = rsslEncodeMsg(&_eIter, rsslMsg)) < RSSL_RET_SUCCESS)
				{
					// If we get BUFFER_TOO_SMALL, this is not fatal for the packedMsg object, so just throw the exception with the BufferTooSmallEnum.
					// User can decide to send the already packed messages, or clear the packedMsg and allocate a larger buffer.
					if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					{
						if (niProvHandleAdded)
						{
							OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
							_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
							_ommNiProviderImpl->returnProviderStreamId(streamId);
						}

						EmaString temp("Failed rsslEncodeBuffer(). Buffer too small. Error code: ");
						temp.append(ret).append(CR);
						throwIueException(temp, OmmInvalidUsageException::BufferTooSmallEnum);
					}
					else
					{
						clear();

						if (niProvHandleAdded)
						{
							OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
							_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
							_ommNiProviderImpl->returnProviderStreamId(streamId);
						}

						EmaString temp("Failed rsslEncodeBuffer() with code: ");
						temp.append(ret).append(CR);
						throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
					}
				}
				_encodeBuffer.length = rsslGetEncodedBufferLength(&_eIter);
			}

			// Encoding has succeeded, we know that this will fit in the remaining space, so copy the data now.
			memcpy(pSessionTransportBuffer->pBuffer->data, _encodeBuffer.data, (size_t)_encodeBuffer.length);
			pSessionTransportBuffer->pBuffer->length = _encodeBuffer.length;

			RsslBuffer* packedBuf = rsslReactorPackBuffer(pReactorChannel, pSessionTransportBuffer->pBuffer, &rsslErrorInfo);
			if (packedBuf != NULL)
			{
				_packedBuf = packedBuf;

				_remainingSize = _packedBuf->length;
				_packedMsgCount++;
				packedMessages++;
			}
			else
			{
				if (niProvHandleAdded)
				{
					OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
					_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
					_ommNiProviderImpl->returnProviderStreamId(streamId);
				}

				EmaString temp("Failed to pack buffer during addMsg().");
				OmmInvalidUsageException::ErrorCode errorCode;

				// Buffer too small shouldn't ever happen because we've already verified this for an OMM buffer, so throw an error here.  Don't clean up the full pack, because the previous packed messages can still be sent. 
				if (rsslErrorInfo.rsslErrorInfoCode == RSSL_EIC_FAILURE && rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL)
				{
					errorCode = OmmInvalidUsageException::BufferTooSmallEnum;
					temp.append(" Buffer too small.");
				}
				else
				{
					switch (rsslErrorInfo.rsslErrorInfoCode)
					{
						
					case RSSL_EIC_FAILURE:
						// RSSL_EIC_FAILURE and the reactorChannel being down/cleaned up or the underling RsslChannel not active means that there was a channel failure between the initial up check and here, so jsut cleanup the packed buffer and continue.
						// This will be cleaned up when the channel is closed by the reactor.  The other are fatal, so clean up the packedMsg structure.
						if (pRoutingChannel->pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pReactorChannel->pRsslChannel->state != RSSL_CH_STATE_ACTIVE)
						{
							pSessionTransportBuffer->clear();
							continue;
						}
						clear();
						break;
					case RSSL_EIC_SHUTDOWN:
						clear();
						errorCode = OmmInvalidUsageException::NoActiveChannelEnum;
						break;
					default:
						clear();
						errorCode = OmmInvalidUsageException::FailureEnum;
					}
				}
				
				temp.append("Msg: ").append(msg.toString()).append(CR)
					.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
					.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
				throwIueException(temp, errorCode);
			}
		}

		if(packedMessages == 0)
		{
			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}

			EmaString temp("AddMsg() failed because connection is not established.");
			throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		}
	}
}

UInt32 PackedMsgImpl::remainingSize() const
{
	return _remainingSize;
}

UInt32 PackedMsgImpl::packedMsgCount() const 
{
	return _packedMsgCount;
}

UInt32 PackedMsgImpl::maxSize() const
{
	return _maxSize;
}

void PackedMsgImpl::clear() 
{	
	RsslRet retVal;
	RsslErrorInfo rsslErrorInfo;

	if (_packedBuf && _packedBuf->data && (retVal = rsslReactorReleaseBuffer(_reactorChannel, _packedBuf, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
	{
		EmaString temp("Failed to release Msg buffer in addMsg().");
		temp.append("Return code: ").append((UInt64)retVal).append(CR);
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if (_ommNiProviderImpl != NULL && _ommNiProviderImpl->getRoutingSession() != NULL)
	{
		for (UInt32 i = 0; i < _rwfSessionBufferList.size(); ++i)
		{
			// Clear release the buffer back to each rssl channel and clear the session transport buffer for reuse.
			NiProvSessionTransportBuffer* pSessionBuffer = _rwfSessionBufferList[i];
			pSessionBuffer->releaseBuffer();
			pSessionBuffer->clear();
		}
	}

	_remainingSize = 0;
	_packedMsgCount = 0;
	_initialized = false;
}

void PackedMsgImpl::reset()
{
	_packedBuf = NULL;
	_remainingSize = 0;
	_packedMsgCount = 0;
	_initialized = false;
}

UInt64 PackedMsgImpl::getClientHandle() const
{
	return _clientHandle;
}

RsslReactorChannel*  PackedMsgImpl::getRsslReactorChannel() const
{
	return _reactorChannel;
}

RsslBuffer* PackedMsgImpl::getTransportBuffer() const
{
	return _packedBuf;
}

void PackedMsgImpl::setTransportBuffer(RsslBuffer* buffer)
{
	_packedBuf = buffer;
}

EmaVector<NiProvSessionTransportBuffer*>& PackedMsgImpl::getRwfSessionBufferList()
{
	return _rwfSessionBufferList;
}
