/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Msg.h"
#include "MsgDecoder.h"
#include "StaticDecoder.h"
#include "StatusMsgDecoder.h"
#include "ExceptionTranslator.h"
#include "rtr/rsslMsgEncoders.h"
#include <stdlib.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

static RsslUInt32 paddingBufferLength = 128; // Padding for StatusMsg headers

MsgDecoder::MsgDecoder() :
 _pRsslDictionary(nullptr),
 _attrib(),
 _payload(),
 _rsslMajVer(RSSL_RWF_MAJOR_VERSION),
 _rsslMinVer(RSSL_RWF_MINOR_VERSION),
 _rsslMsg(),
 _pRsslMsg(nullptr),
 _nameData(),
 _copiedBufferData(),
 _encAttribData()
{
	rsslClearBuffer(&_copiedBuffer);
}

MsgDecoder::~MsgDecoder()
{
	StaticDecoder::morph( &_payload, DataType::NoDataEnum );
	StaticDecoder::morph( &_attrib, DataType::NoDataEnum );

	rsslClearBuffer(&_copiedBuffer);
}

void MsgDecoder::setAtExit()
{
}

const Data& MsgDecoder::getAttribData() const
{
	return _attrib;
}

const Data& MsgDecoder::getPayloadData() const
{
	return _payload;
}

const RsslDataDictionary* MsgDecoder::getRsslDictionary()
{
	return _pRsslDictionary;
}

UInt8 MsgDecoder::getMajorVersion()
{
	return _rsslMajVer;
}

UInt8 MsgDecoder::getMinorVersion()
{
	return _rsslMinVer;
}

RsslBuffer& MsgDecoder::getCopiedBuffer()
{
	return _copiedBuffer;
}

