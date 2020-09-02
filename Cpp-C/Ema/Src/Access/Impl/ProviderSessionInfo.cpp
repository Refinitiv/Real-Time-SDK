/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "ProviderSessionInfo.h"
#include "OmmProvider.h"

using namespace rtsdk::ema::access;

ProviderSessionInfo::ProviderSessionInfo() :
	_handle(0),
	_clientHandle(0),
	_provider(0)
{
}

ProviderSessionInfo::~ProviderSessionInfo()
{
}

UInt64 ProviderSessionInfo::getHandle() const
{
	return _handle;
}

OmmProvider& ProviderSessionInfo::getProvider() const
{
	return *_provider;
}

UInt64 ProviderSessionInfo::getClientHandle() const
{
	return _clientHandle;
}

const ChannelInformation& ProviderSessionInfo::getChannelInformation() const
{
	return _channelInfo;
}
