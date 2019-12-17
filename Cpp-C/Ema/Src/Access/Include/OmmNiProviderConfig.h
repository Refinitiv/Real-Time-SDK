/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProviderConfig_h
#define __thomsonreuters_ema_access_OmmNiProviderConfig_h

/**
	@class thomsonreuters::ema::access::OmmNiProviderConfig OmmNiProviderConfig.h "Access/Include/OmmNiProviderConfig.h"
	@brief OmmNiProviderConfig is used to specify configuration and behaviour of NonInteractive OmmProvider.

	OmmNiProviderConfig provides a default basic NonInteractive OmmProvider configuration.

	The default configuration may be modified and or appended by using EmaConfig.xml file or any interface
	methods of this class.

	The EmaConfig.xml file is read in if it is present in the working directory of the application.

	Calling any interface methods of OmmNiProviderConfig class overrides or appends the existing configuration.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmProvider,
		OmmProviderConfig
*/

#include "Access/Include/EmaString.h"
#include "Access/Include/OmmProviderConfig.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Data;
class ReqMsg;
class RefreshMsg;
class OmmNiProviderConfigImpl;

class EMA_ACCESS_API OmmNiProviderConfig : public OmmProviderConfig
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
		ENC_NONE = 0x00,			/*!< (0x00) No encryption. */
		ENC_TLSV1_2 = 0x04			/*!< (0x08) Encryption using TLSv1.2 protocol */
	};

	/** @enum AdminControl
	*/
	enum AdminControl
	{
		UserControlEnum,		/*!< specifies user submit directory refresh message */
		ApiControlEnum			/*!< specifies API sends down directory refresh message based on the configuration */
	};

	///@name Constructor
	//@{
	/** Constructs OmmNiProviderConfig
	*/
	OmmNiProviderConfig();
	//@}

	///@name Constructor
	//@{
	/** Constructs OmmNiProviderConfig
		@param[in] path specifies configuration file name or name of directory containing a file named EmaConfig.xml
		\remark path is optional. If not specified, application will use EmaConfig.xml (if any)  found in current working directory
	*/
	OmmNiProviderConfig(const EmaString & path);
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmNiProviderConfig();
	//@}

	///@name Accessors
	//@{
	/** Retrieve Provider's role
		@return role of this OmmNiProviderConfig instance
	*/
	ProviderRole getProviderRole() const;
	//@}

	///@name Operations
	//@{
	/** Clears the OmmNiProviderConfig and sets all the defaults.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmNiProviderConfig& clear();

	/** Specifies the username. Overrides a value specified in Login domain via the addAdminMsg( const ReqMsg& ) method.
		@param[in] username specifies name used on login request
		@return reference to this object
	*/
	OmmNiProviderConfig& username( const EmaString& username );

	/** Specifies the password. Overrides a value specified in Login domain via the addAdminMsg( const ReqMsg& ) method.
		@param[in] password specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& password( const EmaString& password );

	/** Specifies the position. Overrides a value specified in Login domain via the addAdminMsg( const ReqMsg& ) method.
		@param[in] position specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& position( const EmaString& position );

	/** Specifies the authorization application identifier. Must be unique for each application.
	    Range 257 to 65535 is available for site-specific use. Range 1 to 256 is reserved.
		@param[in] applicationId specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& applicationId( const EmaString& applicationId );

	/** Specifies the instance identifier. Can be any ASCII string, e.g. "Instance1".
		Used to differentiate applications running on the same client host.
		@param[in] instanceId specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& instanceId( const EmaString& instanceId );

	/** Specifies a hostname and port.  Overrides prior value.
		\remark Implies usage of TCP IP channel or RSSL_SOCKET.
		@param[in] host specifies server and port to which OmmProvider will connect
		\remark if host set to "<hostname>:<port>", then hostname:port is assumed
		\remark if host set to "", then localhost:14003 is assumed
		\remark if host set to ":", then localhost:14003 is assumed
		\remark if host set to "<hostname>", then hostname:14003 is assumed
		\remark if host set to "<hostname>:", then hostname:14003 is assumed
		\remark if host set to ":<port>", then localhost:port is assumed
		@return reference to this object
	*/
	OmmNiProviderConfig& host( const EmaString& host = "localhost:14003" );

	/** Specifies the operation model, overriding the default. The operation model specifies whether
	    to dispatch messages in the user or application thread of control.
		@param[in] specifies threading and dispatching model used by application
		@return reference to this object
	*/
	OmmNiProviderConfig& operationModel( OperationModel operationModel = ApiDispatchEnum );

	/** Specifies whether API or user controls sending of Directory refresh message.
		@param[in] control specifies who sends down the directory refresh message
		@return reference to this object
	*/
	OmmNiProviderConfig& adminControlDirectory( AdminControl control = ApiControlEnum );

	/** Create an OmmProvider with providerName.
		This name identifies configuration section to be used by OmmProvider instance.
		@param[in] providerName specifies name of OmmProvider instance
		@return reference to this object
	*/
	OmmNiProviderConfig& providerName( const EmaString& providerName );

	/** Specifies the address or host name of the proxy server to connect to for an HTTP or HTTPS connection.
		@param[in] proxyHostName specifies the address or host name of the proxy server
		for tunneling connection.
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingProxyHostName(const EmaString& proxyHostName);

	/** Specifies the port number of the proxy server to connect to for an HTTP or HTTPS connection.
		@param[in] proxyPort specifies the port number of the proxy server
		for tunneling connection.
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingProxyPort(const EmaString& proxyPort);

	/** Specifies the cryptographic protocols to be used for an Encrypted connection on a Linux operating system,
		of values TLSv1.2. The highest value of TLS will be selected by
		the Rssl API first, then it will roll back if the encryption handshake fails.
		The protocol defaults to TLSv1.2.
		Use OmmNiProviderConfig::EncryptedProtocolTypes flags to set allowed protocols.
		@param[in] securityProtocol specifies a cryptopgraphic protocol.
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingSecurityProtocol(int securityProtocol);

	/** Specifies the object name to pass along with the underlying
		URL in HTTP and HTTPS connection messages.
		@param[in] objectName specifies the object name.
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingObjectName(const EmaString& objectName);

	/** Specifies the name of the libssl.so shared library for Encrypted connections on
		a Linux operating system.
		@param[in] libsslName specifies the name of the libssl.so shared library
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingLibSslName(const EmaString& libsslName);

	/** Specifies the name of the libcrypto.so shared library for Encrypted connections on
		a Linux operating system.
		@param[in] libsslName specifies the name of the libcrypto.so shared library
		@return reference to this object
	*/
	OmmNiProviderConfig& tunnelingLibCryptoName(const EmaString& libcryptoName);

	/** Specifies the name of the lbicurl.so shared library for connecting through HTTP proxies.
	This is supported on Socket connections and Encrypted connections with Socket encrypted protocol.
	@param[in] libcurlName specifies the name of the libcurl.so shared library
	@return reference to this object
	*/
	OmmNiProviderConfig& libcurlName(const EmaString& libcurlName);

	/** Specifies the user name to authenticate. Needed for all authentication protocols.
	@param[in] proxyUserName specifies user name used for tunneling connection.
	@return reference to this object
	*/
	OmmNiProviderConfig& proxyUserName(const EmaString& proxyUserName);

	/** Specifies the passwd to authenticate. Needed for all authentication protocols.
	@param[in] proxyPasswd specifies password used for tunneling connection.
	@return reference to this object
	*/
	OmmNiProviderConfig& proxyPasswd(const EmaString& proxyPasswd);

	/** Specifies the domain of the user to authenticate.
	Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.

	For Negotiate/Kerberos or for Kerberos authentication protocols, proxyDomain
	should be the same as the domain in the 'realms' and 'domain_realm' sections of
	the Kerberos configuration file.

	@param[in] proxyDomain specifies the domain used for tunneling connection.
	@return reference to this object
	*/
	OmmNiProviderConfig& proxyDomain(const EmaString& proxyDomain);

	/** Specifies the path to an OpenSSL Certificate Authority store.

	@param[in] sslCAStore specifies the file or directory where the CA store is located.
	@return reference to this object
	*/
	OmmNiProviderConfig& sslCAStore(const EmaString& sslCAStore);

	/** Specifies the local configuration, overriding and adding to the current content.
		@param[in] config specifies OmmProvider configuration
		@return reference to this object
	*/
	OmmNiProviderConfig& config( const Data& config );

	/** Specifies an administrative request message to override the default administrative request.
	    Application may call multiple times prior to initialization. Supports Login domain only.
		@param[in] reqMsg specifies administrative domain request message
		@return reference to this object
	*/
	OmmNiProviderConfig& addAdminMsg( const ReqMsg& reqMsg );

	/** Specifies an administrative refresh message to override the default administrative refresh.
	    Application may call multiple times prior to initialization. Supports Directory domain only.
		@param[in] refreshMsg specifies administrative domain refresh message
		@return reference to this object
	*/
	OmmNiProviderConfig& addAdminMsg( const RefreshMsg& refreshMsg );
	//@}

private:

	friend class OmmNiProviderImpl;
	friend class OmmProvider;

	OmmNiProviderConfigImpl*		_pImpl;

	OmmNiProviderConfigImpl* getConfigImpl() const;

	OmmNiProviderConfig( const OmmNiProviderConfig& );
	OmmNiProviderConfig& operator=( const OmmNiProviderConfig& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderConfig_h
