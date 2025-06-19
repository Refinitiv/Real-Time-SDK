/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryConfig.h"

using namespace refinitiv::ema::access;

ServiceEndpointDiscoveryConfig::ServiceEndpointDiscoveryConfig() :
	_tokenServiceURL_V1(),
	_tokenServiceURL_V2(),
	_serviceDiscoveryURL(),
	_restLogOutputStreamFile(),
	_restEnableLog(false),
	_restVerboseMode(false),
	_libsslName(),
	_libcryptoName(),
	_libcurlName(),
	_shouldInitializeCPUIDlib(true)
{
}

void ServiceEndpointDiscoveryConfig::clear()
{
	tokenServiceUrlV1();
	tokenServiceUrlV1();
	serviceDiscoveryUrl();
	restLogOutputStream();

	_restEnableLog = false;
	_restVerboseMode = false;

	libSslName();
	libCryptoName();
	libCurlName();

	_shouldInitializeCPUIDlib = true;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::tokenServiceUrlV1(const EmaString& tokenServiceUrl)
{
	_tokenServiceURL_V1 = &tokenServiceUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::tokenServiceUrlV2(const EmaString& tokenServiceUrl)
{
	_tokenServiceURL_V2 = &tokenServiceUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::serviceDiscoveryUrl(const EmaString& serviceDiscoveryUrl)
{
	_serviceDiscoveryURL = &serviceDiscoveryUrl;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::restLogOutputStream(FILE* restLogOutputStream)
{
	_restLogOutputStreamFile = restLogOutputStream;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::restEnableLog(bool restEnableLog)
{
	_restEnableLog = restEnableLog;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::restVerboseMode(bool restVerboseMode)
{
	_restVerboseMode = restVerboseMode;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::libSslName(const EmaString& libsslName)
{
	_libsslName = &libsslName;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::libCryptoName(const EmaString& libcryptoName)
{
	_libcryptoName = &libcryptoName;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::libCurlName(const EmaString& libcurlName)
{
	_libcurlName = &libcurlName;
	return *this;
}

ServiceEndpointDiscoveryConfig& ServiceEndpointDiscoveryConfig::shouldInitializeCPUIDlib(bool shouldInitializeCPUIDlib)
{
	_shouldInitializeCPUIDlib = shouldInitializeCPUIDlib;
	return *this;
}
