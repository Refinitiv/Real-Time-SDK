/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ServiceEndpointDiscoveryImpl_h
#define __refinitiv_ema_access_ServiceEndpointDiscoveryImpl_h

#include "Access/Include/EmaString.h"
#include "Access/Include/ServiceEndpointDiscoveryClient.h"
#include "Access/Include/ServiceEndpointDiscoveryOption.h"
#include "Access/Include/ServiceEndpointDiscoveryResp.h"
#include "Access/Include/ServiceEndpointDiscoveryInfo.h"
#include "Access/Include/ServiceEndpointDiscoveryEvent.h"
#include "rtr/rsslReactor.h"
#include "Mutex.h"

namespace rtsdk {

namespace ema {

namespace access {

class ServiceEndpointDiscovery;

class ServiceEndpointDiscoveryImpl
{
public:

	ServiceEndpointDiscoveryImpl(ServiceEndpointDiscovery *pServiceEndpointDiscovery, const EmaString& tokenServiceURL, const EmaString& serviceDiscoveryURL);

	void registerClient(const ServiceEndpointDiscoveryOption& params, ServiceEndpointDiscoveryClient& client, void *closure);

	virtual ~ServiceEndpointDiscoveryImpl();

	static RsslReactorCallbackRet serviceEndpointEventCallback(RsslReactor*, RsslReactorServiceEndpointEvent*);

private:
	RsslReactor *_pReactor;
	RsslReactorServiceDiscoveryOptions _serviceDiscoveryOpts;
	ServiceEndpointDiscoveryClient *_pClient;
	ServiceEndpointDiscoveryResp _serviceDiscoveryResp;
	ServiceEndpointDiscoveryEvent _serviceDiscoveryEvent;
	ServiceEndpointDiscovery	*_pServiceEndpointDiscovery;

	Mutex		_userLock;
};

}

}

}

#endif // __refinitiv_ema_access_ServiceEndpointDiscoveryImpl_h
