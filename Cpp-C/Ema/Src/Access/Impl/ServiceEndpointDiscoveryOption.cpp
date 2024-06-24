/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryOption.h"

using namespace refinitiv::ema::access;

ServiceEndpointDiscoveryOption::ServiceEndpointDiscoveryOption()
{
	clear();
}

ServiceEndpointDiscoveryOption::~ServiceEndpointDiscoveryOption()
{
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::clear()
{
	_username.clear();
	_password.clear();
	_clientId.clear();
	_clientSecret.clear();
	_audience.clear();
	_tokenScope.clear();
	_proxyHostName.clear();
	_proxyPort.clear();
	_proxyUserName.clear();
	_proxyPassword.clear();
	_proxyDomain.clear();
	_transport = UnknownTransportEnum;
	_dataFormat = UnknownDataFormatEnum;
	_takeExclusiveSignOnControl = true;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::username(const EmaString& username)
{
	_username = username;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::password(const EmaString& password)
{
	_password = password;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::clientId(const EmaString& clientId)
{
	_clientId = clientId;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::clientSecret(const EmaString& clientSecret)
{
	_clientSecret = clientSecret;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::clientJWK(const EmaString& clientJWK)
{
	_clientJWK = clientJWK;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::audience(const EmaString& audience)
{
	_audience = audience;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::tokenScope(const EmaString& tokenScope)
{
	_tokenScope = tokenScope;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::takeExclusiveSignOnControl( bool takeExclusiveSignOnControl )
{
	_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::transprot(TransportProtocol transport)
{
	return this->transport(transport);
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::transport(TransportProtocol transport)
{
	_transport = transport;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::dataFormat(DataformatProtocol dataFormat)
{
	_dataFormat = dataFormat;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::proxyHostName(const EmaString& proxyHostName)
{
	_proxyHostName = proxyHostName;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::proxyPort(const EmaString& proxyPort)
{
	_proxyPort = proxyPort;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::proxyUserName(const EmaString& proxyUserName)
{
	_proxyUserName = proxyUserName;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::proxyPassword(const EmaString& proxyPassword)
{
	_proxyPassword = proxyPassword;
	return *this;
}

ServiceEndpointDiscoveryOption& ServiceEndpointDiscoveryOption::proxyDomain(const EmaString& proxyDomain)
{
	_proxyDomain = proxyDomain;
	return *this;
}
