/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_ServiceEndpointDiscoveryResp_h
#define __rtsdk_ema_access_ServiceEndpointDiscoveryResp_h

/**
	@class rtsdk::ema::access::ServiceEndpointDiscoveryResp ServiceEndpointDiscoveryResp.h "Access/Include/ServiceEndpointDiscoveryResp.h"
	@brief ServiceEndpointDiscoveryResp represents a response from EDP-RT service discovery which contains a list of ServiceEndpointDiscoveryInfo

	\remark All methods in this class are \ref SingleThreaded.

	@see ServiceEndpointDiscoveryInfo
*/

#include "Access/Include/EmaVector.h"

namespace rtsdk {

namespace ema {

namespace access {

class ServiceEndpointDiscoveryInfo;

class EMA_ACCESS_API ServiceEndpointDiscoveryResp
{
public:

	/** Gets a list of ServiceEndpointDiscoveryInfo of this ServiceEndpointDiscoveryResp.
	*	@return a list of DictionaryEntry
	*/
	const EmaVector<ServiceEndpointDiscoveryInfo>& getServiceEndpointInfoList() const;

	/** Returns a string representation of the class instance.
	@throw OmmMemoryExhaustionException if app runs out of memory
	@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Operator const char* overload.
	@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;

private:
	ServiceEndpointDiscoveryResp();
	virtual ~ServiceEndpointDiscoveryResp();

	friend class ServiceEndpointDiscoveryImpl;

	EmaVector<ServiceEndpointDiscoveryInfo>* _pServiceEndpointDiscoveryInfoList;
	mutable EmaString                       _toString;
};

}

}

}

#endif // __rtsdk_ema_access_ServiceEndpointDiscoveryResp_h