void MsgDecoder::cloneBufferToMsg(Msg* other, const char* functionName)
{
	if(other->_pDecoder)
	{
		if (other->_pDecoder->getRsslBuffer().length > 0)
		{
			// Copies the original RWF encoded buffer
			_copiedBufferData.setFrom(other->_pDecoder->getRsslBuffer().data, other->_pDecoder->getRsslBuffer().length);

			getCopiedBuffer().data = const_cast<char*>(_copiedBufferData.c_buf());
			getCopiedBuffer().length = _copiedBufferData.length();

			if (setRsslData(other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion(),
				&(getCopiedBuffer()), other->_pDecoder->getRsslDictionary(), (void*)0) == false)
			{
				EmaString errorText("Failed to decode the encoded buffer for ");
				errorText.append(functionName);
				throwIueException( errorText.c_str(), OmmInvalidUsageException::InternalErrorEnum );
			}

			_rsslMsg.msgBase.streamId = other->getStreamId();

			// Copy potential modifications by ItemCallbackClient::processStatusMsg
			if (other->getDataType() == DataType::StatusMsgEnum)
			{
				if (static_cast<StatusMsgDecoder*>(other->_pDecoder)->_pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
				{
					static_cast<StatusMsgDecoder*>(this)->_pRsslMsg->statusMsg.flags |= RSSL_STMF_HAS_STATE;
					static_cast<StatusMsgDecoder*>(this)->_pRsslMsg->statusMsg.state.code = static_cast<StatusMsgDecoder*>(other->_pDecoder)->_pRsslMsg->statusMsg.state.code;
					static_cast<StatusMsgDecoder*>(this)->_pRsslMsg->statusMsg.state.dataState = static_cast<StatusMsgDecoder*>(other->_pDecoder)->_pRsslMsg->statusMsg.state.dataState;
					static_cast<StatusMsgDecoder*>(this)->_pRsslMsg->statusMsg.state.streamState = static_cast<StatusMsgDecoder*>(other->_pDecoder)->_pRsslMsg->statusMsg.state.streamState;
				}
			}
		}
		else
		{
			// The status message is generated internally by EMA
			if (other->_pDecoder->_pRsslMsg->msgBase.msgClass == RSSL_MC_STATUS)
			{
				int ret = RSSL_RET_SUCCESS;
				RsslStatusMsg& statusMsg = other->_pDecoder->_pRsslMsg->statusMsg;
				RsslStatusMsg encodeStatusMsg = RSSL_INIT_STATUS_MSG;
				RsslEncodeIterator encodeIter;
				RsslUInt32 bufferLength =
					statusMsg.msgBase.msgKey.name.length +
					statusMsg.state.text.length + paddingBufferLength;

				_copiedBufferData = EmaBuffer(nullptr, bufferLength);

				getCopiedBuffer().data = const_cast<char*>(_copiedBufferData.c_buf());
				getCopiedBuffer().length = _copiedBufferData.length();

				/* clear encode iterator */
				rsslClearEncodeIterator(&encodeIter);

				encodeStatusMsg.msgBase.msgClass = RSSL_MC_STATUS;
				encodeStatusMsg.msgBase.domainType = statusMsg.msgBase.domainType;
				encodeStatusMsg.msgBase.containerType = statusMsg.msgBase.containerType;
				encodeStatusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
				encodeStatusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
				encodeStatusMsg.msgBase.msgKey.name = statusMsg.msgBase.msgKey.name;
				encodeStatusMsg.state.code = statusMsg.state.code;
				encodeStatusMsg.state.dataState = statusMsg.state.dataState;
				encodeStatusMsg.state.streamState = statusMsg.state.streamState;
				encodeStatusMsg.state.text = statusMsg.state.text;
				
				/* encode message */
				rsslSetEncodeIteratorBuffer(&encodeIter, &getCopiedBuffer());
				rsslSetEncodeIteratorRWFVersion(&encodeIter, other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion());
				if ( ( ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&encodeStatusMsg) ) < RSSL_RET_SUCCESS)
				{
					EmaString errorText("rsslEncodeMsg() failed with reason: ");
					errorText.append(rsslRetCodeToString(ret)).append(" for ");
					errorText.append(functionName);
					throwIueException( errorText.c_str(), ret );
				}

				getCopiedBuffer().length = rsslGetEncodedBufferLength(&encodeIter);

				if (setRsslData(other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion(),
					&(getCopiedBuffer()), other->_pDecoder->getRsslDictionary(), (void*)0) == false)
				{
					EmaString errorText("Failed to decode the encoded buffer for ");
					errorText.append(functionName);
					throwIueException( errorText.c_str(), OmmInvalidUsageException::InternalErrorEnum );
				}

				_rsslMsg.msgBase.streamId = other->getStreamId();
			}
			else
			{
				EmaString errorText("Failed to clone empty encoded buffer for ");
				errorText.append(functionName);
				throwIueException( errorText.c_str(), OmmInvalidUsageException::InternalErrorEnum );
			}
		}
	}
	else if (other->_pEncoder)
	{
		EmaString errorText(functionName);
		errorText.append(" does not support passing in just encoded message");
		throwIueException( errorText.c_str(), OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void MsgDecoder::deallocateCopiedBuffer()
{
	RsslMsgKey& rsslMsgKey = _rsslMsg.msgBase.msgKey;

	if (_nameData.c_buf() == rsslMsgKey.name.data)
		rsslClearBuffer(&rsslMsgKey.name);
	_nameData.release();

	if (_encAttribData.c_buf() == rsslMsgKey.encAttrib.data)
		rsslClearBuffer(&rsslMsgKey.encAttrib);
	_encAttribData.release();

	if (_copiedBufferData.c_buf() == _copiedBuffer.data)
		rsslClearBuffer(&_copiedBuffer);
	_copiedBufferData.release();
}

void MsgDecoder::cloneMsgKey(const Msg& other, RsslMsgKey* destMsgKey)
{
	if (other.hasName())
	{
		_nameData.setFrom(other.getName().c_str(), other.getName().length());

		destMsgKey->name.data = const_cast<char*>(_nameData.c_buf());
		destMsgKey->name.length = _nameData.length();

		destMsgKey->flags |= RSSL_MKF_HAS_NAME;
	}

	if (other.hasNameType())
	{
		destMsgKey->nameType = other.getNameType();
		destMsgKey->flags |= RSSL_MKF_HAS_NAME_TYPE;
	}

	if (other.hasServiceId())
	{
		destMsgKey->serviceId = other.getServiceId();
		destMsgKey->flags |= RSSL_MKF_HAS_SERVICE_ID;
	}

	if (other.hasId())
	{
		destMsgKey->identifier = other.getId();
		destMsgKey->flags |= RSSL_MKF_HAS_IDENTIFIER;
	}

	if (other.hasFilter())
	{
		destMsgKey->filter = other.getFilter();
		destMsgKey->flags |= RSSL_MKF_HAS_FILTER;
	}

	if (other.getAttrib().getDataType() != DataType::NoDataEnum)
	{
		const RsslBuffer& rsslBuffer = const_cast<ComplexType&>(other.getAttrib().getData()).getDecoder().getRsslBuffer();

		_encAttribData.setFrom(rsslBuffer.data, rsslBuffer.length);

		destMsgKey->encAttrib.data = const_cast<char*>(_encAttribData.c_buf());
		destMsgKey->encAttrib.length = _encAttribData.length();

		destMsgKey->flags |= RSSL_MKF_HAS_ATTRIB;

		if (other.getAttrib().getDataType() < DataType::MsgEnum)
		{
			destMsgKey->attribContainerType = (RsslUInt8)other.getAttrib().getDataType();
		}
		else
		{
			destMsgKey->attribContainerType = (RsslUInt8)DataType::MsgEnum;
		}

		StaticDecoder::setRsslData(&_attrib, &destMsgKey->encAttrib,
			destMsgKey->attribContainerType, _rsslMajVer, _rsslMinVer, _pRsslDictionary);
	}
	else
	{
		StaticDecoder::setRsslData(&_attrib, &destMsgKey->encAttrib,
			RSSL_DT_NO_DATA , _rsslMajVer, _rsslMinVer, _pRsslDictionary);
	}
}
