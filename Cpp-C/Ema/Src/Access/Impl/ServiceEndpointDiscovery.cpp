/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscovery.h"
#include "ServiceEndpointDiscoveryClient.h"
#include "ServiceEndpointDiscoveryOption.h"
#include "ServiceEndpointDiscoveryImpl.h"
#include "ServiceEndpointDiscoveryConfig.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

ServiceEndpointDiscovery::ServiceEndpointDiscovery()
	: _pImpl(0)
{
	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, NULL);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery().");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const ServiceEndpointDiscoveryConfig& serviceEndpointDiscoveryConfig)
	: _pImpl(0)
{
	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, &serviceEndpointDiscoveryConfig);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const ServiceEndpointDiscoveryConfig& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1)
	: _pImpl(0)
{
	ServiceEndpointDiscoveryConfig config;
	config.tokenServiceUrlV1(tokenServiceURLV1);

	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, &config);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& serviceDiscoveryURL)
: _pImpl(0)
{
	ServiceEndpointDiscoveryConfig config;
	config.tokenServiceUrlV1(tokenServiceURLV1);
	config.serviceDiscoveryUrl(serviceDiscoveryURL);

	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, &config);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& tokenServiceURLV2, const EmaString& serviceDiscoveryURL)
	: _pImpl(0)
{
	ServiceEndpointDiscoveryConfig config;
	config.tokenServiceUrlV1(tokenServiceURLV1);
	config.tokenServiceUrlV2(tokenServiceURLV2);
	config.serviceDiscoveryUrl(serviceDiscoveryURL);


	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, &config);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString&, const EmaString& ).");
	}
}

ServiceEndpointDiscovery::~ServiceEndpointDiscovery()
{
	if (_pImpl)
	{
		delete _pImpl;
		_pImpl = 0;
	}
}

void ServiceEndpointDiscovery::registerClient(const ServiceEndpointDiscoveryOption& params, ServiceEndpointDiscoveryClient& client,void *closure)
{
	_pImpl->registerClient(params, client, closure);
}
