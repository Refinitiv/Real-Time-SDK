/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "Msg.h"
#include "MsgImpl.h"
#include "StaticDecoder.h"
#include "ExceptionTranslator.h"
#include "rtr/rsslMsgEncoders.h"

#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

MsgImpl::MsgImpl() :
 _encoder(this),
 _pRsslMsg(&_rsslMsg),
 _pRsslDictionary(nullptr),
 _rsslMajVer(RSSL_RWF_MAJOR_VERSION),
 _rsslMinVer(RSSL_RWF_MINOR_VERSION),
 _serviceNameSet(false),
 _serviceListNameSet(false),
 _rsslMsg(),
 _attrib(new(_attrib_placeholder)NoDataImpl{}),
 _payload(new(_payload_placeholder)NoDataImpl{}),
 _name(),
 _nameData(),
 _attribData(),
 _payloadData(),
 _hexBuffer(),
 _serviceName(),
 _serviceNameData(),
 _serviceListName()
{
}

MsgImpl::~MsgImpl()
{
	_payload->~Data();
	_attrib->~Data();
}

void MsgImpl::setAtExit()
{
}

const RsslDataDictionary* MsgImpl::getRsslDictionary()
{
	return _pRsslDictionary;
}

UInt8 MsgImpl::getMajorVersion()
{
	return _rsslMajVer;
}

UInt8 MsgImpl::getMinorVersion()
{
	return _rsslMinVer;
}

const EmaBuffer& MsgImpl::getAsHex() const
{
	_hexBuffer.setFromInt( _pRsslMsg->msgBase.encMsgBuffer.data,
						   _pRsslMsg->msgBase.encMsgBuffer.length );

	return _hexBuffer.toBuffer();
}

bool MsgImpl::hasFilter() const
{
	return ( hasMsgKey() && rsslMsgKeyCheckHasFilter( getRsslMsgKey() ) == RSSL_TRUE )
		? true : false;
}

