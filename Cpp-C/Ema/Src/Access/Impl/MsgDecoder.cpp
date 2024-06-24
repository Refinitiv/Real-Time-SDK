/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "Msg.h"
#include "MsgDecoder.h"
#include "StaticDecoder.h"
#include "ExceptionTranslator.h"
#include "rtr/rsslMsgEncoders.h"
#include <stdlib.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

static RsslUInt32 paddingBufferLength = 128; // Padding for StatusMsg headers

MsgDecoder::MsgDecoder() :
 _pRsslDictionary( 0 ),
 _attrib(),
 _payload(),
 _rsslMajVer(RSSL_RWF_MAJOR_VERSION),
 _rsslMinVer(RSSL_RWF_MINOR_VERSION),
 _rsslMsg(),
 _pRsslMsg(0),
 _allocatedMemFlag(UnknownEnum)
{
	rsslClearBuffer(&_copiedBuffer);
}

MsgDecoder::~MsgDecoder()
{
	StaticDecoder::morph( &_payload, DataType::NoDataEnum );
	StaticDecoder::morph( &_attrib, DataType::NoDataEnum );

	if (_copiedBuffer.data)
	{
		free(_copiedBuffer.data);
		rsslClearBuffer(&_copiedBuffer);
	}
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

void MsgDecoder::cloneBufferToMsg(Msg* destMsg, Msg* other, const char* functionName)
{
	if(other->_pDecoder)
	{
		if (other->_pDecoder->getRsslBuffer().length > 0)
		{
			destMsg->_pDecoder->getCopiedBuffer().data = (char*)malloc(size_t(other->_pDecoder->getRsslBuffer().length));

			if (destMsg->_pDecoder->getCopiedBuffer().data == 0)
			{
				EmaString errorText("Failed to copy encoded buffer for ");
				errorText.append(functionName);
				throwMeeException(errorText.c_str());
			}

			destMsg->_pDecoder->_allocatedMemFlag |= MsgDecoder::EncMsgBufferEnum;

			// Copies the original RWF encoded buffer
			destMsg->_pDecoder->getCopiedBuffer().length = other->_pDecoder->getRsslBuffer().length;
			memcpy(destMsg->_pDecoder->getCopiedBuffer().data, other->_pDecoder->getRsslBuffer().data, destMsg->_pDecoder->getCopiedBuffer().length);

			if (destMsg->_pDecoder->setRsslData(other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion(),
				&(destMsg->_pDecoder->getCopiedBuffer()), other->_pDecoder->getRsslDictionary(), (void*)0) == false)
			{
				EmaString errorText("Failed to decode the encoded buffer for ");
				errorText.append(functionName);
				throwIueException( errorText.c_str(), OmmInvalidUsageException::InternalErrorEnum );
			}

			destMsg->_pDecoder->_rsslMsg.msgBase.streamId = other->getStreamId();
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

				destMsg->_pDecoder->getCopiedBuffer().data = (char*)malloc(bufferLength);

				if (destMsg->_pDecoder->getCopiedBuffer().data == 0)
				{
					EmaString errorText("Failed to copy encoded buffer for ");
					errorText.append(functionName);
					throwMeeException(errorText.c_str());
				}

				destMsg->_pDecoder->getCopiedBuffer().length = bufferLength;

				destMsg->_pDecoder->_allocatedMemFlag |= MsgDecoder::EncMsgBufferEnum;

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
				rsslSetEncodeIteratorBuffer(&encodeIter, &destMsg->_pDecoder->getCopiedBuffer());
				rsslSetEncodeIteratorRWFVersion(&encodeIter, other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion());
				if ( ( ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&encodeStatusMsg) ) < RSSL_RET_SUCCESS)
				{
					EmaString errorText("rsslEncodeMsg() failed with reason: ");
					errorText.append(rsslRetCodeToString(ret)).append(" for ");
					errorText.append(functionName);
					throwIueException( errorText.c_str(), ret );
				}

				destMsg->_pDecoder->getCopiedBuffer().length = rsslGetEncodedBufferLength(&encodeIter);

				if (destMsg->_pDecoder->setRsslData(other->_pDecoder->getMajorVersion(), other->_pDecoder->getMinorVersion(),
					&(destMsg->_pDecoder->getCopiedBuffer()), other->_pDecoder->getRsslDictionary(), (void*)0) == false)
				{
					EmaString errorText("Failed to decode the encoded buffer for ");
					errorText.append(functionName);
					throwIueException( errorText.c_str(), OmmInvalidUsageException::InternalErrorEnum );
				}

				destMsg->_pDecoder->_rsslMsg.msgBase.streamId = other->getStreamId();
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

void MsgDecoder::deallocateCopiedBuffer(Msg* msg)
{
	if (msg->_pDecoder->_allocatedMemFlag & MsgDecoder::EncMsgBufferEnum)
	{
		if (msg->_pDecoder->getCopiedBuffer().data)
		{
			free(msg->_pDecoder->getCopiedBuffer().data);
			rsslClearBuffer(&msg->_pDecoder->getCopiedBuffer());
		}
	}

	if (msg->_pDecoder->_allocatedMemFlag & MsgDecoder::NameEnum)
	{
		RsslMsgKey& rsslMsgKey = msg->_pDecoder->_rsslMsg.msgBase.msgKey;
		if (rsslMsgKey.name.data)
		{
			free(rsslMsgKey.name.data);
			rsslClearBuffer(&rsslMsgKey.name);
		}
	}

	if (msg->_pDecoder->_allocatedMemFlag & MsgDecoder::EncAttribEnum)
	{
		RsslMsgKey& rsslMsgKey = msg->_pDecoder->_rsslMsg.msgBase.msgKey;
		if (rsslMsgKey.encAttrib.data)
		{
			free(rsslMsgKey.encAttrib.data);
			rsslClearBuffer(&rsslMsgKey.encAttrib);
		}
	}

	msg->_pDecoder->_allocatedMemFlag = MsgDecoder::UnknownEnum;
}

void MsgDecoder::cloneMsgKey(const Msg& other, RsslMsgKey* destMsgKey, RsslUInt16* destMsgKeyFlag, const char* functionName)
{
	*destMsgKeyFlag |= RSSL_UPMF_HAS_MSG_KEY;

	if (other.hasName())
	{
		free(destMsgKey->name.data);
		destMsgKey->name.data = (char*)malloc(other.getName().length());

		if (destMsgKey->name.data == 0)
		{
			EmaString errorText("Failed to allocate memory for storing message's name in ");
			errorText.append(functionName);
			throwMeeException(errorText.c_str());
		}

		_allocatedMemFlag |= MsgDecoder::NameEnum;

		memcpy(destMsgKey->name.data, other.getName().c_str(), other.getName().length());
		destMsgKey->name.length = other.getName().length();

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

		free(destMsgKey->encAttrib.data);
		destMsgKey->encAttrib.data = (char*)malloc(rsslBuffer.length);

		if (destMsgKey->encAttrib.data == 0)
		{
			EmaString errorText("Failed to allocate memory for storing message's attrib in ");
			errorText.append(functionName);
			throwMeeException(errorText.c_str());
		}

		_allocatedMemFlag |= MsgDecoder::EncAttribEnum;

		memcpy(destMsgKey->encAttrib.data, rsslBuffer.data, rsslBuffer.length);
		destMsgKey->encAttrib.length = rsslBuffer.length;

		destMsgKey->flags |= RSSL_MKF_HAS_ATTRIB;

		if (other.getAttrib().getDataType() < DataType::MsgEnum)
		{
			destMsgKey->attribContainerType = (RsslUInt8)DataType::MsgEnum;
		}
		else
		{
			destMsgKey->attribContainerType = (RsslUInt8)other.getAttrib().getDataType();
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
