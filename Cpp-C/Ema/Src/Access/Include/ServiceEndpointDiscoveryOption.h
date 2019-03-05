/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ServiceEndpointDiscoveryOption_h
#define __thomsonreuters_ema_access_ServiceEndpointDiscoveryOption_h

#include "Access/Include/EmaString.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ServiceEndpointDiscoveryImpl;

/**
	@class thomsonreuters::ema::access::ServiceEndpointDiscoveryOption ServiceEndpointDiscoveryOption.h "Access/Include/ServiceEndpointDiscoveryOption.h"
	@brief ServiceEndpointDiscoveryOption is used to specify query options for ServiceEndpointDiscovery::registerClient().

	\remark All methods in this class are \ref SingleThreaded.

	@see ServiceEndpointDiscovery
*/

class EMA_ACCESS_API ServiceEndpointDiscoveryOption
{
public:

	enum TransportProtocol
	{
		UnknownTransportEnum = 0,	/*!< Indicates undefined transport protocol */
		TcpEnum = 1,				/*!< Indicates TCP transport protocol */
		WebsocketEnum = 2,			/*!< Indicates Websocket transport protocol */
	};

	enum DataformatProtocol
	{
		UnknownDataFormatEnum = 0,	/*!< Indicates undefined data format protocol */
		RwfEnum = 1,				/*!< Indicates RWF data format protocol */
		Json2Enum = 2,				/*!< Indicates tr_json2 data format protocol */
	};

	///@name Constructor
	//@{
	/** Create an ServiceEndpointDiscoveryOption that enables configuration of optional parameters.
	*/
	ServiceEndpointDiscoveryOption();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ServiceEndpointDiscoveryOption();
	//@}

	///@name Operations
	//@{
	/** Clears the ServiceEndpointDiscoveryOption and sets all the defaults.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& clear();

	/** Specifies the username for sending authorization request with the token service.
		@param[in] username specifies name used on login request
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& username(const EmaString& username);

	/** Specifies the password associated with the username for sending authorization request with the token service.
		@param[in] password specifies password on login request
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& password(const EmaString& password);

	/** Specifies the a unique identifier for an application making the request.
		@param[in] clientId specifies the client ID
		@return reference to this object
		\remark EMA uses the username as Client ID by default.
	*/
	ServiceEndpointDiscoveryOption& clientId(const EmaString& clientId);

	/** Specifies a transport protocol to get endpoints according to the protocol.
		@param[in] transport specifies a transport protocol
		@return reference to this object
		\remark this is an optional option to limit number of endpoints
	*/
	ServiceEndpointDiscoveryOption& transprot(TransportProtocol transport);

	/** Specifies a data format protocol to get endpoints according to the protocol.
		@param[in] dataFormat specifies a data format protocol
		@return reference to this object
		\remark this is an optional option to limit number of endpoints
	*/
	ServiceEndpointDiscoveryOption& dataFormat(DataformatProtocol dataFormat);

	/** Specifies a proxy server hostname.
	@param[in] proxyHostName specifies a proxy server
	@return reference to this object
	\remark this is used to send the request via a proxy server
	*/
	ServiceEndpointDiscoveryOption& proxyHostName(const EmaString& proxyHostName);

	/** Specifies a proxy server port.
	@param[in] proxyPort specifies a proxy port
	@return reference to this object
	\remark this is used to send the request via a proxy server
	*/
	ServiceEndpointDiscoveryOption& proxyPort(const EmaString& proxyPort);

	/** Specifies a username to perform authorization with a proxy server.
	@param[in] proxyUserName specifies a proxy user name
	@return reference to this object
	\remark this is used to send the request via a proxy server
	*/
	ServiceEndpointDiscoveryOption& proxyUserName(const EmaString& proxyUserName);

	/** Specifies a password to perform authorization with a proxy server.
	@param[in] proxyPassword specifies a proxy password
	@return reference to this object
	\remark this is used to send the request via a proxy server
	*/
	ServiceEndpointDiscoveryOption& proxyPassword(const EmaString& proxyPassword);

	/** Specifies a proxy domain of the user to perform authenticate.
	@param[in] proxyDomain specifies a proxy domain
	@return reference to this object
	\ Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.
	*/
	ServiceEndpointDiscoveryOption& proxyDomain(const EmaString& proxyDomain);
	//@}

private:

	friend class ServiceEndpointDiscoveryImpl;

	TransportProtocol	_transport;
	DataformatProtocol	_dataFormat;
	EmaString			_username;
	EmaString			_password;
	EmaString			_clientId;
	EmaString			_proxyHostName;
	EmaString			_proxyPort;
	EmaString			_proxyUserName;
	EmaString			_proxyPassword;
	EmaString			_proxyDomain;
};

}

}

}

#endif // __thomsonreuters_ema_access_ServiceEndpointDiscoveryOption_h
