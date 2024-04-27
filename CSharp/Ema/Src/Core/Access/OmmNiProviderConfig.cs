/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Net.Security;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmNiProviderConfig is used to modify configuration and behavior of OmmProvider
    /// for non-interactive application
    /// </summary>
    /// <remarks>
    /// <para>
    /// OmmNiProviderConfig provides a default basic OmmProvider configuration.
    /// </para>
    /// <para>
    /// The default configuration may be modified and or appended by using the EmaConfig.xml<br/>
    /// file or any methods of this class.
    /// </para>
    /// <para>
    /// For encrypted connection, ChannelType::RSSL_ENCRYPTED must be configured in Ema configuration<br/>
    /// file such as EmaConfig.xml or programmatic configuration database.
    /// </para>
    /// <para>
    /// The EmaConfig.xml file is read in if it is present in the working directory of the application.
    /// </para>
    /// <para>
    /// Calling any interface methods of OmmNiProviderConfig class overrides or appends the existing<br/>
    /// configuration.
    /// </para>
    /// </remarks>
    /// <seealso cref="OmmProviderConfig"/>
    /// <seealso cref="OmmProvider"/>
    public sealed class OmmNiProviderConfig : OmmProviderConfig
    {
        internal OmmNiProviderConfigImpl OmmNiProvConfigImpl;
        private string m_configPath = string.Empty;

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
        /// Defines admin control for handling directory refresh message.
        /// </summary>
        public enum AdminControlMode
        {
            /// <summary>
            /// Specifies user submit directory refresh message
            /// </summary>
            USER_CONTROL = 1,

            /// <summary>
            /// Specifies API sends down directory refresh message based on the configuration
            /// </summary>
            API_CONTROL = 2
        }

        /// <summary>
        /// Creates OmmNiProviderConfig
        /// </summary>
        public OmmNiProviderConfig()
        {
            m_configPath = string.Empty;
            OmmNiProvConfigImpl = new OmmNiProviderConfigImpl(m_configPath);
        }

        /// <summary>
        /// Creates OmmNiProviderConfig
        /// </summary>
        /// <param name="path">specifies configuration file name or name of directory containing a file named EmaConfig.xml.<br/>
        /// If path is null or empty, application will use EmaConfig.xml (if any) found in the current working directory.
        /// </param>
        public OmmNiProviderConfig(string path)
        {
            m_configPath = path;
            OmmNiProvConfigImpl = new OmmNiProviderConfigImpl(m_configPath);
        }

        /// <summary>
        /// <inheritdoc/>
        /// </summary>
        public override ProviderRoleEnum ProviderRole {
            get => ProviderRoleEnum.NON_INTERACTIVE;
        }

        /// <summary>
        /// Clears the <see cref="OmmNiProviderConfig"/> and sets all the defaults
        /// </summary>
        /// <returns>reference to this object</returns>
        /// <remarks>
        /// This method loads the EMA configuration file from the current working directory or specified in <see cref="OmmNiProviderConfig(string)"/>
        /// </remarks>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the Xml file is malformed or if there is a parsing error.</exception>
        public OmmNiProviderConfig Clear()
        {
            OmmNiProvConfigImpl = new OmmNiProviderConfigImpl(m_configPath);
            return this;
        }

        /// <summary>
        /// Specifies the name of the configured Provider in either XML or Programmatic configuration. 
        /// If present, overrides the DefaultNiProvider configuration for non-interactive provider.
        /// </summary>
        /// <param name="providerName">specifies name of <see cref="OmmProvider"/> instance</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ProviderName(string providerName)
        {
            OmmNiProvConfigImpl.NiProviderName = providerName;
            return this;
        }

        /// <summary>
        /// Specifies the username. Overrides a value specified in Login domain via the AddAdminMsg(..) method.
        /// </summary>
        /// <param name="userName">specifies name used on login request</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig UserName(string userName)
        {
            OmmNiProvConfigImpl.UserName = userName;
            return this;
        }

        /// <summary>
        /// Specifies the password. Overrides a value specified in Login domain via the AddAdminMsg(..) method.
        /// </summary>
        /// <param name="password">specifies name used on login request</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig Password(string password)
        {
            OmmNiProvConfigImpl.Password = password;
            return this;
        }

        /// <summary>
        /// Specifies the Position
        /// </summary>
        /// <param name="position">specifies the position used on login request</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig Position(string position)
        {
            OmmNiProvConfigImpl.Position = position;
            return this;
        }

        /// <summary>
        /// Specifies the ApplicationId
        /// </summary>
        /// <param name="applicationId">specifies the application ID used on login request</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ApplicationId(string applicationId)
        {
            OmmNiProvConfigImpl.ApplicationId = applicationId;
            return this;
        }

        /// <summary>
        /// Specifies the instance identifier. Can be any ASCII string, e.g. "Instance1".
        /// Used to differentiate applications running on the same client host.
        /// </summary>
        /// <param name="instanceId">specifies respective login request attribute</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig InstanceId(string instanceId)
        {
            OmmNiProvConfigImpl.InstanceId = instanceId;
            return this;
        }

        /// <summary>
        /// Specifies the Host name and port that the OmmProvider will connect to. 
        /// This implies a TCP connection of RSSL_SOCKET type.
        /// </summary>
        /// <param name="host">specifies server and port to which OmmProvider will connect.</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig Host(string host = "localhost:14003")
        {
            OmmNiProvConfigImpl.Host(host);
            return this;
        }

        /// <summary>
        /// Specifies the operation model, overriding the default.<br/>
        /// The operation model specifies whether to dispatch messages
        /// in the user or application thread of control.
        /// </summary>
        /// <param name="operationModel">specifies threading and dispatching model used by application</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig OperationModel(OperationModelMode operationModel)
        {
            OmmNiProvConfigImpl.OperationModel((int)operationModel);
            return this;
        }

        /// <summary>
        /// Specifies whether API or user controls sending of Directory refresh message.
        /// </summary>
        /// <param name="adminControl">specifies who sends down the directory refresh message</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig AdminControlDirectory(AdminControlMode adminControl)
        {
            OmmNiProvConfigImpl.AdminControlDirectory = adminControl;
            return this;
        }

        /// <summary>
        /// Specifies the proxy server host.
        /// </summary>
        /// <param name="proxyHost">specifies proxy host</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ProxyHost(string proxyHost)
        {
            OmmNiProvConfigImpl.ProxyHost = proxyHost;
            return this;
        }

        /// <summary>
        /// Specifies the proxy server port for all connections.
        /// </summary>
        /// <param name="proxyPort">specifies proxy port</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ProxyPort(string proxyPort)
        {
            OmmNiProvConfigImpl.ProxyPort = proxyPort;
            return this;
        }

        /// <summary>
        /// Specifies the proxy user name.
        /// </summary>
        /// <param name="proxyUserName">specifies proxy username</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ProxyUserName(string proxyUserName)
        {
            OmmNiProvConfigImpl.ProxyUserName = proxyUserName;
            return this;
        }

        /// <summary>
        /// Specifies the proxy password.
        /// </summary>
        /// <param name="proxyPassword">specifies proxy passwordt</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig ProxyPassword(string proxyPassword)
        {
            OmmNiProvConfigImpl.ProxyPassword = proxyPassword;
            return this;
        }

        /// <summary>
        /// Specifies the encrypted protocol flags.
        /// </summary>
        /// <param name="protocolFlags">the TLS encryption protocol versions 
        /// defined in <see cref="EmaConfig.EncryptedTLSProtocolFlags"/></param>
        /// <returns>Reference to current <see cref="OmmNiProviderConfig"/> object.</returns>
        public OmmNiProviderConfig EncryptedProtocolFlags(uint protocolFlags)
        {
            OmmNiProvConfigImpl.EncryptedProtocolFlags(protocolFlags);
            return this;
        }

        /// <summary>
        /// Specifies the collection of cipher suites allowed for TLS negotiation.
        /// </summary>
        /// <param name="cipherSuites">The collection of <c>TlsCipherSuite</c></param>
        /// <returns>Reference to current <see cref="OmmNiProviderConfig"/> object.</returns>
        public OmmNiProviderConfig TlsCipherSuites(IEnumerable<TlsCipherSuite> cipherSuites)
        {
            OmmNiProvConfigImpl.TlsCipherSuites(cipherSuites);
            return this;
        }

        /// <summary>
        /// Specifies the programmatic configuration, overriding and adding to the current content.
        /// </summary>
        /// <param name="config">specifies OmmNiProvider configuration via OMM data type</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig Config(Data config)
        {
            OmmNiProvConfigImpl.Config((Map)config);
            return this;
        }

        /// <summary>
        /// Specifies an administrative request message to override the default administrative request.<br/>
        /// Application may call multiple times prior to initialization. Supports Login domain only.
        /// </summary>
        /// <param name="requestMsg">specifies administrative domain request message</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig AddAdminMsg(RequestMsg requestMsg)
        {
            OmmNiProvConfigImpl.AddAdminMsg(requestMsg);
            return this;
        }

        /// <summary>
        /// Specifies an administrative refresh message to override the default administrative refresh.<br/>
        /// Application may call multiple times prior to initialization.<br/>
        /// Supports Directory domain only.
        /// </summary>
        /// <param name="refreshMsg">specifies administrative domain refresh message</param>
        /// <returns>reference to this object</returns>
        public OmmNiProviderConfig AddAdminMsg(RefreshMsg refreshMsg)
        {
            OmmNiProvConfigImpl.AddAdminMsg(refreshMsg);
            return this;
        }
    }
}
