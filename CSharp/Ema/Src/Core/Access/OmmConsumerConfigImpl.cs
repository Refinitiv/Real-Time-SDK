/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using Microsoft.IdentityModel.Tokens;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using static LSEG.Eta.Rdm.Directory;

using System.IO;
using System.Net.Security;

namespace LSEG.Ema.Access
{
    internal class OmmConsumerConfigImpl : OmmConfigBaseImpl
    {
        // ClientId config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string ClientId { get; set; } = string.Empty;
        // ClientSecret config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string ClientSecret { get; set; } = string.Empty;
        // ClientJwk config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string ClientJwk { get; set; } = string.Empty;
        // Audience config from OmmConsumerConfig methods. This is used to get access tokens from RDP with JWT credentials
        internal string Audience { get; set; } = string.Empty;
        // TokenScope config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string TokenScope { get; set; } = string.Empty;
        // Token URL V2 config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string TokenUrlV2 { get; set; } = string.Empty;
        // Service Discovery URL config from OmmConsumerConfig methods. This is used to get access tokens from RDP
        internal string ServiceDiscoveryUrl { get; set; } = string.Empty;
        // ConsumerName configuration from the OmmConsumerConfigMethods
        // For the copied OmmConsumerConfigImpl used in OmmBaseImpl, this contains the name of the consumer that the OmmConsumer is connected to
        internal string ConsumerName { get; set; } = string.Empty;
        internal string RestProxyHostName { get; set; } = string.Empty;
        // RestProxyPort config from OmmConsumerConfig methods
        internal string RestProxyPort { get; set; } = string.Empty;
        // RestProxyUserName config from OmmConsumerConfig methods
        internal string RestProxyUserName { get; set; } = string.Empty;
        // RestProxyPassword config from OmmConsumerConfig methods
        internal string RestProxyPassword { get; set; } = string.Empty;

        // Dictionary tables indexed by the name of the config.
        internal Dictionary<string, ConsumerConfig> ConsumerConfigMap { get; set; }

        // Xml parser class.  This is not used with the "active" configuration in OmmBaseImpl
        internal XmlConfigParser? XmlParser { get; set; }

        // Default consumer configured from the Xml.  If Empty, this will be either the first configured consumer, or if no consumers are configured, the default consumer.
        internal string DefaultConsumer { get; set; } = string.Empty;

        // Name of the first configured Consumer, because .net Dictionaries may not keep everything in order of insertion

        internal string FirstConfiguredConsumerName { get; set; } = string.Empty;

        // Internal reference to the consumer, dictionary, and logger configuration used as the "active" configuration, once copied to the OmmBaseImpl.
        internal ConsumerConfig ConsumerConfig { get; set; } = new ConsumerConfig();
        internal DictionaryConfig DictionaryConfig { get; set; } = new DictionaryConfig();
        internal DirectoryRequest? AdminDirectoryRequest { get; set; } = null;
        internal DictionaryRequest? AdminFieldDictionaryRequest { get; set; } = null;
        internal DictionaryRequest? AdminEnumDictionaryRequest { get; set; } = null;
        internal string FieldDictionaryRequestServiceName { get; set; } = string.Empty;
        internal string EnumDictionaryRequestServiceName { get; set; } = string.Empty;

        private const string DefaultHost = "localhost";
        private const string DefaultPort = "14002";

        private static readonly HostPortParser hostPortParser = new (DefaultHost, DefaultPort);

        // Default constructor
        internal OmmConsumerConfigImpl(string? path)
            : base()
        {
            ConsumerConfigMap = new Dictionary<string, ConsumerConfig>();
          
            // The error log will only be used by a user-created object, not by the internal EMA cache.
            ConfigErrorLog = new ConfigErrorList();

            Clear();

            if (!string.IsNullOrEmpty(path))
                XmlConfigPath = path;

            XmlParser = new XmlConfigParser(this);

            // Overwrite the ApplicationName to "ema"
            AdminLoginRequest.LoginAttrib.ApplicationName.Data("ema");
        }

