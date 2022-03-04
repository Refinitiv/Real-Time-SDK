/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ServiceEndpointDiscovery_h
#define __refinitiv_ema_access_ServiceEndpointDiscovery_h

/**
	@class refinitiv::ema::access::ServiceEndpointDiscovery ServiceEndpointDiscovery.h "Access/Include/ServiceEndpointDiscovery.h"
	@brief ServiceEndpointDiscovery class provides the functionality to query endpoints from RDP service discovery.

	Application interacts with the service discovery through the ServiceEndpointDiscovery interface methods. The results of
	these interactions are communicated back to application through ServiceEndpointDiscoveryClient.

	The following code snippet shows basic usage of ServiceEndpointDiscovery class in a simple consumer type app.

	\code

	// create an implementation for ServiceEndpointDiscoveryClient to process
	class AppClient : public ServiceEndpointDiscoveryClient
	{
		void onSuccess( const ServiceEndpointDiscoveryResp& , const ServiceEndpointDiscoveryEvent& );
		void onError( const EmaString& , const ServiceEndpointDiscoveryEvent& );
	};

	AppClient appClient;

	// instantiate ServiceEndpointDiscovery object
	ServiceEndpointDiscovery service;

	// Query endpoints
	consumer.registerClient( ServiceEndpointDiscoveryOption().username(userName).password(password)
			.transport(ServiceEndpointDiscoveryOption::TcpEnum) , client);

	\endcode

	@see ServiceEndpointDiscoveryOption,
		ServiceEndpointDiscoveryClient,
		OmmException
*/

#include "Access/Include/EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class ServiceEndpointDiscoveryOption;
class ServiceEndpointDiscoveryClient;
class ServiceEndpointDiscoveryImpl;

class EMA_ACCESS_API ServiceEndpointDiscovery
{
public:

	///@name Constructor
//@{
/** Create an ServiceEndpointDiscovery with the default settings.
	\remark This affects exceptions thrown from ServiceEndpointDiscovery methods
 */
	ServiceEndpointDiscovery();
	//@}

	///@name Constructor
	//@{
	/** Create an ServiceEndpointDiscovery.
		@param[in] tokenServiceURLV1 specifies the token service URL V1 to override the default value.
		@param[in] serviceDiscoveryURL specifies the service discovery URL to override the default value.
		\remark This affects exceptions thrown from ServiceEndpointDiscovery methods
	 */
	ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1);
	//@}

	///@name Constructor
	//@{
	/** Create an ServiceEndpointDiscovery.
		@param[in] tokenServiceURLV1 specifies the token service URL V1 to override the default value.
		@param[in] serviceDiscoveryURL specifies the service discovery URL to override the default value.
		\remark This affects exceptions thrown from ServiceEndpointDiscovery methods
	 */
	ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& serviceDiscoveryURL);
	//@}

	///@name Constructor
	//@{
	/** Create an ServiceEndpointDiscovery.
		@param[in] tokenServiceURLV1 specifies the token service URL for V1 to override the default value.
		@param[in] tokenServiceURLV2 specifies the token service URL for V2 to override the default value.
		@param[in] serviceDiscoveryURL specifies the service discovery URL to override the default value.
		\remark This affects exceptions thrown from ServiceEndpointDiscovery methods
	 */
	ServiceEndpointDiscovery(const EmaString& tokenServiceURLV1, const EmaString& tokenServiceURLV2, const EmaString& serviceDiscoveryURL);

	///@name Destructor
	//@{
	/** Destructor.
	*/
	~ServiceEndpointDiscovery();
	//@}

	///@name Operations
	//@{
	/** Queries the RDP service discovery synchronously to get endpoints according to the specified parameters 
		@param[in] params specifies query options to get endpoints
		@param[in] client specifies ServiceEndpointDiscoveryClient instance receiving notifications about this query
		@param[in] closure specifies application defined query identification
		@throw OmmMemoryExhaustionException if system runs out of memory
		@throw OmmInvalidUsageException if application passes invalid ServiceEndpointDiscoveryOption
		\remark This is synchronous call and this method returns after receving a response
		\remark This method is \ref ObjectLevelSafe

	*/
	void registerClient(const ServiceEndpointDiscoveryOption& params, ServiceEndpointDiscoveryClient& client, void *closure = 0);
	//@}

private:
	ServiceEndpointDiscoveryImpl*	_pImpl;
};

}

}

}

#endif // __refinitiv_ema_access_ServiceEndpointDiscovery_h

