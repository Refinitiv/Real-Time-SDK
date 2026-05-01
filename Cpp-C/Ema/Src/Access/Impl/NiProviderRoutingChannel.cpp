/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProviderRoutingChannel.h"
#include "NiProviderRoutingSession.h"



using namespace refinitiv::ema::access;
 
// Default values are taken directly from ActiveConfig.h
NiProviderRoutingSessionChannelConfig::NiProviderRoutingSessionChannelConfig(const EmaString& channelName, ActiveConfig& activeConfig) :
	BaseRoutingSessionChannelConfig(channelName, activeConfig)
{
	pRoutingChannel = NULL;

	rsslClearReactorConnectOptions(&connectOpts);

	useActiveConfigLogger = true;
}

NiProviderRoutingSessionChannelConfig::~NiProviderRoutingSessionChannelConfig()
{
	clear();
}

void NiProviderRoutingSessionChannelConfig::clear()
{
	BaseRoutingSessionChannelConfig::clear();
}

NiProviderRoutingSessionChannel::NiProviderRoutingSessionChannel(OmmBaseImpl& consumerBaseImpl, const EmaString& sessionChannelName, NiProviderRoutingSessionChannelConfig& sessionChannelConfig) :
	BaseRoutingSessionChannel(consumerBaseImpl, sessionChannelName, static_cast<BaseRoutingSessionChannelConfig&>(sessionChannelConfig)),
	routingChannelConfig(sessionChannelConfig),
	_transportBuffer(NULL)
{
}

NiProviderRoutingSessionChannel::~NiProviderRoutingSessionChannel()
{
	clear();
}

// Inherits from BaseRoutingSessionchannel::clear()
void NiProviderRoutingSessionChannel::clear()
{
	BaseRoutingSessionChannel::clear();
	pRoutingSession = NULL;
}