        // Copy Constructor that will be used in OmmBaseImpl.  This will contain only the information needed by EMA to generate
        // ReactorConnectOptions and the ReactorRole.  It will not copy everything, just:
        // the configured required Consumer(in order: ConsuemerName, DefaultConsumer, the first Consumer in the consumer list, the default consumer)
        // any channels directly referenced by the consumer(or 
        // the dictionary referenced by the consumer(or default)
        // 
        // PREREQUSITES: OldConfigImpl has been verified with VerifyConfiguration()
        internal OmmConsumerConfigImpl(OmmConsumerConfigImpl OldConfigImpl)
            : base()
        {
            ConsumerConfigMap = new Dictionary<string, ConsumerConfig>();
            ClientChannelConfigMap = new Dictionary<string, ClientChannelConfig>();
            LoggerConfigMap = new Dictionary<string, LoggerConfig>();
            DictionaryConfigMap = new Dictionary<string, DictionaryConfig>();

            Clear();
            HostName = OldConfigImpl.HostName;
            Port = OldConfigImpl.Port;
            UserName = OldConfigImpl.UserName;
            Password = OldConfigImpl.Password;
            Position = OldConfigImpl.Position;
            ApplicationId = OldConfigImpl.ApplicationId;
            ClientId = OldConfigImpl.ClientId;
            ClientSecret = OldConfigImpl.ClientSecret;
            ClientJwk = OldConfigImpl.ClientJwk;
            TokenUrlV2 = OldConfigImpl.TokenUrlV2;
            ServiceDiscoveryUrl = OldConfigImpl.ServiceDiscoveryUrl;
            Audience = OldConfigImpl.Audience;
            TokenScope = OldConfigImpl.TokenScope;
            ProxyHost = OldConfigImpl.ProxyHost;
            ProxyPort = OldConfigImpl.ProxyPort;
            ProxyUserName = OldConfigImpl.ProxyUserName;
            ProxyPassword = OldConfigImpl.ProxyPassword;
            RestProxyHostName = OldConfigImpl.RestProxyHostName;
            RestProxyPort = OldConfigImpl.RestProxyPort;
            RestProxyUserName = OldConfigImpl.RestProxyUserName;
            RestProxyPassword = OldConfigImpl.RestProxyPassword;
            DispatchModel = OldConfigImpl.DispatchModel;
            OldConfigImpl.AdminLoginRequest.Copy(AdminLoginRequest);
            SetEncryptedProtocolFlags = OldConfigImpl.SetEncryptedProtocolFlags;
            EncryptedTLSProtocolFlags = OldConfigImpl.EncryptedTLSProtocolFlags;
            CipherSuites = OldConfigImpl.CipherSuites;
            ChanType = OldConfigImpl.ChanType;
            EncProtocolType = OldConfigImpl.EncProtocolType;

            if (OldConfigImpl.AdminDirectoryRequest != null)
            {
                AdminDirectoryRequest = new DirectoryRequest();
                OldConfigImpl.AdminDirectoryRequest.Copy(AdminDirectoryRequest);
            }

            if (OldConfigImpl.AdminFieldDictionaryRequest != null)
            {
                AdminFieldDictionaryRequest = new DictionaryRequest();
                OldConfigImpl.AdminFieldDictionaryRequest.Copy(AdminFieldDictionaryRequest);
            }

            if (OldConfigImpl.AdminEnumDictionaryRequest != null)
            {
                AdminEnumDictionaryRequest = new DictionaryRequest();
                OldConfigImpl.AdminEnumDictionaryRequest.Copy(AdminEnumDictionaryRequest);
            }

            ClientChannelConfig? tmpChannelConfig = null;

            ConsumerConfig.Clear();

            // If there aren't any configured consumers or the host and port have been specified, fall into the default case.  Otherwise,
            // check to see if the ConsumerName or DefaultConsumer have been defined, and deep copy the consumer to tmpConsumerConfig 
            if (OldConfigImpl.ConsumerConfigMap.Count > 0)
            {
                if (string.IsNullOrEmpty(OldConfigImpl.ConsumerName) == true)
                {
                    if (string.IsNullOrEmpty(OldConfigImpl.DefaultConsumer) == false)
                    {
                        ConsumerName = OldConfigImpl.DefaultConsumer;
                    }
                    else
                    {
                        ConsumerName = OldConfigImpl.FirstConfiguredConsumerName;
                    }
                }
                else
                {
                    ConsumerName = OldConfigImpl.ConsumerName;
                }

                OldConfigImpl.ConsumerConfigMap[ConsumerName].Copy(ConsumerConfig);

                // If the hostname and port are set, remove all other channels from the channel set.
                if(!string.IsNullOrEmpty(HostName) && !string.IsNullOrEmpty(Port) && ConsumerConfig.ChannelSet.Count > 1)
                {
                    ConsumerConfig.ChannelSet.RemoveRange(1, ConsumerConfig.ChannelSet.Count - 1);
                }
            }
            else
            {
                // Default consumer case where either nothing's configured or the host and port have been specified, we will override as necessary later on.
                // ConsumerConfig was cleared in the earlier Clear() call.
                ConsumerConfig.Name = "DefaultEmaConsumer";
                ConsumerName = ConsumerConfig.Name;
            }

            ConsumerConfigMap.Add(ConsumerConfig.Name, ConsumerConfig);

            // If the channelset is empty we're in the default connection state or HostName and Port have been specified, so create a default config, and set hostName and port if it's been configured.
            if (ConsumerConfig.ChannelSet.Count == 0)
            {
                tmpChannelConfig = new ClientChannelConfig();
                tmpChannelConfig.Name = "DefaultEmaChannel";
                // Both of these will be set as long as the application calls OmmConsumerConfig.Host().
                if (string.IsNullOrEmpty(HostName) == false && string.IsNullOrEmpty(Port) == false)
                {
                    tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = HostName;
                    tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = Port;
                }
                else
                {
                    tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = DefaultHost;
                    tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = DefaultPort;
                }

                // If the proxy is set on the top leve, override the proxy info set in each channel configuration.
                if (!string.IsNullOrEmpty(ProxyHost))
                    tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = ProxyHost;
                if (!string.IsNullOrEmpty(ProxyPort))
                    tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = ProxyPort;
                if (!string.IsNullOrEmpty(ProxyUserName))
                    tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName = ProxyUserName;
                if (!string.IsNullOrEmpty(ProxyPassword))
                    tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword = ProxyPassword;

                // Add the DefaultEmaChannel to the config map and to the ConsumerConfig's channelSet.
                ClientChannelConfigMap.Add(tmpChannelConfig.Name, tmpChannelConfig);
                ConsumerConfig.ChannelSet.Add(tmpChannelConfig.Name);
            }
            else   
            {
                // There are channels in the channelSet, so copy them all over. Don't need to add the name because it's already in there.
                foreach (string channelName in ConsumerConfig.ChannelSet)
                {
                    // If the channel name already exists in ClientChannelConfigMap, just continue, it's already been added. 
                    if (ClientChannelConfigMap.ContainsKey(channelName))
                    {
                        continue;
                    }

                    tmpChannelConfig = new ClientChannelConfig(OldConfigImpl.ClientChannelConfigMap[channelName]);

                    // If hostname and port are set, this should be the only channel in the list, so override the host, port, and connectionType config.
                    if (!string.IsNullOrEmpty(HostName) && !string.IsNullOrEmpty(Port))
                    {
                        tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = HostName;
                        tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = Port;
                        tmpChannelConfig.ConnectInfo.ConnectOptions.ConnectionType = Eta.Transports.ConnectionType.SOCKET;
                    }
                    else
                    {
                        if (tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address is not null)
                        {
                            tmpChannelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName ??= DefaultPort;
                        }
                    }

                    // If the proxy is set on the top leve, override the proxy info set in each channel configuration.
                    if (!string.IsNullOrEmpty(ProxyHost))
                        tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = ProxyHost;
                    if (!string.IsNullOrEmpty(ProxyPort))
                        tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = ProxyPort;
                    if (!string.IsNullOrEmpty(ProxyUserName))
                        tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName = ProxyUserName;
                    if (!string.IsNullOrEmpty(ProxyPassword))
                        tmpChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword = ProxyPassword;

                    ClientChannelConfigMap.Add(tmpChannelConfig.Name, tmpChannelConfig);
                }
            }

            LoggerConfig.Clear();
            
            if (string.IsNullOrEmpty(ConsumerConfig.Logger) == false)
            {
                // There's a configured logger config, so copy it over.
                OldConfigImpl.LoggerConfigMap[ConsumerConfig.Logger].Copy(LoggerConfig);
            }
            else
            {
                // LoggerConfig has alredy been initialized to defaults in the Clear() method.
                LoggerConfig.Name = "DefaultEmaLogger";
                ConsumerConfig.Logger = LoggerConfig.Name;
            }

            LoggerConfigMap.Add(LoggerConfig.Name, LoggerConfig);

            DictionaryConfig.Clear();
            
            if (!string.IsNullOrEmpty(ConsumerConfig.Dictionary))
            {
                // There's a configured dictionary config, so copy it over.
                OldConfigImpl.DictionaryConfigMap[ConsumerConfig.Dictionary].Copy(DictionaryConfig);
            }
            else
            {
                // DictionaryConfig has alredy been initialized to defaults in the Clear() method.
                DictionaryConfig.Name = "DefaultEmaDictionary";
                ConsumerConfig.Dictionary = DictionaryConfig.Name;
            }

            DictionaryConfigMap.Add(DictionaryConfig.Name, DictionaryConfig);
        }

