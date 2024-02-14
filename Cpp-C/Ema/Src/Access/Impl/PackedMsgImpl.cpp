/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the projects LICENSE.md for details.                  --
 *|          Copyright (C) 2023 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "PackedMsgImpl.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"
#include "OmmNiProviderImpl.h"
#include "OmmIProviderImpl.h"
#include "ChannelCallbackClient.h"
#include "ServerChannelHandler.h"


using namespace refinitiv::ema::access;

#define MSG_PACKING_BUFFER_SIZE			6000

PackedMsgImpl::PackedMsgImpl(OmmProvider* ommProvider):
	_ommProvider(ommProvider),
	_remainingSize(MSG_PACKING_BUFFER_SIZE),
	_maxSize(MSG_PACKING_BUFFER_SIZE),
	_packedMsgCount(0),
	_packedBuf(NULL),
	_clientHandle(0),
	_itemHandle(0),
	_reactorChannel(NULL),
	_ommIProviderImpl(NULL),
	_ommNiProviderImpl(NULL)
{
}

PackedMsgImpl::~PackedMsgImpl() 
{
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
	_reactorChannel = const_cast<ChannelList&>(_ommNiProviderImpl->getChannelCallbackClient().getChannelList())[0];

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
}

void PackedMsgImpl::initBuffer(UInt32 maxSize)
{
	clear();

	_maxSize = maxSize;
	_remainingSize = maxSize;

	if (_ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		EmaString temp("This method is used for Non-Interactive provider only. Setting a client handle is required when using Interactive Provider.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_ommNiProviderImpl = static_cast<OmmNiProviderImpl*>(_ommProvider->_pImpl);
	_reactorChannel = const_cast<ChannelList&>(_ommNiProviderImpl->getChannelCallbackClient().getChannelList())[0];

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
		EmaString temp("Failed to get packed buffer in addMsg().");
		temp.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
			.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
}

void PackedMsgImpl::addMsg(const Msg& msg, UInt64 itemHandle)
{
	_itemHandle = itemHandle;

	if (!_packedBuf)
	{
		EmaString temp("addMsg() fails because initBuffer() was not called.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if (!_reactorChannel || !_reactorChannel->pRsslChannel)
	{
		reactorReleaseBuffer();

		EmaString temp("AddMsg() failed because connection is not established.");
		throwIueException(temp, OmmInvalidUsageException::NoActiveChannelEnum);
	}
	else if (_itemHandle == 0 && _ommProvider->getProviderRole() == OmmProviderConfig::InteractiveEnum)
	{
		EmaString temp("Attempt to addMsg() while handle is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else
	{
		RsslErrorInfo rsslErrorInfo;
		RsslRet ret = RSSL_RET_FAILURE;
		bool niProvHandleAdded = false;
		RsslInt32 streamId;
		RsslMsg* rsslMsg = NULL;


		if (!msg._pEncoder)
		{
			reactorReleaseBuffer();

			EmaString temp("Incoming message to pack was null.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		if (_ommIProviderImpl)
		{
			ItemInfo *itemInfo = _ommIProviderImpl->getItemInfo(_itemHandle);

			if (!itemInfo)
			{
				reactorReleaseBuffer();

				EmaString temp("Incorrect handler incoming message.");
				throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
			}

			rsslMsg = msg._pEncoder->getRsslMsg();
			rsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
		else
		{
			OmmNiProviderImpl::StreamInfoPtr* streamInfo = _ommNiProviderImpl->getStreamInfo(_itemHandle);
			rsslMsg = msg._pEncoder->getRsslMsg();

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
					reactorReleaseBuffer();

					EmaString temp("Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )");
					throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
				}
			}
		}

		rsslClearEncodeIterator(&_eIter);
		
		if ((ret = rsslSetEncodeIteratorRWFVersion(&_eIter, msg._pEncoder->getMajVer(), msg._pEncoder->getMinVer())) < RSSL_RET_SUCCESS)
		{
			reactorReleaseBuffer();

			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete *pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}

			EmaString temp("Failed rsslSetEncodeIteratorRWFVersion() with code: ");
			temp.append(ret).append(CR);
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		if ((ret = rsslSetEncodeIteratorBuffer(&_eIter, _packedBuf)) < RSSL_RET_SUCCESS)
		{
			reactorReleaseBuffer();

			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete *pTempStreamInfoPtr;
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
					delete *pTempStreamInfoPtr;
					_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
					_ommNiProviderImpl->returnProviderStreamId(streamId);
				}

				EmaString temp("Failed rsslEncodeBuffer(). Buffer too small. Error code: ");
				temp.append(ret).append(CR);
				throwIueException(temp, OmmInvalidUsageException::BufferTooSmallEnum);
			}
			else
			{
				reactorReleaseBuffer();

				if (niProvHandleAdded)
				{
					OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
					_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete *pTempStreamInfoPtr;
					_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
					_ommNiProviderImpl->returnProviderStreamId(streamId);
				}

				EmaString temp("Failed rsslEncodeBuffer() with code: ");
				temp.append(ret).append(CR);
				throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
			}
		}
	
		_packedBuf->length = rsslGetEncodedBufferLength(&_eIter);

		if ((_packedBuf = rsslReactorPackBuffer(_reactorChannel, _packedBuf, &rsslErrorInfo)) == NULL)
		{
			if (niProvHandleAdded)
			{
				OmmNiProviderImpl::StreamInfoPtr* pTempStreamInfoPtr = _ommNiProviderImpl->_handleToStreamInfo.find(_itemHandle);
				_ommNiProviderImpl->_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete *pTempStreamInfoPtr;
				_ommNiProviderImpl->_handleToStreamInfo.erase(_itemHandle);
				_ommNiProviderImpl->returnProviderStreamId(streamId);
			}
			
			reactorReleaseBuffer();

			EmaString temp;
			OmmInvalidUsageException::ErrorCode errorCode;

			switch (rsslErrorInfo.rsslErrorInfoCode)
			{
			case RSSL_EIC_FAILURE:
				temp.append("Failed to pack buffer during addMsg().");
				errorCode = OmmInvalidUsageException::FailureEnum;
				break;
			case RSSL_EIC_SHUTDOWN:
				temp.append("Failed to pack buffer during addMsg().");
				errorCode = OmmInvalidUsageException::NoActiveChannelEnum;
				break;
			default:
				temp.append("Failed to pack buffer during addMsg().");
				errorCode = OmmInvalidUsageException::FailureEnum;
			}

			temp.append("Msg: ").append(msg.toString()).append(CR)
				.append("RsslChannel: ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
				.append("Error Id: ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError: ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Text: ").append(rsslErrorInfo.rsslError.text);
			throwIueException(temp, errorCode);
		}

		_remainingSize = _packedBuf->length;
		_packedMsgCount++;
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
	reactorReleaseBuffer();
	_remainingSize = 0;
	_packedMsgCount = 0;
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

void PackedMsgImpl::reactorReleaseBuffer()
{
	RsslRet retVal;
	RsslErrorInfo rsslErrorInfo;

	if (_packedBuf  && _packedBuf->data && (retVal = rsslReactorReleaseBuffer(_reactorChannel, _packedBuf, &rsslErrorInfo) < RSSL_RET_SUCCESS))
	{
		EmaString temp("Failed to release Msg buffer in addMsg().");
		temp.append("Return code: ").append((UInt64)retVal).append(CR);
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_packedBuf = NULL;
}
