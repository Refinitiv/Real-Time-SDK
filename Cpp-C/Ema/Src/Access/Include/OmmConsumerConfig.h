/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumerConfig_h
#define __refinitiv_ema_access_OmmConsumerConfig_h

/**
	@class refinitiv::ema::access::OmmConsumerConfig OmmConsumerConfig.h "Access/Include/OmmConsumerConfig.h"
	@brief OmmConsumerConfig is used to modify configuration and behavior of OmmConsumer.

	OmmConsumerConfig provides a default basic OmmConsumer configuration.

	The default configuration may be modified and or appended by using EmaConfig.xml file or any interface methods
	of this class.

	The EmaConfig.xml file is read in if it is present in the working directory of the application.

	Calling any interface methods of OmmConsumerConfig class overrides or appends the existing configuration.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmConsumer
*/

#include "Access/Include/EmaString.h"
#include "Access/Include/OmmLoginCredentialConsumerClient.h"
#include "DataDictionary.h"
#include "Access/Include/EmaConfig.h"
#include "Access/Include/ServiceList.h"

namespace refinitiv {

namespace ema {

namespace access {

class Data;
class ReqMsg;
class OmmConsumerConfigImpl;
class OAuth2Credential;
class OmmOAuth2ConsumerClient;
class OmmRestLoggingClient;

class EMA_ACCESS_API OmmConsumerConfig
{
public :

	/** @enum OperationalModel
	*/
	enum OperationModel
	{
		UserDispatchEnum,		/*!< specifies callbacks happen on user thread of control */
		ApiDispatchEnum			/*!< specifies callbacks happen on API thread of control */
	};

	/** @enum EncryptionProtocolTypes
	*/
	enum EncryptionProtocolTypes 
	{
		ENC_NONE = 0x00,			/*!< @brief (0x00) No encryption. */
		ENC_TLSV1_2 = 0x04,			/*!< @brief (0x04) Encryption using TLSv1.2 protocol */
		ENC_TLSV1_3 = 0x08			/*!< @brief (0x08) Encryption using TLSv1.3 protocol */
	};

	///@name Constructor
	//@{
	/** Create an OmmConsumerConfig that enables customization of default implicit administrative domains and local configuration. 
	*/
	OmmConsumerConfig();
	//@}

	///@name Constructor
	//@{
	/** Create an OmmConsumerConfig that enables customization of default implicit administrative domains and local configuration. 
		@param[in] configPath configPath configuration file name or name of directory containing a file named EmaConfig.xml
		\remark path is optional. If not specified, application will use EmaConfig.xml (if any)  found in current working directory
	*/
	OmmConsumerConfig( const EmaString& configPath );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmConsumerConfig();
	//@}

	///@name Operations
	//@{
	/** Clears the OmmConsumerConfig and sets all the defaults.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmConsumerConfig& clear();

	/** Specifies the username. Mandatory for V1 oAuth Password Credentials logins with session management turned on. Overrides a value specified in Login domain via the addAdminMsg(..) method.
		@param[in] username specifies name used on login request
		@return reference to this object
	*/
	OmmConsumerConfig& username( const EmaString& username );

	/** Specifies the password. Mandatory for V1 oAuth Password Credentials logins with session management turned on. Overrides a value specified in Login domain via the addAdminMsg(..) method.
		@param[in] password specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& password( const EmaString& password );

	/** Specifies the position. Overrides a value specified in Login domain via the addAdminMsg(..) method.
		@param[in] position specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& position( const EmaString& position );

	/** Specifies the authorization application name set in login request attribute.
		@param[in] applicationName specifies respective login request attribute
		@return reference to this object
	 */
	OmmConsumerConfig& applicationName(const EmaString& applicationName);