        internal void Clear()
        {
            HostName = string.Empty;
            Port = string.Empty;
            ConsumerName = string.Empty;
            DispatchModel = (int)OmmConsumerConfig.OperationModelMode.API_DISPATCH;
            XmlConfigPath = string.Empty;
            XmlParser = null;
            DefaultConsumer = string.Empty;
            UserName = string.Empty;
            Password = string.Empty;
            Position = string.Empty;
            ApplicationId = string.Empty;
            ClientId = string.Empty;
            ClientSecret = string.Empty;
            ClientJwk = string.Empty;
            TokenUrlV2 = string.Empty;
            ServiceDiscoveryUrl = string.Empty;
            Audience = string.Empty;
            ProxyHost = string.Empty;
            ProxyPort = string.Empty;
            ProxyUserName = string.Empty;
            ProxyPassword = string.Empty;
            RestProxyHostName = string.Empty;
            RestProxyPort = string.Empty;
            RestProxyUserName = string.Empty;
            RestProxyPassword = string.Empty;
            ConsumerConfig.Clear();
            LoggerConfig.Clear();
            DictionaryConfig.Clear();
            ConsumerConfigMap.Clear();
            ClientChannelConfigMap.Clear();
            LoggerConfigMap.Clear();
            DictionaryConfigMap.Clear();
            ConfigErrorLog?.Clear();
            SetEncryptedProtocolFlags = false;
            CipherSuites = null;
            m_DataDictionary = null;
        }

