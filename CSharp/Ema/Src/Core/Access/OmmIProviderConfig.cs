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
    /// OmmIProviderConfig is used to modify configuration and behavior of OmmProvider
    /// for interactive application.
    /// </summary>
    /// <remarks>
    /// <para>
    /// OmmIProviderConfig provides a default basic OmmProvider configuration.
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
    /// Calling any interface methods of OmmIProviderConfig class overrides or appends the existing<br/>
    /// configuration.
    /// </para>
    /// </remarks>
    /// <seealso cref="OmmProviderConfig"/>
    /// <seealso cref="OmmProvider"/>
    public class OmmIProviderConfig : OmmProviderConfig
    {
        internal OmmIProviderConfigImpl OmmIProvConfigImpl;
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
        /// Defines admin control for handling directory and dictionary refresh message.
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
        /// Creates OmmIProviderConfig
        /// </summary>
        public OmmIProviderConfig()
        {
            m_configPath = string.Empty;
            OmmIProvConfigImpl = new OmmIProviderConfigImpl(m_configPath);
        }

        /// <summary>
        /// Creates OmmIProviderConfig
        /// </summary>
        /// <param name="path">specifies configuration file name or name of directory containing a file named EmaConfig.xml.<br/>
        /// If path is null or empty, application will use EmaConfig.xml (if any) found in the current working directory.
        /// </param>
        public OmmIProviderConfig(string path)
        {
            m_configPath = path;
            OmmIProvConfigImpl = new OmmIProviderConfigImpl(m_configPath);
        }

        /// <summary>
        /// <inheritdoc/>
        /// </summary>
        public override ProviderRoleEnum ProviderRole => OmmProviderConfig.ProviderRoleEnum.INTERACTIVE;

        /// <summary>
        /// Clears the <see cref="OmmIProviderConfig"/> and sets all the defaults
        /// </summary>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        /// <remarks>
        /// This method loads the EMA configuration file from the current working directory or specified in <see cref="OmmIProviderConfig(string)"/>
        /// </remarks>
        /// <exception cref="OmmInvalidConfigurationException">Thrown if the Xml file is malformed or if there is a parsing error.</exception>
        public OmmIProviderConfig Clear()
        {
            OmmIProvConfigImpl = new OmmIProviderConfigImpl(m_configPath);
            return this;
        }

        /// <summary>
        /// Specifies the name of the configured Provider in either XML or Programmatic configuration. 
        /// If present, overrides the DefaultNiProvider configuration for non-interactive provider.
        /// </summary>
        /// <param name="providerName">specifies name of <see cref="OmmProvider"/> instance</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig ProviderName(string providerName)
        {
            OmmIProvConfigImpl.ProviderName(providerName);
            return this;
        }

        /// <summary>
        /// Specifies a server port for accepting client connections.
        /// </summary>
        /// <param name="serverPort">specifies server port on which OmmProvider will accept client connections</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig Port(string serverPort = "14002")
        {
            OmmIProvConfigImpl.Port(serverPort);
            return this;
        }

        /// <summary>
        /// Specifies the operation model, overriding the default.<br/>
        /// The operation model specifies whether to dispatch messages
        /// in the user or application thread of control.
        /// </summary>
        /// <param name="operationModel">specifies threading and dispatching model used by application</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig OperationModel(OperationModelMode operationModel)
        {
            OmmIProvConfigImpl.OperationModel((int)operationModel);
            return this;
        }

        /// <summary>
        /// Specifies whether API or user controls sending of Directory refresh message.
        /// </summary>
        /// <param name="adminControl">specifies who sends down the directory refresh message</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig AdminControlDirectory(AdminControlMode adminControl)
        {
            OmmIProvConfigImpl.AdminControlDirectory = adminControl;
            return this;
        }

        /// <summary>
        /// Specifies whether API or user controls responding to dictionary requests.
        /// </summary>
        /// <param name="adminControl">specifies specifies who responds to dictioanry requests</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig AdminControlDictionary(AdminControlMode adminControl)
        {
            OmmIProvConfigImpl.AdminControlDictionary = adminControl;
            return this;
        }

        /// <summary>
        /// Specifies the programmatic configuration, overriding and adding to the current content.
        /// </summary>
        /// <param name="config">specifies OmmIProvider configuration via OMM data type</param>
        /// <returns>reference to this object</returns>
        public OmmIProviderConfig Config(Data config)
        {
            OmmIProvConfigImpl.Config((Map)config);
            return this;
        }

        /// <summary>
        /// Specifies the location of the server certificate file for encrypted providers.
        /// </summary>
        /// <param name="serverCert">specifies the name of the server certificate file</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig ServerCertificate(string serverCert)
        {
            OmmIProvConfigImpl.ServerCertificate(serverCert);
            return this;
        }

        /// <summary>
        /// Specifies the location of the private key file for encrypted providers.
        /// </summary>
        /// <param name="privateKey">specifies the name of the private key file</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig ServerPrivateKey(string privateKey)
        {
            OmmIProvConfigImpl.ServerPrivateKey(privateKey);
            return this;
        }

        /// <summary>
        /// Specifies the encrypted protocol flags.
        /// </summary>
        /// <param name="protocolFlags">the TLS encryption protocol versions 
        /// defined in <see cref="EmaConfig.EncryptedTLSProtocolFlags"/></param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig EncryptedProtocolFlags(uint protocolFlags)
        {
            OmmIProvConfigImpl.EncryptedProtocolFlags(protocolFlags);
            return this;
        }

        /// <summary>
        /// Specifies the collection of cipher suites allowed for TLS negotiation.
        /// </summary>
        /// <param name="cipherSuites">The collection of <c>TlsCipherSuite</c></param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig TlsCipherSuites(IEnumerable<TlsCipherSuite> cipherSuites)
        {
            OmmIProvConfigImpl.TlsCipherSuites(cipherSuites);
            return this;
        }

        /// <summary>
        /// Specifies an administrative refresh message to override the default administrative refresh.
        /// Application may call multiple times prior to initialization. Supports the Directory domain only.
        /// </summary>
        /// <param name="refreshMsg">specifies administrative domain refresh message</param>
        /// <returns>Reference to current <see cref="OmmIProviderConfig"/> object.</returns>
        public OmmIProviderConfig AddAdminMsg(RefreshMsg refreshMsg)
        {
            OmmIProvConfigImpl.AddAdminMsg(refreshMsg);
            return this;
        }
    }
}
