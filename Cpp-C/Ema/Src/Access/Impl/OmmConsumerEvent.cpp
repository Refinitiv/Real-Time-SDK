/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerEvent.h"
#include "ItemCallbackClient.h"
#include "ActiveConfig.h"
#include "ChannelInfoImpl.h"

using namespace thomsonreuters::ema::access;

OmmConsumerEvent::OmmConsumerEvent() :
	_handle( 0 ),
	_closure( 0 ),
	_parentHandle( 0 ),
	_channel( 0 )
{
}

OmmConsumerEvent::~OmmConsumerEvent()
{
}

UInt64 OmmConsumerEvent::getHandle() const
{
	return _handle;
}

void* OmmConsumerEvent::getClosure() const
{
	return _closure;
}

UInt64 OmmConsumerEvent::getParentHandle() const
{
	return _parentHandle;
}

const ChannelInformation& OmmConsumerEvent::getChannelInformation() const 
{
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>( _channel );
	getChannelInformationImpl( rsslReactorChannel, OmmCommonImpl::ConsumerEnum, const_cast<ChannelInformation&>( _channelInfo ) );
	return _channelInfo;
}
