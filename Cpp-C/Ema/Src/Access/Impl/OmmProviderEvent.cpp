/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProviderEvent.h"
#include "ItemCallbackClient.h"
#include "OmmProvider.h"
#include "ChannelInfoImpl.h"
#include "ChannelStatsImpl.h"

using namespace rtsdk::ema::access;

OmmProviderEvent::OmmProviderEvent() :
	_handle( 0 ),
	_closure( 0 ),
	_clientHandle( 0 ),
	_provider( 0 ),
	_channel( 0 )
{
}

OmmProviderEvent::~OmmProviderEvent()
{
}

UInt64 OmmProviderEvent::getHandle() const
{
	return _handle;
}

void* OmmProviderEvent::getClosure() const
{
	return _closure;
}

OmmProvider& OmmProviderEvent::getProvider() const
{
	return *_provider;
}

UInt64 OmmProviderEvent::getClientHandle() const
{
	return _clientHandle;
}

/* OmmProviderEvents are different depending upon the OmmProvider responsible for
 * the event.
 *
 * In the case of IProviders, the OmmProviderEvent is an event on an attached client
 * and this function will provide channel info (including the source hostname/ip address)
 * for the client responsible for the event. The channel information is derived from the
 * _channel member in the OmmProviderEvent class which is set in LoginHandler::loginCallback
 * when the client first logs into the IProvider.
 *
 * In the case of NiProviders, the OmmProviderEvent in an event on the connection to the ADH
 * (in most cases) and this function will provide channel info for the connection to the ADH.
 * In this case, the hostname will be the name from the configuration used to create the
 * connection and the ip address will be set to a default message. 
 */
const ChannelInformation& OmmProviderEvent::getChannelInformation() const {
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>( _channel );
	if ( getProvider().getProviderRole() == OmmProviderConfig::InteractiveEnum ) 
		getChannelInformationImpl( rsslReactorChannel, OmmCommonImpl::IProviderEnum, const_cast<ChannelInformation&>( _channelInfo ) );
	else 
		getChannelInformationImpl( rsslReactorChannel, OmmCommonImpl::NiProviderEnum, const_cast<ChannelInformation&>( _channelInfo ) );

	return _channelInfo;
}

const ChannelStatistics& OmmProviderEvent::getChannelStatistics() const {
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>(_channel);
	getChannelStats(rsslReactorChannel, const_cast<ChannelStatistics&>(_channelStats));
	return _channelStats;
}
