/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ServiceDiscoveryClient_h
#define __thomsonreuters_ema_access_ServiceDiscoveryClient_h

/**
	@class thomsonreuters::ema::access::ServiceEndpointDiscoveryClient ServiceEndpointDiscoveryClient.h "Access/Include/ServiceEndpointDiscoveryClient.h"
	@brief ServiceEndpointDiscoveryClient class provides callback interfaces to pass received responses.

	Application needs to implement an application client class inheriting from ServiceEndpointDiscoveryClient.
	In its own class, application needs to override callback methods it desires to use for receiving responses.
	Default empty callback methods are implemented by ServiceEndpointDiscoveryClient class.

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	The following code snippet shows basic usage of ServiceEndpointDiscoveryClient class to print recevied messages to screen.

	\code

	class AppClient : public ServiceEndpointDiscoveryClient
	{
		void onSuccess( const ServiceEndpointDiscoveryResp& , const ServiceEndpointDiscoveryEvent& );
		void onError( const EmaString& errorText, const ServiceEndpointDiscoveryEvent& )
	};

	void AppClient::onSuccess( const ServiceEndpointDiscoveryResp& serviceEndpointResp, const ServiceEndpointDiscoveryEvent& event)
	{
		cout << serviceEndpointResp << endl;
	}

	void AppClient::onError(const EmaString& errorText, const ServiceEndpointDiscoveryEvent& event)
	{
		cout << errorText << endl;
	}

	\endcode

	@see ServiceEndpointDiscovery,
		ServiceEndpointDiscoveryResp,
		ServiceEndpointDiscoveryEvent,
*/

#include "Access/Include/EmaString.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ServiceEndpointDiscoveryResp;
class ServiceEndpointDiscoveryEvent;

class EMA_ACCESS_API ServiceEndpointDiscoveryClient
{

public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving a success response.
		@param[out] serviceEndpointResp received ServiceEndpointDiscoveryResp
		@param[out] event identifies open query for which this response is received
		@return void
	*/
	virtual void onSuccess(const ServiceEndpointDiscoveryResp& serviceEndpointResp, const ServiceEndpointDiscoveryEvent& event);

	/** Invoked upon receiving an error response.
	@param[out] errorText received error text message
	@param[out] event identifies open query for which this response is received
	@return void
*/
	virtual void onError(const EmaString& errorText, const ServiceEndpointDiscoveryEvent& event);
	//@}

protected:
	ServiceEndpointDiscoveryClient();
	virtual ~ServiceEndpointDiscoveryClient();

private:

	ServiceEndpointDiscoveryClient(const ServiceEndpointDiscoveryClient&);
	ServiceEndpointDiscoveryClient& operator=(const ServiceEndpointDiscoveryClient&);
};

}

}

}

#endif // __thomsonreuters_ema_access_ServiceDiscoveryClient_h