	/** Specifies the authorization application identifier. Must be unique for each application.
	    Range 257 to 65535 is available for site-specific use. Range 1 to 256 is reserved.
		@param[in] applicationId specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& applicationId( const EmaString& applicationId );

	/** Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
		@param[in] clientId specifies the clientId.
		@return reference to this object
	*/
	OmmConsumerConfig& clientId( const EmaString& clientId );

	/** Specifies the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
		@param[in] clientSecret specifies a client secret.
		@return reference to this object
	*/
	OmmConsumerConfig& clientSecret( const EmaString& clientSecret );

	/** Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 
	@param[in] clientJWK specifies the JWK formatted private key
	@return reference to this object
	*/
	OmmConsumerConfig& clientJWK(const EmaString& clientJWK);

	/** Specifies the audience claim for the JWT. Optional, used for V2 Client Credentials with JWT.
	@param[in] audience specifies the client audience claim
	@return reference to this object
	*/
	OmmConsumerConfig& audience(const EmaString& audience = "https://login.ciam.refinitiv.com/as/token.oauth2");

	/** Specifies the token scope to limit the scope of generated token from the token service. Optional.
		@param[in] tokenScope specifies a token scope
		@return reference to this object
	*/
	OmmConsumerConfig& tokenScope( const EmaString& tokenScope = "trapi.streaming.pricing.read" );

	/** Specifies the take exclusive sign on control value. If set to true, other applications using the same credentials will be force signed-out. Optional and only used for V1 oAuth Password Credentials logins.
		@param[in] takeExclusiveSignOnControl the exclusive sign on control.
		@return reference to this object
	*/
	OmmConsumerConfig& takeExclusiveSignOnControl( bool takeExclusiveSignOnControl = true );

	/** Specifies an URL to override the default for token service V1 Password Credentials to perform authentication to get access and refresh tokens.
		@param[in] tokenServiceUrl specifies an URL for token service.
		@return reference to this object
	*/
	OmmConsumerConfig& tokenServiceUrl( const EmaString& tokenServiceUrl = "https://api.refinitiv.com/auth/oauth2/v1/token" );

	/** Specifies an URL to override the default for token service V1 Password Credentials to perform authentication to get access and refresh tokens.
		@param[in] tokenServiceUrl specifies an URL for token service.
		@return reference to this object
	*/
	OmmConsumerConfig& tokenServiceUrlV1(const EmaString& tokenServiceUrl = "https://api.refinitiv.com/auth/oauth2/v1/token");

	/** Specifies an URL to override the default for token service V2 Client Credentials to perform authentication to get access and refresh tokens.
		@param[in] tokenServiceUrl specifies an URL for token service.
		@return reference to this object
	*/
	OmmConsumerConfig& tokenServiceUrlV2(const EmaString& tokenServiceUrl = "https://api.refinitiv.com/auth/oauth2/v2/token");

	/** Specifies an URL to override the default for the RDP service discovery to get global endpoints
		@param[in] serviceDiscoveryUrl specifies an URL for RDP service discovery.
		@return reference to this object
	*/
	OmmConsumerConfig& serviceDiscoveryUrl( const EmaString& serviceDiscoveryUrl = "https://api.refinitiv.com/streaming/pricing/v1/" );

	/** Specifies a hostname and port.  Overrides prior value.
		\remark Implies usage of TCP IP channel or RSSL_CONN_TYPE_SOCKET.
		@param[in] host specifies server and port to which OmmConsumer will connect
		\remark if host set to "<hostname>:<port>", then hostname:port is assumed
		\remark if host set to "", then localhost:14002 is assumed
		\remark if host set to ":", then localhost:14002 is assumed
		\remark if host set to "<hostname>", then hostname:14002 is assumed
		\remark if host set to "<hostname>:", then hostname:14002 is assumed
		\remark if host set to ":<port>", then localhost:port is assumed
		@return reference to this object
	*/
	OmmConsumerConfig& host( const EmaString& host = "localhost:14002" );