        // Takes in a host name formatted in the following way:
        // ""(blank) => Default Hostname(localhost) and default port(14002)
        // [Hostname] => Configured Hostname and default port
        // [Hostname]: => Configured Hostname and default port
        // :[Port] => Default Hostname and Port
        // [Hostname]:[Port] => Configured Hostname and Configured Port
        // Throws OmmInvalidConfigurationException if the parse fails.
        internal void Host(string host) =>
            hostPortParser.StringToHostPort(host, out HostName, out Port);

        internal void OperationModel(int operationModel)
        {
            DispatchModel = operationModel;
        }

        internal void Config(Map configMap)
        {
            ProgrammaticConfigParser.ParseProgrammaticConsumerConfig(configMap, this);
        }

        // This method's creating temporary admin requests to ensure that they get deep copied into the OmmConsumerConfigImpl's structures.
        // This allows the user to reuse request messages.
        internal void AddAdminMsg(RequestMsg requestMsg)
        {
            DecodeIterator decodeIter;
            IRequestMsg msg;
            switch (requestMsg.m_requestMsgEncoder.m_rsslMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    LoginRequest tmpLoginRequest = new LoginRequest();

                    AdminLoginRequest.Clear();

                    requestMsg.EncodeComplete();

                    decodeIter = new DecodeIterator();

                    decodeIter.SetBufferAndRWFVersion(requestMsg.m_requestMsgEncoder?.m_encodeIterator?.Buffer(), Codec.MajorVersion(), Codec.MinorVersion());

                    msg = new Eta.Codec.Msg();

                    if (msg.Decode(decodeIter) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to decode the provided Login message, setting request to default values", LoggerLevel.ERROR);
                        AdminLoginRequest.InitDefaultRequest(1);
                        return;
                    }

                    if (tmpLoginRequest.Decode(decodeIter, (Eta.Codec.Msg)msg) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to decode the provided Login message, setting request to default values", LoggerLevel.ERROR);
                        AdminLoginRequest.InitDefaultRequest(1);
                        return;
                    }

                    if (tmpLoginRequest.Copy(AdminLoginRequest) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to copy the provided Login message, setting request to default values", LoggerLevel.ERROR);
                        AdminLoginRequest.InitDefaultRequest(1);
                        return;
                    }
                    break;
                case (int)DomainType.SOURCE:
                    DirectoryRequest tmpDirectoryRequest = new DirectoryRequest();
                    requestMsg.EncodeComplete();

                    decodeIter = new DecodeIterator();

                    decodeIter.SetBufferAndRWFVersion(requestMsg.m_requestMsgEncoder?.m_encodeIterator?.Buffer(), Codec.MajorVersion(), Codec.MinorVersion());

                    msg = new Eta.Codec.Msg();

                    if (msg.Decode(decodeIter) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to decode the provided Directory message, ignoring message", LoggerLevel.ERROR);
                        return;
                    }

                    // Check for missing defaults
                    if (!msg.MsgKey.CheckHasFilter() || msg.MsgKey.Filter == 0)
                    {
                        ConfigErrorLog?.Add("Provided Directory request does not have a filter set, or the filter is set to 0.  Setting the Filter to the default(all filters)", LoggerLevel.WARNING);
                        msg.MsgKey.ApplyHasFilter();
                        msg.MsgKey.Filter = ServiceFilterFlags.ALL_FILTERS;
                    }

                    if ((msg.Flags & RequestMsgFlags.STREAMING) == 0)
                    {
                        ConfigErrorLog?.Add("Provided Directory request does not have the STREAMING interaction set. Setting this to true. ", LoggerLevel.WARNING);
                        msg.Flags |= RequestMsgFlags.STREAMING;
                    }

                    AdminDirectoryRequest ??= new DirectoryRequest();
                    if (tmpDirectoryRequest.Decode(decodeIter, (Eta.Codec.Msg)msg) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to decode the provided Directory message, ignoring message", LoggerLevel.ERROR);
                        AdminDirectoryRequest = null;
                        return;
                    }

                    if (tmpDirectoryRequest.Copy(AdminDirectoryRequest) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add("Unable to copy the provided Directory message, ignoring message", LoggerLevel.ERROR);
                        AdminDirectoryRequest = null;
                        return;
                    }
                    break;
                case (int)DomainType.DICTIONARY:
                    if (!requestMsg.HasName)
                    {
                        ConfigErrorLog?.Add("Dictionary RequestMsg does not contain a dictionary name. Message ignored.", LoggerLevel.ERROR);
                        return;
                    }

                    if (!requestMsg.HasServiceId && !requestMsg.HasServiceName)
                    {
                        ConfigErrorLog?.Add("Dictionary RequestMsg does not contain a service name or serviceId. Message ignored.", LoggerLevel.ERROR);
                        return;
                    }

                    if (!requestMsg.HasFilter)
                    {
                        ConfigErrorLog?.Add("Dictionary RequestMsg does not contain a filter. Message ignored.", LoggerLevel.ERROR);
                        return;
                    }

                    if ((requestMsg.m_rsslMsg.Flags & RequestMsgFlags.NO_REFRESH) != 0)
                    {
                        ConfigErrorLog?.Add("Dictionary RequestMsg contains the NO_REFRESH flag. Message ignored.", LoggerLevel.ERROR);
                        return;
                    }

                    // Finish the encoding and decode the dictionary msg to a ETA RequestMsg
                    requestMsg.EncodeComplete();
                    DictionaryRequest tmpDictionaryRequest = new DictionaryRequest();
                    decodeIter = new DecodeIterator();

                    decodeIter.SetBufferAndRWFVersion(requestMsg.m_requestMsgEncoder?.m_encodeIterator?.Buffer(), Codec.MajorVersion(), Codec.MinorVersion());

                    msg = new Eta.Codec.Msg();

                    if (msg.Decode(decodeIter) != CodecReturnCode.SUCCESS)
                    {
                        ConfigErrorLog?.Add(
                            "Dictionary RequestMsg is unable to be decoded. Message ignored.",
                            LoggerLevel.ERROR);
                        AdminFieldDictionaryRequest = null;
                        return;
                    }

                    switch (msg.MsgKey.Name.ToString())
                    {
                        case "RWFFld":
                            AdminFieldDictionaryRequest ??= new DictionaryRequest();

                            if (tmpDictionaryRequest.Decode(decodeIter, (Eta.Codec.Msg)msg) !=
                                CodecReturnCode.SUCCESS)
                            {
                                ConfigErrorLog?.Add(
                                    "Dictionary RequestMsg is unable to be decoded. Message ignored.", LoggerLevel.ERROR);
                                AdminFieldDictionaryRequest = null;
                                return;
                            }

                            if (tmpDictionaryRequest.Copy(AdminFieldDictionaryRequest) != CodecReturnCode.SUCCESS)
                            {
                                ConfigErrorLog?.Add(
                                    "Dictionary RequestMsg cannot be copied. Message ignored.", LoggerLevel.ERROR);
                                AdminFieldDictionaryRequest = null;
                                return;
                            }

                            if (requestMsg.HasServiceName)
                            {
                                FieldDictionaryRequestServiceName = requestMsg.ServiceName();
                            }

                            break;

                        case "RWFEnum":
                            AdminEnumDictionaryRequest ??= new DictionaryRequest();

                            if (tmpDictionaryRequest.Decode(decodeIter, (Eta.Codec.Msg)msg) !=
                                CodecReturnCode.SUCCESS)
                            {
                                ConfigErrorLog?.Add(
                                    "Dictionary RequestMsg is unable to be decoded. Message ignored.",
                                    LoggerLevel.ERROR);
                                AdminEnumDictionaryRequest = null;
                                return;
                            }

                            if (tmpDictionaryRequest.Copy(AdminEnumDictionaryRequest) != CodecReturnCode.SUCCESS)
                            {
                                ConfigErrorLog?.Add(
                                    "Dictionary RequestMsg cannot be copied. Message ignored.", LoggerLevel.ERROR);
                                AdminFieldDictionaryRequest = null;
                                return;
                            }

                            if (requestMsg.HasServiceName)
                            {
                                EnumDictionaryRequestServiceName = requestMsg.ServiceName();
                            }

                            break;

                        default:
                            ConfigErrorLog?.Add(
                                "Dictionary RequestMsg contains an unrecognized dictionary name. Message ignored.",
                                LoggerLevel.ERROR);
                            break;
                    }
                    break;
                default:
                    throw new OmmInvalidConfigurationException("Non-supported domain in request message passed to AddAdminMsg. Supported request message domains are: LSEG.Eta.Rdm.DomainType.LOGIN, LSEG.Eta.Rdm.DomainType.SOURCE, or LSEG.Eta.Rdm.DomainType.DICTIONARY.");

            }
        }

