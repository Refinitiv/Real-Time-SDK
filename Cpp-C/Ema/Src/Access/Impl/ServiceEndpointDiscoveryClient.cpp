/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryClient.h"

using namespace thomsonreuters::ema::access;

ServiceEndpointDiscoveryClient::ServiceEndpointDiscoveryClient()
{
}

ServiceEndpointDiscoveryClient::~ServiceEndpointDiscoveryClient()
{
}

void ServiceEndpointDiscoveryClient::onSuccess(const ServiceEndpointDiscoveryResp& serviceEndpointResp, const ServiceEndpointDiscoveryEvent& event)
{

}

void ServiceEndpointDiscoveryClient::onError(const EmaString& statusText, const ServiceEndpointDiscoveryEvent& event)
{

}
