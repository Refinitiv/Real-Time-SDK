/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Msg.h"
#include "OmmBuffer.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"
#include "MsgDecoder.h"
#include "MsgEncoder.h"

#include "Utilities.h"

using namespace refinitiv::ema::access;

Msg::Msg() :
 _pDecoder( 0 ),
 _pEncoder( 0 ),
 _attrib(),
 _payload()
{
}

Msg::~Msg()
{
}

bool Msg::hasMsgKey() const
{
	return _pDecoder->hasMsgKey();
}

bool Msg::hasName() const
{
	return _pDecoder->hasName();
}

bool Msg::hasNameType() const
{
	return _pDecoder->hasNameType();
}

bool Msg::hasServiceId() const
{
	return _pDecoder->hasServiceId();
}

bool Msg::hasId() const
{
	return _pDecoder->hasId();
}

bool Msg::hasFilter() const
{
	return _pDecoder->hasFilter();
}

bool Msg::hasExtendedHeader() const
{
	return _pDecoder->hasExtendedHeader();
}

Int32 Msg::getStreamId() const
{
	return _pDecoder->getStreamId();
}

UInt16 Msg::getDomainType() const
{
	return _pDecoder->getDomainType();
}

const EmaString& Msg::getName() const
{
	return _pDecoder->getName();
}

UInt8 Msg::getNameType() const
{
	return _pDecoder->getNameType();
}

UInt32 Msg::getServiceId() const
{
	return _pDecoder->getServiceId();
}

Int32 Msg::getId() const
{
	return _pDecoder->getId();
}

UInt32 Msg::getFilter() const
{
	return _pDecoder->getFilter();
}

const EmaBuffer& Msg::getExtendedHeader() const
{
	return _pDecoder->getExtendedHeader();
}

const Encoder& Msg::getEncoder() const
{
	return *_pEncoder;
}

const Attrib& Msg::getAttrib() const
{
	return _attrib;
}

const Payload& Msg::getPayload() const
{
	return _payload;
}

void Msg::setDecoder( MsgDecoder* pDecoder )
{
	_pDecoder = pDecoder;
	_payload._pPayload = &pDecoder->getPayloadData();
	_attrib._pAttrib = &pDecoder->getAttribData();
}

bool Msg::hasDecoder() const
{
	return _pDecoder ? true : false;
}

bool Msg::hasEncoder() const
{
	return _pEncoder ? true : false;
}