        // Iterates through the consumer config and determines everything is correct:
        // The Default Consumer is present
        // All Channels referenced by the Consumers are present
        // All Channels referenced in warm standby(when implemented) are present
        // All Loggers referenced by the consumers are present
        // All Dictionaries referenced by the consumers are present
        internal void VerifyConfiguration()
        {
            // First, if there's a default consumer, verify that it exists in the consumerConfigMap
            if (string.IsNullOrEmpty(ConsumerName) == false)
            {
                if (ConsumerConfigMap.ContainsKey(ConsumerName) == false)
                {
                    throw new OmmInvalidConfigurationException("Consumer " + ConsumerName + " set by OmmConsumerConfig.ConsumerName is not defined in this OmmConsumerConfig");
                }
            }

            if (string.IsNullOrEmpty(DefaultConsumer) == false)
            {
                if (ConsumerConfigMap.ContainsKey(DefaultConsumer) == false)
                {
                    throw new OmmInvalidConfigurationException("Default consumer " + DefaultConsumer + " is not defined in this OmmConsumerConfig");
                }
            }

            // Now iterate through all of the consumers and verify that the Channel, Logger, and Dictionary configs are present in the config database
            foreach (ConsumerConfig consumer in ConsumerConfigMap.Values)
            {
                if (consumer.ChannelSet.Count != 0)
                {
                    foreach (string channelName in consumer.ChannelSet)
                    {
                        if (ClientChannelConfigMap.ContainsKey(channelName) == false)
                        {
                            throw new OmmInvalidConfigurationException("Channel " + channelName + " in Consumer " + consumer.Name + " is not defined in this OmmConsumerConfig");
                        }
                    }
                }

                if (string.IsNullOrEmpty(consumer.Logger) == false)
                {
                    if (LoggerConfigMap.ContainsKey(consumer.Logger) == false)
                    {
                        throw new OmmInvalidConfigurationException("Logger " + consumer.Logger + " in Consumer " + consumer.Name + " is not defined in this OmmConsumerConfig");
                    }
                }

                // Application can provide the DataDictionary directly with an API method call,
                // verify configuration only when that's not the case
                if (m_DataDictionary is null
                    && !string.IsNullOrEmpty(consumer.Dictionary)
                    && !DictionaryConfigMap.ContainsKey(consumer.Dictionary))
                {
                    throw new OmmInvalidConfigurationException("Dictionary " + consumer.Dictionary + " in Consumer " + consumer.Name + " is not defined in this OmmConsumerConfig");
                }
            }
        }

