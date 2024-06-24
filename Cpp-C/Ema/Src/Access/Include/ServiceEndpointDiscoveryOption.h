/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ServiceEndpointDiscoveryOption_h
#define __refinitiv_ema_access_ServiceEndpointDiscoveryOption_h

#include "Access/Include/EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class ServiceEndpointDiscoveryImpl;

/**
	@class refinitiv::ema::access::ServiceEndpointDiscoveryOption ServiceEndpointDiscoveryOption.h "Access/Include/ServiceEndpointDiscoveryOption.h"
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

	/** Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		@param[in] clientId specifies the client ID
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& clientId(const EmaString& clientId);

	/** Specifies the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		@param[in] clientSecret specifies a client secret.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption & clientSecret(const EmaString& clientSecret);

	/** Specifies the clientJWK.  This is used for login V2
	@param[in] clientJWK specifies the client JWK for oAuth2 interactions.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& clientJWK(const EmaString& clientJWK);

	/** Specifies the audience claim used by OAuth client to authenticate to the Authorization Server. This is optional for OAuth V2 JWT interactions.
	@param[in] audience specifies the client audience claim
	@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& audience(const EmaString& audience = "https://login.ciam.refinitiv.com/as/token.oauth2");

	/** Specifies optionally token scope to limit the scope of generated token from the token service.
		@param[in] tokenScope specifies a token scope
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption & tokenScope(const EmaString& tokenScope = "trapi.streaming.pricing.read");

	/** Specifies optionally the exclusive sign on control to force sign-out of other applications using the same credentials.
		@param[in] takeExclusiveSignOnControl the exclusive sign on control.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryOption& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl = true );

	/** \deprecated the transport() function should be used instead.
		Specifies a transport protocol to get endpoints according to the protocol.
		@param[in] transport specifies a transport protocol
		@return reference to this object
		\remark this is an optional option to limit number of endpoints
	*/
	ServiceEndpointDiscoveryOption& transprot(TransportProtocol transport);

	/** Specifies a transport protocol to get endpoints according to the protocol.
		@param[in] transport specifies a transport protocol
		@return reference to this object
		\remark this is an optional option to limit number of endpoints
	*/
	ServiceEndpointDiscoveryOption& transport(TransportProtocol transport);

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
	EmaString			_clientSecret;
	EmaString			_clientJWK;
	EmaString			_audience;
	EmaString			_tokenScope;
	bool				_takeExclusiveSignOnControl;
	EmaString			_proxyHostName;
	EmaString			_proxyPort;
	EmaString			_proxyUserName;
	EmaString			_proxyPassword;
	EmaString			_proxyDomain;
};

}

}

}

#endif // __refinitiv_ema_access_ServiceEndpointDiscoveryOption_h