	/**	Specifies connection type. Overrides prior value
		@param[in] specifies connection type used by application. Connection type defined in EmaConfig::ConnectionTypeEnum
		@throw OmmInvalidUsageException if use this API with WarmStandby channel configuration.
		@throw OmmInvalidUsageException if channelType is not valid.
		@return reference to this object
	*/
	OmmConsumerConfig& channelType(EmaConfig::ConnectionTypeEnum channelType);

	/**	Specifies encrypted protocol type.  Overrides prior value
		@param[in] specifies encrypted protocol type used by application. Encrypted protocol type defined in EmaConfig::EncryptedProtocolTypeEnum
		@throw OmmInvalidUsageException if use this API with WarmStandby channel configuration.
		@throw OmmInvalidUsageException if use this API with not encoded channel type.
		@throw OmmInvalidUsageException if encProtocolType is not valid.
		@return reference to this object
	*/
	OmmConsumerConfig& encryptedProtocolType(EmaConfig::EncryptedProtocolTypeEnum encProtocolType);

	/** Specifies the operation model, overriding the default. The operation model specifies whether
	    to dispatch messages in the user or application thread of control.
		@param[in] specifies threading and dispatching model used by application
		@return reference to this object
	*/
	OmmConsumerConfig& operationModel( OperationModel operationModel = ApiDispatchEnum );

	/** Create an OmmConsumer with consumer name. The OmmConsumer enables functionality that includes
	    subscribing, posting and distributing generic messages. This name identifies configuration
		section to be used by OmmConsumer instance.
		@param[in] consumerName specifies name of OmmConsumer instance
		@return reference to this object
	*/
	OmmConsumerConfig& consumerName( const EmaString& consumerName );

	/** Specifies the address or host name of the proxy server to connect to for an HTTP or HTTPS connection.
		@param[in] proxyHostName specifies the address or host name of the proxy server 
		for tunneling connection.
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingProxyHostName( const EmaString& proxyHostName );

	/** Specifies the port number of the proxy server to connect to for an HTTP or HTTPS connection.
		@param[in] proxyPort specifies the port number of the proxy server
		for tunneling connection.
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingProxyPort(const EmaString& proxyPort);

	/** Specifies the cryptographic protocols to be used for an Encrypted connection on a Linux operating system, 
		of values TLSv1.2. The highest value of TLS will be selected by 
		the Rssl API first, then it will roll back if the encryption handshake fails. 
		The protocol defaults to TLSv1.2 and TLSv1.3.
		Use OmmConsumerConfig::EncryptionProtocolTypes flags to set allowed protocols.
		@param[in] securityProtocol specifies a cryptopgraphic protocol.
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingSecurityProtocol(int securityProtocol);

	/** Specifies the object name to pass along with the underlying
		URL in HTTP and HTTPS connection messages.
		@param[in] objectName specifies the object name.
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingObjectName(const EmaString& objectName);

	/** Specifies the name of the libssl.so shared library for Encrypted connections.
		@param[in] libsslName specifies the name of the libssl.so shared library
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingLibSslName(const EmaString& libsslName);

	/** Specifies the name of the libcrypto.so shared library for Encrypted connections.
		@param[in] libsslName specifies the name of the libcrypto.so shared library
		@return reference to this object
	*/
	OmmConsumerConfig& tunnelingLibCryptoName(const EmaString& libcryptoName);

	/** Specifies the name of the lbicurl.so shared library for connecting through HTTP proxies. 
		This is supported on Socket connections and Encrypted connections with Socket encrypted protocol.
	@param[in] libcurlName specifies the name of the libcurl.so shared library
	@return reference to this object
	*/
	OmmConsumerConfig& libcurlName(const EmaString& libcurlName);

	/** Specifies the user name to authenticate. Needed for all authentication protocols.
		@param[in] proxyUserName specifies user name used for tunneling connection.
		@return reference to this object
	*/
	OmmConsumerConfig& proxyUserName(const EmaString& proxyUserName);