        // Generated the Consumer Role based on the Consumer Config.
        // Prerequsites: The user-supplied OmmConsumerConfig has been verified with OmmConsumerConfigImpl.VerifyConfig and this is run on the "active" configuration copy
        internal ConsumerRole GenerateConsumerRole()
        {
            ConsumerRole role = new ConsumerRole();

            ConsumerConfig consConfig = ConsumerConfigMap[ConsumerName];
            DictionaryConfig dictConfig = DictionaryConfigMap[consConfig.Dictionary];

            role.FieldDictionaryName.Data(dictConfig.RdmFieldDictionaryItemName);
            role.EnumTypeDictionaryName.Data(dictConfig.EnumTypeDefItemName);

            AdminLoginRequest ??= new LoginRequest();
           
            // Setup the Login name if UserName, Password or position are set
            if (!string.IsNullOrEmpty(UserName))
            {
                AdminLoginRequest.UserName.Data(UserName);
            }
            else if (AdminLoginRequest.UserName.Length == 0)
            {
                try
                {
                    AdminLoginRequest.UserName.Data(Environment.UserName);
                }
                catch (Exception)
                {
                    AdminLoginRequest.UserName.Data("ema");
                }
            }

            if (!string.IsNullOrEmpty(Password))
            {
                AdminLoginRequest.HasAttrib = true;
                AdminLoginRequest.Flags |= LoginRequestFlags.HAS_PASSWORD;
                AdminLoginRequest.Password.Data(Password);
            }
            if (!string.IsNullOrEmpty(Position))
            {
                AdminLoginRequest.HasAttrib = true;
                AdminLoginRequest.LoginAttrib.Flags |= LoginAttribFlags.HAS_POSITION;
                AdminLoginRequest.LoginAttrib.Position.Data(Position);
            }

            if (!string.IsNullOrEmpty(ApplicationId))
            {
                AdminLoginRequest.HasAttrib = true;
                AdminLoginRequest.LoginAttrib.Flags |= LoginAttribFlags.HAS_APPLICATION_ID;
                AdminLoginRequest.LoginAttrib.ApplicationId.Data(ApplicationId);
            }

            if (consConfig.EnableRtt)
            {
                AdminLoginRequest.HasAttrib = true;
                AdminLoginRequest.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
            }


            AdminLoginRequest.StreamId = 1; // Set the stream ID for login domain
            role.RdmLoginRequest = AdminLoginRequest;
            role.RdmDirectoryRequest = AdminDirectoryRequest;
            role.RdmEnumTypeDictionaryRequest = AdminEnumDictionaryRequest;
            role.RdmFieldDictionaryRequest = AdminFieldDictionaryRequest;

            if (!string.IsNullOrEmpty(ClientId) && (!string.IsNullOrEmpty(ClientSecret) || !string.IsNullOrEmpty(ClientJwk)))
            {
                ReactorOAuthCredential credentials = new ReactorOAuthCredential();
                if (!string.IsNullOrEmpty(ClientId))
                    credentials.ClientId.Data(ClientId);

                if (!string.IsNullOrEmpty(ClientSecret))
                    credentials.ClientSecret.Data(ClientSecret);

                if (!string.IsNullOrEmpty(ClientJwk))
                    credentials.ClientJwk.Data(ClientJwk);

                if (!string.IsNullOrEmpty(Audience))
                    credentials.Audience.Data(Audience);

                if (!string.IsNullOrEmpty(TokenScope))
                    credentials.TokenScope.Data(TokenScope);

                role.ReactorOAuthCredential = credentials;

                // Clear out the ClientSecret and ClientJWK here.
                ClientSecret = string.Empty;
                ClientJwk = string.Empty;
            }

            ConsumerWatchlistOptions watchlistOptions = role.WatchlistOptions;

            watchlistOptions.EnableWatchlist = true;
            watchlistOptions.ItemCountHint = consConfig.ItemCountHint;
            watchlistOptions.ObeyOpenWindow = consConfig.ObeyOpenWindow;
            watchlistOptions.PostAckTimeout = consConfig.PostAckTimeout;
            watchlistOptions.RequestTimeout = consConfig.RequestTimeout;
            watchlistOptions.MaxOutstandingPosts = consConfig.MaxOutstandingPosts;

            return role;
        }

