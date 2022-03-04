/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscovery.h"
#include "ServiceEndpointDiscoveryClient.h"
#include "ServiceEndpointDiscoveryOption.h"
#include "ServiceEndpointDiscoveryImpl.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

ServiceEndpointDiscovery::ServiceEndpointDiscovery()
	: _pImpl(0)
{
	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, NULL, NULL, NULL);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1)
	: _pImpl(0)
{
	const EmaString* pTempTokenURLV1;

	if (tokenServiceURLV1.empty())
		pTempTokenURLV1 = NULL;
	else
		pTempTokenURLV1 = &tokenServiceURLV1;

	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, pTempTokenURLV1, NULL, NULL);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& serviceDiscoveryURL)
: _pImpl(0)
{
	const EmaString* pTempTokenURLV1;
	const EmaString* pTempDiscoveryURL;

	if (tokenServiceURLV1.empty())
		pTempTokenURLV1 = NULL;
	else
		pTempTokenURLV1 = &tokenServiceURLV1;

	if (serviceDiscoveryURL.empty())
		pTempDiscoveryURL = NULL;
	else
		pTempDiscoveryURL = &serviceDiscoveryURL;

	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, pTempTokenURLV1, NULL, pTempDiscoveryURL);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
	}
}

ServiceEndpointDiscovery::ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& tokenServiceURLV2, const EmaString& serviceDiscoveryURL)
	: _pImpl(0)
{
	const EmaString* pTempTokenURLV1;
	const EmaString* pTempTokenURLV2;
	const EmaString* pTempDiscoveryURL;

	if (tokenServiceURLV1.empty())
		pTempTokenURLV1 = NULL;
	else
		pTempTokenURLV1 = &tokenServiceURLV1;

	if (tokenServiceURLV2.empty())
		pTempTokenURLV2 = NULL;
	else
		pTempTokenURLV2 = &tokenServiceURLV2;

	if (serviceDiscoveryURL.empty())
		pTempDiscoveryURL = NULL;
	else
		pTempDiscoveryURL = &serviceDiscoveryURL;

	try
	{
		_pImpl = new ServiceEndpointDiscoveryImpl(this, pTempTokenURLV1, pTempTokenURLV2, pTempDiscoveryURL);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for ServiceEndpointDiscoveryImpl in ServiceEndpointDiscovery( const EmaString&, const EmaString& ).");
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