	/** Specifies the passwd to authenticate. Needed for all authentication protocols.
		@param[in] proxyPasswd specifies password used for tunneling connection.
		@return reference to this object
	*/
	OmmConsumerConfig& proxyPasswd(const EmaString& proxyPasswd);

	/** Specifies the domain of the user to authenticate.
		Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.

		For Negotiate/Kerberos or for Kerberos authentication protocols, proxyDomain
		should be the same as the domain in the 'realms' and 'domain_realm' sections of
		the Kerberos configuration file.

		@param[in] proxyDomain specifies the domain used for tunneling connection.
		@return reference to this object
	*/
	OmmConsumerConfig& proxyDomain(const EmaString& proxyDomain);

	/** Specifies the path to an OpenSSL Certificate Authority store.

	@param[in] sslCAStore specifies the file or directory where the CA store is located.
	@return reference to this object
	*/
	OmmConsumerConfig& sslCAStore(const EmaString& sslCAStore);

	/** Specifies the local configuration, overriding and adding to the current content. 
		@param[in] config specifies OmmConsumer configuration
		@return reference to this object
	*/
	OmmConsumerConfig& config( const Data& config );

	/** Specifies an administrative request message to override the default administrative request.
	    Application may call multiple times prior to initialization. Supported domains include Login,
	    Directory, and Dictionary. 
		@param[in] reqMsg specifies administrative domain request message
		@return reference to this object
	*/
	OmmConsumerConfig& addAdminMsg( const ReqMsg& reqMsg );


	/** Specifies a set of OAuth2 credentials to be used with a specific set of channels.  
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] credential specifies oAuth2 credentials
		@return reference to this object
	*/
	OmmConsumerConfig& addOAuth2Credential(const OAuth2Credential& credential);

	/** Specifies a set of OAuth2 credentials to be used with a specific set of channels.
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] credential specifies oAuth2 credentials
		@param[in] client callback client associated with the above set of credentials
		@return reference to this object
	*/
	OmmConsumerConfig& addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client);
	
	/** Specifies a set of OAuth2 credentials to be used with a specific set of channels.
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] credential specifies oAuth2 credentials
		@param[in] client callback client associated with the above set of credentials
		@param[in] closure user specified closure
		@return reference to this object
	*/
	OmmConsumerConfig& addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client, void* closure);

	/** Specifies a set of Login credentials to be used with a specific set of channels.
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] ReqMsg Login domain request message. This message must follow the RDM Login domain.
		@param[in] channelList Comma separated list of channel names 
		@return reference to this object
	*/
	OmmConsumerConfig& addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList);