        // Generates the reactor connect options based on the Consumer Config.
        // Prerequsites: The user-supplied OmmConsumerConfig has been verified with OmmConsumerConfigImpl.VerifyConfig and this is run on the "active" configuration copy
        public override ReactorConnectOptions GenerateReactorConnectOpts()
        {
            ReactorConnectOptions connOpts = new ReactorConnectOptions();
            ConsumerConfig consConfig = ConsumerConfigMap[ConsumerName];

            connOpts.SetReconnectMaxDelay(consConfig.ReconnectMaxDelay);
            connOpts.SetReconnectMinDelay(consConfig.ReconnectMinDelay);
            connOpts.SetReconnectAttempLimit(consConfig.ReconnectAttemptLimit);

            foreach(string chnlName in consConfig.ChannelSet)
            {
                ClientChannelConfig chnlConfig = ClientChannelConfigMap[chnlName];
                connOpts.ConnectionList.Add(chnlConfig.ConnectInfo);
            }

            return connOpts;
        }

        private Ema.Rdm.DataDictionary? m_DataDictionary = null;

        internal void DataDictionary(Ema.Rdm.DataDictionary dataDictionary, bool shouldCopyIntoAPI)
        {
            if (dataDictionary is not null
                && dataDictionary.IsFieldDictionaryLoaded
                && dataDictionary.IsEnumTypeDefLoaded)
            {
                if (shouldCopyIntoAPI)
                {
                    m_DataDictionary = new Ema.Rdm.DataDictionary(dataDictionary);
                }
                else
                {
                    m_DataDictionary = dataDictionary;
                }
            }
            else
            {
                throw new OmmInvalidUsageException("The dictionary information is not fully loaded in the passed DataDictionary object.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        internal Ema.Rdm.DataDictionary? DataDictionary()
        {
            return m_DataDictionary;
        }

        // Populates Reactor Options based on the Consumer Config.
        // Prerequsites: The user-supplied OmmConsumerConfig has been verified with OmmConsumerConfigImpl.VerifyConfig and this is run on the "active" configuration copy
        internal void PopulateReactorOptions(ReactorOptions reactorOptions)
        {
            // Set the REST logging options in the consumer here
            if (ConsumerConfig.RestEnableLog)
            {
                reactorOptions.EnableRestLogStream = true;

                if (string.IsNullOrEmpty(ConsumerConfig.RestLogFileName) == false)
                {
                    try
                    {
                        reactorOptions.RestLogOutputStream = File.OpenWrite(ConsumerConfig.RestLogFileName);
                    }
                    catch (SystemException excp)
                    {
                        throw new OmmInvalidConfigurationException("Unable to open file for REST logging. Exception message: " + excp.Message);
                    }
                }
            }

            if (string.IsNullOrEmpty(TokenUrlV2) == false)
            {
                reactorOptions.SetTokenServiceURL(TokenUrlV2);
            }

            if (string.IsNullOrEmpty(ServiceDiscoveryUrl) == false)
            {
                reactorOptions.SetServiceDiscoveryURL(ServiceDiscoveryUrl);
            }

            var rPOpt = reactorOptions.RestProxyOptions;

            // Values from XML & programmatic config
            if (string.IsNullOrEmpty(ConsumerConfig.RestProxyHostName) == false)
            {
                rPOpt.ProxyHostName = ConsumerConfig.RestProxyHostName;
            }
            if (string.IsNullOrEmpty(ConsumerConfig.RestProxyPort) == false)
            {
                rPOpt.ProxyPort = ConsumerConfig.RestProxyPort;
            }

            // Values from fluent interface
            if (string.IsNullOrEmpty(RestProxyHostName) == false)
            {
                rPOpt.ProxyHostName = RestProxyHostName;
            }
            if (string.IsNullOrEmpty(RestProxyPort) == false)
            {
                rPOpt.ProxyPort = RestProxyPort;
            }
            if (string.IsNullOrEmpty(RestProxyUserName) == false)
            {
                rPOpt.ProxyUserName = RestProxyUserName;
            }
            if (string.IsNullOrEmpty(RestProxyPassword) == false)
            {
                rPOpt.ProxyPassword = RestProxyPassword;
            }

            int restRequsetTimeout = (int)ConsumerConfig.RestRequestTimeOut;
            if (reactorOptions.SetRestRequestTimeout(restRequsetTimeout) != ReactorReturnCode.SUCCESS)
            {
                throw new OmmInvalidConfigurationException($"Failed to set REST request timeout with value: {restRequsetTimeout}");
            }
        }
    }
}
