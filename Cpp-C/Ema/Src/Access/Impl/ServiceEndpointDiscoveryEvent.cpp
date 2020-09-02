/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include "ServiceEndpointDiscoveryEvent.h"

using namespace rtsdk::ema::access;

ServiceEndpointDiscoveryEvent::ServiceEndpointDiscoveryEvent() :
_pClosure(0)
{
}


ServiceEndpointDiscoveryEvent::~ServiceEndpointDiscoveryEvent()
{
}

void* ServiceEndpointDiscoveryEvent::getClosure() const
{
	return _pClosure;
}

ServiceEndpointDiscovery& ServiceEndpointDiscoveryEvent::getServiceEndpointDiscovery() const
{
	return *_pServiceEndpointDiscovery;
}
