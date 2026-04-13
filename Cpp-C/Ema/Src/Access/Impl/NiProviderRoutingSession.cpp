/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
#include "NiProviderRoutingSession.h"
#include "ConsumerRoutingChannel.h"
#include "OmmBaseImpl.h"

using namespace refinitiv::ema::access;

NiProviderRoutingSession::NiProviderRoutingSession(OmmBaseImpl& consumerBaseImpl) :
	BaseRoutingSession(consumerBaseImpl)
{
	initialLoginRefreshReceived = false;
	activeChannelCount = 0;
	sessionType = RoutingSessionType::NI_PROVIDER;
}

NiProviderRoutingSession::~NiProviderRoutingSession()
{
	clear();
}

void NiProviderRoutingSession::clear()
{
	BaseRoutingSession::clear();
}

