///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


/**
 * ServiceEndpointDiscoveryOption is used to specify query options for {@link com.refinitiv.ema.access.ServiceEndpointDiscovery#registerClient(ServiceEndpointDiscoveryOption, ServiceEndpointDiscoveryClient, Object)}
 *
 * @see ServiceEndpointDiscovery
 */
public interface ServiceEndpointDiscoveryOption
{

	/**
	 * TransportProtocol represents transport protocol options
	 */
	public class TransportProtocol
	{
		/**
		 * Indicates undefined transport protocol
		 */
		public final static int UNKNOWN = 0;

		/**
		 * Indicates TCP transport protocol
		 */
		public final static int TCP = 1;

		/**
		 * Indicates Websocket transport protocol
		 */
		public final static int WEB_SOCKET = 2;
	}
	
	/**
	 * DataformatProtocol represents data format protocol options
	 */
	public class DataformatProtocol
	{
		/**
		 * Indicates undefined data format protocol
		 */
		public final static int UNKNOWN = 0;

		/**
		 * Indicates RWF data format protocol
		 */
		public final static int RWF = 1;

		/**
		 * Indicates tr_json2 data format protocol
		 */
		public final static int JSON2 = 2;
	}
	
	/**
	 * Clears the ServiceEndpointDiscoveryOption and sets all the defaults.
	 * 
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption clear();
	
	/**
	 * Specifies the username for sending authorization request with the token service.
	 * 
	 * @param username specifies name used on login request
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption username(String username);
	
	/**
	 * Specifies the password associated with the username for sending authorization request with the token service
	 * 
	 * @param password specifies password on login request
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption password(String password);

	/**
	 * Specifies the clientID used for LDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
	 * 
	 * @param clientId specifies the client ID
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption clientId(String clientId);
	
	/**
	 * Specifies optionally a client secret used by OAuth client to authenticate to the Authorization Server.
	 * 
	 * @param clientSecret specifies a client secret.
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption clientSecret(String clientSecret);
	
	/**
	 * Specifies optionally a client JWK string used to authenticate to the Authorization Server.
	 * 
	 * @param clientJwk specifies a client Jwk string.
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption clientJWK(String clientJwk);
	
	/**
	 * Specifies optionally a token scope to limit the scope of generated token from the token service.
	 * 
	 * @param tokenScope specifies a token scope
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption tokenScope(String tokenScope);
	
	/**
	 * Specifies optionally an audience string used with JWT authentication.
	 * 
	 * @param audience specifies a token scope
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption audience(String audience);
	
	/**
     * Sets the exclusive sign on control to force sign-out of other applications using the same credentials.
     * <p>Defaults to true</p>
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     * @return reference to this object
     */
	ServiceEndpointDiscoveryOption takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl);
	
	/**
	 * Specifies a transport protocol to get endpoints according to the protocol.
	 * 
	 * <p>This is an optional option to limit number of endpoints.</p>
	 * 
	 * @param transport specifies a transport protocol
	 * @return reference to this object
	 * 
	 * @see TransportProtocol
	 */
	ServiceEndpointDiscoveryOption transport(int transport);
	
	/**
	 * Specifies a data format protocol to get endpoints according to the protocol.
	 * 
	 * <p>This is an optional option to limit number of endpoints.</p>
	 * 
	 * @param dataFormat specifies a data format protocol
	 * @return reference to this object
	 * 
	 * @see DataformatProtocol
	 */
	ServiceEndpointDiscoveryOption dataFormat(int dataFormat);
	
	/**
	 * Specifies the address or hostname of the HTTP proxy server
	 * 
	 * @param proxyHostName specifies the proxy hostname
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyHostName(String proxyHostName);
	
	/**
	 * Specifies the port number of the HTTP proxy server
	 * 
	 * @param proxyPort specifies the proxy port
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyPort(String proxyPort);
	
	/**
	 * Specifies the proxy user name to authenticate.
	 * 
	 * @param proxyUserName specifies a proxy user name
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyUserName(String proxyUserName);
	
	/**
	 * Specifies the proxy password to authenticate.
	 * 
	 * @param proxyPassword specifies a proxy password
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyPassword(String proxyPassword);
	
	/**
	 * Specifies the proxy domain to authenticate.
	 * 
	 * @param proxyDomain specifies a proxy domain
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyDomain(String proxyDomain);
	
	/**
	 * Specifies the local hostname of the client to authenticate.
	 * <p>Needed for NTLM authentication protocol only. EMA assigns 
	 * hostname/IP address by default if not specified.</p>
	 * 
	 * @param localHostName specifies a local host name
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyLocalHostName(String localHostName);
	
	/**
	 * Specifies the complete path of the Kerberos5 configuration file.
	 * <p>Needed for Negotiate/Kerberos and Kerberos authentications</p>
	 * 
	 * @param krb5ConfigFile specifies a Kerberos5 configuration file
	 * @return reference to this object
	 */
	ServiceEndpointDiscoveryOption proxyKRB5ConfigFile(String krb5ConfigFile);
}
