/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using System.Net.Security;

namespace LSEG.Ema.Access
{
    internal abstract class OmmConfigBaseImpl
    {
        internal enum OperationModelMode
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

        // Hostname config from OmmNiProviderConfig methods
        internal string HostName = string.Empty;
        // Port config from OmmNiProviderConfig methods
        internal string Port = string.Empty;
        // UserName config from OmmNiProviderConfig methods. This is used for the login requests if session management is turned off for the connection
        internal string UserName = string.Empty;
        // Password config from OmmNiProviderConfig methods. This is used for the login requests if session management is turned off for the connection
        internal string Password = string.Empty;
        // Position config from OmmNiProviderConfig methods. 
        internal string Position { get; set; } = string.Empty;

        internal string ApplicationName { get; set; } = string.Empty;
        // ApplicationId config from OmmNiProviderConfig methods. 
        internal string ApplicationId { get; set; } = string.Empty;
        // Instance Id config from OmmNiProviderConfig methods
        internal string InstanceId { get; set; } = string.Empty;
        // ProxyHost config from OmmConsumerConfig methods
        internal string ProxyHost { get; set; } = string.Empty;
        // ProxyPort config from OmmConsumerConfig methods
        internal string ProxyPort { get; set; } = string.Empty;
        // ProxyUserName config from OmmConsumerConfig methods
        internal string ProxyUserName { get; set; } = string.Empty;
        // ProxyPassword config from OmmConsumerConfig methods
        internal string ProxyPassword { get; set; } = string.Empty;
        // ChannleType config from OmmConsumerConfig methods
        internal ConnectionType ChanType { get; set; } = ConnectionType.UNIDENTIFIED;
        // EncProtocolType config from OmmConsumerConfig methods
        internal ConnectionType EncProtocolType { get; set; } = ConnectionType.UNIDENTIFIED;
        // DispatchModel config from OmmConsumerConfig methods.
        internal int DispatchModel;

        // Path of the Xml configuration file. If not specified when the OmmConsumerConfig
        // is created, this will default to "EmaConfig.xml"
        internal string XmlConfigPath { get; set; } = string.Empty;

        internal LoggerConfig LoggerConfig { get; set; } = new LoggerConfig();

        internal Dictionary<string, ClientChannelConfig> ClientChannelConfigMap { get; set; }

        internal Dictionary<string, LoggerConfig> LoggerConfigMap { get; set; }

        // Dictionary tables indexed by the name of the config.
        internal Dictionary<string, DictionaryConfig> DictionaryConfigMap { get; set; }

        internal ConfigErrorList? ConfigErrorLog { get; set; } = null;

        internal LoginRequest AdminLoginRequest { get; set; } = new LoginRequest();

        internal bool SetAdminLoginRequest { get; set; } = false;

        internal bool SetEncryptedProtocolFlags { get; set; } = false;

        internal uint EncryptedTLSProtocolFlags { get; set; } = EmaConfig.EncryptedTLSProtocolFlags.NONE;

        internal IEnumerable<TlsCipherSuite>? CipherSuites { get; set; } = null;

        internal OmmConfigBaseImpl()
        {
            ClientChannelConfigMap = new Dictionary<string, ClientChannelConfig>();
            LoggerConfigMap = new Dictionary<string, LoggerConfig>();
            DictionaryConfigMap = new Dictionary<string, DictionaryConfig>();

            Clear();
        }

        void Clear()
        {
            HostName = string.Empty;
            Port = string.Empty;
            DispatchModel = (int)OmmConfigBaseImpl.OperationModelMode.API_DISPATCH;
            XmlConfigPath = string.Empty;
            UserName = string.Empty;
            Password = string.Empty;
            Position = string.Empty;
            ApplicationId = string.Empty;
            ApplicationName = "ema";
            ProxyHost = string.Empty;
            ProxyPort = string.Empty;
            ProxyUserName = string.Empty;
            ProxyPassword = string.Empty;
            ClientChannelConfigMap.Clear();
            LoggerConfigMap.Clear();
            DictionaryConfigMap.Clear();
            LoggerConfig.Clear();
            ConfigErrorLog?.Clear();
            ChanType = ConnectionType.UNIDENTIFIED;
            EncProtocolType = ConnectionType.UNIDENTIFIED;
        }

        // Generates the reactor connect options based on the Consumer Config.
        // Prerequsites: The user-supplied OmmConsumerConfig has been verified with OmmConsumerConfigImpl.VerifyConfig and this is run on the "active" configuration copy
        public abstract ReactorConnectOptions GenerateReactorConnectOpts();

        internal void TlsCipherSuites(IEnumerable<TlsCipherSuite> cipherSuites)
        {
            CipherSuites = cipherSuites;
        }

        internal void EncryptedProtocolFlags(uint protocolFlags)
        {
            SetEncryptedProtocolFlags = true;
            EncryptedTLSProtocolFlags = protocolFlags;
        }

        internal void ChannelType(ConnectionType channelType)
        {
            ChanType = channelType;
        }
        internal void EncryptedProtocolType(ConnectionType encProtocolType)
        {
            EncProtocolType = encProtocolType;
        }
    }
}
