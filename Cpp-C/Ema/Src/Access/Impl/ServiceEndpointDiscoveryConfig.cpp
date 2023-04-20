/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryConfig.h"

using namespace refinitiv::ema::access;

ServiceEndpointDiscoveryConfig::ServiceEndpointDiscoveryConfig() :
	tokenServiceURL_V1(),
	tokenServiceURL_V2(),
	serviceDiscoveryURL(),
	restLogOutputStreamFile(),
	restEnableLogValue(false)
{
}

void ServiceEndpointDiscoveryConfig::clear()
{
	tokenServiceUrlV1();
	tokenServiceUrlV1();
	serviceDiscoveryUrl();
	restLogOutputStream();
	restEnableLogValue = false;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::tokenServiceUrlV1(const EmaString& tokenServiceUrl)
{
	tokenServiceURL_V1 = &tokenServiceUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::tokenServiceUrlV2(const EmaString& tokenServiceUrl)
{
	tokenServiceURL_V2 = &tokenServiceUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::serviceDiscoveryUrl(const EmaString& serviceDiscoveryUrl)
{
	serviceDiscoveryURL = &serviceDiscoveryUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::restLogOutputStream(FILE* restLogOutputStream)
{
	restLogOutputStreamFile = restLogOutputStream;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::restEnableLog(bool restEnableLog)
{
	restEnableLogValue = restEnableLog;
	return *this;
}