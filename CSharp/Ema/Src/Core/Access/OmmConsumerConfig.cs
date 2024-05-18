/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Security;
using System.Text;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmConsumerConfig is used to modify configuration and behaviour of <see cref="OmmConsumer"/>.
    /// </summary>
    /// <remarks>
    /// <para>OmmConsumerConfig provides a default basic OmmConsumer configuration.</para>
    ///
    /// <para>The default configuration may be modified and or appended by using any methods from
    /// OmmConsumerConfg.</para>
    ///
    /// <para>OmmConsumerconfig methods override or append the existing configuration.</para>
    /// </remarks>
    /// <seealso cref="OmmConsumer"/>
    public sealed class OmmConsumerConfig
    {
        internal OmmConsumerConfigImpl OmmConsConfigImpl;
        internal string m_configPath = string.Empty;

        /// <summary>
        /// Defines operation model for dispatching.
        /// </summary>
        public enum OperationModelMode
        {
            /// <summary>
            /// Specifies callbacks happen on user thread of control
            /// </summary>
            USER_DISPATCH = 1,

            /// <summary>
            /// Specifies callbacks happen on API thread of control
            /// </summary>
            API_DISPATCH = 2
        }

        /// <summary>
        /// Creates an OmmConsumerConfig that enables customization of default implicit administrative
        /// domains and local configuration.
        /// </summary>
        public OmmConsumerConfig()
        {
            m_configPath = string.Empty;
            OmmConsConfigImpl = new OmmConsumerConfigImpl(m_configPath);
        }

        /// <summary>
        /// Creates an OmmConsumerConfig that enables customization of default implicit administrative
        /// domains and local configuration.
        /// </summary>
        /// <remarks>
        /// Path is optional. If not specified, application will use EmaConfig.xml (if any) found in current working directory of the application.
        /// </remarks>
        /// <param name="path">specifies configuration file name or name of directory containing a file
        /// named EmaConfig.xml</param>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the Xml file is malformed or if there is a parsing error.</exception>

        public OmmConsumerConfig(string path)
        {
            m_configPath = path;
            OmmConsConfigImpl = new OmmConsumerConfigImpl(path);
        }

        /// <summary>
        /// Clears the OmmConsumerConfig and sets all the defaults.
        /// </summary>
        /// <remarks>
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </remarks>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <remarks>
        /// This method loads the EMA configuration file from the current working directory or specified in <see cref="OmmConsumerConfig.OmmConsumerConfig(string)"/>
        /// </remarks>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the Xml file is malformed or if there is a parsing error.</exception>
        public OmmConsumerConfig Clear()
        {
            OmmConsConfigImpl.Clear();
            OmmConsConfigImpl = new OmmConsumerConfigImpl(m_configPath);
            return this;
        }

        /// <summary>
        /// Specifies an OMM Map encoded configuration that will override and add to the current configuration.
        /// </summary>
        /// <param name="configMap">specifies the OMM Map encoded configuration</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the map is malformed or if there is a parsing error.</exception>

        public OmmConsumerConfig Config(Map configMap)
        {
            OmmConsConfigImpl.Config(configMap);
            return this;
        }

        /// <summary>
        /// Specifies the name of the configured Consumer in either XML or Programmatic configuration. If present, overrides the DefaultConsumer configuration.
        /// </summary>
        /// <param name="consumerName">specifies the consumer name</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ConsumerName(string consumerName)
        {
            OmmConsConfigImpl.ConsumerName = consumerName;
            return this;
        }

        /// <summary>
        /// Specifies the username. Overrides the value specified in Login domain via the <see cref="AddAdminMsg"/> method.
        /// </summary>
        /// <param name="userName">specifies name used on login request</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig UserName(string userName)
        {
            OmmConsConfigImpl.UserName = userName;
            return this;
        }

        /// <summary>
        /// Specifies the password. Overrides the value specified in Login domain via the <see cref="AddAdminMsg"/> method.
        /// </summary>
        /// <param name="password">specifies name used on login request</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig Password(string password)
        {
            OmmConsConfigImpl.Password = password;
            return this;
        }

        /// <summary>
        /// Specifies the Position. Overrides the value specified in Login domain via the <see cref="AddAdminMsg"/> method.
        /// </summary>
        /// <param name="position">specifies the position used on login request</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig Position(string position)
        {
            OmmConsConfigImpl.Position = position;
            return this;
        }

        /// <summary>
        /// Specifies the authorization application identifier. Must be unique for each application.
        /// Range 257 to 65535 is available for site-specific use.Range 1 to 256 is reserved.
        /// Overrides the value specified in Login domain via the <see cref="AddAdminMsg"/> method.
        /// </summary>
        /// <param name="applicationId">specifies the application ID used on login request</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ApplicationId(string applicationId)
        {
            OmmConsConfigImpl.ApplicationId = applicationId;
            return this;
        }

        /// <summary>
        /// Specifies the clientID used for RDP token service. Mandatory, used to specify the Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
        /// </summary>
        /// <param name="clientId">specifies the Client Id</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ClientId(string clientId)
        {
            OmmConsConfigImpl.ClientId = clientId;
            return this;
        }

        /// <summary>
        /// Specifies the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
        /// </summary>
        /// <param name="clientSecret">specifies the client secret</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ClientSecret(string clientSecret)
        {
            OmmConsConfigImpl.ClientSecret = clientSecret;
            return this;
        }

        /// <summary>
        /// Specifies the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins
        /// </summary>
        /// <param name="clientJwk">specifies the client jwk string</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ClientJwk(string clientJwk)
        {
            OmmConsConfigImpl.ClientJwk = clientJwk;
            return this;
        }

        /// <summary>
        /// Specifies the Audience for the JWT. The JWT is used to authenticate with the RDP token service. Optional and only used for V2 logins with client JWT logins
        /// </summary>
        /// <param name="audience">specifies the client audience string</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig Audience(string audience)
        {
            OmmConsConfigImpl.Audience = audience;
            return this;
        }

        /// <summary>
        /// Specifies the TokenScope for the RDP token service REST request. Optional.
        /// </summary>
        /// <param name="tokenScope">specifies the client audience string</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig TokenScope(string tokenScope)
        {
            OmmConsConfigImpl.TokenScope = tokenScope;
            return this;
        }

        /// <summary>
        /// Specifies URL of RDP Token Service V2. Optional.
        /// </summary>
        /// <param name="tokenUrlV2">specifies the Token service URL</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig TokenUrlV2(string tokenUrlV2)
        {
            OmmConsConfigImpl.TokenUrlV2 = tokenUrlV2;
            return this;
        }

        /// <summary>
        /// Specifies URL of RDP Service Discovery. Optional.
        /// </summary>
        /// <param name="serviceDiscoveryUrl">specifies the Service Discovery URL</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ServiceDiscoveryUrl(string serviceDiscoveryUrl)
        {
            OmmConsConfigImpl.ServiceDiscoveryUrl = serviceDiscoveryUrl;
            return this;
        }

        /// <summary>
        /// Specifies the Host name and port that the OmmConsumer will connect to.
        /// This implies a TCP connection of <see cref="ConnectionType.SOCKET"/> type.
        /// </summary>
        /// <remarks>
        /// The host string uses the following format:
        /// If host is set to &#34;&lt;HostName&gt;:&lt;Port&gt;&#34;, then HostName:Port will be set<br/>
        /// If host is set to &#34;&#34;, then localhost:14002 will be set<br/>
        /// If host is set to &#34;:&#34;, then localhost:14002 will be set<br/>
        /// If host is set to &#34;&lt;HostName&gt;&#34;, then HostName:14002 will be set<br/>
        /// If host is set to &#34;&lt;HostName:&gt;&#34;, then HostName:14002 will be set<br/>
        /// If host is set to &#34;&lt;:Port&gt;&#34;, then localhost:Port will be set<br/>
        /// </remarks>
        /// <param name="host">Host string that </param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the string is not formatted as above.</exception>
        public OmmConsumerConfig Host(string host = "localhost:14002")
        {
            OmmConsConfigImpl.Host(host);
            return this;
        }

        /// <summary>
        /// Specifies a connection type to override EMA file and programmatic configuration.
        /// </summary>
        /// <param name="channelType">specifies a connection type. Connection type defined in <see cref="EmaConfig.ConnectionTypeEnum"/></param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if channelType is not valid.</exception>
        public OmmConsumerConfig ChannelType(int channelType)
        {
            if (channelType != EmaConfig.ConnectionTypeEnum.SOCKET &&
                channelType != EmaConfig.ConnectionTypeEnum.ENCRYPTED)
            {
                throw new OmmInvalidUsageException($"Try to pass invalid argument: {channelType.ToString()} to ChannelType(). " +
					$"Please use channel types present in EmaConfig.ConnectionTypeEnum.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            OmmConsConfigImpl.ChannelType((ConnectionType)channelType);
            return this;
        }

        /// <summary>
        /// Specifies an encrypted protocol type to override EMA file and programmatic configuration.
        /// </summary>
        /// <param name="encProtocolType">specifies encrypted protocol type used by application. Encrypted protocol type defined in <see cref="EmaConfig.EncryptedProtocolTypeEnum"/></param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if call this method with not encrypted connection type.</exception>
        /// <exception cref="OmmInvalidUsageException">Thrown if encProtocolType is not valid.</exception>
        public OmmConsumerConfig EncryptedProtocolType(int encProtocolType)
        {
            if (encProtocolType != EmaConfig.EncryptedProtocolTypeEnum.SOCKET)
            {
                throw new OmmInvalidUsageException($"Try to pass invalid argument: {encProtocolType.ToString()} to EncryptedProtocolType(). " +
                    $"Please use channel types present in EmaConfig.EncryptedProtocolTypeEnum.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            OmmConsConfigImpl.EncryptedProtocolType((ConnectionType)encProtocolType);
            return this;
        }

        /// <summary>
        /// Specifies the proxy host.
        /// </summary>
        /// <param name="proxyHost">specifies proxy host</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ProxyHost(string proxyHost)
        {
            OmmConsConfigImpl.ProxyHost = proxyHost;
            return this;
        }

        /// <summary>
        /// Specifies the proxy port for all connections.
        /// </summary>
        /// <param name="proxyPort">specifies proxy port</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ProxyPort(string proxyPort)
        {
            OmmConsConfigImpl.ProxyPort = proxyPort;
            return this;
        }

        /// <summary>
        /// Specifies the proxy user name.
        /// </summary>
        /// <param name="proxyUserName">specifies proxy username</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ProxyUserName(string proxyUserName)
        {
            OmmConsConfigImpl.ProxyUserName = proxyUserName;
            return this;
        }

        /// <summary>
        /// Specifies the proxy password.
        /// </summary>
        /// <param name="proxyPassword">specifies proxy passwordt</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig ProxyPassword(string proxyPassword)
        {
            OmmConsConfigImpl.ProxyPassword = proxyPassword;
            return this;
        }

        /// <summary>
        /// Specifies the REST proxy host.
        /// </summary>
        /// <param name="restProxyHostName">specifies proxy host name</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig RestProxyHostName(string restProxyHostName)
        {
            OmmConsConfigImpl.RestProxyHostName = restProxyHostName;
            return this;
        }

        /// <summary>
        /// Specifies the REST proxy port for all connections.
        /// </summary>
        /// <param name="restProxyPort">specifies proxy port</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig RestProxyPort(string restProxyPort)
        {
            OmmConsConfigImpl.RestProxyPort = restProxyPort;
            return this;
        }

        /// <summary>
        /// Specifies the REST proxy user name.
        /// </summary>
        /// <param name="restProxyUserName">specifies proxy username</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig RestProxyUserName(string restProxyUserName)
        {
            OmmConsConfigImpl.RestProxyUserName = restProxyUserName;
            return this;
        }

        /// <summary>
        /// Specifies the REST proxy password.
        /// </summary>
        /// <param name="restProxyPassword">specifies proxy password</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig RestProxyPassword(string restProxyPassword)
        {
            OmmConsConfigImpl.RestProxyPassword = restProxyPassword;
            return this;
        }

        /// <summary>
        /// Specifies the encrypted protocol flags.
        /// </summary>
        /// <param name="protocolFlags">the TLS encryption protocol versions 
        /// defined in <see cref="EmaConfig.EncryptedTLSProtocolFlags"/></param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig EncryptedProtocolFlags(uint protocolFlags)
        {
            OmmConsConfigImpl.EncryptedProtocolFlags(protocolFlags);
            return this;
        }

        /// <summary>
        /// Specifies the collection of cipher suites allowed for TLS negotiation.
        /// </summary>
        /// <param name="cipherSuites">The collection of <c>TlsCipherSuite</c></param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig TlsCipherSuites(IEnumerable<TlsCipherSuite> cipherSuites)
        {
            OmmConsConfigImpl.TlsCipherSuites(cipherSuites);
            return this;
        }

        /// <summary>
        /// Specifies the operation model, overriding the default. The operation model specifies whether
        /// to dispatch messages in the user or application thread of control.
        /// </summary>
        /// <param name="operationModel">specifies threading and dispatching model used by application</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        public OmmConsumerConfig OperationModel(OperationModelMode operationModel)
        {
            OmmConsConfigImpl.OperationModel((int)operationModel);
            return this;
        }

        /// <summary>
        /// Specifies an administrative request message to override the default administrative request.
        /// Application may call multiple times prior to initialization. Supported domains include Login,
        /// Directory, and Dictionary.
        /// </summary>
        /// <param name="requestMsg">specifies administrative domain request message</param>
        /// <returns>Reference to current <see cref="OmmConsumerConfig"/> object.</returns>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the request message is malformed or if there is a parsing error.</exception>
        public OmmConsumerConfig AddAdminMsg(RequestMsg requestMsg)
        {
            OmmConsConfigImpl.AddAdminMsg(requestMsg);
            return this;
        }

        /// <summary>
        /// Specifies the DataDictionary object.
        /// </summary>
        ///
        /// <remarks>
        /// <para>
        /// Overrides DataDictionary object that is provided via EmaConfig.xml or
        /// Programmatic configure.</para>
        ///
        /// <para>
        /// If shouldCopyIntoAPI is true, the DataDictionary object will be copied
        /// into the application space, otherwise it will be passed in as a reference.</para>
        /// </remarks>
        ///
        /// <param name="dataDictionary"> specifies the DataDictionary object.</param>
        /// <param name="shouldCopyIntoAPI"> specifies whether to copy dataDictionary
        ///     into API or pass in as reference.</param>
        ///
        /// <returns> reference to this object.</returns>
        ///
        public OmmConsumerConfig DataDictionary(Ema.Rdm.DataDictionary dataDictionary, bool shouldCopyIntoAPI = false)
        {
            OmmConsConfigImpl.DataDictionary(dataDictionary, shouldCopyIntoAPI);
            return this;
        }
    }
}
