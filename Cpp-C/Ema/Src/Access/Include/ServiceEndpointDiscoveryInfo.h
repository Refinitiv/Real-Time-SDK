/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ServiceEndpointDiscoveryInfo_h
#define __thomsonreuters_ema_access_ServiceEndpointDiscoveryInfo_h

/**
	@class thomsonreuters::ema::access::ServiceEndpointDiscoveryInfo ServiceEndpointDiscoveryInfo.h "Access/Include/ServiceEndpointDiscoveryInfo.h"
	@brief ServiceEndpointDiscoveryInfo represents an service endpoint information from EDP-RT service discovery.

	\remark All methods in this class are \ref SingleThreaded.

	@see ServiceEndpointDiscoveryResp
*/


#include "Access/Include/EmaVector.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API ServiceEndpointDiscoveryInfo
{
public:

	/** Gets a list of data format supported by this endpoint.
	*	@return a list of data format
	*/
	const EmaVector<EmaString>& getDataFormatList() const;

	/** Gets an endpoint or domain name for establishing a connection
	*	@return an endpoint
	*/
	const EmaString&	getEndPoint() const;

	/** Gets a list of location where the infrastructure is deployed in the cloud provider
	*	@return a list of location
	*/
	const EmaVector<EmaString>& getLocationList() const;

	/** Gets a port for establishing a connection
	*	@return a port
	*/
	const EmaString&	getPort() const;

	/** Gets a public cloud provider
	*	@return a cloud provider
	*/
	const EmaString&	getProvider() const;

	/** Gets a transport type
	*	@return a transport type
	*/
	const EmaString&	getTransport() const;

	/** Assignment operator
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] other copied in ServiceEndpointDiscoveryInfo object
		@return reference to this object
	*/
	ServiceEndpointDiscoveryInfo& operator=(const ServiceEndpointDiscoveryInfo& other);

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
	ServiceEndpointDiscoveryInfo();
	virtual ~ServiceEndpointDiscoveryInfo();

	const EmaString& toString(UInt64 indent) const;

	template<class T>
	friend class EmaVector;
	friend class ServiceEndpointDiscoveryImpl;
	friend class ServiceEndpointDiscoveryResp;

	EmaVector<EmaString>* _pDataFormatList;
	EmaString            _endPoint;
	EmaVector<EmaString>* _pLocationList;
	EmaString            _port;
	EmaString            _provider;
	EmaString            _transport;
	mutable EmaString    _toString;
};

}

}

}
#endif // #ifndef __thomsonreuters_ema_access_ServiceEndpointDiscoveryInfo_h