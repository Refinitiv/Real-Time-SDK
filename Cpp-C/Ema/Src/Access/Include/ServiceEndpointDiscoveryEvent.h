/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ServiceEndpointDiscoveryEvent_h
#define __refinitiv_ema_access_ServiceEndpointDiscoveryEvent_h

/**
	@class rtsdk::ema::access::ServiceEndpointDiscoveryEvent ServiceEndpointDiscoveryEvent.h "Access/Include/ServiceEndpointDiscoveryEvent.h"
	@brief ServiceEndpointDiscoveryEvent encapsulates query identifiers.

	ServiceEndpointDiscoveryEvent is used to convey query identifiers to application. ServiceEndpointDiscoveryEvent is returned
	through ServiceEndpointDiscoveryClient callback methods.

	\remark ServiceEndpointDiscoveryEvent is a read only class. This class is used for query identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmConsumer,
		OmmConsumerClient
*/

#include "Access/Include/Common.h"

namespace rtsdk {

namespace ema {

namespace access {

class ServiceEndpointDiscovery;

class EMA_ACCESS_API ServiceEndpointDiscoveryEvent
{
	///@name Accessors
	//@{
	/** Returns an identifier (a.k.a., closure) associated with a query by consumer application
		Application associates the closure with a query on ServiceEndpointDiscovery::registerClient( ... , ... , void* closure)
		@return closure value
	*/
	void* getClosure() const;

	/** Return ServiceEndpointDiscovery instance for this event.
		@return reference to ServiceEndpointDiscovery
	*/
	ServiceEndpointDiscovery& getServiceEndpointDiscovery() const;
	//@}

private:
	ServiceEndpointDiscoveryEvent();
	virtual ~ServiceEndpointDiscoveryEvent();

	friend class ServiceEndpointDiscoveryImpl;

	ServiceEndpointDiscovery* _pServiceEndpointDiscovery;
	void* _pClosure;

	ServiceEndpointDiscoveryEvent(const ServiceEndpointDiscoveryEvent&);
	ServiceEndpointDiscoveryEvent& operator=(const ServiceEndpointDiscoveryEvent&);
};

}

}

}

#endif // 

