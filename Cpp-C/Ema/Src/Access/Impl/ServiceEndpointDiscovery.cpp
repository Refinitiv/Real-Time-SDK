/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscovery.h"
#include "ServiceEndpointDiscoveryClient.h"
#include "ServiceEndpointDiscoveryOption.h"
#include "ServiceEndpointDiscoveryImpl.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURL, const EmaString& serviceDiscoveryURL)
: _pImpl(0)
{
	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, tokenServiceURL, serviceDiscoveryURL);
	}
	catch (std::bad_alloc&) {}

	if (!_pImpl)
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
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