	/** Specifies a set of Login credentials to be used with a specific set of channels.
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] ReqMsg Login domain request message. This message must follow the RDM Login domain.
		@param[in] channelList Comma separated list of channel names
		@param[in] client callback client associated with the above request message
		@return reference to this object
	*/
	OmmConsumerConfig& addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client);

	/** Specifies a set of Login credentials to be used with a specific set of channels.
		Application may call this multiple times prior to initialization to set multiple sets of credentials.
		@param[in] ReqMsg Login domain request message. This message must follow the RDM Login domain.
		@param[in] channelList Comma separated list of channel names
		@param[in] client callback client associated with the above request message
		@param[in] closure user specified closure
		@return reference to this object
	*/
	OmmConsumerConfig& addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client, void* closure);


	/** Specifies the user callback client to receive REST logging messages.
		@param[in] ommRestLoggingClient specifies the user callback client used for receiving REST logging messages.
		@param[in] closure specifies application defined identification value.
		@return reference to this object
	*/
	OmmConsumerConfig& restLoggingCallback( OmmRestLoggingClient& ommRestLoggingClient, void* closure = (void*)0 );

	/** Specifies the Cpu core to bind for Reactor Worker thread.
		Application may call multiple times prior to initialization.
		@param[in] cpuString specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format).
		@return reference to this object
	*/
	OmmConsumerConfig& workerThreadBind( const EmaString& cpuString );

	/** Specifies the Cpu core to bind for internal EMA thread that dispatch messages.
		When ApiDispatchEnum set as operational model.
		Application may call multiple times prior to initialization.
		@param[in] cpuString specifies the Cpu core in string format (Cpu core id or P:X C:Y T:Z format).
		@return reference to this object
	*/
	OmmConsumerConfig& apiThreadBind(const EmaString& cpuString);

	/** Specifies should ETA initialize CpuID library. It will analyze CPU topology.
		Application may call multiple times prior to initialization.
		@param[in] shouldInitCPUIDlib true ETA should initialize CpuID library; otherwise ETA will not initialize CpuID library.
		@return reference to this object
	*/
	OmmConsumerConfig& shouldInitializeCPUIDlib(bool shouldInitCPUIDlib);

    /** Specifies the DataDictionary object.
        Overrides DataDictionary object that is provided via EmaConfig.xml or
        Programmatic configure.
        @param[in] dataDictionary specifies the DataDictionary object.
        @param[in] shouldCopyIntoAPI specifies whether to copy dataDictionary into API or pass it in as a reference.
        @throw OmmInvalidUsageException if application passes not fully loaded DataDictionary object
        @return reference to this object.
    */
	OmmConsumerConfig& dataDictionary(const refinitiv::ema::rdm::DataDictionary& dataDictionary, bool shouldCopyIntoAPI = false);

	/** Specifies the address or host name of the proxy server for Rest requests: service discovery and auth token service.
		@param[in] restProxyHostName specifies the address or host name of the proxy server for Rest requests.
		@return reference to this object
	*/
	OmmConsumerConfig& restProxyHostName(const EmaString& restProxyHostName);

	/** Specifies the port number of the proxy server for Rest requests: service discovery and auth token service.
		@param[in] restProxyPort specifies the port number of the proxy server for Rest requests.
		@return reference to this object
	*/
	OmmConsumerConfig& restProxyPort(const EmaString& restProxyPort);

	/** Specifies the user name to authenticate to the proxy server for Rest requests. Needed for all authentication protocols.
		@param[in] restProxyUserName specifies user name for the proxy server authentication for Rest requests.
		@return reference to this object
	*/
	OmmConsumerConfig& restProxyUserName(const EmaString& restProxyUserName);

	/** Specifies the passwd to authenticate to the proxy server for Rest requests. Needed for all authentication protocols.
		@param[in] restProxyPasswd specifies password for the proxy server authentication for Rest requests.
		@return reference to this object
	*/
	OmmConsumerConfig& restProxyPasswd(const EmaString& restProxyPasswd);

	/** Specifies the domain of the user to authenticate to the proxy server for Rest requests: service discovery and auth token service.
		Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.

		For Negotiate/Kerberos or for Kerberos authentication protocols, restProxyDomain
		should be the same as the domain in the 'realms' and 'domain_realm' sections of
		the Kerberos configuration file.

		@param[in] restProxyDomain specifies the domain used for the proxy server authentication for Rest requests.
		@return reference to this object
	*/
	OmmConsumerConfig& restProxyDomain(const EmaString& restProxyDomain);

	/** Specifies a new ServiceList to be added to this configuration. The ServiceList is a list of concrete service names that are can be subscribed to using the name of the service list.

		@param[in] serviceList ServiceList object that contains the concrete service names.
		@return reference to this object
	*/
	OmmConsumerConfig& addServiceList(const ServiceList& serviceList);
	//@}

private :

	friend class OmmConsumerImpl;
	friend class OmmConsumer;

	OmmConsumerConfigImpl* getConfigImpl() const;

	OmmConsumerConfigImpl* _pImpl;

	OmmConsumerConfig( const OmmConsumerConfig& );
	OmmConsumerConfig& operator=( const OmmConsumerConfig& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerConfig_h