UInt32 MsgImpl::getFilter() const
{
	if ( !hasFilter() )
	{
		EmaString temp("Attempt to getFilter() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslMsgKey()->filter;
}

void MsgImpl::setFilter(UInt32 value)
{
	getRsslMsgKey()->filter = value;

	rsslMsgKeyApplyHasFilter( getRsslMsgKey() );
	applyHasMsgKey();
}

void MsgImpl::addFilter( UInt32 filter )
{
	getRsslMsgKey()->filter |= filter;

	rsslMsgKeyApplyHasFilter( getRsslMsgKey() );
	applyHasMsgKey();
}

const Data& MsgImpl::getAttribData() const
{
	return *_attrib;
}

void MsgImpl::setAttrib(const ComplexType& attrib)
{
	if (attrib.getDataType() == DataType::NoDataEnum)
	{
		getRsslMsgKey()->attribContainerType = RSSL_DT_NO_DATA;
		applyHasNoFlag(getRsslMsgKey(), RSSL_MKF_HAS_ATTRIB);
		return;
	}

	if ( attrib.hasEncoder() && attrib.getEncoder().ownsIterator() )
	{
		const RsslBuffer& rsslBuf = attrib.getEncoder().getEncodedBuffer();
		if (! _arena.trySave(getRsslMsgKey()->encAttrib, rsslBuf))
		{
			_attribData.setFrom(rsslBuf.data, rsslBuf.length);
			getRsslMsgKey()->encAttrib.data = (char*)_attribData.c_buf();
			getRsslMsgKey()->encAttrib.length = _attribData.length();
		}
	}
	else if ( attrib.hasDecoder() )
	{
		const RsslBuffer& rsslBuf = const_cast<ComplexType&>(attrib).getDecoder().getRsslBuffer();
		if (! _arena.trySave(getRsslMsgKey()->encAttrib, rsslBuf))
		{
			_attribData.setFrom(rsslBuf.data, rsslBuf.length);
			getRsslMsgKey()->encAttrib.data = (char*)_attribData.c_buf();
			getRsslMsgKey()->encAttrib.length = _attribData.length();
		}
	}
	else
	{
		EmaString temp("Attempt to pass in an empty ComplexType while it is not supported.");
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	getRsslMsgKey()->attribContainerType = Encoder::convertDataType( attrib.getDataType() );

	// note: this decodes only Data header
	StaticDecoder::setRsslData( _attrib,
								&getRsslMsgKey()->encAttrib,
								getRsslMsgKey()->attribContainerType,
								_rsslMajVer, _rsslMinVer, _pRsslDictionary );

	rsslMsgKeyApplyHasAttrib( getRsslMsgKey() );
	applyHasMsgKey();
}

bool MsgImpl::hasPayload() const
{
	return getRsslMsg()->msgBase.containerType != RSSL_DT_NO_DATA ? true : false;
}

const Data& MsgImpl::getPayloadData() const
{
	return *_payload;
}

void MsgImpl::setPayload(const ComplexType& load)
{
	RsslContainerType payloadDataType = Encoder::convertDataType( load.getDataType() );

	if ( payloadDataType != RSSL_DT_NO_DATA )
	{
		if ( load.hasEncoder() && load.getEncoder().ownsIterator() )
		{
			const RsslBuffer& rsslBuf = load.getEncoder().getEncodedBuffer();
			if (! _arena.trySave(getRsslMsg()->msgBase.encDataBody, rsslBuf))
			{
				_payloadData.setFrom( rsslBuf.data, rsslBuf.length );
				getRsslMsg()->msgBase.encDataBody.data = (char*)_payloadData.c_buf();
				getRsslMsg()->msgBase.encDataBody.length = _payloadData.length();
			}
		}
		else if ( load.hasDecoder() )
		{
			const RsslBuffer& rsslBuf = const_cast<ComplexType&>( load ).getDecoder().getRsslBuffer();
			if (! _arena.trySave(getRsslMsg()->msgBase.encDataBody, rsslBuf))
			{
				_payloadData.setFrom( rsslBuf.data, rsslBuf.length );
				getRsslMsg()->msgBase.encDataBody.data = (char*)_payloadData.c_buf();
				getRsslMsg()->msgBase.encDataBody.length = _payloadData.length();
			}
		}
		else
		{
			EmaString temp( "Attempt to pass in an empty ComplexType while it is not supported." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
	}

	getRsslMsg()->msgBase.containerType = payloadDataType;

	// note: this decodes only Data header
	StaticDecoder::setRsslData( _payload,
								&getRsslMsg()->msgBase.encDataBody,
								getRsslMsg()->msgBase.containerType,
								_rsslMajVer, _rsslMinVer, _pRsslDictionary );

	adjustPayload();
}

Int32 MsgImpl::getStreamId() const
{
	return _pRsslMsg->msgBase.streamId;
}

void MsgImpl::setStreamId(Int32 value)
{
	_pRsslMsg->msgBase.streamId = value;
}

UInt16 MsgImpl::getDomainType() const
{
	return _pRsslMsg->msgBase.domainType;
}

void MsgImpl::setDomainType(UInt16 domainType)
{
	if (domainType > 255)
	{
		EmaString temp("Passed in DomainType is out of range.");
		throwDtuException(domainType, temp);
		return;
	}

	_pRsslMsg->msgBase.domainType = (UInt8)domainType;
}

bool MsgImpl::hasName() const
{
	return ( hasMsgKey() && rsslMsgKeyCheckHasName( getRsslMsgKey() ) == RSSL_TRUE )
		? true : false;
}

const EmaString& MsgImpl::getName() const
{
	if (!hasName())
	{
		EmaString temp("Attempt to getName() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_name.setInt( getRsslMsgKey()->name.data,
				  getRsslMsgKey()->name.length, false );

	return _name.toString();
}


void MsgImpl::setName(const EmaString& name)
{
	if (name.length() == 0)
		return;

	if (! _arena.trySave(getRsslMsgKey()->name, name))
	{
		_nameData = name;

		getRsslMsgKey()->name.data = (char*)_nameData.c_str();
		getRsslMsgKey()->name.length = _nameData.length();
	}

	rsslMsgKeyApplyHasName( getRsslMsgKey() );
	applyHasMsgKey();
}

bool MsgImpl::hasNameType() const
{
	return ( hasMsgKey() && rsslMsgKeyCheckHasNameType( getRsslMsgKey() ) == RSSL_TRUE )
		? true : false;
}

UInt8 MsgImpl::getNameType() const
{
	if (!hasNameType())
	{
		EmaString temp("Attempt to getNameType() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslMsgKey()->nameType;
}

void MsgImpl::setNameType(UInt8 value)
{
	_pRsslMsg->msgBase.msgKey.nameType = value;

	rsslMsgKeyApplyHasNameType( getRsslMsgKey() );
	applyHasMsgKey();
}

bool MsgImpl::hasServiceId() const
{
	return ( hasMsgKey() && rsslMsgKeyCheckHasServiceId( getRsslMsgKey() ) == RSSL_TRUE )
		? true : false;
};

UInt32 MsgImpl::getServiceId() const
{
	if (!hasServiceId())
	{
		EmaString temp("Attempt to getServiceId() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslMsgKey()->serviceId;
}

void MsgImpl::setServiceId(UInt16 value)
{
	getRsslMsgKey()->serviceId = value;

	rsslMsgKeyApplyHasServiceId( getRsslMsgKey() );
	applyHasMsgKey();
}

bool MsgImpl::hasServiceName() const
{
	return _serviceNameSet;
}

void MsgImpl::copyServiceName(const EmaString& serviceName)
{
	_serviceNameSet = serviceName.length() ? true : false;

	_serviceNameData = serviceName;

	_serviceName.setInt(_serviceNameData.c_str(), _serviceNameData.length(), true);
}

const EmaString& MsgImpl::getServiceName() const
{
	if (!_serviceNameSet)
	{
		EmaString temp("Attempt to getServiceName() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return _serviceName.toString();
}

bool MsgImpl::hasId() const
{
	return ( hasMsgKey() && rsslMsgKeyCheckHasIdentifier( getRsslMsgKey() ) == RSSL_TRUE )
		? true : false;
};

Int32 MsgImpl::getId() const
{
	if (!hasId())
	{
		EmaString temp("Attempt to getId() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslMsgKey()->identifier;
}

void MsgImpl::setId(Int32 value)
{
	getRsslMsgKey()->identifier = value;

	rsslMsgKeyApplyHasIdentifier( getRsslMsgKey() );
	applyHasMsgKey();
}

bool MsgImpl::hasServiceListName() const
{
	return _serviceListNameSet;
}

const EmaString& MsgImpl::getServiceListName() const
{
	return _serviceListName;
}

void MsgImpl::setServiceListName(const EmaString& serviceListName)
{
	if (hasServiceId() || hasServiceName())
	{
		EmaString text("Attempt to set serviceListName while service id or service name is already set.");
		throwIueException(text, OmmInvalidUsageException::InvalidOperationEnum);
		return;
	}

	_serviceListNameSet = serviceListName.length() ? true : false;
	_serviceListName = serviceListName;
}

void MsgImpl::copyName(const MsgImpl& other)
{
	if (! other.hasName() || other.getRsslMsgKey()->name.length == 0)
		return;

	if (! memberCopyInPlace( getRsslMsgKey()->name, other.getRsslMsgKey()->name,
							 _pRsslMsg->msgBase.encMsgBuffer, other._pRsslMsg->msgBase.encMsgBuffer )
		&& ! _arena.trySave( getRsslMsgKey()->name, other.getRsslMsgKey()->name ))
	{
		// name is located outside of the encoded message buffer, copy it into the
		// dedicated EmaString
		_nameData = other.getName();

		getRsslMsgKey()->name.data = const_cast<char*>(_nameData.c_str());
		getRsslMsgKey()->name.length = _nameData.length();
	}
}

void MsgImpl::copyAttrib(const MsgImpl& other)
{
	if ( !other.hasAttrib<const MsgImpl>() || other.getRsslMsgKey()->encAttrib.length == 0)
	{
		StaticDecoder::setRsslData(_attrib, &getRsslMsgKey()->encAttrib,
			RSSL_DT_NO_DATA , _rsslMajVer, _rsslMinVer, _pRsslDictionary);
		return;
	}

	if (! memberCopyInPlace( getRsslMsgKey()->encAttrib, other.getRsslMsgKey()->encAttrib,
							 getRsslMsg()->msgBase.encMsgBuffer, other.getRsslMsg()->msgBase.encMsgBuffer )
		&& !_arena.trySave( getRsslMsgKey()->encAttrib, other.getRsslMsgKey()->encAttrib ))
	{
		_attribData.setFrom(other._pRsslMsg->msgBase.msgKey.encAttrib.data,
							other._pRsslMsg->msgBase.msgKey.encAttrib.length);

		getRsslMsgKey()->encAttrib.data = const_cast<char*>(_attribData.c_buf());
		getRsslMsgKey()->encAttrib.length = _attribData.length();
	}

	// note: this decodes only Data header
	StaticDecoder::setRsslData( _attrib,
								&getRsslMsgKey()->encAttrib,
								getRsslMsgKey()->attribContainerType,
								_rsslMajVer, _rsslMinVer, _pRsslDictionary );
}

void MsgImpl::copyPayload(const MsgImpl& other)
{
	if ( !other.hasPayload() || other.getRsslMsg()->msgBase.encDataBody.length == 0)
	{
		_pRsslMsg->msgBase.containerType = RSSL_DT_NO_DATA;
		return;
	}

	if (! memberCopyInPlace( getRsslMsg()->msgBase.encDataBody, other.getRsslMsg()->msgBase.encDataBody,
							 getRsslMsg()->msgBase.encMsgBuffer, other.getRsslMsg()->msgBase.encMsgBuffer)
		&& !_arena.trySave( getRsslMsg()->msgBase.encDataBody, other.getRsslMsg()->msgBase.encDataBody ))
	{
		_payloadData.setFrom(other._pRsslMsg->msgBase.encDataBody.data,
							 other._pRsslMsg->msgBase.encDataBody.length);

		_pRsslMsg->msgBase.encDataBody.data = const_cast<char*>(_payloadData.c_buf());
		_pRsslMsg->msgBase.encDataBody.length = _payloadData.length();
	}

	// note: this decodes only Data header
	StaticDecoder::setRsslData( _payload,
								&_pRsslMsg->msgBase.encDataBody,
								_pRsslMsg->msgBase.containerType,
								_rsslMajVer, _rsslMinVer, _pRsslDictionary );
}

void MsgImpl::copyMsgBaseFrom(const MsgImpl& other, size_t msgSize)
{
	_arena.clear();
	_serviceNameSet = false;
	_serviceListNameSet = false;

	// note: assume that dictionaries are long living objects and need not be copied
	// for each message
	_pRsslDictionary = other._pRsslDictionary;

	// Copy the rssl message struct. It contains decoded message with up-to-date
	// values that could be modified by the API
	_pRsslMsg = (RsslMsg*)memcpy(&_rsslMsg, other.getRsslMsg(), msgSize);

	rsslClearBuffer(&(getRsslMsg()->msgBase.msgKey.name));
	rsslClearBuffer(&(getRsslMsg()->msgBase.msgKey.encAttrib));
	rsslClearBuffer(&(getRsslMsg()->msgBase.encDataBody));

	// Copy the backing buffer containing encoded message data
	if (other.getRsslMsg()->msgBase.encMsgBuffer.data != nullptr)
	{
		_arena.reserve(other.getRsslMsg()->msgBase.encMsgBuffer.length);

		if (! _arena.trySave(getRsslMsg()->msgBase.encMsgBuffer, other.getRsslMsg()->msgBase.encMsgBuffer))
		{
			throwMeeException("Failed to copy encoded message buffer");
		}
	}

	if (other.hasServiceName())
		copyServiceName(other.getServiceName());

	// adjust pointers or copy buffers for "reference" type message fields
	if (other.hasMsgKey())
	{
		copyName(other);
		copyAttrib(other);
	}

	copyPayload(other);
}

void MsgImpl::clearBaseMsgImpl()
{
	_pRsslMsg = &_rsslMsg;

	_pRsslDictionary = nullptr;

	_arena.clear();

	// no need to "clear" buffers as the values in RsslMsg define if the field is present

	_serviceNameSet = false;
	_serviceListNameSet = false;
}
