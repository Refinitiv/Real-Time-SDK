/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2023 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
*/

#include "PackedMsg.h"
#include "PackedMsgImpl.h"

using namespace refinitiv::ema::access;

PackedMsg::PackedMsg(OmmProvider& ommProvider)
	:_pImpl(0) 
{
	try {
		_pImpl = new PackedMsgImpl(&ommProvider);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for PackedMsgImpl in PackedMsg().");
	}
}


PackedMsg::~PackedMsg()
{
	if (_pImpl)
		delete _pImpl;
}

PackedMsg& PackedMsg::initBuffer()
{
	_pImpl->initBuffer();
	return *this;
}

PackedMsg& PackedMsg::initBuffer(UInt32 maxSize)
{
	_pImpl->initBuffer(maxSize);
	return *this;
}

PackedMsg& PackedMsg::initBuffer(UInt64 clientHandle)
{
	_pImpl->initBuffer(clientHandle);
	return *this;
}

PackedMsg& PackedMsg::initBuffer(UInt64 clinetHandle, UInt32 maxSize)
{
	_pImpl->initBuffer(clinetHandle, maxSize);
	return *this;
}

PackedMsg& PackedMsg::addMsg(const Msg& msg, UInt64 itemHandle)
{
	_pImpl->addMsg(msg, itemHandle);
	return *this;
}

UInt64 PackedMsg::remainingSize() const
{
	return _pImpl->remainingSize();
}

UInt64 PackedMsg::maxSize() const
{
	return _pImpl->maxSize();
}

UInt64 PackedMsg::packedMsgCount() const
{
	return _pImpl->packedMsgCount();
}

PackedMsg& PackedMsg::clear() 
{
	_pImpl->clear();
	return *this;
}
